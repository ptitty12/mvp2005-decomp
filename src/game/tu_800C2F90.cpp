/*
 * Game code, .text 0x800C2F90-0x800C5158 (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_800BCD38.h"
#include "game/tu_800BCD38_decls.h"

extern Unk8007B2B0_Screen* lbl_8061930C;

void fn_800DC3D8(int* a, int* b, int c);

extern "C" {
int fn_800DC1B8(int a, int b, int c, int d, int e);
void fn_800C42FC(Unk800C341C_Mgr* m, int w, int a);
int fn_800DBAE0(int a, int b, int c, int d);
void fn_800C422C(Unk800C341C_Mgr* m, int w, int a, int b, int c, int d);
void fn_800C429C(Unk800C341C_Mgr* m, int w, int a);
}

extern "C" {
void fn_800C40B4(Unk800C341C_Mgr* m, int w, int c, int obj);
}

struct Unk8024EF40_W;
int fn_8025416C(Unk8024EF40_W* p, int a);
extern "C" void fn_800C3ED4(Unk800C341C_Mgr* m, int w, int obj);

extern "C" {
int sprintf(char* dst, const char* fmt, ...);
}

extern const float lbl_805BBE14;
struct Unk805BBE18_F { float v; };
extern Unk805BBE18_F lbl_805BBE18;
extern const float lbl_805BBE28;
extern const float lbl_805BBE34;
extern const float lbl_805BBE40;
extern const float lbl_805BBE54;
extern const char lbl_805BBE1C[];
extern const char lbl_805BBE2C[];
extern const char lbl_805BBE38[];
extern const char lbl_805BBE4C[];
extern char lbl_805B6F58;
extern const float lbl_805BBF48;
extern const float lbl_805BBEA8;
extern const float lbl_805BBEC4;
extern const char lbl_805BBF40[];
extern const char lbl_805BBEA0[];
extern const char lbl_805BBEBC[];
struct Unk800C493C_Letters {
    char c[6];
};
struct Unk800C49C4_Letters {
    char c[13];
};
extern const Unk800C493C_Letters lbl_805BBE98 __attribute__((aligned(4)));
extern const Unk800C49C4_Letters lbl_805BBEAC __attribute__((aligned(4)));
extern "C" int strcmp(const char* a, const char* b);
extern "C" void DCStoreRange(void* a, int b);
extern "C" int fn_800C5CF8(Unk800C341C_Mgr* m, const char* name);
extern "C" void* fn_803774A8(unsigned n, int heap);
extern "C" void fn_80377488(void* p);
extern "C" int fn_800C519C(Unk800C341C_Mgr* mgr, int n);
extern const float lbl_805BBE44;
extern const float lbl_805BBE48;
extern const char lbl_805BBE6C[];
extern const char lbl_805BBD3C[];
extern const char lbl_805BB0E4[];
extern Unk800C341C_Mgr* lbl_8061ABDC;
extern int lbl_8061A708[];
extern "C" {
int fn_800C2FE8(Unk800C2250_File* f);
void ASYNCFILE_release(int a, int b, int c);
int fn_80400CE4(int a);
void ASYNCFILE_cancel(int a);
int ASYNCFILE_getstatus(int a);
void THREAD_yield(int a);
}

struct Unk802ACD88_Mgr;
void fn_802B0638(Unk802ACD88_Mgr* obj, const char* name, int a);

extern void* lbl_8061E978;
struct Unk800C4C48_Tbl {
    int v[16];
};
extern const Unk800C4C48_Tbl lbl_805BBEF4 __attribute__((aligned(4)));
extern const char lbl_805BBF34[];
extern const float lbl_805BBF3C;
extern const float lbl_805BBE74;
extern const float lbl_805BBE78;
extern const float lbl_805BBE7C;
extern const float lbl_805BBE80;
extern const float lbl_805BBE94;
extern const float lbl_805BBED0;
extern const float lbl_805BBEDC;
extern const float lbl_805BBEE8;
extern const float lbl_805BBEEC;
extern const float lbl_805BBEF0;
extern const char lbl_805BBE84[];
extern const char lbl_805BBE8C[];
extern const char lbl_805BBEC8[];
extern const char lbl_805BBED4[];
extern const char lbl_805BBEE0[];
extern const char* lbl_8061A864[];
extern const char* lbl_8061AA40[];
extern const char* lbl_8061AB78[];
extern const char* lbl_8061AB3C[];

extern "C" int fn_800C2F90(Unk800C2250_File* f, int i)
{
    if (f->m40C[i] != 0)
        return 1;
    return 0;
}

extern "C" Unk800C1BE4_Widget* fn_800C2FB0(Unk800C2250_File* f, int i)
{
    return f->m40C[i];
}

extern "C" Unk800C1BE4_Widget* fn_800C2FC0(Unk800C2250_File* f, int i)
{
    return f->m40C[i];
}

extern "C" float fn_800C2FD0(Unk800C2250_File* f, int i)
{
    return f->m430[i];
}

extern "C" void* fn_800C2FE0(Unk800C2250_File* f)
{
    return f->m45C;
}

extern "C" int fn_800C2FE8(Unk800C2250_File* f)
{
    return f->m458;
}

extern "C" void fn_800C2FF0(Unk800C2250_File* f)
{
    f->m0 = lbl_805B6F58;
    f->m400 = 0;
    f->m404 = 0;
    f->m408 = 0;
    for (int i = 0; i < 9; i++) {
        f->m40C[i] = 0;
        f->m430[i] = lbl_805BBE14;
    }
    f->m454 = 0;
    f->m458 = 0;
    f->m45C = 0;
    f->m460 = 0;
    f->m464 = 0;
}

struct Unk802ACD88_Mgr;
void fn_802B058C(Unk802ACD88_Mgr* obj, const char* name, void* a);
int fn_802B94EC(int path, int a, int b, int c);
int fn_802B96A4(int a, int b, int c);
extern "C" {
void fn_800C2A1C(Unk800C2250_File* f, Unk800C1BE4_Widget* w, float t);
char* strcpy(char* dst, const char* src);
void fn_800C313C(Unk800C2250_File* f);
}

extern "C" void fn_800C3054(Unk800C2250_File* f, const char* name, Unk800C1BE4_Widget* w, int c, float t)
{
    char buf[0x400];
    strcpy(&f->m0, name);
    sprintf(buf, lbl_805BBD3C, name, lbl_805BB0E4);
    if (c != 0) {
        f->m454 = 1;
        f->m458 = fn_802B96A4((int)buf, (int)f->m45C, f->m460);
        f->m404 = 3;
        fn_800C2A1C(f, w, t);
    } else {
        fn_802B94EC((int)buf, (int)f->m45C, f->m460, 14);
        fn_802B058C((Unk802ACD88_Mgr*)lbl_8061E978, (const char*)f, f->m45C);
        f->m404 = 4;
        fn_800C2A1C(f, w, t);
        fn_800C313C(f);
    }
}

extern "C" void fn_800C313C(Unk800C2250_File* f)
{
    int i = 0;
    DCStoreRange(f->m45C, f->m460);
    for (; i < fn_800C2F88(f); i++) {
        if (fn_800C2F90(f, i)) {
            Unk800C1BE4_Widget* w = fn_800C2FB0(f, i);
            w->fn_vslot28(fn_800C2F84(f), 0);
            w->fn_vslot8(1);
        }
    }
}

extern "C" void fn_800C31FC(Unk800C2250_File* f)
{
    switch (fn_800C2F74(f)) {
    case 0:
    case 1:
    case 2:
        break;
    case 4:
    case 5:
        fn_802B0638((Unk802ACD88_Mgr*)lbl_8061E978, fn_800C2F84(f), 0);
        break;
    case 3:
        if (fn_800C33D0()->m0 != 0) {
            ASYNCFILE_release(fn_800C2FE8(f), 0, 0);
            fn_80400CE4(0);
        } else {
            ASYNCFILE_cancel(fn_800C2FE8(f));
            while (!ASYNCFILE_getstatus(fn_800C2FE8(f))) {
                THREAD_yield(50);
                fn_80400CE4(0);
            }
        }
        break;
    }
}


extern "C" int fn_800C32C4(void* unused, int kind, void** out)
{
    int size = fn_800C519C(fn_800C33D0(), kind);
    switch (kind) {
    case 0:
        break;
    case 1:
    case 2:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
        *out = fn_803774A8(size, 0x500);
        break;
    }
    return size;
}

extern "C" void fn_800C334C(Unk800C2250_File* f)
{
    if (f->m45C != 0) {
        switch (f->m400) {
        case 1:
        case 2:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
            if (f->m45C != 0) fn_80377488((void*)f->m45C);
            f->m45C = 0;
            f->m464 = 60;
            break;
        }
    }
}

extern "C" Unk800C341C_Mgr* fn_800C33D0()
{
    if (lbl_8061ABDC == 0)
        lbl_8061ABDC = fn_800C341C((Unk800C341C_Mgr*)operator new(0x5DAC, 0));
    return lbl_8061ABDC;
}

struct Unk80356E88;
void fn_80356E88(Unk80356E88* r);
extern const char lbl_80658378[];

extern "C" Unk800C341C_Mgr* fn_800C341C(Unk800C341C_Mgr* m)
{
    int* w = (int*)m;
    *(const void**)((char*)m + 0x5DA8) = lbl_80658378;
    w[0] = 0;
    w[1] = 0;
    w[2] = 0;
    w[3] = 0;
    w[4] = 0;
    w[5] = 0;
    w[6] = 0;
    w[7] = 0;
    w[8] = 0;
    w[9] = 0;
    w[10] = 0;
    w[11] = 0;
    w[12] = 0;
    w[13] = 0;
    w[14] = 0;
    w[15] = 0;
    w[16] = 0;
    w[17] = 0;
    w[18] = 0;
    w[19] = 0;
    w[20] = 0;
    w[21] = 0;
    w[22] = 0;
    w[23] = 0;
    w[24] = 0;
    w[25] = 0;
    w[26] = 0;
    w[27] = 0;
    w[28] = 0;
    w[29] = 0;
    w[30] = 0;
    w[31] = 0;
    w[32] = 0;
    w[33] = 0;
    w[34] = 0;
    w[35] = 0;
    w[36] = 0;
    w[37] = 0;
    m->m98 = 0;
    Unk800C2250_File* f = &m->files[0];
    int n = 9;
    do {
        new (f) Unk800C2250_File;
        f++;
    } while (n-- != 0);
    Unk800C3A10_Rec* r = &m->recs[0];
    n = 11;
    do {
        fn_80356E88((Unk80356E88*)r);
        r++;
    } while (n-- != 0);
    m->m5D94 = 0;
    m->m5D98 = 0;
    m->m5D9C = 0;
    m->m5DA0 = 1;
    m->m5DA4 = 10;
    fn_800C3A10(m);
    return m;
}

struct Unk800C2580_Cfg {
    char pad0[0x204];
    int m204;
};
extern Unk800C2580_Cfg* lbl_8061FEDC;
extern "C" void fn_803FB3DC(int a, int b);
extern "C" void fn_800C5BC0(void* unused, const char* path, int* h, int* out);
extern const char* lbl_8061AB80[];
extern const char* lbl_8061AB84;

extern "C" void fn_800C355C(Unk800C341C_Mgr* m, int state)
{
    int* w = (int*)m;
    if (state == m->m4)
        return;
    if (m->m4 == 0) {
        fn_803FB3DC(0x18, 0);
        fn_800C5BC0(m, lbl_8061AB84, &w[2], &w[20]);
    }
    if (state == 1) {
        const char** tbl = lbl_8061AB80;
        fn_800C5BC0(m, tbl[4], &w[4], &w[22]);
        fn_800C5BC0(m, tbl[6], &w[5], &w[23]);
        fn_800C5BC0(m, tbl[5], &w[6], &w[24]);
        if (lbl_8061FEDC->m204 != 0) {
            fn_800C5BC0(m, tbl[9], &w[8], &w[26]);
            fn_800C5BC0(m, tbl[3], &w[3], &w[21]);
        } else {
            fn_800C5BC0(m, tbl[8], &w[8], &w[26]);
            fn_800C5BC0(m, tbl[2], &w[3], &w[21]);
        }
        const char** tbl2 = lbl_8061AB80;
        fn_800C5BC0(m, tbl2[10], &w[9], &w[27]);
        fn_800C5BC0(m, tbl2[11], &w[10], &w[28]);
        fn_800C5BC0(m, tbl2[12], &w[11], &w[29]);
        fn_800C5BC0(m, tbl2[13], &w[12], &w[30]);
        fn_800C5BC0(m, tbl2[14], &w[13], &w[31]);
        fn_800C5BC0(m, tbl2[15], &w[14], &w[32]);
        fn_800C5BC0(m, tbl2[16], &w[15], &w[33]);
        fn_800C5BC0(m, tbl2[17], &w[16], &w[34]);
        fn_800C5BC0(m, tbl2[18], &w[17], &w[35]);
        fn_800C5BC0(m, tbl2[19], &w[18], &w[36]);
        fn_800C5BC0(m, tbl2[20], &w[19], &w[37]);
    } else if (m->m4 == 1) {
        fn_800C3828(m);
    }
    m->m4 = state;
}

extern "C" void fn_800C3774(Unk800C341C_Mgr* m)
{
    fn_800C3D50(m);
    fn_800C5010(m);
    fn_800C5AF8(m);
    fn_800C5080(m);
    fn_800C5B58(m);
    fn_800C3A4C(m);
    fn_800C3A10(m);
    if (m->m8 != 0) {
        fn_803FEDB8(m->m8, 100);
        m->m8 = 0;
    }
    if (m->m1C != 0) {
        fn_803FEDB8(m->m1C, 100);
        m->m1C = 0;
    }
    if (m->m4 == 1)
        fn_800C3828(m);
    fn_803FB4B0();
    m->m4 = 0;
}

extern "C" void fn_800C3828(Unk800C341C_Mgr* m)
{
    void** w = (void**)m;
    if (w[3] != 0) {
        fn_803FEDB8(w[3], 100);
        w[3] = 0;
    }
    if (w[4] != 0) {
        fn_803FEDB8(w[4], 100);
        w[4] = 0;
    }
    if (w[5] != 0) {
        fn_803FEDB8(w[5], 100);
        w[5] = 0;
    }
    if (w[6] != 0) {
        fn_803FEDB8(w[6], 100);
        w[6] = 0;
    }
    if (w[8] != 0) {
        fn_803FEDB8(w[8], 100);
        w[8] = 0;
    }
    if (w[9] != 0) {
        fn_803FEDB8(w[9], 100);
        w[9] = 0;
    }
    if (w[10] != 0) {
        fn_803FEDB8(w[10], 100);
        w[10] = 0;
    }
    if (w[11] != 0) {
        fn_803FEDB8(w[11], 100);
        w[11] = 0;
    }
    if (w[12] != 0) {
        fn_803FEDB8(w[12], 100);
        w[12] = 0;
    }
    if (w[13] != 0) {
        fn_803FEDB8(w[13], 100);
        w[13] = 0;
    }
    if (w[14] != 0) {
        fn_803FEDB8(w[14], 100);
        w[14] = 0;
    }
    if (w[15] != 0) {
        fn_803FEDB8(w[15], 100);
        w[15] = 0;
    }
    if (w[16] != 0) {
        fn_803FEDB8(w[16], 100);
        w[16] = 0;
    }
    if (w[17] != 0) {
        fn_803FEDB8(w[17], 100);
        w[17] = 0;
    }
    if (w[18] != 0) {
        fn_803FEDB8(w[18], 100);
        w[18] = 0;
    }
    if (w[19] != 0) {
        fn_803FEDB8(w[19], 100);
        w[19] = 0;
    }
}

extern "C" void fn_800C3A10(Unk800C341C_Mgr* m)
{
    Unk800C3A10_Rec* r = m->recs;
    for (int i = 0; i < 12; i++) {
        r->m0 = 0;
        r->m4 = 0;
        r->m404 = 0;
        r->m408 = lbl_805BBE18.v;
        r->m40C = 0;
        r++;
    }
    m->m5D94 = 0;
}

extern "C" void fn_800C3A4C(Unk800C341C_Mgr* m)
{
    for (int i = 0; i < 10; i++) {
        Unk800C2250_File* f = &m->files[i];
        if (fn_800C2EF0(f) && fn_800C2F74(f) == 6) {
            fn_800C334C(f);
            fn_800C2FF0(f);
        }
    }
}

extern "C" void fn_800C3AB4(Unk800C341C_Mgr* m, int n)
{
    if (n > 10)
        n = 10;
    m->m5DA4 = n;
}

extern "C" void fn_800C3AC8(Unk800C341C_Mgr* m)
{
    for (int i = 0; i < 10; i++)
        fn_800C2C78(&m->files[i]);
    fn_800C3B14(m);
}

extern "C" void fn_800C3B14(Unk800C341C_Mgr* m)
{
    int i = 0;
    int n = m->m5D94;
    if (i < n) {
        do {
            if (fn_800C58E8(m, m->recs[0].m0) == -1)
                break;
            switch (m->recs[0].m0) {
            case 2:
                fn_800C435C(m, m->recs[0].m404, m->recs[0].m408, m->recs[0].m4_name(), m->recs[0].m40C);
                break;
            case 8:
                fn_800C4D90(m, m->recs[0].m404, m->recs[0].m408, m->recs[0].m4_name(), m->recs[0].m40C);
                break;
            default:
                fn_800C537C(m, m->recs[0].m0, m->recs[0].m4_name(), m->recs[0].m404, m->recs[0].m408, m->recs[0].m40C);
                break;
            }
            fn_800C56C0(m, 0);
            i++;
        } while (i < n);
    }
}

extern "C" void fn_800C3BE0(Unk800C341C_Mgr* m)
{
    m->m0 = 1;
    for (int i = 0; i < 10; i++) {
        Unk800C2250_File* f = &m->files[i];
        if (fn_800C2EF0(f) && !fn_800C2F08(f) && !fn_800C2F1C(f) && !fn_800C2F30(f))
            fn_800C2458(f);
    }
    m->m0 = 0;
    fn_800C3A10(m);
    m->m5DA4 = 10;
}

extern "C" void fn_800C3C80(Unk800C341C_Mgr* m, Unk800C1BE4_Widget* w)
{
    int i;
    for (i = 0; i < 10; i++) {
        Unk800C2250_File* f = &m->files[i];
        if (fn_800C2EF0(f) && !fn_800C2F08(f) && !fn_800C2F30(f)) {
            int idx = fn_800C2998(f, w);
            if (idx != -1)
                fn_800C2B94(f, idx);
        }
    }
    int n = m->m5D94;
    for (i = 0; i < n; i++) {
        if (m->recs[i].m404 == (int)w)
            fn_800C56C0(m, i);
    }
}

extern "C" void fn_800C3D50(Unk800C341C_Mgr* m)
{
    for (int i = 0; i < 10; i++) {
        Unk800C2250_File* f = &m->files[i];
        if (fn_800C2EF0(f) && !fn_800C2F08(f) && !fn_800C2F1C(f))
            fn_800C2458(f);
    }
    fn_800C3A10(m);
}

extern "C" void fn_800C3DCC(Unk800C341C_Mgr* m, int w, Unk800C3FEC_Obj* obj)
{
    if (lbl_8061A32C == 0)
        lbl_8061A32C = fn_800B64BC((Unk800B64BC_Mgr*)operator new(0xD8, 0));
    if (lbl_8061A32C->mB8 == 1) {
        char* p = (char*)obj + 0x588;
        int k = fn_8025416C((Unk8024EF40_W*)p, 7);
        if (k >= 0) {
            if (k > 1) {
                if (k == 2) {
                    Unk800C3FEC_Tmp t;
                    fn_80356930((char*)&t);
                    obj->fn_vslot11(&t, -1);
                    fn_800C422C(m, w, t.m2C - 0x385, t.m34, t.m3C, t.m38);
                }
            } else {
                fn_800C429C(m, w, fn_8025416C((Unk8024EF40_W*)p, 0));
            }
        }
    } else {
        fn_800C3ED4(m, w, obj->m704);
    }
}

extern "C" void fn_800C3ED4(Unk800C341C_Mgr* m, int w, int obj)
{
    int x = 0;
    int y = 5;
    int o = obj;
    fn_800DC3D8(&x, &y, obj);
    int k = fn_800DBAE0(x, 7, y, o);
    if (k >= 0) {
        if (k > 1) {
            if (k == 2) {
                int p = fn_800DBAE0(x, 0x2C, y, o);
                int q = fn_800DBAE0(x, 0x25, y, o);
                int r = fn_800DBAE0(x, 0x23, y, o);
                int s = fn_800DBAE0(x, 0x28, y, o);
                fn_800C422C(m, w, p - 0x385, q, r, s);
            }
        } else {
            fn_800C429C(m, w, fn_800DBAE0(x, 0, y, o));
        }
    }
}

extern "C" void fn_800C3FEC(Unk800C341C_Mgr* m, int w, int c, Unk800C3FEC_Obj* obj)
{
    if (lbl_8061A32C == 0)
        lbl_8061A32C = fn_800B64BC((Unk800B64BC_Mgr*)operator new(0xD8, 0));
    if (lbl_8061A32C->mB8 == 1) {
        Unk800C3FEC_Tmp t;
        fn_80356930((char*)&t);
        obj->fn_vslot11(&t, -1);
        fn_800C422C(m, w, t.m2C - 0x385, t.m34, t.m3C, t.m38);
    } else {
        fn_800C40B4(m, w, c, obj->m704);
    }
}

extern "C" void fn_800C40B4(Unk800C341C_Mgr* m, int w, int c, int obj)
{
    int x = 0;
    int y = 5;
    fn_800DC3D8(&x, &y, obj);
    if ((unsigned)c <= 2) {
        int k = 2;
        if (c == 0) {
            if (lbl_8061930C->fn_8007C514(14, obj) != 0)
                k = 1;
        }
        if (k != 1) {
            if (k == 2) {
                int p = fn_800DC1B8(x, 1, y, c, obj);
                int q = fn_800DC1B8(x, 3, y, c, obj);
                int r = fn_800DC1B8(x, 2, y, c, obj);
                int s = fn_800DC1B8(x, 4, y, c, obj);
                fn_800C422C(m, w, p - 0x385, q, r, s);
            }
        } else {
            fn_800C429C(m, w, fn_800DC1B8(x, 0, y, c, obj));
        }
    } else {
        fn_800C42FC(m, w, fn_800DC1B8(x, 1, y, c, obj));
    }
}

extern "C" void fn_800C422C(Unk800C341C_Mgr* m, int idx, int a, int b, int unused, int d)
{
    char buf[1024];
    sprintf(buf, lbl_805BBE1C, a + 0x61, b + 0x61, 0x64, d + 0x61);
    fn_800C537C(m, 1, buf, idx, lbl_805BBE28, 1);
}

extern "C" void fn_800C429C(Unk800C341C_Mgr* m, int idx, int x)
{
    char buf[1024];
    sprintf(buf, lbl_805BBE2C, x);
    fn_800C537C(m, 1, buf, idx, lbl_805BBE34, 1);
}

extern "C" void fn_800C42FC(Unk800C341C_Mgr* m, int idx, int x)
{
    char buf[1024];
    sprintf(buf, lbl_805BBE38, x);
    fn_800C537C(m, 1, buf, idx, lbl_805BBE40, 1);
}

extern "C" {
void fn_800C5AA4(Unk800C341C_Mgr* m);
void fn_800C5A50(Unk800C341C_Mgr* m);
int fn_800C5980(Unk800C341C_Mgr* m);
void fn_800C2580(Unk800C2250_File* f, const char* name, Unk800C1BE4_Widget* w, int c, float t);
void fn_800C5574(Unk800C341C_Mgr* m, int kind, const char* name, Unk800C1BE4_Widget* w, float t, int flag);
void fn_800C5798(Unk800C341C_Mgr* m, Unk800C1BE4_Widget* w);
int fn_800C59E8(Unk800C341C_Mgr* m);
void fn_800C26D0(Unk800C2250_File* f, const char* name, Unk800C1BE4_Widget* w, float t, int c);
}

extern "C" void fn_800C435C(Unk800C341C_Mgr* m, int wi, float t, const char* name, int flag)
{
    Unk800C1BE4_Widget* w = (Unk800C1BE4_Widget*)wi;
    int z = m->m5D9C;
    if (z == 0) {
        fn_800C5AA4(m);
        m->m5DA0 = z;
    }
    int idx = fn_800C5CF8(m, name);
    Unk800C2250_File* f;
    if (idx != -1) {
        while (f = &m->files[idx], fn_800C2F88(f) != 0) {
            if (fn_800C2F90(f, 0)) {
                fn_800C2FB0(f, 0)->fn_vslot28(&lbl_805B6F58, 0);
                fn_800C2FB0(f, 0)->fn_vslot8(0);
            }
            fn_800C2B94(f, 0);
        }
        if (wi != 0)
            fn_800C5798(m, w);
        fn_800C2A1C(f, w, t);
    } else {
        if (wi != 0)
            fn_800C5798(m, w);
        idx = fn_800C59E8(m);
        if (idx == -1)
            fn_800C5574(m, 2, name, w, t, flag);
        else
            fn_800C26D0(&m->files[idx], name, w, t, flag);
    }
}

extern "C" void fn_800C44E8(Unk800C341C_Mgr* m, Unk800C1BE4_Widget* w, int idx)
{
    int name = lbl_8061A708[idx];
    if (w != 0)
        w->fn_vslot20(lbl_805BBE44);
    fn_800C537C(m, 4, (const char*)name, (int)w, lbl_805BBE48, 1);
}

extern "C" void fn_800C4568(Unk800C341C_Mgr* m, int idx, int x)
{
    char buf[1024];
    sprintf(buf, lbl_805BBE4C, x);
    fn_800C537C(m, 5, buf, idx, lbl_805BBE54, 1);
}

struct Unk80084910_N {
    int m0;
    int m4;
};
void fn_80086030(Unk80084910_N* self, char* buf);
void fn_80086078(Unk80084910_N* self, char* buf);
void fn_800860E0(Unk80084910_N* self, char* buf);
extern "C" int sprintf(char*, const char*, ...);
extern "C" int FILESYS_bypassqueuefileinfo(const char* path, int a, int* out);
extern const float lbl_805BBE64;
extern const float lbl_805BBE68;
extern const char lbl_805BBD44[];
extern int lbl_8061AB98;

extern "C" int fn_800C45C8(Unk800C341C_Mgr* m, Unk800D2338_Obj* w, int x)
{
    char a[0x400];
    char b[0x400];
    char c[0x20];
    char d[0x400];
    Unk80084910_N n;
    Unk80084910_N n2;
    int out;
    int cnt;
    int i;
    n.m0 = x;
    n.m4 = 0;
    cnt = 0;
    fn_80086030(&n, a);
    i = 0;
    w->fn_vslot20(lbl_805BBE64);
    fn_800C537C(m, 6, a, (int)fn_800C8038(w), lbl_805BBE68, 1);
    ((Unk800CFEEC_Sel*)((char*)w + 0x98))->fn_vslot15();
    do {
        n2.m0 = x;
        n2.m4 = i;
        fn_80086078(&n2, b);
        sprintf(d, lbl_805BBD44, lbl_8061AB98, b);
        if (FILESYS_bypassqueuefileinfo(d, 1, &out) != 0) {
            cnt++;
            fn_800860E0(&n2, c);
            w->fn_vslot31(c, 0, 1);
        }
        i++;
    } while (i <= 15);
    return cnt;
}

extern "C" void fn_800C4720(Unk800C341C_Mgr* m, Unk800D2338_Obj* w)
{
    char buf[0x400];
    sprintf(buf, lbl_805BBE6C, w->fn_vslot33(0), lbl_805BB0E4);
    int idx = fn_800C5CF8(m, buf);
    if (idx != -1) {
        Unk800C2250_File* f = &m->files[idx];
        fn_800C2B94(f, fn_800C2998(f, 0));
    }
    ((Unk800CFEEC_Sel*)((char*)w + 0x98))->fn_vslot15();
}

extern "C" void fn_800C47DC(Unk800C341C_Mgr* m, int idx, int x)
{
    fn_800C537C(m, 13, lbl_8061A864[x], idx, lbl_805BBE74, 1);
}

extern "C" void fn_800C4820(Unk800C341C_Mgr* m, int idx, int x)
{
    fn_800C537C(m, 10, lbl_8061AA40[x], idx, lbl_805BBE78, 1);
}

extern "C" void fn_800C4864(Unk800C341C_Mgr* m, int idx, int x)
{
    fn_800C537C(m, 0xb, *(const char**)((char*)lbl_8061AB78 + x * 4), idx, lbl_805BBE7C, 1);
}

extern "C" void fn_800C48A8(Unk800C341C_Mgr* m, int idx, int x)
{
    fn_800C537C(m, 12, lbl_8061AB3C[x], idx, lbl_805BBE80, 1);
}

extern "C" void fn_800C48EC(Unk800C341C_Mgr* m, int idx, int flag)
{
    const char* name;
    if (flag != 0)
        name = lbl_805BBE84;
    else
        name = lbl_805BBE8C;
    fn_800C537C(m, 0xc, name, idx, lbl_805BBE94, 1);
}

extern "C" void fn_800C493C(Unk800C341C_Mgr* m, int idx, int k)
{
    Unk800C493C_Letters t __attribute__((aligned(4))) = lbl_805BBE98;
    char buf[24];
    sprintf(buf, lbl_805BBEA0, t.c[k]);
    fn_800C537C(m, 0xe, buf, idx, lbl_805BBEA8, 1);
}

extern "C" void fn_800C49C4(Unk800C341C_Mgr* m, int idx, int k, int z)
{
    Unk800C49C4_Letters t __attribute__((aligned(4))) = lbl_805BBEAC;
    char buf[24];
    if (k == 0)
        z = 0;
    sprintf(buf, lbl_805BBEBC, t.c[k], z);
    fn_800C537C(m, 0xf, buf, idx, lbl_805BBEC4, 1);
}

extern "C" void fn_800C4A68(Unk800C341C_Mgr* m, int idx, int x)
{
    char buf[24];
    sprintf(buf, lbl_805BBEC8, x);
    fn_800C537C(m, 16, buf, idx, lbl_805BBED0, 1);
}

extern "C" void fn_800C4AC8(Unk800C341C_Mgr* m, int idx, int x)
{
    char buf[24];
    sprintf(buf, lbl_805BBED4, x);
    fn_800C537C(m, 17, buf, idx, lbl_805BBEDC, 1);
}

extern "C" void fn_800C4B28(Unk800C341C_Mgr* m, int idx, int x)
{
    char buf[24];
    sprintf(buf, lbl_805BBEE0, x);
    fn_800C537C(m, 18, buf, idx, lbl_805BBEE8, 1);
}

extern "C" void fn_800C4B88(Unk800C341C_Mgr* m, int idx, int x)
{
    char buf[24];
    sprintf(buf, lbl_805BBEE0, x);
    fn_800C537C(m, 18, buf, idx, lbl_805BBEEC, 1);
}

extern "C" void fn_800C4BE8(Unk800C341C_Mgr* m, int idx, int x)
{
    char buf[24];
    sprintf(buf, lbl_805BBEC8, x);
    fn_800C537C(m, 16, buf, idx, lbl_805BBEF0, 1);
}

extern "C" void fn_800C4C48(Unk800C341C_Mgr* m, int w, int idx)
{
    Unk800C4C48_Tbl t __attribute__((aligned(4))) = lbl_805BBEF4;
    char buf[20];
    sprintf(buf, lbl_805BBF34, t.v[idx]);
    fn_800C537C(m, 0x13, buf, w, lbl_805BBF3C, 1);
}

extern "C" void fn_800C4D28(Unk800C341C_Mgr* m, int idx, int x, int y)
{
    char buf[24];
    sprintf(buf, lbl_805BBF40, x, (char)(y + 0x61));
    fn_800C537C(m, 0x14, buf, idx, lbl_805BBF48, 1);
}

extern "C" void fn_800C4D90(Unk800C341C_Mgr* m, int wi, float t, const char* name, int flag)
{
    Unk800C1BE4_Widget* w = (Unk800C1BE4_Widget*)wi;
    if (m->m5D98 == 0)
        fn_800C5A50(m);
    int idx = fn_800C5CF8(m, name);
    Unk800C2250_File* f;
    if (idx != -1) {
        while (f = &m->files[idx], fn_800C2F88(f) != 0) {
            if (fn_800C2F90(f, 0)) {
                fn_800C2FB0(f, 0)->fn_vslot28(&lbl_805B6F58, 0);
                fn_800C2FB0(f, 0)->fn_vslot8(0);
            }
            fn_800C2B94(f, 0);
        }
        if (wi != 0)
            fn_800C5798(m, w);
        fn_800C2A1C(f, w, t);
    } else {
        if (wi != 0)
            fn_800C5798(m, w);
        fn_800C2580(&m->files[fn_800C5980(m)], name, w, flag, t);
    }
}

extern "C" void fn_800C4EEC(Unk800C341C_Mgr* m, const char* name)
{
    int idx = fn_800C5CF8(m, name);
    if (idx != -1) {
        Unk800C2250_File* f;
        while (fn_800C2F88(f = &m->files[idx])) {
            if (fn_800C2F90(f, 0))
                fn_800C2B94(f, 0);
        }
    }
}

extern "C" void fn_800C4F64(Unk800C341C_Mgr* m, const char* name)
{
    int idx = fn_800C5CF8(m, name);
    if (idx != -1)
        fn_800C2848(&m->files[idx]);
}

extern "C" void fn_800C4FA8(Unk800C341C_Mgr* m, const char* name)
{
    if (strcmp(name, &lbl_805B6F58) != 0) {
        int idx = fn_800C5CF8(m, name);
        if (idx != -1)
            fn_800C28C8(&m->files[idx]);
    }
}

extern "C" void fn_800C5010(Unk800C341C_Mgr* m)
{
    for (int i = 0; i < 10; i++) {
        Unk800C2250_File* f = &m->files[i];
        if (fn_800C2F08(f) && fn_800C2F44(f) && !fn_800C2F30(f))
            fn_800C2848(f);
    }
}

extern "C" void fn_800C5080(Unk800C341C_Mgr* m)
{
    if (m->m5DA0 == 0) {
        for (int i = 0; i < 10; i++) {
            Unk800C2250_File* f = &m->files[i];
            if (fn_800C2F1C(f) && fn_800C2F5C(f))
                fn_800C28C8(f);
        }
    }
}

extern "C" int fn_800C50EC(Unk800C341C_Mgr* m, Unk800C1BE4_Widget* w)
{
    return fn_800C5C68(m, w);
}

extern "C" int fn_800C510C(Unk800C341C_Mgr* m, const char* name)
{
    return fn_800C5CF8(m, name);
}

extern "C" int fn_800C512C(Unk800C341C_Mgr* m, int idx)
{
    return fn_800C2F74(&m->files[idx]);
}

