/*
 * Game code, .text 0x802B5234-0x802B9218 (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_802ACD88.h"

typedef Unk802ACD88_Mgr Mgr;
typedef Unk802ACD88_Ent Ent;
typedef Unk802B5EF4_R Rr;

struct Unk802B7B18_L {
    char* m0;
    int m4;
};

struct Unk802B7B18_Ent {
    const char* m0;
    int m4;
    int m8;
    int mC;
};

struct Unk802B7B18_Out {
    Unk802B7B18_L* m0;
    void* m4[15];
};

struct Unk802B8388_O {
    char pad0[0xC];
    unsigned int mC;
};

extern void* (*lbl_8062C668)(int size, int heap);
typedef Unk802B5234_T T;

extern void* lbl_8061EAB4;
extern void* lbl_8061EAB8;
extern void* lbl_8061EABC;
extern void* lbl_8061EAC0;
extern void* lbl_8061EAC4;
extern void* lbl_8061EAC8;
extern void* lbl_8061EACC;
extern void* lbl_8061EAD4;
extern void* lbl_8061EAD8;
extern void* lbl_8061EADC;
extern void* lbl_8061EAE0;
extern const char lbl_805E0138[];
extern const char lbl_805E0EAC[];
extern const char lbl_805E0EBC[];
extern const char lbl_805E0ECC[];
extern const char lbl_805E0EDC[];
extern const char lbl_805E0EEC[];
extern const char lbl_805E0F04[];
extern const char lbl_805E0F1C[];
extern const char lbl_805E0F34[];
extern const char lbl_805E0F48[];

extern void* lbl_8061E978;
struct Unk802B5D78_G {
    char pad0[0x320];
    int m320;
    char pad324[4];
    int m328;
    char pad32C[0x3E4 - 0x32C];
    int m3E4;
    char pad3E8[4];
    int m3EC;
    int m3F0;
};

extern Unk802B5D78_G* lbl_8061EA68;
extern void* lbl_8061EA64;
extern int lbl_8061EA00[];
extern void* lbl_8061FF44;
extern const char lbl_805E0BC8[];
extern const char lbl_805E0BEC[];
extern const char lbl_805E0C10[];
extern const char lbl_805E0C34[];
extern const char lbl_805E0C58[];
extern const char lbl_805E0C7C[];
extern const char lbl_805E0CA0[];
extern const char lbl_805E0CC4[];
extern const char lbl_805E0CE8[];
extern const char lbl_805E0D0C[];
extern const char lbl_805E0D30[];
extern const char lbl_805E0D54[];
extern const char lbl_805E0D78[];
extern const char lbl_805E0D9C[];
extern const char lbl_805E0DC0[];
extern const char lbl_805E0DE4[];
extern const char lbl_805E0E5C[];
extern const char lbl_805E0BC0[];
extern const char lbl_805E03D0[];
extern const char lbl_805E0E44[];
extern const char lbl_805E0E50[];
extern const char lbl_805E0E78[];
extern const char lbl_805E0E88[];
extern const char lbl_805E0E94[];
extern const char lbl_805E0EA0[];

extern "C" {
char* strcpy(char* d, const char* s);
char* strcat(char* d, const char* s);
}

extern "C++" {
int fn_802B96C4(const char* name, int a);
extern "C" void* fn_802B9300(const char* name, int a, int b, int c);
void* fn_802B98AC(const char* name, int a);
extern "C" int fn_803FED48(const char* name, int a, int b, void* c);
extern "C" int fn_803FEDB8(void* p, int a);
extern "C" void MEM_free(void* p);
int fn_8040B574(const char* a, const char* b);
void fn_802AEBF0(void* a, const char* s);
void fn_802AEBCC(void* a, const char* s);
char* fn_8040B61C(char* d, const char* s);
extern "C" void fn_803188FC(void* a, int b);
extern "C" void fn_803DD73C(void* a, int b);
int fn_802B86D4(Mgr* m, const char* s, int* idx);
void fn_802B8760(Mgr* m, const char* s);
void fn_802B881C(Mgr* m, const char* s);
extern "C" int fn_800968C0(void* o, int x, int y);
extern "C" int fn_8033D1E4(char* buf, const char* name, int n);
extern "C" void* fn_8018466C(char* buf, void* a);
extern "C" void* fn_8014EB44(void* x);
extern "C" void fn_8014EBEC(void* x, char* buf);
struct Unk802B8388_O;
extern "C" Unk802B8388_O* SHAPE_create(int a, int b, int c, int d, int e, int f, int g, int h);
extern "C" void* fn_803DD628(void* a, void* b);
void fn_802B832C(Mgr* m);
void* fn_8037C7FC();
int fn_8040B3B4(const char* a, const char* b);
extern "C" void* fn_803D77EC(void* e);
extern "C" void fn_8037C9F4(void* o, char* s);
void fn_802AD538(void* o, int a);
int fn_80089AA0(void* o, int a, int b);
extern "C" int fn_8008CFD8(void* o, int a, int b);
int fn_804089B4(char* buf, const char* fmt, ...);
void fn_802C9284(void* p, int a, void* o, int b, int c);
extern "C" int fn_803E96B8(void* a, int b, int c, int d, int e, void (*f)());
extern "C" int fn_80313C14(int r, int a, int b);
extern "C" void fn_803E97C4(int a);
extern "C" void fn_80312700();
extern "C" char* fn_803EC814(void* a, int b, char* c);
void fn_802B2F64(void* m, int f, void* slot, int a, int b);
extern "C" void fn_803DDA8C(void* a, void* b);
int fn_803D81FC();
int fn_803D817C();
extern "C" int fn_803EC03C(void* im);
extern "C" int SHAPE_type(int f);
extern "C" void fn_803D9E70(void* o, int a, int b, int c, int d, int e, int f, int g, int h);
extern "C" void fn_803D9ECC(void* o, void* img, int a, int b, int c);

extern "C" void* SHAPE_hotspots(void* a);
int fn_8040B750(const char* a, const char* b, int n);
}

struct Unk802B5234_Self {
    char pad0[0x190 - 0x0];
    int m190;
    char pad194[0x1A0 - 0x194];
    int m1A0;
    char pad1A4[0x228 - 0x1A4];
    int m228;
    char pad22C[0x238 - 0x22C];
    int m238;
    char pad23C[0x248 - 0x23C];
    int m248;
    char pad24C[0x258 - 0x24C];
    int m258;
    char pad25C[0x268 - 0x25C];
    int m268;
    char pad26C[0xAB8 - 0x26C];
    int mAB8;
    char padABC[0xAC8 - 0xABC];
    int mAC8;
    char padACC[0xAD8 - 0xACC];
    int mAD8;
    char padADC[0xAE8 - 0xADC];
    int mAE8;
};

void fn_802B5234(Unk802B5234_Self* self)
{
    void* mgr = lbl_8061EA68;
    if (self->m238 != 0)
        fn_802B2F64(mgr, self->m238, (char*)self + 0x3704, 1, 0);
    if (self->m190 != 0)
        fn_802B2F64(mgr, self->m190, (char*)self + 0x3714, 1, 0);
    if (self->m228 != 0)
        fn_802B2F64(mgr, self->m228, (char*)self + 0x3724, 1, 0);
    if (self->m1A0 != 0)
        fn_802B2F64(mgr, self->m1A0, (char*)self + 0x3734, 1, 0);
    if (self->m258 != 0)
        fn_802B2F64(mgr, self->m258, (char*)self + 0x3744, 1, 0);
    if (self->m268 != 0)
        fn_802B2F64(mgr, self->m268, (char*)self + 0x3754, 1, 0);
    if (self->mAB8 != 0)
        fn_802B2F64(mgr, self->mAB8, (char*)self + 0x3764, 1, 0);
    if (self->mAC8 != 0)
        fn_802B2F64(mgr, self->mAC8, (char*)self + 0x3774, 1, 0);
    if (self->mAD8 != 0)
        fn_802B2F64(mgr, self->mAD8, (char*)self + 0x3784, 1, 0);
    if (self->m248 != 0)
        fn_802B2F64(mgr, self->m248, (char*)self + 0x3794, 1, 0);
    if (self->mAE8 != 0)
        fn_802B2F64(mgr, self->mAE8, (char*)self + 0x37A4, 1, 0);
}

Unk802B5234_T* fn_802B53C4(Unk802B5234_T* self)
{
    self->m[0] = 0;
    self->m[1] = 0;
    self->m[2] = 0;
    self->m[3] = 0;
    self->m[4] = 0;
    self->m[5] = 0;
    self->m[6] = 0;
    self->m[7] = 0;
    self->m[8] = 0;
    self->m[9] = 0;
    self->m[10] = 0;
    self->m[11] = 0;
    self->m[12] = 0;
    self->m[13] = 0;
    self->m[14] = 0;
    self->m[15] = 0;
    self->m[16] = 0;
    self->m[17] = 0;
    self->m[18] = 0;
    self->m[19] = 0;
    self->m[20] = 0;
    self->m[21] = 0;
    self->m[22] = 0;
    self->m[23] = 0;
    self->m[24] = 0;
    self->m[25] = 0;
    self->m[26] = 0;
    self->m[27] = 0;
    self->m[28] = 0;
    self->m[29] = 0;
    self->m[30] = 0;
    self->m[31] = 0;
    self->m[32] = 0;
    self->m[33] = 0;
    self->m[34] = 0;
    self->m[35] = 0;
    self->m[36] = 0;
    self->m[37] = 0;
    self->m[38] = 0;
    self->m[39] = 0;
    self->m[40] = 0;
    self->m[41] = 0;
    self->m[42] = 0;
    self->m[43] = 0;
    return self;
}

Unk802B5234_V3 fn_802B5480()
{
    Unk802B5234_V3 a;
    Unk802B5234_V3 b;
    char pad[16];
    a.x = 2702.9f;
    a.y = 11078.4f;
    a.z = -13515.9f;
    b.x = -4908.36f;
    b.y = 19463.4f;
    b.z = -6725.12f;
    return b;
}

struct Unk802B560C_Slot {
    int m0;
    int m4;
    int m8;
};

struct Unk802B560C_Self {
    char pad0[0x3650];
    int m3650;
};

struct Unk802B54F4_Hdr {
    char pad0[8];
    int m8;
    char padC[8];
    int m14;
};

struct Unk802B54F4_Tex {
    char pad0[4];
    short m4;
    short m6;
};

int fn_802B54F4(Unk802B54F4_Hdr* self)
{
    int cnt = self->m8;
    int total = 0;
    int extra = 0;
    int i = 0;
    char tag[5] = "stcs";
    char name[32];
    if (total < cnt) {
        int* e = &self->m14;
        do {
            Unk802B54F4_Tex* t = (Unk802B54F4_Tex*)((char*)self + *e);
            e += 2;
            if (t != 0) {
                fn_803EC814(self, i, name);
                if (fn_8040B750(name, tag, 4) == 0) {
                    int* q = (int*)SHAPE_hotspots(t);
                    int n = q[-1];
                    int j;
                    extra = n / 2;
                    for (j = 1; j < n; j += 2) {
                        int v = q[j * 2] * q[j * 2 + 1] + 0x400;
                        total += v;
                    }
                } else {
                    int v = t->m4 * t->m6 + 0x400;
                    total += v;
                }
            }
            i++;
        } while (i < cnt);
    }
    int ex = extra - 1;
    return total + (cnt + ex) * 0x187;
}

void fn_802B560C(Unk802B560C_Self* self, Unk802B560C_Slot* slot, int* out, int* tbl, int idx, int a, int b, int* c, int flag)
{
    void* h = lbl_8062C668(0x30, 0);
    int r = fn_803E96B8(h, slot->m0, tbl[idx], a, 0, fn_80312700);
    slot->m8 = 14;
    slot->m4 = r;
    *out = fn_80313C14(r, c ? *c : self->m3650, b);
    if (flag)
        fn_803E97C4(slot->m4);
}

void fn_802B56C8()
{
    lbl_8061EA64 = 0;
    for (int i = 0; i < 25; i++)
        lbl_8061EA00[i] = 0;
}

int fn_802B56FC(const char* name)
{
    if (fn_802B96C4(name, 0) == 0) {
        char buf[0x104] = "";
        fn_8040B61C(buf, name);
        if (fn_8033D1E4(buf, name, 0x104) == 0)
            return 0;
        while (fn_802B96C4(name, 0) == 0) {
        }
    }
    return 1;
}

void fn_802B5794(void* a, char* out)
{
    char buf[0x104] = "";
    fn_8014EBEC(fn_8014EB44(fn_8018466C(buf, a)), buf);
    strcpy(out, "stadium\\");
    strcat(out, buf);
    strcat(out, lbl_805E03D0);
}

extern "C" {
unsigned strlen(const char* s);
int fn_80329C4C(const char** pp, unsigned* pn, int* out, int lo, int hi, int delim);
int fn_80329B98(const char** pp, unsigned* pn, float* out, int delim);
void fn_803089A8(void* p, int n);
void fn_80308960__FP15Unk80308874_Mgriiii(void* m, int i, int cnt, float* a, float* b);
void fn_80308AD0__FP15Unk80308874_Mgriiiif(void* m, float* a, float* b, int c, int i, float f);
void fn_80308A7C__FP17Unk80308874_Slotsii(void* s, int i, int m);
void fn_80308B04__FP17Unk80308874_Slotsii(void* s, int k, int i);
}
struct Unk802B5834_V4 {
    float m0;
    float m4;
    float m8;
    float mC;
};
extern const Unk802B5834_V4 lbl_806055E8;

void fn_802B5834(Mgr* owner, const char* text)
{
    int i;
    unsigned len = strlen(text);
    int n = 0;
    const char* cur = text;
    fn_80329C4C(&cur, &len, &n, 0, 100000, 10);
    fn_803089A8((char*)owner + 0x74, n);
    for (i = 0; i < n; i++) {
        int cnt = 0;
        fn_80329C4C(&cur, &len, &cnt, 0, 100000, 0x2C);
        float a[3];
        float b[3];
        fn_80329B98(&cur, &len, &a[0], 0x2C);
        fn_80329B98(&cur, &len, &a[1], 0x2C);
        fn_80329B98(&cur, &len, &a[2], 0x2C);
        fn_80329B98(&cur, &len, &b[0], 0x2C);
        fn_80329B98(&cur, &len, &b[1], 0x2C);
        fn_80329B98(&cur, &len, &b[2], 10);
        b[1] += 40.0f;
        fn_80308960__FP15Unk80308874_Mgriiii((char*)owner + 0x74, i, cnt, a, b);
        for (int j = 0; j < cnt; j++) {
            float row[16];
            fn_80329B98(&cur, &len, &row[0], 0x2C);
            fn_80329B98(&cur, &len, &row[1], 0x2C);
            fn_80329B98(&cur, &len, &row[2], 0x2C);
            fn_80329B98(&cur, &len, &row[3], 0x2C);
            fn_80329B98(&cur, &len, &row[4], 0x2C);
            fn_80329B98(&cur, &len, &row[5], 0x2C);
            fn_80329B98(&cur, &len, &row[6], 0x2C);
            fn_80329B98(&cur, &len, &row[7], 0x2C);
            fn_80329B98(&cur, &len, &row[8], 0x2C);
            fn_80329B98(&cur, &len, &row[9], 0x2C);
            fn_80329B98(&cur, &len, &row[10], 0x2C);
            fn_80329B98(&cur, &len, &row[11], 0x2C);
            fn_80329B98(&cur, &len, &row[12], 0x2C);
            fn_80329B98(&cur, &len, &row[13], 0x2C);
            fn_80329B98(&cur, &len, &row[14], 0x2C);
            fn_80329B98(&cur, &len, &row[15], 0x2C);
            Unk802B5834_V4 c = lbl_806055E8;
            const float scale = 0.003921569f;
            fn_80329B98(&cur, &len, &((float*)&c)[2], 0x2C);
            fn_80329B98(&cur, &len, &((float*)&c)[1], 0x2C);
            fn_80329B98(&cur, &len, &((float*)&c)[0], 0x2C);
            c.m0 *= scale;
            c.m4 *= scale;
            c.m8 *= scale;
            c.mC = 1.0f;
            int mode = 0;
            fn_80329C4C(&cur, &len, &mode, 1, 0x10, 0x2C);
            float v = 0.0f;
            fn_80329B98(&cur, &len, &v, 10);
            fn_80308AD0__FP15Unk80308874_Mgriiiif((char*)owner + 0x74, row, (float*)&c, mode, i, v);
        }
    }
    int n2 = 0;
    fn_80329C4C(&cur, &len, &n2, 0, 100000, 10);
    for (i = 0; i < n2; i++) {
        int m = 0;
        fn_80329C4C(&cur, &len, &m, 0, 100000, 10);
        fn_80308A7C__FP17Unk80308874_Slotsii((char*)owner + 0x74, i, m);
        int k = 0;
        for (int j = 0; j < m - 1; j++) {
            fn_80329C4C(&cur, &len, &k, 0, 100000, 0x2C);
            fn_80308B04__FP17Unk80308874_Slotsii((char*)owner + 0x74, k, i);
        }
        if (m > 0) {
            fn_80329C4C(&cur, &len, &k, 0, 100000, 10);
            fn_80308B04__FP17Unk80308874_Slotsii((char*)owner + 0x74, k, i);
        }
    }
}

/* near-miss: `li r0,0` is scheduled before the second lfs/fcmpu here, after the fcmpu in retail */
void fn_802B5D28(int* out, float x)
{
    int r;
    if (!(x >= 0.33333334f)) {
        r = 2;
    } else {
        if (!(x >= 0.8f))
            r = 1;
        else
            r = 0;
    }
    *out = r;
}

int fn_802B5D78(Mgr*, int kind)
{
    Unk802B5D78_G* g = lbl_8061EA68;
    if (kind == 0x18 || kind == 0x19)
        return g->m328 == 0;
    if (kind == 0x3E || kind == 0x3F) {
        if (g->m328 != 0)
            return 1;
        return 0;
    }
    int a = g->m320;
    if (kind == 0x35 || kind == 0x36 || kind == 0x3B) {
        if (g->m328 == 0)
            return 0;
        return a == 0;
    }
    if (kind == 0x37 || kind == 0x38 || kind == 0x3C) {
        if (g->m328 == 0)
            return 0;
        return (a ^ 1) == 0;
    }
    if (kind == 0x39 || kind == 0x3A || kind == 0x3D) {
        if (g->m328 == 0)
            return 0;
        return (a ^ 2) == 0;
    }
    a = g->m3EC;
    if (kind == 0x40) {
        if (g->m328 == 0)
            return 0;
        if (g->m3E4 != 0)
            return 0;
        return (a ^ 2) == 0;
    }
    if (kind == 0x41) {
        if (g->m328 == 0)
            return 0;
        if (g->m3E4 != 0)
            return 0;
        return a == 0;
    }
    if (kind == 0x42) {
        if (g->m328 == 0)
            return 0;
        if (g->m3E4 != 0)
            return 0;
        return (a ^ 1) == 0;
    }
    a = g->m3F0;
    if (kind == 0x43) {
        if (g->m328 == 0)
            return 0;
        return a == 0;
    }
    if (kind == 0x44) {
        if (g->m328 == 0)
            return 0;
        return (a ^ 1) == 0;
    }
    if (kind == 0x45) {
        if (g->m328 == 0)
            return 0;
        return (a ^ 2) == 0;
    }
    return 1;
}

void fn_802B5EF4(Rr* self)
{
    self->m0 = 0.0f;
    self->m4 = 0.4f;
    self->m34 = 0;
    self->m38 = 0;
    self->mC = 0.0f;
    self->m8 = -1.0f;
}

void fn_802B5F2C()
{
    fn_803188FC(lbl_8061FF44, 5);
}

void fn_802B5F58(Rr* self)
{
    if (self->m34 != 0) {
        fn_803DD73C(self->m34, 3);
        self->m34 = 0;
    }
}

struct Unk8019F8C0_Owner;
struct Unk802B5F9C_Obj {
    float m0;
    float m4;
    float m8;
    float mC;
    float m10;
    float m14;
    float m18;
    float m1C;
    float m20;
    float m24;
    float m28;
    float m2C;
    Unk8019F8C0_Owner* m30;
    int m34;
};
struct Unk802B5F9C_Col {
    float r;
    float g;
    float b;
    float a;
};
struct Unk802B5F9C_G {
    char pad0[0x2C];
    int m2C;
};
struct Unk802B5F9C_H {
    char pad0[0x3C];
    void* m3C;
};
extern Unk802B5F9C_H* lbl_8061FEDC;
extern int lbl_8061EAB0;
extern "C" {
int rand();
void fn_8014D9C4(void* p);
float sinf(float x);
void fn_803185A0(void* a, int b, float* c, void* d, int e, Unk802B5F9C_Col f, int g, int h);
}
void* fn_801A04A8(Unk8019F8C0_Owner* o, int i);
float fn_801A0744(Unk8019F8C0_Owner* o);
inline void Unk802B5F9C_Send(Unk802B5F9C_H* a, int b, float* c, void* d, int e, Unk802B5F9C_Col f, int g, int h)
{
    fn_803185A0(a->m3C, b, c, d, e, f, g, h);
}
inline float Unk802B5F9C_Cl(float v)
{
    return !(v > 1.0f) ? (v >? 0.0f) : 1.0f;
}


void fn_802B5F9C(Unk802B5F9C_Obj* self)
{
    float arr[8];
    arr[0] = 0.0f;
    arr[1] = 0.0f;
    arr[3] = 0.0f;
    arr[4] = 0.0f;
    arr[2] = 1.0f;
    arr[6] = 1.0f;
    arr[7] = 1.0f;
    arr[5] = 1.0f;
    float rnd = (float)(rand() % 10000) * 2.0f * 0.0001f;
    Unk802B5F9C_Col col;
    fn_8014D9C4(&col);
    if (fn_801A04A8(self->m30, 1) != 0) {
        float cur = fn_801A0744(self->m30);
        self->mC = !(self->m8 < 0.0f) ? cur - self->m8 : 0.0f;
        self->m8 = cur;
        float a = sinf(self->m0) * 0.07f + 0.7f;
        col.r = Unk802B5F9C_Cl(1.0f);
        col.g = Unk802B5F9C_Cl(1.0f);
        col.b = Unk802B5F9C_Cl(1.0f);
        col.a = Unk802B5F9C_Cl(a * 0.5f);
        if (!(self->m0 <= 6.284f)) {
            self->m0 = 0.0f;
            self->m4 = (float)(rand() % 10000) * 0.00004f + 0.4f;
        }
        if (self->mC != 0.0f) {
            if (lbl_8061EAB0-- < 0) {
                self->m10 = rnd;
                self->m14 = rnd;
                self->m18 = rnd + 1.0f;
                self->m1C = rnd;
                self->m20 = rnd;
                self->m28 = rnd + 1.0f;
                self->m2C = rnd + 1.0f;
                self->m24 = rnd + 1.0f;
                lbl_8061EAB0 = 2;
            }
            self->m0 = self->m4 * 60.024014f * self->mC + self->m0;
        }
        Unk802B5F9C_H* h = lbl_8061FEDC;
        fn_803188FC(lbl_8061FF44, 5);
        ((Unk802B5F9C_G*)lbl_8061FF44)->m2C = 5;
        Unk802B5F9C_Send(h, self->m34, arr, &self->m10, 0xFF, col, 1, 1);
    }
}

void fn_802B7A28(void* unused, int idx)
{
    char a[0x100] = "";
    char b[0x20] = "";
    fn_8018466C(b, (void*)idx);
    strcpy(a, (char*)lbl_8061E978 + 0x37F8 + idx * 9);
    strcat(a, lbl_805E0BC0);
    fn_8037C9F4(fn_8037C7FC(), a);
    a[0] = 0;
    fn_8018466C(b, (void*)idx);
    strcat(a, b);
    strcat(a, lbl_805E0BC0);
    fn_8037C9F4(fn_8037C7FC(), a);
    fn_802AD538(lbl_8061E978, 0);
}

/* near-miss (parked, 77 vs 78 insns): retail loads the list head/end before copying the 16-pointer name table and walks the output with a pointer register */
void fn_802B7B18(void* unused, Unk802B7B18_Out* out)
{
    for (int i = 0; i < 15; i++)
        out->m4[i] = 0;
    const char* names[16] = {
        lbl_805E0BC8,
        lbl_805E0BEC,
        lbl_805E0C10,
        lbl_805E0C34,
        lbl_805E0C58,
        lbl_805E0C7C,
        lbl_805E0CA0,
        lbl_805E0CC4,
        lbl_805E0CE8,
        lbl_805E0D0C,
        lbl_805E0D30,
        lbl_805E0D54,
        lbl_805E0D78,
        lbl_805E0D9C,
        lbl_805E0DC0,
        lbl_805E0DE4
    };
    Unk802B7B18_L* l = out->m0;
    Unk802B7B18_Ent* it = (Unk802B7B18_Ent*)l->m0;
    Unk802B7B18_Ent* end = it + l->m4;
    while (it != end) {
        for (int j = 0; j < 15; j++) {
            if (fn_8040B3B4(it->m0, names[j]) == 0) {
                void* p = fn_803D77EC(it);
                if (p != 0)
                    out->m4[j] = (void*)it->mC;
                else
                    out->m4[j] = p;
            }
        }
        it++;
    }
}

void fn_802B832C(Mgr* m)
{
    if (*(void**)((char*)m + 0x18718) != 0) {
        fn_803DD73C(*(void**)((char*)m + 0x18718), 3);
        *(void**)((char*)m + 0x18718) = 0;
    }
    if (*(void**)((char*)m + 0x1871C) != 0) {
        MEM_free(*(void**)((char*)m + 0x1871C));
        *(void**)((char*)m + 0x1871C) = 0;
    }
}

void fn_802B8388(Mgr* m)
{
    char* b = (char*)m + 0x20000;
    fn_802B832C(m);
    Unk802B8388_O* o = SHAPE_create(0x100, 0x100, 0x20, 0, 0, 0x100, 0, 0);
    *(Unk802B8388_O**)(b - 0x78E4) = o;
    if (o != 0) {
        o->mC |= 0x20000000;
        void* h = lbl_8062C668(0x58, 0);
        *(void**)(b - 0x78E8) = fn_803DD628(h, *(void**)(b - 0x78E4));
    }
}

struct Unk802B8418_ObjData {
    char pad0[0xC];
};

class Unk802B8418_Obj : public Unk802B8418_ObjData {
public:
    virtual void s1();
    virtual void s2();
    virtual void s3(float* a, float* b);
};

void fn_802B8418(Mgr* m)
{
    char* b = (char*)m + 0x20000;
    if (*(void**)(b - 0x78E8) != 0) {
        char* g = (char*)lbl_8061E978 + 8;
        if (*(int*)(g + 0xAEC) != 0) {
            fn_803DDA8C(*(void**)(b - 0x78E8), *(void**)(g + 0xB18));
            return;
        }
        fn_803DDA8C(*(void**)(b - 0x78E8), *(void**)(b - 0x78E4));
        fn_803D81FC();
        Unk802B8418_Obj* o = (Unk802B8418_Obj*)fn_803D817C();
        void* sub = (char*)o + 0x10;
        short* im = *(short**)(b - 0x78E4);
        int w = im[2];
        int h = im[3];
        int fmt = SHAPE_type(fn_803EC03C(im)) & 0xF;
        float x;
        float y;
        o->s3(&x, &y);
        int a = (int)(x * 0.5f) - w;
        fn_803D9E70(sub, a, 0, (int)x - a * 2, (int)y, w, h, fmt, 1);
        fn_803D9ECC(sub, *(void**)(b - 0x78E8), 0, 1, 0);
    }
}

void fn_802B8548(Mgr* m)
{
    fn_803FED48(lbl_805E0E44, 0, 0, (char*)m + 0x18704);
    fn_803FED48(lbl_805E0E50, 0, 0, (char*)m + 0x18708);
}

void fn_802B85A0(Mgr* m)
{
    fn_803FED48(lbl_805E0E5C, 0, 0, (char*)m + 0x18710);
}

void fn_802B85D8(Mgr* m)
{
    fn_803FED48(lbl_805E0E78, 0, 0, (char*)m + 0x1870C);
}

void fn_802B8610(Mgr* m)
{
    fn_803FEDB8(*(void**)((char*)m + 0x18704), 0);
    *(void**)((char*)m + 0x18704) = 0;
    fn_803FEDB8(*(void**)((char*)m + 0x18708), 0);
    *(void**)((char*)m + 0x18708) = 0;
}

void fn_802B865C(Mgr* m)
{
    fn_803FEDB8(*(void**)((char*)m + 0x18710), 0);
    *(void**)((char*)m + 0x18710) = 0;
}

void fn_802B8698(Mgr* m)
{
    fn_803FEDB8(*(void**)((char*)m + 0x1870C), 0);
    *(void**)((char*)m + 0x1870C) = 0;
}

int fn_802B86D4(Mgr* m, const char* s, int* idx)
{
    int i;
    for (i = 0; i <= 31; i++) {
        if (m->ent[i].cnt != 0 && fn_8040B574(m->ent[i].name, s) == 0) {
            *idx = i;
            return 1;
        }
    }
    *idx = 0;
    return 0;
}

void fn_802B8760(Mgr* m, const char* s)
{
    int idx;
    if (fn_802B86D4(m, s, &idx) != 0) {
        m->ent[idx].cnt++;
        return;
    }
    for (idx = 0; idx <= 31; idx++) {
        if (m->ent[idx].cnt == 0) {
            m->ent[idx].cnt = 1;
            fn_8040B61C(m->ent[idx].name, s);
            fn_802AEBCC(lbl_8061E978, s);
            return;
        }
    }
}

void fn_802B881C(Mgr* m, const char* s)
{
    int idx;
    if (fn_802B86D4(m, s, &idx) != 0) {
        if (--m->ent[idx].cnt == 0)
            fn_802AEBF0(lbl_8061E978, s);
    }
}

void fn_802B8888(Mgr* m, void* o, int x)
{
    char buf[32];
    int t = fn_800968C0(o, x, 1);
    int a = fn_8008CFD8((char*)o + 0xC9C, 0x21, t);
    fn_804089B4(buf, lbl_805E0E88, a);
    fn_802B8760(m, buf);
    fn_804089B4(buf, lbl_805E0E94, 0);
    fn_802B8760(m, buf);
}

void fn_802B891C(Mgr* m, void* o, int x)
{
    char buf[32];
    int t = fn_800968C0(o, x, 1);
    int a = fn_80089AA0((char*)o + 0xC74, 0x1A, t);
    fn_804089B4(buf, lbl_805E0EA0, a);
    fn_802B8760(m, buf);
}

void fn_802B898C(Mgr* m, void* o, int x)
{
    char buf[32];
    int t = fn_800968C0(o, x, 1);
    int a = fn_8008CFD8((char*)o + 0xC9C, 0x21, t);
    fn_804089B4(buf, lbl_805E0E88, a);
    fn_802B881C(m, buf);
    fn_804089B4(buf, lbl_805E0E94, 0);
    fn_802B881C(m, buf);
}

void fn_802B8A20(Mgr* m, void* o, int x)
{
    char buf[32];
    int t = fn_800968C0(o, x, 1);
    int a = fn_80089AA0((char*)o + 0xC74, 0x1A, t);
    fn_804089B4(buf, lbl_805E0EA0, a);
    fn_802B881C(m, buf);
}

void fn_802B8A90(Mgr* m)
{
    for (int i = 0; i < 32; i++) {
        if (m->ent[i].cnt > 0) {
            m->ent[i].cnt = 0;
            fn_802AEBF0(lbl_8061E978, m->ent[i].name);
        }
    }
}

void fn_802B8AFC(Mgr* m, void* o)
{
    fn_802C9284((char*)m + 0x12204, *(int*)((char*)o + 0x2C), o, 0, 1);
}

void fn_802B8B34()
{
    if (fn_802B96C4(lbl_805E0EAC, 0) != 0) {
        void* h = 0;
        fn_803FED48(lbl_805E0EAC, 0x100, 0, &h);
        lbl_8061EAB4 = fn_802B98AC(lbl_805E0EBC, 0x400);
        fn_803FEDB8(h, 0);
    } else {
        lbl_8061EAB4 = 0;
    }
}

void fn_802B8BBC()
{
    if (lbl_8061EAB4 != 0) {
        MEM_free(lbl_8061EAB4);
        lbl_8061EAB4 = 0;
    }
}

void fn_802B8BFC()
{
    if (fn_802B96C4(lbl_805E0ECC, 0) != 0) {
        void* h = 0;
        fn_803FED48(lbl_805E0ECC, 0x100, 0, &h);
        lbl_8061EAB8 = fn_802B98AC(lbl_805E0EDC, 0x500);
        fn_803FEDB8(h, 0);
    } else {
        lbl_8061EAB8 = 0;
    }
}

void fn_802B8C84()
{
    if (lbl_8061EAB8 != 0) {
        MEM_free(lbl_8061EAB8);
        lbl_8061EAB8 = 0;
    }
}

void fn_802B8CC4()
{
}

void fn_802B8CC8()
{
}

void fn_802B8CCC()
{
    if (fn_802B96C4(lbl_805E0EEC, 0) == 1)
        lbl_8061EABC = fn_802B9300(lbl_805E0EEC, 0x500, 14, 0);
    else
        lbl_8061EABC = 0;
}

void fn_802B8D34()
{
    if (lbl_8061EABC != 0) {
        MEM_free(lbl_8061EABC);
        lbl_8061EABC = 0;
    }
}

void fn_802B8D74()
{
    if (fn_802B96C4(lbl_805E0F04, 0) == 1)
        lbl_8061EAC0 = fn_802B9300(lbl_805E0F04, 0x500, 14, 0);
    else
        lbl_8061EAC0 = 0;
}

void fn_802B8DDC()
{
    if (lbl_8061EAC0 != 0) {
        MEM_free(lbl_8061EAC0);
        lbl_8061EAC0 = 0;
    }
}

void fn_802B8E1C()
{
    if (fn_802B96C4(lbl_805E0F1C, 0) == 1)
        lbl_8061EAC4 = fn_802B9300(lbl_805E0F1C, 0x500, 14, 0);
    else
        lbl_8061EAC4 = 0;
}

void fn_802B8E84()
{
    if (lbl_8061EAC4 != 0) {
        MEM_free(lbl_8061EAC4);
        lbl_8061EAC4 = 0;
    }
}

void fn_802B8EC4()
{
    if (fn_802B96C4(lbl_805E0F34, 0) == 1)
        lbl_8061EAC8 = fn_802B9300(lbl_805E0F34, 0x400, 14, 0);
    else
        lbl_8061EAC8 = 0;
}

void fn_802B8F2C()
{
    if (lbl_8061EAC8 != 0) {
        MEM_free(lbl_8061EAC8);
        lbl_8061EAC8 = 0;
    }
}

void fn_802B8F6C(const char* name)
{
    void* r = (void*)fn_802B96C4(name, 0);
    if (r != 0)
        r = fn_802B9300(name, 0x500, 14, 0);
    lbl_8061EACC = r;
}

void fn_802B8FC0()
{
    if (lbl_8061EACC != 0) {
        MEM_free(lbl_8061EACC);
        lbl_8061EACC = 0;
    }
}

void fn_802B9000()
{
    void* r = (void*)fn_802B96C4(lbl_805E0F48, 0);
    if (r != 0)
        r = fn_802B9300(lbl_805E0F48, 0x500, 14, 0);
    lbl_8061EAD4 = r;
}

void fn_802B9058()
{
    if (lbl_8061EAD4 != 0) {
        delete lbl_8061EAD4;
        lbl_8061EAD4 = 0;
    }
}

void fn_802B9098(const char* name)
{
    void* r = (void*)fn_802B96C4(name, 0);
    if (r != 0)
        r = fn_802B9300(name, 0x500, 14, 0);
    lbl_8061EAD8 = r;
}

void fn_802B90EC()
{
    if (lbl_8061EAD8 != 0) {
        MEM_free(lbl_8061EAD8);
        lbl_8061EAD8 = 0;
    }
}

void fn_802B912C(const char* name)
{
    void* r = (void*)fn_802B96C4(name, 0);
    if (r != 0)
        r = fn_802B9300(name, 0x500, 14, 0);
    lbl_8061EADC = r;
}

void fn_802B9180()
{
    if (lbl_8061EADC != 0) {
        MEM_free(lbl_8061EADC);
        lbl_8061EADC = 0;
    }
}

void fn_802B91C0()
{
    void* r = (void*)fn_802B96C4(lbl_805E0138, 0);
    if (r != 0)
        r = fn_802B9300(lbl_805E0138, 0x100, 14, 0);
    lbl_8061EAE0 = r;
}
