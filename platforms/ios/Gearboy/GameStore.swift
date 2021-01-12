//
//  GameStore.swift
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 12/1/21.
//  Copyright © 2021 Apple. All rights reserved.
//

import Foundation

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
