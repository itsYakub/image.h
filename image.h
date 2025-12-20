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

IMGAPI unsigned char *imageLoadPNM(const char *, int *, int *);

IMGAPI int imageSavePNM(const char *, const void *, const int, const int);



/* SECTION: image.h api (.png)
 * */

IMGAPI unsigned char *imageLoadPNG(const char *, int *, int *);



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

static inline uint32_t __png_pack16(uint8_t [2]);

static inline uint32_t __png_pack32(uint8_t [4]);

static inline char *__png_read(FILE *);

/* math.h */

static inline uint32_t __png_abs(const int32_t);

/* string.h */

static inline int __png_memcmp(const void *, const void *, size_t);

static inline void *__png_memcpy(void *, const void *, size_t);

static inline void *__png_memdup(const void *, size_t);

static inline void *__png_memchr(const void *, const unsigned char, size_t);

static inline void *__png_memrchr(const void *, const unsigned char, size_t);

static inline size_t __png_strlen(const char *);

static inline int __png_strcmp(const char *, const char *);

/* stdlib.h */

static inline int __png_isspace(int);

static inline int __png_isdigit(int);

static inline int __png_atoi(const char *);


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


IMGAPI unsigned char *imageLoadPNM(const char *path, int *width, int *height) {
    if (!path)  { return (0); }
    if (!*path) { return (0); }

    FILE *file = fopen(path, "rb");
    if (!file) { return (0); }

    char *f = 0,
         *f_ptr = 0;

    f = __png_read(file);
    if (!f) { return (0); }
    fclose(file), file = 0;

    /* __pnm_trim returns either:
     * - original pointer if no comments were found in the file
     * - new trimmed pointer if any comments were found
     * */    
    f_ptr = __pnm_trim(f);
    if (f_ptr != f) {
        free(f);
        f = f_ptr;
    }

    /* magic...
     * */
    enum e_pnmtype type = PNM_NONE;
    while (__png_isspace(*f)) { f++; }
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
    while (*f && !__png_isdigit(*f)) { f++; }
    size_t pnm_w = __png_atoi(f);
    if (!pnm_w) { free(f_ptr); return (0); }
    while (*f && !__png_isspace(*f)) { f++; }
    
    /* height...
     * */
    while (*f && !__png_isdigit(*f)) { f++; }
    size_t pnm_h = __png_atoi(f);
    if (!pnm_h) { free(f_ptr); return (0); }
    while (*f && !__png_isspace(*f)) { f++; }

    /* maxval (only for .pgm and .ppm)
     *        (.pbm defaults to '1')
     * */
    size_t maxval = 1;
    if (type == PNM_PGM || type == PNM_PPM) {
        while (*f && !__png_isdigit(*f)) { f++; }
        maxval = __png_atoi(f);
        if (!maxval) { free(f_ptr); return (0); }
        while (*f && !__png_isspace(*f)) { f++; }
    }

    /* data...
     * */
    size_t i = 0,
           j = pnm_w * pnm_h * 4;
    uint8_t *data = (uint8_t *) malloc(j * sizeof(uint8_t));
    if (!data) { free(f_ptr); return (0); }
    while (i < j) {
        switch (type) {
            case (PNM_PBM):
            case (PNM_PGM): {
                while (*f && !__png_isdigit(*f)) { f++; }
                uint8_t sample = __png_atoi(f);
                if (sample > maxval) {
                    free(f_ptr);
                    free(data);

                    return (0);
                }
                while (*f && !__png_isspace(*f)) { f++; }

                data[i++] = sample;
                data[i++] = sample;
                data[i++] = sample;
                data[i++] = 255;
            } break;

            case (PNM_PPM): {
                for (size_t k = 0; k < 3; k++) {
                    while (*f && !__png_isdigit(*f)) { f++; }
                    uint8_t sample = __png_atoi(f);
                    if (sample > maxval) {
                        free(f_ptr);
                        free(data);

                        return (0);
                    }

                    data[i++] = sample;
                    while (*f && !__png_isspace(*f)) { f++; }
                }
                data[i++] = 255;
            } break;

            default: { } break;
        } 
    }

    free(f_ptr);
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
    enum e_pnmtype type = PNM_NONE;
    const char *ext = (char *) __png_memrchr(path, '.', __png_strlen(path));
    if (!ext) { type = PNM_PPM; } /* if no extension found - default to .ppm */
    else if (!__png_strcmp(ext, ".pbm")) { type = PNM_PBM; }
    else if (!__png_strcmp(ext, ".pgm")) { type = PNM_PGM; }
    else if (!__png_strcmp(ext, ".ppm")) { type = PNM_PPM; }
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
    if (!__png_memchr(s, '#', __png_strlen(s))) { return ((char *) s); }

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

    char *news = (char *) calloc(modlen + 1, sizeof(char));
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

static int __png_chunk(struct s_chunk *, const char *f);

static int __png_chunk_free(struct s_chunk *);


struct s_ihdr {
    uint32_t width;
    uint32_t height;

    uint8_t bit;
    uint8_t type;
    uint8_t comp;
    uint8_t filter;
    uint8_t interlace;
};

static int __png_ihdr(struct s_ihdr *, struct s_chunk *);

static uint8_t *__png_idat(struct s_ihdr *, uint8_t *, const size_t);


struct s_zlib_cmf {
    uint8_t cm: 4;      /* (4bit: 0 - 3) CM    - compression method */
    uint8_t cinfo: 4;   /* (4bit: 4 - 7) cinfo - compression info */
};


struct s_zlib_flg {
    uint8_t fcheck: 5;  /* (5bit: 0 - 4) fcheck - check bits for CMF and FLG ((CMF * 256 + FLG) % 31 == 0) */
    uint8_t fdict: 1;   /* (1bit: 5 - 5) fdict  - if set, DICT dictionary is present immediately after the FLG byte */
    uint8_t flevel: 2;  /* (2bit: 6 - 7) flevel - compression level */
};


struct s_zlib_block_header {
    uint8_t bfinal: 1;  /* (1bit: 0 - 0) bfinal - marks if this is the last zlib block  */
    uint8_t btype: 2;   /* (2bit: 1 - 2) btype  - compression type */
};

static uint8_t *__png_zlib_inflate(uint8_t *, const size_t);

static uint8_t *__png_zlib_inflate_no_compression(uint8_t **, uint8_t *, size_t *);

static uint8_t *__png_zlib_inflate_fixed_huffman(uint8_t **, uint8_t *, size_t *);

static uint8_t *__png_zlib_inflate_dynamic_huffman(uint8_t **, uint8_t *, size_t *);


IMGAPI unsigned char *imageLoadPNG(const char *path, int *width, int *height) {
    if (!path)  { return (0); }
    if (!*path) { return (0); }

    FILE *file = fopen(path, "rb");
    if (!file) { return (0); }

    char *f = 0,
         *f_ptr = 0;

    f = __png_read(file);
    if (!f) { return (0); }
    fclose(file), file = 0;
    f_ptr = f;

    /* signtaure...
     * */
    
    if (__png_memcmp(f, g_sign_png, 8)) { return (0); }
    f += 8;

    /* file parsing...
     * */

    struct s_ihdr ihdr = { };
    struct s_chunk chunk = { };
    while (chunk.type != (uint32_t) __png_pack32((uint8_t *) "IEND")) {
        f += __png_chunk(&chunk, f);

        /* IHDR: header */
        if (chunk.type == (uint32_t) __png_pack32((uint8_t *) "IHDR")) {
            int result = __png_ihdr(&ihdr, &chunk);

            if (!result) {
                __png_chunk_free(&chunk);
                break;
            }
        }
        
        /* IDAT: data */
        else if (chunk.type == (uint32_t) __png_pack32((uint8_t *) "IDAT")) {
            uint8_t *result = __png_idat(&ihdr, chunk.data, chunk.length);

            if (!result) {
                __png_chunk_free(&chunk);
                break;
            }

            /* ... */

            /* FIXME: we should store the 'result' somewhere and realloc it on every 'IDAT' occurance
             * */
            free(result);
        }

        /* IEND: end */
        else if (chunk.type == (uint32_t) __png_pack32((uint8_t *) "IEND")) { break; }

        /* OTHER: ancillary */
        else { }

        __png_chunk_free(&chunk);
    }

    free(f_ptr);
    if (width)  { *width  = ihdr.width; }
    if (height) { *height = ihdr.height; }
    return (0);
}


static int __png_chunk(struct s_chunk *chunk, const char *str) {
    /* null-check...
     * */
    if (!chunk) { return (0); }

    /* original pointer to the 'str'
     * */
    const char *diff = str;

    /* extract: length */
    chunk->length = __png_pack32((uint8_t *) str);
    str += 4;
    
    /* extract: type / header */
    chunk->type = __png_pack32((uint8_t *) str);
    str += 4;

    /* extract: data */
    chunk->data = (uint8_t *) __png_memdup(str, chunk->length);
    str += chunk->length;
    
    /* extract: CRC */
    chunk->crc = __png_pack32((uint8_t *) str);
    str += 4;

    return (str - diff);
}


static int __png_chunk_free(struct s_chunk *chunk) {
    /* null-check...
     * */
    if (!chunk) { return (0); }

    if (chunk->data)   { free(chunk->data), chunk->data = 0; }
    if (chunk->length) { chunk->length = 0; }
    if (chunk->type)   { chunk->type   = 0; }
    if (chunk->crc)    { chunk->crc    = 0; }
    return (1);
}


static int __png_ihdr(struct s_ihdr *ihdr, struct s_chunk *chunk) {
    /* null-check...
     * */
    if (!ihdr)  { return (0); }
    if (!chunk) { return (0); }

    uint8_t *data = chunk->data;
    if (data == 0) { return (0); }

    uint32_t width = __png_pack32(data); data += 4;
    if (width == 0 ||
        width >= (1u << 24)) { return (0); }
    ihdr->width = width;

    uint32_t height = __png_pack32(data); data += 4;
    if (height == 0 ||
        height >= (1u << 24)) { return (0); }
    ihdr->height = height;

    uint8_t bit = *data++;
    if (bit != 1 &&
        bit != 2 &&
        bit != 4 &&
        bit != 8 &&
        bit != 16) { return (0); }
    ihdr->bit = bit;

    uint8_t type = *data++;
    if (type != 0 &&
        type != 2 &&
        type != 3 &&
        type != 4 &&
        type != 6) { return (0); }
    ihdr->type = type;

    uint8_t comp = *data++;
    if (comp != 0) { return (0); }
    ihdr->comp = comp;

    uint8_t filter = *data++;
    if (filter != 0) { return (0); }
    ihdr->filter = filter;

    uint8_t interlace = *data++;
    if (interlace != 0 &&
        interlace != 1) { return (0); }
    ihdr->interlace = interlace;

    return (1);
}


static uint8_t *__png_idat(struct s_ihdr *ihdr, uint8_t *indata, const size_t insize) {
    /* null-check...
     * */
    if (!ihdr)   { return (0); }
    if (!indata) { return (0); }

    struct s_zlib_cmf cmf = {
        .cm = (uint8_t) (*indata),
        .cinfo = (uint8_t) (*indata >> 4)
    };

    indata++;
    struct s_zlib_flg flg = {
        .fcheck = (uint8_t) (*indata),
        .fdict = (uint8_t) (*indata >> 5),
        .flevel = (uint8_t) (*indata >> 6)
    };

    (void) cmf;
    (void) flg;
    uint8_t *data = __png_zlib_inflate(++indata, insize - 2);
    if (!data) { return (0); }

    /* ... */

    return (data);
}


static uint8_t *__png_zlib_inflate(uint8_t *indata, const size_t insize) {
    /* null-check...
     * */
    if (!indata) { return (0); }

    uint8_t *outdata = 0;
    size_t   outsize = 0;

    struct s_zlib_block_header block_header = { };
    do {
        /* extract the block header from the current byte... */
        block_header.bfinal = *indata, *indata >>= 1;
        block_header.btype  = *indata, *indata >>= 2;

        switch (block_header.btype) {
            /* 00 - no compression */
            case (0): {
                outdata = __png_zlib_inflate_no_compression(&indata, outdata, &outsize);
            } break;

            /* 01 - compressed with fixed Huffman codes */
            case (1): {
                outdata = __png_zlib_inflate_fixed_huffman(&indata, outdata, &outsize);
            } break;

            /* 10 - compressed with dynamic Huffman codes */
            case (2): {
                outdata = __png_zlib_inflate_dynamic_huffman(&indata, outdata, &outsize);
            } break;

            /* 11 - reserved (error) */
            case (3): { }
        }
    } while (!block_header.bfinal);
    /* iterate over the zlib block until BFINAL isn't set... */

    (void) insize;
    return (outdata);
}


static uint8_t *__png_zlib_inflate_no_compression(uint8_t **src, uint8_t *dst, size_t *size) {
    uint16_t len  = **src; **src >>= 16;
    uint16_t nlen = **src; **src >>= 16;

    (void) dst;
    (void) size;
    (void) len;
    (void) nlen;
    return (dst);
}


static uint8_t *__png_zlib_inflate_fixed_huffman(uint8_t **src, uint8_t *dst, size_t *size) {
    /* TODO: implement */
    
    (void) src;
    (void) dst;
    (void) size;
    return (dst);
}


static uint8_t *__png_zlib_inflate_dynamic_huffman(uint8_t **src, uint8_t *dst, size_t *size) {
    /* TODO: implement */
    
    (void) src;
    (void) dst;
    (void) size;
    return (dst);
}





/* SECTION: static functions
 * */

static inline uint32_t __png_pack16(uint8_t data[2]) {

#  if (LITTLE_ENDIAN)
    return (data[1] | (data[0] << 8));
#  else
    return (data[0] | (data[1] << 8));
#  endif /* LITTLE_ENDIAN */

}


static inline uint32_t __png_pack32(uint8_t data[4]) {

#  if (LITTLE_ENDIAN)
    return (data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24));
#  else
    return (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
#  endif /* LITTLE_ENDIAN */

}


static inline char *__png_read(FILE *f) {
    /* Null-check...
     * */
    if (!f) { return (0); }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (!size) { return (0); }

    char *data = (char *) calloc(size + 1, sizeof(char));
    if (!data) { return (0); }
    if (fread(data, sizeof(uint8_t), size, f) != size) { free(data); return (0); }

    return (data);
}


static inline uint32_t __png_abs(const int32_t i) {
    return (i < 0 ? i * -1 : i);
}


static inline int __png_memcmp(const void *s0, const void *s1, size_t n) {
    uint8_t *c0 = (uint8_t *) s0,
                  *c1 = (uint8_t *) s1;

    while (n--) {
        if (*c0 != *c1) { return (*c0 - *c1); }
        c0++;
        c1++;
    }
    return (0);
}


static inline void *__png_memcpy(void *dst, const void *src, size_t n) {
    uint8_t *c0 = (uint8_t *) src,
            *c1 = (uint8_t *) dst;

    while (n--) {
        *c1++ = *c0++;
    }
    return (dst);
}


static inline void *__png_memdup(const void *s0, size_t s) {
    if (!s0) { return (0); }
    if (!s)  { return (0); }

    void *s1 = malloc(s);
    if (!s1) { return (0); }

    return (__png_memcpy(s1, s0, s));
}


static inline void *__png_memchr(const void *s, const unsigned char c, size_t n) {
    uint8_t *c0 = (uint8_t *) s;
    if (!c0) { return (0); }

    while (n--) {
        if (*c0 == c) { return ((void *) c0); }
        c0++;
    }
    return (0);
}


static inline void *__png_memrchr(const void *s, const unsigned char c, size_t n) {
    uint8_t *c0 = (uint8_t *) s;
    if (!c0) { return (0); }

    while (n-- > 0) {
        if (c0[n] == c) {
            return ((void *) &c0[n]);
        }
    }
    return (0);
}


static inline size_t __png_strlen(const char *s) {
    for (size_t i = 0; s; i++) {
        if (!s[i]) { return (i); }
    }
    return (0);
}


static inline int __png_strcmp(const char *s0, const char *s1) {
    while (*s0 && *s1 && *s0 == *s1) {
        s0++;
        s1++;
    }
    return (*s0 - *s1);
}


static inline int __png_isspace(int c) {
    return ((c >= '\t' && c <= '\r') || c == ' ');
}


static inline int __png_isdigit(int c) {
    return (c >= '0' && c <= '9');
}


static inline int __png_atoi(const char *str) {
    while (__png_isspace(*str)) { str++; }

    int sign = 1;
    if (*str == '+' || *str == '-') {
        if (*str == '-') {
            sign *= -1;
        }
        str++;
    }

    int value = 0;
    while (__png_isdigit(*str)) {
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
