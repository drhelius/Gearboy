/*
See LICENSE folder for this sample’s licensing information.

Abstract:
A cell of the rom list collection view.
*/

import UIKit
import SDWebImage

class RomListCell: UICollectionViewCell {
    @IBOutlet weak var titleLabel: UILabel!
    @IBOutlet weak var imageView: UIImageView!
    @IBOutlet weak var favoriteImageView: UIImageView!

    private var borderColor = UIColor.systemGray2.withAlphaComponent(0.2).cgColor
    private var selectedBorderColor = UIColor.systemBlue.cgColor
    
    override func awakeFromNib() {
        super.awakeFromNib()
        
        layer.borderColor = borderColor
        layer.borderWidth = 1
        layer.cornerRadius = 6
    }
    
    override func prepareForReuse() {
        super.prepareForReuse()
        
        titleLabel.text = nil
        imageView.image = nil
        favoriteImageView.alpha = 0
    }
    
    override var isSelected: Bool {
        didSet {
            layer.borderColor = isSelected ? selectedBorderColor : borderColor
            layer.borderWidth = isSelected ? 3 : 2
        }
    }

    func configure(with rom: Rom) {
        titleLabel.text = rom.file
        favoriteImageView.alpha = rom.isFavorite ? 1 : 0
        downloadImage(boxarts: rom.urlImages)
    }
    
}

// MARK: - Private Methods
private extension RomListCell {
    // Best way for now to do it. Searching in all boxarts, using a recursive function
    func downloadImage(boxarts: [URL], index: Int = .zero) {
        guard boxarts.count > index else { return }
        imageView.sd_setImage(with: boxarts[index], placeholderImage: #imageLiteral(resourceName: "Cartridge")) { (_, error, _, _) in
            guard error != nil else { return }
            self.downloadImage(boxarts: boxarts, index: index + 1)
        }
    }
}
