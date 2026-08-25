#import "HomeViewController.h"
#import "APIManager.h"
#import "AppDetailViewController.h"
#import "AppCollectionViewCell.h"

@interface HomeViewController ()
@end

@implementation HomeViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor clearColor];
    self.title = @"yue.store";
    [self setupCollectionView];
    [self setupLoading];
    [self loadApps];
}

- (void)setupCollectionView {
    UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
    CGFloat spacing = 12;
    CGFloat width = (self.view.bounds.size.width - spacing * 3) / 2.0;
    layout.itemSize = CGSizeMake(width, 110);
    layout.minimumInteritemSpacing = spacing;
    layout.minimumLineSpacing = spacing;
    layout.sectionInset = UIEdgeInsetsMake(16, spacing, 100, spacing);
    layout.scrollDirection = UICollectionViewScrollDirectionVertical;

    self.collectionView = [[UICollectionView alloc] initWithFrame:self.view.bounds collectionViewLayout:layout];
    self.collectionView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.collectionView.backgroundColor = [UIColor clearColor];
    self.collectionView.dataSource = self;
    self.collectionView.delegate = self;
    self.collectionView.showsVerticalScrollIndicator = NO;
    [self.collectionView registerClass:[AppCollectionViewCell class] forCellWithReuseIdentifier:@"AppCell"];
    [self.view addSubview:self.collectionView];
}

- (void)setupLoading {
    self.loadingIndicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleLarge];
    self.loadingIndicator.center = CGPointMake(self.view.bounds.size.width / 2, self.view.bounds.size.height / 2 - 50);
    self.loadingIndicator.color = [UIColor whiteColor];
    self.loadingIndicator.hidesWhenStopped = YES;
    [self.view addSubview:self.loadingIndicator];

    self.emptyLabel = [[UILabel alloc] initWithFrame:CGRectMake(20, self.view.bounds.size.height / 2 - 20, self.view.bounds.size.width - 40, 40)];
    self.emptyLabel.textAlignment = NSTextAlignmentCenter;
    self.emptyLabel.textColor = [UIColor colorWithWhite:1.0 alpha:0.6];
    self.emptyLabel.font = [UIFont systemFontOfSize:15];
    self.emptyLabel.hidden = YES;
    [self.view addSubview:self.emptyLabel];
}

- (void)loadApps {
    [self.loadingIndicator startAnimating];
    self.emptyLabel.hidden = YES;
    [[APIManager sharedManager] fetchAppsWithCompletion:^(NSArray<AppModel *> *apps, NSError *error) {
        [self.loadingIndicator stopAnimating];
        if (error || apps.count == 0) {
            self.emptyLabel.text = @"加载失败，请检查网络";
            self.emptyLabel.hidden = NO;
            return;
        }
        self.apps = apps;
        [self.collectionView reloadData];
    }];
}

- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section {
    return self.apps.count;
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath {
    AppCollectionViewCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"AppCell" forIndexPath:indexPath];
    [cell configureWithApp:self.apps[indexPath.item]];
    return cell;
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath {
    AppDetailViewController *detail = [[AppDetailViewController alloc] init];
    detail.app = self.apps[indexPath.item];
    detail.modalPresentationStyle = UIModalPresentationFullScreen;
    [self presentViewController:detail animated:YES completion:nil];
}

- (CGSize)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
    CGFloat spacing = 12;
    CGFloat width = (collectionView.bounds.size.width - spacing * 3) / 2.0;
    return CGSizeMake(width, 110);
}

@end
