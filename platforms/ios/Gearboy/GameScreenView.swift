import UIKit

final class GameScreenView: UIView {
    private static let colorSpace = CGColorSpaceCreateDeviceRGB()
    private static let bitmapInfo = CGBitmapInfo.byteOrder32Little.union(
        CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedFirst.rawValue)
    )

    private var pixels = [UInt32]()

    var isSmoothingEnabled = false {
        didSet {
            updateTextureFiltering()
        }
    }

    override init(frame: CGRect) {
        super.init(frame: frame)
        configure()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        configure()
    }

    func display(frameBuffer: UnsafePointer<UInt16>, width: Int, height: Int) {
        guard width > 0, height > 0 else { return }

        let pixelCount = width * height
        if pixels.count != pixelCount {
            pixels = [UInt32](repeating: 0xFF000000, count: pixelCount)
        }

        pixels.withUnsafeMutableBufferPointer { output in
            for index in 0..<pixelCount {
                let pixel = UInt32(frameBuffer[index])
                let red = ((pixel >> 11) & 0x1F) * 255 / 31
                let green = ((pixel >> 5) & 0x3F) * 255 / 63
                let blue = (pixel & 0x1F) * 255 / 31
                output[index] = 0xFF000000 | (red << 16) | (green << 8) | blue
            }
        }

        let data = pixels.withUnsafeBytes { Data($0) }
        guard
            let provider = CGDataProvider(data: data as CFData),
            let image = CGImage(
                width: width,
                height: height,
                bitsPerComponent: 8,
                bitsPerPixel: 32,
                bytesPerRow: width * MemoryLayout<UInt32>.stride,
                space: Self.colorSpace,
                bitmapInfo: Self.bitmapInfo,
                provider: provider,
                decode: nil,
                shouldInterpolate: isSmoothingEnabled,
                intent: .defaultIntent
            )
        else {
            return
        }

        layer.contents = image
    }

    private func configure() {
        isOpaque = true
        backgroundColor = .black
        layer.contentsGravity = .resize
        updateTextureFiltering()
        accessibilityLabel = L10n("Gameplay::Screen")
    }

    private func updateTextureFiltering() {
        let filter: CALayerContentsFilter = isSmoothingEnabled ? .linear : .nearest
        layer.magnificationFilter = filter
        layer.minificationFilter = filter
    }
}
