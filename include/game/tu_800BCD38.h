#ifndef GAME_TU_800BCD38_H
#define GAME_TU_800BCD38_H

/* Types this range shares with other units live in their original headers. */
#include "game/tu_8001B190.h"  /* Unk8001BC8C */
#include "game/tu_8007A4DC.h"  /* Unk8007B2B0_Screen */

/*
Types for the front-end code in game/tu_800BCD38.cpp .. game/tu_800D1758.cpp
(0x800BCD38..0x800D4D0C; one shared header for the range). Everything is named
by address (Unk<addr>) or offset (m<off>); nothing here comes from original headers.
*/

/* Big front-end screen object (initialised by fn_800C1D6C): small value groups at
 * +0, +0x14, +0x20, +0x7C/+0x84, a 44-byte struct at +0x8C, 25 words at +0x59C. */
struct Unk800C1D6C_Block44 {
    int m0, m4, m8, mC, m10, m14, m18, m1C, m20, m24, m28;
};

struct Unk800C1D6C_Obj {
    int m0;
    int m4;
    int m8;
    char mC;
    char mD;
    short mE;
    int m10;
    int m14;
    int m18;
    int m1C;
    int m20;
    int m24;
    char pad28[0x3C];
    int m64;
    int m68;
    char pad6C[0x8];
    int m74;
    char pad78[0x4];
    int m7C;
    int m80;
    int m84;
    int m88;
    Unk800C1D6C_Block44 m8C;
    char padB8[0x4E0];
    int m598;
    int m59C[25];
    int m600;
    char pad604[0x3C];
    int m640;
    int m644;
};

/* Entry of the list at Unk800C16CC_List::m34 (name string at +0xC). */
struct Unk800C16CC_Entry {
    char pad0[0xC];
    char mC[1];
};

struct Unk800C16CC_List {
    char pad0[0x34];
    Unk800C16CC_Entry* m34[100];
    int m1C4;
};
struct Unk800C16CC_Holder {
    char pad0[0x10];
    Unk800C16CC_List* m10;
};
struct Unk800C1754_Pool {
    int m0;
    char pad4[0x14];
    void* m18;
};


/* On-demand file slot, size 0x46C (array at manager+0x9C). m40C holds the queued objects
 * (front-end widgets) with their priorities in m430. The vptr is at +0x468. */
struct Unk800C1BE4_Widget;
struct Unk800C2250_FileData {
    char m0;
    char pad1[0x3FF];
    int m400;
    int m404;
    int m408;
    Unk800C1BE4_Widget* m40C[9];
    float m430[9];
    int m454;
    int m458;
    void* m45C;
    int m460;
    int m464;
};

struct Unk800C2250_File : Unk800C2250_FileData {
    Unk800C2250_File();
    virtual ~Unk800C2250_File();
};

extern "C" {
int fn_800C2EF0(Unk800C2250_File* f);
int fn_800C2F08(Unk800C2250_File* f);
int fn_800C2F1C(Unk800C2250_File* f);
int fn_800C2F30(Unk800C2250_File* f);
int fn_800C2F44(Unk800C2250_File* f);
int fn_800C2F5C(Unk800C2250_File* f);
int fn_800C2F74(Unk800C2250_File* f);
int fn_800C2F7C(Unk800C2250_File* f);
char* fn_800C2F84(Unk800C2250_File* f);
int fn_800C2F88(Unk800C2250_File* f);
int fn_800C2F90(Unk800C2250_File* f, int i);
Unk800C1BE4_Widget* fn_800C2FB0(Unk800C2250_File* f, int i);
Unk800C1BE4_Widget* fn_800C2FC0(Unk800C2250_File* f, int i);
float fn_800C2FD0(Unk800C2250_File* f, int i);
void fn_800C2FF0(Unk800C2250_File* f);
void fn_800C2B94(Unk800C2250_File* f, int i);
void fn_800C2978(Unk800C2250_File* f);
void fn_800C31FC(Unk800C2250_File* f);
void fn_800C2848(Unk800C2250_File* f);
void fn_800C28C8(Unk800C2250_File* f);
}


/* On-demand manager singleton (fn_800C33D0), size 0x5DAC, vptr at +0x5DA8. The ten file
 * slots start at +0x9C; twelve 0x410-byte request records start at +0x2CD4 (see
 * Unk800C3A10_Rec). */
struct Unk800C3A10_Rec {
    int m0;
    char m4;
    char pad5[0x3FF];
    const char* m4_name() { return &m4; }
    int m404;
    float m408;
    int m40C;
};

struct Unk800C341C_Mgr {
    int m0;
    int m4;
    void* m8;
    char padC[0x10];
    void* m1C;
    char pad20[0x78];
    char m98;
    char pad99[3];
    Unk800C2250_File files[10];
    Unk800C3A10_Rec recs[12];
    int m5D94;
    int m5D98;
    int m5D9C;
    int m5DA0;
    int m5DA4;
};

extern "C" {
void fn_800C537C(Unk800C341C_Mgr* m, int kind, const char* name, int idx, float scale, int flag);
Unk800C341C_Mgr* fn_800C33D0();
int fn_800C5CF8(Unk800C341C_Mgr* m, const char* name);
void fn_800C3D50(Unk800C341C_Mgr* m);
int fn_800C2998(Unk800C2250_File* f, Unk800C1BE4_Widget* w);
void fn_800C2458(Unk800C2250_File* f);
void fn_800C5010(Unk800C341C_Mgr* m);
void fn_800C5080(Unk800C341C_Mgr* m);
void fn_800C3A4C(Unk800C341C_Mgr* m);
void fn_800C3A10(Unk800C341C_Mgr* m);
void fn_800C5AF8(Unk800C341C_Mgr* m);
void fn_800C5B58(Unk800C341C_Mgr* m);
void fn_800C3828(Unk800C341C_Mgr* m);
int fn_800C58E8(Unk800C341C_Mgr* m, int kind);
void fn_800C435C(Unk800C341C_Mgr* m, int idx, float scale, const char* name, int flag);
void fn_800C4D90(Unk800C341C_Mgr* m, int idx, float scale, const char* name, int flag);
void fn_800C56C0(Unk800C341C_Mgr* m, int a);
int fn_803FEDB8(void* p, int a);
void fn_803FB4B0();
int fn_800C5C68(Unk800C341C_Mgr* m, Unk800C1BE4_Widget* w);
int fn_800C587C(Unk800C341C_Mgr* m);
int fn_800C5DAC(Unk800C341C_Mgr* m, int kind);
void fn_800C24B0(Unk800C2250_File* f);
void fn_800C2518(Unk800C2250_File* f);
void fn_800C2948(Unk800C2250_File* f);
int fn_800C510C(Unk800C341C_Mgr* m, const char* name);
int fn_800C512C(Unk800C341C_Mgr* m, int idx);
void fn_800C2C78(Unk800C2250_File* f);
void fn_800C334C(Unk800C2250_File* f);
void fn_800C3B14(Unk800C341C_Mgr* m);
}


/* Data part of the front-end widget base (ctor fn_800C7B40, size 0x40 with the vptr at +0x3C).
 * +0x24 is a word of flag bits (b0 = MSB) whose low half overlaps m26. */
struct Unk800C7B40_Widget;
struct Unk800C7B40_Data {
    int m0;
    Unk800C7B40_Widget* m4;
    Unk800C7B40_Widget* m8;
    char mC[16];
    int m1C;
    int m20;
    unsigned b0 : 1;
    unsigned b1 : 1;
    unsigned b2 : 1;
    unsigned b3 : 1;
    unsigned b4 : 1;
    unsigned b5 : 1;
    unsigned b6 : 1;
    unsigned b7 : 1;
    unsigned b8 : 1;
    unsigned b9 : 1;
    unsigned b10 : 1;
    unsigned b11 : 1;
    unsigned b12 : 1;
    unsigned b13 : 1;
    unsigned b14 : 1;
    unsigned b15 : 1;
    short m26;
    short m28;
    short m2A;
    short m2C;
    short m2E;
    short m30;
    short m32;
    short m34;
    short m36;
    float m38;
};

/* Front-end widget base: the virtual part of the class whose data starts with
 * Unk800C7B40_Data. g++ 2.95 puts the vptr (+0x3C) after the data of the class that
 * introduces virtuals, so data that follows the vptr belongs to a derived class.
 * fn_vslotN = vtable slot N (virtual N is at +8N of the vtable). The vtables are about
 * 0x3F8 bytes for the largest classes, so slots up to 127 are declared. */
struct Unk800C7B40_Widget : Unk800C7B40_Data {
    virtual ~Unk800C7B40_Widget();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5(int a, int b, int c, int d);
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8(int a);
    virtual void fn_vslot9(int a);
    virtual void fn_vslot10(int a);
    virtual void fn_vslot11();
    virtual void fn_vslot12(int a);
    virtual void fn_vslot13(int a);
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15(int a);
    virtual void fn_vslot16(int a);
    virtual void fn_vslot17(int a);
    virtual void fn_vslot18(int a);
    virtual float fn_vslot19();
    virtual void fn_vslot20(float a);
    virtual void fn_vslot21(float a);
    virtual void fn_vslot22();
    virtual void fn_vslot23();
    virtual void fn_vslot24(int a);
    virtual void fn_vslot25();
    virtual void fn_vslot26();
    virtual void fn_vslot27();
    virtual Unk800C7B40_Widget* fn_vslot28(const char* name);
    virtual void fn_vslot29();
    virtual Unk800C7B40_Widget* fn_vslot30(const char* name, int a);
    virtual Unk800C7B40_Widget* fn_vslot31(const char* name, int a, int b);
    virtual Unk800C7B40_Widget* fn_vslot32(const char* name, int a);
    virtual char* fn_vslot33(int a);
    virtual void fn_vslot34();
    virtual void fn_vslot35();
    virtual void fn_vslot36();
    virtual void fn_vslot37();
    virtual void fn_vslot38(int a);
    virtual void fn_vslot39();
    virtual void fn_vslot40(int a);
    virtual void fn_vslot41();
    virtual void fn_vslot42();
    virtual void fn_vslot43();
    virtual void fn_vslot44();
    virtual void fn_vslot45();
    virtual void fn_vslot46();
    virtual void fn_vslot47();
    virtual void fn_vslot48();
    virtual void fn_vslot49();
    virtual void fn_vslot50();
    virtual void fn_vslot51();
    virtual void fn_vslot52();
    virtual void fn_vslot53();
    virtual void fn_vslot54();
    virtual void fn_vslot55();
    virtual void fn_vslot56();
    virtual void fn_vslot57();
    virtual void fn_vslot58();
    virtual void fn_vslot59();
    virtual int fn_vslot60();
    virtual int fn_vslot61();
    virtual void fn_vslot62();
    virtual void fn_vslot63();
    virtual void fn_vslot64();
    virtual void fn_vslot65();
    virtual void fn_vslot66();
    virtual void fn_vslot67();
    virtual void fn_vslot68();
    virtual void fn_vslot69();
    virtual void fn_vslot70();
    virtual void fn_vslot71();
    virtual void fn_vslot72();
    virtual Unk800C7B40_Widget* fn_vslot73(int a, int b);
    virtual void fn_vslot74();
    virtual void fn_vslot75();
    virtual void fn_vslot76();
    virtual void fn_vslot77();
    virtual void fn_vslot78();
    virtual void fn_vslot79();
    virtual void fn_vslot80();
    virtual void fn_vslot81();
    virtual void fn_vslot82();
    virtual void fn_vslot83();
    virtual void fn_vslot84();
    virtual const char* fn_vslot85();
    virtual void fn_vslot86();
    virtual void fn_vslot87();
    virtual void fn_vslot88();
    virtual void fn_vslot89();
    virtual void fn_vslot90();
    virtual void fn_vslot91();
    virtual void fn_vslot92();
    virtual void fn_vslot93();
    virtual void fn_vslot94();
    virtual void fn_vslot95();
    virtual void fn_vslot96();
    virtual void fn_vslot97();
    virtual void fn_vslot98();
    virtual void fn_vslot99();
    virtual void fn_vslot100();
    virtual void fn_vslot101();
    virtual void fn_vslot102();
    virtual void fn_vslot103();
    virtual void fn_vslot104();
    virtual void fn_vslot105();
    virtual void fn_vslot106();
    virtual void fn_vslot107(int a);
    virtual void fn_vslot108();
    virtual void fn_vslot109();
    virtual void fn_vslot110();
    virtual void fn_vslot111();
    virtual void fn_vslot112();
    virtual void fn_vslot113();
    virtual void fn_vslot114();
    virtual void fn_vslot115();
    virtual void fn_vslot116();
    virtual void fn_vslot117();
    virtual void fn_vslot118(int a);
    virtual void fn_vslot119(int a);
    virtual void fn_vslot120();
    virtual void fn_vslot121();
    virtual void fn_vslot122();
    virtual void fn_vslot123();
    virtual void fn_vslot124();
    virtual void fn_vslot125(int a, int b, int c, int d, int e);
    virtual void fn_vslot126();
    virtual void fn_vslot127();
};

/* Container widget: five child pointers at +0x40..+0x50. */
struct Unk800C7A4C_Group : Unk800C7B40_Widget {
    Unk800C7B40_Widget* m40;
    Unk800C7B40_Widget* m44;
    Unk800C7B40_Widget* m48;
    Unk800C7B40_Widget* m4C;
    Unk800C7B40_Widget* m50;
    void fn_800C7A4C(int v);
    void fn_800C7584(Unk800C7B40_Widget* c);
    void fn_800C75F0(Unk800C7B40_Widget* c);
    void fn_800C765C(Unk800C7B40_Widget* c);
    void fn_800C76C8(Unk800C7B40_Widget* c);
    void fn_800C7734(Unk800C7B40_Widget* c);
    void fn_800CF114(int v);
    void fn_800CF178(int v);
    void fn_800CF1DC(int v);
    void fn_800CF240(int v);
};

void* operator new(unsigned int size, int heap); /* 0x80377428 */

/* Singleton at lbl_8061ABE0 (fn_800C5E28, size 0x44, vptr at +0x40). */
struct Unk800C5E74_Data {
    int m0;
    char m4;
    char pad5[0xB];
    int m10;
    int m14;
    char m18;
    char pad19[0xB];
    int m24;
    int m28;
    int m2C;
    int m30;
    float m34;
    int m38;
    int m3C;
};

struct Unk800C5E74_Obj : Unk800C5E74_Data {
    Unk800C5E74_Obj();
    virtual ~Unk800C5E74_Obj();
    void fn_800C5EB0();
    void fn_800C6604();
    void fn_800C6624();
    void fn_800C650C(Unk800C1BE4_Widget* a, const char* n1, Unk800C1BE4_Widget* b, const char* n2, float f, int flag);
};

/* Name/handle list at +0x34 (count at +0x1C4) used by fn_800C16CC and fn_800C68D0. */
struct Unk800C68B4_List {
    char pad0[0x34];
    Unk800C7B40_Widget* m34[100];
    int m1C4;
    void fn_800C68B4(Unk800C7B40_Widget* e);
    Unk800C7B40_Widget* fn_800C68D0(const char* name);
    void fn_800C694C(const char* name);
    void fn_800C696C();
    void fn_800C6A94(void (Unk800C7B40_Widget::*f)());
    void fn_800C6B5C(void (Unk800C7B40_Widget::*f)());
    void fn_800C6C24(int a, int (Unk800C7B40_Widget::*f)(int));
};

extern "C" {
void fn_800C7E0C(Unk800C7B40_Data* w, int v);
void fn_800C7E38(Unk800C7B40_Data* w, int v);
void fn_800C7E64(Unk800C7B40_Data* w, int v);
void fn_800C7E90(Unk800C7B40_Data* w, int v);
void fn_800C7EBC(Unk800C7B40_Data* w, int v);
}

/* Widget with a slot-73 callback pair at +0x68/+0x6C (fn_800CD5A4). */
struct Unk800CD5A4_Obj : Unk800C7B40_Widget {
    char pad40[0x28];
    int m68;
    int m6C;
    void fn_800CD5A4();
    void fn_800CD5E4();
    void fn_800CDA14(int a, int b, int c);
    void fn_800CDA54(int a, int b, int c);
};

/* Widget with six child pointers at +0x40..+0x54 (fn_800D1614 family). */
struct Unk800D1614_Obj : Unk800C7B40_Widget {
    int m40;
    Unk800C7B40_Widget* m44;
    Unk800C7B40_Widget* m48;
    Unk800C7B40_Widget* m4C;
    Unk800C7B40_Widget* m50;
    Unk800C7B40_Widget* m54;
    void fn_800D1614(Unk800C7B40_Widget* c);
    void fn_800D1654(Unk800C7B40_Widget* c);
    void fn_800D1694(Unk800C7B40_Widget* c);
    void fn_800D16D4(Unk800C7B40_Widget* c);
    void fn_800D1714(int v);
};

struct Unk800D244C_Vec4 {
    int m0, m4, m8, mC;
};

/* Widget with a value at +0x5C that its flag setters refresh (fn_800D238C family) and a
 * 16-byte value at +0x6C (fn_800D244C). */
struct Unk800D238C_Obj : Unk800C7B40_Widget {
    char pad40[0x1C];
    int m5C;
    char pad60[0xC];
    Unk800D244C_Vec4 m6C;
    void fn_800D238C(int v);
    void fn_800D23BC(int v);
    void fn_800D23EC(int v);
    void fn_800D241C(int v);
    void fn_800D244C(const Unk800D244C_Vec4& v);
    void fn_800D2474(int v);
};

/* Object with an indexed table of ints at +0x17C. */
struct Unk800D2CD8_Obj {
    char pad0[0x17C];
    int m17C[1];
};

/* Object with its own flag word at +0x70 and a value at +0x84. */
struct Unk800D4B88_Obj {
    char pad0[0x70];
    unsigned b0 : 1;
    unsigned rest : 31;
    char pad74[0x10];
    int m84;
};

extern int lbl_8061A330;

/* ---- pass-2 additions (range 04) ---- */
extern "C" {
Unk800C341C_Mgr* fn_800C341C(Unk800C341C_Mgr* m);
void fn_800C8D48(Unk800D1614_Obj* o, int v, Unk800C7B40_Widget* c);
void fn_800C8E98(Unk800D1614_Obj* o, int v, Unk800C7B40_Widget* c);
void fn_800CF804(Unk800C7B40_Widget* w, int v);
void fn_80356838(void* o, int c);
void fn_800C97A4(void* o, int c);
void fn_800C7C04(Unk800C7B40_Widget* w);
int fn_800C80D4(void* w);
int fn_800C80DC(Unk800C7B40_Widget* w, int v);
int fn_8040B574(const char* a, const char* b);             /* behaves like strcmp */
void fn_80378D40(const char* s, int ch, int* out);
void fn_80377408(void* p); /* __builtin_delete */
}

/* Index cursor with a wrap flag (b0 of the word at +8); vptr at +0x24 (vtable lbl_8065A370:
 * slot 3 = fn_800CFF7C next, slot 5 = fn_800CFFB4 previous). Ctor fn_800CFEEC. */
struct Unk800CFEEC_Data {
    int m0;
    int m4;
    int b0 : 1;
    int b1 : 1;
    int b2 : 1;
    int b3 : 1;
    int rest : 28;
    int mC;
    int m10[4];
    int m20;
};
struct Unk800CFEEC_Raw;
struct Unk800CFEEC_Raw : Unk800CFEEC_Data {
    const void* m24;
    Unk800CFEEC_Raw* fn_800CFEEC(int flag);
};
struct Unk800CFEEC_Sel : Unk800CFEEC_Data {
    virtual int fn_vslot1();
    virtual int fn_vslot2();
    virtual void fn_vslot3(int i);
    virtual void fn_vslot4();
    virtual int fn_vslot5();
    virtual void fn_vslot6(int i);
    virtual int fn_vslot7();
    virtual void fn_vslot8(Unk800C7B40_Widget* p);
    virtual void fn_vslot9();
    virtual void fn_vslot10();
    virtual void fn_vslot11();
    virtual void fn_vslot12();
    virtual int fn_vslot13();
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15();
};

extern "C" {
int fn_800B5520(void* p);
void fn_800D0074(Unk800CFEEC_Sel* s, int id, int q);
void* fn_802B0850(void* obj, const char* name, void* out);
Unk800D244C_Vec4 fn_802B0ACC(void* obj, const char* name);
}
int fn_800E1990(int id);
extern "C" double floor(double x);

/* Two-button list selector: count m40, selected index m44, step widgets m48/m4C (fn_800D0F18 family). */
struct Unk800D0F18_Obj : Unk800C7B40_Widget {
    int m40;
    int m44;
    Unk800C7B40_Widget* m48;
    Unk800C7B40_Widget* m4C;
};

/* Widget holding a looked-up id at +0x70 and a rect at +0x74 (fn_800D03B4). */
struct Unk800D03B4_Obj : Unk800C7B40_Widget {
    char pad40[0x30];
    int m70;
    Unk800D244C_Vec4 m74;
};

/* Widget with two colours at +0x44/+0x54 (fn_800D0C84). */
struct Unk800D0C84_Obj : Unk800C7B40_Widget {
    char pad40[0x4];
    Unk800D244C_Vec4 m44;
    Unk800D244C_Vec4 m54;
};

/* Object with a vptr at +0x24: slot 3 takes an index, slot 13 returns a count (fn_800CFFF0). */
struct Unk800CFFF0_Data {
    char pad0[0x24];
};
struct Unk800CFFF0_Obj : Unk800CFFF0_Data {
    virtual void fn_vslot1();
    virtual void fn_vslot2();
    virtual void fn_vslot3(int i);
    virtual void fn_vslot4();
    virtual void fn_vslot5();
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8();
    virtual void fn_vslot9();
    virtual void fn_vslot10();
    virtual void fn_vslot11();
    virtual void fn_vslot12();
    virtual int fn_vslot13();
};

struct Unk800D0D3C_Obj {
    float m0;
    float m4;
    int m8;
    Unk800D0D3C_Obj();
};

extern "C" {
void fn_800CFF48(Unk800CFEEC_Data* c, int i);
int fn_800CFF7C(Unk800CFEEC_Data* c);
int fn_800CFFB4(Unk800CFEEC_Data* c);
}

/* Raw view of Unk800D1614_Obj for constructors/destructors that store the vptr by hand. */
struct Unk800D1614_Raw : Unk800C7B40_Data {
    const void* m3C;
    int m40;
    Unk800C7B40_Widget* m44;
    Unk800C7B40_Widget* m48;
    Unk800C7B40_Widget* m4C;
    Unk800C7B40_Widget* m50;
    Unk800C7B40_Widget* m54;
};
extern const char lbl_80657A78[];
extern const char lbl_80657C18[];
extern const char lbl_8065CCE8[];
extern const char lbl_8065CA08[];
extern const char lbl_806574E0[];
extern Unk800C7B40_Data* lbl_8061ABE4;
extern "C" {
void fn_800C8010(Unk800C7B40_Data* w, int a, int b, int c, int d);
Unk800D1614_Raw* fn_800D0D9C(Unk800D1614_Raw* self, int a, int b, int c, int d, int e, int f, int g, int h, int i, int j,
                             int k, int l);
void* fn_800C7B40(Unk800D1614_Raw* self, int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, float x);
void fn_800C7EE8(Unk800C7B40_Widget* w);
}

/* Selector-list widget: name buffer at +0x4A, flag word at +0x68, a Sel sub-object at +0x98 (its
 * vptr at +0xBC), five-byte names from +0xC0, id table at +0x17C, current value at +0x210. */
struct Unk800D2338_Obj : Unk800C7B40_Widget {
    char m40[5];
    char m45[5];
    char m4A[5];
    char pad4F[1];
    int m50;
    int m54;
    int m58;
    int m5C;
    int m60;
    int m64;
    int b68_0 : 1;
    int b68_1 : 1;
    int b68_2 : 1;
    int b68_3 : 1;
    int b68_4 : 1;
    int b68_5 : 1;
    int rest68 : 26;
    char pad6C[0x10];
    int m7C;
    Unk800D244C_Vec4 m80;
    float m90;
    char pad94[4];
    char pad98[0x28];
    char mC0[0xBC];
    int m17C[0x20];
    char pad1FC[0x14];
    int m210;
};
extern "C" {
void fn_800D24A4(Unk800D2338_Obj* o);
void fn_800C8118(void* o, void* a, void* b, float t);
void fn_800D2614(Unk800D2338_Obj* o, float f);
void fn_800D1D5C(Unk800D2338_Obj* o);
void fn_800D2338(Unk800D2338_Obj* o, const char* name, int flag);
int fn_800C8070(Unk800C7B40_Data* w);
void fn_802D7EE4(int v);
int fn_800B548C(int id);
int fn_8040B6A0(const char* s); /* behaves like strlen */
char* fn_8040B61C(char* dst, const char* src); /* behaves like strcpy */
int fn_80408A94(const char* s, const char* fmt, ...); /* behaves like sscanf */
}
int fn_800E1AC8(int id);
extern "C" char* fn_80090880(void* t, int id);
extern char lbl_8061AC58;
extern const char lbl_8065BC68[];
struct Unk800D3A38_Raw : Unk800C7B40_Data {
    const void* m3C;
    int* m40;
};

Unk8001BC8C* fn_8001BC4C();
extern "C" {
void* fn_800C8034(void* p);
void* fn_800C8038(void* p);
void* fn_800C8040(void* p);
void* fn_800C8044(void* p);
void* fn_800C8048(void* p);
void* fn_800C804C(void* p);
void* fn_800C8054(void* p);
void* fn_800C805C(void* p);
void fn_800C80C0(int* p, int v);
void* fn_800C8058(void* p);
}

/* Grid/table widget (fn_800CB3A8 family). The virtual part has the same slot layout as the widget
 * base; slots 19, 42-44, 58, 74, 90-92 and 112 have grid-specific signatures. */
struct Unk800CD4B8_Sub;
struct Unk800C9728_Sub;
struct Unk800CB3A8_GridV : Unk800C7B40_Data {
    virtual ~Unk800CB3A8_GridV();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5(int a, int b, int c, int d);
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8(int a);
    virtual void fn_vslot9(int a);
    virtual void fn_vslot10(int a);
    virtual void fn_vslot11();
    virtual void fn_vslot12(int a);
    virtual void fn_vslot13();
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15(int a);
    virtual void fn_vslot16(int a);
    virtual void fn_vslot17(int a);
    virtual void fn_vslot18(int a);
    virtual float fn_vslot19();
    virtual void fn_vslot20(float a);
    virtual void fn_vslot21(float a);
    virtual void fn_vslot22();
    virtual void fn_vslot23();
    virtual void fn_vslot24(int a);
    virtual void fn_vslot25();
    virtual void fn_vslot26();
    virtual void fn_vslot27();
    virtual Unk800C7B40_Widget* fn_vslot28(const char* name);
    virtual void fn_vslot29();
    virtual Unk800C7B40_Widget* fn_vslot30(const char* name, int a);
    virtual Unk800C7B40_Widget* fn_vslot31(const char* name, int a, int b);
    virtual Unk800C7B40_Widget* fn_vslot32(const char* name, int a);
    virtual char* fn_vslot33(int a);
    virtual void fn_vslot34();
    virtual void fn_vslot35();
    virtual void fn_vslot36();
    virtual void fn_vslot37();
    virtual void fn_vslot38(int a);
    virtual void fn_vslot39();
    virtual void fn_vslot40(int a);
    virtual void fn_vslot41();
    virtual void fn_vslot42(int a, int b, int c, int d);
    virtual void fn_vslot43(int a, int b, int c, int d);
    virtual void fn_vslot44(int a, int b, int c, int d);
    virtual void fn_vslot45();
    virtual void fn_vslot46();
    virtual void fn_vslot47();
    virtual void fn_vslot48();
    virtual void fn_vslot49();
    virtual void fn_vslot50();
    virtual void fn_vslot51();
    virtual void fn_vslot52();
    virtual void fn_vslot53();
    virtual void fn_vslot54();
    virtual void fn_vslot55();
    virtual void fn_vslot56();
    virtual void fn_vslot57();
    virtual int fn_vslot58();
    virtual void fn_vslot59();
    virtual void fn_vslot60();
    virtual void fn_vslot61();
    virtual int fn_vslot62();
    virtual int fn_vslot63();
    virtual void fn_vslot64();
    virtual void fn_vslot65();
    virtual void fn_vslot66();
    virtual void fn_vslot67();
    virtual void fn_vslot68();
    virtual void fn_vslot69();
    virtual void fn_vslot70();
    virtual void fn_vslot71();
    virtual void fn_vslot72();
    virtual Unk800C7B40_Widget* fn_vslot73(int a, int b);
    virtual Unk800C7B40_Widget* fn_vslot74(int a, int b);
    virtual void fn_vslot75();
    virtual void fn_vslot76();
    virtual void fn_vslot77();
    virtual void fn_vslot78();
    virtual void fn_vslot79();
    virtual void fn_vslot80();
    virtual void fn_vslot81();
    virtual void fn_vslot82();
    virtual void fn_vslot83();
    virtual void fn_vslot84();
    virtual void fn_vslot85();
    virtual void fn_vslot86();
    virtual void fn_vslot87();
    virtual void fn_vslot88();
    virtual void fn_vslot89();
    virtual void fn_vslot90(int a, int b, int c);
    virtual void fn_vslot91(int a, int b, int c);
    virtual void fn_vslot92(int a, int b, int c);
    virtual void fn_vslot93();
    virtual int fn_vslot94(int a, int b);
    virtual void fn_vslot95();
    virtual void fn_vslot96();
    virtual void fn_vslot97();
    virtual void fn_vslot98();
    virtual void fn_vslot99();
    virtual void fn_vslot100();
    virtual void fn_vslot101();
    virtual void fn_vslot102();
    virtual void fn_vslot103();
    virtual void fn_vslot104();
    virtual void fn_vslot105();
    virtual void fn_vslot106();
    virtual void fn_vslot107(int a);
    virtual void fn_vslot108();
    virtual void fn_vslot109();
    virtual void fn_vslot110();
    virtual void fn_vslot111();
    virtual int fn_vslot112(int a, int b, int* c, int* d);
    virtual void fn_vslot113();
    virtual void fn_vslot114();
    virtual void fn_vslot115(int a, int b, int c, int d);
    virtual void fn_vslot116();
    virtual void fn_vslot117();
    virtual void fn_vslot118(int a);
    virtual void fn_vslot119();
    virtual void fn_vslot120();
    virtual void fn_vslot121();
    virtual void fn_vslot122();
    virtual int fn_vslot123(int a, int b);
    virtual Unk800C7B40_Widget* fn_vslot124(int a);
    virtual void fn_vslot125(int a, int b, int c, int d, int e);
    virtual void fn_vslot126();
    virtual void fn_vslot127();
};

/* Grid fields (evidence: fn_800CB578 row*m54+col index into m98/m9C, fn_800CD354 bounds m48/m4C,
 * m60/m64 scroll window with m70/m74 wrap). m80 holds flag bits (b2 = per-cell storage). */
struct Unk800CB3A8_Grid : Unk800CB3A8_GridV {
    Unk800C7B40_Widget* m40;
    int m44;
    int m48;
    int m4C;
    int m50;
    int m54;
    int m58;
    int m5C;
    int m60;
    int m64;
    int m68;
    int m6C;
    int m70;
    int m74;
    Unk800C9728_Sub* m78;
    Unk800C9728_Sub* m7C;
    unsigned b80_0 : 1;
    unsigned b80_1 : 1;
    unsigned b80_2 : 1;
    unsigned b80_3 : 1;
    unsigned b80_4 : 1;
    unsigned b80_5 : 1;
    unsigned b80_6 : 1;
    unsigned b80_7 : 1;
    unsigned b80_8 : 1;
    unsigned rest80 : 23;
    char pad84[0xC];
    char* m90;
    int* m94;
    unsigned* m98;
    float* m9C;
    char padA0[0x50];
    float mF0;
    Unk800CD4B8_Sub* mF4;
};

/* Widget with a helper widget at +0x78 (fn_800CA8C8 / fn_800CABFC). */
struct Unk800CA8C8_Obj : Unk800C7B40_Widget {
    char pad40[0x38];
    Unk800C7B40_Widget* m78;
};

/* Widget-layout virtuals with class-specific slot signatures. */
struct Unk800C9728_Sub : Unk800C7B40_Data {
    virtual ~Unk800C9728_Sub();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5(int a, int b, int c, int d);
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8(int a);
    virtual void fn_vslot9(int a);
    virtual void fn_vslot10(int a);
    virtual void fn_vslot11();
    virtual void fn_vslot12(int a);
    virtual void fn_vslot13(int a);
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15(int a);
    virtual void fn_vslot16(int a);
    virtual void fn_vslot17(int a);
    virtual void fn_vslot18(int a);
    virtual float fn_vslot19();
    virtual void fn_vslot20(float a);
    virtual void fn_vslot21(float a);
    virtual void fn_vslot22();
    virtual void fn_vslot23();
    virtual void fn_vslot24(int a);
    virtual void fn_vslot25();
    virtual void fn_vslot26();
    virtual void fn_vslot27();
    virtual Unk800C7B40_Widget* fn_vslot28(const char* name);
    virtual void fn_vslot29();
    virtual Unk800C7B40_Widget* fn_vslot30(const char* name, int a);
    virtual Unk800C7B40_Widget* fn_vslot31(const char* name, int a, int b);
    virtual Unk800C7B40_Widget* fn_vslot32(const char* name, int a);
    virtual char* fn_vslot33(int a);
    virtual void fn_vslot34();
    virtual void fn_vslot35(int a);
    virtual void fn_vslot36();
    virtual void fn_vslot37();
    virtual void fn_vslot38(int a);
    virtual void fn_vslot39();
    virtual void fn_vslot40(int a);
    virtual void fn_vslot41();
    virtual void fn_vslot42();
    virtual void fn_vslot43();
    virtual void fn_vslot44();
    virtual void fn_vslot45();
    virtual void fn_vslot46();
    virtual void fn_vslot47();
    virtual void fn_vslot48();
    virtual void fn_vslot49();
    virtual void fn_vslot50();
    virtual void fn_vslot51();
    virtual void fn_vslot52();
    virtual void fn_vslot53();
    virtual void fn_vslot54();
    virtual void fn_vslot55();
    virtual void fn_vslot56();
    virtual void fn_vslot57();
    virtual void fn_vslot58();
    virtual void fn_vslot59();
    virtual void fn_vslot60();
    virtual void fn_vslot61();
    virtual void fn_vslot62();
    virtual void fn_vslot63();
    virtual void fn_vslot64();
    virtual void fn_vslot65();
    virtual void fn_vslot66();
    virtual void fn_vslot67();
    virtual void fn_vslot68();
    virtual void fn_vslot69();
    virtual void fn_vslot70();
    virtual void fn_vslot71();
    virtual void fn_vslot72();
    virtual Unk800C7B40_Widget* fn_vslot73(int a, int b);
    virtual void fn_vslot74();
    virtual void fn_vslot75();
    virtual void fn_vslot76();
    virtual void fn_vslot77();
    virtual void fn_vslot78();
    virtual void fn_vslot79();
    virtual void fn_vslot80();
    virtual void fn_vslot81();
    virtual void fn_vslot82();
    virtual void fn_vslot83();
    virtual void fn_vslot84();
    virtual void fn_vslot85();
    virtual void fn_vslot86();
    virtual void fn_vslot87();
    virtual void fn_vslot88();
    virtual void fn_vslot89();
    virtual void fn_vslot90();
    virtual void fn_vslot91();
    virtual void fn_vslot92();
    virtual void fn_vslot93();
    virtual void fn_vslot94();
    virtual void fn_vslot95();
    virtual void fn_vslot96();
    virtual void fn_vslot97();
    virtual void fn_vslot98();
    virtual void fn_vslot99();
    virtual void fn_vslot100();
    virtual void fn_vslot101();
    virtual void fn_vslot102();
    virtual void fn_vslot103();
    virtual void fn_vslot104();
    virtual void fn_vslot105();
    virtual void fn_vslot106();
    virtual void fn_vslot107();
    virtual void fn_vslot108();
    virtual void fn_vslot109();
    virtual void fn_vslot110();
    virtual void fn_vslot111();
    virtual void fn_vslot112();
    virtual void fn_vslot113();
    virtual void fn_vslot114();
    virtual void fn_vslot115();
    virtual void fn_vslot116();
    virtual void fn_vslot117();
    virtual void fn_vslot118();
    virtual void fn_vslot119();
    virtual void fn_vslot120();
    virtual void fn_vslot121();
    virtual void fn_vslot122();
    virtual void fn_vslot123();
    virtual void fn_vslot124();
    virtual void fn_vslot125(int a, int b, int c, int d, int e);
    virtual void fn_vslot126();
    virtual void fn_vslot127();
};

/* Widget with two helper widgets at +0x78/+0x7C and their values at +0x70/+0x74 (fn_800C9728). */
struct Unk800C9728_Obj : Unk800C7B40_Widget {
    char pad40[0x30];
    int m70;
    int m74;
    Unk800C9728_Sub* m78;
    Unk800C9728_Sub* m7C;
};
extern const char lbl_806571E8[];

extern "C" Unk800D1614_Raw* fn_800CEEF0(Unk800D1614_Raw* self, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int s1, int s2, int s3, int s4, float f);

/* Name list (data up to +0x1C8, vptr at +0x1C8, vtable lbl_8065A470): ctor fn_800C6794. */
struct Unk800C6794_Pair {
    int a, b;
};
struct Unk800C6794_Data {
    int m0;
    int m4;
    char m8[0x2C];
    Unk800C7B40_Widget* m34[100];
    int m1C4;
};
struct Unk800C6794_Raw : Unk800C6794_Data {
    const void* m1C8;
};
struct Unk800C6794_Obj : Unk800C6794_Data {
    virtual void fn_vslot1();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4(const Unk800C6794_Pair* p);
};
extern const char lbl_8065A470[];

extern const char lbl_80657D88[];

/* Widget-layout virtuals with class-specific slot signatures. */
struct Unk800C1BE4_Widget : Unk800C7B40_Data {
    virtual ~Unk800C1BE4_Widget();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5(int a, int b, int c, int d);
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8(int a);
    virtual void fn_vslot9(int a);
    virtual void fn_vslot10(int a);
    virtual void fn_vslot11();
    virtual void fn_vslot12(int a);
    virtual void fn_vslot13(int a);
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15(int a);
    virtual void fn_vslot16(int a);
    virtual void fn_vslot17(int a);
    virtual void fn_vslot18(int a);
    virtual float fn_vslot19();
    virtual void fn_vslot20(float a);
    virtual void fn_vslot21(float a);
    virtual void fn_vslot22();
    virtual void fn_vslot23();
    virtual void fn_vslot24(int a);
    virtual void fn_vslot25();
    virtual void fn_vslot26();
    virtual void fn_vslot27();
    virtual Unk800C1BE4_Widget* fn_vslot28(const char* name, int a);
    virtual void fn_vslot29();
    virtual Unk800C7B40_Widget* fn_vslot30(const char* name, int a);
    virtual Unk800C7B40_Widget* fn_vslot31(const char* name, int a, int b);
    virtual Unk800C7B40_Widget* fn_vslot32(const char* name, int a);
    virtual char* fn_vslot33(int a);
    virtual void fn_vslot34();
    virtual void fn_vslot35();
    virtual void fn_vslot36();
    virtual void fn_vslot37();
    virtual void fn_vslot38(int a);
    virtual void fn_vslot39();
    virtual void fn_vslot40(int a);
    virtual void fn_vslot41();
    virtual void fn_vslot42();
    virtual void fn_vslot43();
    virtual void fn_vslot44();
    virtual void fn_vslot45();
    virtual void fn_vslot46();
    virtual void fn_vslot47();
    virtual void fn_vslot48();
    virtual void fn_vslot49();
    virtual void fn_vslot50();
    virtual void fn_vslot51();
    virtual void fn_vslot52();
    virtual void fn_vslot53();
    virtual void fn_vslot54();
    virtual void fn_vslot55();
    virtual void fn_vslot56();
    virtual void fn_vslot57();
    virtual void fn_vslot58();
    virtual void fn_vslot59();
    virtual void fn_vslot60();
    virtual void fn_vslot61();
    virtual void fn_vslot62();
    virtual void fn_vslot63();
    virtual void fn_vslot64();
    virtual void fn_vslot65();
    virtual void fn_vslot66();
    virtual void fn_vslot67();
    virtual void fn_vslot68();
    virtual void fn_vslot69();
    virtual void fn_vslot70();
    virtual void fn_vslot71();
    virtual void fn_vslot72();
    virtual Unk800C7B40_Widget* fn_vslot73(int a, int b);
    virtual void fn_vslot74();
    virtual void fn_vslot75();
    virtual void fn_vslot76();
    virtual void fn_vslot77();
    virtual void fn_vslot78();
    virtual void fn_vslot79();
    virtual void fn_vslot80();
    virtual void fn_vslot81();
    virtual void fn_vslot82();
    virtual void fn_vslot83();
    virtual void fn_vslot84();
    virtual void fn_vslot85();
    virtual void fn_vslot86();
    virtual void fn_vslot87();
    virtual void fn_vslot88();
    virtual void fn_vslot89();
    virtual void fn_vslot90();
    virtual void fn_vslot91();
    virtual void fn_vslot92();
    virtual void fn_vslot93();
    virtual void fn_vslot94();
    virtual void fn_vslot95();
    virtual void fn_vslot96();
    virtual void fn_vslot97();
    virtual void fn_vslot98();
    virtual void fn_vslot99();
    virtual void fn_vslot100();
    virtual void fn_vslot101();
    virtual void fn_vslot102();
    virtual void fn_vslot103();
    virtual void fn_vslot104();
    virtual void fn_vslot105();
    virtual void fn_vslot106();
    virtual void fn_vslot107();
    virtual void fn_vslot108();
    virtual void fn_vslot109();
    virtual void fn_vslot110();
    virtual void fn_vslot111();
    virtual void fn_vslot112();
    virtual void fn_vslot113();
    virtual void fn_vslot114();
    virtual void fn_vslot115();
    virtual void fn_vslot116();
    virtual void fn_vslot117();
    virtual void fn_vslot118();
    virtual void fn_vslot119();
    virtual void fn_vslot120();
    virtual void fn_vslot121();
    virtual void fn_vslot122();
    virtual void fn_vslot123();
    virtual void fn_vslot124();
    virtual void fn_vslot125(int a, int b, int c, int d, int e);
    virtual void fn_vslot126();
    virtual void fn_vslot127();
};

inline void* operator new(unsigned int, void* p)
{
    return p;
}
/* Widget whose constructor chain is fn_800D1758 -> fn_800D26C8 -> fn_800D30D4 / fn_800D3168. */
struct Unk800D26C8_Raw : Unk800D1614_Raw {
    char pad58[0x18];
    int m70;
    int m74;
    char pad78[0x20];
    Unk800CFEEC_Raw m98;
    char padC0[0x150];
    int m210;
};
extern char lbl_805B6F58;
extern const char lbl_80658188[];
extern const char lbl_80658248[];
extern const char lbl_80657308[];
extern const char lbl_806573C8[];
extern const char lbl_80657078[];
extern "C" {
Unk800D26C8_Raw* fn_800D1758(Unk800D26C8_Raw* self, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int s0, int s1,
                             int s2, int s3, int s4, int s5, int s6, int s7, int s8, int s9, float f1, float f2);
Unk800D26C8_Raw* fn_800D26C8(Unk800D26C8_Raw* self, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int s0, int s1,
                             int s2, int s3, int s4, int s5, float f1, float f2);
}

struct Unk800D1CA4_Tbl {
    const char* v[19];
};
extern const Unk800D1CA4_Tbl lbl_805BC8F0 __attribute__((aligned(4)));

/* Widget with a name pointer at +0x40 and an id at +0x44 (fn_800D3EAC); flag word at +0x68 as in Unk800D2338_Obj. */
struct Unk800D3EAC_Obj : Unk800C7B40_Widget {
    char* m40;
    int m44;
    char pad48[0x20];
    int b68_0 : 1;
    int b68_1 : 1;
    int b68_2 : 1;
    int rest68 : 29;
};

/* Widget with a count/mode pair at +0x50/+0x54 and a helper widget at +0x7C (fn_800CAF58). */
struct Unk800CAF58_Obj : Unk800C7B40_Widget {
    char pad40[0x10];
    int m50;
    int m54;
    char pad58[0xC];
    int m64;
    char pad68[0x14];
    Unk800C7B40_Widget* m7C;
};

/* Vertical sibling of Unk800CAF58_Obj (fn_800CA670 / fn_800CA9A4). */
struct Unk800CA670_Obj : Unk800C7B40_Widget {
    char pad40[0x10];
    int m50;
    int m54;
    char pad58[8];
    int m60;
    char pad64[0x14];
    Unk800C7B40_Widget* m78;
};

/* Widget-layout virtuals with class-specific slot signatures. */
struct Unk800CEE28_Sub : Unk800C7B40_Data {
    virtual ~Unk800CEE28_Sub();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5(int a, int b, int c, int d);
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8(int a);
    virtual void fn_vslot9(int a);
    virtual void fn_vslot10(int a);
    virtual void fn_vslot11();
    virtual void fn_vslot12(int a);
    virtual void fn_vslot13(int a);
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15(int a);
    virtual void fn_vslot16(int a);
    virtual void fn_vslot17(int a);
    virtual void fn_vslot18(int a);
    virtual float fn_vslot19();
    virtual void fn_vslot20(float a);
    virtual void fn_vslot21(float a);
    virtual void fn_vslot22();
    virtual void fn_vslot23();
    virtual void fn_vslot24(int a);
    virtual void fn_vslot25();
    virtual void fn_vslot26();
    virtual void fn_vslot27();
    virtual Unk800C7B40_Widget* fn_vslot28(const char* name);
    virtual void fn_vslot29();
    virtual Unk800C7B40_Widget* fn_vslot30(const char* name, int a);
    virtual int* fn_vslot31();
    virtual Unk800C7B40_Widget* fn_vslot32(const char* name, int a);
    virtual char* fn_vslot33(int a);
    virtual void fn_vslot34();
    virtual void fn_vslot35();
    virtual int* fn_vslot36();
    virtual int* fn_vslot37();
    virtual void fn_vslot38(int a);
    virtual void fn_vslot39();
    virtual void fn_vslot40(int a);
    virtual void fn_vslot41();
    virtual void fn_vslot42();
    virtual void fn_vslot43();
    virtual void fn_vslot44();
    virtual void fn_vslot45();
    virtual void fn_vslot46();
    virtual void fn_vslot47();
    virtual void fn_vslot48();
    virtual void fn_vslot49();
    virtual void fn_vslot50();
    virtual void fn_vslot51();
    virtual void fn_vslot52();
    virtual void fn_vslot53();
    virtual void fn_vslot54();
    virtual void fn_vslot55();
    virtual void fn_vslot56();
    virtual void fn_vslot57();
    virtual void fn_vslot58();
    virtual void fn_vslot59();
    virtual int fn_vslot60();
    virtual int fn_vslot61();
    virtual void fn_vslot62();
    virtual void fn_vslot63();
    virtual void fn_vslot64();
    virtual void fn_vslot65();
    virtual void fn_vslot66();
    virtual void fn_vslot67();
    virtual void fn_vslot68();
    virtual void fn_vslot69();
    virtual void fn_vslot70();
    virtual void fn_vslot71();
    virtual void fn_vslot72();
    virtual Unk800C7B40_Widget* fn_vslot73(int a, int b);
    virtual void fn_vslot74();
    virtual void fn_vslot75();
    virtual void fn_vslot76();
    virtual void fn_vslot77();
    virtual void fn_vslot78();
    virtual void fn_vslot79();
    virtual void fn_vslot80();
    virtual void fn_vslot81();
    virtual void fn_vslot82();
    virtual void fn_vslot83();
    virtual void fn_vslot84();
    virtual const char* fn_vslot85();
    virtual void fn_vslot86();
    virtual void fn_vslot87();
    virtual void fn_vslot88();
    virtual void fn_vslot89();
    virtual void fn_vslot90();
    virtual void fn_vslot91();
    virtual void fn_vslot92();
    virtual void fn_vslot93();
    virtual void fn_vslot94();
    virtual void fn_vslot95();
    virtual void fn_vslot96();
    virtual void fn_vslot97();
    virtual void fn_vslot98();
    virtual void fn_vslot99();
    virtual void fn_vslot100();
    virtual void fn_vslot101();
    virtual void fn_vslot102();
    virtual void fn_vslot103();
    virtual void fn_vslot104();
    virtual void fn_vslot105();
    virtual void fn_vslot106();
    virtual void fn_vslot107(int a);
    virtual void fn_vslot108();
    virtual void fn_vslot109();
    virtual void fn_vslot110();
    virtual void fn_vslot111();
    virtual void fn_vslot112();
    virtual void fn_vslot113();
    virtual void fn_vslot114();
    virtual void fn_vslot115();
    virtual void fn_vslot116();
    virtual void fn_vslot117();
    virtual void fn_vslot118(int a);
    virtual void fn_vslot119(int a);
    virtual void fn_vslot120();
    virtual void fn_vslot121();
    virtual void fn_vslot122();
    virtual void fn_vslot123();
    virtual void fn_vslot124();
    virtual void fn_vslot125(int a, int b, int c, int d, int e);
    virtual void fn_vslot126();
    virtual void fn_vslot127();
};

/* Widget whose helper at +0x78 hands out id slots (fn_800CEE28). */
struct Unk800CEE28_Obj : Unk800C7B40_Widget {
    char pad40[0x38];
    Unk800CEE28_Sub* m78;
};
extern "C" void fn_800C83B4(Unk800C7B40_Widget* w, int v);

/* Widget-layout virtuals with class-specific slot signatures. */
struct Unk800CC208_Sub : Unk800C7B40_Data {
    virtual ~Unk800CC208_Sub();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5(int a, int b, int c, int d);
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8(int a);
    virtual void fn_vslot9(int a);
    virtual void fn_vslot10(int a);
    virtual void fn_vslot11();
    virtual void fn_vslot12(int a);
    virtual void fn_vslot13(int a);
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15(int a);
    virtual void fn_vslot16(int a);
    virtual void fn_vslot17(int a);
    virtual void fn_vslot18(int a);
    virtual float fn_vslot19();
    virtual void fn_vslot20(float a);
    virtual void fn_vslot21(float a);
    virtual void fn_vslot22();
    virtual void fn_vslot23();
    virtual void fn_vslot24(int a);
    virtual void fn_vslot25();
    virtual void fn_vslot26();
    virtual void fn_vslot27();
    virtual Unk800C7B40_Widget* fn_vslot28(const char* name);
    virtual void fn_vslot29();
    virtual void fn_vslot30(const char* name);
    virtual Unk800C7B40_Widget* fn_vslot31(const char* name, int a, int b);
    virtual Unk800C7B40_Widget* fn_vslot32(const char* name, int a);
    virtual char* fn_vslot33(int a);
    virtual void fn_vslot34();
    virtual void fn_vslot35();
    virtual void fn_vslot36();
    virtual void fn_vslot37();
    virtual void fn_vslot38(int a);
    virtual void fn_vslot39();
    virtual void fn_vslot40(int a);
    virtual void fn_vslot41();
    virtual void fn_vslot42();
    virtual void fn_vslot43();
    virtual void fn_vslot44();
    virtual void fn_vslot45();
    virtual void fn_vslot46();
    virtual void fn_vslot47();
    virtual void fn_vslot48();
    virtual void fn_vslot49();
    virtual void fn_vslot50();
    virtual void fn_vslot51();
    virtual void fn_vslot52();
    virtual void fn_vslot53();
    virtual void fn_vslot54();
    virtual void fn_vslot55();
    virtual void fn_vslot56();
    virtual void fn_vslot57();
    virtual void fn_vslot58();
    virtual void fn_vslot59();
    virtual int fn_vslot60();
    virtual int fn_vslot61();
    virtual void fn_vslot62();
    virtual void fn_vslot63();
    virtual void fn_vslot64();
    virtual void fn_vslot65();
    virtual void fn_vslot66();
    virtual void fn_vslot67();
    virtual void fn_vslot68();
    virtual void fn_vslot69();
    virtual void fn_vslot70();
    virtual void fn_vslot71();
    virtual void fn_vslot72();
    virtual Unk800C7B40_Widget* fn_vslot73(int a, int b);
    virtual void fn_vslot74();
    virtual void fn_vslot75();
    virtual void fn_vslot76();
    virtual void fn_vslot77();
    virtual void fn_vslot78();
    virtual void fn_vslot79();
    virtual void fn_vslot80();
    virtual void fn_vslot81();
    virtual void fn_vslot82();
    virtual void fn_vslot83();
    virtual void fn_vslot84();
    virtual const char* fn_vslot85();
    virtual void fn_vslot86();
    virtual void fn_vslot87();
    virtual void fn_vslot88();
    virtual void fn_vslot89();
    virtual void fn_vslot90();
    virtual void fn_vslot91();
    virtual void fn_vslot92();
    virtual void fn_vslot93();
    virtual void fn_vslot94();
    virtual void fn_vslot95();
    virtual void fn_vslot96();
    virtual void fn_vslot97();
    virtual void fn_vslot98();
    virtual void fn_vslot99();
    virtual void fn_vslot100();
    virtual void fn_vslot101();
    virtual void fn_vslot102();
    virtual void fn_vslot103();
    virtual void fn_vslot104();
    virtual void fn_vslot105();
    virtual void fn_vslot106();
    virtual void fn_vslot107(int a);
    virtual void fn_vslot108();
    virtual void fn_vslot109();
    virtual void fn_vslot110();
    virtual void fn_vslot111();
    virtual void fn_vslot112();
    virtual void fn_vslot113();
    virtual void fn_vslot114();
    virtual void fn_vslot115();
    virtual void fn_vslot116();
    virtual void fn_vslot117();
    virtual void fn_vslot118(int a);
    virtual void fn_vslot119(int a);
    virtual void fn_vslot120();
    virtual void fn_vslot121();
    virtual void fn_vslot122();
    virtual void fn_vslot123();
    virtual void fn_vslot124();
    virtual void fn_vslot125(int a, int b, int c, int d, int e);
    virtual void fn_vslot126();
    virtual void fn_vslot127();
};

/* Widget-layout virtuals with class-specific slot signatures. */
struct Unk800CD4B8_Sub : Unk800C7B40_Data {
    virtual ~Unk800CD4B8_Sub();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5(int a, int b, int c, int d);
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8(int a);
    virtual void fn_vslot9(int a);
    virtual void fn_vslot10(int a);
    virtual void fn_vslot11();
    virtual void fn_vslot12(int a);
    virtual void fn_vslot13(int a);
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15(int a);
    virtual void fn_vslot16(int a);
    virtual void fn_vslot17(int a);
    virtual void fn_vslot18(int a);
    virtual float fn_vslot19();
    virtual void fn_vslot20(float a);
    virtual void fn_vslot21(float a);
    virtual void fn_vslot22();
    virtual void fn_vslot23();
    virtual void fn_vslot24(int a);
    virtual void fn_vslot25();
    virtual void fn_vslot26();
    virtual void fn_vslot27();
    virtual Unk800C7B40_Widget* fn_vslot28(const char* name);
    virtual void fn_vslot29();
    virtual Unk800C7B40_Widget* fn_vslot30(const char* name, int a);
    virtual Unk800C7B40_Widget* fn_vslot31(const char* name, int a, int b);
    virtual Unk800C7B40_Widget* fn_vslot32(const char* name, int a);
    virtual char* fn_vslot33(int a);
    virtual void fn_vslot34();
    virtual void fn_vslot35();
    virtual void fn_vslot36();
    virtual void fn_vslot37();
    virtual void fn_vslot38(int a);
    virtual void fn_vslot39();
    virtual void fn_vslot40(int a);
    virtual void fn_vslot41();
    virtual void fn_vslot42();
    virtual void fn_vslot43();
    virtual void fn_vslot44();
    virtual void fn_vslot45();
    virtual void fn_vslot46();
    virtual void fn_vslot47();
    virtual void fn_vslot48();
    virtual void fn_vslot49();
    virtual void fn_vslot50();
    virtual void fn_vslot51();
    virtual void fn_vslot52();
    virtual void fn_vslot53();
    virtual void fn_vslot54();
    virtual void fn_vslot55();
    virtual void fn_vslot56();
    virtual void fn_vslot57();
    virtual void fn_vslot58();
    virtual void fn_vslot59();
    virtual int fn_vslot60();
    virtual int fn_vslot61();
    virtual void fn_vslot62();
    virtual void fn_vslot63();
    virtual void fn_vslot64();
    virtual void fn_vslot65();
    virtual void fn_vslot66();
    virtual void fn_vslot67();
    virtual void fn_vslot68();
    virtual void fn_vslot69(Unk800CB3A8_Grid* g);
    virtual void fn_vslot70();
    virtual void fn_vslot71();
    virtual void fn_vslot72();
    virtual Unk800C7B40_Widget* fn_vslot73(int a, int b);
    virtual void fn_vslot74();
    virtual void fn_vslot75();
    virtual void fn_vslot76();
    virtual void fn_vslot77();
    virtual void fn_vslot78();
    virtual void fn_vslot79();
    virtual void fn_vslot80();
    virtual void fn_vslot81();
    virtual void fn_vslot82();
    virtual void fn_vslot83();
    virtual void fn_vslot84();
    virtual const char* fn_vslot85();
    virtual void fn_vslot86();
    virtual void fn_vslot87();
    virtual void fn_vslot88();
    virtual void fn_vslot89();
    virtual void fn_vslot90();
    virtual void fn_vslot91();
    virtual void fn_vslot92();
    virtual void fn_vslot93();
    virtual void fn_vslot94();
    virtual void fn_vslot95();
    virtual void fn_vslot96();
    virtual void fn_vslot97();
    virtual void fn_vslot98();
    virtual void fn_vslot99();
    virtual void fn_vslot100();
    virtual void fn_vslot101();
    virtual void fn_vslot102();
    virtual void fn_vslot103();
    virtual void fn_vslot104();
    virtual void fn_vslot105();
    virtual void fn_vslot106();
    virtual void fn_vslot107(int a);
    virtual void fn_vslot108();
    virtual void fn_vslot109();
    virtual void fn_vslot110();
    virtual void fn_vslot111();
    virtual void fn_vslot112();
    virtual void fn_vslot113();
    virtual void fn_vslot114();
    virtual void fn_vslot115();
    virtual void fn_vslot116();
    virtual void fn_vslot117();
    virtual void fn_vslot118(int a);
    virtual void fn_vslot119(int a);
    virtual void fn_vslot120();
    virtual void fn_vslot121();
    virtual void fn_vslot122();
    virtual void fn_vslot123();
    virtual void fn_vslot124();
    virtual void fn_vslot125(int a, int b, int c, int d, int e);
    virtual void fn_vslot126();
    virtual void fn_vslot127();
};

/* Widget-layout virtuals with class-specific slot signatures. */
struct Unk800CC08C_W : Unk800C7B40_Data {
    virtual ~Unk800CC08C_W();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5(int a, int b, int c, int d);
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8(int a);
    virtual void fn_vslot9(int a);
    virtual void fn_vslot10(int a);
    virtual void fn_vslot11();
    virtual void fn_vslot12(int a);
    virtual void fn_vslot13(int a);
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15(int a);
    virtual void fn_vslot16(int a);
    virtual void fn_vslot17(int a);
    virtual void fn_vslot18(int a);
    virtual float fn_vslot19();
    virtual void fn_vslot20(float a);
    virtual void fn_vslot21(float a);
    virtual void fn_vslot22();
    virtual void fn_vslot23();
    virtual void fn_vslot24(int a);
    virtual void fn_vslot25();
    virtual void fn_vslot26();
    virtual void fn_vslot27();
    virtual Unk800C7B40_Widget* fn_vslot28(const char* name);
    virtual void fn_vslot29();
    virtual Unk800C7B40_Widget* fn_vslot30(const char* name, int a);
    virtual Unk800C7B40_Widget* fn_vslot31(const char* name, int a, int b);
    virtual Unk800C7B40_Widget* fn_vslot32(const char* name, int a);
    virtual void* fn_vslot33(int a, int b);
    virtual void fn_vslot34();
    virtual void fn_vslot35();
    virtual void fn_vslot36();
    virtual void fn_vslot37();
    virtual void fn_vslot38(int a);
    virtual void fn_vslot39();
    virtual void fn_vslot40(int a);
    virtual void fn_vslot41();
    virtual void fn_vslot42();
    virtual void fn_vslot43();
    virtual void fn_vslot44();
    virtual void fn_vslot45();
    virtual void fn_vslot46();
    virtual void fn_vslot47();
    virtual void fn_vslot48();
    virtual void fn_vslot49();
    virtual void fn_vslot50();
    virtual void fn_vslot51();
    virtual void fn_vslot52();
    virtual void fn_vslot53();
    virtual void fn_vslot54();
    virtual void fn_vslot55();
    virtual void fn_vslot56();
    virtual void fn_vslot57();
    virtual void fn_vslot58();
    virtual void fn_vslot59();
    virtual int fn_vslot60();
    virtual int fn_vslot61();
    virtual void fn_vslot62();
    virtual void fn_vslot63();
    virtual void fn_vslot64();
    virtual void fn_vslot65();
    virtual void fn_vslot66();
    virtual void fn_vslot67();
    virtual void fn_vslot68();
    virtual void fn_vslot69();
    virtual void fn_vslot70();
    virtual void fn_vslot71();
    virtual void fn_vslot72();
    virtual Unk800C7B40_Widget* fn_vslot73(int a, int b);
    virtual void fn_vslot74();
    virtual void fn_vslot75();
    virtual void fn_vslot76();
    virtual void fn_vslot77();
    virtual void fn_vslot78();
    virtual void fn_vslot79();
    virtual void fn_vslot80();
    virtual void fn_vslot81();
    virtual void fn_vslot82();
    virtual void fn_vslot83();
    virtual void fn_vslot84();
    virtual const char* fn_vslot85();
    virtual void fn_vslot86();
    virtual void fn_vslot87();
    virtual void fn_vslot88();
    virtual void fn_vslot89();
    virtual void fn_vslot90();
    virtual void fn_vslot91();
    virtual void fn_vslot92();
    virtual void fn_vslot93();
    virtual void fn_vslot94();
    virtual void fn_vslot95();
    virtual void fn_vslot96();
    virtual void fn_vslot97();
    virtual void fn_vslot98();
    virtual void fn_vslot99();
    virtual void fn_vslot100();
    virtual void fn_vslot101();
    virtual void fn_vslot102();
    virtual void fn_vslot103();
    virtual void fn_vslot104();
    virtual void fn_vslot105();
    virtual void fn_vslot106();
    virtual void fn_vslot107(int a);
    virtual void fn_vslot108();
    virtual void fn_vslot109();
    virtual void fn_vslot110();
    virtual void fn_vslot111();
    virtual void fn_vslot112();
    virtual void fn_vslot113();
    virtual void fn_vslot114();
    virtual void fn_vslot115();
    virtual void fn_vslot116();
    virtual void fn_vslot117();
    virtual void fn_vslot118(int a);
    virtual void fn_vslot119(int a);
    virtual void fn_vslot120();
    virtual void fn_vslot121();
    virtual void fn_vslot122();
    virtual void fn_vslot123();
    virtual void fn_vslot124();
    virtual void fn_vslot125(int a, int b, int c, int d, int e);
    virtual void fn_vslot126();
    virtual void fn_vslot127();
};

/* Widget-layout virtuals with class-specific slot signatures. */
struct Unk800CC3C8_Sub : Unk800C7B40_Data {
    virtual ~Unk800CC3C8_Sub();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5(int a, int b, int c, int d);
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8(int a);
    virtual void fn_vslot9(int a);
    virtual void fn_vslot10(int a);
    virtual void fn_vslot11();
    virtual void fn_vslot12(int a);
    virtual void fn_vslot13(int a);
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15(int a);
    virtual void fn_vslot16(int a);
    virtual void fn_vslot17(int a);
    virtual void fn_vslot18(int a);
    virtual float fn_vslot19();
    virtual void fn_vslot20(float a);
    virtual void fn_vslot21(float a);
    virtual void fn_vslot22();
    virtual void fn_vslot23();
    virtual void fn_vslot24(int a);
    virtual void fn_vslot25();
    virtual void fn_vslot26();
    virtual void fn_vslot27();
    virtual Unk800C7B40_Widget* fn_vslot28(const char* name);
    virtual void fn_vslot29();
    virtual Unk800C7B40_Widget* fn_vslot30(const char* name, int a);
    virtual char* fn_vslot31();
    virtual Unk800C7B40_Widget* fn_vslot32(const char* name, int a);
    virtual char* fn_vslot33(int a);
    virtual void fn_vslot34();
    virtual void fn_vslot35();
    virtual void fn_vslot36();
    virtual void fn_vslot37();
    virtual void fn_vslot38(int a);
    virtual void fn_vslot39();
    virtual void fn_vslot40(int a);
    virtual void fn_vslot41();
    virtual void fn_vslot42();
    virtual void fn_vslot43();
    virtual void fn_vslot44();
    virtual void fn_vslot45();
    virtual void fn_vslot46();
    virtual void fn_vslot47();
    virtual void fn_vslot48();
    virtual void fn_vslot49();
    virtual void fn_vslot50();
    virtual void fn_vslot51();
    virtual void fn_vslot52();
    virtual void fn_vslot53();
    virtual void fn_vslot54();
    virtual void fn_vslot55();
    virtual void fn_vslot56();
    virtual void fn_vslot57();
    virtual void fn_vslot58();
    virtual void fn_vslot59();
    virtual int fn_vslot60();
    virtual int fn_vslot61();
    virtual void fn_vslot62();
    virtual void fn_vslot63();
    virtual void fn_vslot64();
    virtual void fn_vslot65();
    virtual void fn_vslot66();
    virtual void fn_vslot67();
    virtual void fn_vslot68();
    virtual void fn_vslot69();
    virtual void fn_vslot70();
    virtual void fn_vslot71();
    virtual void fn_vslot72();
    virtual Unk800C7B40_Widget* fn_vslot73(int a, int b);
    virtual void fn_vslot74();
    virtual void fn_vslot75();
    virtual void fn_vslot76();
    virtual void fn_vslot77();
    virtual void fn_vslot78();
    virtual void fn_vslot79();
    virtual void fn_vslot80();
    virtual void fn_vslot81();
    virtual void fn_vslot82();
    virtual void fn_vslot83();
    virtual void fn_vslot84();
    virtual const char* fn_vslot85();
    virtual void fn_vslot86();
    virtual void fn_vslot87();
    virtual void fn_vslot88();
    virtual void fn_vslot89();
    virtual void fn_vslot90();
    virtual void fn_vslot91();
    virtual void fn_vslot92();
    virtual void fn_vslot93();
    virtual void fn_vslot94();
    virtual void fn_vslot95();
    virtual void fn_vslot96();
    virtual void fn_vslot97();
    virtual void fn_vslot98();
    virtual void fn_vslot99();
    virtual void fn_vslot100();
    virtual void fn_vslot101();
    virtual void fn_vslot102();
    virtual void fn_vslot103();
    virtual void fn_vslot104();
    virtual void fn_vslot105();
    virtual void fn_vslot106();
    virtual void fn_vslot107(int a);
    virtual void fn_vslot108();
    virtual void fn_vslot109();
    virtual void fn_vslot110();
    virtual void fn_vslot111();
    virtual void fn_vslot112();
    virtual void fn_vslot113();
    virtual void fn_vslot114();
    virtual void fn_vslot115();
    virtual void fn_vslot116();
    virtual void fn_vslot117();
    virtual void fn_vslot118(int a);
    virtual void fn_vslot119(int a);
    virtual void fn_vslot120();
    virtual void fn_vslot121();
    virtual void fn_vslot122();
    virtual void fn_vslot123();
    virtual void fn_vslot124();
    virtual void fn_vslot125(int a, int b, int c, int d, int e);
    virtual void fn_vslot126();
    virtual void fn_vslot127();
};

/* Widget-layout virtuals with class-specific slot signatures. */
struct Unk800CBAD0_Sub : Unk800C7B40_Data {
    virtual ~Unk800CBAD0_Sub();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5(int a, int b, int c, int d);
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8(int a);
    virtual void fn_vslot9(int a);
    virtual void fn_vslot10(int a);
    virtual void fn_vslot11();
    virtual void fn_vslot12(int a);
    virtual void fn_vslot13(int a);
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15(int a);
    virtual void fn_vslot16(int a);
    virtual void fn_vslot17(int a);
    virtual void fn_vslot18(int a);
    virtual float fn_vslot19();
    virtual void fn_vslot20(float a);
    virtual void fn_vslot21(float a);
    virtual void fn_vslot22();
    virtual void fn_vslot23();
    virtual void fn_vslot24(int a);
    virtual void fn_vslot25();
    virtual void fn_vslot26();
    virtual void fn_vslot27();
    virtual Unk800C7B40_Widget* fn_vslot28(const char* name);
    virtual void fn_vslot29(int a);
    virtual Unk800C7B40_Widget* fn_vslot30(const char* name, int a);
    virtual Unk800C7B40_Widget* fn_vslot31(const char* name, int a, int b);
    virtual Unk800C7B40_Widget* fn_vslot32(const char* name, int a);
    virtual char* fn_vslot33(int a);
    virtual void fn_vslot34();
    virtual void fn_vslot35();
    virtual void fn_vslot36();
    virtual void fn_vslot37();
    virtual void fn_vslot38(int a);
    virtual void fn_vslot39();
    virtual void fn_vslot40(int a);
    virtual void fn_vslot41();
    virtual void fn_vslot42();
    virtual void fn_vslot43();
    virtual void fn_vslot44();
    virtual void fn_vslot45();
    virtual void fn_vslot46();
    virtual void fn_vslot47();
    virtual void fn_vslot48();
    virtual void fn_vslot49();
    virtual void fn_vslot50();
    virtual void fn_vslot51();
    virtual void fn_vslot52();
    virtual void fn_vslot53();
    virtual void fn_vslot54();
    virtual void fn_vslot55();
    virtual void fn_vslot56();
    virtual void fn_vslot57();
    virtual void fn_vslot58();
    virtual void fn_vslot59();
    virtual int fn_vslot60();
    virtual int fn_vslot61();
    virtual void fn_vslot62();
    virtual void fn_vslot63();
    virtual void fn_vslot64();
    virtual void fn_vslot65();
    virtual void fn_vslot66();
    virtual void fn_vslot67();
    virtual void fn_vslot68();
    virtual void fn_vslot69();
    virtual void fn_vslot70();
    virtual void fn_vslot71();
    virtual void fn_vslot72();
    virtual Unk800C7B40_Widget* fn_vslot73(int a, int b);
    virtual void fn_vslot74();
    virtual void fn_vslot75();
    virtual void fn_vslot76();
    virtual void fn_vslot77();
    virtual void fn_vslot78();
    virtual void fn_vslot79();
    virtual void fn_vslot80();
    virtual void fn_vslot81();
    virtual void fn_vslot82();
    virtual void fn_vslot83();
    virtual void fn_vslot84();
    virtual const char* fn_vslot85();
    virtual void fn_vslot86();
    virtual void fn_vslot87();
    virtual void fn_vslot88();
    virtual void fn_vslot89();
    virtual void fn_vslot90();
    virtual void fn_vslot91();
    virtual void fn_vslot92();
    virtual void fn_vslot93();
    virtual void fn_vslot94();
    virtual void fn_vslot95();
    virtual void fn_vslot96();
    virtual void fn_vslot97();
    virtual void fn_vslot98();
    virtual void fn_vslot99();
    virtual void fn_vslot100();
    virtual void fn_vslot101();
    virtual void fn_vslot102();
    virtual void fn_vslot103();
    virtual void fn_vslot104();
    virtual void fn_vslot105();
    virtual void fn_vslot106();
    virtual void fn_vslot107(int a);
    virtual void fn_vslot108();
    virtual void fn_vslot109();
    virtual void fn_vslot110();
    virtual void fn_vslot111();
    virtual void fn_vslot112();
    virtual void fn_vslot113();
    virtual void fn_vslot114();
    virtual void fn_vslot115();
    virtual void fn_vslot116();
    virtual void fn_vslot117();
    virtual void fn_vslot118(int a);
    virtual void fn_vslot119(int a);
    virtual void fn_vslot120();
    virtual void fn_vslot121();
    virtual void fn_vslot122();
    virtual void fn_vslot123();
    virtual void fn_vslot124();
    virtual void fn_vslot125(int a, int b, int c, int d, int e);
    virtual void fn_vslot126();
    virtual void fn_vslot127();
};

/* Widget-layout virtuals with class-specific slot signatures. */
struct Unk800D350C_Sub : Unk800C7B40_Data {
    virtual ~Unk800D350C_Sub();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5(int a, int b, int c, int d);
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8(int a);
    virtual void fn_vslot9(int a);
    virtual void fn_vslot10(int a);
    virtual void fn_vslot11();
    virtual void fn_vslot12(int a);
    virtual void fn_vslot13(int a);
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15(int a);
    virtual void fn_vslot16(int a);
    virtual void fn_vslot17(int a);
    virtual void fn_vslot18(int a);
    virtual float fn_vslot19();
    virtual void fn_vslot20(float a);
    virtual void fn_vslot21(float a);
    virtual void fn_vslot22();
    virtual void fn_vslot23();
    virtual void fn_vslot24(int a);
    virtual void fn_vslot25();
    virtual void fn_vslot26();
    virtual void fn_vslot27();
    virtual Unk800C7B40_Widget* fn_vslot28(const char* name);
    virtual void fn_vslot29();
    virtual Unk800C7B40_Widget* fn_vslot30(const char* name, int a);
    virtual Unk800C7B40_Widget* fn_vslot31(const char* name, int a, int b);
    virtual Unk800C7B40_Widget* fn_vslot32(const char* name, int a);
    virtual char* fn_vslot33(int a);
    virtual void fn_vslot34();
    virtual void fn_vslot35();
    virtual void fn_vslot36();
    virtual void fn_vslot37();
    virtual void fn_vslot38(int a);
    virtual void fn_vslot39();
    virtual void fn_vslot40(int a);
    virtual void fn_vslot41();
    virtual void fn_vslot42();
    virtual void fn_vslot43();
    virtual void fn_vslot44();
    virtual void fn_vslot45();
    virtual void fn_vslot46();
    virtual void fn_vslot47();
    virtual void fn_vslot48();
    virtual void fn_vslot49();
    virtual void fn_vslot50();
    virtual void fn_vslot51();
    virtual void fn_vslot52();
    virtual void fn_vslot53();
    virtual void fn_vslot54();
    virtual void fn_vslot55();
    virtual void fn_vslot56();
    virtual void fn_vslot57();
    virtual void fn_vslot58();
    virtual void fn_vslot59();
    virtual int fn_vslot60();
    virtual int fn_vslot61();
    virtual void fn_vslot62();
    virtual void fn_vslot63();
    virtual void fn_vslot64();
    virtual void fn_vslot65();
    virtual void fn_vslot66();
    virtual void fn_vslot67();
    virtual void fn_vslot68(Unk800C7B40_Widget* w, int a);
    virtual void fn_vslot69();
    virtual void fn_vslot70();
    virtual void fn_vslot71();
    virtual void fn_vslot72();
    virtual Unk800C7B40_Widget* fn_vslot73(int a, int b);
    virtual void fn_vslot74();
    virtual void fn_vslot75();
    virtual void fn_vslot76();
    virtual void fn_vslot77();
    virtual void fn_vslot78();
    virtual void fn_vslot79();
    virtual void fn_vslot80();
    virtual void fn_vslot81();
    virtual void fn_vslot82();
    virtual void fn_vslot83();
    virtual void fn_vslot84();
    virtual const char* fn_vslot85();
    virtual void fn_vslot86();
    virtual void fn_vslot87();
    virtual void fn_vslot88();
    virtual void fn_vslot89();
    virtual void fn_vslot90();
    virtual void fn_vslot91();
    virtual void fn_vslot92();
    virtual void fn_vslot93();
    virtual void fn_vslot94();
    virtual void fn_vslot95();
    virtual void fn_vslot96();
    virtual void fn_vslot97();
    virtual void fn_vslot98();
    virtual void fn_vslot99();
    virtual void fn_vslot100();
    virtual void fn_vslot101();
    virtual void fn_vslot102();
    virtual void fn_vslot103();
    virtual void fn_vslot104();
    virtual void fn_vslot105();
    virtual void fn_vslot106();
    virtual void fn_vslot107(int a);
    virtual void fn_vslot108();
    virtual void fn_vslot109();
    virtual void fn_vslot110();
    virtual void fn_vslot111();
    virtual void fn_vslot112();
    virtual void fn_vslot113();
    virtual void fn_vslot114();
    virtual void fn_vslot115();
    virtual void fn_vslot116();
    virtual void fn_vslot117();
    virtual void fn_vslot118(int a);
    virtual void fn_vslot119(int a);
    virtual void fn_vslot120();
    virtual void fn_vslot121();
    virtual void fn_vslot122();
    virtual void fn_vslot123();
    virtual void fn_vslot124();
    virtual void fn_vslot125(int a, int b, int c, int d, int e);
    virtual void fn_vslot126();
    virtual void fn_vslot127();
};

/* Widget-layout virtuals with class-specific slot signatures. */
struct Unk800D3F0C_V : Unk800C7B40_Data {
    virtual ~Unk800D3F0C_V();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5(int a, int b, int c, int d);
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8(int a);
    virtual void fn_vslot9(int a);
    virtual void fn_vslot10(int a);
    virtual void fn_vslot11();
    virtual void fn_vslot12(int a);
    virtual void fn_vslot13(int a);
    virtual void fn_vslot14(int a);
    virtual void fn_vslot15(int a);
    virtual void fn_vslot16(int a);
    virtual void fn_vslot17(int a);
    virtual void fn_vslot18(int a);
    virtual float fn_vslot19();
    virtual void fn_vslot20(float a);
    virtual void fn_vslot21(float a);
    virtual void fn_vslot22();
    virtual void fn_vslot23();
    virtual void fn_vslot24(int a);
    virtual void fn_vslot25();
    virtual void fn_vslot26();
    virtual void fn_vslot27();
    virtual Unk800C7B40_Widget* fn_vslot28(const char* name);
    virtual void fn_vslot29();
    virtual Unk800C7B40_Widget* fn_vslot30(const char* name, int a);
    virtual int fn_vslot31();
    virtual Unk800C7B40_Widget* fn_vslot32(const char* name, int a);
    virtual char* fn_vslot33(int a);
    virtual void fn_vslot34();
    virtual void fn_vslot35();
    virtual void fn_vslot36();
    virtual void fn_vslot37();
    virtual void fn_vslot38(int a);
    virtual void fn_vslot39();
    virtual void fn_vslot40(int a);
    virtual void fn_vslot41();
    virtual void fn_vslot42();
    virtual void fn_vslot43();
    virtual void fn_vslot44();
    virtual void fn_vslot45();
    virtual void fn_vslot46();
    virtual void fn_vslot47();
    virtual void fn_vslot48();
    virtual void fn_vslot49();
    virtual void fn_vslot50();
    virtual void fn_vslot51();
    virtual void fn_vslot52();
    virtual void fn_vslot53();
    virtual void fn_vslot54();
    virtual void fn_vslot55();
    virtual void fn_vslot56();
    virtual void fn_vslot57();
    virtual void fn_vslot58();
    virtual void fn_vslot59();
    virtual int fn_vslot60();
    virtual int fn_vslot61();
    virtual void fn_vslot62();
    virtual void fn_vslot63();
    virtual void fn_vslot64();
    virtual void fn_vslot65();
    virtual void fn_vslot66();
    virtual void fn_vslot67();
    virtual void fn_vslot68();
    virtual void fn_vslot69();
    virtual void fn_vslot70();
    virtual void fn_vslot71();
    virtual void fn_vslot72();
    virtual Unk800C7B40_Widget* fn_vslot73(int a, int b);
    virtual void fn_vslot74();
    virtual void fn_vslot75();
    virtual void fn_vslot76();
    virtual void fn_vslot77();
    virtual void fn_vslot78();
    virtual void fn_vslot79();
    virtual void fn_vslot80();
    virtual void fn_vslot81();
    virtual void fn_vslot82();
    virtual void fn_vslot83();
    virtual void fn_vslot84();
    virtual const char* fn_vslot85();
    virtual void fn_vslot86();
    virtual void fn_vslot87();
    virtual void fn_vslot88();
    virtual void fn_vslot89();
    virtual void fn_vslot90();
    virtual void fn_vslot91();
    virtual void fn_vslot92();
    virtual void fn_vslot93();
    virtual void fn_vslot94();
    virtual void fn_vslot95();
    virtual void fn_vslot96();
    virtual void fn_vslot97();
    virtual void fn_vslot98();
    virtual void fn_vslot99();
    virtual void fn_vslot100();
    virtual void fn_vslot101();
    virtual void fn_vslot102();
    virtual void fn_vslot103();
    virtual void fn_vslot104();
    virtual void fn_vslot105();
    virtual void fn_vslot106();
    virtual void fn_vslot107(int a);
    virtual void fn_vslot108();
    virtual void fn_vslot109();
    virtual void fn_vslot110();
    virtual void fn_vslot111();
    virtual void fn_vslot112();
    virtual void fn_vslot113();
    virtual void fn_vslot114();
    virtual void fn_vslot115();
    virtual void fn_vslot116();
    virtual void fn_vslot117();
    virtual void fn_vslot118(int a);
    virtual void fn_vslot119(int a);
    virtual void fn_vslot120();
    virtual void fn_vslot121();
    virtual void fn_vslot122();
    virtual void fn_vslot123();
    virtual void fn_vslot124();
    virtual void fn_vslot125(int a, int b, int c, int d, int e);
    virtual void fn_vslot126();
    virtual void fn_vslot127();
};

/* Name/id selector widget with an optional owned name buffer (fn_800D3F0C). */
struct Unk800D3F0C_Obj : Unk800D3F0C_V {
    char* m40;
    int m44;
    int m48;
    char pad4C[0x18];
    int m64;
    int m68;
    char pad6C[0x10];
    int m7C;
};
extern "C" int fn_800D43F0(Unk800D3F0C_Obj* o, int v);

/* Raw view of the widget built by fn_800D028C (vptr lbl_80656F80): two flag bits at +0x40, colours at +0x44/+0x54. */
struct Unk800D028C_Raw : Unk800C7B40_Data {
    const void* m3C;
    unsigned b40_0 : 1;
    unsigned b40_1 : 1;
    unsigned rest40 : 30;
    Unk800D244C_Vec4 m44;
    Unk800D244C_Vec4 m54;
    int m64;
    int m68;
    float m6C;
};
extern const char lbl_80656F80[];
extern "C" void fn_800D03B4(Unk800D03B4_Obj* o);

/* Raw view of Unk800C7A4C_Group for destructors (vptr stored by hand). */
struct Unk800C6E34_Raw : Unk800C7B40_Data {
    const void* m3C;
    Unk800C7B40_Widget* m40;
    Unk800C7B40_Widget* m44;
    Unk800C7B40_Widget* m48;
    Unk800C7B40_Widget* m4C;
    Unk800C7B40_Widget* m50;
};

/* Message manager singleton (lbl_8061A32C, 0xD8 bytes, ctor fn_800B64BC) as used by the on-demand loader. */
struct Unk800B64BC_Mgr {
    char pad0[0xA0];
    char* mA0;
    char pad_A4[0x14];
    int mB8;
    char padBC[0xD8 - 0xBC];
};
struct Unk800C3FEC_Tmp {
    char pad0[0x2C];
    int m2C;
    int m30;
    int m34;
    int m38;
    int m3C;
    char pad40[0xC0 - 0x40];
};
void* fn_80356930(char* t);
/* Object whose vptr is at +0x98 (slot 11 fills a temp from the object). */
struct Unk800C3FEC_Data {
    char pad0[0x98];
};
struct Unk800C3FEC_V : Unk800C3FEC_Data {
    virtual void fn_vslot1();
    virtual void fn_vslot2();
    virtual void fn_vslot3();
    virtual void fn_vslot4();
    virtual void fn_vslot5();
    virtual void fn_vslot6();
    virtual void fn_vslot7();
    virtual void fn_vslot8();
    virtual void fn_vslot9();
    virtual void fn_vslot10();
    virtual void fn_vslot11(Unk800C3FEC_Tmp* t, int a);
};
struct Unk800C3FEC_Obj : Unk800C3FEC_V {
    char pad9C[0x668];
    int m704;
};

struct Unk800C77A0_Obj : Unk800C7B40_Widget {
    char pad40[0x14];
    int m54;
};

#endif
