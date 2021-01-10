/*
See LICENSE folder for this sample’s licensing information.

Abstract:

*/

import UIKit
import ImageIO

let dataFileName = "romData.json"
let dataStore = DataStore(roms: load("romData.json") ?? [Rom]())

func getDataDir () -> URL {
    
    let dir: URL
    
    if let iCloudDocumentsDirectory = FileManager.default.url(forUbiquityContainerIdentifier: nil)?.appendingPathComponent("Documents") {
        dir = iCloudDocumentsDirectory
        
        do {
            if (FileManager.default.fileExists(atPath: dir.path, isDirectory: nil) == false) {
                try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true, attributes: nil)
            }
        } catch {
            debugPrint("ERROR: Cannot create /Documents on iCloud")
        }
    } else {
        let paths = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)
        let documentsDirectory = paths[0]
        dir = documentsDirectory
    }
    
    //    guard let dir = Bundle.main.url(forResource: filename, withExtension: nil)
    //    else {
    //        fatalError("Couldn't find \(filename) in main bundle.")
    //    }
    
    return dir
}

func getDataFile () -> URL {
    
    return getDataDir().appendingPathComponent(dataFileName)
}

func load<T: Decodable>(_ filename: String) -> T? {
    
    let file = getDataFile()
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

    static func loadImage(name: String) -> UIImage {
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
                "title": "New ROM",
                "isFavorite": false,
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
        rom.title = fileName
        
        if (dataStore.rom(with: rom.title) == nil) {
            _ = dataStore.add(rom)
        }
    }
    
    func add(_ rom: Rom) -> Rom {
        var romToAdd = rom
        romToAdd.id = (allRoms.map { $0.id }.max() ?? 0) + 1
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
    
    func rom(with title: String) -> Rom? {
        return allRoms.first(where: { $0.title == title })
    }
    
    fileprivate func save() {
        
        let file = getDataFile()
    
        do {
            let encoder = JSONEncoder()
            encoder.dateEncodingStrategy = .iso8601
            
            try encoder.encode(allRoms).write(to: file)
            
        } catch {
            fatalError("Couldn't save \(file):\n\(error)")
        }
    }
    
}
