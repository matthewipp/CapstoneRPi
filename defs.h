
#ifndef MY_DEFS
#define MY_DEFS

#define uchar           unsigned char
#define uint            unsigned int
#define MAX_INT         2147483647
#define RED_DIR         1
#define BLUE_DIR        -1
#define IS_RED(a)       (a == 'R' || a == 'r')
#define PRINT_MOVE(m)   "(" << (int)m.s[0] << ", " << (int)m.s[1] << ") -> (" << (int)m.f[0] << ", " << (int)m.f[1] << ")" << " " << m.cap
#define IS_CROWNED(m)   (m <= 'Z')

// #define uint    unsigned int

#endif