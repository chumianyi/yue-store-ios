#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "zip.h"

#define LOCAL_HEADER_MAGIC 0x04034b50
#define CENTRAL_HEADER_MAGIC 0x02014b50
#define END_CENTRAL_MAGIC 0x06054b50

typedef struct unz_entry_s {
    uint32_t offset;
    uint32_t comp_size;
    uint32_t uncomp_size;
    uint32_t crc32;
    uint16_t method;
    char *filename;
    uint16_t filename_len;
    struct unz_entry_s *next;
} unz_entry;

typedef struct {
    FILE *fp;
    unz_entry *entries;
    unz_entry *cur;
    int cur_open;
    z_stream zstr;
    int zstr_init;
    uint32_t bytes_left;
    uint32_t comp_left;
} unz_internal;

static uint16_t read_le16(FILE *fp) {
    int a = fgetc(fp), b = fgetc(fp);
    return (uint16_t)(a | (b << 8));
}
static uint32_t read_le32(FILE *fp) {
    int a = fgetc(fp), b = fgetc(fp), c = fgetc(fp), d = fgetc(fp);
    return (uint32_t)(a | (b << 8) | (c << 16) | (d << 24));
}

unzFile unzOpen(const char *path) {
    unz_internal *u = calloc(1, sizeof(unz_internal));
    if (!u) return NULL;
    u->fp = fopen(path, "rb");
    if (!u->fp) { free(u); return NULL; }

    fseek(u->fp, 0, SEEK_END);
    long fsize = ftell(u->fp);
    long search_start = fsize - 65557;
    if (search_start < 0) search_start = 0;
    fseek(u->fp, search_start, SEEK_SET);
    unsigned char buf[65557];
    size_t read_size = (size_t)(fsize - search_start);
    fread(buf, 1, read_size, u->fp);

    uint32_t eocd_offset = 0;
    int found = 0;
    for (long i = (long)read_size - 22; i >= 0; i--) {
        if (buf[i] == 0x50 && buf[i+1] == 0x4b && buf[i+2] == 0x05 && buf[i+3] == 0x06) {
            eocd_offset = (uint32_t)(search_start + i);
            found = 1; break;
        }
    }
    if (!found) { fclose(u->fp); free(u); return NULL; }

    fseek(u->fp, eocd_offset + 10, SEEK_SET);
    uint16_t num_entries = read_le16(u->fp);
    fseek(u->fp, eocd_offset + 16, SEEK_SET);
    read_le32(u->fp); /* cd_size */
    uint32_t cd_offset = read_le32(u->fp);

    fseek(u->fp, cd_offset, SEEK_SET);
    unz_entry *last = NULL;
    for (uint16_t i = 0; i < num_entries; i++) {
        uint32_t sig = read_le32(u->fp);
        if (sig != CENTRAL_HEADER_MAGIC) break;
        fseek(u->fp, 6, SEEK_CUR);
        read_le16(u->fp); /* flag */
        uint16_t method = read_le16(u->fp);
        fseek(u->fp, 4, SEEK_CUR);
        uint32_t crc = read_le32(u->fp);
        uint32_t comp_size = read_le32(u->fp);
        uint32_t uncomp_size = read_le32(u->fp);
        uint16_t fn_len = read_le16(u->fp);
        uint16_t ex_len = read_le16(u->fp);
        uint16_t cm_len = read_le16(u->fp);
        fseek(u->fp, 8, SEEK_CUR);
        uint32_t offset = read_le32(u->fp);
        char *filename = malloc(fn_len + 1);
        fread(filename, 1, fn_len, u->fp);
        filename[fn_len] = 0;
        fseek(u->fp, ex_len + cm_len, SEEK_CUR);
        unz_entry *e = calloc(1, sizeof(unz_entry));
        e->offset = offset; e->comp_size = comp_size; e->uncomp_size = uncomp_size;
        e->crc32 = crc; e->method = method; e->filename = filename; e->filename_len = fn_len;
        e->next = NULL;
        if (last) last->next = e; else u->entries = e;
        last = e;
    }
    u->cur = u->entries;
    return (unzFile)u;
}

int unzGetGlobalInfo(unzFile file, unz_global_info *pglobal_info) {
    unz_internal *u = (unz_internal *)file;
    if (!u || !pglobal_info) return UNZ_PARAMERROR;
    uint32_t count = 0;
    unz_entry *e = u->entries;
    while (e) { count++; e = e->next; }
    pglobal_info->number_entry = count;
    return UNZ_OK;
}

int unzGoToFirstFile(unzFile file) {
    unz_internal *u = (unz_internal *)file;
    if (!u) return ZIP_PARAMERROR;
    if (u->cur_open) unzCloseCurrentFile(file);
    u->cur = u->entries;
    return u->cur ? ZIP_OK : ZIP_END;
}

int unzGoToNextFile(unzFile file) {
    unz_internal *u = (unz_internal *)file;
    if (!u || !u->cur) return ZIP_PARAMERROR;
    if (u->cur_open) unzCloseCurrentFile(file);
    u->cur = u->cur->next;
    return u->cur ? ZIP_OK : ZIP_END;
}

int unzGetCurrentFileInfo(unzFile file, unz_file_info *pfile_info,
                           char *filename, uLong filename_size,
                           void *extrafield, uLong extrafield_size,
                           char *comment, uLong comment_size) {
    (void)extrafield; (void)extrafield_size; (void)comment; (void)comment_size;
    unz_internal *u = (unz_internal *)file;
    if (!u || !u->cur) return UNZ_PARAMERROR;
    if (pfile_info) {
        memset(pfile_info, 0, sizeof(unz_file_info));
        pfile_info->compression_method = u->cur->method;
        pfile_info->compressed_size = u->cur->comp_size;
        pfile_info->uncompressed_size = u->cur->uncomp_size;
        pfile_info->crc = u->cur->crc32;
        pfile_info->size_filename = u->cur->filename_len;
    }
    if (filename && filename_size) {
        uLong copy = u->cur->filename_len < filename_size - 1 ? u->cur->filename_len : filename_size - 1;
        memcpy(filename, u->cur->filename, copy);
        filename[copy] = 0;
    }
    return UNZ_OK;
}

int unzOpenCurrentFile(unzFile file) {
    unz_internal *u = (unz_internal *)file;
    if (!u || !u->cur || u->cur_open) return ZIP_PARAMERROR;
    fseek(u->fp, u->cur->offset, SEEK_SET);
    uint32_t sig = read_le32(u->fp);
    if (sig != LOCAL_HEADER_MAGIC) return ZIP_BADZIPFILE;
    fseek(u->fp, 22, SEEK_CUR);
    uint16_t fn_len = read_le16(u->fp);
    uint16_t ex_len = read_le16(u->fp);
    fseek(u->fp, fn_len + ex_len, SEEK_CUR);
    u->comp_left = u->cur->comp_size;
    u->bytes_left = u->cur->uncomp_size;
    if (u->cur->method == 8) {
        memset(&u->zstr, 0, sizeof(u->zstr));
        if (inflateInit2(&u->zstr, -MAX_WBITS) != Z_OK) return ZIP_INTERNALERROR;
        u->zstr_init = 1;
    }
    u->cur_open = 1;
    return ZIP_OK;
}

int unzReadCurrentFile(unzFile file, void *buf, uint32_t len) {
    unz_internal *u = (unz_internal *)file;
    if (!u || !u->cur_open) return ZIP_PARAMERROR;
    if (u->bytes_left == 0) return 0;
    if (len > u->bytes_left) len = u->bytes_left;
    if (u->cur->method == 0) {
        uint32_t to_read = len < u->comp_left ? len : u->comp_left;
        size_t r = fread(buf, 1, to_read, u->fp);
        u->comp_left -= (uint32_t)r; u->bytes_left -= (uint32_t)r;
        return (int)r;
    } else if (u->cur->method == 8) {
        u->zstr.next_out = (Bytef *)buf;
        u->zstr.avail_out = len;
        static unsigned char comp_buf[65536];
        while (u->zstr.avail_out > 0 && u->comp_left > 0) {
            if (u->zstr.avail_in == 0) {
                uint32_t to_read = u->comp_left < sizeof(comp_buf) ? u->comp_left : (uint32_t)sizeof(comp_buf);
                size_t r = fread(comp_buf, 1, to_read, u->fp);
                u->comp_left -= (uint32_t)r;
                u->zstr.next_in = comp_buf;
                u->zstr.avail_in = (uInt)r;
            }
            int ret = inflate(&u->zstr, Z_NO_FLUSH);
            if (ret == Z_STREAM_END) break;
            if (ret != Z_OK) return ZIP_INTERNALERROR;
        }
        uint32_t got = len - u->zstr.avail_out;
        u->bytes_left -= got;
        return (int)got;
    }
    return ZIP_PARAMERROR;
}

int unzCloseCurrentFile(unzFile file) {
    unz_internal *u = (unz_internal *)file;
    if (!u || !u->cur_open) return ZIP_PARAMERROR;
    if (u->zstr_init) { inflateEnd(&u->zstr); u->zstr_init = 0; }
    u->cur_open = 0;
    return ZIP_OK;
}

int unzClose(unzFile file) {
    unz_internal *u = (unz_internal *)file;
    if (!u) return ZIP_PARAMERROR;
    if (u->cur_open) unzCloseCurrentFile(file);
    fclose(u->fp);
    unz_entry *e = u->entries;
    while (e) { unz_entry *n = e->next; free(e->filename); free(e); e = n; }
    free(u);
    return ZIP_OK;
}
