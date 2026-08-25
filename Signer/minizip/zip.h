#ifndef _MINIZIP_H
#define _MINIZIP_H
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long uLong;
typedef unsigned int uInt;

typedef struct tm_zip_s {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
} tm_zip;

typedef struct {
    tm_zip tmz_date;
    uint32_t dosDate;
    uint32_t internalFa;
    uint32_t externalFa;
} zip_fileinfo;

typedef struct {
    uint32_t version;
    uint32_t version_needed;
    uint32_t flag;
    uint32_t compression_method;
    uint32_t dosDate;
    uint32_t crc;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint32_t size_filename;
    uint32_t size_file_extra;
    uint32_t size_file_comment;
    uint32_t disk_num_start;
    uint32_t internal_fa;
    uint32_t external_fa;
    tm_zip tmu_date;
} unz_file_info;

typedef struct {
    uLong number_entry;
} unz_global_info;

typedef void* zipFile;
typedef void* unzFile;

#define ZIP_OK 0
#define ZIP_ERRNO -1
#define ZIP_PARAMERROR -102
#define ZIP_BADZIPFILE -103
#define ZIP_INTERNALERROR -104
#define ZIP_END 50

#define UNZ_OK 0
#define UNZ_END 50
#define UNZ_ERRNO -1
#define UNZ_PARAMERROR -102
#define UNZ_BADZIPFILE -103
#define UNZ_INTERNALERROR -104
#define UNZ_CRCERROR -105

#define APPEND_STATUS_CREATE 0
#define APPEND_STATUS_CREATEAFTER 1
#define APPEND_STATUS_ADDINZIP 2

/* zip write API */
zipFile zipOpen(const char *pathname, int append);
int zipOpenNewFileInZip(zipFile file, const char *filename,
                         const zip_fileinfo *zipfi,
                         const void *extrafield_local, uInt size_extrafield_local,
                         const void *extrafield_global, uInt size_extrafield_global,
                         const char *comment, int method, int level);
int zipWriteInFileInZip(zipFile file, const void *buf, uint32_t len);
int zipCloseFileInZip(zipFile file);
int zipClose(zipFile file, const char *global_comment);

/* unzip read API */
unzFile unzOpen(const char *path);
int unzGetGlobalInfo(unzFile file, unz_global_info *pglobal_info);
int unzGoToFirstFile(unzFile file);
int unzGoToNextFile(unzFile file);
int unzGetCurrentFileInfo(unzFile file, unz_file_info *pfile_info,
                           char *filename, uLong filename_size,
                           void *extrafield, uLong extrafield_size,
                           char *comment, uLong comment_size);
int unzOpenCurrentFile(unzFile file);
int unzReadCurrentFile(unzFile file, void *buf, uint32_t len);
int unzCloseCurrentFile(unzFile file);
int unzClose(unzFile file);

#ifdef __cplusplus
}
#endif
#endif
