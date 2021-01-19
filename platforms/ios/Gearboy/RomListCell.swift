/*
See LICENSE folder for this sample’s licensing information.

Abstract:
A cell of the rom list collection view.
*/

import UIKit
import SDWebImage

class RomListCell: UICollectionViewCell {
    
    static let reuseIdentifier = "romListCell"
    static let nib = UINib(nibName: "RomListCell", bundle: .main)

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
        imageView.sd_setImage(with: rom.urlImage, placeholderImage: #imageLiteral(resourceName: "Cartridge"))
        favoriteImageView.alpha = rom.isFavorite ? 1 : 0
    }
    
}
