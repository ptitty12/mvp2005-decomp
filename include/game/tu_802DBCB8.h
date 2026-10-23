#ifndef GAME_TU_802DBCB8_H
#define GAME_TU_802DBCB8_H

/*
 * Declarations for game/tu_802DBCB8.cpp and tu_802DE2C8.cpp (reconstructed,
 * placeholder names). The units are methods of the pad base class
 * Unk803414A4 (include/game/tu_80008540.h, vtable lbl_806648E8) and of its
 * 0x24-byte sub-object at +0x1558 (Unk802DC9F8).
 */
#include "game/tu_80008540.h"

extern "C" void* MEM_fill(void* dst, int val, unsigned int n);   /* memset */
extern "C" void fn_80377408(void* p);                                          /* __builtin_delete */
void* operator new[](unsigned int size, int heap);                  /* 0x803774A8 */
extern int lbl_806EFA04;
/* 12-byte record of the buffer at +0x10 of Unk802DC9F8 (frame, pad index, value, float). */
struct Unk802DD0BC_Rec {
    int m0;
    short m4;
    short m6;
    float m8;
};

extern "C" int fn_802DC9C0();
extern "C" {
int fn_803FEBDC(int name, int mode, int bufsize, int* handle);   /* open */
int fn_803FEC4C(int h, int pos, char* buf, int n, int mode);     /* read */
int fn_803FEC74(int h, int pos, char* buf, int n, int mode);      /* write */
int fn_803FEC9C(int h, int mode);                                /* close */
int fn_804089B4(char* dst, const char* fmt, ...);                 /* sprintf */
int fn_8040B6A0(const char* s);                                   /* strlen */
}
int fn_80409DAC(const char* s);
int fn_802DCCCC(int h, int* pos, int* out);

#endif
