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
    var usedOn: Date? = nil
    var thumbnailURL: URL {
        return AppConfiguration.thumbnailBaseURL.appendingPathComponent(crc.lowercased() + ".jpg")
    }
    
    init(crc: String, title: String, file: String) {
        self.crc = crc
        self.title = title
        self.file = file
    }
}
