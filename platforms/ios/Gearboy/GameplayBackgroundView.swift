import UIKit

final class GameplayBackgroundView: UIView {
    override class var layerClass: AnyClass {
        CAGradientLayer.self
    }

    override init(frame: CGRect) {
        super.init(frame: frame)
        configure()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        configure()
    }

    private func configure() {
        isUserInteractionEnabled = false
        isOpaque = true
        accessibilityElementsHidden = true

        guard let gradient = layer as? CAGradientLayer else { return }
        gradient.colors = [
            UIColor(red: 0.34, green: 0.37, blue: 0.42, alpha: 1.0).cgColor,
            UIColor(red: 0.24, green: 0.27, blue: 0.31, alpha: 1.0).cgColor,
            UIColor(red: 0.15, green: 0.17, blue: 0.19, alpha: 1.0).cgColor
        ]
        gradient.locations = [0.0, 0.52, 1.0]
        gradient.startPoint = CGPoint(x: 0.12, y: 0.0)
        gradient.endPoint = CGPoint(x: 0.88, y: 1.0)
    }
}
