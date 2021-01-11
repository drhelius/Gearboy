/*
See LICENSE folder for this sample’s licensing information.

Abstract:

*/

import UIKit
import ImageIO

let dbFileName = "db.json"
let gamedbFileName = "gb.json"
let gbBoxartsPath: String = "http://thumbnails.libretro.com/Nintendo%20-%20Game%20Boy/Named_Boxarts/"
let gbcBoxartsPath = "http://thumbnails.libretro.com/Nintendo%20-%20Game%20Boy%20Color/Named_Boxarts/"

let gameStore = GameStore(games: load(gamedbFileName, bundle: true) ?? [Game]())
let dataStore = DataStore(roms: load(dbFileName, bundle: false) ?? [Rom]())

func getDBDir () -> URL {
    
    let dir = getDataDir().appendingPathComponent("database")
    
    do {
        if (FileManager.default.fileExists(atPath: dir.path, isDirectory: nil) == false) {
            try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true, attributes: nil)
        }
    } catch {
        debugPrint("ERROR: Cannot create /database")
    }
    
    return dir
}

func getDataDir () -> URL {
    
    let dir: URL
    
    if let iCloudDocumentsDirectory = FileManager.default.url(forUbiquityContainerIdentifier: nil)?.appendingPathComponent("Documents") {
        dir = iCloudDocumentsDirectory
    } else {
        let paths = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)
        let documentsDirectory = paths[0]
        dir = documentsDirectory
    }
    
    do {
        if (FileManager.default.fileExists(atPath: dir.path, isDirectory: nil) == false) {
            try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true, attributes: nil)
        }
    } catch {
        debugPrint("ERROR: Cannot create /Documents")
    }
    
    return dir
}

func load<T: Decodable>(_ filename: String, bundle: Bool) -> T? {
    
    let file: URL
    
    if bundle {
        file = Bundle.main.url(forResource: filename, withExtension: nil)!
    } else {
        file = getDBDir().appendingPathComponent(filename)
    }
    
    let data: Data
    
    do {
        data = try Data(contentsOf: file)
    } catch {
        debugPrint("Couldn't load \(filename):\n\(error)")
        return nil
    }
    
    do {
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .iso8601
        
        return try decoder.decode(T.self, from: data)
    } catch {
        fatalError("Couldn't parse \(filename) as \(T.self):\n\(error)")
    }
}

final class ImageStore {
    typealias _ImageDictionary = [String: UIImage]
    fileprivate var images: _ImageDictionary = [:]

    fileprivate static var scale = 2
    
    static var shared = ImageStore()
    
    func image(name: String) -> UIImage {
        let index = _guaranteeImage(name: name)
        return images.values[index]
    }
    
    func add(_ image: UIImage, with name: String) {
        images[name] = image
    }
    
    func downloadImage(rom: Rom) {
        
        if rom.title == "" {
            return
        }
        
        let imageFile = rom.title + ".png"
        let imageURL = (URL(string: gbBoxartsPath)?.appendingPathComponent(rom.title + ".png"))!
        
        let dstURL = getDBDir().appendingPathComponent(imageFile)
        
        if FileManager.default.fileExists(atPath: dstURL.path) {
            return
        }
        
        DispatchQueue.global().async { [weak self] in
            if let data = try? Data(contentsOf: imageURL) {
                
                FileManager.default.createFile(atPath: dstURL.path, contents: data)
                
                if let image = UIImage(data: data) {
                    DispatchQueue.main.async {
                        self?.add(image, with: imageFile)
                        //rom.image = image
                        //_ = dataStore.update(rom)
                    }
                }
            }
        }
    }

    static func loadImage(name: String) -> UIImage {
        
        debugPrint("temp \(gameStore.title(crc: "ss")):\n")
        
        guard
            let url = Bundle.main.url(forResource: name, withExtension: "jpg"),
            let imageSource = CGImageSourceCreateWithURL(url as NSURL, nil),
            let cgImage = CGImageSourceCreateImageAtIndex(imageSource, 0, nil)
        else {
            fatalError("Couldn't load image \(name).jpg from the main bundle.")
        }
        return UIImage(cgImage: cgImage, scale: CGFloat(ImageStore.scale), orientation: .up)
    }
    
    fileprivate func _guaranteeImage(name: String) -> _ImageDictionary.Index {
        if let index = images.index(forKey: name) { return index }
        
        images[name] = ImageStore.loadImage(name: name)
        return images.index(forKey: name)!
    }
}

class GameStore {
    typealias _GameDictionary = [String: String]
    
    var gamesDict: _GameDictionary = [:]
    
    init(games: [Game]) {
        for g in games {
            gamesDict[g.crc] = g.title
        }
    }
    
    func title(crc: String) -> String {
        return gamesDict[crc] ?? ""
    }
}

class DataStore: ObservableObject {
    @Published var allRoms: [Rom]

    init(roms: [Rom]) {
        self.allRoms = roms
    }
    
    func newRom() -> Rom {
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd'T'HH:mm:ss.SSSZZZZZ"
        formatter.timeZone = TimeZone(secondsFromGMT: 0)
        formatter.locale = Locale(identifier: "en_US_POSIX")

        let json = """
            {
                "id": 0,
                "file": "file",
                "title": "New ROM",
                "isFavorite": false,
                "crc": "00000000",
                "imageName": "Cartridge"
            }
        """
        let data = Data(json.utf8)
        
        do {
            let decoder = JSONDecoder()
            return try decoder.decode(Rom.self, from: data)
        } catch {
            fatalError("Invalid ROM JSON.")
        }
    }
    
    func addFromURL(_ url: URL) {
        
        let dataDir = getDataDir()
        let fileName = url.lastPathComponent
        let dstURL = dataDir.appendingPathComponent(fileName)
        
        if (url.deletingLastPathComponent().path != dataDir.path) {
            do {
                if FileManager.default.fileExists(atPath: dstURL.path) {
                    try FileManager.default.removeItem(at: dstURL)
                }
                try FileManager.default.copyItem(at: url, to: dstURL)
            } catch (let error) {
                debugPrint("Cannot copy item at \(url) to \(dstURL): \(error)")
            }
        }
        
        var rom = newRom()
        rom.file = fileName
        
        if (dataStore.rom(with: rom.file) == nil) {
            _ = dataStore.add(rom)
        }
    }
    
    func add(_ rom: Rom) -> Rom {
        var romToAdd = rom
        romToAdd.id = (allRoms.map { $0.id }.max() ?? 0) + 1
        
        let romURL = getDataDir().appendingPathComponent(rom.file)
        let romData = FileManager.default.contents(atPath: romURL.path)
        let checksum = CRC32.checksum(bytes: romData!)
        romToAdd.crc = String(format:"%08X", checksum)
        romToAdd.title = gameStore.title(crc: romToAdd.crc)
        
        ImageStore.shared.downloadImage(rom: romToAdd)
        
        allRoms.append(romToAdd)
        save()
        return romToAdd
    }
    
    func delete(_ rom: Rom) -> Bool {
        var deleted = false
        if let index = allRoms.firstIndex(where: { $0.id == rom.id }) {
            allRoms.remove(at: index)
            deleted = true
            save()
        }
        return deleted
    }
    
    func update(_ rom: Rom) -> Rom? {
        var romToReturn: Rom? = nil // Return nil if the rom doesn't exist.
        if let index = allRoms.firstIndex(where: { $0.id == rom.id }) {
            allRoms.remove(at: index)
            allRoms.insert(rom, at: index)
            romToReturn = rom
            save()
        }
        return romToReturn
    }
    
    func rom(with id: Int) -> Rom? {
        return allRoms.first(where: { $0.id == id })
    }
    
    func rom(with file: String) -> Rom? {
        return allRoms.first(where: { $0.file == file })
    }
    
    fileprivate func save() {
        
        let file = getDBDir().appendingPathComponent(dbFileName)
    
        do {
            let encoder = JSONEncoder()
            encoder.dateEncodingStrategy = .iso8601
            
            try encoder.encode(allRoms).write(to: file)
            
        } catch {
            fatalError("Couldn't save \(file):\n\(error)")
        }
    }
}

class CRC32 {
    
    static var table: [UInt32] = {
        (0...255).map { i -> UInt32 in
            (0..<8).reduce(UInt32(i), { c, _ in
                (c % 2 == 0) ? (c >> 1) : (0xEDB88320 ^ (c >> 1))
            })
        }
    }()

    static func checksum<T: DataProtocol>(bytes: T) -> UInt32 {
        return ~(bytes.reduce(~UInt32(0), { crc, byte in
            (crc >> 8) ^ table[(Int(crc) ^ Int(byte)) & 0xFF]
        }))
    }
}
