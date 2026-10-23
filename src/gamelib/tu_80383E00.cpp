/*
 * EA gamelib (C:/mvp2004/libraries/gamelib: memory, datafile, animlib), built with
 * the game flags (cflags_game: -Os -G0 -ffloat-store),
 * .text 0x80383E00-0x80387E28 (code-only unit, 44 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */
#include "game/chooser.h"
#include "game/tu_802CFED4.h"

#include "game/tu_80377048.h"

struct Unk80385470 {
    char pad0[4];
    VCls* m4;
    int m8;
};


struct Unk80362D4C;
void fn_80362D4C(Unk80362D4C* s, int chrg);
extern "C" {
void fn_80377488(void* p);
void fn_80389EBC(void* s);
}
struct Unk80387D58 {
    char pad0[8];
    const void* m8;
    const void* mC;
    char pad10[0x28C - 0x10];
    void* m28C;
    void* m290;
};
extern int* lbl_806242D4_arr[];
extern char lbl_80684A30[];
extern char lbl_80684A50[];





class Unk80362EAC_Base : public Unk8037B3E8 {
public:
    Unk80362EAC_Base(unsigned h) : Unk8037B3E8(h) {}
    virtual ~Unk80362EAC_Base() {}
};

class Unk80384858 : public Unk80362EAC_Base, public Unk80356798 {
public:
    Unk80384858();
    virtual ~Unk80384858();
    virtual void fn_vslot2();
    virtual void fn_vslot1(char* buf, int len);
    union {
        unsigned m8;
        struct {
            int cnt : 3;
            unsigned flag : 1;
            unsigned rest : 28;
        } b;
    };
    int mC;
    int m10;
    int m14;
    Unk80384F28 m18;
    int m28;
    unsigned m2C[3];
    int m38;
    int m3C;
};

extern const char lbl_80605CAC[];
extern const char lbl_80605CB8[];
extern const char lbl_80605CBC[];
extern const char lbl_80605CC8[];
extern const char lbl_80605CD4[];
extern const char lbl_80605CE0[];
extern const char lbl_80605BE8[];
extern const char lbl_80605BF0[];
extern const char lbl_806059E8[];
extern const char lbl_806059EC[];
extern const char* lbl_80624284;
extern unsigned lbl_80624280;


struct ThisV {
    virtual ~ThisV();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void s9();
    virtual void s10();
    virtual void s11();
    virtual void s12();
    virtual void s13();
    virtual void s14();
    virtual void s15();
    virtual void s16();
    virtual void s17();
    virtual void s18();
    virtual void s19();
    virtual void s20();
    virtual void s21();
    virtual void s22();
    virtual int s23();
    virtual char* s24();
};

struct GObj {
    virtual float s1(int x);
};

struct Unk80383C9C_W : ThisV {
    Unk80384F28 m4;
    int m14;
    int m18;
    union {
        unsigned m1C;
        struct {
            int top : 14;
            unsigned rest : 18;
        } b;
    };
};

struct Unk80362C6C {
    const void* m0;
    Unk80362C6C();
};
struct MapperV {
    virtual int v1(int key);
};
extern "C" {
int fn_8037E8C4(int x);
void fn_80377F50(SortArr* a, MapperV* map);
}
extern char lbl_80685000[];
extern const char lbl_80605A18[];
extern GObj* lbl_80624278;

struct SortArrDel {
    char pad0[0x14];
    virtual ~SortArrDel();
};
extern "C" {
void fn_80356798(void* self);
void fn_803780A0(SortArr* a, int idx);
void fn_80377DBC(SortArr* a, int* o1, int* o2, int idx);
void fn_80377DE4(SortArr* a, int v1, int v2, int idx);
int fn_80377E04(SortArr* a, int* out, int key);
SortArr* fn_80377944(void* mem, SortArr* src, int heap, const char* path, int line);
int fn_80385120(Unk80384F28* c, int key, int off);
void fn_80384F74(Unk80384F28* d, Unk80384F28* s);
int fn_80385220(Unk80384F28* c, int key, int* out, int max);
}
extern char lbl_80684EA0[];
extern const char lbl_80605CEC[];


struct VBase {
    virtual void v1();
};

struct Assign32 : VBase {
    int d[7];
};

struct Assign64 : VBase {
    int d[15];
};

struct V3I : VBase {
    int m4;
    int m8;
};

extern "C" {
int fn_80386188(void* link, void* q, int a, int b, float* f1, int c, int* d, void* e, void* g, int h, int i);
void GetStillPose(int a, int b, int c);
void fn_80383E74(struct Unk80383C9C_W* self);
}

struct Unk80387CCC {
    char pad0[0x14];
    int m14;
    char pad18[0x28C - 0x18];
    int m28C;
    int m290;
};
extern int lbl_806242D4[];

extern unsigned lbl_80699B0C;
extern unsigned lbl_80699B10;

class Unk803853F8 : public Unk8037B3E8 {
public:
    Unk803853F8();
    int m4;
    int m8;
};

inline float MinF(float x, float y)
{
    return x <? y;
}

inline float MaxF(float x, float y)
{
    return y >? x;
}

struct V3F {
    float x;
    float y;
    float z;
};
inline V3F ScaleV(const V3F& v, float s)
{
    V3F r;
    r.x = v.x * s;
    r.y = v.y * s;
    r.z = v.z * s;
    return r;
}

struct Unk80385B94 {
    int m0;
    int m4;
    int m8;
    V3F mC;
    float m18;
    float m1C;
    float m20;
    float m24;
    float m28;
};

extern "C" void fn_80383E00(Assign32* src, Assign32* dst)
{
    *dst = *src;
}

extern "C" void fn_80383E74(Unk80383C9C_W* self)
{
    if ((self->m1C & 0x20000) == 0) {
        Unk80384F28* c = &self->m4;
        c->fn_80385344(lbl_80605A18);
        self->m1C |= 0x20000;
        self->m18 = fn_8037E8C4(self->m18);
        SortArr* a = c->mC;
        if (a != 0) {
            Unk80362C6C o;
            o.m0 = lbl_80685000;
            fn_80377F50(a, (MapperV*)&o);
            o.m0 = lbl_80685000;
        }
    }
}

extern "C" int fn_80383F08(Unk80383C9C_W* self)
{
    if ((self->m1C & 0x20000) == 0)
        fn_80383E74(self);
    if ((self->m1C & 0xFFFC0000) == 0) {
        GObj* g = lbl_80624278;
        self->b.top = (int)g->s1(self->s23());
    }
    return self->b.top;
}

extern "C" int fn_80383FB0(Unk80383C9C_W* o, int a)
{
    fn_80383E74(o);
    return o->m4.fn_803851A0(a, 0);
}

struct Out24 {
    int m0;
    char* m4;
    char pad8[0x24 - 8];
    char* m24;
};

extern "C" int fn_80383FF0(ThisV* self, Out24* out, int flag)
{
    if (flag != 0) {
        out->m0 = self->s23();
        out->m4 = self->s24();
        out->m24 = out->m4 + 0x10;
        return 1;
    }
    return 0;
}

/* 80384858 */
Unk80384858::Unk80384858() : Unk80362EAC_Base(lbl_80699B0C)
{
    m8 &= 0x0FFFFFFF;
    m3C = lbl_80624280;
    mC = 0;
    m14 = 0;
    m10 = 0;
    m38 = 0;
}

/* 803848F0 */
void Unk80384858::fn_vslot2()
{
    fn_8037B458(lbl_80605CAC, &m10, 0, 0);
    fn_8037B728(lbl_80605BE8, lbl_80605BE8, &m18, 0);
    fn_8037B5FC(lbl_80605BF0, &m28);
    fn_8037B728(lbl_80605CB8, lbl_80605CBC, (Unk80356798*)this, 0);
    fn_8037B458(lbl_80605CC8, &mC, lbl_806059E8, lbl_806059EC);
    fn_8037B458(lbl_80605CD4, &m14, lbl_806059E8, lbl_806059EC);
    fn_8037B674(lbl_80605CE0, &m3C, lbl_80624284, 0);
}

/* 803849E0 */
Unk80384858::~Unk80384858()
{
}

extern "C" int fn_80384A48(void* o)
{
    return fn_8037C7FC()->fn_8037CE98(lbl_80699B0C, (int)o, 0);
}

extern "C" Unk80384858* fn_80384A88()
{
    return new (lbl_806EF9B4) Unk80384858;
}

extern "C" void fn_80384AB8(Assign64* src, Assign64* dst)
{
    *dst = *src;
}

struct Unk80384B3C {
    char pad0[4];
    int m4;
    int m8;
    struct Unk80384B3C_Inner* mC;
    char pad10[0x38 - 0x10];
    int m38;
};

struct Unk80384B3C_Inner {
    char pad0[4];
    int m4;
};

extern "C" {
void* fn_80384C88(int a, int b, void* c, int d, const void* e, int f);
void fn_8037B5FC(void* self, const char* name, int* field);
}
extern const char lbl_80605D34[];
extern int lbl_806242C0;
extern const char lbl_80605BF0[];
extern int lbl_806EF9B4;

extern "C" void fn_80384B3C(Unk80384B3C* o, int v)
{
    o->m38 = v;
}

extern "C" void fn_80384B44(char* a, char* b)
{
    fn_80384F74((Unk80384F28*)(b + 0x18), (Unk80384F28*)(a + 0x18));
}

extern "C" int fn_80384B70(Unk80384858* o, int a, int* b, int c)
{
    if ((o->m8 & 0x10000000) == 0) {
        o->m8 |= 0x10000000;
        o->m18.fn_80385344(o->fn_vslot3());
    }
    return fn_80385220(&o->m18, a, b, c);
}

/* 80384BF4 */
void Unk80384858::fn_vslot1(char* buf, int len)
{
    char* cursor = buf;
    int remaining = len;
    char* token;
    int tokenLen;
    while (fn_80378E48(&cursor, &remaining, ',', &token, &tokenLen), tokenLen != 0) {
        if (b.cnt <= 2)
            m2C[b.cnt++] = fn_80378568(token, tokenLen);
    }
}

/* 80384F28 */
Unk80384F28::Unk80384F28()
{
    mC = 0;
    m4 = 0;
    m8 = 0;
}

extern "C" void fn_80384F74(Unk80384F28* d, Unk80384F28* s)
{
    if (s->mC != 0) {
        if (d->mC != 0) {
            delete (SortArrDel*)d->mC;
            d->mC = 0;
        }
        d->mC = fn_80377944(operator new(0x18, lbl_806EF9B4 | 0x100), s->mC, 0x100, lbl_80605CEC, 0xAA);
    }
    d->m4 = s->m4;
    d->m8 = s->m8;
}

/* 8038501C */
Unk80384F28::~Unk80384F28()
{
    if (mC != 0) {
        int i = 0;
        if (i < mC->m4) {
            do {
                int k = i;
                i++;
                fn_803780A0(mC, k);
            } while (i < mC->m4);
        }
        if (mC != 0)
            delete (SortArrDel*)mC;
        mC = 0;
    }
}

extern "C" void fn_803850D0(Unk80384B3C* o, int a, int b)
{
    o->m4 = (int)fn_80384C88(a, b, &o->mC, 1, lbl_80605D34, lbl_806242C0);
}

extern "C" int fn_80385120(Unk80384F28* c, int key, int off)
{
    if (c->mC != 0) {
        int idx;
        int n = fn_80377E04(c->mC, &idx, key);
        if (n != 0 && n > off) {
            int a, b;
            idx += off;
            fn_80377DBC(c->mC, &a, &b, idx);
            return a;
        }
    }
    return c->m4;
}

/* 803851A0 */
int Unk80384F28::fn_803851A0(int key, int off)
{
    if (mC != 0) {
        int idx;
        int n = fn_80377E04(mC, &idx, key);
        if (n != 0 && n > off) {
            int a, b;
            idx += off;
            fn_80377DBC(mC, &a, &b, idx);
            return b;
        }
    }
    return m8;
}

extern "C" int fn_80385220(Unk80384F28* c, int key, int* out, int max)
{
    if (c->mC != 0) {
        int idx;
        int n = fn_80377E04(c->mC, &idx, key);
        if (max != 0 && n > max)
            n = max;
        for (int i = 0; i < n; i++) {
            int a, b;
            fn_80377DBC(c->mC, &a, &b, idx);
            out[i] = b;
            idx++;
        }
        return n;
    }
    *out = c->m8;
    int r = 1;
    if (c->m8 == 0)
        r = 0;
    return r;
}

extern "C" int fn_803852E4(Unk80384B3C* o)
{
    if (o->mC == 0)
        return 0;
    return o->mC->m4;
}

/* 803852FC */
int* Unk80384F28::fn_803852FC(int idx)
{
    if (mC != 0) {
        int a, b;
        fn_80377DBC(mC, &a, &b, idx);
        return (int*)b;
    }
    return (int*)m8;
}

/* 80385344 */
void Unk80384F28::fn_80385344(const char* name)
{
    Unk8037C7FC* m = fn_8037C7FC();
    if (mC != 0) {
        int i = mC->m4;
        while (i != 0) {
            i--;
            int a, b;
            fn_80377DBC(mC, &a, &b, i);
            b = m->fn_8037CDE0(name, a);
            if (b != 0)
                fn_80377DE4(mC, a, b, i);
        }
    }
    int r = m->fn_8037CDE0(name, m4);
    if (r != 0)
        m8 = r;
}

/* 803853F8 */
Unk803853F8::Unk803853F8() : Unk8037B3E8(lbl_80699B10)
{
    m4 = 0;
}

extern "C" void fn_80385444(Unk80384B3C* o)
{
    fn_8037B5FC(o, lbl_80605BF0, &o->m8);
}

/* near-miss: only the register of the zero constant differs (retail r3, ours r9) */
extern "C" int fn_80385470(Unk80385470* o, ResBig* out, int a, int kind, int p, int q, int r)
{
    if (o->m4 == 0)
        o->m4 = (VCls*)fn_8037C7FC()->fn_8037D028(o->m8);
    ResBig res;
    int n;
    if (kind == 3)
        n = o->m4->s13(&res, a);
    else if (kind == 0)
        n = o->m4->s11(&res, a, p, q);
    else if (kind == 1)
        n = o->m4->s14(&res, a);
    else
        n = o->m4->s12(&res, a, r);
    if (n)
        *out = res;
    return n;
}

extern "C" void* fn_80385620()
{
    return new (lbl_806EF9B4) Unk803853F8;
}

extern "C" void fn_80385650(V3I* a, V3I* b)
{
    *b = *a;
}

extern "C" void fn_80385674() {}

extern "C" float fn_80385ABC(float a, float b, float c, float d, float e)
{
    if (!(a >= d) && !(a + b <= c)) {
        float hi = MinF(a + b, d);
        float lo = MaxF(a, c);
        float x = (hi - lo) / b;
        x = (1.0f - x) + x / e;
        return x;
    }
    return 1.0f;
}

extern "C" void fn_80385B94(Unk80385B94* o, float a, float b, V3F* out1, float* out2)
{
    float sum = a + b;
    if (o->m0 != 0 && !(sum <= o->m18)) {
        float t = 0.0f;
        if (!(a >= o->m1C)) {
            float hi = MinF(sum, o->m1C);
            float lo = MaxF(a, o->m18);
            t = (hi - lo) / (o->m1C - o->m18);
        }
        V3F v = ScaleV(o->mC, t);
        *out1 = v;
    }
    if (o->m4 != 0 && !(sum <= o->m24)) {
        float t2 = 0.0f;
        if (!(a >= o->m28)) {
            float hi = MinF(sum, o->m28);
            float lo = MaxF(a, o->m24);
            t2 = (hi - lo) / (o->m28 - o->m24);
        }
        *out2 = t2 * o->m20;
    }
}

extern "C" int fn_80386110(void* link, void* q, int a, int b, float* out, int flags)
{
    return fn_80386188(link, q, a, b, out, 0, 0, 0, 0, 0, flags);
}

extern "C" int fn_8038614C(void* link, void* q, int a, int b, float* c, int d, int e, int f, int flags)
{
    return fn_80386188(link, q, a, b, c, 0, 0, (void*)d, (void*)e, f, flags);
}

extern "C" void fn_80387CCC(Unk80387CCC* o)
{
    GetStillPose(o->m14, lbl_806242D4[0], 0);
    GetStillPose(o->m14, lbl_806242D4[1], 0);
    GetStillPose(o->m14, o->m28C, 0);
    GetStillPose(o->m14, o->m290, 0);
}

extern "C" void fn_80387D3C()
{
    for (int i = 0; i < 15; i++) {
    }
    for (int i = 0; i < 15; i++) {
    }
}

extern "C" void fn_80387D58(Unk80387D58* s, int chrg)
{
    s->mC = lbl_80684A30;
    s->m8 = lbl_80684A50;
    if (s->m28C != 0) {
        fn_80377488(s->m28C);
        s->m28C = 0;
    }
    if (s->m290 != 0) {
        fn_80377488(s->m290);
        s->m290 = 0;
    }
    if (lbl_806242D4[0] != 0)
        fn_80377488((void*)lbl_806242D4[0]);
    if (lbl_806242D4[1] != 0)
        fn_80377488((void*)lbl_806242D4[1]);
    if (lbl_806242D4[2] != 0)
        fn_80377488((void*)lbl_806242D4[2]);
    lbl_806242D4[0] = 0;
    lbl_806242D4[1] = 0;
    lbl_806242D4[2] = 0;
    fn_80389EBC(s);
    fn_80362D4C((Unk80362D4C*)s, chrg);
}
