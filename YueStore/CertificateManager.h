#import <Foundation/Foundation.h>

@interface CertificateManager : NSObject
+ (instancetype)sharedManager;
- (BOOL)saveCertificateData:(NSData *)data password:(NSString *)password label:(NSString *)label;
- (NSData *)getCertificateDataWithLabel:(NSString *)label;
- (NSString *)getCertificatePasswordWithLabel:(NSString *)label;
- (NSArray<NSString *> *)listCertificates;
- (BOOL)deleteCertificateWithLabel:(NSString *)label;
- (NSString *)certificatesDirectory;
- (BOOL)saveCertificateFile:(NSURL *)fileURL label:(NSString *)label password:(NSString *)password;
- (NSString *)pathForCertificateLabel:(NSString *)label;
@end
