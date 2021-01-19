/*
See LICENSE folder for this sample’s licensing information.

Abstract:

*/

import UIKit

struct Rom: Hashable, Codable {
    let crc: String
    let title: String
    let file: String
    var isFavorite: Bool = false
    var usedOn: Date? = Date()
    var urlImages: [URL] { BoxArt.allCases.compactMap { $0.urlWithName(title) } }
    
    init(crc: String, title: String, file: String) {
        self.crc = crc
        self.title = title
        self.file = file
    }
}
