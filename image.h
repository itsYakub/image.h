#if defined (__cplusplus)
# pragma once
#endif /* __cplusplus */
#
#if !defined (_image_h_)
# define _image_h_ 1
#
# if !defined (IMGAPI)
#  define IMGAPI extern
# endif /* IMGAPI */
#
# if defined (__cplusplus)

extern "C" {

# endif /* __cplusplus */

/* SECTION: image.h api
 * */

IMGAPI void *imageLoadPNG(const char *, int *, int *, int *, const int);

# if defined (__cplusplus)

}

# endif /* __cplusplus */
#
# if defined (IMAGE_IMPLEMENTATION)
#
#  include <stdio.h>
#  include <stdint.h>
#  include <stddef.h>
#  include <stdlib.h>
#  include <string.h>
#
#  if !defined LITTLE_ENDIAN
#   define LITTLE_ENDIAN (*(uint8_t) &(uint16_t) { 1 })
#  endif /* LITTLE_ENDIAN */
#
#  if defined (__cplusplus)

extern "C" {

#  endif /* __cplusplus */

/* SECTION: static functions
 * */

static inline int32_t __pack(uint8_t data[4]) {

#  if (LITTLE_ENDIAN)
    return (data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24));
#  else
    return (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
#  endif /* LITTLE_ENDIAN */

}

/* SECTION: global objects
 * */

static const uint8_t g_sign_png[] = { 137, 80, 78, 71, 13, 10, 26, 10 };



/* SECTION: image.h api (.png)
 * */

struct s_png {
    struct {
        int32_t width;
        int32_t height;

        int8_t bit;
        int8_t type;
        int8_t comp;
        int8_t filter;
        int8_t interlane;
    } ihdr;

    struct { } plte;

    struct { } idat;
};

IMGAPI void *imageLoadPNG(const char *path, int *width, int *height, int *channel, const int desired) {
    if (!path)  { return (0); }
    if (!*path) { return (0); }

    FILE *f = fopen(path, "rb");
    if (!f) { return (0); }

    /* file verification...
     * */
    
    uint8_t sign[8];
    if (fread(sign, sizeof(uint8_t), 8, f) != 8) { return (0); }
    if (memcmp(sign, g_sign_png, sizeof(sign)))  { return (0); }

    /* file parsing...
     * */

    struct s_png png = { 0 };
    while (1) {

        /* chunk parsing...
         * */

        uint8_t f_length[4] = { 0 }; 
        if (!fread(f_length, sizeof(uint8_t), 4, f)) { break; }

        uint8_t f_type[4] = { 0 };
        if (!fread(f_type, sizeof(uint8_t), 4, f)) { break; }
        if (!memcmp(f_type, "IEND", 4))            { break; }

        size_t length = __pack(f_length);
        uint8_t *f_data = malloc(length * sizeof(uint8_t));
        if (!f_data) { break; }
        if (!fread(f_data, sizeof(uint8_t), length, f)) { break; }

        uint8_t f_crc[4] = { 0 };
        if (!fread(f_crc, sizeof(uint8_t), 4, f)) { break; }


        /* chunk processing...
         * */

        /* IHDR: header */
        if (!memcmp(f_type, "IHDR", 4)) {
            png.ihdr.width = __pack(&f_data[0]);
            png.ihdr.height = __pack(&f_data[4]);
            png.ihdr.bit = f_data[8];
            png.ihdr.type = f_data[9];
            png.ihdr.comp = f_data[10];
            png.ihdr.filter = f_data[11];
            png.ihdr.interlane = f_data[12];
        }
        
        /* PLTE: palette */
        else if (!memcmp(f_type, "PLTE", 4)) {

        }
        
        /* IDAT: data */
        else if (!memcmp(f_type, "IDAT", 4)) {

        }

        /* OTHER: ancillary */
        else { }


        /* chunk free...
         * */

        free(f_data), f_data = 0;
    }
    
    if (width)  { *width = png.ihdr.width; }
    if (height) { *height = png.ihdr.height; }
    fclose(f), f = 0;
    return (0);
}

#  if defined (__cplusplus)

}

#  endif /* __cplusplus */
#
# endif /* IMAGE_IMPLEMENTATION */
#endif /* _image_h_ */
