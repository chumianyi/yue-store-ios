#import "SceneDelegate.h"
#import "StarFieldView.h"
#import "LiquidGlassTabBarController.h"

@implementation SceneDelegate

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {
    UIWindowScene *windowScene = (UIWindowScene *)scene;
    self.window = [[UIWindow alloc] initWithWindowScene:windowScene];
    self.window.frame = windowScene.coordinateSpace.bounds;

    StarFieldView *starField = [[StarFieldView alloc] initWithFrame:self.window.bounds];
    starField.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.window addSubview:starField];

    LiquidGlassTabBarController *tabBarController = [[LiquidGlassTabBarController alloc] init];
    [self.window addSubview:tabBarController.view];
    tabBarController.view.frame = self.window.bounds;
    tabBarController.view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    tabBarController.view.backgroundColor = [UIColor clearColor];

    [self.window makeKeyAndVisible];
}

@end
