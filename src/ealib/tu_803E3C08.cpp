/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803E3C08-0x803E6FB8 (code-only unit, 39 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

#include "../../include/game/tu_803E097C.h"

/* Vertex-array object (alloc 0x3C bytes): ctor fn_803E6660, deleting dtor fn_803E6708. */
struct Unk803E6660_Rec {
    void* m0;
    int m4;
    int m8;
};

struct Unk803E6660_Src {
    int* m0;
};

struct Unk803E6660_Hdr {
    char pad0[4];
    int* m4;
    int* m8;
};

struct Unk803E6660_Ent {
    int m0;
    int m4;
};

struct Unk803E6660_Holder {
    Unk803E6660_Hdr* m0;
    char pad4[4];
    Unk803E6660_Ent e[1];
};

struct Unk803E6660 {
    int m0;
    void* m4;
    Unk803E6660_Holder* m8;
    Unk803E6660_Rec* mC;
    int m10;
    int m14;
    int m18;
    int m1C;
    int m20;
    int m24;
    int m28;
    int m2C;
    int m30;
    char pad34[4];
    int* m38;
};

extern void (*lbl_8062C66C)(void* p, int size);
extern void* (*lbl_8062C668)(int size, int heap);
struct Unk803E4964_Glob {
    int v;
};
extern Unk803E4964_Glob lbl_806AAB5C;
extern int* lbl_8062C660;
extern int lbl_8062C664;

extern "C" void* fn_803D700C(void* mem);
extern "C" void fn_803D7600(void* p, int chrg);
extern "C" int fn_803D76F4(int a, int b, int c);
extern "C" void DCStoreRangeNoSync(void* p, int n);
extern "C" void GXInvalidateVtxCache();
extern "C" void PPCSync();
extern "C" int strcmp(const char* a, const char* b);
extern "C" void fn_803D987C(void* p);
struct Unk803E68B0_Key {
    int m0;
    char pad4[0x1C];
    int m20;
    int m24;
    int m28;
    int m2C;
    int m30;
};

inline void SubCtor(Unk803E6660* o)
{
    o->m0 = 0xB8;
    o->m8 = 0;
    o->mC = 0;
    o->m10 = 0;
    o->m14 = 0;
    o->m18 = 0;
    o->m1C = 0;
    o->m20 = 0;
    o->m24 = 0;
    o->m28 = 0;
    o->m2C = -1;
    o->m30 = 0;
    o->m4 = fn_803D700C(lbl_8062C668(0x58, 0));
    o->m38 = (int*)lbl_8062C668(0x104, 0);
    *o->m38 = -1;
}

inline void ArrDel(int* p)
{
    if (p) {
        int* b = p - 1;
        lbl_8062C66C(b, *b + 4);
    }
}

inline void SubDtor(Unk803E6660* o)
{
    if (o->mC) {
        int* base = (int*)o->mC - 1;
        lbl_8062C66C(base, *base + 4);
    }
    if (o->m4)
        fn_803D7600(o->m4, 3);
    lbl_8062C66C(o->m38, 0x104);
}

extern "C" void InitByHarmonics(float* dst, const float* src)
{
    float* d = dst;
    for (int i = 0; i < 3; i++) {
        d[0] = src[0];
        d[4] = src[3];
        d[8] = -src[3];
        d[12] = src[6];
        d[16] = src[9];
        d[20] = src[12];
        d[24] = src[15];
        d[28] = src[18];
        d[32] = src[21];
        d[36] = src[24];
        src++;
        d++;
    }
    float* p = dst + 39;
    for (int j = 0; j < 10; j++) {
        *p = 0.0f;
        p -= 4;
    }
}

extern "C" void fn_803E4818(float* m);
extern "C" void fn_803D71F0(void* p);

extern "C" void fn_803E4450(float* dst, float* src, unsigned int* color, float scale)
{
    unsigned int c = *color;
    for (int i = 0; i < 10; i++) {
        dst[0] += src[0] * scale * (float)(unsigned char)(c >> 16) / 255.0f;
        dst[1] += src[1] * scale * (float)(unsigned char)(c >> 8) / 255.0f;
        dst[2] += src[2] * scale * (float)(unsigned char)c / 255.0f;
        dst[3] += src[3] * scale * (float)(c >> 24) / 255.0f;
        src += 4;
        dst += 4;
    }
}

extern "C" float* fn_803E47E8(float* m)
{
    fn_803E4818(m);
    return m;
}

extern "C" void fn_803E4818(float* m)
{
    for (int i = 0; i < 10; i++) {
        m[0] = m[1] = m[2] = m[3] = 0.0f;
        m += 4;
    }
}

struct Unk803E4844_Quat {
    float x, y, z, w;
};

extern float lbl_8060CABC;
extern float lbl_8060CAC0;
extern "C" void fn_803E41F4(float* dst, Unk803E4844_Quat* q, float s, float x, float y, float z);

extern "C" void UpdateByDirectionalLight(float* dst, const float* axis, Unk803E4844_Quat* q)
{
    float one = lbl_8060CABC;
    float ax = axis[0];
    float ay = axis[1];
    float az = axis[2];
    Unk803E4844_Quat tmp = *q;
    fn_803E41F4(dst, &tmp, one, ax, ay, az);
    q->x = -q->x / lbl_8060CAC0;
    q->y = -q->y / lbl_8060CAC0;
    q->z = -q->z / lbl_8060CAC0;
    q->w = -q->w / lbl_8060CAC0;
    tmp = *q;
    fn_803E41F4(dst, &tmp, one, -ax, -ay, -az);
}

extern "C" void fn_803E4964(Unk803E6660* o, int idx)
{
    int base = idx * o->m24;
    lbl_806AAB5C.v = o->m1C - base;
    if (lbl_806AAB5C.v > o->m20)
        lbl_806AAB5C.v = o->m20;
    Unk803E6660_Ent* e = o->m8->e;
    for (int i = 0; i < o->m10; i++) {
        int off = i * 12;
        int n = *(int*)(off + (int)o->mC + 4);
        if (n != 0) {
            e[i].m4 = *(int*)(off + (int)o->mC) + base * n;
            int v = *(int*)(off + (int)o->mC + 8);
            if (v == -1)
                e[i].m0 = lbl_806AAB5C.v;
            else
                e[i].m0 = v;
        }
    }
    if (o->m2C != -1) {
        e[o->m2C].m4 = (int)&lbl_806AAB5C;
        e[o->m2C].m0 = 1;
    }
}

extern "C" void fn_803DBD84(void* p, int chrg);
extern "C" void* fn_803E9B98(const char* name, int* count);
extern "C" void* fn_803DBD54(void* p);
extern char lbl_8060CAE0[];
extern char lbl_80693CFC[];
extern char lbl_806AABD0[];

/* Sub-object (data pointer, flag at +0x10, name handle at +0x14) built by the ctors below. */
struct Unk803E4A50_S4C {
    const char* m0;
    int m4;
    char pad8[8];
    int m10;
    void* m14;
};

struct Unk803E4A50 {
    int* m0;
    int* m4;
    int* m8;
    Unk803E6660 sub;
    int m48;
    Unk803E4A50_S4C s4C;
};

extern "C" Unk803E4A50* fn_803E4A50(Unk803E4A50* o)
{
    o->m0 = 0;
    o->m4 = 0;
    o->m8 = 0;
    SubCtor(&o->sub);
    Unk803E4A50_S4C* q = &o->s4C;
    int n;
    o->s4C.m0 = lbl_80693CFC;
    o->m48 = 0xB8;
    q->m4 = 0;
    q->m14 = fn_803E9B98(lbl_8060CAE0, &n);
    q->m10 = 1;
    fn_803DBD54((char*)o + 0x7C);
    return o;
}

struct Unk803E4B40 {
    int* m0;
    int* m4;
    char pad8[4];
    Unk803E6660 sub;
};

extern "C" void fn_803E4B40(Unk803E4B40* o, int chrg)
{
    ArrDel(o->m0);
    ArrDel(o->m4);
    fn_803DBD84((char*)o + 0x7C, 2);
    SubDtor(&o->sub);
    if (chrg & 1)
        lbl_8062C66C(o, 0xC8);
}

inline int BuildInl(Unk803E6660* o, int total)
{
    int ok = 0;
    if (o->m18 == 0) {
        o->m1C = total;
        int* hdr = o->m38;
        o->m14 = (total - o->m30 - 1) / o->m24 + 1;
        if (*hdr == -1 || *(unsigned char*)hdr == ((unsigned char*)o)[3]) {
            lbl_8062C660 = hdr;
            lbl_8062C664 = 0x104;
        }
        for (int i = 0; i < o->m14; i++) {
            fn_803E4964(o, i);
            fn_803D71F0(o->m4);
        }
        lbl_8062C660 = 0;
        lbl_8062C664 = 0;
        ok = 1;
    }
    return ok;
}

inline int FlushInl(Unk803E6660* o)
{
    if (o->m18) {
        for (int i = 0; i < o->m10; i++) {
            int off = i * 12;
            Unk803E6660_Rec* r = (Unk803E6660_Rec*)(off + (int)o->mC);
            if (r->m4 && r->m8 > 0)
                DCStoreRangeNoSync(*(void**)(off + (int)o->mC), o->m8->m0->m8[i] * r->m8);
        }
        GXInvalidateVtxCache();
        PPCSync();
    }
    o->m18 = 0;
    return 1;
}

/* name table lookup: index of the entry equal to `name`, or -1 (stored in m2C) */
struct Unk803E6950_Names {
    char pad0[4];
    char** m4;
};

struct Unk803E6950_Tbl {
    Unk803E6950_Names* m0;
    char pad4[4];
    Unk803E6660_Ent e[1];
};

struct Unk803E6950 {
    int m0;
    int m4;
    Unk803E6950_Tbl* m8;
    Unk803E6660_Rec* mC;
    int m10;
    char pad14[0x2C - 0x14];
    int m2C;
};

extern "C" int fn_803E6A70(Unk803E6950* o, int index, int a, int b, int c);

inline int FindSet(Unk803E6950* o, const char* name, int a, int b, int c)
{
    int found = -1;
    for (int i = 0; i < o->m10; i++) {
        if (strcmp(name, o->m8->m0->m4[i]) == 0) {
            found = i;
            break;
        }
    }
    return fn_803E6A70(o, found, a, b, c);
}

extern "C" void fn_803DBDD8(void* m, int a);
extern "C" void fn_803D707C(void* self, void* item);
extern char lbl_8060CB24[];
extern char lbl_8060CB74[];
extern char lbl_8060CB90[];
extern char lbl_8060CBAC[];
extern char lbl_8060CBB8[];
extern char lbl_8060CBC4[];
extern char lbl_8060CBCC[];
extern char lbl_8060CBD4[];
extern int* lbl_8062C294;

inline int InitInl(Unk803E6660* o, Unk803E6660_Holder* h)
{
    o->m8 = h;
    int ok = 0;
    fn_803D707C(o->m4, h);
    o->m10 = 0;
    while (o->m8->m0->m4[o->m10] != 0)
        o->m10++;
    int sz = o->m10 * 12;
    int* blk = (int*)lbl_8062C668(sz + 4, (int)lbl_8060CB24);
    *blk = sz;
    Unk803E6660_Rec* a = (Unk803E6660_Rec*)(blk + 1);
    o->mC = a;
    if (a != 0) {
        for (int i = 0; i < o->m10; i++) {
            o->mC[i].m0 = 0;
            o->mC[i].m4 = 0;
            o->mC[i].m8 = -1;
        }
        ok = 1;
    }
    return ok;
}

inline int FindIdx(Unk803E6950* o, const char* name)
{
    int found = -1;
    for (int i = 0; i < o->m10; i++) {
        if (strcmp(name, o->m8->m0->m4[i]) == 0) {
            found = i;
            break;
        }
    }
    return found;
}

extern "C" int fn_803E4C30(Unk803E4A50* o)
{
    int n1 = 0x900;
    int n2 = 0x300;
    int* a = (int*)lbl_8062C668(n1 + 4, (int)lbl_8060CB74);
    *a = n1;
    o->m0 = a + 1;
    int* b = (int*)lbl_8062C668(n2 + 4, (int)lbl_8060CB90);
    *b = n2;
    o->m4 = b + 1;
    fn_803DBDD8((char*)o + 0x7c, o->m48);
    InitInl(&o->sub, (Unk803E6660_Holder*)&o->s4C);
    Unk803E68B0_Key* kk = (Unk803E68B0_Key*)&o->sub;
    int k = o->m48;
    int v = 0;
    switch (k) {
    case -4:
    case 0x80:
    case 0x90:
    case 0xA8:
    case 0xB8:
        break;
    case 0x98:
        v = 2;
        break;
    case 0xA0:
        v = -1;
        break;
    default:
        v = -1;
        break;
    }
    *(volatile int*)&kk->m30 = v;
    int t = kk->m30;
    if (t != -1) {
        kk->m0 = k;
        int hi = 0xFFFF - t;
        int lo = hi + t;
        kk->m24 = hi;
        kk->m20 = lo;
        kk->m28 = lo;
    }
    Unk803E6950* s = (Unk803E6950*)&o->sub;
    int found = FindIdx(s, lbl_8060CBAC);
    s->m2C = found;
    FindSet(s, lbl_8060CBB8, (int)o->m0, 0xC, -1);
    FindSet(s, lbl_8060CBC4, (int)o->m4, 4, -1);
    FindSet(s, lbl_8060CBAC, (int)o + 0xB8, 0, -1);
    FindSet(s, lbl_8060CBCC, (int)o + 0x7C, 0, -1);
    FindSet(s, lbl_8060CBD4, (int)lbl_8062C294, 0, -1);
    o->m8 = 0;
    return 1;
}

extern "C" void fn_803DBDD8(void* m, int a);

/* fn_803D987C takes no parameters; r3 is not set at the call. */
inline int ReadyInl(Unk803E6660* o)
{
    ((void (*)())fn_803D987C)();
    o->m18 = 1;
    return 1;
}

extern "C" void fn_803E50BC(char* o, int k)
{
    if (k != *(int*)(o + 0x48)) {
        int st = *(int*)(o + 8);
        if (st != 0) {
            BuildInl((Unk803E6660*)(o + 0xc), st);
            *(int*)(o + 8) = 0;
        }
        *(int*)(o + 0x48) = k;
        Unk803E68B0_Key* kk = (Unk803E68B0_Key*)(o + 0xc);
        int k2 = k;
        int v = 0;
        switch (k2) {
        case -4:
        case 0x80:
        case 0x90:
        case 0xA8:
        case 0xB8:
            break;
        case 0x98:
            v = 2;
            break;
        case 0xA0:
            v = -1;
            break;
        default:
            v = -1;
            break;
        }
        *(volatile int*)&kk->m30 = v;
        int t = kk->m30;
        if (t != -1) {
            kk->m0 = k2;
            int hi = 0xFFFF - t;
            int lo = hi + t;
            kk->m24 = hi;
            kk->m20 = lo;
            kk->m28 = lo;
        }
        fn_803DBDD8(o + 0x7c, *(int*)(o + 0x48));
    }
    ReadyInl((Unk803E6660*)(o + 0xc));
}

extern "C" void fn_803E5270(char* o)
{
    Unk803E6660* s = (Unk803E6660*)(o + 0xc);
    FlushInl(s);
    int st = *(int*)(o + 8);
    if (st != 0) {
        BuildInl((Unk803E6660*)(o + 0xc), st);
        *(int*)(o + 8) = 0;
    }
    int k = *(int*)(o + 0x48);
    if (k == 0x98 || k == 0xb0) {
        int st2 = *(int*)(o + 8);
        if (st2 != 0) {
            BuildInl((Unk803E6660*)(o + 0xc), st2);
            *(int*)(o + 8) = 0;
        }
    }
}

struct Unk803E56F8 {
    int* m0;
    int* m4;
    int* m8;
    int* mC;
    Unk803E6660 sub;
    int m4C;
    Unk803E4A50_S4C s50;
};

extern "C" void fn_803E54B8(char* o, Unk803E6660_Rec* rec, int* val)
{
    if (*(int*)(o + 8) == 0xC0) {
        Unk803E6660* s = (Unk803E6660*)(o + 0xc);
        FlushInl(s);
        int st = *(int*)(o + 8);
        if (st != 0) {
            BuildInl((Unk803E6660*)(o + 0xc), st);
            *(int*)(o + 8) = 0;
        }
        ReadyInl((Unk803E6660*)(o + 0xc));
        switch (*(int*)(o + 0x48)) {
        case 0x98:
            fn_803E54B8(o, (Unk803E6660_Rec*)(*(char**)o + 0x8E8), (int*)(*(char**)(o + 4) + 0x2F8));
            fn_803E54B8(o, (Unk803E6660_Rec*)(*(char**)o + 0x8F4), (int*)(*(char**)(o + 4) + 0x2FC));
            break;
        case 0xB0:
            fn_803E54B8(o, (Unk803E6660_Rec*)(*(char**)o + 0x8F4), (int*)(*(char**)(o + 4) + 0x2FC));
            break;
        }
    }
    int n = *(int*)(o + 8);
    *(Unk803E6660_Rec*)(*(char**)o + n * 12) = *rec;
    ((int*)*(char**)(o + 4))[*(int*)(o + 8)] = *val;
    *(int*)(o + 8) = *(int*)(o + 8) + 1;
}

extern "C" Unk803E56F8* fn_803E56F8(Unk803E56F8* o)
{
    o->m0 = 0;
    o->m4 = 0;
    o->m8 = 0;
    o->mC = 0;
    SubCtor(&o->sub);
    Unk803E4A50_S4C* q = &o->s50;
    int n;
    o->s50.m0 = lbl_806AABD0;
    o->m4C = 0x98;
    q->m4 = 0;
    q->m14 = fn_803E9B98(lbl_8060CAE0, &n);
    q->m10 = 1;
    fn_803DBD54((char*)o + 0x90);
    return o;
}

struct Unk803E57F0 {
    int* m0;
    int* m4;
    int* m8;
    char padC[4];
    Unk803E6660 sub;
};

extern "C" void fn_803E57F0(Unk803E57F0* o, int chrg)
{
    ArrDel(o->m0);
    ArrDel(o->m4);
    ArrDel(o->m8);
    fn_803DBD84((char*)o + 0x90, 2);
    SubDtor(&o->sub);
    if (chrg & 1)
        lbl_8062C66C(o, 0xDC);
}

extern char lbl_8060CBE0[];
extern char lbl_8060CBFC[];
extern char lbl_8060CC18[];
extern char lbl_8060CC1C[];

extern "C" int fn_803E5904(Unk803E56F8* o)
{
    int n1 = 0x900;
    int n2 = 0x300;
    int n3 = 0x600;
    int* a = (int*)lbl_8062C668(n1 + 4, (int)lbl_8060CBE0);
    *a = n1;
    o->m0 = a + 1;
    int* b = (int*)lbl_8062C668(n2 + 4, (int)lbl_8060CB90);
    *b = n2;
    o->m4 = b + 1;
    int* c = (int*)lbl_8062C668(n3 + 4, (int)lbl_8060CBFC);
    *c = n3;
    o->m8 = c + 1;
    fn_803DBDD8((char*)o + 0x90, o->m4C);
    InitInl(&o->sub, (Unk803E6660_Holder*)&o->s50);
    Unk803E68B0_Key* kk = (Unk803E68B0_Key*)&o->sub;
    int k = o->m4C;
    int v = 0;
    switch (k) {
    case -4:
    case 0x80:
    case 0x90:
    case 0xA8:
    case 0xB8:
        break;
    case 0x98:
        v = 2;
        break;
    case 0xA0:
        v = -1;
        break;
    default:
        v = -1;
        break;
    }
    *(volatile int*)&kk->m30 = v;
    int t = kk->m30;
    if (t != -1) {
        kk->m0 = k;
        int hi = 0xFFFF - t;
        int lo = hi + t;
        kk->m24 = hi;
        kk->m20 = lo;
        kk->m28 = lo;
    }
    Unk803E6950* s = (Unk803E6950*)&o->sub;
    int found = FindIdx(s, lbl_8060CBAC);
    s->m2C = found;
    FindSet(s, lbl_8060CBB8, (int)o->m0, 0xC, -1);
    FindSet(s, lbl_8060CBC4, (int)o->m4, 4, -1);
    FindSet(s, lbl_8060CC18, (int)o->m8, 8, -1);
    FindSet(s, lbl_8060CC1C, 0, 0, -1);
    FindSet(s, lbl_8060CBAC, (int)o + 0xCC, 0, -1);
    FindSet(s, lbl_8060CBCC, (int)o + 0x90, 0, -1);
    FindSet(s, lbl_8060CBD4, (int)lbl_8062C294, 0, -1);
    o->mC = 0;
    return 1;
}

extern "C" void fn_803E5E90(char* o, int k)
{
    if (k != *(int*)(o + 0x4c)) {
        int st = *(int*)(o + 0xc);
        if (st != 0) {
            BuildInl((Unk803E6660*)(o + 0x10), st);
            *(int*)(o + 0xc) = 0;
        }
        *(int*)(o + 0x4c) = k;
        Unk803E68B0_Key* kk = (Unk803E68B0_Key*)(o + 0x10);
        int k2 = k;
        int v = 0;
        switch (k2) {
        case -4:
        case 0x80:
        case 0x90:
        case 0xA8:
        case 0xB8:
            break;
        case 0x98:
            v = 2;
            break;
        case 0xA0:
            v = -1;
            break;
        default:
            v = -1;
            break;
        }
        *(volatile int*)&kk->m30 = v;
        int t = kk->m30;
        if (t != -1) {
            kk->m0 = k2;
            int hi = 0xFFFF - t;
            int lo = hi + t;
            kk->m24 = hi;
            kk->m20 = lo;
            kk->m28 = lo;
        }
        fn_803DBDD8(o + 0x90, *(int*)(o + 0x4c));
    }
    ReadyInl((Unk803E6660*)(o + 0x10));
}

extern "C" void fn_803E6044(char* o)
{
    Unk803E6660* s = (Unk803E6660*)(o + 0x10);
    FlushInl(s);
    int st = *(int*)(o + 0xc);
    if (st != 0) {
        BuildInl((Unk803E6660*)(o + 0x10), st);
        *(int*)(o + 0xc) = 0;
    }
    int k = *(int*)(o + 0x4c);
    if (k == 0x98 || k == 0xb0) {
        int st2 = *(int*)(o + 0xc);
        if (st2 != 0) {
            BuildInl((Unk803E6660*)(o + 0x10), st2);
            *(int*)(o + 0xc) = 0;
        }
    }
}

extern "C" void fn_803E628C(char* o, Unk803E6660_Rec* rec, int* val, Unk803E6660_Ent* ent)
{
    if (*(int*)(o + 0xc) == 0xC0) {
        Unk803E6660* s = (Unk803E6660*)(o + 0x10);
        FlushInl(s);
        int st = *(int*)(o + 0xc);
        if (st != 0) {
            BuildInl((Unk803E6660*)(o + 0x10), st);
            *(int*)(o + 0xc) = 0;
        }
        ReadyInl((Unk803E6660*)(o + 0x10));
        switch (*(int*)(o + 0x4c)) {
        case 0x98:
            fn_803E628C(o, (Unk803E6660_Rec*)(*(char**)o + 0x8E8), (int*)(*(char**)(o + 4) + 0x2F8),
                        (Unk803E6660_Ent*)(*(char**)(o + 8) + 0x5F0));
            fn_803E628C(o, (Unk803E6660_Rec*)(*(char**)o + 0x8F4), (int*)(*(char**)(o + 4) + 0x2FC),
                        (Unk803E6660_Ent*)(*(char**)(o + 8) + 0x5F8));
            break;
        case 0xB0:
            fn_803E628C(o, (Unk803E6660_Rec*)(*(char**)o + 0x8F4), (int*)(*(char**)(o + 4) + 0x2FC),
                        (Unk803E6660_Ent*)(*(char**)(o + 8) + 0x5F8));
            break;
        }
    }
    int n = *(int*)(o + 0xc);
    *(Unk803E6660_Rec*)(*(char**)o + n * 12) = *rec;
    ((int*)*(char**)(o + 4))[*(int*)(o + 0xc)] = *val;
    ((Unk803E6660_Ent*)*(char**)(o + 8))[*(int*)(o + 0xc)] = *ent;
    *(int*)(o + 0xc) = *(int*)(o + 0xc) + 1;
}

extern char lbl_8060CC1C[];

extern "C" void fn_803E6508(char* o, int arg)
{
    int st = *(int*)(o + 0xc);
    if (st != 0) {
        BuildInl((Unk803E6660*)(o + 0x10), st);
        *(int*)(o + 0xc) = 0;
    }
    FindSet((Unk803E6950*)(o + 0x10), lbl_8060CC1C, arg, 0, -1);
}

extern "C" Unk803E6660* fn_803E6660(Unk803E6660* o)
{
    SubCtor(o);
    return o;
}

extern "C" void fn_803E6708(Unk803E6660* o, int chrg)
{
    if (o->mC) {
        int* base = (int*)o->mC - 1;
        lbl_8062C66C(base, *base + 4);
    }
    if (o->m4)
        fn_803D7600(o->m4, 3);
    lbl_8062C66C(o->m38, 0x104);
    if (chrg & 1)
        lbl_8062C66C(o, 0x3C);
}

extern "C" void fn_803D707C(void* self, void* item);

extern char lbl_8060CB24[];

extern "C" int fn_803E67A0(Unk803E6660* o, Unk803E6660_Holder* h)
{
    o->m8 = h;
    int ok = 0;
    fn_803D707C(o->m4, h);
    o->m10 = 0;
    while (o->m8->m0->m4[o->m10] != 0)
        o->m10++;
    int sz = o->m10 * 12;
    int* blk = (int*)lbl_8062C668(sz + 4, (int)lbl_8060CB24);
    *blk = sz;
    Unk803E6660_Rec* a = (Unk803E6660_Rec*)(blk + 1);
    o->mC = a;
    if (a != 0) {
        for (int i = 0; i < o->m10; i++) {
            o->mC[i].m0 = 0;
            o->mC[i].m4 = 0;
            o->mC[i].m8 = -1;
        }
        ok = 1;
    }
    return ok;
}

extern "C" int fn_803E68B0(Unk803E68B0_Key* o, int k)
{
    int ok = 0;
    switch (k) {
    case -4:
    case 0x80:
    case 0x90:
    case 0xA8:
    case 0xB8:
        o->m30 = ok;
        break;
    case 0x98:
        o->m30 = 2;
        break;
    case 0xA0:
        o->m30 = -1;
        break;
    default:
        o->m30 = -1;
        break;
    }
    int t = o->m30;
    if (t != -1) {
        o->m0 = k;
        ok = 1;
        int hi = 0xFFFF - t;
        int lo = hi + t;
        o->m24 = hi;
        o->m20 = lo;
        o->m28 = lo;
    }
    return ok;
}

extern "C" int fn_803E6950(Unk803E6950* o, const char* name)
{
    int found = -1;
    for (int i = 0; i < o->m10; i++) {
        if (strcmp(name, o->m8->m0->m4[i]) == 0) {
            found = i;
            break;
        }
    }
    o->m2C = found;
    return found != -1;
}

extern "C" int fn_803E69D8(Unk803E6950* o, const char* name, int a, int b, int c)
{
    return FindSet(o, name, a, b, c);
}

extern char lbl_8060CB34[];
extern "C" int fn_803E7320(int level, const char* fmt, ...);

extern "C" int fn_803E6A70(Unk803E6950* o, int idx, int val, int flag, int extra)
{
    int r = 0;
    if (idx != -1) {
        if (val & 3)
            fn_803E7320(0, lbl_8060CB34, o->m8->m0->m4[idx], 4);
        o->mC[idx].m0 = (void*)val;
        o->mC[idx].m4 = flag;
        Unk803E6950_Tbl* h = o->m8;
        Unk803E6660_Ent* e = &h->e[idx];
        if (flag == 0) {
            e->m4 = (int)o->mC[idx].m0;
            e->m0 = 1;
        } else {
            o->mC[idx].m8 = extra;
        }
        r = 1;
    }
    return r;
}

extern "C" int OnWindowClose(Unk803E6660* o)
{
    fn_803D987C(o);
    o->m18 = 1;
    return 1;
}

extern "C" int fn_803E6B74(Unk803E6660* o)
{
    return FlushInl(o);
}

extern "C" int fn_803E6C28(Unk803E6660* o, int total)
{
    return BuildInl(o, total);
}

extern "C" int fn_803E6D00(Unk803E6660* o, int a, int b)
{
    fn_803D76F4((int)o->m4, a, b);
    return 1;
}

struct Unk803E6D28 {
    char pad0[8];
    int m8;
    Unk803E6660 sub;
};

extern "C" void fn_803E6D28(Unk803E6D28* s)
{
    int total = s->m8;
    if (total) {
        BuildInl(&s->sub, total);
        s->m8 = 0;
    }
}

extern "C" void fn_803DBDD8(void* m, int a);

struct Unk803E6E10_Blk60 {
    float w[15];
};

struct Unk803E6E10 {
    char pad0[0x48];
    int m48;
    char pad4C[0x7C - 0x4C];
    Unk803E6E10_Blk60 m7C;
};

extern "C" void fn_803E6E10(Unk803E6E10* o, const Unk803E6E10_Blk60* src)
{
    o->m7C = *src;
    fn_803DBDD8(&o->m7C, o->m48);
}

extern "C" char* fn_803E6E9C(char* o)
{
    return o + 0x7C;
}

extern "C" int fn_803E6EA4(Unk803E6660* o, int a, int b)
{
    return fn_803D76F4(o->m10, a, b);
}

struct Unk803E6EC8 {
    char pad0[0xC];
    int mC;
    Unk803E6660 sub;
};

extern "C" void fn_803E6EC8(Unk803E6EC8* s)
{
    int total = s->mC;
    if (total) {
        BuildInl(&s->sub, total);
        s->mC = 0;
    }
}

extern "C" char* fn_803E6FB0(char* o)
{
    return o + 0x90;
}
