/*
See LICENSE folder for this sample’s licensing information.

Abstract:
A custom tab bar controller.
*/

import UIKit

class TabBarController: UITabBarController {

    override func viewDidLoad() {
        super.viewDidLoad()

        if let items = tabBar.items {
            for item in TabBarItem.allCases {
                if traitCollection.userInterfaceIdiom == .phone {
                    items[item.rawValue].title = item.title()
                } else {
                    items[item.rawValue].title = ""
                }
                items[item.rawValue].image = item.image()
            }
        }
    }
    
}
