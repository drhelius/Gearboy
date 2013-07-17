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

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    // Override point for customization after application launch.
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        masterViewController = [[MasterViewController alloc] initWithNibName:@"MasterViewController_iPhone" bundle:nil];
        UINavigationController *navigationController = [[UINavigationController alloc] initWithRootViewController:masterViewController];
        navigationController.navigationBar.tintColor = [UIColor blackColor];
        self.window.rootViewController = navigationController;
    } else {
        masterViewController = [[MasterViewController alloc] initWithNibName:@"MasterViewController_iPad" bundle:nil];
        UINavigationController *masterNavigationController = [[UINavigationController alloc] initWithRootViewController:masterViewController];
        
        masterNavigationController.navigationBar.tintColor = [UIColor blackColor];
        
        DetailViewController *detailViewController = [[DetailViewController alloc] initWithNibName:@"DetailViewController_iPad" bundle:nil];
        UINavigationController *detailNavigationController = [[UINavigationController alloc] initWithRootViewController:detailViewController];
        
        detailNavigationController.navigationBar.tintColor = [UIColor blackColor];
    	
    	masterViewController.detailViewController = detailViewController;
    	
        UISplitViewController *splitViewController = [[UISplitViewController alloc] init];
        splitViewController.presentsWithGesture = NO;
        splitViewController.delegate = detailViewController;
        splitViewController.viewControllers = @[masterNavigationController, detailNavigationController];
        
        self.window.rootViewController = splitViewController;
    }
    [self.window makeKeyAndVisible];
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
    masterViewController.detailViewController.theGLViewController.paused = YES;
    [masterViewController.detailViewController.theGLViewController releaseContext];
    [masterViewController.detailViewController.theGLViewController.theEmulator save];
    [NSThread sleepForTimeInterval:1.5];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    [masterViewController.detailViewController.theGLViewController acquireContext];
    masterViewController.detailViewController.theGLViewController.paused = NO;
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
    if (url != nil && [url isFileURL])
    {
        NSString *documentsDirectory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
        NSError* error;
        NSFileManager *fileManager = [[NSFileManager alloc] init];
        
        NSString* srcPath = [url path];
        NSString* destPath = [NSString stringWithFormat:@"%@/%@", documentsDirectory, [srcPath lastPathComponent]];
        
        if ([fileManager copyItemAtPath:srcPath toPath:destPath error:&error])
        {
            [masterViewController reloadTableView];
            [masterViewController loadWithROM:[[url path] lastPathComponent]];
                
            return YES;
        }
        
        NSLog(@"ERROR %@", [error localizedDescription]);
    }
    
    return NO;
}

@end
