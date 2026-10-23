/*
 * EA gamelib (C:/mvp2004/libraries/gamelib: memory, datafile, animlib), built with
 * the game flags (cflags_game: -Os -G0 -ffloat-store),
 * .text 0x80387E28-0x8038B938 (code-only unit, 38 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

#include "game/chooser.h"

struct DtorAt0 { const void* m; };

/* Slot pools of a gamelib object: two arrays of 15 slots (0x14 bytes each), at +0x1C and
 * +0x148; bit 3 of the flags argument selects the second. Evidence: fn_80387E28 zeroes m0 of
 * 15 slots from +0x1C or +0x148; fn_80387EEC claims the first slot with m4 == 0. */
struct GlSlot { int m0; int m4; int m8; int mC; union { int m10; float m10f; }; };

struct GlCb {
    virtual void v1(int, int, int);
    virtual void v2(int, int, int);
    virtual void v3(int, int, int);
};

struct GlPool {
    char pad0[0x10];
    int m10;
    char pad14[0x1C - 0x14];
    GlSlot a1C[15];
    GlSlot a148[15];
    char pad274[0x284 - 0x274];
    float m284;
    char m288;
    char m289;
};

extern GlCb* lbl_80624278;

struct GlV3 {
    float x, y, z;
};

struct E1FC_Q {
    float x;
    float y;
    float z;
    float w;
};
extern E1FC_Q lbl_806E9928;
struct E1FC_Obj {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual int v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void v22();
    virtual void v23();
    virtual void v24();
    virtual void v25();
    virtual void v26(GlV3* out);
    virtual void v27(GlV3* out);
    virtual void v28(GlV3* out);
    virtual void v29(GlV3* out);
};
struct E1FC_G {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual int v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual int v9();
    virtual int v10();
};
struct E1FC_Owner {
    char pad0[0x18];
    E1FC_Obj* m18;
    char pad1C[0x290 - 0x1C];
    char* m290;
};
extern "C" void fn_803895A4(void* self, char* a, float* out, float f0, float f1);

extern "C" void fn_80387E28(GlPool* self, int flags)
{
    GlSlot* b = (flags & 8) ? self->a148 : self->a1C;
    for (int i = 0; i < 15; i++)
        b[i].m0 = 0;
}

extern "C" void fn_80387E58(GlPool* self, int flags)
{
    GlSlot* b = (flags & 8) ? self->a148 : self->a1C;
    for (int i = 0; i < 15; i++) {
        GlSlot* s = &b[i];
        if (s->m0 == 0 && s->m4 != 0) {
            lbl_80624278->v3(s->m4, s->m8, s->mC);
            s->m4 = 0;
            s->m8 = 0;
            s->mC = 0;
        }
    }
}

extern "C" int fn_80387EEC(GlPool* self, int flags)
{
    GlSlot* b = (flags & 8) ? self->a148 : self->a1C;
    int i;
    for (i = 0; i < 15; i++) {
        GlSlot* s = &b[i];
        if (s->m4 == 0) {
            s->m0 = 1;
            return i;
        }
    }
    for (i = 0; i < 15; i++) {
        GlSlot* s = &b[i];
        if (s->m0 == 0) {
            lbl_80624278->v3(s->m4, s->m8, s->mC);
            s->m0 = 1;
            s->m4 = 0;
            s->m8 = 0;
            s->mC = 0;
            return i;
        }
    }
    return -1;
}

extern "C" int fn_80387FC8(GlPool* self, int flags, int id, float f)
{
    GlSlot* b = (flags & 8) ? self->a148 : self->a1C;
    for (int i = 0; i < 15; i++) {
        GlSlot* s = &b[i];
        if (id == s->m4) {
            s->m0 = 1;
            return i;
        }
    }
    return -1;
}

extern "C" void fn_80388020(GlPool* self, int flags)
{
    if (flags & 8)
        self->m10 = 0;
    else
        self->m10 = 1;
    fn_80387E28(self, flags);
    self->m288 = 0;
}

struct GlBig {
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
    virtual void v12();
    virtual void v13();
    virtual int v14();
    virtual int v15();
    virtual void v16();
    virtual int v17();
    virtual int v18();
};

struct GlBufs {
    char* m0;
    char* m4;
    char* m8;
};
extern GlBufs lbl_806242D4;
extern "C" void* MEM_copy(void* d, const void* s, int n);

extern "C" void fn_8038806C(GlPool* self, int flags)
{
    fn_80387E58(self, flags);
    if ((flags & 8) && self->m288 != 0) {
        int a = ((GlBig*)lbl_80624278)->v18();
        int b = ((GlBig*)lbl_80624278)->v17();
        if (self->m288 & 1) {
            int n = a * 0x30;
            MEM_copy(lbl_806242D4.m0 + ((GlBig*)lbl_80624278)->v15() * 0x30, lbl_806242D4.m8, n);
            MEM_copy(lbl_806242D4.m0 + ((GlBig*)lbl_80624278)->v14() * 0x30, lbl_806242D4.m8 + n, b * 0x30);
        }
        if (self->m288 & 2) {
            int n = a * 0x30;
            MEM_copy(lbl_806242D4.m4 + ((GlBig*)lbl_80624278)->v15() * 0x30, lbl_806242D4.m8, n);
            MEM_copy(lbl_806242D4.m4 + ((GlBig*)lbl_80624278)->v14() * 0x30, lbl_806242D4.m8 + n, b * 0x30);
        }
    }
}

struct GlEntry {
    char pad0[4];
    char* m4;
    char pad8[8];
    float m10;
    float m14;
    float m18;
    char pad1C[0x68 - 0x1C];
};

extern float lbl_80605DA8;
extern "C" int fn_80388490(GlPool* self, int flags, GlEntry* arr, int n, int* out);

inline void CopyV3(GlV3* d, const GlV3* s)
{
    *(float*)d = s->x;
    d->y = s->y;
    d->z = s->z;
}
inline void E1FC_Scl(GlV3* v, float k)
{
    v->x *= k;
    v->y *= k;
    v->z *= k;
}
inline void E1FC_CopyQ(E1FC_Q* d, const E1FC_Q* s)
{
    d->x = s->x;
    d->y = s->y;
    d->z = s->z;
    d->w = s->w;
}
extern "C" void fn_803881FC(E1FC_Owner* o, int flag)
{
    E1FC_Obj* p = o->m18;
    GlV3 a;
    GlV3 b;
    if (flag == 0) {
        p->v26(&a);
        p->v27(&b);
        E1FC_Scl(&a, 4.235f);
        E1FC_Scl(&b, 4.235f);
        char* b1 = o->m290;
        int off1 = ((E1FC_G*)lbl_80624278)->v9() * 0x30 + 0x20;
        CopyV3((GlV3*)(b1 + off1), &a);
        char* b2 = o->m290;
        int off2 = ((E1FC_G*)lbl_80624278)->v10() * 0x30 + 0x20;
        CopyV3((GlV3*)(b2 + off2), &b);
    }
    int off = ((E1FC_G*)lbl_80624278)->v5();
    if (off != -1) {
        off *= 0x30;
        p->v28(&a);
        char* b3 = o->m290;
        CopyV3((GlV3*)(b3 + off), &a);
        if (p->v9() == 0) {
            p->v29(&b);
            float k = 4.235f;
            b.x *= k;
            b.y *= k;
            b.z *= k;
            int o2 = off + 0x20;
            char* b4 = o->m290;
            CopyV3((GlV3*)(b4 + o2), &b);
            int o3 = off + 0x10;
            char* b5 = o->m290;
            E1FC_CopyQ((E1FC_Q*)(b5 + o3), &lbl_806E9928);
        }
    }
}

/* near-miss: 57/59 slots; retail loads the compare operand f1 (stack reload) before the zero constant f13, ours the reverse */
extern "C" void fn_8038871C(GlPool* self, GlEntry* arr, int n, int flags)
{
    if (n != 0) {
        GlSlot* b = (flags & 8) ? self->a148 : self->a1C;
        int idx[8];
        fn_80388490(self, flags, arr, n, idx);
        for (int i = 0; i < n; i++) {
            int k = idx[i];
            float f = lbl_80605DA8;
            if (k >= 0)
                f = b[k].m10f;
            else
                f = self->m284;
            char* a = arr[i].m4;
            arr[i].m14 = arr[i].m10;
            if (!(f <= lbl_80605DA8))
                fn_803895A4(self, a, &arr[i].m10, f, arr[i].m18);
        }
    }
}

struct GlV11 {
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
    virtual int v11();
};

struct GlF2 {
    float m0;
    float m4;
};

struct GlV12 {
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
    virtual GlF2* v12();
    virtual void v13();
    virtual GlF2* v14();
    virtual void v15(GlV3* p);
    virtual void v16(float f);
};

struct GlOwner18 {
    char pad0[0x18];
    GlV12* m18;
};

extern float lbl_80605DD0;


struct Gl8035Vec3 {
    float x, y, z;
};
extern Gl8035Vec3 lbl_806055AC;
extern Gl8035Vec3 lbl_80695940;
struct E9BD_Mat {
    float m[16];
};
struct E9BD_Obj {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual int v10();
    virtual int v11();
    virtual Gl8035Vec3* v12();
    virtual void v13();
    virtual Gl8035Vec3* v14();
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual E9BD_Mat* v21();
};
struct E9BD_Owner {
    char pad0[0x14];
    int m14;
    E9BD_Obj* m18;
    char pad1C[0x28C - 0x1C];
    int m28C;
    int m290;
};
extern "C" {
void PoseSQTToGlobal(int a, int b, void* c, int d);
void fn_803DF824(void* p, float a, float b, float c, float d, float e, float f, float g, float h, float i);
void fn_803DF6AC(void* p, float a, float b, float c, float d);
void fn_803DFEE0(void* p, float a, float b, float c, float d);
void fn_803E02E4(void* p, void* q);
}
extern char lbl_8068F200[];
extern int lbl_806242A0;

struct Unk80379324 {
    Unk80379324();
    float x, y, z, w;
};

struct Unk8035A618;
extern "C" void fn_8035A618(Unk8035A618* o);
extern "C" int fn_8035F248(int a, Unk8035A618* o);

/* Local transform object (size 0x9C, vptr at +0x98; same class as in include/game/warp_b.h). */
struct Unk8035A618 {
    Unk8035A618() { fn_8035A618(this); }
    virtual ~Unk8035A618() {}
    virtual void fn_vslot2(float x, float y, float z);
    virtual void fn_vslot3(const Gl8035Vec3& v);
    void A() { m6C = 0; }
    void B() { m0 = 0; }
    void V70(const Gl8035Vec3& v) { m70 = v; }
    void V60(const Gl8035Vec3& v) { m60 = v; }
    int m0;
    char pad4[0x5C];
    Gl8035Vec3 m60;
    int m6C;
    Gl8035Vec3 m70;
    Unk80379324 m7C;
    float m8C;
    float m90;
    float m94;
};

extern "C" int fn_8038950C(int arg0, float f)
{
    Unk8035A618 t;
    t.V60(lbl_806055AC);
    t.B();
    t.A();
    t.V70(lbl_806055AC);
    fn_8035F248(arg0, &t);
    return arg0;
}

extern "C" void fn_803BB7A8(int a, int b, int c, int d, void* e, float f);
extern char lbl_80699B14[];

extern float lbl_80605DB4;
extern float lbl_80605DB8;
extern float lbl_80605DBC;

extern "C" void fn_803895A4(void* self, char* a, float* out, float f0, float f1)
{
    float* v = 0;
    if (a != 0)
        v = (float*)(a + 4);
    float t10, t14, t18, t1c, t20;
    t10 = f0 - lbl_80605DB4;
    t18 = lbl_80605DB8;
    t1c = lbl_80605DB4;
    t14 = *out * t10;
    if (v != 0) {
        t18 = *v;
        t1c = lbl_80605DB4;
    }
    if (!(t18 > lbl_80605DB8))
        t18 = lbl_80605DBC;
    if (!(t1c > lbl_80605DB8))
        t1c = lbl_80605DB4;
    t20 = f1 * t18 * t1c;
    t14 += t20;
    *out = t14 / t10;
}

extern "C" void fn_803899CC(GlPool* self, float f, int a, int b, int idx)
{
    fn_803BB7A8(self->m289, b, a, a, lbl_80699B14 + idx * 32, f);
}

extern "C" void fn_80389A1C(GlOwner18* self, char* base)
{
    int off = ((GlV11*)lbl_80624278)->v11() * 0x30 + 0x20;
    GlV3 v;
    CopyV3(&v, (GlV3*)(base + off));
    self->m18->v16(v.y);
    v.y = self->m18->v14()->m4;
    GlV3 w = v;
    self->m18->v15(&w);
}

struct GlB40 { char d[0x40]; };
struct GlB28 { GlV3 v; char pad[0x28 - 12]; };

extern "C" void fn_80389BD8(void* a, int b, void* c, int d);
extern "C" void ExtractRotTrans(void* m, void* a, void* b);
extern "C" void fn_803DE51C(void* m, int a, void* b);

extern "C" int fn_80389B34(void* x, int a, GlV3* p1, GlV3* p2, int p3, int flags);

extern "C" void fn_80389B10(void* x, int a, GlV3* p1, GlV3* p2, int p3)
{
    fn_80389B34(x, a, p1, p2, p3, 3);
}

extern "C" int fn_80389B34(void* x, int a, GlV3* p1, GlV3* p2, int p3, int flags)
{
    GlB40 m;
    GlB28 t;
    GlV3 u;
    fn_80389BD8(x, a, &m, flags);
    if (p3 == 0) {
        void* q = p2;
        if (q == 0)
            q = &t;
        void* r = p1;
        if (r == 0)
            r = &u;
        ExtractRotTrans(&m, q, r);
    } else {
        fn_803DE51C(&m, p3, &t);
        if (p1 != 0)
            *p1 = t.v;
    }
    return 1;
}

extern "C" void fn_80389BD8(void* ov, int idx, void* matv, int flags)
{
    E9BD_Owner* o = (E9BD_Owner*)ov;
    E9BD_Mat* mat = (E9BD_Mat*)matv;
    E9BD_Obj* p = o->m18;
    char* tbl = lbl_8068F200;
    int hit;
    if (flags & 2) {
        PoseSQTToGlobal(o->m14, o->m28C, tbl, 0);
        hit = p->v10();
    } else {
        hit = p->v11();
        PoseSQTToGlobal(o->m14, o->m290, tbl, 0);
    }
    if (flags & 1) {
        if (flags & 2) {
            Gl8035Vec3 a = lbl_806055AC;
            Gl8035Vec3 b = *p->v12();
            if (lbl_806242A0 == 0) {
                a = *p->v14();
                fn_803DF824(mat, b.x, b.y, b.z, 0.0f, 0.0f, 0.0f, a.x, a.y, a.z);
            } else {
                a = lbl_80695940;
                fn_803DF824(mat, b.x, b.y, b.z, 0.0f, 0.0f, 0.0f, a.x, a.y, a.z);
            }
        } else {
            *mat = *p->v21();
        }
    } else {
        Gl8035Vec3 a = *p->v12();
        fn_803DF6AC(mat, a.x, a.y, a.z, 1.0f);
    }
    if (hit != 0)
        fn_803DFEE0(mat, -1.0f, 1.0f, 1.0f, 1.0f);
    fn_803E02E4(mat, tbl + (idx << 6));
}

extern "C" void fn_80389EBC(GlPool* self)
{
    int off = 0;
    for (int i = 0; i < 15; i++) {
        char* b24 = (char*)self + 0x24;
        char* b150 = (char*)self + 0x150;
        int v = *(int*)(b24 + off);
        if (v != 0) {
            char* b20 = (char*)self + 0x20;
            char* b28 = (char*)self + 0x28;
            lbl_80624278->v3(*(int*)(b20 + off), v, *(int*)(b28 + off));
        }
        int w = *(int*)(b150 + off);
        if (w != 0) {
            char* b14C = (char*)self + 0x14C;
            char* b154 = (char*)self + 0x154;
            lbl_80624278->v3(*(int*)(b14C + off), w, *(int*)(b154 + off));
        }
        char* c20 = (char*)self + 0x20;
        *(int*)(c20 + off) = 0;
        *(int*)(b24 + off) = 0;
        char* c28 = (char*)self + 0x28;
        *(int*)(c28 + off) = 0;
        char* c14C = (char*)self + 0x14C;
        *(int*)(c14C + off) = 0;
        *(int*)(b150 + off) = 0;
        char* c154 = (char*)self + 0x154;
        *(int*)(c154 + off) = 0;
        off += 0x14;
    }
}

extern "C" void fn_80389FAC(GlOwner18* self, char* base, int flag)
{
    int off = ((GlV11*)lbl_80624278)->v11() * 0x30 + 0x20;
    GlV3 v;
    CopyV3(&v, (GlV3*)(base + off));
    v.x = lbl_80605DD0;
    v.z = lbl_80605DD0;
    if (flag != 0)
        v.y /= self->m18->v12()->m4;
    CopyV3((GlV3*)(base + off), &v);
}

struct GlVt13 {
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
    virtual void v12();
    virtual int v13();
};

#define PUT(o, v) (*(const void**)((char*)self + (o)) = (v))

extern char lbl_806850A0[];
extern char lbl_80684E18[];
extern char lbl_806626C0[];
extern char lbl_80684F48[];
extern char lbl_806059C0[];
extern int lbl_8069593C;

extern "C" void fn_8037B424(void* p, int c);
extern "C" void fn_80378620(void*, int);

extern "C" void fn_80383394(void* buf);
extern "C" void* fn_803BE684(void* self, int flag);
extern const char lbl_80605A18[];
extern const char lbl_80605A24[];
extern const char lbl_80605A30[];
extern const char lbl_80605A3C[];
extern const char lbl_80605A48[];
extern const char lbl_80605A50[];
extern const char* lbl_806242A8;
extern int lbl_8069594C;
extern int lbl_8069595C;
extern int lbl_80695960;
extern int lbl_80695964;
extern int lbl_80695968;
extern int lbl_80699B0C;
extern int lbl_80699B10;
extern char lbl_80695950[];
struct Gl12Elem {
    short m0;
    char pad2[2];
    char m4;
    char pad5[7];
};
extern Gl12Elem lbl_8069596C[];

/* Static initializer of the globals at 0x8069593C..0x80699B14 (init flag, priority 0xFFFF), written by hand:
 * the unit split puts its _GLOBAL_ wrappers in another object, so a compiler-generated copy
 * would be a global with a different symbol name.
 * near-miss: 2 of 109 slots differ (struct copy: retail emits the base addi before the first lwz). */
extern "C" void fn_8038B630(int init, int prio)
{
    if (prio == 0xFFFF) {
        if (init != 0) {
            { int h = fn_80378568(lbl_806059C0, 0); lbl_8069593C = h; }
            lbl_80695940 = lbl_806055AC;
            lbl_8069594C = fn_80378568(lbl_806242A8, 0);
            fn_80383394(lbl_80695950);
            { int h = fn_80378568(lbl_80605A18, 0); lbl_8069595C = h; }
            { int h = fn_80378568(lbl_80605A24, 0); lbl_80695960 = h; }
            { int h = fn_80378568(lbl_80605A30, 0); lbl_80695964 = h; }
            { int h = fn_80378568(lbl_80605A3C, 0); lbl_80695968 = h; }
            Gl12Elem* e = lbl_8069596C;
            for (int i = 0; i < 0x578; i++) {
                e->m0 = 0;
                e->m4 = -1;
                e++;
            }
            int k = 3;
            { int h = fn_80378568(lbl_80605A48, 0); lbl_80699B0C = h; }
            { int h = fn_80378568(lbl_80605A50, 0); lbl_80699B10 = h; }
            char* p = lbl_80699B14;
            do {
                fn_803BE684(p, 0);
                p += 0x20;
            } while (k-- != 0);
        } else {
            Gl12Elem* a = lbl_8069596C;
            if (a != 0) {
                Gl12Elem* q = a + 0x578;
                if (q != lbl_8069596C) {
                    do {
                        q--;
                    } while (q != lbl_8069596C);
                }
            }
        }
    }
}

extern "C" void fn_8038B7E4(void* self, int c)
{
    PUT(0x0, lbl_806850A0);
    fn_8037B424(self, c);
}

extern "C" int fn_8038B814()
{
    return 0;
}

extern "C" void fn_8038B81C()
{
}

extern "C" int fn_8038B820()
{
    return 1;
}

extern "C" const char* fn_8038B854(DtorAt0*, int);

extern "C" void fn_8038B828(DtorAt0* s, int chrg)
{
    fn_8038B854(s, chrg);
}

extern "C" int fn_8038B848()
{
    return lbl_8069593C;
}

extern "C" const char* fn_8038B854(DtorAt0*, int)
{
    return lbl_806059C0;
}

extern "C" int fn_8038B860()
{
    return 0;
}

extern "C" int fn_8038B868(GlVt13* self)
{
    return self->v13();
}

extern "C" int fn_8038B89C()
{
    return 0;
}

extern "C" int fn_8038B8A4()
{
    return 0;
}

extern "C" void fn_8038B8AC(void* self, int c)
{
    PUT(0x8, lbl_80684F48);
    fn_80378620(self, c);
}

extern "C" void fn_8038B8DC(void* self, int c)
{
    PUT(0x0, lbl_80684E18);
    ((Unk80384F28*)((char*)self + 4))->Unk80384F28::~Unk80384F28();
    PUT(0x0, lbl_806626C0);
    fn_8037B424(self, c);
}
