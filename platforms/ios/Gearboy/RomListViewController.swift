/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The ROM library collection view controller.
*/

import UIKit
import Combine

final class RomListViewController: UIViewController {
    private static let maximumRecentRoms = 18
    private static let indexGutter: CGFloat = 28.0

    private let dataType: TabBarItem
    private let collectionView = UICollectionView(frame: .zero, collectionViewLayout: UICollectionViewFlowLayout())
    private var dataStoreSubscriber: AnyCancellable?

    private var visibleRoms: [Rom] {
        switch dataType {
        case .all:
            return dataStore.allRoms
        case .favorites:
            return dataStore.allRoms.filter { $0.isFavorite }
        case .recents:
            let recents = dataStore.allRoms
                .filter { $0.usedOn != nil }
                .sorted { ($0.usedOn ?? .distantPast) > ($1.usedOn ?? .distantPast) }
            return Array(recents.prefix(Self.maximumRecentRoms))
        case .settings:
            return []
        }
    }

    init(dataType: TabBarItem) {
        self.dataType = dataType
        super.init(nibName: nil, bundle: nil)
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        navigationItem.title = L10n(AppConfiguration.libraryTitleLocalizationKey)
        view.backgroundColor = .systemBackground
        navigationItem.largeTitleDisplayMode = .never

        if dataType == .all {
            navigationItem.rightBarButtonItem = UIBarButtonItem(
                barButtonSystemItem: .refresh,
                target: self,
                action: #selector(refreshLibrary)
            )
        }

        configureCollectionView()
        registerCells()

        dataStoreSubscriber = dataStore.$allRoms
            .receive(on: RunLoop.main)
            .sink { [weak self] _ in
                self?.collectionView.reloadData()
            }
    }

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        collectionView.reloadData()
    }

    @objc private func refreshLibrary() {
        dataStore.updateAll()
    }
}

extension RomListViewController: UICollectionViewDelegate {
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        guard visibleRoms.indices.contains(indexPath.item) else { return }

        let rom = visibleRoms[indexPath.item]
        collectionView.deselectItem(at: indexPath, animated: true)

        let emulationViewController = EmulationViewController(rom: rom)
        emulationViewController.modalPresentationStyle = .fullScreen
        present(emulationViewController, animated: true)
    }

    func collectionView(
        _ collectionView: UICollectionView,
        contextMenuConfigurationForItemAt indexPath: IndexPath,
        point: CGPoint
    ) -> UIContextMenuConfiguration? {
        guard visibleRoms.indices.contains(indexPath.item) else { return nil }
        let rom = visibleRoms[indexPath.item]

        return UIContextMenuConfiguration(identifier: rom.crc as NSString, previewProvider: nil) { [weak self] _ in
            guard let self else { return UIMenu() }

            let favoriteTitle = rom.isFavorite
                ? L10n("Library::RemoveFavorite")
                : L10n("Library::AddFavorite")
            let favoriteAction = UIAction(
                title: favoriteTitle,
                image: UIImage(systemName: rom.isFavorite ? "star.slash" : "star")
            ) { [weak self] _ in
                self?.toggleFavorite(crc: rom.crc)
            }

            let shareAction = UIAction(
                title: L10n("Library::ShareRom"),
                image: UIImage(systemName: "square.and.arrow.up")
            ) { [weak self, weak collectionView] _ in
                let sourceView = collectionView?.cellForItem(at: indexPath)
                self?.share(rom: rom, sourceView: sourceView)
            }

            let deleteAction = UIAction(
                title: L10n("Common::Delete"),
                image: UIImage(systemName: "trash"),
                attributes: .destructive
            ) { [weak self, weak collectionView] _ in
                let sourceView = collectionView?.cellForItem(at: indexPath)
                self?.confirmDelete(rom: rom, sourceView: sourceView)
            }

            return UIMenu(children: [favoriteAction, shareAction, deleteAction])
        }
    }
}

extension RomListViewController: UICollectionViewDataSource {
    func numberOfSections(in collectionView: UICollectionView) -> Int {
        1
    }

    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        visibleRoms.count
    }

    func collectionView(
        _ collectionView: UICollectionView,
        cellForItemAt indexPath: IndexPath
    ) -> UICollectionViewCell {
        let identifier = String(describing: RomListCell.self)
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: identifier, for: indexPath)
        let rom = visibleRoms[indexPath.item]

        (cell as? RomListCell)?.configure(
            with: rom,
            onFavoriteToggle: { [weak self] in
                self?.toggleFavorite(crc: rom.crc)
            },
            onDelete: { [weak self, weak cell] in
                self?.confirmDelete(rom: rom, sourceView: cell)
            }
        )
        return cell
    }

    func collectionView(
        _ collectionView: UICollectionView,
        indexPathForIndexTitle title: String,
        at index: Int
    ) -> IndexPath {
        let item = visibleRoms.firstIndex {
            $0.file.prefix(1).uppercased() == title
        } ?? 0
        return IndexPath(item: item, section: 0)
    }

    func indexTitles(for collectionView: UICollectionView) -> [String]? {
        Array(Set(visibleRoms.map { String($0.file.prefix(1).uppercased()) })).sorted()
    }
}

private extension RomListViewController {
    func configureCollectionView() {
        collectionView.translatesAutoresizingMaskIntoConstraints = false
        collectionView.backgroundColor = .systemBackground
        collectionView.alwaysBounceVertical = true
        collectionView.delegate = self
        collectionView.dataSource = self
        collectionView.collectionViewLayout = createCollectionViewLayout()
        view.addSubview(collectionView)

        NSLayoutConstraint.activate([
            collectionView.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor),
            collectionView.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor),
            collectionView.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor),
            collectionView.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor)
        ])
    }

    func createCollectionViewLayout() -> UICollectionViewLayout {
        UICollectionViewCompositionalLayout { _, environment in
            let width = environment.container.effectiveContentSize.width - Self.indexGutter
            let isPad = environment.traitCollection.userInterfaceIdiom == .pad
            let idealItemWidth: CGFloat = isPad ? 150.0 : 120.0
            let minimumColumns = isPad ? 2 : 3
            let maximumColumns = isPad ? 8 : 7
            let columnCount = min(max(Int(width / idealItemWidth), minimumColumns), maximumColumns)

            let itemSize = NSCollectionLayoutSize(
                widthDimension: .fractionalWidth(1.0 / CGFloat(columnCount)),
                heightDimension: .fractionalHeight(1.0)
            )
            let item = NSCollectionLayoutItem(layoutSize: itemSize)
            item.contentInsets = NSDirectionalEdgeInsets(top: 5.0, leading: 5.0, bottom: 5.0, trailing: 5.0)

            let groupSize = NSCollectionLayoutSize(
                widthDimension: .fractionalWidth(1.0),
                heightDimension: .fractionalWidth(1.0 / CGFloat(columnCount))
            )
            let group = NSCollectionLayoutGroup.horizontal(
                layoutSize: groupSize,
                repeatingSubitem: item,
                count: columnCount
            )

            let section = NSCollectionLayoutSection(group: group)
            section.contentInsets = NSDirectionalEdgeInsets(
                top: 0.0,
                leading: 0.0,
                bottom: 0.0,
                trailing: Self.indexGutter
            )
            return section
        }
    }

    func registerCells() {
        let identifier = String(describing: RomListCell.self)
        let nib = UINib(nibName: identifier, bundle: .main)
        collectionView.register(nib, forCellWithReuseIdentifier: identifier)
    }

    func toggleFavorite(crc: String) {
        guard var rom = dataStore.allRoms.first(where: { $0.crc == crc }) else { return }
        rom.isFavorite.toggle()
        _ = dataStore.update(rom)
    }

    func share(rom: Rom, sourceView: UIView?) {
        let romURL = PathUtils.getDataDir.appendingPathComponent(rom.file)
        guard FileManager.default.fileExists(atPath: romURL.path) else { return }

        let activityViewController = UIActivityViewController(activityItems: [romURL], applicationActivities: nil)
        activityViewController.popoverPresentationController?.sourceView = sourceView ?? view
        activityViewController.popoverPresentationController?.sourceRect = sourceView?.bounds ?? view.bounds
        present(activityViewController, animated: true)
    }

    func confirmDelete(rom: Rom, sourceView: UIView?) {
        let title = String(format: L10n("Common::DeleteRom"), rom.file)
        let alert = UIAlertController(title: title, message: nil, preferredStyle: .actionSheet)

        alert.addAction(UIAlertAction(title: L10n("Common::Delete"), style: .destructive) { _ in
            _ = dataStore.delete(rom)
        })
        alert.addAction(UIAlertAction(title: L10n("Common::Cancel"), style: .cancel))
        alert.popoverPresentationController?.sourceView = sourceView ?? view
        alert.popoverPresentationController?.sourceRect = sourceView?.bounds ?? view.bounds
        present(alert, animated: true)
    }
}
