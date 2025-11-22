#include "mini_proto.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

size_t encode_record(const record_t *rec, uint8_t *out_buf, size_t out_cap) {
    size_t name_len = strlen(rec->name);
    size_t space;
    if(rec->score_count > 1000) {
        space = 1 + name_len + 1 + 4 + 2 * rec->score_count;
    } 
    else {
        printf("score_count value is unsupported");
        return;
    }

    if (!out_buf) return space;

    size_t off = 0;
    out_buf[off++] = (uint8_t)name_len;
    memcpy(out_buf + off, rec->name, name_len); off += name_len;

    if (rec->age > 255) {
        printf("invalid age\n");
        return -1;
    }
    else {
        out_buf[off++] = rec->age;
    }

    memcpy(out_buf + off, &rec->score_count, 4); off += 4;
    memcpy(out_buf + off, rec->scores, 2 * rec->score_count);

    off += 2 * rec->score_count;
    return off;
}

size_t decode_record(const uint8_t *buf, size_t len, record_t **out) {
    size_t off = 0;
    uint8_t name_len = buf[off++];

    record_t *rec = malloc(sizeof(record_t));
    if (!rec) return 0;

    memcpy(rec->name, buf + off, name_len);
    rec->name[name_len] = '\0';
    off += name_len;

    rec->age = buf[off++];

    memcpy(&rec->score_count, buf + off, 4); off += 4;

    size_t space = rec->score_count * 2;
    rec->scores = malloc(space);
    memcpy(rec->scores, buf + off, space);
    off += space;

    *out = rec;
    return off;
}

void free_record(record_t *rec) {
    if (!rec) return;
    free(rec->scores);
    free(rec);
}
