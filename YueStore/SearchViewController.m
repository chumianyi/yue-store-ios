#import "SearchViewController.h"
#import "APIManager.h"
#import "AppCollectionViewCell.h"
#import "AppDetailViewController.h"

@implementation SearchViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor clearColor];
    self.title = @"搜索";
    [self setupUI];
    [self loadApps];
}

- (void)setupUI {
    self.searchBar = [[UISearchBar alloc] initWithFrame:CGRectMake(0, 0, self.view.bounds.size.width, 56)];
    self.searchBar.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.searchBar.delegate = self;
    self.searchBar.placeholder = @"搜索应用名称、分类...";
    self.searchBar.searchBarStyle = UISearchBarStyleMinimal;
    self.searchBar.barTintColor = [UIColor clearColor];
    self.searchBar.tintColor = [UIColor whiteColor];
    UITextField *tf = [self.searchBar valueForKey:@"searchField"];
    if (tf) {
        tf.textColor = [UIColor whiteColor];
        tf.keyboardAppearance = UIKeyboardAppearanceDark;
    }
    [self.view addSubview:self.searchBar];

    UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
    CGFloat spacing = 12;
    CGFloat width = (self.view.bounds.size.width - spacing * 3) / 2.0;
    layout.itemSize = CGSizeMake(width, 110);
    layout.minimumInteritemSpacing = spacing;
    layout.minimumLineSpacing = spacing;
    layout.sectionInset = UIEdgeInsetsMake(8, spacing, 100, spacing);

    self.collectionView = [[UICollectionView alloc] initWithFrame:CGRectMake(0, 56, self.view.bounds.size.width, self.view.bounds.size.height - 56) collectionViewLayout:layout];
    self.collectionView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.collectionView.backgroundColor = [UIColor clearColor];
    self.collectionView.dataSource = self;
    self.collectionView.delegate = self;
    self.collectionView.showsVerticalScrollIndicator = NO;
    [self.collectionView registerClass:[AppCollectionViewCell class] forCellWithReuseIdentifier:@"AppCell"];
    [self.view addSubview:self.collectionView];

    self.placeholderLabel = [[UILabel alloc] initWithFrame:CGRectMake(20, self.view.bounds.size.height / 2 - 40, self.view.bounds.size.width - 40, 80)];
    self.placeholderLabel.textAlignment = NSTextAlignmentCenter;
    self.placeholderLabel.textColor = [UIColor colorWithWhite:1.0 alpha:0.5];
    self.placeholderLabel.font = [UIFont systemFontOfSize:15];
    self.placeholderLabel.numberOfLines = 0;
    self.placeholderLabel.text = @"输入关键词搜索应用\n支持名称、分类、描述搜索";
    [self.view addSubview:self.placeholderLabel];

    self.loading = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleLarge];
    self.loading.center = CGPointMake(self.view.bounds.size.width / 2, self.view.bounds.size.height / 2);
    self.loading.color = [UIColor whiteColor];
    self.loading.hidesWhenStopped = YES;
    [self.view addSubview:self.loading];
}

- (void)loadApps {
    [self.loading startAnimating];
    [[APIManager sharedManager] fetchAppsWithCompletion:^(NSArray<AppModel *> *apps, NSError *error) {
        [self.loading stopAnimating];
        if (error || apps.count == 0) {
            self.placeholderLabel.text = @"数据加载失败";
            return;
        }
        self.allApps = apps;
        self.searchResults = @[];
    }];
}

- (void)searchBar:(UISearchBar *)searchBar textDidChange:(NSString *)searchText {
    [self performSearch:searchText];
}

- (void)searchBarSearchButtonClicked:(UISearchBar *)searchBar {
    [searchBar resignFirstResponder];
    [self performSearch:searchBar.text];
}

- (void)performSearch:(NSString *)keyword {
    if (keyword.length == 0 || !self.allApps) {
        self.searchResults = @[];
        self.placeholderLabel.hidden = NO;
        self.placeholderLabel.text = @"输入关键词搜索应用\n支持名称、分类、描述搜索";
        [self.collectionView reloadData];
        return;
    }
    NSString *lower = [keyword lowercaseString];
    NSMutableArray *results = [NSMutableArray array];
    for (AppModel *app in self.allApps) {
        BOOL match = NO;
        if ([[app.name lowercaseString] containsString:lower]) match = YES;
        else if ([[app.subCategory lowercaseString] containsString:lower]) match = YES;
        else if ([[app.category lowercaseString] containsString:lower]) match = YES;
        else if ([[app.desc lowercaseString] containsString:lower]) match = YES;
        else if ([[app.tag lowercaseString] containsString:lower]) match = YES;
        if (match) [results addObject:app];
    }
    self.searchResults = results;
    self.placeholderLabel.hidden = (results.count > 0);
    if (results.count == 0) {
        self.placeholderLabel.text = [NSString stringWithFormat:@"未找到与\"%@\"相关的应用", keyword];
    }
    [self.collectionView reloadData];
}

- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section {
    return self.searchResults.count;
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath {
    AppCollectionViewCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"AppCell" forIndexPath:indexPath];
    [cell configureWithApp:self.searchResults[indexPath.item]];
    return cell;
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath {
    AppDetailViewController *detail = [[AppDetailViewController alloc] init];
    detail.app = self.searchResults[indexPath.item];
    detail.modalPresentationStyle = UIModalPresentationFullScreen;
    [self presentViewController:detail animated:YES completion:nil];
}

- (CGSize)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
    CGFloat spacing = 12;
    CGFloat width = (collectionView.bounds.size.width - spacing * 3) / 2.0;
    return CGSizeMake(width, 110);
}

- (void)scrollViewWillBeginDragging:(UIScrollView *)scrollView {
    [self.searchBar resignFirstResponder];
}

@end
