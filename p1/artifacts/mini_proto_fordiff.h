#ifndef MINI_PROTO_H
#define MINI_PROTO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NAME 32         

typedef struct {
    char     name[MAX_NAME];
    uint8_t  age;
    uint32_t score_count;
    uint16_t *scores;
} record_t;

/* Layout:
   [name_len:1][name_bytes][age:1][score_count:4][scores:2*score_count]

   Returns number of bytes written, or 0 on error.
   If out_buf == NULL, returns the size that would be needed (non-zero). */
size_t encode_record(const record_t *rec, uint8_t **out_buf, size_t out_cap);

/* Decodes one record. On success:
     - *out points to a newly allocated record_t that you own
     - return value is number of bytes consumed from buf
   On error, returns 0 and *out is left untouched. */
size_t decode_record(const uint8_t *buf, size_t len, record_t **out);

/* Frees a record allocated by decode_record */
void free_record(record_t *rec);

#ifdef __cplusplus
}
#endif

#endif
