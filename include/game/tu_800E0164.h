#ifndef GAME_TU_800E0164_H
#define GAME_TU_800E0164_H

#define GAME_LBL_8061930C_INT
#include "game/tu_800EBF2C.h"

/*
 * Declarations for game/tu_800E0164.cpp (reconstructed; placeholders only).
 */

extern int lbl_8061930C;
extern int lbl_8061E978;
extern int lbl_805C2F00[];
extern int lbl_805C30F0[];
extern int lbl_805C32E0[];
extern int lbl_805C34D0[];
extern int lbl_805C36C0[];
extern int lbl_805C38B0[];
extern int lbl_805C3990[];
extern int lbl_805C3A10[];
extern int lbl_805C3A88[];
extern int lbl_805C3B00[];
extern int lbl_805C3B78[];
extern int lbl_805C3B80[];
extern const float lbl_805BDBB8;
extern const float lbl_805BDBBC;
extern char lbl_805BA54C[];

int fn_80075D80(int a);
extern "C" int fn_800B5520(int a);
int fn_800E1B68(int a);
int fn_800E18F8(int a);
int fn_800E1930(int a);
int fn_800E1968(int a);
int fn_800E1990(int a);
void fn_8007DA58(int o, int a);
int fn_80075C64(int o, int a);
void fn_8007DA8C(int o, int a, int b);
int fn_802B041C(int a);
void fn_800E2D8C(char* p);
int fn_800E2B84();

extern float lbl_805BDBB0;
extern float lbl_805BDBB4;
extern char lbl_8068B4C0[];
extern "C" float fn_8033CDC4(char* t, float f);
extern int lbl_805C3BC0[];
void fn_800E2A58(int a);
void fn_800E2C80(char* p, int a);
extern char lbl_805BA54C[];
int fn_80084114(int a);
int fn_80083F6C(int a);
int fn_800E190C(int x);
struct Ui800B555C_B {
    char pad0[0x4C4];
    int m4C4;
    int m4C8;
    int m4CC;
    int m4D0;
};
void fn_8007DB74(Unk8007B2B0_Screen* s, int a);
void fn_800E2F24(void* t, char* e, int r);
void fn_800E31A8(void* t, char* e, int r);
extern "C" void fn_800E3D4C(void* t, int r);
extern "C" int fn_80097210(char* e, int a);
extern int lbl_8061977C;
extern "C" int fn_80403144(unsigned short* dst, const unsigned short* src);
extern "C" void fn_8040321C(unsigned short* dst, unsigned short* fmt, unsigned short* a, unsigned short* b);
unsigned short* fn_80090880(int obj, int id);
int fn_802B9804(int a, int b);
extern "C" int fn_803FC4C0();
extern "C" void fn_80149F70(int p, int a);
extern "C" void fn_8014A0EC();
extern "C" void fn_803FED48(int a, int b, int c, int* d);
extern "C" char* fn_803FC548(int p, int i, char* buf);
void fn_802B0638(int g, char* s, int a);
void fn_802B0140(int g, char* s, int a, int b, int c);
extern "C" int fn_803FEDB8(int a, int b);
extern "C" void fn_80377488(int p);
int fn_8040B574(const char* a, const char* b);
extern char lbl_805BA560[];
int fn_80125490(int o, int a, int b, int c, int d);
extern "C" int fn_801254EC(int o, int a, int b, char* p, int c);
extern "C" void fn_80125C84(int o, int a);
extern "C" void fn_80125CF8(int o);
int fn_8007CF44(Unk8007B2B0_Screen* s, unsigned h, int a, int b);
extern "C" int fn_800DC0F4(char* arr, int idx, int b);
void fn_800DC3D8(int* a, int* b, int c);
extern "C" void fn_800E0C38(UiS* self, int a, int b, int c);
extern "C" void fn_800E1388(UiS* self, int a, int b, int c);
extern char lbl_805BDBF4[];
extern char lbl_805BDC04[];
extern char lbl_805BB0FC[];
extern float lbl_805BDD94;
extern float lbl_805BDD98;
void fn_802B91C0();
void fn_802B9218();
void fn_802B03F4(int g);
void fn_802B0388(int g, char* s, int a);
extern "C" void DrawLoadingProgress(int g, int a, float f);
#endif
