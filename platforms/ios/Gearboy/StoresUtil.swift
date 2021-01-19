/*
See LICENSE folder for this sample’s licensing information.

Abstract:

*/

import Foundation

// SINGLETONS !!
let gameStore = GameStore(games: load(gamedbFileName, bundle: true) ?? [Game]())
let dataStore = DataStore(roms: load(dbFileName, bundle: false) ?? [Rom]())



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







