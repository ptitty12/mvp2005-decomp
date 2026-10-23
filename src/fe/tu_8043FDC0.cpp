/*
 * Front-end overlay code, .frontend_codeoverlay 0x8043FDC0-0x804439E4 (code-only unit;
 * placeholder TU). Define every function FE_OVERLAY (game/overlay.h); data extern by lbl_ name.
 */
#include "game/tu_8043FDC0.h"

inline void FeTunableSet(FeTunable* t, float v)
{
    if (t->m48 == 0)
        t->m4C = v;
    else
        *t->m48 = v;
}

extern "C" {
void fn_8032F574(void* it, int a);
float fn_80551984(int id, int k);
float fn_805519CC(int k);
float fn_8055248C();
}
int fn_8007F84C(Unk8007F198_B* s);


struct FeItemB {
    float m0;
    float m4;
    const void* m8;
    int mC;
};
extern "C" {
void fn_8032D868(void* self, const char* name);
void fn_8033FBD8(void* self, void* cb);
FeItem* fn_8032EC38(void* mem, const char* name, int a, int b, int c, int d);
FeItem* fn_8032F428(void* mem, const char* name, int* val, int a, int b);
}
extern char lbl_80666A18[];
extern const char lbl_80666EC0[];
extern const char lbl_80666E70[];
extern const char lbl_80666DD0[];
extern const char lbl_80667238[];
extern const char lbl_806671E8[];
extern const char lbl_805F4C70[];
extern const char lbl_805F4C7C[];
extern const char lbl_805F4C90[];
extern int lbl_806215F4;
extern void* lbl_806215CC;
extern void* lbl_806215D0;

inline void FeLabelInit(FeItemB* it, const char* name)
{
    it->m8 = lbl_80666EC0;
    it->m0 = 0.0f;
    it->m4 = 0.0f;
    it->m8 = lbl_80666E70;
    it->mC = 0;
    fn_8032D868(it, name);
}

inline void FeButtonInit(FeItemB* it, const char* name, void* cb)
{
    FeLabelInit(it, name);
    it->m8 = lbl_80666DD0;
    fn_8033FBD8(it, cb);
}

inline void FeMenuSimInit(FeMenuSim* p, int a)
{
    p->m174 = 0;
    p->m178 = 0;
    p->m1FC = 0;
    p->m200 = a;
    p->m2DC = lbl_80666A18;
    fn_803E4A50((char*)p + 0x210);
    p->m2D8 = 0;
}

extern "C" int sprintf(char* s, const char* fmt, ...);
extern "C" unsigned int strlen(const char* s);
struct FeD3 {
    unsigned char a;
    unsigned char b;
    unsigned short c;
};
extern "C" void fn_80172118__Fv(const char* name);
extern "C" void fn_8017211C__Fv(const char* name, const char* data, int n);
extern "C" void Unicode2Ascii(char* dst, const void* src);

struct FeArrL {
    char* m0;
    int m4;
    int m8;
    int mC;
    const void* m10;
    int m14;
};
extern const char lbl_8064B298[];
extern const char lbl_8064A550[];
extern const char lbl_8064A2A8[];
extern const char lbl_8064A4C8[];
int fn_8007F29C(Unk8007F198_A* s, int a, int b);
extern "C" {
void fn_8040074C(void* p, int n);
void fn_805371C4(FeArrL* l, int a);
void fn_80535764(FeArrL* l, int a);
}
inline int IntAt(char* b, int o) { return *(int*)(b + o); }

extern "C" FE_OVERLAY void fn_8043FDC0(void)
{
}

extern "C" FE_OVERLAY void fn_8043FDC4(void)
{
}

extern "C" FE_OVERLAY void fn_8043FDC8(void)
{
}

extern "C" FE_OVERLAY void fn_8043FDCC(void)
{
}

extern "C" FE_OVERLAY void fn_8043FDD0(void)
{
    int i = lbl_8061930C->fn_8007DA28();
    if (i != -1) {
        lbl_8061930C->fn_8007D180(i, (int)FeTunableGet(&lbl_8061B9A0->m1DE0));
    }
}

extern "C" FE_OVERLAY void fn_8043FE44(void)
{
    fn_800A3DF4(lbl_80619948);
}

extern "C" FE_OVERLAY void fn_8043FE6C(void)
{
    unsigned n = fn_800734D4(lbl_8061930C, 0x100);
    void* a = fn_803774A8(n, 0x100);
    void* b = fn_803774A8(n, 0x100);
    Fe803567AC sp8;
    fn_80072E90(lbl_8061930C, a, n, 0x100, 0, &sp8);
    int len = fn_80401610(a, n, b, 1);
    fn_803FBFE0("rost.bin", b, len);
    fn_80377408(a);
    fn_80377408(b);
}

extern "C" FE_OVERLAY void fn_8043FF1C(void)
{
}

extern "C" FE_OVERLAY void fn_8043FF20(void)
{
}

/* near-miss: retail counts n down with subic./bgt and bumps i separately; every for/while/do-while spelling tried (7 slots off at best) keeps one induction variable and compares twice */
extern "C" FE_OVERLAY void fn_80440210(void)
{
    int i;
    int n = lbl_80621608;
    for (i = 0; i < n; i++) {
        fn_8043FF24(i + 1);
    }
}

extern "C" FE_OVERLAY void fn_8044025C(void)
{
    unsigned n = fn_800734D4(lbl_8061930C, 0x100);
    void* a = fn_803774A8(n, 0x100);
    Fe803567AC sp8;
    fn_80072E90(lbl_8061930C, a, n, 0x100, 0, &sp8);
    fn_803FBFE0("urost.bin", a, n);
}

extern "C" FE_OVERLAY void fn_804402D8(void)
{
}

extern "C" FE_OVERLAY void fn_804402DC(void)
{
    fn_800B8AF0((char*)UiGetMsgMgr() + 0x38);
    fn_80331294((char*)lbl_8061B998 + 0x24);
}

extern "C" FE_OVERLAY void fn_80440340(void)
{
    fn_8005B9D4(fn_8005ACE4(), 0);
}

extern "C" FE_OVERLAY void fn_80440368(void)
{
    UiMsg m;
    UiMsg* p = &m;
    fn_8035682C(p);
    fn_8040B830(p->name, "BACK", 15);
    p->name[15] = 0;
    fn_800B7B4C(UiGetMsgMgr(), &m, 0);
}

extern "C" FE_OVERLAY void fn_804403E8(void)
{
    FeArrL l1;
    int b1[0x82];
    FeArrL l2;
    int b2[0x28];
    FeArrL* s1 = &l1;
    FeArrL* s2 = &l2;
    unsigned i;
    unsigned j;
    s1->m10 = lbl_8064B298;
    l1.m0 = (char*)b1;
    s1->m8 = 0x82;
    Unk8007F198_A* g = (Unk8007F198_A*)lbl_8061930C;
    fn_8040074C(l1.m0, 0x208);
    s1->m4 = 0;
    s1->mC = 0;
    s1->m10 = lbl_8064A550;
    fn_8040074C(l1.m0, s1->m8 * 4);
    s1->m4 = 0;
    s1->mC = 0;
    s1->m10 = lbl_8064A2A8;
    s1->m14 = 0;
    l2.m0 = (char*)b2;
    l2.m10 = lbl_8064B298;
    s2->m8 = 0x28;
    fn_8040074C(l2.m0, l2.m8 * 4);
    s2->m4 = 0;
    s2->mC = 0;
    l2.m10 = lbl_8064A550;
    fn_8040074C(l2.m0, l2.m8 * 4);
    s2->m4 = 0;
    s2->mC = 0;
    s2->m14 = 0;
    l2.m10 = lbl_8064A4C8;
    fn_805371C4(s1, 0x1FFFFFF);
    i = 0;
    for (; i < (unsigned)s1->m4; i++) {
        int id = IntAt(l1.m0, i * 4);
        fn_80535764(s2, id);
        for (j = 0; j < (unsigned)l2.m4; j++)
            fn_8007F29C(g, id, IntAt(l2.m0, j * 4));
    }
    l2.m10 = lbl_8064A550;
    l1.m10 = lbl_8064A550;
}

extern "C" FE_OVERLAY void fn_80442364(void* self, int a)
{
    fn_80330BC8(self, a);
}

/* near-miss: i and y swap callee-saved registers (r30/r31) in every spelling tried (14 slots) */
extern "C" FE_OVERLAY void fn_80442384(void* self, int a)
{
    char buf[0x68];
    int y;
    int i;
    fn_80330F98(self, a);
    y = 0x6E;
    i = 0;
    do {
        i++;
        fn_804089B4(buf, "Cat%d: %d", i, 0);
        fn_80314688(lbl_8061FEDC, 0x96, y, buf, 1, 0x16);
        y += 0x14;
    } while (i <= 7);
    y = 0x6E;
    if (i <= 0xF) {
        do {
            i++;
            fn_804089B4(buf, "Cat%d: %d", i, 0);
            fn_80314688(lbl_8061FEDC, 0x1B8, y, buf, 1, 0x16);
            y += 0x14;
        } while (i <= 0xF);
    }
    fn_804089B4(buf, "Total Emails: %d", 0);
    fn_80314688(lbl_8061FEDC, 0xF0, 0x15E, buf, 1, 0x16);
}

extern "C" FE_OVERLAY void fn_80442488(FeMenuEmail* self)
{
    int z = 0;
    self->m204 = z;
    self->m208 = z;
    self->items[self->m174]->s3();
    self->m174 = z;
    self->items[z]->s2();
    FeItemSet(&self->m2E0, lbl_80622DC8);
    FeItemSet(&self->m334, lbl_80622DC4);
}

extern "C" FE_OVERLAY void fn_80442548(FeMenuEmail* self)
{
    lbl_80622DC8 = FeItemGet(&self->m2E0);
    lbl_80622DC4 = FeItemGet(&self->m334);
}

extern char lbl_80666A18[];
extern char lbl_806672D8[];
extern "C" FE_OVERLAY FeMenuEmail* fn_80442594(FeMenuEmail* self, int a)
{
    self->m174 = 0;
    self->m178 = 0;
    self->m1FC = 0;
    self->m200 = a;
    self->m2DC = lbl_80666A18;
    fn_803E4A50((char*)self + 0x210);
    self->m2D8 = 0;
    self->m2DC = lbl_806672D8;
    fn_8032F120(&self->m2E0, "Increase Spam", 0);
    fn_8032F120(&self->m334, "Test Champion Mail", 0);
    fn_80330ACC(self, &self->m2E0);
    fn_80330ACC(self, &self->m334);
    return self;
}

extern "C" FE_OVERLAY void fn_80442640(FeMenuF* self)
{
    int z = 0;
    self->m204 = z;
    self->m208 = z;
    self->items[self->m174]->s3();
    self->m174 = z;
    self->items[z]->s2();
    self->m730 = z;
    fn_8032F574(&self->m474, 0);
    self->m734 = (void*)1;
    FeItemSet(&self->m664, 1);
    FeTunableSet(&self->m4D4, fn_80551984((int)FeItemObj(&self->m474), 0));
    FeTunableSet(&self->m538, fn_80551984((int)FeItemObj(&self->m474), 1));
    FeTunableSet(&self->m59C, fn_80551984((int)FeItemObj(&self->m474), 2));
    FeTunableSet(&self->m600, fn_80551984((int)FeItemObj(&self->m474), 3));
    FeTunableSet(&self->m6CC, fn_805519CC((int)self->m734 - 1));
    if (fn_8007F84C((Unk8007F198_B*)lbl_8061930C) != 0) {
        fn_8007F7D0((Unk8007F198_B*)lbl_8061930C, 0);
        FeTunableSet(&self->m410, fn_8055248C());
    }
}

extern "C" FE_OVERLAY void fn_80442840(FeMenuF* self)
{
    fn_805519A0((int)FeItemObj(&self->m474), 0, FeTunableGet(&self->m4D4));
    fn_805519A0((int)FeItemObj(&self->m474), 1, FeTunableGet(&self->m538));
    fn_805519A0((int)FeItemObj(&self->m474), 2, FeTunableGet(&self->m59C));
    fn_805519A0((int)FeItemObj(&self->m474), 3, FeTunableGet(&self->m600));
    fn_805519E0((int)self->m734, FeTunableGet(&self->m6CC));
}

extern void* lbl_806215DC;
extern int lbl_806215E0;
extern "C" void fn_80377488(void* p);
/* The count is reset even when the allocation already exists. */
extern "C" FE_OVERLAY void fn_80442930(void)
{
    if (lbl_806215DC == 0) {
        lbl_806215DC = fn_803774A8(40000, 0x100);
    }
    lbl_806215E0 = 0;
}

extern "C" FE_OVERLAY void fn_80442984(void)
{
    char out[0x40];
    char str[0x80];
    FeD3 d;
    int i;
    const char* hd = "Date,MLB Chemistry,MLB Win%,AAA Chemistry,AAA Win%,AA Chemistry,AA Win%,Grade
";
    fn_80172118__Fv("HappyTrack.csv");
    i = 0;
    fn_8017211C__Fv("HappyTrack.csv", "Date,MLB Chemistry,MLB Win%,AAA Chemistry,AAA Win%,AA Chemistry,AA Win%,Grade
", strlen("Date,MLB Chemistry,MLB Win%,AAA Chemistry,AAA Win%,AA Chemistry,AA Win%,Grade
"));
    for (; i < lbl_806215E0; i++) {
        Unicode2Ascii(str, (void*)((FeRec28*)(i * 0x28 + (int)lbl_806215DC))->m24);
        fn_80329FA0(&d, *(int*)(i * 0x28 + (int)lbl_806215DC));
        FeRec28* e = (FeRec28*)(i * 0x28 + (int)lbl_806215DC);
        unsigned short y = (unsigned)d.c % 100;
        sprintf(out, "%02d/%02d/%02d,%3.3f,%1.3f,%3.3f,%1.3f,%3.3f,%1.3f,%s
", d.b, d.a, y, e->m4, e->m14, e->m8, e->m18, e->mC, e->m1C, str);
        fn_8017211C__Fv("HappyTrack.csv", out, strlen(out));
    }
}

extern "C" FE_OVERLAY void fn_80442D48(int a, float f1, float f2, float f3, float f4, float f5, float f6, float f7, float f8)
{
    int g = fn_800A2AE8(lbl_80619948);
    int base = (int)lbl_806215DC;
    if (base != 0) {
        int i = lbl_806215E0;
        int off = i * 0x28;
        FeRec28* e = (FeRec28*)(off + base);
        if (*(int*)(off + base) != g) {
            e->m4 = f1;
            e->m8 = f2;
            e->mC = f3;
            e->m10 = f4;
            e->m14 = f5;
            e->m18 = f6;
            e->m1C = f7;
            e->m20 = f8;
            e->m24 = a;
            *(int*)(off + base) = g;
            lbl_806215E0 = i + 1;
            if (i + 1 > 1000) {
                fn_80442984();
                lbl_806215E0 = 0;
            }
        }
    }
}

extern "C" FE_OVERLAY void fn_80442E2C(void)
{
    if (lbl_806215DC != 0) {
        fn_80377488(lbl_806215DC);
        lbl_806215DC = 0;
    }
}

extern "C" void fn_8032D880(void* item, const char* text);
extern const char lbl_805F4B54[], lbl_805F4B70[], lbl_805F4B88[], lbl_805F4BA0[];
extern "C" FE_OVERLAY void fn_80442E6C(FeMenuF* self, int action)
{
    float value;
    fn_80330BC8(self, action);
    if ((unsigned)(action - 3) <= 1) {
        FeItem* selector = &self->m474;
        FeItem* current = self->items[self->m174];
        if (current == selector) {
            FeTunable* p0 = &self->m4D4;
            fn_805519A0(self->m730, 0, FeTunableGet(p0));
            FeTunable* p1 = &self->m538;
            fn_805519A0(self->m730, 1, FeTunableGet(p1));
            FeTunable* p2 = &self->m59C;
            fn_805519A0(self->m730, 2, FeTunableGet(p2));
            FeTunable* p3 = &self->m600;
            fn_805519A0(self->m730, 3, FeTunableGet(p3));
            self->m730 = selector->m4C;
            {
                value = fn_80551984(self->m730, 0);
                float* slot = p0->m48;
                if (slot == 0) p0->m4C = value;
                else *slot = value;
            }
            {
                value = fn_80551984(self->m730, 1);
                float* slot = p1->m48;
                if (slot == 0) p1->m4C = value;
                else *slot = value;
            }
            {
                value = fn_80551984(self->m730, 2);
                float* slot = p2->m48;
                if (slot == 0) p2->m4C = value;
                else *slot = value;
            }
            {
                value = fn_80551984(self->m730, 3);
                float* slot = p3->m48;
                if (slot == 0) p3->m4C = value;
                else *slot = value;
            }
        } else {
          FeItem* category = &self->m664;
          if (current == category && (int)self->m734 > 0 && (int)self->m734 <= 6) {
            FeTunable* rate = &self->m6CC;
            fn_805519E0((int)self->m734 - 1, FeTunableGet(rate));
            self->m734 = (void*)FeItemGetB(category);
            value = fn_805519CC((int)self->m734 - 1);
            float* slot = rate->m48;
            if (slot == 0)
                rate->m4C = value;
            else
                *slot = value;
        } else if (current == (FeItem*)&self->m4D4 || current == (FeItem*)&self->m538 ||
                   current == (FeItem*)&self->m59C || current == (FeItem*)&self->m600) {
            fn_805519A0(self->m730, 0, FeTunableGet(&self->m4D4));
            fn_805519A0(self->m730, 1, FeTunableGet(&self->m538));
            fn_805519A0(self->m730, 2, FeTunableGet(&self->m59C));
            fn_805519A0(self->m730, 3, FeTunableGet(&self->m600));
          }
        }
    }
    if (fn_8007F84C((Unk8007F198_B*)lbl_8061930C)) {
        FeTunable* live = &self->m410;
        fn_8007F7D0((Unk8007F198_B*)lbl_8061930C, 0);
        value = fn_8055248C();
        float* slot = live->m48;
        if (slot == 0)
            live->m4C = value;
        else
            *slot = value;
    }
    if (lbl_806215DC == 0) {
        fn_8032D880((char*)self + 0x2E0, lbl_805F4B54);
        fn_8032D880((char*)self + 0x32C, lbl_805F4B70);
        fn_8032D880((char*)self + 0x3C4, lbl_805F4B70);
        return;
    }
    fn_8032D880((char*)self + 0x2E0, lbl_805F4B70);
    fn_8032D880((char*)self + 0x32C, lbl_805F4B88);
    fn_8032D880((char*)self + 0x3C4, lbl_805F4BA0);
}

extern const char lbl_805F4B54[], lbl_805F4B88[], lbl_805F4BA0[], lbl_805F4BC0[], lbl_805F4BC8[], lbl_805F4BDC[];
extern const char lbl_805F4BF4[], lbl_805F4C00[], lbl_805F4C0C[], lbl_805F4C14[], lbl_805F4C20[], lbl_805F4C30[];
extern char lbl_80667288[];
extern int lbl_806215E4;
extern "C" void fn_80442AA0(void);
extern "C" void* fn_8032E70C(void*, const char*, float, float, float, float);
extern "C" FE_OVERLAY FeMenuF* fn_80443238(FeMenuF* self, int arg)
{
    FeMenuSimInit((FeMenuSim*)self, arg);
    ((FeMenuSim*)self)->m2DC = lbl_80667288;
    FeItemB* b0 = (FeItemB*)((char*)self + 0x2E0);
    FeItemB* b1 = (FeItemB*)((char*)self + 0x32C);
    FeItemB* b2 = (FeItemB*)((char*)self + 0x378);
    FeItemB* b3 = (FeItemB*)((char*)self + 0x3C4);
    FeButtonInit(b0, lbl_805F4B54, (void*)fn_80442930);
    FeButtonInit(b1, lbl_805F4B88, (void*)fn_80442984);
    FeButtonInit(b2, lbl_805F4BC8, (void*)fn_80442AA0);
    FeButtonInit(b3, lbl_805F4BA0, (void*)fn_80442E2C);
    fn_8032E70C(&self->m410, lbl_805F4BDC, 0.0f, 0.0f, 0.0f, 100.0f);
    fn_8032F428(&self->m474, lbl_805F4BF4, &lbl_806215E4, 4, 0);
    fn_8032E70C(&self->m4D4, lbl_805F4C00, 0.0f, 0.01f, 0.0f, 100.0f);
    fn_8032E70C(&self->m538, lbl_805F4BC0, 0.0f, 0.01f, 0.0f, 100.0f);
    fn_8032E70C(&self->m59C, lbl_805F4C0C, 0.0f, 0.01f, 0.0f, 100.0f);
    fn_8032E70C(&self->m600, lbl_805F4C14, 0.0f, 0.01f, 0.0f, 100.0f);
    fn_8032EC38(&self->m664, lbl_805F4C20, 0, 1, 1, 6);
    fn_8032E70C(&self->m6CC, lbl_805F4C30, 0.0f, 0.005f, 0.0f, 1.0f);
    self->m730 = 0;
    self->m734 = (void*)1;
    fn_80330ACC(self, (FeItem*)b2);
    fn_80330ACC(self, (FeItem*)b0);
    fn_80330ACC(self, (FeItem*)b1);
    fn_80330ACC(self, (FeItem*)b3);
    fn_80330ACC(self, (FeItem*)&self->m410);
    fn_80330ACC(self, &self->m474);
    fn_80330ACC(self, (FeItem*)&self->m4D4);
    fn_80330ACC(self, (FeItem*)&self->m538);
    fn_80330ACC(self, (FeItem*)&self->m59C);
    fn_80330ACC(self, (FeItem*)&self->m600);
    fn_80330ACC(self, &self->m664);
    fn_80330ACC(self, (FeItem*)&self->m6CC);
    return self;
}

extern "C" FE_OVERLAY void fn_80443598(FeMenuEmail* self)
{
    int z = 0;
    self->m204 = z;
    self->m208 = z;
    self->items[self->m174]->s3();
    self->m174 = z;
    self->items[z]->s2();
}

extern "C" FE_OVERLAY void fn_80443610(void)
{
}

extern "C" FE_OVERLAY void fn_80443614(FeMenuSim* self, int a)
{
    fn_80330BC8(self, a);
    if ((unsigned)(a - 3) <= 1) {
        FeItem* cur = self->items[self->m174];
        if (cur == &self->m32C) {
            lbl_80621608 = FeItemGetB(cur);
        } else if (cur == &self->m394) {
            lbl_80621604 = cur->m4C;
            if ((unsigned)lbl_80621604 > 3)
                lbl_80621604 = 0;
        }
    }
}

extern "C" FE_OVERLAY FeMenuSim* fn_804436B4(FeMenuSim* self, int a)
{
    FeMenuSimInit(self, a);
    self->m2DC = lbl_80667238;
    FeItemB* b = (FeItemB*)((char*)self + 0x2E0);
    FeButtonInit(b, lbl_805F4C70, (void*)fn_80440210);
    fn_8032EC38(&self->m32C, lbl_805F4C7C, 1, 1, 0, 0x64);
    fn_8032F428(&self->m394, lbl_805F4C90, &lbl_806215F4, 4, 0);
    fn_80330ACC(self, (FeItem*)b);
    lbl_806215CC = b;
    fn_80330ACC(self, &self->m32C);
    fn_80330ACC(self, &self->m394);
    return self;
}

extern "C" FE_OVERLAY void fn_804437EC(FeMenuEmail* self)
{
    int z = 0;
    self->m204 = z;
    self->m208 = z;
    self->items[self->m174]->s3();
    self->m174 = z;
    self->items[z]->s2();
}

extern "C" FE_OVERLAY void fn_80443864(void)
{
}

extern "C" FE_OVERLAY void fn_80443868(FeMenuSim* self, int a)
{
    fn_80330BC8(self, a);
    if ((unsigned)(a - 3) <= 1) {
        FeItem* cur = self->items[self->m174];
        if (cur == &self->m32C)
            lbl_80621608 = FeItemGetB(cur);
    }
}

extern "C" FE_OVERLAY FeMenuSim* fn_804438DC(FeMenuSim* self, int a)
{
    FeMenuSimInit(self, a);
    self->m2DC = lbl_806671E8;
    FeItemB* b = (FeItemB*)((char*)self + 0x2E0);
    FeButtonInit(b, lbl_805F4C70, (void*)fn_80440210);
    fn_8032EC38(&self->m32C, lbl_805F4C7C, 1, 1, 0, 0x64);
    fn_80330ACC(self, (FeItem*)b);
    lbl_806215D0 = b;
    fn_80330ACC(self, &self->m32C);
    return self;
}
