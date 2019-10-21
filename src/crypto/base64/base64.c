#include "b64.h"
#include <string.h>

ssize_t base64_decode(unsigned char *dst, const char *src)
{
    static const unsigned char b64[256] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255,
        255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
        255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    };

    unsigned char* usrc = (unsigned char*)src;
    unsigned char* p = dst;
    size_t b64sz = strlen((char*)src);
    size_t last_i = b64sz-1;
    size_t i;
    for (i = 0; i < b64sz; i += 4)
    {
        size_t i1 = i + 1;
        size_t i2 = i + 2;
        size_t i3 = i + 3;
        if (b64[usrc[i]] == 255 || b64[usrc[i1]] == 255)
        {
            break;
        }
        *p++ = (b64[usrc[i]] << 2) | (b64[usrc[i1]] >> 4);
        if (i2 > last_i)
        {
            break;
        }
        if (src[i2] == '=')
        {
            if (i3 != last_i)
            {
                break;
            }
            if (src[i3] != '=')
            {
                break;
            }
        }
        else
        {
            if (b64[usrc[i2]] == 255)
            {
                break;
            }
            *p++ = ((b64[usrc[i1]] << 4) & 0xf0) | (b64[usrc[i2]] >> 2);
            if (i2 == last_i)
            {
                break;
            }
            if (src[i3] == '=')
            {
                if (i3 != last_i)
                {
                    break;
                }
            }
            else
            {
                if (b64[usrc[i3]] == 255)
                {
                    break;
                }
                *p++ = ((b64[usrc[i2]] << 6) & 0xc0) | b64[usrc[i3]];
            }
        }
    }
    if (i < b64sz)
    {
        return -(ssize_t)(i + 1);
    }
    return p - dst;
}

size_t base64_encode(char *dst, const unsigned char* src, size_t in_sz)
{
    static const char b64str[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char* p = dst;
    for (size_t i = 0; i < in_sz; i += 3)
    {
        size_t i1 = i + 1;
        size_t i2 = i + 2;
        *p++ = b64str[(src[i] >> 2) & 0x3f];
        *p++ = b64str[((src[i] << 4) + (i1 >= in_sz ? 0 : (src[i1] >> 4))) & 0x3f];
        *p++ = i1 >= in_sz ? '=' : b64str[((src[i1] << 2) + (i2 >= in_sz ? 0 : (src[i2] >> 6))) & 0x3f];
        *p++ = i2 >= in_sz ? '=' : b64str[src[i2] & 0x3f];
    }
    *p = '\0';
    return p - dst;
}
