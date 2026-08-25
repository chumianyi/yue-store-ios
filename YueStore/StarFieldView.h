#import <UIKit/UIKit.h>

@interface StarFieldView : UIView
@property (nonatomic, strong) CAEmitterLayer *starEmitter;
@property (nonatomic, strong) CAEmitterLayer *dustEmitter;
@property (nonatomic, strong) CAGradientLayer *nebulaGradient;
- (void)startAnimation;
- (void)pauseAnimation;
@end
