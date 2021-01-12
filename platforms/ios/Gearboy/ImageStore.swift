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

    fileprivate static var scale = 2
    
    static var shared = ImageStore()
    
    func image(name: String) -> UIImage {
        let index = _guaranteeImage(name: name)
        return images.values[index]
    }
    
    func downloadImage(rom: Rom) {
        
        if rom.title == "" {
            return
        }
        
        let imageFile = rom.title + boxartExtension
        
        let imageURL = (URL(string: gbBoxartsPath)?.appendingPathComponent(imageFile))!
        
        let dstURL = getDBDir().appendingPathComponent(imageFile)
        
        if FileManager.default.fileExists(atPath: dstURL.path) {
            return
        }
        
        DispatchQueue.global().async { [weak self] in
            if let data = try? Data(contentsOf: imageURL) {
                
                FileManager.default.createFile(atPath: dstURL.path, contents: data)
                
                self?.images[imageFile] = ImageStore.loadImage(name: imageFile)
                
                var updatedRom = rom
                updatedRom.image = imageFile
                _ = dataStore.update(updatedRom)
            }
        }
    }

    static func loadImage(name: String) -> UIImage {
        
        let url = getDBDir().appendingPathComponent(name)
        
        guard
            let imageSource = CGImageSourceCreateWithURL(url as NSURL, nil),
            let cgImage = CGImageSourceCreateImageAtIndex(imageSource, 0, nil)
        else {
            return #imageLiteral(resourceName: "Cartridge.jpg")
        }
        return UIImage(cgImage: cgImage, scale: CGFloat(ImageStore.scale), orientation: .up)
    }
    
    fileprivate func _guaranteeImage(name: String) -> _ImageDictionary.Index {
        if let index = images.index(forKey: name) { return index }
        
        images[name] = ImageStore.loadImage(name: name)
        return images.index(forKey: name)!
    }
}
