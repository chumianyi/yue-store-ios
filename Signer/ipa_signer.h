#ifndef IPA_SIGNER_H
#define IPA_SIGNER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int success;
    char error_message[512];
    char output_path[1024];
} ipa_sign_result_t;

ipa_sign_result_t ipa_sign_ipa(
    const char *input_ipa_path,
    const char *p12_path,
    const char *p12_password,
    const char *mobileprovision_path,
    const char *output_ipa_path,
    const char *bundle_id_override
);

int ipa_sign_extract_zip(const char *zip_path, const char *dest_dir);
int ipa_sign_create_zip(const char *src_dir, const char *zip_path);
int ipa_sign_file_exists(const char *path);
int ipa_sign_mkdir_p(const char *path);
char *ipa_sign_find_app_bundle(const char *payload_dir);
int ipa_sign_list_macho_files(const char *app_dir, char ***out_files, int *out_count);
void ipa_sign_free_string_list(char **files, int count);

#ifdef __cplusplus
}
#endif

#endif
