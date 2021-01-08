/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The app delegate.
*/

import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        return true
    }

    func application(_ application: UIApplication,
                     configurationForConnecting connectingSceneSession: UISceneSession,
                     options: UIScene.ConnectionOptions) -> UISceneConfiguration {
        return UISceneConfiguration(name: "Default Configuration", sessionRole: connectingSceneSession.role)
    }
    
    func application(_ app: UIApplication, open url: URL, options: [UIApplication.OpenURLOptionsKey : Any] = [:]) -> Bool {
        print(url)
        
        //let isOpenInPlace = options[.openInPlace] as? Bool
        
//        guard let shouldOpenInPlace = options[UIApplicationOpenURLOptionsOpenInPlaceKey] as? Bool else {
//            return false
//        }
//        
//        guard let navigation = window?.rootViewController as? UINavigationController else {
//            return false
//        }
//
//        guard let documentBrowserController = navigation.viewControllers.first as? DocumentBrowserController else {
//            return false
//        }
//        
//        documentBrowserController.openDocumentAtURL(url, copyBeforeOpening: !shouldOpenInPlace.boolValue)

        return true
    }

}

