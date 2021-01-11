/*
See LICENSE folder for this sample’s licensing information.

Abstract:

*/

import UIKit

struct Rom: Hashable, Codable, Identifiable {
    var id: Int
    var file: String
    var title: String
    var isFavorite: Bool
    var crc: String
    var usedOn: Date? = Date()
    var imageName: String? = nil
}

extension Rom {
    var usedOnDate: Date {
        usedOn ?? Date()
    }

//    var image: UIImage {
//        guard let name = imageName, !name.isEmpty else { return #imageLiteral(resourceName: "Cartridge.jpg") }
//        return ImageStore.shared.image(name: name)
//    }
    
//    func add(_ image: UIImage) {
//        guard let name = imageName, !name.isEmpty else { return }
//        ImageStore.shared.add(image, with: name)
//    }
}
