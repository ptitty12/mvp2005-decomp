#ifndef GAME_TU_800D4D0C_H
#define GAME_TU_800D4D0C_H


/*
 * Types for game/tu_800D4D0C.cpp (reconstructed; nothing here comes from
 * original headers; names are placeholders: Unk<addr>, m<offset>, vslotN).
 *
 * Unk800D4D0C_D is a menu-widget-like object built from two polymorphic bases:
 *   Unk800D4D0C_A  (size 0xDC): a data-only part of 0x3C bytes (flags word at
 *                  +0x24), then the vptr at +0x3C (virtual N at vtable+8N,
 *                  function pointer at +8N+4) and 16-byte records at +0x8C..+0xCC.
 *   Unk800D4D0C_B  (at +0xDC, size 0x28): a list/count interface; its vptr is at
 *                  B+0x24 = +0x100 (fn_800D52E4 stores 0x8065A2B0 there and
 *                  0x8065A0A8 at +0x3C).
 * Evidence: fn_800D4D0C..fn_800D4DD8, fn_800D4E1C, fn_800D5694..fn_800D5D74
 * read these offsets and call these slots; fn_800D52E4 constructs the object;
 * m104 caches an index (-1), m108/m210 are two 66-entry int arrays.
 */

struct Unk800D4D0C_Q {
    int m0;
    int m4;
    int m8;
    int mC;
};

/* flag-bit setters of the data-only part (defined at 0x800C7E38..0x800C7EBC) */
struct Unk800D4D0C_AData {
    char m0[0x24];
    unsigned int m24;
    short m28;
    short m2A;
    short m2C;
    short m2E;
    char m30[0xC];
    void fn_800C7E38(int v);
    void fn_800C7E64(int v);
    void fn_800C7E90(int v);
    void fn_800C7EBC(int v);
};

class Unk800D4D0C_A : public Unk800D4D0C_AData {
public:
    virtual void vslot1();
    virtual void vslot2();
    virtual void vslot3();
    virtual void vslot4();
    virtual void vslot5();
    virtual void vslot6();
    virtual void vslot7();
    virtual void vslot8();
    virtual void vslot9();
    virtual void vslot10(int a);
    virtual void vslot11();
    virtual void vslot12();
    virtual void vslot13(int a);
    virtual void vslot14();
    virtual void vslot15();
    virtual void vslot16();
    virtual void vslot17();
    virtual void vslot18();
    virtual void vslot19();
    virtual void vslot20();
    virtual void vslot21();
    virtual void vslot22();
    virtual void vslot23();
    virtual void vslot24(int a);
    virtual int vslot25();
    virtual void vslot26();
    virtual void vslot27();
    virtual void vslot28();
    virtual Unk800D4D0C_A* vslot29();
    virtual void vslot30();
    virtual Unk800D4D0C_A* vslot31();
    virtual int vslot32();
    virtual void vslot33();
    virtual int vslot34();
    virtual void vslot35(Unk800D4D0C_Q q);
    virtual Unk800D4D0C_A* vslot36();
    virtual Unk800D4D0C_A* vslot37();
    virtual void vslot38();
    virtual void vslot39();
    virtual void vslot40();
    virtual void vslot41();
    virtual void vslot42();
    virtual void vslot43();
    virtual void vslot44();
    virtual void vslot45();
    virtual void vslot46();
    virtual void vslot47();
    virtual void vslot48();
    virtual void vslot49();
    virtual void vslot50();
    virtual void vslot51(int a, int b);
    virtual void vslot52();
    virtual void vslot53();
    virtual int vslot54(int a);
    virtual void vslot55();
    virtual void vslot56();
    virtual void vslot57();
    virtual void vslot58();
    virtual void vslot59(float f);
    virtual void vslot60();
    virtual void vslot61();
    virtual void vslot62();
    virtual void vslot63();
    virtual void vslot64();
};

class Unk800D4D0C_A2 : public Unk800D4D0C_A {
public:
    char m40[0x4C];
    Unk800D4D0C_Q m8C;
    Unk800D4D0C_Q m9C;
    Unk800D4D0C_Q mAC;
    Unk800D4D0C_Q mBC;
    Unk800D4D0C_Q mCC;
};

class Unk800D4D0C_B {
public:
    int bm0;
    int bm4;
    int bm8;
    int bmC;
    char bm10[0x14];
    void fn_800CFF48();
    void fn_800CFF7C();
    void fn_800CFFB4();
    virtual void bslot1();
    virtual void bslot2();
    virtual void bslot3(int a);
    virtual void bslot4();
    virtual int bslot5();
    virtual void bslot6(int a);
    virtual int bslot7();
    virtual void bslot8();
    virtual void bslot9();
    virtual void bslot10();
    virtual void bslot11();
    virtual void bslot12();
    virtual int bslot13();
    virtual void bslot14(int a);
    virtual void bslot15();
};

class Unk800D4D0C_D : public Unk800D4D0C_A2, public Unk800D4D0C_B {
public:
    int m104;
    int m108[66];
    int m210[66];
    int m318;
    float m31C;
    float m320;
    float m324;
    float m328;
    char m32C[0x40];
};

extern "C" int fn_800B548C(int a);
extern "C" int fn_800B5520(int a);
int fn_800E18B0(int a);
int fn_800E18F8(int a);
int fn_800E1930(int a);
int fn_800E1968(int a);
int fn_800E197C(int a);
extern "C" int fn_800C80D4();
void fn_800D1714();
extern "C" void fn_800C8070(Unk800D4D0C_D* t);
extern "C" void fn_802D7EE4();
void fn_800D1614(Unk800D4D0C_D* t);
void fn_800D689C(Unk800D4D0C_D* t);
void fn_800D1654(Unk800D4D0C_D* t);
void fn_800D1694(Unk800D4D0C_D* t);
void fn_8040B61C(char* p);
extern const float lbl_805BCA0C;
struct Unk8037C7FC {
    int fn_8037CE98(unsigned int hash, int a, int b);
    int fn_8037CF5C(unsigned int hash, const char* s);
};
Unk8037C7FC* fn_8037C7FC();
extern int lbl_80687C5C;
extern "C" void fn_800D4538(Unk800D4D0C_D* t, int a);
extern "C" void Unicode2Ascii(char* dst, const void* src);
int fn_80409DAC(const char* s);
void* fn_80090880(void* obj, int id);
extern void* lbl_8061977C;

/* Row table at 0x8061AC5C: 8-byte rows {id, flag bits}. */
struct Unk800D8304_Row {
    int m0;
    unsigned int m4;
};
extern Unk800D8304_Row lbl_8061AC5C[];

struct Unk800D697C {
    char m0[0xD4];
    Unk800D697C();
};
struct Unk800D7624 {
    char m0[0x184];
    Unk800D7624();
};
void* operator new(unsigned int size, int heap);
extern int lbl_806EF9B4;

extern const float lbl_805BCDE4;
extern const float lbl_805BCDE8;
extern const float lbl_805BCDF8;
extern const float lbl_805BCDFC;
extern const float lbl_805BCE00;
extern const float lbl_805BCE04;
extern const float lbl_805BCE08;
extern const float lbl_805BD03C;
extern const float lbl_805BD050;
extern const float lbl_805BD054;
extern const float lbl_805BD058;
extern const float lbl_805BD05C;
extern const float lbl_805BD060;
extern const float lbl_805BD070;
extern const float lbl_805BD074;
extern const float lbl_805BD078;
extern const char lbl_805BD024[];
extern int lbl_8061930C;
float fn_8007BB28(int a, int b, int c, int d);
extern "C" int fn_800768E0(int a, int b);
int fn_804089B4(char* dst, const char* fmt, ...);
float fn_800D7EC8(float a, float b);
float fn_800D7F48(int a, int b);
struct Unk800D7008_Obj {
    char m0[0x58];
    float m58;
    float m5C;
    float m60;
};

inline void* operator new(unsigned int, void* p) { return p; }
/* Chooser/object root (ctor 0x8037B3E8): its constructor and the spline
 * registration member (fn_800D69E8) are called here. The last parameter of
 * fn_8037B728 is really a const char*; `int` keeps the retail mangled name. */
class Unk8037B3E8 {
public:
    Unk8037B3E8(unsigned int hash);
    void fn_8037B728(const char* name, const char* name2, void* list, int flags);
};
extern "C" void fn_8033CCE0(void* p);
extern char lbl_806557A8[];
extern char lbl_8065BDF8[];
extern char lbl_806578D8[];
struct Unk800D697C_Obj {
    char m0[0x60];
    float m60;
    char m64[0xC8 - 0x64];
    int mC8;
    int mCC;
    float mD0;
};
struct UiScrRaw2;
struct UiScrRaw2 {
    char pad0[0x3C];
    const void* m3C;
    UiScrRaw2* fn_800D0D9C(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l);
};
extern const char lbl_805BC1A8[];
extern const char lbl_805BC9DC[];
extern const char lbl_805BD02C[];
extern const char lbl_805BD034[];
extern const char lbl_805BD07C[];
extern "C" char* fn_804031BC(char* dst, const char* src);
int fn_80403DE4(char* dst, const char* fmt, ...);
#endif
