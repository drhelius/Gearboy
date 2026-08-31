/*
See LICENSE folder for this sample’s licensing information.

Abstract:
A custom tab bar controller.
*/

import UIKit

final class TabBarController: UITabBarController {
    override func viewDidLoad() {
        super.viewDidLoad()

        viewControllers = [
            libraryController(for: .all),
            libraryController(for: .favorites),
            libraryController(for: .recents),
            settingsController()
        ]
    }

    private func libraryController(for item: TabBarItem) -> UINavigationController {
        let viewController = RomListViewController(dataType: item)
        let navigationController = UINavigationController(rootViewController: viewController)
        navigationController.title = item.title
        navigationController.tabBarItem = tabBarItem(for: item)
        return navigationController
    }

    private func settingsController() -> UINavigationController {
        let navigationController = UINavigationController(rootViewController: SettingsViewController())
        navigationController.title = TabBarItem.settings.title
        navigationController.tabBarItem = tabBarItem(for: .settings)
        return navigationController
    }

    private func tabBarItem(for item: TabBarItem) -> UITabBarItem {
        UITabBarItem(title: item.title, image: item.image, tag: item.rawValue)
    }
}
