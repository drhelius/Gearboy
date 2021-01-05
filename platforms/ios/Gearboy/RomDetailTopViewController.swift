/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The view controller for the top half of the recipe detail scene.
*/

import UIKit

class RomDetailTopViewController: UIViewController, RomController {

    @IBOutlet weak var imageView: UIImageView!
    @IBOutlet weak var titleLabel: UILabel!
    @IBOutlet weak var detailLabel: UILabel!
    
    var recipe: Rom? {
        didSet {
            updateUI()
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()

        if traitCollection.userInterfaceIdiom == .pad {
            titleLabel.textAlignment = .left
        }
    }

    override func willTransition(to newCollection: UITraitCollection, with coordinator: UIViewControllerTransitionCoordinator) {
        let align: NSTextAlignment
        if newCollection.horizontalSizeClass == .compact && newCollection.verticalSizeClass == .compact {
            align = .left
        } else {
            align = .center
        }
        
        coordinator.animate(
            alongsideTransition: { [unowned self] _ in
                self.titleLabel.textAlignment = align
            },
            completion: nil)
    }

}

extension RomDetailTopViewController {
    
    private func updateUI() {
        guard let recipe = self.recipe else { return }
        
        imageView.image = recipe.fullImage
        titleLabel.text = recipe.title
        
        let bold = [NSAttributedString.Key.font: UIFont.boldSystemFont(ofSize: 17)]
        let normal = [NSAttributedString.Key.font: UIFont.systemFont(ofSize: 17)]
        
        let attributedText = NSMutableAttributedString()
        attributedText.append(NSAttributedString(string: recipe.servings, attributes: bold))
        attributedText.append(NSAttributedString(string: " servings | ", attributes: normal))
        
        let (hours, minutes, seconds) = recipe.totalTime()
        if hours > 0 {
            attributedText.append(NSAttributedString(string: "\(hours)", attributes: bold))
            attributedText.append(NSAttributedString(string: " hr", attributes: normal))
        }
        if minutes > 0 {
            if hours > 0 {
                attributedText.append(NSAttributedString(string: " ", attributes: normal))
                
            }
            attributedText.append(NSAttributedString(string: "\(minutes)", attributes: bold))
            attributedText.append(NSAttributedString(string: " min", attributes: normal))
        }
        if seconds > 0 {
            if hours > 0 || minutes > 0 {
                attributedText.append(NSAttributedString(string: " ", attributes: normal))
                
            }
            attributedText.append(NSAttributedString(string: "\(seconds)", attributes: bold))
            attributedText.append(NSAttributedString(string: " sec", attributes: normal))
        }
        
        detailLabel.attributedText = attributedText
    }
    
}
