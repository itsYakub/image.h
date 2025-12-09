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

IMGAPI void *imageLoadPNG(const char *, int *, int *);

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

static inline int __memcmp(const void *s0, const void *s1, size_t n) {
    const uint8_t *c0 = (uint8_t *) s0,
                  *c1 = (uint8_t *) s1;

    while (n--) {
        if (*c0 != *c1) { return (*c0 - *c1); }
        c0++;
        c1++;
    }
    return (0);
}

static inline void *__memcpy(void *dst, const void *src, size_t n) {
    uint8_t *c0 = (uint8_t *) src,
            *c1 = (uint8_t *) dst;

    while (n--) {
        *c1++ = *c0++;
    }
    return (dst);
}

struct s_file {
    uint8_t *data0, /* original pointer   */
            *data1; /* modifiable pointer */

    size_t   size;
};

static inline int __readf(struct s_file *fs, FILE *f) {
    /* Null-check...
     * */
    if (!fs) { return (0); }
    if (!f)  { return (0); }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (!size) { return (0); }

    uint8_t *data = malloc(size);
    if (!data) { return (0); }
    if (fread(data, sizeof(uint8_t), size, f) != size) { free(data); return (0); }

    fs->data0 = data;
    fs->data1 = data;
    fs->size  = size;
    return (1);
}

static inline int __skipf(struct s_file *fs, const size_t n) {
    /* Null-check...
     * */
    if (!fs)        { return (0); }
    if (!fs->data0) { return (0); }
    if (!fs->data1) { return (0); }
    if (!n)         { return (0); }

    /* Check if we've exhausted the file buffer...
     * */
    if ((size_t) (fs->data1 - fs->data0) >= fs->size) { return (0); }

    fs->data1 += n;
    return (1);
}

static inline int __freef(struct s_file *fs) {
    /* Null-check...
     * */
    if (!fs) { return (0); }

    if (fs->data0) { free(fs->data0); }
    fs->data0 = 0;
    fs->data1 = 0;
    fs->size  = 0;
    return (1);
}

static inline uint8_t *__getf(struct s_file *fs) {
    /* Null-check...
     * */
    if (!fs) { return (0); }
    
    return (fs->data1);
}


/* SECTION: global objects
 * */

static const uint8_t g_sign_png[] = { 137, 80, 78, 71, 13, 10, 26, 10 };



/* SECTION: image.h api (.png)
 * */

struct s_png;

struct s_chunk {
    uint8_t *data;
    uint8_t  length[4];
    uint8_t  type[4];
    uint8_t  crc[4];
};

static inline int __chunk(struct s_chunk *, struct s_file *fs);

struct s_ihdr {
    uint32_t width;
    uint32_t height;

    uint8_t bit;
    uint8_t type;
    uint8_t comp;
    uint8_t filter;
    uint8_t interlane;
};

static inline int __ihdr(struct s_png *, struct s_chunk *);

struct s_plte {
    uint8_t data[256 * 4];
    size_t  size;
};

static inline int __plte(struct s_png *, struct s_chunk *);

struct s_idat {
    uint8_t *data;
    size_t   size;
};

static inline int __idat(struct s_png *, struct s_chunk *);

struct s_iend {
    uint8_t *data;
    size_t   size;
};

static inline int __iend(struct s_png *);

struct s_zlib {
    uint8_t *data;
    uint8_t  flags;
    uint8_t  cmf;
    uint16_t check;
};

static inline int __zlib_read(struct s_png *);

struct s_png {
    struct s_ihdr ihdr;
    struct s_plte plte;
    struct s_idat idat;
    struct s_iend iend;
};



IMGAPI void *imageLoadPNG(const char *path, int *width, int *height) {
    if (!path)  { goto __failure; }
    if (!*path) { goto __failure; }

    FILE *f = fopen(path, "rb");
    if (!f) { goto __failure; }

    struct s_file fs = { 0 };
    if (!__readf(&fs, f)) { goto __failure; } 
    fclose(f), f = 0;

    /* file verification...
     * */
    
    if (__memcmp(__getf(&fs), g_sign_png, 8)) { goto __failure; }
    if (!__skipf(&fs, 8))                     { goto __failure; }

    /* file parsing...
     * */

    struct s_png png = { 0 };
    struct s_chunk chunk = { 0 };
    while (__chunk(&chunk, &fs)) {
        /* IHDR: header */
        if (!__memcmp(chunk.type, "IHDR", 4)) {
            int result = __ihdr(&png, &chunk);
            if (!result) {
                goto __failure;
            }
        }
        
        /* PLTE: palette */
        else if (!__memcmp(chunk.type, "PLTE", 4)) {
            int result = __plte(&png, &chunk);
            if (!result) {
                goto __failure;
            }
        }
        
        /* IDAT: data */
        else if (!__memcmp(chunk.type, "IDAT", 4)) {
            int result = __idat(&png, &chunk);
            if (!result) {
                goto __failure;
            }
        }

        /* OTHER: ancillary */
        else {
            /* tRNS: transparency */
            if (!__memcmp(chunk.type, "tRNS", 4)) { }

            /* gAMA: gamma */
            else if (!__memcmp(chunk.type, "gAMA", 4)) { }

            /* cHRM: chromaticities */
            else if (!__memcmp(chunk.type, "cHRM", 4)) { }

            /* sRGB: RGB color space */
            else if (!__memcmp(chunk.type, "sRGB", 4)) { }

            /* iCCP: embedded ICC profile */
            else if (!__memcmp(chunk.type, "iCCP", 4)) { }

            /* tEXt: textual data */
            else if (!__memcmp(chunk.type, "tEXt", 4)) { }

            /* zTXt: textual data (compressed) */
            else if (!__memcmp(chunk.type, "zTXt", 4)) { }

            /* iTXt: textual data (international) */
            else if (!__memcmp(chunk.type, "iTXt", 4)) { }

            /* bKGD: background color */
            else if (!__memcmp(chunk.type, "bKGD", 4)) { }

            /* pHYs: physical pixel dimensions */
            else if (!__memcmp(chunk.type, "pHYs", 4)) { }

            /* sBIT: significant bits */
            else if (!__memcmp(chunk.type, "sBIT", 4)) { }

            /* sPLT: suggested palette */
            else if (!__memcmp(chunk.type, "sPLT", 4)) { }

            /* hIST: palette histogram */
            else if (!__memcmp(chunk.type, "hIST", 4)) { }

            /* tIME: image timestamp (last modification) */
            else if (!__memcmp(chunk.type, "tIME", 4)) { }
        }

        if (chunk.data) { free(chunk.data), chunk.data = 0; }
    }
    
    /* IEND: end */
    if (!__memcmp(chunk.type, "IEND", 4)) {
        int result = __iend(&png);
        if (!result) {
            goto __failure;
        }
    }

    __freef(&fs);
    if (chunk.data) { free(chunk.data); }
    if (png.idat.data) { free(png.idat.data); }

    if (width)  { *width  = png.ihdr.width;  }
    if (height) { *height = png.ihdr.height; }
    return (png.iend.data);

__failure:
    
    __freef(&fs);
    if (chunk.data) { free(chunk.data); }
    if (png.idat.data) { free(png.idat.data); }

    if (width)  { *width  = 0; }
    if (height) { *height = 0; }
    return (0);
}



static inline int __chunk(struct s_chunk *chunk, struct s_file *fs) {
    /* null-check...
     * */
    if (!chunk) { return (0); }
    if (!fs)    { return (0); }

    /* extract: length */
    if (!__memcpy(chunk->length, __getf(fs), 4)) { return (0); }
    if (!__skipf(fs, 4)) { return (0); }
    
    /* extract: type / header */
    if (!__memcpy(chunk->type, __getf(fs), 4)) { return (0); }
    if (!__skipf(fs, 4)) { return (0); }
    
    size_t length = __pack(chunk->length);
    if (!length) { return (0); }

    chunk->data = malloc(length * sizeof(uint8_t));
    if (!chunk->data) { return (0); }

    /* extract: data */
    if (!__memcpy(chunk->data, __getf(fs), length)) { return (0); }
    if (!__skipf(fs, length)) { return (0); }
    
    /* extract: CRC */
    if (!__memcpy(chunk->crc, __getf(fs), 4)) { return (0); }
    if (!__skipf(fs, 4)) { return (0); }

    return (1);
}

static inline int __ihdr(struct s_png *png, struct s_chunk *chunk) {
    /* null-check...
     * */
    if (!png)   { return (0); }
    if (!chunk) { return (0); }

    png->ihdr.width = __pack(&chunk->data[0]);
    if (!png->ihdr.width)            { return (0); }
    if (png->ihdr.width > (1 << 24)) { return (0); }

    png->ihdr.height = __pack(&chunk->data[4]);
    if (!png->ihdr.height)            { return (0); }
    if (png->ihdr.height > (1 << 24)) { return (0); }

    png->ihdr.bit = chunk->data[8];
    if (png->ihdr.bit != 1 &&
        png->ihdr.bit != 2 &&
        png->ihdr.bit != 4 &&
        png->ihdr.bit != 8 &&
        png->ihdr.bit != 16) { return (0); }

    png->ihdr.type = chunk->data[9];
    if (png->ihdr.type != 0 &&
        png->ihdr.type != 2 &&
        png->ihdr.type != 3 &&
        png->ihdr.type != 4 &&
        png->ihdr.type != 6) { return (0); }

    png->ihdr.comp = chunk->data[10];
    if (png->ihdr.comp) { return (0); }

    png->ihdr.filter = chunk->data[11];
    if (png->ihdr.filter) { return (0); }

    png->ihdr.interlane = chunk->data[12];
    if (png->ihdr.interlane != 0 &&
        png->ihdr.interlane != 1) { return (0); }

    return (1);
}

static inline int __plte(struct s_png *png, struct s_chunk *chunk) {
    /* null-check...
     * */
    if (!png)   { return (0); }
    if (!chunk) { return (0); }

    size_t length = __pack(chunk->length);
    if (length > 256 * 3) { return (0); }
    
    png->plte.size = length / 3.0;
    if (png->plte.size * 3 != length) { return (0); }

    for (size_t i = 0; i < png->plte.size; i++) {
        png->plte.data[i * 4 + 0] = chunk->data[i * 3 + 0];
        png->plte.data[i * 4 + 1] = chunk->data[i * 3 + 1];
        png->plte.data[i * 4 + 2] = chunk->data[i * 3 + 2];
        png->plte.data[i * 4 + 3] = 255;
    }

    return (1);
}

static inline int __idat(struct s_png *png, struct s_chunk *chunk) {
    /* null-check...
     * */
    if (!png)   { return (0); }
    if (!chunk) { return (0); }

    size_t length0 = png->idat.size,
           length1 = __pack(chunk->length);
    if (!length1) { return (0); }

    uint8_t *data = realloc(png->idat.data, (length0 + length1) * sizeof(uint8_t));
    if (!data) { return (0); }
    if (!__memcpy(&data[length0], chunk->data, length1)) { return (0); }

    png->idat.data = data;
    png->idat.size += length1;
    return (1);
}

static inline int __iend(struct s_png *png) {
    /* null-check...
     * */
    if (!png) { return (0); }

    /* ... */

    return (1);
}

static inline int __zlib_read(struct s_png *png) {
    /* null-check...
     * */
    if (!png) { return (0); }

    /* ... */

    return (1);
}

#  if defined (__cplusplus)

}

#  endif /* __cplusplus */
#
# endif /* IMAGE_IMPLEMENTATION */
#endif /* _image_h_ */
