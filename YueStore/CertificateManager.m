#import "CertificateManager.h"
#import <Security/Security.h>

@implementation CertificateManager

+ (instancetype)sharedManager {
    static CertificateManager *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{ instance = [[CertificateManager alloc] init]; });
    return instance;
}

- (NSString *)certificatesDirectory {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docs = paths.firstObject;
    NSString *certDir = [docs stringByAppendingPathComponent:@"certificates"];
    NSFileManager *fm = [NSFileManager defaultManager];
    if (![fm fileExistsAtPath:certDir]) {
        [fm createDirectoryAtPath:certDir withIntermediateDirectories:YES attributes:nil error:nil];
    }
    return certDir;
}

- (NSString *)pathForCertificateLabel:(NSString *)label {
    NSString *safe = [label stringByReplacingOccurrencesOfString:@"/" withString:@"_"];
    return [[self certificatesDirectory] stringByAppendingPathComponent:safe];
}

- (BOOL)saveCertificateData:(NSData *)data password:(NSString *)password label:(NSString *)label {
    if (!data || !label) return NO;
    NSString *path = [self pathForCertificateLabel:label];
    BOOL ok = [data writeToFile:path atomically:YES];
    if (!ok) return NO;
    if (password) {
        [self savePassword:password forLabel:label];
    }
    return YES;
}

- (BOOL)saveCertificateFile:(NSURL *)fileURL label:(NSString *)label password:(NSString *)password {
    NSData *data = [NSData dataWithContentsOfURL:fileURL];
    if (!data) return NO;
    return [self saveCertificateData:data password:password label:label];
}

- (NSData *)getCertificateDataWithLabel:(NSString *)label {
    NSString *path = [self pathForCertificateLabel:label];
    return [NSData dataWithContentsOfFile:path];
}

- (NSString *)getCertificatePasswordWithLabel:(NSString *)label {
    return [self getPasswordForLabel:label];
}

- (NSArray<NSString *> *)listCertificates {
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *dir = [self certificatesDirectory];
    NSError *error = nil;
    NSArray *files = [fm contentsOfDirectoryAtPath:dir error:&error];
    if (error) return @[];
    NSMutableArray *result = [NSMutableArray array];
    for (NSString *f in files) {
        if (![f hasPrefix:@"."]) {
            [result addObject:f];
        }
    }
    return result;
}

- (BOOL)deleteCertificateWithLabel:(NSString *)label {
    NSString *path = [self pathForCertificateLabel:label];
    NSFileManager *fm = [NSFileManager defaultManager];
    NSError *error = nil;
    [fm removeItemAtPath:path error:&error];
    [self deletePasswordForLabel:label];
    return error == nil;
}

- (void)savePassword:(NSString *)password forLabel:(NSString *)label {
    NSDictionary *query = @{
        (__bridge id)kSecClass: (__bridge id)kSecClassGenericPassword,
        (__bridge id)kSecAttrService: @"yue.store.cert",
        (__bridge id)kSecAttrAccount: label,
        (__bridge id)kSecValueData: [password dataUsingEncoding:NSUTF8StringEncoding]
    };
    SecItemDelete((__bridge CFDictionaryRef)query);
    SecItemAdd((__bridge CFDictionaryRef)query, NULL);
}

- (NSString *)getPasswordForLabel:(NSString *)label {
    NSDictionary *query = @{
        (__bridge id)kSecClass: (__bridge id)kSecClassGenericPassword,
        (__bridge id)kSecAttrService: @"yue.store.cert",
        (__bridge id)kSecAttrAccount: label,
        (__bridge id)kSecReturnData: (__bridge id)kCFBooleanTrue,
        (__bridge id)kSecMatchLimit: (__bridge id)kSecMatchLimitOne
    };
    CFDataRef data = NULL;
    OSStatus status = SecItemCopyMatching((__bridge CFDictionaryRef)query, (CFTypeRef *)&data);
    if (status != errSecSuccess || !data) return nil;
    NSString *pw = [[NSString alloc] initWithData:(__bridge NSData *)data encoding:NSUTF8StringEncoding];
    CFRelease(data);
    return pw;
}

- (void)deletePasswordForLabel:(NSString *)label {
    NSDictionary *query = @{
        (__bridge id)kSecClass: (__bridge id)kSecClassGenericPassword,
        (__bridge id)kSecAttrService: @"yue.store.cert",
        (__bridge id)kSecAttrAccount: label
    };
    SecItemDelete((__bridge CFDictionaryRef)query);
}

@end
