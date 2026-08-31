import UIKit

enum DirectionPadDirection {
    case up
    case down
    case left
    case right
}

final class DirectionPadView: UIView {
    var onDirectionChanged: ((DirectionPadDirection, Bool) -> Void)?
    var hapticsEnabled = true

    private var activeDirections = Set<DirectionPadDirection>()
    private let feedback = UIImpactFeedbackGenerator(style: .light)

    override init(frame: CGRect) {
        super.init(frame: frame)
        configure()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        configure()
    }

    override func draw(_ rect: CGRect) {
        let arm = rect.width / 3.0
        let horizontal = UIBezierPath(
            roundedRect: CGRect(x: 0.0, y: arm, width: rect.width, height: arm),
            cornerRadius: arm * 0.18
        )
        let vertical = UIBezierPath(
            roundedRect: CGRect(x: arm, y: 0.0, width: arm, height: rect.height),
            cornerRadius: arm * 0.18
        )
        UIColor.secondarySystemFill.withAlphaComponent(0.96).setFill()
        horizontal.fill()
        vertical.fill()

        tintColor.withAlphaComponent(0.25).setFill()
        for direction in activeDirections {
            highlightPath(for: direction, rect: rect, arm: arm).fill()
        }

        UIColor.label.withAlphaComponent(0.72).setFill()
        arrowPath(direction: .up, rect: rect, arm: arm).fill()
        arrowPath(direction: .down, rect: rect, arm: arm).fill()
        arrowPath(direction: .left, rect: rect, arm: arm).fill()
        arrowPath(direction: .right, rect: rect, arm: arm).fill()
    }

    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        if hapticsEnabled {
            feedback.prepare()
        }
        update(with: touches.first)
    }

    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        update(with: touches.first)
    }

    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        updateDirections([])
    }

    override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        updateDirections([])
    }

    private func configure() {
        isOpaque = false
        isMultipleTouchEnabled = false
        contentMode = .redraw
        accessibilityLabel = L10n("Gameplay::DirectionalPad")
        accessibilityHint = L10n("Gameplay::DirectionalPadHint")
    }

    private func update(with touch: UITouch?) {
        guard let touch else {
            updateDirections([])
            return
        }

        let location = touch.location(in: self)
        let center = CGPoint(x: bounds.midX, y: bounds.midY)
        let dx = location.x - center.x
        let dy = location.y - center.y
        let deadZone = min(bounds.width, bounds.height) * 0.12

        guard hypot(dx, dy) >= deadZone else {
            updateDirections([])
            return
        }

        var directions = Set<DirectionPadDirection>()
        let axisThreshold = deadZone * 0.65

        if dx < -axisThreshold {
            directions.insert(.left)
        } else if dx > axisThreshold {
            directions.insert(.right)
        }

        if dy < -axisThreshold {
            directions.insert(.up)
        } else if dy > axisThreshold {
            directions.insert(.down)
        }

        updateDirections(directions)
    }

    private func updateDirections(_ directions: Set<DirectionPadDirection>) {
        let released = activeDirections.subtracting(directions)
        let pressed = directions.subtracting(activeDirections)

        for direction in released {
            onDirectionChanged?(direction, false)
        }

        if hapticsEnabled && !pressed.isEmpty {
            feedback.impactOccurred(intensity: 0.45)
        }

        for direction in pressed {
            onDirectionChanged?(direction, true)
        }

        activeDirections = directions
        setNeedsDisplay()
    }

    private func highlightPath(
        for direction: DirectionPadDirection,
        rect: CGRect,
        arm: CGFloat
    ) -> UIBezierPath {
        let frame: CGRect

        switch direction {
        case .up:
            frame = CGRect(x: arm, y: 0.0, width: arm, height: rect.midY)
        case .down:
            frame = CGRect(x: arm, y: rect.midY, width: arm, height: rect.midY)
        case .left:
            frame = CGRect(x: 0.0, y: arm, width: rect.midX, height: arm)
        case .right:
            frame = CGRect(x: rect.midX, y: arm, width: rect.midX, height: arm)
        }

        return UIBezierPath(roundedRect: frame, cornerRadius: arm * 0.16)
    }

    private func arrowPath(
        direction: DirectionPadDirection,
        rect: CGRect,
        arm: CGFloat
    ) -> UIBezierPath {
        let center: CGPoint
        let size = arm * 0.22

        switch direction {
        case .up:
            center = CGPoint(x: rect.midX, y: arm * 0.48)
        case .down:
            center = CGPoint(x: rect.midX, y: rect.height - (arm * 0.48))
        case .left:
            center = CGPoint(x: arm * 0.48, y: rect.midY)
        case .right:
            center = CGPoint(x: rect.width - (arm * 0.48), y: rect.midY)
        }

        let path = UIBezierPath()

        switch direction {
        case .up:
            path.move(to: CGPoint(x: center.x, y: center.y - size))
            path.addLine(to: CGPoint(x: center.x - size, y: center.y + size))
            path.addLine(to: CGPoint(x: center.x + size, y: center.y + size))
        case .down:
            path.move(to: CGPoint(x: center.x, y: center.y + size))
            path.addLine(to: CGPoint(x: center.x - size, y: center.y - size))
            path.addLine(to: CGPoint(x: center.x + size, y: center.y - size))
        case .left:
            path.move(to: CGPoint(x: center.x - size, y: center.y))
            path.addLine(to: CGPoint(x: center.x + size, y: center.y - size))
            path.addLine(to: CGPoint(x: center.x + size, y: center.y + size))
        case .right:
            path.move(to: CGPoint(x: center.x + size, y: center.y))
            path.addLine(to: CGPoint(x: center.x - size, y: center.y - size))
            path.addLine(to: CGPoint(x: center.x - size, y: center.y + size))
        }

        path.close()
        return path
    }
}
