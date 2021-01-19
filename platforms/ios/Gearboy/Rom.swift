/*
See LICENSE folder for this sample’s licensing information.

Abstract:

*/

import UIKit

struct Rom: Hashable, Codable, Identifiable {
    var id: Int = .zero
    var file: String
    var title: String = ""
    var isFavorite: Bool = false
    var crc: String = ""
    var usedOn: Date? = Date()
    
    init(file: String) {
        self.file = file
    }
}

extension Rom {
    var usedOnDate: Date { usedOn ?? Date() }
    var urlImage: URL? { URL(string: gbBoxartsPath)?.appendingPathComponent( title + boxartExtension) }
}
