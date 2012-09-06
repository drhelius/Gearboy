//
//  DetailViewController.h
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 06/09/12.
//  Copyright (c) 2012 Ignacio Sánchez Ginés. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface DetailViewController : UIViewController <UISplitViewControllerDelegate>

@property (strong, nonatomic) id detailItem;

@property (weak, nonatomic) IBOutlet UILabel *detailDescriptionLabel;
@end
