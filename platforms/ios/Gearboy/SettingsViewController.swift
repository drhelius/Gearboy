import UIKit
import Combine

final class SettingsViewController: UITableViewController {
    private enum Section: Int, CaseIterable {
        case gameplay
        case system
        case video
        case input
        case library
        case about
    }

    private enum GameplayRow: Int, CaseIterable {
        case audio
        case saveStateSlot
    }

    private enum SystemRow: Int, CaseIterable {
        case model
        case mapper
        case superGameBoy
        case superGameBoyBorder
    }

    private enum VideoRow: Int, CaseIterable {
        case palette
        case colorCorrection
        case noSpriteLimit
        case screenSize
        case smoothing
    }

    private enum InputRow: Int, CaseIterable {
        case haptics
        case motionTilt
        case sensitivityX
        case sensitivityY
        case invertX
        case invertY
    }

    private enum LibraryRow: Int, CaseIterable {
        case refresh
        case importedRoms
    }

    private var dataStoreSubscriber: AnyCancellable?

    init() {
        super.init(style: .insetGrouped)
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        title = L10n("Common::Settings")
        navigationItem.largeTitleDisplayMode = .always
        navigationController?.navigationBar.prefersLargeTitles = true

        dataStoreSubscriber = dataStore.$allRoms
            .removeDuplicates { $0.count == $1.count }
            .receive(on: RunLoop.main)
            .sink { [weak self] _ in
                self?.tableView.reloadData()
            }
    }

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        tableView.reloadData()
    }

    override func numberOfSections(in tableView: UITableView) -> Int {
        Section.allCases.count
    }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch Section(rawValue: section) {
        case .gameplay:
            return GameplayRow.allCases.count
        case .system:
            return SystemRow.allCases.count
        case .video:
            return VideoRow.allCases.count
        case .input:
            return InputRow.allCases.count
        case .library:
            return LibraryRow.allCases.count
        case .about:
            return 1
        case nil:
            return 0
        }
    }

    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch Section(rawValue: section) {
        case .gameplay:
            return L10n("Settings::Gameplay")
        case .system:
            return L10n("Settings::System")
        case .video:
            return L10n("Settings::Video")
        case .input:
            return L10n("Settings::Input")
        case .library:
            return L10n("Settings::Library")
        case .about:
            return L10n("Settings::About")
        case nil:
            return nil
        }
    }

    override func tableView(_ tableView: UITableView, titleForFooterInSection section: Int) -> String? {
        switch Section(rawValue: section) {
        case .gameplay:
            return L10n("Settings::GameplayFooter")
        case .system:
            return L10n("Settings::SystemFooter")
        case .video:
            return L10n("Settings::VideoFooter")
        case .input:
            return L10n("Settings::InputFooter")
        default:
            return nil
        }
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        guard let section = Section(rawValue: indexPath.section) else {
            return UITableViewCell()
        }

        switch section {
        case .gameplay:
            return gameplayCell(row: indexPath.row)
        case .system:
            return systemCell(row: indexPath.row)
        case .video:
            return videoCell(row: indexPath.row)
        case .input:
            return inputCell(row: indexPath.row)
        case .library:
            return libraryCell(row: indexPath.row)
        case .about:
            return aboutCell()
        }
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        tableView.deselectRow(at: indexPath, animated: true)

        guard let section = Section(rawValue: indexPath.section) else { return }

        switch section {
        case .gameplay where GameplayRow(rawValue: indexPath.row) == .saveStateSlot:
            showOptions(
                title: L10n("Settings::SaveStateSlot"),
                options: (1...5).map { String(format: L10n("Settings::SlotFormat"), $0) },
                selectedIndex: AppSettings.saveStateSlot - 1
            ) { AppSettings.saveStateSlot = $0 + 1 }
        case .system where SystemRow(rawValue: indexPath.row) == .model:
            showOptions(
                title: L10n("Settings::GameBoyModel"),
                options: GameBoyModelOption.allCases.map(\.title),
                selectedIndex: AppSettings.gameBoyModel.rawValue
            ) { index in
                AppSettings.gameBoyModel = GameBoyModelOption(rawValue: index) ?? .automatic
            }
        case .system where SystemRow(rawValue: indexPath.row) == .mapper:
            showOptions(
                title: L10n("Settings::Mapper"),
                options: MapperOption.allCases.map(\.title),
                selectedIndex: AppSettings.mapper.rawValue
            ) { index in
                AppSettings.mapper = MapperOption(rawValue: index) ?? .automatic
            }
        case .video where VideoRow(rawValue: indexPath.row) == .palette:
            showOptions(
                title: L10n("Settings::DMGPalette"),
                options: DMGPaletteOption.allCases.map(\.title),
                selectedIndex: AppSettings.dmgPalette.rawValue
            ) { index in
                AppSettings.dmgPalette = DMGPaletteOption(rawValue: index) ?? .original
            }
        case .video where VideoRow(rawValue: indexPath.row) == .screenSize:
            showOptions(
                title: L10n("Settings::ScreenSize"),
                options: ScreenSizeOption.allCases.map(\.title),
                selectedIndex: AppSettings.screenSize.rawValue
            ) { AppSettings.screenSize = ScreenSizeOption(rawValue: $0) ?? .fitToWidth }
        case .input where InputRow(rawValue: indexPath.row) == .sensitivityX && AppSettings.motionTiltEnabled:
            showSensitivityOptions(title: L10n("Settings::HorizontalSensitivity"), currentValue: AppSettings.tiltSensitivityX) {
                AppSettings.tiltSensitivityX = $0
            }
        case .input where InputRow(rawValue: indexPath.row) == .sensitivityY && AppSettings.motionTiltEnabled:
            showSensitivityOptions(title: L10n("Settings::VerticalSensitivity"), currentValue: AppSettings.tiltSensitivityY) {
                AppSettings.tiltSensitivityY = $0
            }
        case .library where LibraryRow(rawValue: indexPath.row) == .refresh:
            dataStore.updateAll()
        default:
            break
        }
    }

    private func gameplayCell(row: Int) -> UITableViewCell {
        switch GameplayRow(rawValue: row) {
        case .audio:
            return toggleCell(
                title: L10n("Settings::Audio"),
                detail: L10n("Settings::AudioDetail"),
                image: "speaker.wave.2",
                isOn: AppSettings.audioEnabled,
                action: #selector(audioChanged(_:))
            )
        case .saveStateSlot:
            return optionCell(
                title: L10n("Settings::SaveStateSlot"),
                value: String(format: L10n("Settings::SlotFormat"), AppSettings.saveStateSlot),
                image: "square.stack.3d.up"
            )
        case nil:
            return UITableViewCell()
        }
    }

    private func systemCell(row: Int) -> UITableViewCell {
        switch SystemRow(rawValue: row) {
        case .model:
            return optionCell(
                title: L10n("Settings::GameBoyModel"),
                value: AppSettings.gameBoyModel.summaryTitle,
                image: "gamecontroller"
            )
        case .mapper:
            return optionCell(
                title: L10n("Settings::Mapper"),
                value: AppSettings.mapper.title,
                image: "memorychip"
            )
        case .superGameBoy:
            return toggleCell(
                title: L10n("Settings::SuperGameBoy"),
                detail: L10n("Settings::SuperGameBoyDetail"),
                image: "rectangle.on.rectangle",
                isOn: AppSettings.superGameBoyEnabled,
                action: #selector(superGameBoyChanged(_:))
            )
        case .superGameBoyBorder:
            return toggleCell(
                title: L10n("Settings::SuperGameBoyBorder"),
                detail: L10n("Settings::SuperGameBoyBorderDetail"),
                image: "rectangle.inset.filled",
                isOn: AppSettings.superGameBoyBorderEnabled,
                action: #selector(superGameBoyBorderChanged(_:)),
                isEnabled: AppSettings.superGameBoyEnabled
            )
        case nil:
            return UITableViewCell()
        }
    }

    private func videoCell(row: Int) -> UITableViewCell {
        switch VideoRow(rawValue: row) {
        case .palette:
            return optionCell(
                title: L10n("Settings::DMGPalette"),
                value: AppSettings.dmgPalette.title,
                image: "paintpalette"
            )
        case .colorCorrection:
            return toggleCell(
                title: L10n("Settings::ColorCorrection"),
                detail: L10n("Settings::ColorCorrectionDetail"),
                image: "camera.filters",
                isOn: AppSettings.colorCorrectionEnabled,
                action: #selector(colorCorrectionChanged(_:))
            )
        case .noSpriteLimit:
            return toggleCell(
                title: L10n("Settings::NoSpriteLimit"),
                detail: L10n("Settings::NoSpriteLimitDetail"),
                image: "sparkles.rectangle.stack",
                isOn: AppSettings.noSpriteLimitEnabled,
                action: #selector(noSpriteLimitChanged(_:))
            )
        case .screenSize:
            return optionCell(
                title: L10n("Settings::ScreenSize"),
                value: AppSettings.screenSize.title,
                image: "arrow.up.left.and.arrow.down.right"
            )
        case .smoothing:
            return toggleCell(
                title: L10n("Settings::Smoothing"),
                detail: L10n("Settings::SmoothingDetail"),
                image: "square.resize",
                isOn: AppSettings.smoothingEnabled,
                action: #selector(smoothingChanged(_:))
            )
        case nil:
            return UITableViewCell()
        }
    }

    private func inputCell(row: Int) -> UITableViewCell {
        let motionEnabled = AppSettings.motionTiltEnabled

        switch InputRow(rawValue: row) {
        case .haptics:
            return toggleCell(
                title: L10n("Settings::Haptics"),
                detail: L10n("Settings::HapticsDetail"),
                image: "hand.tap",
                isOn: AppSettings.hapticsEnabled,
                action: #selector(hapticsChanged(_:))
            )
        case .motionTilt:
            return toggleCell(
                title: L10n("Settings::MotionTilt"),
                detail: L10n("Settings::MotionTiltDetail"),
                image: "gyroscope",
                isOn: motionEnabled,
                action: #selector(motionTiltChanged(_:))
            )
        case .sensitivityX:
            return optionCell(
                title: L10n("Settings::HorizontalSensitivity"),
                value: String(AppSettings.tiltSensitivityX),
                image: "arrow.left.and.right",
                isEnabled: motionEnabled
            )
        case .sensitivityY:
            return optionCell(
                title: L10n("Settings::VerticalSensitivity"),
                value: String(AppSettings.tiltSensitivityY),
                image: "arrow.up.and.down",
                isEnabled: motionEnabled
            )
        case .invertX:
            return toggleCell(
                title: L10n("Settings::InvertHorizontal"),
                detail: L10n("Settings::InvertHorizontalDetail"),
                image: "arrow.left.and.right",
                isOn: AppSettings.tiltInvertX,
                action: #selector(tiltInvertXChanged(_:)),
                isEnabled: motionEnabled
            )
        case .invertY:
            return toggleCell(
                title: L10n("Settings::InvertVertical"),
                detail: L10n("Settings::InvertVerticalDetail"),
                image: "arrow.up.and.down",
                isOn: AppSettings.tiltInvertY,
                action: #selector(tiltInvertYChanged(_:)),
                isEnabled: motionEnabled
            )
        case nil:
            return UITableViewCell()
        }
    }

    private func libraryCell(row: Int) -> UITableViewCell {
        switch LibraryRow(rawValue: row) {
        case .refresh:
            let cell = baseCell(
                title: L10n("Settings::RefreshLibrary"),
                detail: L10n("Settings::RefreshLibraryDetail"),
                image: "arrow.clockwise"
            )
            cell.textLabel?.textColor = view.tintColor
            cell.selectionStyle = .default
            return cell
        case .importedRoms:
            let cell = baseCell(
                title: L10n("Settings::ImportedRoms"),
                detail: nil,
                image: "memorychip"
            )
            cell.detailTextLabel?.text = String(dataStore.allRoms.count)
            return cell
        case nil:
            return UITableViewCell()
        }
    }

    private func aboutCell() -> UITableViewCell {
        let cell = baseCell(title: L10n("Settings::Version"), detail: nil, image: "info.circle")
        let version = Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "-"
        let build = Bundle.main.object(forInfoDictionaryKey: "CFBundleVersion") as? String ?? "-"
        cell.detailTextLabel?.text = "\(version) (\(build))"
        return cell
    }

    private func baseCell(title: String, detail: String?, image: String) -> UITableViewCell {
        let style: UITableViewCell.CellStyle = detail == nil ? .value1 : .subtitle
        let cell = UITableViewCell(style: style, reuseIdentifier: nil)
        cell.textLabel?.text = title
        cell.textLabel?.adjustsFontSizeToFitWidth = true
        cell.textLabel?.minimumScaleFactor = 0.78
        cell.textLabel?.allowsDefaultTighteningForTruncation = true
        cell.detailTextLabel?.text = detail
        cell.detailTextLabel?.adjustsFontSizeToFitWidth = true
        cell.detailTextLabel?.minimumScaleFactor = 0.75
        cell.imageView?.image = UIImage(systemName: image)
        cell.imageView?.tintColor = view.tintColor
        cell.selectionStyle = .none
        return cell
    }

    private func optionCell(title: String, value: String, image: String, isEnabled: Bool = true) -> UITableViewCell {
        let cell = UITableViewCell(style: .value1, reuseIdentifier: nil)
        cell.textLabel?.text = title
        cell.textLabel?.adjustsFontSizeToFitWidth = true
        cell.textLabel?.minimumScaleFactor = 0.78
        cell.textLabel?.allowsDefaultTighteningForTruncation = true
        cell.detailTextLabel?.text = value
        cell.detailTextLabel?.adjustsFontSizeToFitWidth = true
        cell.detailTextLabel?.minimumScaleFactor = 0.7
        cell.imageView?.image = UIImage(systemName: image)
        cell.accessoryType = .disclosureIndicator
        cell.selectionStyle = isEnabled ? .default : .none
        applyEnabledState(isEnabled, to: cell)
        return cell
    }

    private func toggleCell(
        title: String,
        detail: String,
        image: String,
        isOn: Bool,
        action: Selector,
        isEnabled: Bool = true
    ) -> UITableViewCell {
        let cell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        cell.textLabel?.text = title
        cell.textLabel?.adjustsFontSizeToFitWidth = true
        cell.textLabel?.minimumScaleFactor = 0.78
        cell.textLabel?.allowsDefaultTighteningForTruncation = true
        cell.detailTextLabel?.text = detail
        cell.detailTextLabel?.adjustsFontSizeToFitWidth = true
        cell.detailTextLabel?.minimumScaleFactor = 0.75
        cell.imageView?.image = UIImage(systemName: image)
        cell.selectionStyle = .none

        let toggle = UISwitch()
        toggle.isOn = isOn
        toggle.isEnabled = isEnabled
        toggle.accessibilityLabel = title
        toggle.addTarget(self, action: action, for: .valueChanged)
        cell.accessoryView = toggle
        applyEnabledState(isEnabled, to: cell)
        return cell
    }

    private func applyEnabledState(_ isEnabled: Bool, to cell: UITableViewCell) {
        cell.isUserInteractionEnabled = isEnabled
        cell.textLabel?.textColor = isEnabled ? .label : .tertiaryLabel
        cell.detailTextLabel?.textColor = isEnabled ? .secondaryLabel : .tertiaryLabel
        cell.imageView?.tintColor = isEnabled ? view.tintColor : .tertiaryLabel
    }

    private func showOptions(
        title: String,
        options: [String],
        selectedIndex: Int,
        onSelection: @escaping (Int) -> Void
    ) {
        let controller = OptionSelectionViewController(
            title: title,
            optionTitles: options,
            selectedIndex: selectedIndex,
            onSelection: onSelection
        )
        navigationController?.pushViewController(controller, animated: true)
    }

    private func showSensitivityOptions(title: String, currentValue: Int, onSelection: @escaping (Int) -> Void) {
        showOptions(
            title: title,
            options: (1...10).map(String.init),
            selectedIndex: currentValue - 1
        ) { onSelection($0 + 1) }
    }

    @objc private func audioChanged(_ sender: UISwitch) {
        AppSettings.audioEnabled = sender.isOn
    }

    @objc private func superGameBoyChanged(_ sender: UISwitch) {
        AppSettings.superGameBoyEnabled = sender.isOn
        tableView.reloadSections(IndexSet(integer: Section.system.rawValue), with: .automatic)
    }

    @objc private func superGameBoyBorderChanged(_ sender: UISwitch) {
        AppSettings.superGameBoyBorderEnabled = sender.isOn
    }

    @objc private func colorCorrectionChanged(_ sender: UISwitch) {
        AppSettings.colorCorrectionEnabled = sender.isOn
    }

    @objc private func noSpriteLimitChanged(_ sender: UISwitch) {
        AppSettings.noSpriteLimitEnabled = sender.isOn
    }

    @objc private func smoothingChanged(_ sender: UISwitch) {
        AppSettings.smoothingEnabled = sender.isOn
    }

    @objc private func hapticsChanged(_ sender: UISwitch) {
        AppSettings.hapticsEnabled = sender.isOn
    }

    @objc private func motionTiltChanged(_ sender: UISwitch) {
        AppSettings.motionTiltEnabled = sender.isOn
        tableView.reloadSections(IndexSet(integer: Section.input.rawValue), with: .automatic)
    }

    @objc private func tiltInvertXChanged(_ sender: UISwitch) {
        AppSettings.tiltInvertX = sender.isOn
    }

    @objc private func tiltInvertYChanged(_ sender: UISwitch) {
        AppSettings.tiltInvertY = sender.isOn
    }
}
