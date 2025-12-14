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

IMGAPI int imageSavePNM(const char *, const void *, const int, const int);



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
#  /* FIXME: replace this dependency with handmade implementation of zlib
#   * */
#  include <zlib.h>
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

/* custom */

static inline int32_t __pack16(uint8_t [2]);

static inline int32_t __pack32(uint8_t [4]);

static int __isinrange(const int32_t, const int32_t [], const size_t);

static char *__read(FILE *);

/* math.h */

static uint32_t __abs(const int32_t);

/* string.h */

static int __memcmp(const void *, const void *, size_t);

static void *__memcpy(void *, const void *, size_t);

static void *__memdup(const void *, size_t);

static void *__memchr(const void *, const unsigned char, size_t);

static void *__memrchr(const void *, const unsigned char, size_t);

static size_t __strlen(const char *);

static int __strcmp(const char *, const char *);

/* stdlib.h */

static int __isspace(int);

static int __isdigit(int);

static int __atoi(const char *);


/* SECTION: global objects
 * */

static const uint8_t g_sign_png[] = { 137, 80, 78, 71, 13, 10, 26, 10 };





/* SECTION: image.h api (.pnm)
 * */

enum e_pnmtype {
    PNM_NONE = 0, /* initial state */

    PNM_PBM = 1,  /* P1, P4 */
    PNM_PGM = 2,  /* P2, P5 */
    PNM_PPM = 3,  /* P3, P6 */

    /* ... */

    PNM_COUNT
};

static inline char *__pnm_trim(const char *); 


IMGAPI void *imageLoadPNM(const char *path, int *width, int *height) {
    if (!path)  { return (0); }
    if (!*path) { return (0); }

    FILE *file = fopen(path, "rb");
    if (!file) { return (0); }

    const char *f = 0,
               *f_ptr = 0;

    f = __read(file);
    if (!f) { return (0); }
    fclose(file), file = 0;

    /* __pnm_trim returns either:
     * - original pointer if no comments were found in the file
     * - new trimmed pointer if any comments were found
     * */    
    f_ptr = __pnm_trim(f);
    if (f_ptr != f) {
        free((void *) f);
        f = f_ptr;
    }

    /* magic...
     * */
    enum e_pnmtype type = { 0 };
    while (__isspace(*f)) { f++; }
    if (*f++ != 'P') { return (0); }
    switch (*f++) {
        case ('1'):
        case ('4'): {
            type = PNM_PBM;
        } break;
        
        case ('2'):
        case ('5'): {
            type = PNM_PGM;
        } break;
        
        case ('3'):
        case ('6'): {
            type = PNM_PPM;
        } break;
        
        default: { return (0); }
    }

    /* width...
     * */
    while (*f && !__isdigit(*f)) { f++; }
    size_t pnm_w = __atoi(f);
    if (!pnm_w) { free((void *) f_ptr); return (0); }
    while (*f && !__isspace(*f)) { f++; }
    
    /* height...
     * */
    while (*f && !__isdigit(*f)) { f++; }
    size_t pnm_h = __atoi(f);
    if (!pnm_h) { free((void *) f_ptr); return (0); }
    while (*f && !__isspace(*f)) { f++; }

    /* maxval (only for .pgm and .ppm)
     *        (.pbm defaults to '1')
     * */
    size_t maxval = 1;
    if (type == PNM_PGM || type == PNM_PPM) {
        while (*f && !__isdigit(*f)) { f++; }
        maxval = __atoi(f);
        if (!maxval) { free((void *) f_ptr); return (0); }
        while (*f && !__isspace(*f)) { f++; }
    }

    /* data...
     * */
    size_t i = 0,
           j = pnm_w * pnm_h * 4;
    uint8_t *data = malloc(j * sizeof(uint8_t));
    if (!data) { free((void *) f_ptr); return (0); }
    while (i < j) {
        switch (type) {
            case (PNM_PBM):
            case (PNM_PGM): {
                while (*f && !__isdigit(*f)) { f++; }
                uint8_t sample = __atoi(f);
                if (sample > maxval) {
                    free((void *) f_ptr);
                    free(data);

                    return (0);
                }
                while (*f && !__isspace(*f)) { f++; }

                data[i++] = sample;
                data[i++] = sample;
                data[i++] = sample;
                data[i++] = 255;
            } break;

            case (PNM_PPM): {
                for (size_t k = 0; k < 3; k++) {
                    while (*f && !__isdigit(*f)) { f++; }
                    uint8_t sample = __atoi(f);
                    if (sample > maxval) {
                        free((void *) f_ptr);
                        free(data);

                        return (0);
                    }

                    data[i++] = sample;
                    while (*f && !__isspace(*f)) { f++; }
                }
                data[i++] = 255;
            } break;

            default: { } break;
        } 
    }

    free((void *) f_ptr);
    if (width)  { *width  = pnm_w; }
    if (height) { *height = pnm_h; }
    return (data);
}


IMGAPI int imageSavePNM(const char *path, const void *data, const int width, const int height) {
    /* Null-check...
     * */
    if (!data) { return (0); }
    if (!path) { return (0); }

    uint8_t *d = (uint8_t *) data;
    size_t   s = width * height * 4;

    /* Dimensions check...
     * */
    if (!width)  { return (0); }
    if (!height) { return (0); }

    /* figure-out the file type...
     * */
    enum e_pnmtype type = { 0 };
    const char *ext = __memrchr(path, '.', __strlen(path));
    if (!ext) { type = PNM_PPM; } /* if no extension found - default to .ppm */
    else if (!__strcmp(ext, ".pbm")) { type = PNM_PBM; }
    else if (!__strcmp(ext, ".pgm")) { type = PNM_PGM; }
    else if (!__strcmp(ext, ".ppm")) { type = PNM_PPM; }
    else { type = PNM_PPM; } /* if no extension matched - default to .ppm */

    /* figure-out maxval...
     * */
    uint8_t maxval = 1;
    if (type == PNM_PGM || type == PNM_PPM) {
        for (size_t i = 0; i < s; i++) {
            if (i % 4 == 0) { continue; }

            maxval = (d[i] > maxval ? d[i] : maxval);
        }
    }

    /* Create and fill the image file...
     * */
    FILE *f = fopen(path, "wb");
    if (!f) { return (0); }

    switch (type) {
        case (PNM_PBM): { fprintf(f, "P1\n"); } break;
        case (PNM_PGM): { fprintf(f, "P2\n"); } break;
        case (PNM_PPM): { fprintf(f, "P3\n"); } break;
        default: { return (0); }
    }

    fprintf(f, "%d %d\n", width, height);
    if (type == PNM_PGM || type == PNM_PPM) {
        fprintf(f, "%d\n", maxval);
    }

    switch (type) {
        case (PNM_PBM): {
            for (size_t i = 0; i < s; i += 4) {
                fprintf(f, "%d ", d[i] > 0 ? 1 : 0);
            }
        } break;

        case (PNM_PGM): {
            for (size_t i = 0; i < s; i += 4) {
                uint8_t sample = (d[i] + d[i + 1] + d[i + 2]) / 3;

                fprintf(f, "%d ", sample);
            }
        } break;

        case (PNM_PPM): {
            for (size_t i = 0; i < s; i++) {
                if (i % 4 == 0) { continue; }
                
                fprintf(f, "%d ", d[i]);
            }
        } break;

        default: { return (0); }
    }

    fclose(f);
    return (1);
}


static inline char *__pnm_trim(const char *s) {
    /* Null-check...
     * */
    if (!s)  { return (0); }
    if (!*s) { return (0); }

    /* if no comments found, we don't need to perform any comment trimming...
     * */
    if (!__memchr(s, '#', __strlen(s))) { return ((char *) s); }

    /* this is gonna be kind of wasteful on time complexity, but whatever:
     * - iterate over string to get the size of if without comment lines
     * - perform single allocation with that size
     * - iterate again, copying the original string without comment lines
     * */
    size_t modlen = 0;
    for (size_t i = 0; s[i]; i++, modlen++) {
        if (s[i] == '#') {
            while (s[i] && s[i] != '\n') { i++; }
        }
    }

    char *news = calloc(modlen + 1, sizeof(char));
    if (!news) { return (0); }

    for (size_t i = 0; *s; i++, s++) {
        if (*s == '#') {
            while (*s && *s != '\n') { s++; }
        }
        news[i] = *s;
    }

    return (news);
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

/* Structure of zlib block:
 *
 * + - +
 * |...| <- one-byte block (8bit)
 * + - +
 *
 * + - + - + - + - +
 * |...|...|...|...| <- four-byte block (32bit)
 * + - + - + - + - +
 * 
 * + - + - + ------------------ + - + - + - + - +
 * |CMF|FLG| compressed data... | ..|.ad|ler|32  |
 * + - + - + ------------------ + - + - + - + - +
 * */
struct s_zlib {
    uint8_t cmf;    /* cmf - compression method and flags (8bit):
                     *  > bits from 0 to 3: CM    - compression method (4bits)
                     *  > bits from 4 to 7: CINFO - compression info (4bits)
                     * */

    uint8_t flg;    /* flg - flags (8bit):
                     *  > bits from 0 to 4: FCHECK - check bits for CMF and FLG (5bits)
                     *  >            bit 5: FDICT  - preset dictionary (1bit)
                     *  > bits from 6 to 7: FLEVEL - compression level (2bits)
                     * */
};


static int __png_chunk(struct s_chunk *, const char *f);

static int __png_ihdr(struct s_ihdr *, struct s_chunk *);

static int __png_plte(struct s_plte *, struct s_chunk *);

static int __png_idat(struct s_idat *, struct s_chunk *);

static uint8_t *__png_iend(struct s_png *);

static uint8_t *__png_zlib_inflate(const uint8_t *, const size_t, const size_t);

static uint8_t *__png_zlib_unfilter(const uint8_t *, const size_t, const size_t, const size_t);

static uint8_t __png_paeth_predictor(const uint8_t, const uint8_t, const uint8_t);


IMGAPI void *imageLoadPNG(const char *path, int *width, int *height) {
    if (!path)  { return (0); }
    if (!*path) { return (0); }

    FILE *file = fopen(path, "rb");
    if (!file) { return (0); }

    const char *f = 0,
               *f_ptr = 0;

    f = __read(file);
    if (!f) { return (0); }
    fclose(file), file = 0;
    f_ptr = f;

    /* signtaure...
     * */
    
    if (__memcmp(f, g_sign_png, 8)) { return (0); }
    f += 8;

    /* file parsing...
     * */

    uint8_t *data = 0;
    struct s_png png = { 0 };
    struct s_chunk chunk = { 0 };
    while (chunk.type != (uint32_t) __pack32((uint8_t *) "IEND")) {
        f += __png_chunk(&chunk, f);

        /* IHDR: header */
        if (chunk.type == (uint32_t) __pack32((uint8_t *) "IHDR")) {
            int result = __png_ihdr(&png.ihdr, &chunk);

            if (!result) { return (0); }
        }
        
        /* PLTE: palette */
        else if (chunk.type == (uint32_t) __pack32((uint8_t *) "PLTE")) {
            int result = __png_plte(&png.plte, &chunk);

            if (!result) { return (0); }
        }
        
        /* IDAT: data */
        else if (chunk.type == (uint32_t) __pack32((uint8_t *) "IDAT")) {
            int result = __png_idat(&png.idat, &chunk);

            if (!result) { return (0); }
        }

        /* IEND: end */
        else if (chunk.type == (uint32_t) __pack32((uint8_t *) "IEND")) {
            data = __png_iend(&png);

            if (!data) {
                free(chunk.data);
                free(png.idat.data);
                free((void *) f_ptr);
                return (0);
            }
        }

        /* OTHER: ancillary */
        else { }

        if (chunk.data) { free(chunk.data), chunk.data = 0; }
    }

    free((void *) f_ptr);
    if (png.idat.data) { free(png.idat.data); }

    if (width)  { *width  = png.ihdr.width;  }
    if (height) { *height = png.ihdr.height; }
    return (data);
}


static int __png_chunk(struct s_chunk *chunk, const char *str) {
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


static int __png_ihdr(struct s_ihdr *ihdr, struct s_chunk *chunk) {
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


static int __png_plte(struct s_plte *plte, struct s_chunk *chunk) {
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


static int __png_idat(struct s_idat *idat, struct s_chunk *chunk) {
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


static uint8_t *__png_iend(struct s_png *png) {
    /* null-check...
     * */
    if (!png)            { return (0); }
    if (!png->idat.data) { return (0); }
    if (!png->idat.size) { return (0); }

    uint8_t channels  = 0;
    switch (png->ihdr.type) {
        case (0): { channels = 1; } break;
        case (2): { channels = 3; } break;
        case (6): { channels = 4; } break;
        default:  { return (0); }
    }

    size_t width  = png->ihdr.width,
           height = png->ihdr.height,
           stride = channels;
    size_t scanline = width * stride,
           filtered = height * (1 + scanline);

    /* inflate... */
    uint8_t *fdata = __png_zlib_inflate(png->idat.data, png->idat.size, filtered);
    if (!fdata) {
        return (0);
    }

    /* unfilter... */
    uint8_t *data = __png_zlib_unfilter(fdata, height, scanline, stride);
    free(fdata);
    if (!data) {
        return (0);
    }

    return (data);
}


static uint8_t *__png_zlib_inflate(const uint8_t *data, const size_t size, const size_t filtered_size) {
    uint8_t *outdata = malloc(filtered_size * sizeof(uint8_t));
    if (!outdata) { return (0); }

    return (outdata);
}


static uint8_t *__png_zlib_unfilter(const uint8_t *data, const size_t height, const size_t scanline, const size_t stride) {
    uint8_t *outdata = malloc(height * scanline * sizeof(uint8_t));
    if (!outdata) {
        return (0);
    }

    uint8_t *prev_l = malloc(scanline * sizeof(uint8_t));
    if (!prev_l) {
        return (0);
    }

    for (size_t y = 0; y < height; y++) {
        uint8_t type = data[y * (1 + scanline)];
        if (type > 4) {
            free(prev_l);
            free(outdata);
            return (0);
        }

        uint8_t *curr_l = outdata + y * scanline;
        uint8_t *filt_l = (uint8_t *) data + y * (1 + scanline) + 1;
        for (size_t i = 0; i < scanline; i++) {
            uint8_t p = 0;
            uint8_t a = (i >= stride) ? curr_l[i - stride] : 0,
                    b = prev_l[i],
                    c = (i >= stride) ? prev_l[i - stride] : 0;

            switch (type) {
                case (1): { p = a; } break;
                case (2): { p = b; } break;
                case (3): { p = (a + b) / 2; } break;
                case (4): { p = __png_paeth_predictor(a, b, c); } break;

                default: { p = 0; } break;
            }

            curr_l[i] = filt_l[i] + p;
        }

        if (!__memcpy(prev_l, curr_l, scanline)) {
            free(prev_l);
            free(outdata);
            return (0);
        }
    }

    free(prev_l);
    return (outdata);
}


static uint8_t __png_paeth_predictor(const uint8_t a, const uint8_t b, const uint8_t c) {
    int32_t p  = a + b - c,
            pa = __abs(p - a),
            pb = __abs(p - b),
            pc = __abs(p - c);

    if (pa <= pb && pa <= pc) { return (a); }
    if (pb <= pc)             { return (b); }
    else                      { return (c); }
}





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


static int __isinrange(const int32_t v, const int32_t arr[], const size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (v == arr[i]) {
            return (1);
        }
    }
    return (0);
}


static char *__read(FILE *f) {
    /* Null-check...
     * */
    if (!f) { return (0); }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (!size) { return (0); }

    char *data = calloc(size + 1, sizeof(char));
    if (!data) { return (0); }
    if (fread(data, sizeof(uint8_t), size, f) != size) { free(data); return (0); }

    return (data);
}


static uint32_t __abs(const int32_t i) {
    return (i < 0 ? i * -1 : i);
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


static void *__memchr(const void *s, const unsigned char c, size_t n) {
    const uint8_t *c0 = (const uint8_t *) s;
    if (!c0) { return (0); }

    while (n--) {
        if (*c0 == c) { return ((void *) c0); }
        c0++;
    }
    return (0);
}


static void *__memrchr(const void *s, const unsigned char c, size_t n) {
    const uint8_t *c0 = (const uint8_t *) s;
    if (!c0) { return (0); }

    while (n-- > 0) {
        if (c0[n] == c) {
            return ((void *) &c0[n]);
        }
    }
    return (0);
}


static size_t __strlen(const char *s) {
    for (size_t i = 0; s; i++) {
        if (!s[i]) { return (i); }
    }
    return (0);
}


static int __strcmp(const char *s0, const char *s1) {
    while (*s0 && *s1 && *s0 == *s1) {
        s0++;
        s1++;
    }
    return (*s0 - *s1);
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

#  if defined (__cplusplus)

}


#  endif /* __cplusplus */
#
# endif /* IMAGE_IMPLEMENTATION */
#endif /* _image_h_ */
