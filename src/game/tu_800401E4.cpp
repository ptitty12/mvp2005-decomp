/*
 * Part of the game TU 0x8000F598-0x8006902C (2,110 functions; it ends with
 * the static-initializer wrapper at 0x80069000, so it is one large, likely
 * unity-built TU). Original file boundary unknown; placeholder.
 *
 * .text 0x800401E4-0x80042DBC only: every data object is extern by its lbl_ name.
 */
#include "game/tu_800401E4.h"

extern int lbl_80617050;
extern Unk80040640* lbl_806177D4;
extern Unk80040640_Tbl* lbl_806177D8;
extern int lbl_806177DC;
extern Unk805AEAB8_Ent lbl_805AEAB8[];
extern Unk805AF688_Ent lbl_805AF688[];
extern Unk805B10B4_Ent lbl_805B10B4[];
extern Unk805AF370_Ent lbl_805AF370[];
extern char lbl_806872A4[];
extern char lbl_806872AC[];
extern "C" void fn_803A8B4C();
extern "C" void Unsubscribe(int a);
extern "C" void fn_803BA7C4(void* a, int* b);
extern "C" void fn_800418C4(Unk80040640* self);
extern "C" void fn_800410D8(Unk80040640* self, int a);
extern "C" int fn_80040E9C(Unk80040640* self, int a);
void fn_80040C58(Unk80040640* self, int a);
void fn_80040DCC(Unk80040640* self, int a, int b);
extern "C" int fn_800415C0(Unk80040640* self, int id, int a, int b, int c, int d);
int fn_80041350(Unk80040640* self, int id);
int fn_80041418(Unk80040640* self, int i);
void fn_80041478(Unk80040640* self, int id);
extern "C" int SNDbankpatpresent(int a, int b);
extern "C" void SNDSYS_entercritical();
int fn_803A85B0(int a, int b, void* out);
extern "C" void SNDSYS_leavecritical();
void fn_803A9204(void* p);
void fn_8006772C();

int fn_802B96C4(const char* name, int a);
int fn_802B9764(const char* name);
extern "C" int fn_802B9300(const char* name, int a, int b, int* out);
extern "C" void MEM_free(int p);
extern "C" void SNDbankremove(int a);
void fn_803A6644(int a);
extern "C" int fn_803A82A8(int* out, int a);
int fn_803A8530(int a);
void* operator new[](unsigned int size, int heap); /* 0x803774A8 */
int fn_803A84BC(int a, int b);
extern "C" int fn_803A6B64(int a, const char* b, int c, void (*cb)(int, int));
Unk80040640* fn_8004072C();
void fn_800404A8(int u, int size);

Unk800401E4::Unk800401E4()
{
    int i;
    m0 = 0;
    m4 = -1;
    m8 = 0;
    m10 = 0;
    mC = -1;
    m34 = -1;
    for (i = 0; i < 8; i++)
        m14[i] = 0;
}

Unk800401E4::~Unk800401E4()
{
    if (m8 == 1)
        fn_800403EC();
}

int Unk800401E4::fn_8004028C(const char* name, int type, const char* name2, int flag)
{
    int spC = 0;
    int sp8 = 0;
    int ok;
    mC = type;
    ok = 1;
    if (fn_802B96C4(name, 0) == 0)
        ok = 0;
    if (ok == 1) {
        m34 = fn_802B9764(name);
        if (name2 != 0) {
            ok = 0;
            if (fn_802B96C4(name2, 0) != 0)
                ok = 1;
        }
        if (ok == 1) {
            spC = fn_802B9300(name, 0x400, 0xe, &sp8);
            if (sp8 <= 0 || spC == 0)
                ok = 0;
            if (ok == 1) {
                switch (type) {
                case 0:
                    ok = fn_800404E0(&spC, name);
                    break;
                case 1:
                    ok = fn_80040598(&spC, name, name2, flag);
                    break;
                default:
                    ok = 0;
                    break;
                }
            }
        }
    }
    m8 = ok;
    if (ok == 0) {
        if (spC != 0)
            MEM_free(spC);
        m0 = ok;
        m4 = -1;
    } else {
        m10 = (int)name;
    }
    return m8;
}

void Unk800401E4::fn_800403EC()
{
    int i;
    if (m8 == 1) {
        switch (mC) {
        case 0:
            SNDbankremove(m4);
            if (m0 != 0)
                delete[] (char*)m0;
            break;
        case 1:
            fn_803A6644(m4);
            if (m0 != 0)
                delete[] (char*)m0;
            break;
        }
    }
    m4 = -1;
    m0 = 0;
    m8 = 0;
    m10 = 0;
    mC = -1;
    for (i = 0; i < 8; i++)
        m14[i] = 0;
}

void fn_800404A8(int u, int size)
{
    lbl_80617050 = (int)(new (0x400) char[size]);
}

/* near-miss: 46/46 slots; retail stores m0 before the null test and puts the
 * delete[] error path in a different order. */
int Unk800401E4::fn_800404E0(int* p, const char* name)
{
    int ok = 1;
    int r = fn_803A82A8(&m4, *p);
    if (r >= 0) {
        if (r == 7) {
            m0 = (int)(new (0x400) char[fn_803A8530(m4)]);
            if (m0 == 0) {
                ok = 0;
            } else if (fn_803A84BC(m0, m4) < 0) {
                ok = 0;
                if (m0 != 0)
                    delete[] (char*)m0;
            } else {
                MEM_free(*p);
                *p = 0;
            }
        } else {
            m0 = *p;
        }
    } else {
        ok = 0;
    }
    return ok;
}

int Unk800401E4::fn_80040598(int* p, const char* name, const char* name2, int flag)
{
    int ok = 1;
    if (fn_8004072C()->m5D0 == 1) {
        m0 = *p;
        m4 = fn_803A6B64(*p, name2, flag, fn_800404A8);
        if (m4 >= 0) {
            if (lbl_80617050 != 0) {
                m0 = lbl_80617050;
                lbl_80617050 = 0;
                MEM_free(*p);
                *p = 0;
            }
        } else {
            ok = 0;
        }
    } else {
        ok = 0;
    }
    return ok;
}

/* near-miss: 57/59 slots. Retail computes &lbl_806177D8 before the operator
 * new call (kept in r31) and stores it after the init loop; every spelling of
 * the table pointer tried here stores it at the end with a fresh lis. The
 * field-store order matches (found by permutation search). */
Unk80040640::Unk80040640()
{
    int i;
    Unk80040640_Tbl* t;
    m5D0 = 0;
    m5C4 = 0;
    m5C8 = 0;
    m5CC = 0;
    m5F4 = -1;
    m5F8 = -1;
    m5FC = -1;
    for (i = 0; i < 8; i++)
        m5D4[i] = -1;
    for (i = 0; i < 0x63; i++)
        m438[i] = 0x3f;
    t = new (0) Unk80040640_Tbl;
    for (i = 0; i < 0x40; i++)
        t->m4[i * 2] = 0;
    lbl_806177D8 = t;
    t->m200 = 0;
}

Unk80040640* fn_8004072C()
{
    if (lbl_806177D4 == 0)
        lbl_806177D4 = new (0) Unk80040640;
    return lbl_806177D4;
}

Unk80040640::~Unk80040640()
{
}

void fn_800407EC(Unk80040640* self)
{
    if (self->m5C4 == 1) {
        if (self->m5D0 == 0)
            fn_800418C4(self);
        fn_80040DCC(self, 4, -1);
        fn_80040DCC(self, 6, -1);
        fn_80040DCC(self, 7, -1);
        fn_80040DCC(self, 8, -1);
        fn_80040DCC(self, 9, -1);
        fn_80040DCC(self, 10, -1);
        fn_80040DCC(self, 11, -1);
    }
}

void fn_800408A0(Unk80040640* self)
{
    int c = 0;
    if (self->m5C4 == 1)
        c = self->m5F4 >= 0;
    if (c == 1) {
        int i = 0;
        do {
            fn_800410D8(self, i);
            i++;
        } while (i <= 7);
    }
    if (self->m5D0 == 1) {
        fn_803A8B4C();
        MEM_free(self->m5CC);
        Unsubscribe(self->m5C8);
        MEM_free(self->m5C8);
        self->m5D0 = 0;
    }
    if (lbl_806177D4 != 0) {
        delete lbl_806177D4;
        lbl_806177D4 = 0;
    }
}

struct Unk806192E0_C {
    char pad0[0x10];
    float m10;
    char pad14[0x28];
    int m3C;
};
struct Unk806192E0_B {
    char pad0[0xC];
    Unk806192E0_C* mC;
};
struct Unk806192E0 {
    int m0;
    Unk806192E0_B* m4;
};
extern Unk806192E0* lbl_806192E0;
void fn_80040BE8(Unk80040640* self, int a);
int fn_80040CC8(Unk80040640* self, int a);

int fn_80040958(Unk80040640* self, int a)
{
    if (self->m5C4 == 0) {
        self->m5D4[a] = -3;
    } else if (self->m5D4[a] < 0) {
        if (self->m5D0 == 0)
            fn_800418C4(self);
        fn_80040BE8(self, a);
        Unk806192E0_B* o = lbl_806192E0->m4;
        if (a == 0) {
            lbl_806177DC = a;
        } else if (o != 0) {
            int* g = &lbl_806177DC;
            Unk806192E0_C* c = o->mC;
            float x = c->m10;
            float nf = (float)c->m3C;
            float y = x * nf;
            int code = -1;
            if (!(y < 0.0f)) {
                if (!(y >= 12000.0f)) {
                    code = 3;
                } else if (!(y >= 24000.0f)) {
                    code = 2;
                } else {
                    int f = y < 36000.0f;
                    code = f;
                }
            }
            *g = code;
        }
        int v;
        switch (fn_80040CC8(self, a)) {
        case -3:
            v = -3;
            break;
        case 0:
            v = 0;
            break;
        case 1:
            v = 1;
            break;
        case -1:
            v = -1;
            break;
        case -2:
            v = -2;
            break;
        default:
            v = -2;
            break;
        }
        self->m5D4[a] = v;
        int j;
        for (j = 0; j <= 7; j++) {
            if (j != a && self->m5D4[j] >= 0)
                fn_80040CC8(self, j);
        }
        self->m5F4 = -1;
        int k;
        for (k = 0; k < 8; k++) {
            if (self->m5D4[k] >= 0) {
                self->m5F4 = self->m5D4[k];
                break;
            }
        }
    }
    switch (self->m5D4[a]) {
    case -3:
        return -3;
    case -1:
        return -1;
    case 0:
        return 0;
    case 1:
        return 1;
    case -2:
        return -2;
    default:
        return -2;
    }
}

void fn_80040BE8(Unk80040640* self, int a)
{
    int i;
    for (i = 0; i <= 0x11; i++) {
        if (lbl_805AEAB8[i].m30[a] == 1)
            fn_80040C58(self, i);
    }
}

/* near-miss: 27/28 slots. Retail computes idx*0x3C once into two registers
 * (mulli, mr, add) and re-adds it for the call. */
void fn_80040C58(Unk80040640* self, int idx)
{
    if (self->m0[idx].m8 == 1)
        self->m0[idx].fn_800403EC();
    if (idx == 13)
        self->m5F8 = -1;
    else if (idx == 12)
        self->m5FC = -1;
}

/* near-miss: 68/65 slots, structure right (any/fail flags, slot loop) but the
 * register numbering differs: retail strength-reduces m0[i].m14 to a pointer
 * starting at self+0x14 (stepping 0x3C) and keeps a in r28. An explicit
 * `int* p = m0[0].m14; p += 15` version (66/65) copies the start pointer
 * (mr r22,r30) because the m0[t] path shares its base. */
int fn_80040CC8(Unk80040640* self, int a)
{
    int any = 0;
    int fail = 0;
    int i;
    for (i = 0; i <= 0x11; i++) {
        if (lbl_805B10B4[a].m38[i] == 1) {
            if (fn_80040E9C(self, i) == 1) {
                any = 1;
                self->m0[i].m14[a] = 1;
            } else {
                int t = lbl_805AEAB8[i].m50;
                if (t != -1 && fn_80040E9C(self, t) == 1) {
                    any = 1;
                    self->m0[t].m14[a] = 1;
                } else {
                    fail = 1;
                }
            }
        }
    }
    if (any != 0 || fail != 1)
        return fail ^ 1;
    return -2;
}

void fn_80040DCC(Unk80040640* self, int a, int b)
{
    if ((unsigned int)(a - 12) <= 1)
        lbl_806177DC = b;
    fn_80040E9C(self, a);
    if (a == 13) {
        if (self->m0[12].m8 == 1 && self->m5FC != lbl_806177DC) {
            fn_80040C58(self, 12);
            fn_80040E9C(self, 12);
        }
    } else if (a == 12) {
        if (self->m0[13].m8 == 1 && self->m5F8 != lbl_806177DC) {
            fn_80040C58(self, 13);
            fn_80040E9C(self, 13);
        }
    }
}

extern char lbl_805AE920[][0x1b];
extern char lbl_805AE971[][0x51];
extern char lbl_805AE98C[][0x51];
struct Unk805AEACC_Ent {
    char m0[0x44];
    int m44;
    char m48[0x7c - 0x48];
};
extern Unk805AEACC_Ent lbl_805AEACC[];
extern "C" int rand();

extern "C" int fn_80040E9C(Unk80040640* self, int a)
{
    int ret = 1;
    int skip = 0;
    int off = a * 0x3c;
    if (((Unk800401E4*)(off + (int)self))->m8 == 1) {
        if ((a == 13 && self->m5F8 != lbl_806177DC) || (a == 12 && self->m5FC != lbl_806177DC))
            fn_80040C58(self, a);
        else
            skip = 1;
    }
    if (skip == 0) {
        int j;
        for (j = 0; j < 8; j++) {
            if (self->m5D4[j] >= 0 && lbl_805AEAB8[a].m30[j] == 1) {
                ret = 0;
                break;
            }
        }
        if (ret == 1) {
            const char* name;
            const char* name2 = 0;
            switch (a) {
            case 13:
                name = lbl_805AE971[lbl_806177DC];
                name2 = lbl_805AE971[lbl_806177DC] + 0x36;
                break;
            case 12:
                name = lbl_805AE98C[lbl_806177DC];
                break;
            case 0:
                name = lbl_805AE920[rand() % 3];
                break;
            default:
                name = lbl_805AEACC[a].m0;
                if (lbl_805AEACC[a].m44 == 1)
                    name2 = lbl_805AEACC[a].m48;
                break;
            }
            ret = self->m0[a].fn_8004028C(name, lbl_805AEAB8[a].m54, name2, lbl_805AEAB8[a].m78);
            if (ret != 0) {
                if (a == 13)
                    self->m5F8 = lbl_806177DC;
                else if (a == 12)
                    self->m5FC = lbl_806177DC;
            }
        }
    }
    return ret;
}

int fn_800412AC(Unk80040640* self, int id, int a, int b, int c, int d)
{
    int r = -1;
    int ok = 0;
    if (self->m5C4 == 1)
        ok = self->m5F4 >= 0;
    if (ok == 1 && id >= 0 && id <= 0xb4) {
        switch (lbl_805AEAB8[lbl_805AF688[id].mC].m54) {
        case 0:
            r = fn_80041350(self, id);
            break;
        case 1:
            r = fn_800415C0(self, id, a, b, c, d);
            break;
        }
    }
    return r;
}

int fn_80041350(Unk80040640* self, int id)
{
    int ret = -1;
    int a = lbl_805AF688[id].mC;
    int r30 = fn_80041418(self, a);
    int r31 = lbl_805AF688[id].m8;
    if (r30 >= 0 || (r30 = fn_80041418(self, lbl_805AEAB8[a].m50), r31 = lbl_805AF688[id].m10, r30 >= 0)) {
        fn_80041478(self, id);
        if (SNDbankpatpresent(r30, r31) != 0) {
            SNDSYS_entercritical();
            ret = fn_803A85B0(r30, r31, &self->m600);
            SNDSYS_leavecritical();
        }
    }
    return ret;
}

/* near-miss: 23/24 slots; same mulli/mr/add pattern as fn_80040C58 (idx*0x3C
 * kept in two registers). */
int fn_80041418(Unk80040640* self, int i)
{
    int r = -1;
    int ok = 0;
    if (self->m5C4 == 1)
        ok = self->m5F4 >= 0;
    if (ok == 1 && i >= 0 && i <= 0x11) {
        if (self->m0[i].m8 == 1)
            r = self->m0[i].m4;
    }
    return r;
}

/* near-miss: 33/33 slots, 2 differ: retail stores m5 (stb 0x605) before the
 * lwzx of the table entry, here after. 2,520 statement orders were tried. */
void fn_80041478(Unk80040640* self, int id)
{
    int t;
    fn_803A9204(&self->m600);
    t = lbl_805AF688[id].m14;
    self->m600.m5 = 0;
    if (t > -1)
        self->m600.m0 = (unsigned char)self->m438[t];
    self->m600.m2 = 0x3c;
    self->m600.m3 = 0x7f;
    self->m600.m8 = 0;
    self->m600.mA = 0;
}

int fn_800414FC(Unk80040640* self, int a, int idx)
{
    int v;
    if (a <= 100) {
        v = a;
        if (v < 0)
            v = 0;
    } else {
        v = 100;
    }
    if (idx <= 0x62) {
        self->m438[idx] = ((v << 7) - v) / 100;
        if (self->m5D0 == 1) {
            int (*fp)(int) = lbl_805AF370[idx].m0;
            v = ((v << 15) - v) / 100;
            if (fp != 0 && fp != (int (*)(int))fn_8006772C)
                v = fp(v);
        }
    }
    return v;
}

struct Unk805A8680_Ent {
    int m0;
    void (*m4)(int, int*);
    void (*m8)(int*, int);
};
extern Unk805A8680_Ent lbl_805A8680[];

extern "C" void fn_80041778(Unk80040640* self, int id, int slot, int a, int b, int c, int d)
{
    int args[4];
    int ok = 0;
    if (self->m5C4 == 1)
        ok = self->m5F4 >= 0;
    if (ok == 1 && slot >= 0) {
        args[0] = a;
        args[1] = b;
        args[2] = c;
        args[3] = d;
        Unk80040640_Tbl* t = lbl_806177D8;
        int o = slot * 8;
        if (*(int*)((char*)t + o) == id) {
            char* bb = (char*)t + 4;
            int p = *(int*)(bb + o);
            if (p != 0)
                lbl_805A8680[id].m4(p, args);
        }
    }
}

extern "C" void fn_8004181C(Unk80040640* self, int id, int slot, int a, int b, int c, int d)
{
    int ok = 0;
    if (self->m5C4 == 1)
        ok = self->m5F4 >= 0;
    if (ok == 1 && slot >= 0) {
        Unk80040640_Tbl* t = lbl_806177D8;
        int o = slot * 8;
        if (*(int*)((char*)t + o) == id) {
            char* bb = (char*)t + 4;
            if (*(int*)(bb + o) != 0) {
                lbl_805A8680[id].m8((int*)((char*)t + o + 4), 1);
                *(int*)(bb + o) = 0;
            }
        }
    }
}

/* near-miss: 60/57 slots. Retail does not keep the "t < 0" compare in cr4
 * across the call (the second clamp re-tests); an inline Clamp() made it worse. */
void fn_80042C78(Unk80040640* self, int a)
{
    int v;
    int t;
    int sp8;
    int spC;
    if (a <= 100) {
        v = a;
        if (v < 0)
            v = 0;
    } else {
        v = 100;
    }
    t = ((v << 15) - v) / 100;
    sp8 = t;
    if (t < 0)
        sp8 = 0;
    else if (t > 0x7fff)
        sp8 = 0x7fff;
    fn_803BA7C4(lbl_806872AC, &sp8);
    if (self->m5D4[3] < 0 || self->m5D4[6] < 0) {
        spC = t;
        if (t < 0)
            spC = 0;
        else if (t > 0x7fff)
            spC = 0x7fff;
        fn_803BA7C4(lbl_806872A4, &spC);
    }
}

void fn_80042D5C(Unk80040640* self)
{
    fn_80040C58(self, 1);
    fn_80040C58(self, 0);
    fn_80040C58(self, 2);
    fn_80040C58(self, 12);
    fn_80040C58(self, 13);
}
