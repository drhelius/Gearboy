//
//  DetailViewController.m
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 11/08/2020.
//  Copyright © 2020 Ignacio Sánchez Ginés. All rights reserved.
//

#import "DetailViewController.h"

@interface DetailViewController ()

@end

@implementation DetailViewController

- (void)configureView {
    
    UIBarButtonItem *menu = [[UIBarButtonItem alloc] initWithTitle:@"Menu" style:UIBarButtonItemStylePlain target:self action:@selector(menuButtonPressed)];
    
    self.navigationItem.rightBarButtonItems = @[menu];
    
    [self.view addSubview:self.theGLViewController.view];
    
    if (self.detailItem) {
        [self.theGLViewController loadRomWithName:@"aa"];
    }
}


- (void)viewDidLoad {
    [super viewDidLoad];
    [self configureView];
}


#pragma mark - Managing the detail item

- (void)setDetailItem:(NSDate *)newDetailItem {
    if (_detailItem != newDetailItem) {
        _detailItem = newDetailItem;
        
        // Update the view.
        [self configureView];
    }
}


@end
