#ifndef GAME_TU_80052794_H
#define GAME_TU_80052794_H

#include "game/tu_800401E4.h"

/*
 * Shared placeholder types and callee prototypes for the code-only units
 * game/tu_80052794 .. game/tu_8005D268 (AI / event callbacks in the game TU
 * 0x8000F598..0x8006902C). Nothing here comes from original headers; layouts
 * contain only the fields the units actually touch.
 */

struct Unk80052794_Ctx;
struct Unk80052794_Obj;
struct Unk80052794_S130;
struct Unk80052794_S44C;
struct Unk80052794_S6CC;
struct Unk80052794_S6F8;
struct Unk80052794_S10;
struct Unk80052794_S700;
struct Unk80052794_S3B0;
struct Unk80052794_Entry;
struct Unk80052794_Row;
struct Unk80052794_Rec;
struct Unk80052794_Quad;
struct Unk80052794_Vec3;
struct Unk8005ADD0_Obj;
struct Unk8005BA1C_Obj;
struct Unk80056970_Obj;
struct Unk80056970_Entry;
struct Unk80057204_Mgr;
struct Unk8061B8C0_A;
struct Unk800B555C_Obj;

struct Unk80052794_B3B0 {
    char pad0[0x98];
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
};

/* Element of the 0x48-byte records at Ctx+4. Evidence: fn_800527DC (m38), fn_80052FE0 (m4), fn_800533A0 (m0, m4, m8, mC), fn_80053754 (m10), fn_80054328 (m20). */
struct Unk80052794_Entry {
    union {
        int m0;
        Unk80052794_S3B0* m0p;
    };
    union {
        int m4;
        float m4f;
    };
    int m8;
    int mC;
    int m10;
    char pad14[0xC];
    int m20;
    char pad24[0x14];
    Unk80052794_S44C* m38;
    char pad3C[0xC];
};

/* Context passed as the first argument of the AI/event callbacks in 0x80052794..0x8005DF2C (placeholder). Evidence: fn_800527DC, fn_80052908, fn_800533A0. */
struct Unk80052794_Ctx {
    int m0;
    Unk80052794_Entry m4[33];
    char pad94C[0x8C];
    int m9D8;
    int m9DC;
    int m9E0;
    char pad9E4[0x20];
    int mA04;
    int mA08;
    char padA0C[0x2C];
    int mA38[6];
    int mA50;
    int mA54;
    char padA58[0x10];
    int mA68;
    int mA6C;
    char padA70[0x4];
    int mA74;
    char padA78[0x4];
    int mA7C;
    char padA80[0x4];
    int mA84;
    char padA88[0x8];
    int mA90;
    char padA94[0x8];
    int mA9C;
    char padAA0[0x34];
    int mAD4;
    int mAD8;
};

struct Unk80052794_S10 {
    char pad0[0x1C];
    int m1C;
};

/* One 0xE64-byte record per side at S6F8.m14 (two of them). Evidence: fn_80053220 and fn_80054684 sum the 0x80 bytes at +0x1B4. */
struct Unk80052794_Row {
    char pad0[0x17C];
    unsigned char m17C;
    char pad17D[0x37];
    unsigned char vals[0x80];
    char pad234[0xB40];
    int mD74;
    char padD78[0xEC];
};

struct Unk80052794_SC {
    char pad0[0xC];
    int mC;
    char pad10[0x18];
    int m28;
};

struct Unk80052794_S6F8 {
    char pad0[0xC];
    Unk80052794_SC* mC;
    Unk80052794_S10* m10;
    Unk80052794_Row* m14;
};

/* The 0x48-byte record that EntryOf points 4 bytes into (Ctx + idx*0x48): +4 is the kind, +0xC the payload header, +0x10.. the payload words. Evidence: fn_80055204, fn_80055D84, fn_80055F18. */
struct Unk80052794_Rec {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    int m18;
    int m1C;
    int m20;
    int m24;
    int m28;
    int m2C;
    int m30;
    int m34;
    int m38;
    char pad3C[0xC];
};

struct Unk80052794_Quad {
    unsigned m0;
    int m4;
    int m8;
    int mC;
};

struct Unk800B555C_Obj {
    char pad0[0x688];
    Unk80052794_Quad m688;
};

struct Unk80052794_Vec3 {
    float x;
    float y;
    float z;
};

struct Unk80052794_S6CC {
    char pad0[0x36C];
    int m36C;
    char pad370[0x60];
    Unk80052794_Vec3 m3D0;
    Unk80052794_Vec3 m3DC;
};

struct Unk80052794_S700 {
    char pad0[0x3A4];
    int m3A4;
    char pad3A8[0xC];
    int m3B4;
    int m3B8;
    int m3BC;
};

struct Unk80052794_S44C {
    char pad0[0x6CC];
    Unk80052794_S6CC* m6CC;
    char pad6D0[0x28];
    Unk80052794_S6F8* m6F8;
    char pad6FC[0x4];
    Unk80052794_S700* m700;
    char pad704[0x1C];
    int m720;
};

struct Unk80052794_S130 {
    char pad0[0x38C];
    int m38C;
    char pad390[0xBC];
    Unk80052794_S44C* m44C;
    char pad450[0x8];
    int m458;
    char pad45C[0x2A8];
    int m704;
    int m708;
};

/* Class with a vptr at +0x98 (fn_80052A34 and fn_800543B4 call virtual slots 24 and 4 through it). */
struct Unk80052794_S3B0 : Unk80052794_B3B0 {
    char pad9C[0x2EC];
    int m388;
    char pad38C[0xC0];
    int m44C;
    char pad450[0x10];
    int m460;
    char pad464[0x124];
    char m588;
    char pad589[0x7];
    unsigned m590;
    char pad594[0x170];
    int m704;
    int m708;
    char pad70C[0x2F8];
    int mA04;
    char padA08[0x4];
    char mA0C;
};

struct Unk80052794_Obj13C {
    int m0;
    char pad4[0x24];
};

struct Unk80052794_Obj48C {
    int m0;
    Unk80052794_S3B0* m4;
    char pad8[0x10];
};

/* The object (usually a player/fielder) passed as the third argument. Evidence: fn_800527DC, fn_80052908, fn_80053220. */
struct Unk80052794_Obj {
    unsigned char m0;
    char pad1[0x37];
    int m38;
    char pad3C[0x4];
    int m40;
    int m44;
    char pad48[0x4];
    int m4C;
    int m50;
    char pad54[0xB0];
    int m104;
    int m108;
    char pad10C[0x4];
    int m110;
    char pad114[0x4];
    int m118;
    char pad11C[0x4];
    int m120;
    char pad124[0xC];
    Unk80052794_S130* m130;
    char pad134[0x8];
    Unk80052794_Obj13C m13C[5];
    char pad204[0x70];
    int m274;
    int m278;
    int m27C;
    char pad280[0x40];
    int m2C0[2];
    char pad2C8[0x20];
    int m2E8[2];
    int m2F0[2];
    char pad2F8[0xB8];
    Unk80052794_S3B0* m3B0;
    int m3B4;
    char pad3B8[0xC];
    int m3C4;
    char pad3C8[0x4];
    int m3CC;
    int m3D0;
    char pad3D4[0x8];
    int m3DC;
    int m3E0;
    char pad3E4[0x4];
    int m3E8;
    char pad3EC[0xC];
    int m3F8;
    char pad3FC[0xC];
    unsigned m408;
    char pad40C[0x38];
    int m444;
    int m448;
    int m44C;
    char pad450[0x8];
    int m458;
    char pad45C[0x4];
    int m460;
    char pad464[0x28];
    Unk80052794_Obj48C m48C[3];
    int m4D4;
    int m4D8;
    char pad4DC[0x70];
    int m54C;
};

/* Address of the record's data at ctx + idx * 0x48 + 4 (retail forms ctx + idx*0x48 first, then +4). */
static inline Unk80052794_Entry* EntryOf(Unk80052794_Ctx* c, int i)
{
    return (Unk80052794_Entry*)((char*)c + i * 0x48 + 4);
}

/* Switch index type of fn_80059BDC (kind values 0x2A..0x5A). An enumeral switch index makes
 * g++ 2.95 balance the case tree by node count instead of its ASCII cost table, as retail does.
 * The negative enumerator keeps the type signed (cmpwi, not cmplwi). */
enum Unk80059BDC_Kind { Unk80059BDC_Kind_Neg = -1, Unk80059BDC_Kind_Lo = 0x2A, Unk80059BDC_Kind_Hi = 0x5A };

/* Callee prototypes (guessed from call sites; most callees are not matched yet). */
extern const float lbl_805A92D4;
extern int* lbl_8061B9C4;
void fn_800527DC(Unk80052794_Ctx* ctx, int idx, Unk80052794_Obj* obj);
void fn_80052908(Unk80052794_Ctx* ctx, int idx, Unk80052794_Obj* obj);
extern "C" void fn_8004E68C(void);
void fn_80012648(void);
int fn_8001DB70(Unk80052794_Obj* obj);
void fn_80010290(void);
void fn_80010754(int a, int b);
void fn_80011090(int a);
int fn_80054D98(Unk80052794_Ctx* ctx);
int fn_8001D780(Unk80052794_Obj* obj);
int fn_8001DCF8(Unk80052794_Obj* obj);
int fn_8001DA9C(Unk80052794_Obj* obj);
int fn_8001D454(Unk80052794_Obj* obj);
int fn_8001DA4C(Unk80052794_Obj* obj);
void fn_80017978(int a, int b, int c);
void fn_80017594(int a, int b);
float fn_80253F54(void* p, int a, int b, int c);
int fn_80253F0C(void* p, int a, int b, int c);
extern "C" int fn_802F92CC(int a);
int fn_8040ABA0(void);
int fn_8006BE18(Unk80052794_S6F8* p);
extern "C" int fn_801BBF24(int a);
extern "C" int fn_801BBF30(int a, Unk80052794_S6CC* b, Unk80052794_Obj* c);
extern "C" int fn_801A49F0(void* a, int b, int c);
int fn_8005000C(Unk80052794_Ctx* ctx, int idx, int code);
int fn_8004FF78(Unk80052794_Ctx* ctx, int a, int b);
void fn_8004FC60(Unk80052794_Ctx* ctx, Unk80052794_Obj* obj);
void fn_80058FE4(int a, int b, Unk80052794_Obj* obj);
int fn_8001F3FC(Unk80052794_Obj* obj, int a);
extern "C" int fn_8001F45C(Unk80052794_Obj* obj, int a);
int fn_8001E31C(Unk80052794_Obj* obj);
int fn_800463E4(int a);
int fn_800465D4(int a, int b);
int fn_80046580(int a);
int fn_800465C0(int a);
int fn_80046658(int a);
int fn_800464C4(int a);
void fn_80044D3C(int a);
Unk800B555C_Obj* fn_800B555C(void);
extern "C" int fn_800B57EC(void);
void fn_80014A7C(int a, int b, int c, int d, int e, int f);
void fn_80017904(int a, int b, int c, int d);
void fn_800124C0(void);
void fn_8001422C(void);
void fn_80013140(void);
void fn_8001525C(void);
void fn_80015140(int a, int b);
void fn_800151A4(int a);
void fn_80015200(int a);
void fn_800158E4(void);
void fn_80017A50(void);
void fn_80015C84(int a);
void fn_80014DD0(void);
void fn_80015388(void);
void fn_80013DAC(void);
void fn_80013E08(void);
void fn_800186E8(void);
void fn_80015C30(void);
void fn_80015658(void);
void fn_800155FC(void);
void fn_800156AC(void);
void fn_800127E0(void);
void fn_80014514(void);

/* unit 0x8005371C */
extern Unk80056970_Obj* lbl_80687600;
extern Unk80056970_Obj* lbl_80687604;
extern Unk8061B8C0_A* lbl_8061B8C0;
extern const float lbl_805A92D8;
extern const float lbl_805A92F0;
extern int lbl_80617F5C;
void fn_8005371C(void);
int fn_8005481C(Unk80052794_Ctx* ctx, int idx);
int fn_80054750(Unk80052794_Ctx* ctx, int idx);
void fn_800149CC(void);
void fn_80014BFC(void);
Unk80056970_Entry* fn_80056E34(Unk80056970_Obj* r, int id);
int fn_8001DA28(Unk80052794_Obj* obj);
void fn_80013E5C(int a, int b);
void fn_80015B2C(int a);
int fn_8001D8DC(Unk80052794_Obj* obj);
int fn_8001D6CC(Unk80052794_Obj* obj);
void fn_800119F8(int a, int b, int c);
void fn_80018918(int a, int b);
void fn_8001897C(void);
void fn_80014978(void);
void fn_80012834(void);
int fn_80054DAC(Unk80052794_Ctx* ctx, Unk80052794_Obj* obj, int a);
int fn_80054DCC(Unk80052794_Ctx* ctx, Unk80052794_Obj* obj, int a);
void fn_80012418(void);
void fn_8000FA98(int a, int b);
void fn_80013724(int a);
void fn_8000F968(int a);
void fn_8017DD80(Unk80052794_S700* s, int k, void* a, void* b);
int fn_8017DD64(Unk80052794_S700* s, int k);
float fn_80253434(void* p);
void fn_800173E8(int a);
void fn_8001A1F4(void);
void fn_8001A398(void);
void fn_8001A050(void);
void fn_8001A0A4(void);
void fn_8001A14C(void);
void fn_8001A0F8(void);
void fn_8001A1A0(void);
void fn_8001A29C(void);
void fn_8001A248(void);
void fn_8001A2F0(void);
void fn_8001A344(void);
int fn_80043460(Unk80052794_Entry* e, int a);
int fn_8001D694(Unk80052794_Obj* obj);
int fn_8001D47C(Unk80052794_Obj* obj);
void fn_80013370(int a, int b, int c);
void fn_80043650(void);
void fn_800150E4(void);
extern "C" int fn_8009E1D4(void* p, int a, int b);
int fn_800461F8(int a, int b, int c);
void fn_800152B0(int a, int b, int c);
extern "C" int fn_8009DB0C(Unk80052794_Row* r, unsigned char b);
extern "C" int fn_8009DA60(Unk80052794_Row* r, unsigned char b);
int fn_8001DBE0(Unk80052794_Obj* obj);
void fn_80013674(void);
void fn_80014E24(void);
extern "C" int fn_8001F954(Unk80052794_Obj* obj);
int fn_80054574(Unk80052794_Ctx* ctx);

/* unit 0x80054874 */
void fn_8000FB90(void);
void fn_8001022C(int, int);
void fn_80010338(int, int, int, int, int, int);
void fn_80010D04(void);
void fn_80010D58(int, int, int);
void fn_80010DC4(int, int, int, int);
void fn_800111BC(int);
void fn_80011AC0(void);
void fn_80011B14(int, int, int);
void fn_80012118(int, int);
void fn_80012C4C(void);
void fn_80012CA0(int);
void fn_80012DA0(void);
void fn_80012DF4(void);
void fn_80012E48(int, int);
void fn_80012FC8(int);
void fn_800133DC(int, int, int, int);
void fn_800138C4(int, int, int, int);
void fn_80013938(int);
void fn_80013A84(int, int);
void fn_80013B3C(int, int, int, int, int);
void fn_80013C88(int);
void fn_80013D48(int, int);
void fn_80013EC0(int);
void fn_80013F1C(int, int);
void fn_80013F80(void);
void fn_80014030(int, int, int, int);
void fn_800141D8(void);
void fn_800142D4(void);
void fn_80014570(void);
void fn_800145CC(int);
void fn_800146F0(int, int, int);
void fn_800147D8(int);
void fn_80014A20(int);
void fn_80014B00(void);
void fn_80014E80(void);
void fn_80014F28(void);
void fn_8001502C(int, int);
void fn_80015090(void);
void fn_8001531C(int, int, int);
void fn_800153DC(void);
void fn_80015430(void);
void fn_800157C8(int, int);
void fn_80015D44(int);
void fn_80015DA0(int);
void fn_80015DFC(int);
void fn_80015E58(int);
void fn_80015EB4(int);
void fn_80015F10(int);
void fn_80015F6C(int, int);
void fn_80015FD0(int, int);
void fn_80016034(int);
void fn_800161E4(int, int, int);
void fn_80016250(int);
void fn_800162AC(int);
void fn_80016308(int);
void fn_80016668(int, int);
void fn_80016728(int);
void fn_80016784(int, int);
void fn_800167E8(int);
void fn_80016844(int, int);
void fn_800168A8(int, int);
void fn_8001690C(int, int);
void fn_80016DC0(int);
void fn_80016E1C(int);
void fn_80016E78(int);
void fn_80016ED4(int);
void fn_80017244(void);
void fn_80017298(void);
void fn_80017340(void);
void fn_80017444(void);
void fn_80017498(void);
void fn_800174EC(void);
void fn_80017540(void);
void fn_800175F8(void);
void fn_800176F4(void);
void fn_80017748(void);
void fn_800179E4(int, int, int);
void fn_80017F34(int, int, int, int, int, int, int, int);
void fn_80017FC8(int, int, int, int, int);
void fn_80018044(int, int, int, int, int, int, int);
void fn_800180D0(int, int, int, int, int, int, int, int);
void fn_8001817C(int, int, int, int, int);
void fn_80018A80(int);
void fn_80018BA4(int, int, int);
void fn_80018C10(void);
void fn_80018C64(int);
void fn_800190A4(int, int);
void fn_800193A4(int);
void fn_80019400(void);
void fn_80019454(void);
void fn_800194A8(void);
void fn_800194FC(void);
void fn_80019550(void);
void fn_8001A540(void);
void fn_8001A594(int);
void fn_8001A7BC(int a);
void fn_8001A818(int, int);
void fn_8001A87C(int, int, int, int);
void fn_8001A8F0(int, int);
void fn_8001A954(int, int, int, int);
int fn_8001D218(Unk80052794_Obj* obj);
extern "C" int fn_8001D280(Unk80052794_Obj* obj);
int fn_8001D42C(Unk80052794_Obj* obj);
int fn_8001D538(Unk80052794_Obj* obj);
int fn_8001D5D8(Unk80052794_Obj* obj);
int fn_8001D62C(Unk80052794_Obj* obj);
int fn_8001D75C(Unk80052794_Obj* obj);
int fn_8001D7D0(Unk80052794_Obj* obj);
int fn_8001DA7C(Unk80052794_Obj* obj);
int fn_8001DC58(Unk80052794_Obj*);
int fn_8001DC88(Unk80052794_Obj*);
int fn_8001DCCC(Unk80052794_Obj*);
int fn_8001DDD8(Unk80052794_Obj* obj);
int fn_8001DDFC(Unk80052794_Obj* obj);
int fn_8001DE9C(Unk80052794_Obj* obj);
int fn_8001E054(Unk80052794_Obj* obj);
int fn_8001E084(Unk80052794_Obj* obj);
int fn_8001E0C8(Unk80052794_Obj*);
int fn_8001E108(Unk80052794_Obj* obj);
int fn_8001E134(Unk80052794_Obj* obj);
int fn_8001E294(Unk80052794_Obj* obj);
int fn_8001E2EC(Unk80052794_Obj* obj);
int fn_8001E34C(Unk80052794_Obj* obj);
int fn_8001E3C8(Unk80052794_Obj* obj);
extern "C" int fn_8001E444(Unk80052794_Obj* obj);
extern "C" int fn_8001E4F4(Unk80052794_Obj* obj);
extern "C" int fn_8001E664(Unk80052794_Obj* obj);
extern "C" int fn_8001E810(Unk80052794_Obj* obj);
extern "C" int fn_8001EB78(Unk80052794_Obj* obj);
extern "C" int fn_8001EC80(Unk80052794_Obj* obj);
extern "C" int fn_8001ED28(Unk80052794_Obj* obj);
extern "C" int fn_8001ED80(Unk80052794_Obj* obj);
extern "C" int fn_8001EDBC(Unk80052794_Obj* obj);
extern "C" int fn_8001EDE4(Unk80052794_Obj* obj);
extern "C" int fn_8001EE28(Unk80052794_Obj* obj);
extern "C" int fn_8001EFB4(Unk80052794_Obj* obj);
extern "C" int fn_8001F030(Unk80052794_Obj* obj);
extern "C" int fn_8001F218(Unk80052794_Obj* obj);
extern "C" int fn_8001F2A0(Unk80052794_Obj* obj);
extern "C" int fn_8001F2C4(Unk80052794_Obj*);
extern "C" int fn_8001F300(Unk80052794_Obj*);
extern "C" int fn_8001F378(Unk80052794_Obj*);
extern "C" int fn_8001F3B8(Unk80052794_Obj*, void*);
extern "C" int fn_8001F4F0(Unk80052794_Obj* obj);
extern "C" int fn_8001F5CC(Unk80052794_Obj* obj);
extern "C" int fn_8001F614(Unk80052794_Obj* obj);
extern "C" int fn_8001F8B8(Unk80052794_Obj* obj);
extern "C" int fn_8001F914(Unk80052794_Obj*);
extern "C" int fn_8001F984(Unk80052794_Obj* obj);
extern "C" int fn_8001F9B4(Unk80052794_Obj* obj);
extern "C" int fn_8001FA20(Unk80052794_Obj* obj);
extern "C" int fn_8001FA58(Unk80052794_Obj* obj);
extern "C" int fn_8001FC60(Unk80052794_Obj* obj);
extern "C" int fn_8001FC90(Unk80052794_Obj* obj);
extern "C" int fn_8001FCC0(Unk80052794_Obj* obj);
extern "C" int fn_8001FCEC(Unk80052794_Obj* obj);
extern "C" int fn_8001FD18(Unk80052794_Obj* obj);
extern "C" int fn_8001FD44(Unk80052794_Obj* obj);
extern "C" int fn_8001FD74(Unk80052794_Obj* obj);
extern "C" int fn_8001FDA8(Unk80052794_Obj* obj);
extern "C" int fn_8001FDDC(Unk80052794_Obj* obj);
extern "C" int fn_8001FE14(Unk80052794_Obj* obj);
extern "C" int fn_8001FE1C(Unk80052794_Obj* obj);
extern "C" int fn_8001FE24(Unk80052794_Obj* obj);
extern "C" int fn_8001FE2C(Unk80052794_Obj* obj);
int fn_80042EF8(int);
void fn_80044FAC(int);
void fn_80046750(Unk80052794_Obj*);
void fn_80046760(Unk80052794_Obj*);
void fn_80046770(Unk80052794_Obj*);
void fn_80051060(void);
struct Unk80056970_Obj;

int fn_8006A1B4(Unk80052794_S6F8*);
int fn_80076644(int, int, int);
extern "C" Unk80052794_S3B0* fn_801A497C(Unk80052794_S44C*, int, int);
float fn_80254128(void*, int, int);
int fn_8025416C(void*, int);
void fn_8033E958(void*);


extern "C" void fn_803EE7BC(void);

/* 0x80056970 */
struct Unk80056970_Entry {
    float m0;
    int m4;
    int m8;
    Unk80056970_Entry(int x = -1);
};
struct Unk80056970_Obj {
    unsigned m0;
    unsigned m4;
    unsigned m8;
    Unk80056970_Entry* mC;
    void* m10;
};
/* Same layout as Unk80056970_Obj, but with the vptr at +0x10 (virtual dtor fn_80056A08). */
struct Unk80056A08_Ring {
    unsigned m0;
    unsigned m4;
    unsigned m8;
    Unk80056970_Entry* mC;
    virtual ~Unk80056A08_Ring();
};
class Unk80058060_Node {
public:
    virtual void v1();
    virtual void v2();
    virtual void v3(int a);
};
struct Unk80057204_Mgr {
    char pad0[0xB4];
    unsigned* mB4;
    char padB8[4];
    int* mBC;
    unsigned short mC0;
    char padC2[0xF4 - 0xC2];
    Unk80058060_Node** mF4;
    char padF8[8];
    int m100;
};
struct Unk8061B8C0_B {
    char pad0[0x4C];
    unsigned m4C;
};
struct Unk8061B8C0_A {
    char pad0[8];
    Unk8061B8C0_B* m8;
};
extern int lbl_80649C80;
void* operator new[](unsigned n, int heap);
void* operator new(unsigned int size, int heap);
Unk80056970_Obj* fn_80056970(Unk80056970_Obj* self, int n);
void fn_80056960(Unk80056970_Obj* self, int a);

extern const float lbl_805A9318;
void fn_80046750(void* p);
void fn_80046760(void* p);
void fn_80046770(void* p);
void fn_80055D84(Unk80052794_Rec* r);

/* unit 0x80056A08 */
extern const float lbl_805AB8B0;
extern const double lbl_805AB8A8;
extern const float lbl_805ADC18;
extern const char lbl_805ADA74[];
extern int lbl_80618CB0[];
extern int lbl_80618F6C;
extern int lbl_80618F70;
extern int lbl_80618F74;
extern int lbl_80618F78;
extern int lbl_80618F7C;
extern int lbl_80618F80;
void fn_8033E958(const char* fmt, ...);
void fn_80056A68(Unk80056970_Obj* r);
void fn_80056AC0(Unk80056970_Obj* r, int id, float v);
Unk80056970_Entry* fn_80056B30(Unk80056970_Obj* r);
Unk80056970_Entry* fn_80056B94(Unk80056970_Obj* r);
Unk80056970_Entry* fn_80056BE0(Unk80056970_Obj* r, unsigned idx);
Unk80056970_Entry* fn_80056C3C(Unk80056970_Obj* r, Unk80056970_Entry* e);
Unk80056970_Entry* fn_80056EE4(Unk80056970_Obj* r, int id, int* idxp);
void fn_80056D78(Unk80056970_Obj* r, Unk80056970_Entry** out, Unk80056970_Entry** last);
void* fn_800578E4(Unk80057204_Mgr* m, int a, int b);
void fn_80055204(Unk80052794_Rec* r, Unk80052794_Obj* obj);
void fn_80055F18(Unk80052794_Rec* r);

extern const float lbl_805AB8A4;

/* unit 0x8005D268: score functions (this, a, b, obj) -> float */
extern const float lbl_805ADCE0;
extern const float lbl_805ADCE4;
extern const float lbl_805ADCE8;
extern const float lbl_805ADCEC;
extern const float lbl_805ADCF0;
extern const float lbl_805ADCF4;
extern const float lbl_805ADCF8;
extern const float lbl_805ADCFC;
extern const float lbl_805ADD00;
extern const float lbl_805ADD04;
extern const float lbl_805ADD08;
extern const float lbl_805ADD0C;
extern const float lbl_805ADD10;
extern const float lbl_805ADD7C;
extern const float lbl_805ADD80;
extern const float lbl_805ADD84;
extern const float lbl_805ADD88;
extern const float lbl_805ADD8C;
extern const float lbl_805ADD90;
extern const float lbl_805ADD94;
extern const float lbl_805ADD98;
extern const float lbl_805ADD9C;
extern const float lbl_805ADDA0;
extern const float lbl_805ADDA4;
extern const float lbl_805ADDA8;
extern const float lbl_805ADDAC;
extern const float lbl_805ADDB0;
extern const float lbl_805ADDB4;
extern const float lbl_805ADDB8;
extern const float lbl_805ADDEC;
extern const float lbl_805ADDF0;
extern const float lbl_805ADDF4;
extern const float lbl_805ADDF8;
extern const float lbl_805ADDFC;
extern const float lbl_805ADE00;
extern const float lbl_805ADE18;
extern const float lbl_805ADE1C;
extern const float lbl_805ADE20;
extern const float lbl_805ADE24;
void* fn_80057074(Unk80057204_Mgr* m, int k);
void* fn_800570D8(Unk80057204_Mgr* m, int k);
unsigned fn_803440F0(void* p, int x);
unsigned fn_80344138(void* p, int x);
float fn_8005CE4C(Unk80057204_Mgr* self, float x);
float fn_8005D6A0(Unk80057204_Mgr* self, int a, int b, Unk80052794_Obj* obj);
float fn_8025454C(void* p, int idx);
int fn_80045F28(float x);
extern const float lbl_805ADD28;
extern const float lbl_805ADD2C;
extern const float lbl_805ADD30;
extern const float lbl_805ADD34;
extern const float lbl_805ADD48;
extern const float lbl_805ADD4C;
extern const float lbl_805ADD50;
extern const float lbl_805ADD54;
extern const float lbl_805ADD68;
extern const float lbl_805ADD6C;
extern const float lbl_805ADD70;
extern const float lbl_805ADD74;
extern const float lbl_805ADD78;

int fn_8005CDC4(Unk80057204_Mgr* self, Unk80052794_Obj* obj);
Unk80052794_S3B0* fn_801A4B7C(Unk80052794_S44C* a, int b, int c);
extern "C" float fn_8024F20C(void* p, int a, int b, int c, int d);
extern "C" float fn_8024F048(void* p, int a, int b, int c, int d);
int fn_8008838C(int p, int a, int b);
int fn_80088F84(int p, int i);
int fn_80088FFC(int p, int i);
int fn_80044D90(int a, unsigned char b);
void fn_8001CEA8(Unk80052794_Obj* obj, void* a, void* b);
extern const float lbl_805ADDBC;
extern const float lbl_805ADDC0;
extern const float lbl_805ADDC4;
extern const float lbl_805ADDD0;
extern const float lbl_805ADDD4;
extern const float lbl_805ADDD8;
extern const float lbl_805ADDE8;
extern const float lbl_805ADE04;
extern const float lbl_805ADE08;
extern const float lbl_805ADE0C;
extern const float lbl_805ADE10;
extern const float lbl_805ADE14;

/* Voice object stored in the slots: vptr at +0x68 (virtual dtor called through delete). */
class Unk8005BA1C_Voice {
public:
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    int m18;
    int m1C;
    int m20;
    int m24;
    int m28;
    int m2C;
    int m30;
    int m34;
    int m38;
    char pad3C[4];
    int m40;
    char pad44[4];
    int m48;
    char pad4C[4];
    char m50;
    char pad51[0x68 - 0x51];
    Unk8005BA1C_Voice(int unused, int vol, int a3, int a4, int a5, int a6, int a7);
    virtual ~Unk8005BA1C_Voice();
};
struct Unk8005BA1C_Slot {
    Unk8005BA1C_Voice* m0;
    int m4;
    int m8;
    int mC;
    int m10;
};
struct Unk8005BA1C_Obj {
    int m0;
    int m4;
    int m8;
    int mC[8];
    int m2C;
    Unk8005BA1C_Slot m30[4];
    void* m80;
    Unk8005BA1C_Obj();
    ~Unk8005BA1C_Obj();
};
/* unit 0x8005C514 */
struct Unk805B09F4_E {
    char pad0[0x7E];
    char m7E[0x46];
    int mC4;
    int mC8;
    char padCC[8];
    int mD4;
};
extern Unk805B09F4_E lbl_805B09F4[];
struct Unk805AE420_E {
    char pad0[0x18];
    int m18;
    int m1C;
    int m20;
    int m24;
};
struct Unk805AE420_Row {
    Unk805AE420_E e[4];
};
extern Unk805AE420_Row lbl_805AE420[];

int fn_8005A3E8(Unk8005BA1C_Voice* v, int ch, int x);
int fn_8005A780(Unk8005BA1C_Voice* v, int a);
void fn_8005A240(void* p);
extern "C" int SNDSTRM_overhead(int a, int b);
extern "C" int fn_80097520(Unk80052794_Row* r, int k);
extern "C" int fn_8001F984(Unk80052794_Obj* obj);
int fn_803440D0(void* p);
int fn_803440D8(void* p);
int fn_803440E0(void* p);
int fn_803440E8(void* p);
void* fn_80056FAC(Unk80057204_Mgr* m, int k);
void* fn_80057010(Unk80057204_Mgr* m, int k);
void* fn_8005713C(Unk80057204_Mgr* m, int k);
void* fn_800571A0(Unk80057204_Mgr* m, int k);
int fn_8005CDC4(Unk80057204_Mgr* self, Unk80052794_Obj* obj);
extern const float lbl_805ADC6C;
extern const float lbl_805ADC70;
extern const float lbl_805ADC74;
extern const float lbl_805ADC78;
extern const float lbl_805ADC7C;
extern const float lbl_805ADC80;
extern const float lbl_805ADC84;
extern const float lbl_805ADC88;
extern const float lbl_805ADC8C;
extern const float lbl_805ADC90;
extern const float lbl_805ADC94;
extern const float lbl_805ADC98;
extern const float lbl_805ADC9C;
extern const float lbl_805ADCA0;
extern const float lbl_805ADCA4;
extern const float lbl_805ADCA8;
extern const float lbl_805ADCAC;
extern const float lbl_805ADCB0;
extern const float lbl_805ADCB4;
extern const float lbl_805ADCB8;
extern const float lbl_805ADCBC;
extern const float lbl_805ADCC0;
extern const float lbl_805ADCC4;
extern const float lbl_805ADCC8;
extern const float lbl_805ADCCC;
extern const float lbl_805ADCD0;
extern const float lbl_805ADCD4;
extern const float lbl_805ADCD8;
extern const float lbl_805ADCDC;

int fn_8005A4F8(Unk8005BA1C_Voice* v, int ch, int a, int b);
/* 0x8005C97C: queue of 0x50-byte records (+0 key, +4 a 0x4C-byte block) */
struct Unk8005C97C_Sub {
    int w[19];
};
extern Unk8005C97C_Sub lbl_806875B4;
extern const float lbl_805ADC68;
extern int lbl_80649768;
struct Unk8005C97C_Elem {
    float m0;
    Unk8005C97C_Sub m4;
    Unk8005C97C_Elem()
    {
        m0 = lbl_805ADC68;
        m4 = lbl_806875B4;
    }
};
struct Unk8005C97C_Q {
    unsigned m0;
    unsigned m4;
    Unk8005C97C_Elem* m8;
    void* mC;
};
/* Same layout as Unk8005C97C_Q with the vptr at +0xC (virtual dtor fn_8005CA60). */
struct Unk8005CA60_Q {
    unsigned m0;
    unsigned m4;
    Unk8005C97C_Elem* m8;
    virtual ~Unk8005CA60_Q();
};
extern "C" void fn_803EE7BC(void);

/* unit 0x8005ADD0 */
struct Unk8005ADD0_Chan {
    int mState;
    int mHandle;
    int mMode;
};
class Unk803785E4_Base {
public:
    int m0;
    int m4;
    Unk803785E4_Base();
    virtual ~Unk803785E4_Base();
};
/* Mid-level class between Unk803785E4_Base and Unk8005ADD0_Obj: its inline dtor stores its own vtable (lbl_80649DB8) before calling fn_80378620 (evidence: the Unk8005ADD0_Obj dtor stores lbl_80649DB8 between the member dtor and fn_80378620; the ctor at 0x800234D8 stores it after fn_803785E4). */
class Unk803785E4_Mid : public Unk803785E4_Base {
public:
    Unk803785E4_Mid() {}
    virtual ~Unk803785E4_Mid() {}
};
/* The real 0x3C-byte slot class (ctor 0x800401E4); Unk800401E4_Sub was a placeholder for it. */
typedef Unk800401E4 Unk800401E4_Sub;
class Unk8005ADD0_Obj : public Unk803785E4_Mid {
public:
    Unk8005ADD0_Chan mC[9];
    int m78;
    int m7C;
    int m80[9];
    int mA4;
    int mA8;
    Unk800401E4_Sub mAC;
    int mE8[8];
    int m108;
    int m10C;
    Unk8005ADD0_Obj();
    virtual ~Unk8005ADD0_Obj();
};
extern int lbl_805B1DC0[];
extern Unk8005BA1C_Obj* lbl_80618F8C;
int fn_8005C344(Unk8005BA1C_Obj* o, int k);
void fn_8005B58C(Unk8005ADD0_Obj* o, int a);
void fn_8005AF00(Unk8005ADD0_Obj* o, int a);
void fn_8005B06C(Unk8005ADD0_Obj* o);
void fn_8005A814(Unk8005BA1C_Voice* v, int a);
Unk8005BA1C_Obj* fn_8005BA9C(void);

struct Unk805B10B4_Row {
    char pad0[0xA0];
    int mA0[4];
};
extern Unk805B10B4_Row lbl_805B10B4[];
int fn_8005A010(Unk8005BA1C_Voice* v, int a, int b);
void fn_8005A2E4(void* p, int a);

/* unit 0x80058060 */
extern const char lbl_80649A60_unused[];
extern const double lbl_805ADC50;
extern float lbl_805ADC60;
extern "C" void SNDSYS_entercritical(void);
extern "C" void SNDSYS_leavecritical(void);
extern "C" int SNDSTRM_status(int h, void* p);
extern "C" int SNDSTRM_requeststatus(int h, void* p);
int fn_803AB278(int h);
extern "C" void fn_803AAA3C(int h);
extern "C" int SNDSTRM_destroy(int h);
extern "C" int SNDSTRM_setgreedylevel(int h, int v);
void* fn_8004072C(void);
extern "C" void fn_8004181C(void* self, int a, int b, int c, int d, int e, int f);
extern "C" int fn_80038EB4(void);
extern "C" void fn_800396D0(int a, int b);
extern "C" int fn_803927D0(int h);
int SNDplaysetdef(void* p, int a);
int fn_8005A19C(Unk8005BA1C_Voice* v);
int fn_8005A118(Unk8005BA1C_Voice* v, float f);
int fn_8005A5FC(Unk8005BA1C_Voice* v, int k);
int fn_8005A94C(Unk8005BA1C_Voice* v);
int fn_8005AB44(Unk8005BA1C_Voice* v);
int fn_8005AAA8(Unk8005BA1C_Voice* v);
void fn_8005A9A8(Unk8005BA1C_Voice* v);
void fn_8005AA00(Unk8005BA1C_Voice* v);
int fn_8005A8B4(Unk8005BA1C_Voice* v);
void fn_8005A814(Unk8005BA1C_Voice* v, int a);
int fn_8005A674(Unk8005BA1C_Voice* v, int a, int b);

extern "C" int fn_803AB9F0(int h, int a, float f);
extern "C" int SNDSTRM_autovol(int h, int a, int b);
struct Unk800475EC_E {
    int m0;
    char pad4[0x18];
};
struct Unk800475EC_T {
    char pad0[0x18];
    Unk800475EC_E e[8];
};
Unk800475EC_T* fn_800475EC(void);
extern "C" int fn_803AAA18(int h, int a, int name, int x);
int fn_800412AC(void* self, int a, int b, int c, int d, int e);
extern char lbl_805B0780[][0x42];

void fn_8005B0EC(Unk8005ADD0_Obj* o, int ch);
extern "C" void fn_8005B658(Unk8005ADD0_Obj* o);
extern Unk8005ADD0_Obj* lbl_80618F88;
Unk8005ADD0_Obj* fn_8005ACE4(void);
void fn_8005B1F0(Unk8005ADD0_Obj* o, int ch);
void fn_8005B2D8(Unk8005ADD0_Obj* o);
void fn_8005B4D0(Unk8005ADD0_Obj* o);
int fn_8005C7E8(Unk8005BA1C_Obj* b, int ch);
extern "C" int fn_803A914C(int h);
void fn_800403EC(void* p);

void fn_8005C75C(Unk8005BA1C_Obj* o, int a, int b);
int fn_8005C654(Unk8005BA1C_Obj* o, int a, int b, int c);
extern "C" int SNDstop(int h);
extern "C" void SNDvol(int h, int v);

void fn_8005AEB0(Unk8005ADD0_Obj* o, int ch);
/* fn_80053C88 */
extern "C" int fn_801DC67C(int a);
extern "C" int fn_801DC570(int a);
int fn_8026DC18(int a, int b);
int fn_8026D628(int a, int b);
int fn_8026DAE8(int a, int b);
void fn_8001922C(int a, int b, int c);
void fn_80013450(int a, int b);
void fn_800132A0(int a, int b);
int fn_800507D8(Unk80052794_Ctx* ctx, int idx, Unk80052794_Obj* obj);
void fn_80015590(int a, int b, int c);
void fn_80014488(int a, int b, int c, int d, int e, int f, int g);

int fn_8001D70C(Unk80052794_Obj* obj);
int fn_8001E054(Unk80052794_Obj* obj);

void fn_80017CD4(int a, int b);
/* fn_80053A68 */
extern const float lbl_805A92DC;
extern const float lbl_805A92E0;
extern const float lbl_805A92E4;
extern const float lbl_805A92E8;
extern const float lbl_805A92EC;
inline Unk80052794_Vec3 operator-(const Unk80052794_Vec3& a, const Unk80052794_Vec3& b)
{
    Unk80052794_Vec3 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}
int fn_80258C90(int a);
void fn_80189618(Unk80052794_Vec3* out, Unk80052794_S6CC* p, float f);
extern "C" float fn_804028E4(float x);
int fn_80044EC0(int a, int b, int c, int d, int e);
void fn_8000FE60(int a);
void fn_80016F30(void);
void fn_8000F598(int a);

struct Unk80052794_Payload;
struct Unk80052794_Payload {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    int m18;
    int m1C;
    int m20;
    int m24;
    int m28;
    int m2C;
};
extern const char lbl_805A931C[];
void fn_8033E958(const char* fmt, ...);
void fn_8005B414(Unk8005ADD0_Obj* o);
int fn_8005B6E4(Unk8005ADD0_Obj* o);
extern "C" int fn_803AA934(void* sub, int a, int b, int c, int d);
extern "C" int SNDAEMS_streaminit(int a, int b, int c);
extern int lbl_80620548;
int fn_8005A010(Unk8005BA1C_Voice* v, int a, int ch);
void fn_8005A240(Unk8005BA1C_Voice* v);

extern "C" void SPCH_Choose(int h);
extern "C" void SPCH_Play(int h);
struct Unk800580D4_Rec {
    float m0;
    Unk8005C97C_Sub m4;
};
extern const float lbl_805ADC2C;
Unk800580D4_Rec fn_800580D4(void* self, int kind);

void fn_80058060(Unk80057204_Mgr* m, int a);

#endif
