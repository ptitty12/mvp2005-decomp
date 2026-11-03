/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803D50E4-0x803D7830 (code-only unit, 34 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

extern "C" {
unsigned strlen(const char* s);
int strncmp(const char* a, const char* b, unsigned n);
}

struct Mtx4 {
    float m[16];
};

/* Sequence of 16-byte records (name pointer at +0): begin at +0, count at +4. */
struct Unk803D50E4Rec {
    const char* m0;
    int m4;
    int m8;
    int mC;
};
struct Unk803D50E4Seq {
    Unk803D50E4Rec* m0;
    int m4;
};
struct Unk803D50E4It {
    Unk803D50E4Rec* p;
};

/* Scene-like object: child pointer array at +8 (count at +0x38), scale at +0x54, int table at +0x5C. */
struct Unk803D50E4Obj {
    int m0;
    int m4;
    void** m8;
    int mC;
    char pad10[0x38 - 0x10];
    int m38;
    int m3C;
    char pad40[0x54 - 0x40];
    float m54;
    int pad58;
    int* m5C;
};

/* Matrix-like object: 4x4 floats at +0 (identity by default), int words at +0x40..+0x50. */
struct Unk803D50E4Mat {
    float m[14];
    float m38;
    float m3C;
    int m40;
    int m44;
    int m48;
    int m4C;
    int m50;
};

struct Unk803D50E4Rec6 {
    char pad0[6];
    unsigned short m6;
};

struct Unk803D50E4Dev {
    char padDC[0xDC];
    int mDC;
};

extern int lbl_806AA900[];
extern int lbl_806279B8;

extern "C" {
void fn_803DFA2C(Mtx4* dst, Mtx4* src);
void fn_803DFCB0(Mtx4* dst, float x, float y, float z, float w);
void fn_803E1C44(void* o, Mtx4* m);
void fn_803D74EC(Unk803D50E4Dev* d);
void fn_803D7830(void);
/* The retail return copies five words at offsets 0 through 0x10. */
struct Result803D6528 { int words[5]; };
Result803D6528 fn_803D6528(Unk803D50E4Obj* self, int a);
void fn_803D675C(Unk803D50E4Obj* self, void* t, int b);
void fn_803D7414(int a, int b);
void fn_803D7600(struct Unk803D7600Obj* self, int c);
}

/* Source of the constructor below: three data words, then the vptr at +0xC; slot 3 returns two floats (width, height). */
struct Unk803D5E84SrcData {
    int m0;
    int m4;
    int m8;
};
struct Unk803D5E84Src : Unk803D5E84SrcData {
    virtual void v1();
    virtual void v2();
    virtual void v3(float* a, float* b);
};
struct Unk803D5E84Obj {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    char pad14[0x24 - 0x14];
    int m24;
    int m28;
    int m2C;
    int m30;
    int m34;
    int m38;
    int m3C;
    int m40;
    int m44;
    int m48;
    int m4C;
    int m50;
    float m54;
    int m58;
    int m5C;
    int m60;
    int m64;
};
struct Unk803D74ECObj {
    char pad0[0x9C];
    int m9C;
    char padA0[0xCC - 0xA0];
    int* mCC;
};
extern "C" int fn_803DA4B8(int a, int b);
extern void (*lbl_8062C66C)(void* p, int size);
extern "C" void fn_803E6708(void* p, int c);
struct Unk803D7600Obj {
    char pad0[0x44];
    int m44;
    int m48;
    void* m4C;
    void** m50;
};
/* Source of fn_803D5D34: three data words, vptr at +0xC; slot 1 begins and slot 2 ends the update. */
struct Unk803D5D34SrcData {
    int m0;
    int m4;
    int m8;
};
struct Unk803D5D34Src : Unk803D5D34SrcData {
    virtual void v1();
    virtual void v2();
};
struct Unk803D5D34Blk {
    int w[40];
};
struct Unk803D5D34Obj {
    char pad0[4];
    Unk803D5D34Src* m4;
    void** m8;
    char padC[0x28 - 0xC];
    int m28;
    char pad2C[0x34 - 0x2C];
    char* m34;
    int m38;
    char pad3C[0x58 - 0x3C];
    Unk803D5D34Blk* m58;
    Unk803D5D34Blk** m5C;
};
extern "C" {
int fn_803E7320(int ch, const char* s, ...);
void fn_803E0F9C(void* p);
void fn_803E1E88(void* p);
void fn_803D68AC(void* obj, void* mtx);
void fn_803D71F0(int p);
}

extern "C" void fn_803D50E4(Unk803D50E4Obj* self, int v)
{
    self->mC = v;
    *(short*)((char*)self + 0x10) = 0;
}

extern "C" int fn_803D50F4(int unused, float* out)
{
    int z = 0;
    *out = (float)z;
    return 1;
}

struct Unk803D7ED4Item;
class Unk803D7ED4Pool {
public:
    void fn_803D803C(Unk803D7ED4Item* item);
};
int fn_803D81FC();

struct Unk803D5130Geo {
    char pad0[0x24];
    void* m24;
    char pad28[4];
    void* m2C;
};
struct Unk803D5130Obj {
    char pad0[4];
    Unk803D7ED4Item* m4;
    void* m8;
    void* mC;
    Unk803D5130Geo* m10;
    int m14;
    char pad18[0x24 - 0x18];
    void* m24;
    void* m28;
    char pad2C[4];
    void* m30;
    char pad34[4];
    int m38;
    char pad3C[0x58 - 0x3C];
    void* m58;
    void* m5C;
};
extern "C" {
void* fn_803DD7C0(void* o);
void MEM_free(void* o);
void fn_803DD73C(void* o, int c);
void fn_803E9744(void* o, int c);
void fn_803DBD84(void* o, int c);
}

extern "C" void fn_803D5130(Unk803D5130Obj* self, int c)
{
    if (fn_803D81FC() != 0) {
        if (self->m4 != 0) {
            ((Unk803D7ED4Pool*)fn_803D81FC())->fn_803D803C(self->m4);
            self->m4 = 0;
            if (self->m8 != 0) {
                lbl_8062C66C(self->m8, self->m38 * 4);
                self->m8 = 0;
            }
        }
        self->m4 = 0;
    }
    if (self->mC != 0) {
        MEM_free(fn_803DD7C0(self->mC));
        if (self->mC != 0)
            fn_803DD73C(self->mC, 3);
        self->mC = 0;
    }
    if (self->m30 != 0) {
        fn_803E9744(self->m30, 3);
        self->m30 = 0;
    }
    if (self->m5C != 0) {
        lbl_8062C66C(self->m5C, self->m38 * 4);
        self->m5C = 0;
    }
    if (self->m10 != 0) {
        if (self->m10->m24 != 0)
            lbl_8062C66C(self->m10->m24, self->m14 * 12);
        if (self->m10->m2C != 0)
            lbl_8062C66C(self->m10->m2C, self->m14 * 16);
        if (self->m24 != 0) {
            fn_803DBD84(self->m24, 3);
            self->m24 = 0;
        }
        if (self->m58 != 0) {
            lbl_8062C66C(self->m58, 0xA0);
            self->m58 = 0;
        }
        lbl_8062C66C(self->m10, 0x38);
        self->m10 = 0;
    }
    if (self->m28 != 0) {
        fn_803D7600((Unk803D7600Obj*)self->m28, 3);
        self->m28 = 0;
    }
    if (c & 1)
        lbl_8062C66C(self, 0x68);
}

extern "C" int fn_803D5D34(Unk803D5D34Obj* self)
{
    if (self->m4 == 0) {
        fn_803E7320(0, "ERROR : SphereMapManager::CreateSphereMap() - SphereMapManager::SetupSphereMaps() must be called before this function is used
");
        return 0;
    }
    self->m4->v1();
    for (int i = 0; i < self->m38; i++) {
        fn_803E0F9C(self->m8[i]);
        *self->m58 = *self->m5C[i];
        if (self->m34)
            fn_803D68AC(self->m34, self->m34 + 0xC);
        else
            fn_803D71F0(self->m28);
        fn_803E1E88(self->m8[i]);
    }
    self->m4->v2();
    return 1;
}

extern "C" Unk803D5E84Obj* fn_803D5E84(Unk803D5E84Obj* self, int tag, Unk803D5E84Src* src)
{
    self->m0 = tag;
    self->m54 = 1.0f;
    self->m4 = 0;
    self->m8 = 0;
    self->mC = 0;
    self->m10 = 0;
    self->m24 = 0;
    self->m28 = 0;
    self->m2C = 0;
    self->m30 = 0;
    self->m34 = 0;
    self->m38 = 0;
    self->m3C = 0;
    self->m40 = 0;
    self->m44 = 0;
    self->m48 = 0;
    self->m4C = 0;
    self->m50 = 0;
    self->m58 = 0;
    self->m5C = 0;
    if (src) {
        float w;
        float h;
        src->v3(&w, &h);
        self->m60 = (int)w;
        self->m64 = (int)h;
    } else {
        self->m60 = 0;
        self->m64 = 0;
    }
    return self;
}

extern "C" void fn_803D5F6C(Unk803D50E4Obj* self, int v)
{
    self->m38 = v;
}

extern "C" void fn_803D5F74(Unk803D50E4Obj* self, int v)
{
    self->m3C = v;
}

/*
 * Not attempted: the large functions fn_803D5130 (126), fn_803D5328 (219), fn_803D5694 (424), fn_803D5D34 (84),
 * fn_803D5E84 (58), fn_803D6144 (249), fn_803D6528 (141), fn_803D675C (84), fn_803D68AC (246), fn_803D6C84 (226),
 * fn_803D707C (93), fn_803D71F0 (98), fn_803D74EC (53), fn_803D7600 (61); fn_803D5F7C and fn_803D5FE8 convert
 * integer fields with an unused sixth parameter copy; fn_803D75C0 is a pointer/value pair copy loop.
 * The original code stays linked for these.
 */

extern "C" void fn_803D5F7C(Unk803D50E4Mat* self, float* a, float* b)
{
    *a = 1.0f / (float)self->m4C;
    *b = 1.0f / (float)self->m50;
}

extern "C" void fn_803D5FE8(Unk803D50E4Mat* self, int idx, float* u, float* v)
{
    int w = self->m4C;
    *u = (float)(idx % w) / (float)w;
    *v = (float)(idx / w) / (float)self->m50;
}

extern "C" void fn_803D6098(Unk803D50E4Obj* self, Mtx4* src)
{
    Mtx4 m;
    int i = 0;
    fn_803DFA2C(&m, src);
    float s = self->m54;
    fn_803DFCB0(&m, s, s, s, 1.0f);
    for (; i < self->m38; i++) {
        void* o = self->m8[i];
        if (o)
            fn_803E1C44(o, &m);
    }
}

extern "C" int fn_803D6124(Unk803D50E4Obj* self)
{
    return self->mC;
}

extern "C" void fn_803D612C(Unk803D50E4Obj* self, int idx, int v)
{
    int arr = (int)self->m5C;
    if (arr == 0)
        return;
    *(int*)(arr + idx * 4) = v;
}

extern "C" Unk803D50E4Mat* fn_803D700C(Unk803D50E4Mat* self)
{
    self->m40 = 0;
    self->m44 = 0;
    self->m48 = 0;
    self->m4C = 0;
    self->m[0] = 1.0f;
    self->m[1] = 0.0f;
    self->m[2] = 0.0f;
    self->m50 = 0;
    self->m[3] = 0.0f;
    self->m[4] = 0.0f;
    self->m[5] = 1.0f;
    self->m[6] = 0.0f;
    self->m[7] = 0.0f;
    self->m[8] = 0.0f;
    self->m[9] = 0.0f;
    self->m[10] = 1.0f;
    self->m[11] = 0.0f;
    self->m[12] = 0.0f;
    self->m[13] = 0.0f;
    self->m38 = 0.0f;
    self->m3C = 1.0f;
    return self;
}

struct Unk803D707CObj {
    char pad0[0x44];
    int m44;
    int m48;
    int* m4C;
    int* m50;
};
extern void* (*lbl_8062C668)(int size, const char* tag);
extern "C" void* memset(void* p, int c, unsigned n);

extern "C" int fn_803D707C(Unk803D707CObj* self, int item)
{
    int cap = self->m44;
    if (self->m48 >= cap) {
        if (cap > 15)
            self->m44 = cap * 2;
        else
            self->m44 = 16;
        int* np = (int*)lbl_8062C668(self->m44 * 4, "Dynamic Model Geoprim list");
        for (int i = 0; i < self->m48; i++)
            np[i] = self->m4C[i];
        if (self->m4C)
            lbl_8062C66C(self->m4C, cap * 4);
        self->m4C = np;
        int* np2 = (int*)lbl_8062C668(self->m44 * 4, "Dynamic Model Draw Array list");
        memset(np2, 0, self->m44 * 4);
        for (int i = 0; i < self->m48; i++)
            np2[i] = self->m50[i];
        if (self->m50)
            lbl_8062C66C(self->m50, cap * 4);
        self->m50 = np2;
    }
    self->m4C[self->m48++] = item;
    return self->m48 - 1;
}

struct Unk803D71F0Obj {
    Mtx4 m0;
    char pad40[8];
    int m48;
    int m4C;
    void** m50;
};
extern Mtx4 lbl_806AAA10;
extern Mtx4 lbl_806AA9D0;
extern Mtx4 lbl_806AAA50;
extern Mtx4 lbl_806AAA90;
extern Mtx4 lbl_806AAAD0;
extern int lbl_80627290;
extern int lbl_806279C0;
extern int lbl_806279C4;
extern void (*lbl_806279C8)();
extern "C" {
void PSMTX44Concat(Mtx4* a, Mtx4* b, Mtx4* c);
int fn_803D9DF4();
void fn_803D9528();
int fn_803E6D00(void* a, void* b);
int fn_803E6C28(void* a, int b);
}

extern "C" void fn_803D71F0(int p)
{
    Unk803D71F0Obj* self = (Unk803D71F0Obj*)p;
    lbl_806AAA10 = self->m0;
    PSMTX44Concat(&lbl_806AAA10, &lbl_806AAA90, &lbl_806AAAD0);
    PSMTX44Concat(&lbl_806AAA10, &lbl_806AA9D0, &lbl_806AAA50);
    lbl_80627290 = 0;
    if (fn_803D9DF4() != 0)
        fn_803D9528();
    lbl_806279C0 = 0;
    lbl_806279C4 = -1;
    for (int i = 0; i < self->m48; i++) {
        if (self->m50[i] != 0) {
            fn_803E6D00(self->m50[i], self);
            fn_803E6C28(self->m50[i], ((int*)self->m50[i])[0x34 / 4]);
        } else {
            void* o = ((void**)self->m4C)[i];
            (**(void (***)(void*))o)(o);
        }
    }
    if (lbl_806279C8 != 0) {
        lbl_806279C8();
        lbl_806279C8 = 0;
    }
}

/* near-miss (2 instructions longer): the reverse search over the 16-byte records; retail keeps the
   record pointer one element ahead and rebuilds the end pointer on exit. */
extern "C" Unk803D50E4It fn_803D7378(Unk803D50E4Seq* s, const char* name)
{
    Unk803D50E4It it;
    Unk803D50E4Rec* p = s->m0 + s->m4;
    Unk803D50E4Rec* stop = s->m0 - 1;
    for (; p != stop; p--) {
        if (strncmp(name, p->m0, strlen(name)) == 0) {
            it.p = p + 1;
            return it;
        }
    }
    it.p = s->m0 + s->m4;
    return it;
}

extern "C" void fn_803D7414(int a, int b)
{
    if (b == 0xFFFF && a != 0) {
        lbl_806AA900[0] = 4;
        lbl_806AA900[1] = 4;
        lbl_806AA900[2] = 1;
    }
}

extern "C" void fn_803D744C(Unk803D50E4Dev* self)
{
    self->mDC = lbl_806279B8 - 5;
    fn_803D74EC(self);
}

extern "C" void fn_803D7480()
{
    fn_803D7830();
}

extern "C" void fn_803D74A0(Unk803D50E4Obj* self, int a, int b)
{
    Result803D6528 t = fn_803D6528(self, a);
    fn_803D675C(self, &t, b);
}

extern "C" void fn_803D74EC(Unk803D50E4Dev* d)
{
    Unk803D74ECObj* self = (Unk803D74ECObj*)d;
    int* p = self->mCC;
    int v = *p++;
    if (v == 0) {
        if (self->m9C == 0) {
            int cnt = *p++;
            int idx = 0;
            while (cnt-- != 0)
                fn_803DA4B8(*p++, idx++);
        } else {
            for (int i = 0; i < self->m9C; i++) {
                int idx = 0;
                int cnt = *p++;
                while (cnt-- != 0)
                    fn_803DA4B8(*p++, idx++);
            }
        }
    }
}

extern "C" void fn_803D75C0(void* self, int* list)
{
    if (list) {
        int n = list[0];
        if (n > 0) {
            int* p = list + 1;
            int k = n;
            while (k-- != 0) {
                int* d = (int*)*p++;
                *d = *p++;
            }
        }
    }
}

extern "C" void fn_803D7600(Unk803D7600Obj* self, int c)
{
    if (self->m4C)
        lbl_8062C66C(self->m4C, self->m44 * 4);
    self->m4C = 0;
    if (self->m50) {
        for (int i = 0; i < self->m48; i++) {
            if (self->m50[i])
                fn_803E6708(self->m50[i], 3);
            self->m50[i] = 0;
        }
        lbl_8062C66C(self->m50, self->m44 * 4);
    }
    self->m50 = 0;
    if (c & 1)
        lbl_8062C66C(self, 0x58);
}

extern "C" void fn_803D76F4(Mtx4* dst, Mtx4* src)
{
    *dst = *src;
}

/* near-miss (4 instructions shorter): the forward search; retail recomputes the end pointer (count * 16 +
   begin) after every step and stores it in a stack slot (stw r0,8(r1)) that is never read, as if the end
   iterator were a struct temporary. Tried an inline struct-returning EndOf() in three loop shapes (plain
   register use, or an extra flag); none gives the stack store. */
extern "C" Unk803D50E4It fn_803D775C(Unk803D50E4Seq* s, const char* name)
{
    Unk803D50E4It it;
    it.p = s->m0;
    while (it.p != s->m0 + s->m4) {
        if (strncmp(name, it.p->m0, strlen(name)) == 0)
            break;
        it.p++;
    }
    return it;
}

extern "C" int fn_803D77EC(Unk803D50E4Rec6* e)
{
    if (e->m6 & 3)
        return 1;
    return 0;
}

extern "C" void fn_803D7804()
{
    fn_803D7414(1, 0xFFFF);
}

/*
 * Not matched yet: fn_803D5328, fn_803D5694, fn_803D6144, fn_803D6528, fn_803D675C (iterator
 * compare loop), fn_803D68AC, fn_803D6C84. The original code stays linked for these.
 */
