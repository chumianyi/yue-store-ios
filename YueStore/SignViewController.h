#import <UIKit/UIKit.h>
#import <MobileCoreServices/MobileCoreServices.h>

@interface SignViewController : UIViewController <UITableViewDataSource, UITableViewDelegate, UITextFieldDelegate, UIDocumentPickerDelegate>
@property (nonatomic, strong) UIScrollView *scrollView;
@property (nonatomic, strong) UIView *contentView;
@property (nonatomic, strong) UILabel *titleLabel;
@property (nonatomic, strong) UIView *certCard;
@property (nonatomic, strong) UILabel *certLabel;
@property (nonatomic, strong) UIButton *importCertButton;
@property (nonatomic, strong) UITextField *passwordField;
@property (nonatomic, strong) UIView *ipaCard;
@property (nonatomic, strong) UILabel *ipaLabel;
@property (nonatomic, strong) UITableView *ipaTable;
@property (nonatomic, strong) UIButton *importIPAButton;
@property (nonatomic, strong) UIButton *signButton;
@property (nonatomic, strong) UIActivityIndicatorView *signProgress;
@property (nonatomic, strong) UILabel *statusLabel;
@property (nonatomic, strong) NSArray<NSString *> *downloadedIPAs;
@property (nonatomic, strong) NSString *selectedIPA;
@property (nonatomic, strong) NSString *importedCertPath;
@property (nonatomic, strong) NSString *importedCertLabel;
@end
