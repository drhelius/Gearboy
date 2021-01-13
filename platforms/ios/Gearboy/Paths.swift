//
//  Paths.swift
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 12/1/21.
//  Copyright © 2021 Apple. All rights reserved.
//

import Foundation

let boxartExtension = ".png"
let dbFileName = "db.json"
let gamedbFileName = "gbdb.json"
let gbBoxartsPath = "http://thumbnails.libretro.com/Nintendo%20-%20Game%20Boy/Named_Boxarts/"
let gbcBoxartsPath = "http://thumbnails.libretro.com/Nintendo%20-%20Game%20Boy%20Color/Named_Boxarts/"
let romExtensions = [ "gb", "gbc", "cgb", "sgb", "rom", "dmg", "zip" ]

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
