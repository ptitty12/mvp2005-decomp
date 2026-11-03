/*
 * EA shared library code that uses small data (cflags_ealib_sda: -O2, default -G8):
 * VP6/MAD video codecs and SPCH, or RealCore,
 * .text 0x803EB2AC-0x803EBF90 (code-only unit, 29 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

/* Handler table registered through fn_803EB520: hooks at +0xC (enter) and +0x10 (leave). */
struct Unk803EB2AC_Hooks {
    char pad0[0xC];
    void (*mC)();
    void (*m10)();
};

extern Unk803EB2AC_Hooks* lbl_806EE9D8;
extern void* lbl_806EE9DC;
extern int lbl_8062C76C[];

extern "C" void REAL_removeexit(void (*cb)());
extern "C" void MEM_copy(int* a, int* b, int c);
extern "C" void FONT_destroy(void* p);
extern "C" void FONT_restore(void);
extern "C" void SYSTEM_addexit(void (*cb)());
extern "C" void* FONT_create(void* self);
extern char lbl_8062C7A0[];
extern "C" int SHAPE_type(int c);
extern "C" int fn_803EC03C(char* p);

/* binary search over `n` records of `size` bytes whose first halfword is the key */
extern "C" void* FONT_bsearch(int key, char* base, int n, int size)
{
    while (n != 0) {
        int off = (n >> 1) * size;
        unsigned short v = *(unsigned short*)(base + off);
        char* p = base + off;
        int d = key - v;
        if (d == 0)
            return p;
        if (d > 0) {
            base = p + size;
            n--;
        }
        n >>= 1;
    }
    return 0;
}

extern "C" void* FONT_create(void* self)
{
    if (lbl_806EE9D8->mC)
        lbl_806EE9D8->mC();
    return self;
}

extern "C" void FONT_destroy(void* p)
{
    if (lbl_806EE9D8->m10)
        lbl_806EE9D8->m10();
}

/* swaps the case of an ASCII or Latin-1 letter, 0 for anything else */
extern "C" int FONT_switchcase(int c)
{
    if ((unsigned)(c - 0x41) <= 0x19)
        return c + 0x20;
    if ((unsigned)(c - 0x61) <= 0x19)
        return c - 0x20;
    if ((unsigned)(c - 0xC0) <= 0x16)
        return c + 0x20;
    if ((unsigned)(c - 0xD8) <= 0x6)
        return c + 0x20;
    if ((unsigned)(c - 0xE0) <= 0x16)
        return c - 0x20;
    if ((unsigned)(c - 0xF8) <= 0x6)
        return c - 0x20;
    return 0;
}

extern "C" int fn_80364B54(void* a, void* b, int c);

extern "C" int fn_803EB3CC(void* a, void* b)
{
    return fn_80364B54(a, b, 0);
}

extern "C" int fn_80364F34(void* a, void* b, int c);

extern "C" int fn_803EB3F0(void* a, void* b)
{
    return fn_80364F34(a, b, 0);
}

extern "C" void fn_80365314(void* menu, int a);

extern "C" void fn_803EB414(void* self, int a)
{
    fn_80365314(self, a);
}

extern "C" void fn_8036578C(void* menu, int a);

extern "C" void fn_803EB434(void* self, int a)
{
    fn_8036578C(self, a);
}

/* SysV varargs record, same as include/game/tu_8000C2C4.h */
struct va_list_tag {
    char gpr;
    char fpr;
    char* overflow_arg_area;
    char* reg_save_area;
};
typedef va_list_tag va_list[1];
#define va_start(AP, LASTARG) (__builtin_next_arg(LASTARG), *(AP) = *(va_list_tag*)__builtin_saveregs())

extern "C" int vsprintf(char* buf, const char* fmt, va_list ap);

extern "C" void FONT_drawtextfa(int obj, float f1, float f2, const char* fmt, ...)
{
    va_list ap;
    char buf[0x2000];
    char* p = buf;
    va_start(ap, fmt);
    vsprintf(p, fmt, ap);
    ((int (*)(int, float, float, char*))fn_803EB3CC)(obj, f1, f2, p);
}

extern "C" void fn_803EB520(Unk803EB2AC_Hooks* h)
{
    lbl_806EE9D8 = h;
}

extern "C" void FONT_installdriverextensions(int* p)
{
    MEM_copy(lbl_8062C76C, p, *p);
}

extern "C" int fn_803EB558(char* p)
{
    return *(int*)(p + 0x24);
}

extern "C" void FONT_restore(void)
{
    if (lbl_806EE9DC) {
        FONT_destroy(lbl_806EE9DC);
        lbl_806EE9DC = 0;
        REAL_removeexit(FONT_restore);
    }
}

extern "C" void FONT_init(void)
{
    if (lbl_806EE9DC == 0) {
        lbl_806EE9DC = FONT_create(lbl_8062C7A0);
        SYSTEM_addexit(FONT_restore);
    }
}

/* linear search of `n` 4-byte records for the one whose first halfword is `key` */
struct Unk803EB5E4_Rec {
    unsigned short m0;
    signed char m2;
    unsigned char m3;
};

/* near-miss (flags): both functions are exact at -O2 -fno-strength-reduce (0 slots, probed with the unit's other
 * flags); with the configured flags the loop gets strength-reduced and 12 / 14 slots differ. */
extern "C" Unk803EB5E4_Rec* FONT_findkern16(int n, Unk803EB5E4_Rec* tbl, int key)
{
    for (int i = 0; i < n; i++) {
        if (tbl[i].m0 == key)
            return &tbl[i];
    }
    return 0;
}

extern "C" Unk803EB5E4_Rec* FONT_findkern12(int n, Unk803EB5E4_Rec* tbl, int key, int flag)
{
    for (int i = 0; i < n; i++) {
        if (tbl[i].m0 == key && tbl[i].m3 == flag)
            return &tbl[i];
    }
    return 0;
}

struct Unk803EB660_Entry {
    unsigned short m0;
    char pad2[9];
    unsigned char mB;
    unsigned short mC;
};

struct Unk803EB660_Tbl {
    char pad0[0xC];
    unsigned int mC;
    char pad10[8];
    int m18;
};

extern "C" int FONT_getkern(Unk803EB660_Tbl* t, Unk803EB660_Entry* e, int key)
{
    signed char r = 0;
    if (e->mB) {
        Unk803EB5E4_Rec* f;
        if (t->mC & 0x40000) {
            f = FONT_findkern16(e->mB, (Unk803EB5E4_Rec*)((char*)t + t->m18 + (e->mC * 4 + 4)), key);
        } else {
            int off = t->m18;
            f = FONT_findkern12(*(int*)((char*)t + off), (Unk803EB5E4_Rec*)((char*)t + off + 4), key, e->m0);
        }
        if (f)
            r = f->m2;
    }
    return r;
}

extern "C" void fn_803EB6F4(void)
{
}

extern "C" int SHAPE_bpctobpp(int c)
{
    if (c == 0)
        c = 0x20;
    char ch = SHAPE_type(c);
    return fn_803EC03C(&ch);
}

extern "C" int SHAPE_imagesize(int w, int h, int fmt, int lvl)
{
    int k = SHAPE_bpctobpp(fmt);
    int total = 0;
    int bpp = 0x10;
    if (k != 0xf)
        bpp = k;
    if ((unsigned)(w - 1) > 0xffff || h <= 0 || h > 0x10000 || w * h > 1000000000 || SHAPE_type(k) == 0)
        return 0;
    int i;
    for (i = 0; i <= lvl; i++) {
        int ch = h >> i;
        int cw = w >> i;
        if (cw <= 0)
            cw = 1;
        if (ch <= 0)
            ch = 1;
        if (total != 0)
            total = (total + 0x1f) & ~0x1f;
        total += (cw * bpp + 7) / 8 * ch;
        if (cw == 1 && ch == 1)
            break;
    }
    return total;
}

extern "C" int fn_803EB83C(int k)
{
    switch (k) {
    case 8:
        return 0x100;
    case 4:
        return 0x10;
    default:
        return 0;
    }
}

extern "C" int SHAPE_createsize(int a, int b, int c, int d, int e, int f, int g)
{
    int k = SHAPE_bpctobpp(c);
    int sz = SHAPE_imagesize(a, b, c, e);
    sz += 0x10;
    if (k <= 8 && d != 0) {
        if (d == 0xf)
            d = 0x10;
        if (sz != 0)
            sz = (sz + 0x1f) & ~0x1f;
        int w = fn_803EB83C(k) * d / 8 + 0x10;
        sz = ((sz + 0xf) & ~0xf) + w;
    }
    if (f != 0)
        sz = sz + (8 + f);
    if (g != 0)
        sz = sz + (0x10 + g);
    return sz;
}

extern char lbl_8060DBF8[];
extern "C" int sprintf(char*, const char*, ...);
extern "C" void SHAPE_createat(int r, int a, int b, int c, int d, int e, int g, int h);
extern int (*lbl_806EE9E4)(int a, int b, int c, int d, int e);

extern "C" int SHAPE_create(int a, int b, int c, int d, int e, int f, int g, int h)
{
    int r = 0;
    int sz = SHAPE_createsize(a, b, c, d, e, g, h);
    if (sz != 0) {
        char buf[16];
        sprintf(buf, lbl_8060DBF8, a, b, c);
        r = lbl_806EE9E4((int)buf, sz, 0, 0x10, f);
        if (r != 0)
            SHAPE_createat(r, a, b, c, d, e, g, h);
    }
    return r;
}

struct Unk803EBC70_Ent {
    int off;
    int m4;
};

struct Unk803EBC70_Tbl {
    int m0, m4, m8, mC, m10;
    Unk803EBC70_Ent e[1];
};

extern "C" int SHAPE_longname(int x);
extern "C" int fn_803EC814(Unk803EBC70_Tbl* t, int i, char* out);
extern "C" int strcmp(const char*, const char*);

/* near-miss (flags): exact at -O2 -fno-strength-reduce (see FONT_findkern16); with the configured flags the loop is strength-reduced */
extern "C" char* fn_803EBC70(Unk803EBC70_Tbl* t, int name)
{
    int i;
    int n = t->m8;
    for (i = 0; i < n; i++) {
        char buf[8];
        char* s = (char*)SHAPE_longname((int)((char*)t + *(int*)((char*)t + i * 8 + 0x14)));
        if (s == 0) {
            buf[4] = 0;
            fn_803EC814(t, i, buf);
            s = buf;
        }
        if (strcmp(s, (const char*)name) == 0)
            return (char*)t + t->e[i].off;
    }
    return 0;
}

extern "C" void fn_803EBD14(Unk803EBC70_Tbl* self, int a)
{
    fn_803EBC70(self, a);
}

extern "C" int SHAPE_clonesize(int x);
extern "C" void SHAPE_cloneat(int r, int x);
extern "C" int SHAPE_isclut(int c);
extern "C" int SHAPE_size(unsigned char* n);

struct Unk803EBDF4_Nd {
    unsigned tag : 8;
    int off : 24;
    short s4;
};

extern char lbl_8060DC04[];
extern "C" void MEM_move(void* dst, void* src, int n);

/* size of a chain of nodes (24-bit relative links) with 16/32 byte alignment between them */
inline unsigned char* Unk803EBD34_Next(unsigned char* n)
{
    int o = (*(int*)n << 8) >> 8;
    unsigned char* r = 0;
    if (o != 0)
        r = n + o;
    return r;
}

extern "C" int SHAPE_clonesize(int x)
{
    unsigned char* n = (unsigned char*)x;
    int r = 0;
    int a32 = (((int)n + 0x1f) & ~0x1f) - (int)n;
    int a16 = (((int)n + 0xf) & ~0xf) - (int)n;
    if (n != 0) {
        int k = a32 - 0x10;
        do {
            int sz = SHAPE_size(n);
            if (r != 0) {
                if (SHAPE_isclut(*n) != 0)
                    r = (((r - a32) + 0x2f) & ~0x1f) + k;
                else
                    r = (((r - a16) + 0xf) & ~0xf) + a16;
            }
            r += sz;
            n = Unk803EBD34_Next(n);
        } while (n != 0);
    }
    return r;
}

extern "C" void SHAPE_cloneat(int dst, int x)
{
    Unk803EBDF4_Nd* n = (Unk803EBDF4_Nd*)x;
    int a32 = (((int)n + 0x1f) & ~0x1f) - (int)n;
    int a16 = (((int)n + 0xf) & ~0xf) - (int)n;
    int pos = 0;
    Unk803EBDF4_Nd* prev = 0;
    if (n != 0) {
        int k = a32 - 0x10;
        do {
            int sz = SHAPE_size((unsigned char*)n);
            if (pos != 0) {
                if (SHAPE_isclut(n->tag) != 0)
                    pos = (((pos - a32) + 0x2f) & ~0x1f) + k;
                else
                    pos = (((pos - a16) + 0xf) & ~0xf) + a16;
            }
            Unk803EBDF4_Nd* p0 = prev;
            prev = (Unk803EBDF4_Nd*)(dst + pos);
            if (p0 != 0)
                p0->off = (char*)prev - (char*)p0;
            prev->off = 0;
            if (sz != 0) {
                MEM_move(prev, n, sz);
                if (prev->tag == 0x69 && sz > 0x14)
                    sprintf((char*)prev + 0x10, lbl_8060DC04, prev->s4);
            }
            pos += sz;
            n = (Unk803EBDF4_Nd*)Unk803EBD34_Next((unsigned char*)n);
        } while (n != 0);
    }
}

extern "C" int SHAPE_longname(int x);

extern "C" int SHAPE_clone(int x)
{
    int a = SHAPE_clonesize(x);
    int r = lbl_806EE9E4(SHAPE_longname(x), a, 0, 0x10, 0x400);
    SHAPE_cloneat(r, x);
    return r;
}
