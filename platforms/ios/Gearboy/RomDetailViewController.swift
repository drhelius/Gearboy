/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The recipe detail view controller.
*/

import UIKit
import os
import Combine

class RomDetailViewController: UIViewController, RomController {

    static let storyboardID = "RomDetail"
    static func instantiateFromStoryboard() -> RomDetailViewController? {
        let storyboard = UIStoryboard(name: "RomDetail", bundle: .main)
        return storyboard.instantiateViewController(identifier: storyboardID) as? RomDetailViewController
    }
    
    var recipe: Rom? {
        didSet {
            updateUI()
        }
    }
    
    @IBOutlet weak var recipeFavoriteButton: UIBarButtonItem!
    
    private var topChildController: RomController?
    private var bottomChildController: RomController?

    var noRecipeView: UIView!
    
    private var dataStoreSubscriber: AnyCancellable?

    override func viewDidLoad() {
        super.viewDidLoad()
        addNoRecipeView()
        if recipe == nil {
            toggleNoRecipeView(show: true, animated: false)
        }

        
        
        // Listen for recipe changes in the data store.
        dataStoreSubscriber = dataStore.$allRecipes
            .receive(on: RunLoop.main)
            .sink { [weak self] recipes in
                guard
                    let self = self,
                    let recipe = self.recipe,
                    let updatedRecipe = recipes.first(where: { $0.id == recipe.id })
                else { return }
                
                self.recipe = updatedRecipe
            }
        
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        if let splitViewController = self.splitViewController {
            navigationItem.leftBarButtonItem = splitViewController.displayModeButtonItem
            navigationItem.leftItemsSupplementBackButton = true
        }
        
        navigationController?.setToolbarHidden(false, animated: animated)
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        navigationController?.setToolbarHidden(true, animated: animated)
    }
    
}

// MARK: - Actions

extension RomDetailViewController {
    
    @IBAction func toggleFavorite(_ sender: Any?) {
        guard var recipe = self.recipe else { return }
        
        recipe.isFavorite.toggle()
        self.recipe = dataStore.update(recipe)
    }
    
    @IBAction func deleteRecipe(_ sender: Any?) {
        guard let recipe = self.recipe else { return }
        
        let deleteAction = UIAlertAction(title: "Delete", style: .destructive) { (action) in
            if dataStore.delete(recipe) {
                self.recipe = nil
            }
        }
        
        let cancelAction = UIAlertAction(title: "Cancel", style: .cancel, handler: nil)
        
        let alert = UIAlertController(title: "Are you sure you want to delete \(recipe.title)?", message: nil, preferredStyle: .actionSheet)
        alert.addAction(deleteAction)
        alert.addAction(cancelAction)
        
        if let popoverPresentationController = alert.popoverPresentationController {
            popoverPresentationController.barButtonItem = sender as? UIBarButtonItem
        }
        
        present(alert, animated: true, completion: nil)
    }
    
    @IBAction func shareRecipe(_ sender: Any?) {
        guard let recipe = self.recipe else { return }
        
        let items: [Any] = [recipe.title, recipe.fullImage]
        let activityViewController = UIActivityViewController(activityItems: items, applicationActivities: nil)
        activityViewController.completionWithItemsHandler = { activity, completed, items, error in
            os_log("Activity completed: %s", completed ? "true" : "false")
        }

        // Configure the popover presentation controller for iPad.
        if let popover = activityViewController.popoverPresentationController {
            if let barButtonItem = sender as? UIBarButtonItem {
                popover.barButtonItem = barButtonItem
            }
        }
        present(activityViewController, animated: true, completion: nil)
    }
    
}

// MARK: - UI Helpers

extension RomDetailViewController {
    
    fileprivate func updateUI() {
        loadViewIfNeeded()

        guard  let recipe = self.recipe
        else {
            toggleNoRecipeView(show: true)
            return
        }

        self.title = recipe.title
        topChildController?.recipe = recipe
        bottomChildController?.recipe = recipe
        
        let imageName = recipe.isFavorite ? "star.fill" : "star"
        recipeFavoriteButton.image = UIImage(systemName: imageName)
        
        toggleNoRecipeView(show: false)
    }
    
    fileprivate func addNoRecipeView() {
        let backgroundView = UIView()
        backgroundView.backgroundColor = UIColor.systemBackground
        backgroundView.translatesAutoresizingMaskIntoConstraints = false
        
        let label = UILabel()
        label.translatesAutoresizingMaskIntoConstraints = false
        label.textAlignment = .center
        label.textColor = UIColor.systemGray2
        label.font = UIFont.preferredFont(forTextStyle: .largeTitle)
        label.adjustsFontForContentSizeCategory = true
        label.text = "No ROM Selected"
        
        noRecipeView = UIView()
        noRecipeView.translatesAutoresizingMaskIntoConstraints = false
        noRecipeView.alpha = 0.0
        noRecipeView.addSubview(backgroundView)
        noRecipeView.addSubview(label)
        self.view.addSubview(noRecipeView)

        NSLayoutConstraint.activate([
            noRecipeView.topAnchor.constraint(equalTo: self.view.safeAreaLayoutGuide.topAnchor),
            noRecipeView.bottomAnchor.constraint(equalTo: self.view.safeAreaLayoutGuide.bottomAnchor),
            noRecipeView.leadingAnchor.constraint(equalTo: self.view.safeAreaLayoutGuide.leadingAnchor),
            noRecipeView.trailingAnchor.constraint(equalTo: self.view.safeAreaLayoutGuide.trailingAnchor),

            backgroundView.topAnchor.constraint(equalTo: noRecipeView.topAnchor),
            backgroundView.bottomAnchor.constraint(equalTo: noRecipeView.bottomAnchor),
            backgroundView.leadingAnchor.constraint(equalTo: noRecipeView.leadingAnchor),
            backgroundView.trailingAnchor.constraint(equalTo: noRecipeView.trailingAnchor),

            label.centerXAnchor.constraint(equalTo: noRecipeView.centerXAnchor),
            label.centerYAnchor.constraint(equalTo: noRecipeView.centerYAnchor),
            label.widthAnchor.constraint(equalTo: noRecipeView.widthAnchor)
        ])
    }
    
    fileprivate func toggleNoRecipeView(show: Bool, animated: Bool = true) {
        if animated {
            UIViewPropertyAnimator.runningPropertyAnimator(
                withDuration: 0.2,
                delay: 0.0,
                options: .curveEaseIn,
                animations: {
                    self.noRecipeView.alpha = show ? 1.0 : 0.0
                    if show == true {
                        self.title = nil
                        self.navigationController?.isToolbarHidden = true
                        self.navigationItem.rightBarButtonItem = nil
                    }
                },
                completion: nil
            )
        } else {
            noRecipeView.alpha = show ? 1.0 : 0.0
        }
    }
    
    fileprivate func displayContent(controller: UIViewController, to view: UIView?) {
        guard let view = view else { return }
        
        addChild(controller)
        controller.view.frame = view.bounds
        view.addSubview(controller.view)
        controller.didMove(toParent: self)
    }
    
    fileprivate func hideContent(controller: UIViewController) {
        controller.willMove(toParent: nil)
        controller.view.removeFromSuperview()
        controller.removeFromParent()
    }
}
