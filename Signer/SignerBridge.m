#import "SignerBridge.h"
#import "ipa_signer.h"

@implementation SignerBridge

+ (instancetype)sharedBridge {
    static SignerBridge *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{ instance = [[SignerBridge alloc] init]; });
    return instance;
}

- (BOOL)signIPAAtPath:(NSString *)ipaPath
       certificatePath:(NSString *)p12Path
              password:(NSString *)password
            provisionPath:(NSString *)mobileprovisionPath
             outputPath:(NSString *)outputPath
         bundleIdOverride:(NSString *)bundleId
                  error:(NSError **)error
{
    const char *ipa_c = [ipaPath UTF8String];
    const char *p12_c = [p12Path UTF8String];
    const char *pw_c = password ? [password UTF8String] : "";
    const char *mp_c = mobileprovisionPath ? [mobileprovisionPath UTF8String] : NULL;
    const char *out_c = [outputPath UTF8String];
    const char *bid_c = bundleId ? [bundleId UTF8String] : NULL;

    ipa_sign_result_t result = ipa_sign_ipa(ipa_c, p12_c, pw_c, mp_c, out_c, bid_c);

    if (!result.success) {
        if (error) {
            *error = [NSError errorWithDomain:@"yue.store.signer" code:-1
                                      userInfo:@{NSLocalizedDescriptionKey: [NSString stringWithUTF8String:result.error_message] ?: @"签名失败"}];
        }
        return NO;
    }
    return YES;
}

- (NSString *)lastErrorMessage {
    return nil;
}

@end
