//
//  ImageStore.swift
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 12/1/21.
//  Copyright © 2021 Apple. All rights reserved.
//

import UIKit

final class ImageStore {
    typealias _ImageDictionary = [String: UIImage]
    fileprivate var images: _ImageDictionary = [:]

    fileprivate static var scale = 64
    
    static var shared = ImageStore()
    
    func image(name: String) -> UIImage {
        let index = _guaranteeImage(name: name)
        return images.values[index]
    }
    
    func downloadImage(rom: Rom, blocking: Bool) {
        
        if rom.title == "" {
            return
        }
        
        let imageFile = rom.title + boxartExtension
        
        let imageURL = (URL(string: gbBoxartsPath)?.appendingPathComponent(imageFile))!
        
        let dstURL = getDBDir().appendingPathComponent(imageFile)
        
        if FileManager.default.fileExists(atPath: dstURL.path) {
            
            if rom.image == "" {
                var updatedRom = rom
                updatedRom.image = imageFile
                _ = dataStore.update(updatedRom)
            }
            return
        }
        
        if blocking {
            _download(rom: rom, imageURL: imageURL, imageFile: imageFile, dstURL: dstURL)
        } else {
            DispatchQueue.global(qos: .background).async { [weak self] in
                self?._download(rom: rom, imageURL: imageURL, imageFile: imageFile, dstURL: dstURL)
            }
        }
    }
    
    func resizedImage(at url: URL, for size: CGSize) -> UIImage? {
        guard let image = UIImage(contentsOfFile: url.path) else {
            return nil
        }

        let renderer = UIGraphicsImageRenderer(size: size)
        return renderer.image { (context) in
            image.draw(in: CGRect(origin: .zero, size: size))
        }
    }

    static func loadImage(name: String) -> UIImage {
        
        let url = getDBDir().appendingPathComponent(name)
        
        guard let image = UIImage(contentsOfFile: url.path) else {
            return #imageLiteral(resourceName: "Cartridge.jpg")
        }
        
        let size = CGSize(width: scale, height: scale)
        
        let renderer = UIGraphicsImageRenderer(size: size)
        return renderer.image { (context) in
            image.draw(in: CGRect(origin: .zero, size: size))
        }
    }
    
    fileprivate func _guaranteeImage(name: String) -> _ImageDictionary.Index {
        if let index = images.index(forKey: name) { return index }
        
        images[name] = ImageStore.loadImage(name: name)
        return images.index(forKey: name)!
    }
    
    fileprivate func _download(rom: Rom, imageURL: URL, imageFile: String, dstURL: URL) {
        if let data = try? Data(contentsOf: imageURL) {
            
            FileManager.default.createFile(atPath: dstURL.path, contents: data)
            
            self.images[imageFile] = ImageStore.loadImage(name: imageFile)
            
            var updatedRom = rom
            updatedRom.image = imageFile
            _ = dataStore.update(updatedRom)
        }
    }
}
