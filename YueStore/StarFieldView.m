#import "StarFieldView.h"

@implementation StarFieldView

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self setupBackground];
        [self setupNebula];
        [self setupStarEmitter];
        [self setupDustEmitter];
        [self startAnimation];
    }
    return self;
}

- (void)setupBackground {
    self.backgroundColor = [UIColor colorWithRed:0.02 green:0.02 blue:0.08 alpha:1.0];
    CAGradientLayer *bg = [CAGradientLayer layer];
    bg.frame = self.bounds;
    bg.colors = @[
        (id)[UIColor colorWithRed:0.01 green:0.01 blue:0.05 alpha:1.0].CGColor,
        (id)[UIColor colorWithRed:0.03 green:0.02 blue:0.10 alpha:1.0].CGColor,
        (id)[UIColor colorWithRed:0.05 green:0.03 blue:0.15 alpha:1.0].CGColor,
        (id)[UIColor colorWithRed:0.02 green:0.01 blue:0.08 alpha:1.0].CGColor
    ];
    bg.startPoint = CGPointMake(0, 0);
    bg.endPoint = CGPointMake(1, 1);
    [self.layer addSublayer:bg];
}

- (void)setupNebula {
    self.nebulaGradient = [CAGradientLayer layer];
    self.nebulaGradient.frame = CGRectMake(-self.bounds.size.width * 0.5, -self.bounds.size.height * 0.5,
                                             self.bounds.size.width * 2, self.bounds.size.height * 2);
    self.nebulaGradient.type = @"radial";
    self.nebulaGradient.colors = @[
        (id)[UIColor colorWithRed:0.2 green:0.1 blue:0.5 alpha:0.15].CGColor,
        (id)[UIColor colorWithRed:0.1 green:0.2 blue:0.6 alpha:0.08].CGColor,
        (id)[UIColor clearColor].CGColor
    ];
    self.nebulaGradient.startPoint = CGPointMake(0.3, 0.3);
    self.nebulaGradient.endPoint = CGPointMake(1.0, 1.0);
    [self.layer addSublayer:self.nebulaGradient];

    CABasicAnimation *drift = [CABasicAnimation animationWithKeyPath:@"position"];
    drift.fromValue = [NSValue valueWithCGPoint:CGPointMake(self.nebulaGradient.position.x - 100, self.nebulaGradient.position.y - 60)];
    drift.toValue = [NSValue valueWithCGPoint:CGPointMake(self.nebulaGradient.position.x + 100, self.nebulaGradient.position.y + 60)];
    drift.duration = 40.0;
    drift.autoreverses = YES;
    drift.repeatCount = HUGE_VALF;
    [self.nebulaGradient addAnimation:drift forKey:@"nebulaDrift"];
}

- (void)setupStarEmitter {
    self.starEmitter = [CAEmitterLayer layer];
    self.starEmitter.emitterPosition = CGPointMake(self.bounds.size.width / 2.0, self.bounds.size.height / 2.0);
    self.starEmitter.emitterSize = CGSizeMake(self.bounds.size.width * 1.5, self.bounds.size.height * 1.5);
    self.starEmitter.emitterShape = kCAEmitterLayerRectangle;
    self.starEmitter.renderMode = kCAEmitterLayerOldestFirst;
    self.starEmitter.birthRate = 1.0;

    CAEmitterCell *brightStars = [CAEmitterCell emitterCell];
    brightStars.contents = (id)[self starImageWithColor:[UIColor whiteColor] size:3.0].CGImage;
    brightStars.birthRate = 80;
    brightStars.lifetime = 8.0;
    brightStars.lifetimeRange = 4.0;
    brightStars.velocity = 8.0;
    brightStars.velocityRange = 5.0;
    brightStars.emissionRange = M_PI * 2.0;
    brightStars.scale = 0.6;
    brightStars.scaleRange = 0.8;
    brightStars.alphaSpeed = -0.05;
    brightStars.spin = 0.0;
    [brightStars setValue:@3 forKeyPath:@"layer.drawsAsynchronously"];

    CAEmitterCell *coloredStars = [CAEmitterCell emitterCell];
    coloredStars.contents = (id)[self starImageWithColor:[UIColor colorWithRed:0.6 green:0.8 blue:1.0 alpha:1.0] size:2.0].CGImage;
    coloredStars.birthRate = 40;
    coloredStars.lifetime = 10.0;
    coloredStars.lifetimeRange = 5.0;
    coloredStars.velocity = 5.0;
    coloredStars.velocityRange = 3.0;
    coloredStars.emissionRange = M_PI * 2.0;
    coloredStars.scale = 0.4;
    coloredStars.scaleRange = 0.6;
    coloredStars.alphaSpeed = -0.03;

    CAEmitterCell *twinkleStars = [CAEmitterCell emitterCell];
    twinkleStars.contents = (id)[self starImageWithColor:[UIColor colorWithRed:1.0 green:0.9 blue:0.7 alpha:1.0] size:4.0].CGImage;
    twinkleStars.birthRate = 20;
    twinkleStars.lifetime = 6.0;
    twinkleStars.lifetimeRange = 3.0;
    twinkleStars.velocity = 3.0;
    twinkleStars.velocityRange = 2.0;
    twinkleStars.emissionRange = M_PI * 2.0;
    twinkleStars.scale = 0.8;
    twinkleStars.scaleRange = 1.0;
    twinkleStars.alphaSpeed = -0.08;

    self.starEmitter.emitterCells = @[brightStars, coloredStars, twinkleStars];
    [self.layer addSublayer:self.starEmitter];
}

- (void)setupDustEmitter {
    self.dustEmitter = [CAEmitterLayer layer];
    self.dustEmitter.emitterPosition = CGPointMake(self.bounds.size.width / 2.0, -20);
    self.dustEmitter.emitterSize = CGSizeMake(self.bounds.size.width * 2, 10);
    self.dustEmitter.emitterShape = kCAEmitterLayerLine;
    self.dustEmitter.renderMode = kCAEmitterLayerOldestFirst;

    CAEmitterCell *dust = [CAEmitterCell emitterCell];
    dust.contents = (id)[self starImageWithColor:[UIColor colorWithRed:0.5 green:0.6 blue:0.9 alpha:0.3] size:1.5].CGImage;
    dust.birthRate = 30;
    dust.lifetime = 15.0;
    dust.velocity = 12.0;
    dust.velocityRange = 6.0;
    dust.emissionLongitude = M_PI_2;
    dust.emissionRange = M_PI / 6.0;
    dust.scale = 0.3;
    dust.scaleRange = 0.4;
    dust.alphaSpeed = -0.02;

    self.dustEmitter.emitterCells = @[dust];
    [self.layer addSublayer:self.dustEmitter];
}

- (UIImage *)starImageWithColor:(UIColor *)color size:(CGFloat)size {
    CGSize s = CGSizeMake(size * 4, size * 4);
    UIGraphicsBeginImageContextWithOptions(s, NO, [UIScreen mainScreen].scale);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSetFillColorWithColor(ctx, color.CGColor);
    CGFloat cx = s.width / 2.0;
    CGFloat cy = s.height / 2.0;
    for (int i = 0; i < 8; i++) {
        CGFloat angle = i * M_PI / 4.0;
        CGFloat len = (i % 2 == 0) ? size * 2 : size * 0.5;
        CGFloat x = cx + cos(angle) * len;
        CGFloat y = cy + sin(angle) * len;
        if (i == 0) CGContextMoveToPoint(ctx, x, y);
        else CGContextAddLineToPoint(ctx, x, y);
    }
    CGContextClosePath(ctx);
    CGContextFillPath(ctx);
    CGContextAddEllipseInRect(ctx, CGRectMake(cx - size * 0.5, cy - size * 0.5, size, size));
    CGContextFillPath(ctx);
    UIImage *img = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return img;
}

- (void)startAnimation {
    self.starEmitter.lifetime = 1.0;
    self.dustEmitter.lifetime = 1.0;
    self.starEmitter.speed = 1.0;
    self.dustEmitter.speed = 1.0;
}

- (void)pauseAnimation {
    self.starEmitter.speed = 0.0;
    self.dustEmitter.speed = 0.0;
}

- (void)layoutSubviews {
    [super layoutSubviews];
    for (CALayer *layer in self.layer.sublayers) {
        if ([layer isKindOfClass:[CAGradientLayer class]]) {
            layer.frame = self.bounds;
        }
    }
    self.starEmitter.emitterPosition = CGPointMake(self.bounds.size.width / 2.0, self.bounds.size.height / 2.0);
    self.starEmitter.emitterSize = CGSizeMake(self.bounds.size.width * 1.5, self.bounds.size.height * 1.5);
    self.dustEmitter.emitterPosition = CGPointMake(self.bounds.size.width / 2.0, -20);
    self.dustEmitter.emitterSize = CGSizeMake(self.bounds.size.width * 2, 10);
}

@end
