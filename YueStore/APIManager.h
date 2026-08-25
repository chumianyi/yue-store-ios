#import <Foundation/Foundation.h>
#import "AppModel.h"

typedef void (^APICompletion)(NSArray<AppModel *> *apps, NSError *error);

@interface APIManager : NSObject
+ (instancetype)sharedManager;
- (void)fetchAppsWithCompletion:(APICompletion)completion;
@end
