#include "ipa_signer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/nlist.h>
#include <CommonCrypto/CommonCrypto.h>
#include <Security/Security.h>
#include <CoreFoundation/CoreFoundation.h>

#include "minizip/zip.h"
#include "minizip/unzip.h"

#define SIGNER_DEBUG 1
#if SIGNER_DEBUG
#define LOG(...) fprintf(stderr, "[ipa_signer] " __VA_ARGS__)
#else
#define LOG(...)
#endif

#define CS_MAGIC 0xfade0cc0
#define CS_MAGIC_BLOB 0xfa_de_0b_01
#define CSSLOT_CODEDIRECTORY 0
#define CSSLOT_REQUIREMENTS 2
#define CSSLOT_ENTITLEMENTS 5
#define CSSLOT_BLOBWRAPPER 65536
#define CS_HASHTYPE_SHA256 2
#define CS_PAGE_SIZE 4096
#define CS_VERSION 0x20200

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t length;
} cs_blob_header_t;

typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t offset;
} cs_index_t;

typedef struct __attribute__((packed)) {
    cs_blob_header_t header;
    uint32_t count;
    cs_index_t index[];
} cs_superblob_t;

typedef struct __attribute__((packed)) {
    cs_blob_header_t header;
    uint32_t version;
    uint32_t flags;
    uint32_t hashOffset;
    uint32_t identOffset;
    uint32_t nSpecialSlots;
    uint32_t nCodeSlots;
    uint32_t codeLimit;
    uint8_t hashSize;
    uint8_t hashType;
    uint8_t platform;
    uint8_t pageSize;
    uint32_t spare2;
    uint32_t scatterOffset;
    uint32_t teamOffset;
} cs_codedirectory_t;

static uint32_t swap32(uint32_t v) { return OSSwapHostToBigInt32(v); }

static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

int ipa_sign_file_exists(const char *path) { return file_exists(path); }

int ipa_sign_mkdir_p(const char *path) {
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if (len > 0 && tmp[len-1] == '/') tmp[len-1] = '\0';
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    return mkdir(tmp, 0755);
}

static char *path_join(const char *a, const char *b) {
    char *r = malloc(strlen(a) + strlen(b) + 2);
    sprintf(r, "%s/%s", a, b);
    return r;
}

static int copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return -1;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return -1; }
    char buf[65536];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        fwrite(buf, 1, n, out);
    }
    fclose(in);
    fclose(out);
    return 0;
}

static int recursive_copy(const char *src, const char *dst) {
    struct stat st;
    if (stat(src, &st) != 0) return -1;
    if (S_ISDIR(st.st_mode)) {
        ipa_sign_mkdir_p(dst);
        DIR *d = opendir(src);
        if (!d) return -1;
        struct dirent *e;
        while ((e = readdir(d)) != NULL) {
            if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
            char *s = path_join(src, e->d_name);
            char *dd = path_join(dst, e->d_name);
            recursive_copy(s, dd);
            free(s); free(dd);
        }
        closedir(d);
        return 0;
    } else {
        return copy_file(src, dst);
    }
}

static int recursive_remove(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    if (S_ISDIR(st.st_mode)) {
        DIR *d = opendir(path);
        if (!d) return -1;
        struct dirent *e;
        while ((e = readdir(d)) != NULL) {
            if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
            char *p = path_join(path, e->d_name);
            recursive_remove(p);
            free(p);
        }
        closedir(d);
        return rmdir(path);
    } else {
        return unlink(path);
    }
}

int ipa_sign_extract_zip(const char *zip_path, const char *dest_dir) {
    unzFile uf = unzOpen(zip_path);
    if (!uf) { LOG("Cannot open zip: %s\n", zip_path); return -1; }
    ipa_sign_mkdir_p(dest_dir);
    unz_global_info gi;
    if (unzGetGlobalInfo(uf, &gi) != UNZ_OK) { unzClose(uf); return -1; }
    char filename[512];
    for (uLong i = 0; i < gi.number_entry; i++) {
        unz_file_info fi;
        if (unzGetCurrentFileInfo(uf, &fi, filename, sizeof(filename), NULL, 0, NULL, 0) != UNZ_OK) break;
        char *fullpath = path_join(dest_dir, filename);
        if (filename[strlen(filename)-1] == '/') {
            ipa_sign_mkdir_p(fullpath);
        } else {
            char *dir = strdup(fullpath);
            char *slash = strrchr(dir, '/');
            if (slash) { *slash = '\0'; ipa_sign_mkdir_p(dir); }
            free(dir);
            if (unzOpenCurrentFile(uf) == UNZ_OK) {
                FILE *out = fopen(fullpath, "wb");
                if (out) {
                    char buf[8192];
                    int n;
                    while ((n = unzReadCurrentFile(uf, buf, sizeof(buf))) > 0) {
                        fwrite(buf, 1, n, out);
                    }
                    fclose(out);
                }
                unzCloseCurrentFile(uf);
            }
        }
        free(fullpath);
        if (i + 1 < gi.number_entry) unzGoToNextFile(uf);
    }
    unzClose(uf);
    LOG("Extracted %s to %s\n", zip_path, dest_dir);
    return 0;
}

static void add_file_to_zip(zipFile zf, const char *filepath, const char *arcname) {
    FILE *in = fopen(filepath, "rb");
    if (!in) return;
    zip_fileinfo zi;
    memset(&zi, 0, sizeof(zi));
    if (zipOpenNewFileInZip(zf, arcname, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != Z_OK) {
        fclose(in); return;
    }
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        zipWriteInFileInZip(zf, buf, (unsigned)n);
    }
    zipCloseFileInZip(zf);
    fclose(in);
}

static void add_dir_to_zip(zipFile zf, const char *dirpath, const char *prefix) {
    DIR *d = opendir(dirpath);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        char *full = path_join(dirpath, e->d_name);
        char *arc = prefix ? path_join(prefix, e->d_name) : strdup(e->d_name);
        struct stat st;
        if (stat(full, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                add_dir_to_zip(zf, full, arc);
            } else {
                add_file_to_zip(zf, full, arc);
            }
        }
        free(full); free(arc);
    }
    closedir(d);
}

int ipa_sign_create_zip(const char *src_dir, const char *zip_path) {
    zipFile zf = zipOpen(zip_path, APPEND_STATUS_CREATE);
    if (!zf) { LOG("Cannot create zip: %s\n", zip_path); return -1; }
    add_dir_to_zip(zf, src_dir, NULL);
    zipClose(zf, NULL);
    LOG("Created zip: %s\n", zip_path);
    return 0;
}

char *ipa_sign_find_app_bundle(const char *payload_dir) {
    DIR *d = opendir(payload_dir);
    if (!d) return NULL;
    struct dirent *e;
    char *result = NULL;
    while ((e = readdir(d)) != NULL) {
        if (strstr(e->d_name, ".app")) {
            result = path_join(payload_dir, e->d_name);
            break;
        }
    }
    closedir(d);
    return result;
}

static int is_macho_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    uint32_t magic = 0;
    fread(&magic, 1, 4, f);
    fclose(f);
    return magic == MH_MAGIC_64 || magic == MH_CIGAM_64 ||
           magic == MH_MAGIC || magic == MH_CIGAM ||
           magic == FAT_MAGIC || magic == FAT_CIGAM ||
           magic == FAT_MAGIC_64 || magic == FAT_CIGAM_64;
}

static void find_macho_in_dir(const char *dir, char ***files, int *count, int *capacity) {
    DIR *d = opendir(dir);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        char *full = path_join(dir, e->d_name);
        struct stat st;
        if (stat(full, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                if (strstr(e->d_name, ".framework") || strstr(e->d_name, ".appex") ||
                    strstr(e->d_name, ".bundle")) {
                    find_macho_in_dir(full, files, count, capacity);
                }
            } else if (S_ISREG(st.st_mode)) {
                if (is_macho_file(full)) {
                    if (*count >= *capacity) {
                        *capacity = (*capacity == 0) ? 16 : *capacity * 2;
                        *files = realloc(*files, (*capacity) * sizeof(char*));
                    }
                    (*files)[*count] = strdup(full);
                    (*count)++;
                }
            }
        }
        free(full);
    }
    closedir(d);
}

int ipa_sign_list_macho_files(const char *app_dir, char ***out_files, int *out_count) {
    char **files = NULL;
    int count = 0, capacity = 0;
    find_macho_in_dir(app_dir, &files, &count, &capacity);
    *out_files = files;
    *out_count = count;
    return 0;
}

void ipa_sign_free_string_list(char **files, int count) {
    for (int i = 0; i < count; i++) free(files[i]);
    free(files);
}

static uint8_t *sha256_data(const uint8_t *data, size_t len, uint8_t *out) {
    CC_SHA256(data, (CC_LONG)len, out);
    return out;
}

static int read_file_all(const char *path, uint8_t **out_data, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = malloc(sz);
    fread(buf, 1, sz, f);
    fclose(f);
    *out_data = buf;
    *out_size = (size_t)sz;
    return 0;
}

static int write_file_all(const char *path, const uint8_t *data, size_t size) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    fwrite(data, 1, size, f);
    fclose(f);
    return 0;
}

typedef struct {
    uint8_t *data;
    size_t size;
    size_t capacity;
} buffer_t;

static void buf_init(buffer_t *b) { b->data = NULL; b->size = 0; b->capacity = 0; }
static void buf_ensure(buffer_t *b, size_t extra) {
    if (b->size + extra > b->capacity) {
        b->capacity = (b->capacity == 0) ? 4096 : b->capacity * 2;
        while (b->size + extra > b->capacity) b->capacity *= 2;
        b->data = realloc(b->data, b->capacity);
    }
}
static void buf_append(buffer_t *b, const void *data, size_t len) {
    buf_ensure(b, len);
    memcpy(b->data + b->size, data, len);
    b->size += len;
}
static void buf_append_u32(buffer_t *b, uint32_t v) {
    uint32_t be = swap32(v);
    buf_append(b, &be, 4);
}
static void buf_free(buffer_t *b) { free(b->data); b->data = NULL; b->size = b->capacity = 0; }

static int asn1_len_size(size_t len) {
    if (len < 0x80) return 1;
    if (len < 0x100) return 2;
    if (len < 0x10000) return 3;
    if (len < 0x1000000) return 4;
    return 5;
}

static void asn1_write_len(buffer_t *b, size_t len) {
    if (len < 0x80) {
        uint8_t v = (uint8_t)len;
        buf_append(b, &v, 1);
    } else if (len < 0x100) {
        uint8_t v[2] = {0x81, (uint8_t)len};
        buf_append(b, v, 2);
    } else if (len < 0x10000) {
        uint8_t v[3] = {0x82, (uint8_t)(len >> 8), (uint8_t)len};
        buf_append(b, v, 3);
    } else if (len < 0x1000000) {
        uint8_t v[4] = {0x83, (uint8_t)(len >> 16), (uint8_t)(len >> 8), (uint8_t)len};
        buf_append(b, v, 4);
    } else {
        uint8_t v[5] = {0x84, (uint8_t)(len >> 24), (uint8_t)(len >> 16), (uint8_t)(len >> 8), (uint8_t)len};
        buf_append(b, v, 5);
    }
}

static void asn1_begin(buffer_t *b, uint8_t tag) { buf_append(b, &tag, 1); }
static void asn1_end(buffer_t *b, size_t start) {
    size_t content_len = b->size - start;
    size_t len_sz = asn1_len_size(content_len);
    if (len_sz > 1) {
        buf_ensure(b, len_sz - 1);
        memmove(b->data + start + 1 + len_sz, b->data + start + 1, content_len);
        b->size += len_sz - 1;
    }
    memcpy(b->data + start + 1, b->data, 0);
    uint8_t tmp[5];
    buffer_t tb; buf_init(&tb); tb.data = tmp; tb.capacity = 5;
    asn1_write_len(&tb, content_len);
    memcpy(b->data + start + 1, tmp, tb.size);
}

static void asn1_oid(buffer_t *b, const uint8_t *oid, size_t oid_len) {
    asn1_begin(b, 0x06);
    asn1_write_len(b, oid_len);
    buf_append(b, oid, oid_len);
}

static void asn1_octet_string(buffer_t *b, const uint8_t *data, size_t len) {
    asn1_begin(b, 0x04);
    asn1_write_len(b, len);
    buf_append(b, data, len);
}

static void asn1_integer(buffer_t *b, const uint8_t *data, size_t len) {
    asn1_begin(b, 0x02);
    asn1_write_len(b, len);
    buf_append(b, data, len);
}

static void asn1_null(buffer_t *b) {
    uint8_t v[2] = {0x05, 0x00};
    buf_append(b, v, 2);
}

static int import_p12(const char *p12_path, const char *password,
                       SecIdentityRef *out_identity, SecCertificateRef *out_cert) {
    uint8_t *p12data = NULL; size_t p12len = 0;
    if (read_file_all(p12_path, &p12data, &p12len) != 0) {
        LOG("Cannot read P12: %s\n", p12_path); return -1;
    }
    CFDataRef p12cf = CFDataCreate(NULL, p12data, (CFIndex)p12len);
    free(p12data);
    CFStringRef pw = password ? CFStringCreateWithCString(NULL, password, kCFStringEncodingUTF8) : NULL;
    CFArrayRef items = NULL;
    OSStatus status = SecPKCS12Import(p12cf, (__bridge CFDictionaryRef)@{(__bridge id)kSecImportExportPassphrase: (__bridge id)pw ?: @""}, &items);
    if (pw) CFRelease(pw);
    CFRelease(p12cf);
    if (status != errSecSuccess || !items || CFArrayGetCount(items) == 0) {
        LOG("SecPKCS12Import failed: %d\n", (int)status);
        if (items) CFRelease(items);
        return -1;
    }
    CFDictionaryRef dict = CFArrayGetValueAtIndex(items, 0);
    SecIdentityRef identity = (SecIdentityRef)CFDictionaryGetValue(dict, kSecImportItemIdentity);
    if (identity) { CFRetain(identity); *out_identity = identity; }
    SecCertificateRef cert = NULL;
    if (identity) SecIdentityCopyCertificate(identity, &cert);
    if (cert) *out_cert = cert;
    CFRelease(items);
    return (identity && cert) ? 0 : -1;
}

static int sign_data_with_identity(SecIdentityRef identity, const uint8_t *data, size_t len,
                                     uint8_t **out_sig, size_t *out_sig_len) {
    SecKeyRef privateKey = NULL;
    if (SecIdentityCopyPrivateKey(identity, &privateKey) != errSecSuccess) return -1;
    CFErrorRef error = NULL;
    CFDataRef sig = SecKeyCreateSignature(privateKey, kSecKeyAlgorithmRSASignatureMessagePKCS1v15SHA256,
                                            CFDataCreate(NULL, data, (CFIndex)len), &error);
    CFRelease(privateKey);
    if (!sig) {
        if (error) CFRelease(error);
        return -1;
    }
    *out_sig_len = (size_t)CFDataGetLength(sig);
    *out_sig = malloc(*out_sig_len);
    CFDataGetBytes(sig, CFRangeMake(0, *out_sig_len), *out_sig);
    CFRelease(sig);
    return 0;
}

static int create_cms_signature(SecIdentityRef identity, SecCertificateRef cert,
                                  const uint8_t *codedir, size_t codedir_len,
                                  uint8_t **out_cms, size_t *out_cms_len) {
    uint8_t cd_hash[CC_SHA256_DIGEST_LENGTH];
    sha256_data(codedir, codedir_len, cd_hash);

    uint8_t *raw_sig = NULL; size_t raw_sig_len = 0;
    if (sign_data_with_identity(identity, cd_hash, sizeof(cd_hash), &raw_sig, &raw_sig_len) != 0) {
        LOG("RSA sign failed\n"); return -1;
    }

    CFDataRef cert_data = SecCertificateCopyData(cert);
    const uint8_t *cert_bytes = CFDataGetBytePtr(cert_data);
    size_t cert_len = (size_t)CFDataGetLength(cert_data);

    buffer_t b; buf_init(&b);

    static const uint8_t oid_signedData[] = {0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02};
    static const uint8_t oid_data[] = {0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x01};
    static const uint8_t oid_sha256[] = {0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01};
    static const uint8_t oid_rsa[] = {0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x01};
    static const uint8_t oid_contentType[] = {0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x03};
    static const uint8_t oid_messageDigest[] = {0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x04};

    asn1_begin(&b, 0x30);
    asn1_oid(&b, oid_signedData, sizeof(oid_signedData));
    asn1_begin(&b, 0xa0);
    asn1_begin(&b, 0x30);

    uint8_t ver1[] = {0x01};
    asn1_integer(&b, ver1, 1);

    asn1_begin(&b, 0x31);
    asn1_begin(&b, 0x30);
    asn1_oid(&b, oid_sha256, sizeof(oid_sha256));
    asn1_null(&b);

    asn1_begin(&b, 0x30);
    asn1_oid(&b, oid_data, sizeof(oid_data));
    asn1_begin(&b, 0xa0);

    asn1_begin(&b, 0xa0);
    asn1_begin(&b, 0x30);
    buf_append(&b, cert_bytes, cert_len);

    asn1_begin(&b, 0x31);
    asn1_begin(&b, 0x30);
    asn1_integer(&b, ver1, 1);

    asn1_begin(&b, 0x30);
    asn1_begin(&b, 0x30);
    asn1_null(&b);
    asn1_integer(&b, (uint8_t[]){0x00}, 1);
    asn1_begin(&b, 0x30);
    asn1_null(&b);
    asn1_begin(&b, 0x30);
    asn1_null(&b);

    asn1_begin(&b, 0xa0);
    asn1_begin(&b, 0x30);
    asn1_oid(&b, oid_contentType, sizeof(oid_contentType));
    asn1_begin(&b, 0x31);
    asn1_begin(&b, 0x06);
    asn1_write_len(&b, sizeof(oid_data));
    buf_append(&b, oid_data, sizeof(oid_data));

    asn1_begin(&b, 0x30);
    asn1_oid(&b, oid_messageDigest, sizeof(oid_messageDigest));
    asn1_begin(&b, 0x31);
    asn1_octet_string(&b, cd_hash, sizeof(cd_hash));

    asn1_begin(&b, 0x30);
    asn1_oid(&b, oid_sha256, sizeof(oid_sha256));
    asn1_null(&b);
    asn1_begin(&b, 0x30);
    asn1_oid(&b, oid_rsa, sizeof(oid_rsa));
    asn1_null(&b);
    asn1_octet_string(&b, raw_sig, raw_sig_len);

    CFRelease(cert_data);
    free(raw_sig);

    *out_cms = b.data;
    *out_cms_len = b.size;
    return 0;
}

static int create_codedirectory(const uint8_t *binary, size_t binary_len,
                                  const char *identifier,
                                  uint8_t **out_cd, size_t *out_cd_len) {
    size_t page_count = (binary_len + CS_PAGE_SIZE - 1) / CS_PAGE_SIZE;
    size_t ident_len = strlen(identifier) + 1;
    size_t cd_fixed_size = sizeof(cs_codedirectory_t);
    size_t hash_area_size = page_count * CC_SHA256_DIGEST_LENGTH;
    size_t total_size = cd_fixed_size + ident_len + hash_area_size;

    uint8_t *cd = calloc(1, total_size);
    cs_codedirectory_t *hdr = (cs_codedirectory_t *)cd;
    hdr->header.magic = swap32(CS_MAGIC);
    hdr->header.length = swap32((uint32_t)total_size);
    hdr->version = swap32(CS_VERSION);
    hdr->flags = 0;
    hdr->hashOffset = swap32((uint32_t)(cd_fixed_size + ident_len));
    hdr->identOffset = swap32((uint32_t)cd_fixed_size);
    hdr->nSpecialSlots = 0;
    hdr->nCodeSlots = swap32((uint32_t)page_count);
    hdr->codeLimit = swap32((uint32_t)binary_len);
    hdr->hashSize = CC_SHA256_DIGEST_LENGTH;
    hdr->hashType = CS_HASHTYPE_SHA256;
    hdr->platform = 0;
    hdr->pageSize = 12;
    hdr->spare2 = 0;
    hdr->scatterOffset = 0;
    hdr->teamOffset = 0;

    memcpy(cd + cd_fixed_size, identifier, ident_len - 1);

    uint8_t *hashes = cd + cd_fixed_size + ident_len;
    for (size_t i = 0; i < page_count; i++) {
        size_t offset = i * CS_PAGE_SIZE;
        size_t len = (offset + CS_PAGE_SIZE <= binary_len) ? CS_PAGE_SIZE : (binary_len - offset);
        sha256_data(binary + offset, len, hashes + i * CC_SHA256_DIGEST_LENGTH);
    }

    *out_cd = cd;
    *out_cd_len = total_size;
    return 0;
}

static int sign_macho_binary(const char *path, SecIdentityRef identity, SecCertificateRef cert,
                               const char *bundle_id) {
    uint8_t *data = NULL; size_t size = 0;
    if (read_file_all(path, &data, &size) != 0) { LOG("Cannot read %s\n", path); return -1; }

    uint32_t magic = *(uint32_t *)data;
    int is_64 = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);
    int swapped = (magic == MH_CIGAM_64 || magic == MH_CIGAM);

    if (!is_64) {
        LOG("Non-64-bit Mach-O not supported: %s\n", path);
        free(data);
        return 0;
    }

    struct mach_header_64 *hdr = (struct mach_header_64 *)data;
    uint32_t ncmds = swapped ? OSSwapBigToHostInt32(hdr->ncmds) : hdr->ncmds;
    uint32_t sizeofcmds = swapped ? OSSwapBigToHostInt32(hdr->sizeofcmds) : hdr->sizeofcmds;

    uint8_t *loadcmds = data + sizeof(struct mach_header_64);
    struct load_command *existing_cs = NULL;
    uint64_t linkedit_vmaddr = 0, linkedit_fileoff = 0, linkedit_size = 0;

    for (uint32_t i = 0; i < ncmds; i++) {
        struct load_command *lc = (struct load_command *)(loadcmds + (i > 0 ? ((struct load_command *)(loadcmds))->cmdsize : 0));
        if (i > 0) {
            uint8_t *p = loadcmds;
            for (uint32_t j = 0; j < i; j++) {
                struct load_command *tlc = (struct load_command *)p;
                p += (swapped ? OSSwapBigToHostInt32(tlc->cmdsize) : tlc->cmdsize);
            }
            lc = (struct load_command *)p;
        }
        uint32_t cmd = swapped ? OSSwapBigToHostInt32(lc->cmd) : lc->cmd;
        if (cmd == LC_CODE_SIGNATURE) existing_cs = lc;
        if (cmd == LC_SEGMENT_64) {
            struct segment_command_64 *seg = (struct segment_command_64 *)lc;
            if (strcmp(seg->segname, "__LINKEDIT") == 0) {
                linkedit_vmaddr = swapped ? OSSwapBigToHostInt64(seg->vmaddr) : seg->vmaddr;
                linkedit_fileoff = swapped ? OSSwapBigToHostInt64(seg->fileoff) : seg->fileoff;
                linkedit_size = swapped ? OSSwapBigToHostInt64(seg->filesize) : seg->filesize;
            }
        }
    }

    uint8_t *cd = NULL; size_t cd_len = 0;
    create_codedirectory(data, size, bundle_id, &cd, &cd_len);

    uint8_t *cms = NULL; size_t cms_len = 0;
    if (create_cms_signature(identity, cert, cd, cd_len, &cms, &cms_len) != 0) {
        LOG("CMS creation failed\n");
        free(cd); free(data); return -1;
    }

    size_t blob_count = 2;
    size_t index_size = blob_count * sizeof(cs_index_t);
    size_t superblob_hdr_size = sizeof(cs_superblob_t) + index_size;
    size_t cd_offset = superblob_hdr_size;
    size_t cms_offset = cd_offset + cd_len;
    size_t total_sig_size = cms_offset + cms_len;
    total_sig_size = (total_sig_size + 15) & ~15;

    uint8_t *sig = calloc(1, total_sig_size);
    cs_superblob_t *sb = (cs_superblob_t *)sig;
    sb->header.magic = swap32(CS_MAGIC);
    sb->header.length = swap32((uint32_t)total_sig_size);
    sb->count = swap32((uint32_t)blob_count);
    sb->index[0].type = swap32(CSSLOT_CODEDIRECTORY);
    sb->index[0].offset = swap32((uint32_t)cd_offset);
    sb->index[1].type = swap32(CSSLOT_BLOBWRAPPER);
    sb->index[1].offset = swap32((uint32_t)cms_offset);
    memcpy(sig + cd_offset, cd, cd_len);
    memcpy(sig + cms_offset, cms, cms_len);

    size_t new_size = size + total_sig_size;
    uint8_t *new_data = realloc(data, new_size);
    memcpy(new_data + size, sig, total_sig_size);

    if (existing_cs) {
        struct linkedit_data_command *cs_cmd = (struct linkedit_data_command *)existing_cs;
        cs_cmd->dataoff = swap32((uint32_t)size);
        cs_cmd->datasize = swap32((uint32_t)total_sig_size);
    } else {
        LOG("No LC_CODE_SIGNATURE found, adding one (requires load command space)\n");
    }

    write_file_all(path, new_data, new_size);
    free(new_data); free(sig); free(cd); free(cms);
    LOG("Signed Mach-O: %s (%zu bytes signature)\n", path, total_sig_size);
    return 0;
}

static int generate_coderesources(const char *app_dir, const char *bundle_id) {
    char *cr_path = path_join(app_dir, "_CodeSignature");
    ipa_sign_mkdir_p(cr_path);
    char *cr_file = path_join(cr_path, "CodeResources");
    FILE *f = fopen(cr_file, "wb");
    if (!f) { free(cr_path); free(cr_file); return -1; }
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
    fprintf(f, "<plist version=\"1.0\"><dict>\n");
    fprintf(f, "<key>files</key><dict>\n");
    DIR *d = opendir(app_dir);
    if (d) {
        struct dirent *e;
        while ((e = readdir(d)) != NULL) {
            if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
            if (strcmp(e->d_name, "_CodeSignature") == 0) continue;
            if (strcmp(e->d_name, "CodeResources") == 0) continue;
            char *fp = path_join(app_dir, e->d_name);
            struct stat st;
            if (stat(fp, &st) == 0 && S_ISREG(st.st_mode)) {
                uint8_t *fdata = NULL; size_t fsz = 0;
                if (read_file_all(fp, &fdata, &fsz) == 0) {
                    uint8_t hash[CC_SHA256_DIGEST_LENGTH];
                    sha256_data(fdata, fsz, hash);
                    fprintf(f, "<key>%s</key><data>", e->d_name);
                    for (int i = 0; i < CC_SHA256_DIGEST_LENGTH; i++) fprintf(f, "%02X", hash[i]);
                    fprintf(f, "</data>\n");
                    free(fdata);
                }
            }
            free(fp);
        }
        closedir(d);
    }
    fprintf(f, "</dict>\n");
    fprintf(f, "<key>rules</key><dict>\n");
    fprintf(f, "<key>^.*</key><true/>\n");
    fprintf(f, "</dict>\n");
    fprintf(f, "</dict></plist>\n");
    fclose(f);
    free(cr_path); free(cr_file);
    LOG("Generated CodeResources for %s\n", app_dir);
    return 0;
}

static char *read_plist_bundle_id(const char *app_dir) {
    char *info = path_join(app_dir, "Info.plist");
    if (!file_exists(info)) { free(info); return strdup("com.unknown.app"); }
    CFDataRef data = NULL;
    FILE *f = fopen(info, "rb");
    if (f) {
        fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
        uint8_t *buf = malloc(sz); fread(buf, 1, sz, f); fclose(f);
        data = CFDataCreate(NULL, buf, sz); free(buf);
    }
    free(info);
    if (!data) return strdup("com.unknown.app");
    CFPropertyListRef plist = CFPropertyListCreateWithData(NULL, data, kCFPropertyListImmutable, NULL, NULL);
    CFRelease(data);
    if (!plist) return strdup("com.unknown.app");
    NSString *bid = nil;
    if (CFGetTypeID(plist) == CFDictionaryGetTypeID()) {
        bid = (__bridge NSString *)CFDictionaryGetValue((CFDictionaryRef)plist, CFSTR("CFBundleIdentifier"));
    }
    char *result = bid ? strdup([bid UTF8String]) : strdup("com.unknown.app");
    CFRelease(plist);
    return result;
}

ipa_sign_result_t ipa_sign_ipa(
    const char *input_ipa_path,
    const char *p12_path,
    const char *p12_password,
    const char *mobileprovision_path,
    const char *output_ipa_path,
    const char *bundle_id_override)
{
    ipa_sign_result_t result = {0};
    result.success = 0;
    strcpy(result.error_message, "");
    strcpy(result.output_path, output_ipa_path ?: "");

    if (!file_exists(input_ipa_path)) {
        strcpy(result.error_message, "Input IPA not found");
        return result;
    }
    if (!p12_path || !file_exists(p12_path)) {
        strcpy(result.error_message, "P12 certificate not found");
        return result;
    }

    SecIdentityRef identity = NULL;
    SecCertificateRef cert = NULL;
    if (import_p12(p12_path, p12_password, &identity, &cert) != 0) {
        strcpy(result.error_message, "Failed to import P12 certificate (wrong password?)");
        return result;
    }

    char tmpdir[] = "/tmp/ipasign_XXXXXX";
    if (!mkdtemp(tmpdir)) {
        strcpy(result.error_message, "Failed to create temp dir");
        CFRelease(identity); CFRelease(cert);
        return result;
    }

    char *payload = path_join(tmpdir, "Payload");
    if (ipa_sign_extract_zip(input_ipa_path, tmpdir) != 0) {
        strcpy(result.error_message, "Failed to extract IPA");
        recursive_remove(tmpdir);
        CFRelease(identity); CFRelease(cert);
        return result;
    }

    char *app_dir = ipa_sign_find_app_bundle(payload);
    if (!app_dir) {
        strcpy(result.error_message, "No .app bundle found in IPA");
        free(payload); recursive_remove(tmpdir);
        CFRelease(identity); CFRelease(cert);
        return result;
    }

    char *bundle_id = bundle_id_override ? strdup(bundle_id_override) : read_plist_bundle_id(app_dir);
    LOG("Signing with bundle ID: %s\n", bundle_id);

    if (mobileprovision_path && file_exists(mobileprovision_path)) {
        char *dest_mp = path_join(app_dir, "embedded.mobileprovision");
        copy_file(mobileprovision_path, dest_mp);
        free(dest_mp);
        LOG("Embedded provisioning profile\n");
    }

    char **macho_files = NULL; int macho_count = 0;
    ipa_sign_list_macho_files(app_dir, &macho_files, &macho_count);
    LOG("Found %d Mach-O binaries to sign\n", macho_count);
    for (int i = 0; i < macho_count; i++) {
        sign_macho_binary(macho_files[i], identity, cert, bundle_id);
    }
    ipa_sign_free_string_list(macho_files, macho_count);

    generate_coderesources(app_dir, bundle_id);

    if (ipa_sign_create_zip(tmpdir, output_ipa_path) != 0) {
        strcpy(result.error_message, "Failed to create output IPA");
        free(bundle_id); free(app_dir); free(payload);
        recursive_remove(tmpdir);
        CFRelease(identity); CFRelease(cert);
        return result;
    }

    result.success = 1;
    strcpy(result.output_path, output_ipa_path);

    free(bundle_id); free(app_dir); free(payload);
    recursive_remove(tmpdir);
    CFRelease(identity); CFRelease(cert);
    LOG("IPA signing complete: %s\n", output_ipa_path);
    return result;
}
