//
//  main.m
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 11/08/2020.
//  Copyright © 2020 Ignacio Sánchez Ginés. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
