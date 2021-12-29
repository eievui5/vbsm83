#pragma once

/*
MIT License

Copyright (c) 2021 Eievui

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

I've said before that one of the only reasons I use C++ is for std::string,
std::vector, and std::unordered_map. This tiny library aims to make vectors and
strings more convienient to use in C.

This library has been made as lightweight as possible. There is no compiled
code, just a collection of macros and static inline functions. Ultimately it is
able to produce very small and fast output while abstracting away the logic
behind variably-sized arrays.

The "V" in "VArray" can be interpreted as either "variable" or "vector",
whichever you prefer to read it as :)
*/

#include <stdlib.h>
#include <string.h>

// Return the header of a VArray.
#define va_header(va) ((struct VArrayHeader*) (va) - 1)
// Get the number of elements in a VArray. Only works if the type is known.
#define va_len(va) (va_size(va) / sizeof(__typeof__(*va)))
// Append a value of any type to the end of a VArray.
#define va_append(va, i) \
    (va_expand((&va), sizeof(i))), \
    ((va)[va_len(va) - 1] = (i))
#define va_free_contents(va) \
    for (int __i = 0; __i < va_len(va); __i++) \
        free(va[__i]); \
    va_free(va)

// Header for VArray objects. This is placed *before* the array, so that the
// user is able to directly index the array and use any type of their choosing.
struct VArrayHeader {
    size_t size; // Current size of the array in bytes.
    size_t _true_size; // The actual size of the allocated buffer.
};

// Construct a new VArray of a given size.
static inline void* va_new(size_t s) {
    struct VArrayHeader* head = malloc(s * 2 + sizeof(struct VArrayHeader));
    head->size = s;
    head->_true_size = s * 2;
    return head + 1;
}

// Get the size of a VArray.
static inline size_t va_size(void* va) {
    return va_header(va)->size;
}

// Resizes a VArray. Requires a pointer to the VArray.
static inline void va_resize(void* va, size_t s) {
    struct VArrayHeader* head = va_header(*(void**) va);
    head->size = s;
    if (head->size > head->_true_size) {
        if (head->_true_size == 0)
            head->_true_size = head->size * 2;
        else while (head->size > head->_true_size)
            head->_true_size *= 2;
        head = realloc(va_header(*(void**) va), sizeof(struct VArrayHeader) + head->_true_size);
    }
    *(void**) va = head + 1;
}

// Appends additional bytes to a VArray. Requires a pointer to the VArray.
static inline void va_expand(void* va, size_t s) {
    va_resize(va, va_header(*(void**) va)->size + s);
}

// Duplicate a VArray.
static inline void* va_dup(void* va) {
    struct VArrayHeader* new_va = malloc(va_header(va)->_true_size + sizeof(struct VArrayHeader));
    memcpy(new_va, va_header(va), va_header(va)->_true_size + sizeof(struct VArrayHeader));
    return new_va + 1;
}

// Free a VArray's buffer.
static inline void va_free(void* va) {
    free(va_header(va));
}

// Strip the header of a VArray, converting it to a normal memory buffer and
// allowing it to be freed with `free()`.
static inline void va_strip(void* va) {
    struct VArrayHeader* head = va_header(*(void**) va);
    memmove(head, *(void**) va, va_size(va));
    *(void**) va = head;
}

// Create a new VArray using a string.
static inline void* va_newstr(const char* str) {
    size_t string_len = strlen(str);
    void* result = va_new(string_len);
    memcpy(result, str, string_len);
    return result;
}

// Append a single byte to a VArray string and adjust the null byte. Requires
// a pointer to the VArray.
static inline void va_append_char(char** va, char c) {
    va_expand(va, sizeof(char));
    (*va)[va_size(*va) - 1] = c;
    (*va)[va_size(*va)] = 0;
}

// Append a string to the end of a VArray. Requires a pointer to the VArray.
static inline void va_strcat(char** va, const char* str) {
    size_t s = strlen(str);
    size_t old_size = va_size(*va);
    va_expand(va, s);
    memcpy(*va + old_size, str, s);
    va_header(*va)->size += s;
}
