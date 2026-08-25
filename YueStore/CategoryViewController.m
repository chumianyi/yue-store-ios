#import "CategoryViewController.h"
#import "APIManager.h"
#import "AppCollectionViewCell.h"
#import "AppDetailViewController.h"

@implementation CategoryViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor clearColor];
    self.title = @"分类";
    [self setupUI];
    [self loadData];
}

- (void)setupUI {
    CGFloat tableWidth = 100;
    self.categoryTable = [[UITableView alloc] initWithFrame:CGRectMake(0, 0, tableWidth, self.view.bounds.size.height) style:UITableViewStylePlain];
    self.categoryTable.autoresizingMask = UIViewAutoresizingFlexibleHeight;
    self.categoryTable.backgroundColor = [UIColor colorWithWhite:0 alpha:0.3];
    self.categoryTable.separatorStyle = UITableViewCellSeparatorStyleNone;
    self.categoryTable.dataSource = self;
    self.categoryTable.delegate = self;
    self.categoryTable.showsVerticalScrollIndicator = NO;
    [self.categoryTable registerClass:[UITableViewCell class] forCellReuseIdentifier:@"CatCell"];
    [self.view addSubview:self.categoryTable];

    UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
    CGFloat spacing = 10;
    CGFloat availWidth = self.view.bounds.size.width - tableWidth - spacing * 3;
    layout.itemSize = CGSizeMake(availWidth / 2.0, 100);
    layout.minimumInteritemSpacing = spacing;
    layout.minimumLineSpacing = spacing;
    layout.sectionInset = UIEdgeInsetsMake(12, spacing, 100, spacing);

    self.appsCollection = [[UICollectionView alloc] initWithFrame:CGRectMake(tableWidth, 0, self.view.bounds.size.width - tableWidth, self.view.bounds.size.height) collectionViewLayout:layout];
    self.appsCollection.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.appsCollection.backgroundColor = [UIColor clearColor];
    self.appsCollection.dataSource = self;
    self.appsCollection.delegate = self;
    self.appsCollection.showsVerticalScrollIndicator = NO;
    [self.appsCollection registerClass:[AppCollectionViewCell class] forCellWithReuseIdentifier:@"AppCell"];
    [self.view addSubview:self.appsCollection];

    self.loading = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleLarge];
    self.loading.center = CGPointMake(self.view.bounds.size.width / 2, self.view.bounds.size.height / 2);
    self.loading.color = [UIColor whiteColor];
    self.loading.hidesWhenStopped = YES;
    [self.view addSubview:self.loading];
}

- (void)loadData {
    [self.loading startAnimating];
    [[APIManager sharedManager] fetchAppsWithCompletion:^(NSArray<AppModel *> *apps, NSError *error) {
        [self.loading stopAnimating];
        if (error || apps.count == 0) return;
        self.allApps = apps;
        NSMutableOrderedSet *catSet = [NSMutableOrderedSet orderedSet];
        [catSet addObject:@"全部"];
        for (AppModel *app in apps) {
            if (app.subCategory.length > 0) [catSet addObject:app.subCategory];
            else if (app.category.length > 0) [catSet addObject:app.category];
        }
        self.categories = catSet.array;
        self.selectedCategoryIndex = 0;
        [self filterApps];
        [self.categoryTable reloadData];
        [self.appsCollection reloadData];
    }];
}

- (void)filterApps {
    if (self.selectedCategoryIndex == 0) {
        self.filteredApps = self.allApps;
    } else {
        NSString *cat = self.categories[self.selectedCategoryIndex];
        NSMutableArray *filtered = [NSMutableArray array];
        for (AppModel *app in self.allApps) {
            if ([app.subCategory isEqualToString:cat] || [app.category isEqualToString:cat]) {
                [filtered addObject:app];
            }
        }
        self.filteredApps = filtered;
    }
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.categories.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"CatCell" forIndexPath:indexPath];
    cell.backgroundColor = [UIColor clearColor];
    cell.textLabel.text = self.categories[indexPath.row];
    cell.textLabel.textAlignment = NSTextAlignmentCenter;
    cell.textLabel.font = [UIFont systemFontOfSize:13];
    if (indexPath.row == self.selectedCategoryIndex) {
        cell.contentView.backgroundColor = [UIColor colorWithRed:0.3 green:0.5 blue:0.9 alpha:0.4];
        cell.textLabel.textColor = [UIColor whiteColor];
        cell.textLabel.font = [UIFont boldSystemFontOfSize:13];
    } else {
        cell.contentView.backgroundColor = [UIColor clearColor];
        cell.textLabel.textColor = [UIColor colorWithWhite:1.0 alpha:0.6];
    }
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    self.selectedCategoryIndex = indexPath.row;
    [self filterApps];
    [self.categoryTable reloadData];
    [self.appsCollection reloadData];
}

- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section {
    return self.filteredApps.count;
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath {
    AppCollectionViewCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"AppCell" forIndexPath:indexPath];
    [cell configureWithApp:self.filteredApps[indexPath.item]];
    return cell;
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath {
    AppDetailViewController *detail = [[AppDetailViewController alloc] init];
    detail.app = self.filteredApps[indexPath.item];
    detail.modalPresentationStyle = UIModalPresentationFullScreen;
    [self presentViewController:detail animated:YES completion:nil];
}

- (CGSize)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
    CGFloat spacing = 10;
    CGFloat tableWidth = 100;
    CGFloat availWidth = collectionView.bounds.size.width - spacing * 3;
    return CGSizeMake(availWidth / 2.0, 100);
}

@end
