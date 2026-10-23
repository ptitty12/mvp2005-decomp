/*
 * Game code, .text 0x80310934-0x80313C14 (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_802F8324.h"

extern char lbl_806657A8[];
extern char lbl_80665748[];
extern char lbl_806656E8[];
extern const char lbl_805E9474[];
extern const char lbl_805E9478[];
extern const char lbl_805E9484[];
extern unsigned int lbl_8061FE88;
extern unsigned int lbl_8061FE1C;
extern int lbl_8061FE20;
extern int lbl_8061FE84;
extern int lbl_8061FF74;
extern void* lbl_8061E978;
extern int lbl_8061F94C;
extern char lbl_8068E1C0[];

struct Unk80310934_At0 {
    const void* m0;
};
struct Unk80310934_At1C {
    const void* m0;
    char pad4[0x1C - 4];
    int m1C;
    char pad20[0x20 - 0x20];
};
struct Unk80310934_V3 {
    int a;
    int b;
    int c;
};
struct Unk80310934_F3 {
    float x;
    float y;
    float z;
};
struct Unk80310934_Cp {
    char pad0[0x20];
    Unk80310934_V3 m20;
    char pad2C[0x3C - 0x2C];
    Unk80310934_V3 m3C;
};
struct Unk80310934_I0 {
    int m0;
};
struct Unk80310934_Z {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    int m18;
};
struct Unk80310934_O1C {
    char pad0[0x1C];
    int m1C;
};
struct Unk80310934_Hd {
    char pad0[4];
    int m4;
};
struct Unk80310934_P60 {
    char pad0[0x38];
    Unk80310934_Hd* m38;
    char pad3C[0x60 - 0x3C];
    int m60;
};
struct Unk80310934_P10 {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
};
struct Unk80310934_X24 {
    char pad0[0x24];
    void* m24;
};
struct Unk80310934_P38 {
    char pad0[0x38];
    Unk80310934_X24* m38;
};
struct Unk80310934_P1F8 {
    char pad0[0x1F8];
    int m1F8;
};

struct Unk80310934_Bx {
    char pad0[0x10];
    int m10;
    int m14;
    char pad18[4];
    int m1C;
    int m20;
};
class Unk80310934_ScV {
public:
    virtual int s1();
    virtual int s2();
    virtual int s3();
    virtual int s4();
    virtual ~Unk80310934_ScV();
};
class Unk80310934_Sc : public Unk80310934_ScV {
public:
    char pad4[0x38 - 4];
    Unk80310934_Hd* m38;
};
extern Unk80310934_Sc* lbl_8061FEDC;
extern char lbl_80634B54[];
extern char lbl_80634AA0[];

struct lbl_Node;
class lbl_Owner {
public:
    char pad0[0x8];
    void* m8;
    void fn_803DA098(lbl_Node* node);
    lbl_Node* fn_803DA038();
};
struct Unk803D7ED4Item;
class Unk803D7ED4Pool {
public:
    char pad0[0xC];
    Unk803D7ED4Item* mC;
    void fn_803D803C(Unk803D7ED4Item* item);
    lbl_Owner* fn_803D7F74();
    void fn_803D7FD0(Unk803D7ED4Item* item);
    Unk803D7ED4Item* fn_803D7F0C();
};
struct Unk80310934_Sd {
    lbl_Node* m0;
    lbl_Owner* m4;
    void* m8;
    void* mC;
    void* m10;
    void* m14;
    void* m18;
};
struct Unk80310934_Big {
    int w[40];
};
int fn_803D81FC();
Unk80310934_Big* fn_80311508(void* p, int a, int b, int c, int d);

struct Unk80310934_Mx {
    char pad0[4];
    float v[4][30];
};
extern "C" double atof(const char* s);

extern const char lbl_805E9498[];
extern const char lbl_805E949C[];
extern const char lbl_805E94A0[];
extern const char lbl_805E94AC[];
extern const char lbl_805E94B4[];
extern const char lbl_805E94C0[];
extern const char lbl_805E94CC[];
extern const char lbl_805E94D0[];
extern const char lbl_805E94D8[];
extern const char lbl_805E94E4[];
extern const char lbl_805E94E8[];
extern const char lbl_805E94F4[];
extern const char lbl_805E9500[];
extern const char lbl_805E950C[];
extern const char lbl_805E9518[];
extern const char lbl_805E9530[];
extern const char lbl_805E9540[];
extern const char lbl_805E7AD0[];
struct Unk80310934_S134 {
    char pad0[0x14];
    int m14;
    int m18;
    int m1C;
    char pad20[0x38 - 0x20];
    Unk80310934_Hd* m38;
};
class Unk80310934_V0C {
public:
    char pad0[0xC];
    virtual int s1();
    virtual int s2();
    virtual int s3(float* a, float* b);
};
extern int lbl_8061FEAC;
struct Unk80310934_Tx {
    char m0;
    char pad1[0xB];
    unsigned mC;
};
struct Unk80310934_Sd2 {
    lbl_Node* m0;
    lbl_Owner* m4;
    Unk80310934_Tx* m8;
    Unk80310934_Tx* mC;
    void* m10;
    void* m14;
    int m18;
    unsigned m1C;
    unsigned m20;
    int m24;
};
extern void* (*lbl_8062C668)(int, int);
class Unk8031378C_DB {
public:
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual ~Unk8031378C_DB();
};
struct Unk8031378C_H {
    Unk803D7ED4Pool* m0;
    lbl_Owner* m4;
    char pad8[4];
    Unk8031378C_DB* mC;
};
struct Unk8031378C_Obj {
    const void* m0;
    char pad4[4];
    int m8;
    int mC;
    char pad10[0x38 - 0x10];
    Unk8031378C_H* m38;
    int m3C;
    char pad40[0x5C - 0x40];
    int m5C;
    char pad60[0x1D4 - 0x60];
    int m1D4;
    void* m1D8;
};
struct Unk80308874_AtD0;
struct U;
void fn_80308CFC(Unk80308874_AtD0* p, int c);
void fn_8035F7B8(U* p, int c);
void fn_80317B30();
extern char lbl_80665618[];
extern char lbl_805E9908[];
extern lbl_Node* lbl_8061FED8;
extern void* lbl_8061FED4;
struct Unk803109FC_Obj {
    char pad0[0x1B4];
    Unk80310934_F3 a[4];
    char pad1E4[0x10];
    Unk80310934_F3 b[4];
    char pad224[0x10];
    Unk80310934_F3 c[4];
    char pad264[0x504 - 0x264];
    Unk80310934_F3 d[4][8];
    char pad684[0xE84 - 0x684];
    Unk80310934_F3 e;
    char padE90[4];
    Unk80310934_V3 ia[4];
    Unk80310934_V3 ib[4];
    Unk80310934_V3 ic[4];
    char padF24[0xF54 - 0xF24];
    Unk80310934_V3 id[4][8];
    Unk80310934_V3 ie;
    char pad10E0[0x10FC - 0x10E0];
    Unk80310934_F3 of[3];
    Unk80310934_F3 og;
    Unk80310934_V3 jf[3];
    Unk80310934_V3 jg;
};
void fn_80314D08(int a, void* b);
void fn_80310964(Unk80310934_F3* out, Unk80310934_V3* in, float* s);
struct Unk80313C14_P38;
struct Unk8031392C_R {
    char pad0[0xC];
    int mC;
};
struct Unk8031392C_Obj {
    char pad0[0x38];
    Unk8031378C_H* m38;
    int m3C;
    char pad40[0x5C - 0x40];
    int m5C;
    char pad60[0x204 - 0x60];
    int m204;
    int m208;
};
Unk8031392C_R* fn_80314790(Unk80313C14_P38* p);
void fn_802B1CB4();
void fn_80317AB0(int k);
U* fn_8035F820(U* p);
void fn_803D81E4(void* (*v)(int, int));
void fn_803D81F0(int v);
int fn_8031263C(int a, int b);
void fn_8031268C(int a);
extern int lbl_8061FF44;
extern char lbl_8062C620[];
extern char lbl_8062C634[];
extern "C" {
void fn_80312DB0(void* p);
void* fn_803D7CDC(void* p);
void fn_803D7D20(void* p);
void InitInternal(int a, int b);
void fn_803E73F4(int v);
void* fn_8031DD54(void* p, int a, int b, int c, int d);
void fn_803EB520(void* p);
void FONT_installdriverextensions(void* p);
void FONT_init();
void* fn_802CB924(void* p, int a);
void* fn_803E4A50(void* p);
int fn_803E4C30(void* p);
char* fn_803E6E9C(int p);
int fn_803DBE44(void* p, int k);
int fn_803DBE5C(void* p, int k);
int fn_803DBE2C(void* p, int k);
int fn_803DBE38(void* p, int k);
int fn_803DBE78(void* p, int k);
void fn_803DBF00(void* p, int k);
void fn_803E6E10(int a, void* b);
void* fn_803E56F8(void* p);
int fn_803E5904(void* p);
void fn_80377408(void* p);
}
extern "C" {
void fn_803164A0__Fv(void* x);
void fn_803E4B40(void* p, int c);
void fn_803BD75C();
void FONT_restore();
void fn_803E9B6C(void* p);
void fn_803E57F0(void* p, int c);
void fn_803D7B40(void* p, int c);
}
extern "C" {
void VISetBlack(int b);
void VIFlush(void);
void VIWaitForRetrace(void);
void OSSetProgressiveMode(int a);
void fn_803D9DDC(int h, void* p, int n);
void fn_803D8918(int h);
void fn_803D9E70(int h, int a, int b, int c, int d, int e, int f, int g, int s);
void fn_803D9ECC(int h, int a, int b, int c, int d);
void fn_80314894(int a, int b, int c, int d, int e, int f);
void* fn_8030FECC(void* p);
void fn_8037B424(void* p, int c);
void* fn_80356798(void* p);
void fn_8040074C(void* p, int n);
void fn_802AD538(void* p, int k);
void fn_80311A70(Unk80310934_At0* p, int c);
void fn_80377408(void* p);
void fn_803DD73C(void* h, int k);
void fn_80359A40(void* p);
void fn_80312018(Unk80310934_I0* p);
int fn_8031228C(void* p);
void fn_80319CD4(void* p);
void fn_80319D20(void* p);
void fn_803D983C(int h, int k);
int fn_803D986C(int h, int* out);
int fn_803ED604(int b, int a, int k);
void MEM_fill(int a, unsigned int tag, int b);
int fn_803EE310(int a);
void MEM_free(int a);
int fn_803DBD84(void* p, int k);
void fn_803198B8(void* p, int k);
int fn_803120CC(void* p, int a, int b, int c, int d, int e);
int fn_803DD7C8(void* p, int k);
void fn_802B0C44(void* p, int k);
void fn_803D9804(int h, float x, float y);
void fn_803D9E54(int h, int v);
void fn_803D9DF0(int v);
void fn_803D982C(int h, int v);
int fn_803D9824(int h);
int OSGetResetCode();
int OSGetProgressiveMode();
int VIGetDTVStatus();
Unk80310934_Tx* SHAPE_create(int a, int b, int c, int d, int e, int f, int g, int h);
void* fn_803DD628(void* a, void* b);
int fn_803DD7C8(void* h, int v);
void fn_803E0810(lbl_Owner* o, void* a, int b);
void fn_803E0ACC(lbl_Node* n, float a, float b, float c, float d, float e, float f);
void fn_803E2208(lbl_Node* n, int* p);
}

void* fn_80311A28(void* p);
void fn_803123E0(Unk80310934_Bx* p, int a);
Unk80310934_O1C* fn_80311F2C();

void* fn_80310934()
{
    return fn_8030FECC(operator new(0x115C, lbl_806EF9B4));
}

void fn_80310964(Unk80310934_F3* out, Unk80310934_V3* in, float* s)
{
    out->x = (float)in->a * *s;
    out->y = (float)in->b * *s;
    out->z = (float)in->c * *s;
}

extern "C" void fn_803109FC(Unk803109FC_Obj* p)
{
    float s = 0.003921569f;
    int j;
    for (int i = 0; i < 4; i++) {
        fn_80310964(&p->a[i], &p->ia[i], &s);
        fn_80310964(&p->b[i], &p->ib[i], &s);
        fn_80310964(&p->c[i], &p->ic[i], &s);
        for (j = 0; j < 8; j++)
            fn_80310964(&p->d[i][j], &p->id[i][j], &s);
    }
    p->e.x = (float)p->ie.a * s;
    p->e.y = (float)p->ie.b * s;
    p->e.z = (float)p->ie.c * s;
    for (j = 0; j < 3; j++)
        fn_80310964(&p->of[j], &p->jf[j], &s);
    fn_80310964(&p->og, &p->jg, &s);
    if (lbl_8061E978)
        fn_802AD538(lbl_8061E978, (int)p);
    if (lbl_8061FEDC)
        fn_80314D08((int)lbl_8061FEDC, p);
    lbl_8061FE20 = 0;
}

void fn_80311464(void* p, int a, int b)
{
    *(Unk80310934_Big*)((char*)p + 4) = *fn_80311508(p, 0, 1, b, a);
}


struct Unk80311508_M {
    int w[16];
};
class Unk80311508_VB {
public:
    char pad0[0x98];
};
class Unk80311508_V : public Unk80311508_VB {
public:
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4(Unk80311508_M* out);
};
struct Unk80311508_Ob {
    char pad0[0xC];
    Unk80311508_V* mC;
};
extern "C" void fn_803E4450(Unk80310934_Big* a, Unk80310934_Big* b, int* c, float f);
extern "C" void fn_803E456C(Unk80310934_Big* a, Unk80311508_M* m);

Unk80310934_Big* fn_80311508(void* p, int a, int b, int c, int d)
{
    Unk80310934_Big* dst = (Unk80310934_Big*)((char*)p + 0xA4);
    *dst = *(Unk80310934_Big*)((char*)p + a * 0x140 + b * 0xA0 + 0x704);
    Unk80311508_M m1;
    ((Unk80311508_Ob*)c)->mC->s4(&m1);
    Unk80311508_M m2 = m1;
    fn_803E456C(dst, &m2);
    int neg = -1;
    fn_803E4450(dst, (Unk80310934_Big*)((char*)p + (a * 0xA0 + 0xC04)), &neg, 1.0f);
    return dst;
}

void* fn_80311680(void* p, int a, int b, int c, int d)
{
    if ((a != 0 || b != 1) && (c == 1 || c == 5))
        return fn_80311508(p, a, b, d, c);
    return (char*)p + 4;
}

extern "C" void* fn_803119C0()
{
    if (lbl_8061FE20 == 0) {
        Unk8037C7FC* m = fn_8037C7FC();
        int* q = &lbl_8061FE84;
        *q = m->fn_8037CE98(0xABA1DAEF, lbl_8061FE1C, 0);
        lbl_8061FE20 = 1;
    }
    return (void*)lbl_8061FE84;
}

void* fn_80311A28(void* p)
{
    fn_80356798(p);
    ((Unk80310934_At0*)p)->m0 = lbl_806657A8;
    fn_8040074C((char*)p + 4, 0x1E0);
    return p;
}

void fn_80311A70(Unk80310934_At0* p, int c)
{
    p->m0 = lbl_806657A8;
    if (c & 1)
        fn_80377408(p);
}

void fn_80311AA4(Unk80310934_Mx* p, char* a, int n)
{
    char* cur;
    int rem;
    char* tok;
    int len;
    cur = a;
    tok = 0;
    len = 0;
    rem = n;
    for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 0x1D; j++) {
            fn_80378E48(&cur, &rem, ',', &tok, &len);
            if (len <= 0)
                return;
            p->v[i][j] = (float)atof(tok);
        }
    }
}

void* fn_80311B4C(Unk80310934_At0* p)
{
    new (p) Unk8037B3E8(0xC22CAFD4u);
    p->m0 = lbl_80665748;
    fn_80311A28((char*)p + 4);
    return p;
}

void fn_80311B98(Unk8037B3E8* p)
{
    p->fn_8037B7E4(lbl_805E9474);
    p->fn_8037B728(lbl_805E9478, lbl_805E9484, (char*)p + 4, 0);
}

extern "C" void fn_80311BEC(Unk80310934_At0* p, int c)
{
    p->m0 = lbl_80665748;
    fn_80311A70((Unk80310934_At0*)((char*)p + 4), 2);
    fn_8037B424(p, c);
}

void* fn_80311C3C()
{
    return fn_80311B4C((Unk80310934_At0*)operator new(0x1E8, lbl_806EF9B4));
}

void fn_80311C6C()
{
    if (lbl_8061E978)
        fn_802AD538(lbl_8061E978, 0);
}

void* fn_80311CA0(Unk80310934_At1C* p)
{
    new (p) Unk8037B3E8(0x10B3208Eu);
    p->m0 = lbl_806656E8;
    p->m1C = 0;
    if (lbl_8061FE88 == 0)
        lbl_8061FE88 = 0xCFCDABCEu;
    return p;
}

extern "C" void fn_80311D08(Unk80310934_At0* p, int c)
{
    p->m0 = lbl_806656E8;
    fn_8037B424(p, c);
}

void fn_80311D38(Unk8037B3E8* p)
{
    p->fn_8037B7E4(lbl_805E9498);
    p->fn_8037B7E8(lbl_805E949C);
    p->fn_8037B458(lbl_805E94A0, (int*)((char*)p + 0x1C), 0, (const char*)0);
    p->fn_8037B6EC(lbl_805E94AC, (int*)((char*)p + 0x3C));
    p->fn_8037B50C(lbl_805E94B4, (float*)((char*)p + 0x2C), lbl_805E94C0, lbl_805E94CC);
    p->fn_8037B50C(lbl_805E94D0, (float*)((char*)p + 0x30), lbl_805E94C0, lbl_805E94CC);
    p->fn_8037B50C(lbl_805E94D8, (float*)((char*)p + 0x34), (const char*)0, (const char*)0);
    p->fn_8037B830(lbl_805E949C);
    p->fn_8037B7E8(lbl_805E94E4);
    p->fn_8037B458(lbl_805E94E8, (int*)((char*)p + 0x4), 0, (const char*)0);
    p->fn_8037B50C(lbl_805E94F4, (float*)((char*)p + 0x8), lbl_805E9500, (const char*)0);
    p->fn_8037B50C(lbl_805E950C, (float*)((char*)p + 0xC), lbl_805E9500, (const char*)0);
    p->fn_8037B458(lbl_805E9518, (int*)((char*)p + 0x10), 0, lbl_805E7AD0);
    p->fn_8037B50C(lbl_805E9530, (float*)((char*)p + 0x14), lbl_805E9500, (const char*)0);
    p->fn_8037B50C(lbl_805E9540, (float*)((char*)p + 0x18), lbl_805E9500, (const char*)0);
    p->fn_8037B830(lbl_805E94E4);
}

void* fn_80311EE0()
{
    return fn_80311CA0((Unk80310934_At1C*)operator new(0x48, lbl_806EF9B4));
}

void fn_80311F10(Unk80310934_Cp* p)
{
    p->m20.a = p->m3C.a;
    p->m20.b = p->m3C.b;
    p->m20.c = p->m3C.c;
}

Unk80310934_O1C* fn_80311F2C()
{
    return (Unk80310934_O1C*)fn_8037C7FC()->fn_8037CE98(0x10B3208E, lbl_8061FE88, 0);
}

void* fn_80311F64(Unk80310934_I0* p)
{
    void* q = (char*)p + 4;
    p->m0 = 0;
    fn_80359A40(q);
    return p;
}

void fn_80311FA0(Unk80310934_I0* p, int c)
{
    fn_80312018(p);
    if (c & 1)
        fn_80377408(p);
}

void fn_80311FE0(Unk80310934_I0* p)
{
    fn_80319CD4(p);
    p->m0 = lbl_8061FF74;
}

void fn_80312018(Unk80310934_I0* p)
{
    fn_80319D20(p);
    p->m0 = 0;
}

void fn_8031204C(Unk80310934_P10* p, int a, int b)
{
    p->m10 = a != 0 ? b : 0;
}

inline void ClearZ(Unk80310934_Z* z)
{
    z->m8 = 0;
    z->mC = 0;
    z->m10 = 0;
    z->m14 = 0;
    z->m18 = 0;
    z->m0 = 0;
    z->m4 = 0;
}

Unk80310934_Z* fn_80312064(Unk80310934_Z* p)
{
    ClearZ(p);
    return p;
}

void fn_8031208C(void* p, int c)
{
    fn_8031228C(p);
    if (c & 1)
        fn_80377408(p);
}

int fn_803120CC(void* p, int a, int b, int d, int e, int f)
{
    Unk80310934_Sd2* self = (Unk80310934_Sd2*)p;
    unsigned w = a;
    unsigned h = b;
    fn_8031228C(self);
    self->m1C = w;
    self->m20 = h;
    self->m24 = d;
    self->m8 = SHAPE_create(w, h, d, 0, 0, 0, 0, 0);
    self->m8->mC |= 0x20000000;
    self->m8->m0 = 0x16;
    self->m14 = fn_803DD628(lbl_8062C668(0x58, 0), self->m8);
    fn_803DD7C8(self->m14, 0);
    self->mC = SHAPE_create(w, h, d, 0, 0, 0, 0, 0);
    self->mC->m0 = 0x16;
    self->mC->mC |= 0x20000000;
    self->m10 = fn_803DD628(lbl_8062C668(0x58, 0), self->mC);
    fn_803DD7C8(self->m10, 0);
    self->m18 = 0;
    self->m4 = ((Unk803D7ED4Pool*)fn_803D81FC())->fn_803D7F74();
    fn_803E0810(self->m4, self->m14, self->m18);
    self->m0 = self->m4->fn_803DA038();
    fn_803E0ACC(self->m0, 0.0f, 0.0f, (float)w, (float)h, 0.0f, 1.0f);
    int sp8 = 0;
    fn_803E2208(self->m0, &sp8);
    return 1;
}

extern "C" int fn_8031228C(void* q)
{
    Unk80310934_Sd* p = (Unk80310934_Sd*)q;
    if (p->m4 != 0 && p->m0 != 0) {
        p->m4->fn_803DA098(p->m0);
        ((Unk803D7ED4Pool*)fn_803D81FC())->fn_803D803C((Unk803D7ED4Item*)p->m4);
        p->m4 = 0;
        p->m0 = 0;
    }
    if (p->m14 != 0) {
        fn_803DD73C(p->m14, 3);
        p->m14 = 0;
    }
    if (p->m18 != 0) {
        fn_803DD73C(p->m18, 3);
        p->m18 = 0;
    }
    if (p->m10 != 0) {
        fn_803DD73C(p->m10, 3);
        p->m10 = 0;
    }
    if (p->m8 != 0) {
        fn_80377408(p->m8);
        p->m8 = 0;
    }
    if (p->mC != 0) {
        fn_80377408(p->mC);
        p->mC = 0;
    }
    return 1;
}

void fn_80312370(Unk80310934_Bx* p)
{
    fn_803123E0(p, p->m14);
    fn_80314894(p->m10, 0, 0, p->m1C, p->m20, -1);
}

void fn_803123BC(Unk80310934_Bx* p)
{
    fn_803123E0(p, p->m10);
}

void fn_803123E0(Unk80310934_Bx* p, int a)
{
    fn_803D9E70(lbl_8061FEDC->m38->m4 + 0x10, 0, 0, p->m1C, p->m20, p->m1C, p->m20, 6, 0);
    fn_803D9ECC(lbl_8061FEDC->m38->m4 + 0x10, a, 0, 1, 0);
}

struct Unk8035F734 {
    char pad0[4];
    Unk8035F734();
};
struct Unk80308C74_O {
    char pad0[0xD4];
    Unk80308C74_O();
};
struct FxU {
    int m0;
    int m4;
    FxU();
};
struct Unk80312468_Obj {
    const void* m0;
    int m4;
    int m8;
    int mC;
    int m10;
    char pad14[0xC];
    int m20;
    int m24;
    int m28;
    int m2C;
    int m30;
    int m34;
    char pad38[4];
    int m3C;
    Unk80310934_I0 m40;
    char pad44[0x58 - 0x44];
    int m58;
    int m5C;
    int m60;
    int m64;
    char pad68[0x1D4 - 0x68];
    int m1D4;
    int m1D8;
    int m1DC;
    char pad1E0[0x10];
    int m1F0;
    int m1F4;
    int m1F8;
    int m1FC;
    int m200;
    int m204;
    int m208;
};
extern char lbl_80665618[];
extern char lbl_8068F0CC[];
extern "C" void fn_803E4818(void* p);
void* fn_80311F64(Unk80310934_I0* p);

extern "C" void* fn_80312468(Unk80312468_Obj* p)
{
    new (p) Unk8035F734;
    p->m4 = 0;
    p->m8 = 0;
    p->m3C = 0;
    p->m0 = lbl_80665618;
    fn_80311F64(&p->m40);
    p->m60 = 1;
    p->m64 = 1;
    new ((char*)p + 0x100) Unk80308C74_O;
    new ((char*)p + 0x1F0) FxU;
    p->m200 = 1;
    fn_803E4818(lbl_8068F0CC);
    p->mC = 0;
    p->m1F8 = 0;
    p->m1FC = 0;
    if (lbl_8061FEDC == 0)
        lbl_8061FEDC = (Unk80310934_Sc*)p;
    p->m5C = 0;
    p->m58 = 0;
    p->m1DC = 0;
    p->m1D4 = 0;
    p->m1D8 = 0;
    p->m10 = 0;
    p->m204 = 0;
    p->m208 = 0;
    p->m20 = 0;
    p->m24 = 0;
    p->m2C = 0x100;
    p->m30 = 0x100;
    p->m28 = 0;
    p->m34 = 0;
    return p;
}

void fn_80312544()
{
    if (lbl_8061FEDC) {
        delete lbl_8061FEDC;
        lbl_8061FEDC = 0;
    }
}

extern "C" void fn_8031259C(Unk80310934_P60* p)
{
    p->m60 = 0;
    fn_803D983C(p->m38->m4, 0);
}

extern "C" void fn_803125D0(Unk80310934_P60* p)
{
    p->m60 = 1;
    fn_803D983C(p->m38->m4, 1);
}

int fn_80312604(Unk80310934_P60* p)
{
    int v = 0;
    fn_803D986C(p->m38->m4, &v);
    return v;
}

int fn_8031263C(int a, int b)
{
    int r = fn_803ED604(b, a, 0);
    MEM_fill(r, 0xCC55CC55u, a);
    return r;
}

void fn_8031268C(int a)
{
    MEM_fill(a, 0xDD55DD55u, fn_803EE310(a));
    MEM_free(a);
}

int fn_803126D4()
{
    return fn_803DBD84(lbl_8068E1C0, 2);
}

void fn_80312DA0()
{
}

void fn_80312DA4()
{
}

void fn_80312DA8()
{
}

void fn_80312DAC()
{
}

void fn_803134A0(Unk80310934_S134* self, int x, int y, int a3, int a4, int c, int d)
{
    fn_803D9804(self->m38->m4, (float)x, (float)y);
    fn_803D9E54(self->m38->m4 + 0x10, 0x20000);
    fn_803D9DF0(0);
    if (lbl_8061FEAC != 0)
        fn_803D982C(self->m38->m4, 0);
    else
        fn_803D982C(self->m38->m4, c);
    ((void (*)(void*))fn_80312DA8)(self);
    if (OSGetResetCode() < 0 && OSGetProgressiveMode() != 0 && VIGetDTVStatus() == 1)
        fn_803D9DDC(lbl_8061FEDC->m38->m4 + 0x10, lbl_80634B54, 0x10);
    fn_803D8918(self->m38->m4);
    float sp8;
    float spC;
    ((Unk80310934_V0C*)self->m38->m4)->s3(&sp8, &spC);
    self->m14 = (int)sp8;
    self->m18 = (int)spC;
    self->m1C = fn_803D9824(self->m38->m4);
    ((void (*)(void*, int))fn_80312DAC)(self, 1);
}

extern "C" void fn_80313634(void* p)
{
    VISetBlack(1);
    VIFlush();
    VIWaitForRetrace();
    VIWaitForRetrace();
    OSSetProgressiveMode(1);
    fn_803D9DDC(((Unk80310934_P60*)p)->m38->m4 + 0x10, lbl_80634B54, 0x10);
    fn_803D8918(((Unk80310934_P60*)p)->m38->m4);
}

extern "C" void fn_803136A0(void* p)
{
    VISetBlack(1);
    VIFlush();
    VIWaitForRetrace();
    VIWaitForRetrace();
    OSSetProgressiveMode(0);
    fn_803D9DDC(((Unk80310934_P60*)p)->m38->m4 + 0x10, lbl_80634AA0, 0x10);
    fn_803D8918(((Unk80310934_P60*)p)->m38->m4);
}

void fn_8031370C()
{
    VISetBlack(0);
    VIFlush();
    VIWaitForRetrace();
}

void fn_80313738(void* a, int b)
{
    Unk80310934_O1C* o = fn_80311F2C();
    char* q = (char*)o + 0x20;
    int k = 0;
    if (o->m1C != 0 && b != 0)
        k = 1;
    fn_803198B8(q, k);
}

extern "C" void fn_8031378C(Unk8031378C_Obj* p, int c)
{
    p->m0 = lbl_80665618;
    fn_803164A0__Fv(p);
    if (p->m5C != 0) {
        fn_803E4B40((void*)p->m5C, 3);
        p->m5C = 0;
    }
    fn_803BD75C();
    FONT_restore();
    if (p->m1D4 != 0) {
        fn_803DD73C((void*)p->m1D4, 3);
        p->m1D4 = 0;
    }
    if (p->m1D8 != 0) {
        fn_80377408(p->m1D8);
        p->m1D8 = 0;
    }
    if (p->m38->mC != 0) {
        delete p->m38->mC;
        p->m38->mC = 0;
    }
    fn_80317B30();
    p->m3C = 0;
    fn_803E9B6C(lbl_805E9908);
    p->m38->m4->fn_803DA098(lbl_8061FED8);
    lbl_8061FED8 = 0;
    if (lbl_8061FED4 != 0) {
        fn_803E57F0(lbl_8061FED4, 3);
        lbl_8061FED4 = 0;
    }
    if (p->mC != 0) {
        fn_803E4B40((void*)p->mC, 3);
        p->mC = 0;
    }
    p->m38->m0->fn_803D7FD0((Unk803D7ED4Item*)p->m38->m4);
    if (p->m38->m0 != 0) {
        fn_803D7B40(p->m38->m0, 3);
        p->m38->m0 = 0;
    }
    if (p->m38 != 0) {
        fn_8035F7B8((U*)p->m38, 3);
        p->m38 = 0;
    }
    fn_80308CFC((Unk80308874_AtD0*)((char*)p + 0x100), 2);
    fn_80311FA0((Unk80310934_I0*)((char*)p + 0x40), 2);
    if (c & 1)
        fn_80377408(p);
}

extern "C" void fn_8031392C(Unk8031392C_Obj* p)
{
    void** a = &lbl_8061FED4;
    p->m38 = (Unk8031378C_H*)fn_8035F820((U*)operator new(0x88, 0));
    fn_803D81E4((void* (*)(int, int))fn_8031263C);
    fn_803D81F0((int)fn_8031268C);
    fn_803E73F4(-1);
    Unk8031378C_H* h = p->m38;
    h->m0 = (Unk803D7ED4Pool*)fn_803D7CDC(lbl_8062C668(0x14, 0));
    fn_803D7D20(p->m38->m0);
    p->m38->m4 = (lbl_Owner*)p->m38->m0->fn_803D7F0C();
    p->m204 = 0;
    p->m208 = 0;
    ((void (*)(void*, int, int, int, int, int))fn_803134A0)(p, 0x280, 0x1C0, 0x18, 0x20, 1);
    fn_80312DB0(p);
    InitInternal(0x43238, 0);
    h = p->m38;
    h->mC = (Unk8031378C_DB*)fn_8031DD54(operator new(0xB4, 0), 0, 0, 3, 0);
    fn_803EB520(lbl_8062C620);
    FONT_installdriverextensions(lbl_8062C634);
    FONT_init();
    fn_80317AB0(1);
    p->m3C = lbl_8061FF44;
    fn_802B1CB4();
    int v = (int)fn_802CB924(operator new(0x1E0, 0), 0);
    fn_80314790((Unk80313C14_P38*)p)->mC = v;
    fn_803E4C30((void*)(p->m5C = (int)fn_803E4A50(lbl_8062C668(0xC8, 0))));
    char* hh = fn_803E6E9C(p->m5C);
    fn_803DBE44(hh, 1);
    fn_803DBE5C(hh, 7);
    fn_803DBE2C(hh, 7);
    fn_803DBE38(hh, 1);
    fn_803DBE78(hh, 1);
    fn_803DBF00(hh, 1);
    fn_803E6E10(p->m5C, hh);
    *a = fn_803E56F8(lbl_8062C668(0xDC, 0));
    fn_803E5904(*a);
    lbl_8061FED8 = p->m38->m4->fn_803DA038();
}

void fn_80313B44(Unk80310934_P38* p)
{
    fn_803120CC((char*)p->m38 + 0x10, 0x80, 0x80, 0x20, 0, 0);
    void* x = p->m38->m24;
    if (x)
        fn_803DD7C8(x, 0);
}

void fn_80313BA4(Unk80310934_P38* p)
{
    fn_8031228C((char*)p->m38 + 0x10);
}

extern "C" void fn_80313BCC(Unk80310934_P1F8* p)
{
    fn_802B0C44(lbl_8061E978, 0x16);
    p->m1F8 = 0;
    lbl_8061F94C = 0;
}
