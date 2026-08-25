#import <UIKit/UIKit.h>
#import "AppModel.h"

@interface AppCollectionViewCell : UICollectionViewCell
@property (nonatomic, strong) UIView *cardView;
@property (nonatomic, strong) UILabel *nameLabel;
@property (nonatomic, strong) UILabel *categoryLabel;
@property (nonatomic, strong) UILabel *sizeLabel;
@property (nonatomic, strong) UIView *iconView;
@property (nonatomic, strong) UILabel *iconLetter;
- (void)configureWithApp:(AppModel *)app;
@end
