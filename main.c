#include "qoi.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void run_compression(const char* filepath) {  
    int width, height, channels;
    uint8_t* pixels = stbi_load(filepath, &width, &height, &channels, 0); // Load the image as an array of pixels
    
    if (!pixels) {
        printf("Failed to load image at %s\n", filepath);
        return;
    }
    else if (channels != 3) {
        printf("Aborting image at path %s due to %d channels\n", filepath, channels);
    }

    printf("Loaded image: %dx%d, %d channels\n", width, height, channels);

    qoi_image* qoiimage = qoi_encode(pixels, width, height, channels);
    uint8_t* decoded = qoi_decode(qoiimage);

    size_t sizeBefore = width * height * channels, sizeAfter = qoiimage->length;
    for (size_t i = 0; i < sizeBefore; i++) {
        if (pixels[i] != decoded[i]) {
            printf("Mismatch at index %d: %d != %d\n", i, pixels[i], decoded[i]);
            break;
        }
    }

    printf("Reduced size to %.2f %%, from %d --> %d bytes\n", 100 * ((float) sizeAfter) / sizeBefore, sizeBefore, sizeAfter);
    printf("- - - - - - - -\n");
    stbi_image_free(pixels);
}
