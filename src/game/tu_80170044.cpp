/*
 * Game code, .text 0x80170044-0x80171A10 (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_80170044.h"

extern int lbl_8068B87C;
extern int lbl_8061977C;
extern const Unk80170518_Fn lbl_805CB054;
extern const Unk80170518_Fn lbl_805CB05C;
extern const Unk80170518_Fn lbl_805CB064;
extern const Unk80170518_Fn lbl_805CB06C;
extern const Unk80170518_Fn lbl_805CB074;
extern const Unk80170518_Fn lbl_805CB07C;
extern char lbl_8065EDB8[];
extern char lbl_8065ED60[];
struct Unk80170488_T {
    void* m0;
    int m4;
};

extern Unk80170488_T lbl_8068B884;

struct Unk801704C0_T {
    void* m0;
    char pad4[0x10];
    int m14;
    char pad18[0x330 - 0x18];
    int m330;
};

extern Unk801704C0_T lbl_8068B8AC;

int Unk8016FF40::fn_80170044(int x)
{
    if (m0 == 0) {
        return 0;
    }
    if (x != 0) {
        if (x == m74) {
            return 1;
        }
    } else if (m4 != 0) {
        return 1;
    }
    return 0;
}

void fn_8017008C(Unk8016FF40* info, const char* name)
{
    char* p = info->mE4;
    fn_8040074C(p, 0x1B);
    if (name) {
        fn_8040B61C(p, name);
    }
}

void fn_801700D8(Unk8016FF40* info, unsigned short* ws)
{
    unsigned short* a = info->mAE;
    unsigned short* b = info->m18;
    fn_8040074C(a, 0x36);
    fn_8040074C(b, 0x36);
    if (ws) {
        fn_8040316C(a, ws, 0x1B);
        fn_8040316C(b, ws, 0x1B);
        info->mAE[0x1A] = 0;
        info->m18[0x1A] = 0;
    }
}

void fn_80170158(Unk8016FF40* info, unsigned short* ws)
{
    unsigned short* p = info->m78;
    fn_8040074C(p, 0x16);
    if (ws) {
        fn_8040316C(p, ws, 0xB);
        info->m78[10] = 0;
    }
}

void fn_801701B4(Unk8016FF40* info, unsigned short* ws, int a)
{
    unsigned short* p = info->m8E;
    fn_8040074C(p, 0x20);
    if (ws) {
        fn_8040316C(p, ws, 0x10);
        info->m74 = a;
        info->m8E[15] = 0;
    }
}

int Unk8016FF40::fn_80170218(Unk8016FF40* other)
{
    return fn_8040B574(mE4, other->mE4) == 0;
}

int Unk8016FF40::fn_80170248()
{
    for (int i = 0; i < 4; i++) {
        if (m8[i] == -1) {
            return 1;
        }
    }
    if (m0 != 0 && m4 != 0 && m74 == 0) {
        return 1;
    }
    return 0;
}

void Unk8016FF40::fn_801702A4()
{
    int check;
    Unk8016FF40_Entry* e0 = m108[0];
    int base = e0->m0 + 0x10;
    int size = e0->m4 - 0x10;
    for (int i = 0; i < 4; i++) {
        Unk8016FF40_Entry* e = m108[i];
        fn_80170420(e->m0, e->m4, &check);
        int ok = fn_8040B100(&check, &m50[i], 4) == 0;
        m8[i] = ok ? 1 : -1;
    }
    fn_80170420(base, size, &check);
    m8[0] = fn_8040B100(&check, m50, 4) == 0 ? 1 : -1;
}

void fn_8017037C(Unk8016FF40* info)
{
    Unk8016FF40_Entry* e0 = info->m108[0];
    int base = e0->m0 + 0x10;
    int size = e0->m4 - 0x10;
    for (int i = 0; i < 4; i++) {
        Unk8016FF40_Entry* e = info->m108[i];
        info->fn_80170420(e->m0, e->m4, &info->m50[i]);
        info->m60[i] = info->m108[i]->m4;
        info->m8[i] = 0;
    }
    info->fn_80170420(base, size, info->m50);
}

void Unk8016FF40::fn_80170420(int p, int n, int* out)
{
    int r = p;
    if (r != 0) {
        r = fn_80400560(p, n);
    }
    *out = r;
}

void fn_8017045C()
{
    fn_80359B78(&lbl_8068B87C, 2);
}

void fn_80170488()
{
    lbl_8068B884.m0 = lbl_8065EDB8;
    fn_804007FC(&lbl_8068B884.m4);
}

void fn_801704C0()
{
    lbl_8068B8AC.m0 = lbl_8065ED60;
    if (lbl_8068B8AC.m330 != 0) {
        fn_8035A09C(&lbl_8068B8AC, 0);
        THREAD_destroy(&lbl_8068B8AC.m14);
    }
}

Unk80170518::Unk80170518(int mode)
{
    m30 = 0;
    m34 = 0;
    m38 = 0;
    m3C = 0;
    m40 = 0;
    m44 = 0;
    m48 = 0;
    mBC = 0;
    mC0 = 0;
    mCC = 0;
    mD0 = 0;
    mC4 = 1;
    m2C = mode;
    m4C = &m48;
    mC8 = 1;
    fn_801709F8();
    mA0 = 0;
    for (int i = 0; i < 4; i++) {
        mA4[i] = 0;
    }
    mB4 = 0;
    mB8 = 0;
    fn_80403144(m68, (unsigned short*)fn_80090880(lbl_8061977C, 0x604));
    int n = fn_80403060(m68);
    for (int i = 0; i < n; i++) {
        if (m68[i] == 0x20) {
            m68[i] = 0xA0;
        }
    }
}

Unk80170518::~Unk80170518()
{
    fn_801709F8();
}

void Unk80170518::fn_80170668(Unk8016FF40* info, int state)
{
    m38 = info;
    m28 = state;
    switch (state) {
    case 6:
        fn_800060B4();
        break;
    case 4:
        fn_8017095C(lbl_805CB054);
        break;
    case 5:
        fn_8017095C(lbl_805CB05C);
        break;
    case 3:
        fn_8017095C(lbl_805CB064);
        break;
    case 2:
        fn_8017095C(lbl_805CB06C);
        break;
    case 1:
        fn_8017095C(lbl_805CB074);
        break;
    case 0:
        fn_8017095C(lbl_805CB07C);
        break;
    }
    m30 = 2;
}

void Unk80170518::fn_80170780(int a, int b, const void* name, int c)
{
    m3C = a;
    m40 = b;
    Unicode2Ascii(m50, name);
    m60 = c;
    fn_8017095C(lbl_805CB05C);
    m30 = 2;
}

void Unk80170518::fn_801707F0()
{
    Unk80170518_Fn f = fn_801709A8();
    if (f) {
        (this->*f)();
        if (m30 == 0) {
            m30 = 2;
        }
        if (m30 != 2) {
            fn_801709F8();
        }
        fn_801708C0();
    } else {
        m30 = 0;
    }
}

int Unk80170518::fn_801708C0()
{
    if (m30 == 1) {
        fn_80005C8C();
    }
    if (m30 == 2) {
        fn_80005C8C();
        if (m30 == 0) {
            fn_801707F0();
        } else if (m30 != 2) {
            fn_801709F8();
        }
    }
    return m30;
}

int Unk80170518::fn_8017095C(Unk80170518_Fn f)
{
    Unk80170518_Fn* slot = m0;
    for (int i = 0; i < 5; i++) {
        if (((Unk80170518_Slot*)slot)->m2 == 0) {
            *slot = f;
            return 1;
        }
        slot++;
    }
    m30 = 4;
    return 0;
}

Unk80170518_Fn Unk80170518::fn_801709A8()
{
    Unk80170518_Fn front = m0[0];
    for (int i = 0; i < 4; i++) {
        m0[i] = m0[i + 1];
    }
    m0[4] = 0;
    return front;
}

void Unk80170518::fn_801709F8()
{
    for (int i = 0; i < 5; i++) {
        m0[i] = 0;
    }
}

int fn_80170A1C(Unk80170518* self)
{
    self->m38->fn_801702A4();
    return self->m38->fn_80170248();
}

void Unk80170518::fn_80170A54(int a)
{
    if (mBC != 0 && a > 0) {
        if (mB4 != 0) {
            m64->vslot22(5, 0);
            mB4 = 0;
        } else if (mB8 != 0) {
            m64->vslot22(a == 3 ? 4 : 3, 0);
            mB8 = 0;
        } else {
            m64->vslot22(a, 0);
        }
        mBC = 0;
    }
}

int Unk80170518::fn_80170B3C()
{
    return 0;
}

/* near-miss: retail keeps the stack buffer's address in r29 across both calls (46 instrs, an extra
 * mr); a local char* alias and a struct-wrapped buffer were also tried (6 slots). */
Unk80170B44::Unk80170B44()
{
    char buf[12];
    fn_804089B4(buf, lbl_805CB084, 4);
    fn_80402EF8(m584, buf);
    fn_80170F2C(6, 1, 0);
    fn_80170F2C(0, 0, 0);
    fn_80170EC4((const unsigned short*)fn_80171280(0x11));
    fn_80170EF8((const unsigned short*)fn_80171280(0x11));
}

Unk80170B44::~Unk80170B44() {}

Unk80170B44* Unk80170B44::fn_80170C30(int idx)
{
    fn_80170FEC((const unsigned short*)fn_80171458(idx));
    return this;
}

Unk80170B44* Unk80170B44::fn_80170C6C(int idx)
{
    fn_80170FEC((const unsigned short*)fn_801713D4(idx));
    return this;
}

Unk80170B44* Unk80170B44::fn_80170CA8(int idx)
{
    fn_80170FEC((const unsigned short*)fn_801710D4(idx));
    return this;
}

Unk80170B44* Unk80170B44::fn_80170CE4(int idx)
{
    fn_80170FEC((const unsigned short*)fn_801711E8(idx));
    return this;
}

void Unk80170B44::fn_80170D20(int a)
{
    fn_80403144(m200, (const unsigned short*)fn_80171308(a, 1));
    fn_80403144(m25A, (const unsigned short*)fn_80171280(0));
    fn_80403144(m2B4, (const unsigned short*)fn_80171280(1));
    fn_80403144(m368, (const unsigned short*)fn_80171308(a, 0));
    fn_80403144(m4D0, (const unsigned short*)fn_80171280(0xC));
    int n = fn_80403060(m4D0);
    for (int i = 0; i < n; i++) {
        if (m4D0[i] == 0x20) {
            m4D0[i] = 0xA0;
        }
    }
}

void Unk80170B44::fn_80170E0C(int a, const unsigned short* s)
{
    if (s) {
        fn_80403144(m2B4, s);
        int n = fn_80403060(m2B4);
        for (int i = 0; i < n; i++) {
            if (m2B4[i] == 0xA0) {
                m2B4[i] = 0x20;
            }
        }
    } else {
        fn_80403144(m2B4, (const unsigned short*)fn_80171280(a + 2));
    }
    fn_80170FEC((const unsigned short*)fn_80171280(0x10));
    fn_80403144(m30E, (const unsigned short*)this);
}

void Unk80170B44::fn_80170EC4(const unsigned short* s)
{
    if (s) {
        fn_80403144(m3C2, s);
    } else {
        m3C2[0] = (unsigned short)(int)s;
    }
}

void Unk80170B44::fn_80170EF8(const unsigned short* s)
{
    if (s) {
        fn_80403144(m52A, s);
    } else {
        m52A[0] = (unsigned short)(int)s;
    }
}

void Unk80170B44::fn_80170F2C(int a, int b, int c)
{
    if (a > 49999 && c != 0) {
        fn_80403DE4(m41C, fn_80171280(0xF), 50000);
    } else {
        fn_80403DE4(m41C, fn_80171280(0xE), a);
    }
    if (b > 0) {
        fn_80403DE4(m476, fn_80171280(0xD), b);
    }
}

/* near-miss: same 58 instructions, but retail rotates the loop so the null test and the 0x100 bound sit
 * at the bottom (entry `b` to the test); the while/do-while/for spellings tried here differ by 19+ slots. */
void Unk80170B44::fn_80170FEC(const unsigned short* s)
{
    unsigned short* dst = (unsigned short*)this;
    unsigned int i = 0;
    const unsigned short* p = s;
    while (i <= 0xFF) {
        if (p == 0 || *p == 0) {
            fn_8040074C(&dst[i], (0x100 - i) * 2);
            return;
        }
        if (*p == 0x25) {
            unsigned short c;
            do {
                c = p[1];
                p++;
            } while (c == 0x25);
            unsigned short* slot = (unsigned short*)((char*)this + c * 0x5A - 0xEE0);
            int len;
            if (*slot != 0) {
                len = fn_80403060(slot);
                if (i + len > 0x100) {
                    return;
                }
            } else {
                len = 0;
            }
            if (len > 0) {
                fn_80403144(&dst[i], slot);
            }
            i += len - 1;
        } else {
            dst[i] = *p;
        }
        i++;
        p++;
    }
}

int Unk80170B44::fn_801710D4(int idx)
{
    int id;
    switch (idx) {
    case 2: id = 0x6A3; break;
    case 10: id = 0x68F; break;
    case 8: id = 0x691; break;
    case 9: id = 0x692; break;
    case 19: id = 0x693; break;
    case 7: id = 0x698; break;
    case 16: id = 0x6A1; break;
    case 15: id = 0x69A; break;
    case 12: id = 0x69D; break;
    case 11: id = 0x686; break;
    case 13: id = 0x695; break;
    case 17: id = 0x688; break;
    case 18: id = 0x6A7; break;
    case 20: id = 0x6A8; break;
    default: id = 0x63F; break;
    }
    return fn_80090880(lbl_8061977C, id);
}

int Unk80170B44::fn_801711E8(int idx)
{
    int id;
    switch (idx) {
    case 5: id = 0x655; break;
    case 6:
    case 7: id = 0x657; break;
    case 10: id = 0x658; break;
    case 11: id = 0x695; break;
    case 12: id = 0x65B; break;
    case 13: id = 0x64F; break;
    default: id = 0x63F; break;
    }
    return fn_80090880(lbl_8061977C, id);
}

int Unk80170B44::fn_80171280(int idx)
{
    int id;
    switch (idx) {
    case 13: id = 0x62F; break;
    case 0: id = 0x5FA; break;
    case 2: id = 0x5FB; break;
    case 16: id = 0x641; break;
    case 12: id = 0x604; break;
    default: id = 0x63F; break;
    }
    return fn_80090880(lbl_8061977C, id);
}

int Unk80170B44::fn_80171308(int a, int b)
{
    int id;
    if (b != 0) {
        switch (a) {
        case 0: id = 0x605; break;
        case 1: id = 0x606; break;
        case 2: id = 0x607; break;
        case 4: id = 0x608; break;
        default: id = 0x63F; break;
        }
    } else {
        switch (a) {
        case 0: id = 0x619; break;
        case 1: id = 0x618; break;
        case 2: id = 0x61A; break;
        case 4: id = 0x61B; break;
        default: id = 0x63F; break;
        }
    }
    return fn_80090880(lbl_8061977C, id);
}

int Unk80170B44::fn_801713D4(int idx)
{
    int id;
    switch (idx) {
    case 0: id = 0x636; break;
    case 1: id = 0x636; break;
    case 4: id = 0x636; break;
    case 3: id = 0x63A; break;
    case 2: id = 0x639; break;
    case 5: id = 0x63B; break;
    default: id = 0x63F; break;
    }
    return fn_80090880(lbl_8061977C, id);
}

int Unk80170B44::fn_80171458(int idx)
{
    int id;
    switch (idx) {
    case 3: id = 0x65C; break;
    case 4: id = 0x65A; break;
    case 2: id = 0x64C; break;
    case 7: id = 0x64D; break;
    case 8: id = 0x64E; break;
    default: id = 0x63F; break;
    }
    return fn_80090880(lbl_8061977C, id);
}

Unk801714E0::Unk801714E0()
{
    m0 = 0;
    m4 = 0;
    fn_801716E4(this);
}

Unk801714E0::~Unk801714E0()
{
    fn_8017171C(this);
    if (m4) {
        delete m4;
        m4 = 0;
    }
}

void Unk801714E0::fn_8017158C(const unsigned short* s)
{
    int n = fn_80403060(s);
    if (m4) {
        delete m4;
        m4 = 0;
    }
    m4 = new (0x100) unsigned short[n + 1];
    fn_80403144(m4, s);
}

void fn_801715F8()
{
    while (lbl_8061C66C) {
        delete lbl_8061C66C;
    }
}

Unk801714E0* fn_8017165C(unsigned int i)
{
    if (i < fn_801716BC()) {
        Unk801714E0* n = lbl_8061C66C;
        for (unsigned int j = 0; j < i; j++) {
            n = n->m0;
        }
        return n;
    }
    return 0;
}

unsigned int fn_801716BC()
{
    unsigned int n = 0;
    for (Unk801714E0* p = lbl_8061C66C; p != 0; p = p->m0) {
        n++;
    }
    return n;
}

void fn_801716E4(Unk801714E0* p)
{
    if (lbl_8061C66C == 0) {
        lbl_8061C66C = p;
        return;
    }
    Unk801714E0* q = lbl_8061C66C;
    while (q->m0 != 0) {
        q = q->m0;
    }
    q->m0 = p;
}

/* near-miss: the original loop never advances (an unlinked node hangs it); retail re-tests h with a
 * second cmpwi instead of reusing the loop-entry compare (16 vs 17 instrs). */
void fn_8017171C(Unk801714E0* p)
{
    Unk801714E0* h = lbl_8061C66C;
    if (p == h) {
        lbl_8061C66C = h->m0;
        return;
    }
    while (h != 0) {
        if (h->m0 == p) {
            h->m0 = p->m0;
        }
    }
}

Unk80171760::Unk80171760()
{
    m0 = 9;
    m4 = 0;
    m8 = 0;
    mC = 0;
    m10 = 0;
}

Unk80171760::~Unk80171760()
{
    fn_801717DC();
}

void Unk80171760::fn_801717DC()
{
    if (mC) {
        delete mC;
        mC = 0;
    }
}

void Unk80171760::fn_8017181C(const unsigned short* s)
{
    if (s == 0) {
        fn_vslot2();
        return;
    }
    int n = fn_80403060(s);
    if (mC) {
        delete mC;
        mC = 0;
    }
    mC = new (0x100) unsigned short[n + 1];
    fn_80403144(mC, s);
}

void Unk80171760::fn_801718A8(int a)
{
    if (m10 != 0) {
        fn_801715F8();
    }
    m10 = a;
}

int Unk80171760::fn_801718E8()
{
    if (m10 != 0) {
        return fn_801716BC();
    }
    return 0;
}

void Unk80171760::fn_8017191C(unsigned int i)
{
    fn_8017165C(i);
}

Unk80171940::Unk80171940()
{
    m0 = new (0x100) Unk80170B44;
    m4 = 0;
}

Unk80171940::~Unk80171940()
{
    fn_80171FE8(this);
    if (m0) {
        delete m0;
        m0 = 0;
    }
}
