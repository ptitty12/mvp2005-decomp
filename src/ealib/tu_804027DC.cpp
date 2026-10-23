/*
 * EA shared library code that uses small data (cflags_ealib_sda: -O2, default -G8):
 * VP6/MAD video codecs and SPCH, or RealCore,
 * .text 0x804027DC-0x80403E80 (code-only unit, 34 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

#include "../../include/game/tu_8000C2C4.h"

extern "C" double sqrt(double x);

struct V3 {
    float x, y, z;
};

extern "C" void v3crossprod(const V3* a, const V3* b, V3* out);
extern "C" void v3unit(const V3* v, V3* out);

extern "C" int sprintf(char*, const char*, ...);
extern const char lbl_8060E9A0[];

/* Little-endian field readers of the locale string table. */
inline unsigned ReadLE32(const unsigned char* p)
{
    return (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
}

inline unsigned ReadLE16(const unsigned char* p)
{
    return (p[1] << 8) | p[0];
}

/* 192-bit additive generator state (six words, w5 least significant); lbl_8062EEC0. */
struct Gen6 {
    unsigned w0, w1, w2, w3, w4, w5;
};
extern Gen6 lbl_8062EEC0;

extern "C" unsigned fn_804027DC()
{
    unsigned c = 0;
    unsigned x = lbl_8062EEC0.w5 + lbl_8062EEC0.w4;
    if (x < lbl_8062EEC0.w5 || x < lbl_8062EEC0.w4)
        c = 1;
    unsigned o0 = lbl_8062EEC0.w0;
    unsigned o3 = lbl_8062EEC0.w3;
    unsigned o2 = lbl_8062EEC0.w2;
    unsigned o1 = lbl_8062EEC0.w1;
    lbl_8062EEC0.w4 = x;
    x = x + o3 + c;
    ((volatile Gen6*)&lbl_8062EEC0)->w3 = x;
    x = x + o2 + (x < o3);
    ((volatile Gen6*)&lbl_8062EEC0)->w2 = x;
    x = x + o1 + (x < o2);
    ((volatile Gen6*)&lbl_8062EEC0)->w1 = x;
    x = x + o0 + (x < o1);
    lbl_8062EEC0.w0 = x;
    if (++lbl_8062EEC0.w5 == 0)
        if (++lbl_8062EEC0.w4 == 0)
            if (++lbl_8062EEC0.w3 == 0)
                if (++lbl_8062EEC0.w2 == 0)
                    if (++lbl_8062EEC0.w1 == 0)
                        x = ++lbl_8062EEC0.w0;
    return x;
}

extern "C" float fn_804028E4(double x)
{
    return (float)sqrt(x);
}

extern "C" double sin(double x);
extern "C" double cos(double x);

extern "C" float fn_80402908(float x)
{
    return (float)sin((double)x * 6.2831855f);
}

extern "C" float fn_8040293C(float x)
{
    return (float)cos((double)x * 6.2831855f);
}

extern "C" void rsincos(const float* a, float* s, float* c)
{
    *s = fn_80402908(*a);
    *c = fn_8040293C(*a);
}

/* Vec3 array helpers of the EA math library */
extern "C" void v3add(int n, const V3* a, const V3* b, V3* out)
{
    int i;
    for (i = 0; i < n; i++) {
        out[i].x = a[i].x + b->x;
        out[i].y = a[i].y + b->y;
        out[i].z = a[i].z + b->z;
    }
}

extern "C" void v3sub(int n, const V3* a, const V3* b, V3* out)
{
    int i;
    for (i = 0; i < n; i++) {
        out[i].x = a[i].x - b->x;
        out[i].y = a[i].y - b->y;
        out[i].z = a[i].z - b->z;
    }
}

extern "C" void v3scale(int n, const V3* a, V3* out, float k)
{
    int i;
    for (i = 0; i < n; i++) {
        out[i].x = a[i].x * k;
        out[i].y = a[i].y * k;
        out[i].z = a[i].z * k;
    }
}

extern "C" float v3length(const V3* v)
{
    float y = v->y;
    float x = v->x;
    x = x * x + y * y;
    float z = v->z;
    x = z * z + x;
    return (float)sqrt(x);
}

/* near-miss: same length; retail interleaves the six loads and three products differently (locals in draft order tried). */
extern "C" void v3crossprod(const V3* a, const V3* b, V3* out)
{
    float bx = b->x;
    float ax = a->x;
    float ay = a->y;
    float bz = b->z;
    float az = a->z;
    float by = b->y;
    out->y = az * bx - ax * bz;
    out->z = ax * by - ay * bx;
    out->x = ay * bz - az * by;
}

extern "C" void fn_80402B0C(const V3* a, const V3* b, V3* out)
{
    V3 t;
    v3crossprod(a, b, &t);
    v3unit(&t, out);
}

extern "C" void v3unit(const V3* v, V3* out)
{
    float s = 1.0f / v3length(v);
    out->x = v->x * s;
    out->y = v->y * s;
    out->z = v->z * s;
}

extern "C" void v3dot(int n, const V3* a, const V3* b, float* out)
{
    int i;
    for (i = 0; i < n; i++)
        out[i] = a[i].x * b->x + a[i].y * b->y + a[i].z * b->z;
}

struct Rec0E {
    char pad[0xe];
    unsigned char b[2];
};

extern "C" void fn_80402BEC(Rec0E* r, int z, unsigned short v)
{
    if (z == 0) {
        *(unsigned short*)r->b = v;
        r->b[0] = v;
        r->b[1] = v >> 8;
    }
}

extern "C" int fn_80402C08(const unsigned short* a, const unsigned char* b)
{
    return *a - ((b[1] << 8) | b[0]);
}

extern "C" int getstringidbyindex(unsigned short key, const unsigned char* t);

extern "C" void* bsearch(const void* key, const void* base, unsigned n, unsigned sz, int (*cmp)(const void*, const void*));

extern "C" int getstringidbyindex(unsigned short key, const unsigned char* t)
{
    unsigned short k = key;
    const unsigned char* r = (const unsigned char*)bsearch(&k, t + 0x10, ReadLE32(t + 8), 4, (int (*)(const void*, const void*))fn_80402C08);
    int v;
    if (r != 0) {
        r += 2;
        v = ReadLE16(r);
    } else
        v = -1;
    return v;
}

extern "C" const unsigned char* LOCALE_getstr(const unsigned char* t, int id)
{
    int k = id;
    if (t[8] & 1)
        k = getstringidbyindex(k, t + ReadLE32(t + 4));
    const unsigned char* sec = t + ReadLE32(t + 0x10 + ReadLE16(t + 0xE) * 4);
    const unsigned char* r;
    if (k >= 0 && (unsigned)k < ReadLE32(sec + 0xC))
        r = sec + ReadLE32(sec + 0x10 + k * 4);
    else
        r = 0;
    return r;
}

extern "C" void LOCALE_create(unsigned char* t, int flag)
{
    if (flag == 0)
        return;
    int count = ReadLE16(t + 0xC);
    for (int i = 0; i < count; i++) {
        unsigned char* sec = t + ReadLE32(t + 0x10 + i * 4);
        int n = ReadLE32(sec + 0xC);
        for (int j = 0; j < n; j++) {
            unsigned short* s = (unsigned short*)(sec + ReadLE32(sec + 0x10 + j * 4));
            if (*s != 0) {
                do {
                    *s = ReadLE16((unsigned char*)s);
                    s++;
                } while (*s != 0);
            }
        }
    }
}

extern "C" int fn_80402C08(const unsigned short* a, const unsigned char* b);

extern "C" void Unicode2Ascii(char* out, const unsigned short* in)
{
    while (*in != 0) {
        if (*in > 0xFF || (*in == 0x5C && in[1] == 0x78)) {
            sprintf(out, lbl_8060E9A0, *in);
            out += 6;
        } else {
            *out++ = (char)*in;
        }
        in++;
    }
    *out = 0;
}

extern "C" int fn_80403060(const unsigned short* s)
{
    int n = 0;
    while (*s) {
        s++;
        n++;
    }
    return n;
}

extern "C" int fn_80403088(const unsigned short* s, int ch)
{
    int i = 0;
    while (s[i] != 0) {
        if (s[i] == ch)
            return i;
        i++;
    }
    return 0xFFFF;
}

extern "C" int fn_804030F0(const unsigned short* a, const unsigned short* b, int n);

extern "C" int fn_804030C8(const unsigned short* a, const unsigned short* b)
{
    return fn_804030F0(a, b, 0x7FFFFFFF);
}

extern "C" int fn_804030F0(const unsigned short* a, const unsigned short* b, int n)
{
    int d = 0;
    int i;
    for (i = 0; i < n && d == 0; i++) {
        d = a[i] - b[i];
        if (a[i] == 0)
            break;
    }
    return d;
}

extern "C" void fn_8040316C(unsigned short* d, const unsigned short* s, int n);

extern "C" void fn_80403144(unsigned short* d, const unsigned short* s)
{
    fn_8040316C(d, s, 0x7FFFFFFF);
}

extern "C" void fn_8040316C(unsigned short* d, const unsigned short* s, int n)
{
    int i = 0;
    while (i < n && s[i] != 0) {
        d[i] = s[i];
        i++;
    }
    d[i] = 0;
}

extern "C" void fn_804031BC(unsigned short* d, const unsigned char* s)
{
    int i = 0;
    while (s[i] != 0) {
        d[i] = s[i];
        i++;
    }
    d[i] = 0;
}

extern "C" void fn_8040321C(unsigned short*, const unsigned short*, const unsigned short*, const unsigned short*);
extern "C" void fn_80403144(unsigned short* d, const unsigned short* s);
extern "C" int fn_80403060(const unsigned short* s);

extern "C" void fn_804031F8(unsigned short* a, const unsigned short* b, const unsigned short* c)
{
    fn_8040321C(a, b, c, 0);
}

extern "C" void fn_8040321C(unsigned short* out, const unsigned short* fmt, const unsigned short* a, const unsigned short* b)
{
    while (*fmt != 0) {
        if (*fmt == '%' && fmt[1] == '1') {
            fn_80403144(out, a);
            fmt += 2;
            out += fn_80403060(a);
        } else if (*fmt == '%' && fmt[1] == '2') {
            fn_80403144(out, b);
            fmt += 2;
            out += fn_80403060(b);
        } else {
            *out++ = *fmt++;
        }
    }
    *out = 0;
}

extern "C" unsigned short* fn_804032D8(unsigned short* end, unsigned v, unsigned base, const unsigned short* digits)
{
    if (v == 0) {
        *--end = digits[0];
        return end;
    }
    do {
        unsigned q = v / base;
        *--end = digits[v - q * base];
        v = q;
    } while (v != 0);
    return end;
}

extern "C" unsigned short* fn_80403310(unsigned short* end, int v, unsigned base, const unsigned short* digits)
{
    if (v < 0)
        v = -v;
    return fn_804032D8(end, v, base, digits);
}

extern "C" void USTR_vsprintf(unsigned short* dst, const unsigned short* fmt, va_list ap);

extern "C" void fn_80403DE4(unsigned short* dst, const unsigned short* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    USTR_vsprintf(dst, fmt, ap);
}
