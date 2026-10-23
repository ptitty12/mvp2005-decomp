/*
 * Translation unit at 0x8000C2C4 (original file name unknown; placeholder).
 * A real TU: it ends with its static-initializer wrapper (docs/UNITS.md
 * ctor anchors). 17 functions.
 *
 * .text 0x8000C2C4-0x8000D650, .rodata 0x805A3B38-0x805A4014, .data 0x80615850-0x806164B8, .bss 0x80686AF8-0x80686B94.
 * Not split (declare extern if used): small-data (.sdata/.sbss) objects and
 * any second, non-contiguous range of a section.
 */
#include "game/tu_8000C2C4.h"

extern Unk80634ADC lbl_80634ADC;
extern const Unk80615854 lbl_80615854;
extern Unk8000C2C4* lbl_80615850;
extern void* (*lbl_8062C668)(unsigned int size, int flags);
extern unsigned char lbl_806158B8[];

void fn_8000C704(short x, short y, unsigned short w, unsigned char* base, const char* s, unsigned short color);
extern "C" void DCFlushRange(void* p, unsigned int n);
int fn_804089B4(char* buf, const char* fmt, ...);
extern "C" int fn_803E7320(int ch, const char* s, ...);
void fn_8043D380(void* fb);
void fn_8043D250();
int fn_80409C4C(char* buf, int n, const char* fmt, va_list ap);

inline void* operator new(unsigned int, void* p) { return p; }

Unk8000C2C4::Unk8000C2C4(unsigned char* fb, Unk80634ADC* rmode)
{
    m0 = fb;
    m4 = &lbl_80634ADC;
    mC = 0;
    mE = 0;
    m1C = 0;
    if (rmode)
        m4 = rmode;
    m8 = ((m4->m4 + 15) & 0xFFF0) * m4->m8 * 2;
    m10.m0 = lbl_80615854.m0;
    m10.m4 = lbl_80615854.m4;
    m10.m8 = lbl_80615854.m8;
}

void Unk8000C2C4::fn_8000C340(int v)
{
    m1C = v;
}

void Unk8000C2C4::fn_8000C348(short x, short y)
{
    mC = x;
    mE = y;
}

void Unk8000C2C4::fn_8000C354(const char* s)
{
    char buf[0x400];
    if (s) {
        fn_8000C704(mC, mE, m4->m4, m0, s, (unsigned short)m10.m0 * 256 + (unsigned short)m10.m4);
        DCFlushRange(m0, m8);
        if (m1C) {
            fn_804089B4(buf, "%.*s\n", 0x3fe, s);
            buf[0x3fe] = '\n';
            buf[0x3ff] = 0;
            fn_803E7320(0, buf);
        }
    }
}

void Unk8000C2C4::fn_8000C408(const Unk80615854* c)
{
    unsigned int* p = (unsigned int*)m0;
    while (p < (unsigned int*)(m0 + m8)) {
        *p = (*p & 0xFF000000) | (c->m4 << 16 | c->m0 << 8 | c->m8);
        p++;
    }
}

void Unk8000C2C4::fn_8000C464(const Unk80615854* c)
{
    m10.m0 = c->m0;
    m10.m4 = c->m4;
    m10.m8 = c->m8;
}

void Unk8000C2C4::fn_8000C480()
{
    fn_8043D380(m0);
    fn_8043D250();
}

void fn_8000C4A8(unsigned char* fb, Unk80634ADC* rmode)
{
    if (!lbl_80615850)
        lbl_80615850 = new (lbl_8062C668(0x20, 0)) Unk8000C2C4(fb, rmode);
}

void fn_8000C50C(int v)
{
    if (lbl_80615850)
        lbl_80615850->fn_8000C340(v);
}

void fn_8000C540(const Unk80615854* c)
{
    if (lbl_80615850)
        lbl_80615850->fn_8000C408(c);
}

void fn_8000C574(const Unk80615854* c)
{
    if (lbl_80615850)
        lbl_80615850->fn_8000C464(c);
}

void fn_8000C5A8()
{
    if (lbl_80615850)
        lbl_80615850->fn_8000C480();
}

void fn_8000C5D8(short x, short y, const char* fmt, va_list ap)
{
    char buf[0x400];
    int n;
    if (ap) {
        n = fn_80409C4C(buf, 0x400, fmt, ap);
    } else {
        fn_804089B4(buf, "%.*s", 0x3fe, fmt);
        n = -1;
    }
    if (n == -1)
        buf[0x3ff] = 0;
    if (lbl_80615850) {
        lbl_80615850->fn_8000C348(x, y);
        lbl_80615850->fn_8000C354(buf);
    }
}

void fn_8000C674(short x, short y, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fn_8000C5D8(x, y, fmt, ap);
}

/*
 * Draws s into the framebuffer with the 8x8 font (32 bytes per glyph from
 * ' ', one 4-bit nibble per pixel, set when the nibble's top bit is set); each
 * font pixel is a 32-bit word (two screen pixels) written to two lines, so a
 * glyph row advances 0x278 words after its 8 pixels.
 *
 * near-miss (8 attempts, 8 loop-shape families scanned, 100+ slots differ, 37
 * vs 79 instructions): retail unrolls the 8-pixel loop completely (also with
 * the loop written out by hand) but keeps the row loop as `li r14,8` /
 * `subf. r14,r12,r14` with a constant 1 in r12 (mine becomes mtctr/bdnz or
 * addic.), keeps the glyph-pointer loop un-rotated, keeps the colour and the
 * row counter in callee-saved r15/r14 with LR saved although there is no call,
 * and tests the 8th pixel after a 7th `slwi.` (mine folds it into andis.).
 * -funroll-loops, -funroll-all-loops, -O2 do not reproduce it.
 */
void fn_8000C704(short x, short y, unsigned short w, unsigned char* base, const char* s, unsigned short color)
{
    unsigned int* dst = (unsigned int*)(base + (x << 5) + (y << 5) * w);
    unsigned int col = color | (color << 16);
    unsigned char c;
    while ((c = *s++) != 0) {
        const unsigned int* g = (const unsigned int*)(lbl_806158B8 + (c - 0x20) * 32);
        int row = 8;
        do {
            unsigned int bits = *g++;
            for (int px = 0; px < 8; px++) {
                if ((int)bits < 0) {
                    dst[0] = col;
                    dst[0x140] = col;
                }
                bits <<= 4;
                dst++;
            }
            dst += 0x278;
        } while (--row);
        dst -= 0x13F8;
    }
}

extern "C" float fn_804028E4(float x);                  /* square root */

extern const Unk8000C840_Vec3 lbl_805A3D9C[4];   /* point table (7.5, 0, 1072.5) ... */
extern const Unk8000C840_Vec3 lbl_805A3DCC;      /* (513.3595, 0, 513.3595) */
extern const Unk8000C840_Vec3 lbl_806055AC;      /* shared constant vector, result for a zero length */

inline Unk8000C840_Vec3 operator-(const Unk8000C840_Vec3& a, const Unk8000C840_Vec3& b)
{
    Unk8000C840_Vec3 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}

inline float Length(const Unk8000C840_Vec3& v)
{
    return fn_804028E4(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline Unk8000C840_Vec3 Normalize(const Unk8000C840_Vec3& v, float len)
{
    Unk8000C840_Vec3 r;
    float inv = 1.0f / len;
    r.x = v.x * inv;
    r.y = v.y * inv;
    r.z = v.z * inv;
    return r;
}

inline Unk8000C840_Vec3 Normalized(const Unk8000C840_Vec3& v)
{
    float len = Length(v);
    if (len != 0.0f)
        return Normalize(v, len);
    return lbl_806055AC;
}

Unk8000C840_Vec3 lbl_80686AF8 = Normalized(lbl_805A3DCC - lbl_805A3D9C[3]);
Unk8000C840_Vec3 lbl_80686B04 = Normalized(lbl_805A3D9C[3] - lbl_805A3DCC);
Unk8000C840_Vec3 lbl_80686B10 = Normalized(lbl_805A3D9C[2] - lbl_805A3D9C[1]);
Unk8000C840_Vec3 lbl_80686B1C = Normalized(lbl_805A3D9C[1] - lbl_805A3D9C[2]);
Unk8000C840_Vec3 lbl_80686B28 = Normalized(lbl_805A3D9C[1] - lbl_805A3D9C[0]);
Unk8000C840_Vec3 lbl_80686B34 = Normalized(lbl_805A3D9C[0] - lbl_805A3D9C[1]);
Unk8000C840_Vec3 lbl_80686B40 = Normalized(lbl_805A3D9C[2] - lbl_805A3D9C[3]);
Unk8000C840_Vec3 lbl_80686B4C = Normalized(lbl_805A3D9C[3] - lbl_805A3D9C[2]);
Unk8000C840_Vec3 lbl_80686B58 = Normalized(lbl_805A3D9C[3] - lbl_805A3D9C[1]);
Unk8000C840_Vec3 lbl_80686B64 = Normalized(lbl_805A3D9C[0] - lbl_805A3D9C[3]);
Unk8000C840_Vec3 lbl_80686B70 = Normalized(lbl_805A3D9C[3] - lbl_805A3D9C[0]);
Unk8000C840_Vec3 lbl_80686B7C = Normalized(lbl_805A3D9C[2] - lbl_805A3D9C[0]);
Unk8000C840_Vec3 lbl_80686B88 = Normalized(lbl_805A3D9C[0] - lbl_805A3D9C[2]);
