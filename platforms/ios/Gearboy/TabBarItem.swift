/*
See LICENSE folder for this sample’s licensing information.

Abstract:
A set of tab bar items.
*/

import UIKit

enum TabBarItem: Int, CaseIterable {
    case all, favorites, recents, collections
    
    func title() -> String {
        switch self {
        case .all:
            return "All Recipes"
        case .favorites:
            return "Favorites"
        case .recents:
            return "Recents"
        case .collections:
            return "Collections"
        }
    }
    
    func image() -> UIImage? {
        switch self {
        case .all:
            return UIImage(systemName: "tray")
        case .favorites:
            return UIImage(systemName: "heart.circle")
        case .recents:
            return UIImage(systemName: "clock")
        case .collections:
            return UIImage(systemName: "folder")
        }
    }
}
