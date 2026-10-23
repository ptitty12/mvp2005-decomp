/*
 * Game code, .text 0x80308874-0x8030C5D8 (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_802F8324.h"

extern char lbl_80665950[];
extern char lbl_80665A88[];
extern char lbl_80665470[];
extern char lbl_80665500[];
extern char lbl_80665D88[];
extern char lbl_806653F8[];
extern int lbl_806E0FA4[];

inline void SetD(char* p, int i, int v)
{
    int off = i * 4;
    *(int*)(p + off) = v;
}

struct Unk80308874_Buf;
int fn_80308874(Unk80308874_Buf* p);
struct Unk80308874_Ent50 {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    int m18;
    int m1C;
    int m20;
    char pad24[0x50 - 0x24];
    Unk80308874_Ent50()
    {
        m0 = 0;
        m4 = 0;
        m8 = 0;
        mC = 0;
        m10 = 0;
        m14 = 0;
        m18 = 0;
    }
    ~Unk80308874_Ent50() { fn_80308874((Unk80308874_Buf*)this); }
};
struct Unk80308874_Mgr {
    char pad0[0x100];
    Unk80308874_Ent50* m100;
    int m104;
    int m108;
    int m10C;
};
struct Unk80308874_Slot3 {
    int* p;
    int n;
    int cap;
};
struct Unk80308874_Slots {
    Unk80308874_Slot3 s[1];
};
struct Unk80308874_At20C {
    char pad0[0x20C];
    char* m20C;
};
struct Unk80308874_At24 {
    char pad0[0x24];
    int m24;
};
struct Unk80308874_AtD0 {
    char pad0[0xD0];
    const void* mD0;
};
struct Unk80308874_At18 {
    char pad0[0x18];
    const void* m18;
    char pad1C[8];
    int m24;
};
struct Unk80308874_At4 {
    int m0;
    int m4;
};
struct Unk80308874_P8 {
    int a;
    int b;
};
struct Unk80308874_Pair {
    char pad0[0x28];
    int m28;
    int m2C;
};
struct Unk80308874_FSlot {
    float* m0;
    int m4;
    float m8;
};
struct Unk80308874_FSlots {
    char pad0[0x28];
    float m28;
    char pad2C[0x4C - 0x2C];
    Unk80308874_FSlot s[1];
    Unk80308874_FSlot m58;
};
struct Unk80308874_Hdr {
    short m0;
    short m2;
};
struct Unk80308874_List {
    int m0;
    int m4;
    Unk80308874_Hdr* m8[0x50];
};
struct Unk80308874_B8 {
    char pad0[0x98];
    const void* m98;
    char pad9C[0xB8 - 0x9C];
    const void* mB8;
};
struct Unk80308874_At8 {
    int m0;
    int m4;
    const void* m8;
};
struct Unk80308874_At10 {
    char pad0[0x10];
    int m10;
};

struct Unk8030BDE8_Q {
    float m0;
    float m4;
    float m8;
    float mC;
};
struct Unk8030BDE8 {
    Unk8030BDE8_Q q[3];
    int m30;
    float m34;
    float m38;
    float m3C;
};
struct Unk8030BEB8_V3 {
    float x;
    float y;
    float z;
};
struct Unk8030BEB8 {
    const void* m0;
    float m4;
    float m8;
    int mC;
    int m10;
    int m14;
    int m18;
    int m1C;
    Unk8030BEB8_V3 m20[6];
    float m68;
    float m6C;
    float m70;
    float m74;
    float m78;
    float m7C;
    float m80;
    float m84;
    float m88;
    float m8C;
    float m90;
    float m94;
    Unk8030BEB8_V3 m98;
    int mA4;
};
extern const Unk8030BEB8_V3 lbl_806055AC;

struct Unk802ACD88_Mgr;
void* fn_802AFA54(Unk802ACD88_Mgr* m, const char* s);
extern Unk802ACD88_Mgr* lbl_8061E978;
extern const char* lbl_8061FBF0;
extern void* (*lbl_8062C668)(int size, int heap);
inline int fn_8030BFF8_Has(Unk802ACD88_Mgr* m)
{
    return *(int*)((char*)m + 0x3658) != 0;
}

struct Unk8030B63C_Obj {
    char pad0[0x50];
    int m50;
    char pad54[0x90 - 0x54];
    void* m90;
    char m94[4];
};
extern const char lbl_805E8A0C[];
extern void* lbl_8061FEDC;

struct Unk80308D30_Obj {
    char m0[0x14];
    int m14;
    const void* m18;
    char pad1C[8];
    int m24;
    float m28;
    float m2C;
    float m30;
    float m34;
    float m38;
    int m3C;
    int m40;
    float m44;
    float m48;
};
extern const char lbl_805E88B0[];
extern int lbl_8061FBAC[];
extern int lbl_8061FB98[];
extern float lbl_80691410[];
struct Unk8019F8C0_Owner;
float fn_801A0744(Unk8019F8C0_Owner* o);
extern int lbl_8061B928;
extern int lbl_8061FBD8;
struct Unk8030B08C_Obj {
    char padx[0xBC];
    void* mBC;
    char padC0[0x244 - 0xC0];
    char m244[4];
};

struct Unk8030B358_Obj {
    void* m0;
    void* m4;
    void* m8;
    char padC[0x50 - 0xC];
    int m50;
    char m54[0x94 - 0x54];
    char m94[0x8D4 - 0x94];
    const void* m8D4;
};
extern char lbl_80665458[];

struct Unk80308E5C_Obj {
    int m0;
    int m4;
    void* m8;
    int mC;
    void* m10;
    void* m14;
    void* m18;
    void* m1C;
    void* m20;
    int m24;
};
struct Unk8030B728_Obj {
    char pad0[0x50];
    int m50;
    char m54[0xD0 - 0x54];
    int mD0;
};

struct Unk80308FF4_Obj {
    char pad0[0x28];
    float m28;
    float m2C;
    float m30;
    float m34;
    char pad38[4];
    int m3C;
    int m40;
    float m44;
    float m48;
};
extern float lbl_8061FBC0[];
extern float lbl_8061FBCC;
extern int lbl_8061B9C8;

struct Unk8030B45C_Obj {
    void* m0;
    void* m4;
    void* m8;
    char padC[0x4C - 0xC];
    int m4C;
    char pad50[4];
    char m54[0x90 - 0x54];
    void* m90;
    char m94[4];
};
void fn_8030B5F4(void* p);
void fn_8030B618(void* p);
extern const char lbl_805E89D4[];
extern const char lbl_805E89E0[];
extern const char lbl_805E89EC[];
extern const char lbl_805E89F0[];
extern const char lbl_805E89F8[];
extern const char lbl_805E8A00[];
extern char lbl_8061FBDC[];
extern void* lbl_8062C290;
extern "C" {
void* fn_80308678(void* e, int a, int b, int c, int d, int f);
void fn_80308414(void* e, int a, int b, int c, float f);
void* fn_803774A8(int size, int heap);
void fn_80308B5C(void* p);
void fn_80308E5C(void* p);
void fn_80377408(void* p);
void fn_80308FF0();
void fn_80308F70(void* p);
void fn_80328234(void* p, int a, int b);
void fn_80328374(void* p, int c);
void fn_8030ACCC(Unk80308874_List* p);
void* fn_8026C75C(void* p, int a, int b);
void fn_8026C864(void* p, int c);
void fn_80319310(int a, void* p, int k);
void OnWindowClose(void* p);
void fn_803E6B74(void* p);
void fn_8030C11C(Unk80308874_At10* p);
void fn_803DD73C(int h, int k);
void fn_8037B424(void* p, int c);
void* fn_8030BEB8(void* p);
void* fn_8014D9C4(void* p);
void fn_8040074C(void* p, unsigned n);
void fn_8030BFF8(Unk8030BEB8* p);
unsigned int strlen(const char* s);
char* strncpy(char* d, const char* s, unsigned n);
int rand(void);
void* fn_803DD628(void* self, void* info);
void fn_803DD7C8(void* tex, int a);
int fn_803E69D8(void* p, const char* name, void* data, int size, int count);
void fn_8031259C(void* g);
void fn_803125D0(void* g);
void fn_803E6C28(void* p, int n);
void fn_8040B61C(void* d, const char* s);
Unk802F8324_VT8* fn_80319330(void* sub, int k);
int fn_802B2DB8(void* p);
void fn_8031479C(void* g);
void fn_803DBD84(void* p, int k);
void fn_803E6708(void* p, int k);
void fn_803DBE2C(void* p, int k);
void fn_803DBE38(void* p, int k);
void fn_803DBF00(void* p, int k);
void fn_803E67A0(void* h, void* p);
int fn_803E6950(void* h, const char* s);
void fn_803E68B0(void* h, int k);
int fn_803DBDD8(void* p, int k);
int fn_803DBE18(void* p, int k);
int fn_803DBE20(void* p, int k);
int fn_803DBE68(void* p, int k);
int fn_803DBE78(void* p, int k);
int fn_803DBE44(void* p, int k);
}

struct Unk80308C74_R {
    float m0;
    float m4;
    float m8;
    float mC;
    float m10;
    float m14;
};
struct Unk80308C74_O {
    Unk80308C74_R r[6];
    char pad90[0x24];
    int mB4;
    int mB8;
    int mBC;
    int mC0;
    int mC4;
    int mC8;
    int mCC;
    const void* mD0;
    Unk80308C74_O();
};
struct Unk80308874_Buf {
    int m0;
    int m4;
    void* m8;
    void* mC;
    int m10;
    int m14;
    int m18;
};
extern void (*lbl_8062C66C)(void*, int);
extern const float lbl_805E8898;
extern "C" void fn_803D79A0(int h, int k);
extern "C" void fn_80377488(void* p);
inline void Unk80308874_FreeArr(void* p, int sz)
{
    int n = *(int*)((char*)p - 8);
    lbl_8062C66C((char*)p - 8, n * sz | 8);
}

inline void Unk80308874_FreeArr2(void* p, int sz)
{
    int n = *(int*)((char*)p - 8);
    lbl_8062C66C((char*)p - 8, n * sz + 8);
}

int fn_80308874(Unk80308874_Buf* p)
{
    if (p->m18 != 0) {
        fn_803D79A0(p->m18, 3);
        p->m18 = 0;
    }
    if (p->m4 != 0) {
        fn_80377488((void*)p->m4);
        p->m4 = 0;
    }
    if (p->m0 != 0) {
        fn_80377488((void*)p->m0);
        p->m0 = 0;
    }
    if (p->mC != 0) {
        Unk80308874_FreeArr(p->mC, 16);
        p->mC = 0;
    }
    if (p->m8 != 0) {
        Unk80308874_FreeArr(p->m8, 16);
        p->m8 = 0;
    }
    p->m14 = 0;
    p->m10 = 0;
    return 1;
}

int fn_80308960(Unk80308874_Mgr* m, int i, int a, int b, int c)
{
    fn_80308678(m->m100 + i, a, m->m108, m->m10C, b, c);
    return 1;
}

extern "C" int fn_803089A8(Unk80308874_Mgr* m, int n)
{
    if (m->m100 != 0)
        return 0;
    m->m104 = n;
    m->m100 = new (0) Unk80308874_Ent50[n];
    for (int i = 0; i < n; i++) {
        m->m100[i].m1C = 0;
        m->m100[i].m20 = 0;
    }
    return 1;
}

int fn_80308A7C(Unk80308874_Slots* m, int i, int n)
{
    m->s[i].cap = n;
    m->s[i].n = 0;
    m->s[i].p = (int*)fn_803774A8(n * 4, 0);
    return 1;
}

void fn_80308AD0(Unk80308874_Mgr* m, int a, int b, int c, int i, float f)
{
    fn_80308414(m->m100 + i, a, b, c, f);
}

void fn_80308B04(Unk80308874_Slots* m, int v, int i)
{
    int n = m->s[i].n;
    int q = (int)m->s[i].p;
    *(int*)(q + n * 4) = v;
    m->s[i].n = ++n;
}

/* near-miss: retail keeps this in r30 and the argument in r29 (callee-saved order swapped; pass-through of r4 tried) */
void fn_80308B28(Unk80308874_At4* m, int v)
{
    fn_80308B5C(m);
    *(int*)((char*)m + 0x10C) = v;
}

extern "C" void fn_80308B5C(void* vp)
{
    Unk80308874_Mgr* m = (Unk80308874_Mgr*)vp;
    if (m->m100 != 0) {
        for (int i = 0; i < m->m104; i++)
            fn_80308874((Unk80308874_Buf*)(m->m100 + i));
        if (m->m100 != 0) {
            Unk80308874_Ent50* e = m->m100 + *(int*)((char*)m->m100 - 8);
            if (m->m100 != e) {
                do {
                    e--;
                    fn_80308874((Unk80308874_Buf*)e);
                } while (m->m100 != e);
            }
            fn_80377488((char*)m->m100 - 8);
            m->m100 = 0;
        }
    }
    for (unsigned i = 0; i <= 15; i++) {
        int off = i * 12;
        if (*(void**)((char*)m + off)) {
            fn_80377488(*(void**)((char*)m + off));
            *(void**)((char*)m + off) = 0;
        }
    }
    m->m108 = 0;
    if (m->m10C) {
        fn_803DD73C(m->m10C, 3);
        m->m10C = 0;
    }
}

extern "C" void fn_80308C60(Unk80308874_At20C* p, int i, int v)
{
    SetD(p->m20C + 0xC0, i, v);
}

//@80308C74
//@80308C74
Unk80308C74_O::Unk80308C74_O()
{
    mD0 = lbl_80665950;
    Unk80308C74_R* q = r;
    for (int i = 0; i < 6; i++) {
        r[i].mC = lbl_805E8898;
        q->m8 = lbl_805E8898;
        q->m4 = lbl_805E8898;
        q->m0 = lbl_805E8898;
        q++;
    }
    int z = 0;
    mB4 = 4;
    mB8 = 2;
    mBC = 3;
    mC0 = 1;
    mC4 = z;
    mC8 = 5;
    mCC = 6;
}

void fn_80308CFC(Unk80308874_AtD0* p, int c)
{
    p->mD0 = lbl_80665950;
    if (c & 1)
        fn_80377408(p);
}

extern "C" void* fn_80308D30(Unk80308D30_Obj* p, int owner)
{
    fn_80328234(p, 0x10, owner);
    p->m24 = 0;
    p->m30 = 0.0f;
    p->m34 = 0.0f;
    p->m38 = 0.0f;
    p->m14 = 0;
    p->m18 = lbl_80665A88;
    fn_8040B61C(p, lbl_805E88B0);
    p->m28 = 0.0f;
    p->m2C = 0.0f;
    p->m3C = 0;
    p->m40 = 1;
    p->m44 = 1.0f;
    p->m48 = 1.0f;
    fn_80308F70(p);
    return p;
}

void fn_80308DD8(Unk80308874_At18* p, int c)
{
    p->m18 = lbl_80665A88;
    p->m24 = 0;
    fn_80308FF0();
    fn_80328374(p, c);
}

/* near-miss: same register order issue as fn_80308B28 */
void fn_80308E28(Unk80308874_At4* p, int v)
{
    fn_80308E5C(p);
    p->m4 = v;
}

extern "C" void fn_80308E5C(void* vp)
{
    Unk80308E5C_Obj* p = (Unk80308E5C_Obj*)vp;
    p->m0 = 0;
    if (p->m4 != 0) {
        fn_803DD73C(p->m4, 3);
        p->m4 = 0;
    }
    if (p->m24 != 0) {
        fn_803D79A0(p->m24, 3);
        p->m24 = 0;
    }
    if (p->m10 != 0) {
        fn_80377488(p->m10);
        p->m10 = 0;
    }
    if (p->m8 != 0) {
        fn_80377488(p->m8);
        p->m8 = 0;
    }
    if (p->m14 != 0) {
        fn_80377488(p->m14);
        p->m14 = 0;
    }
    if (p->m18 != 0) {
        Unk80308874_FreeArr(p->m18, 16);
        p->m18 = 0;
    }
    if (p->m1C != 0) {
        Unk80308874_FreeArr(p->m1C, 16);
        p->m1C = 0;
    }
    if (p->m20 != 0) {
        fn_80377488(p->m20);
        p->m20 = 0;
    }
    p->mC = 0;
}

extern "C" void fn_80308F70(void* p)
{
    fn_80308FF0();
    for (int i = 0; i < 4; i++)
        lbl_80691410[i] = (float)(lbl_8061FBAC[i] - lbl_8061FB98[i]);
}

void fn_80308FF0()
{
}

extern "C" void fn_80308FF4(Unk80308FF4_Obj* p, float t)
{
    float x = t * 0.05681818f;
    float a = lbl_8061FBCC;
    float b = a;
    if (!(x >= 0.0f)) {
        p->m3C = 3;
        a = 1.0f;
        p->m40 = 3;
        b = 1.0f;
    } else if (!(x >= 10.1f)) {
        a = (x - 5.0f) * 0.4f + 8.0f;
        b = ((x - 10.0f) - 5.0f) * 0.4f + 8.0f;
        p->m3C = 2;
        p->m40 = 2;
    } else if (!(x >= 20.1f)) {
        p->m3C = 1;
        p->m40 = 2;
        a = (x - 12.5f) + 12.5f;
        b = ((x - 10.0f) - 5.0f) * 0.4f + 8.0f;
    } else {
        p->m3C = 0;
        p->m40 = 1;
        a = (x - 33.0f) * 0.71428573f + 33.0f;
        b = ((x - 10.0f) - 12.5f) + 12.5f;
    }
    p->m44 = a / lbl_8061FBC0[p->m3C];
    p->m48 = b / lbl_8061FBC0[p->m40];
}

extern "C" void fn_803091B4(Unk80308FF4_Obj* p)
{
    float s = (float)lbl_8061B9C8 * 0.016666668f;
    float d = s * (lbl_8061FBC0[p->m3C] * p->m44 / (lbl_80691410[p->m3C] + 1.0f));
    p->m30 = p->m30 + d;
    if (!(p->m30 <= 1.0f))
        p->m30 = p->m30 - 1.0f;
    p->m28 = p->m30;
    float d2 = s * (lbl_8061FBC0[p->m40] * p->m48 / (lbl_80691410[p->m40] + 1.0f));
    p->m34 = p->m34 + d2;
    if (!(p->m34 <= 1.0f))
        p->m34 = p->m34 - 1.0f;
    p->m2C = p->m34;
}

void fn_80309A74(Unk80308874_At24* p, int v)
{
    p->m24 = v;
}

int fn_80309A7C(Unk80308874_Pair* s, int** pp)
{
    int* d = *pp;
    *(Unk80308874_P8*)d = *(Unk80308874_P8*)&s->m28;
    *pp = d + 2;
    return 1;
}

void fn_80309AA0(Unk80308874_FSlots* p, int i)
{
    Unk80308874_FSlot* s = &p->s[i];
    s->m0 = 0;
    s->m4 = 0;
    s->m8 = 0.0f;
}

void fn_80309AC8(Unk80308874_FSlots* p, int i, float* a, int b, float f)
{
    Unk80308874_FSlot* s = &p->s[i];
    s->m0 = a;
    s->m4 = b;
    s->m8 = f;
}

void fn_80309AF4(Unk80308874_FSlots* p, int i)
{
    int off = i * 0xC + 0x4C;
    *(Unk80308874_FSlot*)((char*)p + off) = p->m58;
}

void fn_80309B20(Unk80308874_FSlots* p, int i)
{
    int off = i * 0xC + 0x4C;
    float* q = *(float**)((char*)p + off);
    if (q)
        p->m28 = *q;
}

void* fn_8030AC4C(Unk80308874_List* p)
{
    p->m0 = 0;
    p->m4 = 0;
    fn_8030ACCC(p);
    return p;
}

extern "C" void fn_8030AC88(Unk80308874_List* p)
{
    for (int i = 0; i < p->m0; i++) {
        Unk80308874_Hdr* h = p->m8[i];
        if (h)
            h->m2 = 1;
    }
}

void fn_8030ACCC(Unk80308874_List* p)
{
    for (int i = 0; i < 0x50; i++) {
        p->m8[i] = 0;
        if (p->m4 != 0)
            lbl_806E0FA4[i] = 0x51;
    }
    p->m0 = 0;
}

void fn_8030AF34(void* pv, int k)
{
    Unk80308874_List* p = (Unk80308874_List*)pv;
    if (p->m4 != 0) {
        if (k != 0x51) {
            do {
                char* b = (char*)p + 8;
                int o = k * 4;
                Unk80308874_Hdr* h = *(Unk80308874_Hdr**)(b + o);
                if (h)
                    h->m2 = 1;
                k = lbl_806E0FA4[k];
            } while (k != 0x51);
        }
    } else {
        int o = k * 4;
        char* b = (char*)p + 8;
        Unk80308874_Hdr* h = *(Unk80308874_Hdr**)(b + o);
        if (h)
            h->m2 = 1;
    }
}

void fn_8030AF9C(void* pv, int k)
{
    Unk80308874_List* p = (Unk80308874_List*)pv;
    if (p->m4 != 0) {
        if (k != 0x51) {
            do {
                char* b = (char*)p + 8;
                int o = k * 4;
                Unk80308874_Hdr* h = *(Unk80308874_Hdr**)(b + o);
                if (h)
                    h->m2 = 0;
                k = lbl_806E0FA4[k];
            } while (k != 0x51);
        }
    } else {
        int o = k * 4;
        char* b = (char*)p + 8;
        Unk80308874_Hdr* h = *(Unk80308874_Hdr**)(b + o);
        if (h)
            h->m2 = 0;
    }
}

void* fn_8030B000(Unk80308874_B8* p, int a)
{
    fn_8026C75C(p, 7, a);
    p->mB8 = lbl_80665470;
    p->m98 = lbl_80665500;
    return p;
}

void fn_8030B050(Unk80308874_B8* p, int c)
{
    p->mB8 = lbl_80665470;
    p->m98 = lbl_80665500;
    fn_8026C864(p, c);
}

void fn_8030B088()
{
}

extern "C" void fn_8030B08C(Unk8030B08C_Obj* p)
{
    Unk802F8324_VT8* r = fn_80319330(p->m244, 1);
    if (lbl_8061B928 == 0) {
        char* q = (char*)lbl_8061E978 + 0xAF4;
        *(float*)(q + 0x14) = fn_801A0744((Unk8019F8C0_Owner*)p->mBC);
        fn_802B2DB8(q);
        if (r) {
            if (lbl_8061FBD8 != 0) {
                fn_8031479C(lbl_8061FEDC);
                r->s4();
            }
        }
    }
}

void* fn_8030B130(Unk80308874_At8* p, int a, int b)
{
    p->m8 = lbl_80665D88;
    fn_80319310(a, p, 0);
    p->m4 = b;
    p->m0 = 0;
    return p;
}

extern "C" void fn_8030B358(Unk8030B358_Obj* p, int c)
{
    p->m8D4 = lbl_80665458;
    if (p->m0) {
        Unk80308874_FreeArr2(p->m0, 12);
        p->m0 = 0;
    }
    if (p->m4) {
        Unk80308874_FreeArr2(p->m4, 8);
        p->m4 = 0;
    }
    if (p->m8) {
        Unk80308874_FreeArr2(p->m8, 4);
        p->m8 = 0;
    }
    p->m50 = 0;
    fn_803E6708(p->m94, 2);
    fn_803DBD84(p->m54, 2);
    if (c & 1)
        fn_80377408(p);
}

extern "C" void fn_8030B45C(void* pp, int xi)
{
    Unk8030B45C_Obj* p = (Unk8030B45C_Obj*)pp;
    void* x = (void*)xi;
    char* h = p->m94;
    fn_803E67A0(h, (char*)p + 0xC);
    char* s = p->m54;
    fn_8030B5F4(p);
    fn_803E6950(h, lbl_805E89D4);
    fn_803E69D8(h, lbl_805E89D4, lbl_8061FBDC, 4, 1);
    fn_803E69D8(h, lbl_805E89E0, p->m0, 0xC, p->m4C * 6);
    fn_803E69D8(h, lbl_805E89EC, p->m4, 8, p->m4C * 6);
    fn_803E69D8(h, lbl_805E89F0, p->m8, 4, p->m4C * 6);
    fn_803DBDD8(s, 0x90);
    fn_803DBE18(s, 1);
    fn_803DBE20(s, 0);
    fn_803DBE68(s, 1);
    fn_803DBE78(s, 1);
    ((int (*)(void*, int))fn_803DBE38)(s, 1);
    fn_803DBE44(s, 0);
    fn_803E69D8(h, lbl_805E89F8, s, 0, -1);
    fn_803E69D8(h, lbl_805E8A00, lbl_8062C290, 0x40, 1);
    fn_803E68B0(h, 0x90);
    fn_803E69D8(h, lbl_805E8A0C, x, 0, -1);
    p->m90 = x;
    fn_8030B618(p);
}

void fn_8030B5F4(void* p)
{
    OnWindowClose((char*)p + 0x94);
}

void fn_8030B618(void* p)
{
    fn_803E6B74((char*)p + 0x94);
}

extern "C" void fn_8030B63C(Unk8030B63C_Obj* p, void* x)
{
    if (x != 0) {
        if (p->m90 != x) {
            fn_8030B5F4(p);
            p->m90 = x;
            fn_803DD7C8(x, 0);
            p->m50 = 0;
            fn_803E69D8(p->m94, lbl_805E8A0C, x, 0, -1);
            fn_8030B618(p);
        }
    }
}

extern "C" void fn_8030B6B8(Unk8030B63C_Obj* p, int clear)
{
    if (p->m50 > 0) {
        void* g = lbl_8061FEDC;
        fn_8031259C(g);
        fn_803E6C28(p->m94, p->m50 * 6);
        fn_803125D0(g);
    }
    if (clear)
        p->m50 = 0;
}

extern "C" void fn_8030B728(Unk8030B728_Obj* p, int mode)
{
    if (mode != p->mD0) {
        if (p->m50 > 0)
            p->m50 = 0;
        fn_8030B5F4(p);
        fn_803DBE2C(p->m54, 1);
        switch (mode) {
        case 0:
            fn_803DBF00(p->m54, 1);
            fn_803DBE38(p->m54, 2);
            break;
        case 1:
            fn_803DBF00(p->m54, 0);
            fn_803DBE38(p->m54, 2);
            break;
        case 2:
            fn_803DBF00(p->m54, 1);
            fn_803DBE38(p->m54, 1);
            break;
        case 3:
            fn_803DBF00(p->m54, 0);
            fn_803DBE38(p->m54, 1);
            break;
        }
        fn_8030B618(p);
        p->mD0 = mode;
    }
}

extern "C" void* fn_8030BDE8(Unk8030BDE8* p)
{
    int k = 2;
    Unk8030BDE8_Q* q = p->q;
    do {
        fn_8014D9C4(q);
        q++;
    } while (k-- != 0);
    p->m30 = 2;
    p->m34 = 0.5f;
    fn_8040074C(p, 0x30);
    return p;
}

extern "C" void fn_8030BE54(Unk8030BDE8* p, float f)
{
    p->m34 = f;
    if (p->m30 > 2) {
        if (!(p->m34 <= 0.0f || p->m34 >= 1.0f)) {
            p->m38 = 1.0f / p->m34;
            p->m3C = 1.0f / (1.0f - p->m34);
        }
    }
}

extern "C" void* fn_8030BEB8(void* vp)
{
    Unk8030BEB8* p = (Unk8030BEB8*)vp;
    new (p) Unk8037B3E8(0xD99143B6u);
    p->m0 = lbl_806653F8;
    p->mC = 0;
    p->m10 = 0;
    p->m14 = 0;
    p->m18 = 0;
    p->m1C = 0;
    p->m68 = 0.0f;
    p->m74 = 0.0f;
    p->m78 = 0.0f;
    p->m7C = 0.0f;
    p->m90 = 0.0f;
    p->m94 = 0.0f;
    p->m8 = 0.0f;
    p->mA4 = 0;
    p->m4 = 0.0f;
    for (int i = 0; i < 6; i++)
        p->m20[i] = lbl_806055AC;
    p->m70 = 0.0f;
    p->m6C = 0.0f;
    p->m84 = 0.0f;
    p->m80 = 0.0f;
    p->m8C = 0.0f;
    p->m88 = 0.0f;
    p->m98 = lbl_806055AC;
    return p;
}

extern "C" void fn_8030BFB0(Unk80308874_At4* p, int c)
{
    *(const void**)p = lbl_806653F8;
    fn_8030C11C((Unk80308874_At10*)p);
    fn_8037B424(p, c);
}

extern "C" void fn_8030BFF8(Unk8030BEB8* p)
{
    Unk802ACD88_Mgr* m = lbl_8061E978;
    fn_8030C11C((Unk80308874_At10*)p);
    if (m) {
        if (fn_8030BFF8_Has(m)) {
            int idx = p->m14;
            int n = strlen(lbl_8061FBF0);
            int cnt = 0;
            char buf[5] = "NONE";
            for (int i = 0; i < n; i++) {
                if (lbl_8061FBF0[i] == ',') {
                    cnt++;
                } else if (cnt == idx) {
                    strncpy(buf, lbl_8061FBF0 + i, 4);
                    void* r = fn_802AFA54(m, buf);
                    if (r)
                        p->m10 = (int)fn_803DD628(lbl_8062C668(0x58, 0), r);
                    break;
                }
            }
        }
    }
    p->mA4 = rand() % 1024;
}

void fn_8030C11C(Unk80308874_At10* p)
{
    if (p->m10) {
        fn_803DD73C(p->m10, 3);
        p->m10 = 0;
    }
}

void* fn_8030C554()
{
    return fn_8030BEB8(operator new(0xA8, lbl_806EF9B4));
}

extern "C" void fn_8030C584(Unk8030BEB8* p)
{
    float a = p->m90 * p->m94;
    float b = 440.0f;
    if (!(a < b))
        p->m94 = b / p->m90;
    fn_8030BFF8(p);
}
