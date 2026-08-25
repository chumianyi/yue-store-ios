#import <UIKit/UIKit.h>
#import <SafariServices/SafariServices.h>
#import "AppModel.h"

@interface AppDetailViewController : UIViewController <SFSafariViewControllerDelegate>
@property (nonatomic, strong) AppModel *app;
@property (nonatomic, strong) UIScrollView *scrollView;
@property (nonatomic, strong) UIView *contentView;
@property (nonatomic, strong) UIView *iconView;
@property (nonatomic, strong) UILabel *iconLetter;
@property (nonatomic, strong) UILabel *nameLabel;
@property (nonatomic, strong) UILabel *versionLabel;
@property (nonatomic, strong) UILabel *categoryLabel;
@property (nonatomic, strong) UILabel *sizeLabel;
@property (nonatomic, strong) UILabel *downloadsLabel;
@property (nonatomic, strong) UILabel *signedLabel;
@property (nonatomic, strong) UITextView *descriptionView;
@property (nonatomic, strong) UIButton *downloadButton;
@property (nonatomic, strong) UIButton *closeButton;
@property (nonatomic, strong) UIActivityIndicatorView *downloadProgress;
@end
