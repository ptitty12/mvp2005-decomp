#ifndef GAME_TU_800D833C_H
#define GAME_TU_800D833C_H

#include "game/tu_8007A4DC.h"
#include "game/tu_8023A6C8.h"

/*
 * Types and declarations for game/tu_800D833C.cpp (reconstructed; placeholders:
 * Unk<addr>, m<offset>).
 *
 * Unk800DB840_Slot: elements of a per-player array with a 0xE64-byte stride
 * (fn_800DB840..fn_800DC088 index it as o + idx, call fn_800968C0 on the slot
 * and pass it to the setter). When the array pointer is null the same setter
 * of the global object lbl_8061930C is used instead.
 */

struct Unk800D8304_Row2 {
    int m0;
    unsigned int m4;
};
extern Unk800D8304_Row2 lbl_8061AC5C[];

struct Unk800DB840_Slot {
    char m0[0xE64];
};

extern int lbl_8061930C;
extern int lbl_8061A328;
extern "C" int fn_800968C0(void* p, int b, int c);
int fn_8040ABA0();
extern "C" int fn_80552D98();
int fn_8007DA28(int o);
int fn_8007D098(int o, int a, int b);
extern "C" int fn_800B5520(int a);
extern "C" int fn_800B548C(int a);

void fn_80073710(int lbl, int a1, int a2, int a3);
extern "C" void fn_80097620(void* p, int a1, int a2, int a3);
void fn_80074754(int lbl, int a1, int a2, int a3);
extern "C" void fn_8009764C(void* p, int a1, int a2, int a3);
extern "C" void fn_800748D4(int lbl, int a1, int a2, int a3);
extern "C" void fn_80097678(void* p, int a1, int a2, int a3);
void fn_80075094(int lbl, int a1, int a2, int a3);
extern "C" void fn_80097A9C(void* p, int a1, int a2, int a3, int a4);
void fn_800739E0(int lbl, int a1, int a2, int a3);
extern "C" void fn_80097EA4(void* p, int a1, int a2, int a3, int a4);
extern "C" void fn_80073C10(int lbl, int a1, int a2, int a3);
extern "C" void fn_80097F68(void* p, int a1, int a2, int a3, int a4);
void fn_8007B054(int lbl, int a1, int a2);
extern "C" void fn_80099278(void* p, int a1, int a2);
void fn_8007B0B4(int lbl, int a1, int a2);
extern "C" void fn_800993C0(void* p, int a1, int a2);
void fn_8007AEBC(int lbl, int a1, int a2);
extern "C" void fn_8009908C(void* p, int a1, int a2);
void fn_8007AF1C(int lbl, int a1, int a2);
extern "C" void fn_800991AC(void* p, int a1, int a2);
void fn_8007ABFC(int lbl, int a1, int a2);
void fn_80089E48(void* p, int a1, int a2);
void fn_8007A768(int lbl, int a1, int a2);
void fn_8008D7B8(void* p, int a1, int a2);
void fn_8007A700(int lbl, int a1, int a2, int a3);
void fn_8008D738(void* p, int a1, int a2, int a3);

struct Unk800D876C_A {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
};
struct Unk800D878C_C {
    char m0[0x120];
    int m120;
};
struct Unk800D878C_B {
    char m0[0x10];
    Unk800D878C_C* m10;
    Unk800D878C_C* m14;
};
struct Unk800D878C_Root {
    int m0;
    Unk800D878C_B* m4;
    Unk800D878C_B* m8;
};
extern Unk800D878C_Root* lbl_806192E0;
struct Unk800DA12C_B {
    char m0[0x2C];
    int m2C;
};
struct Unk800DB5F4_S {
    char m0[0x558];
    int m558;
    int m55C;
};
extern int lbl_8061B2B4;
extern int lbl_80619948;
extern int lbl_8061ADFC;
extern int lbl_8061AE00;
extern const float lbl_805BD790;
extern const float lbl_805BD794;
extern const float lbl_805BD79C;
extern const float lbl_805BD488;
extern const float lbl_805BD490;
int fn_8007FC20(int o, int a, int b);
int fn_8007FD2C(int o, int a, int b, int c);
int fn_8007FC40(int o, int a, int b, unsigned char c, int d);
void fn_800DA544(Unk800D876C_A* t, int v);
extern "C" int fn_800A1C54(int a, int b, int c, int d, int e, int f);
Unk800DB5F4_S* fn_800B555C();
void fn_8007F858(int o);
void fn_8007F8E4(int o, int a);
extern "C" void fn_800766B8(int o, int a);
void fn_800DB7DC();
int fn_800D86E8(float f);
int fn_800D867C();
int fn_800D9A64(int a);
int fn_800D9B24(int a);
int fn_800D9B9C(int a);
int fn_800D9C18(int a);
int fn_800D9EBC(int a, int b);
int fn_800D9FF0(int a, int b);
int fn_800DA038(int a, int b);
int fn_800D9D44(int a, int b);
int fn_800DA12C(int a, Unk800DA12C_B* b);
int fn_800DA134();
int fn_800DA1AC(int a);
int fn_80075D80(int a);
int fn_80076058(int a, int b);

extern Unk8023BE30 lbl_8068CC18;
extern Unk8023BE30 lbl_8068CCA8;
int fn_80089AA0(char* p, int a, int b);
extern "C" int fn_8008CFD8(char* p, int a, int b);
extern "C" int fn_80099570(void* p, int a, int b, int c);
extern "C" void fn_800D8EE0(void* t, int a);
extern "C" void fn_800D9360(void* t, int a);
extern "C" int fn_800B59DC(Unk800DB5F4_S* p);
extern "C" int fn_800B5A70(Unk800DB5F4_S* p);
extern "C" int fn_800B5B40(Unk800DB5F4_S* p);
int fn_804089B4(char* dst, const char* fmt, ...);
extern "C" int fn_803FEBDC(char* s, int a, int b, int* c);
extern "C" int fn_803FEC9C(int a, int b);
extern char lbl_805BD4B4[];
extern char lbl_805BD4B8[];
extern int lbl_8061ADE4;
extern int lbl_8061ADE8;
extern int lbl_8062151C;
struct Unk80084910_N {
    int m0;
    int m4;
};
void fn_80086078(Unk80084910_N* n, char* s);
extern "C" int fn_803FEE14(char* s, int a);
int fn_802C3A2C(int a, int b);
extern char lbl_805B9A30[];
Unk80084910_N fn_800D9978(int a, int b);

/* fn_800DA3D8: lbl_8061B8C0->m8 is the game object; its m6F8->m14 is an array of 0xE64-byte slots
 * (m0 points at a byte pair, records of 0x34 bytes start at +0x3E8). */
struct Unk800DA3D8_Ev {
    char m0;
    unsigned char m1;
    unsigned char m2;
};
struct Unk800DA3D8_Slot {
    Unk800DA3D8_Ev* m0;
};
struct Unk800DA3D8_Tbl {
    char m0[0x14];
    char* m14;
};
struct Unk800DA3D8_Game {
    char m0[0x6F8];
    Unk800DA3D8_Tbl* m6F8;
    char m6FC[4];
    int m700;
};
struct Unk800DA3D8_Glob {
    char m0[8];
    Unk800DA3D8_Game* m8;
};
extern Unk800DA3D8_Glob* lbl_8061B8C0;
extern "C" int fn_801A49F0(void* g, int idx, int rec);
void fn_8017DE28(int a, int b);
void fn_800DA208(Unk800D876C_A* t, int a, int b);
void fn_800DA3D8(Unk800D876C_A* t, int idx);
void fn_801A23B8(int a, int b);
#endif
