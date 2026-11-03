/*
 * EA shared library code that uses small data (cflags_ealib_sda: -O2, default -G8):
 * VP6/MAD video codecs and SPCH, or RealCore,
 * .text 0x803EE8F8-0x803F00FC (code-only unit, 34 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

extern char lbl_8060E220[];
extern char lbl_8060E358[];
extern void* lbl_806AAD80[4];
extern char lbl_806AAD84[];
extern int lbl_806AADAC[4];

struct RcIfB {
    virtual void v0(int);
};
extern RcIfB* lbl_806EEA2C;

extern "C" void* memset(void*, int, unsigned);
extern "C" void fn_80377408(void*);
extern "C" int fn_803F8944(RcIfB*);
extern "C" int fn_803F7604(RcIfB*);
extern "C" void fn_803EF23C(int, int);
extern "C" void fn_803F009C(void*, int*);
extern "C" int fn_803F0074(void*, int*);

struct RcSys {
    int m0, m4;
    char pad8[0x6c - 8];
    int m6C, m70, m74, m78, m7C, m80, m84, m88;
    const void* m8C;
};

extern RcSys lbl_806AB4C8;
extern "C" void fn_803F13A0();
extern "C" void UpdateTaskGetCardInfo();
extern "C" void UpdateTaskMount();
extern "C" void UpdateTaskUnmount();
extern "C" void UpdateTaskOpen();
extern "C" void UpdateTaskClose();
extern "C" void fn_803F1924();
extern "C" void fn_803F1AAC();
extern "C" void UpdateTaskSeek();
extern "C" void UpdateTaskFlush();
extern "C" void UpdateTaskDelete();
extern "C" void UpdateTaskSetAttribute();
extern "C" void UpdateTaskFindFile();
extern "C" void UpdateTaskTrcStartGame();
extern "C" void UpdateTaskTrcCardExists();
extern "C" void UpdateTaskTrcGetCardInfo();
extern "C" void fn_803F2CA8();
extern "C" void fn_803F391C();
extern "C" void fn_803F4448();
extern "C" void fn_803F4B50();
extern "C" void fn_803F4EE4();
extern "C" void fn_803F564C();
extern "C" void fn_803F5C90();
extern RcSys* lbl_806EEA40;
extern "C" void fn_803EFE60(RcSys*);

inline void RcSysInit(RcSys* s)
{
    s->m0 = 0;
    s->m4 = 0;
    s->m6C = 0;
    s->m70 = 0;
    s->m74 = 0;
    s->m78 = 0;
    s->m7C = 0;
    s->m80 = 0;
    s->m84 = 0;
    s->m88 = 0;
}

struct RcStk {
    void* m0;
    void* arr[16];
    int m44;
    int m48;
};

/* near-miss: 7 slots (register numbering of top/a/offset); load of m44 twice and store order as retail */

struct RcVBase {
    int m0, m4, m8, mC;
};

struct RcV0 : RcVBase {
    virtual void s0();
    virtual void s1();
};

struct RcV : RcV0 {
    short m14;
    int m18;
    int m1C;
};

extern char lbl_8060E0D0[];
extern "C" int sprintf(char*, const char*, ...);

struct RcV12 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
};

extern RcV12* lbl_806EEA38;
extern int lbl_806EEA30;
extern int lbl_806EEA28;
extern int lbl_806AAD9C[3];

struct RcIfA {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5(int);
    virtual void v6(int);
};

struct RcIfC {
    virtual void v0();
};

extern RcIfA* lbl_806EEA34;
extern int lbl_806EEA3C;
extern RcIfC* lbl_806EEA60;
extern char lbl_8060E120[];
extern "C" void fn_803F693C(void*, int);
extern "C" void fn_803F6FC4(void*, int);
extern "C" void fn_803F6F48(int);
extern "C" void* fn_803F6F50(int, int, int, int, int);
extern "C" RcIfC* fn_803EE978(void*, int*);

extern "C" RcIfC* fn_803EE8F8(int* p)
{
    fn_803F6F48(p[0]);
    if (lbl_806EEA60 == 0) {
        void* m = fn_803F6F50(0, 0x1c, 0, 0, 0);
        lbl_806EEA60 = fn_803EE978(m, p);
    } else
        lbl_806EEA60->v0();
    return lbl_806EEA60;
}

struct RcGetA {
    virtual RcV12* g0();
};
struct RcGetB {
    virtual RcIfA* g0();
};
struct RcIfAx {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3(int);
    virtual void v4(void*);
    virtual void v5();
    virtual void v6();
    virtual void v7(int);
};
extern char lbl_806EE950[];
extern int lbl_806AADA8[4];
extern int lbl_806EEA20;
extern int lbl_806EEA24;
extern char lbl_806AADFC[];
extern "C" void* fn_803F684C(void*, int*);
extern "C" RcIfB* fn_803F713C(void*, void*);
extern "C" void fn_803F7048(void*, RcIfB*, int);
extern "C" int fn_803EEC20();

extern "C" RcIfC* fn_803EE978(void* self, int* p)
{
    fn_803F684C(self, p);
    *(void**)self = lbl_8060E120;
    memset(lbl_806EE950, 0, 0x60);
    lbl_806AADA8[0] = 3;
    lbl_806EEA2C = 0;
    lbl_806EEA30 = 0;
    lbl_806EEA3C = 0;
    lbl_806EEA20 = 0;
    lbl_806EEA24 = 0;
    fn_803EFE60(&lbl_806AB4C8);
    memset(lbl_806AADFC, 0, 0xb4);
    fn_803EFE60((RcSys*)lbl_806AADFC);
    void* o = fn_803F6F50(0, 0x2d0, 0, 0, 0);
    RcIfB* t = fn_803F713C(o, (char*)self + 4);
    RcGetA* ga = *(RcGetA**)((char*)self + 0x18);
    lbl_806EEA2C = t;
    RcV12* r38 = ga->g0();
    RcGetB* gb = *(RcGetB**)((char*)self + 8);
    lbl_806EEA38 = r38;
    lbl_806EEA34 = gb->g0();
    ((RcIfAx*)lbl_806EEA34)->v3(0x4000);
    ((RcIfAx*)lbl_806EEA34)->v4((void*)fn_803EEC20);
    ((RcIfAx*)lbl_806EEA34)->v7(-2);
    RcIfB* h = lbl_806EEA2C;
    short sp8[2];
    sp8[0] = 0;
    sp8[1] = 0;
    fn_803F7048(lbl_806AAD84, h, ((int (*)(RcIfB*, short*))fn_803F7604)(h, sp8));
    return (RcIfC*)self;
}

extern "C" void fn_803EEB08(const void** self, int c)
{
    RcIfA* a = (RcIfA*)lbl_806EEA38;
    *self = lbl_8060E120;
    a->v3();
    lbl_806EEA3C = 1;
    ((RcIfA*)lbl_806EEA38)->v4();
    lbl_806EEA34->v5(0);
    lbl_806EEA34->v2();
    ((RcIfA*)lbl_806EEA38)->v2();
    if (lbl_806EEA2C != 0)
        lbl_806EEA2C->v0(3);
    fn_803F693C(self, 0);
    if (c & 1)
        fn_803F6FC4(self, 0x1c);
}

extern "C" int fn_803EEC20()
{
    if (lbl_806EEA3C == 0) {
        do {
            RcSys* t = lbl_806EEA40;
            if (t == 0 && lbl_806AADAC[0] != 0) {
                ((RcIfA*)lbl_806EEA38)->v3();
                lbl_806AB4C8.m0 = (int)t;
                lbl_806AB4C8.m4 = (int)t;
                lbl_806AB4C8.m6C = (int)t;
                lbl_806AB4C8.m70 = (int)t;
                lbl_806AB4C8.m74 = (int)t;
                lbl_806AB4C8.m78 = (int)t;
                lbl_806AB4C8.m7C = (int)t;
                lbl_806AB4C8.m80 = (int)t;
                lbl_806AB4C8.m84 = (int)t;
                lbl_806AB4C8.m88 = (int)t;
                lbl_806EEA40 = t;
                switch (((int*)lbl_806AADAC[0])[1]) {
                    case 1:
                        UpdateTaskTrcStartGame();
                        break;
                    case 2:
                        UpdateTaskTrcCardExists();
                        break;
                    case 3:
                        UpdateTaskTrcGetCardInfo();
                        break;
                    case 5:
                        fn_803F391C();
                        break;
                    case 4:
                        fn_803F2CA8();
                        break;
                    case 6:
                        fn_803F4448();
                        break;
                    case 7:
                        fn_803F4B50();
                        break;
                    case 8:
                        fn_803F4EE4();
                        break;
                    case 9:
                        fn_803F564C();
                        break;
                    case 25:
                        fn_803F5C90();
                        break;
                    case 10:
                        fn_803F13A0();
                        break;
                    case 11:
                        UpdateTaskGetCardInfo();
                        break;
                    case 12:
                        UpdateTaskMount();
                        break;
                    case 13:
                        UpdateTaskUnmount();
                        break;
                    case 16:
                        UpdateTaskOpen();
                        break;
                    case 17:
                        UpdateTaskClose();
                        break;
                    case 20:
                        fn_803F1924();
                        break;
                    case 21:
                        fn_803F1AAC();
                        break;
                    case 19:
                        UpdateTaskSeek();
                        break;
                    case 22:
                        UpdateTaskFlush();
                        break;
                    case 18:
                        UpdateTaskDelete();
                        break;
                    case 23:
                        UpdateTaskSetAttribute();
                        break;
                    case 24:
                        UpdateTaskFindFile();
                        break;
                }
                if (lbl_806AB4C8.m0 != 0)
                    lbl_806EEA40 = &lbl_806AB4C8;
                ((RcIfA*)lbl_806EEA38)->v4();
            }
            lbl_806EEA34->v6(5);
        } while (lbl_806EEA3C == 0);
    }
    return 0;
}

extern "C" void fn_803EEECC()
{
    lbl_806AAD80[0] = lbl_8060E220;
}

struct RcShort2 {
    short a, b;
};
struct RcSysV {
    int m0, m4;
    char pad8[0x2c];
    RcShort2 m34;
    char pad38[0x34];
    int m6C, m70, m74, m78, m7C, m80, m84, m88;
    const void* vp;
};
inline void RcClr(RcShort2* s)
{
    s->b = 0;
}
extern RcSysV lbl_806AACF4;
extern int lbl_806EF45C;
extern "C" int atexit(void (*)());

extern "C" RcSysV* fn_803EEEE0(int unused, int n)
{
    if (lbl_806AAD9C[0] != 0)
        lbl_806AAD9C[1] += n;
    RcSysV* ret = 0;
    if (lbl_806EF45C == 0) {
        lbl_806AACF4.m34.a = 0;
        RcClr(&lbl_806AACF4.m34);
        lbl_806AACF4.m0 = 0;
        lbl_806AACF4.vp = lbl_8060E220;
        lbl_806AACF4.m4 = 0;
        fn_803EFE60((RcSys*)&lbl_806AACF4);
        lbl_806EF45C = 1;
        atexit(fn_803EEECC);
    }
    lbl_806AACF4.m0 = 0;
    lbl_806AACF4.m4 = 0;
    lbl_806AACF4.m6C = 0;
    lbl_806AACF4.m70 = 0;
    lbl_806AACF4.m74 = 0;
    lbl_806AACF4.m78 = 0;
    lbl_806AACF4.m7C = 0;
    lbl_806AACF4.m80 = 0;
    lbl_806AACF4.m84 = 0;
    lbl_806AACF4.m88 = 0;
    if (lbl_806EEA40 != 0) {
        ((RcIfA*)lbl_806EEA38)->v3();
        const void* volatile sv = lbl_806AACF4.vp;
        lbl_806AACF4 = *(RcSysV*)lbl_806EEA40;
        lbl_806AACF4.vp = sv;
        ret = &lbl_806AACF4;
        lbl_806EEA40 = 0;
        ((RcIfA*)lbl_806EEA38)->v4();
    }
    return ret;
}

extern "C" void SendMessage(int unused, int msg, int arg)
{
    lbl_806EEA38->v3();
    lbl_806EEA30 = msg;
    switch (msg) {
    case 9:
        lbl_806EEA28 = arg == 0;
        break;
    case 8:
        lbl_806AAD9C[0] = 0;
        break;
    }
    lbl_806EEA38->v4();
}

extern "C" int fn_803EF0F0(int kind, int mask, int flag)
{
    int arr[4];
    char buf[0x80];
    if (flag != 1)
        mask &= ~0x10;
    int n = 0;
    int v = 1;
    if (mask != 0) {
        int* p = arr;
        do {
            if (mask & 1) {
                *p = v;
                n++;
                p++;
            }
            v <<= 1;
            mask >>= 1;
        } while (mask != 0);
    }
    if (kind == 10 && n > 0)
        return arr[0];
    if (kind == 11 && n > 1)
        return arr[1];
    if (kind == 12 && n > 2)
        return arr[2];
    if (kind == 13 && n > 3)
        return arr[3];
    sprintf(buf, lbl_8060E0D0, kind);
    return 0;
}

extern "C" int CheckCard()
{
    if (fn_803F8944(lbl_806EEA2C) == 0)
        return 3;
    return 0;
}

extern "C" void ClearTask()
{
    lbl_806AADAC[0] = 0;
    RcSysInit(&lbl_806AB4C8);
}

struct RcA {
    char pad0[4];
    int m4, m8, mC, m10;
    char m14[4];
    char m18[2];
};

extern "C" void fn_803EFBA4(RcA* a)
{
    a->m4 = 0;
    a->m8 = 0;
    a->mC = 0;
    a->m10 = 0;
    memset(a->m14, 0, 4);
    memset(a->m18, 0, 2);
}

struct RcB {
    char pad0[8];
    int m8;
    char padC[8];
    short m14;
    short m16;
    int m18;
    int m1C;
};

extern "C" void fn_803EFC00(RcB* b, int v)
{
    b->m14 = 0;
    b->m18 = -1;
    b->m1C = v;
    b->m8 = 0;
}

struct RcC {
    char pad0[0x20];
    int m20, m24, m28;
    char m2C[0x30];
    int m5C, m60, m64, m68, m6C, m70, m74, m78, m7C;
};

extern "C" void fn_803EFC20(RcC* c)
{
    c->m20 = 0;
    c->m24 = 0;
    c->m28 = 0;
}

extern "C" void fn_803EFC34(RcC* c)
{
    c->m20 = 0;
    c->m24 = 0;
    c->m28 = 0;
    memset(c->m2C, 0, 0x4c);
    c->m78 = 0;
}

extern "C" void fn_803EFC80(RcC* c)
{
    c->m20 = 0;
    c->m24 = 0;
    c->m28 = 0;
    memset(c->m2C, 0, 0xc);
}

extern "C" void fn_803EFCC0(RcC* c)
{
    c->m20 = 0;
    c->m24 = 0;
    c->m28 = 0;
    memset(c->m2C, 0, 0x30);
    c->m70 = 0xf;
    c->m5C = 0;
    c->m60 = 0;
    c->m64 = 0;
    c->m68 = 0;
    c->m6C = 0;
    c->m74 = 0;
    c->m78 = 0;
    c->m7C = 0;
}

extern "C" void fn_803EFD30(RcC* c)
{
    c->m20 = 0;
    c->m24 = 0;
    c->m28 = 0;
    memset(c->m2C, 0, 0x30);
    c->m24 = 0;
}

extern "C" void fn_803EFD7C(RcC* c)
{
    c->m20 = 0;
    c->m24 = 0;
    c->m28 = 0;
    memset(c->m2C, 0, 0x30);
    c->m60 = 0xf;
    c->m20 = 0;
    c->m5C = 0;
}

extern "C" void fn_803EFDD4(RcC* c)
{
    c->m20 = 0;
    c->m24 = 0;
    c->m28 = 0;
    memset(c->m2C, 0, 0x30);
    c->m60 = 0xf;
    c->m5C = 0;
    c->m64 = 0;
}

extern "C" void fn_803EFE2C(RcSys* self, int c)
{
    self->m8C = lbl_8060E220;
    if (c & 1)
        fn_80377408(self);
}

extern "C" void fn_803EFE60(RcSys* s)
{
    RcSysInit(s);
}

extern "C" int fn_803EFE90()
{
    int r = 1;
    if (lbl_806AADAC[0] == 0)
        r = 0;
    return r;
}

extern "C" void fn_803EFEAC()
{
    fn_803F7604(lbl_806EEA2C);
}

extern "C" void* fn_803EFED0()
{
    return lbl_806AAD84;
}

extern "C" void fn_803EFEDC(void** self, int c)
{
    *self = lbl_8060E358;
    if (c & 1)
        fn_80377408(self);
}

extern "C" void fn_803EFF10(RcStk* s, int* it)
{
    fn_803F009C(s, it);
    int top = (int)s->m0;
    if (top != 0) {
        int* a = (int*)((char*)s + 4);
        if (a[16] <= 0xf) {
            int n = s->m44;
            int o = n << 2;
            *(int*)((char*)a + o) = top;
            s->m44 = n + 1;
        }
    }
    s->m0 = (void*)it;
    it[2] = 0;
    it[3] = 0;
    if (fn_803F0074(s, it) != 0)
        s->m48 = s->m48 + 1;
}

extern "C" void fn_803EFFA0(RcStk* s, int* it)
{
    int* a = (int*)((char*)s + 4);
    void* v;
    int n = a[16];
    if (n > 0) {
        n = n - 1;
        a[16] = n;
        n = n << 2;
        v = *(void**)((char*)a + n);
    } else
        v = 0;
    s->m0 = v;
    if (fn_803F0074(s, it) != 0)
        s->m48 = s->m48 - 1;
}

extern "C" void fn_803F0010()
{
}

extern "C" void fn_803F0014(RcV* v, int a)
{
    v->s1();
    v->m14 = 0;
    v->m18 = -1;
    v->m1C = a;
    v->m8 = 0;
}

extern "C" int fn_803F0074(void* s, int* it)
{
    if (it != 0) {
        switch (it[1]) {
        case 1: case 2: case 3: case 4: case 5: case 6: case 7:
            return 1;
        }
    }
    return 0;
}

extern "C" void fn_803F00A4()
{
    fn_803EF23C(1, 0xffff);
}

extern "C" void fn_803F00D0()
{
    fn_803EF23C(0, 0xffff);
}
