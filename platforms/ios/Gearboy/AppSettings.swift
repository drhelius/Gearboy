import Foundation

enum GameBoyModelOption: Int, CaseIterable {
    case automatic
    case gameBoy
    case gameBoyAdvance

    var title: String {
        switch self {
        case .automatic:
            return L10n("Settings::Automatic")
        case .gameBoy:
            return "Game Boy DMG"
        case .gameBoyAdvance:
            return "Game Boy Advance"
        }
    }

    var summaryTitle: String {
        switch self {
        case .automatic:
            return title
        case .gameBoy:
            return "DMG"
        case .gameBoyAdvance:
            return "GBA"
        }
    }
}

enum MapperOption: Int, CaseIterable {
    case automatic
    case romOnly
    case mbc1
    case mbc2
    case mbc3
    case mbc5
    case mbc1Multicart
    case huC1
    case huC3
    case mmm01
    case camera
    case mbc7
    case tama5
    case wisdomTree
    case m161
    case sachenMMC1
    case sachenMMC2
    case pkjd
    case bungEMS
    case poke2in1
    case mbc6

    var title: String {
        switch self {
        case .automatic:
            return L10n("Settings::Automatic")
        case .romOnly:
            return "ROM Only"
        case .mbc1:
            return "MBC 1"
        case .mbc2:
            return "MBC 2"
        case .mbc3:
            return "MBC 3"
        case .mbc5:
            return "MBC 5"
        case .mbc1Multicart:
            return "MBC 1 Multicart"
        case .huC1:
            return "HuC 1"
        case .huC3:
            return "HuC 3"
        case .mmm01:
            return "MMM01"
        case .camera:
            return L10n("Settings::Camera")
        case .mbc7:
            return "MBC 7"
        case .tama5:
            return "TAMA5"
        case .wisdomTree:
            return "Wisdom Tree"
        case .m161:
            return "M161"
        case .sachenMMC1:
            return "Sachen MMC1"
        case .sachenMMC2:
            return "Sachen MMC2"
        case .pkjd:
            return "PKJD"
        case .bungEMS:
            return "Bung/EMS"
        case .poke2in1:
            return "Poke 2-in-1"
        case .mbc6:
            return "MBC 6"
        }
    }
}

enum DMGPaletteOption: Int, CaseIterable {
    case original
    case sharp
    case blackAndWhite
    case autumn
    case soft
    case slime

    var title: String {
        switch self {
        case .original:
            return L10n("Settings::PaletteOriginal")
        case .sharp:
            return L10n("Settings::PaletteSharp")
        case .blackAndWhite:
            return L10n("Settings::PaletteBlackWhite")
        case .autumn:
            return L10n("Settings::PaletteAutumn")
        case .soft:
            return L10n("Settings::PaletteSoft")
        case .slime:
            return L10n("Settings::PaletteSlime")
        }
    }
}

enum AppSettings {
    private enum Key {
        static let audioEnabled = "settings.audioEnabled"
        static let hapticsEnabled = "settings.hapticsEnabled"
        static let smoothingEnabled = "settings.smoothingEnabled"
        static let screenSize = "settings.screenSize"
        static let gameBoyModel = "settings.gameBoyModel"
        static let mapper = "settings.mapper"
        static let superGameBoyEnabled = "settings.superGameBoyEnabled"
        static let superGameBoyBorderEnabled = "settings.superGameBoyBorderEnabled"
        static let dmgPalette = "settings.dmgPalette"
        static let colorCorrectionEnabled = "settings.colorCorrectionEnabled"
        static let noSpriteLimitEnabled = "settings.noSpriteLimitEnabled"
        static let saveStateSlot = "settings.saveStateSlot"
        static let motionTiltEnabled = "settings.motionTiltEnabled"
        static let tiltSensitivityX = "settings.tiltSensitivityX"
        static let tiltSensitivityY = "settings.tiltSensitivityY"
        static let tiltInvertX = "settings.tiltInvertX"
        static let tiltInvertY = "settings.tiltInvertY"
    }

    static func registerDefaults() {
        UserDefaults.standard.register(defaults: [
            Key.audioEnabled: true,
            Key.hapticsEnabled: true,
            Key.smoothingEnabled: false,
            Key.screenSize: ScreenSizeOption.fitToWidth.rawValue,
            Key.gameBoyModel: GameBoyModelOption.automatic.rawValue,
            Key.mapper: MapperOption.automatic.rawValue,
            Key.superGameBoyEnabled: true,
            Key.superGameBoyBorderEnabled: false,
            Key.dmgPalette: DMGPaletteOption.original.rawValue,
            Key.colorCorrectionEnabled: false,
            Key.noSpriteLimitEnabled: false,
            Key.saveStateSlot: 1,
            Key.motionTiltEnabled: true,
            Key.tiltSensitivityX: 5,
            Key.tiltSensitivityY: 5,
            Key.tiltInvertX: false,
            Key.tiltInvertY: false
        ])
    }

    static var audioEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: Key.audioEnabled) }
        set { UserDefaults.standard.set(newValue, forKey: Key.audioEnabled) }
    }

    static var hapticsEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: Key.hapticsEnabled) }
        set { UserDefaults.standard.set(newValue, forKey: Key.hapticsEnabled) }
    }

    static var smoothingEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: Key.smoothingEnabled) }
        set { UserDefaults.standard.set(newValue, forKey: Key.smoothingEnabled) }
    }

    static var screenSize: ScreenSizeOption {
        get { ScreenSizeOption(rawValue: UserDefaults.standard.integer(forKey: Key.screenSize)) ?? .fitToWidth }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.screenSize) }
    }

    static var gameBoyModel: GameBoyModelOption {
        get {
            GameBoyModelOption(rawValue: UserDefaults.standard.integer(forKey: Key.gameBoyModel)) ?? .automatic
        }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.gameBoyModel) }
    }

    static var mapper: MapperOption {
        get {
            MapperOption(rawValue: UserDefaults.standard.integer(forKey: Key.mapper)) ?? .automatic
        }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.mapper) }
    }

    static var superGameBoyEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: Key.superGameBoyEnabled) }
        set { UserDefaults.standard.set(newValue, forKey: Key.superGameBoyEnabled) }
    }

    static var superGameBoyBorderEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: Key.superGameBoyBorderEnabled) }
        set { UserDefaults.standard.set(newValue, forKey: Key.superGameBoyBorderEnabled) }
    }

    static var dmgPalette: DMGPaletteOption {
        get {
            DMGPaletteOption(rawValue: UserDefaults.standard.integer(forKey: Key.dmgPalette)) ?? .original
        }
        set { UserDefaults.standard.set(newValue.rawValue, forKey: Key.dmgPalette) }
    }

    static var colorCorrectionEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: Key.colorCorrectionEnabled) }
        set { UserDefaults.standard.set(newValue, forKey: Key.colorCorrectionEnabled) }
    }

    static var noSpriteLimitEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: Key.noSpriteLimitEnabled) }
        set { UserDefaults.standard.set(newValue, forKey: Key.noSpriteLimitEnabled) }
    }

    static var saveStateSlot: Int {
        get { min(max(UserDefaults.standard.integer(forKey: Key.saveStateSlot), 1), 5) }
        set { UserDefaults.standard.set(min(max(newValue, 1), 5), forKey: Key.saveStateSlot) }
    }

    static var motionTiltEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: Key.motionTiltEnabled) }
        set { UserDefaults.standard.set(newValue, forKey: Key.motionTiltEnabled) }
    }

    static var tiltSensitivityX: Int {
        get { min(max(UserDefaults.standard.integer(forKey: Key.tiltSensitivityX), 1), 10) }
        set { UserDefaults.standard.set(min(max(newValue, 1), 10), forKey: Key.tiltSensitivityX) }
    }

    static var tiltSensitivityY: Int {
        get { min(max(UserDefaults.standard.integer(forKey: Key.tiltSensitivityY), 1), 10) }
        set { UserDefaults.standard.set(min(max(newValue, 1), 10), forKey: Key.tiltSensitivityY) }
    }

    static var tiltInvertX: Bool {
        get { UserDefaults.standard.bool(forKey: Key.tiltInvertX) }
        set { UserDefaults.standard.set(newValue, forKey: Key.tiltInvertX) }
    }

    static var tiltInvertY: Bool {
        get { UserDefaults.standard.bool(forKey: Key.tiltInvertY) }
        set { UserDefaults.standard.set(newValue, forKey: Key.tiltInvertY) }
    }
}
