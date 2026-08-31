//
//  DataStore.swift
//  Gear iOS
//
//  Created by Ignacio Sánchez Ginés on 12/1/21.
//  Copyright © 2021 Apple. All rights reserved.
//

import Foundation

class DataStore: ObservableObject {
    private static let recentsMigrationKey = "migration.recents.v1"
    private static let archiveCRCMigrationKey = "migration.archive-crc.v1"

    @Published var allRoms: [Rom]
    
    var runningUpdate = false

    init(roms: [Rom]) {
        self.allRoms = roms
        var changed = false

        if !UserDefaults.standard.bool(forKey: Self.recentsMigrationKey) {
            for index in allRoms.indices {
                allRoms[index].usedOn = nil
            }

            UserDefaults.standard.set(true, forKey: Self.recentsMigrationKey)
            changed = true
        }

        if !UserDefaults.standard.bool(forKey: Self.archiveCRCMigrationKey) {
            for index in allRoms.indices
            where (allRoms[index].file as NSString).pathExtension.lowercased() == "zip" {
                let url = PathUtils.getDataDir.appendingPathComponent(allRoms[index].file)
                guard let crc = Self.crc(for: url), crc != allRoms[index].crc else { continue }

                var migratedRom = Rom(crc: crc, title: allRoms[index].title, file: allRoms[index].file)
                migratedRom.isFavorite = allRoms[index].isFavorite
                migratedRom.usedOn = allRoms[index].usedOn
                allRoms[index] = migratedRom
            }

            UserDefaults.standard.set(true, forKey: Self.archiveCRCMigrationKey)
            changed = true
        }

        if changed {
            save()
        }
    }

    private static func crc(for url: URL) -> String? {
        if url.pathExtension.lowercased() == "zip" {
            return AppConfiguration.romCRC(inArchiveAt: url)
        }

        guard let romData = FileManager.default.contents(atPath: url.path) else { return nil }
        let checksum = CRC32.checksum(bytes: romData)
        return String(format: "%08X", checksum)
    }
    
    func addFromURL(_ url: URL) {
        
        let dataDir = PathUtils.getDataDir
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
        
        guard dataStore.rom(with: fileName) == nil else { return }
        dataStore.addWithFileName(fileName)
    }
    
    func addWithFileName(_ fileName: String) {
        let romURL = PathUtils.getDataDir.appendingPathComponent(fileName)
        guard let crc = Self.crc(for: romURL) else { return }
        let title = (fileName as NSString).deletingPathExtension
        
        allRoms.append(Rom(crc: crc, title: title, file: fileName))
        save()
    }
    
    func delete(_ rom: Rom) -> Bool {
        guard let index = (allRoms.firstIndex { $0.crc == rom.crc }) else { return false }

        let romURL = PathUtils.getDataDir.appendingPathComponent(rom.file)
        do {
            if FileManager.default.fileExists(atPath: romURL.path) {
                try FileManager.default.removeItem(at: romURL)
            }
        } catch {
            debugPrint("Cannot delete \(rom.file): \(error)")
            return false
        }

        allRoms.remove(at: index)
        save()
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
        guard !runningUpdate else { return }
        runningUpdate = true

        let romsDirectory = PathUtils.getDataDir
        DispatchQueue.global(qos: .userInitiated).async { [weak self] in
            let romFiles: [URL]

            do {
                let directoryContents = try FileManager.default.contentsOfDirectory(
                    at: romsDirectory,
                    includingPropertiesForKeys: nil
                )
                romFiles = directoryContents.filter {
                    RomExtension(rawValue: $0.pathExtension.lowercased()) != nil
                }
            } catch {
                debugPrint(error)
                romFiles = []
            }

            DispatchQueue.main.async {
                guard let self else { return }

                self.allRoms.removeAll {
                    !FileManager.default.fileExists(
                        atPath: romsDirectory.appendingPathComponent($0.file).path
                    )
                }

                for file in romFiles where self.rom(with: file.lastPathComponent) == nil {
                    self.addWithFileName(file.lastPathComponent)
                }

                self.save()
                self.runningUpdate = false
            }
        }
    }
    
    func rom(with file: String) -> Rom? {
        return allRoms.first(where: { $0.file == file })
    }
    
    fileprivate func save() {
        
        allRoms.sort { $0.file.uppercased() < $1.file.uppercased() }
        
        let file = PathUtils.getDBDir.appendingPathComponent(PathUtils.dbFileName)
        
    
        do {
            let encoder = JSONEncoder()
            encoder.dateEncodingStrategy = .iso8601
            
            try encoder.encode(allRoms).write(to: file, options: .atomic)
            
        } catch {
            fatalError("Couldn't save \(file):\n\(error)")
        }
    }
}
