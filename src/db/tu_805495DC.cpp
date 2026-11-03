/*
 * Database overlay code, .database_codeoverlay 0x805495DC-0x8054CBBC (code-only unit;
 * placeholder TU). Define every function DB_OVERLAY (game/overlay.h); data extern by lbl_ name.
 */
#include "game/db_8053967C.h"

typedef Db805495DC_Rec8 R8;
typedef Db805495DC_Hold H;

extern const float lbl_805FF248;
extern const float lbl_805FF18C;
extern const float lbl_805FF190;
extern const float lbl_805FF194;
extern const float lbl_805FF198;
extern const float lbl_805FF19C;
extern const float lbl_805FF1A0;
extern const float lbl_805FF1A4;
extern const float lbl_805FF1A8;

/* Bit views of R8 words used by the fn_8054C28C..C340 setters. */
struct W0V {
    unsigned pre : 17;
    unsigned fl : 1;
    unsigned p : 4;
    unsigned h : 7;
    unsigned post : 3;
};
struct W4A {
    unsigned p0 : 2;
    unsigned g1 : 7;
    unsigned p1 : 4;
    unsigned g2 : 7;
    unsigned p2 : 2;
    unsigned g3 : 7;
    unsigned p3 : 3;
};
struct W4B {
    unsigned g1 : 7;
    unsigned p0 : 1;
    unsigned g2 : 7;
    unsigned p1 : 1;
    unsigned g3 : 7;
    unsigned p2 : 1;
    unsigned g4 : 7;
    unsigned p3 : 1;
};
struct W4C {
    unsigned p0 : 1;
    unsigned g1 : 7;
    unsigned p1 : 1;
    unsigned g2 : 7;
    unsigned p2 : 1;
    unsigned g3 : 7;
    unsigned p3 : 1;
    unsigned g4 : 7;
};

static inline int IntAt(char* b, int off)
{
    return *(int*)(b + off);
}
static inline void IntSet(char* b, int off, int v)
{
    *(int*)(b + off) = v;
}
static inline float FloatAt(char* b, int off)
{
    return *(float*)(b + off);
}

/* Q18 types */
struct Db805495DC_WE {
    float w;
    int flag;
};

extern "C" int rand();
/* end Q18 types */

extern "C" {
/* Q18 protos */
int fn_805495DC(int a, int k);
char* fn_80549F54(char* o);
int fn_8054B9D4(int x, int a, int b, int c);
int fn_8054BA5C(int x, int y, int k, int z);
int fn_8054BAE0(int x, int y, int k, int z);
float fn_8054BB48(char* o);
int fn_8054BBA0(char* b, int kind, int a);
H* fn_8054BDDC(H* o, Unk8007F198_C* s);
void fn_8054BE40(int* p, int mode);
void fn_8054BEA4(int* p, int mode);
void fn_8054BF08(int* p, int mode);
void fn_8054BF6C(int* p, int mode);
void fn_8054BFD0(int* p, int mode);
void fn_8054C034(int* p, R8* r);
void fn_8054C0AC(int* p, R8* r);
void fn_8054C124(int* p, R8* r);
void fn_8054C19C(int* p, R8* r);
void fn_8054C214(int* p, R8* r);
void fn_8054C28C(int* p, R8* r);
void fn_8054C2B8(int* p, R8* r);
void fn_8054C2E0(int* p, R8* r);
void fn_8054C310(int* p, R8* r);
void fn_8054C340(int* p, R8* r);
void fn_8054C358(int* p, signed char* buf, int n);
int fn_8054C4AC(int* p, R8* r);
int fn_8054C4FC(int* p, R8* r);
int fn_8054C550(int* p, R8* r);
int fn_8054C5B4(int* p, R8* r);
int fn_8054C618(int* p, R8* r);
int fn_8054C648(int* p, int v, int lo, int hi);
H* fn_8054C6E4(H* o);
void fn_8054C75C(H* o, int chrg);
void fn_8054C7D0(H* o);
int fn_8054C940(H* o);
int fn_8054C94C(H* o);
/* end Q18 protos */

/* protos */
int fn_8054B9D4(int x, int a, int b, int c);
int fn_8054BA5C(int x, int y, int k, int z);
int fn_8054BAE0(int x, int y, int k, int z);
float fn_8054BB48(char* o);
H* fn_8054BDDC(H* o, Unk8007F198_C* s);
void fn_8054BE40(int* p, int mode);
void fn_8054BEA4(int* p, int mode);
void fn_8054BF08(int* p, int mode);
void fn_8054BF6C(int* p, int mode);
void fn_8054BFD0(int* p, int mode);
H* fn_8054C6E4(H* o);
void fn_8054C75C(H* o, int chrg);
void fn_8054C7D0(H* o);
int fn_8054C940(H* o);
int fn_8054C94C(H* o);
void fn_8054C034(int* p, R8* r);
void fn_8054C0AC(int* p, R8* r);
void fn_8054C124(int* p, R8* r);
void fn_8054C19C(int* p, R8* r);
void fn_8054C214(int* p, R8* r);
int fn_8054C4AC(int* p, R8* r);
int fn_8054C4FC(int* p, R8* r);
int fn_8054C550(int* p, R8* r);
int fn_8054C5B4(int* p, R8* r);
int fn_8054C618(int* p, R8* r);
int fn_8054C648(int* p, int v, int lo, int hi);
void fn_8054C28C(int* p, R8* r);
void fn_8054C2B8(int* p, R8* r);
void fn_8054C2E0(int* p, R8* r);
void fn_8054C310(int* p, R8* r);
void fn_8054C340(int* p, R8* r);
int fn_805495DC(int a, int k);
char* fn_80549F54(char* o);
/* end protos */

char* fn_805496F4(char* o);

void fn_8054C034(int* p, R8* r);
void fn_8054C0AC(int* p, R8* r);
void fn_8054C124(int* p, R8* r);
void fn_8054C19C(int* p, R8* r);
void fn_8054C214(int* p, R8* r);
void fn_8054C28C(int* p, R8* r);
void fn_8054C2B8(int* p, R8* r);
void fn_8054C2E0(int* p, R8* r);
void fn_8054C310(int* p, R8* r);
void fn_8054C340(int* p, R8* r);
void fn_8054C358(int* p, signed char* buf, int n);
int fn_8054C3F4(int* p, int lo, int hi);
int fn_8054C4AC(int* p, R8* r);
int fn_8054C4FC(int* p, R8* r);
int fn_8054C550(int* p, R8* r);
int fn_8054C5B4(int* p, R8* r);
int fn_8054C618(int* p, R8* r);
int fn_8054C648(int* p, int v, int lo, int hi);
void* fn_8057DF2C(void* p);
void* fn_8055A32C(void* p);
void fn_8057DF38(void* p, int a);
void fn_8055A338(void* p, int a);
void fn_8057DF60(void* p);
void fn_8055A360(void* p);
void fn_80377408(void* p);
int fn_8040ABA0();
/* Q18 decls */
extern "C" int fn_80534078(char* o, int a, float f);
extern "C" int fn_80534304(char* o, float f);
extern const float lbl_805FF230;
struct Db8054BCC8_E {
    float w;
    int used;
};
struct Db8054BCC8_S {
    char pad0[0x14E5C];
    Db8054BCC8_E e[15];
};
int fn_8054BCC8(Db8054BCC8_S* o);
struct Db8054C814_N {
    int m0;
    char pad4[0x10];
    int m14;
};
struct Db8054C814_S {
    int m0;
    int m4;
    char pad8[0xC];
    int* m14;
    Db8054C814_N* m18;
};
extern "C" void fn_8057DF80(int* p);
extern "C" void fn_8055A3B0(Db8054C814_N* p);
extern "C" void fn_8055EAFC(void* x);
extern "C" int fn_80400CE4(int a);
extern "C" int fn_8054C958(Db8054C814_S* o, int a, int b);
int fn_8054C814(Db8054C814_S* o);
/* end Q18 decls */

DB_OVERLAY int fn_805495DC(int a, int k)
{
    int r;
    switch (k) {
    case 2:
        r = 2;
        if (a == 0)
            r = 4;
        {
            int m = fn_8040ABA0() % 3;
            r += m;
        }
        break;
    case 4:
        r = 4;
        if (a == 0)
            r = 2;
        {
            int m = fn_8040ABA0() % 3;
            r += m;
        }
        break;
    case 6:
        r = 2;
        if (a == 0)
            r = 6;
        break;
    case 5:
    case 7:
    case 3:
    case 1:
        r = 4;
        break;
    case 8:
        r = 3;
        if (a == 0)
            r = 5;
        break;
    case 9:
        r = 1;
        if (a == 0)
            r = 7;
        break;
    default:
        r = 0;
        break;
    }
    return r;
}

/* near-miss: float stores 20116..20108 and the lis/lfs constant order differ (scheduling) */
DB_OVERLAY char* fn_80549F54(char* o)
{
    fn_805496F4(o);
    char* b = o + 0x10000;
    int off = 0;
    int c = 15;
    do {
        IntSet(b + 0x4E60, off, 0);
        off += 8;
        c--;
    } while (c != 0);
    *(float*)(b + 0x4E5C) = lbl_805FF18C;
    *(float*)(b + 0x4E64) = lbl_805FF190;
    *(float*)(b + 0x4E6C) = lbl_805FF194;
    *(float*)(b + 0x4E84) = lbl_805FF19C;
    *(float*)(b + 0x4E94) = lbl_805FF1A0;
    *(float*)(b + 0x4E9C) = lbl_805FF1A4;
    *(float*)(b + 0x4EA4) = lbl_805FF1A8;
    *(float*)(b + 0x4E74) = lbl_805FF198;
    *(float*)(b + 0x4E7C) = lbl_805FF198;
    *(float*)(b + 0x4E8C) = lbl_805FF1A0;
    *(float*)(b + 0x4EAC) = lbl_805FF198;
    *(float*)(b + 0x4EB4) = lbl_805FF198;
    *(float*)(b + 0x4EBC) = lbl_805FF198;
    *(float*)(b + 0x4EC4) = lbl_805FF198;
    *(float*)(b + 0x4ECC) = lbl_805FF198;
    return o;
}

DB_OVERLAY int fn_8054B9D4(int x, int a, int b, int c)
{
    float f = fn_8056FFB0();
    int t = fn_805320B0(c, a, b, f);
    int r = fn_8040ABA0() % 5;
    return lbl_806015E0[t] + r;
}

DB_OVERLAY int fn_8054BA5C(int x, int y, int k, int z)
{
    int v = 3;
    if (k == 4 || k == 1)
        v = 1;
    else if (k == 0 || k == 10)
        v = 5;
    else if (k == 6)
        v = 2;
    else if (k == 2)
        v = 0;
    else if (k == 8)
        v = 4;
    return fn_8054B9D4(x, y, v, z);
}

DB_OVERLAY int fn_8054BAE0(int x, int y, int k, int z)
{
    float f = (float)fn_8054BA5C(x, y, k, z);
    return (int)(f * lbl_805FF228);
}

DB_OVERLAY float fn_8054BB48(char* o)
{
    float f = lbl_805FF22C;
    int off = 0;
    for (int i = 0; i < 15; i++) {
        if (IntAt(o + 0x14E60, off) == 0)
            f += FloatAt(o + 0x14E5C, off);
        off += 8;
    }
    return f;
}

DB_OVERLAY int fn_8054BBA0(char* b, int kind, int a)
{
    float f = fn_8056FFB0();
    int v = fn_80534078(b + 0x14AB8, a, f);
    switch (kind) {
    case 1:
    case 2: {
        int r = rand();
        int t = v - 10;
        v = t - r % 7;
        break;
    }
    case 3:
        v = fn_80534304(b + 0x14C00, f);
        break;
    case 4:
    case 6:
    case 7: {
        int r = rand();
        int t = v - 7;
        v = t - r % 3;
        break;
    }
    case 5:
    case 9: {
        int r = rand();
        int t = v - 5;
        v = t - r % 3;
        break;
    }
    case 8: {
        int r = rand();
        int t = v - 3;
        v = t - r % 3;
        break;
    }
    }
    return v;
}

DB_OVERLAY int fn_8054BCC8(Db8054BCC8_S* o)
{
    int r = 1;
    float rnd = fn_8056FFB0();
    float acc = lbl_805FF230;
    float d = fn_8054BB48((char*)o);
    unsigned i;
    for (i = 1; i <= 14; i++) {
        if (o->e[i].used == 0) {
            acc += o->e[i].w / d;
            if (rnd < acc) {
                o->e[i].used = 1;
                r = i;
                break;
            }
        }
    }
    if (i > 14) {
        int j = 14;
        char* b = (char*)o + 0x14E60;
        if (*(int*)(b + 0x70) == 0) {
            r = 14;
            *(int*)(b + 0x70) = 1;
        } else {
        again:
            j--;
            if (j > 0) {
                int off = j * 8;
                char* b2 = (char*)o + 0x14E60;
                if (*(int*)(b2 + off) != 0)
                    goto again;
                r = j;
                *(int*)(b2 + off) = 1;
            }
        }
    }
    return r;
}

/* near-miss: retail reads the 4-byte fn_80329FA0 buffer straight from the frame (addi r3,r1,8); ours keeps &buf in a saved register */
DB_OVERLAY H* fn_8054BDDC(H* o, Unk8007F198_C* s)
{
    Db8053CC9C_Buf3 buf;
    o->m4 = 0;
    o->m0 = (int)s;
    fn_80329FA0(&buf, fn_8007FBC0(s, 0, 0));
    o->m4 = buf.b;
    return o;
}

DB_OVERLAY void fn_8054BE40(int* p, int mode)
{
    R8 sp8;
    fn_8054C034(p, &sp8);
    if (mode != 0) {
        if (mode == 1)
            fn_8054C28C(p, &sp8);
    }
    ((Unk8007B2B0_Screen*)*p)->fn_8007BEA0(sp8.w0, (int)&sp8);
}

DB_OVERLAY void fn_8054BEA4(int* p, int mode)
{
    R8 sp8;
    fn_8054C0AC(p, &sp8);
    if (mode != 0) {
        if (mode == 1)
            fn_8054C2B8(p, &sp8);
    }
    ((Unk8007B2B0_Screen*)*p)->fn_8007BEF4(sp8.w0, (int)&sp8);
}

DB_OVERLAY void fn_8054BF08(int* p, int mode)
{
    R8 sp8;
    fn_8054C124(p, &sp8);
    if (mode != 0) {
        if (mode == 1)
            fn_8054C2E0(p, &sp8);
    }
    ((Unk8007B2B0_Screen*)*p)->fn_8007BF44(sp8.w0, (int)&sp8);
}

DB_OVERLAY void fn_8054BF6C(int* p, int mode)
{
    R8 sp8;
    fn_8054C19C(p, &sp8);
    if (mode != 0) {
        if (mode == 1)
            fn_8054C310(p, &sp8);
    }
    ((Unk8007B2B0_Screen*)*p)->fn_8007BF94(sp8.w0, (int)&sp8);
}

DB_OVERLAY void fn_8054BFD0(int* p, int mode)
{
    R8 sp8;
    fn_8054C214(p, &sp8);
    if (mode != 0) {
        if (mode == 1)
            fn_8054C340(p, &sp8);
    }
    ((Unk8007B2B0_Screen*)*p)->fn_8007BFE4(sp8.w0, (int)&sp8);
}

DB_OVERLAY void fn_8054C034(int* p, R8* r)
{
    fn_8040074C(r, 8);
    fn_8054C358(p, (signed char*)r, 8);
    Db805495DC_Bits* b = (Db805495DC_Bits*)r;
    b->a = fn_8054C3F4(p, 0x28, 0x37);
    b->f1 = fn_8054C4AC(p, r);
    b->f2 = 0;
    b->fl = 0;
}

DB_OVERLAY void fn_8054C0AC(int* p, R8* r)
{
    fn_8040074C(r, 8);
    fn_8054C358(p, (signed char*)r, 8);
    Db805495DC_Bits* b = (Db805495DC_Bits*)r;
    b->a = fn_8054C3F4(p, 0x23, 0x37);
    b->f1 = fn_8054C4FC(p, r);
    b->f2 = 0;
    b->fl = 0;
}

DB_OVERLAY void fn_8054C124(int* p, R8* r)
{
    fn_8040074C(r, 8);
    fn_8054C358(p, (signed char*)r, 8);
    Db805495DC_Bits* b = (Db805495DC_Bits*)r;
    b->a = fn_8054C3F4(p, 0x23, 0x37);
    b->f1 = fn_8054C550(p, r);
    b->f2 = 0;
    b->fl = 0;
}

DB_OVERLAY void fn_8054C19C(int* p, R8* r)
{
    fn_8040074C(r, 8);
    fn_8054C358(p, (signed char*)r, 8);
    Db805495DC_Bits* b = (Db805495DC_Bits*)r;
    b->a = fn_8054C3F4(p, 0x23, 0x37);
    b->f1 = fn_8054C5B4(p, r);
    b->f2 = 0;
    b->fl = 0;
}

DB_OVERLAY void fn_8054C214(int* p, R8* r)
{
    fn_8040074C(r, 8);
    fn_8054C358(p, (signed char*)r, 8);
    Db805495DC_Bits* b = (Db805495DC_Bits*)r;
    b->a = fn_8054C3F4(p, 0x23, 0x37);
    b->f1 = fn_8054C618(p, r);
    b->f2 = 0;
    b->fl = 0;
}

DB_OVERLAY void fn_8054C28C(int* p, R8* r)
{
    W0V* a = (W0V*)&r->w0;
    W4A* b = (W4A*)&r->w4;
    a->fl = 1;
    a->h = 0x44;
    b->g1 = 0x44;
    b->g2 = 0x44;
    b->g3 = 0x44;
}

DB_OVERLAY void fn_8054C2B8(int* p, R8* r)
{
    W0V* a = (W0V*)&r->w0;
    W4A* b = (W4A*)&r->w4;
    b->g1 = 0x44;
    b->g2 = 0x44;
    a->fl = 1;
    a->h = 0x44;
}

DB_OVERLAY void fn_8054C2E0(int* p, R8* r)
{
    W0V* a = (W0V*)&r->w0;
    W4B* b = (W4B*)&r->w4;
    a->fl = 1;
    a->h = 0x44;
    b->g1 = 0x44;
    b->g2 = 0x44;
    b->g3 = 0x44;
    b->g4 = 0x44;
}

DB_OVERLAY void fn_8054C310(int* p, R8* r)
{
    W0V* a = (W0V*)&r->w0;
    W4C* b = (W4C*)&r->w4;
    a->fl = 1;
    a->h = 0x44;
    b->g1 = 0x44;
    b->g2 = 0x44;
    b->g3 = 0x44;
    b->g4 = 0x44;
}

DB_OVERLAY void fn_8054C340(int* p, R8* r)
{
    W0V* a = (W0V*)&r->w0;
    a->fl = 1;
    a->h = 0x44;
}

DB_OVERLAY void fn_8054C358(int* p, signed char* buf, int n)
{
    for (int i = 0; i < n; i++) {
        float r = fn_8056FFB0();
        *buf = (unsigned char)(unsigned)(r * 256.0f);
        buf++;
    }
}

struct Db8054C3F4_Out {
    unsigned char m0;
    unsigned char m1;
    unsigned short m2;
};
typedef Db8054C3F4_Out (*Db8054C3F4_Fn)(int, ...);
DB_OVERLAY int fn_8054C3F4(int* p, int lo, int hi)
{
    float f = fn_8056FFB0();
    int base = lo + (int)(f * (float)(hi - lo));
    int t = p[1];
    t -= base;
    t -= ((Db8054C3F4_Fn)fn_80329FA0)(1).m2;
    int c;
    if (t <= 0xFF) {
        c = t;
        if (c < 0)
            c = 0;
    } else {
        c = 0xFF;
    }
    return c;
}

DB_OVERLAY int fn_8054C4AC(int* p, R8* r)
{
    return fn_8054C648(p, (((r->w0 >> 3) & 0x7F) + ((r->w4 >> 23) & 0x7F) + ((r->w4 >> 12) & 0x7F) + ((r->w4 >> 3) & 0x7F)) >> 2, 1, 0x14);
}

DB_OVERLAY int fn_8054C4FC(int* p, R8* r)
{
    return fn_8054C648(p, (((r->w0 >> 3) & 0x7F) + ((r->w4 >> 23) & 0x7F) + ((r->w4 >> 12) & 0x7F)) / 3, 1, 0xA);
}

DB_OVERLAY int fn_8054C550(int* p, R8* r)
{
    return fn_8054C648(p, (((r->w0 >> 3) & 0x7F) + (r->w4 >> 25) + ((r->w4 >> 17) & 0x7F) + ((r->w4 >> 9) & 0x7F) + ((r->w4 >> 1) & 0x7F)) / 5, 1, 0xA);
}

DB_OVERLAY int fn_8054C5B4(int* p, R8* r)
{
    return fn_8054C648(p, (((r->w0 >> 3) & 0x7F) + ((r->w4 >> 24) & 0x7F) + ((r->w4 >> 16) & 0x7F) + ((r->w4 >> 8) & 0x7F) + (r->w4 & 0x7F)) / 5, 0, 4);
}

DB_OVERLAY int fn_8054C618(int* p, R8* r)
{
    return fn_8054C648(p, (r->w0 >> 3) & 0x7F, 0, 2);
}

DB_OVERLAY int fn_8054C648(int* p, int v, int lo, int hi)
{
    int k = (int)(2.0f * fn_8056FFB0() * lbl_805FF248);
    k -= 0x10;
    int t = v + k;
    int c;
    if (t <= 0x7F) {
        c = t;
        if (c < 0)
            c = 0;
    } else {
        c = 0x7F;
    }
    return lo + (c * (hi - lo)) / 127;
}

DB_OVERLAY H* fn_8054C6E4(H* o)
{
    o->m4 = 0;
    o->m14 = 0;
    o->m18 = 0;
    o->m0 = 3;
    o->m14 = fn_8057DF2C(operator new(4, 0));
    if (o->m18 == 0)
        o->m18 = (Db805495DC_Sub*)fn_8055A32C(operator new(0x20, 0));
    return o;
}

DB_OVERLAY void fn_8054C75C(H* o, int chrg)
{
    if (o->m14 != 0) {
        fn_8057DF38(o->m14, 3);
        o->m14 = 0;
    }
    if (o->m18 != 0) {
        fn_8055A338(o->m18, 3);
        o->m18 = 0;
    }
    if (chrg & 1)
        fn_80377408(o);
}

DB_OVERLAY void fn_8054C7D0(H* o)
{
    o->m0 = 0;
    o->m4 = 0;
    fn_8057DF60(o->m14);
    fn_8055A360(o->m18);
}

DB_OVERLAY int fn_8054C814(Db8054C814_S* o)
{
    o->m4 = 1;
    switch (o->m0) {
    case 0:
        fn_8057DF80(o->m14);
        {
            int v = *o->m14;
            if (v == 2)
                goto next;
            if (v == 1)
                o->m4 = 5;
        }
        break;
    case 1: {
        unsigned n = 0;
        do {
            if ((n & 0x1FF) == 0)
                fn_80400CE4(0);
            n++;
            fn_8055A3B0(o->m18);
        } while (o->m18->m0 == 0);
        switch (o->m18->m0) {
        case 3:
            goto next;
        case 2:
            fn_8055EAFC((void*)o->m18->m14);
            break;
        case 1:
            o->m4 = 3;
            break;
        }
        break;
    }
    case 2:
        if (fn_8054C958(o, 0, 1) != 0) {
            o->m4 = 4;
            break;
        }
    next:
        o->m0 = o->m0 + 1;
        break;
    case 3:
        o->m4 = 0;
        break;
    default:
        o->m4 = 0;
    }
    return o->m4;
}

DB_OVERLAY int fn_8054C940(H* o)
{
    return o->m18->m10;
}

DB_OVERLAY int fn_8054C94C(H* o)
{
    return o->m18->m1C;
}

extern "C" int fn_800A2AE8(void*);
extern "C" int fn_8007F674__FP13Unk8007F198_Aii(Unk8007F198_A*, int, int);
extern "C" int fn_8007F6C4__FP13Unk8007F198_Aiii(Unk8007F198_A*, int, int, int);
extern "C" int fn_8007F724__FP13Unk8007F198_Aiiii(Unk8007F198_A*, int, int, int, int);
extern "C" int fn_8007B320__18Unk8007B2B0_Screenii(void*, int, int);
extern "C" int fn_80075948__18Unk8007B2B0_Screeni(void*, int);
extern "C" int fn_80078A38__18Unk8007B2B0_Screeni(void*, int);
extern "C" void fn_805601F0(int, int);

DB_OVERLAY int fn_8054C958(Db8054C814_S* out, int a, int b)
{
    Unk8007A4DC_Screen* screen = (Unk8007A4DC_Screen*)lbl_8061930C;
    Db8053E240_Tmp list;
    int values[0x82];
    list.C1(values, 0x82);
    list.C2();
    list.C3(lbl_8064A2A8);
    fn_805371C4(&list, 0x01FFFFFF);
    int token = fn_800A2AE8(lbl_80619948);
    Db8053E240_Tmp* q = &list;
    int result = 0;
    unsigned x = 0;
    if (x < q->m4) {
        int off = 0;
        do {
            int id = *(int*)((char*)list.m0 + off);
            unsigned mode = 0;
            do {
                unsigned n = fn_8007F674__FP13Unk8007F198_Aii((Unk8007F198_A*)screen, id, mode);
                unsigned i = 0;
                if (i < n) {
                    do {
                        int value = fn_8007F6C4__FP13Unk8007F198_Aiii((Unk8007F198_A*)screen, id, mode, i);
                        if (fn_8007F724__FP13Unk8007F198_Aiiii((Unk8007F198_A*)screen, id, mode, value, token) <= 0) {
                            int category = fn_8007F7D0((Unk8007F198_B*)screen, 0);
                            int c0 = fn_8007B320__18Unk8007B2B0_Screenii(screen, 0, category);
                            int c1 = fn_8007B320__18Unk8007B2B0_Screenii(screen, 1, category);
                            int c2 = fn_8007B320__18Unk8007B2B0_Screenii(screen, 2, category);
                            int c3 = fn_8007B320__18Unk8007B2B0_Screenii(screen, 3, category);
                            int known = id == c0 || id == c1 || id == c2 || id == c3;
                            if ((known && a != 0) || (!known && b != 0)) {
                                int sub = fn_80075948__18Unk8007B2B0_Screeni(lbl_8061930C, id);
                                int kind = fn_80078A38__18Unk8007B2B0_Screeni(lbl_8061930C, id);
                                fn_805601F0(sub, kind);
                                fn_8055EAFC((void*)sub);
                                goto nextMode;
                            } else {
                                *(int*)((char*)out + 8) = value;
                                result = 1;
                                *(int*)((char*)out + 0xC) = id;
                                *(int*)((char*)out + 0x10) = mode;
                            }
                        }
                        ++i;
                    } while (i < n);
                }
nextMode:
                ++mode;
            } while (mode <= 1);
            ++x;
            off += 4;
        } while (x < q->m4);
    }
    list.vp = lbl_8064A550;
    return result;
}


}
