/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803D2B20-0x803D50E4 (code-only unit, 34 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 *
 * Three sibling classes with a vptr at +0 (vtables lbl_80686518, lbl_806865D0, lbl_80686688), a common base
 * (ctor fn_803BE8EC, dtor fn_803BE90C) and a data record pointer at +0xC. The vtable slots used here are
 * declared as virtuals that are never defined, so no vtable is emitted.
 */

#include "../../include/game/eagl_key_803d2d74.h"

extern "C" {
int fn_803D2D74(void* table, Unk803D2B20Key* key, int* out);
void fn_803BE8EC(void* self);
void fn_803BE90C(void* self, int c);
void Invisible(int a, int b);
void SetBone(unsigned int* set, int k, int v);
}

/* Data record at +0xC of the objects. */
struct Unk803D2B20Rec {
    char pad0[2];
    unsigned short m2;
    int m4;
    unsigned short* m8;
    unsigned char* mC;
    unsigned short m10;
    unsigned char m12;
    char pad13;
    unsigned short m14;
};

class Unk803D2B20 {
public:
    virtual ~Unk803D2B20();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7(int a, float f, int z);
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void* v16();
    virtual void v17();
    virtual int v18(int a, int* b, int* c, int* d);
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void v22();
};

struct Unk803D2B20Obj {
    const void* m0;
    int m4;
    int m8;
    Unk803D2B20Rec* mC;
    short m10;
    unsigned char m12;
    unsigned char m13;
};

struct Unk803D2BC0Quat {
    float x, y, z, w;
};
struct Unk803D2BC0Self {
    char pad0[0x48];
    Unk803D2BC0Quat q;
};
struct Unk803D2BC0Other {
    char pad0[0x10];
    Unk803D2BC0Quat q;
};

struct Unk803D39C0Rec {
    char pad0[0xC];
    unsigned char* mC;
    Unk803D2B20* m10;
    char pad14[2];
    unsigned char m16;
};
struct Unk803D39C0Obj {
    char pad0[0xC];
    Unk803D39C0Rec* mC;
};

struct Unk803D4A7CRec {
    char pad0[0xC];
    unsigned short* mC;
    char pad10[2];
    unsigned char m12;
};
struct Unk803D4A7CObj {
    char pad0[0xC];
    Unk803D4A7CRec* mC;
};

extern const void* lbl_80686518[];
extern const void* lbl_806865D0[];
extern const void* lbl_80686688[];
extern void (*lbl_8062C66C)(void* p, int size);

extern "C" void fn_803D2B20(Unk803D2B20* self, int a, float x, float y)
{
    self->v7(a, y, 0);
}

extern "C" int fn_803D2B5C()
{
    return 0;
}

extern "C" int fn_803D2B64(float x, float lo, float hi)
{
    float range = hi - lo;
    float t;
    if (x < lo) {
        t = lo - x;
        return (int)(t / range);
    }
    if (x >= hi) {
        t = x - hi;
        return (int)(t / range) + 1;
    }
    return 0;
}

inline void QMul(Unk803D2BC0Quat* t, Unk803D2BC0Quat* a, Unk803D2BC0Quat* b)
{
    t->x = b->w * a->x + (b->z * a->y + (b->x * a->w - b->y * a->z));
    t->y = b->w * a->y + ((b->x * a->z + b->y * a->w) - b->z * a->x);
    t->z = b->w * a->z + (b->z * a->w + (-b->x * a->y + b->y * a->x));
    t->w = ((-b->x * a->x - b->y * a->y) - b->z * a->z) + b->w * a->w;
}

extern "C" void fn_803D2BC0(Unk803D2BC0Self* self, Unk803D2BC0Other* other)
{
    Unk803D2BC0Quat t;
    QMul(&t, &self->q, &other->q);
    other->q = t;
}

extern "C" float fn_803D2CBC(float x, float lo, float hi)
{
    float range = hi - lo;
    float t;
    int k;
    if (x < lo) {
        t = lo - x;
        float m;
        k = (int)(t / range);
        m = t - (float)k * range;
        return hi - m;
    }
    if (x >= hi) {
        t = x - hi;
        float m;
        k = (int)(t / range);
        m = t - (float)k * range;
        return lo + m;
    }
    return x;
}


/* near-miss (5 instructions short): sorted-table search by 16-bit key. Retail keeps a stack copy of the key
   (sth r0,8(r1)) while comparing a register copy, tests lo <= hi at the top of the loop, and returns the
   entry through the table register (li r3,0 on a miss). The stack copy only survives when the key
   struct's address is passed on; the register copy then costs an extra clrlwi here. */
extern "C" int fn_803D2D74(void* table, Unk803D2B20Key* key, int* out)
{
    struct Ent {
        unsigned short m0;
        unsigned short pad2;
        int m4;
    };
    int count = *(int*)table;
    int lo = 0;
    int hi = count - 1;
    unsigned short k = key->m0;
    Ent* e = 0;
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        Ent* c = (Ent*)((char*)table + mid * 8 + 4);
        if (k > c->m0) {
            lo = mid + 1;
        } else if (k < c->m0) {
            hi = mid - 1;
        } else {
            e = c;
            break;
        }
    }
    if (e) {
        *out = e->m4;
        return 1;
    }
    return 0;
}

extern "C" unsigned short fn_803D3748(Unk803D2B20Obj* self)
{
    return self->mC->m2;
}

extern "C" Unk803D2B20Obj* fn_803D3754(Unk803D2B20Obj* self)
{
    fn_803BE8EC(self);
    self->m0 = lbl_80686518;
    self->m8 = 0x16;
    self->m10 = 0;
    return self;
}

extern "C" void fn_803D37A0(Unk803D2B20Obj* self, int c)
{
    self->m0 = lbl_80686518;
    fn_803BE90C(self, 0);
    if (c & 1)
        lbl_8062C66C(self, 0x18);
}

extern "C" void fn_803D3800(Unk803D2B20Obj* self, Unk803D2B20Rec* r)
{
    self->mC = r;
}

extern "C" int fn_803D3808(Unk803D2B20Obj* self, float* out)
{
    Unk803D2B20Rec* r = self->mC;
    int cnt;
    if (r->m8 == 0)
        cnt = r->m14;
    else
        cnt = r->m8[r->m14 - 2] + 1;
    *out = (float)cnt;
    return 1;
}

extern "C" int fn_803D386C(Unk803D2B20Obj* self)
{
    return self->mC->m4;
}

extern "C" void fn_803D3878(Unk803D2B20* self, int a, float x, float y)
{
    self->v7(a, y, 0);
}

/* near-miss (4 instructions short): retail copies the key twice on the stack (0x18, 0x10) after the virtual
   call, passes &key and a result slot at 0x20(r1), and reads the result byte with lbz 0x20(r1). */
extern "C" void fn_803D38B4(Unk803D2B20Obj* self, int flag)
{
    self->m12 = flag;
    if (self->m13 == 0 && self->m12 != 0) {
        Unk803D2B20Key key(1);
        void* tbl = ((Unk803D2B20*)self)->v16();
        if (tbl) {
            Unk803D2B20Key k2(key);
            Unk803D2B20Key k3(k2);
            int res;
            if (fn_803D2D74(tbl, &k3, &res))
                self->m13 = ((unsigned char*)&res)[0];
        }
    }
}

extern "C" int fn_803D394C(Unk803D2B20* self, int a, int* b, int* c, int* d)
{
    Invisible(a, 0);
    *b = 0;
    return self->v18(a, b, c, d);
}

inline int BitTest(unsigned* set, int id)
{
    int o = (id >> 5) * 4;
    return (*(unsigned*)((char*)set + o) & (1 << (id & 31))) != 0;
}

extern "C" int fn_803D39C0(Unk803D39C0Obj* self, unsigned* set, int* count, int* lo, int* hi)
{
    Unk803D39C0Rec* rec = self->mC;
    for (int k = rec->m16 - 1; k >= 0; k--) {
        unsigned char id = rec->mC[k];
        if (!BitTest(set, id)) {
            if (*count == 0) {
                *lo = id;
                *hi = id;
            } else {
                if (id < *lo)
                    *lo = id;
                if (id > *hi)
                    *hi = id;
            }
            *count = *count + 1;
            SetBone(set, id, 1);
        }
    }
    Unk803D2B20* child = rec->m10;
    if (child)
        return child->v18((int)set, count, lo, hi);
    return 1;
}

extern "C" unsigned short fn_803D480C(Unk803D2B20Obj* self)
{
    return self->mC->m2;
}

extern "C" Unk803D2B20Obj* fn_803D4818(Unk803D2B20Obj* self)
{
    fn_803BE8EC(self);
    self->m0 = lbl_806865D0;
    self->m8 = 0x17;
    return self;
}

extern "C" void fn_803D485C(Unk803D2B20Obj* self, int c)
{
    self->m0 = lbl_806865D0;
    fn_803BE90C(self, 0);
    if (c & 1)
        lbl_8062C66C(self, 0x18);
}

extern "C" void fn_803D48BC(Unk803D2B20Obj* self, Unk803D2B20Rec* r)
{
    self->mC = r;
}

extern "C" int fn_803D48C4(Unk803D2B20Obj* self, float* out)
{
    Unk803D2B20Rec* r = self->mC;
    int cnt;
    if (r->m8 == 0)
        cnt = r->m10;
    else
        cnt = r->m8[r->m10 - 2] + 1;
    *out = (float)(unsigned short)cnt;
    return 1;
}

extern "C" int fn_803D4928(Unk803D2B20Obj* self)
{
    return self->mC->m4;
}

extern "C" void fn_803D4934(Unk803D2B20* self, int a, float x, float y)
{
    self->v7(a, y, 0);
}

/* near-miss: same shape as fn_803D38B4 (twin class). */
extern "C" void fn_803D4970(Unk803D2B20Obj* self, int flag)
{
    self->m12 = flag;
    if (self->m13 == 0 && self->m12 != 0) {
        Unk803D2B20Key key(1);
        void* tbl = ((Unk803D2B20*)self)->v16();
        if (tbl) {
            Unk803D2B20Key k2(key);
            Unk803D2B20Key k3(k2);
            int res;
            if (fn_803D2D74(tbl, &k3, &res))
                self->m13 = ((unsigned char*)&res)[0];
        }
    }
}

extern "C" int fn_803D4A08(Unk803D2B20* self, int a, int* b, int* c, int* d)
{
    Invisible(a, 0);
    *b = 0;
    return self->v18(a, b, c, d);
}

extern "C" int fn_803D4A7C(Unk803D4A7CObj* self, unsigned* set, int* count, int* lo, int* hi)
{
    Unk803D4A7CRec* rec = self->mC;
    for (int k = rec->m12 - 1; k >= 0; k--) {
        unsigned short id = rec->mC[k] / 12;
        if (!BitTest(set, id)) {
            if (*count == 0) {
                *lo = id;
                *hi = id;
            } else {
                if (id < *lo)
                    *lo = id;
                if (id > *hi)
                    *hi = id;
            }
            *count = *count + 1;
            SetBone(set, id, 1);
        }
    }
    return 1;
}

extern "C" unsigned short fn_803D508C(Unk803D2B20Obj* self)
{
    return self->mC->m2;
}

extern "C" Unk803D2B20Obj* fn_803D5098(Unk803D2B20Obj* self)
{
    fn_803BE8EC(self);
    self->m0 = lbl_80686688;
    self->m8 = 0x19;
    self->m10 = 0;
    return self;
}

/*
 * Not matched yet: fn_803D2DFC, fn_803D33C4, fn_803D3AC8, fn_803D408C, fn_803D44A8, fn_803D4B60 (uses sqrtf).
 * fn_803D33C4 (p5-20 notes): packed-half reads are ((h & 0x8000) << 16) | ((h & 0x7FFF) << 15) stored to a stack word
 * and reloaded as a float; eight distinct slots come from one unsigned w[8]; the walk is lhzu through *++p; stores use
 * out[k], out[++k] with k = bone * 12; section 3 re-reads rec->m16. That form gives 225 of 225 instructions, but retail
 * keeps n in r31 (cur takes r3), ours puts n in r3. The original code stays linked for these.
 */
