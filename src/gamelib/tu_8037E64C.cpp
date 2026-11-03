/*
 * EA gamelib (C:/mvp2004/libraries/gamelib: memory, datafile, animlib), built with
 * the game flags (cflags_game: -Os -G0 -ffloat-store),
 * .text 0x8037E64C-0x80382BB8 (code-only unit, 47 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */
#include "game/chooser.h"
#include "game/tu_80377048.h"



struct VOut {
    virtual ~VOut();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void s9();
    virtual void s10();
    virtual int s11(int a, int b);
};








struct ThisV11 {
    virtual ~ThisV11();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void s9();
    virtual void s10();
    virtual int s11();
};

struct VMc {
    char pad0[8];
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4(void* a, int b, int c, int d);
    virtual void v5();
    virtual void v6(int a);
    virtual void v7(int a);
    virtual void v8();
};

struct KnotBuf2 {
    char pad[12];
};

class Unk8037E73C : public Unk8037B3E8 {
public:
    Unk8037E73C();
    virtual void fn_vslot2();
    int m4;
    int m8;
    int mC;
    KnotBuf2 m10;
};

class Unk80362DD4 {
public:
    Unk80362DD4();
    char pad0[0x88];
};
class Unk80362D00 {
public:
    Unk80362D00();
    int m0;
    char pad4[0x344 - 4];
};
struct Q5 {
    float a;
    float b;
    float c;
    float d;
    float e;
    void Clear()
    {
        a = 0.0f;
        b = 0.0f;
        c = 0.0f;
        d = 0.0f;
        e = 0.0f;
    }
};

struct Unk80362E30;
void fn_80362E30(Unk80362E30* p);
class Unk803785E4 {
public:
    Unk803785E4();
    char pad0[8];
    virtual void vf();
};
class Unk8037E928 : public Unk803785E4 {
public:
    Unk8037E928(VMc* mc, char b, int x, int y);
    VMc* mC;
    Unk80362DD4 slots[3];
    Unk80362D00 m1A8;
    Unk80362D00 m4EC;
    char pad830[0x854 - 0x830];
    int m854;
    int m858;
    char pad85C[0x862 - 0x85C];
    char m862;
    char pad863[5];
    char m868[0x18];
    int m880;
    int m884;
};
struct Unk8037E9D0 {
    char pad0[8];
    const void* m8;
    VMc* mC;
    Unk80362DD4 slots[3];
    Unk80362D00 m1A8;
    Unk80362D00 m4EC;
    char pad830[4];
    int m834;
    int m838;
    int m83C;
    int m840;
    int m844;
    int m848;
    int m84C;
    int m850;
    int m854;
    int m858;
    int m85C;
    char m860;
    char m861;
    char m862;
    char pad863;
    int m864;
    Q5 q868;
    float m87C;
    int m880;
    int m884;
    float m888;
    int m88C;
};

extern "C" {
int fn_8037E078(KnotBuf2* p);
void fn_8037E0C0(KnotBuf2* p, int chrg);
void fn_8037893C(int a, int b, void* c, int d, int e);
void fn_8037E9D0(Unk8037E9D0* o);
void fn_8037EA7C(Unk8037E9D0* o, int v);
void fn_80380920(char* o);
}
void fn_8037B424(void* p, int c);
extern unsigned lbl_8069593C;
extern char lbl_80685020[];
extern int lbl_806242AC;
extern char lbl_80684F48[];


struct Unk8037E64C {
    char pad0[4];
    int m4;
    char* m8;
};

struct Unk8037E6F4 {
    float m0;
    char m4;
    int m8;
};

struct Unk8037E714 {
    int m0;
    int m4;
    int m8;
};

struct Unk8037ECA4 {
    char pad0[0x2C];
    int m2C;
    int m30;
    int m34;
    char pad38[0x84 - 0x38];
    int m84;
};

struct Unk8037FCE4_Inner {
    char pad0[8];
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual int v7();
};

struct Unk8037FCE4 {
    char padC[0xC];
    Unk8037FCE4_Inner* mC;
};

struct U {
    char pad0[0x4000];
};

extern "C" {
int fn_80384A48(void* p);
int fn_80384B70(int o, int a, int* b, int c);
int fn_803809BC(char* o, int a, char* slot, int b, int c, int d);
}

struct Ent68 {
    int m0;
    int m4;
    char pad8[8];
    float m10;
    float m14;
    char pad18[0x68 - 0x18];
};
extern "C" {
void fn_80381A5C(Unk8037E9D0* o);
int fn_803818BC(int a, int b, int c, float f1, float f2);
int fn_80380BB0(char* o, int a);
VOut* fn_80381E08(Unk8037E9D0* o, int a, int b);
}

struct Unk80383C9C_W;
struct VSO {
    virtual ~VSO();
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
    virtual VSO* s12();
    virtual void s13();
    virtual void s14();
    virtual void s15();
    virtual void s16();
    virtual void s17();
    virtual Unk80383C9C_W* s18();
};
struct VM884 {
    virtual ~VM884();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual int s8();
    virtual int s9();
};
extern "C" int fn_80383FB0(Unk80383C9C_W* o, int a);

extern "C" {
int fn_803814A4(Unk8037E9D0* o, int a, float f, int b);
int fn_80381BCC(Unk8037E9D0* o, int h);
int fn_80383674(int o);
}

extern "C" {
void fn_8037B4D0(Unk8037B3E8* self, const char* name, int* field, const char* range, int a);
void fn_8037BB0C(char* buf, int* cap);
}
extern const char lbl_806059D0[];
extern const char lbl_806059D8[];
extern const char lbl_806059E0[];
extern const char lbl_806059E8[];
extern const char lbl_806059EC[];
extern const char lbl_806059F0[];
extern const char lbl_806059FC[];
extern const char lbl_80605A04[];
extern const char lbl_80605A0C[];

struct GObj5 {
    virtual float s1(float a, float b, float c, float d, float e);
};
struct Pair2 {
    float m0;
    float m4;
};
extern GObj5* lbl_80624274;

struct Slot88 {
    char d[0x88];
};
class Unk80362DFC {
public:
    Unk80362DFC();
    int m0;
    int m4;
    int m8;
    int mC;
    char m10;
    char pad11[3];
    int m14;
    int m18;
    int m1C[3];
    int m28;
    int m2C;
    int m30;
    Slot88 m34;
};

extern "C" int sprintf(char*, const char*, ...);
struct Ent68X {
    int m0;
    struct Ent68Obj* m4;
    char pad8[8];
    float m10;
    float m14;
    int m18;
    float m1C;
    char pad20[0x68 - 0x20];
};
struct Ent68Obj {
    char pad0[4];
    float m4;
    char pad8[4];
    int mC;
};
struct VM884b {
    virtual ~VM884b();
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
    virtual void s23();
    virtual void s24();
    virtual void s25();
    virtual void s26();
    virtual void s27();
    virtual void s28();
    virtual void s29();
    virtual int s30();
};
extern const char lbl_80605A94[];
extern const char lbl_80605AB4[];
extern const char lbl_80605ABC[];

struct Slot5C {
    char d[0x5C];
};
struct SlotX {
    int m0;
    float m4;
    Slot5C m8;
    VCls* m64;
    int m68[3];
    int m74;
    char m78;
    char pad79[3];
    int m7C;
    int m80;
    int m84;
};
struct EntX {
    char pad0[8];
    int m8;
    int mC;
    char pad10[0x28 - 0x10];
    int m28;
    int m2C[3];
};
extern "C" {
int fn_80382B94(U* self, void* p);
void fn_80382CD0(Unk8037E9D0* o, int v);
}

extern "C" {
void fn_80381330(Unk8037E9D0* o, Unk80362DFC* s);
void fn_80381198(Unk8037E9D0* o, Unk80362DFC* s);
void fn_80380C14(Unk8037E9D0* o, int a, float f, int b, int c, int d);
}

struct PX {
    int m0;
    int m4;
    int m8;
    float mC;
    float m10;
    int m14;
    int m18;
};
extern "C" void fn_80383C78(PX* o, float* a, float* b, int* c, int* d);

struct Obj14 {
    char pad0[0x14];
    int m14;
};
struct VX13 {
    virtual ~VX13();
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
    virtual Obj14* s13(int key);
};
struct GObj1 {
    virtual float s1(int key);
};
struct SegEnt {
    int m0;
    int m4;
    float m8;
    float mC;
};
struct SegList {
    char pad0[0x10];
};
struct SegBase {
    int m0;
    int m4;
    SegEnt* m8;
};
extern GObj1* lbl_80624278;
extern int lbl_806242B0;

struct VM848 {
    virtual ~VM848();
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
    virtual int s13(int key);
    virtual int s14();
    virtual int s15();
};
extern "C" float fn_80381650(VX13** objs, int n, SegList* p, int key, int* out1, Obj14** out2, float a, float b);
extern "C" int fn_80381BCC(Unk8037E9D0* o, int a);

struct VB4 {
    virtual ~VB4();
    virtual void s2();
    virtual void s3();
    virtual int s4();
};
struct VY884 {
    virtual ~VY884();
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
    virtual void s23();
    virtual void s24();
    virtual void s25(int a, VB4* b, int c);
};
extern int lbl_8062427C;
extern int lbl_806242B4;

extern "C" int fn_8037E64C(Unk8037E64C* o, int key)
{
    for (int i = 0; i < o->m4; i++) {
        if (*(int*)(o->m8 + i * 16) == key)
            return i;
    }
    return -1;
}

extern "C" void fn_8037E68C(Unk8037E73C* o, int a, int b, float f1, float f2)
{
    fn_8037893C(a, ((ThisV11*)o)->s11(), o, 1, b);
}

extern "C" void fn_8037E6F4(Unk8037E6F4* o)
{
    o->m0 = 60.0f;
    o->m4 = 0;
    o->m8 = 0;
}

extern "C" void fn_8037E714(Unk8037E714* dst, const Unk8037E714* src)
{
    if (src != 0)
        *dst = *src;
}

extern "C" void fn_8037E738() {}

/* 8037E73C */
Unk8037E73C::Unk8037E73C() : Unk8037B3E8(lbl_8069593C)
{
    fn_8037E078(&m10);
}

/* 8037E788 */
void Unk8037E73C::fn_vslot2()
{
    fn_8037B4D0(this, lbl_806059D0, &m8, lbl_806059D8, 0);
    fn_8037B458(lbl_806059E0, &mC, lbl_806059E8, lbl_806059EC);
    fn_8037B50C(lbl_806059F0, (float*)&m4, lbl_806059FC, 0);
    char buf[0x7D0];
    int cap = 0x7D0;
    fn_8037BB0C(buf, &cap);
    fn_8037B728(lbl_80605A04, lbl_80605A0C, &m10, (int)buf);
}

extern "C" void fn_8037E844(Unk8037E73C* o, int chrg)
{
    *(const void**)o = lbl_80685020;
    fn_8037E0C0(&o->m10, 2);
    fn_8037B424(o, chrg);
}

extern "C" void* fn_8037E894()
{
    return new (lbl_806EF9B4) Unk8037E73C;
}

extern "C" void fn_8037E8C4(void* o)
{
    fn_8037C7FC()->fn_8037CE98(lbl_8069593C, (int)o, 0);
}

extern "C" void fn_8037E904(char* self)
{
    fn_8037E6F4((Unk8037E6F4*)(self + 4));
}

/* 8037E928 */
Unk8037E928::Unk8037E928(VMc* mc, char b, int x, int y) : mC(mc)
{
    m862 = b;
    fn_80362E30((Unk80362E30*)m868);
    m880 = x;
    m884 = y;
    m854 = 0;
    m858 = 0;
    fn_8037E9D0((Unk8037E9D0*)this);
}

extern "C" void fn_8037E9D0(Unk8037E9D0* o)
{
    o->m834 = 0;
    o->m838 = 0;
    o->m83C = 0;
    o->m840 = 0;
    o->m844 = 0;
    o->m848 = 0;
    o->m850 = 0;
    o->m84C = 0;
    fn_8037EA7C(o, 0);
    o->m861 = 0;
    o->m864 = -1;
    o->q868.Clear();
    o->m87C = 0.0f;
    o->m860 = -1;
    fn_80380920((char*)o);
    o->m888 = 0.0f;
    o->m88C = -1;
    o->m1A8.m0 = 0;
    o->m4EC.m0 = 0;
    o->m85C = 0;
}

extern "C" void fn_8037EA7C(Unk8037E9D0* o, int v)
{
    if (v != 0 && o->m854 == 0) {
        if (o->m862 == 0)
            o->mC->v8();
    } else if (o->m862 == 0 && v == 0) {
        o->mC->v6(0);
        o->mC->v7(0);
    }
    o->m854 = v;
}

extern "C" void fn_8037EB3C(Unk8037E9D0* o, int v)
{
    if (v == 0)
        goto t74;
    if (o->m858 != 0)
        goto t74;
    if (o->m862 == 0)
        goto done;
t74:
    if (o->m862 != 0)
        goto done;
    if (v != 0)
        goto done;
    if (o->m858 == 0)
        goto done;
    o->mC->v6(8);
    o->mC->v7(8);
done:
    o->m858 = v;
}

extern "C" void fn_8037EBE8(Unk8037E9D0* o)
{
    o->mC->v6(0);
    o->mC->v7(0);
    lbl_806242AC = 1;
    o->mC->v6(8);
    o->mC->v7(8);
    lbl_806242AC = 0;
}

extern "C" void fn_8037ECA4(Unk8037ECA4* o, int v)
{
    o->m84 = v;
    if (v == 0) {
        o->m2C = v;
        o->m34 = v;
        o->m30 = v;
    }
}

extern "C" void fn_8037ECC0(Unk8037E9D0* o, Pair2* out)
{
    float t;
    float ratio;
    if (o->q868.e != 0.0f)
        ratio = o->m87C / o->q868.e;
    else
        ratio = 1.0f;
    if (!(ratio > 0.0f))
        t = o->q868.a;
    if (!(ratio < 1.0f))
        t = o->q868.b;
    else
        t = lbl_80624274->s1(o->q868.a, o->q868.b, o->q868.c, o->q868.d, ratio);
    out->m4 = t;
    out->m0 = 1.0f - t;
    if (out->m0 == 0.0f || ratio == 1.0f)
        fn_8037ECA4((Unk8037ECA4*)((char*)o + (((o->m861 + 2) % 3) * 0x88 + 0x10)), 0);
}

extern "C" int fn_8037F2C4(Unk8037E9D0* o, char** out, int max)
{
    int i = 0;
    if (!(i < o->m4EC.m0))
        goto done;
    if (!(i < max))
        goto done;
    {
    int off = 0;
    char** p = out;
    do {
        char* base = (char*)o + 0x4F4;
        int busy = 0;
        Ent68Obj* ob = *(Ent68Obj**)(base + off);
        if (ob->mC != 0 || ((VM884b*)o->m884)->s30() != 0)
            busy = 1;
        char* b50C = (char*)o + 0x50C;
        char* dst = *p;
        int v = (int)(*(float*)(b50C + off) * 100.0f);
        const char* s = busy ? lbl_80605AB4 : lbl_80605ABC;
        char* b500 = (char*)o + 0x500;
        char* b4F0 = (char*)o + 0x4F0;
        sprintf(dst, lbl_80605A94, v, s, *(int*)(b4F0 + off), *(float*)(b500 + off) * 100.0f, (*(Ent68Obj**)(base + off))->m4);
        off += 0x68;
        p++;
        i++;
        if (!(i < o->m4EC.m0))
            break;
    } while (i < max);
    }
done:
    return i;
}

extern "C" int fn_8037FCE4(Unk8037FCE4* o)
{
    return o->mC->v7();
}

extern "C" void fn_8037FD1C(Unk8037E9D0* o)
{
    if (o->m862 == 0)
        o->mC->v6(8);
    if (o->m862 == 0 || ((VM884*)o->m884)->s8() == 0 || (((VM884*)o->m884)->s9() != 0 && o->m862 == 1))
        o->mC->v4((char*)&o->m4EC + 4, o->m4EC.m0, 10, o->m862);
}

extern "C" void fn_80380920(char* o)
{
    char* p = o + 0x10;
    for (int i = 3; i != 0; i--) {
        fn_8037ECA4((Unk8037ECA4*)p, 0);
        p += 0x88;
    }
}

/* near-miss: retail keeps the byte offset in r11 and forms o + 0x10 only when the slot is used (addi r9,r3,16; lwzx r10,r9,r11); 21 vs 22 insns */
extern "C" int fn_80380964(char* o)
{
    int r = 0;
    int off = (((signed char)o[0x861] + 2) % 3) * 0x88;
    if (*(int*)(o + off + 0x94) != 0) {
        char* b = o + 0x10;
        r = *(int*)(b + off);
    }
    return r;
}

extern "C" int fn_803809BC(char* ov, int key, char* slotv, int a, int b, int c)
{
    Unk8037E9D0* o = (Unk8037E9D0*)ov;
    SlotX* slot = (SlotX*)slotv;
    fn_80382CD0(o, (o->m861 + 1) % 3);
    EntX* ent = (EntX*)fn_80382B94((U*)o, (void*)key);
    ResBig res;
    VCls* v = (VCls*)fn_8037C7FC()->fn_8037D028(ent->m28);
    slot->m64 = v;
    if (v->s11(&res, a, b, c) == 0 || (*(unsigned*)&res & 0xF0000000) == 0)
        return 0;
    slot->m0 = key;
    slot->m4 = 0.0f;
    fn_8037ECA4((Unk8037ECA4*)slot, 0);
    slot->m8 = *(Slot5C*)&res;
    int cnt = ent->m8 >> 29;
    slot->m78 = cnt;
    int i = 0;
    do {
        if (i < cnt) {
            int h = fn_8037C7FC()->fn_8037D028(ent->m2C[i]);
            if (h != -1)
                slot->m68[i] = h;
        } else
            slot->m68[i] = 0;
        i++;
    } while (i <= 2);
    slot->m7C = ent->mC;
    slot->m80 = 0;
    return 1;
}

extern "C" int fn_80380BB0(char* o, int a)
{
    return fn_803809BC(o, a, o + 0x10 + (((signed char)o[0x861] + 1) % 3) * 0x88, *(int*)(o + 0x884), 0, 0);
}

extern "C" void fn_80381198(Unk8037E9D0* o, Unk80362DFC* s)
{
    s->m14 = o->m854;
    s->m0 = o->m844;
    s->m4 = o->m848;
    s->m8 = o->m84C;
    s->mC = o->m83C;
    s->m18 = o->m850;
    s->m34 = *(Slot88*)((char*)o + (((o->m861 + 1) % 3) * 0x88 + 0x10));
    char* a = (char*)o + 0x3C;
    char* b = (char*)o + 0x44;
    char* c = (char*)o + 0x40;
    s->m10 = o->m861;
    s->m28 = *(int*)(a + o->m861 * 0x88);
    s->m30 = *(int*)(b + o->m861 * 0x88);
    s->m2C = *(int*)(c + o->m861 * 0x88);
    s->m28 = *(int*)(a + o->m861 * 0x88);
    s->m30 = *(int*)(b + o->m861 * 0x88);
    s->m2C = *(int*)(c + o->m861 * 0x88);
    char* p = (char*)o + 0x10;
    for (int i = 0; i < 3; i++) {
        s->m1C[i] = ((Unk8037ECA4*)p)->m84;
        p += 0x88;
    }
}

extern "C" void fn_80381330(Unk8037E9D0* o, Unk80362DFC* s)
{
    fn_8037EA7C(o, s->m14);
    o->m844 = s->m0;
    o->m848 = s->m4;
    o->m84C = s->m8;
    o->m83C = s->mC;
    o->m850 = s->m18;
    o->m861 = s->m10;
    *(Slot88*)((char*)o + (((o->m861 + 1) % 3) * 0x88 + 0x10)) = s->m34;
    char* a = (char*)o + 0x3C;
    char* b = (char*)o + 0x44;
    char* c = (char*)o + 0x40;
    *(int*)(a + o->m861 * 0x88) = s->m28;
    *(int*)(b + o->m861 * 0x88) = s->m30;
    *(int*)(c + o->m861 * 0x88) = s->m2C;
    for (int i = 0; i < 3; i++)
        fn_8037ECA4((Unk8037ECA4*)((char*)o + 0x10 + i * 0x88), s->m1C[i]);
}

extern "C" int fn_803814A4(Unk8037E9D0* o, int a, float f, int b)
{
    int ret = 1;
    if (a != 0) {
        Unk80362DFC snap;
        fn_80381198(o, &snap);
        if (f == 0.0f) {
            char* p = (char*)o + 0x10;
            for (int i = 0; i < 3; i++) {
                fn_8037ECA4((Unk8037ECA4*)p, 0);
                p += 0x88;
            }
        }
        fn_8037EA7C(o, 1);
        o->m844 = 0;
        o->m848 = 0;
        o->m83C = 0;
        if (fn_80380BB0((char*)o, a) != 0) {
            fn_80380C14(o, a, 0.0f, 0, 0, 0);
            o->m83C = a;
            if (b != 0)
                o->m840 = a;
            else
                o->m840 = b;
            o->q868.a = 0.0f;
            o->q868.b = 1.0f;
            o->m87C = 0.0f;
            if (f != 0.0f) {
                o->q868.d = 0.0f;
                o->q868.c = 0.0f;
                o->q868.e = f * 0.016666668f;
            } else {
                fn_8037ECA4((Unk8037ECA4*)((char*)o + (((o->m861 + 2) % 3) * 0x88 + 0x10)), 0);
                o->q868.d = 0.0f;
                o->q868.c = 0.0f;
                o->q868.e = 0.0f;
            }
        } else {
            ret = 0;
            fn_80381330(o, &snap);
        }
    } else
        fn_8037E9D0(o);
    return ret;
}

extern "C" int fn_803818BC(int ov, int pv, int key, float a, float b)
{
    VX13* o = (VX13*)ov;
    SegList* p = (SegList*)pv;
    Obj14* r = o->s13(lbl_806242B0);
    if (r == 0 || r->m14 <= 0) {
        if (p != 0) {
            float sc;
            float lo;
            float hi;
            sc = -1.0f;
            lo = 0.0f;
            hi = 0.0f;
            SegBase* sb = (SegBase*)((char*)p + 0x10);
            int i = sb->m4;
            if (r == 0 && i-- != 0) {
                                do {
                    r = o->s13(sb->m8[i].m0);
                    if (r == 0 || r->m14 > 0) {
                        if (!(sc >= 0.0f)) {
                            sc = lbl_80624278->s1(key);
                            lo = a * sc;
                            hi = b * sc;
                        }
                        float e8 = sb->m8[i].m8;
                        if (e8 >= hi)
                            r = 0;
                        else {
                            float sum = e8 + sb->m8[i].mC;
                            if (!(sum >= lo))
                                r = 0;
                        }
                    } else
                        r = 0;
                    if (r != 0)
                        break;
                } while (i-- != 0);
            }
        }
    }
    return (int)r;
}

extern "C" void fn_80381A5C(Unk8037E9D0* o)
{
    char* base = (char*)o + 0x1C;
    VSO* c = *(VSO**)(base + o->m861 * 0x88);
    Unk80383C9C_W* t = c->s12()->s18();
    VSO** pp = (VSO**)(base + ((o->m861 + 1) % 3) * 0x88);
    VSO* n = *pp;
    o->m848 = fn_80383FB0(t, (int)n->s12());
    if (o->m848 == 0)
        o->m848 = o->m844;
}

extern "C" void fn_80381B38(Unk8037E9D0* o)
{
    o->m844 = (int)fn_80381E08(o, o->m834, o->m838);
    if (fn_80380BB0((char*)o, o->m838) != 0) {
        fn_80381A5C(o);
        if (o->m860 != -1) {
            Ent68* e = (Ent68*)((char*)o + 0x1AC) + o->m860;
            o->m84C = fn_803818BC(o->m848, e->m4, e->m0, e->m14, e->m10);
        }
        o->m850 = 1;
    }
}

extern "C" int fn_80381BCC(Unk8037E9D0* o, int a)
{
    int ret = 1;
    Unk80362DFC snap;
    fn_80381198(o, &snap);
    fn_8037EA7C(o, 1);
    o->m848 = 0;
    if (a != 0) {
        o->m844 = a;
    } else {
        int objs[4];
        Ent68* e = (Ent68*)((char*)o + 0x1AC) + o->m860;
        int n = fn_80384B70(o->m834, o->m83C, objs, 4);
        if (!(e->m14 > e->m10)) {
            fn_80381650((VX13**)objs, n, (SegList*)e->m4, e->m0, &o->m844, (Obj14**)&o->m84C, e->m14, e->m10);
        } else {
            if (!(e->m14 >= 1.0f))
                fn_80381650((VX13**)objs, n, (SegList*)e->m4, e->m0, &o->m844, (Obj14**)&o->m84C, e->m14, 1.0f);
            if (o->m84C == 0)
                fn_80381650((VX13**)objs, n, (SegList*)e->m4, e->m0, &o->m844, (Obj14**)&o->m84C, 0.0f, e->m10);
        }
    }
    if (o->m844 != 0) {
        VOut* v = (VOut*)o->m844;
        int h = v->s11(o->m83C, o->m884);
        if (fn_80380BB0((char*)o, h) == 0) {
            ret = 0;
        } else {
            fn_80381A5C(o);
            if (o->m850 == 0) {
                if (((VM848*)o->m848)->s15() != 0) {
                    Ent68* e = (Ent68*)((char*)o + 0x1AC) + o->m860;
                    o->m84C = fn_803818BC(o->m848, e->m4, e->m0, e->m14, e->m10);
                    if (o->m84C != 0)
                        o->m850 = 1;
                }
            }
        }
    } else {
        ret = fn_80380BB0((char*)o, o->m83C);
        if (ret != 0) {
            if (((Obj14*)o->m834)->m14 != 0 || fn_80381E08(o, o->m834, o->m838) != 0)
                o->m850 = 1;
        }
    }
    if (ret == 0)
        fn_80381330(o, &snap);
    return ret;
}

extern "C" VOut* fn_80381E08(Unk8037E9D0* o, int a, int b)
{
    if (a == 0)
        a = fn_80384A48((void*)b);
    VOut* out;
    if (fn_80384B70(a, b, (int*)&out, 1) != 0 && out != 0) {
        if (out->s11(o->m83C, o->m884) == b)
            return out;
    }
    return 0;
}

extern "C" int fn_80381E9C(Unk8037E9D0* o, int a, int b, int c)
{
    if (a != 0) {
        if (o->m838 == 0 || (o->m860 == -1 && o->m840 == 0))
            return fn_803814A4(o, a, 0.0f, 0);
        int old = o->m83C;
        if (old != a) {
            int p = o->m840;
            if (p == 0 || p == a) {
                o->m83C = a;
                int h = c ? fn_80383674(c) : 0;
                if (fn_80381BCC(o, h) == 0) {
                    o->m83C = old;
                    return 0;
                }
                if (b != 0)
                    o->m840 = a;
                else
                    o->m840 = b;
            } else
                return 0;
        }
    } else
        fn_8037E9D0(o);
    return 1;
}

extern "C" void fn_80381F94(Unk8037E9D0* o, PX* p)
{
    int a = o->m83C;
    if (a != 0 && o->m834 != 0) {
        float f = 0.0f;
        float g;
        int c;
        int cnt = 0;
        int h;
        VOut* v = (VOut*)o->m844;
        if (v != 0) {
            h = v->s11(a, o->m884);
            fn_80380BB0((char*)o, h);
        } else
            h = a;
        if (p != 0) {
            fn_80383C78(p, &f, &g, &c, &cnt);
            fn_80380C14(o, h, g, c, p->m4, p->m8);
        } else
            fn_80380C14(o, h, 0.0f, 0, 0, 0);
        o->q868.a = 0.0f;
        o->q868.b = 1.0f;
        if (p != 0) {
            if (cnt == 1) {
                o->q868.d = 1.0f;
                o->q868.c = 1.0f;
            } else {
                o->q868.d = 0.0f;
                o->q868.c = 0.0f;
            }
            o->q868.e = f * 0.016666668f;
            o->m87C = 0.0f;
        } else {
            o->q868.c = 0.0f;
            o->q868.e = 0.06666667f;
            o->q868.d = 0.0f;
            o->m87C = 0.0f;
        }
    }
}

extern "C" void fn_80382118(Unk8037E9D0* o, int a, VB4* b, int c)
{
    if (o->m850 == 0) {
        if (a == lbl_8062427C) {
            o->m84C = 0;
            VM848* t = (VM848*)o->m848;
            if (t != 0 && t->s14() != 0)
                o->m84C = ((VM848*)o->m848)->s13(lbl_806242B4);
            if (o->m84C != 0 || !((o->m844 == 0 || c != 0) && o->m838 == o->m83C)) {
                o->m850 = 1;
            } else {
                VOut* r = fn_80381E08(o, o->m834, o->m838);
                if (r != 0) {
                    fn_80381BCC(o, (int)r);
                    o->m850 = 1;
                    t = (VM848*)o->m848;
                    if (t != 0 && t->s14() != 0)
                        o->m84C = ((VM848*)o->m848)->s13(lbl_806242B4);
                    else
                        o->m84C = 0;
                }
            }
        } else {
            fn_8037893C((int)o, a, b, c, 0);
            VM848* t = (VM848*)o->m848;
            if (t != 0 && t->s14() != 0) {
                o->m84C = ((VM848*)o->m848)->s13(b->s4());
                if (o->m84C != 0)
                    o->m850 = 1;
            }
        }
        if (o->m850 != 0)
            ((VY884*)o->m884)->s25(a, b, c);
    }
}

extern "C" int fn_80382B94(U* self, void* p)
{
    return fn_80384A48(p);
}
