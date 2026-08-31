import Foundation

enum AppConfiguration {
    static let libraryTitleLocalizationKey = "Common::Gearboy"
    static let thumbnailBaseURL = URL(string: "https://www.drhelius.com/thumbnails/gearboy/")!

    static func romCRC(inArchiveAt url: URL) -> String? {
        GearboyEmulator.romCRC(inArchiveAt: url)
    }
}
