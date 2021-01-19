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
    
    func addFromURL(_ url: URL) {
        
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
        
        guard dataStore.rom(with: fileName) != nil else { return }
        dataStore.addWithFileName(fileName)
    }
    
    func addWithFileName(_ fileName: String) {
        let romURL = getDataDir().appendingPathComponent(fileName)
        guard let romData = FileManager.default.contents(atPath: romURL.path) else { return }
        
        let checksum = CRC32.checksum(bytes: romData)
        let crc = String(format:"%08X", checksum)
        let title = gameStore.titleWithCRC(crc)
        
        allRoms.append(Rom(crc: crc, title: title, file: fileName))
        save()
    }
    
    func delete(_ rom: Rom) -> Bool {
        guard let index = (allRoms.firstIndex { $0.crc == rom.crc }) else { return false }
        
        allRoms.remove(at: index)
        return true
    }
    
    func update(_ rom: Rom) -> Rom? {
        var romToReturn: Rom? = nil // Return nil if the rom doesn't exist.
        if let index = allRoms.firstIndex(where: { $0.crc == rom.crc }) {
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
                        
                        guard dataStore.rom(with: file.lastPathComponent) == nil else { return }
                        self?.addFromURL(file)
                    }
                } catch {
                    debugPrint(error)
                }
                
                self?.runningUpdate = false;
            }
        }
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
