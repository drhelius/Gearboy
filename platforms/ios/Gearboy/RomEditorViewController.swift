/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The recipe editor view controller.
*/

import UIKit
import AVFoundation

class RomEditorViewController: UIViewController, RomController {

    static let storyboardID = "RomEditor"
    static func instantiateFromStoryboard() -> RomEditorViewController? {
        let storyboard = UIStoryboard(name: "RomEditor", bundle: .main)
        return storyboard.instantiateViewController(identifier: storyboardID) as? RomEditorViewController
    }

    var recipe: Rom?

    @IBOutlet weak var titleField: UITextField!
    @IBOutlet weak var servingsField: UITextField!
    @IBOutlet weak var prepTimeField: UITextField!
    @IBOutlet weak var cookTimeField: UITextField!
    @IBOutlet weak var ingredientsField: UITextView!
    @IBOutlet weak var directionsField: UITextView!
    @IBOutlet weak var photoImageView: UIImageView!
    @IBOutlet weak var collectionsTextField: UITextField!

    private let imagePickerController = UIImagePickerController()
    
    override func viewDidLoad() {
        super.viewDidLoad()

        if let recipe = self.recipe {
            self.title = NSLocalizedString("Edit", comment: "Recipe Editor Title")
         
            titleField.text = recipe.title
            servingsField.text = recipe.servings
            prepTimeField.text = "\(recipe.prepTime)"
            cookTimeField.text = "\(recipe.cookTime)"
            ingredientsField.text = recipe.ingredients
            directionsField.text = recipe.directions
            photoImageView.image = recipe.fullImage
            if recipe.collections.isEmpty == false {
                collectionsTextField.text = recipe.collections.joined(separator: ", ")
            }
            
        } else {
            self.title = NSLocalizedString("Add", comment: "Recipe Editor Title")
            recipe = dataStore.newRecipe()
        }
        
        addBorder(to: ingredientsField)
        addBorder(to: directionsField)
        
        imagePickerController.modalPresentationStyle = .currentContext
        imagePickerController.delegate = self
    }
    
    func addBorder(to textView: UITextView) {
        textView.layer.borderColor = UIColor.systemGray3.cgColor
        textView.layer.borderWidth = 1
        textView.layer.cornerRadius = 4
    }
    
    func editedRecipe() -> Rom? {
        guard var recipe = self.recipe else { return nil }
        
        recipe.title = titleField.text ?? "Recipe"
        recipe.servings = servingsField.text ?? ""
        recipe.prepTime = Int(prepTimeField.text ?? "0") ?? 0
        recipe.cookTime = Int(cookTimeField.text ?? "0") ?? 0
        recipe.ingredients = ingredientsField.text
        recipe.directions = directionsField.text
        
        if let text = collectionsTextField.text {
            recipe.collections = text.replacingOccurrences(of: ", ", with: ",").components(separatedBy: ",")
        }
        
        if let image = photoImageView.image {
            recipe.add(image)
        }
        
        return recipe
    }

}

// MARK: - Actions

extension RomEditorViewController {
    
    @IBAction func editPhoto(_ sender: Any?) {
        
        // If there's an available camera...
        if UIImagePickerController.isSourceTypeAvailable(.camera) {
            presentPhotoPickerMenu(sender)
        } else {
            presentPhotoLibrary(sender)
        }
    }
    
}

// MARK: - Photos Helper

extension RomEditorViewController {
    
    func presentPhotoPickerMenu(_ sender: Any?) {
        let takePhotoAction = UIAlertAction(title: "Take Photo", style: .default) { (action) in
            self.presentCamera(sender)
        }
        
        let chooseExistingAction = UIAlertAction(title: "Choose from Existing", style: .default) { (action) in
            self.presentPhotoLibrary(sender)
        }
        
        let cancelAction = UIAlertAction(title: "Cancel", style: .cancel, handler: nil)
        
        let alert = UIAlertController(title: nil, message: nil, preferredStyle: .actionSheet)
        alert.addAction(takePhotoAction)
        alert.addAction(chooseExistingAction)
        alert.addAction(cancelAction)
        
        if let popoverPresentationController = alert.popoverPresentationController,
           let sourceView = sender as? UIView {
            popoverPresentationController.sourceView = sourceView
            popoverPresentationController.sourceRect = sourceView.bounds
        }
        
        present(alert, animated: true, completion: nil)

    }
    
    func presentCamera(_ sender: Any?) {
        guard UIImagePickerController.isSourceTypeAvailable(.camera) else { return }
        
        let authStatus = AVCaptureDevice.authorizationStatus(for: AVMediaType.video)
        switch authStatus {
        case .denied:
            // Tell the user that the apps needs access to the camera to take pictures.
            presentCameraDeniedAlert()
        case .notDetermined:
            // Request camera access from the user.
            requestCameraAccess()
        default:
            showCamera()
        }
    }
    
    func presentCameraDeniedAlert() {
        let alert = UIAlertController(title: "Cannot to access the camera",
                                      message: "To turn on camera access, choose Settings > Privacy > Camera and turn on Camera access for this app.",
                                      preferredStyle: .alert)
        
        let okAction = UIAlertAction(title: "OK", style: .cancel, handler: nil)
        alert.addAction(okAction)
        
        let settingsAction = UIAlertAction(title: "Settings", style: .default, handler: { _ in
            // Take the user to the Settings app to change the permissions.
            guard let settingsUrl = URL(string: UIApplication.openSettingsURLString) else { return }
            if UIApplication.shared.canOpenURL(settingsUrl) {
                UIApplication.shared.open(settingsUrl, completionHandler: { (success) in
                    // The resource finished opening.
                })
            }
        })
        alert.addAction(settingsAction)
        
        present(alert, animated: true, completion: nil)
    }
    
    func requestCameraAccess() {
        AVCaptureDevice.requestAccess(for: AVMediaType.video, completionHandler: { (granted) in
            if granted {
                DispatchQueue.main.async {
                    self.showCamera()
                }
            }
        })
    }
    
    func showCamera() {
        imagePickerController.sourceType = .camera
        imagePickerController.modalPresentationStyle = .fullScreen

        present(imagePickerController, animated: true, completion: nil)
    }
    
    func presentPhotoLibrary(_ sender: Any?) {
        guard UIImagePickerController.isSourceTypeAvailable(.photoLibrary) else { return }
        
        imagePickerController.sourceType = .photoLibrary
        
        if let popoverPresentationController = imagePickerController.popoverPresentationController,
           let sourceView = sender as? UIView {
            popoverPresentationController.sourceView = sourceView
            popoverPresentationController.sourceRect = sourceView.bounds
        }

        present(imagePickerController, animated: true, completion: nil)
    }
    
}

extension RomEditorViewController: UIImagePickerControllerDelegate, UINavigationControllerDelegate {
    
    func imagePickerController(_ picker: UIImagePickerController, didFinishPickingMediaWithInfo info: [UIImagePickerController.InfoKey : Any]) {
        
        if let image = info[.originalImage] as? UIImage {
            photoImageView.image = image
        }
        
        picker.dismiss(animated: true, completion: nil)
    }
    
    func imagePickerControllerDidCancel(_ picker: UIImagePickerController) {
        picker.dismiss(animated: true, completion: nil)
    }
    
}
