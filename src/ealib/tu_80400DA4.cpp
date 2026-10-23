/*
 * EA shared library code that uses small data (cflags_ealib_sda: -O2, default -G8):
 * VP6/MAD video codecs and SPCH, or RealCore,
 * .text 0x80400DA4-0x804027DC (code-only unit, 30 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

/* RealCore timing, wait and one-slot message helpers (EA). */
extern "C" {
int fn_80400CE4(void);
void THREAD_yield(int);
void OSInitMessageQueue(void* q, void* buf, int n);
int OSSendMessage(void* q, void* msg, int flags);
int OSReceiveMessage(void* q, void* msg, int flags);
void OSReport(const char* fmt, ...);
}
extern int lbl_806EEAE0;
extern int lbl_806EEADC;
extern int lbl_806EEAF0;
extern int lbl_806EEAF4;
extern char lbl_8060E8E8[];

extern int lbl_806EEB08;
extern int lbl_806EEB10;
extern "C" int fn_80401044(int a, int v, unsigned char* buf, int b, int c);

struct Unk80402288_V3 {
    float x, y, z;
};

struct Unk80402288_Mtx {
    float m00, m01, m02, m03;
    float m10, m11, m12, m13;
    float m20, m21, m22, m23;
    float m30, m31, m32, m33;
};

struct Unk804020F0_V4 {
    float x, y, z, w;
};

extern "C" int fn_80400DA4()
{
    return lbl_806EEAE0;
}

extern "C" int TIMER_elapsedticks()
{
    int old = lbl_806EEAF4;
    int now = fn_80400DA4();
    lbl_806EEAF4 = now;
    return now - old;
}

extern "C" void fn_80400DE4(int d)
{
    lbl_806EEAF0 = fn_80400DA4() + d;
}

extern "C" unsigned fn_80400E54();

/* near-miss: same length; retail loads the zero return value (li r3,0) before the loop-back branch, ours after it. */
extern "C" int fn_80400E18()
{
    while (fn_80400E54() == 0) {
        fn_80400CE4();
        THREAD_yield(0);
    }
    return 0;
}

extern "C" unsigned fn_80400E54()
{
    int t = fn_80400DA4() - lbl_806EEAF0;
    return (unsigned)~t >> 31;
}

extern "C" int fn_80400E84()
{
    return lbl_806EEADC;
}

extern int lbl_806EEAD4;
extern void (*lbl_8062EE20[8])(void);

extern "C" int MEMSYS_validaddress(unsigned x)
{
    return (x >= 0x80003100 && x <= 0x82FFFFFF)
        || (x >= 0xC0003100 && x <= 0xC2FFFFFF)
        || (x >= 0xE0000000 && x <= 0xE0004000)
        || (x >= 0xC8000000 && x <= 0xC8083FFF)
        || (x >= 0x7E000000 && x <= 0x7FFFFFFF);
}

extern "C" void ttDoVTimerMsg()
{
    lbl_806EEAD4++;
    for (int i = 0; i < 8; i++) {
        void (*f)(void) = lbl_8062EE20[i];
        if (f != 0)
            f();
    }
}

extern "C" void fn_80400F54()
{
    OSReport(lbl_8060E8E8);
}

extern "C" int SIGNAL_create(char* s)
{
    OSInitMessageQueue(s + 4, s + 0x24, 1);
    return 1;
}

extern "C" void SIGNAL_set(char* s)
{
    int msg = 0;
    OSSendMessage(s + 4, &msg, 0);
}

extern "C" void fn_80400FE4(char* s)
{
    int msg;
    OSReceiveMessage(s + 4, &msg, 1);
}

extern "C" unsigned fn_80401010(const unsigned char* a, const unsigned char* b, unsigned n)
{
    unsigned i;
    for (i = 0; i < n; i++) {
        if (*a++ != *b++)
            break;
    }
    return i;
}

extern "C" int fn_80401548(int a, int v, char* buf)
{
    int g1 = lbl_806EEB08;
    int g2 = lbl_806EEB10;
    int n;
    if (v > 0xFFFFFF) {
        if (buf != 0) {
            buf[0] = 0x90;
            buf[1] = 0xFB;
            char* q = buf + 2;
            q[0] = (unsigned)v >> 24;
            q[1] = (unsigned)v >> 16;
            q[2] = (unsigned)v >> 8;
            q[3] = v;
        }
        n = 6;
    } else {
        if (buf != 0) {
            buf[0] = 0x10;
            buf[1] = 0xFB;
            char* q = buf + 2;
            q[0] = (unsigned)v >> 16;
            q[1] = (unsigned)v >> 8;
            q[2] = v;
        }
        n = 5;
    }
    return n + fn_80401044(a, v, (unsigned char*)(buf != 0 ? buf + n : 0), g1, g2);
}

extern "C" int fn_80401610(int a, int v, char* buf, int flag)
{
    return fn_80401548(a, v, flag ? buf : 0);
}

/* near-miss (97.3%): same instructions except the first flag test: retail keeps `slwi; andi.` on (p[0] << 8) & 0x100 and the constant addi; ours folds them into rlwinm and addi p+5. */
extern "C" int sizerefpack(const unsigned char* p, int* outLen)
{
    int v = 0;
    if (p != 0) {
        const unsigned char* q = p + 2;
        int t = p[0] << 8;
        if ((t & 0x100) != 0)
            q += 3;
        v = *q;
        v = (v << 8) + *++q;
        v = (v << 8) + *++q;
        q++;
        if (outLen != 0) {
            for (;;) {
                unsigned char b = *q++;
                if ((b & 0x80) == 0) {
                    q += 1;
                    q += b & 3;
                    continue;
                }
                if ((b & 0x40) == 0) {
                    unsigned k = *q;
                    q += 2;
                    q += k >> 6;
                    continue;
                }
                if ((b & 0x20) == 0) {
                    q += 3;
                    q += b & 3;
                    continue;
                }
                unsigned n = ((b & 0x1F) << 2) + 4;
                if (n <= 0x70) {
                    q += n;
                    continue;
                }
                q += b & 3;
                *outLen = q - p;
                break;
            }
        }
    }
    return v;
}

extern "C" int unpackz(const unsigned char* p, int a);

extern "C" int REF_decode(unsigned char* dst, const unsigned char* src, int* outLen);

extern "C" int unpackz(const unsigned char* p, int a)
{
    int r = 0;
    if (p[1] == 0xFB) {
        switch (p[0] & 0xFE) {
        case 0x10:
        case 0x90:
            r = REF_decode((unsigned char*)a, p, 0);
            break;
        case 0x18:
        case 0x1A:
        case 0x1C:
        case 0x30:
        case 0x32:
        case 0x34:
        case 0x46:
        case 0x98:
        case 0x9A:
        case 0x9E:
        case 0xB0:
        case 0xB2:
        case 0xB4:
        case 0xC6:
            break;
        }
    }
    return r;
}

extern "C" int fn_804017B0(const unsigned char* p, int a)
{
    return unpackz(p, a);
}

extern "C" int unpacksizez(const unsigned char* p)
{
    int r = 0;
    if (p[1] == 0xFB) {
        switch (p[0] & 0xFE) {
        case 0x10:
        case 0x18:
        case 0x1A:
        case 0x1E:
        case 0x30:
        case 0x32:
        case 0x34:
        case 0x46: {
            const unsigned char* q = p + 2;
            r = (p[2] << 16) | (q[1] << 8) | q[2];
            break;
        }
        case 0x90:
        case 0x98:
        case 0x9A:
        case 0x9E:
        case 0xB0:
        case 0xB2:
        case 0xB4:
        case 0xC6: {
            const unsigned char* q = p + 2;
            r = (p[2] << 24) | (q[1] << 16) | (q[2] << 8) | q[3];
            break;
        }
        case 0xC0:
            break;
        }
    }
    return r;
}

extern "C" int fn_80401904(const unsigned char* p)
{
    return unpacksizez(p);
}

extern "C" int fixedmult(int a, int b)
{
    return (int)(((long long)a * b + 0x8000) >> 16);
}

extern "C" void rsincos(const float* a, float* s, float* c);

extern "C" void rsincos(const float* a, float* s, float* c);

extern "C" void rsincos(const float* a, float* s, float* c);

extern "C" void rsincos(const float* a, float* s, float* c);

extern "C" void MATRIX4_multyrot(const Unk80402288_Mtx* in, Unk80402288_Mtx* out, float angle)
{
    float s;
    float c;
    float a;
    rsincos(&angle, &s, &c);
    a = in->m00;
    out->m00 = a * c + in->m02 * s;
    out->m01 = in->m01;
    out->m02 = in->m02 * c - a * s;
    out->m03 = in->m03;
    a = in->m10;
    out->m10 = a * c + in->m12 * s;
    out->m11 = in->m11;
    out->m12 = in->m12 * c - a * s;
    out->m13 = in->m13;
    a = in->m20;
    out->m20 = a * c + in->m22 * s;
    out->m21 = in->m21;
    out->m22 = in->m22 * c - a * s;
    out->m23 = in->m23;
    a = in->m30;
    out->m30 = a * c + in->m32 * s;
    out->m31 = in->m31;
    out->m32 = in->m32 * c - a * s;
    out->m33 = in->m33;
}

extern "C" void MATRIX4_mult(const Unk80402288_Mtx* a, const Unk80402288_Mtx* b, Unk80402288_Mtx* out)
{
    Unk80402288_Mtx tmp;
    Unk80402288_Mtx* d = (out == a || out == b) ? &tmp : out;
    d->m00 = a->m00 * b->m00 + a->m01 * b->m10 + a->m02 * b->m20 + a->m03 * b->m30;
    d->m01 = a->m00 * b->m01 + a->m01 * b->m11 + a->m02 * b->m21 + a->m03 * b->m31;
    d->m02 = a->m00 * b->m02 + a->m01 * b->m12 + a->m02 * b->m22 + a->m03 * b->m32;
    d->m03 = a->m00 * b->m03 + a->m01 * b->m13 + a->m02 * b->m23 + a->m03 * b->m33;
    d->m10 = a->m10 * b->m00 + a->m11 * b->m10 + a->m12 * b->m20 + a->m13 * b->m30;
    d->m11 = a->m10 * b->m01 + a->m11 * b->m11 + a->m12 * b->m21 + a->m13 * b->m31;
    d->m12 = a->m10 * b->m02 + a->m11 * b->m12 + a->m12 * b->m22 + a->m13 * b->m32;
    d->m13 = a->m10 * b->m03 + a->m11 * b->m13 + a->m12 * b->m23 + a->m13 * b->m33;
    d->m20 = a->m20 * b->m00 + a->m21 * b->m10 + a->m22 * b->m20 + a->m23 * b->m30;
    d->m21 = a->m20 * b->m01 + a->m21 * b->m11 + a->m22 * b->m21 + a->m23 * b->m31;
    d->m22 = a->m20 * b->m02 + a->m21 * b->m12 + a->m22 * b->m22 + a->m23 * b->m32;
    d->m23 = a->m20 * b->m03 + a->m21 * b->m13 + a->m22 * b->m23 + a->m23 * b->m33;
    d->m30 = a->m30 * b->m00 + a->m31 * b->m10 + a->m32 * b->m20 + a->m33 * b->m30;
    d->m31 = a->m30 * b->m01 + a->m31 * b->m11 + a->m32 * b->m21 + a->m33 * b->m31;
    d->m32 = a->m30 * b->m02 + a->m31 * b->m12 + a->m32 * b->m22 + a->m33 * b->m32;
    d->m33 = a->m30 * b->m03 + a->m31 * b->m13 + a->m32 * b->m23 + a->m33 * b->m33;
    if (d == &tmp)
        *out = tmp;
}

extern "C" void MATRIX4_vect3mult(const Unk804020F0_V4* in, const Unk80402288_Mtx* m, Unk804020F0_V4* out)
{
    if (in == out) {
        Unk804020F0_V4 t;
        t.x = in->x * m->m00 + in->y * m->m10 + in->z * m->m20 + m->m30;
        t.y = in->x * m->m01 + in->y * m->m11 + in->z * m->m21 + m->m31;
        t.z = in->x * m->m02 + in->y * m->m12 + in->z * m->m22 + m->m32;
        t.w = in->x * m->m03 + in->y * m->m13 + in->z * m->m23 + m->m33;
        *out = t;
    } else {
        out->x = in->x * m->m00 + in->y * m->m10 + in->z * m->m20 + m->m30;
        out->y = in->x * m->m01 + in->y * m->m11 + in->z * m->m21 + m->m31;
        out->z = in->x * m->m02 + in->y * m->m12 + in->z * m->m22 + m->m32;
        out->w = in->x * m->m03 + in->y * m->m13 + in->z * m->m23 + m->m33;
    }
}

extern "C" void MATRIX4_vect3mult3(const Unk80402288_V3* in, const Unk80402288_Mtx* m, Unk80402288_V3* out)
{
    if (in == out) {
        Unk80402288_V3 t;
        t.x = in->x * m->m00 + in->y * m->m10 + in->z * m->m20 + m->m30;
        t.y = in->x * m->m01 + in->y * m->m11 + in->z * m->m21 + m->m31;
        t.z = in->x * m->m02 + in->y * m->m12 + in->z * m->m22 + m->m32;
        *out = t;
    } else {
        out->x = in->x * m->m00 + in->y * m->m10 + in->z * m->m20 + m->m30;
        out->y = in->x * m->m01 + in->y * m->m11 + in->z * m->m21 + m->m31;
        out->z = in->x * m->m02 + in->y * m->m12 + in->z * m->m22 + m->m32;
    }
}

extern "C" void MATRIX4_vect4mult(const Unk804020F0_V4* in, const Unk80402288_Mtx* m, Unk804020F0_V4* out)
{
    if (in == out) {
        Unk804020F0_V4 t;
        t.x = in->x * m->m00 + in->y * m->m10 + in->z * m->m20 + in->w * m->m30;
        t.y = in->x * m->m01 + in->y * m->m11 + in->z * m->m21 + in->w * m->m31;
        t.z = in->x * m->m02 + in->y * m->m12 + in->z * m->m22 + in->w * m->m32;
        t.w = in->x * m->m03 + in->y * m->m13 + in->z * m->m23 + in->w * m->m33;
        *out = t;
    } else {
        out->x = in->x * m->m00 + in->y * m->m10 + in->z * m->m20 + in->w * m->m30;
        out->y = in->x * m->m01 + in->y * m->m11 + in->z * m->m21 + in->w * m->m31;
        out->z = in->x * m->m02 + in->y * m->m12 + in->z * m->m22 + in->w * m->m32;
        out->w = in->x * m->m03 + in->y * m->m13 + in->z * m->m23 + in->w * m->m33;
    }
}

extern "C" void fn_80402570(int a, int b);

extern "C" void fn_804027B0()
{
    fn_80402570(1, 0xFFFF);
}
