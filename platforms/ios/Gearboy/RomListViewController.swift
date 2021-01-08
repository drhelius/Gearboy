/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The rom list view controller.
*/

import UIKit
import Combine

class RomListViewController: UIViewController {

    static let storyboardID = "RomList"
    static func instantiateFromStoryboard() -> RomListViewController? {
        let storyboard = UIStoryboard(name: "Main", bundle: .main)
        return storyboard.instantiateViewController(identifier: storyboardID) as? RomListViewController
    }

    @IBOutlet weak var collectionView: UICollectionView!

    enum Section: Int {
        case main
    }

    private var dataSource: UICollectionViewDiffableDataSource<Section, Rom>!

    private var selectedDataType: TabBarItem = .all
    private var selectedRom: Rom? {
        guard
            let indexPathsForSelectedItems = collectionView.indexPathsForSelectedItems,
            let selectedIndexPath = indexPathsForSelectedItems.first,
            let selectedRom = dataSource.itemIdentifier(for: selectedIndexPath)
        else { return nil }
        
        return selectedRom
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
        
        // Listen for rom changes in the data store.
        dataStoreSubscriber = dataStore.$allRoms
            .receive(on: RunLoop.main)
            .sink { [weak self] roms in
                guard let self = self else { return }
                self.apply(roms)
            }
    }
    
}

extension RomListViewController: UICollectionViewDelegate {

    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        guard let romDetailViewController = RomDetailViewController.instantiateFromStoryboard() else { return }
        
        romDetailViewController.rom = dataSource.itemIdentifier(for: indexPath)
        let navigationController = UINavigationController(rootViewController: romDetailViewController)
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
        let romItemSize = NSCollectionLayoutSize(widthDimension: .fractionalWidth(1.0), heightDimension: .fractionalHeight(1.0))
        let romItem = NSCollectionLayoutItem(layoutSize: romItemSize)
        romItem.contentInsets = NSDirectionalEdgeInsets(top: 5.0, leading: 10.0, bottom: 5.0, trailing: 10.0)
        
        let groupSize = NSCollectionLayoutSize(widthDimension: .fractionalWidth(1.0), heightDimension: .fractionalWidth(0.333))
        let group = NSCollectionLayoutGroup.horizontal(layoutSize: groupSize, subitem: romItem, count: 3)
        
        let section = NSCollectionLayoutSection(group: group)
        let layout = UICollectionViewCompositionalLayout(section: section)
        
        return layout
    }
    
}

extension RomListViewController {
    
    func configureDataSource() {
        // Register the cell that displays a rom in the collection view.
        collectionView.register(RomListCell.nib, forCellWithReuseIdentifier: RomListCell.reuseIdentifier)

        // Create a diffable data source, and configure the cell with rom data.
        dataSource = UICollectionViewDiffableDataSource <Section, Rom>(collectionView: self.collectionView) { (
            collectionView: UICollectionView,
            indexPath: IndexPath,
            rom: Rom) -> UICollectionViewCell? in
        
            let cell = collectionView.dequeueReusableCell(withReuseIdentifier: RomListCell.reuseIdentifier, for: indexPath)
            
            if let romListCell = cell as? RomListCell {
                romListCell.configure(with: rom)
            }
        
            return cell
        }
    }
    
    func apply(_ roms: [Rom]) {
        var snapshot = NSDiffableDataSourceSnapshot<Section, Rom>()
        snapshot.appendSections([.main])
        switch selectedDataType {
        case .favorites:
            snapshot.appendItems(roms.filter { $0.isFavorite })
        case .recents:
            //let thirtyDaysAgo = Calendar.current.date(byAdding: .day, value: -30, to: Date()) ?? Date()
            //snapshot.appendItems(roms.filter { $0.usedOnDate > thirtyDaysAgo })
            snapshot.appendItems(roms)
        default:
            snapshot.appendItems(roms)
        }
        dataSource.apply(snapshot, animatingDifferences: true)
    }

}
