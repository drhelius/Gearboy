import UIKit

final class GameControlsView: UIView {
    var onButtonChanged: ((GearboyButton, Bool) -> Void)? {
        didSet {
            actionA.onButtonChanged = onButtonChanged
            actionB.onButtonChanged = onButtonChanged
            start.onButtonChanged = onButtonChanged
            select.onButtonChanged = onButtonChanged
        }
    }

    var hapticsEnabled = true {
        didSet {
            dPad.hapticsEnabled = hapticsEnabled
            actionA.hapticsEnabled = hapticsEnabled
            actionB.hapticsEnabled = hapticsEnabled
            start.hapticsEnabled = hapticsEnabled
            select.hapticsEnabled = hapticsEnabled
        }
    }

    let dPad = DirectionPadView()
    let actionA = GameControlButton(title: "A", button: .A, shape: .circle)
    let actionB = GameControlButton(title: "B", button: .B, shape: .circle)
    let start = GameControlButton(title: "START", button: .start, shape: .capsule)
    let select = GameControlButton(title: "SELECT", button: .select, shape: .capsule)

    private let actionGuide = UILayoutGuide()
    private var portraitConstraints = [NSLayoutConstraint]()
    private var landscapeConstraints = [NSLayoutConstraint]()
    private var usingLandscapeConstraints = false

    override init(frame: CGRect) {
        super.init(frame: frame)
        configure()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        configure()
    }

    override func layoutSubviews() {
        let landscape = bounds.width > bounds.height
        if landscape != usingLandscapeConstraints {
            usingLandscapeConstraints = landscape
            NSLayoutConstraint.deactivate(landscape ? portraitConstraints : landscapeConstraints)
            NSLayoutConstraint.activate(landscape ? landscapeConstraints : portraitConstraints)
        }

        super.layoutSubviews()
    }

    private func configure() {
        isMultipleTouchEnabled = true
        backgroundColor = .clear
        dPad.onDirectionChanged = { [weak self] direction, pressed in
            guard let self else { return }
            self.onButtonChanged?(self.emulatorButton(for: direction), pressed)
        }

        dPad.translatesAutoresizingMaskIntoConstraints = false
        actionA.translatesAutoresizingMaskIntoConstraints = false
        actionB.translatesAutoresizingMaskIntoConstraints = false
        start.translatesAutoresizingMaskIntoConstraints = false
        select.translatesAutoresizingMaskIntoConstraints = false

        addSubview(dPad)
        addSubview(actionA)
        addSubview(actionB)
        addSubview(start)
        addSubview(select)
        addLayoutGuide(actionGuide)

        let isPad = UIDevice.current.userInterfaceIdiom == .pad
        let dPadSize: CGFloat = isPad ? 176.0 : 132.0
        let actionSize: CGFloat = isPad ? 88.0 : 72.0
        let actionOffset: CGFloat = isPad ? 36.0 : 30.0
        let primaryOffset: CGFloat = isPad ? 220.0 : 120.0
        let menuSpacing: CGFloat = isPad ? 64.0 : 48.0
        let menuWidth: CGFloat = isPad ? 104.0 : 80.0
        let menuHeight: CGFloat = 44.0
        let portraitPrimaryConstraint = isPad
            ? select.bottomAnchor.constraint(equalTo: safeAreaLayoutGuide.bottomAnchor, constant: -24.0)
            : dPad.centerYAnchor.constraint(equalTo: safeAreaLayoutGuide.centerYAnchor, constant: primaryOffset)

        NSLayoutConstraint.activate([
            dPad.widthAnchor.constraint(equalToConstant: dPadSize),
            dPad.heightAnchor.constraint(equalTo: dPad.widthAnchor),

            actionA.widthAnchor.constraint(equalToConstant: actionSize),
            actionA.heightAnchor.constraint(equalTo: actionA.widthAnchor),

            actionB.trailingAnchor.constraint(equalTo: actionA.leadingAnchor, constant: -14.0),
            actionB.centerYAnchor.constraint(equalTo: actionA.centerYAnchor, constant: actionOffset),
            actionB.widthAnchor.constraint(equalTo: actionA.widthAnchor),
            actionB.heightAnchor.constraint(equalTo: actionB.widthAnchor),

            select.widthAnchor.constraint(equalToConstant: menuWidth),
            select.heightAnchor.constraint(equalToConstant: menuHeight),
            start.widthAnchor.constraint(equalTo: select.widthAnchor),
            start.heightAnchor.constraint(equalTo: select.heightAnchor),

            actionGuide.leadingAnchor.constraint(equalTo: actionB.leadingAnchor),
            actionGuide.trailingAnchor.constraint(equalTo: actionA.trailingAnchor)
        ])

        portraitConstraints = [
            dPad.leadingAnchor.constraint(equalTo: safeAreaLayoutGuide.leadingAnchor, constant: 20.0),
            actionA.trailingAnchor.constraint(equalTo: safeAreaLayoutGuide.trailingAnchor, constant: -20.0),
            portraitPrimaryConstraint,
            actionA.centerYAnchor.constraint(equalTo: dPad.centerYAnchor, constant: -(actionOffset * 0.5)),
            select.trailingAnchor.constraint(equalTo: safeAreaLayoutGuide.centerXAnchor, constant: -12.0),
            start.leadingAnchor.constraint(equalTo: safeAreaLayoutGuide.centerXAnchor, constant: 12.0),
            select.topAnchor.constraint(equalTo: dPad.bottomAnchor, constant: menuSpacing),
            start.topAnchor.constraint(equalTo: select.topAnchor),
            select.bottomAnchor.constraint(lessThanOrEqualTo: safeAreaLayoutGuide.bottomAnchor, constant: -24.0),
            start.bottomAnchor.constraint(lessThanOrEqualTo: safeAreaLayoutGuide.bottomAnchor, constant: -24.0)
        ]

        landscapeConstraints = [
            dPad.leadingAnchor.constraint(equalTo: safeAreaLayoutGuide.leadingAnchor, constant: 8.0),
            actionA.trailingAnchor.constraint(equalTo: safeAreaLayoutGuide.trailingAnchor, constant: -8.0),
            dPad.centerYAnchor.constraint(equalTo: safeAreaLayoutGuide.centerYAnchor),
            actionA.centerYAnchor.constraint(equalTo: safeAreaLayoutGuide.centerYAnchor, constant: -(actionOffset * 0.5)),
            select.centerXAnchor.constraint(equalTo: dPad.centerXAnchor),
            start.centerXAnchor.constraint(equalTo: actionGuide.centerXAnchor),
            select.bottomAnchor.constraint(equalTo: safeAreaLayoutGuide.bottomAnchor, constant: -8.0),
            start.bottomAnchor.constraint(equalTo: select.bottomAnchor)
        ]

        NSLayoutConstraint.activate(portraitConstraints)
    }

    private func emulatorButton(for direction: DirectionPadDirection) -> GearboyButton {
        switch direction {
        case .up: return .up
        case .down: return .down
        case .left: return .left
        case .right: return .right
        }
    }
}

final class GameControlButton: UIButton {
    enum Shape {
        case circle
        case capsule
    }

    var onButtonChanged: ((GearboyButton, Bool) -> Void)?
    var hapticsEnabled = true

    private let emulatorButton: GearboyButton
    private let shape: Shape
    private var pressed = false
    private let feedback = UIImpactFeedbackGenerator(style: .light)

    init(title: String, button: GearboyButton, shape: Shape) {
        self.emulatorButton = button
        self.shape = shape
        super.init(frame: .zero)

        setTitle(title, for: .normal)
        setTitleColor(.label, for: .normal)
        titleLabel?.font = shape == .circle
            ? .systemFont(ofSize: 24.0, weight: .bold)
            : .systemFont(ofSize: 11.0, weight: .semibold)
        backgroundColor = UIColor.secondarySystemFill.withAlphaComponent(0.92)
        layer.borderColor = UIColor.separator.withAlphaComponent(0.65).cgColor
        layer.borderWidth = 1.0
        accessibilityLabel = title

        addTarget(self, action: #selector(press), for: [.touchDown, .touchDragEnter])
        addTarget(self, action: #selector(releaseButton), for: [.touchUpInside, .touchUpOutside, .touchCancel, .touchDragExit])
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override func layoutSubviews() {
        super.layoutSubviews()
        layer.cornerRadius = shape == .circle ? bounds.width * 0.5 : bounds.height * 0.5
    }

    @objc private func press() {
        guard !pressed else { return }
        pressed = true
        if hapticsEnabled {
            feedback.prepare()
            feedback.impactOccurred(intensity: 0.55)
        }
        backgroundColor = tintColor.withAlphaComponent(0.28)
        transform = CGAffineTransform(scaleX: 0.94, y: 0.94)
        onButtonChanged?(emulatorButton, true)
    }

    @objc private func releaseButton() {
        guard pressed else { return }
        pressed = false
        backgroundColor = UIColor.secondarySystemFill.withAlphaComponent(0.92)
        transform = .identity
        onButtonChanged?(emulatorButton, false)
    }
}
