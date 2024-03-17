/*
MIT License

Copyright (c) 2024 Alexander Wentz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef _COMMON_H_
#define _COMMON_H_

#include <cstdio>
#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)



#define MSG_AND_RETURN_IF(cond, val, ...) do {  \
    if(cond){                                   \
        std::fprintf(stderr, __VA_ARGS__);      \
        std::fprintf(stderr, "\n");             \
        return val;                             \
    }                                           \
} while (false)

#define TR() do {                                                                       \
    std::fprintf(stderr, "%s \t\t -> \t\t %s:%d \n", __FILENAME__, __func__, __LINE__);      \
} while (false)

#define TR_MSG(...) do {                                                                \
    std::fprintf(stderr, "%s \t\t -> \t\t %s:%d \t ", __FILENAME__, __func__, __LINE__);     \
    std::fprintf(stderr, __VA_ARGS__);                                                  \
    std::fprintf(stderr, "\n");                                                         \
} while (false)

#endif