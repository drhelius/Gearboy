/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#import "AppDelegate.h"
#import "DetailViewController.h"
#import "MasterViewController.h"

@interface AppDelegate () <UISplitViewControllerDelegate>

@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    UISplitViewController *splitViewController = (UISplitViewController *)self.window.rootViewController;
    UINavigationController *navigationController = [splitViewController.viewControllers lastObject];
    navigationController.topViewController.navigationItem.leftBarButtonItem = splitViewController.displayModeButtonItem;
    splitViewController.delegate = self;
    splitViewController.presentsWithGesture = NO;
    splitViewController.preferredDisplayMode = UISplitViewControllerDisplayModePrimaryOverlay;
    masterViewController = (MasterViewController *)[[splitViewController.viewControllers firstObject] topViewController];
    
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
    masterViewController.theGLViewController.displayLink.paused = YES;
    masterViewController.theGLViewController.paused = YES;
    [masterViewController.theGLViewController releaseContext];
    [masterViewController.theGLViewController.theEmulator save];
    [NSThread sleepForTimeInterval:1.5];
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    [masterViewController.theGLViewController acquireContext];
    masterViewController.theGLViewController.paused = NO;
    masterViewController.theGLViewController.displayLink.paused = NO;
}

- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
    return [self openGearboyURL:url];
}

- (BOOL)application:(UIApplication *)app openURL:(NSURL *)url options:(NSDictionary<UIApplicationOpenURLOptionsKey,id> *)options {
    return [self openGearboyURL:url];
}

#pragma mark - Private Methods

- (BOOL)openGearboyURL:(NSURL *)url {
    if (url != nil && [url isFileURL])
    {
        NSString *documentsDirectory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
        NSError* error;
        NSFileManager *fileManager = [[NSFileManager alloc] init];

        NSString* srcPath = [url path];
        NSString* destPath = [NSString stringWithFormat:@"%@/%@", documentsDirectory, [srcPath lastPathComponent]];

        if ([fileManager copyItemAtPath:srcPath toPath:destPath error:&error])
        {
            if ([fileManager removeItemAtPath:[url path] error:&error])
            {
                [masterViewController reloadTableView];
                NSArray *extensions = [NSArray arrayWithObjects:@"zip", @"gb", @"sgb", @"gbc", @"rom", @"dmg", @"cgb", @"ZIP", @"GB", @"SGB", @"GBC", @"ROM", @"DMG", @"CGB", nil];
                if ([extensions containsObject:[srcPath pathExtension]]) {
                    [masterViewController loadWithROM:[[url path] lastPathComponent]];
                }
                return YES;
            }
        }

        NSLog(@"ERROR %@", [error localizedDescription]);
    }

    return NO;
}

#pragma mark - Split view

- (BOOL)splitViewController:(UISplitViewController *)splitViewController collapseSecondaryViewController:(UIViewController *)secondaryViewController ontoPrimaryViewController:(UIViewController *)primaryViewController {
    if ([secondaryViewController isKindOfClass:[UINavigationController class]] && [[(UINavigationController *)secondaryViewController topViewController] isKindOfClass:[DetailViewController class]] && ([(DetailViewController *)[(UINavigationController *)secondaryViewController topViewController] detailItem] == nil)) {
        // Return YES to indicate that we have handled the collapse by doing nothing; the secondary controller will be discarded.
        return YES;
    } else {
        return NO;
    }
}

@end
