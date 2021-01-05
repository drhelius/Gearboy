/*
See LICENSE folder for this sample’s licensing information.

Abstract:
A cell of the recipe list collection view.
*/

import UIKit

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
        layer.cornerRadius = 8
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
            layer.borderWidth = isSelected ? 2 : 1
        }
    }

    func configure(with recipe: Rom) {
        titleLabel.text = recipe.title
        imageView.image = recipe.smallImage
        favoriteImageView.alpha = recipe.isFavorite ? 1 : 0
    }
    
}
