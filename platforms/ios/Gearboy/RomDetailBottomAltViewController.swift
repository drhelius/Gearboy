/*
See LICENSE folder for this sample’s licensing information.

Abstract:
An alternate view controller for the bottom half of the recipe detail scene.
*/

import UIKit

class RomDetailBottomAltViewController: UIViewController, RomController {
    
    static let storyboardID = "recipeDetailBottomAlt"
    static func instantiate(with storyboard: UIStoryboard) -> RomDetailBottomAltViewController? {
        return storyboard.instantiateViewController(identifier: storyboardID) as? RomDetailBottomAltViewController
    }
    
    var recipe: Rom? {
        didSet {
            updateUI()
        }
    }
    
    @IBOutlet weak var ingredientsHeaderLabel: UILabel!
    @IBOutlet weak var ingredientsLabel: UILabel!
    @IBOutlet weak var directionsLabel: UILabel!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
    }
    
    override func traitCollectionDidChange(_ previousTraitCollection: UITraitCollection?) {
        super.traitCollectionDidChange(previousTraitCollection)
        updateUI()
    }

}

extension RomDetailBottomAltViewController {
    
    private func updateUI() {
        guard let recipe = self.recipe else { return }
        
        let textColor = traitCollection.userInterfaceStyle == .light ? UIColor.darkGray : UIColor.lightGray
        ingredientsHeaderLabel.textColor = textColor
        
        let ingredientsParagraphStyle = NSMutableParagraphStyle()
        ingredientsParagraphStyle.paragraphSpacingBefore = 8
        ingredientsParagraphStyle.lineHeightMultiple = 1.25

        let ingredientsAttributes = [
            NSAttributedString.Key.paragraphStyle: ingredientsParagraphStyle,
            NSAttributedString.Key.font: UIFont.systemFont(ofSize: 14),
            NSAttributedString.Key.foregroundColor: textColor
        ]

        let directionsParagraphStyle = NSMutableParagraphStyle()
        directionsParagraphStyle.paragraphSpacingBefore = 12
        directionsParagraphStyle.lineHeightMultiple = 1.25

        let directionsAttributes = [
            NSAttributedString.Key.paragraphStyle: directionsParagraphStyle,
            NSAttributedString.Key.font: UIFont.preferredFont(forTextStyle: .body),
            NSAttributedString.Key.foregroundColor: UIColor.label
        ]

        ingredientsLabel.attributedText = NSMutableAttributedString(string: recipe.ingredients, attributes: ingredientsAttributes)
        directionsLabel.attributedText = NSMutableAttributedString(string: recipe.directions, attributes: directionsAttributes)
    }
    
}
