#import <UIKit/UIKit.h>

@interface LiquidGlassTabBarController : UIViewController
@property (nonatomic, strong) NSArray *viewControllers;
@property (nonatomic, assign) NSInteger selectedIndex;
@property (nonatomic, strong) UIViewController *selectedViewController;
@end
