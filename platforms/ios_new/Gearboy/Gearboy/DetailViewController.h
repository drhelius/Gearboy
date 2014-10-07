//
//  DetailViewController.h
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 04/10/14.
//  Copyright (c) 2014 Ignacio Sánchez Ginés. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface DetailViewController : UIViewController

@property (strong, nonatomic) id detailItem;
@property (weak, nonatomic) IBOutlet UILabel *detailDescriptionLabel;

@end

