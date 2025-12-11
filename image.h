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

/* SECTION: image.h api (.pnm)
 * */

IMGAPI void *imageLoadPNM(const char *, int *, int *);

/* SECTION: image.h api (.png)
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

static int __memcmp(const void *s0, const void *s1, size_t n) {
    const uint8_t *c0 = (uint8_t *) s0,
                  *c1 = (uint8_t *) s1;

    while (n--) {
        if (*c0 != *c1) { return (*c0 - *c1); }
        c0++;
        c1++;
    }
    return (0);
}

static void *__memcpy(void *dst, const void *src, size_t n) {
    uint8_t *c0 = (uint8_t *) src,
            *c1 = (uint8_t *) dst;

    while (n--) {
        *c1++ = *c0++;
    }
    return (dst);
}

static void *__memdup(const void *s0, size_t s) {
    if (!s0) { return (0); }

    uint8_t *c0 = (uint8_t *) s0,
            *c1 = (uint8_t *) 0;

    c1 = malloc(s);
    if (!c1) { return (0); }

    return (__memcpy(c1, c0, s));
}

static size_t __strlen(const char *str) {
    if (!str)  { return (0); }
    if (!*str) { return (0); }
    for (size_t i = 0; str; i++) {
        if (!str[i]) { return (i); }
    }
    return (0);
}

static char *__strdup(const char *str) {
    if (!str) { return (0); }

    const size_t length = __strlen(str);
    if (!length) { return (calloc(1, 1)); }

    char *s0 = calloc(length + 1, sizeof(char));
    if (!s0) { return (0); }

    return (__memcpy(s0, str, length));
}

static char *__strndup(const char *str, size_t n) {
    if (!str) { return (0); }
    if (!n)   { return (calloc(1, 1)); }

    const size_t length = __strlen(str);
    if (length <= n) { return (__strdup(str)); }

    char *s0 = calloc(n + 1, sizeof(char));
    if (!s0) { return (0); }

    return (__memcpy(s0, str, n));
}

static char *__getline(const char *str) {
    if (!str) { return (0); }

    const char *find = str;
    while (*find && *find != '\n') { find++; }

    return (__strndup(str, find - str));
}

static int __isinrange(const int32_t v, const int32_t arr[], const size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (v == arr[i]) {
            return (1);
        }
    }
    return (0);
}

static int __isspace(int c) {
    return ((c >= '\t' && c <= '\r') || c == ' ');
}

static int __isdigit(int c) {
    return (c >= '0' && c <= '9');
}

static int __atoi(const char *str) {
    while (__isspace(*str)) { str++; }

    int sign = 1;
    if (*str == '+' || *str == '-') {
        if (*str == '-') {
            sign *= -1;
        }
        str++;
    }

    int value = 0;
    while (__isdigit(*str)) {
        value *= 10;
        value += (int) (*str - '0');
        str++;
    }

    return (value * sign);
}



static char *__read(FILE *f) {
    /* Null-check...
     * */
    if (!f) { return (0); }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (!size) { return (0); }

    char *data = malloc(size);
    if (!data) { return (0); }
    if (fread(data, sizeof(uint8_t), size, f) != size) { free(data); return (0); }

    return (data);
}


/* SECTION: global objects
 * */

static const uint8_t g_sign_pgm0[] = { 80, 49 },
                     g_sign_pgm1[] = { 80, 52 };

static const uint8_t g_sign_pbm0[] = { 80, 50 },
                     g_sign_pbm1[] = { 80, 53 };

static const uint8_t g_sign_ppm0[] = { 80, 51 },
                     g_sign_ppm1[] = { 80, 54 };

static const uint8_t g_sign_png[] = { 137, 80, 78, 71, 13, 10, 26, 10 };


/* SECTION: image.h api (.pnm)
 * */

enum e_pnmtype {
    T_NONE = 0,

    T_PGM = 1,
    T_PBM = 2,
    T_PPM = 3,
    T_PAM = 4,

    /* ... */

    T_COUNT
};

enum e_pnmmode {
    M_NONE = 0,

    M_HEADER = 1,
    M_DATA   = 2,

    /* ... */

    M_COUNT
};

IMGAPI void *imageLoadPNM(const char *path, int *width, int *height) {
    if (!path)  { goto __failure; }
    if (!*path) { goto __failure; }

    FILE *f = fopen(path, "rb");
    if (!f) { goto __failure; }

    const char *f0 = 0,
               *f1 = 0;

    f0 = __read(f);
    if (!f0) { goto __failure; }
    fclose(f), f = 0;
    f1 = f0;

    enum e_pnmmode mode = M_HEADER;
    enum e_pnmtype type = T_NONE;
    char *line = __getline(f1);
    while (line && *line) {
        /* signature
         * */
        if (*line == 'P') {
            /* corrupted pnm: parsing mode not M_HEADER */
            if (type != M_HEADER) { goto __failure; }

            /* corrupted pnm: multiple signatures */
            if (type != T_NONE) { goto __failure; }

            /* P1, P4 - .pgm file sign. */
            if (
                !__memcmp(f1, g_sign_pgm0, 2) ||
                !__memcmp(f1, g_sign_pgm1, 2)
            ) { type = T_PGM; }

            /* P2, P5 - .pbm file sign. */
            else if (
                !__memcmp(f1, g_sign_pbm0, 2) ||
                !__memcmp(f1, g_sign_pbm1, 2)
            ) { type = T_PBM; }

            /* P3, P6 - .ppm file sign. */
            else if (
                !__memcmp(f1, g_sign_ppm0, 2) ||
                !__memcmp(f1, g_sign_ppm1, 2)
            ) { type = T_PPM; }
            
            /* corrupted pnm: invalid signature */
            else { goto __failure; }
        }

        else { /* ...ignoreme... */ }

        /* move the buffer to the next line by skipping all the bytes of the current line... */
        f1 += __strlen(line) + 1;
        printf("%s\n", f1);
        
        /* release the current line and allocate a new line based on the buffer... */
        free(line), line = __getline(f1);
    }

    if (f0) { free((void *) f0); }
    if (line) { free(line); }

    if (width)  { *width  = 0; }
    if (height) { *height = 0; }
    return (0);

__failure:
    
    if (f0) { free((void *) f0); }
    if (line) { free(line); }

    if (width)  { *width  = 0; }
    if (height) { *height = 0; }
    return (0);
}


/* SECTION: image.h api (.png)
 * */

struct s_chunk {
    uint8_t  *data;
    uint32_t  length;
    uint32_t  type;
    uint32_t  crc;
};

struct s_ihdr {
    uint32_t width;
    uint32_t height;

    uint8_t bit;
    uint8_t type;
    uint8_t comp;
    uint8_t filter;
    uint8_t interlace;
};

struct s_plte {
    uint8_t data[256 * 4];
    size_t  size;
};

struct s_idat {
    uint8_t *data;
    size_t   size;
};

struct s_png {
    struct s_ihdr ihdr;
    struct s_plte plte;
    struct s_idat idat;

    /* TODO:
     *  we should count the occurances of each chunk.
     *  general rules:
     *  - IHDR - first one, only one
     *  - PLTE - after IHDR, before IDAT, only one
     *  - IDAT - after IHDR and PLTE, can be multiple of them
     *  - IEND - after IHDR, PLTE and IDAT('s), only one
     * */
};

static int __chunk(struct s_chunk *, const char *f);

static int __ihdr(struct s_ihdr *, struct s_chunk *);

static int __plte(struct s_plte *, struct s_chunk *);

static int __idat(struct s_idat *, struct s_chunk *);

static int __iend(struct s_png *);


IMGAPI void *imageLoadPNG(const char *path, int *width, int *height) {
    if (!path)  { goto __failure; }
    if (!*path) { goto __failure; }

    FILE *f = fopen(path, "rb");
    if (!f) { goto __failure; }

    const char *f0 = 0,
               *f1 = 0;

    f0 = __read(f);
    if (!f0) { goto __failure; }
    fclose(f), f = 0;
    f1 = f0;

    /* file verification...
     * */
    
    if (__memcmp(f1, g_sign_png, 8)) { goto __failure; }
    f1 += 8;

    /* file parsing...
     * */

    struct s_png png = { 0 };
    struct s_chunk chunk = { 0 };
    while (chunk.type != (uint32_t) __pack32((uint8_t *) "IEND")) {
        f1 += __chunk(&chunk, f1);

        /* IHDR: header */
        if (chunk.type == (uint32_t) __pack32((uint8_t *) "IHDR")) {
            int result = __ihdr(&png.ihdr, &chunk);

            if (!result) { goto __failure; }
        }
        
        /* PLTE: palette */
        else if (chunk.type == (uint32_t) __pack32((uint8_t *) "PLTE")) {
            int result = __plte(&png.plte, &chunk);

            if (!result) { goto __failure; }
        }
        
        /* IDAT: data */
        else if (chunk.type == (uint32_t) __pack32((uint8_t *) "IDAT")) {
            int result = __idat(&png.idat, &chunk);

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

    if (f0) { free((void *) f0); }
    if (png.idat.data) { free(png.idat.data); }

    if (width)  { *width  = png.ihdr.width;  }
    if (height) { *height = png.ihdr.height; }
    return (0);

__failure:
    
    if (f0) { free((void *) f0); }
    if (png.idat.data) { free(png.idat.data); }

    if (width)  { *width  = 0; }
    if (height) { *height = 0; }
    return (0);
}


static int __chunk(struct s_chunk *chunk, const char *str) {
    /* null-check...
     * */
    if (!chunk) { return (0); }

    /* original pointer to the 'str'
     * */
    const char *diff = str;

    /* extract: length */
    chunk->length = __pack32((uint8_t *) str);
    str += 4;
    
    /* extract: type / header */
    chunk->type = __pack32((uint8_t *) str);
    str += 4;

    /* extract: data */
    chunk->data = __memdup(str, chunk->length);
    str += chunk->length;
    
    /* extract: CRC */
    chunk->crc = __pack32((uint8_t *) str);
    str += 4;

    return (str - diff);
}


static int __ihdr(struct s_ihdr *ihdr, struct s_chunk *chunk) {
    /* null-check...
     * */
    if (!ihdr)  { return (0); }
    if (!chunk) { return (0); }

    uint8_t *data = chunk->data;
    if (!data) { return (0); }

    uint32_t width = __pack32(data); data += 4;
    if (!width) { return (0); }
    if (width > (1u << 24)) { return (0); }
    ihdr->width = width;

    uint32_t height = __pack32(data); data += 4;
    if (!height) { return (0); }
    if (height > (1u << 24)) { return (0); }
    ihdr->height = height;

    uint8_t bit = *data++;
    if (!__isinrange(bit, (int32_t []) { 1, 2, 4, 8, 16 }, 5)) { return (0); }
    ihdr->bit = bit;

    uint8_t type = *data++;
    if (!__isinrange(type, (int32_t []) { 0, 2, 3, 4, 6 }, 5)) { return (0); }
    ihdr->type = type;

    uint8_t comp = *data++;
    if (comp) { return (0); }
    ihdr->comp = comp;

    uint8_t filter = *data++;
    if (filter) { return (0); }
    ihdr->filter = filter;

    uint8_t interlace = *data++;
    if (!__isinrange(interlace, (int32_t []) { 0, 1 }, 2)) { return (0); }
    ihdr->interlace = interlace;

    return (1);
}


static int __plte(struct s_plte *plte, struct s_chunk *chunk) {
    /* null-check...
     * */
    if (!plte)  { return (0); }
    if (!chunk) { return (0); }

    size_t length = chunk->length;
    if (length >= 256 * 3) { return (0); }
    
    plte->size = length / 3.0;
    if (plte->size * 3 != length) { return (0); }

    uint8_t *data0 = plte->data,
            *data1 = chunk->data;
    if (!data0) { return (0); }
    if (!data1) { return (0); }

    for (size_t i = 0; i < plte->size; i++) {
        *data0++ = *data1++;
        *data0++ = *data1++;
        *data0++ = *data1++;
        *data0++ = 255;
    }

    return (1);
}


static int __idat(struct s_idat *idat, struct s_chunk *chunk) {
    /* null-check...
     * */
    if (!idat)  { return (0); }
    if (!chunk) { return (0); }

    /* Extract the PNG data...
     * */
    size_t length0 = idat->size,
           length1 = chunk->length;
    if (!length1) { return (0); }
    if (length1 > (1u << 30)) { return (0); }

    uint8_t *data = realloc(idat->data, (length0 + length1) * sizeof(uint8_t));
    if (!data) { return (0); }
    if (!__memcpy(data + length0, chunk->data, length1)) { return (0); }

    idat->data = data;
    idat->size += length1;
    return (1);
}


static int __iend(struct s_png *png) {
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
