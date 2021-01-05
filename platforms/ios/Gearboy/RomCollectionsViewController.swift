/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The recipe collections view controller.
*/

import UIKit
import Combine

class RomCollectionsViewController: UITableViewController {
    
    private var dataStoreSubscriber: AnyCancellable?
    private var collectionNames = [String]()
    private static var reuseIdentifier = "Category Row Cell"
    
    fileprivate var selectedRecipe: Rom?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        collectionNames = dataStore.collections.sorted()
        dataStoreSubscriber = dataStore.$collections
            .receive(on: RunLoop.main)
            .sink { [weak self] recipes in
                guard let self = self else { return }
                self.collectionNames = dataStore.collections.sorted()
                self.tableView.reloadData()
            }
    }
     
    override func numberOfSections(in tableView: UITableView) -> Int {
        return collectionNames.count
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return 1
    }
    
    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        return collectionNames[section]
    }
    
    override func tableView(_ tableView: UITableView, heightForRowAt indexPath: IndexPath) -> CGFloat {
        return 120
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: RomCollectionsViewController.reuseIdentifier) as! RecipeCollectionRowCell
        let allRecipes = dataStore.allRecipes
        let rowRecipes = allRecipes.filter { $0.collections.contains(collectionNames[indexPath.section]) }
        cell.recipes = rowRecipes
        cell.categoryName = collectionNames[indexPath.section]
        cell.recipeCollectionViewController = self
        return cell
    }
    
    override func tableView(_ tableView: UITableView, willDisplayHeaderView view: UIView, forSection section: Int) {
        view.tintColor = UIColor.systemBackground
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        guard let navController = segue.destination as? UINavigationController else { return }
        
        if let destination = navController.topViewController as? RomDetailViewController {
            destination.recipe = selectedRecipe
            selectedRecipe = nil
        }
    }
}

class RecipeCollectionRowCell: UITableViewCell, UICollectionViewDataSource, UICollectionViewDelegate {
    @IBOutlet weak var collectionView: UICollectionView!
    var recipeCollectionViewController: RomCollectionsViewController?
    var recipes: [Rom]?
    var categoryName: String?
    
    private static var cellIdentifier = "Collection Recipe Cell"
    
    override func awakeFromNib() {
        super.awakeFromNib()
        collectionView.register(RomListCell.nib, forCellWithReuseIdentifier: RomListCell.reuseIdentifier)
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        guard let recipes = recipes else { return 0 }
        return recipes.count
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: RomListCell.reuseIdentifier, for: indexPath)
        if let cell = cell as? RomListCell {
            if let recipes = recipes {
                cell.configure(with: recipes[indexPath.row])
            }
        }
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        if let recipe = recipes?[indexPath.row], let recipeCollectionViewController = recipeCollectionViewController {
            recipeCollectionViewController.selectedRecipe = recipe
            recipeCollectionViewController.performSegue(withIdentifier: "CollectionToDetail", sender: self)
            collectionView.deselectItem(at: indexPath, animated: false)
        }
    }
}
