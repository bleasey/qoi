#if !defined(QOI_H)
#define QOI_H

#include <stdint.h>

#define INDEX_LEN 64
#define QOI_END 0x01 // 8-byte value

typedef enum {
    QOI_OP_INDEX  = 0b00,
    QOI_OP_DIFF   = 0b01,
    QOI_OP_LUMA   = 0b10,
    QOI_OP_RUN    = 0b11,
    QOI_OP_RGB    = 0xFE,
    QOI_OP_RGBA   = 0xFF
} qoi_tag;

typedef struct {
    uint32_t    width;
    uint32_t    height;
    uint32_t    channels;
} qoi_header;

typedef struct {
    qoi_header  header;
    uint32_t    length;
    uint8_t*    data;
} qoi_image;

qoi_image* qoi_encode(uint8_t* pixels, uint32_t width, uint32_t height, uint32_t channels);
uint8_t* qoi_decode(qoi_image* image);
void qoi_index_update(uint8_t (*index)[3], uint8_t* rgb_curr);

#endif // QOI_H
