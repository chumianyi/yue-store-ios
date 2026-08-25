#import <Foundation/Foundation.h>

typedef void (^DownloadCompletion)(NSURL *fileURL, NSError *error);

@interface IPADownloadManager : NSObject
+ (instancetype)sharedManager;
- (void)downloadIPAFromURL:(NSString *)urlString filename:(NSString *)filename completion:(DownloadCompletion)completion;
- (NSArray<NSString *> *)listDownloadedIPAs;
- (NSString *)documentsPath;
@end
