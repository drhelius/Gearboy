//
//  PathUtils.swift
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 12/1/21.
//  Copyright © 2021 Apple. All rights reserved.
//

import Foundation

struct PathUtils {
    static let dbFileName = "db.json"
    static let gamedbFileName = "gbdb.json"
    
    static var getDBDir: URL {
        
        let dir = getDataDir.appendingPathComponent("database")
        
        do {
            if (FileManager.default.fileExists(atPath: dir.path, isDirectory: nil) == false) {
                try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true, attributes: nil)
            }
        } catch {
            debugPrint("ERROR: Cannot create /database")
        }
        
        return dir
    }

    static var getDataDir: URL {
        
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
}



