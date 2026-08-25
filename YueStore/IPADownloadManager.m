#import "IPADownloadManager.h"

@implementation IPADownloadManager

+ (instancetype)sharedManager {
    static IPADownloadManager *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{ instance = [[IPADownloadManager alloc] init]; });
    return instance;
}

- (NSString *)documentsPath {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    return paths.firstObject;
}

- (void)downloadIPAFromURL:(NSString *)urlString filename:(NSString *)filename completion:(DownloadCompletion)completion {
    if (!urlString || urlString.length == 0) {
        if (completion) completion(nil, [NSError errorWithDomain:@"IPADownload" code:-1 userInfo:@{NSLocalizedDescriptionKey:@"URL为空"}]);
        return;
    }
    NSURL *url = [NSURL URLWithString:urlString];
    if (!url) {
        if (completion) completion(nil, [NSError errorWithDomain:@"IPADownload" code:-2 userInfo:@{NSLocalizedDescriptionKey:@"URL无效"}]);
        return;
    }

    NSString *safeName = filename ?: @"download.ipa";
    safeName = [safeName stringByReplacingOccurrencesOfString:@"/" withString:@"_"];
    NSString *destPath = [[self documentsPath] stringByAppendingPathComponent:safeName];

    NSURLSessionConfiguration *config = [NSURLSessionConfiguration defaultSessionConfiguration];
    config.timeoutIntervalForRequest = 60;
    config.timeoutIntervalForResource = 300;
    NSURLSession *session = [NSURLSession sessionWithConfiguration:config];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
    [request setValue:@"Mozilla/5.0 (iPhone; CPU iPhone OS 18_0 like Mac OS X) AppleWebKit/605.1.15" forHTTPHeaderField:@"User-Agent"];

    NSURLSessionDownloadTask *task = [session downloadTaskWithRequest:request completionHandler:^(NSURL *location, NSURLResponse *response, NSError *error) {
        if (error) {
            if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(nil, error); });
            return;
        }
        NSHTTPURLResponse *httpResp = (NSHTTPURLResponse *)response;
        if (httpResp.statusCode != 200) {
            if (completion) dispatch_async(dispatch_get_main_queue(), ^{
                completion(nil, [NSError errorWithDomain:@"IPADownload" code:httpResp.statusCode userInfo:@{NSLocalizedDescriptionKey:[NSString stringWithFormat:@"HTTP %ld", (long)httpResp.statusCode]}]);
            });
            return;
        }
        NSFileManager *fm = [NSFileManager defaultManager];
        NSError *moveError = nil;
        if ([fm fileExistsAtPath:destPath]) {
            [fm removeItemAtPath:destPath error:nil];
        }
        [fm moveItemAtURL:location toURL:[NSURL fileURLWithPath:destPath] error:&moveError];
        if (moveError) {
            if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(nil, moveError); });
            return;
        }
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion([NSURL fileURLWithPath:destPath], nil); });
    }];
    [task resume];
}

- (NSArray<NSString *> *)listDownloadedIPAs {
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *docs = [self documentsPath];
    NSError *error = nil;
    NSArray *files = [fm contentsOfDirectoryAtPath:docs error:&error];
    if (error) return @[];
    NSMutableArray *ipas = [NSMutableArray array];
    for (NSString *file in files) {
        if ([[file pathExtension] isEqualToString:@"ipa"]) {
            [ipas addObject:file];
        }
    }
    return ipas;
}

@end
