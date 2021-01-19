/*
See LICENSE folder for this sample’s licensing information.

Abstract:
A set of tab bar items.
*/

import UIKit

enum TabBarItem: Int, CaseIterable {
    case all, favorites, recents, settings
    
    var title: String {
        switch self {
        case .all: return L10n("Common::AllRoms")
        case .favorites: return L10n("Common::Favourites")
        case .recents: return L10n("Common::Recents")
        case .settings: return L10n("Common::Settings")
        }
    }
    
    var image: UIImage? {
        switch self {
        case .all: return UIImage(systemName: "memorychip")
        case .favorites: return UIImage(systemName: "star")
        case .recents: return UIImage(systemName: "clock")
        case .settings: return UIImage(systemName: "gearshape")
        }
    }
}
