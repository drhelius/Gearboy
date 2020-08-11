//
//  MasterViewController.h
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 11/08/2020.
//  Copyright © 2020 Ignacio Sánchez Ginés. All rights reserved.
//

#import <UIKit/UIKit.h>

@class DetailViewController;

@interface MasterViewController : UITableViewController

@property (strong, nonatomic) DetailViewController *detailViewController;


@end

