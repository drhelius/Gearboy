//
//  PathUtils.swift
//  Gear iOS
//
//  Created by Ignacio Sánchez Ginés on 12/1/21.
//  Copyright © 2021 Apple. All rights reserved.
//

import Foundation

struct PathUtils {
    static let dbFileName = "library.json"
    private static let dataDirectoryLock = NSLock()
    private static var dataDirectory: URL?

    static func prepareDataDirectories(createBIOSDirectory: Bool, completion: @escaping () -> Void) {
        DispatchQueue.global(qos: .userInitiated).async {
            let fileManager = FileManager.default
            let directory: URL

            if fileManager.ubiquityIdentityToken != nil,
               let container = fileManager.url(forUbiquityContainerIdentifier: nil) {
                directory = container.appendingPathComponent("Documents", isDirectory: true)
            } else {
                directory = localDocumentsDirectory
            }

            do {
                if !fileManager.fileExists(atPath: directory.path) {
                    try fileManager.createDirectory(at: directory, withIntermediateDirectories: true)
                }

                if createBIOSDirectory {
                    let biosDirectory = directory.appendingPathComponent("BIOS", isDirectory: true)
                    let legacyDirectory = directory.appendingPathComponent("Firmware", isDirectory: true)

                    if fileManager.fileExists(atPath: legacyDirectory.path),
                       !fileManager.fileExists(atPath: biosDirectory.path) {
                        try fileManager.moveItem(at: legacyDirectory, to: biosDirectory)
                    } else if !fileManager.fileExists(atPath: biosDirectory.path) {
                        try fileManager.createDirectory(at: biosDirectory, withIntermediateDirectories: true)
                    }
                }
            } catch {
                debugPrint("ERROR: Cannot prepare Documents directory: \(error)")
            }

            dataDirectoryLock.lock()
            dataDirectory = directory
            dataDirectoryLock.unlock()

            DispatchQueue.main.async {
                completion()
            }
        }
    }
    
    static var getDBDir: URL {
        
        let paths = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask)
        let dir = paths[0]
        
        do {
            if (FileManager.default.fileExists(atPath: dir.path, isDirectory: nil) == false) {
                try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true, attributes: nil)
            }
        } catch {
            debugPrint("ERROR: Cannot create /Library/Application Support")
        }
        
        return dir
    }

    static var getDataDir: URL {
        dataDirectoryLock.lock()
        let preparedDirectory = dataDirectory
        dataDirectoryLock.unlock()

        let dir = preparedDirectory ?? localDocumentsDirectory
        
        do {
            if (FileManager.default.fileExists(atPath: dir.path, isDirectory: nil) == false) {
                try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true, attributes: nil)
            }
        } catch {
            debugPrint("ERROR: Cannot create /Documents")
        }
        
        return dir
    }

    private static var localDocumentsDirectory: URL {
        FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
    }
}
