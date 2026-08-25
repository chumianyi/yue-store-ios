#import <UIKit/UIKit.h>
#import "AppModel.h"

@interface HomeViewController : UIViewController <UICollectionViewDataSource, UICollectionViewDelegateFlowLayout>
@property (nonatomic, strong) UICollectionView *collectionView;
@property (nonatomic, strong) NSArray<AppModel *> *apps;
@property (nonatomic, strong) UIActivityIndicatorView *loadingIndicator;
@property (nonatomic, strong) UILabel *emptyLabel;
@end
