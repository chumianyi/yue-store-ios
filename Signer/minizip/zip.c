#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zlib.h>
#include "zip.h"

#define LOCAL_HEADER_MAGIC 0x04034b50
#define CENTRAL_HEADER_MAGIC 0x02014b50
#define END_CENTRAL_MAGIC 0x06054b50
#define VERSION_MADEBY 20
#define VERSION_NEEDED 20

typedef struct central_dir_entry_s {
    uint32_t offset;
    uint32_t comp_size;
    uint32_t uncomp_size;
    uint32_t crc32;
    uint16_t method;
    uint16_t flag;
    uint16_t mod_time;
    uint16_t mod_date;
    char *filename;
    uint16_t filename_len;
    struct central_dir_entry_s *next;
} central_dir_entry;

typedef struct {
    FILE *fp;
    central_dir_entry *entries;
    central_dir_entry *last_entry;
    uint32_t num_entries;
    uint32_t cd_offset;
    int in_file;
    central_dir_entry *cur_entry;
    z_stream zstr;
    int zstr_init;
    uint32_t crc;
    unsigned char *comp_buf;
    uint32_t comp_buf_size;
} zip_internal;

static uint16_t dos_time(struct tm *t) {
    return (uint16_t)((t->tm_hour << 11) | (t->tm_min << 5) | (t->tm_sec / 2));
}
static uint16_t dos_date(struct tm *t) {
    return (uint16_t)(((t->tm_year + 1900 - 1980) << 9) | ((t->tm_mon + 1) << 5) | t->tm_mday);
}

static void write_le16(FILE *fp, uint16_t v) { fputc(v & 0xff, fp); fputc((v >> 8) & 0xff, fp); }
static void write_le32(FILE *fp, uint32_t v) { fputc(v & 0xff, fp); fputc((v >> 8) & 0xff, fp); fputc((v >> 16) & 0xff, fp); fputc((v >> 24) & 0xff, fp); }

zipFile zipOpen(const char *pathname, int append) {
    (void)append;
    zip_internal *z = calloc(1, sizeof(zip_internal));
    if (!z) return NULL;
    z->fp = fopen(pathname, "wb");
    if (!z->fp) { free(z); return NULL; }
    z->comp_buf_size = 65536;
    z->comp_buf = malloc(z->comp_buf_size);
    if (!z->comp_buf) { fclose(z->fp); free(z); return NULL; }
    return (zipFile)z;
}

int zipOpenNewFileInZip(zipFile file, const char *filename,
                         const zip_fileinfo *zipfi,
                         const void *extrafield_local, uint32_t size_extrafield_local,
                         const void *extrafield_global, uint32_t size_extrafield_global,
                         const char *comment, int method, int level) {
    (void)extrafield_local; (void)size_extrafield_local;
    (void)extrafield_global; (void)size_extrafield_global;
    (void)comment;
    zip_internal *z = (zip_internal *)file;
    if (!z || z->in_file) return ZIP_PARAMERROR;

    central_dir_entry *e = calloc(1, sizeof(central_dir_entry));
    if (!e) return ZIP_INTERNALERROR;
    e->filename = strdup(filename);
    e->filename_len = (uint16_t)strlen(filename);
    e->offset = (uint32_t)ftell(z->fp);
    e->method = (method == 0) ? 0 : 8;
    e->flag = 0;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (zipfi) {
        e->mod_time = dos_time(& (struct tm){.tm_sec=zipfi->tmz_date.tm_sec, .tm_min=zipfi->tmz_date.tm_min, .tm_hour=zipfi->tmz_date.tm_hour, .tm_mday=zipfi->tmz_date.tm_mday, .tm_mon=zipfi->tmz_date.tm_mon-1, .tm_year=zipfi->tmz_date.tm_year-1900});
        e->mod_date = e->mod_time;
        e->mod_time = dos_time(& (struct tm){.tm_sec=zipfi->tmz_date.tm_sec, .tm_min=zipfi->tmz_date.tm_min, .tm_hour=zipfi->tmz_date.tm_hour});
    } else {
        e->mod_time = dos_time(t);
        e->mod_date = dos_date(t);
    }

    /* Write local file header (with placeholder sizes/crc) */
    write_le32(z->fp, LOCAL_HEADER_MAGIC);
    write_le16(z->fp, VERSION_NEEDED);
    write_le16(z->fp, e->flag);
    write_le16(z->fp, e->method);
    write_le16(z->fp, e->mod_time);
    write_le16(z->fp, e->mod_date);
    write_le32(z->fp, 0); /* crc placeholder */
    write_le32(z->fp, 0); /* comp size placeholder */
    write_le32(z->fp, 0); /* uncomp size placeholder */
    write_le16(z->fp, e->filename_len);
    write_le16(z->fp, 0); /* extra len */
    fwrite(filename, 1, e->filename_len, z->fp);

    /* Init deflate */
    if (e->method == 8) {
        memset(&z->zstr, 0, sizeof(z->zstr));
        if (deflateInit2(&z->zstr, level ? level : Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            free(e->filename); free(e);
            return ZIP_INTERNALERROR;
        }
        z->zstr_init = 1;
    }
    z->crc = crc32(0L, Z_NULL, 0);
    z->cur_entry = e;
    z->in_file = 1;
    return ZIP_OK;
}

int zipWriteInFileInZip(zipFile file, const void *buf, uint32_t len) {
    zip_internal *z = (zip_internal *)file;
    if (!z || !z->in_file) return ZIP_PARAMERROR;
    z->cur_entry->uncomp_size += len;
    z->crc = crc32(z->crc, (const Bytef *)buf, len);

    if (z->cur_entry->method == 8) {
        z->zstr.next_in = (Bytef *)buf;
        z->zstr.avail_in = len;
        do {
            z->zstr.next_out = z->comp_buf;
            z->zstr.avail_out = z->comp_buf_size;
            deflate(&z->zstr, Z_NO_FLUSH);
            uint32_t have = z->comp_buf_size - z->zstr.avail_out;
            if (have) { fwrite(z->comp_buf, 1, have, z->fp); z->cur_entry->comp_size += have; }
        } while (z->zstr.avail_in > 0);
    } else {
        fwrite(buf, 1, len, z->fp);
        z->cur_entry->comp_size += len;
    }
    return ZIP_OK;
}

int zipCloseFileInZip(zipFile file) {
    zip_internal *z = (zip_internal *)file;
    if (!z || !z->in_file) return ZIP_PARAMERROR;
    central_dir_entry *e = z->cur_entry;

    if (e->method == 8 && z->zstr_init) {
        int ret;
        do {
            z->zstr.next_out = z->comp_buf;
            z->zstr.avail_out = z->comp_buf_size;
            ret = deflate(&z->zstr, Z_FINISH);
            uint32_t have = z->comp_buf_size - z->zstr.avail_out;
            if (have) { fwrite(z->comp_buf, 1, have, z->fp); e->comp_size += have; }
        } while (ret != Z_STREAM_END);
        deflateEnd(&z->zstr);
        z->zstr_init = 0;
    }

    e->crc32 = z->crc;

    /* Patch local header */
    long cur = ftell(z->fp);
    fseek(z->fp, e->offset + 14, SEEK_SET);
    write_le32(z->fp, e->crc32);
    write_le32(z->fp, e->comp_size);
    write_le32(z->fp, e->uncomp_size);
    fseek(z->fp, cur, SEEK_SET);

    /* Add to linked list */
    e->next = NULL;
    if (z->entries) z->last_entry->next = e;
    else z->entries = e;
    z->last_entry = e;
    z->num_entries++;
    z->in_file = 0;
    z->cur_entry = NULL;
    return ZIP_OK;
}

int zipClose(zipFile file, const char *global_comment) {
    zip_internal *z = (zip_internal *)file;
    if (!z) return ZIP_PARAMERROR;
    if (z->in_file) zipCloseFileInZip(file);

    z->cd_offset = (uint32_t)ftell(z->fp);

    /* Write central directory */
    central_dir_entry *e = z->entries;
    while (e) {
        write_le32(z->fp, CENTRAL_HEADER_MAGIC);
        write_le16(z->fp, VERSION_MADEBY);
        write_le16(z->fp, VERSION_NEEDED);
        write_le16(z->fp, e->flag);
        write_le16(z->fp, e->method);
        write_le16(z->fp, e->mod_time);
        write_le16(z->fp, e->mod_date);
        write_le32(z->fp, e->crc32);
        write_le32(z->fp, e->comp_size);
        write_le32(z->fp, e->uncomp_size);
        write_le16(z->fp, e->filename_len);
        write_le16(z->fp, 0); /* extra */
        write_le16(z->fp, 0); /* comment */
        write_le16(z->fp, 0); /* disk num */
        write_le16(z->fp, 0); /* internal attrs */
        write_le32(z->fp, 0); /* external attrs */
        write_le32(z->fp, e->offset);
        fwrite(e->filename, 1, e->filename_len, z->fp);
        e = e->next;
    }

    uint32_t cd_size = (uint32_t)ftell(z->fp) - z->cd_offset;
    uint16_t gc_len = global_comment ? (uint16_t)strlen(global_comment) : 0;

    /* End of central directory */
    write_le32(z->fp, END_CENTRAL_MAGIC);
    write_le16(z->fp, 0); /* disk */
    write_le16(z->fp, 0); /* cd disk */
    write_le16(z->fp, z->num_entries);
    write_le16(z->fp, z->num_entries);
    write_le32(z->fp, cd_size);
    write_le32(z->fp, z->cd_offset);
    write_le16(z->fp, gc_len);
    if (global_comment) fwrite(global_comment, 1, gc_len, z->fp);

    fclose(z->fp);
    e = z->entries;
    while (e) { central_dir_entry *n = e->next; free(e->filename); free(e); e = n; }
    free(z->comp_buf);
    free(z);
    return ZIP_OK;
}
