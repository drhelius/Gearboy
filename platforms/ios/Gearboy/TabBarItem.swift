/*
See LICENSE folder for this sample’s licensing information.

Abstract:
A set of tab bar items.
*/

import UIKit

enum TabBarItem: Int, CaseIterable {
    case all, favorites, recents, settings
    
    func title() -> String {
        switch self {
        case .all:
            return "All ROMs"
        case .favorites:
            return "Favorites"
        case .recents:
            return "Recents"
        case .settings:
            return "Settings"
        }
    }
    
    func image() -> UIImage? {
        switch self {
        case .all:
            return UIImage(systemName: "memorychip")
        case .favorites:
            return UIImage(systemName: "star")
        case .recents:
            return UIImage(systemName: "clock")
        case .settings:
            return UIImage(systemName: "gearshape")
        }
    }
}
