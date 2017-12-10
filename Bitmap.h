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

/*
 * Bitmap image write and read.
 */
#ifndef TCG_BITMAP_H
#define TCG_BITMAP_H

#ifdef __cplusplus
extern "C" {
#endif


#ifndef __pixel_format_
#define __pixel_format_
enum
{
                      /* Bytes: 0 1 2 3 (Low to high)*/
    RGBA_8888 = 306,  /*        R G B A */
    BGRA_8888 = 212,  /*        B G R A */
    RGBX_8888 = 305,  /*        R G B - */
    BGRX_8888 = 211,  /*        B G R - */

                      /* Bytes: 0 1 2 (Low to high) */
    RGB_888   = 303,  /*        R G B */
    BGR_888   = 210,  /*        B G R */

                      /* Bits: 15 11 5  0 (MSB to LSB) */
    RGB_565   = 209,  /*         R  G  B */
    BGR_565   = 302,  /*         B  G  R */

                      /* Bits: 15 12 8 4 0 (MSB to LSB) */
    RGBA_4444 = 205,  /*         R  G B A */
    BGRA_4444 = 313,  /*         B  G R A */

                      /* Bits: 15 14 10 5  0 (MSB to LSB) */
    ARGB_1555 = 208,  /*        A   R  G  B */

    L8 = 801          /* L8 bit */
};
#endif

/**
 * Read pixel data from bitmap file.
 *
 * @param filename bitmap file name.
 * @param width image width.
 * @param height image height.
 * @param format image format.
 * @return pixels data in 'format'. Do not forget to free.
 */
void* ReadBitmap(const char * filename,
                 unsigned int * width,
                 unsigned int * height,
                 int * format);

/**
 * Write pixel data to bitmap file.
 *
 * @param filename bitmap file name.
 * @param data pixel data.
 * @param width image width.
 * @param height image height.
 * @param stride stride in bytes, can be 0 if equels to width * bypp.
 * @param format pixel of 'data'.
 * @return 0 if success, else non-zero.
 */
int WriteBitmap(const char * filename,
                const void * data,
                unsigned int width,
                unsigned int height,
                unsigned int stride,
                int format,
                int flip = 1);

/**
 * Format conversion for raw pixels.
 *
 * @param dest memory to hold converted pixels. Make sure the memory
 *             size is enough.
 * @param destformat required dest format.
 * @param source source pixels.
 * @param srcformat source format.
 * @param count pixel count to convert.
 */
int fconv(
    void * dest,
    int    destformat,
    const void * source,
    int    srcformat,
    int    count
    );

#ifdef __cplusplus
}
#endif

#endif // TCG_BITMAP_H
