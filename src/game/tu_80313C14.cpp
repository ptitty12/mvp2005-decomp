/*
 * Game code, .text 0x80313C14-0x80317B88 (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_802F8324.h"

struct lbl_Node;
class lbl_Owner {
public:
    char pad0[0x8];
    void* m8;
    lbl_Node* fn_803DA038();
    void fn_803DA098(lbl_Node* node);
};

extern int lbl_8061FF44;
struct Unk80313C14_Q {
    int w[4];
};
extern Unk80313C14_Q lbl_8068F1C4;
struct Unk80313C14_F {
    char pad0[4];
    float m4;
    char pad8[8];
    float m10;
    char pad14[8];
    float m1C;
};
extern Unk80313C14_F lbl_8061FEB0;

struct Unk80313C14_P38C {
    char pad0[0xC];
    int mC;
};
struct Unk80313C14_P38 {
    char pad0[0x38];
    Unk80313C14_P38C* m38;
};
struct Unk80313C14_P38O {
    char pad0[0x38];
    lbl_Owner* m38;
};
struct Unk80313C14_Own4 {
    char pad0[4];
    lbl_Owner* m4;
};
struct Unk80313C14_P38H {
    char pad0[0x38];
    Unk80313C14_Own4* m38;
};
struct Unk80313C14_P58 {
    char pad0[0x58];
    int m58;
};
struct Unk80313C14_P4 {
    char pad0[4];
    void* m4;
    void* m8;
};
struct Unk80313C14_P200 {
    char pad0[0x200];
    int m200;
};
struct Unk80313C14_Cmp88 {
    char pad0[0x88];
    int m88;
    int m8C;
};
struct Unk80313C14_Cmp18 {
    char pad0[0x18];
    int m18;
    int m1C;
};
class Unk80313C14_Big {
public:
    char pad0[0x18];
    virtual ~Unk80313C14_Big();
};
extern Unk80313C14_Big* lbl_8061FF44_big;

struct Unk80313C14_Slot {
    char pad0[0x20];
    int m20;
};
struct Unk80313C14_Eng {
    char pad0[0x11E88];
    Unk80313C14_Slot* t[1];
    char pad11E8C[0x11EE0 - 0x11E8C];
    int m11EE0;
};
extern Unk80313C14_Eng* lbl_8061E978;
struct Unk80313C14_Fl {
    char pad0[0x18];
    int m18;
    char pad1C[0x1E0 - 0x1C];
    float m1E0;
    float m1E4;
    float m1E8;
    float m1EC;
};
class Unk80313C14_VC {
public:
    char pad0[0xC];
    virtual int s1();
    virtual int s2();
};
struct Unk80313C14_P38V {
    char pad0[4];
    Unk80313C14_VC* m4;
};
struct Unk80313C14_Sd {
    char pad0[0x38];
    Unk80313C14_P38V* m38;
};
struct Unk80313C14_Ld {
    char pad0[0xC];
    int mC;
    int m10;
};
struct Unk80313C14_P38I {
    char pad0[4];
    int m4;
};
struct Unk80313C14_SdI {
    char pad0[0x38];
    Unk80313C14_P38I* m38;
};
extern int lbl_8061FEAC;
extern int lbl_8061FF10;
extern int lbl_8061E9F4;
extern int lbl_8061D46C;
extern float lbl_8061D470;
extern void* (*lbl_8062C668)(int, int);

struct Unk80313C14_Sz {
    char pad0[0x14];
    int m14;
    int m18;
};
struct Unk80313C14_Tbl {
    char pad0[8];
    int m8;
    char padC[8];
    int m14;
};
struct Unk80313C14_Nd {
    char pad0[8];
    void* m8;
};
struct Unk80313C14_Fx3C {
    char pad0[0x3C];
    Unk802F8324_VT18* m3C;
};
struct Unk80313C14_Scr {
    char pad0[0x14];
    int m14;
    int m18;
    char pad1C[0xC];
    int m28;
    int m2C;
    int m30;
    char pad34[0x204 - 0x34];
    int m204;
};
extern Unk80313C14_Scr* lbl_8061FEDC;
extern int lbl_8061FF48;
extern float lbl_8061FF4C;
extern float lbl_8068E518;
extern int lbl_8068E51C;
struct Unk80313C14_TxO {
    char m0;
    char pad1[0xB];
    unsigned mC;
};
struct Unk80313C14_TxP {
    char pad0[4];
    char* m4;
};
struct Unk80313C14_Tx {
    char pad0[4];
    Unk80313C14_TxO* m4;
    void* m8;
    char padC[8];
    int m14;
    int m18;
    char pad1C[0x38 - 0x1C];
    Unk80313C14_TxP* m38;
};
struct Unk80313C14_Col {
    float m0;
    float m4;
    float m8;
    float mC;
};
extern const char lbl_805E9AFC[];
extern const char lbl_805E9B24[];
extern int lbl_8061FF38;
extern int lbl_8061FF3C;


extern "C" {
void FONT_drawtextfa(Unk80313C14_Slot* s, char* str, ...);
void fn_803EB3CC(Unk80313C14_Slot* s, int x, float a, float b);
void fn_803D982C(int h, int x);
void fn_8033E958(const char* fmt, ...);
int fn_803E4A50(void* o);
void fn_803E4C30(int t);
int fn_803E6E9C(int t);
int fn_803DBE38(int h, int k);
int fn_803DBE78(int h, int k);
void fn_803EB414(int h, int x, float* a, float* b, float* c, float* d);
int fn_803063F8(int a, int b, int c);
void fn_80314B4C(Unk80313C14_P58* p);
void fn_80306438();
void fn_80377408(void* p);
void fn_803DD73C(void* h, int k);
int fn_803D9E1C(int k);
int fn_80400C1C(void (*f)(), int a, int b);
void* fn_80317810(void* p, int a, int b, int c, int d);
int fn_803E9994(void* a, char* tbl, int* n, void* z);
void fn_803D6144(int a, int b);
int fn_803E9A54(void* a, const char* key, int* n, int* out);
int fn_803DD7C8(int h, int v);
char* SHAPE_longname(void* p);
int fn_803EC814(void* t, int i, char* out);
void fn_803D74A0(void* h, char* s, void* v);
unsigned int strlen(const char* s);
void fn_803E1DB0(void* n, float a, float b);
void fn_803E0F9C(void* n);
void fn_8031883C(void* o);
void fn_8031F04C(void* n, int k);
void fn_8031E110(void* n);
void fn_803E1E88(void* n);
void fn_80315C90(void* self);
void fn_80314368(void* c, unsigned short* s, int a, int b, float x, float y);
void fn_80317624(int* a, int* b, int c);
Unk80313C14_TxO* SHAPE_create(int a, int b, int c, int d, int e, int f, int g, int h);
int SHAPE_createsize(int a, int b, int c, int d, int e, int f, int g);
void DCStoreRange(void* a, int n);
void* fn_803DD628(void* a, void* b);
void fn_803D9E70(void* o, int a, int b, int c, int d, int e, int f, int g, int h);
void fn_803D9ECC(void* o, void* img, int a, int b, int c);
void* fn_800057E4();
void fn_8014D9EC(Unk80313C14_Col* c, float a, float b, float d, float e, float f);
int fn_8014DDAC(Unk80313C14_Col* c);
void fn_803EB3F0(Unk80313C14_Slot* s, unsigned short* str, float a, float b);
void fn_80322164(void* s, char* a, int n, char* b, int m);
int fn_802B9300(char* name, int a, int b, int* c);
void fn_802B961C(int h, int k);
int fn_803FC480(int h, char* s);
int unpacksizez();
void* fn_803ED604(const char* name, int sz, int k);
void unpackz(int h, void* b);
void DCStoreRange(void* a, int n);
int fn_80400DA4();
void* fn_803DD628(void* a, void* b);
void fn_803155B4(void* self, int tex, int a, int b, float f1, float f2, float f3, float f4);
void MEM_free(void* b);
void fn_803220F4(void* s, char* a, int n, char* b, int m);
void fn_803221D4(void* s, char* a, int n, char* b, int m);
}

struct Unk80313C14_Z {
    int m0;
    int m4;
    int m8;
};
extern char lbl_8060D190[];
extern const char lbl_8060D1CC[];
struct Unk802ACD88_Mgr;
void fn_802B8418(Unk802ACD88_Mgr* m);
struct Unk80313C14_Eng2 {
    char padA[0xAF4];
    int mAF4;
    char padB[0x18718 - 0xAF8];
    int m18718;
};
struct Unk80313C14_V4 {
    float m0;
    float m4;
    float m8;
    float mC;
};
extern Unk80313C14_V4 lbl_806E2124;

void fn_8000446C();

extern "C" int fn_80313C14(void* obj, int a, int* out)
{
    int h = 0;
    int n = 0;
    Unk80313C14_Z z;
    Unk80313C14_Z* pz = &z;
    z.m0 = 0;
    pz->m8 = 0;
    fn_803E9994(obj, lbl_8060D190, &n, pz);
    h = z.m8;
    fn_803D6144(h, a);
    n = 0;
    int v;
    int r = fn_803E9A54(obj, lbl_8060D1CC, &n, &v);
    if (r)
        *out = v;
    else
        *out = r;
    return h;
}

void fn_80313CBC(void* a, Unk80313C14_Tbl* t)
{
    int i = 0;
    int n = t->m8;
    if (i < n) {
        int* e = &t->m14;
        do {
            char buf[5];
            char* node = (char*)t + *e;
            e += 2;
            char* s = SHAPE_longname(node);
            if (s == 0 || strlen(s) > 4) {
                fn_803EC814(t, i, buf);
                char* p = buf + 3;
                while (p >= buf && *p == 0x20) {
                    *p = 0;
                    p--;
                }
                s = buf;
            }
            s[4] = 0;
            i++;
            fn_803D74A0(a, s, node);
        } while (i < n);
    }
}

extern "C" void fn_80313D9C(void* sp, int kind)
{
    Unk80313C14_Sd* self = (Unk80313C14_Sd*)sp;
    if (kind == 3) {
        if (lbl_8061D46C != 0)
            lbl_8061D470 = 0.1f;
        self->m38->m4->s1();
    }
}

extern "C" void fn_80313E00(void* sp, int kind)
{
    Unk80313C14_Sd* self = (Unk80313C14_Sd*)sp;
    if (kind == 3) {
        self->m38->m4->s2();
        int v = lbl_8061FEAC;
        if (v != lbl_8061FF10) {
            lbl_8061FF10 = v;
            fn_803D982C(((Unk80313C14_SdI*)self)->m38->m4, v ^ 1);
            fn_8033E958("%d Switching it
", lbl_8061E9F4);
        }
    }
}

extern "C" float fn_80313E94(void* sp, int x)
{
    Unk80313C14_Fl* self = (Unk80313C14_Fl*)sp;
    float a = 0.0f;
    float b = 0.0f;
    float c = 0.0f;
    float d = 0.0f;
    fn_803EB414(lbl_8061E978->m11EE0, x, &a, &b, &c, &d);
    return d / (float)self->m18;
}

extern "C" float fn_80313F28(void* sp, int x)
{
    Unk80313C14_Fl* self = (Unk80313C14_Fl*)sp;
    float a = 0.0f;
    float b = 0.0f;
    float c = 0.0f;
    float d = 0.0f;
    fn_803EB414(lbl_8061E978->m11EE0, x, &a, &b, &c, &d);
    return c / (float)((Unk80313C14_Sz*)self)->m14;
}

void fn_80313FBC(Unk80313C14_Ld* self)
{
    int t = fn_803E4A50(lbl_8062C668(0xC8, 0));
    self->mC = t;
    fn_803E4C30(t);
    int h = fn_803E6E9C(self->mC);
    fn_803DBE38(h, 1);
    fn_803DBE78(h, 1);
    self->m10 = 1;
}

struct Unk80313C14_V2 {
    float x;
    float y;
};
struct Unk80313C14_Vtx {
    float x;
    float y;
    float u;
    float v;
};
inline int Unk80313C14_ScrW()
{
    return lbl_8061FEDC->m28 != 0 ? lbl_8061FEDC->m2C : lbl_8061FEDC->m14;
}
extern "C" {
void fn_803E50BC(int h, int k);
void fn_803E54B8(int h, Unk80313C14_Vtx* v, int* c);
void fn_803E5270(int h);
void fn_803E6D28(int h);
}
/* near-miss 97.2%: 94/94 insns, 9 slots differ: retail loads self->m10 after the first V2 copy pair, ours hoists it above the stores; frame and registers match */
extern "C" void fn_80314034(Unk80313C14_Ld* self, Unk80313C14_V2* a, Unk80313C14_V2* b, Unk80313C14_Col* c)
{
    Unk80313C14_V2 pa = *a;
    Unk80313C14_V2 pb = *b;
    if (self->m10 == 0)
        fn_80313FBC(self);
    unsigned w = Unk80313C14_ScrW();
    unsigned h = lbl_8061FEDC->m28 != 0 ? lbl_8061FEDC->m30 : lbl_8061FEDC->m18;
    Unk80313C14_Vtx q[4];
    q[0].x = (float)w * pa.x;
    q[0].y = (float)h * pa.y;
    q[0].u = 0.0f;
    q[0].v = 1.0f;
    q[1].x = (float)w * pb.x;
    q[1].y = (float)h * pb.y;
    q[1].u = 0.0f;
    q[1].v = 1.0f;
    int col = fn_8014DDAC(c);
    fn_803E50BC(self->mC, 0xA8);
    int c1 = col;
    fn_803E54B8(self->mC, &q[0], &c1);
    int c2 = col;
    fn_803E54B8(self->mC, &q[1], &c2);
    fn_803E5270(self->mC);
    fn_803E6D28(self->mC);
}

/* near-miss 97.7%: 111/111 insns, 9 slots differ: same as fn_80314034 (the m10 load is hoisted above the V2 copies); frame and registers match */
extern "C" void fn_803141AC(Unk80313C14_Ld* self, Unk80313C14_V2* a, Unk80313C14_V2* b, Unk80313C14_V2* c2, Unk80313C14_Col* c)
{
    Unk80313C14_V2 pa = *a;
    Unk80313C14_V2 pb = *b;
    Unk80313C14_V2 pc = *c2;
    if (self->m10 == 0)
        fn_80313FBC(self);
    unsigned w = Unk80313C14_ScrW();
    unsigned h = lbl_8061FEDC->m28 != 0 ? lbl_8061FEDC->m30 : lbl_8061FEDC->m18;
    Unk80313C14_Vtx q[4];
    q[0].x = (float)w * pa.x;
    q[0].y = (float)h * pa.y;
    q[0].u = 0.0f;
    q[0].v = 1.0f;
    q[1].x = (float)w * pb.x;
    q[1].y = (float)h * pb.y;
    q[1].u = 0.0f;
    q[1].v = 1.0f;
    q[2].x = (float)w * pc.x;
    q[2].y = (float)h * pc.y;
    q[2].u = 0.0f;
    q[2].v = 1.0f;
    int col = fn_8014DDAC(c);
    fn_803E50BC(self->mC, 0x90);
    int c1 = col;
    fn_803E54B8(self->mC, &q[0], &c1);
    int cc2 = col;
    fn_803E54B8(self->mC, &q[1], &cc2);
    int c3 = col;
    fn_803E54B8(self->mC, &q[2], &c3);
    fn_803E5270(self->mC);
    fn_803E6D28(self->mC);
}

extern "C" void fn_803145D8(void* pp, int unused, int v, float a, float b, int idx)
{
    Unk80313C14_Sz* p = (Unk80313C14_Sz*)pp;
    if (lbl_8061E978 != 0) {
        if (lbl_8061E978->t[idx] != 0) {
            lbl_8061E978->t[idx]->m20 = v;
            fn_803EB3CC(lbl_8061E978->t[idx], unused, a * (float)p->m14, b * (float)p->m18);
        }
    }
}

/* near-miss: 51 vs 46 instructions; retail reloads the slot after forming the floats and keeps a junk copy of an unset register */
extern "C" void fn_80314688(void* o, int x, int y, char* str, int flag, int idx)
{
    if (lbl_8061E978 != 0) {
        if (lbl_8061E978->t[idx] != 0) {
            lbl_8061E978->t[idx]->m20 = flag == 0 ? 0x80B5AC95 : 0x80FFFFFF;
            FONT_drawtextfa(lbl_8061E978->t[idx], str, (float)x, (float)y);
        }
    }
}

extern "C" void fn_8031479C(void* g)
{
    ((Unk80313C14_P200*)g)->m200 = 1;
    Unk80313C14_Eng2* e = (Unk80313C14_Eng2*)lbl_8061E978;
    int h = e->m18718;
    if (e->mAF4 == 0) {
        lbl_806E2124.m0 = 1279.0f;
        lbl_806E2124.m4 = 447.0f;
        lbl_806E2124.m8 = 1.0f;
        lbl_806E2124.mC = 1.0f;
        fn_802B8418((Unk802ACD88_Mgr*)e);
    } else {
        lbl_806E2124.m0 = 1.0f;
        lbl_806E2124.m4 = 1.0f;
        lbl_806E2124.m8 = 1.0f;
        lbl_806E2124.mC = 1.0f;
        fn_802B8418((Unk802ACD88_Mgr*)e);
    }
    fn_803DD7C8(h, 0);
}

void fn_80314740(Unk80313C14_P38H* p)
{
    p->m38->m4->fn_803DA038();
}

void fn_80314768(Unk80313C14_P38H* p, lbl_Node* n)
{
    p->m38->m4->fn_803DA098(n);
}

int fn_80314790(Unk80313C14_P38* p)
{
    return p->m38->mC;
}

void fn_80314854(Unk80313C14_P58* p)
{
    fn_80314B4C(p);
    p->m58 = fn_803063F8(0x100, 0x100, 0x10);
}

void fn_80314B4C(Unk80313C14_P58* p)
{
    if (p->m58 != 0) {
        fn_80306438();
        p->m58 = 0;
    }
}

void fn_80314D08(int a, void* b)
{
    lbl_8068F1C4 = *(Unk80313C14_Q*)((char*)b + 0xE84);
}

void fn_80314D38(float v)
{
    lbl_8061FEB0.m4 = v;
    lbl_8061FEB0.m10 = v;
    lbl_8061FEB0.m1C = v;
}

void fn_80314D60()
{
}

void fn_803157B4(Unk80313C14_Tx* self)
{
    int w = self->m14;
    int h = self->m18;
    self->m4 = SHAPE_create(w, h, 0x20, 0, 0, 0x100, 0, 0);
    int sz = SHAPE_createsize(w, h, 0x20, 0xF, 0, 0, 0);
    if (self->m4 != 0) {
        DCStoreRange(self->m4, sz);
        self->m4->m0 = 0x16;
        self->m4->mC |= 0x20000000;
        void* o = lbl_8062C668(0x58, 0);
        self->m8 = fn_803DD628(o, self->m4);
        if (self->m8 != 0) {
            fn_803D9E70(self->m38->m4 + 0x10, 0, 0, self->m14, self->m18, w, h, 6, 0);
            fn_803D9ECC(self->m38->m4 + 0x10, self->m8, 0, 1, 0);
        }
    }
}

void fn_80315AD8(Unk80313C14_P4* p)
{
    if (p->m4) {
        fn_80377408(p->m4);
        p->m4 = 0;
    }
    if (p->m8) {
        fn_803DD73C(p->m8, 3);
        p->m8 = 0;
    }
}

void fn_80315B34(Unk80313C14_Fx3C* self, unsigned short* name, int flag)
{
    fn_80313D9C(self, 3);
    Unk80313C14_Nd* nd = (Unk80313C14_Nd*)fn_80314790((Unk80313C14_P38*)self);
    fn_803E1DB0(nd->m8, 0.0f, 200.0f);
    fn_803E0F9C(nd->m8);
    fn_8031883C(self->m3C);
    fn_8031F04C(nd, 0);
    fn_8031E110(nd);
    self->m3C->s12();
    if (flag != 0)
        fn_80315C90(self);
    const float a = 0.5f;
    const float b = 0.5f;
    fn_80314368(self, name, 1, 0x16, a, b);
    fn_803E1E88(nd->m8);
    fn_80313E00(self, 3);
}

void fn_80315C20(float* p, float* v, float a, float b, float c)
{
    *p = a + *v;
    if (!(*p <= b)) {
        *v = -*v;
        *p = b;
    }
    if (!(*p >= c)) {
        *v = -*v;
        *p = c;
    }
}

/* near-miss: 100/100 insns, 5 slots differ: retail keeps the fn_803FC480 result in r30 and the unpacksizez size in r29; ours swaps them (local-alloc priority 2 refs/10 insns vs 3 refs/14 insns); inline wrappers and declaration order did not change it */
void fn_80315FFC(char* self)
{
    char a[256];
    char b[256];
    fn_80322164(self + 0x1F0, a, 0x100, b, 0x100);
    int h1 = fn_802B9300(a, 0x500, 0xE, 0);
    int h2 = fn_803FC480(h1, b);
    int sz = unpacksizez();
    char* buf = (char*)fn_803ED604(lbl_805E9AFC, sz, 0x500);
    unpackz(h2, buf);
    fn_802B961C(h1, 0xE);
    DCStoreRange(buf, sz);
    fn_803D9E1C(0);
    lbl_8061FF38 = fn_80400DA4();
    int tex = (int)fn_803DD628(lbl_8062C668(0x58, 0), buf + *(int*)(buf + 0x14));
    fn_803155B4(lbl_8061FEDC, tex, 1, 1, 0.0f, 0.0f, 1.0f, 1.0f);
    fn_803155B4(lbl_8061FEDC, tex, 1, 1, 0.0f, 0.0f, 1.0f, 1.0f);
    fn_803155B4(lbl_8061FEDC, tex, 1, 1, 0.0f, 0.0f, 1.0f, 1.0f);
    fn_80313D9C(lbl_8061FEDC, 3);
    fn_80313E00(lbl_8061FEDC, 3);
    if (tex != 0)
        fn_803DD73C((void*)tex, 3);
    MEM_free(buf);
}

/* near-miss: 100/100 insns, 5 slots differ: retail keeps the fn_803FC480 result in r30 and the unpacksizez size in r29; ours swaps them (local-alloc priority 2 refs/10 insns vs 3 refs/14 insns); inline wrappers and declaration order did not change it */
extern "C" void fn_8031618C(char* self)
{
    char a[256];
    char b[256];
    fn_803220F4(self + 0x1F0, a, 0x100, b, 0x100);
    int h1 = fn_802B9300(a, 0x500, 0xE, 0);
    int h2 = fn_803FC480(h1, b);
    int sz = unpacksizez();
    char* buf = (char*)fn_803ED604(lbl_805E9AFC, sz, 0x500);
    unpackz(h2, buf);
    fn_802B961C(h1, 0xE);
    DCStoreRange(buf, sz);
    fn_803D9E1C(0);
    int tex = (int)fn_803DD628(lbl_8062C668(0x58, 0), buf + *(int*)(buf + 0x14));
    fn_803155B4(lbl_8061FEDC, tex, 1, 1, 0.0f, 0.0f, 1.0f, 1.0f);
    fn_803155B4(lbl_8061FEDC, tex, 1, 1, 0.0f, 0.0f, 1.0f, 1.0f);
    fn_803155B4(lbl_8061FEDC, tex, 1, 1, 0.0f, 0.0f, 1.0f, 1.0f);
    fn_80313D9C(lbl_8061FEDC, 3);
    fn_80313E00(lbl_8061FEDC, 3);
    if (tex != 0)
        fn_803DD73C((void*)tex, 3);
    MEM_free(buf);
}

/* near-miss: 100/100 insns, 5 slots differ: retail keeps the fn_803FC480 result in r30 and the unpacksizez size in r29; ours swaps them (local-alloc priority 2 refs/10 insns vs 3 refs/14 insns); inline wrappers and declaration order did not change it */
void fn_80316310(char* self)
{
    char a[256];
    char b[256];
    fn_803221D4(self + 0x1F0, a, 0x100, b, 0x100);
    int h1 = fn_802B9300(a, 0x500, 0xE, 0);
    int h2 = fn_803FC480(h1, b);
    int sz = unpacksizez();
    char* buf = (char*)fn_803ED604(lbl_805E9B24, sz, 0x500);
    unpackz(h2, buf);
    fn_802B961C(h1, 0xE);
    DCStoreRange(buf, sz);
    fn_803D9E1C(0);
    int tex = (int)fn_803DD628(lbl_8062C668(0x58, 0), buf + *(int*)(buf + 0x14));
    fn_803155B4(lbl_8061FEDC, tex, 1, 1, 0.0f, 0.0f, 1.0f, 1.0f);
    fn_803155B4(lbl_8061FEDC, tex, 1, 1, 0.0f, 0.0f, 1.0f, 1.0f);
    fn_803155B4(lbl_8061FEDC, tex, 1, 1, 0.0f, 0.0f, 1.0f, 1.0f);
    fn_80313D9C(lbl_8061FEDC, 3);
    fn_80313E00(lbl_8061FEDC, 3);
    if (tex != 0)
        fn_803DD73C((void*)tex, 3);
    MEM_free(buf);
    lbl_8061FF3C = fn_80400DA4();
}

int fn_803164A0()
{
    return fn_803D9E1C(1);
}

struct Unk80313C14_Pt {
    float m0;
    char pad4[4];
    float m8;
};
inline int Unk80313C14_Near(float px, float pz, float cx, float cz, float r)
{
    return (px - cx) * (px - cx) + (pz - cz) * (pz - cz) < r * r;
}
extern "C" int fn_803164C4(Unk80313C14_Pt* p)
{
    const float r = 120.0f;
    const float s = 1080.0f;
    if (Unk80313C14_Near(p->m0, p->m8, s, 0.0f, r))
        goto hit;
    if (Unk80313C14_Near(p->m0, p->m8, 0.0f, s, r))
        goto hit;
    if (Unk80313C14_Near(p->m0, p->m8, s, s, r))
        goto hit;
    return Unk80313C14_Near(p->m0, p->m8, 0.0f, 0.0f, r);
hit:
    return 1;
}

struct Unk80313C14_Fade {
    char pad0[0x1D4];
    int m1D4;
    int m1D8;
    int m1DC;
    float m1E0;
    float m1E4;
    float m1E8;
    float m1EC;
};
extern int lbl_8068E510;
extern int lbl_8068E514;
extern "C" {
int fn_80400E84();
int sprintf(char* buf, const char* fmt, ...);
void* memset(void* p, int c, unsigned n, ...);
}

/* Draws the loading-progress gradient (misc/textures/grad.gsh); progress is clamped to [0, 1].
   The first call stamps the time base (fn_80400DA4) that the debug line reports against. */
extern "C" void DrawLoadingProgress(Unk80313C14_Fade* self, int state, float progress)
{
    if (self->m1D8 == 0) {
        char name[0x40] = "";
        sprintf(name, "misc/textures/grad.%s", "gsh");
        self->m1D8 = fn_802B9300(name, 0x400, 0xE, 0);
        self->m1D4 = (int)fn_803DD628(lbl_8062C668(0x58, 0), (char*)self->m1D8 + *(int*)(self->m1D8 + 0x14));
    }
    if (self->m1DC == state) {
        if (lbl_8068E514 == 0) {
            lbl_8068E510 = fn_80400DA4();
            lbl_8068E514 = 1;
        }
        fn_8033E958("DrawLoadingProgress: %d, %.3f @ %.3f seconds
", state, progress,
                    (float)(fn_80400DA4() - lbl_8068E510) / (float)fn_80400E84());
        int tex = self->m1D4;
        if (!(progress <= 1.0f))
            progress = 1.0f;
        if (!(progress >= 0.0f))
            progress = 0.0f;
        fn_803155B4(lbl_8061FEDC, tex, 0, 0, self->m1E0, self->m1E4, self->m1E8 * progress, self->m1EC);
    }
}

void fn_803167F0(Unk80313C14_Fl* p, float a, float b, float c, float d)
{
    p->m1E0 = a;
    p->m1E4 = b;
    p->m1E8 = c;
    p->m1EC = d;
}

void fn_80316B88()
{
}

void fn_80316B8C()
{
}

int fn_80316B90()
{
    return fn_80400C1C(fn_8000446C, 1, 0);
}

void fn_80316BC0(Unk80313C14_P200* p)
{
    p->m200 = 1;
}

float fn_8031708C()
{
    return 0.0f;
}

void fn_80317098()
{
}

void fn_8031709C()
{
}

int fn_803170A0(Unk80313C14_Cmp88* a, Unk80313C14_Cmp88* b)
{
    if (a->m88 < b->m88)
        return -1;
    if (a->m88 > b->m88)
        return 1;
    if (a->m8C < b->m8C)
        return -1;
    return 1;
}

int fn_803170E0(Unk80313C14_Cmp18* a, Unk80313C14_Cmp18* b)
{
    if (a->m18 < b->m18)
        return -1;
    if (a->m18 > b->m18)
        return 1;
    if (a->m1C < b->m1C)
        return -1;
    return 1;
}

extern "C" void fn_80317120(int a, int b)
{
    lbl_8061FF48 = a;
    if (a != 0 && lbl_8061FEDC->m204 != 0) {
        if (lbl_8068E51C == 0) {
            float* q = &lbl_8068E518;
            int v = lbl_8061FEDC->m28 != 0 ? lbl_8061FEDC->m2C : lbl_8061FEDC->m14;
            lbl_8068E51C = 1;
            *q = (float)v;
        }
        int base = b - 0x140;
        int sp8 = base;
        int spC = 0;
        fn_80317624(&sp8, &spC, 1);
        lbl_8061FF4C = (float)(sp8 - base) / lbl_8068E518;
        return;
    }
    lbl_8061FF4C = 0.0f;
}

extern float lbl_8068E550;
extern int lbl_8068E554;
extern float lbl_8068E558;
extern int lbl_8068E55C;
extern float lbl_8068E560;
extern int lbl_8068E564;
extern float lbl_8068E568;
extern int lbl_8068E56C;
extern float lbl_8068E570;
extern int lbl_8068E574;
extern float lbl_8068E520;
extern int lbl_8068E524;
extern float lbl_8068E528;
extern int lbl_8068E52C;
extern float lbl_8068E530;
extern int lbl_8068E534;
extern float lbl_8068E538;
extern int lbl_8068E53C;
extern float lbl_8068E540;
extern int lbl_8068E544;
extern float lbl_8068E548;
extern int lbl_8068E54C;
extern "C" void fn_80317240(float* p, float* unused, int a, int b)
{
    if (lbl_8061FEDC->m204 != 0) {
        if (lbl_8068E524 == 0) {
            lbl_8068E520 = 1.3333334f;
            lbl_8068E524 = 1;
        }
        if (lbl_8068E52C == 0) {
            float* q = &lbl_8068E528;
            int v = lbl_8061FEDC->m28 != 0 ? lbl_8061FEDC->m2C : lbl_8061FEDC->m14;
            lbl_8068E52C = 1;
            *q = (float)v;
        }
        if (lbl_8068E534 == 0) {
            float* q = &lbl_8068E530;
            int v = lbl_8061FEDC->m28 != 0 ? lbl_8061FEDC->m30 : lbl_8061FEDC->m18;
            lbl_8068E534 = 1;
            *q = (float)v;
        }
        if (lbl_8068E53C == 0) {
            lbl_8068E538 = lbl_8068E530 * lbl_8068E520 <? lbl_8068E528;
            lbl_8068E53C = 1;
        }
        if (lbl_8068E544 == 0) {
            lbl_8068E540 = lbl_8068E538 / lbl_8068E528;
            lbl_8068E544 = 1;
        }
        if (lbl_8068E54C == 0) {
            lbl_8068E548 = (lbl_8068E528 - lbl_8068E538) * 0.5f / lbl_8068E528;
            lbl_8068E54C = 1;
        }
        if (b == 0 && a == 0) {
            *p *= lbl_8068E540;
            *p += lbl_8068E548;
        }
        *p += lbl_8061FF4C;
    }
}

extern "C" void fn_8031746C(float* p, int unused, int flag)
{
    if (lbl_8061FEDC->m204 != 0) {
        if (lbl_8068E554 == 0) {
            lbl_8068E550 = 1.3333334f;
            lbl_8068E554 = 1;
        }
        if (lbl_8068E55C == 0) {
            float* q = &lbl_8068E558;
            int v = lbl_8061FEDC->m28 != 0 ? lbl_8061FEDC->m2C : lbl_8061FEDC->m14;
            lbl_8068E55C = 1;
            *q = (float)v;
        }
        if (lbl_8068E564 == 0) {
            float* q = &lbl_8068E560;
            int v = lbl_8061FEDC->m28 != 0 ? lbl_8061FEDC->m30 : lbl_8061FEDC->m18;
            lbl_8068E564 = 1;
            *q = (float)v;
        }
        if (lbl_8068E56C == 0) {
            lbl_8068E568 = lbl_8068E560 * lbl_8068E550 <? lbl_8068E558;
            lbl_8068E56C = 1;
        }
        if (lbl_8068E574 == 0) {
            lbl_8068E574 = 1;
            lbl_8068E570 = lbl_8068E568 / lbl_8068E558;
        }
        if (flag == 0)
            *p *= lbl_8068E570;
    }
}

extern float lbl_8068E578;
extern int lbl_8068E57C;
extern float lbl_8068E580;
extern int lbl_8068E584;
extern float lbl_8068E588;
extern int lbl_8068E58C;
extern float lbl_8068E590;
extern int lbl_8068E594;
extern float lbl_8068E598;
extern int lbl_8068E59C;
extern "C" void fn_80317624(int* p, int* unused, int flag)
{
    if (lbl_8061FEDC->m204 != 0 && flag != 0) {
        if (lbl_8068E57C == 0) {
            lbl_8068E578 = 1.3333334f;
            lbl_8068E57C = 1;
        }
        if (lbl_8068E584 == 0) {
            float* q = &lbl_8068E580;
            int v = lbl_8061FEDC->m28 != 0 ? lbl_8061FEDC->m2C : lbl_8061FEDC->m14;
            lbl_8068E584 = 1;
            *q = (float)v;
        }
        if (lbl_8068E58C == 0) {
            float* q = &lbl_8068E588;
            int v = lbl_8061FEDC->m28 != 0 ? lbl_8061FEDC->m30 : lbl_8061FEDC->m18;
            lbl_8068E58C = 1;
            *q = (float)v;
        }
        if (lbl_8068E594 == 0) {
            lbl_8068E590 = lbl_8068E588 * lbl_8068E578 <? lbl_8068E580;
            lbl_8068E594 = 1;
        }
        if (lbl_8068E59C == 0) {
            lbl_8068E598 = lbl_8068E580 / lbl_8068E590;
            lbl_8068E59C = 1;
        }
        *p = (int)((float)*p * lbl_8068E598);
    }
}

struct Unk80313C14_M16 {
    float f[16];
};
extern Unk80313C14_M16 lbl_806E2144;
extern const void* const lbl_806658C0[];
extern const void* const lbl_80693CAC[];
extern const char lbl_805E79A4[];
struct Unk80313C14_FxA {
    char pad0[0x18];
    const void* m18;
    char pad1C[8];
    int m24;
    int m28;
    int m2C;
    char* m30;
    char* m34;
    char* m38;
    const void* m3C;
    char pad40[0x3C];
    char m7C[60];
    char mB8[60];
    int mF4;
    int mF8;
    int mFC;
    int m100[6];
    int m118;
    int m11C;
    int m120;
    int m124[6];
    void* m13C;
    void* m140;
    void* m144;
    char* m148[6];
    float m160;
    float m164;
};
extern "C" {
void fn_80328234(void* p, int a, int b);
int fn_803E9B98(const void* a, void* b);
int fn_803DBD54(void* p);
void fn_803E6660(void* p);
void* fn_803774A8(unsigned size, int heap);
void fn_8035F77C(void* p);
void fn_80317C94(void* p);
void* memset(void* p, int c, unsigned n, ...);
}
void* fn_80317810(void* pp, int a, int b, int c, int d)
{
    Unk80313C14_FxA* self = (Unk80313C14_FxA*)pp;
    int zero = 0;
    char* sub = (char*)self + 0x3C;
    int k = a - 1;
    char tmp[8];
    fn_80328234(self, 0xA, 0);
    self->m18 = lbl_806658C0;
    self->m3C = lbl_80693CAC;
    self->m24 = zero;
    *(int*)(sub + 4) = zero;
    *(int*)(sub + 0x14) = fn_803E9B98(lbl_805E79A4, tmp);
    *(int*)(sub + 0x10) = 1;
    fn_803DBD54(self->m7C);
    fn_803E6660(self->mB8);
    lbl_8061FF44 = (int)self;
    self->m2C = zero;
    self->mF4 = a;
    self->mF8 = b;
    self->mFC = d;
    char* arr = (char*)fn_803774A8(a * 0x98, 0x100);
    char* q = arr;
    if (a != 0) {
        do {
            fn_8035F77C(q);
            q += 0x98;
        } while (k-- != 0);
    }
    self->m13C = arr;
    int k2 = b - 1;
    char* arr2 = (char*)fn_803774A8(b * 0x98, 0x100);
    char* q2 = arr2;
    if (b != 0) {
        do {
            fn_8035F77C(q2);
            q2 += 0x98;
        } while (k2-- != 0);
    }
    self->m140 = arr2;
    self->m144 = fn_803774A8(d * 0x24, 0x100);
    self->m118 = 0;
    self->m11C = 0;
    self->m120 = 0;
    int i = 0;
    int cnt = 6;
    do {
        char* b1 = (char*)self + 0x100;
        *(int*)(b1 + i) = c;
        char* b2 = (char*)self + 0x148;
        *(char**)(b2 + i) = (char*)fn_803774A8(c * 2, 0x100);
        char* b3 = (char*)self + 0x124;
        *(int*)(b3 + i) = 0;
        i += 4;
    } while (--cnt != 0);
    Unk80313C14_M16 m;
    Unk80313C14_M16* mp = &m;
    memset(mp, 0, 0x40);
    m.f[0] = -1.0f;
    m.f[5] = 1.0f;
    m.f[10] = 1.0f;
    m.f[15] = 1.0f;
    lbl_806E2144 = *mp;
    self->m28 = 0;
    int n = self->mF4;
    if (n < self->mF8)
        n = self->mF8;
    int n6 = n * 6;
    self->m30 = (char*)fn_803774A8(n * 0x48, 0x100);
    self->m34 = (char*)fn_803774A8(n6 * 8, 0x100);
    self->m38 = (char*)fn_803774A8(n6 * 4, 0x100);
    self->m160 = 0.0015625f;
    self->m164 = 0.0020833334f;
    fn_8031883C(self);
    fn_80317C94(self);
    return self;
}

void fn_80317AB0(int flag)
{
    if (lbl_8061FF44 == 0) {
        if (flag)
            lbl_8061FF44 = (int)fn_80317810(operator new(0x168, 0x100), 0x10E, 0, 0x1000, 0x12C);
        else
            lbl_8061FF44 = (int)fn_80317810(operator new(0x168, 0x100), 0x82, 0x1E, 0x800, 0xA5);
    }
}

void fn_80317B30()
{
    if (lbl_8061FF44_big) {
        delete lbl_8061FF44_big;
        lbl_8061FF44_big = 0;
    }
}
