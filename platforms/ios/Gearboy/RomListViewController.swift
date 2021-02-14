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

    enum Section: Int, CaseIterable {
        case main
    }

    private var selectedDataType: TabBarItem = .all
    
    private var allRoms: [Rom] {
        switch selectedDataType {
        case .favorites: return dataStore.allRoms.filter { $0.isFavorite }
        default: return dataStore.allRoms
        }
    }
    
    private var dataStoreSubscriber: AnyCancellable?

    override func viewDidLoad() {
        super.viewDidLoad()
        
        if let tabBarController = self.tabBarController,
           let dataType = TabBarItem(rawValue: tabBarController.selectedIndex) {
            selectedDataType = dataType
        }

        configureCollectionView()
        registerCells()
        
        // Listen for rom changes in the data store.
        dataStoreSubscriber = dataStore.$allRoms
            .receive(on: RunLoop.main)
            .sink { [weak self] roms in
                guard let self = self else { return }
                self.apply(roms)
            }
    }

    @IBAction func updateAll(_ sender: Any) {
        dataStore.updateAll()
    }
    
}

extension RomListViewController: UICollectionViewDelegate {

    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        guard let romDetailViewController = RomDetailViewController.instantiateFromStoryboard() else { return }
        
        romDetailViewController.rom = dataStore.allRoms[indexPath.row]
        let navigationController = UINavigationController(rootViewController: romDetailViewController)
        showDetailViewController(navigationController, sender: self)
    }

}

extension RomListViewController: UICollectionViewDataSource {
    func numberOfSections(in collectionView: UICollectionView) -> Int {
        Section.allCases.count
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        allRoms.count
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let identifier = String(describing: RomListCell.self)
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: identifier, for: indexPath)

        (cell as? RomListCell)?.configure(with: allRoms[indexPath.row])
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, indexPathForIndexTitle title: String, at index: Int) -> IndexPath {
        guard let index = allRoms.firstIndex(where: { $0.file.prefix(1).uppercased() == title }) else {
            return IndexPath(item: .zero, section: .zero)
        }
        return IndexPath(item: index, section: .zero)
    }
    
    func indexTitles(for collectionView: UICollectionView) -> [String]? {
        return Array(Set(allRoms.map{ String($0.file.prefix(1).uppercased()) })).sorted(by: { $0 < $1 })
    }
}

extension RomListViewController {
    func configureCollectionView() {
        collectionView.delegate = self
        collectionView.dataSource = self
        collectionView.alwaysBounceVertical = true
        collectionView.collectionViewLayout = createCollectionViewLayout()
    }
    
    func createCollectionViewLayout() -> UICollectionViewLayout {
        let romItemSize = NSCollectionLayoutSize(widthDimension: .fractionalWidth(1.0), heightDimension: .fractionalHeight(1.0))
        let romItem = NSCollectionLayoutItem(layoutSize: romItemSize)
        romItem.contentInsets = NSDirectionalEdgeInsets(top: 5.0, leading: 5.0, bottom: 5.0, trailing: 5.0)
        
        let groupSize = NSCollectionLayoutSize(widthDimension: .fractionalWidth(1.0), heightDimension: .fractionalWidth(0.333))
        let group = NSCollectionLayoutGroup.horizontal(layoutSize: groupSize, subitem: romItem, count: 3)
        
        let section = NSCollectionLayoutSection(group: group)
        let layout = UICollectionViewCompositionalLayout(section: section)
        
        return layout
    }
    
}

// MARK: - Private Methods
private extension RomListViewController {
    func registerCells() {
        let identifier = String(describing: RomListCell.self)
        let nibFile = UINib(nibName: identifier, bundle: .main)
        collectionView.register(nibFile, forCellWithReuseIdentifier: identifier)
    }
    
    func apply(_ roms: [Rom]) {
        //collectionView.reloadSections([Section.main.rawValue])
        collectionView.reloadData()
    }
}
