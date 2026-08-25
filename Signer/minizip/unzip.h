#ifndef _unz_H
#define _unz_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ZLIB_H
#include <zlib.h>
#endif

#ifndef _ZLIBIOAPI_H
#include "ioapi.h"
#endif

#define UNZ_OK                                  (0)
#define UNZ_END_OF_LIST_OF_FILE                 (-100)
#define UNZ_ERRNO                               (Z_ERRNO)
#define UNZ_EOF                                 (0)
#define UNZ_PARAMERROR                          (-102)
#define UNZ_BADZIPFILE                          (-103)
#define UNZ_INTERNALERROR                       (-104)
#define UNZ_CRCERROR                            (-105)

typedef int (*unzFileNameComparer) OF((const char *filename1, const char *filename2));

typedef struct unz_global_info_s {
    uLong number_entry;
    uLong size_comment;
} unz_global_info;

typedef struct unz_file_info_s {
    uLong version;
    uLong version_needed;
    uLong flag;
    uLong compression_method;
    uLong dosDate;
    uLong crc;
    uLong compressed_size;
    uLong uncompressed_size;
    uLong size_filename;
    uLong size_file_extra;
    uLong size_file_comment;
    uLong disk_num_start;
    uLong internal_fa;
    uLong external_fa;
    tm_zip tmu_date;
} unz_file_info;

extern voidp unzOpen OF((const char *path));
extern int unzClose OF((voidp file));
extern int unzGetGlobalInfo OF((voidp file, unz_global_info *pglobal_info));
extern int unzGetCurrentFileInfo OF((voidp file, unz_file_info *pfile_info,
                                       char *szFileName, uLong fileNameBufferSize,
                                       void *extraField, uLong extraFieldBufferSize,
                                       char *szComment, uLong commentBufferSize));
extern int unzOpenCurrentFile OF((voidp file));
extern int unzReadCurrentFile OF((voidp file, voidp buf, unsigned len));
extern int unzCloseCurrentFile OF((voidp file));
extern int unzGoToNextFile OF((voidp file));

#ifdef __cplusplus
}
#endif

#endif
