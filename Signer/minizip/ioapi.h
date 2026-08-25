#ifndef _ZLIBIOAPI_H
#define _ZLIBIOAPI_H
#include <stdio.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef voidpf (*open_file_func)(voidpf opaque, const char *filename, int mode);
typedef uint64_t (*read_file_func)(voidpf opaque, voidpf stream, void *buf, uint64_t size);
typedef uint64_t (*write_file_func)(voidpf opaque, voidpf stream, const void *buf, uint64_t size);
typedef long (*tell_file_func)(voidpf opaque, voidpf stream);
typedef long (*seek_file_func)(voidpf opaque, voidpf stream, uint64_t offset, int origin);
typedef int (*close_file_func)(voidpf opaque, voidpf stream);
typedef int (*testerror_file_func)(voidpf opaque, voidpf stream);
typedef struct zlib_filefunc_def_s {
    open_file_func zopen_file;
    read_file_func zread_file;
    write_file_func zwrite_file;
    tell_file_func ztell_file;
    seek_file_func zseek_file;
    close_file_func zclose_file;
    testerror_file_func zerror_file;
    voidpf opaque;
} zlib_filefunc_def;
void fill_fopen_filefunc(zlib_filefunc_def *pzlib_filefunc_def);
#ifdef __cplusplus
}
#endif
#endif
