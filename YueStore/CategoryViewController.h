#import <UIKit/UIKit.h>
#import "AppModel.h"

@interface CategoryViewController : UIViewController <UITableViewDataSource, UITableViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout>
@property (nonatomic, strong) UITableView *categoryTable;
@property (nonatomic, strong) UICollectionView *appsCollection;
@property (nonatomic, strong) NSArray<AppModel *> *allApps;
@property (nonatomic, strong) NSArray<NSString *> *categories;
@property (nonatomic, strong) NSArray<AppModel *> *filteredApps;
@property (nonatomic, assign) NSInteger selectedCategoryIndex;
@property (nonatomic, strong) UIActivityIndicatorView *loading;
@end
