#if defined (__cplusplus)
# pragma once
#endif /* __cplusplus */
#
#if !defined (_image_h_)
# define _image_h_ 1
#
# include <stdint.h>
#
# if !defined (IMGAPI)
#  define IMGAPI extern
# endif /* IMGAPI */
#
# if defined (__cplusplus)

extern "C" {

# endif /* __cplusplus */

IMGAPI void *imageLoad(const char *, int *, int *, int *, const int);
IMGAPI void *imageLoadPNG(const char *, int *, int *, int *, const int);

# if defined (__cplusplus)

}

# endif /* __cplusplus */
#
# if defined (IMAGE_IMPLEMENTATION)
#
#  include <stdio.h>
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

static inline int32_t __pack(uint8_t [4]);

static const uint8_t g_sign_png[] = { 137, 80, 78, 71, 13, 10, 26, 10 };



IMGAPI void *imageLoad(const char *path, int *width, int *height, int *channel, const int desired) {

    if (!path)  { return (0); }
    if (!*path) { return (0); }

    FILE *f = fopen(path, "rb");
    if (!f) { return (0); }

    uint8_t sign[8];
    if (!fread(sign, sizeof(uint8_t), 8, f)) { return (0); }

    fclose(f), f = 0;
    if (!memcmp(sign, g_sign_png, sizeof(sign))) { return (imageLoadPNG(path, width, height, channel, desired)); }
    else {
        return (0);
    }
}

IMGAPI void *imageLoadPNG(const char *path, int *width, int *height, int *channel, const int desired) {
    if (!path)  { return (0); }
    if (!*path) { return (0); }

    FILE *f = fopen(path, "rb");
    if (!f) { return (0); }

    /* signature verification...
     * */
    uint8_t sign[8];
    if (fread(sign, sizeof(uint8_t), 8, f) != 8) { return (0); }
    if (memcmp(sign, g_sign_png, sizeof(sign))) { return (0); }

    /* chunk reading...
     * */
    while (1) {
        uint8_t *f_data;
        uint8_t f_length[4],
                f_type[4],
                f_crc[4];
       
        if (!fread(f_length, sizeof(uint8_t), 4, f)) { break; }
        if (!fread(f_type, sizeof(uint8_t), 4, f))   { break; }

        int length = __pack(f_length);
        f_data = malloc(length * sizeof(uint8_t));
        if (!f_data) { break; }
        if (!fread(f_data, sizeof(uint8_t), length, f)) { break; }
        if (!fread(f_crc, sizeof(uint8_t), 4, f))       { break; }

        printf("%.4s: %d\n", f_type, length);
        free(f_data), f_data = 0;
    }
    
    fclose(f), f = 0;
    return (0);
}



static inline int32_t __pack(uint8_t data[4]) {

#  if (LITTLE_ENDIAN)
    return (data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24));
#  else
    return (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
#  endif /* LITTLE_ENDIAN */

}

#  if defined (__cplusplus)

}

#  endif /* __cplusplus */
#
# endif /* IMAGE_IMPLEMENTATION */
#endif /* _image_h_ */
