/*
 * Database overlay code, .database_codeoverlay 0x805467E0-0x805495DC (code-only unit;
 * placeholder TU). Define every function DB_OVERLAY (game/overlay.h); data extern by lbl_ name.
 */
#include "game/db_8053967C.h"

typedef Db805467E0_Ev Ev;
typedef Db805467E0_Q Q;
typedef Db805467E0_V V;

extern const float lbl_805FF024;
extern const float lbl_805FF028;
extern char lbl_80682D68[];
extern char lbl_80682D50[];
extern int lbl_80622BEC;
extern int lbl_80692E38;
extern int lbl_80622BF4;

static inline int IntAt(char* b, int off)
{
    return *(int*)(b + off);
}

static inline void IntSet(char* b, int off, int v)
{
    *(int*)(b + off) = v;
}

/* Q18 types */
int fn_8008FED8(int a);

int fn_8008FBE8(int a);
int fn_8008FE14(int a);

unsigned fn_8007F84C(Unk8007F198_B* g);

struct Db805467E0_T3 {
    float f;
    int a;
    int b;
};
/* end Q18 types */

extern "C" {
/* Q18 protos */
int fn_80546A24(char* o, int id, int a, int b, float f);
int fn_80546B18(char* o, int a, int b);
int fn_80546BCC(char* o, int a, int b);
int fn_80546C88(char* o, int i);
int fn_80546CA8(char* o, int a, int b);
void fn_80546D0C(char* o);
void fn_80546D4C(char* o);
void fn_80546D5C(char* o, int a);
int fn_805474B8(char* o, int a);
void fn_80547594(char* o, int a);
void fn_805479F0(char* o, int kind, int x);
void fn_80547B38(char* o, int kind, int x);
void fn_80547C5C(char* o);
int fn_80547E10(char* o, int a);
int fn_80547FFC(char* o, int a, int id);
void fn_805480E8(char* o);
void fn_80548128(char* o, int a);
int fn_805481D8(char* o);
void fn_805482D4(char* o);
void fn_805483C4(char* o);
Ev* fn_80548858(Ev* e);
void fn_80548894(Ev* e, int a, int b, int c, V* v);
void fn_805488E0(Ev* e, int v);
void fn_805488E8(Ev* e, V* v);
void fn_80548908(Ev* e);
Q* fn_80548934(Q* o);
void fn_8054899C(Q* q);
void fn_805489FC(Q* q, int i);
void fn_80548B08(Q* q, int v);
int fn_80548B6C(Ev* e, int id);
int fn_80548BA0(Q* q, int a, int id, V* v, int c, int flag);
void fn_80548C7C(Q* q, int a, int b);
int fn_80548CFC(Ev* q, int id);
int* fn_80548D2C(Q* q, int id);
void* fn_80548D78(Q* q, int id);
int fn_80548DC0(Q* q, int id);
int fn_80548E0C(Q* q, int i);
int fn_80548E1C(Q* q, int i);
int* fn_80548E2C(Q* q, int i);
Ev* fn_80548E3C(Q* q, int i);
Db805467E0_W3* fn_80548E48(Q* q, int i);
int* fn_80548E58(Q* q, int i);
void fn_80548E68(Q* q, int n);
void fn_805494DC(char* o, int id, int a, int b, int c, int flag, int d, float f);
/* end Q18 protos */

/* protos */
int fn_80546B18(char* o, int a, int b);
int fn_80546BCC(char* o, int a, int b);
int fn_80546C88(char* o, int i);
int fn_80546CA8(char* o, int a, int b);
void fn_80546D0C(char* o);
void fn_80546D4C(char* o);
void fn_80546D5C(char* o, int a);
int fn_805474B8(char* o, int a);
void fn_80547594(char* o, int a);
void fn_805480E8(char* o);
Ev* fn_80548858(Ev* e);
void fn_80548894(Ev* e, int a, int b, int c, V* v);
void fn_805488E0(Ev* e, int v);
void fn_805488E8(Ev* e, V* v);
void fn_80548908(Ev* e);
Q* fn_80548934(Q* o);
void fn_8054899C(Q* q);
void fn_80548B08(Q* q, int v);
int fn_80548B6C(Ev* e, int id);
int fn_80548BA0(Q* q, int a, int id, V* v, int c, int flag);
void fn_80548C7C(Q* q, int a, int b);
int fn_80548CFC(Ev* q, int id);
int* fn_80548D2C(Q* q, int id);
void* fn_80548D78(Q* q, int id);
int fn_80548DC0(Q* q, int id);
int fn_80548E0C(Q* q, int i);
int fn_80548E1C(Q* q, int i);
int* fn_80548E2C(Q* q, int i);
Ev* fn_80548E3C(Q* q, int i);
Db805467E0_W3* fn_80548E48(Q* q, int i);
int* fn_80548E58(Q* q, int i);
void fn_80548E68(Q* q, int n);
/* end protos */
/* Q18 decls */
extern "C" void fn_8053967C(void* l);
extern "C" void fn_8053971C(void* l);
extern "C" void fn_805362AC(char* o, int a);
extern "C" int rand();
extern "C" void fn_80535EAC(void* l);
extern "C" int fn_80400CE4(int a);
extern "C" void fn_800A1930(void* g, int id, int b, int d, float f);
extern "C" int fn_80077BC4(void* s, int id);
extern "C" void fn_800793E8(void* s, int id, void* p1, void* p2, void* p3);
extern "C" int fn_80564ABC(int a, int b, int c, int d, float* e, int* f, int g);
extern "C" void fn_80539DB4(void* l, int e);
extern "C" void fn_8053A044(void* l, int a, int b);
extern "C" void fn_80539EDC(void* l, int e);
extern "C" void fn_80542580(void* o);
/* end Q18 decls */

extern "C" Db805495DC_Hold* fn_8054BDDC(Db805495DC_Hold* o, Unk8007F198_C* s);
extern "C" void fn_8054BE40(Db805495DC_Hold* o, int mode);
extern "C" void fn_8054BEA4(Db805495DC_Hold* o, int mode);
extern "C" void fn_8054BF08(Db805495DC_Hold* o, int mode);
extern "C" void fn_8054BF6C(Db805495DC_Hold* o, int mode);
extern "C" void fn_8054BFD0(Db805495DC_Hold* o, int mode);
extern "C" void fn_80377408(void* p);
struct Db805467E0_G3C {
    char pad0[8];
    float m8;
    char padC[4];
    int m10;
    char pad14[4];
    int m18;
};
struct Db805467E0_G {
    char pad0[0x3C];
    void* m3C;
};
extern "C" void fn_80563F3C(void* m3c, int a, int b, float* f, int* z, int* o, int x, int y);
void fn_8007F330(Unk8007F198_A* self, int a, int b);
DB_OVERLAY int fn_805467E0(char* o, int id)
{
    int r = fn_805474B8(o, id);
    if (r != 0)
        return r;
    {
        unsigned n = *(unsigned*)(o + 4);
        unsigned i = 0;
        int idx;
        for (; i < n; i++) {
            if (*(int*)((char*)*(int**)o + i * 4) == id) {
                idx = i;
                goto found;
            }
        }
        idx = -1;
    found:
        int off = idx * 16;
        IntSet(o + 0x3CC8, off, id);
        Db8053967C_List* l = (Db8053967C_List*)(o + 0x32E0);
        IntSet(o + 0x3CCC, off, *(int*)(l->mC * 4 + (int)*(int**)(o + 0x32E0)));
        IntSet(o + 0x3CD0, off, IntAt(o + 0x10000, 0x7E8));
        IntSet(o + 0x3CD4, off, 0);
        IntSet(o + 0x10000, 0x7EC, IntAt(o + 0x10000, 0x7EC) - 1);
        if (IntAt(o + 0x10000, 0x7F0) == 1) {
            int cur = *(int*)(l->mC * 4 + (int)*(int**)(o + 0x32E0));
            Unk8007B2B0_Screen* s = (Unk8007B2B0_Screen*)lbl_8061930C;
            fn_8007F330((Unk8007F198_A*)s, cur, id);
            Db805467E0_G3C* g = (Db805467E0_G3C*)((Db805467E0_G*)lbl_80619948)->m3C;
            float f8;
            int sp40;
            int sp44;
            fn_80563F3C(g, cur, id, &f8, &sp40, &sp44, 1, 1);
            Unk800798BC_P p10;
            Unk800798BC_P p20;
            Unk800798BC_P p30;
            p20.m0 = f8;
            p20.m4 = sp40;
            p20.m8 = sp44;
            p10.m0 = s->fn_800799D0(((Unk8007A4DC_Screen*)s)->fn_8007A6A0(0x18, id));
            p10.m4 = ((Unk8007A4DC_Screen*)s)->fn_8007A6A0(0x19, id);
            p10.m8 = sp44;
            p30.m0 = g->m8;
            p30.m4 = g->m10;
            p30.m8 = g->m18;
            ((Unk8007B2B0_Screen*)lbl_8061930C)->fn_800798BC(id, &p10, &p20, &p30);
        }
        if (IntAt(o + 0x10000, 0x7EC) > 0) {
            if (l->mC == (int)l->m4 - 1) {
                int v = IntAt(o + 0x10000, 0x7E8) + 1;
                IntSet(o + 0x10000, 0x7E8, v);
                if (IntAt(o + 0x10000, 0x7F0) == 1) {
                    if (v == 0x19)
                        fn_80548128(o, 1);
                    else if (v == 0x32)
                        fn_80548128(o, 2);
                    else if (v == 0x4B)
                        fn_80548128(o, 3);
                }
            }
            l->mC = (l->mC + 1) % l->m4;
        }
    }
    return 0;
}

/* near-miss: 96.7%; only the prologue order of the float-param spill and the r6 copy differs */
DB_OVERLAY int fn_80546A24(char* o, int id, int a, int b, float f)
{
    unsigned n = *(unsigned*)(o + 4);
    unsigned i = 0;
    int idx;
    for (; i < n; i++) {
        if (*(int*)((char*)*(int**)o + i * 4) == id) {
            idx = i;
            goto found;
        }
    }
    idx = -1;
found:
    Unk8007B2B0_Screen* s = (Unk8007B2B0_Screen*)lbl_8061930C;
    int off = idx * 16;
    struct {
        float f;
        int a;
        int c;
    } st;
    st.f = f;
    int t = IntAt(o + 0x3CCC, off);
    st.a = a;
    st.c = ((Unk8007A4DC_Screen*)s)->fn_8007A270(id) != 0 ? 0xE : 0xB;
    int r = s->fn_80079094(id, s->fn_80075948(t), (int)&st.f, (int)&st.f, (int)&st.f, b);
    if (r == 0)
        IntSet(o + 0x3CD4, off, 1);
    return r;
}

DB_OVERLAY int fn_80546B18(char* o, int a, int b)
{
    Unk8007B2B0_Screen* s = (Unk8007B2B0_Screen*)lbl_8061930C;
    int ao = a * 4;
    int h = s->fn_80075948(*(int*)((char*)*(int**)(o + 0x32E0) + ao));
    unsigned i = 0;
    if (i < *(unsigned*)(o + 4)) {
        int off = 0;
        do {
            int k = IntAt(o + 0x3CCC, off);
            if (k != 0 && s->fn_80075948(k) == h && IntAt(o + 0x3CD0, off) == b)
                return IntAt(o + 0x3CC8, off);
            i++;
            off += 0x10;
        } while (i < *(unsigned*)(o + 4));
    }
    return 0;
}

DB_OVERLAY int fn_80546BCC(char* o, int a, int b)
{
    if (*(int*)(o + 0x107F0) == 0) {
        int ao = a * 4;
        return *(int*)((char*)*(int**)(o + 0x32E0) + ao);
    }
    Unk8007B2B0_Screen* s = (Unk8007B2B0_Screen*)lbl_8061930C;
    int h = s->fn_80075948(IntAt((char*)IntAt(o,0x32E0),a*4));
    if (b <= 0x18) return s->fn_8007B320(0,h);
    if (b <= 0x31) return s->fn_8007B320(1,h);
    if (b <= 0x4A) return s->fn_8007B320(2,h);
    if (b <= 0x63) return s->fn_8007B320(3,h);
    return 0;
}

void fn_8036179C(void* p);

DB_OVERLAY int fn_80546C88(char* o, int i)
{
    char* b = o + 0x3CCC;
    int off = i * 0x10;
    if (*(int*)(b + off) != 0)
        return 1;
    return 0;
}

int fn_805467E0(char* o, int a);
int fn_805470F0(char* o);
int fn_805425FC(char* s, int a);
void fn_805489FC(Q* q, int i);
void fn_80538258(char* o);
void fn_80537514(void* p);
void fn_805381F0(char* o);
void fn_805362AC(char* o, int a);
void fn_805484A0(char* o);

DB_OVERLAY int fn_80546CA8(char* o, int a, int b)
{
    unsigned n = *(unsigned*)(o + 4);
    int ao = a * 4;
    int key = *(int*)((char*)*(int**)(o + 0x32E0) + ao);
    unsigned i = 0;
    if (i < n) {
        int off = 0;
        do {
            if (IntAt(o + 0x3CCC, off) == key && IntAt(o + 0x3CD0, off) == b)
                return IntAt(o + 0x3CD4, off);
            i++;
            off += 0x10;
        } while (i < n);
    }
    return 0;
}

DB_OVERLAY void fn_80546D0C(char* o)
{
    int off = 0;
    int c = 0xCB2;
    do {
        int z = 0;
        IntSet(o + 0x3CC8, off, z);
        IntSet(o + 0x3CCC, off, z);
        IntSet(o + 0x3CD0, off, -1);
        off += 0x10;
        c--;
    } while (c != 0);
    *(int*)(o + 0x107E8) = 0;
}

DB_OVERLAY void fn_80546D4C(char* o)
{
    *(int*)(o + 0x107E8) = 0;
}

DB_OVERLAY void fn_80546D5C(char* o, int a)
{
    int r;
    if (*(int*)(o + 0x107F0) == 1)
        r = fn_805425FC(o + 0x10800, a);
    else
        r = fn_805470F0(o);
    fn_805467E0(o, r);
}

/* near-miss: 1 slot; retail initialises i with `mr r30,r27` (copy of cnt's zero), ours uses li */
/* near-miss: 1 slot; retail initialises i with `mr r30,r27` (copy of cnt's zero), ours uses li */
extern const float lbl_805FF020;
extern "C" int fn_8056087C(void* self);
extern "C" float fn_8007824C(int id, int a);
/* Team list of the roster screen: ids at +0 (count at +4), an iterator vtable at +0x10 (slot 1 = begin, 3 = current, 10 = next), the team-id
 * list shuffled by fn_805479F0 at +0x32E0 and its cursor at +0x32EC. */
struct Db805470F0_LB {
    int* m0;
    unsigned m4;
    unsigned m8;
    int mC;
};
class Db805470F0_V : public Db805470F0_LB {
public:
    virtual void s1();
    virtual void s2();
    virtual int s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void s9();
    virtual void s10();
};
class Db805470F0_O : public Db805470F0_V {
public:
    char pad14[0x32E0 - 0x14];
    int* m32E0;
    int m32E4;
    int m32E8;
    int m32EC;
};
struct Db805470F0_Best {
    int id;
    int score;
    float f;
};
/* K3: picks the best candidate for the current team (the entry of the team-id list at +0x32E0, shuffled by fn_805479F0, at the
 * cursor m32EC): one rand() % 4 chooses the rule (0, 2: every candidate; 1, 3: only candidates whose position kinds a / b suit the
 * team's kind from fn_8056087C) and, among those, keeps the highest rating (rules 0, 1) or the highest float key fn_8007824C (rules 2, 3).
 * Returns the chosen id (0 when none). near-miss: jump2 shares the call tail of the iterator-begin call (3 insns), retail keeps it. */
extern "C" DB_OVERLAY int fn_805470F0(char* oc)
{
    Db805470F0_O* o = (Db805470F0_O*)oc;
    Unk8007A4DC_Screen* s = (Unk8007A4DC_Screen*)lbl_8061930C;
    unsigned i = 0;
    unsigned n = o->m4;
    int m = rand() % 4;
    int kind = fn_8056087C((void*)o->m32E0[o->m32EC]);
    Db805470F0_Best best;
    best.id = 0;
    best.score = 0;
    best.f = lbl_805FF020;
    o->s1();
    for (; i < n; i++) {
        if (fn_80546C88((char*)o, i) == 0) {
            int sc = s->fn_8007A6A0(0x16, o->s3());
            float f = fn_8007824C(o->s3(), 0);
            int a = s->fn_8007A6A0(5, o->s3());
            int b = s->fn_8007A6A0(6, o->s3());
            if (best.id == 0)
                best.id = o->s3();
            switch (m) {
            case 1:
            case 3:
                if (kind == 0 && !(a == 0 || b == 0 || a == 10 || b == 10))
                    break;
                if (kind == 2 && !(a == 2 || b == 2 || a == 12 || b == 12 || a == 13 || b == 13))
                    break;
                if (kind == 3 && !(a == 3 || b == 3 || a == 12 || b == 12 || a == 13 || b == 13))
                    break;
                if (kind == 4 && !(a == 4 || b == 4 || a == 12 || b == 12 || a == 13 || b == 13))
                    break;
                if (kind == 5 && !(a == 5 || b == 5 || a == 12 || b == 12 || a == 13 || b == 13))
                    break;
                if (kind == 1 && !(a == 1 || b == 1 || a == 12 || b == 12 || a == 13 || b == 13))
                    break;
                if ((unsigned)(kind - 6) <= 2 && !(a == 6 || b == 6 || a == 8 || b == 8 || a == 7 || b == 7 || a == 11 || b == 11 || a == 13 || b == 13))
                    break;
            case 0:
            case 2:
                if (((unsigned)m <= 1 && sc >= best.score) || ((unsigned)(m - 2) <= 1 && !(f < best.f))) {
                    best.id = o->s3();
                    best.score = sc;
                    best.f = f;
                }
                break;
            }
        }
        o->s10();
    }
    return best.id;
}

DB_OVERLAY int fn_805474B8(char* o, int a)
{
    if (*(int*)(o + 0x107F0) != 0) {
        unsigned cnt = 0;
        Unk8007A4DC_Screen* s = (Unk8007A4DC_Screen*)lbl_8061930C;
        int key = (*(int**)(o + 0x32E0))[*(int*)(o + 0x32EC)];
        unsigned i = 0;
        int ioff = 0;
        int off = 0;
        do {
            if (fn_80546C88(o, i) != 0 && IntAt(o + 0x3CCC, off) == key
                && s->fn_8007A270(*(int*)((char*)*(int**)o + ioff)) != 0)
                cnt++;
            i++;
            ioff += 4;
            off += 0x10;
        } while (i <= 0xCB1);
        if (s->fn_8007A270(a) != 0 && cnt > 0xE)
            return 2;
    }
    return 0;
}

DB_OVERLAY void fn_80547594(char* o, int a)
{
    if (a == 1) {
        fn_80538258(o);
        *(int*)(o + 0x107F0) = 0;
        return;
    }
    int z = 0;
    *(int*)(o + 0x107F0) = z;
    fn_80546D0C(o);
    char* p = o + 0x32E0;
    fn_80537514(p);
    *(int*)(p + 0xC) = z;
    fn_805381F0(o);
    fn_805362AC(o, 1);
}

DB_OVERLAY void fn_8054788C(void* unused, int mode)
{
    Unk8007B2B0_Screen* sc = (Unk8007B2B0_Screen*)lbl_8061930C;
    Db805495DC_Hold* h = fn_8054BDDC((Db805495DC_Hold*)operator new(8, 0x100), (Unk8007F198_C*)sc);
    if (mode == 0) {
        fn_8054BE40(h, 1);
        fn_8054BEA4(h, 1);
        fn_8054BF08(h, 1);
        fn_8054BF6C(h, 1);
        fn_8054BFD0(h, 1);
    } else if (mode == 1) {
        while ((unsigned)(sc->fn_8007C458() + sc->fn_8007C4C8()) <= 0x4F)
            fn_8054BE40(h, 0);
        while ((unsigned)sc->fn_8007C724() <= 0x4F)
            fn_8054BEA4(h, 0);
        while ((unsigned)sc->fn_8007C948() <= 0x4F)
            fn_8054BF08(h, 0);
        while ((unsigned)sc->fn_8007CB6C() <= 0x77)
            fn_8054BF6C(h, 0);
        while ((unsigned)sc->fn_8007CD90() <= 0x31)
            fn_8054BFD0(h, 0);
    }
    if (h != 0)
        fn_80377408(h);
    sc->fn_80072C28();
    fn_80400CE4(0);
}

/* K3: Fisher-Yates shuffle of the int list at +0x32E0 with rand() (called from the roster/lineup code; seed comes from the global rand state). */
DB_OVERLAY void fn_805479F0(char* o, int kind, int x)
{
    char* oo;
    IntSet(o + 0x10000, 0x7F8, x);
    IntSet(o + 0x10000, 0x7F0, 1);
    oo = o;
    char* l = oo + 0x32E0;
    fn_80546D0C(oo);
    unsigned i = 0;
    fn_8053967C(l);
    if (kind == 0) {
    } else if (kind == 1) {
        Db8053967C_List* ll = (Db8053967C_List*)l;
        if (i < ll->m4) {
            int off = 0;
            do {
                ll = (Db8053967C_List*)(oo + 0x32E0);
                unsigned r;
                if (ll->m4 != i)
                    r = rand() % (ll->m4 - i);
                else
                    r = 0;
                unsigned j = r + i;
                int flag = i >= ll->m4;
                int jo = j * 4;
                int* d = (int*)IntAt(oo, 0x32E0);
                int yv = *(int*)((char*)d + off);
                int xv = *(int*)((char*)d + jo);
                if (flag)
                    ll->m4 = i + 1;
                *(int*)(off + IntAt(oo, 0x32E0)) = xv;
                if (j >= ll->m4)
                    ll->m4 = j + 1;
                *(int*)(jo + IntAt(oo, 0x32E0)) = yv;
                i++;
                off += 4;
            } while (i < ll->m4);
        }
    } else if (kind == 2) {
        fn_805482D4(oo);
    }
    IntSet(oo, 0x32EC, 0);
    fn_8053971C(oo);
    IntSet(oo + 0x10000, 0x7EC, *(int*)(oo + 4));
    fn_805362AC(oo, 1);
    fn_80542580(oo + 0x10800);
    fn_80400CE4(0);
}

/* K3: second copy of the rand() shuffle (list at +0x3500), see fn_805479F0. */
DB_OVERLAY void fn_80547B38(char* o, int kind, int x)
{
    char* oo;
    IntSet(o + 0x10000, 0x7F4, 1);
    IntSet(o + 0x10000, 0x7FC, x);
    oo = o;
    char* l = oo + 0x3500;
    fn_80546D4C(oo);
    unsigned i = 0;
    fn_80535EAC(l);
    if (kind == 0) {
    } else if (kind == 1) {
        Db8053967C_List* ll = (Db8053967C_List*)l;
        if (i < ll->m4) {
            int off = 0;
            do {
                ll = (Db8053967C_List*)(oo + 0x3500);
                unsigned r;
                if (ll->m4 != i)
                    r = rand() % (ll->m4 - i);
                else
                    r = 0;
                unsigned j = r + i;
                int flag = i >= ll->m4;
                int jo = j * 4;
                int* d = (int*)IntAt(oo, 0x3500);
                int yv = *(int*)((char*)d + off);
                int xv = *(int*)((char*)d + jo);
                if (flag)
                    ll->m4 = i + 1;
                *(int*)(off + IntAt(oo, 0x3500)) = xv;
                if (j >= ll->m4)
                    ll->m4 = j + 1;
                *(int*)(jo + IntAt(oo, 0x3500)) = yv;
                i++;
                off += 4;
            } while (i < ll->m4);
        }
    } else if (kind == 2) {
        fn_805483C4(oo);
    }
    IntSet(oo, 0x350C, 0);
    fn_80400CE4(0);
}

DB_OVERLAY void fn_80547C5C(char* o)
{
    unsigned n = *(unsigned*)(o + 0x3504);
    Unk8006A138_It<4> it(0);
    int e = 0;
    do {
        char* l = o + 0x35B8;
        fn_80539EDC(l, e);
        *(int*)(l + 0xC) = 0;
        int j = 0;
        int c = fn_8008FBE8(it.v);
        if (j < c) {
            do {
                unsigned k = 0;
                int id = fn_8008FE14(fn_8008FC40(it.v, j));
                if (k < n) {
                    do {
                        fn_80547E10(o, id);
                        k++;
                    } while (k < n);
                }
                j++;
            } while (j < c);
        }
        e = it.Next();
    } while (e <= 4);
}

DB_OVERLAY int fn_80547E10(char* o, int a)
{
    char* l = o + 0x3500;
    Unk8007B2B0_Screen* s = (Unk8007B2B0_Screen*)lbl_8061930C;
    int x = *(int*)(IntAt(o, 0x35C4) * 4 + IntAt(o, 0x35B8));
    int h = s->fn_8007B380(a, *(int*)(IntAt(l, 0xC) * 4 + IntAt(l, 0)));
    int r;
    if (h == 0) {
        r = fn_80547FFC(o, a, x);
    } else {
        fn_800905FC(fn_8008FD78(fn_8008FED8(a)), h, 3);
        r = 0;
    }
    *(unsigned*)(l + 0xC) = (*(unsigned*)(l + 0xC) + 1) % *(unsigned*)(l + 4);
    return r;
}

DB_OVERLAY int fn_80547FFC(char* o, int a, int id)
{
    if (IntAt(o + 0x10000, 0x7F4) == 1) {
        Unk8007B2B0_Screen* s = (Unk8007B2B0_Screen*)lbl_8061930C;
        char* l = o + 0x3500;
        int u = fn_8008FED8(a);
        struct { float f; int v; } p;
        fn_80564ABC(lbl_80619948->m40, *(int*)(IntAt(l, 0xC) * 4 + IntAt(o, 0x3500)), id, u, &p.f, &p.v, 0);
        int h = s->fn_80079ADC(p.f);
        int w = fn_8008FD78(u);
        fn_80090540(w, id, h);
        fn_800905FC(w, id, p.v);
        s->fn_8007B440(a, *(int*)(IntAt(l, 0xC) * 4 + IntAt(o, 0x3500)), id);
    }
    return 0;
}

DB_OVERLAY void fn_805480E8(char* o)
{
    fn_80546D0C(o);
    fn_805484A0(o);
    *(int*)(o + 0x107EC) = *(int*)(o + 4);
}

DB_OVERLAY void fn_80548128(char* o, int a)
{
    Unk8007B2B0_Screen* s = (Unk8007B2B0_Screen*)lbl_8061930C;
    int h = fn_80083FC0(a);
    Db8053967C_List* l = (Db8053967C_List*)(o + 0x32E0);
    unsigned i = 0;
    if (i < l->m4) {
        int off = 0;
        do {
            int r = s->fn_8007B320(h, s->fn_80075948(*(int*)(IntAt(o, 0x32E0) + off)));
            if (i >= l->m4)
                l->m4 = i + 1;
            i++;
            *(int*)(off + IntAt(o, 0x32E0)) = r;
            off += 4;
        } while (i < l->m4);
    }
    fn_80542580(o + 0x10800);
}

DB_OVERLAY int fn_805481D8(char* o)
{
    Unk8007B2B0_Screen* s = (Unk8007B2B0_Screen*)lbl_8061930C;
    Unk8007F198_B* g = (Unk8007F198_B*)lbl_8061930C;
    int e = *(int*)(IntAt(o, 0x32EC) * 4 + IntAt(o, 0x32E0));
    int h = s->fn_80075948(e);
    unsigned v = s->fn_80078A38(e);
    if (IntAt(o + 0x10000, 0x7F0) == 1) {
        switch (IntAt(o + 0x10000, 0x7F8)) {
        case 1:
            if (v != 0)
                return 0;
            break;
        case 2:
            if (v > 1)
                return 0;
            break;
        case 3:
            if (v > 2)
                return 0;
            break;
        case 4:
            break;
        default:
            return 0;
        }
    }
    unsigned i;
    for (i = 0; i < fn_8007F84C(g); i++) {
        if (fn_8007F7D0(g, i) == h)
            return 1;
    }
    return 0;
}

DB_OVERLAY void fn_805482D4(char* o)
{
    Unk8007B2B0_Screen* s = (Unk8007B2B0_Screen*)lbl_8061930C;
    Unk8007F198_B* g = (Unk8007F198_B*)lbl_8061930C;
    unsigned i = 0;
    int off = 0;
    for (; i < fn_8007F84C(g); off += 4, i++) {
        int id = fn_8007F7D0(g, i);
        Db8053967C_List* l = (Db8053967C_List*)(o + 0x32E0);
        unsigned j = i + 1;
        if (j < l->m4) {
            int joff = j * 4;
            do {
                if (s->fn_80075948(*(int*)(IntAt(o, 0x32E0) + joff)) == id) {
                    int* d = (int*)IntAt(o, 0x32E0);
                    int y = *(int*)((char*)d + off);
                    int x = *(int*)((char*)d + joff);
                    if (i >= l->m4)
                        l->m4 = i + 1;
                    *(int*)(off + IntAt(o, 0x32E0)) = x;
                    if (j >= l->m4)
                        l->m4 = j + 1;
                    *(int*)(joff + IntAt(o, 0x32E0)) = y;
                    break;
                }
                j++;
                joff += 4;
            } while (j < l->m4);
        }
    }
}

/* near-miss: 97.7%; the inner loop's back edge reloads the list count before the increments in retail, after them in ours */
DB_OVERLAY void fn_805483C4(char* o)
{
    Unk8007F198_B* g = (Unk8007F198_B*)lbl_8061930C;
    unsigned i = 0;
    int off = 0;
    for (; i < fn_8007F84C(g); i++, off += 4) {
        int id = fn_8007F7D0(g, i);
        Db8053967C_List* l = (Db8053967C_List*)(o + 0x3500);
        unsigned j = i + 1;
        if (j < l->m4) {
            int joff = j * 4;
            do {
                int* d = (int*)IntAt(o, 0x3500);
                int x = *(int*)((char*)d + joff);
                if (x == id) {
                    int y = *(int*)((char*)d + off);
                    if (i >= l->m4)
                        l->m4 = i + 1;
                    *(int*)(off + IntAt(o, 0x3500)) = x;
                    if (j >= l->m4)
                        l->m4 = j + 1;
                    *(int*)(joff + IntAt(o, 0x3500)) = y;
                    break;
                }
                j++;
                joff += 4;
            } while (j < l->m4);
        }
    }
}

DB_OVERLAY Ev* fn_80548858(Ev* e)
{
    e->m28 = lbl_80682D68;
    fn_80548908(e);
    return e;
}

DB_OVERLAY void fn_80548894(Ev* e, int a, int b, int c, V* v)
{
    if (e->m0 < v->m0 || a == e->m20) {
        e->m20 = a;
        e->m18 = b;
        *(Db805467E0_W3*)e = *(Db805467E0_W3*)v;
        e->m1C = c;
        e->m24 = 0;
    }
}

DB_OVERLAY void fn_805488E0(Ev* e, int v)
{
    e->m24 = v;
}

DB_OVERLAY void fn_805488E8(Ev* e, V* v)
{
    e->vC = *(Db805467E0_W3*)v;
}

DB_OVERLAY void fn_80548908(Ev* e)
{
    e->m0 = lbl_805FF024;
    e->m18 = 0;
    e->m8 = 0;
    e->m4 = 0;
    e->m20 = 0;
    e->m1C = 0;
    e->m24 = 0;
}

DB_OVERLAY Q* fn_80548934(Q* o)
{
    *(const void**)((char*)o + 0x3FD4) = lbl_80682D50;
    Ev* e = o->ev;
    int c = 0x4A;
    do {
        fn_80548858(e);
        e++;
    } while (c-- != 0);
    fn_8036179C(&o->sCE8);
    fn_8054899C(o);
    return o;
}

DB_OVERLAY void fn_8054899C(Q* q)
{
    int e = (int)q->ev;
    int end = (int)q + 0xCB8;
    do {
        fn_80548908((Ev*)e);
        e += 0x2C;
    } while (e <= end);
    q->sCE8.m0 = 6;
    q->mCE4 = 0;
    Db805467E0_S* s = &q->sCE8;
    s->m4 = 0;
    s->m8 = 0;
}

/* near-miss: 98.4%; retail stores the last copied m28 to an 8-byte frame slot after the loop (frame 24 vs ours 16); loop body matches */
DB_OVERLAY void fn_805489FC(Q* q, int i)
{
    if ((unsigned)i < (unsigned)q->mCE4) {
        int last = q->mCE4 - 1;
        int j = i;
        for (; j < last; j++) {
            Ev* p = &q->ev[j];
            const void* keep = p->m28;
            *p = q->ev[j + 1];
            p->m28 = keep;
        }
        fn_80548908(&q->ev[j]);
        q->mCE4--;
    }
}

DB_OVERLAY void fn_80548B08(Q* q, int v)
{
    int i = q->mCE4 - 1;
    if (i >= 0) {
        Ev* p = (Ev*)(i * 0x2C + (int)q);
        do {
            if (p->m24 == v)
                fn_805489FC(q, i);
            p--;
            i--;
        } while (i >= 0);
    }
}

DB_OVERLAY int fn_80548B6C(Ev* e, int id)
{
    int i = 0;
    do {
        if (e->m18 != id) {
            e++;
            i++;
            continue;
        }
        return i;
    } while (i < 75);
    return -1;
}

DB_OVERLAY int fn_80548BA0(Q* q, int a, int id, V* v, int c, int flag)
{
    if (q->mCE4 == 0x4B) {
        fn_80548B08(q, 2);
        if (q->mCE4 == 0x4B) {
            fn_80548B08(q, 1);
            if (q->mCE4 == 0x4B)
                fn_805489FC(q, 0);
        }
    }
    int i = fn_80548B6C(q->ev, id);
    if (i == -1) {
        i = q->mCE4;
        q->mCE4 = i + 1;
    }
    Ev* e = (Ev*)((char*)q + i * 0x2C);
    fn_80548894(e, a, id, c, v);
    if (flag != 0)
        fn_805488E8(e, v);
    return i;
}

DB_OVERLAY void fn_80548C7C(Q* q, int a, int b)
{
    V tmp;
    int i = fn_80548B6C(q->ev, a);
    if (i != -1) {
        tmp.m4 = 0;
        tmp.m0 = lbl_805FF028;
        fn_805488E8(&q->ev[i], &tmp);
        if (b == fn_80548E0C(q, i))
            fn_805489FC(q, i);
    }
}

DB_OVERLAY int fn_80548CFC(Ev* q, int id)
{
    return fn_80548B6C(q, id) == -1 ? 0 : 1;
}

DB_OVERLAY int* fn_80548D2C(Q* q, int id)
{
    int i = fn_80548B6C(q->ev, id);
    if (i != -1)
        return &((Ev*)(i * 0x2C + (int)q))->m24;
    return &lbl_80622BEC;
}

DB_OVERLAY void* fn_80548D78(Q* q, int id)
{
    int i = fn_80548B6C(q->ev, id);
    if (i != -1)
        return (Ev*)(i * 0x2C + (int)q);
    return &lbl_80692E38;
}

DB_OVERLAY int fn_80548DC0(Q* q, int id)
{
    int i = fn_80548B6C(q->ev, id);
    if (i != -1)
        return ((Ev*)(i * 0x2C + (int)q))->m20;
    return lbl_80622BF4;
}

DB_OVERLAY int fn_80548E0C(Q* q, int i)
{
    Ev* e = (Ev*)(i * 0x2C + (int)q);
    return e->m20;
}

DB_OVERLAY int fn_80548E1C(Q* q, int i)
{
    Ev* e = (Ev*)(i * 0x2C + (int)q);
    return e->m18;
}

DB_OVERLAY int* fn_80548E2C(Q* q, int i)
{
    Ev* e = (Ev*)(i * 0x2C + (int)q);
    return &e->m24;
}

DB_OVERLAY Ev* fn_80548E3C(Q* q, int i)
{
    return (Ev*)(i * 0x2C + (int)q);
}

DB_OVERLAY Db805467E0_W3* fn_80548E48(Q* q, int i)
{
    Ev* e = (Ev*)(i * 0x2C + (int)q);
    return &e->vC;
}

DB_OVERLAY int* fn_80548E58(Q* q, int i)
{
    Ev* e = (Ev*)(i * 0x2C + (int)q);
    return &e->m1C;
}

DB_OVERLAY void fn_80548E68(Q* q, int n)
{
    q->sCE8.m0 = 0;
    Db805467E0_S* s = &q->sCE8;
    s->m8 = n;
    s->m4 = 0;
    Db8053967C_List* l = &q->mCF4;
    fn_8040074C(l->m0, l->m8 * 4);
    l->m4 = 0;
    l->mC = 0;
    l->m14 = 0;
}

DB_OVERLAY void fn_805494DC(char* o, int id, int a, int b, int c, int flag, int d, float f)
{
    Db805467E0_T3 v1;
    Db805467E0_T3 v2;
    Db805467E0_T3 v3;
    v1.f = f;
    v1.a = b;
    v1.b = c;
    v2.f = *(float*)(o + 4);
    v2.a = *(int*)(o + 0xC);
    v2.b = *(int*)(o + 0x1C);
    v3.f = *(float*)(o + 8);
    v3.a = *(int*)(o + 0x10);
    v3.b = *(int*)(o + 0x18);
    if (flag != 0) {
        fn_800A1930(lbl_80619948, id, b, d, v1.f);
    } else {
        Unk8007B2B0_Screen* s = (Unk8007B2B0_Screen*)lbl_8061930C;
        int h = s->fn_80075948(a);
        if (fn_80077BC4(s, id) != 0)
            fn_800793E8(s, id, &v1, &v2, &v3);
        else
            s->fn_80079094(id, h, (int)&v1, (int)&v2, (int)&v3, d);
    }
}

}
