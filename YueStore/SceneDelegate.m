#import "SceneDelegate.h"
#import "StarFieldView.h"
#import "LiquidGlassTabBarController.h"

@interface SceneDelegate ()
@property (nonatomic, strong) StarFieldView *starField;
@end

@implementation SceneDelegate

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {
    UIWindowScene *windowScene = (UIWindowScene *)scene;
    self.window = [[UIWindow alloc] initWithWindowScene:windowScene];
    self.window.frame = windowScene.coordinateSpace.bounds;

    LiquidGlassTabBarController *tabBarController = [[LiquidGlassTabBarController alloc] init];
    // Force view load
    [tabBarController view];

    // Add star field as background (at index 0, behind everything)
    self.starField = [[StarFieldView alloc] initWithFrame:tabBarController.view.bounds];
    self.starField.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [tabBarController.view insertSubview:self.starField atIndex:0];

    self.window.rootViewController = tabBarController;
    [self.window makeKeyAndVisible];
}

- (void)sceneDidDisconnect:(UIScene *)scene {}
- (void)sceneDidBecomeActive:(UIScene *)scene {}
- (void)sceneWillResignActive:(UIScene *)scene {}
- (void)sceneWillEnterForeground:(UIScene *)scene {}
- (void)sceneDidEnterBackground:(UIScene *)scene {}

@end
