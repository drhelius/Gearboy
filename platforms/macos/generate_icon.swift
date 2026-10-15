import Foundation
import CoreGraphics
import ImageIO

struct Bitmap
{
    var width: Int
    var height: Int
    var pixels: [Float]
}

let usage = """
Usage: generate_icon png <input> <output> <size>
       generate_icon ico <input> <output> <size>...
       generate_icon appiconset <input> <appiconset_dir>
"""

func fail(_ message: String) -> Never
{
    FileHandle.standardError.write((message + "\n").data(using: .utf8)!)
    exit(1)
}

func loadBitmap(_ path: String) -> (Bitmap, Bool, CGColorSpace)
{
    guard let source = CGImageSourceCreateWithURL(URL(fileURLWithPath: path) as CFURL, nil),
          let image = CGImageSourceCreateImageAtIndex(source, 0, nil) else
    {
        fail("Unable to load \(path)")
    }
    let alphaInfo = image.alphaInfo
    let hasAlpha = alphaInfo != .none && alphaInfo != .noneSkipFirst && alphaInfo != .noneSkipLast
    var space = CGColorSpace(name: CGColorSpace.sRGB)!
    if let imageSpace = image.colorSpace, imageSpace.model == .rgb
    {
        space = imageSpace
    }
    let width = image.width
    let height = image.height
    guard let context = CGContext(data: nil, width: width, height: height, bitsPerComponent: 8, bytesPerRow: width * 4,
                                  space: space, bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue) else
    {
        fail("Unable to decode \(path)")
    }
    context.draw(image, in: CGRect(x: 0, y: 0, width: width, height: height))
    let data = context.data!.assumingMemoryBound(to: UInt8.self)
    var bitmap = Bitmap(width: width, height: height, pixels: [Float](repeating: 0, count: width * height * 4))
    for i in 0..<bitmap.pixels.count
    {
        bitmap.pixels[i] = Float(data[i]) / 255
    }
    return (bitmap, hasAlpha, space)
}

func transpose(_ src: Bitmap) -> Bitmap
{
    var dst = Bitmap(width: src.height, height: src.width, pixels: [Float](repeating: 0, count: src.pixels.count))
    for y in 0..<src.height
    {
        for x in 0..<src.width
        {
            let s = (y * src.width + x) * 4
            let d = (x * src.height + y) * 4
            for c in 0..<4
            {
                dst.pixels[d + c] = src.pixels[s + c]
            }
        }
    }
    return dst
}

func convolveRows(_ src: Bitmap, _ width: Int, _ first: [Int], _ weights: [[Float]]) -> Bitmap
{
    var dst = Bitmap(width: width, height: src.height, pixels: [Float](repeating: 0, count: width * src.height * 4))
    for y in 0..<src.height
    {
        let row = y * src.width
        for x in 0..<width
        {
            var r: Float = 0
            var g: Float = 0
            var b: Float = 0
            var a: Float = 0
            let w = weights[x]
            for i in 0..<w.count
            {
                let s = (row + min(max(first[x] + i, 0), src.width - 1)) * 4
                r += src.pixels[s] * w[i]
                g += src.pixels[s + 1] * w[i]
                b += src.pixels[s + 2] * w[i]
                a += src.pixels[s + 3] * w[i]
            }
            let d = (y * width + x) * 4
            dst.pixels[d] = r
            dst.pixels[d + 1] = g
            dst.pixels[d + 2] = b
            dst.pixels[d + 3] = a
        }
    }
    return dst
}

func lanczos(_ x: Double) -> Double
{
    if x == 0
    {
        return 1
    }
    if x <= -3 || x >= 3
    {
        return 0
    }
    let px = Double.pi * x
    return 3 * sin(px) * sin(px / 3) / (px * px)
}

func resizeRows(_ src: Bitmap, _ width: Int) -> Bitmap
{
    let scale = Double(src.width) / Double(width)
    let filterScale = max(scale, 1)
    let support = 3 * filterScale
    var first = [Int]()
    var weights = [[Float]]()
    for x in 0..<width
    {
        let center = (Double(x) + 0.5) * scale
        let start = max(Int(center - support + 0.5), 0)
        let end = min(Int(center + support + 0.5), src.width)
        var w = [Double]()
        for i in start..<end
        {
            w.append(lanczos((Double(i) - center + 0.5) / filterScale))
        }
        let total = w.reduce(0, +)
        first.append(start)
        weights.append(w.map { Float($0 / total) })
    }
    return convolveRows(src, width, first, weights)
}

func blurRows(_ src: Bitmap, _ sigma: Double) -> Bitmap
{
    let radius = Int((sigma * 3).rounded(.up))
    var w = [Double]()
    for i in -radius...radius
    {
        w.append(exp(-Double(i * i) / (2 * sigma * sigma)))
    }
    let total = w.reduce(0, +)
    let kernel = w.map { Float($0 / total) }
    var first = [Int]()
    for x in 0..<src.width
    {
        first.append(x - radius)
    }
    return convolveRows(src, src.width, first, [[Float]](repeating: kernel, count: src.width))
}

func resize(_ src: Bitmap, _ size: Int) -> Bitmap
{
    return transpose(resizeRows(transpose(resizeRows(src, size)), size))
}

func sharpen(_ src: Bitmap, _ sigma: Double, _ amount: Float) -> Bitmap
{
    let blurred = transpose(blurRows(transpose(blurRows(src, sigma)), sigma))
    var dst = src
    for i in stride(from: 0, to: src.pixels.count, by: 4)
    {
        let a = min(max(src.pixels[i + 3] + (src.pixels[i + 3] - blurred.pixels[i + 3]) * amount, 0), 1)
        for c in 0..<3
        {
            dst.pixels[i + c] = min(max(src.pixels[i + c] + (src.pixels[i + c] - blurred.pixels[i + c]) * amount, 0), a)
        }
        dst.pixels[i + 3] = a
    }
    return dst
}

func makeIcon(_ src: Bitmap, _ size: Int) -> Bitmap
{
    let icon = resize(src, size)
    if size <= 24
    {
        return sharpen(icon, 0.6, 0.6)
    }
    if size <= 48
    {
        return sharpen(icon, 0.7, 0.4)
    }
    if size <= 128
    {
        return sharpen(icon, 0.8, 0.25)
    }
    return icon
}

func pngData(_ bitmap: Bitmap, _ alpha: Bool, _ space: CGColorSpace) -> Data
{
    var bytes = [UInt8](repeating: 0, count: bitmap.pixels.count)
    for i in 0..<bytes.count
    {
        bytes[i] = UInt8(min(max(bitmap.pixels[i] * 255, 0), 255).rounded())
    }
    let info = alpha ? CGImageAlphaInfo.premultipliedLast : CGImageAlphaInfo.noneSkipLast
    let output = NSMutableData()
    guard let provider = CGDataProvider(data: Data(bytes) as CFData),
          let image = CGImage(width: bitmap.width, height: bitmap.height, bitsPerComponent: 8, bitsPerPixel: 32,
                              bytesPerRow: bitmap.width * 4, space: space, bitmapInfo: CGBitmapInfo(rawValue: info.rawValue),
                              provider: provider, decode: nil, shouldInterpolate: false, intent: .defaultIntent),
          let destination = CGImageDestinationCreateWithData(output as CFMutableData, "public.png" as CFString, 1, nil) else
    {
        fail("Unable to encode PNG")
    }
    CGImageDestinationAddImage(destination, image, nil)
    if !CGImageDestinationFinalize(destination)
    {
        fail("Unable to encode PNG")
    }
    return output as Data
}

func save(_ data: Data, _ path: String)
{
    if (try? data.write(to: URL(fileURLWithPath: path))) == nil
    {
        fail("Unable to write \(path)")
    }
}

func appendLE(_ data: inout Data, _ value: Int, _ count: Int)
{
    for i in 0..<count
    {
        data.append(UInt8((value >> (i * 8)) & 0xFF))
    }
}

func icoData(_ source: Bitmap, _ space: CGColorSpace, _ sizes: [Int]) -> Data
{
    let images = sizes.map { pngData(makeIcon(source, $0), true, space) }
    var data = Data()
    appendLE(&data, 0, 2)
    appendLE(&data, 1, 2)
    appendLE(&data, images.count, 2)
    var offset = 6 + 16 * images.count
    for i in 0..<images.count
    {
        appendLE(&data, sizes[i] & 0xFF, 1)
        appendLE(&data, sizes[i] & 0xFF, 1)
        appendLE(&data, 0, 2)
        appendLE(&data, 1, 2)
        appendLE(&data, 32, 2)
        appendLE(&data, images[i].count, 4)
        appendLE(&data, offset, 4)
        offset += images[i].count
    }
    for image in images
    {
        data.append(image)
    }
    return data
}

func generateAppIconSet(_ source: Bitmap, _ space: CGColorSpace, _ dir: String)
{
    guard let json = try? Data(contentsOf: URL(fileURLWithPath: dir + "/Contents.json")),
          let contents = (try? JSONSerialization.jsonObject(with: json)) as? [String: Any],
          let images = contents["images"] as? [[String: Any]] else
    {
        fail("Unable to read \(dir)/Contents.json")
    }
    for image in images
    {
        guard let filename = image["filename"] as? String,
              let size = image["size"] as? String,
              let scale = image["scale"] as? String,
              let points = Double(size.split(separator: "x")[0]),
              let factor = Double(scale.dropLast()) else
        {
            continue
        }
        save(pngData(makeIcon(source, Int((points * factor).rounded())), false, space), dir + "/" + filename)
    }
}

let args = CommandLine.arguments

if args.count < 4
{
    fail(usage)
}

let (source, hasAlpha, space) = loadBitmap(args[2])

switch args[1]
{
case "png":
    guard args.count == 5, let size = Int(args[4]), size > 0 else
    {
        fail(usage)
    }
    save(pngData(makeIcon(source, size), hasAlpha, space), args[3])
case "ico":
    let sizes = args.dropFirst(4).compactMap { Int($0) }.filter { $0 > 0 && $0 <= 256 }
    if sizes.isEmpty || sizes.count != args.count - 4
    {
        fail(usage)
    }
    save(icoData(source, space, sizes), args[3])
case "appiconset":
    generateAppIconSet(source, space, args[3])
default:
    fail(usage)
}
