#import "LiquidGlassTabBarController.h"
#import "HomeViewController.h"
#import "CategoryViewController.h"
#import "SearchViewController.h"
#import "SignViewController.h"

@interface LiquidGlassTabBarController ()
@property (nonatomic, strong) UIView *contentContainer;
@property (nonatomic, strong) UIVisualEffectView *tabBarBackground;
@property (nonatomic, strong) UIView *tabBar;
@property (nonatomic, strong) NSMutableArray *tabButtons;
@property (nonatomic, strong) UIView *selectionIndicator;
@property (nonatomic, strong) NSArray *tabIcons;
@property (nonatomic, strong) NSArray *tabTitles;
@end

@implementation LiquidGlassTabBarController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor clearColor];
    self.tabIcons = @[@"house.fill", @"square.grid.2x2.fill", @"magnifyingglass", @"signature"];
    self.tabTitles = @[@"主页", @"分类", @"搜索", @"签名"];
    [self setupContentContainer];
    [self setupTabBar];
    [self setupViewControllers];
    self.selectedIndex = 0;
}

- (void)setupContentContainer {
    CGFloat tabBarHeight = 83.0;
    self.contentContainer = [[UIView alloc] initWithFrame:CGRectMake(0, 0, self.view.bounds.size.width, self.view.bounds.size.height - tabBarHeight)];
    self.contentContainer.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.contentContainer.backgroundColor = [UIColor clearColor];
    [self.view addSubview:self.contentContainer];
}

- (void)setupTabBar {
    CGFloat tabBarHeight = 83.0;
    CGFloat bottomInset = [self safeAreaBottom];
    CGFloat totalHeight = tabBarHeight + bottomInset;

    self.tabBar = [[UIView alloc] initWithFrame:CGRectMake(0, self.view.bounds.size.height - totalHeight, self.view.bounds.size.width, totalHeight)];
    self.tabBar.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleTopMargin;
    self.tabBar.backgroundColor = [UIColor clearColor];

    UIBlurEffect *blurEffect;
    if (@available(iOS 26.0, *)) {
        blurEffect = [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemUltraThinMaterial];
    } else {
        blurEffect = [UIBlurEffect effectWithStyle:UIBlurEffectStyleDark];
    }
    self.tabBarBackground = [[UIVisualEffectView alloc] initWithEffect:blurEffect];
    self.tabBarBackground.frame = self.tabBar.bounds;
    self.tabBarBackground.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.tabBar addSubview:self.tabBarBackground];

    UIView *border = [[UIView alloc] initWithFrame:CGRectMake(0, 0, self.tabBar.bounds.size.width, 0.5)];
    border.backgroundColor = [UIColor colorWithWhite:1.0 alpha:0.15];
    border.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    [self.tabBar addSubview:border];

    self.tabButtons = [NSMutableArray array];
    CGFloat btnWidth = self.view.bounds.size.width / 4.0;
    for (NSInteger i = 0; i < 4; i++) {
        UIButton *btn = [UIButton buttonWithType:UIButtonTypeCustom];
        btn.frame = CGRectMake(i * btnWidth, 8, btnWidth, tabBarHeight - 8);
        btn.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
        btn.tag = i;

        UIImage *icon = [UIImage systemImageNamed:self.tabIcons[i]];
        if (!icon) icon = [UIImage systemImageNamed:@"circle"];
        [btn setImage:icon forState:UIControlStateNormal];
        [btn setTitle:self.tabTitles[i] forState:UIControlStateNormal];
        btn.titleLabel.font = [UIFont systemFontOfSize:10 weight:UIFontWeightMedium];
        btn.tintColor = [UIColor colorWithWhite:1.0 alpha:0.6];
        [btn setTitleColor:[UIColor colorWithWhite:1.0 alpha:0.6] forState:UIControlStateNormal];
        [btn setTitleColor:[UIColor whiteColor] forState:UIControlStateSelected];

        CGSize imgSize = CGSizeMake(24, 24);
        CGSize titleSize = [btn.titleLabel sizeThatFits:CGSizeMake(btnWidth, 20)];
        CGFloat totalHeight_img = imgSize.height + 4 + titleSize.height;
        CGFloat startY = (btn.bounds.size.height - totalHeight_img) / 2.0;
        btn.imageEdgeInsets = UIEdgeInsetsMake(startY - btn.bounds.size.height/2 + imgSize.height/2, 0, btn.bounds.size.height/2 - startY - imgSize.height/2, 0);
        btn.titleEdgeInsets = UIEdgeInsetsMake(startY + imgSize.height + 4 - btn.bounds.size.height/2 + titleSize.height/2, -imgSize.width, btn.bounds.size.height/2 - (startY + imgSize.height + 4) - titleSize.height/2, 0);

        [btn addTarget:self action:@selector(tabButtonTapped:) forControlEvents:UIControlEventTouchUpInside];
        [self.tabButtons addObject:btn];
        [self.tabBar addSubview:btn];
    }

    self.selectionIndicator = [[UIView alloc] initWithFrame:CGRectMake(btnWidth * 0.3, 4, btnWidth * 0.4, 3)];
    self.selectionIndicator.backgroundColor = [UIColor colorWithRed:0.3 green:0.6 blue:1.0 alpha:0.9];
    self.selectionIndicator.layer.cornerRadius = 1.5;
    [self.tabBar addSubview:self.selectionIndicator];

    [self.view addSubview:self.tabBar];
}

- (void)setupViewControllers {
    HomeViewController *home = [[HomeViewController alloc] init];
    CategoryViewController *category = [[CategoryViewController alloc] init];
    SearchViewController *search = [[SearchViewController alloc] init];
    SignViewController *sign = [[SignViewController alloc] init];
    self.viewControllers = @[home, category, search, sign];
    for (UIViewController *vc in self.viewControllers) {
        [self addChildViewController:vc];
        vc.view.frame = self.contentContainer.bounds;
        vc.view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        [self.contentContainer addSubview:vc.view];
        [vc didMoveToParentViewController:self];
        vc.view.hidden = YES;
    }
}

- (void)setSelectedIndex:(NSInteger)selectedIndex {
    if (selectedIndex < 0 || selectedIndex >= self.viewControllers.count) return;
    _selectedIndex = selectedIndex;
    self.selectedViewController = self.viewControllers[selectedIndex];
    for (NSInteger i = 0; i < self.viewControllers.count; i++) {
        UIViewController *vc = self.viewControllers[i];
        vc.view.hidden = (i != selectedIndex);
        UIButton *btn = self.tabButtons[i];
        btn.selected = (i == selectedIndex);
        btn.tintColor = (i == selectedIndex) ? [UIColor whiteColor] : [UIColor colorWithWhite:1.0 alpha:0.6];
    }
    CGFloat btnWidth = self.view.bounds.size.width / 4.0;
    [UIView animateWithDuration:0.3 delay:0 usingSpringWithDamping:0.7 initialSpringVelocity:0.5 options:UIViewAnimationOptionCurveEaseInOut animations:^{
        self.selectionIndicator.frame = CGRectMake(selectedIndex * btnWidth + btnWidth * 0.3, 4, btnWidth * 0.4, 3);
    } completion:nil];
}

- (void)tabButtonTapped:(UIButton *)sender {
    self.selectedIndex = sender.tag;
}

- (CGFloat)safeAreaBottom {
    if (@available(iOS 11.0, *)) {
        return self.view.safeAreaInsets.bottom;
    }
    return 0;
}

- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    CGFloat tabBarHeight = 83.0;
    CGFloat bottomInset = [self safeAreaBottom];
    CGFloat totalHeight = tabBarHeight + bottomInset;
    self.contentContainer.frame = CGRectMake(0, 0, self.view.bounds.size.width, self.view.bounds.size.height - totalHeight);
    self.tabBar.frame = CGRectMake(0, self.view.bounds.size.height - totalHeight, self.view.bounds.size.width, totalHeight);
}

@end
