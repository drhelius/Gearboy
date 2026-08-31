enum ScreenSizeOption: Int, CaseIterable {
    case fitToWidth
    case integerScale

    var title: String {
        switch self {
        case .fitToWidth:
            return L10n("Settings::FitToWidth")
        case .integerScale:
            return L10n("Settings::IntegerScale")
        }
    }
}
