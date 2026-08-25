#import "SignViewController.h"
#import "IPADownloadManager.h"
#import "CertificateManager.h"
#import "SignerBridge.h"

@implementation SignViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor clearColor];
    self.title = @"签名";
    [self setupUI];
    [self refreshIPAList];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    [self refreshIPAList];
}

- (void)setupUI {
    self.scrollView = [[UIScrollView alloc] initWithFrame:self.view.bounds];
    self.scrollView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.scrollView.showsVerticalScrollIndicator = NO;
    [self.view addSubview:self.scrollView];

    self.contentView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, self.view.bounds.size.width, 700)];
    self.contentView.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    [self.scrollView addSubview:self.contentView];

    self.titleLabel = [[UILabel alloc] initWithFrame:CGRectMake(20, 20, self.view.bounds.size.width - 40, 32)];
    self.titleLabel.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.titleLabel.text = @"本地 IPA 签名";
    self.titleLabel.textColor = [UIColor whiteColor];
    self.titleLabel.font = [UIFont boldSystemFontOfSize:22];
    [self.contentView addSubview:self.titleLabel];

    [self setupCertCard];
    [self setupIPACard];
    [self setupSignButton];
}

- (void)setupCertCard {
    self.certCard = [[UIView alloc] initWithFrame:CGRectMake(16, 64, self.view.bounds.size.width - 32, 160)];
    self.certCard.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.certCard.backgroundColor = [UIColor colorWithWhite:1.0 alpha:0.08];
    self.certCard.layer.cornerRadius = 16;
    self.certCard.layer.borderWidth = 0.5;
    self.certCard.layer.borderColor = [UIColor colorWithWhite:1.0 alpha:0.12].CGColor;
    [self.contentView addSubview:self.certCard];

    UIVisualEffect *blur = [UIBlurEffect effectWithStyle:UIBlurEffectStyleDark];
    UIVisualEffectView *bv = [[UIVisualEffectView alloc] initWithEffect:blur];
    bv.frame = self.certCard.bounds;
    bv.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.certCard addSubview:bv];

    UILabel *header = [[UILabel alloc] initWithFrame:CGRectMake(16, 12, 200, 20)];
    header.text = @"1. 证书";
    header.textColor = [UIColor whiteColor];
    header.font = [UIFont boldSystemFontOfSize:15];
    [self.certCard addSubview:header];

    self.certLabel = [[UILabel alloc] initWithFrame:CGRectMake(16, 40, self.certCard.bounds.size.width - 140, 24)];
    self.certLabel.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.certLabel.text = @"未导入证书";
    self.certLabel.textColor = [UIColor colorWithWhite:1.0 alpha:0.6];
    self.certLabel.font = [UIFont systemFontOfSize:13];
    [self.certCard addSubview:self.certLabel];

    self.importCertButton = [UIButton buttonWithType:UIButtonTypeSystem];
    self.importCertButton.frame = CGRectMake(self.certCard.bounds.size.width - 110, 36, 94, 32);
    self.importCertButton.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin;
    [self.importCertButton setTitle:@"导入证书" forState:UIControlStateNormal];
    [self.importCertButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
    self.importCertButton.backgroundColor = [UIColor colorWithRed:0.2 green:0.5 blue:0.9 alpha:0.8];
    self.importCertButton.layer.cornerRadius = 8;
    self.importCertButton.titleLabel.font = [UIFont systemFontOfSize:13 weight:UIFontWeightMedium];
    [self.importCertButton addTarget:self action:@selector(importCertTapped) forControlEvents:UIControlEventTouchUpInside];
    [self.certCard addSubview:self.importCertButton];

    UILabel *pwLabel = [[UILabel alloc] initWithFrame:CGRectMake(16, 76, 100, 20)];
    pwLabel.text = @"证书密码";
    pwLabel.textColor = [UIColor colorWithWhite:1.0 alpha:0.7];
    pwLabel.font = [UIFont systemFontOfSize:12];
    [self.certCard addSubview:pwLabel];

    self.passwordField = [[UITextField alloc] initWithFrame:CGRectMake(16, 100, self.certCard.bounds.size.width - 32, 40)];
    self.passwordField.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.passwordField.placeholder = @"输入 .p12 证书密码";
    self.passwordField.secureTextEntry = YES;
    self.passwordField.backgroundColor = [UIColor colorWithWhite:0 alpha:0.3];
    self.passwordField.textColor = [UIColor whiteColor];
    self.passwordField.layer.cornerRadius = 8;
    self.passwordField.leftView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 12, 40)];
    self.passwordField.leftViewMode = UITextFieldViewModeAlways;
    self.passwordField.delegate = self;
    [self.certCard addSubview:self.passwordField];
}

- (void)setupIPACard {
    CGFloat y = 240;
    self.ipaCard = [[UIView alloc] initWithFrame:CGRectMake(16, y, self.view.bounds.size.width - 32, 280)];
    self.ipaCard.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.ipaCard.backgroundColor = [UIColor colorWithWhite:1.0 alpha:0.08];
    self.ipaCard.layer.cornerRadius = 16;
    self.ipaCard.layer.borderWidth = 0.5;
    self.ipaCard.layer.borderColor = [UIColor colorWithWhite:1.0 alpha:0.12].CGColor;
    [self.contentView addSubview:self.ipaCard];

    UIVisualEffect *blur = [UIBlurEffect effectWithStyle:UIBlurEffectStyleDark];
    UIVisualEffectView *bv = [[UIVisualEffectView alloc] initWithEffect:blur];
    bv.frame = self.ipaCard.bounds;
    bv.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.ipaCard addSubview:bv];

    UILabel *header = [[UILabel alloc] initWithFrame:CGRectMake(16, 12, 200, 20)];
    header.text = @"2. 选择 IPA";
    header.textColor = [UIColor whiteColor];
    header.font = [UIFont boldSystemFontOfSize:15];
    [self.ipaCard addSubview:header];

    self.ipaLabel = [[UILabel alloc] initWithFrame:CGRectMake(16, 38, self.ipaCard.bounds.size.width - 140, 20)];
    self.ipaLabel.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.ipaLabel.text = @"从已下载列表选择";
    self.ipaLabel.textColor = [UIColor colorWithWhite:1.0 alpha:0.6];
    self.ipaLabel.font = [UIFont systemFontOfSize:12];
    [self.ipaCard addSubview:self.ipaLabel];

    self.importIPAButton = [UIButton buttonWithType:UIButtonTypeSystem];
    self.importIPAButton.frame = CGRectMake(self.ipaCard.bounds.size.width - 110, 34, 94, 30);
    self.importIPAButton.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin;
    [self.importIPAButton setTitle:@"导入IPA" forState:UIControlStateNormal];
    [self.importIPAButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
    self.importIPAButton.backgroundColor = [UIColor colorWithRed:0.2 green:0.5 blue:0.9 alpha:0.8];
    self.importIPAButton.layer.cornerRadius = 8;
    self.importIPAButton.titleLabel.font = [UIFont systemFontOfSize:12 weight:UIFontWeightMedium];
    [self.importIPAButton addTarget:self action:@selector(importIPATapped) forControlEvents:UIControlEventTouchUpInside];
    [self.ipaCard addSubview:self.importIPAButton];

    self.ipaTable = [[UITableView alloc] initWithFrame:CGRectMake(8, 70, self.ipaCard.bounds.size.width - 16, 200) style:UITableViewStylePlain];
    self.ipaTable.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.ipaTable.backgroundColor = [UIColor clearColor];
    self.ipaTable.separatorColor = [UIColor colorWithWhite:1.0 alpha:0.1];
    self.ipaTable.dataSource = self;
    self.ipaTable.delegate = self;
    self.ipaTable.showsVerticalScrollIndicator = NO;
    self.ipaTable.layer.cornerRadius = 8;
    [self.ipaTable registerClass:[UITableViewCell class] forCellReuseIdentifier:@"IPACell"];
    [self.ipaCard addSubview:self.ipaTable];
}

- (void)setupSignButton {
    self.signButton = [UIButton buttonWithType:UIButtonTypeCustom];
    self.signButton.frame = CGRectMake(16, 540, self.view.bounds.size.width - 32, 52);
    self.signButton.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.signButton.backgroundColor = [UIColor colorWithRed:0.2 green:0.6 blue:0.4 alpha:1.0];
    self.signButton.layer.cornerRadius = 16;
    [self.signButton setTitle:@"开始签名" forState:UIControlStateNormal];
    [self.signButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
    self.signButton.titleLabel.font = [UIFont boldSystemFontOfSize:17];
    self.signButton.layer.shadowColor = [UIColor colorWithRed:0.2 green:0.6 blue:0.4 alpha:0.5].CGColor;
    self.signButton.layer.shadowOffset = CGSizeMake(0, 4);
    self.signButton.layer.shadowRadius = 12;
    self.signButton.layer.shadowOpacity = 0.6;
    [self.signButton addTarget:self action:@selector(signTapped) forControlEvents:UIControlEventTouchUpInside];
    [self.contentView addSubview:self.signButton];

    self.signProgress = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleMedium];
    self.signProgress.center = CGPointMake(self.signButton.bounds.size.width / 2, self.signButton.bounds.size.height / 2);
    self.signProgress.hidesWhenStopped = YES;
    [self.signButton addSubview:self.signProgress];

    self.statusLabel = [[UILabel alloc] initWithFrame:CGRectMake(16, 604, self.view.bounds.size.width - 32, 40)];
    self.statusLabel.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.statusLabel.textAlignment = NSTextAlignmentCenter;
    self.statusLabel.textColor = [UIColor colorWithWhite:1.0 alpha:0.6];
    self.statusLabel.font = [UIFont systemFontOfSize:12];
    self.statusLabel.numberOfLines = 2;
    self.statusLabel.text = @"导入证书和密码，选择IPA后点击签名";
    [self.contentView addSubview:self.statusLabel];

    self.scrollView.contentSize = CGSizeMake(self.view.bounds.size.width, 660);
}

- (void)refreshIPAList {
    self.downloadedIPAs = [[IPADownloadManager sharedManager] listDownloadedIPAs];
    [self.ipaTable reloadData];
    if (self.downloadedIPAs.count == 0) {
        self.ipaLabel.text = @"暂无已下载IPA，可从详情页下载或导入";
    } else {
        self.ipaLabel.text = [NSString stringWithFormat:@"已下载 %lu 个IPA", (unsigned long)self.downloadedIPAs.count];
    }
}

- (void)importCertTapped {
    NSArray *types = @[@"com.rsa.pkcs-12", @"public.x509-certificate", @"public.data"];
    UIDocumentPickerViewController *picker = [[UIDocumentPickerViewController alloc] initWithDocumentTypes:types inMode:UIDocumentPickerModeImport];
    picker.delegate = self;
    picker.modalPresentationStyle = UIModalPresentationFormSheet;
    [self presentViewController:picker animated:YES completion:nil];
}

- (void)importIPATapped {
    NSArray *types = @[@"com.apple.itunes.ipa", @"public.zip-archive", @"public.data"];
    UIDocumentPickerViewController *picker = [[UIDocumentPickerViewController alloc] initWithDocumentTypes:types inMode:UIDocumentPickerModeImport];
    picker.delegate = self;
    picker.modalPresentationStyle = UIModalPresentationFormSheet;
    [self presentViewController:picker animated:YES completion:nil];
}

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    NSURL *url = urls.firstObject;
    if (!url) return;
    NSString *ext = url.pathExtension.lowercaseString;
    if ([ext isEqualToString:@"p12"] || [ext isEqualToString:@"pfx"] || [ext isEqualToString:@"cer"] || [ext isEqualToString:@"crt"]) {
        NSString *label = url.lastPathComponent;
        NSData *data = [NSData dataWithContentsOfURL:url];
        if (data) {
            [[CertificateManager sharedManager] saveCertificateData:data password:nil label:label];
            self.importedCertPath = [[CertificateManager sharedManager] pathForCertificateLabel:label];
            self.importedCertLabel = label;
            self.certLabel.text = label;
            self.certLabel.textColor = [UIColor colorWithRed:0.4 green:0.8 blue:0.5 alpha:1.0];
            self.statusLabel.text = @"证书已导入，请输入密码";
        }
    } else if ([ext isEqualToString:@"ipa"] || [ext isEqualToString:@"zip"]) {
        NSString *docs = [[IPADownloadManager sharedManager] documentsPath];
        NSString *dest = [docs stringByAppendingPathComponent:url.lastPathComponent];
        NSData *data = [NSData dataWithContentsOfURL:url];
        if (data) {
            [data writeToFile:dest atomically:YES];
            [self refreshIPAList];
            self.statusLabel.text = [NSString stringWithFormat:@"已导入: %@", url.lastPathComponent];
        }
    }
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.downloadedIPAs.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"IPACell" forIndexPath:indexPath];
    cell.backgroundColor = [UIColor clearColor];
    cell.textLabel.text = self.downloadedIPAs[indexPath.row];
    cell.textLabel.textColor = [UIColor whiteColor];
    cell.textLabel.font = [UIFont systemFontOfSize:13];
    cell.accessoryType = ([self.selectedIPA isEqualToString:self.downloadedIPAs[indexPath.row]]) ? UITableViewCellAccessoryCheckmark : UITableViewCellAccessoryNone;
    cell.tintColor = [UIColor colorWithRed:0.4 green:0.8 blue:0.5 alpha:1.0];
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    self.selectedIPA = self.downloadedIPAs[indexPath.row];
    [tableView reloadData];
    self.statusLabel.text = [NSString stringWithFormat:@"已选择: %@", self.selectedIPA];
}

- (void)signTapped {
    if (!self.importedCertPath && self.importedCertLabel.length == 0) {
        NSArray *certs = [[CertificateManager sharedManager] listCertificates];
        if (certs.count > 0) {
            self.importedCertLabel = certs.firstObject;
            self.importedCertPath = [[CertificateManager sharedManager] pathForCertificateLabel:self.importedCertLabel];
            self.certLabel.text = self.importedCertLabel;
        }
    }
    if (!self.importedCertPath || ![[NSFileManager defaultManager] fileExistsAtPath:self.importedCertPath]) {
        [self showAlert:@"需要证书" message:@"请先导入 .p12 证书文件"];
        return;
    }
    if (self.passwordField.text.length == 0) {
        [self showAlert:@"需要密码" message:@"请输入证书密码"];
        return;
    }
    if (!self.selectedIPA) {
        [self showAlert:@"选择IPA" message:@"请从列表中选择要签名的IPA"];
        return;
    }

    [self.signProgress startAnimating];
    [self.signButton setTitle:@"" forState:UIControlStateNormal];
    self.statusLabel.text = @"正在签名...";

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSString *docs = [[IPADownloadManager sharedManager] documentsPath];
        NSString *inputPath = [docs stringByAppendingPathComponent:self.selectedIPA];
        NSString *outputName = [self.selectedIPA.stringByDeletingPathExtension stringByAppendingString:@"_signed.ipa"];
        NSString *outputPath = [docs stringByAppendingPathComponent:outputName];

        NSError *error = nil;
        BOOL success = [[SignerBridge sharedBridge] signIPAAtPath:inputPath
                                                    certificatePath:self.importedCertPath
                                                           password:self.passwordField.text
                                                      provisionPath:nil
                                                         outputPath:outputPath
                                                     bundleIdOverride:nil
                                                              error:&error];

        dispatch_async(dispatch_get_main_queue(), ^{
            [self.signProgress stopAnimating];
            [self.signButton setTitle:@"开始签名" forState:UIControlStateNormal];
            if (success) {
                self.statusLabel.text = [NSString stringWithFormat:@"签名完成: %@", outputName];
                [self refreshIPAList];
                UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"签名成功" message:[NSString stringWithFormat:@"已输出: %@", outputName] preferredStyle:UIAlertControllerStyleAlert];
                [alert addAction:[UIAlertAction actionWithTitle:@"分享" style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
                    NSURL *url = [NSURL fileURLWithPath:outputPath];
                    UIActivityViewController *avc = [[UIActivityViewController alloc] initWithActivityItems:@[url] applicationActivities:nil];
                    avc.modalPresentationStyle = UIModalPresentationPopover;
                    avc.popoverPresentationController.sourceView = self.signButton;
                    [self presentViewController:avc animated:YES completion:nil];
                }]];
                [alert addAction:[UIAlertAction actionWithTitle:@"确定" style:UIAlertActionStyleCancel handler:nil]];
                [self presentViewController:alert animated:YES completion:nil];
            } else {
                self.statusLabel.text = [NSString stringWithFormat:@"签名失败: %@", error.localizedDescription ?: @"未知错误"];
                [self showAlert:@"签名失败" message:error.localizedDescription ?: @"未知错误"];
            }
        });
    });
}

- (void)showAlert:(NSString *)title message:(NSString *)message {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:title message:message preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"确定" style:UIAlertActionStyleDefault handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    [textField resignFirstResponder];
    return YES;
}

- (void)scrollViewWillBeginDragging:(UIScrollView *)scrollView {
    [self.passwordField resignFirstResponder];
}

@end
