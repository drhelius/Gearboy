import Foundation

enum ThumbnailAuthorization {
    static let headerName = "X-Gear-Thumbnail-Key"

    static let key: String? = {
        guard
            let value = Bundle.main.object(forInfoDictionaryKey: "GearThumbnailKey") as? String,
            value.range(of: "^[0-9a-f]{64}$", options: .regularExpression) != nil
        else {
            return nil
        }

        return value
    }()
}
