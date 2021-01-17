//
//  DataStore.swift
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 12/1/21.
//  Copyright © 2021 Apple. All rights reserved.
//

import Foundation

class DataStore: ObservableObject {
    @Published var allRoms: [Rom]
    
    var runningUpdate = false

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
                "file": "",
                "title": "",
                "isFavorite": false,
                "crc": "00000000",
                "image": ""
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
    
    func addFromURL(_ url: URL, downloadImage: Bool) -> Rom? {
        
        let dataDir = getDataDir()
        let fileName = url.lastPathComponent
        let dstURL = dataDir.appendingPathComponent(fileName)
        
        if (url.deletingLastPathComponent().path != dataDir.path) {
            
            _ = url.startAccessingSecurityScopedResource()
            do {
                if FileManager.default.fileExists(atPath: dstURL.path) {
                    try FileManager.default.removeItem(at: dstURL)
                }
                try FileManager.default.copyItem(at: url, to: dstURL)
            } catch (let error) {
                debugPrint("Cannot copy item at \(url) to \(dstURL): \(error)")
            }
            url.stopAccessingSecurityScopedResource()
        }
        
        var rom = dataStore.rom(with: fileName)
        
        if (rom == nil) {
            var new = newRom()
            new.file = fileName
            rom = dataStore.add(new)
        }
        
        if downloadImage {
            ImageStore.shared.downloadImage(rom: rom!, blocking: false)
        }
        
        return rom
    }
    
    func add(_ rom: Rom) -> Rom {
        var romToAdd = rom
        romToAdd.id = (allRoms.map { $0.id }.max() ?? 0) + 1
        
        let romURL = getDataDir().appendingPathComponent(rom.file)
        let romData = FileManager.default.contents(atPath: romURL.path)
        let checksum = CRC32.checksum(bytes: romData!)
        romToAdd.crc = String(format:"%08X", checksum)
        romToAdd.title = gameStore.title(crc: romToAdd.crc)
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
    
    func updateAll() {
        
        if !runningUpdate {
            DispatchQueue.global(qos: .background).async { [weak self] in
                
                self?.runningUpdate = true;
                
                let romsDirectory = getDataDir()
                
                self?.allRoms.forEach { rom in
                    
                    let romFile = rom.file
                    let romFileURL = romsDirectory.appendingPathComponent(romFile)
                    
                    debugPrint("DB: \(rom.file)")
                    
                    if !FileManager.default.fileExists(atPath: romFileURL.path) {
                        _ = self?.delete(rom)
                    }
                }
                
                do {
                    let directoryContents = try FileManager.default.contentsOfDirectory(at: romsDirectory, includingPropertiesForKeys: nil)

                    let romFiles = directoryContents.filter{ romExtensions.contains($0.pathExtension.lowercased()) }
                    
                    romFiles.forEach { file in
                        debugPrint("File: \(file)")
                        
                        let fileName = file.lastPathComponent
                        
                        let rom = dataStore.rom(with: fileName)
                        
                        if (rom == nil) {
                            let newRom = self?.addFromURL(file, downloadImage: false)
                            
                            if (newRom != nil) {
                                ImageStore.shared.downloadImage(rom: newRom!, blocking: true)
                            }
                        }
                    }
                    
                    romFiles.forEach { file in
                        debugPrint("Image: \(file)")
                        
                        let fileName = file.lastPathComponent
                        let rom = dataStore.rom(with: fileName)
                        if (rom != nil) {
                            ImageStore.shared.downloadImage(rom: rom!, blocking: true)
                        }
                    }

                } catch {
                    debugPrint(error)
                }
                
                self?.runningUpdate = false;
            }
        }
    }
    
    func rom(with id: Int) -> Rom? {
        return allRoms.first(where: { $0.id == id })
    }
    
    func rom(with file: String) -> Rom? {
        return allRoms.first(where: { $0.file == file })
    }
    
    fileprivate func save() {
        
        allRoms.sort { $0.file.uppercased() < $1.file.uppercased() }
        
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
