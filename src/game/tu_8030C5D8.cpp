/*
 * Game code, .text 0x8030C5D8-0x80310934 (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_802F8324.h"

extern char lbl_80665308[];
extern char lbl_80665278[];
extern Unk802F8324_VT18* lbl_8061FE10;
extern const char* lbl_8061FBF4[];

struct Unk8030C5D8_V2 {
    float x;
    float y;
};
struct Unk8030C5D8_Slot {
    int m0;
    int m4;
    float m8;
};
struct Unk8030C5D8_Slots {
    char pad0[0x24];
    Unk8030C5D8_Slot s[1];
    Unk8030C5D8_Slot m30;
};
struct Unk8030C5D8_At18 {
    char pad0[0x18];
    const void* m18;
};
struct Unk8030C5D8_Ent {
    int m0;
    int m4;
    char pad8[0x58 - 8];
};
struct Unk8030C5D8_List {
    char pad0[0x1C];
    int m1C;
    Unk8030C5D8_Ent e[1];
};
struct Unk8030C5D8_Obj4 {
    int m0;
    int m4;
};
struct Unk8030C5D8_P6C {
    char pad0[0x6C];
    Unk8030C5D8_Obj4* m6C[1];
};

struct V3x {
    float x;
    float y;
    float z;
};
struct Unk8030C5D8_V3 {
    int a;
    int b;
    int c;
};
struct Unk8030C5D8_FxObj {
    char pad0[4];
    float m4;
    char pad8[0x94 - 8];
    float m94;
    Unk8030C5D8_V3 m98;
};
struct Unk8030C5D8_FxEnt {
    Unk8030C5D8_FxObj* m0;
    int m4;
    float m8;
    Unk8030C5D8_V3 mC;
    char pad18[0x58 - 0x18];
};
struct Unk8030C5D8_Fx {
    const void* m0;
    int m4;
    char pad8[4];
    float mC;
    float m10;
    char pad14[8];
    int m1C;
    Unk8030C5D8_FxEnt e[5];
    float m1D8;
};
struct Unk8030C5D8_Md {
    char pad0[0x4C];
    unsigned int m4C;
};
struct Unk8030C5D8_PM {
    char pad0[0x6C];
    Unk8030C5D8_Fx* m6C[17];
};
struct Unk8030C5D8_Cur {
    int* p;
};
extern float lbl_806E10E4[];
extern void (*lbl_8062C66C)(void* p, int n);
struct Unk8030C5D8_Dl {
    char pad0[0xB8];
    void* mB8;
    void* mBC;
};
struct Unk8030C5D8_Em {
    char pad0[0x64];
    float m64;
    float m68;
    float m6C;
    float m70;
    float m74;
    float m78;
    float m7C;
    char pad80[0xC4 - 0x80];
    float mC4[8];
    float mE4[8];
};
struct Unk8030C5D8_F3 {
    float x;
    float y;
    float z;
};
struct Unk8030C5D8_Em2 {
    char pad0[0x84];
    Unk8030C5D8_F3 m84;
    Unk8030C5D8_F3 m90;
    float m9C;
    char pad_A0[0xA0 - 0xA0];
    float mA0;
    float mA4;
    float mA8;
    float mAC;
    float mB0;
};
struct Unk8030C5D8_Ctl {
    char pad0[0x6F4];
    int m6F4;
};

struct Unk8030C5D8_Pm {
    char pad0[0x14];
    int m14;
    const void* m18;
    char pad1C[0x6C - 0x1C];
    Unk8030C5D8_Fx* m6C[17];
};
struct Unk802FCAC0_Ent {
    char pad0[4];
    int m4;
    char pad8[0xC - 8];
    int mC;
    char pad10[0x24 - 0x10];
    Unk802F8324_V2 m24;
};

struct Unk802FCAC0_Obj {
    char pad0[0x18];
    const void* m18;
    char pad1C[8];
    int m24;
    Unk802FCAC0_Ent* m28[3];
    int m34_unused;
};

extern "C" void fn_8030D090(Unk802FCAC0_Obj* p);

struct Unk8030C728_Ent {
    char b[0x58];
    ~Unk8030C728_Ent() {}
};
extern char lbl_80665398[];
struct Unk8030C728_Obj {
    const void* m0;
    char pad4[0x1C];
    Unk8030C728_Ent e[5];
};

struct Unk8030E63C_Obj {
    char pad0[0x18];
    const void* m18;
    char pad1C[8];
    int m24;
    char pad28[0x38 - 0x28];
    int m38;
    int m3C;
    char pad40[0x58 - 0x40];
    int m58;
    int m5C;
    int m60;
    float m64;
    float m68;
    float m6C;
    float m70;
    float m74;
    float m78;
    float m7C;
    int m80;
    char pad84[0xC0 - 0x84];
    float mC0;
    char padC4[0x110 - 0xC4];
    int m110;
    char m114[0x150 - 0x114];
    char m150[0x18C - 0x150];
    const void* m18C;
    int m190;
    char pad194[0x19C - 0x194];
    int m19C;
    int m1A0;
};
extern char lbl_80693CFC[];
extern const char lbl_805E79A4[];


struct Unk8030C654_V3 {
    int a;
    int b;
    int c;
};
struct Unk8030C654_E {
    int m0;
    int m4;
    float m8;
    Unk8030C654_V3 mC;
    char m18[0x40];
};
struct Unk8030C654_P {
    const void* m0;
    int m4;
    int m8;
    float mC;
    float m10;
    float m14;
    float m18;
    int m1C;
    Unk8030C654_E e[5];
    float m1D8;
};
extern char lbl_80665398[];
extern const Unk8030C654_V3 lbl_806055AC;
struct Unk8030FC1C_V3 {
    float x;
    float y;
    float z;
};
struct Unk8030FC1C_Src {
    char pad0[0x1D44];
    Unk8030FC1C_V3 v;
};
struct Unk8030FC1C_P {
    char pad0[0x104];
    Unk8030FC1C_V3 m104;
};
struct Unk8030CCC4_V4 {
    float x;
    float y;
    float z;
    float w;
};
inline Unk8030CCC4_V4 Unk8030CCC4_Cp4(const Unk8030CCC4_V4* s)
{
    return *s;
}
struct Unk8030CCC4_E {
    Unk8030CCC4_V4 q[3];
    int m30;
    float m34;
    char pad38[0x58 - 0x38];
};
struct Unk8030CCC4_L {
    char pad0[0x1C];
    int m1C;
    char pad20[0x38 - 0x20];
    Unk8030CCC4_E e[1];
};
extern "C" Unk8030CCC4_V4 fn_8014DAFC(Unk8030CCC4_V4 v, float s);
extern "C" void fn_8030BE54(void* p, float f);
struct Unk8030D2E4_Fx {
    const void* m0;
    int m4;
    int m8;
    float mC;
    float m10;
    char pad14[8];
    int m1C;
    Unk8030C5D8_FxEnt e[5];
};
struct Unk8030D2E4_Obj {
    char pad0[0x1C];
    Unk8030C5D8_Md* m1C;
    char pad20[0x6C - 0x20];
    Unk8030D2E4_Fx* m6C[0x11];
};
inline float Unk8030D2E4_Secs(unsigned n)
{
    return (float)n * 0.016666668f;
}
struct Unk8030EF64_Obj {
    char pad0[0x24];
    int m24;
    float m28;
    float m2C;
    float m30;
    float m34;
    char pad38[0x64 - 0x38];
    float m64;
};
void fn_8030FA9C(Unk8030C5D8_Em* p);
void fn_8030FB5C(Unk8030C5D8_Em* p);
struct Unk8030FD50_Pr {
    int m0;
    int m4;
};
struct Unk8030FD50_Obj {
    char pad0[0x58];
    int m58;
    int m5C;
    char pad60[0xB8 - 0x60];
    Unk8030C5D8_F3* mB8;
    Unk8030FD50_Pr* mBC;
    char padC0[0xE4 - 0xC0];
    float mE4[8];
    Unk8030C5D8_F3 m104;
};
struct U;
U* fn_8035F788(U* self);
struct Unk8030FECC_B {
    int w[40];
};
struct Unk8030FECC_Q {
    int w[4];
};
struct Unk8030FECC_Obj {
    const void* m0;
    char pad4[0xA0];
    Unk8030FECC_B mA4;
    char pad144[0x704 - 0x144];
    Unk8030FECC_B g[4][2];
    Unk8030FECC_B h[4];
    Unk8030FECC_Q mE84;
    char padE94[0x10E0 - 0xE94];
    int m10E0;
};
extern Unk8030FECC_Q lbl_806055E8;
extern char lbl_806657C8[];
extern unsigned int lbl_8061FE1C;
struct Unk8019F8C0_Owner;
struct Unk8030F598_Obj {
    char pad0[0x1C];
    Unk8019F8C0_Owner* m1C;
    char pad20[0x60 - 0x20];
    int m60;
    float m64;
    char pad68[0x80 - 0x68];
    int m80;
    char pad84[0xB8 - 0x84];
    void* mB8;
    void* mBC;
    float mC0;
};
float fn_801A0744(Unk8019F8C0_Owner* o);
struct Unk8030FD50_Obj;
extern "C" void fn_8030FD50(Unk8030FD50_Obj* p, int i);
extern "C" void fn_8030F2C4(void* p, int a, float b);
extern "C" void fn_8030EF64(Unk8030EF64_Obj* p, float a, float b);
struct Unk8030F0C8_Own {
    char pad0[0x730];
    void* m730;
};
struct Unk8030F0C8_Obj {
    char pad0[0x1C];
    Unk8030F0C8_Own* m1C;
    char pad20[0x30 - 0x20];
    float m30;
    float m34;
    int m38;
    int m3C;
    Unk8030C5D8_F3 m40;
    Unk8030C5D8_F3 m4C;
    char pad58[0x64 - 0x58];
    float m64;
    char pad68[0xB4 - 0x68];
    float mB4;
};
struct Unk8030F0C8_Ent {
    char pad0[0x120];
    char* m120;
};
inline Unk8030C5D8_F3 Unk8030F0C8_Sub(const Unk8030C5D8_F3& a, const Unk8030C5D8_F3& b)
{
    Unk8030C5D8_F3 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}
struct Unk8030F768_Mtx {
    float m[16];
};
struct Unk8030F768_C {
    char pad0[0x54];
    int m54;
};
struct Unk8030F768_O {
    char pad0[0x54C];
    Unk8030F768_C* m54C;
};
struct Unk8030F768_Obj {
    char pad0[0x1C];
    Unk8019F8C0_Owner* m1C;
    char pad20[0x3C - 0x20];
    int m3C;
    char pad40[0x4C - 0x40];
    float m4C;
    char pad50[0x54 - 0x50];
    float m54;
    char pad58[0x80 - 0x58];
    int m80;
    float m84[6];
    char pad9C[0x9C - 0x9C];
    float m9C;
    float mA0;
    float mA4;
    float mA8;
    float mAC;
    float mB0;
    float mB4;
    char padB8[0x110 - 0xB8];
    int m110;
};
extern int lbl_8061FDCC;
extern const Unk8030F768_Mtx lbl_8060E938;
int fn_801A04A8(Unk8019F8C0_Owner* o, int k);
struct Unk8030EBC8_Obj {
    char pad0[0x1C];
    Unk8030C5D8_Md* m1C;
    char pad20[0x24 - 0x20];
    int m24;
    float m28;
    float m2C;
    float m30;
    float m34;
    char pad38[0x3C - 0x38];
    int m3C;
    char pad40[0x64 - 0x40];
    float m64;
};
inline float Unk8030EBC8_Secs(unsigned n)
{
    return (float)n * 0.016666668f;
}
extern "C" {
int fn_803E6D00(void* p, void* m);
int fn_803E6C28(void* p, int n);
}
extern "C" {
Unk8030F0C8_Ent* fn_8031C1E4(void* h, int id);
int fn_8031C2A4(void* h, int id, int* found, float t);
void fn_8031C328(void* h, int id);
}
extern "C" {
void OnWindowClose(void* p);
void fn_803E6B74(void* p);
void DCStoreRangeNoSync(void* p, int n);
}
extern "C" {
void fn_803E47E8(void* p);
void fn_803E4818(void* p);
void fn_8040074C(void* p, int n);
}
extern "C" {
char* fn_8040B61C(char* d, const char* s);
void fn_80328234(void* p, int a, int b);
void fn_8030B188(void* p, int n);
int fn_8030B45C(void* p, int n);
int rand(void);
int fn_803285A0(int n);
void fn_8030CE58(Unk8030C5D8_Fx* p);
void fn_8030CECC(Unk8030C5D8_Fx* p, int k);
void fn_8030CF7C(Unk8030C5D8_Fx* p, const Unk8030C5D8_V3* a, const Unk8030C5D8_V3* b);
void fn_8030E47C(Unk8030C5D8_PM* p);
void* fn_8030C654(void* p);
void* fn_8030E63C(void* mem, Unk8030C5D8_Ctl* p, int c);
void fn_8030E78C(void* p, int a);
float fn_80323AC0(void* a, int k);
void fn_8030E504();
void fn_8030B358(void* p, int k);
void fn_80328374(void* p, int c);
void fn_8030EB38(void* p);
void fn_803DBD84(void* p, int k);
void fn_803E6708(void* p, int k);
void fn_8030BFF8(int p);
void fn_8030C11C(void* p);
int fn_8030CE18(int h);
int fn_8030C5D8(int h);
void fn_8037B424(void* p);
void fn_803E6660(void* p);
int fn_803DBD54(void* p);
int fn_803E9B98(const void* a, void* b);
void* fn_8030BDE8(void* p);
void v3unit(const Unk8030FC1C_V3* v, Unk8030FC1C_V3* out);
}

int fn_8030C5D8(int p)
{
    return fn_8037C7FC()->fn_8037CE98(0xD99143B6, p, 0);
}

float fn_8030C618(void* a, const Unk8030C5D8_V2* v, int k)
{
    return v->y * fn_80323AC0(a, k) + v->x;
}

static inline void Unk8030C654_Init(Unk8030C654_E* e)
{
    e->m0 = 0;
    e->m4 = 0;
    e->m8 = 0.0f;
    e->mC = lbl_806055AC;
}

extern "C" void* fn_8030C654(void* vp)
{
    Unk8030C654_P* p = (Unk8030C654_P*)vp;
    new (p) Unk8037B3E8(0x77F3B9ABu);
    p->m0 = lbl_80665398;
    p->m4 = 0;
    p->m8 = 0;
    p->mC = 0.0f;
    p->m10 = 0.0f;
    p->m14 = 0.0f;
    p->m18 = 0.0f;
    p->m1C = 0;
    Unk8030C654_E* e = p->e;
    int k = 4;
    do {
        Unk8030C654_Init(e);
        fn_8030BDE8(e->m18);
        e++;
    } while (k-- != 0);
    p->m1D8 = 1.0f;
    return p;
}

extern "C" void fn_8030C728(Unk8030C728_Obj* p)
{
    p->m0 = lbl_80665398;
    Unk8030C728_Ent* b = p->e;
    if (b != 0) {
        Unk8030C728_Ent* e = b + 5;
        if (b != e) {
            do {
                e--;
            } while (b != e);
        }
    }
    fn_8037B424(p);
}

void fn_8030C774(Unk8030C5D8_List* p)
{
    for (int i = 0; i < p->m1C; i++) {
        p->e[i].m0 = fn_8030C5D8(p->e[i].m4);
        if (p->e[i].m0)
            fn_8030BFF8(p->e[i].m0);
    }
}

void fn_8030C7E4(Unk8030C5D8_List* p)
{
    for (int i = 0; i < p->m1C; i++) {
        int q = fn_8030C5D8(p->e[i].m4);
        if (q)
            fn_8030C11C((void*)q);
    }
}

void* fn_8030CC08()
{
    return fn_8030C654(operator new(0x1DC, lbl_806EF9B4));
}

void fn_8030CC38(Unk802F8324_VT0* a, Unk802F8324_VT0* b)
{
    if (b) {
        if (a->s4() == b->s4()) {
            Unk8030C5D8_Fx* x = (Unk8030C5D8_Fx*)a;
            Unk8030C5D8_Fx* y = (Unk8030C5D8_Fx*)b;
            y->m4 = x->m4;
            y->mC = x->mC;
            y->m10 = x->m10;
            y->m1D8 = x->m1D8;
        }
    }
}

extern "C" void fn_8030CCC4(Unk8030CCC4_L* p)
{
    for (int i = 0; i < p->m1C; i++) {
        if (p->e[i].m30 > 2)
            fn_8030BE54(&p->e[i], p->e[i].m34);
        for (int j = 0; j < p->e[i].m30; j++) {
            Unk8030CCC4_V4 a = Unk8030CCC4_Cp4(&p->e[i].q[j]);
            a.x *= 2.0f;
            a.y *= 2.0f;
            a.z *= 2.0f;
            p->e[i].q[j] = fn_8014DAFC(a, 1.0f);
        }
    }
    fn_8030C774((Unk8030C5D8_List*)p);
}

int fn_8030CE18(int p)
{
    return fn_8037C7FC()->fn_8037CE98(0x77F3B9AB, p, 0);
}

extern "C" void fn_8030CE58(Unk8030C5D8_Fx* p)
{
    Unk8030C5D8_Md* m = (Unk8030C5D8_Md*)fn_803285A0(1);
    p->m4 = 1;
    p->mC = (float)m->m4C * 0.016666668f;
    p->m10 = p->mC;
}

extern "C" void fn_8030CECC(Unk8030C5D8_Fx* p, int k)
{
    float t;
    for (int i = 0; i < p->m1C; i++) {
        Unk8030C5D8_FxObj* o = p->e[i].m0;
        if (o) {
            if (k != 0 || !(o->m94 >= 0.0f)) {
                o->m94 = -1.0f;
                p->m10 = -1.0f;
            } else {
                t = p->mC + p->e[i].m8 + o->m94 + o->m4;
                if (!(t <= p->m10))
                    p->m10 = t;
            }
        }
    }
}

extern "C" void fn_8030CF7C(Unk8030C5D8_Fx* p, const Unk8030C5D8_V3* a, const Unk8030C5D8_V3* b)
{
    for (int i = 0; i < p->m1C; i++) {
        if (p->e[i].m0)
            p->e[i].m0->m98 = *b;
        p->e[i].mC = *a;
    }
}

extern "C" void* fn_8030CFF4(Unk8030C5D8_Pm* p, int a)
{
    fn_80328234(p, 6, a);
    p->m18 = lbl_80665308;
    fn_8030B188((char*)p + 0xB0, 0x1B8);
    for (int i = 0; i < 0x11; i++)
        p->m6C[i] = 0;
    fn_8030D090((Unk802FCAC0_Obj*)p);
    fn_8030B45C((char*)p + 0xB0, 0);
    p->m14 = 0;
    fn_8040B61C((char*)p, "ParticleManager");
    return p;
}

extern "C" void fn_8030D090(Unk802FCAC0_Obj* q)
{
    Unk8030C5D8_PM* p = (Unk8030C5D8_PM*)q;
    for (int i = 0; i < 0x400; i++)
        lbl_806E10E4[i] = (float)(rand() % 10000) * 0.0002f - 1.0f;
    for (int i = 0; i < 0x11; i++) {
        Unk8030C5D8_Fx* o = p->m6C[i];
        if (o) {
            o->mC = 0.0f;
            o->m10 = 0.0f;
        }
    }
    fn_8030E47C(p);
}

extern "C" void fn_8030D170(Unk802FCAC0_Obj* p)
{
    fn_8030D090(p);
}

void fn_8030D190(Unk8030C5D8_At18* p, int c)
{
    p->m18 = lbl_80665308;
    fn_8030E504();
    fn_8030B358((char*)p + 0xB0, 2);
    fn_80328374(p, c);
}

extern "C" void fn_8030D1E4(void* pp, int idx, V3x* a, V3x* b, int depth, float f)
{
    Unk8030C5D8_PM* p = (Unk8030C5D8_PM*)pp;
    Unk8030C5D8_Fx* o = p->m6C[idx];
    if (o && o->m4 == 0) {
        fn_8030CE58(o);
        fn_8030CECC(o, 0);
        fn_8030CF7C(o, (const Unk8030C5D8_V3*)a, (const Unk8030C5D8_V3*)b);
        o->m1D8 = f;
    } else if (depth > 0 && idx <= 0x10) {
        fn_8030D1E4(p, idx + 1, a, b, depth - 1, f);
    }
}

void fn_8030D28C(Unk8030C5D8_P6C* p, int i)
{
    Unk8030C5D8_Obj4* q = p->m6C[i];
    if (q && q->m4 == 0) {
        fn_8030CE58((Unk8030C5D8_Fx*)q);
        fn_8030CECC((Unk8030C5D8_Fx*)q, 0);
    }
}

extern "C" void fn_8030D2E4(Unk8030D2E4_Obj* p)
{
    float t = Unk8030D2E4_Secs(p->m1C->m4C);
    for (int i = 0; i < 0x11; i++) {
        Unk8030D2E4_Fx* o = p->m6C[i];
        if (o) {
            o->m4 = 1;
            if (!(o->m10 >= 0.0f)) {
                int found = 0;
                for (int j = 0; j < o->m1C; j++) {
                    if (o->e[j].m0 && !(o->e[j].m0->m94 >= 0.0f)) {
                        found = 1;
                        break;
                    }
                }
                if (!found)
                    o->m10 = t;
            } else if (o->m10 < t || !(o->mC <= t)) {
                o->m4 = 0;
                if (o->m8) {
                    fn_8030CE58((Unk8030C5D8_Fx*)o);
                    fn_8030CECC((Unk8030C5D8_Fx*)o, 0);
                }
            }
        }
    }
}

int fn_8030E2F8(Unk8030C5D8_PM* p, int** out)
{
    int* cur = *out;
    int** pc = &cur;
    for (int i = 0; i < 0x11; i++) {
        Unk8030C5D8_Obj4* o = (Unk8030C5D8_Obj4*)p->m6C[i];
        if (o) {
            int* q = *pc;
            *q = o->m4;
            *pc = q + 1;
        }
    }
    *out = cur;
    return 1;
}

void fn_8030E354(Unk8030C5D8_Slots* p, int i)
{
    Unk8030C5D8_Slot* s = &p->s[i];
    s->m0 = 0;
    s->m4 = 0;
    s->m8 = 0.0f;
}

void fn_8030E37C(Unk8030C5D8_Slots* p, int i, int a, int b, float f)
{
    Unk8030C5D8_Slot* s = &p->s[i];
    s->m0 = a;
    s->m4 = b;
    s->m8 = f;
}

void fn_8030E3A8(Unk8030C5D8_Slots* p, int i)
{
    int off = i * 0xC + 0x24;
    *(Unk8030C5D8_Slot*)((char*)p + off) = p->m30;
}

void fn_8030E3D4(Unk8030C5D8_PM* p, int idx)
{
    int off = idx * 0xC + 0x24;
    Unk8030C5D8_Slot* s = (Unk8030C5D8_Slot*)((char*)p + off);
    int* a = (int*)s->m0;
    int* b = (int*)s->m4;
    int* x = a;
    int* y = b;
    for (int i = 0; i < 0x11; i++) {
        Unk8030C5D8_Fx* o = p->m6C[i];
        if (o)
            o->m4 = 0;
    }
    if (a == 0)
        return;
    if (b == 0)
        return;
    for (int i = 0; i < 0x11; i++) {
        Unk8030C5D8_Fx* o = p->m6C[i];
        if (o) {
            int f = 0;
            if (*x != 0 || *y != 0)
                f = 1;
            o->m4 = f;
            x++;
            y++;
        }
    }
}

extern "C" void fn_8030E47C(Unk8030C5D8_PM* p)
{
    Unk8030C5D8_Fx* o;
    for (int i = 0; i < 0x11; i++) {
        o = p->m6C[i] = (Unk8030C5D8_Fx*)fn_8030CE18(fn_80378568(lbl_8061FBF4[i], 0));
        if (o)
            fn_8030C774((Unk8030C5D8_List*)o);
    }
    o = p->m6C[0];
    if (o) {
        fn_8030CE58(o);
        fn_8030CECC(o, 1);
    }
}

void fn_8030E504()
{
    for (int i = 0; i < 0x11; i++) {
        int o = fn_8030CE18(fn_80378568(lbl_8061FBF4[i], 0));
        if (o)
            fn_8030C7E4((Unk8030C5D8_List*)o);
    }
}

void* fn_8030E560(Unk8030C5D8_Ctl* p, int a, int c)
{
    if (p && lbl_8061FE10 == 0 && p->m6F4 != 2 && p->m6F4 != 3) {
        void* t = fn_8030E63C(operator new(0x1BC, 0x400), p, c);
        lbl_8061FE10 = (Unk802F8324_VT18*)t;
        fn_8030E78C(t, a);
    }
    return lbl_8061FE10;
}

void fn_8030E5E4()
{
    if (lbl_8061FE10) {
        delete lbl_8061FE10;
        lbl_8061FE10 = 0;
    }
}

extern "C" void* fn_8030E63C(void* vp, Unk8030C5D8_Ctl* ctl, int c)
{
    Unk8030E63C_Obj* p = (Unk8030E63C_Obj*)vp;
    char* sub = (char*)p + 0x18C;
    char tmp[8];
    fn_80328234(p, 6, (int)ctl);
    p->m38 = c;
    p->m24 = 0;
    p->m3C = 0;
    p->m58 = 0;
    p->m5C = 0;
    p->m60 = 0;
    p->m64 = 0.0f;
    p->m78 = 0.0f;
    p->m7C = 0.0f;
    p->m80 = 0;
    p->m110 = 0;
    p->m18 = lbl_80665278;
    p->mC0 = -1.0f;
    fn_803E6660(p->m114);
    fn_803DBD54(p->m150);
    p->m18C = lbl_80693CFC;
    *(int*)(sub + 4) = 0;
    *(int*)(sub + 0x14) = fn_803E9B98(lbl_805E79A4, tmp);
    *(int*)(sub + 0x10) = 1;
    p->m5C = 0;
    p->m58 = 0;
    p->m74 = 0.0f;
    p->m70 = 0.0f;
    p->m6C = 0.0f;
    p->m68 = 0.0f;
    return p;
}

void fn_8030E72C(Unk8030C5D8_At18* p, int c)
{
    p->m18 = lbl_80665278;
    fn_8030EB38(p);
    fn_803DBD84((char*)p + 0x150, 2);
    fn_803E6708((char*)p + 0x114, 2);
    fn_80328374(p, c);
}

extern "C" int fn_8030EBC8(Unk8030EBC8_Obj* p, int st, int fl);
void fn_8030F9DC(Unk8030C5D8_Em2* p);
extern "C" void fn_8030FC1C(Unk8030FC1C_P* p, Unk8030FC1C_Src* src);
extern const float lbl_805E8FE8;
extern const float lbl_805E8FEC;
extern const float lbl_805E8FF0;
extern const float lbl_805E8FF4;
extern const float lbl_805E8FF8;
extern const float lbl_805E8FFC;
extern const float lbl_805E9000;
extern const float lbl_805E9004;
extern const float lbl_805E9008;
extern const float lbl_805E900C;
extern const float lbl_805E9010;
extern const float lbl_805E9014;
extern const float lbl_805E9018;
extern const float lbl_805E901C;
extern int lbl_8062157C;
extern char lbl_805E89D4[];
extern char lbl_805E89E0[];
extern char lbl_805E89F0[];
extern char lbl_805E89F8[];
extern char lbl_8061FDD0[];
extern void* (*lbl_8062C668)(int size, int heap);
extern "C" {
int fn_803E67A0(void* o, void* h);
int fn_803E6950(void* o, const char* name);
int fn_803E69D8(void* o, const char* name, void* data, int stride, int count);
int fn_803E68B0(void* o, int k);
int fn_803DBDD8(void* o, int v);
void fn_803DBF40(void* o, int a, int b, int c, int d);
int fn_803DBE18(void* o, int v);
int fn_803DBE20(void* o, int v);
int fn_803DBE2C(void* o, int v);
int fn_803DBE38(void* o, int v);
int fn_803DBE44(void* o, int v);
int fn_803DBE68(void* o, int v);
int fn_803DBE78(void* o, int v);
void fn_803DBF00(void* o, int v);
}
/* Emitter object: flags at +0x38/+0x3C, particle count at +0x60, constants and tuning floats at +0x64..+0xC0,
 * the "Coordinates" and "Colours" arrays at +0xB8/+0xBC (count-prefixed, allocated through lbl_8062C668), the
 * EAGL::DrawArray at +0x114 and its GeoPrimState at +0x150. */
struct Unk8030E78C_Obj {
    char pad0[0x38];
    int m38;
    int m3C;
    char pad40[0x58 - 0x40];
    int m58;
    int m5C;
    int m60;
    float m64;
    float m68;
    float m6C;
    float m70;
    float m74;
    float m78;
    float m7C;
    char pad80[0x9C - 0x80];
    float m9C;
    float mA0;
    float mA4;
    float mA8;
    float mAC;
    float mB0;
    float mB4;
    int* mB8;
    int* mBC;
    float mC0;
};
/* K3: setup of an ambient particle emitter (a point-sprite EAGL::DrawArray with the named vars "Parameters", "Coordinates",
 * "Colours" and "State"): picks the effect variant with rand() (rand() % 4 when m38 == 0x3E, else rand() % 9 kept only if <= 2), stores
 * it in lbl_8062157C and calls fn_8030EBC8 with it, fills 2 * 512 coordinate/colour slots and seeds them with fn_8030FD50 (rand-driven).
 * near-miss: 98.7%; same code, the float constants land in different FPRs and the lis/lfs pairs are scheduled differently. */
void fn_8030E78C(void* pp, int a)
{
    Unk8030E78C_Obj* p = (Unk8030E78C_Obj*)pp;
    int r = 0;
    p->m60 = 0x200;
    p->m58 = 0x6C79800F;
    p->m5C = 0x6C79804B;
    int flag = p->m38 == 0x3E;
    p->m68 = lbl_805E8FE8;
    p->m6C = lbl_805E8FEC;
    p->m74 = lbl_805E8FF4;
    p->m78 = lbl_805E8FF8;
    p->m7C = lbl_805E8FF0;
    p->m70 = lbl_805E8FF0;
    p->mC0 = lbl_805E8FFC;
    p->m64 = lbl_805E9000;
    if (flag) {
        r = rand() % 4;
        p->m3C = 1;
    } else {
        int t = rand() % 9;
        if (t <= 2)
            r = t;
    }
    fn_8030EBC8((Unk8030EBC8_Obj*)p, r, 0);
    lbl_8062157C = r;
    p->m9C = lbl_805E9004;
    p->mA0 = lbl_805E9008;
    p->mA4 = lbl_805E900C;
    if (!flag)
        p->mA4 = p->mA4 * lbl_805E9010;
    p->mA8 = lbl_805E9014;
    p->mB0 = lbl_805E901C;
    p->mAC = lbl_805E9018;
    p->mB4 = p->mA8 * p->mB0;
    fn_8030F9DC((Unk8030C5D8_Em2*)p);
    int n = p->m60 * 2;
    int* q = (int*)lbl_8062C668(n * 12 + 8, 0) + 2;
    q[-2] = n;
    p->mB8 = q;
    int* z = (int*)lbl_8062C668(n * 4 + 8, 0) + 2;
    z[-2] = n;
    int* zz = z;
    if (n != 0) {
        int c = n - 1;
        do {
            *zz = 0;
            zz++;
        } while (c-- != 0);
    }
    p->mBC = z;
    for (int i = 0; i < p->m60; i++)
        fn_8030FD50((Unk8030FD50_Obj*)p, i);
    char* h = (char*)p + 0x114;
    fn_8030FA9C((Unk8030C5D8_Em*)p);
    char* s = (char*)p + 0x150;
    fn_8030FB5C((Unk8030C5D8_Em*)p);
    fn_8030FC1C((Unk8030FC1C_P*)p, (Unk8030FC1C_Src*)a);
    fn_803E67A0(h, (char*)p + 0x18C);
    OnWindowClose(h);
    fn_803E6950(h, lbl_805E89D4);
    fn_803E69D8(h, lbl_805E89D4, lbl_8061FDD0, 0x10, 1);
    fn_803E69D8(h, lbl_805E89E0, p->mB8, 0xC, n);
    fn_803E69D8(h, lbl_805E89F0, p->mBC, 4, n);
    fn_803DBDD8(s, 0xA8);
    fn_803E68B0(h, 0xA8);
    fn_803DBF40(s, 1, 4, 1, 5);
    fn_803DBE38(s, 1);
    fn_803DBE78(s, 1);
    fn_803DBE18(s, 1);
    fn_803DBE68(s, 0);
    fn_803DBE44(s, 0);
    fn_803DBE20(s, 0);
    fn_803DBE2C(s, 3);
    fn_803DBF00(s, 0);
    fn_803E69D8(h, lbl_805E89F8, s, 0x3C, 1);
    fn_803E6B74(h);
}

extern "C" void fn_8030EB38(void* pp)
{
    Unk8030C5D8_Dl* p = (Unk8030C5D8_Dl*)pp;
    if (p->mB8 != 0) {
        lbl_8062C66C((char*)p->mB8 - 8, ((int*)p->mB8)[-2] * 12 + 8);
        p->mB8 = 0;
    }
    if (p->mBC != 0) {
        lbl_8062C66C((char*)p->mBC - 8, ((int*)p->mBC)[-2] * 4 + 8);
        p->mBC = 0;
    }
}

extern "C" int fn_8030EBC8(Unk8030EBC8_Obj* p, int st, int fl)
{
    float t0 = 0.0f;
    if (p->m1C != 0)
        t0 = Unk8030EBC8_Secs(p->m1C->m4C);
    if (st == 3 && p->m3C == 0)
        st = 2;
    p->m24 = st;
    p->m64 = 0.0f;
    p->m2C = -1.0f;
    if (fl != 0)
        p->m34 = 0.167f;
    else
        p->m34 = 0.00167f;
    switch (p->m24) {
    case 1:
        if (fl != 0) {
            p->m28 = t0 + 5.0f;
            p->m2C = p->m28 + 20.0f;
        } else {
            p->m28 = t0 + 300.0f + (float)(rand() % 600);
            p->m2C = p->m28 + 180.0f + (float)(rand() % 120);
        }
        p->m30 = 0.1f;
        break;
    case 2:
        if (fl != 0)
            p->m28 = t0 + 5.0f;
        else
            p->m28 = t0 + (float)(rand() % 600);
        p->m2C = p->m28 + 300.0f + (float)(rand() % 300);
        p->m30 = 0.15f;
        goto tail;
    case 3:
        if (fl != 0)
            p->m28 = t0 + 5.0f;
        else
            p->m28 = t0 + (float)(rand() % 600);
        p->m30 = 1.0f;
        break;
    case 0:
    default:
        p->m24 = 0;
        p->m28 = -1.0f;
        p->m30 = 0.0f;
        break;
    }
tail:
    p->m30 = p->m30 * 3.0f;
    p->m30 = p->m30 <? 1.0f;
    return p->m24;
}

extern "C" void fn_8030EF64(Unk8030EF64_Obj* p, float a, float b)
{
    if (!(b <= 0.0f)) {
        int changed = 0;
        if ((!(a <= p->m28) && a < p->m2C) || !(p->m2C >= 0.0f)) {
            if (!(p->m64 >= p->m30)) {
                changed = 1;
                float t = b * p->m34;
                p->m64 = p->m64 + t;
            }
        } else if (!(p->m2C <= 0.0f)) {
            if (!(a <= p->m2C)) {
                if (!(p->m64 <= 0.0f)) {
                    changed = 1;
                    float t = b * p->m34;
                    p->m64 = p->m64 - t;
                    if (!(p->m64 > 0.0f)) {
                        p->m64 = 0.0f;
                        if (p->m24 == 1)
                            fn_8030EBC8((Unk8030EBC8_Obj*)p, 1, 0);
                    }
                }
            }
        }
        p->m64 = !(p->m64 > 1.0f) ? (p->m64 >? 0.0f) : 1.0f;
        if (changed) {
            fn_8030FA9C((Unk8030C5D8_Em*)p);
            fn_8030FB5C((Unk8030C5D8_Em*)p);
        }
    }
}

extern "C" void fn_8030F0C8(Unk8030F0C8_Obj* p, float a, float b)
{
    int s = p->m3C;
    Unk8030F0C8_Own* o = p->m1C;
    if (s == 1 && !(p->m64 <= 0.6f) && p->m38 == 0x3E) {
        void* h = o->m730;
        p->m40 = *(Unk8030C5D8_F3*)(fn_8031C1E4(h, 0x60)->m120 + 0x60);
        fn_8031C328(h, 0x60);
        p->m3C = 2;
        return;
    }
    if (s == 2) {
        void* h = o->m730;
        Unk8030C5D8_F3 pos = *(Unk8030C5D8_F3*)(fn_8031C1E4(h, 0x60)->m120 + 0x60);
        Unk8030C5D8_F3 d2 = Unk8030F0C8_Sub(pos, p->m40);
        p->m4C = d2;
        int found = 0;
        if (fn_8031C2A4(h, 0x60, &found, a) == 0 && found != 0)
            p->m3C = 3;
    } else if (s == 3) {
        if (!(p->mB4 >= p->m30)) {
            if (!(b <= 0.0f)) {
                float t = b * p->m34;
                p->mB4 = p->mB4 + t;
                p->mB4 = p->mB4 <? 1.0f;
            }
        }
    }
}

extern "C" int fn_8030F598(Unk8030F598_Obj* p)
{
    float now = fn_801A0744(p->m1C);
    float dt = p->mC0 < 0.0f ? 0.0f : now - p->mC0;
    p->mC0 = now;
    if (!(dt >= -16.0f))
        dt = 0.0f;
    int target = (int)((float)p->m60 * p->m64);
    int changed = 0;
    if (dt != 0.0f) {
        changed = 1;
        OnWindowClose((char*)p + 0x114);
        fn_8030EF64((Unk8030EF64_Obj*)p, now, dt);
        fn_8030F0C8((Unk8030F0C8_Obj*)p, now, dt);
        fn_8030F2C4(p, target, dt);
    }
    if (target > p->m80) {
        if (!changed) {
            OnWindowClose((char*)p + 0x114);
            changed = 1;
        }
        int c = p->m80;
        int lim = c + 0x32;
        if (lim > target)
            lim = target;
        for (int i = c; i < lim; i++)
            fn_8030FD50((Unk8030FD50_Obj*)p, i);
        p->m80 = lim;
    } else if (target < p->m80) {
        int v = p->m80 - 0x32;
        if (v < target)
            v = target;
        p->m80 = v;
    }
    if (changed) {
        fn_803E6B74((char*)p + 0x114);
        DCStoreRangeNoSync(p->mB8, p->m80 * 0x18);
        DCStoreRangeNoSync(p->mBC, p->m80 * 8);
    }
    return p->m80 != 0 ? 1 : 0;
}

extern "C" void fn_8030F768(Unk8030F768_Obj* p)
{
    Unk8030F768_C* cc = ((Unk8030F768_O*)p->m1C)->m54C;
    if (lbl_8061FDCC != 0 && cc->m54 == 3 && fn_8030F598((Unk8030F598_Obj*)p) != 0 && fn_801A04A8(p->m1C, 5) == 0) {
        Unk8030F768_Mtx tbl = lbl_8060E938;
        int k = 0;
        int off = 0;
        Unk8030C5D8_F3 pos;
        pos.x = p->m9C * 0.5f;
        pos.y = p->mA0 * 0.5f;
        pos.z = p->mA4 * 0.5f;
        int n = p->m80 * 2;
        tbl.m[0] = p->m9C;
        tbl.m[5] = p->mA0;
        tbl.m[10] = p->mA4;
        do {
            if (k != 0 || p->m3C != 3) {
                char* b1 = (char*)p + 0x84;
                char* b2 = (char*)p + 0x88;
                char* b3 = (char*)p + 0x8C;
                tbl.m[12] = *(float*)(b1 + off) - pos.x;
                tbl.m[13] = *(float*)(b2 + off) - pos.y;
                tbl.m[14] = *(float*)(b3 + off) - pos.z;
                if (k == 0 && p->m3C == 2) {
                    tbl.m[12] += p->m4C;
                    tbl.m[14] += p->m54;
                }
                fn_803E6D00((char*)p + 0x114, &tbl);
                fn_803E6C28((char*)p + 0x114, n);
                p->m110 = 1;
            }
            if (k == 0) {
                if (p->m3C == 0)
                    break;
                n = (int)((float)n * p->mB4);
                n = n & -2;
                tbl.m[0] = tbl.m[0] * p->mA8;
                tbl.m[10] = tbl.m[10] * p->mB0;
                pos.x = pos.x * p->mA8;
                pos.z = pos.z * p->mB0;
                if (n <= 0)
                    break;
            }
            k++;
            off += 12;
        } while (k <= 1);
    }
}

void fn_8030F9DC(Unk8030C5D8_Em2* p)
{
    float a = 1500.0f;
    float b = -150.0f;
    float c = 500.0f;
    p->m84.x = a;
    p->m84.y = p->mA0 * 0.5f + b;
    p->m84.z = c;
    p->m90 = p->m84;
    p->m90.x = p->m9C * ((p->mA8 + 1.0f) * 0.5f) + p->m90.x;
    p->m90.z = p->mA4 * ((p->mB0 - 1.0f) * 0.25f) + p->m90.z;
}

void fn_8030FA9C(Unk8030C5D8_Em* p)
{
    for (int i = 0; i < 8; i++) {
        float v = p->m78 * ((float)(rand() % 1024) * 0.0009775171f) + p->m64;
        p->mC4[i] = p->m6C * v + p->m68 * (1.0f - v);
    }
}

void fn_8030FB5C(Unk8030C5D8_Em* p)
{
    for (int i = 0; i < 8; i++) {
        float v = p->m7C * ((float)(rand() % 1024) * 0.0009775171f) + p->m64;
        p->mE4[i] = p->m74 * v + p->m70 * (1.0f - v);
    }
}

inline Unk8030FC1C_V3 Unk8030FC1C_Cp(const Unk8030FC1C_Src* s)
{
    return s->v;
}

inline float Unk8030FC1C_R01()
{
    return (float)(rand() % 1024) * 0.0009775171f;
}

extern "C" void fn_8030FC1C(Unk8030FC1C_P* p, Unk8030FC1C_Src* src)
{
    float a = 0.5f;
    Unk8030FC1C_V3 t = Unk8030FC1C_Cp(src);
    p->m104.x = a * t.x + Unk8030FC1C_R01() * 0.03f;
    p->m104.y = -980.0f;
    p->m104.z = a * t.z + Unk8030FC1C_R01() * 0.03f;
    v3unit(&p->m104, &p->m104);
}

extern "C" void fn_8030FD50(Unk8030FD50_Obj* p, int i)
{
    int k = i * 2;
    int off = k * 12;
    float s = p->mE4[i & 7];
    char* b = (char*)p->mB8;
    int off2 = off + 12;
    Unk8030C5D8_F3* v0 = (Unk8030C5D8_F3*)(b + off);
    Unk8030C5D8_F3* v1 = (Unk8030C5D8_F3*)(b + off2);
    v0->x = (float)(rand() % 1024) * 0.0009775171f;
    v0->y = (float)(rand() % 1024) * 0.0009775171f;
    v0->z = (float)(rand() % 1024) * 0.0009775171f;
    v1->x = p->m104.x * s + v0->x;
    v1->y = p->m104.y * s + v0->y;
    v1->z = p->m104.z * s + v0->z;
    *(int*)((char*)p->mBC + k * 4) = p->m58;
    ((Unk8030FD50_Pr*)((char*)p->mBC + k * 4))->m4 = p->m5C;
}

/* near-miss: only the scheduling of the first word store of the 16-byte copy into
 * mE84 differs (stw before the third lwz in retail). */
extern "C" void* fn_8030FECC(void* vp)
{
    Unk8030FECC_Obj* p = (Unk8030FECC_Obj*)vp;
    new (p) Unk8037B3E8(0xABA1DAEFu);
    Unk8030FECC_B* row = &p->g[0][0];
    int k = 3;
    p->m0 = lbl_806657C8;
    fn_8035F788((U*)((char*)p + 4));
    fn_803E47E8(&p->mA4);
    do {
        Unk8030FECC_B* e = row;
        int j = 1;
        do {
            fn_803E47E8(e);
            e++;
        } while (j-- != 0);
        row += 2;
    } while (k-- != 0);
    Unk8030FECC_B* h = p->h;
    int m = 3;
    do {
        fn_803E47E8(h);
        h++;
    } while (m-- != 0);
    p->mE84 = lbl_806055E8;
    p->m10E0 = 0;
    if (lbl_8061FE1C == 0)
        lbl_8061FE1C = 0x18556765;
    fn_8040074C((char*)p + 0x1A4, 0x10);
    fn_8040074C((char*)p + 0x1E4, 0x10);
    fn_8040074C((char*)p + 0x224, 0x10);
    fn_8040074C((char*)p + 0x1B4, 0x30);
    fn_8040074C((char*)p + 0x1F4, 0x30);
    fn_8040074C((char*)p + 0x234, 0x30);
    fn_8040074C((char*)p + 0x264, 0x20);
    fn_8040074C((char*)p + 0x284, 0x140);
    fn_8040074C((char*)p + 0x3C4, 0x40);
    fn_8040074C((char*)p + 0x404, 0x100);
    fn_8040074C((char*)p + 0x504, 0x180);
    fn_8040074C((char*)p + 0x684, 0x80);
    for (unsigned i = 0; i <= 3; i++)
        fn_803E4818(&p->h[i]);
    return p;
}
