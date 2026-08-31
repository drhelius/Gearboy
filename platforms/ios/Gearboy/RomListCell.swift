/*
See LICENSE folder for this sample’s licensing information.

Abstract:
A cell of the rom list collection view.
*/

import UIKit

final class RomListCell: UICollectionViewCell {
    @IBOutlet weak var titleLabel: UILabel!
    @IBOutlet weak var imageView: UIImageView!
    @IBOutlet weak var favoriteImageView: UIImageView!

    private let borderColor = UIColor.systemGray2.withAlphaComponent(0.2).cgColor
    private let selectedBorderColor = UIColor.systemBlue.cgColor
    private let coverImageView = UIImageView()
    private static let imageCache = NSCache<NSURL, UIImage>()
    private var imageTask: URLSessionDataTask?
    private var representedCRC: String?
    private var onFavoriteToggle: (() -> Void)?
    private var onDelete: (() -> Void)?
    
    override func awakeFromNib() {
        super.awakeFromNib()
        
        layer.borderColor = borderColor
        layer.borderWidth = 1
        layer.cornerRadius = 6

        imageView.image = nil
        imageView.contentMode = .scaleToFill
        imageView.addSubview(coverImageView)
        coverImageView.contentMode = .scaleToFill
        coverImageView.isUserInteractionEnabled = false

        favoriteImageView.tintColor = .systemYellow
        favoriteImageView.clipsToBounds = false
    }

    override func layoutSubviews() {
        super.layoutSubviews()
        layoutCoverImage()
    }
    
    override func prepareForReuse() {
        super.prepareForReuse()
        
        titleLabel.text = nil
        setCoverImage(nil)
        favoriteImageView.alpha = 0
        representedCRC = nil
        onFavoriteToggle = nil
        onDelete = nil
        accessibilityCustomActions = nil
        imageTask?.cancel()
        imageTask = nil
    }
    
    override var isSelected: Bool {
        didSet {
            layer.borderColor = isSelected ? selectedBorderColor : borderColor
            layer.borderWidth = isSelected ? 3 : 2
        }
    }

    func configure(
        with rom: Rom,
        onFavoriteToggle: @escaping () -> Void,
        onDelete: @escaping () -> Void
    ) {
        titleLabel.text = rom.file
        representedCRC = rom.crc
        self.onFavoriteToggle = onFavoriteToggle
        self.onDelete = onDelete
        accessibilityLabel = rom.title.isEmpty ? rom.file : rom.title
        favoriteImageView.alpha = rom.isFavorite ? 1.0 : 0.0
        accessibilityCustomActions = [
            UIAccessibilityCustomAction(
                name: rom.isFavorite ? L10n("Library::RemoveFavorite") : L10n("Library::AddFavorite"),
                target: self,
                selector: #selector(accessibilityToggleFavorite)
            ),
            UIAccessibilityCustomAction(
                name: L10n("Common::Delete"),
                target: self,
                selector: #selector(accessibilityDelete)
            )
        ]
        downloadImage(url: rom.thumbnailURL, crc: rom.crc)
    }

    @objc private func accessibilityToggleFavorite() -> Bool {
        onFavoriteToggle?()
        return true
    }

    @objc private func accessibilityDelete() -> Bool {
        onDelete?()
        return true
    }
    
}

// MARK: - Private Methods
private extension RomListCell {
    func setCoverImage(_ image: UIImage?) {
        coverImageView.image = image
        setNeedsLayout()
    }

    func layoutCoverImage() {
        guard let image = coverImageView.image,
              image.size.width > 0.0,
              imageView.bounds.width > 0.0 else {
            coverImageView.frame = .zero
            return
        }

        let width = imageView.bounds.width
        let height = width * image.size.height / image.size.width
        coverImageView.frame = CGRect(x: 0.0, y: 0.0, width: width, height: height)
    }

    func downloadImage(url: URL, crc: String) {
        setCoverImage(UIImage(named: "Cartridge"))
        if let cachedImage = Self.imageCache.object(forKey: url as NSURL) {
            setCoverImage(cachedImage)
            return
        }

        var request = URLRequest(url: url)
        if let key = ThumbnailAuthorization.key {
            request.setValue(key, forHTTPHeaderField: ThumbnailAuthorization.headerName)
        }

        imageTask = URLSession.shared.dataTask(with: request) { [weak self] data, response, error in
            guard let self else { return }

            if
                error == nil,
                let response = response as? HTTPURLResponse,
                (200..<300).contains(response.statusCode),
                let data,
                let image = UIImage(data: data)
            {
                Self.imageCache.setObject(image, forKey: url as NSURL)
                DispatchQueue.main.async {
                    guard self.representedCRC == crc else { return }
                    self.setCoverImage(image)
                }
            }
        }
        imageTask?.resume()
    }
}
