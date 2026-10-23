/*
 * Database overlay code, .database_codeoverlay 0x80535360-0x8053967C (code-only unit;
 * placeholder TU). Define every function DB_OVERLAY (game/overlay.h); data extern by lbl_ name.
 */
#include "game/overlay.h"
#include "game/tu_8052A220.h"
#include "game/tu_8007F198.h"
#include "game/clampiter.h"

struct Unk8007DB74_A;

extern Unk8007A4DC_Screen* lbl_8061930C;
int fn_80076644(int a, int b, int c);
unsigned fn_800765A8(int x);
extern "C" int fn_80077BC4(void* g, int a);
extern Unk80538DD8_G* lbl_80619948;
extern float lbl_805FEC40;
extern const char* lbl_80622A4C[];
extern "C" int fn_80400CE4(int a);
extern "C" float fn_8007824C(int a, int b);
int fn_8007F674(Unk8007F198_A* self, int a, int x);
int fn_8007F6C4(Unk8007F198_A* self, int a, int x, int y);
int fn_8007E7F0(Unk8007DB74_A* self, int a, int b);
int fn_8007EAF0(Unk8007DB74_A* self, int a, int b);
int fn_8007EDF0(Unk8007DB74_A* self, int a, int b);
int fn_8007E48C(Unk8007DB74_A* self, int a, int b);
int fn_8007F0F0(Unk8007DB74_A* self, int a, int b);
int fn_8007E0C0(Unk8007DB74_A* self, int a, int target, int c);
int fn_8007DED4(Unk8007DB74_A* self, int a, int target, int c);
extern "C" int fn_8040B574(const char* a, const char* b);
struct Unk8055B0D8_Obj;
extern "C" Unk8055B0D8_Obj* fn_8055B0D8(void* mem, Unk805352B0_List* list, Unk8007A4DC_Screen* sc);
extern "C" void fn_8055B260(Unk8055B0D8_Obj* o);
extern "C" void fn_8055B1F4(Unk8055B0D8_Obj* o, int chrg);
extern "C" void fn_80535954(Unk805352B0_List* self, int x, int k);
extern "C" void fn_80535650(Unk805352B0_List* self, int x);
extern "C" void fn_80538CD8(Unk805352B0_List* dst, int want, Unk805352B0_List* src);
extern "C" void fn_80536D68(Unk805352B0_List* self, int x);
extern "C" void fn_80536E3C(Unk805352B0_List* self, int x);
inline void Append(Unk805352B0_List* dst, Unk80535650_Tmp* src)
{
    unsigned i;
    for (i = 0; i < src->Count(); i++)
        dst->Push(src->At(i));
    dst->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80535360(Unk805352B0_List* self, int x)
{
    Unk8007B2B0_Screen* sc = (Unk8007B2B0_Screen*)lbl_8061930C;
    Unk80535650_Tmp t;
    int buf[0x78];
    Unk80535650_Tmp* q = &t;
    int z = 0;
    fn_8040074C(self->m0, self->m8 << 2);
    self->m4 = z;
    self->mC = z;
    self->m14 = z;
    q->C1(buf, 0x78);
    q->C2();
    q->C3(lbl_8064A440);
    int g1 = sc->fn_8007B320(0, x);
    int g2 = sc->fn_8007B320(1, x);
    int g3 = sc->fn_8007B320(2, x);
    int g4 = sc->fn_8007B320(3, x);
    fn_80535650((Unk805352B0_List*)&t, g1);
    Append(self, &t);
    if (g2) {
        fn_80535650((Unk805352B0_List*)&t, g2);
        Append(self, &t);
    }
    if (g3) {
        fn_80535650((Unk805352B0_List*)&t, g3);
        Append(self, &t);
    }
    if (g4) {
        fn_80535650((Unk805352B0_List*)&t, g4);
        Append(self, &t);
    }
    self->m14 = 1;
    t.D();
}

extern "C" void fn_805366FC(Unk805352B0_List* self, int i, int last, int mode);
extern "C" void fn_80535764(Unk805352B0_List* self, int x);
extern "C" void fn_80535820(Unk805352B0_List* self, int x);
extern "C" void fn_80535F84(Unk805352B0_List* self, int a, int p, int q);
int fn_80083FC0(int k);
extern "C" void fn_8053DF70(void* o, int a, int b, float f);
extern float lbl_805FEC44;
extern "C" void fn_8053DE38(void* o, int a, int b, float f);
unsigned char fn_8007F198(Unk8007F198_A* self, int x);
int fn_8007F24C(Unk8007F198_A* self, int a, int x);

extern "C" DB_OVERLAY void fn_80535650(Unk805352B0_List* self, int x)
{
    fn_80535764(self, x);
    Unk80535650_Tmp t;
    int buf[0x28];
    Unk80535650_Tmp* q = &t;
    q->C1(buf, 0x28);
    q->C2();
    q->C3(lbl_8064A4C8);
    fn_80535820((Unk805352B0_List*)q, x);
    unsigned i;
    for (i = 0; i < q->Count(); i++)
        self->Push(q->At(i));
    q->D();
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80535764(Unk805352B0_List* self, int x)
{
    unsigned i;
    fn_8040074C(self->m0, self->m8 << 2);
    Unk8007F198_A* c = (Unk8007F198_A*)lbl_8061930C;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    unsigned n = fn_8007F198(c, x);
    for (i = 0; i < n; i++) {
        int v = fn_8007F24C(c, x, (unsigned char)i);
        self->Push(v);
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80535820(Unk805352B0_List* self, int x)
{
    unsigned k;
    fn_8040074C(self->m0, self->m8 << 2);
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    for (k = 0; k <= 1; k++) {
        Unk80535650_Tmp t;
        int buf[0x28];
        Unk80535650_Tmp* q = &t;
        q->C1(buf, 0x28);
        q->C2();
        q->C3(lbl_8064A4C8);
        fn_80535954((Unk805352B0_List*)q, x, k);
        unsigned n = q->Count();
        unsigned i;
        for (i = 0; i < n; i++)
            self->Push(t.At(i));
        q->D();
    }
}

extern "C" DB_OVERLAY void fn_80535954(Unk805352B0_List* self, int x, int k)
{
    unsigned i;
    fn_8040074C(self->m0, self->m8 << 2);
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    unsigned n = fn_8007F674((Unk8007F198_A*)sc, x, k);
    for (i = 0; i < n; i++) {
        int v = fn_8007F6C4((Unk8007F198_A*)sc, x, k, i);
        self->Push(v);
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80535A1C(Unk805352B0_List* self, int x)
{
    Unk8007B2B0_Screen* sc = (Unk8007B2B0_Screen*)lbl_8061930C;
    Unk80535650_Tmp t;
    int buf[0x78];
    Unk80535650_Tmp* q = &t;
    int z = 0;
    fn_8040074C(self->m0, self->m8 << 2);
    self->m4 = z;
    self->mC = z;
    self->m14 = z;
    q->C1(buf, 0x78);
    q->C2();
    q->C3(lbl_8064A440);
    int g1 = sc->fn_8007B320(1, x);
    int g2 = sc->fn_8007B320(2, x);
    int g3 = sc->fn_8007B320(3, x);
    fn_80535650((Unk805352B0_List*)&t, g1);
    Append(self, &t);
    fn_80535650((Unk805352B0_List*)&t, g2);
    Append(self, &t);
    fn_80535650((Unk805352B0_List*)&t, g3);
    Append(self, &t);
    q->D();
}

extern "C" DB_OVERLAY void fn_80535C64(Unk805352B0_List* self, int x)
{
    Unk8007B2B0_Screen* sc = (Unk8007B2B0_Screen*)lbl_8061930C;
    Unk80535650_Tmp t;
    int buf[0x78];
    Unk80535650_Tmp* q = &t;
    int z = 0;
    fn_8040074C(self->m0, self->m8 << 2);
    self->m4 = z;
    self->mC = z;
    self->m14 = z;
    q->C1(buf, 0x78);
    q->C2();
    q->C3(lbl_8064A440);
    int g1 = sc->fn_8007B320(0, x);
    int g2 = sc->fn_8007B320(1, x);
    int g3 = sc->fn_8007B320(2, x);
    fn_80535650((Unk805352B0_List*)&t, g1);
    Append(self, &t);
    fn_80535650((Unk805352B0_List*)&t, g2);
    Append(self, &t);
    fn_80535650((Unk805352B0_List*)&t, g3);
    Append(self, &t);
    q->D();
}

extern "C" DB_OVERLAY void fn_80535EAC(Unk805352B0_List* self)
{
    unsigned i;
    fn_8040074C(self->m0, self->m8 << 2);
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    for (i = 0; i <= 29; i++) {
        unsigned h = fn_800736EC(lbl_80622A4C[i]);
        int v = ((Unk8007B2B0_Screen*)lbl_8061930C)->fn_80075948(h);
        self->Push(v);
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80535F5C(Unk805352B0_List* self, int a)
{
    fn_80535F84(self, a, 1, 1);
}

extern "C" DB_OVERLAY void fn_80535F84(Unk805352B0_List* self, int a, int p, int q)
{
    fn_8040074C(self->m0, self->m8 << 2);
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    Unk8006A138_It<3> it(0);
    int k = 0;
    do {
        int g = ((Unk8007B2B0_Screen*)sc)->fn_8007B320(fn_80083FC0(k), a);
        unsigned n = fn_8007F198((Unk8007F198_A*)sc, g);
        unsigned i;
        for (i = 0; i < n; i++) {
            int v = fn_8007F24C((Unk8007F198_A*)sc, g, (unsigned char)i);
            int t = sc->fn_8007A270(v);
            if ((p != 0 && t != 0) || (q != 0 && t == 0))
                self->Push(v);
        }
        k = it.Next();
    } while (k <= 3);
    self->m14 = 1;
}

extern "C" void fn_805385B8(Unk805352B0_List* self, int x);
extern "C" DB_OVERLAY void fn_805360BC(Unk805352B0_List* self, int x)
{
    fn_8040074C(self->m0, self->m8 << 2);
    Unk80535650_Tmp a;
    int bufa[0x78];
    Unk80535650_Tmp* qa = &a;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    unsigned i = 0;
    qa->C1(bufa, 0x78);
    qa->C2();
    qa->C3(lbl_8064A440);
    Unk80535650_Tmp b;
    int bufb[0x28];
    Unk80535650_Tmp* qb = &b;
    b.m0 = bufb;
    b.vp = lbl_8064B298;
    qb->m8 = 0x28;
    fn_8040074C(b.m0, b.m8 << 2);
    qb->m4 = 0;
    qb->mC = 0;
    b.vp = lbl_8064A550;
    fn_8040074C(b.m0, b.m8 << 2);
    qb->m4 = 0;
    qb->mC = 0;
    qb->m14 = 0;
    b.vp = lbl_8064A4C8;
    fn_80535650((Unk805352B0_List*)qa, x);
    fn_805385B8((Unk805352B0_List*)qb, x);
    for (; i < a.Count(); i++) {
        unsigned j = 0;
        int found = 0;
        for (; j < b.Count(); j++) {
            if (b.At(j) == a.At(i)) {
                found = 1;
                break;
            }
        }
        if (found == 0)
            self->Push(a.At(i));
    }
    self->m14 = 1;
    b.D();
    a.D();
}

extern "C" DB_OVERLAY void fn_805362AC(Unk805352B0_List* self, int mode)
{
    float keys[3250];
    unsigned n = self->m4;
    unsigned i;
    for (i = 0; i < n; i++) {
        if ((i & 0x3F) == 0)
            fn_80400CE4(0);
        keys[i] = fn_8007824C(self->At(i), 0);
    }
    unsigned m = self->m4;
    float* k = keys;
    unsigned x;
    for (x = 0; x < m; x++) {
        unsigned y;
        for (y = x + 1; y < m; y++) {
            if ((mode == 1 && k[x] < k[y]) || (mode == 0 && !(k[x] <= k[y]))) {
                float t = k[x];
                k[x] = k[y];
                k[y] = t;
                int b = self->At(x);
                int a = self->At(y);
                self->Set(x, a);
                self->Set(y, b);
            }
        }
    }
}

extern "C" DB_OVERLAY int fn_805363FC(int k)
{
    switch (k) {
    case 1:
        return 0;
    case 2:
        return 1;
    case 3:
        return 2;
    case 5:
        return 3;
    case 4:
        return 4;
    case 12:
        return 5;
    case 6:
        return 6;
    case 7:
        return 7;
    case 8:
        return 8;
    case 11:
        return 9;
    case 13:
        return 10;
    case 0:
        return 11;
    case 10:
        return 12;
    default:
        return 13;
    }
}

extern "C" DB_OVERLAY void fn_805364E0(Unk805352B0_List* self, int mode)
{
    int keys[3250];
    unsigned n = self->m4;
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    unsigned i;
    for (i = 0; i < n; i++)
        keys[i] = fn_805363FC(sc->fn_8007A6A0(5, self->At(i)));
    unsigned m = self->m4;
    int* k = keys;
    unsigned x;
    for (x = 0; x < m; x++) {
        unsigned y;
        for (y = x + 1; y < m; y++) {
            if ((mode == 1 && k[x] < k[y]) || (mode == 0 && k[x] > k[y])) {
                int t = k[x];
                k[x] = k[y];
                k[y] = t;
                int b = self->At(x);
                int a = self->At(y);
                self->Set(x, a);
                self->Set(y, b);
            }
        }
    }
}

extern "C" DB_OVERLAY void fn_80536624(Unk805352B0_List* self, int mode)
{
    int n = self->m4;
    if (n > 1) {
        int i;
        for (i = n / 2 - 1; i >= 0; i--)
            fn_805366FC(self, i, n - 1, mode);
        for (i = n - 1; i > 0; i--) {
            int x = self->At(0);
            int y = self->At(i);
            self->Set(0, y);
            self->Set(i, x);
            fn_805366FC(self, 0, i - 1, mode);
        }
    }
}

extern "C" char* fn_8040B448(char* dst, const char* src);
extern "C" int fn_8040B6A0(const char* s);
extern "C" int fn_8040B3B4(const char* a, const char* b);
extern const char lbl_805FEC38[];

/* near-miss: control flow and call sequence match; retail keeps the chosen child in its own
 * register (r23) apart from the loop index (r31, reused for the second key id), ours folds
 * them together so the register numbers and the c == last branch layout differ. */
extern "C" DB_OVERLAY void fn_805366FC(Unk805352B0_List* self, int root, int last, int mode)
{
    char bufA[0x80];
    char bufB[0x80];
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    int j = root;
    int done = 0;
    unsigned c;
    for (c = j * 2; c <= last && !done; c = j * 2) {
        unsigned child;
        if (c == last) {
            child = c;
        } else {
            int a = sc->fn_8007A11C(self->At(c));
            sc->fn_8007A914(1, a, bufA);
            fn_8040B448(bufA, lbl_805FEC38);
            sc->fn_8007A914(0, a, bufA + fn_8040B6A0(bufA));
            int b = sc->fn_8007A11C(self->At(c + 1));
            sc->fn_8007A914(1, b, bufB);
            fn_8040B448(bufB, lbl_805FEC38);
            sc->fn_8007A914(0, b, bufB + fn_8040B6A0(bufB));
            int r = fn_8040B3B4(bufA, bufB);
            if ((mode == 1 && r < 0) || (mode == 0 && r > 0))
                child = j * 2;
            else
                child = j * 2 + 1;
        }
        int a = sc->fn_8007A11C(self->At(j));
        sc->fn_8007A914(1, a, bufA);
        fn_8040B448(bufA, lbl_805FEC38);
        sc->fn_8007A914(0, a, bufA + fn_8040B6A0(bufA));
        int b = sc->fn_8007A11C(self->At(child));
        sc->fn_8007A914(1, b, bufB);
        fn_8040B448(bufB, lbl_805FEC38);
        sc->fn_8007A914(0, b, bufB + fn_8040B6A0(bufB));
        int r = fn_8040B3B4(bufB, bufA);
        if ((mode == 1 && r < 0) || (mode == 0 && r > 0)) {
            int x = self->At(j);
            int y = self->At(child);
            self->Set(j, y);
            self->Set(child, x);
            j = child;
        } else {
            done = 1;
        }
    }
}

extern "C" DB_OVERLAY void fn_80536990(Unk805352B0_List* self, int mode)
{
    int keys[3250];
    unsigned n = self->m4;
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    unsigned i;
    for (i = 0; i < n; i++)
        keys[i] = (int)((Unk80536990_Screen*)sc)->fn_8007A768(3, self->At(i));
    unsigned m = self->m4;
    int* k = keys;
    unsigned x;
    for (x = 0; x < m; x++) {
        unsigned y;
        for (y = x + 1; y < m; y++) {
            if ((mode == 1 && k[x] < k[y]) || (mode == 0 && k[x] > k[y])) {
                int t = k[x];
                k[x] = k[y];
                k[y] = t;
                int b = self->At(x);
                int a = self->At(y);
                self->Set(x, a);
                self->Set(y, b);
            }
        }
    }
}

extern "C" DB_OVERLAY void fn_80536ADC(Unk805352B0_List* self, float* k, int mode, int limit)
{
    int up = 0;
    if (mode != 0)
        up = 1;
    unsigned m = self->m4;
    unsigned x;
    for (x = 0; x < m; x++) {
        unsigned y;
        for (y = x + 1; y < m; y++) {
            if ((up == 1 && k[x] < k[y]) || (up == 0 && !(k[x] <= k[y]))) {
                float t = k[x];
                k[x] = k[y];
                k[y] = t;
                int b = self->At(x);
                int a = self->At(y);
                self->Set(x, a);
                self->Set(y, b);
            }
        }
    }
    int cnt = self->m4;
    while (cnt > limit) {
        self->RemoveAt(limit);
        cnt--;
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80536C34(Unk805352B0_List* self, int a)
{
    fn_80535F84(self, a, 1, 0);
}

extern "C" DB_OVERLAY void fn_80536C5C(Unk805352B0_List* self, int x)
{
    fn_80536D68(self, x);
    Unk80535650_Tmp t;
    int buf[0x28];
    Unk80535650_Tmp* q = &t;
    q->C1(buf, 0x28);
    q->C2();
    q->C3(lbl_8064A4C8);
    fn_80536E3C((Unk805352B0_List*)q, x);
    unsigned i;
    for (i = 0; i < q->Count(); i++)
        self->Push(t.At(i));
    t.vp = lbl_8064A550;
}

extern "C" DB_OVERLAY void fn_80536D68(Unk805352B0_List* self, int x)
{
    unsigned i;
    fn_8040074C(self->m0, self->m8 << 2);
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    unsigned n = fn_8007F198((Unk8007F198_A*)sc, x);
    for (i = 0; i < n; i++) {
        int v = fn_8007F24C((Unk8007F198_A*)sc, x, (unsigned char)i);
        if (sc->fn_8007A270(v) == 1)
            self->Push(v);
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80536E3C(Unk805352B0_List* self, int x)
{
    unsigned j;
    fn_8040074C(self->m0, self->m8 << 2);
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    for (j = 0; j <= 1; j++) {
        unsigned n = fn_8007F674((Unk8007F198_A*)sc, x, j);
        unsigned i;
        for (i = 0; i < n; i++) {
            int v = fn_8007F6C4((Unk8007F198_A*)sc, x, j, i);
            if (sc->fn_8007A270(v) == 1)
                self->Push(v);
        }
    }
    self->m14 = 1;
}

/* near-miss: structure matches (75 insns); the callee-saved registers are permuted (retail self r31,
 * y r28, bufB pointer r30; ours self r30, y r26, bufB r31). */
extern "C" DB_OVERLAY void fn_80536F28(Unk805352B0_List* self, int mode)
{
    char bufA[0x80];
    char bufB[0x80];
    unsigned n = self->m4;
    Unk8007B2B0_Screen* sc = (Unk8007B2B0_Screen*)lbl_8061930C;
    unsigned x;
    for (x = 0; x < n; x++) {
        unsigned y;
        for (y = 0; y < n - x - 1;) {
            unsigned yp = y + 1;
            sc->fn_8007B924(1, self->At(y), bufA);
            sc->fn_8007B924(1, self->At(yp), bufB);
            if (mode != 0 ? fn_8040B574(bufA, bufB) >= 0 : fn_8040B574(bufA, bufB) > 0) {
                int b = self->At(y);
                int a = self->At(yp);
                self->Set(y, a);
                self->Set(yp, b);
            }
            y = yp;
        }
    }
}

/* near-miss: same shape as fn_80536F28 with a second key; registers differ the same way. */
extern "C" DB_OVERLAY void fn_80537054(Unk805352B0_List* self, int mode)
{
    char bufA[0x80];
    char bufB[0x80];
    unsigned n = self->m4;
    Unk8007B2B0_Screen* sc = (Unk8007B2B0_Screen*)lbl_8061930C;
    unsigned x;
    for (x = 0; x < n; x++) {
        unsigned y;
        for (y = 0; y < n - x - 1;) {
            unsigned yp = y + 1;
            sc->fn_8007B924(3, self->At(y), bufA);
            sc->fn_8007B924(3, self->At(yp), bufB);
            if (fn_8040B574(bufA, bufB) == 0) {
                sc->fn_8007B924(1, self->At(y), bufA);
                sc->fn_8007B924(1, self->At(yp), bufB);
            }
            if (mode != 0 ? fn_8040B574(bufA, bufB) >= 0 : fn_8040B574(bufA, bufB) > 0) {
                int b = self->At(y);
                int a = self->At(yp);
                self->Set(y, a);
                self->Set(yp, b);
            }
            y = yp;
        }
    }
}

int fn_80075868(int id);
int fn_80075808(int id);

extern "C" DB_OVERLAY void fn_805371C4(Unk805352B0_List* self, unsigned f)
{
    unsigned i;
    fn_8040074C(self->m0, self->m8 << 2);
    Unk8007B2B0_Screen* sc = (Unk8007B2B0_Screen*)lbl_8061930C;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    unsigned n = sc->fn_8007BC9C();
    for (i = 0; i < n; i++) {
        int id = sc->fn_80072934(i);
        int r = 0;
        int g = sc->fn_8007B8C4(1, id);
        if (((f & 0x1) && g == 0) ||
            ((f & 0x2) && g == 1) ||
            ((f & 0x4) && g == 2) ||
            ((f & 0x8) && g == 3) ||
            ((f & 0x10) && g == 4) ||
            ((f & 0x20) && g == 5) ||
            ((f & 0x40) && g == 6) ||
            ((f & 0x80) && g == 7) ||
            ((f & 0x100) && g == 8) ||
            ((f & 0x200) && g == 9) ||
            ((f & 0x400) && g == 10) ||
            ((f & 0x800) && g == 11) ||
            ((f & 0x1000) && g == 12) ||
            ((f & 0x2000) && g == 13) ||
            ((f & 0x4000) && g == 14) ||
            ((f & 0x8000) && g == 15) ||
            ((f & 0x10000) && g == 16) ||
            ((f & 0x20000) && g == 17) ||
            ((f & 0x40000) && g == 18) ||
            ((f & 0x80000) && g == 19) ||
            ((f & 0x100000) && g == 20) ||
            ((f & 0x200000) && g == 21) ||
            ((f & 0x400000) && g == 22) ||
            ((f & 0x800000) && g == 23) ||
            ((f & 0x1000000) && g == 24) ||
            ((f & 0x2000000) && g == 25))
            r = id;
        if (sc->fn_80078A38(id) == 0 && (f & 0xC000000) && sc->fn_80075948(id) == id) {
            int h = sc->fn_8007B3E0(0x14, id);
            if (((f & 0x4000000) && h == 0) || ((f & 0x8000000) && h == 1))
                r = id;
        }
        if (r != 0 && (f & 0x10000000) && sc->fn_8007B8C4(4, r) == 1)
            r = 0;
        if (r != 0 && (f & 0x40000000) && fn_80075868(r) == 1)
            r = 0;
        if (r != 0 && (f & 0x20000000) && fn_80075808(r) == 1)
            r = 0;
        if (r != 0)
            self->Push(r);
    }
    self->m14 = 1;
}

extern "C" int fn_800A2ED8(void* p);
int fn_8007FB2C(Unk8007F198_C* self, int idx);
int fn_8007FB98(Unk8007F198_C* self, int a, int b, int c);
int fn_8007FD2C(Unk8007F198_C* self, int a, int b, int c);
#define SCR ((Unk8007F198_C*)lbl_8061930C)

extern "C" DB_OVERLAY void fn_80537AEC(Unk805352B0_List* self, int x)
{
    fn_8040074C(self->m0, self->m8 << 2);
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    if (fn_800A2ED8(lbl_80619948) == 10) {
        int i;
        for (i = fn_8007FB2C(SCR, x) - 1; i >= 0; i--) {
            if (fn_8007FB98(SCR, i, 0, x) != fn_8007FB98(SCR, i, 1, x))
                break;
        }
        int a = fn_8007FB98(SCR, i, 1, x);
        int b = fn_8007FB98(SCR, i, 0, x);
        if (fn_8007FD2C(SCR, i, 1, x) > fn_8007FD2C(SCR, i, 0, x))
            self->Push(a);
        else
            self->Push(b);
        self->m14 = 1;
    }
}

extern "C" DB_OVERLAY void fn_80537C60(Unk805352B0_List* self, int a, int b)
{
    if (self->m4 != 0 && b == self->m0[0] && a == self->m0[1])
        return;
    fn_8040074C(self->m0, self->m8 << 2);
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    self->Push(b);
    self->Push(a);
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80537D24(Unk805352B0_List* self, int mode)
{
    float keys[126];
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    int n = self->m4;
    int i;
    for (i = 0; i < n; i++)
        keys[i] = ((Unk80537D24_Screen*)sc)->fn_8007BB28(5, self->At(i), 1);
    int up = 0;
    if (mode != 0)
        up = 1;
    unsigned m = self->m4;
    float* k = keys;
    unsigned x;
    for (x = 0; x < m; x++) {
        unsigned y;
        for (y = x + 1; y < m; y++) {
            if ((up == 1 && k[x] < k[y]) || (up == 0 && !(k[x] <= k[y]))) {
                float t = k[x];
                k[x] = k[y];
                k[y] = t;
                int b = self->At(x);
                int a = self->At(y);
                self->Set(x, a);
                self->Set(y, b);
            }
        }
    }
}

extern "C" DB_OVERLAY void fn_80537E80(Unk805352B0_List* self, int x, int want, int both)
{
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    unsigned i;
    fn_8040074C(self->m0, self->m8 << 2);
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    unsigned n = fn_8007F198((Unk8007F198_A*)sc, x);
    for (i = 0; i < n; i++) {
        int v = fn_8007F24C((Unk8007F198_A*)sc, x, (unsigned char)i);
        int p = sc->fn_8007A6A0(5, v);
        int q = both ? sc->fn_8007A6A0(6, v) : p;
        if (want == 0xB) {
            if (p == 0xB || p == 8 || p == 7 || p == 6 || q == 0xB || q == 8 || q == 7 || q == 6)
                self->Push(v);
        } else if (p == want || q == want) {
            self->Push(v);
        }
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80537FD4(Unk805352B0_List* self, int x, int want)
{
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    Unk80535650_Tmp t;
    int buf[0x78];
    Unk80535650_Tmp* q = &t;
    int z = 0;
    fn_8040074C(self->m0, self->m8 << 2);
    unsigned i;
    self->m4 = z;
    self->mC = z;
    self->m14 = z;
    q->C1(buf, 0x78);
    q->C2();
    q->C3(lbl_8064A440);
    fn_80535650((Unk805352B0_List*)q, x);
    for (i = 0; i < q->Count(); i++) {
        int id = t.At(i);
        if (sc->fn_8007A6A0(5, id) == want)
            self->Push(id);
    }
    t.vp = lbl_8064A550;
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80538124(Unk805352B0_List* src, Unk805352B0_List* dst, int want)
{
    unsigned i;
    fn_8040074C(dst->m0, dst->m8 << 2);
    dst->m4 = 0;
    dst->mC = 0;
    dst->m14 = 0;
    for (i = 0; i < src->m4; i++) {
        int id = src->At(i);
        if (lbl_8061930C->fn_8007A6A0(5, id) == want)
            dst->Push(id);
    }
    dst->m14 = 1;
}

extern "C" DB_OVERLAY void fn_805381F0(Unk805352B0_List* self)
{
    void* mem = operator new(0x7108, 0x100);
    Unk8055B0D8_Obj* o = fn_8055B0D8(mem, self, lbl_8061930C);
    fn_8055B260(o);
    if (o)
        fn_8055B1F4(o, 3);
    self->m14 = 1;
}

struct Unk80090880;
extern "C" void Unicode2Ascii(char* dst, const void* src);
char* fn_80090880(Unk80090880* p);
unsigned fn_8007366C(const char* a, const char* b);
extern Unk80090880* lbl_8061977C;
extern int lbl_806013B0[];
extern int lbl_806014AC[];

extern "C" DB_OVERLAY void fn_80538258(Unk805352B0_List* self)
{
    typedef char* (*Look)(Unk80090880*, int);
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    Unk80090880* tbl = lbl_8061977C;
    unsigned i = 0;
    fn_8040074C(self->m0, self->m8 << 2);
    self->m4 = i;
    self->mC = i;
    self->m14 = i;
    for (; i <= 0x3E; i++) {
        char bufA[0x64];
        char bufB[0x64];
        char* a = ((Look)fn_80090880)(tbl, lbl_806013B0[i]);
        char* b = ((Look)fn_80090880)(tbl, lbl_806014AC[i]);
        Unicode2Ascii(bufA, a);
        Unicode2Ascii(bufB, b);
        unsigned id = fn_8007366C(bufA, bufB);
        if (sc->fn_8007A6A0(7, id) == 1)
            self->Push(id);
    }
    self->m14 = 1;
}

int fn_800758F8(int id);

/* near-miss: 14 differing slots, only register numbers and which Tmp stores go through
 * the q pointer (retail: q=r31, k=r26; ours q=r26, k=r31). The Tmp ctor is written out
 * field by field because the retail mixes r1-relative and q-relative accesses. */
extern "C" DB_OVERLAY void fn_80538368(Unk805352B0_List* self, Unk805352B0_List* excl)
{
    fn_8040074C(self->m0, self->m8 << 2);
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    unsigned nA = sc->fn_8007B1EC();
    unsigned nB = ((Unk8007B2B0_Screen*)sc)->fn_8007BC9C();
    int flags[3250];
    fn_8040074C(flags, 0x32C8);
    unsigned i;
    if (excl) {
        for (i = 0; i < excl->m4; i++)
            flags[sc->fn_8007A11C(excl->At(i))] = 1;
    }
    Unk80535650_Tmp t;
    int buf[0x78];
    Unk80535650_Tmp* q = &t;
    t.m0 = buf;
    t.vp = lbl_8064B298;
    q->m8 = 0x78;
    fn_8040074C(t.m0, t.m8 << 2);
    q->m4 = 0;
    q->mC = 0;
    t.vp = lbl_8064A550;
    fn_8040074C(t.m0, t.m8 << 2);
    t.m14 = 0;
    t.m4 = 0;
    t.mC = 0;
    t.vp = lbl_8064A440;
    for (i = 0; i < nB; i++) {
        int v = ((Unk8007B2B0_Screen*)sc)->fn_8007290C(i);
        if (fn_800758F8(v) == 0) {
            fn_80535650((Unk805352B0_List*)q, v);
            unsigned j;
            for (j = 0; j < t.Count(); j++)
                flags[sc->fn_8007A11C(t.At(j))] = 1;
        }
    }
    for (i = 1; i < nA; i++) {
        if (flags[i] == 0 && sc->fn_8007A8DC(0xB, i) == 0)
            self->Push(sc->fn_800728BC(i));
    }
    self->m14 = 1;
    t.D();
}

extern "C" DB_OVERLAY void fn_805385B8(Unk805352B0_List* self, int x)
{
    unsigned i;
    fn_8040074C(self->m0, self->m8 << 2);
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    unsigned n = fn_8007F198((Unk8007F198_A*)sc, x);
    for (i = 0; i < n; i++) {
        int v = fn_8007F24C((Unk8007F198_A*)sc, x, (unsigned char)i);
        int p = sc->fn_8007A6A0(0x1E, v);
        int q = sc->fn_8007A6A0(0x1F, v);
        if (p != 0 && p != 0x31 && q > 0)
            self->Push(v);
    }
    for (i = 0; i <= 1; i++) {
        unsigned k;
        unsigned m = fn_8007F674((Unk8007F198_A*)sc, x, i);
        for (k = 0; k < m; k++) {
            int v = fn_8007F6C4((Unk8007F198_A*)sc, x, i, k);
            int p = sc->fn_8007A6A0(0x1E, v);
            int q = sc->fn_8007A6A0(0x1F, v);
            if (p != 0 && p != 0x31 && q > 0)
                self->Push(v);
        }
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_8053876C(Unk805352B0_List* self, int x)
{
    unsigned i;
    fn_8040074C(self->m0, self->m8 << 2);
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    unsigned n = fn_8007F198((Unk8007F198_A*)sc, x);
    for (i = 0; i < n; i++) {
        int v = fn_8007F24C((Unk8007F198_A*)sc, x, (unsigned char)i);
        int p = sc->fn_8007A6A0(0x1E, v);
        int q = sc->fn_8007A6A0(0x1F, v);
        if (p != 0 && p != 0x31 && q > 0)
            self->Push(v);
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80538868(Unk805352B0_List* self, int x)
{
    unsigned i;
    fn_8040074C(self->m0, self->m8 << 2);
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    unsigned n = fn_8007F198((Unk8007F198_A*)sc, x);
    for (i = 0; i < n; i++) {
        int v = fn_8007F24C((Unk8007F198_A*)sc, x, (unsigned char)i);
        int p = sc->fn_8007A6A0(0x1E, v);
        int q = sc->fn_8007A6A0(0x1F, v);
        if (p == 0x31 && q > 0)
            self->Push(v);
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_8053895C(Unk805352B0_List* self, int x, int y)
{
    int i;
    fn_8040074C(self->m0, self->m8 << 2);
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    int n = fn_8007F198((Unk8007F198_A*)sc, x);
    for (i = 0; i < n; i++) {
        int v = fn_8007F24C((Unk8007F198_A*)sc, x, (unsigned char)i);
        if (fn_8007E0C0((Unk8007DB74_A*)sc, x, v, y) == 0 && sc->fn_8007A270(v) == 0)
            self->Push(v);
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80538A50(Unk805352B0_List* self, int x, int y)
{
    int i;
    fn_8040074C(self->m0, self->m8 << 2);
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    int n = fn_8007F198((Unk8007F198_A*)sc, x);
    for (i = 0; i < n; i++) {
        int v = fn_8007F24C((Unk8007F198_A*)sc, x, (unsigned char)i);
        if (fn_8007DED4((Unk8007DB74_A*)sc, x, v, y) == 0 && sc->fn_8007A270(v) == 0)
            self->Push(v);
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80538B44(Unk805352B0_List* self, int flag)
{
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    int i;
    for (i = 0; i <= 0x18; i++) {
        unsigned id = fn_800765A8(i);
        if (sc->fn_8007A6A0(0xB, id) == 0 && (flag == 0 || fn_80077BC4(sc, id) == 0))
            self->Push(id);
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80538BF8(Unk805352B0_List* self, int x, int y)
{
    int z = 0;
    Unk80535650_Tmp t;
    int buf[0x78];
    Unk80535650_Tmp* q = &t;
    fn_8040074C(self->m0, self->m8 << 2);
    self->m4 = z;
    self->mC = z;
    self->m14 = z;
    q->C1(buf, 0x78);
    q->C2();
    q->C3(lbl_8064A440);
    fn_80535650((Unk805352B0_List*)q, y);
    fn_80538CD8(self, x, (Unk805352B0_List*)q);
    q->D();
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80538CD8(Unk805352B0_List* dst, int want, Unk805352B0_List* src)
{
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    int i;
    for (i = src->m4 - 1; i >= 0; i--) {
        if (sc->fn_8007A6A0(0x16, src->At(i)) == want) {
            dst->Push(src->At(i));
            src->RemoveAt(i);
        }
        if (!(dst->m4 < dst->m8))
            break;
    }
    dst->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80538DD8(int a, int b)
{
    fn_8053DF70(lbl_80619948->m5C, a, b, lbl_805FEC40);
}

extern "C" DB_OVERLAY void fn_80538E18(int a, int b)
{
    fn_8053DE38(lbl_80619948->m5C, a, b, lbl_805FEC44);
}

extern "C" DB_OVERLAY int fn_80538E58(int id)
{
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    int r = 0;
    if (sc->fn_8007A6A0(5, id) == 0 || sc->fn_8007A6A0(5, id) == 10)
        r = 1;
    return r;
}

extern "C" DB_OVERLAY void fn_80538EC4(Unk805352B0_List* self, int x, int y)
{
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    int i;
    fn_8040074C(self->m0, self->m8 << 2);
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    int n = fn_8007F198((Unk8007F198_A*)sc, x);
    for (i = 0; i < n; i++) {
        int v = fn_8007F24C((Unk8007F198_A*)sc, x, (unsigned char)i);
        if (v != 0 && fn_8007DED4((Unk8007DB74_A*)sc, x, v, y) == 0 && fn_8007E48C((Unk8007DB74_A*)sc, x, v) == 0
            && fn_8007E7F0((Unk8007DB74_A*)sc, x, v) == 0 && fn_8007EAF0((Unk8007DB74_A*)sc, x, v) == 0
            && fn_8007EDF0((Unk8007DB74_A*)sc, x, v) == 0 && fn_8007F0F0((Unk8007DB74_A*)sc, x, v) == 0)
            self->Push(v);
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80539020(Unk805352B0_List* self, int x)
{
    int i;
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    fn_8040074C(self->m0, self->m8 << 2);
    self->m4 = 0;
    self->mC = 0;
    self->m14 = 0;
    int n = fn_8007F198((Unk8007F198_A*)sc, x);
    for (i = 0; i < n; i++) {
        int v = fn_8007F24C((Unk8007F198_A*)sc, x, (unsigned char)i);
        if (v != 0 && sc->fn_8007A270(v) != 0 && fn_8007E48C((Unk8007DB74_A*)sc, x, v) == 0
            && fn_8007F0F0((Unk8007DB74_A*)sc, x, v) == 0)
            self->Push(v);
    }
    self->m14 = 1;
}

extern "C" DB_OVERLAY void fn_80539128(Unk805352B0_List* self, Unk805352B0_List* out)
{
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    unsigned i;
    for (i = 0; i < self->m4;) {
        int id = self->At(i);
        int p = sc->fn_8007A6A0(0x1E, id);
        int q = sc->fn_8007A6A0(0x1F, id);
        if (p != 0 && q > 0) {
            self->RemoveAt(i);
            if (out)
                out->Push(id);
        } else {
            i++;
        }
    }
}

extern "C" DB_OVERLAY void fn_8053923C(Unk805352B0_List* self, int a, Unk805352B0_List* out)
{
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    unsigned i;
    for (i = 0; i < self->m4;) {
        int id = self->At(i);
        int p = sc->fn_80078A64(a, 0, id) != 0;
        if ((p != 0) | (sc->fn_80078A64(a, 1, id) != 0)) {
            self->RemoveAt(i);
            if (out)
                out->Push(id);
        } else {
            i++;
        }
    }
}

extern "C" DB_OVERLAY void fn_80539380(Unk805352B0_List* self)
{
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    unsigned i;
    for (i = 0; i < self->m4;) {
        if (sc->fn_8007A270(self->At(i)) != 0) {
            self->RemoveAt(i);
        } else {
            i++;
        }
    }
}

extern "C" DB_OVERLAY void fn_8053943C(Unk805352B0_List* self)
{
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    unsigned i;
    for (i = 0; i < self->m4;) {
        if (sc->fn_8007A270(self->At(i)) == 0) {
            self->RemoveAt(i);
        } else {
            i++;
        }
    }
}

extern "C" DB_OVERLAY void fn_805394F8(Unk805352B0_List* self, int a)
{
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    unsigned i;
    for (i = 0; i < self->m4;) {
        int id = self->At(i);
        int p = sc->fn_8007A6A0(5, id);
        int q = sc->fn_8007A6A0(5, id);
        if (fn_80076644(a, p, q) == 0)
            self->RemoveAt(i);
        else
            i++;
    }
}

extern "C" DB_OVERLAY unsigned fn_805395E4(Unk805352B0_List* self, int a)
{
    unsigned n = self->m4;
    Unk8007A4DC_Screen* sc = lbl_8061930C;
    unsigned i;
    for (i = 0; i < n; i++) {
        int id = self->At(i);
        int p = sc->fn_8007A6A0(5, id);
        int q = sc->fn_8007A6A0(5, id);
        fn_80076644(a, p, q);
    }
    return n;
}
