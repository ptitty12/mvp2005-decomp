#ifndef GAME_TU_802D73B4_H
#define GAME_TU_802D73B4_H

/*
 * Types for game/tu_802D73B4.cpp (reconstructed, placeholder names). The
 * manager object at lbl_8061EDF8 is 0x2c bytes with its vptr at +0x28
 * (vtable lbl_806647E8; ctor fn_802D73E8 allocates it). It owns four pad
 * objects at +0x8 (vptr at +0x157C). Evidence per field is the load or store
 * in the function named beside it.
 */

void* operator new(unsigned int size, int heap); /* 0x80377428 */
void* operator new[](unsigned int size, int heap); /* 0x803774A8 */
extern "C" void fn_80377408(void* p);                       /* __builtin_delete */
extern "C" void fn_80377488(void* p);                       /* __builtin_vec_delete */
extern "C" double floor(double x);
float fn_80410CE8(float a, float b);             /* atan2 */
float __builtin_fabsf(float x);
double __builtin_fabs(double x);

/* Pad object: vptr at +0x157C; +0x10B4 tested in fn_802D7CE0, +0x10B8
 * written in fn_802D81A4 / fn_802D821C. Slot numbers are vtable byte
 * offset / 8. */
class Unk802D73B4_Pad {
public:
    char pad0[0x10B4];
    int m10B4;
    int m10B8;
    char pad10BC[0x157C - 0x10BC];
    virtual ~Unk802D73B4_Pad();
    virtual int s2();
    virtual void s3();
    virtual void s4();
    virtual int s5();
    virtual int s6();
    virtual int s7();
    virtual int s8();
    virtual int s9();
    virtual int s10();
    virtual int s11();
    virtual int s12();
    virtual int s13();
    virtual int s14();
    virtual int s15();
    virtual int s16();
    virtual int s17();
    virtual float s18();
    virtual int s19();
    virtual int s20();
    virtual int s21();
    virtual int s22();
    virtual void s23(int a, float b);
    virtual void s24(int a);
    virtual int s25(int a);
    virtual void s26();
    virtual void s27();
    virtual void s28();
    virtual void s29();
    virtual void s30();
    virtual void s31();
    virtual void s32();
    virtual void s33();
    virtual void s34();
    virtual void s35();
    virtual void s36();
    virtual void s37();
    virtual void s38();
    virtual void s39();
    virtual void s40();
    virtual int s41();
    virtual void s42(char* a);
    virtual void s43(char* a);
};

/* Event objects built on the stack: id at +0, vptr at +4 (base vtable 0x80649DA0). */
class Unk80649DA0 {
public:
    Unk80649DA0(int id) { m0 = id; }
    int m0;
    virtual ~Unk80649DA0() {}
};
/* vtable 0x806647B8 (dtor 0x8035F448) */
class Unk8035F448 : public Unk80649DA0 {
public:
    Unk8035F448(int id) : Unk80649DA0(id) {}
    virtual ~Unk8035F448() {}
};

/* Manager at lbl_8061EDF8 (vtable lbl_806647E8). */
class Unk802D73B4_Mgr {
public:
    Unk802D73B4_Mgr() {}
    unsigned m0;
    unsigned m4;
    Unk802D73B4_Pad* m8[4];
    char* m18[4];
    virtual ~Unk802D73B4_Mgr();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual int s5(int* a);
    virtual void s6();
    virtual void s7();
    virtual void s8(int a, int b);
    virtual void s9(int a, int b);
    virtual void s10();
    virtual void s11(int a, int b);
    virtual int s12(int a);
    virtual void s13();
    virtual int s14();
    void fn_802D83E4(int a);
    int fn_802D845C();
    Unk802D73B4_Pad* fn_802D8490(int a);
    void fn_802D8624(int a, int b);
    int fn_802D890C(int a, int b);
    void fn_802D8A84();
    void fn_802D8AD8(int a, int b);
};

inline Unk802D73B4_Pad* Unk802D73B4_PadAt(Unk802D73B4_Mgr* m, int i)
{
    return m->m8[i];
}

/* Owner of the manager (vtable lbl_806647D0: slot 1 is fn_802D7464). */
class Unk802D73B4_W {
public:
    Unk802D73B4_W();
    virtual ~Unk802D73B4_W();
};

/* lbl_8061B8C0 -> +0x8 / +0xC: the two player objects; +0x6F8 -> +0x10 is
 * the record read in fn_802D78D8. */
struct Unk802D73B4_Rec {
    char pad0[0x6C];
    int m6C;
    int m70;
    int m74;
};
struct Unk802D73B4_Ref {
    char pad0[0x10];
    Unk802D73B4_Rec* m10;
};
struct Unk802D73B4_Plr {
    char pad0[0x65C];
    int m65C;
    char pad660[0x6F8 - 0x660];
    Unk802D73B4_Ref* m6F8;
};
struct Unk802D73B4_G {
    int m0;
    int m4;
    Unk802D73B4_Plr* m8[2];
};

extern Unk802D73B4_Mgr* lbl_8061EDF8;
extern int lbl_8061EDFC;
extern int lbl_8061EE00;
extern float lbl_8061EE04;
extern int lbl_8061EE0C;
extern int lbl_8061EE10;
extern int lbl_8061EE14;
extern int lbl_8061EE18;
extern int lbl_8061EE1C;
extern int lbl_8061EE20;
extern int lbl_8061EE24[];
extern int lbl_8061F24C[];
extern int lbl_8061F678;
struct Unk802D73B4_H {
    int m0;
    Unk802D73B4_Ref* m4;
};
extern Unk802D73B4_H* lbl_806192E0;
extern Unk802D73B4_Pad* lbl_8068DF34;
extern Unk802D73B4_Plr* lbl_8068B618;
extern Unk802D73B4_G* lbl_8061B8C0;
extern int lbl_8061B9C8;

struct Unk80084910_W;
int fn_80086D34(Unk80084910_W* self, int i);
extern "C" void fn_802D6304(int a, int b);
extern "C" void* MEM_fill(void* dst, int val, unsigned int n);
struct Unk800085B0;
int fn_80008540(Unk800085B0** arr, int n);
extern "C" void fn_802D8B64(Unk802D73B4_Pad* p);
int fn_802DC5C0(Unk802D73B4_Pad* p);
extern "C" void fn_802DC61C(Unk802D73B4_Pad* p);
void fn_802DC8E4(Unk802D73B4_Pad* p);
void fn_802DC974(Unk802D73B4_Pad* p);
extern "C" void fn_802CBE04(int a, float* b, float* c, float* d);
extern "C" void fn_8037893C(void* q, int id, Unk80649DA0* ev, int a, int b);
extern int lbl_8061F674;
/* Called under their plain names from already-compiled units. */
extern "C" {
void fn_802D7588();
void fn_802D75D4(int a);
int fn_802D7A84(int* a);
void fn_802D7ACC();
void fn_802D7ADC();
int fn_802D7CE0(int a, int b);
int fn_802D7DB4(int a, int b);
int fn_802D7EE4(int a);
}
int fn_802D7AC8(int a);
int fn_802D8150();
Unk802D73B4_Pad* fn_802D8178(int a);
int fn_802D7890(int a);
int fn_802D78D8(int a);
int fn_802D7C70();
void fn_800085AC();
extern "C" int fn_80400DA4();
extern "C" int fn_80400CE4(int a);
float fn_802DC1D8(Unk802D73B4_Pad* p, short b);
float fn_802DC3E8(Unk802D73B4_Pad* p, short b);
float fn_802D81A4(int a, int b);
float fn_802D821C(int a, int b);
int fn_802D80EC(int a);

#endif
