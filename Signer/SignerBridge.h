#import <Foundation/Foundation.h>

@interface SignerBridge : NSObject
+ (instancetype)sharedBridge;
- (BOOL)signIPAAtPath:(NSString *)ipaPath
       certificatePath:(NSString *)p12Path
              password:(NSString *)password
            provisionPath:(NSString *)mobileprovisionPath
             outputPath:(NSString *)outputPath
         bundleIdOverride:(NSString *)bundleId
                  error:(NSError **)error;
- (NSString *)lastErrorMessage;
@end
