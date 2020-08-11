//
//  DetailViewController.h
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 11/08/2020.
//  Copyright © 2020 Ignacio Sánchez Ginés. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GLViewController.h"

@interface DetailViewController : UIViewController

@property (strong, nonatomic) NSDate *detailItem;
@property (strong, nonatomic) GLViewController* theGLViewController;

@end

