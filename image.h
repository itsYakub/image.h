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

static inline int32_t __pack16(uint8_t data[2]) {

#  if (LITTLE_ENDIAN)
    return (data[1] | (data[0] << 8));
#  else
    return (data[0] | (data[1] << 8));
#  endif /* LITTLE_ENDIAN */

}

static inline int32_t __pack32(uint8_t data[4]) {

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

static inline void *__memdup(const void *s0, size_t s) {
    uint8_t *c0 = (uint8_t *) s0,
            *c1 = (uint8_t *) 0;

    c1 = malloc(s);
    if (!c1) { return (0); }

    return (__memcpy(c1, c0, s));
}

static inline int __isinrange(const int32_t v, const int32_t arr[], const size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (v == arr[i]) {
            return (1);
        }
    }
    return (0);
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
    uint8_t  *data;
    uint32_t  length;
    uint32_t  type;
    uint32_t  crc;
};

static inline int __chunk(struct s_chunk *, struct s_file *fs);


struct s_ihdr {
    uint32_t width;
    uint32_t height;

    uint8_t bit;
    uint8_t type;
    uint8_t comp;
    uint8_t filter;
    uint8_t interlace;
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
    while (chunk.type != (uint32_t) __pack32((uint8_t *) "IEND")) {
        __chunk(&chunk, &fs);

        /* IHDR: header */
        if (chunk.type == (uint32_t) __pack32((uint8_t *) "IHDR")) {
            int result = __ihdr(&png, &chunk);

            if (!result) { goto __failure; }
        }
        
        /* PLTE: palette */
        else if (chunk.type == (uint32_t) __pack32((uint8_t *) "PLTE")) {
            int result = __plte(&png, &chunk);

            if (!result) { goto __failure; }
        }
        
        /* IDAT: data */
        else if (chunk.type == (uint32_t) __pack32((uint8_t *) "IDAT")) {
            int result = __idat(&png, &chunk);

            if (!result) { goto __failure; }
        }

        /* IEND: end */
        else if (chunk.type == (uint32_t) __pack32((uint8_t *) "IEND")) {
            int result = __iend(&png);

            if (!result) { goto __failure; }
        }

        /* OTHER: ancillary */
        else { }

        if (chunk.data) { free(chunk.data), chunk.data = 0; }
    }

    __freef(&fs);
    // if (chunk.data) { free(chunk.data); }
    if (png.idat.data) { free(png.idat.data); }

    if (width)  { *width  = png.ihdr.width;  }
    if (height) { *height = png.ihdr.height; }
    return (png.iend.data);

__failure:
    
    __freef(&fs);
    // if (chunk.data) { free(chunk.data); }
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
    chunk->length = __pack32(__getf(fs));
    __skipf(fs, 4);
    
    /* extract: type / header */
    chunk->type = __pack32(__getf(fs));
    __skipf(fs, 4);

    /* extract: data */
    chunk->data = __memdup(__getf(fs), chunk->length);
    __skipf(fs, chunk->length);
    
    /* extract: CRC */
    chunk->crc = __pack32(__getf(fs));
    __skipf(fs, 4);

    return (1);
}

static inline int __ihdr(struct s_png *png, struct s_chunk *chunk) {
    /* null-check...
     * */
    if (!png)   { return (0); }
    if (!chunk) { return (0); }

    uint8_t *data = chunk->data;
    if (!data) { return (0); }

    uint32_t width = __pack32(data); data += 4;
    if (!width) { return (0); }
    if (width > (1u << 24)) { return (0); }

    uint32_t height = __pack32(data); data += 4;
    if (!height) { return (0); }
    if (height > (1u << 24)) { return (0); }

    uint8_t bit = *data++;
    if (!__isinrange(bit, (int32_t []) { 1, 2, 4, 8, 16 }, 5)) { return (0); }

    uint8_t type = *data++;
    if (!__isinrange(type, (int32_t []) { 0, 2, 3, 4, 6 }, 5)) { return (0); }

    uint8_t comp = *data++;
    if (comp) { return (0); }

    uint8_t filter = *data++;
    if (filter) { return (0); }

    uint8_t interlace = *data++;
    if (!__isinrange(interlace, (int32_t []) { 0, 1 }, 2)) { return (0); }

    png->ihdr = (struct s_ihdr) {
        .width     = width,
        .height    = height,
        .bit       = bit,
        .type      = type,
        .comp      = comp,
        .filter    = filter,
        .interlace = interlace
    };

    return (1);
}

static inline int __plte(struct s_png *png, struct s_chunk *chunk) {
    /* null-check...
     * */
    if (!png)   { return (0); }
    if (!chunk) { return (0); }

    size_t length = chunk->length;
    if (length >= 256 * 3) { return (0); }
    
    png->plte.size = length / 3.0;
    if (png->plte.size * 3 != length) { return (0); }

    uint8_t *data0 = png->plte.data,
            *data1 = chunk->data;
    if (!data0) { return (0); }
    if (!data1) { return (0); }

    for (size_t i = 0; i < png->plte.size; i++) {
        *data0++ = *data1++;
        *data0++ = *data1++;
        *data0++ = *data1++;
        *data0++ = 255;
    }

    return (1);
}

static inline int __idat(struct s_png *png, struct s_chunk *chunk) {
    /* null-check...
     * */
    if (!png)   { return (0); }
    if (!chunk) { return (0); }

    /* Extract the PNG data...
     * */
    size_t length0 = png->idat.size,
           length1 = chunk->length;
    if (!length1) { return (0); }
    if (length1 > (1u << 30)) { return (0); }

    uint8_t *data = realloc(png->idat.data, (length0 + length1) * sizeof(uint8_t));
    if (!data) { return (0); }
    if (!__memcpy(data + length0, chunk->data, length1)) { return (0); }

    png->idat.data = data;
    png->idat.size += length1;
    return (1);
}

static inline int __iend(struct s_png *png) {
    /* null-check...
     * */
    if (!png)            { return (0); }
    if (!png->idat.data) { return (0); }
    if (!png->idat.size) { return (0); }

    return (1);
}

#  if defined (__cplusplus)

}

#  endif /* __cplusplus */
#
# endif /* IMAGE_IMPLEMENTATION */
#endif /* _image_h_ */
