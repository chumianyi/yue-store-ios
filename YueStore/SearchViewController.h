#import <UIKit/UIKit.h>
#import "AppModel.h"

@interface SearchViewController : UIViewController <UISearchBarDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout>
@property (nonatomic, strong) UISearchBar *searchBar;
@property (nonatomic, strong) UICollectionView *collectionView;
@property (nonatomic, strong) NSArray<AppModel *> *allApps;
@property (nonatomic, strong) NSArray<AppModel *> *searchResults;
@property (nonatomic, strong) UILabel *placeholderLabel;
@property (nonatomic, strong) UIActivityIndicatorView *loading;
@end
