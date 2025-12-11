# image.h

## Example:

```c

/* Example of using image.h with standard PNG file... */

#define IMAGE_IMPLEMENTATION
#include "image.h"

int main(void) {
    int width = 0, height = 0;

    char *image = imageLoadPNG("image.png", &width, &height);

    /* ... */

    free(image);
}
```

## Resources:
- **[PNG (Portable Network Graphics) Specification, Version 1.2](https://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html)**
- **[Writing a (simple) PNG decoder might be easier than you think](https://pyokagan.name/blog/2019-10-14-png/)**
- **[PNG - Wikipedia](https://en.wikipedia.org/wiki/PNG)**
- **[Implementing a Basic PNG reader the handmade way](https://handmade.network/forums/articles/t/2363-implementing_a_basic_png_reader_the_handmade_way)**
- **[How PNG Works: Compromising Speed for Quality - Reducible](https://youtu.be/EFUYNoFRHQI)**

- **[PPM - Netpbm color image format](https://netpbm.sourceforge.net/doc/ppm.html)**
- **[Portable anymap](https://pl.wikipedia.org/wiki/Portable_anymap)**
