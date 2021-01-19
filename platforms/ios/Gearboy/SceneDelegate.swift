/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The main scene delegate.
*/

import UIKit

class SceneDelegate: UIResponder, UIWindowSceneDelegate {

    var window: UIWindow?
    var splitViewDelegate = SplitViewDelegate()

    func scene(_ scene: UIScene, willConnectTo session: UISceneSession, options connectionOptions: UIScene.ConnectionOptions) {
        
        configureSplitViewController()
    }
    
    func scene(_ scene: UIScene, openURLContexts URLContexts: Set<UIOpenURLContext>) {
        guard let url = URLContexts.first?.url, url.isFileURL else { return }
        dataStore.addFromURL(url)
    }
}

// MARK: - Private Methods
private extension SceneDelegate {
    func configureSplitViewController() {
        guard
            let window = window,
            let splitViewController = window.rootViewController as? UISplitViewController
        else {
            fatalError()
        }

        splitViewController.delegate = splitViewDelegate
    }
}

