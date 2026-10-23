/*
 * EA shared library code that uses small data (cflags_ealib_sda: -O2, default -G8):
 * VP6/MAD video codecs and SPCH, or RealCore,
 * .text 0x803ED700-0x803EE8F8 (code-only unit, 32 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

/*
 * RealCore heap manager types (src/ealib/tu_803ED700.cpp and neighbours).
 * Evidence: fn_803ED700 (block header init), FREE_add/fn_803EDEE8 (free list
 * insert/unlink through +0x10/+0x14), MEM_allocaligna (allocator), heap pointer table lbl_806AABF4[64].
 * Block tags: 0x4246 'BF' free (linked), 0x424D 'BM' allocated, 0x424F 'BO' unlinked, 0x4253 'BS' sentinel.
 */
struct RcBlk {
    unsigned short m0;
    unsigned short m2;
    int m4;
    RcBlk* m8;
    RcBlk* mC;
    RcBlk* m10;
    RcBlk* m14;
};

struct RcHeap {
    int m0;
    int m4;
    RcBlk* m8;
    int mC;
    RcBlk head;
    int m28;
    int m2C;
    int m30;
    int m34;
    int m38;
    char m3C[0x1c];
    void* m58;
};

extern RcHeap* lbl_806AABF4[64];

extern "C" int fn_804007FC(void*);
extern "C" void MEM_fill(void*, int, int);
extern "C" int MEM_free(void*);
extern "C" int fn_803EE020(int, const void*, int (*)(const char*, ...));
extern "C" int fn_803EE81C();
extern "C" void fn_803EE878();
extern "C" int fn_803EDF10(const char* fmt, ...);
extern "C" void fn_803EE238();
extern "C" int MEMCLASS_create(int, const void*, int, int, int, int, int, int, int, int);
extern "C" int fn_803EE318(char* p);
extern "C" int fn_803EE320(char* p);
extern "C" int checksentinel(RcBlk*);
extern "C" int MEM_crc16(const void* p, int n, int seed);
extern char lbl_8060DF90[];
extern char lbl_8060DFE8[];
extern char lbl_8060E040[];
extern char lbl_8060E054[];
extern char lbl_8060E058[];
extern char lbl_8060E064[];
extern char lbl_8060E068[];
extern int lbl_8060E098[];
extern "C" int fn_803ED170(int a);
extern "C" void fn_803EDEE8(RcBlk* b);

struct RcInfo {
    int m0, m4, m8, mC, m10, m14, m18, m1C;
    RcInfo(int v0, int v8, int v14, int v1C, int v4, int v18)
    {
        m0 = v0;
        mC = 0;
        m10 = 0;
        m8 = v8;
        m14 = v14;
        m1C = v1C;
        m4 = v4;
        m18 = v18;
    }
    RcInfo(int v18)
    {
        m0 = 0;
        m4 = 0;
        m8 = 0;
        mC = 0;
        m10 = 0;
        m14 = 0;
        m1C = 0;
        m18 = v18;
    }
};

struct RcCb {
    int m0, m4;
    void (*m8)(RcInfo*);
    void (*mC)(RcInfo*);
};

extern RcCb lbl_806AABE4;
extern "C" void FREE_add(RcHeap*, RcBlk*);
extern "C" int fn_804007D4(void*);
extern "C" char* strcpy(char*, const char*);
extern "C" int sprintf(char*, const char*, ...);
extern char lbl_8060DF50[];
extern char lbl_8060DF58[];
extern "C" int MEM_initadr(void*, int);
extern "C" int fn_803EDC04(int);
extern "C" int SYSTEM_addexit(void (*)(void));
extern "C" void* OSAllocFromHeap(void*, int);
extern "C" int OSGetArenaLo();
extern "C" int OSGetArenaHi();
extern "C" int OSInitAlloc(int, int, int);
extern "C" int OSSetArenaLo(int);
extern "C" int OSCreateHeap(int, int);
extern "C" int OSSetCurrentHeap();
extern char lbl_8060DF60[];
extern char lbl_8060DF64[];
extern void* lbl_806EF2C8;
extern int lbl_806EE9F0;
extern int lbl_806EEA08;
extern int lbl_806EEA0C;
extern int lbl_806EEA00;
extern "C" int fn_80400828(void*);
extern "C" int fn_8040084C(void*);
extern "C" RcBlk* FREE_findlargest(RcHeap*, int, int);
extern "C" int FREE_gettotalfree(RcHeap*, int);
extern "C" int MEMSYS_validaddress(void*, int, int);
extern "C" int fn_803EE450(RcBlk*);
extern "C" int checksentinel(RcBlk*);
extern "C" int fn_803EE5A0(RcBlk*);
extern "C" int fn_803EE688(RcBlk*);
extern "C" int fn_803EE708(int);

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
extern "C" int fn_80400ADC(int level, const char* s, ...);

inline unsigned RcRdBE(const unsigned char* b)
{
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

extern "C" int fn_803ED700(RcBlk* b, int a, int size, int c, int tag, int flags, RcBlk* d, RcBlk* e)
{
    b->mC = d;
    b->m0 = tag;
    b->m2 = flags;
    b->m8 = e;
    b->m4 = size;
    return size + 0x10;
}

extern "C" int MEMCLASS_create(int idx, const void* nm, int bs, int size, int a4, int align, int hdr, int fa, int fb, int fc)
{
    const char* name = (const char*)nm;
    RcBlk* base = (RcBlk*)bs;
    char buf[0x100];
    int flags = idx;
    if (fa != 0)
        flags |= 0x2000;
    if (fb != 0)
        flags |= 0x1000;
    int t1 = hdr + 0x6c;
    int t2 = align + 0x2f;
    int t3 = hdr + 0x30;
    int e = ((int)base + t1 + t2) & -align;
    RcBlk* end = (RcBlk*)((int)base + size - t3);
    RcBlk* mid = (RcBlk*)(e - 0x10);
    sprintf(buf, lbl_8060DF50, name);
    RcHeap* h = (RcHeap*)((char*)base + 0x10);
    fn_803ED700(base, (int)buf, 0x5c, hdr, 0x4253, flags | 0x8000, 0, mid);
    fn_803ED700(mid, 0, (int)end - (int)mid - 0x10, hdr, 0x4246, flags, base, end);
    sprintf(buf, lbl_8060DF58, name);
    fn_803ED700(end, (int)buf, 0, hdr, 0x4253, flags | 0x8100, mid, 0);
    lbl_806AABF4[idx & 0x3f] = h;
    MEM_fill(h, 0, 0x5c);
    strcpy((char*)h, name);
    h->mC = (int)end;
    h->m8 = base;
    base->m0 = 0x4253;
    ((RcBlk*)h->mC)->m0 = 0x4253;
    h->head.m10 = &h->head;
    h->m28 = a4;
    h->m2C = align;
    h->m30 = hdr;
    h->m34 = flags;
    h->head.m0 = 0x4253;
    h->head.m14 = &h->head;
    h->head.m4 = 0x7fffffff;
    h->m38 = 0;
    FREE_add(h, mid);
    if (fc != 0) {
        fn_804007D4((char*)base + 0x4c);
        h->m38 = 1;
    }
    return mid->m4;
}

extern "C" int MEMCLASS_remove(int idx)
{
    RcHeap** tab = lbl_806AABF4;
    int off = (idx & 0x3f) * 4;
    int r = 0;
    RcHeap* h = *(RcHeap**)((char*)tab + off);
    if (h != 0) {
        if (h->m38 != 0)
            fn_804007FC(h->m3C);
        MEM_fill(h, 0, 0x5c);
        *(RcHeap**)((char*)tab + off) = 0;
        r = 1;
    }
    return r;
}

extern "C" int MEMCLASS_purge(int idx)
{
    RcHeap** tab = lbl_806AABF4;
    int off = (idx & 0x3f) * 4;
    int r = 1;
    RcBlk* b = (*(RcHeap**)((char*)tab + off))->m8;
    for (; b != 0 && r != 0; b = b->m8) {
        if ((b->m2 & 0xc000) == 0)
            r = MEM_free((char*)b + 0x10);
    }
    return r;
}

extern "C" int fn_803ED9D4()
{
    int i = 2;
    if (lbl_806AABF4[2] != 0) {
        RcHeap** p = &lbl_806AABF4[2];
        for (;;) {
            i++;
            if (i > 0x3f)
                break;
            if (*++p == 0)
                break;
        }
    }
    return i;
}

/* near-miss: 2 slots; retail loads the neighbour flags halfword into r4 instead of r0 */
extern "C" int MEM_free(void* pp)
{
    char* p = (char*)pp;
    if (p == 0)
        return 1;
    RcBlk* b = (RcBlk*)(p - 0x10);
    RcHeap* h = lbl_806AABF4[b->m2 & 0x3f];
    if (h == 0)
        return 0;
    if (h->m38 != 0)
        fn_80400828(h->m3C);
    RcInfo* info = 0;
    if (lbl_806AABE4.m8 != 0) {
        int d = (int)b->m8 - (int)b;
        RcInfo i(fn_803EE318(p), d, b->m2, b->m4, (int)b, (int)p);
        info = &i;
        lbl_806AABE4.m8(info);
    }
    RcBlk* nx = b->mC;
    RcBlk* pv = b->m8;
    if (nx->m2 & 0x4000) {
        ((RcBlk* (*)(RcBlk*))fn_803EDEE8)(nx);
        b = nx;
        nx = b->mC;
        b->m8 = pv;
        nx->m8 = b;
        pv->mC = b;
    }
    if (pv->m2 & 0x4000) {
        ((RcBlk* (*)(RcBlk*))fn_803EDEE8)(pv);
        pv = pv->m8;
        b->mC = nx;
        b->m8 = pv;
        pv->mC = b;
    }
    if ((nx->m2 & 0xc000) == 0) {
        int sz = nx->m4 + fn_803ED170(0);
        sz = sz > 8 ? sz : 8;
        RcBlk* nb = (RcBlk*)((char*)nx + ((sz + 0x13) & ~3));
        if (b != nb) {
            unsigned short f = b->m2;
            nb->m2 = f;
            b = nb;
            b->m8 = pv;
            b->mC = nx;
            nx->m8 = b;
            pv->mC = b;
        }
    }
    FREE_add(h, b);
    if (lbl_806AABE4.mC != 0) {
        if (info == 0) {
            RcInfo i2((int)p);
            info = &i2;
        }
        lbl_806AABE4.mC(info);
    }
    if (h->m38 != 0)
        fn_8040084C(h->m3C);
    return 1;
}

extern "C" int fn_803EDC04(int a)
{
    int r = 0;
    SYSTEM_addexit(fn_803EE238);
    fn_803EE878();
    void* p = OSAllocFromHeap(lbl_806EF2C8, a);
    if (p != 0) {
        r = MEM_initadr(p, a);
        lbl_806AABF4[0]->m58 = p;
    }
    return r;
}

extern "C" int fn_803EDC74()
{
    return fn_803EDC04(fn_803EE81C());
}

extern "C" int MEM_initadr(void* a, int b)
{
    return MEMCLASS_create(0, lbl_8060DF60, (int)a, b, 0x20, 0x20, 0, 0, 0, 1);
}

extern "C" RcBlk* FREE_findlargest(RcHeap* h, int n, int rev)
{
    RcBlk* best = 0;
    RcBlk* p = &h->head;
    n--;
    if (n < 0)
        n = 0;
    if (rev == 0) {
        for (;;) {
            p = p->m10;
            if (p->m4 - 0x10 > n) {
                if (p->m0 == 0x4253)
                    break;
                best = p;
                n = best->m4 - 0x10;
            }
        }
    } else {
        for (;;) {
            p = p->m14;
            if (p->m4 - 0x10 > n) {
                if (p->m0 == 0x4253)
                    break;
                best = p;
                n = best->m4 - 0x10;
            }
        }
    }
    return best;
}

extern "C" RcBlk* FREE_find(RcHeap* h, int n, int rev, int off, int mask, int* out)
{
    RcBlk* p = &h->head;
    int pad,address;
    if (rev == 0) {
        do {
            p = p->m10;
            address=(int)p+off;pad=((mask & ~address)+1)&mask;
        } while (n + pad > p->m4);
    } else {
        do {
            p = p->m14;
            address=(int)p+(p->m4+off);pad=address&mask;
        } while (n + pad > p->m4);
    }
    if (p->m0 == 0x4253)
        p = 0;
    *out = pad;
    return p;
}

extern "C" int FREE_gettotalfree(RcHeap* h, int rev)
{
    RcBlk* p = &h->head;
    int total = 0;
    if (rev == 0) {
        p = p->m10;
        while (p->m0 != 0x4253) {
            total += p->m4 - 0x10;
            p = p->m10;
        }
    } else {
        p = p->m14;
        while (p->m0 != 0x4253) {
            total += p->m4 - 0x10;
            p = p->m14;
        }
    }
    return total;
}

extern "C" void FREE_add(RcHeap* h, RcBlk* blk)
{
    RcBlk* head = &h->head;
    RcBlk* prev = head;
    RcBlk* next = head;
    int size = (char*)blk->m8 - (char*)blk;
    RcBlk* mid = (RcBlk*)((char*)head->m10 + ((int)head->m14 - (int)head->m10) / 2);
    if (blk > mid) {
        do {
            prev = prev->m14;
        } while (blk < prev);
        next = prev->m10;
    } else {
        do {
            next = next->m10;
        } while (blk > next);
        prev = next->m14;
    }
    blk->m4 = size;
    blk->m10 = next;
    blk->m14 = prev;
    prev->m10 = blk;
    next->m14 = blk;
    blk->m0 = 0x4246;
    blk->m2 |= 0x4000;
}

extern "C" void fn_803EDEE8(RcBlk* b)
{
    RcBlk* prev = b->m14;
    RcBlk* next = b->m10;
    prev->m10 = next;
    next->m14 = prev;
    b->m0 = 0x424f;
    b->m2 &= 0xbfff;
}

extern "C" int fn_803EDF10(const char* fmt, ...)
{
    va_list ap;
    char buf[0x200];
    char* p = buf;
    va_start(ap, fmt);
    vsprintf(p, fmt, ap);
    return fn_80400ADC(2, p);
}

extern "C" int fn_803EDFCC(int a)
{
    return fn_803EE020(a, lbl_8060DF64, fn_803EDF10);
}

extern "C" int fn_803EDFFC()
{
    return fn_803EDFCC(lbl_806EE9F0);
}

/* near-miss: 2 slots; mtlr and addi of the lbl_8060E068 argument are swapped before the total printf */
extern "C" int fn_803EE020(int idx, const void* out, int (*pr)(const char*, ...))
{
    int total = 0;
    int crc = 0;
    pr(lbl_8060DF90);
    pr(lbl_8060DFE8);
    RcHeap* h = lbl_806AABF4[idx & 0x3f];
    int r;
    if (h == 0) {
        r = pr(lbl_8060E040);
    } else {
    RcBlk* b = h->m8;
    if (b != 0) {
        do {
            char* data = (char*)b + 0x10;
            const char* nm = (const char*)fn_803EE318(data);
            if (nm == 0)
                nm = lbl_8060E054;
            const char* nm2 = (const char*)fn_803EE320(data);
            if (nm2 == 0)
                nm2 = lbl_8060E054;
            if (b->m2 & 0x4000) {
                int sz = b->m4;
                if (sz > 0x10 && !(b->m2 & 0x8000))
                    crc = MEM_crc16((char*)b + 0x18, sz - 0x18, 0xfbea);
                total += sz;
                pr((const char*)out, lbl_8060E058, data, sz, (int)b->m8 - (int)b, b->m2, crc, lbl_8060E098[checksentinel(b)], lbl_8060E054);
            } else {
                crc = 0;
                if (b->m4 != 0 && !(b->m2 & 0x8000))
                    crc = MEM_crc16(data, b->m4, 0xfbea);
                pr((const char*)out, nm, data, b->m4, (int)b->m8 - (int)b, b->m2, crc, lbl_8060E098[checksentinel(b)], nm2);
            }
            pr(lbl_8060E064);
            b = b->m8;
        } while (b != 0);
    }
    pr(lbl_8060E068, total, total);
        r = pr(lbl_8060DFE8);
    }
    return r;
}

extern "C" void OSFreeToHeap(void*, void*);

extern "C" void fn_803EE238()
{
    int i, j;
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            if (i != j) {
                RcHeap* a = lbl_806AABF4[i];
                if (a == lbl_806AABF4[j] && a != 0)
                    lbl_806AABF4[j] = 0;
            }
        }
    }
    for (i = 62; i >= 0; i--) {
        RcHeap* h = lbl_806AABF4[i];
        if (h != 0) {
            void* m = h->m58;
            MEMCLASS_remove(i);
            if (m != 0)
                OSFreeToHeap(lbl_806EF2C8, m);
            lbl_806AABF4[i] = 0;
        }
    }
}

extern "C" int fn_803EE310(char* p)
{
    return *(int*)(p - 0xc);
}

extern "C" int fn_803EE318(char* p)
{
    return 0;
}

extern "C" int fn_803EE320(char* p)
{
    RcBlk* b = (RcBlk*)(p - 0x10);
    int r = 0;
    if (b->m2 & 0x800)
        r = *(int*)(p + b->m4 + 4);
    return r;
}

extern "C" int MEM_largestunused(int idx)
{
    if (idx == 0)
        idx = lbl_806EE9F0;
    RcHeap* h = lbl_806AABF4[idx & 0x3f];
    if (h->m38 != 0)
        fn_80400828(h->m3C);
    RcBlk* b = FREE_findlargest(h, 0, 0);
    if (h->m38 != 0)
        fn_8040084C(h->m3C);
    return b ? b->m4 : 0;
}

extern "C" int MEM_totalunused(int idx)
{
    if (idx == 0)
        idx = lbl_806EE9F0;
    RcHeap* h = lbl_806AABF4[idx & 0x3f];
    if (h->m38 != 0)
        fn_80400828(h->m3C);
    int r = FREE_gettotalfree(h, 0);
    if (h->m38 != 0)
        fn_8040084C(h->m3C);
    return r;
}

extern "C" int fn_803EE450(RcBlk* p)
{
    int r = 0;
    if (MEMSYS_validaddress(p, 0x18, 0) != 0 && (p->m0 == 0x4246 || p->m0 == 0x4253))
        r = 1;
    else
        lbl_806EEA00 = 1;
    return r;
}

extern "C" int checksentinel(RcBlk* p)
{
    int r = 0;
    int t = p->m0;
    if (t == 0x4253) {
        if (p->m2 & 0x8000)
            r = 1;
        else
            lbl_806EEA00 = 2;
    } else if (t == 0x4246) {
        if (p->m2 & 0x4000) {
            r = fn_803EE450(p->m10);
            if (r != 0)
                r = fn_803EE450(p->m14);
        }
    } else if (t == 0x424d) {
        if (p->m2 & 0x2000) {
            if (RcRdBE((unsigned char*)(p->m4 + (int)p) + 0x10) == 0x42454e44)
                r = 1;
            else
                lbl_806EEA00 = 3;
        } else
            r = 1;
    } else
        lbl_806EEA00 = 4;
    return r;
}

extern "C" int fn_803EE5A0(RcBlk* p)
{
    int r = 0;
    if (p != 0) {
        int t = p->m0;
        if (t == 0x4253) {
            if (p->m2 & 0x8000)
                r = 1;
            else
                lbl_806EEA00 = 2;
        } else if (t == 0x4246) {
            if (p->m2 & 0x4000) {
                RcBlk* n = p->m10;
                if (n != 0 && n->m14 == p) {
                    RcBlk* q = p->m14;
                    if (q != 0)
                        r = q->m10 == p;
                }
                if (r == 0)
                    lbl_806EEA00 = 5;
            }
        } else if (t == 0x424d) {
            RcBlk* n = p->m8;
            if (n != 0 && n->mC == p) {
                RcBlk* q = p->mC;
                if (q != 0)
                    r = q->m8 == p;
            }
            if (r == 0)
                lbl_806EEA00 = 5;
        } else
            lbl_806EEA00 = 4;
    }
    return r;
}

extern "C" int fn_803EE688(RcBlk* p)
{
    RcBlk* head = p;
    int r = 0;
    do {
        p = p->m10;
    } while ((*(unsigned*)p & 0xffff4000) == 0x42464000);
    if (p->m0 == 0x4253) {
        do {
            p = p->m14;
        } while ((*(unsigned*)p & 0xffff4000) == 0x42464000);
        if (p == head)
            r = 1;
    }
    if (r == 0)
        lbl_806EEA00 = 6;
    return r;
}

extern "C" int fn_803EE708(int idx)
{
    RcHeap* h = lbl_806AABF4[idx & 0x3f];
    if (h->m38 != 0)
        fn_80400828(h->m3C);
    RcBlk* b = h->m8;
    int ok;
    do {
        ok = 0;
        if (checksentinel(b) != 0) {
            if (fn_803EE5A0(b) != 0)
                ok = 1;
        }
        if (ok != 0)
            b = b->m8;
    } while (b != 0 && ok != 0);
    if (ok != 0)
        ok = fn_803EE688(&h->head);
    if (h->m38 != 0)
        fn_8040084C(h->m3C);
    return ok;
}

extern "C" int fn_803EE7BC()
{
    int r = 1;
    RcHeap** p = lbl_806AABF4;
    int i = 0;
    do {
        if (*p++ != 0)
            r = fn_803EE708(i);
        i++;
    } while (i <= 0x3f && r != 0);
    return r;
}

extern "C" int fn_803EE81C()
{
    int r;
    if (lbl_806EEA0C != 0)
        r = 0;
    else {
        int a = OSGetArenaLo();
        int b = OSGetArenaHi();
        a = (a + 0x1f) & ~0x1f;
        b &= ~0x1f;
        r = b - a - lbl_806EEA08;
    }
    return r;
}

extern "C" void fn_803EE878()
{
    if (lbl_806EEA0C == 0) {
        lbl_806EEA0C = 1;
        int a = OSGetArenaLo();
        int b = OSGetArenaHi();
        a = OSInitAlloc(a, b, 1);
        b &= ~0x1f;
        OSSetArenaLo(a);
        OSCreateHeap((a + 0x1f) & ~0x1f, b);
        OSSetCurrentHeap();
        OSSetArenaLo(b);
    }
}
