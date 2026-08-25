#ifndef _ZLIB_H
#include <zlib.h>
#endif

#ifndef _zip64_H
#define _zip64_H

#define ZIP_BZIP2 0

#define APPEND_STATUS_CREATE    0
#define APPEND_STATUS_CREATEAFTER 1
#define APPEND_STATUS_ADDINZIP 2

#ifndef OF
#define OF(args) args
#endif

typedef voidp zipFile;

#define ZIP_OK                                  (0)
#define ZIP_EOF                                  (0)
#define ZIP_ERRNO                               (Z_ERRNO)
#define ZIP_PARAMERROR                          (-102)
#define ZIP_BADZIPFILE                          (-103)
#define ZIP_INTERNALERROR                       (-104)

#ifndef DEF_MEM_LEVEL
#  if MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  else
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#endif

typedef struct
{
    tm_zip      tmz_date;
    uLong       dosDate;
    uLong       internalFa;
    uLong       externalFa;
} zip_fileinfo;

extern zipFile zipOpen OF((const char *pathname, int append));
extern zipFile zipOpen64 OF((const void *pathname, int append));
extern int zipOpenNewFileInZip OF((zipFile file, const char *filename, const zip_fileinfo *zipfi,
                                    const void *extrafield_local, uInt size_extrafield_local,
                                    const void *extrafield_global, uInt size_extrafield_global,
                                    const char *comment, int method, int level));
extern int zipWriteInFileInZip OF((zipFile file, const void *buf, unsigned len));
extern int zipCloseFileInZip OF((zipFile file));
extern int zipClose OF((zipFile file, const char *global_comment));

#endif
