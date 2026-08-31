import UIKit
import CoreMotion

final class EmulationViewController: UIViewController, UIAdaptivePresentationControllerDelegate {
    private var rom: Rom
    private let emulator = GearboyEmulator()
    private let backgroundView = GameplayBackgroundView(frame: .zero)
    private let screenView = GameScreenView(frame: .zero)
    private let controlsView = GameControlsView(frame: .zero)
    private let closeButton = UIButton(type: .system)
    private let menuButton = UIButton(type: .system)
    private let motionManager = CMMotionManager()
    private let gameControllerInput = GameControllerInput()
    private var displayLink: CADisplayLink?
    private var previousTargetTimestamp: CFTimeInterval?
    private var frameAccumulator = 0.0
    private var contentFrameRate = 60.0
    private var isVisible = false
    private var hasLoadedROM = false
    private var gameplayMenuPresented = false
    private var screenAspectRatio: CGFloat = 160.0 / 144.0
    private var nativeScreenWidth: CGFloat = 160.0
    private var nativeScreenHeight: CGFloat = 144.0

    init(rom: Rom) {
        self.rom = rom
        super.init(nibName: nil, bundle: nil)
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        view.backgroundColor = UIColor(red: 0.15, green: 0.17, blue: 0.19, alpha: 1.0)

        configureViews()
        configureLifecycleObservers()
        loadROM()
    }

    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        isVisible = true
        startEmulation()
    }

    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        isVisible = false
        stopEmulation(save: true)
        displayLink?.invalidate()
        displayLink = nil
    }

    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        controlsView.layoutIfNeeded()
        layoutScreen()
    }

    override var prefersHomeIndicatorAutoHidden: Bool {
        true
    }

    override var prefersStatusBarHidden: Bool {
        true
    }

    deinit {
        NotificationCenter.default.removeObserver(self)
        displayLink?.invalidate()
        motionManager.stopAccelerometerUpdates()
        gameControllerInput.stop()
        emulator.saveRAM()
    }

    private func configureViews() {
        backgroundView.translatesAutoresizingMaskIntoConstraints = false
        controlsView.translatesAutoresizingMaskIntoConstraints = false
        closeButton.translatesAutoresizingMaskIntoConstraints = false
        menuButton.translatesAutoresizingMaskIntoConstraints = false
        emulator.configure(
            model: AppSettings.gameBoyModel.rawValue,
            mapper: AppSettings.mapper.rawValue,
            palette: AppSettings.dmgPalette.rawValue,
            colorCorrection: AppSettings.colorCorrectionEnabled,
            noSpriteLimit: AppSettings.noSpriteLimitEnabled,
            superGameBoy: AppSettings.superGameBoyEnabled,
            superGameBoyBorder: AppSettings.superGameBoyBorderEnabled,
            saveStateSlot: AppSettings.saveStateSlot
        )
        emulator.isMuted = !AppSettings.audioEnabled
        screenView.isSmoothingEnabled = AppSettings.smoothingEnabled
        controlsView.hapticsEnabled = AppSettings.hapticsEnabled
        controlsView.onButtonChanged = { [weak self] button, pressed in
            self?.emulator.setButton(button, pressed: pressed)
        }
        gameControllerInput.onButtonChanged = { [weak self] button, pressed in
            guard let self, self.isVisible, !self.gameplayMenuPresented,
                  let emulatorButton = self.emulatorButton(for: button) else { return }
            self.emulator.setButton(emulatorButton, pressed: pressed)
        }
        gameControllerInput.start()

        configureOverlayButton(closeButton, systemImage: "xmark", accessibilityLabel: L10n("Gameplay::Close"))
        configureOverlayButton(menuButton, systemImage: "ellipsis", accessibilityLabel: L10n("Gameplay::Options"))
        closeButton.addTarget(self, action: #selector(closeGameplay), for: .touchUpInside)
        menuButton.addTarget(self, action: #selector(showMenu), for: .touchUpInside)

        view.addSubview(backgroundView)
        view.addSubview(screenView)
        view.addSubview(controlsView)
        view.addSubview(closeButton)
        view.addSubview(menuButton)

        NSLayoutConstraint.activate([
            backgroundView.topAnchor.constraint(equalTo: view.topAnchor),
            backgroundView.bottomAnchor.constraint(equalTo: view.bottomAnchor),
            backgroundView.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            backgroundView.trailingAnchor.constraint(equalTo: view.trailingAnchor),

            controlsView.topAnchor.constraint(equalTo: view.topAnchor),
            controlsView.bottomAnchor.constraint(equalTo: view.bottomAnchor),
            controlsView.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            controlsView.trailingAnchor.constraint(equalTo: view.trailingAnchor),

            closeButton.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 8.0),
            closeButton.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor, constant: 12.0),
            closeButton.widthAnchor.constraint(equalToConstant: 44.0),
            closeButton.heightAnchor.constraint(equalTo: closeButton.widthAnchor),

            menuButton.topAnchor.constraint(equalTo: closeButton.topAnchor),
            menuButton.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor, constant: -12.0),
            menuButton.widthAnchor.constraint(equalTo: closeButton.widthAnchor),
            menuButton.heightAnchor.constraint(equalTo: closeButton.heightAnchor)
        ])
    }

    private func configureOverlayButton(_ button: UIButton, systemImage: String, accessibilityLabel: String) {
        var configuration = UIButton.Configuration.gray()
        configuration.image = UIImage(systemName: systemImage)
        configuration.baseForegroundColor = .label
        configuration.cornerStyle = .capsule
        button.configuration = configuration
        button.accessibilityLabel = accessibilityLabel
    }

    private func layoutScreen() {
        let safeFrame = view.safeAreaLayoutGuide.layoutFrame
        let landscape = view.bounds.width > view.bounds.height
        let availableFrame: CGRect

        if landscape {
            let left = controlsView.dPad.frame.maxX + 12.0
            let right = controlsView.actionB.frame.minX - 12.0
            let horizontalInset = max(left, view.bounds.width - right)
            availableFrame = CGRect(
                x: horizontalInset,
                y: safeFrame.minY + 8.0,
                width: view.bounds.width - (horizontalInset * 2.0),
                height: safeFrame.height - 16.0
            )
        } else {
            let top = closeButton.frame.maxY + 8.0
            let bottom = controlsView.dPad.frame.minY - 18.0
            availableFrame = CGRect(
                x: safeFrame.minX + 12.0,
                y: top,
                width: safeFrame.width - 24.0,
                height: bottom - top
            )
        }

        guard availableFrame.width > 0.0, availableFrame.height > 0.0 else { return }

        let displayScale = max(traitCollection.displayScale, 1.0)
        var screenSize: CGSize

        switch AppSettings.screenSize {
        case .fitToWidth:
            let requestedWidth = landscape ? availableFrame.width : view.bounds.width
            let width = min(requestedWidth, availableFrame.height * screenAspectRatio)
            screenSize = CGSize(width: width, height: width / screenAspectRatio)
        case .integerScale:
            let horizontalScale = availableFrame.width * displayScale / nativeScreenWidth
            let verticalScale = availableFrame.height * displayScale / nativeScreenHeight
            let scale = max(floor(min(horizontalScale, verticalScale)), 1.0)
            screenSize = CGSize(
                width: nativeScreenWidth * scale / displayScale,
                height: nativeScreenHeight * scale / displayScale
            )
        }

        let originXValue = availableFrame.midX - (screenSize.width * 0.5)
        let originX = (originXValue * displayScale).rounded() / displayScale
        let originYValue = landscape
            ? availableFrame.midY - (screenSize.height * 0.5)
            : availableFrame.minY
        let originY = (originYValue * displayScale).rounded() / displayScale
        screenView.frame = CGRect(origin: CGPoint(x: originX, y: originY), size: screenSize)
    }

    private func configureLifecycleObservers() {
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(applicationWillResignActive),
            name: UIApplication.willResignActiveNotification,
            object: nil
        )
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(applicationDidBecomeActive),
            name: UIApplication.didBecomeActiveNotification,
            object: nil
        )
    }

    private func loadROM() {
        let url = PathUtils.getDataDir.appendingPathComponent(rom.file)

        do {
            try emulator.loadROM(at: url)
            hasLoadedROM = true
            controlsView.isUserInteractionEnabled = true
            markROMAsPlayed()

            updateScreenGeometry()
            screenView.display(
                frameBuffer: emulator.frameBuffer,
                width: emulator.frameWidth,
                height: emulator.frameHeight
            )
        } catch {
            hasLoadedROM = false
            controlsView.isUserInteractionEnabled = false
            presentLoadError(error)
        }
    }

    private func markROMAsPlayed() {
        rom.usedOn = Date()
        if let updatedROM = dataStore.update(rom) {
            rom = updatedROM
        }
    }

    private func startEmulation() {
        guard hasLoadedROM, isVisible, !gameplayMenuPresented else { return }

        emulator.resume()
        emulator.startAudio()
        startMotionInput()
        UIApplication.shared.isIdleTimerDisabled = true

        previousTargetTimestamp = nil
        contentFrameRate = max(emulator.framesPerSecond, 1.0)
        frameAccumulator = 1.0

        if displayLink == nil {
            let link = CADisplayLink(target: self, selector: #selector(step(_:)))
            link.preferredFrameRateRange = CAFrameRateRange(minimum: 50.0, maximum: 120.0, preferred: 120.0)
            link.add(to: .main, forMode: .common)
            displayLink = link
        }

        displayLink?.isPaused = false
    }

    private func stopEmulation(save: Bool) {
        displayLink?.isPaused = true
        previousTargetTimestamp = nil
        frameAccumulator = 0.0
        emulator.pause()
        emulator.stopAudio()
        gameControllerInput.releaseAllButtons()
        stopMotionInput()

        if save {
            emulator.saveRAM()
        }

        if UIApplication.shared.isIdleTimerDisabled {
            UIApplication.shared.isIdleTimerDisabled = false
        }
    }

    @objc private func step(_ link: CADisplayLink) {
        guard hasLoadedROM else { return }

        updateMotionInput()

        let frameRate = max(emulator.framesPerSecond, 1.0)
        if abs(frameRate - contentFrameRate) > 0.000001 {
            frameAccumulator *= frameRate / contentFrameRate
            contentFrameRate = frameRate
        }

        if let previousTargetTimestamp {
            let elapsed = min(
                max(link.targetTimestamp - previousTargetTimestamp, 0.0),
                3.0 / contentFrameRate
            )
            frameAccumulator += elapsed * contentFrameRate
        }
        self.previousTargetTimestamp = link.targetTimestamp

        var framesRun = 0
        while frameAccumulator + 0.000001 >= 1.0 && framesRun < 3 {
            emulator.runFrame()
            frameAccumulator -= 1.0
            if frameAccumulator < 0.0 {
                frameAccumulator = 0.0
            }
            framesRun += 1
        }

        if framesRun > 0 {
            updateScreenGeometry()
            screenView.display(
                frameBuffer: emulator.frameBuffer,
                width: emulator.frameWidth,
                height: emulator.frameHeight
            )
        }
    }

    private func updateScreenGeometry() {
        let width = max(emulator.frameWidth, 1)
        let height = max(emulator.frameHeight, 1)
        let aspectRatio = CGFloat(width) / CGFloat(height)
        guard CGFloat(width) != nativeScreenWidth || CGFloat(height) != nativeScreenHeight ||
              abs(aspectRatio - screenAspectRatio) > 0.0001 else { return }

        screenAspectRatio = aspectRatio
        nativeScreenWidth = CGFloat(width)
        nativeScreenHeight = CGFloat(height)
        view.setNeedsLayout()
    }

    private func emulatorButton(for button: GameControllerInput.Button) -> GearboyButton? {
        switch button {
        case .up: return .up
        case .down: return .down
        case .left: return .left
        case .right: return .right
        case .a: return .A
        case .b: return .B
        case .menu: return .start
        case .options: return .select
        default: return nil
        }
    }

    private func startMotionInput() {
        guard AppSettings.motionTiltEnabled, emulator.isTiltGame, motionManager.isAccelerometerAvailable else {
            emulator.setAccelerometer(x: 0.0, y: 0.0)
            return
        }

        guard !motionManager.isAccelerometerActive else { return }

        motionManager.accelerometerUpdateInterval = 1.0 / max(emulator.framesPerSecond, 1.0)
        motionManager.startAccelerometerUpdates()
    }

    private func updateMotionInput() {
        guard motionManager.isAccelerometerActive,
              let acceleration = motionManager.accelerometerData?.acceleration else { return }

        var horizontal = acceleration.x

        switch view.window?.windowScene?.interfaceOrientation {
        case .portraitUpsideDown:
            horizontal = -acceleration.x
        case .landscapeLeft:
            horizontal = -acceleration.y
        case .landscapeRight:
            horizontal = acceleration.y
        default:
            break
        }

        var vertical = acceleration.z

        if abs(horizontal) < 0.05 {
            horizontal = 0.0
        }
        if abs(vertical) < 0.05 {
            vertical = 0.0
        }

        horizontal *= Double(AppSettings.tiltSensitivityX) / 5.0
        vertical *= Double(AppSettings.tiltSensitivityY) / 5.0

        if AppSettings.tiltInvertX {
            horizontal = -horizontal
        }
        if AppSettings.tiltInvertY {
            vertical = -vertical
        }

        horizontal = min(max(horizontal, -4.0), 4.0)
        vertical = min(max(vertical, -4.0), 4.0)
        emulator.setAccelerometer(x: horizontal, y: vertical)
    }

    private func stopMotionInput() {
        motionManager.stopAccelerometerUpdates()
        emulator.setAccelerometer(x: 0.0, y: 0.0)
    }

    @objc private func applicationWillResignActive() {
        if isVisible {
            stopEmulation(save: true)
        }
    }

    @objc private func applicationDidBecomeActive() {
        if isVisible {
            startEmulation()
        }
    }

    @objc private func closeGameplay() {
        dismiss(animated: true)
    }

    @objc private func showMenu() {
        guard !gameplayMenuPresented else { return }
        pauseForGameplayMenu()

        let alert = UIAlertController(
            title: L10n(AppConfiguration.libraryTitleLocalizationKey),
            message: nil,
            preferredStyle: .actionSheet
        )

        alert.addAction(UIAlertAction(title: L10n("Gameplay::Reset"), style: .default) { [weak self] _ in
            guard let self else { return }
            self.emulator.reset()
            self.resumeAfterGameplayMenu()
        })
        alert.addAction(UIAlertAction(title: L10n("Gameplay::SaveState"), style: .default) { [weak self] _ in
            guard let self else { return }
            self.emulator.saveState()
            self.resumeAfterGameplayMenu()
        })
        alert.addAction(UIAlertAction(title: L10n("Gameplay::LoadState"), style: .default) { [weak self] _ in
            guard let self else { return }
            self.emulator.loadState()
            self.resumeAfterGameplayMenu()
        })

        let muteTitle = emulator.isMuted ? L10n("Gameplay::EnableAudio") : L10n("Gameplay::MuteAudio")
        alert.addAction(UIAlertAction(title: muteTitle, style: .default) { [weak self] _ in
            guard let self else { return }
            self.emulator.isMuted.toggle()
            AppSettings.audioEnabled = !self.emulator.isMuted
            self.resumeAfterGameplayMenu()
        })

        let favoriteTitle = rom.isFavorite ? L10n("Library::RemoveFavorite") : L10n("Library::AddFavorite")
        alert.addAction(UIAlertAction(title: favoriteTitle, style: .default) { [weak self] _ in
            guard let self else { return }
            self.rom.isFavorite.toggle()
            if let updatedROM = dataStore.update(self.rom) {
                self.rom = updatedROM
            }
            self.resumeAfterGameplayMenu()
        })
        alert.addAction(UIAlertAction(title: L10n("Common::Cancel"), style: .cancel) { [weak self] _ in
            self?.resumeAfterGameplayMenu()
        })
        alert.popoverPresentationController?.sourceView = menuButton
        alert.popoverPresentationController?.sourceRect = menuButton.bounds
        present(alert, animated: true)
        alert.presentationController?.delegate = self
    }

    private func pauseForGameplayMenu() {
        gameplayMenuPresented = true
        displayLink?.isPaused = true
        previousTargetTimestamp = nil
        frameAccumulator = 0.0
        gameControllerInput.releaseAllButtons()
        emulator.pause()
    }

    private func resumeAfterGameplayMenu() {
        guard gameplayMenuPresented else { return }
        gameplayMenuPresented = false
        startEmulation()
    }

    func presentationControllerDidDismiss(_ presentationController: UIPresentationController) {
        resumeAfterGameplayMenu()
    }

    private func presentLoadError(_ error: Error) {
        let alert = UIAlertController(
            title: L10n("Gameplay::UnableToLoad"),
            message: error.localizedDescription,
            preferredStyle: .alert
        )
        alert.addAction(UIAlertAction(title: L10n("Common::OK"), style: .default))
        present(alert, animated: true)
    }
}
