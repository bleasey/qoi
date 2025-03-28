#include <stdlib.h>
#include "qoi.h"

qoi_image* qoi_encode(uint8_t* pixels, uint32_t width, uint32_t height, uint32_t channels) {
    qoi_image* image = (qoi_image*)malloc(sizeof(qoi_image));
    image->header.width = width;
    image->header.height = height;
    image->header.channels = channels;
    
    size_t max_size = width * height * (channels + 1), size = 0;
    uint8_t* output = (uint8_t*)malloc(max_size);
    uint8_t index[INDEX_LEN][3] = {0};

    uint8_t rgb_prev[3], rgb_curr[3];
    uint8_t output_byte;
    uint8_t run_length = 0;

    // encode the image: assuming channgels is 3
    for (size_t offset = 0; offset < width * height * channels; offset+=channels) {
        rgb_curr[0] = pixels[offset];
        rgb_curr[1] = pixels[offset + 1];
        rgb_curr[2] = pixels[offset + 2];

        uint8_t diff[3] = {rgb_curr[0] - rgb_prev[0], rgb_curr[1] - rgb_prev[1], rgb_curr[2] - rgb_prev[2]};
        
        // Check for same rgb value
        if (offset != 0 && !diff[0] && !diff[1] && !diff[2] && run_length < 62) {
            run_length++;
            continue;
        }
        else if (run_length) {
            output_byte = (QOI_OP_RUN << 6) | ((run_length - 1) & 0x3F);
            run_length = 0; 
            output[size++] = output_byte;
        }

        rgb_prev[0] = rgb_curr[0];
        rgb_prev[1] = rgb_curr[1];
        rgb_prev[2] = rgb_curr[2];

        // Index check
        uint8_t index_pos = (rgb_curr[0] * 3 + rgb_curr[1] * 5 + rgb_curr[2] * 7) % 64;
        if (index[index_pos][0] == rgb_curr[0] && index[index_pos][1] == rgb_curr[1] && index[index_pos][2] == rgb_curr[2]) {
            output_byte = (QOI_OP_INDEX << 6) | (index_pos & 0x3F);
            output[size++] = output_byte;
            continue;
        }
        else {
            index[index_pos][0] = rgb_curr[0];
            index[index_pos][1] = rgb_curr[1];
            index[index_pos][2] = rgb_curr[2];
        }

        // Diff check
        if (offset != 0 && diff[0] >= -2 && diff[0] <= 1 && diff[1] >= -2 && diff[1] <= 1 && diff[2] >= -2 && diff[2] <= 1) {
            output_byte = (QOI_OP_DIFF << 6) | ((diff[0] + 2) << 4) | ((diff[1] + 2) << 2) | (diff[2] + 2);
            output[size++] = output_byte;
            continue;
        }

        // Luma check
        if (offset != 0 && diff[1] >= -32 && diff[1] <= 31 && diff[0] >= -8 && diff[0] <= 7 && diff[2] >= -8 && diff[2] <= 7) {
            output[size++] = (QOI_OP_LUMA << 6) | ((diff[1] + 32) & 0x3F);
            output[size++] = ((diff[0] + 8) << 4) | (diff[2] + 8);
            continue;
        }

        // RGB direct
        output[size++] = QOI_OP_RGB;
        output[size++] = rgb_curr[0];
        output[size++] = rgb_curr[1];
        output[size++] = rgb_curr[2];
    }

    // Check for run length
    if (run_length) {
        output_byte = (QOI_OP_RUN << 6) | ((run_length - 1) & 0x3F);
        run_length = 0; 
        output[size++] = output_byte;
    }

    // Free the extra memory
    uint8_t* output_trimmed = realloc(output, size);
    if (output_trimmed) {
        output = output_trimmed;
    }

    image->length = size;
    image->data = output;

    return image;
}

uint8_t* qoi_decode(qoi_image* image) {
    size_t img_size = image->header.width * image->header.height * image->header.channels;
    size_t dec_idx = 0, enc_idx = 0;

    uint8_t* pixels = (uint8_t*)malloc(img_size);
    uint8_t index[INDEX_LEN][3] = {0};
    uint8_t curr_byte;
    uint8_t rgb_curr[3];

    while(dec_idx < img_size) {
        curr_byte = image->data[enc_idx++];
        
        // Direct rgb
        if (curr_byte == QOI_OP_RGB) {
            rgb_curr[0] = image->data[enc_idx++];
            rgb_curr[1] = image->data[enc_idx++];
            rgb_curr[2] = image->data[enc_idx++];
            pixels[dec_idx++] = rgb_curr[0];
            pixels[dec_idx++] = rgb_curr[1];
            pixels[dec_idx++] = rgb_curr[2];

            qoi_index_update(index, rgb_curr);
            continue;
        }

        uint8_t op = curr_byte >> 6;
        
        if (op == QOI_OP_RUN) {
            uint8_t run_length = (curr_byte & 0x3F) + 1;
            for (size_t i = 0; i < run_length; i++) {
                rgb_curr[0] = pixels[dec_idx - 3];
                rgb_curr[1] = pixels[dec_idx - 2];
                rgb_curr[2] = pixels[dec_idx - 1];
                pixels[dec_idx++] = rgb_curr[0];
                pixels[dec_idx++] = rgb_curr[1];
                pixels[dec_idx++] = rgb_curr[2];
            }

            continue;
        }

        if (op == QOI_OP_INDEX) {
            uint8_t index_pos = curr_byte & 0x3F;
            pixels[dec_idx++] = index[index_pos][0];
            pixels[dec_idx++] = index[index_pos][1];
            pixels[dec_idx++] = index[index_pos][2];
            
            continue;
        }

        if (op == QOI_OP_DIFF) {
            uint8_t diff[3] = {((curr_byte >> 4) & 0x3), ((curr_byte >> 2) & 0x3), (curr_byte & 0x3)};
            rgb_curr[0] = pixels[dec_idx - 3] + diff[0] - 2;
            rgb_curr[1] = pixels[dec_idx - 2] + diff[1] - 2;
            rgb_curr[2] = pixels[dec_idx - 1] + diff[2] - 2;
            pixels[dec_idx++] = rgb_curr[0];
            pixels[dec_idx++] = rgb_curr[1];
            pixels[dec_idx++] = rgb_curr[2];
            
            qoi_index_update(index, rgb_curr);
            continue;
        }

        if (op == QOI_OP_LUMA) {
            uint8_t luma = curr_byte & 0x3F;
            curr_byte = image->data[enc_idx++];
            uint8_t diff[3] = {((curr_byte >> 4) & 0xF), luma, (curr_byte & 0xF)};
            rgb_curr[0] = pixels[dec_idx - 3] + diff[0] - 8;
            rgb_curr[1] = pixels[dec_idx - 2] + diff[1] - 32;
            rgb_curr[2] = pixels[dec_idx - 1] + diff[2] - 8;
            pixels[dec_idx++] = rgb_curr[0];
            pixels[dec_idx++] = rgb_curr[1];
            pixels[dec_idx++] = rgb_curr[2];
            
            qoi_index_update(index, rgb_curr);
            continue;
        }
    }

    return pixels;
}

void qoi_index_update(uint8_t (*index)[3], uint8_t* rgb_curr) {
    uint8_t index_pos = (rgb_curr[0] * 3 + rgb_curr[1] * 5 + rgb_curr[2] * 7) % 64;
    index[index_pos][0] = rgb_curr[0];
    index[index_pos][1] = rgb_curr[1];
    index[index_pos][2] = rgb_curr[2];
}
