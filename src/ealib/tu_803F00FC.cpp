/*
 * EA shared library code that uses small data (cflags_ealib_sda: -O2, default -G8):
 * VP6/MAD video codecs and SPCH, or RealCore,
 * .text 0x803F00FC-0x803F2000 (code-only unit, 34 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

struct RcLockIf {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
};

struct RcSysX {
    char pad[0x18];
    RcLockIf* m18;
};

struct RcEv;

struct RcTgt0 {
    char d[0x4c];
};

struct RcTgt : RcTgt0 {
    virtual void v0(RcEv*);
    virtual void v1(RcEv*);
};

struct RcEvBase {
    RcTgt* m0;
    int m4;
    int m8;
    int mC;
};

struct RcEv0 : RcEvBase {
    virtual void s0();
    virtual void s1();
};

struct RcEv : RcEv0 {
    union {
        short s;
        int i;
    } m14;
    int m18;
    int m1C;
    int m20;
    int m24;
    int m28;
    int m2C;
};

extern RcEv lbl_806AB158;
extern RcEv lbl_806AB17C;
extern RcEv lbl_806AB19C;
extern RcEv lbl_806AB1BC;
extern RcEv lbl_806AB1DC;
extern RcEv lbl_806AB204;
extern RcEv lbl_806AB228;
extern RcEv lbl_806AB258;
extern RcEv lbl_806AB288;
extern RcEv lbl_806AB2B4;
extern RcEv lbl_806AB1D0;

struct RcEvS : RcEv0 {
    union {
        short s;
        int i;
    } m14;
    int m18;
    int m1C;
    char m20[0x24];
    int m44;
};

extern RcEvS lbl_806AB2D8;
extern RcEvS lbl_806AB364;
extern RcEvS lbl_806AB31C;
extern RcEv lbl_806AAF2C;
extern RcEv lbl_806AAF58;

struct RcBig {
    int w[19];
};

struct RcEvB : RcEv0 {
    union {
        short s;
        int i;
    } m14;
    int m18;
    int m1C;
    int m20, m24, m28;
    RcBig m2C;
};

extern RcEvB lbl_806AAEB0;

struct RcBig12 {
    int w[12];
};

struct RcEvC : RcEv0 {
    union {
        short s;
        int i;
    } m14;
    int m18;
    int m1C;
    int m20, m24, m28;
    RcBig12 m2C;
};

extern RcEvC lbl_806AB010;

struct RcEvD : RcEvC {
    char pad5C[0x10];
    int m6C;
    char pad70[8];
    int m78;
};

extern RcEvD lbl_806AAF90;
extern RcEvD lbl_806AB0DC;
extern "C" void CheckVersion(RcSysX*);

struct RcSys {
    int m0, m4;
    char pad8[0x2c];
    int m34, m38, m3C;
    char pad40[0x14];
    int m54, m58, m5C, m60;
    char pad64[8];
    int m6C, m70, m74, m78, m7C, m80, m84, m88;
};

extern RcSys lbl_806AB4C8;
extern void* lbl_806EEA2C;
extern char lbl_806AB190[];
extern "C" int Seek(void*, int, int, int);
extern "C" int fn_803F8730(void*, void*, int*, int*);
extern int lbl_806AB170[3];
extern int lbl_806AB1B4[3];
extern "C" int fn_803F7748(void*, void*);
extern int lbl_806EEA20;
extern int lbl_806EEA24;
extern RcEv lbl_806AB228;
extern RcEv lbl_806AB258;
extern "C" int fn_803F8434(void*, int, int, int, int*);
extern "C" int fn_803F7EE4(void*, int, int, int, int*);
struct RcQ {
    void* m0;
    int m4, m8, mC, m10, m14, m18, m1C, m20, m24, m28, m2C;
    RcQ()
    {
        m0 = 0;
        m4 = 0;
        m8 = 0;
        mC = 0;
        m10 = 0;
        m14 = 0;
        m18 = 0;
        m1C = 0;
        m20 = 0;
        m24 = 0;
        m28 = 0;
        m2C = 0;
    }
};

extern "C" int fn_803F87A8(void*, void*, RcQ*);
extern int lbl_806AB330[16];
extern "C" int fn_803F8C44(void*, int*, int*, int);
extern "C" int fn_803F74E0(void*, void*);
extern "C" int fn_803F7958(void*, void*, int, int, int*);
extern "C" int FlushWriteBuffer(void*, int);
extern "C" int CloseFile(void*, int);
extern "C" int fn_803F78CC(void*, void*);
extern "C" char* strncpy(char*, const char*, unsigned);
extern RcSysX* lbl_806EEA60;
extern char lbl_8060E388[];
extern char lbl_806AB558[];
extern "C" void* memset(void*, int, unsigned);
extern "C" void* memcpy(void*, const void*, unsigned);
extern "C" unsigned short* fn_803F6AE0(int, const char*, ...);
extern "C" int fn_803F6F18(unsigned short*);
extern "C" unsigned short* fn_803F0918(const unsigned short*, unsigned short*, int);

extern "C" void TrcStartGame(RcSysX* s, RcBig* a)
{
    int one = 1;
    CheckVersion(s);
    s->m18->v3();
    RcEvB* e = &lbl_806AAEB0;
    e->s1();
    e->s1();
    e->m14.s = 0;
    e->m18 = -1;
    e->m1C = one;
    e->m8 = 0;
    e->m14.i = a->w[13];
    e->m2C = *a;
    e->m0->v0((RcEv*)e);
    s->m18->v4();
}

extern "C" void TrcCardExists(RcSysX* s, int* a)
{
    int one = 1;
    s->m18->v3();
    RcEv* e = &lbl_806AAF2C;
    e->s1();
    e->s1();
    e->m14.s = 0;
    e->m18 = -1;
    e->m1C = one;
    e->m8 = 0;
    e->m14.i = a[0];
    e->m0->v0(e);
    s->m18->v4();
}

extern "C" void TrcGetCardInfo(RcSysX* s, int* a)
{
    int one = 1;
    s->m18->v3();
    RcEv* e = &lbl_806AAF58;
    e->s1();
    e->s1();
    e->m18 = -1;
    e->m14.s = 0;
    e->m8 = 0;
    e->m1C = one;
    e->m14.i = a[0];
    memset(&e->m2C, 0, 12);
    e->m2C = a[0];
    e->m0->v0(e);
    s->m18->v4();
}

extern "C" void fn_803F0418(RcSysX* s, int* a, RcBig12* b)
{
    int one = 1;
    s->m18->v3();
    int v = a[0];
    RcEvC* e = &lbl_806AB010;
    e->s1();
    e->s1();
    e->m14.s = 0;
    e->m18 = -1;
    e->m8 = 0;
    e->m1C = one;
    e->m14.i = v;
    e->m2C = *b;
    e->m0->v0((RcEv*)e);
    s->m18->v4();
}

extern "C" void TrcSaveFile(RcSysX* s, int* a, RcBig12* b)
{
    int one = 1;
    s->m18->v3();
    int v = a[0];
    RcEvD* e = &lbl_806AAF90;
    e->s1();
    e->s1();
    e->m14.s = 0;
    e->m18 = -1;
    e->m8 = 0;
    e->m1C = one;
    e->m14.i = v;
    e->m2C = *b;
    e->m6C = e->m2C.w[0];
    e->m0->v0((RcEv*)e);
    s->m18->v4();
}

extern "C" void TrcListFiles(RcSysX* s, int* a, RcBig12* b)
{
    int one = 1;
    s->m18->v3();
    int v = a[0];
    RcEvD* e = &lbl_806AB0DC;
    e->s1();
    e->s1();
    e->m14.s = 0;
    e->m18 = -1;
    e->m8 = 0;
    e->m1C = one;
    e->m14.i = v;
    e->m2C = *b;
    e->m78 = 4;
    e->m0->v0((RcEv*)e);
    s->m18->v4();
}

extern RcEvC lbl_806AB074;

extern "C" void fn_803F07B8(RcSysX* s, int* a, RcBig12* b)
{
    int one = 1;
    s->m18->v3();
    int v = a[0];
    RcEvC* e = &lbl_806AB074;
    e->s1();
    e->s1();
    e->m14.s = 0;
    e->m18 = -1;
    e->m8 = 0;
    e->m1C = one;
    e->m14.i = v;
    e->m2C = *b;
    e->m0->v0((RcEv*)e);
    s->m18->v4();
}

extern "C" void fn_803F08E8(void* a, unsigned short* b)
{
    fn_803F0918((const unsigned short*)b, (unsigned short*)lbl_806AB558, 0x60);
}

extern "C" unsigned short* fn_803F0918(const unsigned short* a, unsigned short* buf, int n)
{
    lbl_806EEA60->m18->v3();
    memset(buf, 0, n * 2);
    union {
        unsigned short s;
        int i;
    } u;
    u.i = 0;
    u.s = 0x41;
    u.s = *a + u.s;
    unsigned short* r = fn_803F6AE0(0x21, lbl_8060E388, &u);
    int len = fn_803F6F18(r) * 2;
    if (len > n)
        len = n;
    memcpy(buf, r, len);
    lbl_806EEA60->m18->v4();
    return buf;
}

extern "C" void CardExists(RcSysX* s, int* a)
{
    s->m18->v3();
    int v = a[0];
    RcEv* e = &lbl_806AB158;
    e->m14.s = 0;
    e->m1C = 1;
    e->m18 = -1;
    e->m14.i = v;
    e->m8 = 0;
    e->m20 = 0;
    e->m0->v0(e);
    s->m18->v4();
}

extern "C" void fn_803F0AB4(RcSysX* s, int* a)
{
    s->m18->v3();
    int v = a[0];
    RcEv* e = &lbl_806AB17C;
    e->m14.s = 0;
    e->m8 = 0;
    e->m18 = -1;
    e->m1C = 1;
    e->m14.i = v;
    e->m0->v0(e);
    s->m18->v4();
}

extern "C" void fn_803F0B68(RcSysX* s, int* a)
{
    s->m18->v3();
    int v = a[0];
    RcEv* e = &lbl_806AB19C;
    e->m14.s = 0;
    e->m8 = 0;
    e->m18 = -1;
    e->m1C = 1;
    e->m14.i = v;
    e->m0->v0(e);
    s->m18->v4();
}

extern "C" void fn_803F0C1C(RcSysX* s, int* a)
{
    s->m18->v3();
    int v = a[0];
    RcEv* e = &lbl_806AB1BC;
    e->m14.s = 0;
    e->m8 = 0;
    e->m18 = -1;
    e->m1C = 1;
    e->m14.i = v;
    e->m0->v0(e);
    s->m18->v4();
}

extern "C" void OpenFile(RcSysX* s, int* a, int x, int y)
{
    s->m18->v3();
    int v = a[0];
    RcEv* e = &lbl_806AB1DC;
    e->m14.s = 0;
    e->m8 = 0;
    e->m18 = -1;
    e->m1C = 1;
    e->m14.i = v;
    e->m20 = x;
    e->m24 = y;
    e->m0->v0(e);
    s->m18->v4();
}

/* near-miss: 9 slots; retail stores m14 (short) last with its own zero register, every store order tried */
extern "C" void fn_803F0D94(RcSysX* s, int v)
{
    s->m18->v3();
    RcEv* e = &lbl_806AB204;
    e->m14.s = 0;
    e->m1C = 1;
    e->m18 = -1;
    e->m8 = 0;
    e->m20 = v;
    e->m0->v0(e);
    s->m18->v4();
}

extern "C" void fn_803F0E44(RcSysX* s, int p1, int p2, int p3)
{
    s->m18->v3();
    RcEv* e = &lbl_806AB228;
    e->m1C = 1;
    e->m14.s = 0;
    e->m18 = -1;
    e->m8 = 0;
    e->m20 = p1;
    e->m24 = p2;
    e->m28 = p3;
    e->m2C = 0;
    e->m0->v0(e);
    s->m18->v4();
}

extern "C" void fn_803F0F08(RcSysX* s, int p1, int p2, int p3)
{
    s->m18->v3();
    RcEv* e = &lbl_806AB258;
    e->m1C = 1;
    e->m14.s = 0;
    e->m18 = -1;
    e->m8 = 0;
    e->m20 = p1;
    e->m24 = p2;
    e->m28 = p3;
    e->m2C = 0;
    e->m0->v0(e);
    s->m18->v4();
}

/* near-miss: 5 slots; store order of m1C/m18/m24/m28 not found by hill-climb */
extern "C" void fn_803F0FCC(RcSysX* s, int p1, int p2, int p3)
{
    s->m18->v3();
    RcEv* e = &lbl_806AB288;
    e->m1C = 1;
    e->m18 = -1;
    e->m24 = p2;
    e->m14.s = 0;
    e->m8 = 0;
    e->m20 = p1;
    e->m28 = p3;
    e->m0->v0(e);
    s->m18->v4();
}

/* near-miss: same as fn_803F0D94 (9 slots) */
extern "C" void fn_803F108C(RcSysX* s, int v)
{
    s->m18->v3();
    RcEv* e = &lbl_806AB2B4;
    e->m14.s = 0;
    e->m1C = 1;
    e->m18 = -1;
    e->m8 = 0;
    e->m20 = v;
    e->m0->v0(e);
    s->m18->v4();
}

/* One initialized state value preserves retail scheduling of the paired 1/-1 stores. */
extern "C" void DeleteFile(RcSysX* s, int* a, int x, const char* str)
{
    int active;
    s->m18->v3();
    int v = a[0];
    RcEvS* e = &lbl_806AB2D8;
    active = 1;
    e->m14.s = 0;
    e->m18 = -active;
    e->m1C = active;
    e->m8 = 0;
    e->m14.i = v;
    strncpy(e->m20, str, 0x24);
    e->m0->v0((RcEv*)e);
    s->m18->v4();
}

extern "C" void SetFileAttribute(RcSysX* s, int* a, int x, const char* str, int w)
{
    int active;
    s->m18->v3();
    int v = a[0];
    RcEvS* e = &lbl_806AB31C;
    e->m14.s = 0;
    active = 1;
    e->m18 = -active;
    e->m1C = active;
    e->m8 = 0;
    e->m14.i = v;
    strncpy(e->m20, str, 0x24);
    e->m44 = w;
    e->m0->v0((RcEv*)e);
    s->m18->v4();
}

/* The paired state stores use the same initialized value as DeleteFile. */
extern "C" void FindFile(RcSysX* s, int* a, int x, const char* str)
{
    int active;
    s->m18->v3();
    int v = a[0];
    RcEvS* e = &lbl_806AB364;
    active = 1;
    e->m14.s = 0;
    e->m18 = -active;
    e->m1C = active;
    e->m14.i = v;
    e->m8 = 0;
    strncpy(e->m20, str, 0x24);
    e->m44 = 0;
    e->m0->v0((RcEv*)e);
    s->m18->v4();
}

/* near-miss: 96%; retail has two lis with no relocation (case 0 and case 3 arms) that no source can produce, and lays the arms out differently */
extern "C" void fn_803F13A0()
{
    int k = fn_803F74E0(lbl_806EEA2C, &lbl_806AB158.m14);
    switch (k) {
    case 0:
        lbl_806AB170[0] = k;
        break;
    case 3:
        lbl_806AB170[0] = 2;
        break;
    case 2:
        lbl_806AB170[0] = 6;
        break;
    default:
        lbl_806AB170[0] = 1;
        break;
    }
    RcEv* e = &lbl_806AB158;
    if (e->m1C != 0) {
        RcSys* y = &lbl_806AB4C8;
        y->m0 = 8;
        y->m34 = e->m14.i;
        y->m88 = 0;
        y->m4 = e->m18;
        y->m6C = 0;
        y->m70 = 0;
        y->m74 = 0;
        y->m78 = 0;
        y->m7C = 0;
        y->m80 = 0;
        y->m84 = 0;
    }
    e->m0->v1(e);
}

extern "C" void UpdateTaskGetCardInfo()
{
    int a = 0;
    int b = 0;
    int ok = 1;
    if (fn_803F8730(lbl_806EEA2C, lbl_806AB190, &a, &b) == 0)
        ok = 0;
    RcEv* e = &lbl_806AB17C;
    if (e->m1C != 0) {
        RcSys* y = &lbl_806AB4C8;
        int ra = a;
        int rb = b;
        y->m0 = 9;
        y->m3C = rb;
        y->m4 = ok;
        y->m38 = ra;
        y->m6C = 0;
        y->m70 = 0;
        y->m74 = 0;
        y->m78 = 0;
        y->m7C = 0;
        y->m80 = 0;
        y->m84 = 0;
        y->m88 = 0;
    }
    e->m0->v1(e);
}

extern "C" void UpdateTaskMount()
{
    RcEv* e = &lbl_806AB19C;
    e->m18 = 1;
    int k = fn_803F7748(lbl_806EEA2C, &e->m14);
    switch (k) {
    case 0:
        e->m18 = k;
        break;
    case 3:
        e->m18 = 2;
        break;
    case 13:
    case 6:
        lbl_806AB1B4[0] = 4;
        break;
    case 16:
        e->m18 = 5;
        break;
    case 2:
        e->m18 = 6;
        break;
    default:
        lbl_806AB1B4[0] = 1;
        break;
    }
    e = &lbl_806AB19C;
    e->m0->v1(e);
    if (e->m1C != 0) {
        int q = e->m18;
        RcSys* y = &lbl_806AB4C8;
        y->m0 = 0x15;
        y->m6C = 0;
        y->m70 = 0;
        y->m74 = 0;
        y->m78 = 0;
        y->m7C = 0;
        y->m80 = 0;
        y->m84 = 0;
        y->m88 = 0;
        y->m4 = q;
    }
}

extern "C" void UpdateTaskUnmount()
{
    int r = 1;
    int k = fn_803F78CC(lbl_806EEA2C, &lbl_806AB1D0);
    if (k == 0)
        r = 0;
    else if (k == 3)
        r = 2;
    RcEv* e = &lbl_806AB1BC;
    e->m0->v1(e);
    if (e->m1C != 0) {
        RcSys* y = &lbl_806AB4C8;
        y->m0 = 0x16;
        y->m4 = r;
        y->m6C = 0;
        y->m70 = 0;
        y->m74 = 0;
        y->m78 = 0;
        y->m7C = 0;
        y->m80 = 0;
        y->m84 = 0;
        y->m88 = 0;
    }
}

extern "C" void UpdateTaskOpen()
{
    int sp8 = 0;
    int r = 1;
    int k = fn_803F7958(lbl_806EEA2C, &lbl_806AB1DC.m14, lbl_806AB1DC.m20, lbl_806AB1DC.m24, &sp8);
    switch (k) {
    case 0:
        r = 0;
        break;
    case 4:
        r = 0xb;
        break;
    case 6:
        r = 0xe;
        break;
    case 5:
        r = 5;
        break;
    case 1:
        r = 1;
        break;
    case 7:
        r = 1;
        break;
    }
    RcEv* e = &lbl_806AB1DC;
    e->m0->v1(e);
    if (e->m1C != 0) {
        RcSys* y = &lbl_806AB4C8;
        y->m0 = 0xd;
        y->m4 = r;
        y->m6C = 0;
        y->m70 = 0;
        y->m74 = 0;
        y->m78 = 0;
        y->m7C = 0;
        y->m80 = 0;
        y->m84 = 0;
        y->m88 = 0;
        y->m58 = *(int*)e->m20;
        y->m54 = sp8;
    }
}

extern "C" void UpdateTaskClose()
{
    int k = CloseFile(lbl_806EEA2C, lbl_806AB204.m20);
    RcEv* e = &lbl_806AB204;
    e->m0->v1(e);
    if (e->m1C != 0) {
        int ok = 1;
        if (k == 0)
            ok = 0;
        RcSys* y = &lbl_806AB4C8;
        y->m0 = 0xe;
        y->m4 = ok;
        y->m6C = 0;
        y->m70 = 0;
        y->m74 = 0;
        y->m78 = 0;
        y->m7C = 0;
        y->m80 = 0;
        y->m84 = 0;
        y->m88 = 0;
    }
}

extern "C" void fn_803F1924()
{
    int sp8 = 0;
    int r;
    int k = fn_803F8434(lbl_806EEA2C, lbl_806AB228.m20, lbl_806AB228.m24, lbl_806AB228.m28, &sp8);
    switch (k) {
    case 0:
        r = 0;
        break;
    case 2:
        r = 6;
        break;
    case 3:
        r = 2;
        break;
    case 4:
        r = 0xa;
        break;
    case 5:
        r = 5;
        break;
    case 6:
        r = 0xe;
        break;
    case 8:
        r = 7;
        break;
    case 9:
        r = 9;
        break;
    case 10:
        r = 8;
        break;
    case 11:
        r = 0x17;
        break;
    case 12:
    default:
        r = 1;
        break;
    }
    if (r != 0)
        lbl_806EEA20 = 1;
    RcEv* e = &lbl_806AB228;
    if (e->m1C != 0) {
        int q = sp8;
        RcSys* y = &lbl_806AB4C8;
        y->m0 = 0x12;
        y->m6C = 0;
        y->m70 = 0;
        y->m74 = 0;
        y->m78 = 0;
        y->m7C = 0;
        y->m80 = 0;
        y->m84 = 0;
        y->m88 = 0;
        y->m4 = r;
        y->m5C = q;
    }
    e->m0->v1(e);
}

extern "C" void fn_803F1AAC()
{
    int sp8 = 0;
    int r;
    int k = fn_803F7EE4(lbl_806EEA2C, lbl_806AB258.m20, lbl_806AB258.m24, lbl_806AB258.m28, &sp8);
    switch (k) {
    case 0:
        r = 0;
        break;
    case 2:
        r = 6;
        break;
    case 3:
        r = 2;
        break;
    case 4:
        r = 0xa;
        break;
    case 5:
        r = 5;
        break;
    case 6:
        r = 0xe;
        break;
    case 8:
        r = 7;
        break;
    case 9:
        r = 9;
        break;
    case 10:
        r = 8;
        break;
    case 11:
        r = 0x17;
        break;
    case 12:
    default:
        r = 1;
        break;
    }
    if (r != 0)
        lbl_806EEA24 = 1;
    RcEv* e = &lbl_806AB258;
    if (e->m1C != 0) {
        int q = sp8;
        RcSys* y = &lbl_806AB4C8;
        y->m0 = 0x13;
        y->m6C = 0;
        y->m70 = 0;
        y->m74 = 0;
        y->m78 = 0;
        y->m7C = 0;
        y->m80 = 0;
        y->m84 = 0;
        y->m88 = 0;
        y->m4 = r;
        y->m60 = q;
    }
    e->m0->v1(e);
}

extern "C" void UpdateTaskSeek()
{
    int r = 1;
    int k = Seek(lbl_806EEA2C, lbl_806AB288.m20, lbl_806AB288.m24, lbl_806AB288.m28);
    switch (k) {
    case 0:
        r = 0;
        break;
    case 0xb:
        r = 0x17;
        break;
    }
    RcEv* e = &lbl_806AB288;
    if (e->m1C != 0) {
        RcSys* y = &lbl_806AB4C8;
        y->m0 = 0x11;
        y->m4 = r;
        y->m6C = 0;
        y->m70 = 0;
        y->m74 = 0;
        y->m78 = 0;
        y->m7C = 0;
        y->m80 = 0;
        y->m84 = 0;
        y->m88 = 0;
    }
    e->m0->v1(e);
}

extern "C" void UpdateTaskFlush()
{
    RcEv* e = &lbl_806AB2B4;
    int ok = 1;
    if (FlushWriteBuffer(lbl_806EEA2C, e->m20) == 0)
        ok = 0;
    e->m0->v1(e);
    if (e->m1C != 0) {
        RcSys* y = &lbl_806AB4C8;
        y->m0 = 0x14;
        y->m4 = ok;
        y->m6C = 0;
        y->m70 = 0;
        y->m74 = 0;
        y->m78 = 0;
        y->m7C = 0;
        y->m80 = 0;
        y->m84 = 0;
        y->m88 = 0;
    }
}

extern "C" void UpdateTaskDelete()
{
    RcQ q;
    memset(&q, 0, 0x30);
    q.m0 = lbl_806AB2D8.m20;
    int k = fn_803F87A8(lbl_806EEA2C, &lbl_806AB2D8.m14, &q);
    int r;
    switch (k) {
    case 0:
        r = 0;
        break;
    case 3:
        r = 2;
        break;
    case 5:
        r = 5;
        break;
    case 10:
        r = 8;
        break;
    case 4:
        r = 0xb;
        break;
    case 11:
    default:
        r = 1;
        break;
    }
    RcEvS* e = &lbl_806AB2D8;
    e->m0->v1((RcEv*)e);
    if (e->m1C != 0) {
        RcSys* y = &lbl_806AB4C8;
        y->m0 = 0xf;
        y->m4 = r;
        y->m6C = 0;
        y->m70 = 0;
        y->m74 = 0;
        y->m78 = 0;
        y->m7C = 0;
        y->m80 = 0;
        y->m84 = 0;
        y->m88 = 0;
    }
}

extern "C" void UpdateTaskSetAttribute()
{
    int sp8 = lbl_806AB330[0];
    int k = fn_803F8C44(lbl_806EEA2C, &sp8, &lbl_806AB330[3], lbl_806AB330[12]);
    int r;
    switch (k) {
    case 0:
        r = 0;
        break;
    case 3:
        r = 2;
        break;
    case 4:
        r = 0xb;
        break;
    case 10:
        r = 8;
        break;
    default:
        r = 1;
        break;
    }
    RcEvS* e = &lbl_806AB31C;
    if (e->m1C != 0) {
        RcSys* y = &lbl_806AB4C8;
        y->m0 = 0x10;
        y->m4 = r;
        y->m6C = 0;
        y->m70 = 0;
        y->m74 = 0;
        y->m78 = 0;
        y->m7C = 0;
        y->m80 = 0;
        y->m84 = 0;
        y->m88 = 0;
    }
    e->m0->v1((RcEv*)e);
}
