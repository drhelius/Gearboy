/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The recipe list view controller.
*/

import UIKit
import Combine

class RomListViewController: UIViewController {

    static let storyboardID = "RecipeList"
    static func instantiateFromStoryboard() -> RomListViewController? {
        let storyboard = UIStoryboard(name: "Main", bundle: .main)
        return storyboard.instantiateViewController(identifier: storyboardID) as? RomListViewController
    }

    @IBOutlet weak var collectionView: UICollectionView!

    enum Section: Int {
        case main
    }

    private var dataSource: UICollectionViewDiffableDataSource<Section, Rom>!

    private var recipeCollectionName: String?
    private var selectedDataType: TabBarItem = .all
    private var selectedRecipe: Rom? {
        guard
            let indexPathsForSelectedItems = collectionView.indexPathsForSelectedItems,
            let selectedIndexPath = indexPathsForSelectedItems.first,
            let selectedRecipe = dataSource.itemIdentifier(for: selectedIndexPath)
        else { return nil }
        
        return selectedRecipe
    }
    
    private var dataStoreSubscriber: AnyCancellable?

    override func viewDidLoad() {
        super.viewDidLoad()
        
        if let tabBarController = self.tabBarController,
           let dataType = TabBarItem(rawValue: tabBarController.selectedIndex) {
            selectedDataType = dataType
        }

        configureCollectionView()
        configureDataSource()
        
        // Listen for recipe changes in the data store.
        dataStoreSubscriber = dataStore.$allRecipes
            .receive(on: RunLoop.main)
            .sink { [weak self] recipes in
                guard let self = self else { return }
                self.apply(recipes)
            }
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        guard let navController = segue.destination as? UINavigationController else { return }
        
        // Prevent the user from dismissing the recipe editor by swiping down.
        if let recipeEditor = navController.topViewController as? RomEditorViewController {
            recipeEditor.isModalInPresentation = true
        }
    }
    
}

extension RomListViewController: UICollectionViewDelegate {

    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        guard let recipeDetailViewController = RomDetailViewController.instantiateFromStoryboard() else { return }
        
        recipeDetailViewController.recipe = dataSource.itemIdentifier(for: indexPath)
        let navigationController = UINavigationController(rootViewController: recipeDetailViewController)
        showDetailViewController(navigationController, sender: self)
    }

}

extension RomListViewController {
    func configureCollectionView() {
        collectionView.delegate = self
        collectionView.alwaysBounceVertical = true
        collectionView.collectionViewLayout = createCollectionViewLayout()
    }
    
    func createCollectionViewLayout() -> UICollectionViewLayout {
        let recipeItemSize = NSCollectionLayoutSize(widthDimension: .fractionalWidth(1.0), heightDimension: .fractionalHeight(1.0))
        let recipeItem = NSCollectionLayoutItem(layoutSize: recipeItemSize)
        recipeItem.contentInsets = NSDirectionalEdgeInsets(top: 5.0, leading: 10.0, bottom: 5.0, trailing: 10.0)
        
        let groupSize = NSCollectionLayoutSize(widthDimension: .fractionalWidth(1.0), heightDimension: .fractionalWidth(0.333))
        let group = NSCollectionLayoutGroup.horizontal(layoutSize: groupSize, subitem: recipeItem, count: 3)
        
        let section = NSCollectionLayoutSection(group: group)
        let layout = UICollectionViewCompositionalLayout(section: section)
        
        return layout
    }
    
}

extension RomListViewController {
    
    func configureDataSource() {
        // Register the cell that displays a recipe in the collection view.
        collectionView.register(RomListCell.nib, forCellWithReuseIdentifier: RomListCell.reuseIdentifier)

        // Create a diffable data source, and configure the cell with recipe data.
        dataSource = UICollectionViewDiffableDataSource <Section, Rom>(collectionView: self.collectionView) { (
            collectionView: UICollectionView,
            indexPath: IndexPath,
            recipe: Rom) -> UICollectionViewCell? in
        
            let cell = collectionView.dequeueReusableCell(withReuseIdentifier: RomListCell.reuseIdentifier, for: indexPath)
            
            if let romListCell = cell as? RomListCell {
                romListCell.configure(with: recipe)
            }
        
            return cell
        }
    }
    
    func apply(_ recipes: [Rom]) {
        var snapshot = NSDiffableDataSourceSnapshot<Section, Rom>()
        snapshot.appendSections([.main])
        switch selectedDataType {
        case .favorites:
            snapshot.appendItems(recipes.filter { $0.isFavorite })
        case .recents:
            let thirtyDaysAgo = Calendar.current.date(byAdding: .day, value: -30, to: Date()) ?? Date()
            snapshot.appendItems(recipes.filter { $0.addedOnDate > thirtyDaysAgo })
        case .collections:
            if let collectionName = self.recipeCollectionName {
                let recipeCollection = recipes.filter { $0.collections.contains(collectionName) }
                snapshot.appendItems(recipeCollection)
            }
        default:
            snapshot.appendItems(recipes)
        }
        dataSource.apply(snapshot, animatingDifferences: true)
    }

}

extension RomListViewController {
    
    func showRecipes(_ tabBarItem: TabBarItem) {
        selectedDataType = tabBarItem
        apply(dataStore.allRecipes)
    }
    
    func showRecipes(from collection: String) {
        selectedDataType = .collections
        recipeCollectionName = collection
        apply(dataStore.allRecipes)
    }

}

// MARK: - Unwind Segues
extension RomListViewController {
    
    @IBAction func cancelRomEditor(_ unwindSegue: UIStoryboardSegue) {
        // Do nothing.
    }
    
    @IBAction func saveRomEditor(_ unwindSegue: UIStoryboardSegue) {
        guard
            let recipeEditor = unwindSegue.source as? RomEditorViewController,
            let recipe = recipeEditor.editedRecipe()
        else { return }

        let recipeToSelect = dataStore.add(recipe)

        if UIDevice.current.userInterfaceIdiom == .pad {
            if let indexPath = dataSource.indexPath(for: recipeToSelect) {
                collectionView.selectItem(at: indexPath, animated: true, scrollPosition: .top)
                self.collectionView(collectionView, didSelectItemAt: indexPath)
            }
        }
    }

}
