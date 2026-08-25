#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ioapi.h"

static voidpf call_zopen OF((voidpf opaque, const char *filename, int mode)) {
    FILE *file = NULL;
    const char *mode_fopen = NULL;
    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_READ) mode_fopen = "rb";
    else if (mode & ZLIB_FILEFUNC_MODE_EXISTING) mode_fopen = "r+b";
    else if (mode & ZLIB_FILEFUNC_MODE_CREATE) mode_fopen = "wb";
    if ((filename != NULL) && (mode_fopen != NULL)) file = fopen(filename, mode_fopen);
    return file;
}

static uLong call_zread OF((voidpf opaque, voidpf stream, void *buf, uLong size)) {
    uLong ret = fread(buf, 1, size, (FILE *)stream);
    return ret;
}

static uLong call_zwrite OF((voidpf opaque, voidpf stream, const void *buf, uLong size)) {
    uLong ret = fwrite(buf, 1, size, (FILE *)stream);
    return ret;
}

static long call_ztell OF((voidpf opaque, voidpf stream)) {
    return ftell((FILE *)stream);
}

static long call_zseek OF((voidpf opaque, voidpf stream, uLong offset, int origin)) {
    int fseek_origin = 0;
    switch (origin) {
        case ZLIB_FILEFUNC_SEEK_CUR: fseek_origin = SEEK_CUR; break;
        case ZLIB_FILEFUNC_SEEK_END: fseek_origin = SEEK_END; break;
        case ZLIB_FILEFUNC_SEEK_SET: fseek_origin = SEEK_SET; break;
        default: return -1;
    }
    return fseek((FILE *)stream, offset, fseek_origin);
}

static int call_zclose OF((voidpf opaque, voidpf stream)) {
    return fclose((FILE *)stream);
}

static int call_zerror OF((voidpf opaque, voidpf stream)) {
    return ferror((FILE *)stream);
}

void fill_fopen_filefunc (zlib_filefunc_def *pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = call_zopen;
    pzlib_filefunc_def->zread_file = call_zread;
    pzlib_filefunc_def->zwrite_file = call_zwrite;
    pzlib_filefunc_def->ztell_file = call_ztell;
    pzlib_filefunc_def->zseek_file = call_zseek;
    pzlib_filefunc_def->zclose_file = call_zclose;
    pzlib_filefunc_def->zerror_file = call_zerror;
    pzlib_filefunc_def->opaque = NULL;
}
