/*
 * Translation unit at 0x800057D8 (original file name unknown; placeholder).
 * A real TU: it ends with its static-initializer wrapper (docs/UNITS.md
 * ctor anchors). 37 functions.
 *
 * .text 0x800057D8-0x80008540, .rodata 0x805A344C-0x805A3A54, .data 0x8060F720-0x80615848, .bss 0x806869B0-0x80686A4C.
 * Not split (declare extern if used): small-data (.sdata/.sbss) objects and
 * any second, non-contiguous range of a section.
 *
 * Contents: one class (ctor 0x800058BC, vtable 0x806492A8, 0x254 bytes) derived
 * from the base built at 0x80170518. It is a message-driven state machine
 * ("TRC" messages, save files and blocks per the format strings): the current
 * state is a pointer to member function at +0x144, run by fn_80005C8C. The
 * rest of the unit is thirteen static Vec3 direction vectors with their
 * static-init pair. Names are placeholders (no string names the class); its
 * virtual functions are named after the retail vtable entries.
 *
 * Note: rodata 0x805A3450..0x805A368C (0x23C bytes: "soa allocator NEW",
 * geomlib assert paths, "EAGL::..." names, "% .3f" formats) has no reference
 * in this unit; it looks like another TU's strings inside this unit's split.
 */
#include "game/tu_800057D8.h"

extern "C" {
void* fn_8040B320(void* dst, int c, unsigned n);       /* behaves like memset */
char* fn_8040B61C(char* dst, const char* src);         /* behaves like strcpy */
int fn_8040B6A0(const char* s);                        /* behaves like strlen */
char* fn_8040B830(char* dst, const char* src, unsigned n); /* behaves like strncpy */
char* fn_8040B948(const char* hay, const char* needle); /* behaves like strstr */
char* fn_8040B4D8(const char* s, int c);               /* behaves like strchr */
int fn_8040B574(const char* a, const char* b);         /* behaves like strcmp */
int fn_804089B4(char* dst, const char* fmt, ...);      /* behaves like sprintf */
void Unicode2Ascii(char* dst, const void* src);
Unk80170034_Entry* fn_80170034(Unk80170034_Info* info, int idx);
void fn_8017008C(Unk80170034_Info* info, const char* name);
void OSResetSystem(int a, int b, int c);
void fn_80402EF8(void* dst, const char* src);
void fn_80403144(void* dst, const void* src);
int fn_80403060(const unsigned short* s);
void* fn_803774A8(unsigned n, int heap);
void fn_80170158(Unk80170034_Info* info, unsigned short* ws);
void fn_801701B4(Unk80170034_Info* info, unsigned short* ws, int a);
int TIMER_elapsedticks();
int fn_80400E84();
void THREAD_yield(int a);
int fn_8017037C(Unk80170034_Info* info);
int fn_80170A1C(void* self);
}
extern int lbl_8061C668;

struct Unk8060F740 {
    char pad0[0x28];
    float m28;
};
extern Unk8060F740 lbl_8060F740;
extern Unk8060F740* lbl_806EF98C;
extern int lbl_8060F720;
extern const char* lbl_80615840;
extern const char* lbl_80615844;
extern const char lbl_805A3800[];
extern char* lbl_806EEA1C;

extern "C" Unk8060F740* FONT_create(Unk8060F740*);

extern "C" Unk8060F740* fn_800057D8()
{
    return &lbl_8060F740;
}

extern "C" Unk8060F740* fn_800057E4()
{
    return lbl_806EF98C;
}

extern "C" void fn_800057F0()
{
    Unk8060F740* p = FONT_create(&lbl_8060F740);
    lbl_806EF98C = p;
    float f = 0.0f;
    p->m28 = f;
    lbl_8060F720 = 1;
}

extern "C" Unk800058BC* fn_80005840(int mode)
{
    return new (0x100) Unk800058BC(mode);
}

extern "C" void fn_8000587C(Unk800058BC* p)
{
    delete p;
}

Unk800058BC::Unk800058BC(int mode)
    : Unk80170518(mode), m14C(0), m150(0), m154(0), m1FC(0), m200(0), m244(0), m248(0), m24C(0), m250(0)
{
    m140.m0 = mode;
    m144 = 0;
    fn_8040B320(&m158, 0, 0x30);
    m158.m0 = 1;
    m158.m4 = 0;
    m158.m28 = 0;
    fn_8040B320(&m188, 0, 8);
    m188.m4 = 0;
    m188.m0 = 0;
    fn_8040B61C(m190, lbl_805A3800);
    fn_8040B61C(m1AB, lbl_80615840);
    m110.mC = m190;
    m110.m10 = fn_8040B6A0(m190);
    m110.m14 = m1AB;
    m110.m18 = fn_8040B6A0(m1AB);
    m110.m1C = &m158;
    m110.m20 = &m188;
    m110.m2C = 1;
    m34 = -1;
}

Unk800058BC::~Unk800058BC()
{
}

void Unk800058BC::fn_80005A50()
{
    OSResetSystem(1, 1, 1);
}

void Unk800058BC::fn_80005A7C(Unk80170034_Info* info)
{
    fn_80005AB0(info);
}

void Unk800058BC::fn_80005AB0(Unk80170034_Info* info)
{
    char a[16];
    char b[16];
    char c[32];
    Unicode2Ascii(a, info->m78);
    Unicode2Ascii(b, info->m8E);
    fn_804089B4(c, "%s_%s%s", b, a, lbl_80615844);
    char* p = fn_8040B4D8(b, 0x20);
    if (p) {
        *p = 0;
    }
    fn_8040B61C(m190, lbl_805A3800);
    fn_804089B4(m1AB, lbl_80615840, b, a);
    fn_8017008C(info, c);
}

int Unk800058BC::fn_80005B74(int a)
{
    return a <= 0;
}

void Unk800058BC::fn_80005B88(Unk80170034_Info* info)
{
    Unk80005B88_Obj* o = m64->vslot26();
    m110.m0 = lbl_806EEA1C;
    Unk80170034_Entry* e0 = fn_80170034(info, 0);
    m110.m4 = e0->m4 + fn_80170034(info, 1)->m4;
    m158.m4 = fn_80170034(info, 2)->m0 + 0x40;
    m188.m4 = fn_80170034(info, 3)->m0 + 0x40;
    o->vslot1();
    o->vslot3(&m140, &m110);
    o->vslot2();
}

void Unk800058BC::fn_80005C8C()
{
    if (m144) {
        m30 = 0;
        (this->*m144)();
        if (m30 != 8) {
            m30 = m144 ? 2 : 0;
        }
    } else {
        m30 = 0;
    }
}

void Unk800058BC::fn_80005D40()
{
    fn_8040B830(mD8, m38->mE4, 0x1B);
    m44 = fn_80170034(m38, 0)->m0;
    Unk80170034_Entry* e = fn_80170034(m38, 0);
    m48 = e->m4 + fn_80170034(m38, 1)->m4;
    m200 = 1;
    m158.m4 = fn_80170034(m38, 2)->m0 + 0x40;
    m188.m4 = fn_80170034(m38, 3)->m0 + 0x40;
    m110.m10 = fn_8040B6A0(m190);
    m110.m18 = fn_8040B6A0(m1AB);
    m110.m8 = m68;
    m110.m24 = m38->m8E;
    m110.m28 = m38->m78;
    m110.m0 = mD8;
    m110.m4 = m48;
}

void Unk800058BC::fn_80005E38()
{
    fn_80005AB0(m38);
    fn_8040B61C(mD8, m38->mE4);
}

int Unk800058BC::fn_80005E8C()
{
    int done = 0;
    m64->vslot10(&m140);
    mC0 = 0;
    do {
        Unk800074BC_Msg* msg = m64->vslot21(10000);
        if (msg) {
            if (msg->m0 == 8) {
                done = 1;
                int v = 0;
                if (msg->m4 == 0 || msg->m4 == 3) {
                    v = 1;
                }
                mC0 = v;
            }
        } else {
            THREAD_yield(1);
        }
    } while (!done);
    return mC0;
}

void Unk800058BC::fn_80005F4C(int a, int b, int clear)
{
    Unk80005B88_Obj* o = m64->vslot26();
    if (clear) {
        for (int i = 0; i < 5; i++) {
            m204[i].m0 = 0;
            m204[i].m4 = 0;
        }
        m154 = 0;
        m240 = 0;
    }
    o->vslot1();
    m158.m4 = fn_80170034(m38, 2)->m0 + 0x40;
    m188.m4 = fn_80170034(m38, 3)->m0 + 0x40;
    m110.m4 = b;
    m110.m0 = lbl_806EEA1C;
    o->vslot3(&m140, &m110);
    m240 += o->vslot2();
    m204[m154].m8 = o->vslot2();
    m204[m154++].m0 = a;
}

void Unk800058BC::fn_800060B4()
{
    Unk800060B4_Req req;
    req.m38 = 0;
    req.m34 = m140;
    req.m8 = m68;
    req.m0 = lbl_806EEA1C;
    req.m48 = 0;
    req.m30 = m240;
    req.m4 = 0;
    m24C = 0;
    req.m40 = m154;
    req.m2C = 1;
    m244 = 1;
    TIMER_elapsedticks();
    m64->vslot3(&req);
    m144 = &Unk800058BC::fn_800061C8;
}

void Unk800058BC::fn_800061C8()
{
    if (m144) {
        int ms = TIMER_elapsedticks() * 1000 / fn_80400E84();
        Unk800074BC_Msg* msg = m64->vslot21(ms);
        if (msg) {
            if (m1FC) {
                delete m1FC;
                m1FC = 0;
            }
            switch (msg->m0) {
            case 0x18:
                fn_800074BC(msg);
                break;
            case 0x17: {
                char a[0x20];
                char b[0x10];
                int found = 0;
                fn_8040B61C(a, msg->m40);
                char* p = fn_8040B948(a, "_");
                if (p) {
                    *p = 0;
                    for (int i = 0; i < m154; i++) {
                        Unicode2Ascii(b, (const void*)m204[i].m0);
                        if (fn_8040B574(a, b) == 0) {
                            if (m204[i].m4 == 0) {
                                m204[i].m4 = 1;
                                if (fn_8040B574(b, "Options") == 0) {
                                    m24C = 1;
                                }
                            }
                            break;
                        }
                    }
                }
                if (found) {
                    m64->vslot22(10, 0);
                } else {
                    m64->vslot22(11, 0);
                }
                break;
            }
            case 1: {
                mA0 = 0;
                m144 = 0;
                for (int i = 0; i < 4; i++) {
                    mA4[i] = 0;
                }
                m244 = 1;
                int code = msg->m4;
                if (code == 0x18 || code == 3) {
                    fn_800060B4();
                } else if (code == 0x1A) {
                    fn_80005A50();
                } else if (m248) {
                    if (m24C) {
                        fn_80006B60();
                    } else {
                        mC4 = 0;
                    }
                } else {
                    mC4 = code == 0;
                }
                break;
            }
            }
        } else {
            THREAD_yield(1);
        }
    }
}

void Unk800058BC::fn_8000643C()
{
    m64->vslot5(&m140);
    TIMER_elapsedticks();
    m144 = &Unk800058BC::fn_800064A0;
}

void Unk800058BC::fn_800064A0()
{
    if (m144) {
        int ms = TIMER_elapsedticks() * 1000 / fn_80400E84();
        Unk800074BC_Msg* msg = m64->vslot21(ms);
        if (msg) {
            switch (msg->m0) {
            case 0x18:
                fn_800074BC(msg);
                break;
            case 3: {
                m34 = msg->m38;
                m144 = 0;
                mA0 = 0;
                for (int i = 0; i < 4; i++) {
                    mA4[i] = 0;
                }
                int code = msg->m4;
                if (code == 0x18 || code == 3) {
                    fn_8000643C();
                } else {
                    mC4 = code == 0;
                }
                break;
            }
            }
        } else {
            THREAD_yield(1);
        }
    }
}

void Unk800058BC::fn_800065B4()
{
    m110.m0 = lbl_806EEA1C;
    m64->vslot9(&m140, &m110);
    TIMER_elapsedticks();
    m150 = 0;
    m144 = &Unk800058BC::fn_80006630;
}

void Unk800058BC::fn_80006630()
{
    if (m144) {
        int ms = TIMER_elapsedticks() * 1000 / fn_80400E84();
        Unk800074BC_Msg* msg = m64->vslot21(ms);
        if (msg) {
            switch (msg->m0) {
            case 0x18:
                fn_800074BC(msg);
                break;
            case 0x19:
                if (fn_8040B948(msg->m40, m50)) {
                    if (m150 < m40) {
                        fn_8017008C(m3C[m150], msg->m40);
                        m3C[m150]->m0 = 1;
                        m3C[m150]->m120 = msg->m48;
                        m150++;
                    } else {
                        m30 = 8;
                    }
                }
                break;
            case 7: {
                mA0 = 0;
                m144 = 0;
                for (int i = 0; i < 4; i++) {
                    mA4[i] = 0;
                }
                int code = msg->m4;
                if (code == 0x18 || code == 3) {
                    fn_800065B4();
                } else {
                    int v = 0;
                    if (code == 0xB || code == 0) {
                        v = 1;
                    }
                    mC4 = v;
                    if (v) {
                        fn_800067FC();
                    }
                }
                break;
            }
            }
        } else {
            THREAD_yield(1);
        }
    }
}

void Unk800058BC::fn_800067FC()
{
    char buf[0x20];
    unsigned short ws1[0x1C];
    unsigned short ws2[0x10];
    int i;
    for (i = 0; i < m150; i++) {
        for (int j = 0; j < m150 - i - 1; j++) {
            unsigned x = m3C[j]->m120;
            unsigned y = m3C[j + 1]->m120;
            if (x < y) {
                fn_8040B61C(buf, m3C[j]->mE4);
                fn_8017008C(m3C[j], m3C[j + 1]->mE4);
                m3C[j]->m120 = y;
                fn_8017008C(m3C[j + 1], buf);
                m3C[j + 1]->m120 = x;
            }
        }
    }
    for (i = 0; i < m150; i++) {
        fn_8040B61C(buf, m3C[i]->mE4);
        char* p = fn_8040B948(buf, "_");
        if (p) {
            fn_8040B61C(buf, p + 1);
        }
        buf[fn_8040B6A0(buf) - fn_8040B6A0(lbl_80615844)] = 0;
        fn_80402EF8(ws1, buf);
        fn_80170158(m3C[i], ws1);
        fn_80402EF8(ws2, m50);
        fn_801701B4(m3C[i], ws2, m60);
    }
}

void Unk800058BC::fn_800069B4()
{
    fn_80005D40();
    m64->vslot8(&m140, &m110);
    TIMER_elapsedticks();
    m144 = &Unk800058BC::fn_80006A2C;
    lbl_8061C668 = 0;
}

void Unk800058BC::fn_80006A2C()
{
    if (m144) {
        int ms = TIMER_elapsedticks() * 1000 / fn_80400E84();
        Unk800074BC_Msg* msg = m64->vslot21(ms);
        lbl_8061C668 = mBC;
        if (msg) {
            switch (msg->m0) {
            case 0x18:
                fn_800074BC(msg);
                break;
            case 6: {
                mA0 = 0;
                m144 = 0;
                for (int i = 0; i < 4; i++) {
                    mA4[i] = 0;
                }
                lbl_8061C668 = 1;
                m200 = 0;
                int code = msg->m4;
                if (code == 0x18 || code == 3) {
                    fn_800069B4();
                } else {
                    int v = 1;
                    if (code == 0x14) {
                        v = 0;
                    }
                    mC4 = v;
                }
                break;
            }
            }
        } else {
            THREAD_yield(1);
        }
    }
}

void Unk800058BC::fn_80006B60()
{
    fn_80005D40();
    m64->vslot6(&m140, &m110);
    if (m244) {
        m248 = 1;
        for (int i = 0; i < m154; i++) {
            m204[i].m4 = 0;
        }
    }
    TIMER_elapsedticks();
    m144 = &Unk800058BC::fn_80006C14;
}

void Unk800058BC::fn_80006C14()
{
    if (m144) {
        Unk800074BC_Msg* msg = m64->vslot21(TIMER_elapsedticks() * 1000 / fn_80400E84());
        if (msg) {
            switch (msg->m0) {
            case 0x18:
                fn_800074BC(msg);
                break;
            case 13:
                m14C = msg->m54;
                break;
            case 0x10:
                break;
            case 0x1A:
                m200 = 1;
                m44 = fn_80170034(m38, 0)->m0;
                m48 = fn_80170034(m38, 0)->m4;
                m64->vslot15(m14C, m44, m48);
                break;
            case 0x12:
                if (m200 == 2) {
                    m64->vslot22(2, 0);
                } else {
                    m200 = 2;
                    m44 = fn_80170034(m38, 1)->m0;
                    m48 = fn_80170034(m38, 1)->m4;
                    m64->vslot15(m14C, m44, m48);
                }
                break;
            case 0x1C:
                if (m200 != 2 || fn_80170A1C(this) != 0 || m200 != 2) {
                    m64->vslot22(7, 0);
                } else {
                    m64->vslot22(6, 0);
                }
                break;
            case 4: {
                mA0 = 0;
                m144 = 0;
                for (int i = 0; i < 4; i++) {
                    mA4[i] = 0;
                }
                m200 = 0;
                int code = msg->m4;
                if (code == 0x18 || code == 3) {
                    if (m248 == 0) {
                        fn_80006B60();
                    } else {
                        fn_800060B4();
                    }
                } else {
                    int t = m250;
                    mC4 = code == 0;
                    if (t) {
                        m244 = t;
                        m250 = 0;
                    }
                }
                break;
            }
            }
        } else {
            THREAD_yield(1);
        }
    }
}

void Unk800058BC::fn_80006E94()
{
    fn_80005D40();
    fn_8017037C(m38);
    m64->vslot7(&m140, &m110);
    TIMER_elapsedticks();
    m144 = &Unk800058BC::fn_80006F60;
    lbl_8061C668 = 0;
    Unk80170034_Entry* e = fn_80170034(m38, 0);
    int sum = e->m4 + fn_80170034(m38, 1)->m4;
    fn_80005F4C((int)m38->m8E, sum, 1);
}

void Unk800058BC::fn_80006F60()
{
    if (m144) {
        int ms = TIMER_elapsedticks() * 1000 / fn_80400E84();
        Unk800074BC_Msg* msg = m64->vslot21(ms);
        lbl_8061C668 = mBC;
        if (msg) {
            if (m1FC) {
                delete m1FC;
                m1FC = 0;
            }
            switch (msg->m0) {
            case 0x18:
                fn_800074BC(msg);
                break;
            case 13:
                m14C = msg->m54;
                break;
            case 0x1B:
                m200 = 1;
                m44 = fn_80170034(m38, 0)->m0;
                m48 = fn_80170034(m38, 0)->m4;
                m64->vslot16(m14C, m44, m48);
                break;
            case 0x13:
                if (m200 == 2) {
                    m64->vslot22(1, 0);
                } else {
                    m200 = 2;
                    m44 = fn_80170034(m38, 1)->m0;
                    m48 = fn_80170034(m38, 1)->m4;
                    m64->vslot16(m14C, m44, m48);
                }
                break;
            case 5: {
                mA0 = 0;
                m144 = 0;
                for (int i = 0; i < 4; i++) {
                    mA4[i] = 0;
                }
                m200 = 0;
                lbl_8061C668 = 1;
                int code = msg->m4;
                if (code == 0x18 || code == 3) {
                    fn_80006E94();
                } else {
                    mC4 = code == 0;
                }
                break;
            }
            }
        } else {
            THREAD_yield(1);
        }
    }
}

void Unk800058BC::fn_80007184(Unk800074BC_Msg* msg)
{
    mA0 = msg->mC;
    int* p = mA4;
    mA4[0] = msg->m10 > 0 ? msg->m24 : 0;
    p[1] = msg->m10 > 1 ? msg->m28 : 0;
    p[2] = msg->m10 > 2 ? msg->m2C : 0;
    p[3] = msg->m10 > 3 ? msg->m30 : 0;
}

void Unk800058BC::fn_800071F4(Unk800074BC_Msg* msg)
{
    char a[0x190];
    char b[0x20];
    int i = 0;
    Unicode2Ascii(a, (const void*)msg->mC);
    for (i = 0; i < m154;) {
        int n = m204[i].m8;
        Unicode2Ascii(b, (const void*)m204[i].m0);
        i++;
        fn_804089B4(a, "%s \\n%s requires 1 file and %i blocks.", a, b, n);
    }
    fn_804089B4(a, "%s \\nExisting files may be loaded and overwritten.", a);
    m1FC = fn_803774A8((fn_8040B6A0(a) + 1) * 2, 0x100);
    fn_80402EF8(m1FC, a);
    mA0 = (int)m1FC;
    int* p = mA4;
    mA4[0] = msg->m10 > 0 ? msg->m24 : 0;
    p[1] = msg->m10 > 1 ? msg->m28 : 0;
    p[2] = msg->m10 > 2 ? msg->m2C : 0;
    p[3] = msg->m10 > 3 ? msg->m30 : 0;
}

void Unk800058BC::fn_8000733C(Unk800074BC_Msg* msg)
{
    unsigned short w[0x190];
    int found = 0;
    fn_80403144(w, (const void*)msg->mC);
    int n = fn_80403060(w);
    unsigned short* s = (unsigned short*)m204[0].m0;
    if (s[0] == 0x4F && s[1] == 0x70) {
        for (int i = 0; i < n; i++) {
            if (w[i] == 0x35) {
                i += 2;
                for (int j = i; j < n; j++) {
                    unsigned short c = w[j];
                    if (c == 0x4D) {
                        found = 1;
                    }
                    if (found) {
                        w[i++] = c;
                    }
                }
                w[i] = 0;
                break;
            }
        }
    }
    m1FC = fn_803774A8((fn_80403060(w) + 1) * 2, 0x100);
    fn_80403144(m1FC, w);
    mA0 = (int)m1FC;
    int* p = mA4;
    mA4[0] = msg->m10 > 0 ? msg->m24 : 0;
    p[1] = msg->m10 > 1 ? msg->m28 : 0;
    p[2] = msg->m10 > 2 ? msg->m2C : 0;
    p[3] = msg->m10 > 3 ? msg->m30 : 0;
}

void Unk800058BC::fn_800074BC(Unk800074BC_Msg* msg)
{
    char buf[0x20];
    if (msg && msg->m0 == 0x18) {
        switch (msg->m8) {
        case 0x13:
            fn_800071F4(msg);
            break;
        case 0x14:
            fn_8000733C(msg);
            break;
        default:
            fn_80007184(msg);
            break;
        }
        switch (msg->m8) {
        case 0x11:
        case 0x12:
        case 0x13:
            mBC = 1;
            for (int i = 0; i < m154; i++) {
                m204[i].m4 = 0;
            }
            break;
        case 0x19:
            if (m248) {
                m64->vslot22(0xA, 0);
                mA0 = 0;
            } else {
                mBC = 1;
            }
            break;
        case 0xD:
        case 0xE:
        case 0x10:
            if (m244 != 0 && m144 == &Unk800058BC::fn_800064A0) {
                m64->vslot22(0xB, 0);
                mA0 = 0;
            } else {
                mBC = 1;
            }
            break;
        case 0xF:
            if (m244 != 0 && m144 == &Unk800058BC::fn_800064A0) {
                m64->vslot22(0xC, 0);
                mA0 = 0;
            } else {
                mBC = 1;
            }
            break;
        case 0x14:
        case 0x15:
        case 0x17:
        case 0x18:
        case 0x1B:
        case 0x1C:
        case 0x1E:
        case 0x20:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
            mBC = 1;
            break;
        case 0x16:
        case 0x1A:
        case 0x1D:
        case 0x1F:
            break;
        default:
            fn_804089B4(buf, "Unhandled TRC msg: %d
", msg->m8);
            break;
        }
    }
}

/*
 * Static initializers: thirteen unit direction vectors (global Vec3 objects at
 * 0x806869B0..0x80686A4C) built from five constant points (rodata 0x805A37C4:
 * four in an array, the fifth at 0x805A37F4). The helpers are the inline
 * vector operations, shaped to reproduce retail's stack temporaries
 * (-ffloat-store gives every float local and by-value struct its own slot).
 */
struct Vec3 {
    float x, y, z;
};
extern "C" float fn_804028E4(float x);
extern const Vec3 lbl_806055AC;
const Vec3 lbl_805A37C4[4] = { { 7.5f, 0.0f, 1072.5f }, { 1080.0f, 0.0f, 1080.0f }, { 1072.5f, 0.0f, 7.5f }, { 7.5f, 0.0f, 7.5f } };
const Vec3 lbl_805A37F4 = { 513.3595f, 0.0f, 513.3595f };

inline Vec3 operator-(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}

inline Vec3 operator/(const Vec3& a, float s)
{
    Vec3 r;
    float inv = 1.0f / s;
    r.x = a.x * inv;
    r.y = a.y * inv;
    r.z = a.z * inv;
    return r;
}

inline Vec3 Normalized(const Vec3& v)
{
    float len = fn_804028E4(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len != 0.0f) {
        return v / len;
    }
    return lbl_806055AC;
}

Vec3 lbl_806869B0 = Normalized(lbl_805A37F4 - lbl_805A37C4[3]);
Vec3 lbl_806869BC = Normalized(lbl_805A37C4[3] - lbl_805A37F4);
Vec3 lbl_806869C8 = Normalized(lbl_805A37C4[2] - lbl_805A37C4[1]);
Vec3 lbl_806869D4 = Normalized(lbl_805A37C4[1] - lbl_805A37C4[2]);
Vec3 lbl_806869E0 = Normalized(lbl_805A37C4[1] - lbl_805A37C4[0]);
Vec3 lbl_806869EC = Normalized(lbl_805A37C4[0] - lbl_805A37C4[1]);
Vec3 lbl_806869F8 = Normalized(lbl_805A37C4[2] - lbl_805A37C4[3]);
Vec3 lbl_80686A04 = Normalized(lbl_805A37C4[3] - lbl_805A37C4[2]);
Vec3 lbl_80686A10 = Normalized(lbl_805A37C4[3] - lbl_805A37C4[1]);
Vec3 lbl_80686A1C = Normalized(lbl_805A37C4[0] - lbl_805A37C4[3]);
Vec3 lbl_80686A28 = Normalized(lbl_805A37C4[3] - lbl_805A37C4[0]);
Vec3 lbl_80686A34 = Normalized(lbl_805A37C4[2] - lbl_805A37C4[0]);
Vec3 lbl_80686A40 = Normalized(lbl_805A37C4[0] - lbl_805A37C4[2]);
