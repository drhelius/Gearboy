/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The view controller for the bottom half of the recipe detail scene.
*/

import UIKit

class RomDetailBottomViewController: UIViewController, RomController {
    
    @IBOutlet weak var segmentedControl: UISegmentedControl!
    @IBOutlet weak var textView: UITextView!
    
    private enum Segments: Int {
        case ingredients, directions
    }
    
    var recipe: Rom? {
        didSet {
            updateUI()
        }
    }
    
    @IBAction func segmentedControlValueChanged(_ sender: Any?) {
        updateUI()
    }
}

extension RomDetailBottomViewController {
    
    private func updateUI() {
        guard let recipe = self.recipe else { return }
        
        let paragraphStyle = NSMutableParagraphStyle()
        let text: String
        if segmentedControl.selectedSegmentIndex == Segments.ingredients.rawValue {
            paragraphStyle.lineHeightMultiple = 1.75
            text = recipe.ingredients
        } else {
            paragraphStyle.paragraphSpacingBefore = 12
            paragraphStyle.lineHeightMultiple = 1.25
            text = recipe.directions
        }
        
        let attributes = [
            NSAttributedString.Key.paragraphStyle: paragraphStyle,
            NSAttributedString.Key.font: UIFont.preferredFont(forTextStyle: .body),
            NSAttributedString.Key.foregroundColor: UIColor.label
        ]
        
        textView.attributedText = NSMutableAttributedString(string: text, attributes: attributes)
    }
    
}
