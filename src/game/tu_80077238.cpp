/*
 * Game code, .text 0x80077238-0x8007A4DC (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_8007A4DC.h"
#include "game/clampiter.h"
int fn_80083FC0(int x);
void fn_800B555C(void);
extern "C" int fn_800B5758(void);
int fn_8009047C(int k, int e);
float fn_80090380(int k, int e);
extern float lbl_805B4070;

void fn_8009532C(Handle* h);
void fn_80095EE4(Handle* h);
void fn_80091574(Handle* h);
void fn_80095460(Handle* h);
void fn_80095FE4(Handle* h);
void fn_800915F0(Handle* h);
extern "C" void fn_80083448(Unk8007A4DC_Obj1E8* o, int a, unsigned short b);
extern "C" void fn_800835E8(Unk8007A4DC_Obj1E8* o, int a, unsigned short b);
void fn_80083CEC(Unk8007A4DC_Obj1E8* o);
int fn_80084114(void);
extern "C" int fn_80377E04(Obj* o, int* out, int id);
extern "C" int fn_80400CE4(int a);
extern "C" int fn_80377E04(Obj* o, int* out, int id);
void fn_800949B4(Handle* h, int a);
void fn_80095844(Handle* h, int a);
void fn_80377E90(Obj* list, int* a, int* b, int idx);
void* fn_8040B190(void* dst, const void* src, int n);
extern "C" int fn_80400CE4(int a);
extern Unk8007A4DC_Screen* lbl_8061930C;
extern int lbl_80619310;
extern float lbl_80619314;
extern const float lbl_805B4078;
extern "C" void fn_8059A3CC(void* p);
struct Unk80079240_Obj {
    char pad0[0x1C];
    void* m1C;
};

/* The seven roster-walk functions fn_80077238..fn_80077934 differ only in the handle (+0x17C /
 * +0x1C0) and the call. The id load needs the byte offset in its own variable: `m0[j]` (or
 * `m0 + j * 4`) is canonicalised to (plus (ashift j 2) m0) and gives `lwzx rD,off,base`. */
int Unk8007A4DC_Screen::fn_80077238(int a, int b)
{
    Unk805A19C8_Roster* r = m58;
    int total = 0;
    unsigned i = 0;
    unsigned char cnt = r->m10[fn_805A19C8(r, a)].m7;
    for (; i < cnt; i++) {
        unsigned char j = i;
        int k = fn_805A19C8(r, a);
        int o = j * 4;
        int id = *(int*)((char*)r->m10[k].m0 + o);
        int x;
        int f = fn_80377E04(fn_800A883C(&m17C), &x, id) != 0;
        if (f == 1)
            total += fn_800739E0(0, id, b, 2);
    }
    return total;
}

int Unk8007A4DC_Screen::fn_80077320(int a, int b)
{
    Unk805A19C8_Roster* r = m58;
    int total = 0;
    unsigned i = 0;
    unsigned char cnt = r->m10[fn_805A19C8(r, a)].m7;
    for (; i < cnt; i++) {
        unsigned char j = i;
        int k = fn_805A19C8(r, a);
        int o = j * 4;
        int id = *(int*)((char*)r->m10[k].m0 + o);
        int x;
        int f = fn_80377E04(fn_800A883C(&m17C), &x, id) != 0;
        if (f == 1)
            total += fn_80073710(1, id, b);
    }
    return total;
}

int Unk8007A4DC_Screen::fn_80077404(int a, int b)
{
    Unk805A19C8_Roster* r = m58;
    int total = 0;
    unsigned i = 0;
    unsigned char cnt = r->m10[fn_805A19C8(r, a)].m7;
    for (; i < cnt; i++) {
        unsigned char j = i;
        int k = fn_805A19C8(r, a);
        int o = j * 4;
        int id = *(int*)((char*)r->m10[k].m0 + o);
        int x;
        int f = fn_80377E04(fn_800A883C(&m17C), &x, id) != 0;
        if (f == 1)
            total += fn_8007455C(1, id, b);
    }
    return total;
}

float Unk8007A4DC_Screen::fn_800774E8(int a, int b)
{
    Unk805A19C8_Roster* r = m58;
    unsigned t0 = 0;
    unsigned t1 = 0;
    unsigned t2 = 0;
    unsigned i = 0;
    unsigned char cnt = r->m10[fn_805A19C8(r, a)].m7;
    for (; i < cnt; i++) {
        unsigned char j = i;
        int k = fn_805A19C8(r, a);
        int o = j * 4;
        int id = *(int*)((char*)r->m10[k].m0 + o);
        int x;
        int f = fn_80377E04(fn_800A883C(&m17C), &x, id) != 0;
        if (f == 1) {
            t0 += fn_8007455C(0, id, b);
            t1 += fn_8007455C(1, id, b);
            t2 += fn_8007455C(2, id, b);
        }
    }
    float tot = ((float)t0 + (float)t2) + (float)t1;
    float res = 0.0f;
    if (!(tot <= 0.0f))
        res = ((float)t0 + (float)t2) / tot;
    return res;
}

int Unk8007A4DC_Screen::fn_80077684(int a, int b)
{
    Unk805A19C8_Roster* r = m58;
    int total = 0;
    unsigned i = 0;
    unsigned char cnt = r->m10[fn_805A19C8(r, a)].m7;
    for (; i < cnt; i++) {
        unsigned char j = i;
        int k = fn_805A19C8(r, a);
        int o = j * 4;
        int id = *(int*)((char*)r->m10[k].m0 + o);
        int x;
        int f = fn_80377E04(fn_800A883C(&m1C0), &x, id) != 0;
        if (f == 1)
            total += fn_80075094(2, id, b, 2);
    }
    return total;
}

int Unk8007A4DC_Screen::fn_8007776C(int a, int b)
{
    Unk805A19C8_Roster* r = m58;
    int total = 0;
    unsigned i = 0;
    unsigned char cnt = r->m10[fn_805A19C8(r, a)].m7;
    for (; i < cnt; i++) {
        unsigned char j = i;
        int k = fn_805A19C8(r, a);
        int o = j * 4;
        int id = *(int*)((char*)r->m10[k].m0 + o);
        int x;
        int f = fn_80377E04(fn_800A883C(&m1C0), &x, id) != 0;
        if (f == 1)
            total += fn_80074754(7, id, b);
    }
    return total;
}

int Unk8007A4DC_Screen::fn_80077850(int a, int b)
{
    Unk805A19C8_Roster* r = m58;
    int total = 0;
    unsigned i = 0;
    unsigned char cnt = r->m10[fn_805A19C8(r, a)].m7;
    for (; i < cnt; i++) {
        unsigned char j = i;
        int k = fn_805A19C8(r, a);
        int o = j * 4;
        int id = *(int*)((char*)r->m10[k].m0 + o);
        int x;
        int f = fn_80377E04(fn_800A883C(&m1C0), &x, id) != 0;
        if (f == 1)
            total += fn_80074754(10, id, b);
    }
    return total;
}

int Unk8007A4DC_Screen::fn_80077934(int a, int b)
{
    Unk805A19C8_Roster* r = m58;
    int total = 0;
    unsigned i = 0;
    unsigned char cnt = r->m10[fn_805A19C8(r, a)].m7;
    for (; i < cnt; i++) {
        unsigned char j = i;
        int k = fn_805A19C8(r, a);
        int o = j * 4;
        int id = *(int*)((char*)r->m10[k].m0 + o);
        int x;
        int f = fn_80377E04(fn_800A883C(&m1C0), &x, id) != 0;
        if (f == 1)
            total += fn_80074754(11, id, b);
    }
    return total;
}

float Unk8007A4DC_Screen::fn_80077A18(int a, int b)
{
    Unk805A19C8_Roster* r = m58;
    unsigned t0 = 0;
    unsigned t1 = 0;
    unsigned t2 = 0;
    unsigned i = 0;
    unsigned char cnt = r->m10[fn_805A19C8(r, a)].m7;
    for (; i < cnt; i++) {
        unsigned char j = i;
        int k = fn_805A19C8(r, a);
        int o = j * 4;
        int id = *(int*)((char*)r->m10[k].m0 + o);
        int x;
        int f = fn_80377E04(fn_800A883C(&m1C0), &x, id) != 0;
        if (f == 1) {
            t0 += fn_80074754(1, id, b);
            t1 += fn_80074754(2, id, b);
            t2 += fn_80074754(6, id, b);
        }
    }
    float den = (float)t0 + (float)t1 * 0.33333334f;
    float res = 0.0f;
    if (t0 != 0 || t1 != 0)
        res = (float)t2 * 9.0f / den;
    return res;
}

static inline unsigned Cnt4C(Handle* h)
{
    return fn_800A883C(h) ? fn_800A883C(h)->m4 : 0;
}

extern "C" int fn_80077BC4(Unk8007A4DC_Screen* self, int key)
{
    unsigned n = Cnt4C(&self->m4C);
    unsigned i;
    for (i = 0; i < n; i++) {
        int id = self->C2()->fn_8007290C(i);
        if (self->C2()->fn_800757B8(id) == 1) {
            unsigned c = self->fn_8007F198(id);
            for (unsigned j = 0; j < c; j++) {
                if (self->fn_8007F24C(id, (unsigned char)j) == key)
                    return id;
            }
            for (unsigned k = 0; k <= 1; k++) {
                unsigned m = self->fn_8007F674(id, k);
                for (unsigned j = 0; j < m; j++) {
                    if (self->fn_8007F6C4(id, k, j) == key)
                        return id;
                }
            }
        }
    }
    return 0;
}

void Unk8007A4DC_Screen::fn_80077CF8()
{
    fn_8009532C(&m17C);
    fn_80095EE4(&m1C0);
    fn_80091574(&m4C);
}

void Unk8007A4DC_Screen::fn_80077D38(int a, int b)
{
    fn_80083448(m1E8, a, b);
}

void Unk8007A4DC_Screen::fn_80077D60(int a, int b)
{
    fn_800835E8(m1E8, a, b);
}

void Unk8007A4DC_Screen::fn_80077D88()
{
    fn_80083CEC(m1E8);
}

void Unk8007A4DC_Screen::fn_80077DAC()
{
    fn_80095460(&m17C);
    fn_80095FE4(&m1C0);
    fn_800915F0(&m4C);
}

extern "C" int fn_800787E8(Unk8007B2B0_Screen* g, int kind, int p)
{
    int r = 0;
    switch (kind) {
    case 0: {
        r = g->fn_8007C514(0x12, p);
        r += g->fn_8007C514(0x13, p);
        r += g->fn_8007C514(0x14, p);
        r += g->fn_8007C514(0x15, p);
        r = r / 4;
        break;
    }
    case 1: {
        r = g->fn_8007C770(0xD, p);
        r += g->fn_8007C770(0xE, p);
        r += g->fn_8007C770(0xF, p);
        r += g->fn_8007C770(0x10, p);
        r = r / 4;
        break;
    }
    case 2: {
        r = g->fn_8007C994(0xD, p);
        r += g->fn_8007C994(0xE, p);
        r += g->fn_8007C994(0xF, p);
        r += g->fn_8007C994(0x10, p);
        r += g->fn_8007C994(0x11, p);
        r = r / 5;
        break;
    }
    case 3: {
        r = g->fn_8007CBB8(0xA, p);
        r += g->fn_8007CBB8(0xB, p);
        r += g->fn_8007CBB8(0xC, p);
        r += g->fn_8007CBB8(0xD, p);
        r += g->fn_8007CBB8(0xE, p);
        r = r / 5;
        break;
    }
    case 4: {
        r = g->fn_8007CDDC(0xA, p);
        r += g->fn_8007CDDC(0xB, p);
        r += g->fn_8007CDDC(0xC, p);
        r += g->fn_8007CDDC(0xD, p);
        r = r / 4;
        break;
    }
    }
    return r;
}

int Unk8007B2B0_Screen::fn_80078A38(int a)
{
    fn_8007B8C4(0, a);
    return fn_80084114();
}

int Unk8007A4DC_Screen::fn_80078A64(int a, int b, int c)
{
    unsigned n = fn_8007F674(a, b);
    unsigned i;
    for (i = 0; i < n; i++) {
        if (fn_8007F6C4(a, b, i) == c)
            return 1;
    }
    return 0;
}

void Unk8007A4DC_Screen::fn_80078AE0(int a, int* o1, int* o2, int* o3, int* o4)
{
    *o1 = 0;
    *o2 = 0;
    *o3 = 0;
    *o4 = 0;
    unsigned i = 0;
    unsigned n = fn_8007F198(a);
    for (; i < n; i++) {
        int e = fn_8007F24C(a, (unsigned char)i);
        int p = fn_8007A6A0(0x1E, e);
        int q = fn_8007A6A0(0x1F, e);
        int r = fn_8007A270(e);
        int f = 0;
        if (p != 0 && q > 0)
            f = 1;
        if (r != 0) {
            if (f)
                (*o1)++;
            else
                (*o2)++;
        } else {
            if (f)
                (*o3)++;
            else
                (*o4)++;
        }
    }
}

void Unk8007A4DC_Screen::fn_80078C04(int a, int* o1, int* o2, int* o3, int* o4)
{
    int t[4];
    *o1 = 0;
    *o2 = 0;
    *o3 = 0;
    *o4 = 0;
    t[0] = 0;
    t[1] = 0;
    t[2] = 0;
    t[3] = 0;
    Unk8006A138_It<3> it(0);
    int j = 0;
    while (j <= 3) {
        int k = C2()->fn_8007B320(fn_80083FC0(j), a);
        fn_80078AE0(k, &t[0], &t[1], &t[2], &t[3]);
        *o1 += t[0];
        *o2 += t[1];
        *o3 += t[2];
        *o4 += t[3];
        j = it.Next();
    }
}

int Unk8007A4DC_Screen::fn_80078D00(int a)
{
    int sp8 = 0;
    int spC = 0;
    int sp10 = 0;
    int sp14 = 0;
    fn_80078AE0(a, &sp8, &spC, &sp10, &sp14);
    int r = spC > 6;
    if (sp14 <= 9)
        r = 0;
    int i = 0;
    do {
        if (fn_8007F674(a, i) > 1)
            r = 0;
        i++;
    } while (i <= 1);
    return r;
}

int fn_80078DA4(void* self, int a, int b, int c)
{
    if (a == 10)
        a = 0;
    int f = 0;
    if ((c == 0 && a == 0) || (c == 1 && a == 1) || c == 2)
        f = 1;
    if (f == 0) {
        if (b != 0)
            return 3;
        return 1;
    }
    if (b == 0)
        return 0;
    return 2;
}

int fn_8007900C(void* self, int x)
{
    Unk8007A4DC_Screen* o = lbl_8061930C;
    int cnt = 0;
    unsigned i = 0;
    while (i < o->fn_8007F198(x)) {
        int e = o->fn_8007F24C(x, (unsigned char)i);
        if (e != 0 && o->fn_8007A270(e) != 0)
            cnt++;
        i++;
    }
    return cnt;
}

int Unk8007B2B0_Screen::fn_80079094(int a, int b, int c, int d, int e, int kind)
{
    int arr[4];
    int r;
    if (kind == 3) {
        arr[0] = fn_8007B320(3, b);
        arr[1] = fn_8007B320(2, b);
        arr[2] = fn_8007B320(1, b);
        arr[3] = fn_8007B320(0, b);
    } else if (kind == 2) {
        arr[0] = fn_8007B320(2, b);
        arr[1] = fn_8007B320(1, b);
        arr[2] = fn_8007B320(3, b);
        arr[3] = fn_8007B320(0, b);
    } else if (kind == 1) {
        arr[0] = fn_8007B320(1, b);
        arr[1] = fn_8007B320(0, b);
        arr[2] = fn_8007B320(2, b);
        arr[3] = fn_8007B320(3, b);
    } else {
        arr[0] = fn_8007B320(0, b);
        arr[1] = fn_8007B320(1, b);
        arr[2] = fn_8007B320(2, b);
        arr[3] = fn_8007B320(3, b);
    }
    Unk8006A138_It<3> it(0);
    int j = 0;
    do {
        r = fn_80079264(a, arr[j], c, d, e);
        j = it.Next();
    } while (j <= 3 && r != 0);
    return r;
}

void fn_80079240(Unk80079240_Obj* o)
{
    fn_8059A3CC(o->m1C);
}

void Unk8007B2B0_Screen::fn_800798BC(int id, Unk800798BC_P* a, Unk800798BC_P* b, Unk800798BC_P* c)
{
    C1()->fn_8007A808(0x35, id, (a->m4 >= b->m4) * 2);
    int v = 1;
    if (!(a->m0 < b->m0))
        v = 2;
    else if (!(a->m0 > c->m0))
        v = 0;
    C1()->fn_8007A808(0x34, id, v);
    int d = c->m8 - a->m8;
    int lo = -2;
    if (lo < d)
        lo = d;
    int hi = lo;
    if (hi > 2)
        hi = 2;
    int t[5] = { 0, 1, 2, 3, 4 };
    C1()->fn_8007A808(0x36, id, t[hi + 2]);
}

float Unk8007B2B0_Screen::fn_800799D0(int n)
{
    float v = m1F4;
    v = (float)n * m1F8 + v;
    return v;
}

float Unk8007B2B0_Screen::fn_80079A1C()
{
    return fn_800799D0(C1()->fn_8007A550(0x18));
}

int Unk8007B2B0_Screen::fn_80079A58(float f)
{
    f = f - m1F4;
    return (int)(f / m1F8);
}

float Unk8007B2B0_Screen::fn_80079A90(int n)
{
    float v = m1FC;
    v = (float)n * m200 + v;
    return v;
}

int Unk8007B2B0_Screen::fn_80079ADC(float f)
{
    f = f - m1FC;
    return (int)(f / m200);
}

float Unk8007B2B0_Screen::fn_80079B14(int a)
{
    float r = fn_80079F54(fn_8007B3E0(0x15, a));
    return r;
}

float Unk8007B2B0_Screen::fn_80079B5C(int a, int kind)
{
    int sel;
    if (kind != 0) {
        if (kind != 1) {
            sel = 0x19;
            if (kind == 2)
                sel = 0x18;
        } else {
            sel = 0x17;
        }
    } else {
        sel = 0x16;
    }
    float r = (float)fn_8007B3E0(sel, a) * m204;
    return r;
}

float Unk8007B2B0_Screen::fn_80079BF8(int a)
{
    fn_800B555C();
    if (fn_800B5758() == 0)
        return lbl_805B4070;
    Unk8007B2B0_Screen* o = lbl_8061930C->C2();
    float sum = lbl_805B4070;
    Unk8006A138_It<4> it(0);
    int idx = 0;
    while (idx <= 4) {
        int n = fn_8008FBE8(idx);
        for (int i = 0; i < n; i++) {
            int e = o->fn_8007B380(fn_8008FE14(fn_8008FC40(it.v, i)), a);
            if (e != 0 && fn_8009047C(it.v, e) > 0)
                sum += fn_80090380(it.v, e);
        }
        idx = it.Next();
    }
    return sum;
}

float Unk8007B2B0_Screen::fn_80079E74(int a)
{
    if (lbl_80619310 == 0) {
        float f = lbl_805B4078;
        f += fn_80079CF8(a);
        f += fn_80079BF8(a);
        return f;
    }
    return lbl_80619314;
}

float Unk8007B2B0_Screen::fn_80079EFC(int a)
{
    float x = fn_80079B14(a);
    float y = fn_80079E74(a);
    float r = x - y;
    return r;
}

/* Both fields are read through an inline accessor: an operand that is an inline call is loaded when
 * it is expanded, which gives retail's m204-before-conversion, m1F4-after load order. */
inline float Rd79F54(const float* p) { return *p; }
float Unk8007B2B0_Screen::fn_80079F54(int n)
{
    return Rd79F54(&m1F4) + (float)n * Rd79F54(&m204);
}

int Unk8007B2B0_Screen::fn_80079F90(float f)
{
    return (int)((f - m1F4) / m204);
}

void fn_80079FC0(void* self, char** cursor, void* dst, int n)
{
    fn_8040B190(dst, *cursor, n);
    *cursor += n;
}

void Unk8007A4DC_Screen::fn_8007A008(int a)
{
    int x;
    Handle* h = &m17C;
    int f = fn_80377E04(fn_800A883C(h), &x, a) != 0;
    if (f == 1)
        fn_800949B4(h, a);
}

void Unk8007A4DC_Screen::fn_8007A06C(int a)
{
    int x;
    Handle* h = &m1C0;
    int f = fn_80377E04(fn_800A883C(h), &x, a) != 0;
    if (f == 1)
        fn_80095844(h, a);
}

int Unk8007B2B0_Screen::fn_8007A0D0(int idx)
{
    int x, y;
    int* px = &x;
    int* py = &y;
    fn_80377E90(fn_800A883C(&m4C), px, py, idx);
    return x;
}

int Unk8007A4DC_Screen::fn_8007A11C(int idx)
{
    int x, y;
    int* px = &x;
    int* py = &y;
    fn_80377E90(fn_800A883C(&m17C), px, py, idx);
    return x;
}

int Unk8007A4DC_Screen::fn_8007A168(int idx)
{
    int x, y;
    int* px = &x;
    int* py = &y;
    fn_80377E90(fn_800A883C(&m1C0), px, py, idx);
    return x;
}

int Unk8007A4DC_Screen::fn_8007A1B4(int a)
{
    return fn_80071814(m1C8, a);
}

int Unk8007A4DC_Screen::fn_8007A1D8(int a)
{
    int x;
    return fn_80377E04(fn_800A883C(&m17C), &x, a) != 0;
}

int Unk8007A4DC_Screen::fn_8007A224(int a)
{
    int x;
    return fn_80377E04(fn_800A883C(&m1C0), &x, a) != 0;
}

int Unk8007A4DC_Screen::fn_8007A270(int a)
{
    int r = fn_8007A6A0(5, a);
    int x = 0;
    if (r == 0 || r == 10)
        x = 1;
    return x;
}

/* Retail treats the vcall thunk as returning nothing (sched2 argument-move order); the member declares int. */
void Unk8007A4DC_Screen::fn_8007A2B4(int flags)
{
    unsigned i = 0;
    unsigned n = fn_8007B1EC();
    int v = fn_8007A550(0x1D);
    for (; i < n; i++) {
        if ((i & 0x1F) == 0)
            fn_80400CE4(0);
        if (flags & 1) {
            fn_8007A94C(0x1D, i, v);
            fn_8007A94C(0x1C, i, 0);
        }
        if (flags & 2) {
            fn_8007A94C(0x1E, i, 0);
            fn_8007A94C(0x1F, i, 0);
        }
    }
}

void Unk8007A4DC_Screen::fn_8007A380()
{
    unsigned n = fn_8007B238();
    unsigned i;
    for (i = 0; i < n; i++) {
        if ((i & 0x1F) == 0)
            fn_80400CE4(0);
        fn_8007ACFC(0x1B, i, fn_8007ACC4(0, i));
    }
}

float Unk8007A4DC_Screen::fn_8007A3FC(int a, int b)
{
    W* w = m184;
    return fn_8007189C(w, a, fn_80071814(w, b));
}

float Unk8007A4DC_Screen::fn_8007A444(int a, int b)
{
    W* w = m1C8;
    return fn_8007189C(w, a, fn_80071814(w, b));
}

float Unk8007A4DC_Screen::fn_8007A48C(int a, int b, int i)
{
    W* w = m188[i];
    return fn_8007189C(w, a, fn_80071814(w, b));
}
