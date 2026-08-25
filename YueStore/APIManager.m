#import "APIManager.h"

static NSString *const kAPIHTTPS = @"https://iosipa.cc.cd/data/app.js";
static NSString *const kAPIHTTP  = @"http://iosipa.cc.cd/data/app.js";

@implementation APIManager

+ (instancetype)sharedManager {
    static APIManager *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{ instance = [[APIManager alloc] init]; });
    return instance;
}

- (void)fetchAppsWithCompletion:(APICompletion)completion {
    [self requestURL:kAPIHTTPS fallbackURL:kAPIHTTP completion:completion];
}

- (void)requestURL:(NSString *)urlString fallbackURL:(NSString *)fallback completion:(APICompletion)completion {
    NSURL *url = [NSURL URLWithString:urlString];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
    request.timeoutInterval = 15;
    request.cachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
    NSURLSessionConfiguration *config = [NSURLSessionConfiguration defaultSessionConfiguration];
    config.requestCachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
    NSURLSession *session = [NSURLSession sessionWithConfiguration:config];
    NSURLSessionDataTask *task = [session dataTaskWithRequest:request completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
        NSHTTPURLResponse *httpResp = (NSHTTPURLResponse *)response;
        if (error || data.length == 0 || httpResp.statusCode != 200) {
            if (fallback) {
                NSLog(@"APIManager: %@ failed (%@), falling back to %@", urlString, error.localizedDescription ?: [NSString stringWithFormat:@"HTTP %ld", (long)httpResp.statusCode], fallback);
                [self requestURL:fallback fallbackURL:nil completion:completion];
            } else {
                dispatch_async(dispatch_get_main_queue(), ^{
                    completion(nil, error ?: [NSError errorWithDomain:@"APIManager" code:httpResp.statusCode userInfo:@{NSLocalizedDescriptionKey:@"请求失败"}]);
                });
            }
            return;
        }
        NSArray<AppModel *> *apps = [AppModel parseFromJSData:data];
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(apps, nil);
        });
    }];
    [task resume];
}

@end
