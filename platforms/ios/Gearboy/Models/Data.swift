/*
See LICENSE folder for this sample’s licensing information.

Abstract:

*/

import UIKit
import ImageIO

let dataStore = DataStore(recipes: load("recipeData.json"))

func load<T: Decodable>(_ filename: String) -> T {
    let data: Data
    
    guard let file = Bundle.main.url(forResource: filename, withExtension: nil)
    else {
        fatalError("Couldn't find \(filename) in main bundle.")
    }
    
    do {
        data = try Data(contentsOf: file)
    } catch {
        fatalError("Couldn't load \(filename) from main bundle:\n\(error)")
    }
    
    do {
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .iso8601
        
        return try decoder.decode(T.self, from: data)
    } catch {
        fatalError("Couldn't parse \(filename) as \(T.self):\n\(error)")
    }
}

final class ImageStore {
    typealias _ImageDictionary = [String: UIImage]
    fileprivate var images: _ImageDictionary = [:]

    fileprivate static var scale = 2
    
    static var shared = ImageStore()
    
    func image(name: String) -> UIImage {
        let index = _guaranteeImage(name: name)
        return images.values[index]
    }
    
    func add(_ image: UIImage, with name: String) {
        images[name] = image
    }

    static func loadImage(name: String) -> UIImage {
        guard
            let url = Bundle.main.url(forResource: name, withExtension: "jpg"),
            let imageSource = CGImageSourceCreateWithURL(url as NSURL, nil),
            let cgImage = CGImageSourceCreateImageAtIndex(imageSource, 0, nil)
        else {
            fatalError("Couldn't load image \(name).jpg from the main bundle.")
        }
        return UIImage(cgImage: cgImage, scale: CGFloat(ImageStore.scale), orientation: .up)
    }
    
    fileprivate func _guaranteeImage(name: String) -> _ImageDictionary.Index {
        if let index = images.index(forKey: name) { return index }
        
        images[name] = ImageStore.loadImage(name: name)
        return images.index(forKey: name)!
    }
}

class DataStore: ObservableObject {
    @Published var allRecipes: [Rom]
    @Published var collections: [String]

    init(recipes: [Rom]) {
        self.allRecipes = recipes
        self.collections = DataStore.collection(from: recipes)
    }
    
    fileprivate static func collection(from recipes: [Rom]) -> [String] {
        var allCollections = Set<String>()
        for recipe in recipes {
            allCollections.formUnion(recipe.collections)
        }
        return allCollections.sorted()
    }
    
    func newRecipe() -> Rom {
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd'T'HH:mm:ss.SSSZZZZZ"
        formatter.timeZone = TimeZone(secondsFromGMT: 0)
        formatter.locale = Locale(identifier: "en_US_POSIX")

        let json = """
            {
                "id": 0,
                "title": "New Recipe",
                "prepTime": 0,
                "cookTime": 0,
                "servings": "",
                "ingredients": "",
                "directions": "",
                "isFavorite": false,
                "collections": [],
                "imageNames": [\"\(UUID().uuidString)\"]
            }
        """
        let data = Data(json.utf8)
        
        do {
            let decoder = JSONDecoder()
            return try decoder.decode(Rom.self, from: data)
        } catch {
            fatalError("Invalid recipe JSON.")
        }
    }
    
    func add(_ recipe: Rom) -> Rom {
        var recipeToAdd = recipe
        recipeToAdd.id = (allRecipes.map { $0.id }.max() ?? 0) + 1
        allRecipes.append(recipeToAdd)
        updateCollectionsIfNeeded()
        return recipeToAdd
    }
    
    func delete(_ recipe: Rom) -> Bool {
        var deleted = false
        if let index = allRecipes.firstIndex(where: { $0.id == recipe.id }) {
            allRecipes.remove(at: index)
            deleted = true
            updateCollectionsIfNeeded()
        }
        return deleted
    }
    
    func update(_ recipe: Rom) -> Rom? {
        var recipeToReturn: Rom? = nil // Return nil if the recipe doesn't exist.
        if let index = allRecipes.firstIndex(where: { $0.id == recipe.id }) {
            allRecipes.remove(at: index)
            allRecipes.insert(recipe, at: index)
            recipeToReturn = recipe
            updateCollectionsIfNeeded()
        }
        return recipeToReturn
    }
    
    func recipe(with id: Int) -> Rom? {
        return allRecipes.first(where: { $0.id == id })
    }

    fileprivate func updateCollectionsIfNeeded() {
        let updatedCollection = DataStore.collection(from: allRecipes)
        if collections != updatedCollection {
            collections = updatedCollection
        }
    }
    
}
