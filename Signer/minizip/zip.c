#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "zip.h"
#include "ioapi.h"

#define LOCAL_HEADER_MAGIC 0x04034b50
#define CENTRAL_HEADER_MAGIC 0x02014b50
#define END_CENTRAL_MAGIC 0x06054b50
#define ZIP64_END_CENTRAL_MAGIC 0x06064b50
#define ZIP64_END_CENTRAL_LOCATOR_MAGIC 0x07064b50

#define VERSION_MADEBY 20
#define VERSION_NEEDED 20

typedef struct {
    zlib_filefunc_def filefunc;
    voidpf filestream;
    uLong base_seek;
    uLong byte_before_the_zipfile;
    uLong number_entry;
    time_t tm_zip;
    struct {
        uLong size_array;
        uLong gap;
        char *pData;
    } central_array;
} zip_internal;

static uLong zipInt64(uLong v) { return v; }

static void put_uint16(FILE *f, uInt v) {
    uInt16 b = (uInt16)v;
    fwrite(&b, 1, 2, f);
}
static void put_uint32(FILE *f, uLong v) {
    uInt32 b = (uInt32)v;
    fwrite(&b, 1, 4, f);
}

zipFile zipOpen(const char *pathname, int append) {
    zip_internal *zi;
    zlib_filefunc_def filefunc;
    fill_fopen_filefunc(&filefunc);
    voidpf filestream = ZOPEN(filefunc, pathname, ZLIB_FILEFUNC_MODE_CREATE | ZLIB_FILEFUNC_MODE_WRITE);
    if (!filestream) return NULL;
    zi = (zip_internal *)malloc(sizeof(zip_internal));
    if (!zi) { ZCLOSE(filefunc, filestream); return NULL; }
    memset(zi, 0, sizeof(zip_internal));
    zi->filefunc = filefunc;
    zi->filestream = filestream;
    zi->base_seek = 0;
    zi->byte_before_the_zipfile = 0;
    zi->number_entry = 0;
    zi->central_array.size_array = 16384;
    zi->central_array.pData = (char *)malloc(zi->central_array.size_array);
    zi->central_array.gap = 0;
    return (zipFile)zi;
}

typedef struct {
    int in_opened_file;
    z_stream stream;
    uLong crc;
    uLong uncompressed_size;
    uLong compressed_size;
    uLong pos_local_header;
    char filename[512];
    uInt filename_len;
    int method;
    int level;
    time_t tm_zip;
    uLong dosDate;
    uLong externalFa;
    uLong internalFa;
} zip_current_file;

static zip_current_file *get_current(zipFile file) {
    static zip_current_file cf;
    return &cf;
}

int zipOpenNewFileInZip(zipFile file, const char *filename, const zip_fileinfo *zipfi,
                          const void *extrafield_local, uInt size_extrafield_local,
                          const void *extrafield_global, uInt size_extrafield_global,
                          const char *comment, int method, int level) {
    zip_internal *zi = (zip_internal *)file;
    zip_current_file *cf = get_current(file);
    if (cf->in_opened_file) return ZIP_PARAMERROR;
    memset(cf, 0, sizeof(*cf));
    cf->in_opened_file = 1;
    cf->method = method;
    cf->level = level;
    strncpy(cf->filename, filename ?: "", sizeof(cf->filename)-1);
    cf->filename_len = (uInt)strlen(cf->filename);
    if (zipfi) {
        cf->dosDate = zipfi->dosDate;
        cf->externalFa = zipfi->externalFa;
        cf->internalFa = zipfi->internalFa;
    }
    cf->pos_local_header = ZTELL(zi->filefunc, zi->filestream);

    put_uint32((FILE *)zi->filestream, LOCAL_HEADER_MAGIC);
    put_uint16((FILE *)zi->filestream, VERSION_NEEDED);
    put_uint16((FILE *)zi->filestream, 0);
    put_uint16((FILE *)zi->filestream, (uInt)method);
    put_uint16((FILE *)zi->filestream, 0);
    put_uint16((FILE *)zi->filestream, 0);
    put_uint32((FILE *)zi->filestream, 0);
    put_uint32((FILE *)zi->filestream, 0);
    put_uint32((FILE *)zi->filestream, 0);
    put_uint16((FILE *)zi->filestream, cf->filename_len);
    put_uint16((FILE *)zi->filestream, size_extrafield_local);
    fwrite(cf->filename, 1, cf->filename_len, (FILE *)zi->filestream);
    if (extrafield_local && size_extrafield_local > 0)
        fwrite(extrafield_local, 1, size_extrafield_local, (FILE *)zi->filestream);

    if (method == Z_DEFLATED) {
        cf->stream.zalloc = Z_NULL;
        cf->stream.zfree = Z_NULL;
        cf->stream.opaque = Z_NULL;
        deflateInit2(&cf->stream, level, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);
    }
    cf->crc = crc32(0L, Z_NULL, 0);
    return ZIP_OK;
}

int zipWriteInFileInZip(zipFile file, const void *buf, unsigned len) {
    zip_internal *zi = (zip_internal *)file;
    zip_current_file *cf = get_current(file);
    if (!cf->in_opened_file) return ZIP_PARAMERROR;
    if (len == 0) return ZIP_OK;
    cf->crc = crc32(cf->crc, (const Bytef *)buf, len);
    cf->uncompressed_size += len;
    if (cf->method == Z_DEFLATED) {
        Bytef buffer[16384];
        cf->stream.next_in = (Bytef *)buf;
        cf->stream.avail_in = len;
        do {
            cf->stream.next_out = buffer;
            cf->stream.avail_out = sizeof(buffer);
            deflate(&cf->stream, Z_NO_FLUSH);
            uInt written = sizeof(buffer) - cf->stream.avail_out;
            if (written > 0) {
                fwrite(buffer, 1, written, (FILE *)zi->filestream);
                cf->compressed_size += written;
            }
        } while (cf->stream.avail_in > 0);
    } else {
        fwrite(buf, 1, len, (FILE *)zi->filestream);
        cf->compressed_size += len;
    }
    return ZIP_OK;
}

int zipCloseFileInZip(zipFile file) {
    zip_internal *zi = (zip_internal *)file;
    zip_current_file *cf = get_current(file);
    if (!cf->in_opened_file) return ZIP_PARAMERROR;
    if (cf->method == Z_DEFLATED) {
        Bytef buffer[16384];
        int err;
        do {
            cf->stream.next_out = buffer;
            cf->stream.avail_out = sizeof(buffer);
            err = deflate(&cf->stream, Z_FINISH);
            uInt written = sizeof(buffer) - cf->stream.avail_out;
            if (written > 0) {
                fwrite(buffer, 1, written, (FILE *)zi->filestream);
                cf->compressed_size += written;
            }
        } while (err != Z_STREAM_END);
        deflateEnd(&cf->stream);
    }

    uLong pos_after_data = ZTELL(zi->filefunc, zi->filestream);
    ZSEEK(zi->filefunc, zi->filestream, cf->pos_local_header + 14, ZLIB_FILEFUNC_SEEK_SET);
    put_uint32((FILE *)zi->filestream, cf->crc);
    put_uint32((FILE *)zi->filestream, cf->compressed_size);
    put_uint32((FILE *)zi->filestream, cf->uncompressed_size);
    ZSEEK(zi->filefunc, zi->filestream, pos_after_data, ZLIB_FILEFUNC_SEEK_SET);

    uLong central_pos = ZTELL(zi->filefunc, zi->filestream);
    uLong needed = 46 + cf->filename_len;
    if (zi->central_array.gap + needed > zi->central_array.size_array) {
        zi->central_array.size_array *= 2;
        zi->central_array.pData = realloc(zi->central_array.pData, zi->central_array.size_array);
    }
    char *p = zi->central_array.pData + zi->central_array.gap;
    uInt32 magic = CENTRAL_HEADER_MAGIC;
    memcpy(p, &magic, 4); p += 4;
    uInt16 vmade = VERSION_MADEBY; memcpy(p, &vmade, 2); p += 2;
    uInt16 vneed = VERSION_NEEDED; memcpy(p, &vneed, 2); p += 2;
    uInt16 flag = 0; memcpy(p, &flag, 2); p += 2;
    uInt16 meth = (uInt16)cf->method; memcpy(p, &meth, 2); p += 2;
    uInt16 tmod = 0; memcpy(p, &tmod, 2); p += 2;
    uInt16 dmod = 0; memcpy(p, &dmod, 2); p += 2;
    uInt32 crc = cf->crc; memcpy(p, &crc, 4); p += 4;
    uInt32 csize = cf->compressed_size; memcpy(p, &csize, 4); p += 4;
    uInt32 usize = cf->uncompressed_size; memcpy(p, &usize, 4); p += 4;
    uInt16 fnlen = cf->filename_len; memcpy(p, &fnlen, 2); p += 2;
    uInt16 eflen = 0; memcpy(p, &eflen, 2); p += 2;
    uInt16 cmlen = 0; memcpy(p, &cmlen, 2); p += 2;
    uInt16 dsk = 0; memcpy(p, &dsk, 2); p += 2;
    uInt16 ifa = (uInt16)cf->internalFa; memcpy(p, &ifa, 2); p += 2;
    uInt32 efa = cf->externalFa; memcpy(p, &efa, 4); p += 4;
    uInt32 lho = cf->pos_local_header; memcpy(p, &lho, 4); p += 4;
    memcpy(p, cf->filename, cf->filename_len); p += cf->filename_len;
    zi->central_array.gap += needed;

    zi->number_entry++;
    cf->in_opened_file = 0;
    return ZIP_OK;
}

int zipClose(zipFile file, const char *global_comment) {
    zip_internal *zi = (zip_internal *)file;
    if (!zi) return ZIP_PARAMERROR;
    zip_current_file *cf = get_current(file);
    if (cf->in_opened_file) zipCloseFileInZip(file);

    uLong central_offset = ZTELL(zi->filefunc, zi->filestream);
    fwrite(zi->central_array.pData, 1, zi->central_array.gap, (FILE *)zi->filestream);
    uLong central_size = zi->central_array.gap;

    uInt comment_len = global_comment ? (uInt)strlen(global_comment) : 0;
    put_uint32((FILE *)zi->filestream, END_CENTRAL_MAGIC);
    put_uint16((FILE *)zi->filestream, 0);
    put_uint16((FILE *)zi->filestream, 0);
    put_uint16((FILE *)zi->filestream, (uInt)zi->number_entry);
    put_uint16((FILE *)zi->filestream, (uInt)zi->number_entry);
    put_uint32((FILE *)zi->filestream, central_size);
    put_uint32((FILE *)zi->filestream, central_offset);
    put_uint16((FILE *)zi->filestream, comment_len);
    if (global_comment) fwrite(global_comment, 1, comment_len, (FILE *)zi->filestream);

    ZCLOSE(zi->filefunc, zi->filestream);
    free(zi->central_array.pData);
    free(zi);
    return ZIP_OK;
}
