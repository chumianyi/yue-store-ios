#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unzip.h"
#include "ioapi.h"

#define LOCAL_HEADER_MAGIC 0x04034b50
#define CENTRAL_HEADER_MAGIC 0x02014b50
#define END_CENTRAL_MAGIC 0x06054b50

typedef struct {
    zlib_filefunc_def filefunc;
    voidpf filestream;
    uLong central_pos;
    uLong central_size;
    uLong number_entry;
    uLong cur_file_ok;
    uLong cur_pos_incentral;
    struct {
        uLong stream_initialised;
        uInt method;
        z_stream stream;
        uLong compressed_size;
        uLong uncompressed_size;
        uLong crc;
        uLong stream_pos;
        Bytef *read_buffer;
        uInt read_buffer_size;
        uLong pos_local_header;
        char filename[512];
        uInt filename_len;
    } file;
} unz_internal;

static uInt16 read_uint16(FILE *f) { uInt16 v; fread(&v, 1, 2, f); return v; }
static uInt32 read_uint32(FILE *f) { uInt32 v; fread(&v, 1, 4, f); return v; }

static int unzGoToNextFile(unzFile file) {
    unz_internal *s = (unz_internal *)file;
    if (s->cur_file_ok == 0) return UNZ_END_OF_LIST_OF_FILE;
    if (s->cur_pos_incentral >= s->central_pos + s->central_size) {
        s->cur_file_ok = 0;
        return UNZ_END_OF_LIST_OF_FILE;
    }
    FILE *f = (FILE *)s->filestream;
    fseek(f, s->cur_pos_incentral, SEEK_SET);
    uInt32 magic = read_uint32(f);
    if (magic != CENTRAL_HEADER_MAGIC) { s->cur_file_ok = 0; return UNZ_END_OF_LIST_OF_FILE; }
    fseek(f, 16, SEEK_CUR);
    uInt32 crc = read_uint32(f);
    uInt32 csize = read_uint32(f);
    uInt32 usize = read_uint32(f);
    uInt16 fnlen = read_uint16(f);
    uInt16 eflen = read_uint16(f);
    uInt16 cmlen = read_uint16(f);
    fseek(f, 8, SEEK_CUR);
    uInt32 lho = read_uint32(f);
    if (fnlen > 511) fnlen = 511;
    fread(s->file.filename, 1, fnlen, f);
    s->file.filename[fnlen] = '\0';
    s->file.filename_len = fnlen;
    s->file.pos_local_header = lho;
    s->file.method = 0;
    s->cur_pos_incentral = ftell(f) + eflen + cmlen;
    s->cur_file_ok = 1;
    return UNZ_OK;
}

voidp unzOpen(const char *path) {
    zlib_filefunc_def filefunc;
    fill_fopen_filefunc(&filefunc);
    voidpf filestream = ZOPEN(filefunc, path, ZLIB_FILEFUNC_MODE_READ);
    if (!filestream) return NULL;
    FILE *f = (FILE *)filestream;
    fseek(f, 0, SEEK_END);
    long filelen = ftell(f);
    if (filelen < 22) { fclose(f); return NULL; }
    long back = (filelen > 65557) ? 65557 : filelen;
    fseek(f, filelen - back, SEEK_SET);
    uByte *buf = (uByte *)malloc(back);
    fread(buf, 1, back, f);
    uLong central_pos = 0, central_size = 0, num_entry = 0;
    int found = 0;
    for (long i = back - 22; i >= 0; i--) {
        if (buf[i] == 0x50 && buf[i+1] == 0x4b && buf[i+2] == 0x05 && buf[i+3] == 0x06) {
            uLong off = filelen - back + i;
            fseek(f, off + 10, SEEK_SET);
            num_entry = read_uint16(f);
            central_size = read_uint32(f);
            central_pos = read_uint32(f);
            found = 1;
            break;
        }
    }
    free(buf);
    if (!found) { fclose(f); return NULL; }
    unz_internal *s = (unz_internal *)malloc(sizeof(unz_internal));
    memset(s, 0, sizeof(*s));
    s->filefunc = filefunc;
    s->filestream = filestream;
    s->central_pos = central_pos;
    s->central_size = central_size;
    s->number_entry = num_entry;
    s->cur_pos_incentral = central_pos;
    s->cur_file_ok = 1;
    s->file.read_buffer_size = 16384;
    s->file.read_buffer = (Bytef *)malloc(s->file.read_buffer_size);
    unzGoToNextFile((unzFile)s);
    return (unzFile)s;
}

int unzClose(unzFile file) {
    unz_internal *s = (unz_internal *)file;
    if (!s) return UNZ_PARAMERROR;
    if (s->file.stream_initialised) inflateEnd(&s->file.stream);
    free(s->file.read_buffer);
    ZCLOSE(s->filefunc, s->filestream);
    free(s);
    return UNZ_OK;
}

int unzGetGlobalInfo(unzFile file, unz_global_info *pglobal_info) {
    unz_internal *s = (unz_internal *)file;
    pglobal_info->number_entry = s->number_entry;
    pglobal_info->size_comment = 0;
    return UNZ_OK;
}

int unzGetCurrentFileInfo(unzFile file, unz_file_info *pfile_info,
                            char *szFileName, uLong fileNameBufferSize,
                            void *extraField, uLong extraFieldBufferSize,
                            char *szComment, uLong commentBufferSize) {
    unz_internal *s = (unz_internal *)file;
    if (!s->cur_file_ok) return UNZ_END_OF_LIST_OF_FILE;
    memset(pfile_info, 0, sizeof(*pfile_info));
    pfile_info->size_filename = s->file.filename_len;
    if (szFileName && fileNameBufferSize > 0) {
        uLong cp = (s->file.filename_len < fileNameBufferSize - 1) ? s->file.filename_len : fileNameBufferSize - 1;
        memcpy(szFileName, s->file.filename, cp);
        szFileName[cp] = '\0';
    }
    FILE *f = (FILE *)s->filestream;
    fseek(f, s->file.pos_local_header, SEEK_SET);
    uInt32 magic = read_uint32(f);
    if (magic == LOCAL_HEADER_MAGIC) {
        fseek(f, 4, SEEK_CUR);
        pfile_info->flag = read_uint16(f);
        pfile_info->compression_method = read_uint16(f);
        fseek(f, 4, SEEK_CUR);
        pfile_info->crc = read_uint32(f);
        pfile_info->compressed_size = read_uint32(f);
        pfile_info->uncompressed_size = read_uint32(f);
        uInt16 fnlen = read_uint16(f);
        uInt16 eflen = read_uint16(f);
        pfile_info->size_file_extra = eflen;
        if (extraField && extraFieldBufferSize > 0 && eflen > 0) {
            fseek(f, fnlen, SEEK_CUR);
            uLong rd = (eflen < extraFieldBufferSize) ? eflen : extraFieldBufferSize;
            fread(extraField, 1, rd, f);
        }
    }
    return UNZ_OK;
}

int unzOpenCurrentFile(unzFile file) {
    unz_internal *s = (unz_internal *)file;
    if (!s->cur_file_ok) return UNZ_END_OF_LIST_OF_FILE;
    FILE *f = (FILE *)s->filestream;
    fseek(f, s->file.pos_local_header, SEEK_SET);
    uInt32 magic = read_uint32(f);
    if (magic != LOCAL_HEADER_MAGIC) return UNZ_BADZIPFILE;
    fseek(f, 4, SEEK_CUR);
    uInt flag = read_uint16(f);
    uInt method = read_uint16(f);
    fseek(f, 4, SEEK_CUR);
    uLong crc = read_uint32(f);
    uLong csize = read_uint32(f);
    uLong usize = read_uint32(f);
    uInt16 fnlen = read_uint16(f);
    uInt16 eflen = read_uint16(f);
    s->file.method = method;
    s->file.crc = crc;
    s->file.compressed_size = csize;
    s->file.uncompressed_size = usize;
    s->file.stream_pos = 0;
    long data_start = s->file.pos_local_header + 30 + fnlen + eflen;
    fseek(f, data_start, SEEK_SET);
    if (method == Z_DEFLATED) {
        s->file.stream.zalloc = Z_NULL;
        s->file.stream.zfree = Z_NULL;
        s->file.stream.opaque = Z_NULL;
        s->file.stream.avail_in = 0;
        s->file.stream.next_in = Z_NULL;
        inflateInit2(&s->file.stream, -MAX_WBITS);
        s->file.stream_initialised = 1;
    }
    return UNZ_OK;
}

int unzReadCurrentFile(unzFile file, voidp buf, unsigned len) {
    unz_internal *s = (unz_internal *)file;
    if (!s->cur_file_ok) return UNZ_END_OF_LIST_OF_FILE;
    FILE *f = (FILE *)s->filestream;
    if (s->file.method == 0) {
        uLong remain = s->file.compressed_size - s->file.stream_pos;
        uLong rd = (len < remain) ? len : remain;
        if (rd > 0) {
            fread(buf, 1, rd, f);
            s->file.stream_pos += rd;
        }
        return (int)rd;
    } else if (s->file.method == Z_DEFLATED) {
        s->file.stream.next_out = (Bytef *)buf;
        s->file.stream.avail_out = len;
        while (s->file.stream.avail_out > 0) {
            if (s->file.stream.avail_in == 0 && s->file.stream_pos < s->file.compressed_size) {
                uLong remain = s->file.compressed_size - s->file.stream_pos;
                uLong rd = (s->file.read_buffer_size < remain) ? s->file.read_buffer_size : remain;
                fread(s->file.read_buffer, 1, rd, f);
                s->file.stream_pos += rd;
                s->file.stream.next_in = s->file.read_buffer;
                s->file.stream.avail_in = (uInt)rd;
            }
            int err = inflate(&s->file.stream, Z_NO_FLUSH);
            if (err == Z_STREAM_END) break;
            if (err != Z_OK) return 0;
        }
        return (int)(len - s->file.stream.avail_out);
    }
    return 0;
}

int unzCloseCurrentFile(unzFile file) {
    unz_internal *s = (unz_internal *)file;
    if (s->file.stream_initialised) {
        inflateEnd(&s->file.stream);
        s->file.stream_initialised = 0;
    }
    return UNZ_OK;
}
