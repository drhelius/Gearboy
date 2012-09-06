//
//  MasterViewController.h
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 06/09/12.
//  Copyright (c) 2012 Ignacio Sánchez Ginés. All rights reserved.
//

#import <UIKit/UIKit.h>

@class DetailViewController;

@interface MasterViewController : UITableViewController

@property (strong, nonatomic) DetailViewController *detailViewController;

@end
