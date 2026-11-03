/*
 * Game code, .text 0x80147748-0x801498C0 (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_80144CF0.h"

#include "game/chooser.h"
struct UnkAt3CV { char pad0[0x3C]; const void* m3C; };
struct UnkAt164 { char pad0[0x164]; int m164; };
struct UnkAt168 { char pad0[0x168]; int m168; };
struct UnkAt150 { char pad0[0x150]; int m150; };
struct Unk80147D4C_Obj { char pad0[0x3C]; const void* m3C; char pad40[0x13C - 0x40]; const void* m13C; };
struct Unk80147D4C_Mem { char pad0[8]; const void* m8; };
inline void* operator new(unsigned int, void* p) { return p; }
void* operator new(unsigned int size, int heap);
extern int lbl_806EF9B4;
struct Unk80148280_Obj {
    const void* m0;
    int m4;
    int m8[3];
    char pad14[0x28 - 0x14];
    int m28;
    int m2C;
    void* m30;
    void* m34;
};
void* operator new[](unsigned int size, int heap);
struct UnkAt0V { const void* m0; };
struct Unk8Arr { char pad0[8]; int m8[3]; };
struct Unk8ArrV { char pad0[8]; VT18* m8[3]; };
struct Unk20Pair { char pad0[0x20]; int m20; int m24; };
struct UnkB8_Obj { char pad0[0xB8]; int mB8; };
struct Unk2C_Obj { char pad0[0x2C]; int m2C; };
struct Unk8_Obj { char pad0[8]; int m8; };
struct UnkC_Obj { char pad0[0xC]; int mC; };
struct Unk4C_Obj { char pad0[0x4C]; int m4C; };

extern "C" {
int fn_80147748(VT3C* p, int a);
int fn_801477E4(VT3C* p, int a);
int fn_80147880(VT3C* p, int a);
int fn_8014791C(VT3C* p, int a);
int fn_801479B8(VT3C* p, int a);
int fn_80147A54(VT3C* p, int a);
/* near-miss: only the callee-saved register pair differs (retail p in r29, result in r30; we use r30/r29) */
int fn_80147AF0(void* p);
int fn_80147B2C();
void fn_80147B34(UnkAt3CV* p, int c);
void fn_80147B64(UnkAt3CV* p, int c);
void fn_80147B94(UnkAt3CV* p, int c);
int fn_80147BC4();
void fn_80147BCC(UnkAt3CV* p, int c);
void fn_80147BFC(UnkAt3CV* p, int c);
void fn_80147C2C(UnkAt3CV* p, int c);
void fn_80147C5C(UnkAt3CV* p, int c);
int fn_80147C8C();
void fn_80147C94(UnkAt3CV* p, int c);
void fn_80147CC4(UnkAt3CV* p, int c);
int fn_80147CF4();
void fn_80147CFC(UnkAt3CV* p, int c);
int fn_80147D2C(UnkAt164* p);
int fn_80147D34(UnkAt168* p);
void fn_80147D3C(UnkAt150* p, int v);
void fn_80147D44();
void fn_80147D48();
void fn_80147D4C(Unk80147D4C_Obj* p, int c);
void fn_80147DC4(UnkAt3CV* p, int c);
void fn_80147DF4(UnkAt3CV* p, int c);
void fn_80147E24(UnkAt3CV* p, int c);
void fn_80147E54();
void fn_80147E58();
int fn_80147E78();
char* fn_80147E84();
void fn_80147E90();
void* fn_80147EBC(void* p);
void* fn_8014824C();
void fn_8014827C();
void* fn_80148280(Unk80148280_Obj* p);
void fn_80148328(UnkAt0V* p, int c);
void fn_8014835C(Unk8Arr* p, int v);
void fn_80148394(Unk8ArrV* p);
int fn_80148404(Unk8Arr* p);
void fn_80148438(Unk8ArrV* p);
void fn_801484A8(int i);
void fn_801484E4(int i);
float fn_80148520(int i);
void fn_80148580();
void fn_80148604(Unk20Pair* p);
void fn_8014865C();
/* near-miss (62 vs 63 insns): retail keeps the constant 1 of the sp8 retry loop in r30 and the
   lbl_8061B8F4 high part in r31, both set in the entry block; plain nested do/while, for(;;),
   continue/break forms and an 'int one = 1' local all keep 'li r0,1' in the loop */
void fn_80148698(Unk2C_Obj* p);
void fn_80148790(void* self, Unk4C_Obj* e, int a, int* flagA, int* flagB);
void fn_8014887C(void* self, VT18D* e, int unused, int* flag);
int fn_80148930(Unk20Pair* p);
void fn_80148A40(void* p);
void fn_801498B8();
void fn_801498BC();
void fn_800F4A30(void*);
int fn_800EBDB0();
extern const char lbl_80653E60[];
void fn_804D2BCC(void*, int);
extern const char lbl_80653B90[];
extern const char lbl_806538C0[];
extern const char lbl_80651628[];
void fn_8035686C(void*, int);
extern const char lbl_80650368[];
extern const char lbl_806500C0[];
extern const char lbl_8064FB70[];
extern const char lbl_8064E388[];
extern const char lbl_8064E0E0[];
extern const char lbl_8064DB50[];
extern const char lbl_8064C820[];
extern const char lbl_8064C848[];
extern const char lbl_80649DB8[];
extern int lbl_8061B870;
void fn_80378620(void*, int);
extern const char lbl_8064BD58[];
extern const char lbl_8064BA88[];
extern const char lbl_8064B798[];
void fn_800EC1B8(void*, int);
extern char lbl_805C2824[];
void fn_801431A8(int, int);
extern const char lbl_8065D4F0[];
extern const char lbl_8065D4D0[];
extern int lbl_8061E978;
extern Unk80148280_Obj* lbl_8061B8C0;
void fn_80357A5C(void*);
void fn_80377408(void*);
extern int lbl_8068B5DC[];
int fn_8014849C();
extern int lbl_8068B5E8[];
extern float lbl_8068B5F4;
unsigned CPU_detect();
extern const float lbl_805C4C98;
extern UnkB8_Obj* lbl_8061A32C;
void fn_802D7ACC();
UnkB8_Obj* fn_800B64BC(void*);
extern char lbl_8068B5FC[];
void fn_80400828(void*);
void fn_8040084C(void*);
void fn_802D7588();
extern int lbl_8061B8C4;
extern int lbl_8061B8F4;
extern int lbl_8061B8F0;
extern Unk8_Obj* lbl_8068B618;
int fn_802D7A84(int*);
UnkC_Obj* fn_801A04A8(int);
void fn_802D75D4(int);
void fn_803289C0(int);
extern int lbl_8061B8F8;
extern int lbl_8061B8E8;
extern VT28* lbl_8061EDF8;
float fn_801A0744(void*);
int fn_801A0874(void*);
void fn_801A08D8(void*);
void fn_803289F0(int);
extern int lbl_8061B8EC;
extern const char lbl_805C4D84[];
extern const char lbl_805C4D98[];
extern int lbl_8061B900;
void fn_8033E958(const char*, ...);
void fn_8014A0EC();
int fn_804007D4(void*);
void fn_80148AB4(void*);
void fn_80400498(void (*)());
extern int lbl_8061B914;
extern int lbl_8061B91C;
extern float lbl_8061B918;
extern float lbl_8061B904;
extern float lbl_8061B908;
extern int lbl_8061B90C;
extern int lbl_8061B910;
extern int lbl_80616504;
extern const float lbl_805C4DA8;

int fn_80147748(VT3C* p, int a) { int r = p->s88() ? p->s84(a)->s56(a) : 0; fn_800F4A30(p); return r; }

int fn_801477E4(VT3C* p, int a) { int r = p->s88() ? p->s84(a)->s57(a) : 0; fn_800F4A30(p); return r; }

int fn_80147880(VT3C* p, int a) { int r = p->s88() ? p->s84(a)->s58(a) : 0; fn_800F4A30(p); return r; }

int fn_8014791C(VT3C* p, int a) { int r = p->s88() ? p->s84(a)->s59(a) : 0; fn_800F4A30(p); return r; }

int fn_801479B8(VT3C* p, int a) { int r = p->s88() ? p->s84(a)->s60(a) : 0; fn_800F4A30(p); return r; }

int fn_80147A54(VT3C* p, int a) { int r = p->s88() ? p->s84(a)->s61(a) : 0; fn_800F4A30(p); return r; }

/* near-miss: only the callee-saved register pair differs (retail p in r29, result in r30; we use r30/r29) */
int fn_80147AF0(void* p)
{
    int r = fn_800EBDB0();
    fn_800F4A30(p);
    return r;
}

int fn_80147B2C() { return 0x1; }

void fn_80147B34(UnkAt3CV* p, int c) { p->m3C = lbl_80653E60; fn_804D2BCC(p, c); }

void fn_80147B64(UnkAt3CV* p, int c) { p->m3C = lbl_80653B90; fn_804D2BCC(p, c); }

void fn_80147B94(UnkAt3CV* p, int c) { p->m3C = lbl_806538C0; fn_804D2BCC(p, c); }

int fn_80147BC4() { return 0x0; }

void fn_80147BCC(UnkAt3CV* p, int c) { p->m3C = lbl_80651628; fn_8035686C(p, c); }

void fn_80147BFC(UnkAt3CV* p, int c) { p->m3C = lbl_80650368; fn_8035686C(p, c); }

void fn_80147C2C(UnkAt3CV* p, int c) { p->m3C = lbl_806500C0; fn_8035686C(p, c); }

void fn_80147C5C(UnkAt3CV* p, int c) { p->m3C = lbl_8064FB70; fn_8035686C(p, c); }

int fn_80147C8C() { return 0x0; }

void fn_80147C94(UnkAt3CV* p, int c) { p->m3C = lbl_8064E388; fn_8035686C(p, c); }

void fn_80147CC4(UnkAt3CV* p, int c) { p->m3C = lbl_8064E0E0; fn_8035686C(p, c); }

int fn_80147CF4() { return 0x0; }

void fn_80147CFC(UnkAt3CV* p, int c) { p->m3C = lbl_8064DB50; fn_8035686C(p, c); }

int fn_80147D2C(UnkAt164* p) { return p->m164; }

int fn_80147D34(UnkAt168* p) { return p->m168; }

void fn_80147D3C(UnkAt150* p, int v) { p->m150 = v; }

void fn_80147D44() {}

void fn_80147D48() {}

void fn_80147D4C(Unk80147D4C_Obj* p, int c)
{
    p->m13C = lbl_8064C820;
    p->m3C = lbl_8064C848;
    lbl_8061B870 = 0;
    Unk80147D4C_Mem* m = (Unk80147D4C_Mem*)((char*)p + 0x134);
    m->m8 = lbl_80649DB8;
    fn_80378620(m, 0);
    fn_8035686C(p, c);
}

void fn_80147DC4(UnkAt3CV* p, int c) { p->m3C = lbl_8064BD58; fn_804D2BCC(p, c); }

void fn_80147DF4(UnkAt3CV* p, int c) { p->m3C = lbl_8064BA88; fn_804D2BCC(p, c); }

void fn_80147E24(UnkAt3CV* p, int c) { p->m3C = lbl_8064B798; fn_800EC1B8(p, c); }

void fn_80147E54() {}

void fn_80147E58() { fn_80147E84(); }

int fn_80147E78() { return 0x6DB1A80F; }

char* fn_80147E84() { return lbl_805C2824; }

void fn_80147E90() { fn_801431A8(0x1, 0xFFFF); }

void* fn_80147EBC(void* p)
{
    new (p) Unk8037B3E8(0xA22DD41B);
    *(const void**)p = lbl_8065D4F0;
    return p;
}

extern "C" {
void fn_8037B7E4(void* s, const char* name);
void fn_8037B7E8(void* s, const char* name);
void fn_8037B830(void* s, const char* name);
void fn_8037B50C(void* s, const char* name, void* field, const char* type, const char* def);
void fn_8037B458(void* s, const char* name, void* field, const char* type, const char* def);
int sprintf(char* dst, const char* fmt, ...);
}
extern const char lbl_805C4ACC[];
extern const char lbl_805C4AD0[];
extern const char lbl_805C4ADC[];
extern const char lbl_805C4AF0[];
extern const char lbl_805C4B00[];
extern const char lbl_805C4B10[];
extern const char lbl_805C4B18[];
extern const char lbl_805C4B28[];
extern const char lbl_805C4B38[];
extern const char lbl_805C4B40[];
extern const char lbl_805C4B4C[];
extern const char lbl_805C4B58[];
extern const char lbl_805C4B6C[];
extern const char lbl_805C4B70[];
extern const char lbl_805C4B88[];
extern const char lbl_805C4B8C[];
extern const char lbl_805C4BA0[];
extern const char lbl_805C4BAC[];
extern const char lbl_805C4BBC[];
extern const char lbl_805C4BC8[];
extern const char lbl_805C4BE4[];
extern const char lbl_805C4BEC[];
extern const char lbl_805C4BF8[];
extern const char lbl_805C4C0C[];
extern const char lbl_805C4C24[];
extern const char lbl_805C4C38[];
extern const char lbl_805C4C40[];
extern const char lbl_805C4C4C[];
extern const char lbl_805C4C58[];
extern const char lbl_805C4C64[];
extern const char lbl_805C4C6C[];
extern const char lbl_805C4C78[];
extern int lbl_8068B5D8;
extern int lbl_8061B898;
extern int lbl_8061B89C;
extern int lbl_8068BF30;
extern int lbl_8068B5D0;
extern int lbl_8061B8C8;
extern int lbl_806242A4;
extern int lbl_8061D2A8;
extern int lbl_8068B5C8;
extern int lbl_8068B5CC;
extern int lbl_8068B4C0;
extern float lbl_8068B5D4;
extern float lbl_8061CBC4;
extern float lbl_8061B888;
extern float lbl_8061D4B8;
extern float lbl_8061D4BC;
extern float lbl_8061D4C0;

extern "C" void fn_80147F00(Unk8037B3E8* s)
{
    char buf[0x18];
    fn_8037B7E4(s, lbl_805C4ACC);
    fn_8037B458(s, lbl_805C4AD0, &lbl_8068B5D8, 0, 0);
    fn_8037B458(s, lbl_805C4ADC, &lbl_8061B898, 0, 0);
    fn_8037B458(s, lbl_805C4AF0, &lbl_8061B89C, 0, 0);
    fn_8037B50C(s, lbl_805C4B00, &lbl_8068B5D4, lbl_805C4B10, 0);
    fn_8037B458(s, lbl_805C4B18, &lbl_8068BF30, 0, 0);
    fn_8037B458(s, lbl_805C4B28, &lbl_8068B5D0, 0, 0);
    fn_8037B7E8(s, lbl_805C4B38);
    fn_8037B50C(s, lbl_805C4B40, &lbl_8061CBC4, lbl_805C4B4C, 0);
    fn_8037B830(s, lbl_805C4B38);
    fn_8037B458(s, lbl_805C4B58, &lbl_8061B8C8, 0, 0);
    sprintf(buf, lbl_805C4B6C, lbl_806242A4);
    fn_8037B458(s, lbl_805C4B70, &lbl_806242A4, lbl_805C4B88, buf);
    sprintf(buf, lbl_805C4B6C, lbl_8061D2A8);
    fn_8037B458(s, lbl_805C4B8C, &lbl_8061D2A8, lbl_805C4B88, buf);
    fn_8037B7E8(s, lbl_805C4BA0);
    fn_8037B458(s, lbl_805C4BAC, &lbl_8068B5C8, 0, 0);
    fn_8037B458(s, lbl_805C4BBC, &lbl_8068B5CC, 0, 0);
    fn_8037B830(s, lbl_805C4BA0);
    fn_8037B50C(s, lbl_805C4BC8, &lbl_8061B888, lbl_805C4BE4, 0);
    s->fn_8037B728(lbl_805C4BEC, lbl_805C4BF8, &lbl_8068B4C0, (int)lbl_805C4C0C);
    fn_8037B7E8(s, lbl_805C4C24);
    sprintf(buf, lbl_805C4C38, lbl_8061D4B8);
    fn_8037B50C(s, lbl_805C4C40, &lbl_8061D4B8, lbl_805C4C4C, buf);
    sprintf(buf, lbl_805C4C38, lbl_8061D4BC);
    fn_8037B50C(s, lbl_805C4C58, &lbl_8061D4BC, lbl_805C4C4C, buf);
    sprintf(buf, lbl_805C4C64, lbl_8061D4C0);
    fn_8037B50C(s, lbl_805C4C6C, &lbl_8061D4C0, lbl_805C4C78, buf);
    fn_8037B830(s, lbl_805C4C24);
}

void* fn_8014824C() { return fn_80147EBC(operator new(0x4, lbl_806EF9B4)); }

void fn_8014827C() {}

void* fn_80148280(Unk80148280_Obj* p)
{
    fn_80357A5C(p);
    p->m0 = lbl_8065D4D0;
    p->m28 = 0;
    p->m2C = 0;
    p->m4 = lbl_8061E978;
    if (!lbl_8061B8C0) {
        lbl_8061B8C0 = p;
    }
    p->m30 = operator new[](0x1D4C, 0);
    p->m34 = operator new[](0x1D4C, 0);
    for (int i = 0; i < 3; i++) {
        p->m8[i] = 0;
    }
    return p;
}

void fn_80148328(UnkAt0V* p, int c) { p->m0 = lbl_8065D4D0; if (c & 1) fn_80377408(p); }

void fn_8014835C(Unk8Arr* p, int v)
{
    if (v) {
        for (int i = 0; i < 3; i++) {
            if (!p->m8[i]) {
                p->m8[i] = v;
                return;
            }
        }
    }
}

void fn_80148394(Unk8ArrV* p)
{
    int i;
    for (i = 0; i < 3; i++) {
        if (p->m8[i]) {
            delete p->m8[i];
            p->m8[i] = 0;
        }
    }
}

int fn_80148404(Unk8Arr* p)
{
    int n = 0;
    for (int i = 0; i < 3; i++) {
        if (p->m8[i]) {
            n++;
        }
    }
    return n;
}

void fn_80148438(Unk8ArrV* p)
{
    for (int i = 0; i < 3; i++) {
        if (p->m8[i]) {
            p->m8[i]->s18();
        }
    }
}

void fn_801484A8(int i) { lbl_8068B5DC[i] = fn_8014849C(); }

void fn_801484E4(int i) { lbl_8068B5E8[i] = fn_8014849C(); }

float fn_80148520(int i)
{
    float t = (float)(unsigned)(lbl_8068B5E8[i] - lbl_8068B5DC[i]);
    return t / lbl_8068B5F4;
}

void fn_80148580()
{
    int i = 0;
    float t = (float)CPU_detect();
    lbl_8068B5F4 = t * lbl_805C4C98;
    do {
        fn_801484A8(i);
        fn_801484E4(i);
        i++;
    } while (i <= 2);
}

void fn_80148604(Unk20Pair* p)
{
    p->m20 = 0;
    p->m24 = 0;
    fn_802D7ACC();
    if (!lbl_8061A32C) {
        lbl_8061A32C = fn_800B64BC(operator new(0xD8, 0));
    }
}

void fn_8014865C()
{
    fn_80400828(lbl_8068B5FC);
    fn_802D7588();
    fn_8040084C(lbl_8068B5FC);
}

/* near-miss (62 vs 63 insns): retail keeps the constant 1 of the sp8 retry loop in r30 and the
   lbl_8061B8F4 high part in r31, both set in the entry block; plain nested do/while, for(;;),
   continue/break forms and an 'int one = 1' local all keep 'li r0,1' in the loop */
void fn_80148698(Unk2C_Obj* p)
{
    int sp8;
    fn_80400828(lbl_8068B5FC);
    lbl_8061B8C4 += 1;
    if (lbl_8061B8F4 == 0) {
        do {
            do {
                sp8 = 1;
            } while (fn_802D7A84(&sp8) == 0);
        } while (sp8 <= 0);
        lbl_8061B8F4 = sp8;
    }
    int v = 0;
    lbl_8061B8F4 -= 1;
    if (lbl_8068B618->m8 != 0) {
        UnkC_Obj* o = fn_801A04A8(1);
        if (o) {
            v = o->mC;
        }
    }
    fn_802D75D4(v);
    fn_8040084C(lbl_8068B5FC);
    lbl_8061B8F0 += 1;
    fn_803289C0(0);
    if (p->m2C > 0) {
        p->m2C = p->m2C - 1;
    }
}

void fn_80148790(void* self, Unk4C_Obj* e, int a, int* flagA, int* flagB)
{
    float sp8;
    float spC;
    sp8 = fn_801A0744(e);
    if (fn_801A0874(e) == 0) {
        int t = *flagA;
        if (t == 0) {
            *flagB = 1;
            fn_801A08D8(e);
            lbl_8061EDF8->s4();
            lbl_8061B8F8 = 1;
            spC = sp8;
            fn_803289C0(a);
            lbl_8061B8F8 = t;
            sp8 = fn_801A0744(e);
            if (spC != sp8) {
                lbl_8061B8E8 += 1;
                *flagA = 1;
            }
            if ((e->m4C & 1) == 0) {
                fn_803289F0(a);
            }
        }
    }
}

void fn_8014887C(void* self, VT18D* e, int unused, int* flag)
{
    float sp8;
    if (e->m744->m20 == 0) {
        int v = 0;
        if (e->m780 != 0 || e->m784 != 0) {
            v = 1;
        }
        if (v == 0) {
            sp8 = fn_801A0744(e);
            e->s19();
            if (sp8 != fn_801A0744(e)) {
                lbl_8061B8EC += 1;
                *flag = 1;
            }
        }
    }
}

int fn_80148930(Unk20Pair* p)
{
    lbl_8061B8F0 = 0;
    lbl_8061B8C4 = 0;
    lbl_8061B8F4 = 0;
    fn_8033E958(lbl_805C4D84);
    if (!lbl_8061A32C) {
        lbl_8061A32C = fn_800B64BC(operator new(0xD8, 0));
    }
    if (lbl_8061A32C->mB8 == 0) {
        fn_8014A0EC();
    }
    if (lbl_8061B900 == 0) {
        fn_804007D4(lbl_8068B5FC);
        lbl_8061B900 = 1;
    }
    fn_80148604(p);
    fn_8033E958(lbl_805C4D98);
    fn_80148A40(p);
    while (p->m20 == 0) {
        fn_8014865C();
        fn_80148AB4(p);
    }
    fn_80400828(lbl_8068B5FC);
    fn_80400498(fn_8014865C);
    fn_8040084C(lbl_8068B5FC);
    return p->m20;
}

void fn_80148A40(void* p)
{
    lbl_8061B904 = lbl_805C4DA8;
    lbl_8061B908 = lbl_805C4DA8;
    lbl_8061B90C = 0;
    lbl_8061B910 = 0;
    lbl_8061B918 = lbl_805C4DA8;
    lbl_8061B91C = 1;
    lbl_8061B914 = 0;
    fn_80148580();
    lbl_80616504 = 7;
}

void fn_801498B8() {}

void fn_801498BC() {}

}
