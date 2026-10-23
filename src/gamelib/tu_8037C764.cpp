/*
 * EA gamelib (C:/mvp2004/libraries/gamelib: memory, datafile, animlib), built with
 * the game flags (cflags_game: -Os -G0 -ffloat-store),
 * .text 0x8037C764-0x8037E64C (code-only unit, 45 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */
#include "game/chooser.h"
#include "game/tu_80377048.h"

struct V3F {
    float x;
    float y;
    float z;
};
struct Unk8037D768 {
    int m0;
    char pad4[0x1C];
    char m20[0x40];
    V3F m60;
    int m6C;
    V3F m70;
    float m7C;
    float m80;
    float m84;
    float m88;
    float m8C;
    float m90;
    float m94;
};
struct V4S {
    int m0;
    int m4;
    int m8;
    int mC;
};
extern "C" {
void fn_803E00EC(void* p, float a, float b, float c, float d, float e, float f, float g, float h, float i, float j);
void fn_803DF824(void* p, float a, float b, float c, float d, float e, float f, float g, float h, float i);
void fn_803DEE7C(void* p, V4S* a, V4S* b);
}
extern const float lbl_8060590C;


struct SortArrDel {
    char pad0[0x14];
    virtual ~SortArrDel();
};
struct VDel0 {
    virtual ~VDel0();
};
extern "C" VDel0* fn_803780A0(SortArr* a, int idx);
struct Ent16 {
    int m0;
    int m4;
    int m8;
    int mC;
};
struct Unk8037E600 {
    char pad0[8];
    Ent16* m8;
};
struct V3S {
    int m0;
    int m4;
    int m8;
};
struct Unk8037D92C {
    char pad0[0x60];
    V3S m60;
};
struct Ent16F {
    int m0;
    int m4;
    float m8;
    float mC;
    Ent16F()
    {
        m0 = 0;
        m4 = 0;
        m8 = 0.0f;
        mC = 0.0f;
    }
};
struct Unk8037E118 {
    char pad0[4];
    int m4;
    Ent16F* m8;
};
extern "C" int atoi(const char* s);
extern "C" double atof(const char* s);
void* operator new[](unsigned int size, int heap);
extern "C" void fn_8037E600(Unk8037E600* o, int idx);
struct Unk80379324 {
    Unk80379324();
    float x;
    float y;
    float z;
    float w;
};
struct E3B8_V3 {
    float x;
    float y;
    float z;
};
struct E3B8_Ent {
    int m0;
    int m4;
    float m8;
    float mC;
};
struct E3B8_Arg0 {
    char pad0[8];
    E3B8_Ent* m8;
};
struct E3B8_VB31 {
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void s9();
    virtual void s10();
    virtual void s11();
    virtual void s12();
    virtual void s13();
    virtual int s14();
    virtual int s15();
};
struct E3B8_Obj31 : E3B8_VB31 {
    E3B8_V3 m4;
    Unk80379324 m10;
};
struct E3B8_Arg4 {
    virtual void s1();
    virtual void s2(int a, E3B8_V3* b, int c, Unk80379324* d);
};
struct E3B8_Arg5 {
    char pad0[8];
    virtual void s1();
    virtual void s2(int a, int b, int c);
};
struct E3B8_VB27 {
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void s9();
    virtual void s10();
    virtual void s11();
    virtual void s12(int a, E3B8_Arg5* b, float f1, float f2);
    virtual E3B8_Obj31* s13();
};
struct V4F {
    float x;
    float y;
    float z;
    float w;
};
struct Unk8037D958 {
    char pad0[0x98];
    virtual void s1();
    virtual void s2();
    virtual void s3(V3F* r);
};
inline V3F operator-(const V3F& a, const V3F& b)
{
    V3F r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}
inline V3F operator+(const V3F& a, const V3F& b)
{
    V3F r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    return r;
}
inline V3F operator*(const V3F& v, float s)
{
    V3F r;
    r.x = v.x * s;
    r.y = v.y * s;
    r.z = v.z * s;
    return r;
}
struct Unk8037DAD0 {
    char pad0[0x6C];
    int m6C;
    V3S m70;
    V4F m7C;
};


extern "C" {
int fn_80401904(void* h);
int fn_804017B0(void* h, void* buf);
void MEM_free(void* h);
void fn_8037D2B0(void* tbl, void* data, int size);
char* strcat(char* dst, const char* src);
char* strcpy(char* dst, const char* src);
}
extern const char lbl_80605820[];
extern const char lbl_80605824[];


struct VDel {
    char pad0[0xC];
    virtual ~VDel();
};

struct Unk8037CC30 {
    VDel* m0;
    VDel* m4;
    VDel* m8;
    VDel* mC;
};

extern int lbl_80624290;
extern int lbl_80624294;

extern "C" {
int fn_803FEDB8(void* p, int v);
int fn_8037CA4C(void* self, const char* name, void* field, int id);
void fn_8037CBB4(void* self, void* field, int id);
}

struct Unk8037D0E8 {
    SortArr* m0;
    void* m4;
    void* m8;
    const void* mC;
};
struct Unk8037D0E8_P {
    void* m0;
};
extern const char lbl_80605830[];
extern const char lbl_806849B8[];
extern int lbl_80624288;
extern const char lbl_80685080[];

extern "C" {
void* fn_803779B8(void* mem, int cap, int heap, int flag, const char* path, int line);
void* fn_803774A8(unsigned size, int heap);
void fn_8037B8B4(int x);
void fn_80377408(void* p);
void fn_80377488(void* p);
int fn_8037C5D8(int x);
int fn_80377E90(void* arr, int* o1, int* o2, int key);
void fn_8037D768(struct Unk8037D768* self);
void fn_803DED40(void* a, int b, int c);
}
unsigned fn_80378568(const char* s, int n);
void* operator new(unsigned int size, int heap);

struct Unk8037C884 {
    char pad0[0x18];
    void* m18;
};

struct Unk8037D0E8_Q {
    Unk8037D0E8_P* m0;
};
struct Unk8037C7FC_F {
    Unk8037D0E8_P* m0;
    Unk8037D0E8_P* m4;
    Unk8037D0E8_P* m8;
    Unk8037D0E8_P* mC;
};
struct VDel1C {
    char pad0[0x1C];
    virtual ~VDel1C();
};
struct VG2 {
    virtual void* s1(const char* path, int flags, int* outSize);
    virtual int s2();
};
class Unk8037C704 {
public:
    Unk8037C704();
    char pad0[0x20];
};
extern "C" {
int fn_8037D6E4(Unk8037D0E8_P* o, const char* name, int y);
int fn_8037D680(Unk8037D0E8_P* o, int x, int y);
int fn_8037D72C(Unk8037D0E8_P* o, int key);
void fn_8037C8F4(Unk8037C884* o);
void fn_8037CDA4(Unk8037CC30* x);
struct DtorAt0;
void fn_8037D290(DtorAt0* s, int chrg);
Unk8037D0E8* fn_8037D140(Unk8037D0E8* o, int a, int count);
int fn_803FED48(const char* name, int a, int b, void* out);
void fn_8037C884(void* self, const char* name);
}
extern VDel1C* lbl_80624260;
extern VG2* lbl_8062426C;
extern const char lbl_806057E0[];
extern const char lbl_806057F8[];
extern const char lbl_80605808[];
extern const char lbl_80605814[];
extern int lbl_8062428C;
extern int lbl_8062429C;
extern int lbl_80624298;
extern int lbl_8061A5A8;
extern void* lbl_80624254;

extern "C" void fn_8037C764(Unk8037CC30* o)
{
    fn_8037C8F4((Unk8037C884*)o);
    fn_8037CDA4(o);
    if (o->m0 != 0)
        delete o->m0;
    o->m0 = 0;
    ((void (*)())fn_8037D290)();
    if (lbl_80624260 != 0)
        delete lbl_80624260;
    lbl_80624260 = 0;
}

Unk8037C7FC* fn_8037C7FC()
{
    if (lbl_80624260 == 0) {
        lbl_80624260 = (VDel1C*)new (0) Unk8037C704;
        fn_8037C884(lbl_80624260, lbl_806057E0);
        int flag = lbl_80624254 != 0;
        if (flag == 0)
            fn_8037B8B4(lbl_80624288);
    }
    return (Unk8037C7FC*)lbl_80624260;
}

extern "C" void fn_8037C884(void* self, const char* name)
{
    Unk8037C884* o = (Unk8037C884*)self;
    o->m18 = 0;
    if (lbl_8062426C->s2())
        fn_803FED48(name, 0, 0, &o->m18);
}

extern "C" void fn_8037C8F4(Unk8037C884* o)
{
    if (o->m18 != 0)
        fn_803FEDB8(o->m18, 100);
}

extern "C" int fn_8037C924(void* self)
{
    int r = fn_8037CA4C(self, lbl_806057F8, self, lbl_8062428C);
    if (r != 0)
        r = fn_8037CA4C(self, lbl_80605808, self, lbl_8062429C);
    return r;
}

extern "C" int fn_8037C988(Unk8037CC30* o)
{
    int r = 0;
    if (o->m4 == 0) {
        int saved = lbl_806EF9B4;
        lbl_806EF9B4 = lbl_8061A5A8;
        r = fn_8037CA4C(o, lbl_80605814, &o->m4, lbl_80624298);
        lbl_806EF9B4 = saved;
    }
    return r;
}

extern "C" void fn_8037C9F4(Unk8037CC30* o, const char* name)
{
    fn_8037CA4C(o, name, &o->m8, lbl_80624290);
}

extern "C" void fn_8037CA20(Unk8037CC30* o, const char* name)
{
    fn_8037CA4C(o, name, &o->mC, lbl_80624294);
}

extern "C" int fn_8037CA4C(void* selfv, const char* name, void* fieldv, int id)
{
    Unk8037C884* self = (Unk8037C884*)selfv;
    void** field = (void**)fieldv;
    char buf[0x80] = "";
    int sz;
    if (self->m18 == 0)
        strcpy(buf, "datafile/");
    strcat(buf, name);
    void* h = lbl_8062426C->s1(buf, 0x100, &sz);
    if (h != 0) {
        void* data = h;
        int size = sz;
        if (self->m18 != 0) {
            size = fn_80401904(h);
            data = fn_803774A8(size + 0x400, 0x100);
            int n = fn_804017B0(h, data);
            ((char*)data)[n] = 0;
        }
        if (*field == 0)
            fn_8037CBB4(self, field, id);
        if (*field != 0)
            fn_8037D2B0(*field, data, size);
        if (self->m18 != 0 && data != 0)
            fn_80377488(data);
        MEM_free(h);
    }
    return 1;
}

extern "C" void fn_8037CBB4(void* self, void* field, int id)
{
    VDel** f = (VDel**)field;
    if (*f != 0) {
        delete *f;
        *f = 0;
    }
    *f = (VDel*)fn_8037D140((Unk8037D0E8*)operator new(0x10, 0), id, lbl_80624288);
}

extern "C" void fn_8037CC30(Unk8037CC30* o)
{
    if (o->m4 != 0) {
        delete o->m4;
        o->m4 = 0;
    }
}

extern "C" void fn_8037CC88(Unk8037CC30* o)
{
    if (o->m8 != 0)
        delete o->m8;
    o->m8 = 0;
}

extern "C" void fn_8037CCE0(Unk8037CC30* o)
{
    if (o->mC != 0)
        delete o->mC;
    o->mC = 0;
}

extern "C" void fn_8037CD38(Unk8037CC30* o)
{
    if (o->mC != 0)
        delete o->mC;
    int id = lbl_80624294;
    o->mC = 0;
    fn_8037CBB4(o, &o->mC, id);
}

extern "C" void fn_8037CDA4(Unk8037CC30* x)
{
    fn_8037CC30(x);
    fn_8037CC88(x);
    fn_8037CCE0(x);
}

extern "C" int fn_8037CDE0(Unk8037C7FC_F* m, const char* name, int key)
{
    int r = 0;
    if (m->m8 != 0)
        r = fn_8037D6E4(m->m8, name, key);
    if (r == 0 && m->mC != 0)
        r = fn_8037D6E4(m->mC, name, key);
    if (r == 0 && m->m4 != 0)
        r = fn_8037D6E4(m->m4, name, key);
    if (r == 0 && m->m0 != 0)
        r = fn_8037D6E4(m->m0, name, key);
    return r;
}

extern "C" int fn_8037CE98(Unk8037C7FC_F* m, int hash, int key, int flag)
{
    int r = 0;
    if (m->m8 != 0)
        r = fn_8037D680(m->m8, hash, key);
    if (r == 0 && m->mC != 0)
        r = fn_8037D680(m->mC, hash, key);
    if (r == 0 && m->m4 != 0)
        r = fn_8037D680(m->m4, hash, key);
    if (flag == 0 && r == 0 && m->m0 != 0)
        r = fn_8037D680(m->m0, hash, key);
    return r;
}

extern "C" int fn_8037CF5C(Unk8037C7FC_F* m, int key, const char* name)
{
    unsigned h = fn_80378568(name, 0);
    int r = 0;
    if (m->m8 != 0)
        r = fn_8037D680(m->m8, key, h);
    if (r == 0 && m->mC != 0)
        r = fn_8037D680(m->mC, key, h);
    if (r == 0 && m->m4 != 0)
        r = fn_8037D680(m->m4, key, h);
    if (r == 0 && m->m0 != 0)
        r = fn_8037D680(m->m0, key, h);
    return r;
}

extern "C" int fn_8037D028(Unk8037C7FC_F* m, int key)
{
    int r = 0;
    if (m->m8 != 0)
        r = fn_8037D72C(m->m8, key);
    if (r == -1)
        goto t1;
    if (r != 0)
        goto n1;
t1:
    if (m->mC != 0)
        r = fn_8037D72C(m->mC, key);
n1:
    if (r == -1)
        goto t2;
    if (r != 0)
        goto n2;
t2:
    if (m->m4 != 0)
        r = fn_8037D72C(m->m4, key);
n2:
    if (r == -1)
        goto t3;
    if (r != 0)
        goto done;
t3:
    if (m->m0 != 0)
        r = fn_8037D72C(m->m0, key);
done:
    return r;
}

extern "C" void fn_8037D0E8(Unk8037D0E8_P* out, int count)
{
    void* mem = operator new(0x18, 0);
    out->m0 = fn_803779B8(mem, count, 0, 0, lbl_80605830, 0x60);
}

extern "C" Unk8037D0E8* fn_8037D140(Unk8037D0E8* o, int a, int count)
{
    o->mC = lbl_806849B8;
    fn_8037D0E8((Unk8037D0E8_P*)o, a);
    o->m4 = fn_803774A8(count * 0x38, 0);
    o->m8 = fn_803774A8(count << 2, 0);
    return o;
}

extern "C" void fn_8037D1A0(Unk8037D0E8* o, int chrg)
{
    o->mC = lbl_806849B8;
    int n = o->m0->m4;
    for (int i = 0; i < n; i++) {
        VDel0* p = fn_803780A0(o->m0, i);
        if (p != 0)
            delete p;
    }
    if (o->m0 != 0)
        delete (SortArrDel*)o->m0;
    o->m0 = 0;
    if (o->m4 != 0)
        fn_80377488(o->m4);
    if (o->m8 != 0)
        fn_80377488(o->m8);
    o->m4 = 0;
    o->m8 = 0;
    if (chrg & 1)
        fn_80377408(o);
}

struct Ld2B0 {
    SortArr* m0;
    char* m4;
    int* m8;
};
struct Ld2B0View;
extern "C" {
int atoi(const char* s);
int fn_803791BC(const char* s, int n);
void* fn_8037BDA8(int hash, int a, void* c);
int fn_8037BEE4(int hash, const char* name);
void fn_8037C3F8(int hash, int a, int flag, void* obj);
void fn_80377FCC(SortArr* a, int v1, int v2, int key);
void fn_803780D0(SortArr* a, int v1, int v2, int key);
void fn_80378184(SortArr* a);
void fn_80378D70(const char* s, int a, char* dst, int max, int ch, int* len);
void fn_8037BFCC(int hash, int idx, const char* s);
}
extern int lbl_80624264;
extern const char* lbl_806EFC18;
struct DtorAt0 { const void* m; };

extern "C" void fn_8037BF88(DtorAt0* s, int chrg);

extern "C" void fn_8037D290(DtorAt0* s, int chrg)
{
    fn_8037BF88(s, chrg);
}

extern "C" void fn_8037D2B0(void* ov, void* datav, int size)
{
    Ld2B0* o = (Ld2B0*)ov;
    const char* data = (const char*)datav;
    if (data != 0 && size != 0) {
        char* tmp = (char*)fn_803774A8(0x7530, 0x100);
        const char* cur = data;
        int used = 0;
        while (data[size - 1] == '\n' || data[size - 1] == '\r')
            size--;
        int maxn = 0;
        int remain = size;
        char* tok;
        int tokLen;
        if ((unsigned)(cur - data) < (unsigned)size) {
            do {
                fn_80378E48((char**)&cur, &remain, ' ', &tok, &tokLen);
                int h = fn_803791BC(tok, 0);
                fn_80378E48((char**)&cur, &remain, ' ', &tok, &tokLen);
                int n = atoi(tok);
                if (n > maxn)
                    maxn = n;
                fn_80378E48((char**)&cur, &remain, ';', &tok, &tokLen);
                int m = atoi(tok);
                if (n > 0) {
                    int off = 0;
                    int i = n;
                    do {
                        fn_80378E48((char**)&cur, &remain, ' ', &tok, &tokLen);
                        int id = atoi(tok);
                        fn_80378D70(cur, size, o->m4 + id * 0x38, 0x32, ';', &used);
                        cur += used + 1;
                        *(int*)(off + (int)o->m8) = id;
                        off += 4;
                    } while (--i != 0);
                }
                lbl_80624264++;
                for (int j = 0; j < m; j++) {
                    int ok = 1;
                    fn_80378E48((char**)&cur, &remain, ' ', &tok, &tokLen);
                    int key = fn_803791BC(tok, 0);
                    void* ent = fn_8037BDA8(h, key, o);
                    if (ent == 0)
                        ok = 0;
                    if (ok != 0) {
                        if (j == 0 && j < n) {
                            int off = 0;
                            int i = n;
                            do {
                                int id = *(int*)(off + (int)o->m8);
                                off += 4;
                                int rec = id * 0x38;
                                *(int*)(rec + (int)o->m4 + 0x34) = fn_8037BEE4(h, o->m4 + rec);
                            } while (--i != 0);
                        }
                    }
                    if (ok != 0) {
                        if (fn_8037D680((Unk8037D0E8_P*)o, h, key) != 0)
                            fn_80377FCC(o->m0, (int)ent, h, key);
                        else
                            fn_803780D0(o->m0, (int)ent, h, key);
                    }
                    lbl_806EFC18 = cur;
                    fn_80378E48((char**)&cur, &remain, ';', &tok, &tokLen);
                    int cnt = atoi(tok);
                    if (cnt > 0) {
                        int c = cnt;
                        do {
                            fn_80378E48((char**)&cur, &remain, ' ', &tok, &tokLen);
                            int idx = atoi(tok);
                            fn_80378D70(cur, size, tmp, 0x7530, ';', &used);
                            cur += used + 1;
                            if (ok != 0)
                                fn_8037BFCC(h, *(int*)(o->m4 + idx * 0x38 + 0x34), tmp);
                        } while (--c != 0);
                    }
                    if (ok != 0)
                        fn_8037C3F8(h, key, 1, ent);
                }
            } while ((unsigned)(cur - data) < (unsigned)size);
        }
        fn_80378184(o->m0);
        fn_80377408(tmp);
    }
}

extern "C" int fn_8037D680(Unk8037D0E8_P* o, int x, int y)
{
    int sp8;
    int spC;
    if ((unsigned)fn_8037C5D8(x) <= 0xa4) {
        if (fn_80377E90(o->m0, &sp8, &spC, y))
            return sp8;
    }
    return 0;
}

extern "C" int fn_8037D6E4(Unk8037D0E8_P* o, const char* name, int y)
{
    return fn_8037D680(o, fn_80378568(name, 0), y);
}

struct Unk80361034_Pad18 {
    char pad[0x18];
};

struct Unk80361034_OwnV : Unk80361034_Pad18 {
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
};

struct Unk80361034_Own : Unk80361034_OwnV {
    char pad1C[0x74 - 0x1C];
    int m74;
    char pad78[0x80 - 0x78];
    int m80;
};

struct Unk80361034 {
    Unk80361034_Own* m0;
    const void* m4;
};

extern "C" int fn_80378024(Unk80361034_Own* o, int key);

extern "C" int fn_8037D72C(Unk8037D0E8_P* s, int key)
{
    return fn_80378024((Unk80361034_Own*)s->m0, key);
}

extern "C" void fn_8037D750() {}

extern "C" void fn_8037D754() {}

extern "C" void fn_8037D758() {}

extern "C" void fn_8037D75C() {}

extern "C" int fn_8037D760()
{
    return 0;
}

extern "C" void fn_8037D768(Unk8037D768* o)
{
    float t = 57.295776f;
    const V3F& p = o->m60;
    if (o->m6C != 0)
        fn_803E00EC(o->m20, o->m8C, o->m90, o->m94, o->m7C, o->m80, o->m84, o->m88, p.x, p.y, p.z);
    else {
        const V3F& d = o->m70;
        fn_803DF824(o->m20, o->m8C, o->m90, o->m94, d.x * t, d.y * t, d.z * t, p.x, p.y, p.z);
    }
}

extern "C" void fn_8037D824(Unk8037D768* o, V4S* a, V4S* b)
{
    if (o->m0 == 0)
        fn_8037D768(o);
    V4S ta = *a;
    V4S tb = *b;
    fn_803DEE7C(o->m20, &ta, &tb);
    *b = tb;
}

extern "C" void fn_8037D8DC(Unk8037D768* o, int a, int b)
{
    if (o->m0 == 0)
        fn_8037D768(o);
    fn_803DED40((char*)o + 0x20, a, b);
}

extern "C" void fn_8037D92C(Unk8037D92C* s, V3S** out)
{
    *(*out)++ = s->m60;
}

extern "C" void fn_8037D958(Unk8037D958* o, V3F** pa, V3F** pb, float t)
{
    float tt = t;
    if (*pa != 0 && *pb != 0) {
        V3F a = *(*pa)++;
        V3F b = *(*pb)++;
        o->s3(&((b - a) * tt + a));
    }
}

extern "C" void fn_8037DAD0(Unk8037DAD0* o, float** out)
{
    if (o->m6C != 0) {
        const V4F& q = o->m7C;
        *(*out)++ = q.x;
        *(*out)++ = q.y;
        *(*out)++ = q.z;
        *(*out)++ = q.w;
    } else {
        *(*(V3S**)out)++ = o->m70;
    }
}

extern "C" void fn_8037E050(int x)
{
    lbl_80624288 = x;
    fn_8037B8B4(x);
}

struct KnotBuf { const void* m0; void* m4; int m8; };

extern "C" void fn_80356798(void* p);

extern const char lbl_80685080[];

extern "C" KnotBuf* fn_8037E078(KnotBuf* p)
{
    fn_80356798(p);
    p->m4 = 0;
    p->m0 = lbl_80685080;
    p->m8 = 0;
    return p;
}

extern "C" void fn_8037E0C0(KnotBuf* p, int chrg)
{
    p->m0 = lbl_80685080;
    if (p->m8)
        fn_80377488((void*)p->m8);
    if (chrg & 1)
        fn_80377408(p);
}

extern "C" void fn_8037E118(Unk8037E118* o, char* data, int len)
{
    o->m4 = atoi(data);
    if (o->m8 != 0) {
        fn_80377488(o->m8);
        o->m8 = 0;
    }
    int n = o->m4;
    if (n != 0) {
        o->m8 = new (lbl_806EF9B4 | 0x100) Ent16F[n];
    }
    char* cursor = data;
    int rem = len;
    char* tok;
    int tokLen;
    int i = 0;
    fn_80378E48(&cursor, &rem, '#', &tok, &tokLen);
    if (i < o->m4) {
        int off = 0;
        do {
            i++;
            fn_80378E48(&cursor, &rem, ',', &tok, &tokLen);
            *(unsigned*)(off + (int)o->m8) = fn_80378568(tok, tokLen);
            fn_80378E48(&cursor, &rem, ',', &tok, &tokLen);
            ((Ent16F*)(off + (int)o->m8))->m4 = fn_80378568(tok, tokLen);
            fn_80378E48(&cursor, &rem, ',', &tok, &tokLen);
            ((Ent16F*)(off + (int)o->m8))->m8 = (float)atof(tok);
            fn_80378E48(&cursor, &rem, '#', &tok, &tokLen);
            ((Ent16F*)(off + (int)o->m8))->mC = (float)atof(tok);
            off += 0x10;
        } while (i < o->m4);
    }
    i = 0;
    if (o->m4 - 1 > 0) {
        do {
            int off = i * 0x10;
            Ent16F* e = (Ent16F*)(off + (int)o->m8);
            Ent16F* f = e + 1;
            if (!(e->m8 <= f->m8)) {
                Ent16F t;
                t = *f;
                *f = t;
                o->m8[i] = t;
                if (i > 0)
                    i--;
            } else {
                i++;
            }
        } while (i < o->m4 - 1);
    }
}

extern int lbl_8062427C;
inline void CpQ(Unk80379324* d, const Unk80379324& s)
{
    *d = s;
}
inline void CpV(E3B8_V3* d, const E3B8_V3& s)
{
    *d = s;
}
extern int lbl_8062427C;
extern "C" void fn_8037E3B8(E3B8_Arg0* o, int arg1, int* keys, int n, E3B8_Arg4* a4, E3B8_Arg5* a5)
{
    int k0 = lbl_8062427C;
    for (int i = 0; i < n; i++) {
        int key = keys[i];
        if (key == -1) {
            a5->s2(k0, 0, 0);
        } else if (key == -2) {
            a5->s2(k0, 0, 1);
        } else {
            E3B8_VB27* p = ((E3B8_VB27 * (*)(E3B8_Arg0*, int)) fn_8037E600)(o, key);
            if (p != 0) {
                E3B8_Obj31* q = p->s13();
                if (a4 != 0 && q != 0) {
                    E3B8_V3 sp8;
                    if (q->s15() != 0) {
                        Unk80379324 sp18;
                        a4->s2(q->s14(), &sp8, 0, &sp18);
                        CpQ(&q->m10, sp18);
                        q->m4 = sp8;
                    } else {
                        a4->s2(q->s14(), &sp8, 0, 0);
                        q->m4 = sp8;
                    }
                }
                int kk = keys[i] << 4;
                E3B8_Ent* e = (E3B8_Ent*)(kk + (int)o->m8);
                p->s12(arg1, a5, e->m8, e->mC);
            }
        }
    }
}

extern "C" void fn_8037E600(Unk8037E600* o, int idx)
{
    fn_8037C7FC()->fn_8037CE98(o->m8[idx].m0, o->m8[idx].m4, 0);
}
