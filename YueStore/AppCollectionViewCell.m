#import "AppCollectionViewCell.h"

@implementation AppCollectionViewCell

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self setupUI];
    }
    return self;
}

- (void)setupUI {
    self.cardView = [[UIView alloc] initWithFrame:self.contentView.bounds];
    self.cardView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.cardView.backgroundColor = [UIColor colorWithWhite:1.0 alpha:0.08];
    self.cardView.layer.cornerRadius = 14;
    self.cardView.layer.borderWidth = 0.5;
    self.cardView.layer.borderColor = [UIColor colorWithWhite:1.0 alpha:0.12].CGColor;
    self.cardView.clipsToBounds = YES;
    [self.contentView addSubview:self.cardView];

    UIVisualEffect *blur = [UIBlurEffect effectWithStyle:UIBlurEffectStyleDark];
    UIVisualEffectView *blurView = [[UIVisualEffectView alloc] initWithEffect:blur];
    blurView.frame = self.cardView.bounds;
    blurView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.cardView addSubview:blurView];

    self.iconView = [[UIView alloc] initWithFrame:CGRectMake(12, 14, 44, 44)];
    self.iconView.backgroundColor = [UIColor colorWithRed:0.2 green:0.4 blue:0.8 alpha:0.6];
    self.iconView.layer.cornerRadius = 10;
    self.iconView.clipsToBounds = YES;
    [self.cardView addSubview:self.iconView];

    self.iconLetter = [[UILabel alloc] initWithFrame:self.iconView.bounds];
    self.iconLetter.textAlignment = NSTextAlignmentCenter;
    self.iconLetter.textColor = [UIColor whiteColor];
    self.iconLetter.font = [UIFont boldSystemFontOfSize:20];
    [self.iconView addSubview:self.iconLetter];

    self.nameLabel = [[UILabel alloc] initWithFrame:CGRectMake(64, 14, self.cardView.bounds.size.width - 74, 22)];
    self.nameLabel.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.nameLabel.textColor = [UIColor whiteColor];
    self.nameLabel.font = [UIFont systemFontOfSize:14 weight:UIFontWeightSemibold];
    self.nameLabel.lineBreakMode = NSLineBreakByTruncatingTail;
    [self.cardView addSubview:self.nameLabel];

    self.categoryLabel = [[UILabel alloc] initWithFrame:CGRectMake(64, 38, self.cardView.bounds.size.width - 74, 16)];
    self.categoryLabel.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.categoryLabel.textColor = [UIColor colorWithWhite:1.0 alpha:0.6];
    self.categoryLabel.font = [UIFont systemFontOfSize:11];
    [self.cardView addSubview:self.categoryLabel];

    self.sizeLabel = [[UILabel alloc] initWithFrame:CGRectMake(12, 68, self.cardView.bounds.size.width - 24, 16)];
    self.sizeLabel.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.sizeLabel.textColor = [UIColor colorWithWhite:1.0 alpha:0.5];
    self.sizeLabel.font = [UIFont systemFontOfSize:10];
    [self.cardView addSubview:self.sizeLabel];
}

- (void)configureWithApp:(AppModel *)app {
    self.nameLabel.text = app.name ?: @"未知应用";
    self.categoryLabel.text = app.subCategory ?: app.category ?: @"";
    NSMutableString *info = [NSMutableString string];
    if (app.version && ![app.version isEqualToString:@"--"]) [info appendFormat:@"v%@  ", app.version];
    if (app.size && ![app.size isEqualToString:@"--"]) [info appendString:app.size];
    if (app.signed) [info appendString:@"  已签名"];
    self.sizeLabel.text = info;

    if (app.name.length > 0) {
        self.iconLetter.text = [[app.name substringToIndex:1] uppercaseString];
    }
    CGFloat hue = ([app.name hash] % 100) / 100.0;
    self.iconView.backgroundColor = [UIColor colorWithHue:hue saturation:0.5 brightness:0.7 alpha:0.7];
}

@end
