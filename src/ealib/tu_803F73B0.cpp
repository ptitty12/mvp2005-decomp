/*
 * EA shared library code that uses small data (cflags_ealib_sda: -O2, default -G8):
 * VP6/MAD video codecs and SPCH, or RealCore,
 * .text 0x803F73B0-0x803F9350 (code-only unit, 40 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

#define RCI(p, off) (*(int*)((char*)(p) + (off)))

struct RcSlotC {
    char pad[0x40];
    int m40;
    char pad2[0xa0];
};

struct RcIfV {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6(int);
};

extern RcIfV* lbl_806EEA34;
extern int lbl_806EEA88;
extern char lbl_8060E6E8[];

extern "C" int fn_803F73B0(void*, int);
extern "C" int CARDRename(int, const char*, const char*);
extern "C" int CARDClose(void*);
extern "C" int CARDFormat(int);
extern "C" int CARDFreeBlocks(int, int*, int*);
extern "C" int CARDSetAttributes(int, int, int);
extern "C" int fn_803F8CB4(void*, const char*, int*);
extern "C" int FindNext(void*);
extern "C" void* memset(void*, int, unsigned);
extern "C" int strncmp(const char*, const char*, unsigned);
extern "C" void fn_803F6FC4(void*, int);
extern "C" int fn_803F7EB4(void*, void*);
extern "C" int fn_803F7E98(void*, void*);

struct RcB30 {
    int w[12];
};

struct RcRecA {
    int m0, m4, m8, mC, m10;
    char m14[4];
    char m18[2];
};

extern "C" int CARDFastDelete(int, int);
extern "C" int CARDDelete(int, const char*);
extern "C" int CARDGetStatus(int, int, void*);
extern "C" int CARDRead(void*, void*, int, int);
extern "C" int CARDWrite(void*, void*, int, int);
extern "C" int strcmp(const char*, const char*);

extern "C" int CARDUnmount(int);
extern "C" int CARDProbeEx(int, int*, int*);
extern "C" int fn_803F8434(void*, void*, void*, int, int*);
extern int lbl_806EEA84;
extern int lbl_806EEA80;
/* Card status buffer (lbl_806ADDE0): FindNext copies the +0x20..+0x2C words into the open record. */
struct CardStatV {
    char pad[0x20];
    int m20;
    int m24;
    int m28;
    unsigned short m2C;
};
extern CardStatV lbl_806ADDE0;
/* Directory-scan record (pointed to by object +4). */
struct RcRecB {
    unsigned short m0;
    unsigned short m2;
    CardStatV* m4;
    int m8;
    int mC;
    int m10;
    int m14;
    unsigned short m18;
};

extern "C" int CARDCreate(int, const char*, int, void*);
extern "C" int FlushWriteBuffer(void*, void*);
extern "C" int fn_803F8F80(void*, char*);

extern "C" int CARDGetSectorSize(int, int*);
extern "C" int fn_803F78CC(void*, unsigned short*);

extern "C" int fn_803F8D8C(int unused, void* f, int off, int whence);

extern "C" int CARDMount(int, int, void (*)());
extern "C" int CARDCheck(int);
extern "C" void fn_803F773C(void);

extern "C" int fn_803F73B0(void* o, int k)
{
    switch (k) {
    case 0:
        return 0;
    case -1:
        return 1;
    case -2:
        return 2;
    case -3:
        return 3;
    case -4:
        return 4;
    case -5:
        return 5;
    case -6:
        return 6;
    case -7:
        return 7;
    case -8:
        return 8;
    case -9:
        return 9;
    case -10:
        return 10;
    case -11:
        return 11;
    case -12:
        return 12;
    case -13:
        return 13;
    case -14:
        return 14;
    case -128:
        return 15;
    }
    return 16;
}

extern "C" RcSlotC* fn_803F74B0(char* p)
{
    RcSlotC* s = (RcSlotC*)(p + 8);
    for (int i = 0; i < 3; i++) {
        if (s->m40 == 0)
            return s;
        s = (RcSlotC*)((char*)s + 0xe4);
    }
    return 0;
}

extern "C" void* fn_803F74D8(void* a, void* b)
{
    return b;
}

extern "C" int fn_803F74E0(void* o, unsigned short* h)
{
    int a = 0;
    int b = 0;
    int r;
    do {
        r = CARDProbeEx(*h, &a, &b);
        lbl_806EEA34->v6(1);
    } while (r == -1);
    int rc = fn_803F73B0(o, r);
    if (rc == 0 && b != lbl_806EEA80)
        rc = 2;
    return rc;
}

extern "C" int GetFileBlocks(void* o, int a, void* b, int* out)
{
    int n = fn_803F7EB4(o, b) + 0x44;
    if (RCI(b, 0x20) != 0)
        n += fn_803F7E98(o, b);
    n += 4;
    if (out != 0)
        *out = n;
    n += RCI(b, 4);
    unsigned d = RCI(o, 0x2b4);
    return (unsigned)(n + d - 1) / d;
}

extern "C" int fn_803F7748(void* o, unsigned short* h);

extern "C" int fn_803F7748(void* o, unsigned short* h);

extern "C" int fn_803F7604(char* o, unsigned short* p)
{
    int saved = RCI(o, 0x2c4);
    int a = 0;
    int b = 0;
    if (lbl_806EEA84 != 0) {
        if (*p != *(unsigned short*)(o + 0x2c4)) {
            fn_803F78CC(o, (unsigned short*)(o + 0x2c4));
            a = 1;
            fn_803F7748(o, p);
        }
    } else {
        b = 1;
        fn_803F7748(o, p);
    }
    int r = 0;
    int out = 0;
    do {
        r = CARDGetSectorSize(*p, &out);
        lbl_806EEA34->v6(1);
    } while (r == -1);
    int rc = fn_803F73B0(o, r);
    if (rc != 0) {
        if (rc == 3)
            out = lbl_806EEA80;
    }
    if (a) {
        fn_803F78CC(o, p);
        fn_803F7748(o, (unsigned short*)&saved);
    } else if (b) {
        fn_803F78CC(o, p);
    }
    return out;
}

extern "C" void fn_803F773C()
{
    lbl_806EEA88 = 0;
}

extern "C" int fn_803F7748(void* o, unsigned short* h)
{
    int a;
    int b;
    int r;
    int rc;
    lbl_806EEA84 = 0;
    a = 0;
    b = 0;
    do {
        r = CARDProbeEx(*h, &a, &b);
        lbl_806EEA34->v6(1);
    } while (r == -1);
    rc = fn_803F73B0(o, r);
    if (rc == 0) {
        do {
            r = CARDMount(*h, RCI(o, 0x2bc), (void (*)())fn_803F773C);
            lbl_806EEA34->v6(1);
        } while (r == -1);
        rc = fn_803F73B0(o, r);
        if (rc == 0 || rc == 0xd || rc == 6) {
            do {
                r = CARDCheck(*h);
                lbl_806EEA34->v6(1);
            } while (r == -1);
            rc = fn_803F73B0(o, r);
            switch (rc) {
            case 0:
            case 6:
            case 0xd:
                lbl_806EEA84 = 1;
            }
            if (rc != 3)
                lbl_806EEA88 = 1;
            if (lbl_806EEA84 != 0)
                RCI(o, 0x2c4) = *(int*)h;
        }
    }
    return rc;
}

extern "C" int fn_803F78CC(void* o, unsigned short* h)
{
    int r;
    if (lbl_806EEA84 != 0) {
        lbl_806EEA84 = 0;
        lbl_806EEA88 = 0;
        do {
            r = CARDUnmount(*h);
            lbl_806EEA34->v6(1);
        } while (r == -1);
        return fn_803F73B0(o, r);
    }
    return 0;
}

struct RcSave {
    char pad0[4];
    int m4;
    char pad8[4];
    char* mC;
    int m10;
    char* m14;
    int m18;
    struct RcBuf* m1C;
    struct RcBuf* m20;
};
struct RcBuf {
    int m0;
    signed char* m4;
};
extern "C" int fn_803F7EE4(void*, void*, void*, int, int*);
extern "C" void* memcpy();

extern "C" int WriteHeaderData(void* o, char* f)
{
    RcSave* h = (RcSave*)(f + 0xC);
    int res = 0;
    if (fn_803F7EE4(o, f, f + 0x10, 4, 0) == 0xF)
        res = 0xF;
    signed char buf[0x20];
    int w = h->m4;
    int sum = 0;
    signed char* p = (signed char*)&w;
    for (int i = 0; i < 4; i++)
        sum += *p++;
    int n = h->m10;
    if (n > 0x20)
        n = 0x20;
    memset(buf, 0, 0x20);
    memcpy(buf, h->mC, n);
    if (fn_803F7EE4(o, f, buf, 0x20, 0) == 0xF)
        res = 0xF;
    p = buf;
    for (int i = 0; i < 0x20; i++)
        sum += *p++;
    n = h->m18;
    if (n > 0x20)
        n = 0x20;
    memset(buf, 0, 0x20);
    memcpy(buf, h->m14, n);
    if (fn_803F7EE4(o, f, buf, 0x20, 0) == 0xF)
        res = 0xF;
    p = buf;
    for (int i = 0; i < 0x20; i++)
        sum += *p++;
    if (h->m20 != 0) {
        unsigned len = fn_803F7E98(o, h);
        if (fn_803F7EE4(o, f, h->m20->m4, len, 0) == 0xF)
            res = 0xF;
        p = h->m20->m4;
        for (unsigned i = 0; i < len; i++)
            sum += *p++;
    }
    if (h->m1C != 0) {
        unsigned len = fn_803F7EB4(o, h);
        if (fn_803F7EE4(o, f, h->m1C->m4, len, 0) == 0xF)
            res = 0xF;
        p = h->m1C->m4;
        for (unsigned i = 0; i < len; i++)
            sum += *p++;
    }
    if (fn_803F7EE4(o, f, &sum, 4, 0) == 0xF)
        res = 0xF;
    return res;
}

struct RcSlotBase {
    char d[0x44];
};
struct RcSlotV : RcSlotBase {
    virtual void v0();
    virtual void v1(unsigned short*, int*, int);
};
extern "C" int CARDOpen(int, const char*, void*);
extern "C" int CARDFastOpen(int, int, void*);
extern "C" int fn_803F8FF8(void*, char*);
extern "C" int WriteHeaderData(void*, char*);
extern "C" int SetHeaderInfo(void*, char*);
extern "C" int VerifyHeaderChecksum(void*, void*);
extern "C" int fn_803F87A8(void*, unsigned short*, int*);

/* near-miss: 98%; the out pointer and the map flag swap callee-saved registers (r21/r22) */
extern "C" int fn_803F7958(void* o, unsigned short* h, int* name, int flags, void** out)
{
    char* s = (char*)fn_803F74B0((char*)o);
    if (s == 0)
        return 0xF;
    *out = 0;
    int rc = 0xF;
    int st = 0;
    int cnt = 0;
    ((RcSlotV*)s)->v1(h, name, flags);
    if (h[1] != 1)
        cnt = GetFileBlocks(o, (int)h, s + 0xC, (int*)(s + 0xD0));
    int mapRc = 1;
    if (flags & 0x200) {
        if (h[1] == 1) {
            st = fn_803F8FF8(o, s);
        } else {
            rc = fn_803F87A8(o, h, name);
            if (rc != 5) {
                int sz = cnt * RCI(o, 0x2b4);
                do {
                    st = CARDCreate(h[0], (const char*)*name, sz, s + 0x48);
                    lbl_806EEA34->v6(1);
                } while (st == -1);
            } else
                st = -5;
        }
    } else {
        do {
            if (h[1] == 1) {
                int tmp = -1;
                rc = fn_803F8CB4(o, (const char*)*name, &tmp);
                if (rc == 0)
                    st = CARDFastOpen(h[0], tmp, s + 0x48);
                else
                    mapRc = 0;
            } else
                st = CARDOpen(h[0], (const char*)*name, s + 0x48);
            lbl_806EEA34->v6(1);
        } while (st == -1);
    }
    if (mapRc)
        rc = fn_803F73B0(o, st);
    if (rc == 0) {
        int x = RCI(s, 0x4c);
        RCI(s, 0x40) = 1;
        RCI(s, 0) = x;
        CARDGetStatus(*(unsigned short*)(s + 8), x, s + 0x5c);
        *out = s;
        if (h[1] == 1) {
            RCI(s, 0xe0) = rc;
        } else {
            RCI(s, 0xe0) = 1;
            if (flags & 0x200) {
                if (WriteHeaderData(o, s) != 0 || SetHeaderInfo(o, s) == -1)
                    rc = 0xF;
            } else
                rc = VerifyHeaderChecksum(o, s);
            if (rc != 0) {
                CARDClose(s + 0x48);
                RCI(s, 0x40) = 0;
                *out = 0;
            }
        }
    }
    return rc;
}

extern "C" int fn_803F7E98(void* a, void* b)
{
    int* t = (int*)RCI(b, 0x20);
    if (t[0] == 0)
        return 0x1800;
    return 0xe00;
}

extern "C" int fn_803F7EB4(void* a, void* b)
{
    int* t = (int*)RCI(b, 0x1c);
    if (t[10] == 0)
        return t[0] << 11;
    return t[0] * 3 << 9;
}

extern "C" int FlushWriteBuffer(void* o, void* b)
{
    int r = 0;
    char* f = (char*)fn_803F74D8(o, b);
    if (RCI(f, 0xc8) > 0) {
        char* card = f + 0x48;
        if (RCI(f, 0xd0) + RCI(f, 0x10) <= RCI(f, 0xcc) - RCI(o, 0x2b4) + RCI(f, 0xc8)) {
            RCI(f, 0xc8) = (unsigned)(RCI(f, 0xd0) + RCI(f, 0x10)) % (unsigned)RCI(o, 0x2b4);
            if (RCI(f, 0xc8) < RCI(o, 0x2b4)) {
                do {
                    ((char*)RCI(o, 0x2c0))[RCI(f, 0xc8)] = 0;
                    RCI(f, 0xc8)++;
                } while (RCI(f, 0xc8) < RCI(o, 0x2b4));
            }
        }
        int w;
        do {
            w = CARDWrite(card, (void*)RCI(o, 0x2c0), RCI(o, 0x2b4), RCI(f, 0xcc));
            lbl_806EEA34->v6(1);
        } while (w == -1);
        r = fn_803F73B0(o, w);
    }
    RCI(f, 0xd4) = 0;
    return r;
}

extern "C" int CloseFile(void* o, void* b)
{
    int rc = 0;
    char* f = (char*)fn_803F74D8(o, b);
    int r;
    if (RCI(f, 0xe0) == 0) {
        rc = fn_803F8F80(o, f);
    } else if (RCI(f, 0x3c) & 0x200) {
        if (FlushWriteBuffer(o, b) == 0xf)
            rc = 0xf;
        do {
            r = CARDClose(f + 0x48);
            lbl_806EEA34->v6(1);
        } while (r == -1);
    } else {
        do {
            r = CARDClose(f + 0x48);
            lbl_806EEA34->v6(1);
        } while (r == -1);
        rc = fn_803F73B0(o, r);
    }
    RCI(f, 0x40) = 0;
    return rc;
}

extern "C" int fn_803F870C(void* a, void* b)
{
    return RCI(fn_803F74D8(a, b), 0x10);
}

/* near-miss: 9 slots; retail loads *h (lhz) before zeroing the two out slots, order of the zero stores differs */
struct RcCardStat {
    char pad0[0x2e];
    unsigned char bannerFormat;
    char pad2f;
    unsigned iconAddr;
    unsigned short iconFormat;
    unsigned short iconSpeed;
    unsigned commentAddr;
    char pad3c[0x6c - 0x3c];
};

struct RcCardDesc {
    int m0;
    char pad4[0x24];
    int m28;
    int m2C;
};

struct RcCardFile {
    int m0;
    char pad4[4];
    unsigned short m8;
    char padA[0x1e];
    RcCardDesc* m28;
    int* m2C;
    char pad30[0x2c];
    RcCardStat stat;
};

#define RC_SET_BANNER(s, f) ((s)->bannerFormat = (unsigned char)(((s)->bannerFormat & ~3) | (f)))
#define RC_SET_ANIM(s, f) ((s)->bannerFormat = (unsigned char)(((s)->bannerFormat & ~4) | (f)))
#define RC_SET_ICONFMT(s, n, f) ((s)->iconFormat = (unsigned short)(((s)->iconFormat & ~(3 << (2 * (n)))) | ((f) << (2 * (n)))))
#define RC_SET_ICONSPD(s, n, f) ((s)->iconSpeed = (unsigned short)(((s)->iconSpeed & ~(3 << (2 * (n)))) | ((f) << (2 * (n)))))
extern "C" int CARDSetStatus(int, int, void*);

extern "C" int SetHeaderInfo(void* o, char* fp)
{
    RcCardFile* f = (RcCardFile*)fp;
    memset(&f->stat, 0, 0x6c);
    f->stat.commentAddr = 4;
    f->stat.iconAddr = 0x44;
    if (f->m2C != 0) {
        if (*f->m2C == 0)
            RC_SET_BANNER(&f->stat, 2);
        else
            RC_SET_BANNER(&f->stat, 1);
    } else
        RC_SET_BANNER(&f->stat, 0);
    int n = f->m28->m0;
    if (n <= 7) {
        RC_SET_ICONFMT(&f->stat, n, 0);
        RC_SET_ICONSPD(&f->stat, n, 0);
    }
    if (f->m28->m2C == 2)
        RC_SET_ANIM(&f->stat, 4);
    else if (f->m28->m2C == 1)
        RC_SET_ANIM(&f->stat, 0);
    for (int i = 0; i < n; i++) {
        if (f->m28->m28 == 0)
            RC_SET_ICONFMT(&f->stat, i, 2);
        else
            RC_SET_ICONFMT(&f->stat, i, 1);
        char* b = (char*)f->m28 + 8;
        int off = i * 4;
        int s = *(int*)(b + off);
        switch (s) {
        case 1:
            f->stat.iconSpeed = f->stat.iconSpeed | (3 << (2 * i));
            break;
        case 2:
            RC_SET_ICONSPD(&f->stat, i, s);
            break;
        case 3:
            RC_SET_ICONSPD(&f->stat, i, 1);
            break;
        default:
            RC_SET_ICONSPD(&f->stat, i, 2);
            break;
        }
    }
    int r;
    do {
        r = CARDSetStatus(f->m8, f->m0, &f->stat);
        lbl_806EEA34->v6(1);
    } while (r == -1);
    if (fn_803F73B0(o, r) != 0)
        return -1;
    return 0;
}

extern "C" int fn_803F8DDC(void*, char*, void*, int, int*);

extern "C" int fn_803F8434(void* o, void* f, void* buf, int len, int* outlen)
{
    char* rec = (char*)fn_803F74D8(o, f);
    if (RCI(rec, 0xe0) == 0)
        return fn_803F8DDC(o, rec, buf, len, outlen);
    unsigned char* src = (unsigned char*)buf;
    int rc = 0;
    int remain = len;
    if (RCI(rec, 0xd4) != 0)
        FlushWriteBuffer(o, f);
    int cc = RCI(rec, 0xcc);
    int total = RCI(rec, 0x7c);
    int off;
    if (cc != 0)
        off = cc - RCI(o, 0x2b4);
    else
        off = 0;
    while (remain != 0) {
        int pos = off + RCI(rec, 0xc8);
        if (pos < 0 || pos >= total) {
            rc = 0xb;
            break;
        }
        if (RCI(rec, 0xdc) != 0) {
            int st;
            do {
                st = CARDRead(rec + 0x48, (void*)RCI(o, 0x2c0), RCI(o, 0x2b4), RCI(rec, 0xcc));
                lbl_806EEA34->v6(1);
            } while (st == -1);
            rc = fn_803F73B0(o, st);
            if (rc != 0)
                break;
            RCI(rec, 0xcc) += RCI(o, 0x2b4);
            RCI(rec, 0xdc) = rc;
        }
        int i = RCI(rec, 0xc8);
        while (i < RCI(o, 0x2b4) && remain != 0) {
            if (src != 0) {
                *src = ((unsigned char*)RCI(o, 0x2c0))[i];
                src++;
            }
            remain--;
            RCI(rec, 0xc8) = RCI(rec, 0xc8) + 1;
            i = RCI(rec, 0xc8);
        }
        if (RCI(rec, 0xc8) == RCI(o, 0x2b4)) {
            RCI(rec, 0xdc) = 1;
            RCI(rec, 0xc8) = 0;
        }
    }
    if (outlen != 0)
        *outlen = len - remain;
    return rc;
}

extern "C" int fn_803F8EA4(void*, char*, void*, int, int*);

/* near-miss: 95%; same shape as retail, but the hoisted file pointer (rec + 0x48) is shared by both CARD loops here (callee-saved regs r26..r28 rotate) */
extern "C" int fn_803F7EE4(void* o, void* f, void* buf, int len, int* outlen)
{
    char* rec = (char*)fn_803F74D8(o, f);
    if (RCI(rec, 0xe0) == 0)
        return fn_803F8EA4(o, rec, buf, len, outlen);
    if (outlen != 0)
        *outlen = len;
    if (len > 0) {
        do {
            int i = RCI(rec, 0xc8);
            if (i < RCI(o, 0x2b4) && len > 0) {
                do {
                    unsigned char c = *(unsigned char*)buf;
                    buf = (unsigned char*)buf + 1;
                    len--;
                    ((unsigned char*)RCI(o, 0x2c0))[i] = c;
                    RCI(rec, 0xc8) = i + 1;
                    i = i + 1;
                    RCI(rec, 0xd4) = 1;
                } while (i < RCI(o, 0x2b4) && len > 0);
            }
            if (RCI(rec, 0xc8) == RCI(o, 0x2b4)) {
                if (RCI(rec, 0xcc) < 0)
                    return 0;
                int st;
                do {
                    st = CARDWrite(rec + 0x48, (void*)RCI(o, 0x2c0), RCI(o, 0x2b4), RCI(rec, 0xcc));
                    lbl_806EEA34->v6(1);
                } while (st == -1);
                int rc = fn_803F73B0(o, st);
                if (rc != 0)
                    return rc;
                RCI(rec, 0xcc) += RCI(o, 0x2b4);
                RCI(rec, 0xc8) = rc;
                RCI(rec, 0xd4) = rc;
                if ((unsigned)(GetFileBlocks(o, (int)(rec + 8), rec + 0xc, 0) * RCI(o, 0x2b4)) < (unsigned)RCI(rec, 0xcc)) {
                    do {
                        st = CARDRead(rec + 0x48, (void*)RCI(o, 0x2c0), RCI(o, 0x2b4), RCI(rec, 0xcc));
                        lbl_806EEA34->v6(1);
                    } while (st == -1);
                    rc = fn_803F73B0(o, st);
                    if (rc != 0)
                        return rc;
                    RCI(rec, 0xcc) += RCI(o, 0x2b4);
                } else
                    memset((void*)RCI(o, 0x2c0), 0, RCI(o, 0x2b4));
            }
        } while (len > 0);
    }
    return 0;
}

extern "C" int fn_803F8730(void* o, unsigned short* h, int* out1, int* out2)
{
    int s0 = 0;
    int s1 = 0;
    int r = 0xf;
    if (CARDFreeBlocks(*h, &s1, &s0) == 0)
        r = 0;
    *out1 = s1 / RCI(o, 0x2b4);
    *out2 = s0;
    return r;
}

extern "C" int fn_803F87A8(void* o, unsigned short* h, int* name)
{
    int r;
    do {
        if (h[1] == 1) {
            int s8 = -1;
            r = fn_803F8CB4(o, (const char*)*name, &s8);
            if (r == 0) {
                int q = CARDFastDelete(h[0], s8);
                r = fn_803F73B0(o, q);
            }
        } else {
            int q = CARDDelete(h[0], (const char*)*name);
            r = fn_803F73B0(o, q);
        }
        lbl_806EEA34->v6(1);
    } while (r == 1);
    return r;
}

extern "C" int RenameFile(void* o, unsigned short* h, int* name1, const char* name2)
{
    int r;
    do {
        r = CARDRename(*h, (const char*)*name1, name2);
        lbl_806EEA34->v6(1);
    } while (r == -1);
    return fn_803F73B0(o, r);
}

extern "C" int fn_803F88D8(void* o, unsigned short* h)
{
    int r;
    do {
        r = CARDFormat(*h);
        lbl_806EEA34->v6(1);
    } while (r == -1);
    return fn_803F73B0(o, r);
}

extern "C" int fn_803F8944()
{
    return lbl_806EEA88;
}

extern "C" int fn_803F894C(void* o, const char* a, const char* b)
{
    int r = 0;
    if (strncmp(b, (char*)o + 0x2c8, 4) == 0)
        r = strncmp(a, (char*)o + 0x2cc, 2) == 0;
    return r;
}

extern "C" void FindFirst(void* o, int* a, RcRecA* p)
{
    RCI(o, 4) = (int)p;
    p->m4 = 0;
    p->m8 = 0;
    p->mC = 0;
    p->m10 = 0;
    memset(p->m14, 0, 4);
    memset(p->m18, 0, 2);
    RCI(RCI(o, 4), 0) = a[0];
    FindNext(o);
}

extern "C" int FindNext(void* o)
{
    while (((RcRecB*)RCI(o, 4))->m10 <= 0x7e) {
        int r;
        do {
            RcRecB* p = (RcRecB*)RCI(o, 4);
            r = CARDGetStatus(p->m0, p->m10, &lbl_806ADDE0);
            lbl_806EEA34->v6(1);
        } while (r == -1);
        ((RcRecB*)RCI(o, 4))->m10++;
        if (fn_803F73B0(o, r) == 0) {
            ((RcRecB*)RCI(o, 4))->m4 = &lbl_806ADDE0;
            ((RcRecB*)RCI(o, 4))->m8 = lbl_806ADDE0.m20;
            ((RcRecB*)RCI(o, 4))->m14 = lbl_806ADDE0.m28;
            ((RcRecB*)RCI(o, 4))->m18 = lbl_806ADDE0.m2C;
            ((RcRecB*)RCI(o, 4))->mC = lbl_806ADDE0.m24;
            return 0;
        }
    }
    return 4;
}

extern "C" int Seek(void* o, void* b, int off, int whence)
{
    char* f = (char*)fn_803F74D8(o, b);
    if (RCI(f, 0xe0) == 0)
        return fn_803F8D8C((int)o, f, off, whence);
    if (RCI(f, 0xd4) != 0)
        FlushWriteBuffer(o, b);
    switch (whence) {
    case 0:
        off += RCI(f, 0xd0);
        break;
    case 1:
        off += RCI(f, 0xcc) - RCI(o, 0x2b4) + RCI(f, 0xc8);
        break;
    case 2:
        off = RCI(f, 0x10) + RCI(f, 0xd0) - off;
        break;
    }
    if (off < 0 || off >= RCI(f, 0x7c))
        return 0xb;
    RCI(f, 0xcc) = off / RCI(o, 0x2b4) * RCI(o, 0x2b4);
    RCI(f, 0xc8) = off % RCI(o, 0x2b4);
    RCI(f, 0xdc) = 1;
    return 0;
}

extern "C" int fn_803F8C44(void* o, int* a, void* b, int c)
{
    unsigned cur = a[0];
    int s8 = -1;
    int r = fn_803F8CB4(o, (const char*)b, &s8);
    if (r == 0) {
        int q;
        do {
            q = CARDSetAttributes(cur >> 16, s8, (unsigned char)c);
        } while (q == -1);
        r = fn_803F73B0(o, q);
    }
    return r;
}

extern "C" int fn_803F8CB4(void* o, const char* name, int* out)
{
    char buf[0x70];
    *out = -1;
    int i = 0;
    do {
        int r;
        do {
            r = CARDGetStatus(0, i, buf);
            lbl_806EEA34->v6(1);
        } while (r == -1);
        int rc = fn_803F73B0(o, r);
        switch (rc) {
        case 0:
            if (strcmp(buf, name) == 0) {
                *out = i;
                return 0;
            }
            break;
        case 4:
        case 10:
            break;
        default:
            return rc;
        }
        i++;
    } while (i <= 0x7e);
    return 4;
}

extern "C" int fn_803F8D8C(int unused, void* f, int off, int whence)
{
    switch (whence) {
    case 0:
        RCI(f, 0xcc) = off;
        break;
    case 1:
        RCI(f, 0xcc) += off;
        break;
    }
    int p = RCI(f, 0xcc);
    if (p < 0 || p >= RCI(f, 0x7c))
        return 0xb;
    return 0;
}

extern "C" int fn_803F8DDC(void* o, char* f, void* buf, int len, int* outlen)
{
    int r;
    do {
        r = CARDRead(f + 0x48, buf, len, RCI(f, 0xcc));
        lbl_806EEA34->v6(1);
    } while (r == -1);
    int rc = fn_803F73B0(o, r);
    switch (rc) {
    case 0:
        *outlen = len;
        RCI(f, 0xcc) += len;
        return rc;
    case 0xb:
        *outlen = 0;
        return 0;
    default:
        *outlen = len;
        return 0xf;
    }
}

/* near-miss: tree shape; retail roots the 0/0xa/0xb switch at 0xa (enum-typed index tried), 55/55 insns here */
extern "C" int fn_803F8EA4(void* o, char* f, void* buf, int len, int* outlen)
{
    int r;
    do {
        r = CARDWrite(f + 0x48, buf, len, RCI(f, 0xcc));
        lbl_806EEA34->v6(1);
    } while (r == -1);
    int rc = fn_803F73B0(o, r);
    switch (rc) {
    case 0:
        *outlen = len;
        RCI(f, 0xcc) += len;
        return rc;
    case 0xa:
        *outlen = -1;
        return 0;
    case 0xb:
        *outlen = 0;
        return 0;
    default:
        *outlen = 0;
        return rc;
    }
}

extern "C" int fn_803F8F80(void* o, char* p)
{
    int r;
    do {
        r = CARDClose(p + 0x48);
        lbl_806EEA34->v6(1);
    } while (r == -1);
    int rc = fn_803F73B0(o, r);
    if (rc == 0)
        RCI(p, 0x40) = rc;
    return rc;
}

extern "C" int fn_803F8FF8(void* o, char* f)
{
    int r;
    do {
        r = CARDCreate(*(unsigned short*)(f + 8), (const char*)RCI(f, 0xc), RCI(f, 0x10), f + 0x48);
        lbl_806EEA34->v6(1);
    } while (r == -1);
    switch (r) {
    case 0:
        RCI(f, 0xe0) = 0;
        RCI(f, 0xd8) = 0;
        break;
    case -128: RCI(f, 0xd8) = -1; break;
    case -3: RCI(f, 0xd8) = -2; break;
    case -1: RCI(f, 0xd8) = -3; break;
    case -7: RCI(f, 0xd8) = -4; break;
    case -8: RCI(f, 0xd8) = -5; break;
    case -9: RCI(f, 0xd8) = -6; break;
    case -12: RCI(f, 0xd8) = -7; break;
    case -5: RCI(f, 0xd8) = -8; break;
    default: RCI(f, 0xd8) = -10; break;
    }
    return r;
}

extern "C" int VerifyHeaderChecksum(void* o, void* f)
{
    int rc = 0xf;
    int blk = lbl_806EEA80;
    int sum = 0;
    int remain = RCI(f, 0xd0) - 4;
    while (remain > 0) {
        if (remain < lbl_806EEA80)
            blk = remain;
        rc = fn_803F8434(o, f, 0, blk, 0);
        if (rc != 0)
            break;
        remain -= blk;
        for (int i = 0; i < blk; i++)
            sum += ((signed char*)RCI(o, 0x2c0))[i];
    }
    if (rc == 0) {
        int stored = 0;
        rc = fn_803F8434(o, f, &stored, 4, 0);
        if (rc == 0) {
            rc = 6;
            if (stored == sum)
                rc = 0;
        }
    }
    return rc;
}

extern "C" void fn_803F91FC(void** self, int c)
{
    *self = lbl_8060E6E8;
    if (c & 1)
        fn_803F6FC4(self, 4);
}

extern "C" void fn_803F9234(char* o)
{
    memset(o + 0xc, 0, 0x30);
    RCI(o, 0x3c) = 1;
    RCI(o, 0x40) = 0;
}

extern "C" void fn_803F927C(char* o, int* a, RcB30* b, int c)
{
    RCI(o, 8) = a[0];
    *(RcB30*)(o + 0xc) = *b;
    RCI(o, 0x3c) = c;
    RCI(o, 0x40) = 0;
}

extern "C" void fn_803F92D8(char* o)
{
    memset(o + 0xc, 0, 0x30);
    RCI(o, 0x3c) = 1;
    RCI(o, 0x40) = 0;
    RCI(o, 0) = 0;
    memset(o + 0x48, 0, 0x14);
    RCI(o, 0xc8) = 0;
    RCI(o, 0xcc) = 0;
    RCI(o, 0xd4) = 0;
    RCI(o, 0xd0) = 0;
    RCI(o, 0xe0) = 0;
    RCI(o, 0xd8) = 0;
    RCI(o, 0xdc) = 1;
}
