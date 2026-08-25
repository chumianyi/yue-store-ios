#include "ioapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static voidpf fopen_open(voidpf opaque, const char *filename, int mode) {
    (void)opaque;
    const char *mode_fopen = "rb";
    if ((mode & 2) && (mode & 8)) mode_fopen = "wb+";
    else if (mode & 2) mode_fopen = "rb+";
    else if (mode & 8) mode_fopen = "wb";
    return (voidpf)fopen(filename, mode_fopen);
}
static uLong fopen_read(voidpf opaque, voidpf stream, void *buf, uLong size) {
    (void)opaque;
    return (uLong)fread(buf, 1, (size_t)size, (FILE *)stream);
}
static uLong fopen_write(voidpf opaque, voidpf stream, const void *buf, uLong size) {
    (void)opaque;
    return (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
}
static long fopen_tell(voidpf opaque, voidpf stream) { (void)opaque; return ftell((FILE *)stream); }
static long fopen_seek(voidpf opaque, voidpf stream, uLong offset, int origin) {
    (void)opaque;
    int fseek_origin = SEEK_SET;
    if (origin == 1) fseek_origin = SEEK_CUR;
    else if (origin == 2) fseek_origin = SEEK_END;
    return fseek((FILE *)stream, (long)offset, fseek_origin);
}
static int fopen_close(voidpf opaque, voidpf stream) { (void)opaque; return fclose((FILE *)stream); }
static int fopen_error(voidpf opaque, voidpf stream) { (void)opaque; (void)stream; return 0; }
void fill_fopen_filefunc(zlib_filefunc_def *pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_open;
    pzlib_filefunc_def->zread_file = fopen_read;
    pzlib_filefunc_def->zwrite_file = fopen_write;
    pzlib_filefunc_def->ztell_file = fopen_tell;
    pzlib_filefunc_def->zseek_file = fopen_seek;
    pzlib_filefunc_def->zclose_file = fopen_close;
    pzlib_filefunc_def->zerror_file = fopen_error;
    pzlib_filefunc_def->opaque = NULL;
}
