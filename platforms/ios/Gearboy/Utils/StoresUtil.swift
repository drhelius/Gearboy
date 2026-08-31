/*
See LICENSE folder for this sample’s licensing information.

Abstract:

*/

import Foundation

// SINGLETONS !!
let dataStore: DataStore = {
    let file = PathUtils.getDBDir.appendingPathComponent(PathUtils.dbFileName)
    let fileExists = FileManager.default.fileExists(atPath: file.path)
    let store = DataStore(roms: load(PathUtils.dbFileName, bundle: false) ?? [Rom]())

    if !fileExists {
        store.updateAll()
    }

    return store
}()



func load<T: Decodable>(_ filename: String, bundle: Bool) -> T? {
    
    let file: URL
    
    if bundle {
        file = Bundle.main.url(forResource: filename, withExtension: nil)!
    } else {
        file = PathUtils.getDBDir.appendingPathComponent(filename)
        guard FileManager.default.fileExists(atPath: file.path) else { return nil }
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




