/*
See LICENSE folder for this sample’s licensing information.

Abstract:
The main scene delegate.
*/

import UIKit

final class SceneDelegate: UIResponder, UIWindowSceneDelegate {
    var window: UIWindow?
    private var storageReady = false
    private var pendingURLs = [URL]()

    func scene(
        _ scene: UIScene,
        willConnectTo session: UISceneSession,
        options connectionOptions: UIScene.ConnectionOptions
    ) {
        guard let windowScene = scene as? UIWindowScene else { return }

        let window = UIWindow(windowScene: windowScene)
        let loadingViewController = UIViewController()
        loadingViewController.view.backgroundColor = .systemBackground
        window.rootViewController = loadingViewController
        self.window = window
        window.makeKeyAndVisible()

        if let url = connectionOptions.urlContexts.first?.url, url.isFileURL {
            pendingURLs.append(url)
        }

        PathUtils.prepareDataDirectories(createBIOSDirectory: false) { [weak self] in
            guard let self else { return }

            self.storageReady = true
            self.window?.rootViewController = TabBarController()

            for url in self.pendingURLs {
                dataStore.addFromURL(url)
            }
            self.pendingURLs.removeAll()
            dataStore.updateAll()
        }
    }

    func scene(_ scene: UIScene, openURLContexts urlContexts: Set<UIOpenURLContext>) {
        guard let url = urlContexts.first?.url, url.isFileURL else { return }

        if storageReady {
            dataStore.addFromURL(url)
        } else {
            pendingURLs.append(url)
        }
    }
}
