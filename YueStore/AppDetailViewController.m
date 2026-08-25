#import "AppDetailViewController.h"
#import "IPADownloadManager.h"

@implementation AppDetailViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor colorWithRed:0.02 green:0.02 blue:0.08 alpha:0.98];
    [self setupUI];
    [self populateData];
}

- (void)setupUI {
    self.scrollView = [[UIScrollView alloc] initWithFrame:self.view.bounds];
    self.scrollView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.scrollView.showsVerticalScrollIndicator = NO;
    [self.view addSubview:self.scrollView];

    self.contentView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, self.view.bounds.size.width, 800)];
    self.contentView.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    [self.scrollView addSubview:self.contentView];

    self.closeButton = [UIButton buttonWithType:UIButtonTypeSystem];
    self.closeButton.frame = CGRectMake(self.view.bounds.size.width - 50, 50, 36, 36);
    self.closeButton.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin;
    [self.closeButton setImage:[UIImage systemImageNamed:@"xmark.circle.fill"] forState:UIControlStateNormal];
    self.closeButton.tintColor = [UIColor whiteColor];
    self.closeButton.backgroundColor = [UIColor colorWithWhite:1.0 alpha:0.1];
    self.closeButton.layer.cornerRadius = 18;
    [self.closeButton addTarget:self action:@selector(closeTapped) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:self.closeButton];

    self.iconView = [[UIView alloc] initWithFrame:CGRectMake(20, 80, 90, 90)];
    self.iconView.layer.cornerRadius = 20;
    self.iconView.clipsToBounds = YES;
    [self.contentView addSubview:self.iconView];

    self.iconLetter = [[UILabel alloc] initWithFrame:self.iconView.bounds];
    self.iconLetter.textAlignment = NSTextAlignmentCenter;
    self.iconLetter.textColor = [UIColor whiteColor];
    self.iconLetter.font = [UIFont boldSystemFontOfSize:40];
    [self.iconView addSubview:self.iconLetter];

    self.nameLabel = [[UILabel alloc] initWithFrame:CGRectMake(120, 85, self.view.bounds.size.width - 140, 28)];
    self.nameLabel.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.nameLabel.textColor = [UIColor whiteColor];
    self.nameLabel.font = [UIFont boldSystemFontOfSize:20];
    self.nameLabel.numberOfLines = 2;
    [self.contentView addSubview:self.nameLabel];

    self.versionLabel = [[UILabel alloc] initWithFrame:CGRectMake(120, 118, self.view.bounds.size.width - 140, 18)];
    self.versionLabel.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.versionLabel.textColor = [UIColor colorWithWhite:1.0 alpha:0.6];
    self.versionLabel.font = [UIFont systemFontOfSize:13];
    [self.contentView addSubview:self.versionLabel];

    self.categoryLabel = [[UILabel alloc] initWithFrame:CGRectMake(120, 140, self.view.bounds.size.width - 140, 18)];
    self.categoryLabel.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.categoryLabel.textColor = [UIColor colorWithRed:0.4 green:0.7 blue:1.0 alpha:1.0];
    self.categoryLabel.font = [UIFont systemFontOfSize:12 weight:UIFontWeightMedium];
    [self.contentView addSubview:self.categoryLabel];

    UIView *infoBar = [[UIView alloc] initWithFrame:CGRectMake(20, 190, self.view.bounds.size.width - 40, 50)];
    infoBar.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    infoBar.backgroundColor = [UIColor colorWithWhite:1.0 alpha:0.06];
    infoBar.layer.cornerRadius = 12;
    [self.contentView addSubview:infoBar];

    self.sizeLabel = [[UILabel alloc] initWithFrame:CGRectMake(10, 0, (infoBar.bounds.size.width - 20) / 3, 50)];
    self.sizeLabel.textAlignment = NSTextAlignmentCenter;
    self.sizeLabel.textColor = [UIColor whiteColor];
    self.sizeLabel.font = [UIFont systemFontOfSize:13 weight:UIFontWeightMedium];
    [infoBar addSubview:self.sizeLabel];

    self.downloadsLabel = [[UILabel alloc] initWithFrame:CGRectMake(10 + (infoBar.bounds.size.width - 20) / 3, 0, (infoBar.bounds.size.width - 20) / 3, 50)];
    self.downloadsLabel.textAlignment = NSTextAlignmentCenter;
    self.downloadsLabel.textColor = [UIColor whiteColor];
    self.downloadsLabel.font = [UIFont systemFontOfSize:13 weight:UIFontWeightMedium];
    [infoBar addSubview:self.downloadsLabel];

    self.signedLabel = [[UILabel alloc] initWithFrame:CGRectMake(10 + (infoBar.bounds.size.width - 20) * 2 / 3, 0, (infoBar.bounds.size.width - 20) / 3, 50)];
    self.signedLabel.textAlignment = NSTextAlignmentCenter;
    self.signedLabel.textColor = [UIColor whiteColor];
    self.signedLabel.font = [UIFont systemFontOfSize:13 weight:UIFontWeightMedium];
    [infoBar addSubview:self.signedLabel];

    UILabel *descTitle = [[UILabel alloc] initWithFrame:CGRectMake(20, 260, self.view.bounds.size.width - 40, 22)];
    descTitle.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    descTitle.text = @"应用介绍";
    descTitle.textColor = [UIColor whiteColor];
    descTitle.font = [UIFont boldSystemFontOfSize:16];
    [self.contentView addSubview:descTitle];

    self.descriptionView = [[UITextView alloc] initWithFrame:CGRectMake(20, 290, self.view.bounds.size.width - 40, 200)];
    self.descriptionView.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.descriptionView.backgroundColor = [UIColor clearColor];
    self.descriptionView.textColor = [UIColor colorWithWhite:1.0 alpha:0.8];
    self.descriptionView.font = [UIFont systemFontOfSize:14];
    self.descriptionView.editable = NO;
    self.descriptionView.scrollEnabled = NO;
    [self.contentView addSubview:self.descriptionView];

    self.downloadButton = [UIButton buttonWithType:UIButtonTypeCustom];
    self.downloadButton.frame = CGRectMake(20, self.view.bounds.size.height - 100, self.view.bounds.size.width - 40, 52);
    self.downloadButton.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleTopMargin;
    self.downloadButton.backgroundColor = [UIColor colorWithRed:0.2 green:0.5 blue:0.95 alpha:1.0];
    self.downloadButton.layer.cornerRadius = 16;
    [self.downloadButton setTitle:@"下载应用" forState:UIControlStateNormal];
    [self.downloadButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
    self.downloadButton.titleLabel.font = [UIFont boldSystemFontOfSize:17];
    self.downloadButton.layer.shadowColor = [UIColor colorWithRed:0.2 green:0.5 blue:0.95 alpha:0.5].CGColor;
    self.downloadButton.layer.shadowOffset = CGSizeMake(0, 4);
    self.downloadButton.layer.shadowRadius = 12;
    self.downloadButton.layer.shadowOpacity = 0.6;
    [self.downloadButton addTarget:self action:@selector(downloadTapped) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:self.downloadButton];

    self.downloadProgress = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleMedium];
    self.downloadProgress.center = CGPointMake(self.downloadButton.bounds.size.width / 2, self.downloadButton.bounds.size.height / 2);
    self.downloadProgress.hidesWhenStopped = YES;
    [self.downloadButton addSubview:self.downloadProgress];
}

- (void)populateData {
    if (!self.app) return;
    self.nameLabel.text = self.app.name ?: @"未知应用";
    self.versionLabel.text = [NSString stringWithFormat:@"版本: %@", self.app.version ?: @"--"];
    self.categoryLabel.text = [NSString stringWithFormat:@"%@ · %@", self.app.category ?: @"", self.app.subCategory ?: @""];
    self.sizeLabel.text = [NSString stringWithFormat:@"大小\n%@", self.app.size ?: @"--"];
    self.sizeLabel.numberOfLines = 2;
    self.downloadsLabel.text = [NSString stringWithFormat:@"下载\n%ld", (long)self.app.downloads];
    self.downloadsLabel.numberOfLines = 2;
    self.signedLabel.text = [NSString stringWithFormat:@"状态\n%@", self.app.signed ? @"已签名" : @"未签名"];
    self.signedLabel.numberOfLines = 2;
    self.descriptionView.text = self.app.desc ?: @"暂无描述";

    if (self.app.name.length > 0) {
        self.iconLetter.text = [[self.app.name substringToIndex:1] uppercaseString];
    }
    CGFloat hue = ([self.app.name hash] % 100) / 100.0;
    self.iconView.backgroundColor = [UIColor colorWithHue:hue saturation:0.5 brightness:0.7 alpha:0.8];

    if ([self.app isDirectInstallLink]) {
        [self.downloadButton setTitle:@"直接安装" forState:UIControlStateNormal];
    }

    CGSize descSize = [self.descriptionView sizeThatFits:CGSizeMake(self.descriptionView.bounds.size.width, CGFLOAT_MAX)];
    CGRect descFrame = self.descriptionView.frame;
    descFrame.size.height = descSize.height;
    self.descriptionView.frame = descFrame;
    self.contentView.frame = CGRectMake(0, 0, self.view.bounds.size.width, descFrame.origin.y + descFrame.size.height + 120);
    self.scrollView.contentSize = CGSizeMake(self.view.bounds.size.width, self.contentView.frame.size.height);
}

- (void)downloadTapped {
    if (!self.app.downloadUrl || self.app.downloadUrl.length == 0) return;

    if ([self.app isDirectInstallLink]) {
        NSURL *url = [NSURL URLWithString:self.app.downloadUrl];
        if (url && [[UIApplication sharedApplication] canOpenURL:url]) {
            [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:^(BOOL success) {
                if (!success) {
                    [self showAlert:@"安装失败" message:@"无法打开安装链接"];
                }
            }];
        }
        return;
    }

    if ([self.app.downloadUrl hasPrefix:@"http"]) {
        [self.downloadProgress startAnimating];
        [self.downloadButton setTitle:@"" forState:UIControlStateNormal];

        [[IPADownloadManager sharedManager] downloadIPAFromURL:self.app.downloadUrl filename:self.app.filename completion:^(NSURL *fileURL, NSError *error) {
            [self.downloadProgress stopAnimating];
            [self.downloadButton setTitle:@"下载应用" forState:UIControlStateNormal];
            if (error) {
                SFSafariViewController *safari = [[SFSafariViewController alloc] initWithURL:[NSURL URLWithString:self.app.downloadUrl]];
                safari.delegate = self;
                safari.modalPresentationStyle = UIModalPresentationPageSheet;
                [self presentViewController:safari animated:YES completion:nil];
            } else {
                [self showAlert:@"下载完成" message:[NSString stringWithFormat:@"IPA已保存到应用文档目录\n%@", fileURL.lastPathComponent]];
            }
        }];
        return;
    }

    SFSafariViewController *safari = [[SFSafariViewController alloc] initWithURL:[NSURL URLWithString:self.app.downloadUrl]];
    safari.delegate = self;
    [self presentViewController:safari animated:YES completion:nil];
}

- (void)showAlert:(NSString *)title message:(NSString *)message {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:title message:message preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"确定" style:UIAlertActionStyleDefault handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
}

- (void)closeTapped {
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)safariViewControllerDidFinish:(SFSafariViewController *)controller {
    [controller dismissViewControllerAnimated:YES completion:nil];
}

- (UIStatusBarStyle)preferredStatusBarStyle {
    return UIStatusBarStyleLightContent;
}

@end
