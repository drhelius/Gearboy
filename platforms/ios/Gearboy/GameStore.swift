//
//  GameStore.swift
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 12/1/21.
//  Copyright © 2021 Apple. All rights reserved.
//

import Foundation

struct GameStore {
    private let store: [String: String]
    
    init(games: [Game]) {
        store = games.reduce(into: [String: String]()) { $0[$1.crc] = $1.title }
    }
    
    func titleWithCRC(_ crc: String) -> String { store[crc] ?? "" }
}
