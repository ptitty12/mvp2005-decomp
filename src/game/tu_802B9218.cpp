/*
 * Game code, .text 0x802B9218-0x802BB4BC (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
#include "game/tu_802B9218.h"
#include "game/tu_8000E618.h"

extern int lbl_8061EAE0;
extern int lbl_8061E978;
extern Unk802BA828* lbl_8061EAE4;
void* operator new(unsigned int n, int heap);
float fn_801A0744(int a);
extern int lbl_8061EAB4;
extern int lbl_8061EAB8;
extern int lbl_8061EABC;
extern int lbl_8061EAC0;
extern int lbl_8061EAC4;
extern int lbl_8061EAC8;
extern int lbl_8061EACC;
extern int lbl_8061EAD0;
extern int lbl_8061EAD4;
extern int lbl_8061EAD8;
extern int lbl_8061EADC;
extern Unk8068B4F8 lbl_8068B4F8;

extern "C" void MEM_free(int p);
Unk8000E618* fn_8000E65C();
void* fn_8040B190(void* dst, const void* src, ...);
extern "C" void MEMCLASS_purge(int a);
extern "C" void MEMCLASS_remove(int a);
extern "C" int fn_803ED9D4();
extern "C" int MEMCLASS_create(int a, const char* b, int c, int d, int e, int f, int g, int h, int i, int j);
extern const char lbl_805E10B8[];
extern const char lbl_805E0F5C[];
extern const char lbl_805E0F68[];
void fn_8033E958(const char* fmt, ...);
extern "C" void fn_8033E7A4();
extern "C" int fn_803FC410(int a, const char* name, int b, int* c, void* d);
extern "C" int unpacksizez(int a);
extern "C" int fn_803ED604(const char* name, int a, int b);
extern "C" int fn_803EE310(int a);
extern "C" void MEM_copy(int a, int b, int n);
extern "C" int FILE_loadat(int a, int b, int c);
extern "C" int fn_803FB954(int a);
extern "C" int FILE_size(int a);
extern "C" int fn_803FBF10(int a, int b);
extern "C" int fn_803FC190(int a, int b);
extern "C" int fn_803FC2E0(int a, int b, int c);
extern "C" int fn_803FC034(int a);
extern "C" int ASYNCFILE_loadat(int a, int b, int c);
void fn_802B9EFC(Unk8068B4F8* p, int v);
void* fn_802B9CCC(int a, int* b, int c);
void fn_802B9ED4(int a);
extern "C" int FILESYS_open(int a, int b, int c, int d);
extern "C" void FILESYS_waitop(int h);
extern "C" int FILESYS_completeop(int h);
extern "C" int FILESYS_size(int a, int b, int c);
extern "C" void* fn_803774A8(unsigned n, int heap);
extern "C" int FILESYS_read(int a, int b, void* c, int d, int e, int f);
extern "C" int fn_803FEC9C(int a, int b);
extern "C" void fn_80377488(void* p);
int fn_8000446C(int a, int b);
extern "C" int fn_804017B0(int a, int b);
extern "C" int fn_80401904(int a);
extern "C" void DCStoreRange(void* p, int n);
extern "C" int SHAPE_comment(void* p);
extern "C" int SHAPE_longname(void* p);
void fn_8040B61C(void* dst, int src);
int fn_803EBD14(void* a, int b);
void* SHAPE_clut(void* p);
int fn_8040B574(const char* a, const char* b);
void fn_802B0638(int a, void* b, int c);
extern "C" int SHAPE_readpixel(void* a, int x, int y, int z);
extern "C" void SHAPE_writepixel(void* a, int x, int y, int z, unsigned v);

void fn_802B9218()
{
    MEM_free(lbl_8061EAE0);
    lbl_8061EAE0 = 0;
}

int fn_802B9250(int a, int b, int c)
{
    int r;
    int tries = 1;
    do {
        fn_802B9EFC(&lbl_8068B4F8, 0);
        r = FILE_loadat(a, b, c);
    } while (lbl_8068B4F8.Get() != 0 && --tries > 0);
    if (lbl_8068B4F8.Get() != 0) {
        fn_802B9ED4(a);
        return 0;
    }
    return r;
}

/* near-miss: 123 insns as retail, stack slots and callee-saved numbering now match; the only difference is where the
   low half (addi) of the "Loading %s" format address is scheduled (retail: after the first two zeroing stores).
   Notes: the five address-taken locals must be members of one struct (as separate scalars GCC numbers them in order
   of first use, which swapped sp14/sp18); routing sp8 + sp10 through r (r = ...; int a = r;) raises r's register
   priority above name's, which is what makes name r28 / r r29 as in retail (permuter find). */
extern "C" const char* fn_802B9300(const char* name, int size, int kind, unsigned int* outn)
{
    struct {
        int sp8;
        int spC;
        int sp10;
        int sp14;
        int sp18;
    } s;
    int r = 0;
    int h;
    s.sp10 = 0;
    s.sp18 = 0;
    s.sp14 = 0;
    s.sp8 = 0;
    s.spC = 0;
    fn_8033E958(lbl_805E0F5C, name);
    fn_802B9D7C(kind, &s.sp8, &s.spC);
    switch (s.spC) {
    case 0:
        break;
    case 1:
        if (s.sp8 != 0) {
            fn_803FC410(s.sp8, name, 0, &s.sp10, outn);
            r = s.sp8 + s.sp10;
            break;
        }
        goto L_default;
    case 2:
        if (s.sp8 != 0) {
            fn_803FC410(s.sp8, name, 0, &s.sp10, &s.sp18);
            r = s.sp8 + s.sp10;
            int a = r;
            int n = unpacksizez(a);
            if (n != 0) {
                if (outn != 0) {
                    *outn = n;
                }
                h = fn_803ED604(name, n, size);
                fn_804017B0(a, h);
            } else {
                if (outn != 0) {
                    *outn = s.sp18;
                }
                h = fn_803ED604(name, s.sp18, size);
                MEM_copy(h, a, s.sp18);
            }
            r = h;
            break;
        }
    L_default:
        r = (int)fn_802B9CCC((int)name, &s.sp14, size);
        if (outn != 0) {
            *outn = s.sp14;
        }
        break;
    case 3:
        r = (int)fn_802B9CCC((int)name, &s.sp14, size);
        if (r != 0) {
            if (outn != 0) {
                *outn = fn_803EE310(r);
            }
        } else if (outn != 0) {
            *outn = r;
        }
        break;
    }
    if (r == 0) {
        fn_8033E958(lbl_805E0F68);
    }
    fn_8033E7A4();
    return (const char*)r;
}

/* near-miss (75 vs 76 insns): retail zeroes sp14 with its own li r0,0 and sp8/spC with a second li r9,0 (two constant registers); ours shares one. */
int fn_802B94EC(int h, int dst, int max, int kind)
{
    int sp8;
    int spC;
    int sp10;
    int sp14;
    sp14 = 0;
    spC = 0;
    sp8 = 0;
    fn_802B9D7C(kind, &sp8, &spC);
    switch (spC) {
    case 0:
        break;
    case 1:
        if (sp8 == 0) {
            goto L_call;
        }
        fn_803FC410(sp8, (const char*)h, 0, &sp10, &sp14);
        if (sp14 > max) {
            sp14 = max;
        }
        goto L_copy;
    case 2:
        if (sp8 == 0) {
            goto L_call;
        }
        fn_803FC410(sp8, (const char*)h, 0, &sp10, &sp14);
        if (fn_80401904(sp8 + sp10) != 0) {
            fn_804017B0(sp8 + sp10, dst);
            break;
        }
    L_copy:
        MEM_copy(dst, sp8 + sp10, sp14);
        break;
    case 3:
    L_call:
        sp14 = fn_802B9250(h, dst, max);
        break;
    }
    return sp14;
}

extern "C" void fn_802B961C(const char* p, int x)
{
    int obj = (int)p;
    int sp8 = 0;
    int spC = 0;
    fn_802B9D7C(x, &sp8, &spC);
    switch (spC) {
    case 0:
        break;
    case 1:
        if (sp8 != 0) {
            break;
        }
    case 2:
        MEM_free(obj);
        break;
    case 3:
        MEM_free(obj);
        break;
    }
}

int fn_802B96A4(int a, int b, int c)
{
    return ASYNCFILE_loadat(a, b, c);
}

int fn_802B96C4(int a)
{
    int r;
    int tries = 1;
    do {
        fn_802B9EFC(&lbl_8068B4F8, 0);
        r = fn_803FB954(a);
    } while (lbl_8068B4F8.Get() != 0 && --tries > 0);
    if (lbl_8068B4F8.Get() != 0) {
        fn_802B9ED4(a);
        return 0;
    }
    return r;
}

int fn_802B9764(int a)
{
    int r;
    int tries = 1;
    do {
        fn_802B9EFC(&lbl_8068B4F8, 0);
        r = FILE_size(a);
    } while (lbl_8068B4F8.Get() != 0 && --tries > 0);
    if (lbl_8068B4F8.Get() != 0) {
        fn_802B9ED4(a);
        return 0;
    }
    return r;
}

int fn_802B9804(int a, int b)
{
    int r;
    int tries = 1;
    do {
        fn_802B9EFC(&lbl_8068B4F8, 0);
        r = fn_803FBF10(a, b);
    } while (lbl_8068B4F8.Get() != 0 && --tries > 0);
    if (lbl_8068B4F8.Get() != 0) {
        fn_802B9ED4(a);
        return 0;
    }
    return r;
}

int fn_802B98AC(int a, int b)
{
    int r;
    int tries = 1;
    do {
        fn_802B9EFC(&lbl_8068B4F8, 0);
        r = fn_803FC190(a, b);
    } while (lbl_8068B4F8.Get() != 0 && --tries > 0);
    if (lbl_8068B4F8.Get() != 0) {
        fn_802B9ED4(a);
        return 0;
    }
    return r;
}

int fn_802B9954(int a, int b, int c)
{
    int r;
    int tries = 1;
    do {
        fn_802B9EFC(&lbl_8068B4F8, 0);
        r = fn_803FC2E0(a, b, c);
    } while (lbl_8068B4F8.Get() != 0 && --tries > 0);
    if (lbl_8068B4F8.Get() != 0) {
        fn_802B9ED4(a);
        return 0;
    }
    return r;
}

int fn_802B9A04(int a)
{
    int r;
    int tries = 1;
    do {
        fn_802B9EFC(&lbl_8068B4F8, 0);
        r = fn_803FC034(a);
    } while (lbl_8068B4F8.Get() != 0 && --tries > 0);
    if (lbl_8068B4F8.Get() != 0) {
        fn_802B9ED4(a);
        return 0;
    }
    return r;
}

void* fn_802B9AA4(int a, int* b, int c)
{
    int h = FILESYS_open(a, 0x21, 0, 0);
    FILESYS_waitop(h);
    int s = FILESYS_completeop(h);
    h = FILESYS_size(s, 0, 0);
    FILESYS_waitop(h);
    *b = FILESYS_completeop(h);
    void* p = fn_803774A8(*b, c | 0x400);
    h = FILESYS_read(s, 0, p, *b, 0, 0);
    FILESYS_waitop(h);
    FILESYS_completeop(h);
    fn_803FEC9C(s, 0);
    return p;
}

void* fn_802B9B60(int a, int* b, int c)
{
    int h = FILESYS_open(a, 0x21, 0, 0);
    FILESYS_waitop(h);
    int s = FILESYS_completeop(h);
    h = FILESYS_size(s, 0, 0);
    FILESYS_waitop(h);
    *b = FILESYS_completeop(h);
    void* p = fn_803774A8(*b, c);
    h = FILESYS_read(s, 0, p, *b, 0, 0);
    FILESYS_waitop(h);
    FILESYS_completeop(h);
    fn_803FEC9C(s, 0);
    return p;
}

void* fn_802B9C1C(int a, int* b, int c)
{
    void* r;
    int tries = 1;
    do {
        fn_802B9EFC(&lbl_8068B4F8, 0);
        r = fn_802B9B60(a, b, c);
    } while (lbl_8068B4F8.Get() != 0 && --tries > 0);
    if (lbl_8068B4F8.Get() != 0) {
        fn_802B9ED4(a);
        return 0;
    }
    return r;
}

void* fn_802B9CCC(int a, int* b, int c)
{
    void* r;
    int tries = 1;
    do {
        fn_802B9EFC(&lbl_8068B4F8, 0);
        r = fn_802B9AA4(a, b, c);
    } while (lbl_8068B4F8.Get() != 0 && --tries > 0);
    if (lbl_8068B4F8.Get() != 0) {
        fn_802B9ED4(a);
        return 0;
    }
    return r;
}

void fn_802B9D7C(int kind, int* a, int* b)
{
    switch (kind) {
    case 0:
        break;
    case 1:
        *a = lbl_8061EAB4;
        *b = kind;
        break;
    case 2:
        *a = lbl_8061EAB8;
        *b = 1;
        break;
    case 3:
        *b = 2;
        break;
    case 4:
        *a = lbl_8061EABC;
        *b = 2;
        break;
    case 5:
        *a = lbl_8061EAC0;
        *b = 2;
        break;
    case 6:
        *a = lbl_8061EAC8;
        *b = 2;
        break;
    case 7:
        *a = lbl_8061EAC4;
        *b = 2;
        break;
    case 8:
        *a = lbl_8061EACC;
        *b = 2;
        break;
    case 9:
        *a = lbl_8061EAD0;
        *b = 2;
        break;
    case 10:
        *a = lbl_8061EAD4;
        *b = 1;
        break;
    case 11:
        *a = lbl_8061EAD8;
        *b = 1;
        break;
    case 12:
        *a = lbl_8061EADC;
        *b = 1;
        break;
    case 13:
        *a = lbl_8061EAE0;
        *b = 1;
        break;
    case 14:
        *a = 0;
        *b = 3;
        break;
    }
}

/* needs fn_8000446C(int, int) (r3 = 1, r4 = 0 set here); tu_800042B0.cpp defines it with no parameters, so the call
   relocates against a different mangled name until that definition gets its two parameters. */
void fn_802B9ED4(int)
{
    fn_8000446C(1, 0);
}

void fn_802B9EFC(Unk8068B4F8* p, int v)
{
    fn_80400828(p->m4);
    p->m0 = v;
    fn_8040084C(p->m4);
}

/* near-miss (7 slots): retail keeps p live and forms the second loop pointer in r11 (addi r11,r3,0x1ef0); ours reuses r3.
   Pointer, index and p->a[i] loop forms and all 120 store orders of the second loop tried. */
void fn_802B9F40(Unk802B9F40* p)
{
    p->m2354 = 0;
    p->m2350 = 0;
    for (int i = 0; i < 0x84; i++) {
        p->a0[i].m38 = 0;
        p->a0[i].m0 = 0;
        p->a0[i].m4 = 0;
        p->a0[i].m8 = 0;
    }
    for (int j = 0; j < 0x14; j++) {
        p->a1EF0[j].m0 = -1;
        p->a1EF0[j].m2C = 0;
        p->a1EF0[j].m4 = 0;
        p->a1EF0[j].m8 = 0;
        p->a1EF0[j].m28 = 0;
    }
}

unsigned fn_802B9FA8(unsigned x)
{
    if (x & 0x8000) {
        return ((x << 9) & 0x00F80000) | 0xFF000000u | ((x << 6) & 0x0000F800) | ((x & 0x1F) << 3);
    }
    return ((x << 17) & 0xE0000000) | ((x << 12) & 0x00F00000) | ((x << 8) & 0x0000F000) | ((x & 0xF) << 4);
}

/* near-miss (4 slots): retail has the palette base in r11 and the index*2 in r9 (add r9,r3,r3; lhzx r3,r9,r11); ours r9/r0. */
void fn_802B9FF0(void* a, Unk802B9FF0_Hdr* b)
{
    int cnt = b->m8;
    if (cnt > 0) {
        int* pp = &b->m14;
        for (int n = cnt; n != 0; n--) {
            char* e = (char*)b + *pp;
            pp += 2;
            void* tex = (void*)fn_803EBD14(a, SHAPE_longname(e));
            int w = *(short*)(e + 4);
            int h = *(short*)(e + 6);
            Unk802B9FF0_Pal* pal = (Unk802B9FF0_Pal*)SHAPE_clut(e);
            for (int x = 0; x < w; x++) {
                for (int y = 0; y < h; y++) {
                    int idx = SHAPE_readpixel(e, x, y, 0);
                    unsigned short* tab;
                    if (pal->mC & 0x10000000) {
                        tab = (unsigned short*)((char*)pal + pal->m10);
                    } else {
                        tab = (unsigned short*)((char*)pal + 0x10);
                    }
                    unsigned c = fn_802B9FA8(tab[idx]) & 0xFFFFFF;
                    unsigned o = SHAPE_readpixel(tex, x, y, 0) & 0xFF000000;
                    SHAPE_writepixel(tex, x, y, 0, c | o);
                }
            }
        }
    }
}

void fn_802BA110(void* a, Unk802BA110_Src* b)
{
    fn_804017B0(b->m4, (int)a);
    void* buf = fn_803774A8(fn_80401904(b->m0), 0x100);
    fn_804017B0(b->m0, (int)buf);
    fn_802B9FF0(a, (Unk802B9FF0_Hdr*)buf);
    delete[] (char*)buf;
    DCStoreRange(a, 0x19260);
}

void fn_802BA194(Unk802B9F40* p, Unk802B9FF0_Hdr* h)
{
    if (SHAPE_comment((char*)h + h->m14) == 0) {
        int n = h->m8;
        int i;
        if (n > 0) {
            char* d = (char*)p + 0x12;
            int* s = &h->m14;
            for (i = n; i != 0; i--) {
                fn_8040B61C(d, SHAPE_longname((char*)h + *s));
                s += 2;
                d += 5;
            }
        }
        p->a0[0].m38 = n;
    }
}

void fn_802BA218(Unk802B9F40* p)
{
    int off = 0;
    for (int i = 0; i < 0x14; i++) {
        char* base = (char*)p + 0x1F18;
        *(void**)(base + off) = fn_803774A8(0x19260, 0x400);
        off += 0x38;
    }
    p->m2354 = 1;
}

void fn_802BA274(Unk802B9F40* t, Unk802B9F40_A0* s, const void* a1, int n1, const void* a2, int n2)
{
    if (fn_802BA45C(t, s)) {
        s->m0 = fn_803774A8(n1, fn_8000E65C()->fn_8000E6F0());
        s->m4 = fn_803774A8(n2, fn_8000E65C()->fn_8000E6F0());
    }
    fn_8040B190(s->m0, a1, n1);
    fn_8040B190(s->m4, a2, n2);
}

extern const char lbl_805E0134[];
char* fn_8040B830(char* dst, const char* src, unsigned n);
Unk802B9F40_A0* fn_802BA47C(Unk802B9F40* t);

void fn_802BA310(Unk802B9F40* t, const char* name)
{
    Unk802B9F40_A0* a = fn_802BA47C(t);
    if (a != 0) {
        unsigned int n1 = 0;
        const char* h1 = fn_802B9300(name, 0x500, 14, &n1);
        n1 = (n1 + 0x1F) & ~0x1F;
        void* p = fn_803774A8(fn_80401904((int)h1), 0x500);
        fn_804017B0((int)h1, (int)p);
        fn_802BA194((Unk802B9F40*)a, (Unk802B9FF0_Hdr*)p);
        if (p != 0)
            fn_80377488(p);
        char buf[0x400] = "";
        fn_8040B830(buf, name, 0x400);
        buf[0] = 'a';
        buf[0x3FF] = 0;
        unsigned int n2 = 0;
        const char* h2 = fn_802B9300(buf, 0x100, 13, &n2);
        n2 = (n2 + 0x1F) & ~0x1F;
        fn_802BA274(t, a, h1, n1, h2, n2);
        fn_802B961C(h2, 13);
        fn_8040B61C((char*)a + 8, (int)name);
        if (h1 != 0)
            fn_80377488((void*)h1);
    }
}

int fn_802BA45C(Unk802B9F40*, Unk802B9F40_A0* s)
{
    return s->m0 == 0 && s->m4 == 0;
}

Unk802B9F40_A0* fn_802BA47C(Unk802B9F40* t)
{
    int s = (int)t;
    int last = s + 0x1EB4;
    do {
        if (!fn_802BA45C(t, (Unk802B9F40_A0*)s)) {
            s += 0x3C;
            continue;
        }
        return (Unk802B9F40_A0*)s;
    } while (s <= last);
    return 0;
}

Unk802B9F40_A1EF0* fn_802BA4D8(Unk802B9F40* t, int key)
{
    for (int i = 0; i < 0x14; i++) {
        Unk802B9F40_A1EF0* s = &t->a1EF0[i];
        if (s->m0 != -1) {
            int cnt = s->m8;
            for (int j = 0; j < cnt; j++) {
                if (s->mC[j] == key) {
                    return s;
                }
            }
        }
    }
    return 0;
}

void fn_802BA540(Unk802B9F40_A1EF0* s, int v)
{
    s->mC[s->m8++] = v;
}

Unk802B9F40_A0* fn_802BA55C(Unk802B9F40* t, const char* name)
{
    int s = (int)t;
    int last = s + 0x1EB4;
    do {
        Unk802B9F40_A0* e = (Unk802B9F40_A0*)s;
        if (!fn_802BA45C(t, e)) {
            int n = e->m38;
            for (int i = 0; i < n; i++) {
                if (fn_8040B574(e->m12 + i * 5, name) == 0) {
                    return e;
                }
            }
        }
        s += 0x3C;
    } while (s <= last);
    return 0;
}

Unk802B9F40_A1EF0* fn_802BA5F4(Unk802B9F40* t)
{
    Unk802B9F40_A1EF0* best = 0;
    for (int i = 0; i < 0x14; i++) {
        Unk802B9F40_A1EF0* s = &t->a1EF0[i];
        if (s->m0 != -1) {
            if (best == 0 || s->m4 < best->m4) {
                best = s;
            }
        } else {
            fn_802BA67C(t, s);
            return s;
        }
    }
    fn_802BA67C(t, best);
    return best;
}

void fn_802BA67C(Unk802B9F40* t, Unk802B9F40_A1EF0* s)
{
    if (s->m0 != -1) {
        fn_802B0638(lbl_8061E978, &s->m2C, 0);
    }
    s->m0 = -1;
    s->m8 = 0;
    s->m2C = 0;
    for (int i = 0; i < 7; i++) {
        s->mC[i] = 0;
    }
    s->m4 = t->m2350 + 1;
}

void fn_802BA708(Unk802B9F40* t)
{
    for (int i = 0; i < 0x14; i++) {
        fn_802BA67C(t, &t->a1EF0[i]);
    }
}


/* near-miss (52 vs 53 insns): retail uses a do-while with s=r29, off=r28, e=r27 and forms t+4 inside the branch; ours hoists it. */
void fn_802BA750(Unk802B9F40* t)
{
    t->m2354 = 0;
    int s = (int)t;
    int last = s + 0x1EB4;
    int off = 0;
    for (; s <= last; s += 0x3C, off += 0x3C) {
        int e = fn_802BA45C(t, (Unk802B9F40_A0*)s);
        if (e == 0) {
            if (*(void**)((char*)t + off) != 0) {
                delete[] *(char**)((char*)t + off);
                *(int*)((char*)t + off) = e;
            }
            char* b = (char*)t + 4;
            if (*(void**)(b + off) != 0) {
                delete[] *(char**)(b + off);
                *(int*)(b + off) = e;
            }
        }
    }
    int so = 0;
    Unk802B9F40_A1EF0* q = t->a1EF0;
    for (int i = 0; i < 0x14; i++) {
        fn_802BA67C(t, q);
        char* b = (char*)t + 0x1F18;
        if (*(void**)(b + so) != 0) {
            delete[] *(char**)(b + so);
            *(int*)(b + so) = 0;
        }
        q++;
        so += 0x38;
    }
}

void fn_802BA820(Unk802B9F40* t, int v)
{
    t->m2350 = v;
}

/* near-miss (store order): retail stores m24, m0, m4, m8, mC, m18, m10, m14, m1C, m20, m28, m2C. */
Unk802BA828::Unk802BA828(int a)
{
    m24 = 0;
    m0 = -1.0f;
    m4 = 0.0f;
    m8 = a;
    mC = 0;
    m18 = 0;
    m10 = 0.0f;
    m14 = 0.0f;
    m1C = 0.0f;
    m20 = 0.0f;
    m28 = 0.0f;
    m2C = 0.0f;
}

Unk802BA828* fn_802BA874(int a)
{
    if (lbl_8061EAE4 == 0) {
        lbl_8061EAE4 = new (0x400) Unk802BA828(a);
    }
    return lbl_8061EAE4;
}

void fn_802BA8C8()
{
    if (lbl_8061EAE4 != 0) {
        delete lbl_8061EAE4;
        lbl_8061EAE4 = 0;
    }
}

void fn_802BA908(Unk802BA828* p)
{
    float v = fn_801A0744(p->m8);
    p->m4 = !(p->m0 < 0.0f) ? v - p->m0 : 0.0f;
    p->m0 = v;
}

/* near-miss (8 slots): retail keeps the two spilled floats of each (x, y) pair at sp+0x10/0x14 (an 8-aligned pair); float locals land at 0xC/0x10.
   Struct locals do not spill at all. */
void fn_802BA96C(Unk802BA96C* p, int k, Unk802BA96C_Q* q, float dt)
{
    if (k == 0x64) {
        p->m30 = &p->mC;
    }
    if (k == 0x62) {
        p->m30 = &p->m18;
    }
    if (k == 0x63) {
        p->m30 = &p->m24;
    }
    if (p->m30->m0 & 1) {
        p->m30->m4 = p->m4 * q->m30 + p->m30->m4;
        p->m30->m8 = p->m4 * q->m34 + p->m30->m8;
        if (!(p->m30->m4 <= 1.0f)) {
            p->m30->m4 = p->m30->m4 - 1.0f;
        }
        if (!(p->m30->m8 <= 1.0f)) {
            p->m30->m8 = p->m30->m8 - 1.0f;
        }
        float vx = p->m30->m4;
        float vy = p->m30->m8;
        if (q->m3C != 0) {
            q->m3C->x = vx;
            q->m3C->y = vy;
        }
    }
    float wx;
    float wy;
    if (p->m30->m0 & 2) {
        wx = p->m30->m4;
        wy = 0.5f;
    } else {
        wx = p->m30->m4;
        wy = 0.0f;
    }
    if (q->m3C != 0) {
        q->m3C->x = wx;
        q->m3C->y = wy;
    }
}

extern const char lbl_805E0FBC[];
extern const char lbl_805DFC34[];
extern const char lbl_805E0FE4[];
extern const char lbl_805E0FF4[];
extern const char lbl_805E1008[];
extern char lbl_8061C9F0[];
extern const int lbl_805E101C[];
extern void* lbl_8061EAEC;

int fn_804089B4(char* buf, const char* fmt, ...);
void fn_802AD2B8(void* a, void* b, const char* name, void* c, int d);
void fn_8035A71C(void* a, void* b);
void fn_80314D60(int a);
extern "C" void* fn_803773D8(unsigned int n);
void fn_802BB4BC(Unk802BB1E4_Obj* o);
void fn_802BB4B8(Unk802BB1E4_Obj*, Unk802BB1E4_Obj*);
void RestoreOverlay(Unk802BB1E4_Obj* a, Unk802BB1E4_Obj* b);
void PrepareForDataUsage(Unk802BB1E4_Obj* a, Unk802BB1E4_Obj* b);
struct Unk802BB0E4_O;
Unk802BB0E4_O* fn_802BB0E4(Unk802BB0E4_O* p);

struct Unk802BAAB0_List {
    int w[0x52];
};
extern const char lbl_806604E0[];
void* fn_8030AC4C(Unk802BAAB0_List* p);

struct Unk802BAAB0_SlotD {
    int m0;
    Unk802BAAB0_List m4;
    int m14C;
    int m150;
    int m154;
    int m158;
    int m15C;
    int m160;
};

class Unk802BAAB0_Slot : public Unk802BAAB0_SlotD {
public:
    Unk802BAAB0_Slot()
    {
        m0 = 0;
        fn_8030AC4C(&m4);
        m14C = 0;
        m150 = 0;
        m154 = 0;
        m158 = 0;
        m15C = 0;
        m160 = 0;
    }
    virtual ~Unk802BAAB0_Slot();
};

struct Unk802BAAB0_H3 {
    Unk802BAAB0_H3()
    {
        m0 = 0;
        m4 = 0;
        m8 = 0;
    }
    int m0;
    int m4;
    int m8;
};

struct Unk802BAAB0 {
    Unk802BAAB0();
    int m0;
    Unk802BAAB0_H3 h4;
    Unk802BAAB0_Slot a[8];
    Unk802BAAB0_H3 hB50;
    Unk802BAAB0_Slot b[8];
    Unk802BAAB0_H3 h169C;
    Unk802BAAB0_Slot c;
};

/* near-miss (72 vs 73 insns): retail addresses the last slot off this (no pointer pseudo) and keeps this/zero/pointer in r30/r29/r31; ours r29/r31/r30 */
Unk802BAAB0::Unk802BAAB0() : m0(0)
{
}

struct Unk802BB948;
void fn_802BB948(Unk802BB948* p);
extern "C" void fn_802B961C(const char* p, int a);

struct Unk802BAAB0D_H3 {
    ~Unk802BAAB0D_H3() { fn_802BB948((Unk802BB948*)this); }
    int m0;
    int m4;
    int m8;
};

struct Unk802BAAB0D_SlotI : public Unk802BAAB0_SlotD {
    ~Unk802BAAB0D_SlotI() { vp = lbl_806604E0; }
    const void* vp;
};

struct Unk802BAAB0D {
    ~Unk802BAAB0D();
    int m0;
    Unk802BAAB0D_H3 h4;
    Unk802BAAB0_Slot a[8];
    Unk802BAAB0D_H3 hB50;
    Unk802BAAB0_Slot b[8];
    Unk802BAAB0D_H3 h169C;
    Unk802BAAB0D_SlotI c;
};

Unk802BAAB0D::~Unk802BAAB0D()
{
    if (m0 != 0) {
        fn_802B961C((const char*)m0, 1);
        m0 = 0;
    }
    fn_802BB948((Unk802BB948*)&h4);
    fn_802BB948((Unk802BB948*)&hB50);
}

void fn_802BACDC(Unk802BACDC_Bat* b)
{
    char buf[0x40];
    fn_804089B4(buf, lbl_805E0FBC, lbl_805DFC34);
    b->m0 = (int)fn_802B9300(buf, 0x400, 1, 0);
    fn_802BAFD0(b);
    fn_802BB038(b);
}

void* fn_802BAD48(void* o, char* base, int idx)
{
    fn_8035A71C(o, base + (idx * 0x168 + 0x10));
    return o;
}

void* fn_802BAD84(void* o, char* base, int idx)
{
    fn_8035A71C(o, base + (idx * 0x168 + 0xB5C));
    return o;
}

struct Unk802BADC0_Hdr {
    int m0;
    int m4;
    int m8;
};

struct Unk802BADC0_S {
    int m0;
    char m4[0x148];
    int m14C;
    int m150;
    int m154;
    int m158;
    int m15C;
    int m160;
};

struct Unk802BADC0_T {
    Unk802BADC0_T()
    {
        m0 = 0;
        m8 = 0;
    }
    int m0;
    int m4;
    int m8;
};

extern const char lbl_805DEFC4[];
extern const char lbl_805DEFDC[];
extern const char lbl_805DEFE4[];
extern const char lbl_805E0FD4[];
extern const char lbl_805E0FDC[];
extern const char lbl_8060D190[];
extern void* (*lbl_8062C668)(int size, int heap);
extern "C" void fn_80312700();
extern "C" void* fn_803E9640(void* a, const char* b, int c, void (*f)());
extern "C" void fn_803E9994(int a, const char* b, int* c, Unk802BADC0_T* d);
int fn_803EBD14(int a, const char* b);
void fn_803D74A0(int h, const char* s, int v);
extern "C" void fn_803D775C(int* out, int h, const char* s, ...);
extern "C" void fn_803D7378(int* out, int h, const char* s, ...);
extern "C" void fn_8030ACCC(void* p);
void fn_80305C88(void* p, int v);
void fn_80305C30(void* base, char* a, int b, int c);

/* near-miss (131/132): retail recomputes slots + off for the fn_80305C30 base (add r3,r26,r28); ours reuses the slot pointer (mr r3,r31) */
void fn_802BADC0(Unk802BACDC_Bat* b, const char* name, Unk802BADC0_Hdr* hdr, char* slots, char* arg4, int arg5)
{
    unsigned int sz;
    hdr->m0 = (int)fn_802B9300(name, 0x400, 1, &sz);
    hdr->m8 = 1;
    hdr->m4 = (int)fn_803E9640(lbl_8062C668(0x30, 0), (const char*)hdr->m0, sz, fn_80312700);
    int cnt = 0;
    unsigned char i = 0;
    do {
        Unk802BADC0_T t;
        char buf[0x10];
        fn_803E9994(hdr->m4, lbl_8060D190, &cnt, &t);
        int h = t.m8;
        int next = i + 1;
        int off = i * 0x168;
        fn_804089B4(buf, lbl_805E0FD4, next);
        Unk802BADC0_S* slot = (Unk802BADC0_S*)(off + (int)slots);
        fn_803D74A0(h, lbl_805E0FDC, fn_803EBD14(b->m0, buf));
        fn_80314D60(h);
        slot->m0 = 0;
        fn_8030ACCC(slot->m4);
        slot->m0 = h;
        if (h != 0) {
            int v1, v2, v3, v4, v5, v6;
            fn_803D775C(&v1, slot->m0, lbl_805DEFC4);
            slot->m14C = v1;
            fn_803D7378(&v2, slot->m0, lbl_805DEFC4);
            slot->m150 = v2;
            fn_803D775C(&v3, slot->m0, lbl_805DEFDC);
            slot->m154 = v3;
            fn_803D7378(&v4, slot->m0, lbl_805DEFDC);
            slot->m158 = v4;
            fn_803D775C(&v5, slot->m0, lbl_805DEFE4);
            slot->m15C = v5;
            fn_803D7378(&v6, slot->m0, lbl_805DEFE4);
            slot->m160 = v6;
            fn_80305C88(slot, 0);
        }
        if (arg4 != 0)
            fn_80305C30((char*)(off + (int)slots), arg4, arg5, 0);
        i = next;
    } while (i <= 7);
}

void fn_802BAFD0(Unk802BACDC_Bat* b)
{
    fn_802BADC0(b, lbl_805E0FE4, (Unk802BADC0_Hdr*)&b->m4, (char*)&b->m10, 0, 0);
    Unk802BACDC_Bat* c = b;
    char* d = lbl_8061C9F0;
    fn_802BADC0(c, lbl_805E0FF4, (Unk802BADC0_Hdr*)&c->mB50, (char*)&c->mB5C, d, 2);
}

void fn_802BB038(Unk802BACDC_Bat* b)
{
    char* d = lbl_8061C9F0;
    fn_802AD2B8(&b->m16A8, &b->m169C, lbl_805E1008, d, 2);
}

void fn_802BB074(Unk802BACDC_Bat* b)
{
    unsigned char i;
    for (i = 0; i <= 7; i++) {
        fn_80314D60(*(int*)((char*)b + i * 0x168 + 0x10));
    }
    for (i = 0; i <= 7; i++) {
        fn_80314D60(*(int*)((char*)b + i * 0x168 + 0xB5C));
    }
}

struct Unk802BB0E4_O {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    const char* m18;
    int m1C;
    int m20;
    int m24;
    int m28;
    int m2C;
    int m30;
    const char* m34;
};

extern const char lbl_805E1024[];
extern const char lbl_805E1040[];
extern const char lbl_805E105C[];
extern const char lbl_805E108C[];
extern const char lbl_805E1B78[];
extern "C" void fn_8043FDC0();
extern "C" void fn_8052A220();
extern "C" int fn_80400560(int p, int n);
extern "C" void* fn_8014EB44();
extern "C" void fn_8014EB90(void* h, int ok, const char* msg);
void* fn_8040B320(void* dst, int c, unsigned int n);
void fn_802BB400(Unk802BB1E4_Obj*, Unk802BB1E4_Obj*);
void fn_802BB474(Unk802BB1E4_Obj*, Unk802BB1E4_Obj*);
extern "C" int fn_803FED48(const char* name, int a, int b, void* c);
extern "C" int fn_803FEDB8(void* p, int a);
int fn_802B9A04(int a);
int fn_802B9954(int a, int b, int c);

/* near-miss (parked): retail forms the end addresses 0x8052A208 / 0x805A2BB0 as lis+addi (address-style @ha/@l); a plain integer constant gives lis+ori */
Unk802BB0E4_O* fn_802BB0E4(Unk802BB0E4_O* p)
{
    p->m8 = 0;
    p->mC = 0;
    p->m10 = 0;
    p->m0 = (int)fn_8043FDC0;
    p->m4 = 0x8052A208 - (int)fn_8043FDC0;
    p->m14 = fn_80400560(p->m0, p->m4);
    p->m2C = 0;
    p->m24 = 0;
    p->m28 = 0;
    p->m20 = 0x805A2BB0 - (int)fn_8052A220;
    p->m18 = lbl_805E1024;
    p->m1C = (int)fn_8052A220;
    p->m30 = fn_80400560(p->m1C, p->m20);
    p->m34 = lbl_805E1040;
    return p;
}

void* fn_802BB19C()
{
    if (lbl_8061EAEC == 0) {
        lbl_8061EAEC = fn_802BB0E4((Unk802BB0E4_O*)fn_803773D8(0x38));
    }
    return lbl_8061EAEC;
}

void fn_802BB1E4(Unk802BB1E4_Obj* o, int k)
{
    fn_802BB4BC(o);
    switch (k) {
    case 0:
        RestoreOverlay(o, o);
        RestoreOverlay(o, o + 1);
        break;
    case 1:
        PrepareForDataUsage(o, o);
        PrepareForDataUsage(o, o + 1);
        break;
    case 2:
        PrepareForDataUsage(o, o);
        break;
    }
}

int fn_802BB27C(char* o, int idx)
{
    char* b = o + 8;
    int t = *(int*)((char*)lbl_805E101C + idx * 4);
    return *(int*)(b + t * 0x1C);
}

void PrepareForDataUsage(Unk802BB1E4_Obj* self, Unk802BB1E4_Obj* o)
{
    void* r = fn_8014EB44();
    int cs = fn_80400560(o->m0, o->m4);
    fn_8014EB90(r, cs == o->m14, lbl_805E105C);
    o->m10 = 1;
    fn_8040B320((void*)o->m0, 0, o->m4);
    fn_802BB4B8(self, o);
    fn_802BB400(self, o);
    fn_802BB4BC(self);
}

void RestoreOverlay(Unk802BB1E4_Obj* self, Unk802BB1E4_Obj* o)
{
    int z = 0;
    int sp8;
    fn_802BB4BC(self);
    o->m10 = z;
    fn_802BB474(self, o);
    fn_8040B320((void*)o->m0, 0, o->m4);
    sp8 = z;
    fn_803FED48(lbl_805E1B78, 0x100, 0, &sp8);
    int v = fn_802B9A04(o->m18);
    fn_802B9954(o->m18, o->m0, v);
    fn_803FEDB8((void*)sp8, 0);
    sp8 = z;
    void* r = fn_8014EB44();
    int cs = fn_80400560(o->m0, o->m4);
    fn_8014EB90(r, cs == o->m14, lbl_805E108C);
}

void fn_802BB400(Unk802BB1E4_Obj*, Unk802BB1E4_Obj* o)
{
    int h = fn_803ED9D4();
    o->m8 = h;
    int hi = o->m4;
    o->mC = MEMCLASS_create(h, lbl_805E10B8, o->m0 + 0x10, hi - 0x20, 0x20, 0x20, 0, 0, 0, 1);
}

void fn_802BB474(Unk802BB1E4_Obj*, Unk802BB1E4_Obj* o)
{
    MEMCLASS_purge(o->m8);
    MEMCLASS_remove(o->m8);
    o->m8 = 0;
    o->mC = 0;
}

void fn_802BB4B8(Unk802BB1E4_Obj*, Unk802BB1E4_Obj*)
{
}
