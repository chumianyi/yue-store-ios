#import "AppModel.h"

@implementation AppModel

+ (NSArray<AppModel *> *)parseFromJSData:(NSData *)data {
    if (!data) return @[];
    NSString *raw = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    if (!raw) return @[];
    NSMutableArray<AppModel *> *result = [NSMutableArray array];
    NSScanner *scanner = [NSScanner scannerWithString:raw];
    [scanner scanUpToString:@"{" intoString:nil];
    while (![scanner isAtEnd]) {
        if (![scanner scanString:@"{" intoString:nil]) break;
        AppModel *app = [[AppModel alloc] init];
        app.version = @"--";
        app.size = @"--";
        app.isSigned = NO;
        app.tag = @"";
        app.downloads = 0;
        while (1) {
            NSString *key = [self scanJSKey:scanner];
            if (!key) break;
            if ([key isEqualToString:@"name"]) {
                app.name = [self scanJSString:scanner];
            } else if ([key isEqualToString:@"filename"]) {
                app.filename = [self scanJSString:scanner];
            } else if ([key isEqualToString:@"downloadUrl"]) {
                app.downloadUrl = [self scanJSString:scanner];
            } else if ([key isEqualToString:@"version"]) {
                app.version = [self scanJSString:scanner];
            } else if ([key isEqualToString:@"size"]) {
                app.size = [self scanJSString:scanner];
            } else if ([key isEqualToString:@"category"]) {
                app.category = [self scanJSString:scanner];
            } else if ([key isEqualToString:@"subCategory"]) {
                app.subCategory = [self scanJSString:scanner];
            } else if ([key isEqualToString:@"description"]) {
                app.desc = [self scanJSString:scanner];
            } else if ([key isEqualToString:@"signed"]) {
                app.isSigned = [self scanJSBool:scanner];
            } else if ([key isEqualToString:@"tag"]) {
                app.tag = [self scanJSString:scanner];
            } else if ([key isEqualToString:@"downloads"]) {
                app.downloads = [self scanJSNumber:scanner];
            } else {
                [self scanJSValue:scanner];
            }
            [scanner scanString:@"," intoString:nil];
            if ([scanner scanString:@"}" intoString:nil]) break;
        }
        if (app.name && app.name.length > 0) {
            [result addObject:app];
        }
        [scanner scanString:@"," intoString:nil];
    }
    return result;
}

+ (NSString *)scanJSKey:(NSScanner *)scanner {
    [scanner scanCharactersFromSet:[NSCharacterSet whitespaceAndNewlineCharacterSet] intoString:nil];
    NSString *key = nil;
    if ([scanner scanString:@"\"" intoString:nil]) {
        [scanner scanUpToString:@"\"" intoString:&key];
        [scanner scanString:@"\"" intoString:nil];
    } else {
        [scanner scanUpToString:@":" intoString:&key];
        key = [key stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    }
    [scanner scanString:@":" intoString:nil];
    return key;
}

+ (NSString *)scanJSString:(NSScanner *)scanner {
    [scanner scanCharactersFromSet:[NSCharacterSet whitespaceAndNewlineCharacterSet] intoString:nil];
    if (![scanner scanString:@"\"" intoString:nil]) {
        [self scanJSValue:scanner];
        return @"";
    }
    NSMutableString *str = [NSMutableString string];
    while (![scanner isAtEnd]) {
        NSString *part = nil;
        [scanner scanUpToString:@"\\" intoString:&part];
        if (part) [str appendString:part];
        if ([scanner scanString:@"\\" intoString:nil]) {
            NSString *esc = nil;
            [scanner scanCharactersFromSet:[NSCharacterSet characterSetWithCharactersInString:@"\"\\/bfnrtu"] intoString:&esc];
            if ([esc isEqualToString:@"n"]) [str appendString:@"\n"];
            else if ([esc isEqualToString:@"t"]) [str appendString:@"\t"];
            else if ([esc isEqualToString:@"r"]) [str appendString:@"\r"];
            else if ([esc isEqualToString:@"\\\""]) [str appendString:@"\""];
            else if ([esc isEqualToString:@"\\\\"]) [str appendString:@"\\"];
            else if ([esc isEqualToString:@"/"]) [str appendString:@"/"];
            else if ([esc isEqualToString:@"u"]) {
                NSString *hex = nil;
                [scanner scanCharactersFromSet:[NSCharacterSet characterSetWithCharactersInString:@"0123456789abcdefABCDEF"] intoString:&hex];
                if (hex.length == 4) {
                    unsigned int val = 0;
                    NSScanner *hs = [NSScanner scannerWithString:hex];
                    [hs scanHexInt:&val];
                    [str appendFormat:@"%C", (unichar)val];
                }
            }
        } else {
            break;
        }
    }
    [scanner scanString:@"\"" intoString:nil];
    return str;
}

+ (BOOL)scanJSBool:(NSScanner *)scanner {
    [scanner scanCharactersFromSet:[NSCharacterSet whitespaceAndNewlineCharacterSet] intoString:nil];
    if ([scanner scanString:@"true" intoString:nil]) return YES;
    [scanner scanString:@"false" intoString:nil];
    return NO;
}

+ (NSInteger)scanJSNumber:(NSScanner *)scanner {
    [scanner scanCharactersFromSet:[NSCharacterSet whitespaceAndNewlineCharacterSet] intoString:nil];
    long long val = 0;
    [scanner scanLongLong:&val];
    return (NSInteger)val;
}

+ (void)scanJSValue:(NSScanner *)scanner {
    [scanner scanCharactersFromSet:[NSCharacterSet whitespaceAndNewlineCharacterSet] intoString:nil];
    if ([scanner scanString:@"\"" intoString:nil]) {
        [scanner scanUpToString:@"\"" intoString:nil];
        [scanner scanString:@"\"" intoString:nil];
    } else if ([scanner scanString:@"{" intoString:nil]) {
        int depth = 1;
        while (depth > 0 && ![scanner isAtEnd]) {
            NSString *c = nil;
            [scanner scanCharactersFromSet:[NSCharacterSet characterSetWithCharactersInString:@"{}"] intoString:&c];
            for (int i = 0; i < c.length; i++) {
                unichar ch = [c characterAtIndex:i];
                if (ch == '{') depth++;
                else if (ch == '}') depth--;
            }
        }
    } else {
        [scanner scanUpToCharactersFromSet:[NSCharacterSet characterSetWithCharactersInString:@",}"] intoString:nil];
    }
}

- (BOOL)isDirectInstallLink {
    if (!self.downloadUrl) return NO;
    return [self.downloadUrl hasPrefix:@"itms-services://"];
}

@end
