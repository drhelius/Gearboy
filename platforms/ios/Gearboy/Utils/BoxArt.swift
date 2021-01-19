//
//  BoxArt.swift
//  Gearboy
//
//  Created by Francisco Javier Trujillo Mata on 19/01/2021.
//  Copyright © 2021 Apple. All rights reserved.
//

import Foundation

enum BoxArt: String, CaseIterable {
    case libretroGB
    case libretroGBC
    
    var pathExtension: String {
        switch self {
        case .libretroGB, .libretroGBC: return ".png"
        }
    }
    
    func urlWithName(_ name: String) -> URL? {
        let baseURL: URL?
        switch self {
        case .libretroGB: baseURL = URL(string: "http://thumbnails.libretro.com/Nintendo%20-%20Game%20Boy/Named_Boxarts/")
        case .libretroGBC: baseURL = URL(string: "http://thumbnails.libretro.com/Nintendo%20-%20Game%20Boy%20Color/Named_Boxarts/")
        }
        return baseURL?.appendingPathComponent(name + pathExtension)
    }
}
