#import <Foundation/Foundation.h>

@interface AppModel : NSObject
@property (nonatomic, copy) NSString *name;
@property (nonatomic, copy) NSString *filename;
@property (nonatomic, copy) NSString *downloadUrl;
@property (nonatomic, copy) NSString *version;
@property (nonatomic, copy) NSString *size;
@property (nonatomic, copy) NSString *category;
@property (nonatomic, copy) NSString *subCategory;
@property (nonatomic, copy) NSString *desc;
@property (nonatomic, assign) BOOL isSigned;
@property (nonatomic, copy) NSString *tag;
@property (nonatomic, assign) NSInteger downloads;
+ (NSArray<AppModel *> *)parseFromJSData:(NSData *)data;
- (BOOL)isDirectInstallLink;
@end
