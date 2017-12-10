/******************************************************************************\
|* Copyright (c) 2014- by Tencent Corporation.  All Rights Reserved.      *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of Tencent Corporation.  This is proprietary information owned by Tencent  *|
|* Corporation.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of Tencent Corporation.                                 *|
|*                                                                            *|
\******************************************************************************/

#include "./Bitmap.h"

#if defined(__linux__) || defined(__APPLE__) || defined(__QNXNTO__)

#   include <stdint.h>
typedef uint8_t         BYTE;
typedef uint32_t        BOOL;
typedef uint32_t        DWORD;
typedef uint16_t        WORD;
typedef uint32_t        LONG;

#   define BI_RGB           0L
#   define BI_RLE8          1L
#   define BI_RLE4          2L
#   define BI_BITFIELDS     3L

typedef struct tagBITMAPINFOHEADER
{
        DWORD   biSize;
        LONG    biWidth;
        LONG    biHeight;
        WORD    biPlanes;
        WORD    biBitCount;
        DWORD   biCompression;
        DWORD   biSizeImage;
        LONG    biXPelsPerMeter;
        LONG    biYPelsPerMeter;
        DWORD   biClrUsed;
        DWORD   biClrImportant;
}
__attribute__((packed)) BITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER
{
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
}
__attribute__((packed)) BITMAPFILEHEADER;

#elif defined(_WIN32)
#   include <windows.h>
typedef DWORD64 uint64_t;
typedef DWORD32 uint32_t;
typedef WORD  uint16_t;
typedef BYTE  uint8_t;
#else
#   error Platform Not supported
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


static int _big_endian = -1;

static void _check_endian()
{
    unsigned long v = 0x01020304;
    _big_endian = ((unsigned char *) &v)[0] == 0x01;
}

static unsigned short _le16(unsigned short word)
{
    if (_big_endian)
    {
        word = ((word & 0x00ff) << 8) |
               ((word & 0xff00) >> 8);
    }

    return word;
}

static unsigned long _le32(unsigned long dword)
{
    if (_big_endian)
    {
        dword = ((dword & 0x000000ff) << 24) |
                ((dword & 0x0000ff00) << 8)  |
                ((dword & 0x00ff0000) >> 8)  |
                ((dword & 0xff000000) >> 24);
    }

    return dword;
}

int WriteBitmap(const char * filename,
                const void * data,
                unsigned int width,
                unsigned int height,
                unsigned int stride,
                int format,
                int flip)
{
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;

    FILE * fp = NULL;
    unsigned int i;
    unsigned short  bypp;
    /* Buffer for dest data line. */
    unsigned char * buff = NULL;
    /* Source data line. */
    const unsigned char * line;
    /* Dest stride. */
    unsigned int    dstride;

    /* Check endian. */
    _check_endian();

    /* Check filename. */
    if (filename == NULL)
    {
        fprintf(stderr, "filename is NULL.\n");
        return -EINVAL;
    }

    /* Check data. */
    if (data == NULL)
    {
        fprintf(stderr, "Pixels data is empty.\n");
        return -EINVAL;
    }

    /* Check width and height. */
    if (width == 0 || height == 0)
    {
        fprintf(stderr, "Width: %d, Height: %d\n", width, height);
        return -EINVAL;
    }

    /* Check source format and get bytes per pixel. */
    switch (format)
    {
    case RGBA_8888:
    case BGRA_8888:
    case RGBX_8888:
    case BGRX_8888:
        bypp = 4;
        break;

    case RGB_888:
    case BGR_888:
        bypp = 3;
        break;

    case RGB_565:
    case BGR_565:
    case RGBA_4444:
    case BGRA_4444:
    case ARGB_1555:
        bypp = 2;
        break;

    case L8:
        bypp = 1;
        break;

    default:
        fprintf(stderr, "Invalid format: %d\n", format);
        return -EINVAL;
    }

    /* Check source stride. */
    if (stride == 0)
    {
        /* Source is contiguous. */
        stride = width * bypp;
    }
    else
    /* Source has stride. */
    if (stride < width * bypp)
    {
        printf("Invalid stride: %d\n", stride);
        return -EINVAL;
    }

    /* Compute dest bypp. */
    switch (format)
    {
    case RGBA_8888:
    case BGRA_8888:
        bypp = 4;
        break;

    case ARGB_1555:
        bypp = 2;
        break;

    default:
        /* Convert to BGR 888 for other formats. */
        bypp = 3;
        break;
    }

    /* Compute number of bytes per dstride in bitmap file: align by 4. */
    dstride = ((width * bypp) + 3) & ~3;

    /* Build file header. */
    file_header.bfType        = _le16(0x4D42);
    file_header.bfSize        = _le32(
        sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER)
            + dstride * height);
    file_header.bfReserved1   = _le16(0);
    file_header.bfReserved2   = _le16(0);
    file_header.bfOffBits     = _le32(
        sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER));

    /* Build info_header. */
    memset(&info_header, 0, sizeof (BITMAPINFOHEADER));
    info_header.biSize        = _le32(sizeof (BITMAPINFOHEADER));
    info_header.biWidth       = _le32(width);
    info_header.biHeight      = _le32(height);
    info_header.biPlanes      = _le16(1);
    info_header.biBitCount    = _le16(bypp * 8);
    info_header.biCompression = _le32(BI_RGB);
    info_header.biSizeImage   = _le32(dstride * height);

    /* Open dest file. */
    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        fprintf(stderr, "Can not open %s for write.\n", filename);
        return -EACCES;
    }

    /* Write file header. */
    if (fwrite(&file_header, 1, sizeof (BITMAPFILEHEADER), fp)
        != sizeof (BITMAPFILEHEADER))
    {
        fprintf(stderr, "Can not write file header.\n");
        return -EACCES;
    }

    /* Write info header. */
    if (fwrite(&info_header, 1, sizeof (BITMAPINFOHEADER), fp)
        != sizeof (BITMAPINFOHEADER))
    {
        fprintf(stderr, "Can not write info header.\n");
        return -EACCES;
    }

    /* Malloc memory for single dstride. */
    buff = (unsigned char *) malloc(dstride);

    if (buff == NULL)
    {
        fprintf(stderr, "Out of memory.\n");
        return -ENOMEM;
    }

    /* Go to last line.
     * Bitmap is bottom-top, our date is top-bottom. */
    if(flip == 1)
    line = (const unsigned char *) data + stride * (height - 1);
    else
    line = (const unsigned char *) data;

    for (i = 0; i < height; i++)
    {
        /* Process each line. */
        switch (format)
        {
        case RGBA_8888:
            /* Swap RB to BGRA 8888. */
            fconv(buff, BGRA_8888, line, RGBA_8888, width);
            break;

        case BGRA_8888:
            /* No conversion BGRA 8888. */
            memcpy(buff, line, width * 4);
            break;

        case RGBX_8888:
            /* Swap RB and skip A to BGR 888. */
            fconv(buff, BGR_888, line, RGBX_8888, width);
            break;

        case BGRX_8888:
            /* Skip A to BGR 888. */
            fconv(buff, BGR_888, line, BGRX_8888, width);
            break;

        case RGB_888:
            /* Swap RB to BGR 888. */
            fconv(buff, BGR_888, line, RGB_888, width);
            break;

        case BGR_888:
            /* No conversion: BGR 888. */
            fconv(buff, BGR_888, line, BGR_888, width);
            break;

        case RGB_565:
            /* Convert to BGR 888. */
            fconv(buff, BGR_888, line, RGB_565, width);
            break;

        case BGR_565:
            /* Convert to BGR 888. */
            fconv(buff, BGR_888, line, BGR_565, width);
            break;

        case RGBA_4444:
            /* Convert to BGR 888. */
            fconv(buff, BGR_888, line, RGBA_4444, width);
            break;

        case BGRA_4444:
            /* Convert to BGR 888. */
            fconv(buff, BGR_888, line, BGRA_4444, width);
            break;

        case L8:
            /* Convert to BGR 888. */
            fconv(buff, BGR_888, line, L8, width);
            break;

        case ARGB_1555:
            /* No conversion: ARGB 15555. */
            memcpy(buff, line, width * 2);
            break;
        }

        /* Write a line. */
        if (fwrite(buff, 1, dstride, fp) != dstride)
        {
            fprintf(stderr, "Error: Can not write file\n");
            fprintf(stderr, "Bitmap is incomplete.\n");

            /* Difficult to recover. */
            free(buff);
            fclose(fp);

            return -EACCES;
        }

        /* Go to next source line. */
        if(flip == 1)
        line -= stride;
        else
        line += stride;
    }

    /* Free buffer. */
    free(buff);

    /* Close file. */
    fclose(fp);

    return 0;
}

void* ReadBitmap(const char * filename,
                 unsigned int * width,
                 unsigned int * height,
                 int * format)
{
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;

    FILE * fp = NULL;
    unsigned int i;
    unsigned int w, h, bypp;
    /* Whole output pixels. */
    unsigned char * pixels = NULL;
    /* Source(bitmap) stride. */
    unsigned int    stride;
    /* Dest(linear memory) stride. */
    unsigned int    dstride;
    /* Source padding bytes (stride - dstride). */
    unsigned int    padding;

    /* Check endian. */
    _check_endian();

    /* Check filename. */
    if (filename == NULL)
    {
        fprintf(stderr, "filename is NULL.\n");
        return NULL;
    }

    /* Try to open file. */
    fp = fopen(filename, "rb");

    if (fp == NULL)
    {
        fprintf(stderr, "Can not open %s for read.\n", filename);
        return NULL;
    }

    do
    {
        /* Read file header. */
        if (fread(&file_header, 1, sizeof (BITMAPFILEHEADER), fp)
            != sizeof (BITMAPFILEHEADER))
        {
            fprintf(stderr, "Can not read file header.\n");
            break;
        }

        /* Check magic number. */
        if (file_header.bfType != 0x4D42)
        {
            fprintf(stderr, "Not a BMP file.\n");
            break;
        }

        /* Read info hreader. */
        if (fread(&info_header, 1, sizeof (BITMAPINFOHEADER), fp)
            != sizeof (BITMAPINFOHEADER))
        {
            fprintf(stderr, "Can not read info header.\n");
            break;
        }

        /* Check info header size. */
        if (_le32(info_header.biSize) != sizeof (BITMAPINFOHEADER))
        {
            fprintf(stderr, "Invalid bitmap file.\n");
            break;
        }

        /* Get width/height/bypp. */
        w    = _le32(info_header.biWidth);
        h    = _le32(info_header.biHeight);
        bypp = _le16(info_header.biBitCount) / 8;

        /* Compute dest stride. */
        dstride = w * bypp;

        /* Compute stride. */
        stride  = (dstride + 3) & ~3;

        /* Compute padding. */
        padding = stride - dstride;

        /* malloc pixels. */
        pixels = (unsigned char * ) malloc(w * h * bypp);

        /* Read pixels. */
        for (i = 0; i < h; i++)
        {
            /* Read pixels line by line. */
            if (fread(&pixels[dstride * (h - i - 1)], 1, dstride, fp) != dstride)
            {
                printf("Error: Can not read pixels\n");
                printf("Data read are incomplete\n");
                break;
            }

            if (padding != 0)
            {
                /* Skip padding bytes per line in bitmap. */
                fseek(fp, padding, SEEK_CUR);
            }
        }

        /* Close file. */
        fclose(fp);

        /* Output width. */
        if (width != NULL)
        {
            *width = w;
        }

        /* Output height. */
        if (height != NULL)
        {
            *height = h;
        }

        /* Output format. */
        if (format != NULL)
        {
            switch (bypp)
            {
            case 4:
                *format = BGRA_8888;
                break;

            case 3:
                *format = BGR_888;
                break;

            case 2:
            default:
                *format = ARGB_1555;
                break;
            }
        }

        return pixels;
    }
    while (0);

    /* Error roll back. */
    if (fp != NULL)
    {
        fclose(fp);
    }

    if (pixels != NULL)
    {
        free(pixels);
    }

    return NULL;
}

typedef union _pixel_
{
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    } u;

    uint32_t rgba;
} pixel_t;

int fconv(
    void * dest,
    int    destformat,
    const void * source,
    int    srcformat,
    int    count
    )
{
    int i;

    /* Get some shortcuts. */
    const uint8_t  * csource = (const uint8_t  *)  source;
    const uint16_t * ssource = (const uint16_t *) source;
    uint8_t  * cdest = (uint8_t  *)  dest;
    uint16_t * sdest = (uint16_t *) dest;

    /* Check source input. */
    if (source == NULL)
    {
        fprintf(stderr, "Null source input\n");
        return -EINVAL;
    }

    switch (srcformat)
    {
    case RGBA_8888:
    case BGRA_8888:
    case RGBX_8888:
    case BGRX_8888:

    case RGB_888:
    case BGR_888:

    case RGB_565:
    case BGR_565:

    case RGBA_4444:
    case BGRA_4444:

    case ARGB_1555:

    case L8:
        break;

    default:
        fprintf(stderr, "Invalid source format\n");
        return -EINVAL;
    }

    /* Check dest input. */
    if (dest == NULL)
    {
        fprintf(stderr, "Null dest input\n");
        return -EINVAL;
    }

    switch (destformat)
    {
    case RGBA_8888:
    case BGRA_8888:
    case RGBX_8888:
    case BGRX_8888:

    case RGB_888:
    case BGR_888:

    case RGB_565:
    case BGR_565:

    case RGBA_4444:
    case BGRA_4444:

    case ARGB_1555:

    case L8:
        break;

    default:
        fprintf(stderr, "Invalid dest format\n");
        return -EINVAL;
    }

    /* Special case for src format == dest format. */
    if (srcformat == destformat)
    {
        int bypp;

        switch (srcformat)
        {
        case RGBA_8888:
        case BGRA_8888:
        case RGBX_8888:
        case BGRX_8888:
            bypp = 4;
            break;

        case RGB_888:
        case BGR_888:
            bypp = 3;
            break;

        case RGB_565:
        case BGR_565:
        case RGBA_4444:
        case BGRA_4444:
        case ARGB_1555:
        default:
            bypp = 2;
            break;

        case L8:
            bypp = 1;
            break;
        }

        memcpy(dest, source, bypp * count);
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        pixel_t p;

        /* Convert to rgba8888. */
        switch (srcformat)
        {
        case RGBA_8888:
        case RGBX_8888:
            p.rgba = ((uint32_t *) source)[i];
            break;

        case BGRA_8888:
        case BGRX_8888:
            p.u.r = csource[i * 4 + 2];
            p.u.g = csource[i * 4 + 1];
            p.u.b = csource[i * 4 + 0];
            p.u.a = csource[i * 4 + 3];
            break;

        case RGB_888:
            p.u.r = csource[i * 3 + 0];
            p.u.g = csource[i * 3 + 1];
            p.u.b = csource[i * 3 + 2];
            p.u.a = 0xFF;
            break;

        case BGR_888:
            p.u.r = csource[i * 3 + 2];
            p.u.g = csource[i * 3 + 1];
            p.u.b = csource[i * 3 + 0];
            p.u.a = 0xFF;
            break;

        case RGB_565:
            /* R: 11111 000000 00000
             * G: 00000 111111 00000
             * B: 00000 000000 11111 */
            p.u.r = ((ssource[i] & 0xF800) >> 8) | ((ssource[i] & 0xF800) >> 13);
            p.u.g = ((ssource[i] & 0x07E0) >> 3) | ((ssource[i] & 0x07E0) >> 9);
            p.u.b = ((ssource[i] & 0x001F) << 3) | ((ssource[i] & 0x001F) >> 2);
            p.u.a = 0xFF;
            break;

        case BGR_565:
            /* R: 00000 000000 11111
             * G: 00000 111111 00000
             * B: 11111 000000 00000 */
            p.u.r = ((ssource[i] & 0x001F) << 3) | ((ssource[i] & 0x001F) >> 2);
            p.u.g = ((ssource[i] & 0x07E0) >> 3) | ((ssource[i] & 0x07E0) >> 9);
            p.u.b = ((ssource[i] & 0xF800) >> 8) | ((ssource[i] & 0xF800) >> 13);
            p.u.a = 0xFF;
            break;

        case RGBA_4444:
            /* R: 1111 0000 0000 0000
             * G: 0000 1111 0000 0000
             * B: 0000 0000 1111 0000
             * A: 0000 0000 0000 1111 */
            p.u.r = (ssource[i] & 0xF000) >> 8;
            p.u.g = (ssource[i] & 0x0F00) >> 4;
            p.u.b = (ssource[i] & 0x00F0) << 0;
            p.u.a = (ssource[i] & 0x000F) << 4;
            p.rgba |= p.rgba >> 4;
            break;

        case BGRA_4444:
            /* R: 0000 0000 1111 0000
             * G: 0000 1111 0000 0000
             * B: 1111 0000 0000 0000
             * A: 0000 0000 0000 1111 */
            p.u.r = (ssource[i] & 0x00F0) << 0;
            p.u.g = (ssource[i] & 0x0F00) >> 4;
            p.u.b = (ssource[i] & 0xF000) >> 8;
            p.u.a = (ssource[i] & 0x000F) << 4;
            p.rgba |= p.rgba >> 4;
            break;

        case ARGB_1555:
            /* R: 0 11111 00000 00000
             * G: 0 00000 11111 00000
             * B: 0 00000 00000 11111
             * A: 1 00000 00000 00000 */
            p.u.r = ((ssource[i] & 0x7C00) >> 7) | ((ssource[i] & 0x7C00) >> 12);
            p.u.g = ((ssource[i] & 0x03E0) >> 2) | ((ssource[i] & 0x03E0) >> 7);
            p.u.b = ((ssource[i] & 0x001F) << 3) | ((ssource[i] & 0x001F) >> 2);
            p.u.a = (ssource[i] & 0x8000) ? 0xFF : 0x00;
            break;

        case L8:
        default:
            p.u.r = csource[i];
            p.u.g = csource[i];
            p.u.b = csource[i];
            p.u.a = 0xFF;
            break;
        }

        /* Convert to dest format. */
        switch (destformat)
        {
        case RGBA_8888:
        case RGBX_8888:
            ((uint32_t *) dest)[i] = p.rgba;
            break;

        case BGRA_8888:
        case BGRX_8888:
            cdest[i * 4 + 2] = p.u.r;
            cdest[i * 4 + 1] = p.u.g;
            cdest[i * 4 + 0] = p.u.b;
            cdest[i * 4 + 3] = p.u.a;
            break;

        case RGB_888:
            cdest[i * 3 + 0] = p.u.r;
            cdest[i * 3 + 1] = p.u.g;
            cdest[i * 3 + 2] = p.u.b;
            break;

        case BGR_888:
            cdest[i * 3 + 2] = p.u.r;
            cdest[i * 3 + 1] = p.u.g;
            cdest[i * 3 + 0] = p.u.b;
            break;

        case RGB_565:
            /* R: 11111 000000 00000
             * G: 00000 111111 00000
             * B: 00000 000000 11111 */
            sdest[i] =
                ((p.u.r & 0xF8) << 8) |
                ((p.u.g & 0xFC) << 3) |
                ((p.u.b & 0xF8) >> 3);
            break;

        case BGR_565:
            /* R: 00000 000000 11111
             * G: 00000 111111 00000
             * B: 11111 000000 00000 */
            sdest[i] =
                ((p.u.r & 0xF8) >> 3) |
                ((p.u.g & 0xFC) << 3) |
                ((p.u.b & 0xF8) << 8);
            break;

        case RGBA_4444:
            /* R: 1111 0000 0000 0000
             * G: 0000 1111 0000 0000
             * B: 0000 0000 1111 0000
             * A: 0000 0000 0000 1111 */
            sdest[i] =
                ((p.u.r & 0xF0) << 8) |
                ((p.u.g & 0xF0) << 4) |
                ((p.u.b & 0xF0) >> 0) |
                ((p.u.a & 0xF0) >> 4);
            break;

        case BGRA_4444:
            /* R: 0000 0000 1111 0000
             * G: 0000 1111 0000 0000
             * B: 1111 0000 0000 0000
             * A: 0000 0000 0000 1111 */
            sdest[i] =
                ((p.u.r & 0xF0) >> 0) |
                ((p.u.g & 0xF0) << 4) |
                ((p.u.b & 0xF0) << 8) |
                ((p.u.a & 0xF0) >> 4);
            break;

        case ARGB_1555:
        default:
            /* R: 0 11111 00000 00000
             * G: 0 00000 11111 00000
             * B: 0 00000 00000 11111
             * A: 1 00000 00000 00000 */
            sdest[i] =
                ((p.u.r & 0xF8) << 7) |
                ((p.u.g & 0xF8) << 2) |
                ((p.u.b & 0xF8) >> 3) |
                ((p.u.a & 0x80) << 8);
            break;

        case L8:
            cdest[i] = (p.u.r + p.u.g + p.u.b) / 3;
            break;
        }
    }

    return 0;
}

