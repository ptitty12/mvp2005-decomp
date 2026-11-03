/*
 * Game code, .text 0x800CB3A8-0x800CF178 (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_800BCD38.h"
#include "game/tu_800BCD38_decls.h"

extern "C" void fn_80317120(int a, int b);
extern "C" void fn_800D0D90(void* p);
extern "C" void Unicode2Ascii(char* dst, const char* src);
extern "C" void fn_80403144(char* dst, const char* src);

inline int AbsI(int x)
{
    return x < 0 ? -x : x;
}

extern "C" int fn_800CB3A8(Unk800CAF58_Obj* o)
{
    fn_800D0D90((char*)o + 0xC8);
    if (!o->b1)
        return 0;
    if (o->m54 == 1 && o->m50 > 0 && o->fn_vslot85()) {
        Unk800C7B40_Widget* w = o->fn_vslot32(o->fn_vslot85(), 0);
        for (Unk800C7B40_Widget* c = w->m4; c != 0 && (unsigned)c->m20 > (unsigned)w->m20; c = c->m4) {
            if (c->m1C == 11)
                ((Unk800C7B40_Widget*)fn_800C805C(c))->fn_vslot41();
            if (c->m1C == 6)
                ((Unk800C7B40_Widget*)fn_800C804C(c))->fn_vslot46();
        }
    }
    if (o->m7C != 0)
        o->m7C->fn_vslot46();
    Unk800C7B40_Widget* v = o->fn_vslot30("HSPIN", 0);
    if (v != 0)
        ((Unk800C7B40_Widget*)fn_800C805C(((Unk800C7B40_Widget*)fn_800C8044(v))->fn_vslot32("SG1", 1)))->fn_vslot41();
    return 1;
}

extern char lbl_80687A68[0x64];
extern char lbl_80687ACC[0x64];
extern char lbl_80687B30[0x64];

#define GRID_BOOL_CELL(g, row, col) ((g)->m98[(row) * (g)->m54 + (col)])

extern "C" int fn_800CB578(Unk800CB3A8_Grid* g, int row, int col)
{
    if (g->b80_2)
        return (GRID_BOOL_CELL(g, row, col) >> 1) & 1;
    return g->fn_vslot73(row, col)->b1;
}

extern "C" int fn_800CB5E4(Unk800CB3A8_Grid* g, int row, int col)
{
    if (g->b80_2)
        return (GRID_BOOL_CELL(g, row, col) >> 2) & 1;
    return g->fn_vslot73(row, col)->b0;
}

extern "C" int fn_800CB650(Unk800CB3A8_Grid* g, int row, int col)
{
    if (g->b80_2) {
        int r = 0;
        if (GRID_BOOL_CELL(g, row, col) & 1)
            r = 1;
        return r;
    }
    return g->fn_vslot73(row, col)->b2;
}

extern "C" void fn_800CB6C8(Unk800CB3A8_Grid* g, int row, int col, float v)
{
    Unk800C7B40_Widget* w = g->fn_vslot74(row, col);
    if (w != 0)
        w->fn_vslot20(v);
    if (g->b80_2)
        g->m9C[row * g->m54 + col] = v;
}

extern "C" float fn_800CB760(Unk800CB3A8_Grid* g, int row, int col)
{
    if (g->b80_2)
        return g->m9C[row * g->m54 + col];
    return g->fn_vslot73(row, col)->fn_vslot19();
}

extern "C" void fn_800CB7D8(Unk800CB3A8_Grid* g, int back)
{
    int step = 1;
    int last = g->m50 - 1;
    int old = g->m68;
    int first = g->m60;
    if (back != 0)
        step = -1;
    for (; first < g->m50 - 1; first++) {
        if (g->fn_vslot123(first, g->m6C))
            break;
    }
    for (; last > g->m60; last--) {
        if (g->fn_vslot123(last, g->m6C))
            break;
    }
    while (1) {
        int v = g->m68 + step;
        g->m68 = v;
        if (v < g->m60) {
            if (g->b80_0)
                v = g->m50 - 1;
            else
                v = first;
            g->m68 = v;
        } else if (v >= g->m50) {
            int w = last;
            if (g->b80_0)
                w = g->m60;
            g->m68 = w;
        }
        if (g->m68 == old)
            break;
        if (g->m68 < g->m60)
            continue;
        if (g->fn_vslot123(g->m68, g->m6C))
            break;
    }
    g->mF4->fn_vslot69(g);
}

extern "C" void fn_800CB954(Unk800CB3A8_Grid* g, int back)
{
    int step = 1;
    int last = g->m54 - 1;
    int old = g->m6C;
    int first = g->m64;
    if (back != 0)
        step = -1;
    for (; first < g->m54 - 1; first++) {
        if (g->fn_vslot123(g->m68, first))
            break;
    }
    for (; last > g->m64; last--) {
        if (g->fn_vslot123(g->m68, last))
            break;
    }
    while (1) {
        int v = g->m6C + step;
        g->m6C = v;
        if (v < g->m64) {
            if (g->b80_1)
                v = g->m54 - 1;
            else
                v = first;
            g->m6C = v;
        } else if (v >= g->m54) {
            int w = last;
            if (g->b80_1)
                w = g->m64;
            g->m6C = w;
        }
        if (g->m6C == old)
            break;
        if (g->m6C < g->m64)
            continue;
        if (g->fn_vslot123(g->m68, g->m6C))
            break;
    }
    g->mF4->fn_vslot69(g);
}

extern "C" void fn_800CBAD0(Unk800CB3A8_Grid* g, int row, int col, int val, int kind)
{
    Unk800CC08C_W* w = (Unk800CC08C_W*)g->fn_vslot74(row, col);
    if (w != 0) {
        switch (kind) {
        case 5:
            ((Unk800CBAD0_Sub*)fn_800C8048(w->fn_vslot33(5, 1)))->fn_vslot29(val);
            break;
        case 4:
            ((Unk800CBAD0_Sub*)fn_800C8040(w->fn_vslot33(4, 1)))->fn_vslot29(val);
            break;
        case 10:
            ((Unk800CFEEC_Sel*)((char*)fn_800C8058(w->fn_vslot33(10, 1)) + 0xDC))->fn_vslot6(val);
            break;
        case 9:
            ((Unk800CFEEC_Sel*)((char*)fn_800C8054(w->fn_vslot33(9, 1)) + 0x98))->fn_vslot6(val);
            break;
        }
    }
    if (g->b80_2) {
        if ((unsigned)(kind - 4) <= 1) {
            fn_80403144(g->m90 + (row * g->m54 + col) * g->m44 * 2, fn_80090880(lbl_8061977C, val));
        } else {
            char buf[100];
            fn_804089B4(buf, "%d", val);
            fn_804031BC(g->m90 + (row * g->m54 + col) * g->m44 * 2, buf);
        }
    }
}

extern "C" void fn_800CBCE4(Unk800CB3A8_Grid* g, int a, int* arr, int c, int start, int end)
{
    int last;
    if (end == -1)
        last = g->m50 - 1;
    else
        last = end;
    for (int i = start; i <= last; i++)
        g->fn_vslot42(i, a, arr[i - start], c);
}

extern "C" void fn_800CBD84(Unk800CB3A8_Grid* g, int a, int* arr, int c, int start, int end)
{
    int last;
    if (end == -1)
        last = g->m54 - 1;
    else
        last = end;
    for (int i = start; i <= last; i++)
        g->fn_vslot42(a, i, arr[i - start], c);
}

extern "C" void fn_800CBE28(Unk800CB3A8_Grid* g, int row, int col, const char* str, int kind)
{
    if (g->b80_2) {
        fn_80403144(g->m90 + (row * g->m54 + col) * g->m44 * 2, str);
    } else {
        Unk800CC08C_W* w = (Unk800CC08C_W*)g->fn_vslot73(row, col);
        switch (kind) {
        case 5:
            ((Unk800C7B40_Widget*)fn_800C8048(w->fn_vslot33(5, 1)))->fn_vslot28(str);
            break;
        case 4:
            ((Unk800C7B40_Widget*)fn_800C8040(w->fn_vslot33(4, 1)))->fn_vslot28(str);
            break;
        }
    }
}

extern "C" void fn_800CBF48(Unk800CB3A8_Grid* g, int a, int* arr, int c, int start, int end)
{
    int last;
    if (end == -1)
        last = g->m50 - 1;
    else
        last = end;
    for (int i = start; i <= last; i++)
        g->fn_vslot43(i, a, arr[i - start], c);
}

extern "C" void fn_800CBFE8(Unk800CB3A8_Grid* g, int a, int* arr, int c, int start, int end)
{
    int last;
    if (end == -1)
        last = g->m54 - 1;
    else
        last = end;
    for (int i = start; i <= last; i++)
        g->fn_vslot43(a, i, arr[i - start], c);
}

extern "C" void fn_800CC08C(Unk800CB3A8_Grid* g, int row, int col, const char* str, int kind)
{
    if (g->b80_2) {
        fn_804031BC(g->m90 + (row * g->m54 + col) * g->m44 * 2, str);
    } else {
        Unk800CC08C_W* w = (Unk800CC08C_W*)g->fn_vslot73(row, col);
        switch (kind) {
        case 2:
            ((Unk800C1BE4_Widget*)fn_800C8038(w->fn_vslot33(2, 1)))->fn_vslot28(str, 1);
            break;
        case 4:
            ((Unk800CC208_Sub*)fn_800C8040(w->fn_vslot33(4, 1)))->fn_vslot30(str);
            break;
        case 5:
            ((Unk800CC208_Sub*)fn_800C8048(w->fn_vslot33(5, 1)))->fn_vslot30(str);
            break;
        }
    }
}

extern "C" void fn_800CC208(Unk800CB3A8_Grid* g, int row, int col, const char* str, const char* extra)
{
    if (g->b80_2) {
        fn_804031BC(g->m90 + (row * g->m54 + col) * g->m44 * 2, str);
    } else {
        Unk800C7B40_Widget* w = g->fn_vslot73(row, col);
        ((Unk800CC208_Sub*)fn_800C8040(w->fn_vslot32(extra, 1)))->fn_vslot30(str);
    }
}

extern "C" void fn_800CC2D0(Unk800CB3A8_Grid* g, int row, int col, int id, const char* extra)
{
    if (g->b80_2) {
        char* dst = g->m90 + (row * g->m54 + col) * g->m44 * 2;
        fn_80403144(dst, fn_80090880(lbl_8061977C, id));
    } else {
        Unk800C7B40_Widget* w = g->fn_vslot73(row, col);
        char buf[100];
        fn_804089B4(buf, "%d", id);
        ((Unk800CC208_Sub*)fn_800C8040(w->fn_vslot32(extra, 1)))->fn_vslot30(buf);
    }
}

extern "C" char* fn_800CC3C8(Unk800CB3A8_Grid* g, int row, int col, int kind)
{
    if (g->b80_2)
        return g->m90 + (row * g->m54 + col) * g->m44 * 2;
    Unk800CC08C_W* w = (Unk800CC08C_W*)g->fn_vslot73(row, col);
    switch (kind) {
    case 4:
        return ((Unk800CC3C8_Sub*)fn_800C8040(w->fn_vslot33(4, 1)))->fn_vslot31();
    case 5:
        return ((Unk800CC3C8_Sub*)fn_800C8048(w->fn_vslot33(5, 1)))->fn_vslot31();
    }
    return 0;
}

extern "C" void fn_800CC4C0(Unk800CB3A8_Grid* g, int a, int* arr, int c, int start, int end)
{
    int last;
    if (end == -1)
        last = g->m50 - 1;
    else
        last = end;
    for (int i = start; i <= last; i++)
        g->fn_vslot44(i, a, arr[i - start], c);
}

extern "C" void fn_800CC560(Unk800CB3A8_Grid* g, int a, int* arr, int c, int start, int end)
{
    int last;
    if (end == -1)
        last = g->m54 - 1;
    else
        last = end;
    for (int i = start; i <= last; i++)
        g->fn_vslot44(a, i, arr[i - start], c);
}

/* near-miss: 27 slots, register assignment only (retail keeps g in r31 and the widget in r30; ours swaps them); function-scope i/j/w permutations were worse */
extern "C" void fn_800CC604(Unk800CB3A8_Grid* g, int n, int flag)
{
    Unk800C7B40_Widget* w;
    g->m70 = 0;
    g->m74 = 0;
    if (!g->b80_2 && g->m50 > g->m58 && n <= g->m58) {
        for (int i = 0; i < g->m58; i++) {
            for (int j = 0; j < g->m54; j++)
                g->fn_vslot115(i, j, i, j);
        }
    }
    int cur = g->m50;
    if (n > cur) {
        for (int i = cur; i < (n > g->m48 ? g->m48 : n); i++) {
            for (int j = 0; j < g->m4C; j++) {
                w = g->fn_vslot73(i, j);
                w->fn_vslot8(1);
                w->fn_vslot16(1);
            }
        }
    } else if (n < g->m48) {
        for (int i = n; i < g->m48; i++) {
            for (int j = 0; j < g->m4C; j++) {
                w = g->fn_vslot73(i, j);
                w->fn_vslot8(flag);
                w->fn_vslot16(0);
            }
        }
    }
    g->m50 = n;
    if (g->m68 >= n)
        g->m68 = n - 1;
    if (g->m68 == -1)
        g->m68 = 0;
    if (g->m78 != 0) {
        int d = g->m50 - g->m58 + 1;
        if (d <= 0)
            d = 1;
        g->m78->fn_vslot33(d);
        g->m78->fn_vslot35(0);
    }
}

extern "C" void fn_800CC8A0(Unk800CB3A8_Grid* g, int n, int flag)
{
    Unk800C7B40_Widget* w;
    int cur = g->m54;
    if (n > cur) {
        for (int i = cur; i < (n > g->m4C ? g->m4C : n); i++) {
            for (int r = 0; r < g->m48; r++) {
                w = g->fn_vslot73(r, i);
                w->fn_vslot8(1);
                w->fn_vslot16(1);
            }
        }
    } else if (n < g->m4C) {
        for (int i = n; i < g->m4C; i++) {
            for (int r = 0; r < g->m48; r++) {
                w = g->fn_vslot73(r, i);
                w->fn_vslot8(flag);
                w->fn_vslot16(0);
            }
        }
    }
    g->m54 = n;
    if (g->m7C != 0) {
        int d = n - g->m5C + 1;
        if (d <= 0)
            d = 1;
        g->m7C->fn_vslot33(d);
        g->m7C->fn_vslot35(0);
    }
}

extern "C" char* fn_800CD2DC(Unk800CB3A8_Grid* g, int p1, int p2)
{
    int a;
    int b;
    if (g->fn_vslot112(p1, p2, &a, &b)) {
        fn_804089B4(lbl_80687A68, "X%dY%d", b, a);
        return lbl_80687A68;
    }
    return 0;
}

extern "C" int fn_800CD354(Unk800CB3A8_Grid* g, int row, int col, int* outRow, int* outCol)
{
    int c = col;
    if (g->b80_2) {
        if (row >= g->m60) {
            row -= g->m70;
            if (row < g->m60)
                return 0;
        }
        if (c >= g->m64) {
            c -= g->m74;
            if (c < g->m64)
                return 0;
        }
    }
    *outRow = row;
    *outCol = c;
    if (*outRow < 0 || *outRow >= g->m48 || c < 0 || c >= g->m4C)
        return 0;
    return 1;
}

extern "C" char* fn_800CD3E0(Unk800CB3A8_Grid* g, int id)
{
    int v = id;
    v = v == -1 ? g->m6C : v;
    if (v >= g->m64)
        v -= g->m74;
    fn_804089B4(lbl_80687ACC, "X%dHILITE", v);
    return lbl_80687ACC;
}

extern "C" char* fn_800CD44C(Unk800CB3A8_Grid* g, int id)
{
    int v = id;
    v = v == -1 ? g->m68 : v;
    if (v >= g->m60)
        v -= g->m70;
    fn_804089B4(lbl_80687B30, "Y%dHILITE", v);
    return lbl_80687B30;
}

extern "C" void fn_800CD4B8(Unk800CB3A8_Grid* g, int row, int col)
{
    g->fn_vslot14(0);
    g->m68 = row;
    g->m6C = col;
    if (!g->fn_vslot123(row, col))
        g->fn_vslot118(1);
    g->fn_vslot120();
    g->fn_vslot14(1);
    g->mF4->fn_vslot69(g);
}

void Unk800CD5A4_Obj::fn_800CD5A4()
{
    fn_vslot73(m68, m6C);
}

void Unk800CD5A4_Obj::fn_800CD5E4()
{
    fn_vslot74();
}

extern "C" int fn_800CD618(Unk800CB3A8_Grid* g, int p1, int p2)
{
    int a;
    int b;
    if (g->fn_vslot112(p1, p2, &a, &b))
        return g->m94[a * g->m4C + b];
    return 0;
}

extern "C" void fn_800CD690(Unk800CB3A8_Grid* g)
{
    Unk800C7B40_Widget* a = g->fn_vslot30("METER", 1);
    if (a != 0) {
        Unk800C7B40_Widget* b = ((Unk800C7B40_Widget*)fn_800C8044(a))->fn_vslot30("METERBAR", 1);
        if (b == 0)
            return;
        g->mF0 = ((Unk800C7B40_Widget*)fn_800C8038(b))->m2A;
    } else
        g->mF0 = 0.0f;
}

struct Unk800CD750_Vec {
    int* m0;
    unsigned m4;
};

extern "C" void fn_800CD750(Unk800CB3A8_Grid* g, unsigned a, unsigned b, Unk800CD750_Vec* c, int* d, int* e)
{
    char buf[0xD0];
    int j = 0;
    while (j < g->m54) {
        fn_80403144(buf, g->m90 + ((b * g->m54 + j) * g->m44) * 2);
        g->fn_vslot43(b, j, (int)(g->m90 + ((a * g->m54 + j) * g->m44) * 2), 4);
        g->fn_vslot43(a, j, (int)buf, 4);
        if (g->m98 != 0) {
            unsigned t = g->m98[a * g->m54 + j];
            g->m98[a * g->m54 + j] = g->m98[b * g->m54 + j];
            g->m98[b * g->m54 + j] = t;
            g->fn_vslot91(a, j, (g->m98[a * g->m54 + j] >> 1) & 1);
            g->fn_vslot91(b, j, (g->m98[b * g->m54 + j] >> 1) & 1);
            g->fn_vslot90(a, j, g->m98[a * g->m54 + j] & 1);
            g->fn_vslot90(b, j, g->m98[b * g->m54 + j] & 1);
        }
        j++;
    }
    if (c != 0) {
        char* p = (char*)c->m0;
        int oa = a * 4;
        int ob = b * 4;
        int va = *(int*)(p + oa);
        int vb = *(int*)(p + ob);
        if (a >= c->m4)
            c->m4 = a + 1;
        c->m0[a] = vb;
        if (b >= c->m4)
            c->m4 = b + 1;
        c->m0[b] = va;
    } else if (d != 0) {
        int x = d[b];
        int y = d[a];
        d[a] = x;
        d[b] = y;
    } else if (e != 0) {
        int x = e[b];
        int y = e[a];
        e[a] = x;
        e[b] = y;
    }
}

void Unk800CD5A4_Obj::fn_800CDA14(int a, int b, int c)
{
    fn_vslot125(a, b, 0, c, 0);
}

void Unk800CD5A4_Obj::fn_800CDA54(int a, int b, int c)
{
    fn_vslot125(a, b, c, 0, 0);
}

extern "C" void fn_800CDA90(char* s)
{
    int i = 0;
    char* p = s;
    while (i < fn_8040B6A0(s)) {
        if (*p == ',') {
            int n = fn_8040B6A0(s);
            for (int j = i + 1; j < n - 1; j++)
                *p = p[1];
            s[n - 1] = 0;
        } else {
            p++;
            i++;
        }
    }
}

extern "C" int fn_800CE23C(void* unused, const char* s)
{
    if (fn_8040B6A0(s) > 3 && s[1] == '.' && s[2] == ' ')
        return 1;
    return 0;
}

extern "C" int fn_800CE294(Unk800CB3A8_Grid* g, int col)
{
    char buf[100];
    Unicode2Ascii(buf, g->m90 + (g->m60 * g->m54 + col) * g->m44 * 2);
    int n = fn_8040B6A0(buf);
    int neg = 0;
    int all = 1;
    int dot = 0;
    for (int i = 0; i < n; i++) {
        unsigned char u = buf[i];
        char c = u;
        if (c == '.')
            dot = 1;
        if (c == '-')
            neg = 1;
        if (!((unsigned)(u - '0') <= 9 || c == '-' || c == '+' || c == '.' || c == ','))
            all = 0;
    }
    if (n == 0 || !all) {
        if (n > 6 && buf[2] == 'Y' && buf[4] == '@' && buf[n - 1] == 'Y' && buf[n - 3] == '/')
            return 4;
        return 1;
    }
    if (dot == 1 || neg == 1)
        return 3;
    return 2;
}

extern "C" void fn_800CE3D4(Unk800CB3A8_Grid* g, int row, int col, int flag)
{
    Unk800C7B40_Widget* w = g->fn_vslot74(row, col);
    if (w != 0) {
        if (w->m4 != 0)
            w->m4->fn_vslot17(flag);
        w->fn_vslot17(flag);
    }
    if (g->b80_2) {
        if (flag != 0)
            g->m98[row * g->m54 + col] |= 1;
        else
            g->m98[row * g->m54 + col] &= ~1;
    }
}

extern "C" void fn_800CE4C0(Unk800CB3A8_Grid* g, int row, int col, int flag)
{
    Unk800C7B40_Widget* w = g->fn_vslot74(row, col);
    if (w != 0) {
        if (w->m4 != 0)
            w->m4->fn_vslot16(flag);
        w->fn_vslot16(flag);
    }
    if (g->b80_2) {
        if (flag != 0)
            g->m98[row * g->m54 + col] |= 2;
        else
            g->m98[row * g->m54 + col] &= ~2;
    }
    if (flag == 0 && g->m68 == row && g->m6C == col) {
        g->fn_vslot14(0);
        g->fn_vslot118(0);
        g->fn_vslot14(1);
    }
}

extern "C" void fn_800CE620(Unk800CB3A8_Grid* g, int row, int col, int flag)
{
    Unk800C7B40_Widget* w = g->fn_vslot74(row, col);
    if (w != 0)
        w->fn_vslot8(flag);
    if (g->b80_2) {
        if (flag != 0)
            g->m98[row * g->m54 + col] |= 4;
        else
            g->m98[row * g->m54 + col] &= ~4;
    }
}

extern "C" void fn_800CE6E4(Unk800CB3A8_Grid* g)
{
    g->fn_vslot14(0);
    if (g->m74 > 0)
        g->m74 = g->m74 - 1;
    g->fn_vslot14(1);
}

extern "C" void fn_800CE758(Unk800CB3A8_Grid* g)
{
    g->fn_vslot14(0);
    if (g->m74 < g->m54 - g->m5C)
        g->m74 = g->m74 + 1;
    g->fn_vslot14(1);
}

extern "C" void fn_800CE7D8(Unk800CB3A8_Grid* g, int a, int c)
{
    if (g->m50 != 0) {
        int n = g->fn_vslot58();
        for (int i = 0; i < n; i++)
            g->fn_vslot90(a, i, c);
    }
}

extern "C" void fn_800CE86C(Unk800CB3A8_Grid* g, int a, int c)
{
    if (g->m50 != 0) {
        int n = g->fn_vslot58();
        for (int i = 0; i < n; i++)
            g->fn_vslot91(a, i, c);
    }
}

extern "C" void fn_800CE900(Unk800CB3A8_Grid* g, int a, int c)
{
    if (g->m50 != 0) {
        int n = g->fn_vslot58();
        for (int i = 0; i < n; i++)
            g->fn_vslot92(a, i, c);
    }
}

extern "C" int fn_800CE994(Unk800CB3A8_Grid* g, const char* s, int* row, int* col)
{
    if (fn_80408A94(s, "X%dY%d", col, row) != 2)
        return 0;
    if (g->b80_2) {
        if (*row >= g->m60)
            *row = *row + g->m70;
        if (*col >= g->m64)
            *col = *col + g->m74;
    }
    return 1;
}

extern "C" void fn_800CEA38(Unk800CB3A8_Grid* g)
{
    g->fn_vslot14(0);
    int old = g->m68;
    g->fn_vslot118(1);
    if (old != g->m68)
        g->fn_vslot107(0x99);
    g->fn_vslot120();
    g->fn_vslot14(1);
}

extern "C" void fn_800CEAF8(Unk800CB3A8_Grid* g)
{
    g->fn_vslot14(0);
    int old = g->m68;
    g->fn_vslot118(0);
    if (old != g->m68)
        g->fn_vslot107(0x9A);
    g->fn_vslot120();
    g->fn_vslot14(1);
}

extern "C" void fn_800CEBB8()
{
}

extern "C" void fn_800CEBBC(Unk800CB3A8_Grid* g, int a)
{
    if (g->b80_8)
        fn_8001BC4C()->fn_8001BDAC(a);
}

extern "C" void fn_800CEBFC(Unk800CB3A8_Grid* g, int id)
{
    char buf[100];
    int v = id;
    v = v == -1 ? g->m68 : v;
    if (g->b80_2 && v >= g->m60)
        v -= g->m70;
    fn_804089B4(buf, "Y%dFLASH", v);
    g->fn_vslot30(buf, 1);
}

extern "C" void fn_800CEC90(Unk800CB3A8_Grid* g, int a, int b, float t)
{
    fn_800C8118(g, (void*)a, (void*)b, t);
    if (g->b0) {
        int n = g->fn_vslot62();
        int k = (int)(t * (float)n);
        if (k < n) {
            for (int i = 0; i <= k; i++) {
                Unk800C7B40_Widget* c = g->fn_vslot124(i);
                if (c != 0 && !c->b3)
                    c->fn_vslot14(1);
            }
        }
        int m = g->fn_vslot63();
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++)
                g->fn_vslot73(i, j)->fn_vslot8(i <= k);
        }
    }
}

extern "C" void fn_800CEE28(Unk800CEE28_Obj* o, int v)
{
    fn_800C83B4(o, v);
    if (o->m78 != 0) {
        fn_800C80C0(o->m78->fn_vslot36(), v);
        fn_800C80C0(o->m78->fn_vslot37(), v);
        if (o->m78->fn_vslot31() != 0)
            fn_800C80C0(o->m78->fn_vslot31(), v);
    }
}

extern "C" Unk800D1614_Raw* fn_800CEEF0(Unk800D1614_Raw* self, int p1, int p2, int p3, int p4, int p5, int p6, int p7,
                                        int s1, int s2, int s3, int s4, float f)
{
    fn_800C7B40(self, p1, 3, p2, p3, p4, p5, p6, p7, s1, s3, s4, 0, f);
    self->m3C = lbl_8065CA08;
    self->m40 = 0;
    self->b7 = s2;
    return self;
}

extern "C" void fn_800CEF98(Unk800CB3A8_Grid* g)
{
    for (Unk800C7B40_Widget* c = g->m40; c != 0; c = c->m8)
        c->fn_vslot3();
}

extern "C" void fn_800CEFEC(Unk800CB3A8_Grid* g, int v)
{
    Unk800C7B40_Widget* c = g->m40;
    g->m2E = v;
    for (; c != 0; c = c->m8)
        c->fn_vslot18(v);
}

extern "C" void fn_800CF04C(Unk800CB3A8_Grid* g)
{
    Unk800C7B40_Widget* c = g->m40;
    if (g->b9) {
        int a = g->m26;
        int e = a + g->m2A;
        fn_80317120(1, AbsI(320 - a) > AbsI(320 - e) ? a : e);
    }
    for (; c != 0; c = c->m8) {
        if (c->b0)
            c->fn_vslot2();
    }
    if (g->b9)
        fn_80317120(0, 0);
}

void Unk800C7A4C_Group::fn_800CF114(int v)
{
    fn_800C7E38(this, v);
    for (Unk800C7B40_Widget* c = m40; c != 0; c = c->m8)
        c->fn_vslot14(v);
}
