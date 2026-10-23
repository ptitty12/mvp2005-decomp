/*
 * Game code, .text 0x802ACD88-0x802B0ACC (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_802ACD88.h"

typedef Unk802ACD88_Mgr Mgr;

class Unk802ACD88_G {
public:
    virtual int s1();
    virtual int s2();
    virtual int s3();
    virtual int s4();
    virtual int s5();
    virtual int s6();
    virtual int s7();
    virtual int s8();
    virtual int s9();
    virtual int s10();
    virtual int s11(void* a);
    virtual int s12(void* a);
};

struct Unk802AE9C8_R {
    void* m0;
    void* m4;
    struct Unk802AE9C8_Q* m8;
    float mC;
};

struct Unk802AE9C8_Q {
    char pad0[0x10];
    int m10;
};

class Unk802AE200_VT18 {
public:
    char pad0[0x18];
    virtual int s1();
    virtual int s2();
    virtual int s3();
    virtual int s4(const char* name);
};

struct Unk802AE13C_Z {
    int m0;
    int m4;
    int m8;
};

extern void* (*lbl_8062C668)(int size, int heap);
extern const char lbl_805DFFB0[];
extern const char lbl_805DFFC0[];
extern char lbl_8060D190[];
extern Unk802ACD88_G* lbl_80626DD0;
extern const char* lbl_8061C728[];
extern const char lbl_805E00B4[];
extern const char lbl_805DFC08[];
extern const char lbl_805DFBF4[];
struct Unk802ACE20_S7 {
    char c[7];
};
struct Unk802ACE20_S8 {
    int a;
    int b;
};
struct Unk802ACE20_S8c {
    char c[8];
};
typedef Unk802ACE20_S8c S8c;
typedef Unk802ACE20_S7 S7;
typedef Unk802ACE20_S8 S8;
extern const S7 lbl_805DFBF8;
extern const S8 lbl_805DFC00;
extern "C" char* strcpy(char* d, const char* s);
extern const char lbl_805DFFD0[];
extern const char lbl_805E0004[];

void* operator new(unsigned int size, int heap);

extern "C++" {
extern "C" void* fn_803E9640(void* a, void* b, int c, void* fn);
extern "C" void fn_803E9994(void* a, char* tbl, int* n, void* z);
extern "C" void fn_803D6144(int a, int b);
void fn_80314D60(int a);
extern "C" void fn_80312700();
extern "C" void* fn_80187190(void* mem, void* a, void* b);
void* fn_80306350(void* mem, int a, void* b, void* c);
void fn_8030427C(void* a, int b);
extern "C" void fn_802AA7A4(int a, int b);
int fn_8040B574(const char* a, const char* b);
extern "C" void* fn_802B9300(const char* name, int a, int b, int* n);
void fn_802ACE20(Mgr* m, void* p, int n);
extern "C" void fn_80329AEC(const char** pp, const char** out, int* pn, int delim);
extern "C" int fn_802B6368(void* p, float x);
void* fn_802AFA54(Mgr* m, const char* s);
extern "C" void fn_803DDA8C(void* a, void* b);
extern "C" void fn_802AF1C4(Mgr* m, void* a, int b);
void fn_802BBA88(void* p);
int fn_8040B6A0(const char* s);
extern "C" int fn_80379094(const char* s, int len, int a);
extern "C" void fn_80378EF8(const char* s, int len, char* buf, int cap, int* n);
int fn_8033E958(const char* fmt, ...);
char* fn_8040B8FC(const char* s, int c);
extern "C" int fn_802AF6F8(Mgr* m, char* buf, int flag);
extern "C" void fn_802B7C50(void* p);
void fn_802C40BC(void* p, void* a);
void fn_802C40DC(void* p, void* a);
void* fn_80086140(void* x, char* buf);
extern "C" void* fn_8014EB44(void* x);
extern "C" void fn_8014EC24(void* x, char* buf);
extern "C" void fn_8014EC5C(void* x, char* buf);
void fn_802C3D2C(void* p, int a);
void fn_802C79F0(void* p, int a);
void fn_802C7B54(void* p);
void fn_802C3EC0(void* p);
extern "C" void fn_802C78B8(void* p);
void fn_802C4030(void* p);
extern "C" int fn_802B34E0(void* p);
int fn_802B7A28(void* p);
int fn_802C4100(void* p);
int fn_802C417C(void* p);
extern "C" int* fn_80378024(void* p, void* q);
void* fn_802AEC14(Mgr* m, const char* s, int a, int b);
extern "C" int fn_802AED2C(void* a, void* b, int c);
int fn_802AF624(void* a, void* b, int c);
extern "C" void* fn_803ED604(void* a, int b, void* c);
extern "C" void* fn_803DD628(void* a, void* b);
extern "C" void fn_803DD73C(void* a, int b);
extern "C" void* MEM_free(void* a);
unsigned int fn_80378568(const char* s, int n);
int fn_80377E90(void* t, int* a, int* b, unsigned int h);
int fn_802AFB88(Mgr* m);
extern "C" void fn_80377CD8(void* t, int idx, int a, unsigned int h);
extern "C" void fn_802AFC7C(Mgr* m, void* e, int idx, int a, int b);
void* fn_802B98AC(const char* name, int a);
void fn_802BA310(void* t, const char* s);
void fn_802BA218(void* t);
void fn_802BA750(void* t);
void fn_802BA820(void* t, int b);
int* fn_802BA4D8(void* t, int a);
Mgr* fn_802BAD48(Mgr* m, int a);
Mgr* fn_802BAD84(Mgr* m, int a);
}

void fn_802ACD88()
{
    fn_802AA7A4(1, 0xFFFF);
}

void fn_802ACDB4() {}

int fn_802ACDB8(Mgr* m, const char* s)
{
    for (int i = 0; i <= 0x56; i++) {
        if (fn_8040B574(lbl_8061C728[i], s) == 0)
            return i;
    }
    return 0;
}

void fn_802ACE20(Mgr* m, void* p, int len)
{
    const char* text = (const char*)p;
    const char* s;
    char* e = (char*)m + 0x37F8;
    int i;
    for (i = 0; i < 0x57; i++) {
        if (fn_8040B574(lbl_8061C728[i] + fn_8040B6A0(lbl_8061C728[i]) - 3, lbl_805DFBF4) == 0)
            *(S7*)e = lbl_805DFBF8;
        else
            { S8* d = (S8*)e; d->a = lbl_805DFC00.a; d->b = lbl_805DFC00.b; }
        e += 9;
    }
    for (i = 0; i < 0x57; i++) {
        fn_80329AEC(&text, &s, &len, 0x2C);
        int idx = fn_802ACDB8(m, s);
        fn_80329AEC(&text, &s, &len, 10);
        *(char*)(fn_8040B6A0(s) + (int)s - 1) = 0;
        strcpy((char*)m + (idx * 9 + 0x37F8), s);
    }
}

void fn_802ACF58(Mgr* m)
{
    int n = 0;
    void* p = fn_802B9300(lbl_805DFC08, 0, 14, &n);
    fn_802ACE20(m, p, n);
}

extern "C" int sprintf(char* buf, const char* fmt, ...);
extern "C" void __assert(const char* file, int line, const char* expr);
struct Unk802B0778_C;
extern Unk802B0778_C* lbl_8061FEDC;
extern int lbl_8061E9F8;
extern int lbl_8061E9FC;
extern const char lbl_805DFC20[];
extern const char lbl_805DFC34[];
extern const char lbl_805DFC38[];
extern const char lbl_805DFC48[];
extern const char lbl_805DFC64[];
extern const char lbl_805DFC9C[];
extern const char lbl_805DFD0C[];
extern const char lbl_805DFD28[];
extern const char lbl_805DFD84[];
extern const char lbl_805DFDA0[];
extern const char lbl_805DFDBC[];
extern const char lbl_805DFE14[];
int fn_802B9764(int a);
int fn_802B96C4(int a);
void fn_802B8B34();
void fn_802B8BFC();
extern "C" int fn_803FED48(const char* fmt, int a, int b, int* out);
extern "C" void DrawLoadingProgress(Unk802B0778_C* h, int a, float v);
extern "C" void* fn_803DD628(void* mem, void* p);
void fn_802BACDC(void* p);
void fn_802C4294(void* p);
void fn_802C8008(void* p);
void fn_802ACF58(Mgr* m);
void fn_802AE13C(Mgr* m);
void fn_802AD538(Mgr* m, int h);
int fn_802AEB7C(Mgr* m);

struct Unk802ACFB0_M {
    char pad0[0x3B0C];
    void* m3B0C;
    char pad3B10[0x3B50 - 0x3B10];
    void* m3B50;
    void* m3B54;
    char pad3B58[4];
    int m3B5C;
    int m3B60;
    char pad3B64[4];
    int m3B68;
    void* m3B6C;
    void* m3B70;
    int m3B74;
    char pad3B78[0x18700 - 0x3B78];
    int m18700;
};

void fn_802ACFB0(Mgr* m)
{
    Unk802ACFB0_M* v = (Unk802ACFB0_M*)m;
    char b1[32];
    char b2[64];
    sprintf(b1, lbl_805DFC20, lbl_805DFC34);
    int n = fn_802B9764((int)b1);
    lbl_8061E9FC = n;
    if (n > 0) {
        const char* nm = lbl_805DFC38;
        if (((int (*)(const char*, int))fn_802B96C4)(nm, 0) != 0) {
            lbl_8061E9F8 = 1;
            fn_803FED48(nm, 0, 0, &v->m3B74);
        }
        DrawLoadingProgress(lbl_8061FEDC, 2, 0.4292683f);
        fn_802B8B34();
        fn_802B8BFC();
        fn_802ACF58(m);
        sprintf(b2, lbl_805DFC48, lbl_805DFC34);
        v->m3B50 = fn_802B9300(b2, 0x400, 1, 0);
        if (v->m3B50 == 0)
            __assert(lbl_805DFC64, 0x2B8, lbl_805DFC9C);
        int* q = &v->m18700;
        *q = (int)fn_803DD628(lbl_8062C668(0x58, 0), (char*)v->m3B50 + *(int*)((char*)v->m3B50 + 0x14));
        fn_802BACDC((char*)m + 0x16EF0);
        sprintf(b2, lbl_805DFD0C, lbl_805DFC34);
        v->m3B54 = fn_802B9300(b2, 0x400, 1, 0);
        if (v->m3B54 == 0)
            __assert(lbl_805DFC64, 0x2C2, lbl_805DFD28);
        v->m3B60 = (int)fn_803DD628(lbl_8062C668(0x58, 0), (char*)v->m3B54 + *(int*)((char*)v->m3B54 + 0x14));
        sprintf(b2, lbl_805DFD84, lbl_805DFC34);
        v->m3B6C = fn_802B9300(b2, 0x400, 1, 0);
        v->m3B5C = (int)fn_803DD628(lbl_8062C668(0x58, 0), (char*)v->m3B6C + *(int*)((char*)v->m3B6C + 0x14));
        sprintf(b2, lbl_805DFDA0, lbl_805DFC34);
        v->m3B70 = fn_802B9300(b2, 0x400, 1, 0);
        if (v->m3B70 == 0)
            __assert(lbl_805DFC64, 0x2D6, lbl_805DFDBC);
        v->m3B68 = (int)fn_803DD628(lbl_8062C668(0x58, 0), (char*)v->m3B70 + *(int*)((char*)v->m3B70 + 0x14));
        sprintf(b2, lbl_805DFE14, lbl_805DFC34);
        v->m3B0C = fn_802B9300(b2, 0x400, 1, 0);
        fn_802AE13C(m);
        fn_802C4294((char*)m + 0x11F78);
        fn_802C8008((char*)m + 0x12204);
        DrawLoadingProgress(lbl_8061FEDC, 2, 0.5512195f);
        fn_802AD538(m, 0);
        fn_802AEB7C(m);
    }
}

struct Unk802AD2B8_Rec {
    char pad0[0xC];
    int mC;
};
struct Unk802AD2B8_Seq {
    Unk802AD2B8_Rec* m0;
    int m4;
};
struct Unk802AD2B8_It {
    Unk802AD2B8_Rec* p;
};
struct Unk802AD2B8_Res {
    void* m0;
    void* m4;
    int m8;
};
struct Unk802AD2B8_O {
    Unk802AD2B8_Seq* m0;
    char pad4[0x14C - 4];
    Unk802AD2B8_Rec* m14C;
    Unk802AD2B8_Rec* m150;
    Unk802AD2B8_Rec* m154;
    Unk802AD2B8_Rec* m158;
    Unk802AD2B8_Rec* m15C;
    Unk802AD2B8_Rec* m160;
};
typedef Unk802AD2B8_It (*Unk802AD2B8_Find)(Unk802AD2B8_Seq*, const char*, ...);
extern "C" Unk802AD2B8_It fn_803D775C(Unk802AD2B8_Seq* s, const char* name);
extern "C" Unk802AD2B8_It fn_803D7378(Unk802AD2B8_Seq* s, const char* name);
extern "C" int fn_803D77EC(Unk802AD2B8_Rec* e);
extern "C" int fn_803DBE2C(int a, int b);
extern "C" void fn_8030ACCC(void* p);
void fn_80305C88(void* p, int a);
void fn_80305C30(void* p, int a, int b, int c);
extern const char lbl_805DEFC4[];
extern const char lbl_805DEFDC[];
extern const char lbl_805DEFE4[];
extern const char lbl_805DFE34[];

/* near-miss 97.0%: retail stores fn_803E9640's result straight from r3 (stw r3,4(r29)); ours copies it through r0 first and loads res->m0/out in the other order */
void fn_802AD2B8(Unk802AD2B8_O* o, Unk802AD2B8_Res* res, const char* name, int a, int b)
{
    int out;
    int n;
    Unk802AE13C_Z z;
    res->m0 = fn_802B9300(name, 0x400, 1, &out);
    res->m8 = 1;
    void* alloc = lbl_8062C668(0x30, 0);
    void* r = fn_803E9640(alloc, res->m0, out, (void*)fn_80312700);
    n = 0;
    z.m0 = 0;
    res->m4 = r;
    Unk802AE13C_Z* pz = &z;
    pz->m8 = 0;
    fn_803E9994(r, lbl_8060D190, &n, pz);
    Unk802AD2B8_Seq* t = (Unk802AD2B8_Seq*)z.m8;
    o->m0 = 0;
    fn_8030ACCC((char*)o + 4);
    o->m0 = t;
    if (t != 0) {
        o->m14C = ((Unk802AD2B8_Find)fn_803D775C)(o->m0, lbl_805DEFC4).p;
        o->m150 = ((Unk802AD2B8_Find)fn_803D7378)(o->m0, lbl_805DEFC4).p;
        o->m154 = ((Unk802AD2B8_Find)fn_803D775C)(o->m0, lbl_805DEFDC).p;
        o->m158 = ((Unk802AD2B8_Find)fn_803D7378)(o->m0, lbl_805DEFDC).p;
        o->m15C = ((Unk802AD2B8_Find)fn_803D775C)(o->m0, lbl_805DEFE4).p;
        o->m160 = ((Unk802AD2B8_Find)fn_803D7378)(o->m0, lbl_805DEFE4).p;
        fn_80305C88(o, 0);
    }
    if (a != 0)
        fn_80305C30(o, a, b, 0);
    Unk802AD2B8_Seq* s = o->m0;
    Unk802AD2B8_Rec* it = ((Unk802AD2B8_Find)fn_803D775C)(s, lbl_805DFE34).p;
    if (it != s->m0 + s->m4) {
        int v;
        int ok;
        if (fn_803D77EC(it) != 0) {
            v = it->mC;
            ok = 1;
        } else {
            v = 0;
            ok = 0;
        }
        if (ok != 0)
            fn_803DBE2C(v, 1);
    }
}

Mgr* fn_802AD4C8(Mgr* m, int a)
{
    fn_802BAD48(m, a + 0x16EF0);
    return m;
}

Mgr* fn_802AD500(Mgr* m, int a)
{
    fn_802BAD84(m, a + 0x16EF0);
    return m;
}

class Unk8037C7FC {
public:
    int fn_8037CE98(int arg, int value, int flags);
};
Unk8037C7FC* fn_8037C7FC();
extern "C" int fn_803119C0();
extern "C" void fn_80310E1C(int h, unsigned i, void* a, void* b, void* c, void* d);
void fn_802C8058(void* p);
void fn_802BB074(void* p);
extern int lbl_8068D9D4;
extern "C" void* memset(void* d, int c, unsigned int n);

struct Unk802AD538_R {
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
};
struct Unk802AD538_T {
    Unk802AD538_R r[5];
};
extern Unk802AD538_T lbl_805DFE48;
extern Unk802AD538_T lbl_805DFEC0;
extern Unk802AD538_T lbl_805DFF38;

void fn_802AD538(Mgr* m, int h)
{
    Unk802AD538_T t1 = lbl_805DFE48;
    Unk802AD538_T t2 = lbl_805DFEC0;
    Unk802AD538_T t3 = lbl_805DFF38;
    Unk802AD538_T t4 = {};
    int hd;
    if (h != 0)
        hd = h;
    else
        hd = fn_803119C0();
    if (hd != 0) {
        int p = fn_8037C7FC()->fn_8037CE98(0xC22CAFD4, lbl_8068D9D4, 0);
        Unk802AD538_T* a = &t1;
        Unk802AD538_T* b = &t2;
        Unk802AD538_T* c = &t3;
        if (p != 0) {
            a = (Unk802AD538_T*)(p + 8);
            b = (Unk802AD538_T*)(p + 0x80);
            c = (Unk802AD538_T*)(p + 0xF8);
            h = p + 0x170;
        } else {
            h = (int)&t4;
        }
        for (unsigned i = 0; i <= 3; i++)
            fn_80310E1C(hd, i, a, b, c, (Unk802AD538_T*)h);
        fn_802C8058((char*)m + 0x12204);
        fn_802BB074((char*)m + 0x16EF0);
        fn_80314D60(m->m3B10);
        fn_802ACDB4();
    }
}

void fn_802AE13C(Mgr* m)
{
    int n = 0;
    Unk802AE13C_Z z;
    Unk802AE13C_Z* pz = &z;
    z.m0 = 0;
    pz->m8 = 0;
    int out;
    m->m3B14 = fn_802B9300(lbl_805DFFB0, 0x400, 1, &out);
    m->m3B1C = 1;
    void* o = lbl_8062C668(0x30, 0);
    void* r = fn_803E9640(o, m->m3B14, out, (void*)fn_80312700);
    m->m3B18 = r;
    fn_803E9994(r, lbl_8060D190, &n, pz);
    m->m3B10 = z.m8;
    fn_803D6144(z.m8, m->m3B0C);
    fn_80314D60(m->m3B10);
}

void* fn_802AE200(Mgr* m, void* a, void* b)
{
    void* r = fn_80187190(operator new(0xC20, 0), a, b);
    ((Unk802AE200_VT18*)((char*)r + 0xA0))->s4(lbl_805DFFC0);
    fn_8030427C(fn_80306350(operator new(0x198, 0), 0x1C, r, a), m->m3B10);
    return r;
}

extern "C" {
char* strchr(const char* s, int c);
int strncmp(const char* a, const char* b, unsigned int n);
}
extern char lbl_8060D1A0[];
extern int lbl_8061B92C;
extern int lbl_8061B930;
extern const char lbl_805DFFC8[];
extern const char lbl_805DFFCC[];
extern "C" void fn_803DD9FC(void* p, float f);

void fn_802AE298(void* s, float f)
{
    int n = 0;
    char* v[3];
    char** pv = v;
    v[0] = 0;
    pv[2] = 0;
    while (((int (*)(void*, char*, int*, void*))fn_803E9994)(s, lbl_8060D1A0, &n, pv) != 0) {
        char* t = strchr(strchr(v[0], 0x3D) + 1, 0x3D) + 1;
        fn_803DD9FC(v[2], f);
        if (lbl_8061B930 == 0 && lbl_8061B92C == 0 && strncmp(t, lbl_805DFFC8, 3) != 0)
            strncmp(t, lbl_805DFFCC, 3);
    }
}

int fn_802AE374(Mgr* m)
{
    return fn_802B34E0((char*)m + 8);
}

int fn_802AE398(Mgr* m, float x)
{
    if (!fn_802B6368((char*)m + 8, x))
        return 0;
    void* r = fn_802AFA54(m, lbl_805DFFD0);
    if (r != 0)
        fn_803DDA8C(*(void**)((char*)m + 0x18700), r);
    return 1;
}

int fn_802AE408(Mgr* m)
{
    return fn_802B7A28((char*)m + 8);
}

void fn_802AE42C(Mgr* m)
{
    char* p = m->m3B50;
    fn_803DDA8C(*(void**)((char*)m + 0x18700), p + *(int*)(p + 0x14));
    fn_802B7C50((char*)m + 8);
}

void fn_802AE474(Mgr* m, void* x)
{
    char buf[16];
    fn_802C40BC((char*)m + 0x11F78, x);
    fn_8014EC24(fn_8014EB44(fn_80086140(x, buf)), buf);
}

void fn_802AE4C0(Mgr* m, void* x)
{
    char buf[16];
    fn_802C40DC((char*)m + 0x11F78, x);
    fn_8014EC5C(fn_8014EB44(fn_80086140(x, buf)), buf);
}

int fn_802AE50C(Mgr* m)
{
    return fn_802C4100((char*)m + 0x11F78);
}

int fn_802AE534(Mgr* m)
{
    return fn_802C417C((char*)m + 0x11F78);
}

void fn_802AE55C(Mgr* m, int a)
{
    char* b = (char*)m + 0x10000;
    fn_802C3D2C(b + 0x1F78, 0x23);
    fn_802C79F0(b + 0x2204, a);
}

void fn_802AE5A0(Mgr* m)
{
    char* b = (char*)m + 0x10000;
    fn_802C7B54(b + 0x2204);
    fn_802C3EC0(b + 0x1F78);
}

void fn_802AE5D8(Mgr* m)
{
    char* b = (char*)m + 0x10000;
    fn_802C78B8(b + 0x2204);
    fn_802C4030(b + 0x1F78);
}

struct Unk80270BA0_X;
struct Unk8019F8C0_Owner;
class Unk80270BA0_Obj;
class Unk80304858_P198;
extern signed char lbl_806EF9C7;
extern "C" Unk80270BA0_X* fn_8023CB50(void* mem, int a, int b, int d, unsigned char e);
extern "C" void* fn_80387718(void* mem, Unk80270BA0_Obj* o, int i, signed char c);
void* fn_8037E928(void* mem, void* h, int idx, int c, Unk80270BA0_Obj* o);
void fn_80244E80(Unk80270BA0_X* x, int h, int i);
extern "C" void fn_80304098(void* mem, int k, Unk80270BA0_X* x, int c, int z, void* h);
void fn_80305B88(Unk80304858_P198* mem, int k, int x, int c, int h);

inline Unk80270BA0_Obj* Unk802AE610_Cv(Unk80270BA0_X* x)
{
    return x ? (Unk80270BA0_Obj*)((char*)x + 0x5F0) : 0;
}

Unk80270BA0_X* fn_802AE610(void* m, Unk8019F8C0_Owner* a_, int b, Unk8019F8C0_Owner* c_, int d, int e)
{
    int a = (int)a_;
    int c = (int)c_;
    Unk80270BA0_X* x = fn_8023CB50(operator new(0xB80, 0), a, b, d, e);
    void* mem2 = operator new(0x2A0, 0);
    void* h = fn_80387718(mem2, Unk802AE610_Cv(x), *(int*)((char*)m + 0x16EE8), lbl_806EF9C7);
    *(void**)((char*)x + 0xAB8) = h;
    fn_80244E80(x, (int)fn_8037E928(operator new(0x890, 0), h, 0, c, Unk802AE610_Cv(x)), 0);
    fn_80244E80(x, (int)fn_8037E928(operator new(0x890, 0), h, 1, c, Unk802AE610_Cv(x)), 1);
    fn_80244E80(x, (int)fn_8037E928(operator new(0x890, 0), h, 2, c, Unk802AE610_Cv(x)), 2);
    fn_80304098(operator new(0x198, 0), 0x19, x, c, 0, h);
    fn_80304098(operator new(0x198, 0), 0x18, x, c, 0, h);
    fn_80305B88((Unk80304858_P198*)operator new(0x19C, 0), 0x1A, (int)x, c, (int)h);
    return x;
}

class Unk801B6604_T;
Unk80270BA0_X* fn_801B6604(Unk801B6604_T* mem, int a, int i);

Unk80270BA0_X* fn_802AE7D4(void* m, Unk8019F8C0_Owner* a_, int i, Unk8019F8C0_Owner* c_)
{
    int a = (int)a_;
    int c = (int)c_;
    Unk80270BA0_X* x = fn_801B6604((Unk801B6604_T*)operator new(0xBA0, 0), a, i);
    void* mem2 = operator new(0x2A0, 0);
    void* h = fn_80387718(mem2, Unk802AE610_Cv(x), *(int*)((char*)m + 0x16EE8), lbl_806EF9C7);
    *(void**)((char*)x + 0xAB8) = h;
    fn_80244E80(x, (int)fn_8037E928(operator new(0x890, 0), h, 0, c, Unk802AE610_Cv(x)), 0);
    fn_80244E80(x, (int)fn_8037E928(operator new(0x890, 0), h, 1, c, Unk802AE610_Cv(x)), 1);
    fn_80244E80(x, (int)fn_8037E928(operator new(0x890, 0), h, 2, c, Unk802AE610_Cv(x)), 2);
    fn_80304098(operator new(0x198, 0), 0x19, x, c, 0, h);
    fn_80304098(operator new(0x198, 0), 0x18, x, c, 0, h);
    fn_80305B88((Unk80304858_P198*)operator new(0x19C, 0), 0x1A, (int)x, c, (int)h);
    return x;
}

int fn_802AE984(Mgr* m, void* p)
{
    int* r = fn_80378024((char*)m + 0x3B78, p);
    int ok = 0;
    if (r != (int*)-1) {
        if (*r != 0)
            ok = 1;
    }
    return ok;
}

int fn_802AE9C8(Mgr* m, void* key, float* a, int* b, int* c)
{
    Unk802AE9C8_R* r = (Unk802AE9C8_R*)fn_80378024((char*)m + 0x3B78, key);
    if (r != (Unk802AE9C8_R*)-1) {
        if (*(int*)r == 0)
            fn_802AF1C4(m, (char*)r->m8 + 0x18, 0);
        r->m8->m10++;
        *b = lbl_80626DD0->s11(r->m0);
        *a = r->mC;
        *c = lbl_80626DD0->s11(r->m4);
        return 1;
    }
    return 0;
}

void fn_802AEAA0(Mgr* m, void* key, void* a, void* b)
{
    void* p = 0;
    int* r = fn_80378024((char*)m + 0x3B78, key);
    if (r != (int*)-1)
        p = (void*)r[2];
    if (a != 0)
        lbl_80626DD0->s12(a);
    if (b != 0)
        lbl_80626DD0->s12(b);
    if (p != 0)
        fn_802BBA88(p);
}

float fn_802AEB4C(Mgr* m, void* p)
{
    float r = ((float*)fn_80378024((char*)m + 0x3B78, p))[3];
    return r;
}

int fn_802AEB7C(Mgr* m)
{
    if (m->m3B08 == 0)
        m->m3B08 = fn_802AEC14(m, lbl_805E0004, 1, 1);
    return 1;
}

int fn_802AEBCC(void* a, void* b)
{
    return fn_802AED2C(a, b, 0);
}

int fn_802AEBF0(void* a, void* b)
{
    return fn_802AF624(a, b, 0);
}

/* The cursor is a local copy of the parameter: combine folds the parameter copy into it, so `mr r31,r4`
 * comes after the flag copy (retail order), and before the n store because it is declared first. */
extern "C" int sprintf(char* buf, const char* fmt, ...);
extern int lbl_8061E9F8;
extern const char lbl_805E0010[];
extern const char lbl_805E0018[];
extern const char lbl_805E1B70[];
int fn_802B96C4(int a);
int fn_802AF624(Mgr* m, const char* s, int a);

void* fn_802AEC14(Mgr* m, const char* name, int a, int b)
{
    char buf[0x400];
    int r = 0;
    if (lbl_8061E9F8 != 0)
        sprintf(buf, lbl_805E0010, name);
    else
        sprintf(buf, lbl_805E0018, lbl_805E1B70, name);
    if (((int (*)(char*, int))fn_802B96C4)(buf, 1) == 0)
        return 0;
    int cnt = 0;
    char* data = (char*)fn_802B9300(buf, 0x100, 0xE, &cnt);
    if (cnt > 0) {
        data[cnt - 1] = 0;
        if (b != 0)
            r = fn_802AED2C(m, data, a);
        else
            r = fn_802AF624(m, data, a);
    }
    if (data != 0)
        delete data;
    return (void*)r;
}

int fn_802AF624(Mgr* m, const char* str, int flag)
{
    char buf[0x1388];
    const char* s = str;
    int n = 0;
    while (fn_80379094(s, fn_8040B6A0(s), 0), *s != 0) {
        fn_80378EF8(s, fn_8040B6A0(s), buf, 0x1388, &n);
        s += n;
        if (n != 0) {
            fn_8033E958(lbl_805E00B4, buf);
            char* p = fn_8040B8FC(buf, 0x2F);
            if (p != 0)
                *p = 0;
            fn_802AF6F8(m, buf, flag);
        }
    }
    return 1;
}

extern "C" {
unsigned int strlen(const char* s);
int strncmp(const char* a, const char* b, unsigned int n);
}
struct Unk802AFA54_E {
    char pad0[0x14];
    int off;
};
struct Unk802AFA54_T {
    char pad0[8];
    unsigned int n;
    char padC[4];
};
struct Unk802AFA54_V {
    char pad0[0x3658];
    Unk802AFA54_T* a;
    char pad365C[4];
    Unk802AFA54_T* b;
};
extern "C" int fn_803EC814(void* t, unsigned int i, char* buf);

void* fn_802AFA54(Mgr* m, const char* name)
{
    Unk802AFA54_V* v = (Unk802AFA54_V*)m;
    char buf[0x40];
    char* r = 0;
    unsigned int n = v->a->n;
    unsigned int i;
    for (i = 0; i < n; i++) {
        fn_803EC814(v->a, i, buf);
        if (strncmp(name, buf, strlen(name)) == 0) {
            r = (char*)v->a + ((Unk802AFA54_E*)((char*)v->a + i * 8))->off;
            break;
        }
    }
    n = v->b->n;
    for (i = 0; i < n; i++) {
        fn_803EC814(v->b, i, buf);
        if (strncmp(name, buf, strlen(name)) == 0) {
            r = (char*)v->b + ((Unk802AFA54_E*)((char*)v->b + i * 8))->off;
            break;
        }
    }
    return r;
}

int fn_802AFB50(Mgr* m)
{
    int i;
    for (i = 0; i < 0x5C3; i++) {
        char* b = (char*)m + 0x402C;
        int o = i * 0x1C;
        if (*(int*)(b + o) == 0)
            return i;
    }
    return 0;
}

int fn_802AFB88(Mgr* m)
{
    int i;
    for (i = 0; i < 0x96; i++) {
        char* b = (char*)m + 0xE180;
        int o = i * 4;
        if (*(int*)(b + o) == 0)
            return i;
    }
    return 0;
}

int fn_802AFBC4(Mgr* m)
{
    int i;
    for (i = 0; i < 0x5AA; i++) {
        char* b = (char*)m + 0xE3D8;
        int o = i * 4;
        if (*(int*)(b + o) == 0)
            return i;
    }
    return 0;
}

void* fn_802AFC00(void* a, void* b, void* c)
{
    return fn_803DD628(fn_803ED604(a, 0x58, b), c);
}

void* fn_802AFC3C(void* p)
{
    if (p != 0)
        fn_803DD73C(p, 2);
    return MEM_free(p);
}

/* near-miss: retail keeps only m+0x10000 in a register and re-forms every table address per use (no cached table pointers) */
void fn_802B0140(Mgr* m, const char* name, int flag, int n)
{
    unsigned int h = fn_80378568(name, 0);
    char* t = (char*)m + 0x10000 - 0x568;
    int a;
    int b;
    if (fn_80377E90(t, &a, &b, h) == 0) {
        int idx = fn_802AFB88(m);
        void* asset;
        if (flag != 0)
            asset = fn_802B98AC(name, n);
        else
            asset = fn_802B9300(name, n, 14, 0);
        char* tb = (char*)m + 0x10000 - 0x1E80;
        int o = idx * 4;
        *(void**)(tb + o) = asset;
        fn_80377CD8(t, idx, 0, h);
        char* e = *(char**)(tb + o);
        fn_802AFC7C(m, e + *(int*)(e + 0x14), idx, a, 0);
    }
}

extern "C" {
char* strncpy(char* d, const char* s, unsigned int n);
}
struct Unk802B9FF0_Hdr;
extern const char lbl_805E0138[];
extern int lbl_805E0F88;
extern "C" int fn_803FED48(const char* fmt, int a, int b, int* out);
extern "C" int fn_803FEDB8(int h, int n);
int fn_802B98AC(int a, int b);
void fn_802B9FF0(void* p, Unk802B9FF0_Hdr* h);
extern "C" void DCStoreRange(void* p, int n);
extern "C" void fn_80377488(void* p);

extern "C" void fn_802B0224(Mgr* m, const char* name, int n)
{
    unsigned int h = fn_80378568(name, 0);
    int a;
    int b;
    int found = fn_80377E90((char*)m + 0xFA98, &a, &b, h);
    if (found == 0) {
        int k = fn_802AFB88(m);
        char buf[0x400] = "";
        char* tb = (char*)m + 0xE180;
        void** e = (void**)(tb + k * 4);
        strncpy(buf, name, 0x400);
        buf[0] = 'a';
        buf[0x3FF] = 0;
        int sp = found;
        fn_803FED48(lbl_805E0138, 0x100, 0, &sp);
        *e = (void*)fn_802B98AC((int)buf, n);
        fn_803FEDB8(sp, 100);
        Unk802B9FF0_Hdr* x = (Unk802B9FF0_Hdr*)fn_802B98AC((int)name, 0x100);
        fn_802B9FF0(*e, x);
        if (x != 0)
            delete[] (char*)x;
        DCStoreRange(*e, (lbl_805E0F88 + 0x1F) & ~0x1F);
        fn_80377CD8((char*)m + 0xFA98, k, 0, h);
        char* o = (char*)*e;
        fn_802AFC7C(m, o + *(int*)(o + 0x14), k, a, 0);
    }
}

void fn_802B0388(Mgr* m, const char* s, int flag)
{
    char* b = (char*)m + 0x10000;
    int x;
    int y;
    unsigned int h = fn_80378568(s, 0);
    if (fn_80377E90(b - 0x568, &x, &y, h) == 0 && flag != 0)
        fn_802BA310(b - 0x550, s);
}

void fn_802B03F4(Mgr* m)
{
    fn_802BA218((char*)m + 0xFAB0);
}

void fn_802B041C(Mgr* m)
{
    fn_802BA750((char*)m + 0xFAB0);
}

void fn_802B0444(Mgr* m, int a, int b)
{
    char* t = (char*)m + 0xFAB0;
    fn_802BA820(t, b);
    int* r = fn_802BA4D8(t, a);
    r[1] = b;
}

extern "C" char* strcpy(char* d, const char* s);
struct Unk802B9F40;
struct Unk802BA110_Src {
    char pad0[8];
    char m8[1];
};
struct Unk802B0494_Rec {
    int m0;
    char pad4[0x24];
    void* m28;
    char m2C[1];
};
Unk802BA110_Src* fn_802BA55C(Unk802B9F40* t, const char* name);
Unk802B0494_Rec* fn_802BA5F4(Unk802B9F40* t);
void fn_802BA110(void* p, Unk802BA110_Src* s);

int fn_802B0494(Mgr* m, const char* name)
{
    char* t = (char*)m + 0x10000;
    Unk802B9F40* tb = (Unk802B9F40*)(t - 0x550);
    Unk802BA110_Src* src = fn_802BA55C(tb, name);
    if (src != 0) {
        char* t2 = t - 0x568;
        Unk802B0494_Rec* rec = fn_802BA5F4(tb);
        int k = fn_802AFB88(m);
        rec->m0 = k;
        strcpy(rec->m2C, src->m8);
        unsigned int h = fn_80378568(src->m8, 0);
        int a;
        int b;
        if (fn_80377E90(t2, &a, &b, h) == 0) {
            fn_802BA110(rec->m28, src);
            char* tc = t - 0x1E80;
            void** e = (void**)(tc + k * 4);
            *e = rec->m28;
            fn_80377CD8(t2, k, 0, h);
            char* o = (char*)*e;
            fn_802AFC7C(m, o + *(int*)(o + 0x14), k, a, (int)rec);
            return 1;
        }
    }
    return 0;
}

void fn_802B058C(Mgr* m, const char* name, void* asset)
{
    unsigned int h = fn_80378568(name, 0);
    char* t = (char*)m + 0x10000 - 0x568;
    int a;
    int b;
    if (fn_80377E90(t, &a, &b, h) == 0) {
        int idx = fn_802AFB88(m);
        char* tb = (char*)m + 0x10000 - 0x1E80;
        int o = idx * 4;
        *(void**)(tb + o) = asset;
        fn_80377CD8(t, idx, 0, h);
        char* e = *(char**)(tb + o);
        fn_802AFC7C(m, e + *(int*)(e + 0x14), idx, a, 0);
    }
}

extern "C" int fn_80378060(void* a, const char* s, int n);
extern "C" void fn_80377C50(void* a, int key);
extern "C" void fn_80377DBC(void* a, int* o1, int* o2, int idx);
extern "C" void fn_80377BEC(void* a, int idx);
struct Unk802B0638_Rec {
    int m0;
    int m4;
    int m8;
    char padC[0x1C - 0xC];
};

void fn_802B0638(Mgr* m, const char* name, int del)
{
    unsigned int h = fn_80378060((char*)m + 0xFA98, name, 0);
    if (h <= 0x95) {
        fn_80377C50((char*)m + 0xFA98, (int)name);
        int i = 0;
        int n = *(int*)((char*)m + 0xFA84);
        while (i < n) {
            int x;
            int y;
            fn_80377DBC((char*)m + 0xFA80, &x, &y, i);
            if (y == (int)h) {
                n--;
                fn_80377BEC((char*)m + 0xFA80, i);
            } else {
                i++;
            }
        }
        int* p = (int*)((char*)m + 0x402C);
        int off = 0;
        for (i = 0; i < 0x5C3; i++) {
            char* b1 = (char*)m + 0x4034;
            if (*(int*)(b1 + off) == (int)h) {
                char* b2 = (char*)m + 0x4030;
                char* t = (char*)m + 0xE3D8;
                int o = *(int*)(b2 + off) * 4;
                fn_802AFC3C(*(void**)(t + o));
                *(void**)(t + o) = 0;
                *p = 0;
            }
            p += 7;
            off += 0x1C;
        }
        if (del != 0) {
            char* tb = (char*)m + 0xE180;
            int o = h * 4;
            if (*(void**)(tb + o) != 0) {
                delete *(void**)(tb + o);
                *(void**)(tb + o) = 0;
            }
        }
        char* tz = (char*)m + 0xE180;
        int oz = h * 4;
        *(void**)(tz + oz) = 0;
    }
}

struct Unk802B09CC_Ent {
    int m0;
    char pad4[0x18];
};
struct Unk802B09CC_Obj {
    char pad0[6];
    short m6;
};
extern "C" int fn_80377F00(void* t, int* a, int* b, int key, int c);
void* SHAPE_clut(void* p);
extern "C" void SHAPE_writepixel(void* a, int x, int y, int z, int v);
extern "C" int fn_8014DD30(void* p);
extern "C" void fn_803DDA68(void* a, void* b);
extern "C" int SHAPE_size(void* p);
extern "C" void DCStoreRange(void* a, int b);

struct Unk802B9F40;
void fn_802BA750(Unk802B9F40* t);
struct Unk802B0778_C {
    char pad0[0x3C];
    int m3C;
};
struct Unk802B0778_Rec {
    int m0;
    char pad4[0x18];
};
extern Unk802B0778_C* lbl_8061FEDC;
extern "C" void fn_80318890(int h);

extern "C" void fn_802B0778(void* mem)
{
    Mgr* m = (Mgr*)mem;
    if (lbl_8061FEDC != 0)
        fn_80318890(lbl_8061FEDC->m3C);
    char* b = (char*)m + 0x10000;
    unsigned int i;
    fn_802BA750((Unk802B9F40*)(b - 0x550));
    for (i = 0; i <= 0x5A9; i++) {
        char* t = b - 0x1C28;
        void** e = (void**)(t + i * 4);
        fn_802AFC3C(*e);
        *e = 0;
    }
    for (i = 0; i <= 0x95; i++) {
        char* t = b - 0x1E80;
        void** e = (void**)(t + i * 4);
        if (*e != 0) {
            delete *e;
            *e = 0;
        }
    }
    Unk802B0778_Rec* r = (Unk802B0778_Rec*)((char*)m + 0x402C);
    Unk802B0778_Rec* p = r;
    for (int k = 0; k < 0x5C3; k++) {
        p->m0 = 0;
        p++;
    }
    int* q0 = (int*)(b - 0x568);
    int* q1 = (int*)(b - 0x580);
    q0[1] = 0;
    q1[1] = 0;
}

void* fn_802BA4D8(Unk802B9F40* t, void* h);
extern int lbl_8061E970;
struct Unk802B0850_Ent {
    int m0;
    char pad4[0x18];
};
struct Unk802B0850_Obj {
    int m0;
    int m4;
};
int fn_802B0494(Mgr* m, const char* name);

extern "C" void* fn_802B0850(Mgr* m, const char* name, int* found)
{
    int a = -1;
    int b = -1;
    int r = fn_80377F00((char*)m + 0xFA80, &a, &b, (int)name, 0);
    if (found != 0) {
        *found = 0;
        if (r != 0) {
            Unk802B9F40* tb = (Unk802B9F40*)((char*)m + 0xFAB0);
            Unk802B0850_Ent* e1 = (Unk802B0850_Ent*)((char*)m + 0x4030);
            void** t = (void**)((char*)m + 0xE3D8);
            Unk802B0850_Obj* x = (Unk802B0850_Obj*)fn_802BA4D8(tb, t[e1[a].m0]);
            if (x != 0) {
                *found = 1;
                x->m4 = *(int*)((char*)tb + 0x2350);
            }
        }
    }
    if (r == 0) {
        if (fn_802B0494(m, name) != 0) {
            *found = 1;
            a = -1;
            b = -1;
            fn_80377F00((char*)m + 0xFA80, &a, &b, (int)name, 0);
            Unk802B0850_Ent* e1 = (Unk802B0850_Ent*)((char*)m + 0x4030);
            void** t = (void**)((char*)m + 0xE3D8);
            return t[e1[a].m0];
        }
        a = -1;
        b = -1;
        if (fn_80377F00((char*)m + 0xFA80, &a, &b, lbl_8061E970, 0) == 0)
            return 0;
    }
    Unk802B0850_Ent* e1 = (Unk802B0850_Ent*)((char*)m + 0x4030);
    void** t = (void**)((char*)m + 0xE3D8);
    return t[e1[a].m0];
}

extern "C" void fn_802B09CC(void* mem, const char* name, int base, int n, int pp)
{
    Mgr* m = (Mgr*)mem;
    int key = (int)name;
    char* p = (char*)pp;
    int a = -1;
    int b = -1;
    char* t = (char*)m + 0x10000;
    if (fn_80377F00(t - 0x580, &a, &b, key, 0) != 0) {
        Unk802B09CC_Ent* e0 = (Unk802B09CC_Ent*)((char*)m + 0x402C);
        Unk802B09CC_Obj* o = (Unk802B09CC_Obj*)SHAPE_clut((void*)e0[a].m0);
        if (o != 0 && o->m6 == 1) {
            for (int i = 0; i < n; i++) {
                int v = fn_8014DD30(p);
                p += 0x10;
                SHAPE_writepixel(o, base + i, 0, 0, v);
            }
            Unk802B09CC_Ent* e1 = (Unk802B09CC_Ent*)((char*)m + 0x4030);
            void** tb = (void**)(t - 0x1C28);
            fn_803DDA68(tb[e1[a].m0], o);
            DCStoreRange(o, SHAPE_size(o));
        }
    }
}
