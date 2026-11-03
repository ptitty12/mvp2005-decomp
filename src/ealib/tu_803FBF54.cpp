/*
 * EA shared library code that uses small data (cflags_ealib_sda: -O2, default -G8):
 * VP6/MAD video codecs and SPCH, or RealCore,
 * .text 0x803FBF54-0x803FEB34 (code-only unit, 53 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

extern int lbl_806EEAA0;

struct RcArgs {
    int m0, m4, m8, mC, m10, m14;
};

struct RcSlot {
    int m0;
    int m4;
    char pad[0x11c];
};

struct RcTab {
    char pad[0x20];
    RcSlot* m20;
    int m24;
};

struct RcHnd {
    int m0;
    int m4;
    int m8;
};

struct RcNd {
    int m0;
    int m4;
    RcNd* m8;
    RcNd* mC;
};

struct RcMap {
    int m0;
    int m4;
    int m8;
};

/* evidence: validatehandle (handle -> object 'MRTS'), getfreerequest/CE50/CF1C (node lists at +0x68..+0x74), STREAM_setfilter (map at +0x28/+0x2C) */
struct RcFile {
    int m0;
    char m4[0x24];
    RcMap* m28;
    int m2C;
    char* m30;
    int m34;
    int m38;
    int pad3C;
    int m40;
    int m44;
    int m48;
    int m4C;
    int m50;
    int m54;
    int m58;
    int pad5C[3];
    RcNd* m68;
    RcNd* m6C;
    RcNd* m70;
    RcNd* m74;
    char pad78[0x100];
    int m178;
    int m17C;
    int m180;
    int m184;
};

struct RcCtx {
    int m0, m4, m8;
};
struct RcBuf14 {
    int m0, m4, m8, mC, m10;
};
extern RcBuf14 lbl_806ADE8C;
extern void (*lbl_806ADE94[3])(int);
extern "C" int FILE_loadsizez(int, int*, int);
extern "C" int unpackz(void*, void*);
extern "C" void DCFlushRange(void*, int);
extern "C" void MEM_copy(void*, void*, int);
extern "C" int fn_803FBF54(int, RcCtx*);
extern "C" int FILESYS_atomic(int (*)(int, RcCtx*), int, int, RcArgs*);
extern "C" void FILE_loadatz(int, char*, int);
extern "C" void unpacksizez(char*);
extern "C" int FILE_loadpackz(int, int);
extern "C" int fn_803FC1B0(int, char*, int);
extern "C" int fn_803FC798(int, int, int, int, int, int, int);
extern "C" int fn_803FCCAC(int, int, int, int, int, int, int);
extern "C" int fn_803FC438(int, int);
extern "C" int validatehandle(RcHnd*, RcFile**, RcHnd**);

extern int lbl_806EF478;
extern "C" int fn_80400828(void*);
extern "C" int fn_8040084C(void*);
extern "C" void FILESYS_priorityop(int, int);
extern "C" char* strcpy(char*, const char*);
extern "C" int strcmp(const char*, const char*);
extern "C" int FILESYS_close(int, int, RcFile*);

extern "C" int FILESYS_completeop(int);
extern "C" int FILESYS_open(void*, int, int, void*);
extern "C" void FILESYS_callbackop(int, void (*)(int, int, RcFile*));
extern "C" void restartstream(RcFile*, int);

struct RcMapTab {
    char pad[0x28];
    RcMap* m28;
    int m2C;
};

extern "C" void STREAM_kill(int);
extern "C" int THREAD_iscurrent(int);
extern "C" int fn_80400CE4(int);
extern "C" void THREAD_yield(int);
extern "C" void fn_804007FC(void*);
extern "C" int fn_803FEC9C(int, int);
extern "C" void STREAM_setgreedystate(int, int);

extern "C" int fn_803FEBDC(int, int, int, int*);
extern "C" int fn_803FEC74(int, int, int, int, int);

/* One async transfer request (see tu_803FEB34: FsReq). Accessed through a volatile pointer in the completion callback. */
struct FsReqC {
    volatile int e;
    volatile int a;
    volatile int b;
    volatile int d;
    volatile int result;
    volatile int len;
    int c;
    int (*fn)(int, int, int, int, int, FsReqC*);
    int h;
};
extern "C" void fn_803FEA38(int h, int k, FsReqC* r);

extern "C" char* strncpy(char*, const char*, unsigned);
extern "C" void startnextrequest(RcFile*, int);
extern "C" RcNd* getfreerequest(RcFile*);
extern "C" void queuerequest(RcFile*, RcNd*);
#define RCI(p, off) (*(int*)((char*)(p) + (off)))

extern "C" int parsechunks(RcFile*);

extern "C" int fn_803FBF54(int n, RcCtx* ctx)
{
    int out;
    if (fn_803FEBDC(ctx->m0, 6, n, &out) != 0) {
        int m = n - 1;
        int v = fn_803FEC74(out, 0, ctx->m4, ctx->m8, m);
        fn_803FEC9C(out, m);
        return v == ctx->m8;
    } else
        return 0;
}

extern "C" int fn_803FBFE0(int a, int b, int c)
{
    RcArgs args;
    args.m0 = a;
    args.m4 = b;
    args.m8 = c;
    args.mC = 1;
    return FILESYS_atomic(fn_803FBF54, 0, lbl_806EEAA0, &args) == 1;
}

extern "C" void fn_803FC034(int a)
{
    char buf[0x80];
    FILE_loadatz(a, buf, 0x80);
    unpacksizez(buf);
}

/* near-miss: 2 slots; retail moves r3 (src) before r4 (len) for the DCFlushRange call; return-type toggles of the neighbouring callees did not change it */
extern "C" int FILE_loadpackz(int a, int b)
{
    int sz;
    char* src = (char*)FILE_loadsizez(a, &sz, b);
    if (src == 0)
        return 0;
    int len = ((int (*)(char*))unpacksizez)(src);
    if (sz != 0 && len != 0) {
        RcBuf14* q = &lbl_806ADE8C;
        char* tmp = (char*)((void* (*)(int, int, int))q->m4)(a, sz, b ^ 0x100);
        if (tmp == 0) {
            ((void (*)(char*))q->m8)(src);
            return 0;
        }
        MEM_copy(tmp, src, sz);
        ((void (*)(char*))q->m8)(src);
        src = (char*)((void* (*)(int, int, int))q->m4)(a, len, b);
        if (src != 0 && unpackz(tmp, src) == 0) {
            char* t = src;
            src = 0;
            ((void (*)(char*))q->m8)(t);
        }
        lbl_806ADE94[0]((int)tmp);
        DCFlushRange(src, len);
    }
    return (int)src;
}

extern "C" void fn_803FC190(int a, int b)
{
    FILE_loadpackz(a, b);
}

extern "C" void fn_803FC2E0(int a, char* b, int c)
{
    fn_803FC1B0(a, b, c);
}

extern "C" int fn_803FC300(const unsigned char* p)
{
    int r = 0;
    if (((p[0] << 24) | (p[1] << 16) | (p[2] << 8)) == 0x42494700)
        r = 3;
    else if (((p[0] << 8) | p[1]) == 0xc0fb)
        r = 1;
    return r;
}

extern "C" int fn_803FC360(const unsigned char* p)
{
    int r = 0;
    switch (fn_803FC300(p)) {
    case 3: {
        const unsigned char* q = p + 0xc;
        r = (q[0] << 24) | (q[1] << 16) | (q[2] << 8) | q[3];
        break;
    }
    case 1:
        r = ((p[2] << 8) | p[3]) + 4;
        break;
    }
    return r;
}

extern "C" int fn_803FC3E8(int a, int b, int c, int d, int e)
{
    return fn_803FC798(a, b, c, d, e, 0, 0);
}

extern "C" int fn_803FC410(int a, int b, int c, int d, int e)
{
    return fn_803FC798(a, b, c, d, e, 0, 0);
}

extern "C" int fn_803FC438(int a, int b)
{
    int r = 0;
    if (b != 0)
        fn_803FCCAC(a, b, 0, (int)&r, 0, 0, 0);
    return r;
}

extern "C" int fn_803FC480(int a, int b)
{
    int r = 0;
    int n = fn_803FC438(a, b);
    if (n != 0)
        r = a + n;
    return r;
}

extern "C" int fn_803FC4C0(const unsigned char* p)
{
    int r = 0;
    switch (fn_803FC300(p)) {
    case 1:
        r = (p[4] << 8) | p[5];
        break;
    case 3: {
        const unsigned char* q = p + 8;
        r = (q[0] << 24) | (q[1] << 16) | (q[2] << 8) | q[3];
        break;
    }
    }
    return r;
}

extern "C" char* fn_803FC548(int a, int b, char* buf)
{
    char* r;
    char* s = (char*)fn_803FCCAC(a, 0, b, 0, 0, 0, 0);
    if (s == 0) {
        *buf = 0;
        r = 0;
    } else {
        strcpy(buf, s);
        r = buf;
    }
    return r;
}

extern "C" int fn_803FC5B0(const unsigned char* p)
{
    int r = 0;
    if (((p[0] << 24) | (p[1] << 16) | (p[2] << 8)) == 0x42494700)
        r = 3;
    else if (((p[0] << 8) | p[1]) == 0xc0fb)
        r = 1;
    return r;
}

extern "C" int fn_803FC610(const unsigned char* p)
{
    int r = 0;
    switch (fn_803FC5B0(p)) {
    case 3: {
        const unsigned char* q = p + 0xc;
        r = (q[0] << 24) | (q[1] << 16) | (q[2] << 8) | q[3];
        break;
    }
    case 1:
        r = ((p[2] << 8) | p[3]) + 4;
        break;
    }
    return r;
}

extern "C" int BIG_debuginfo(const unsigned char* base, int* pa, int* pb)
{
    int r = 0;
    const unsigned char* e = base + fn_803FC610(base);
    const unsigned char* s = e - 8;
    unsigned c0 = s[0];
    if (c0 - 0x41 <= 0x19 || c0 - 0x61 <= 0x19) {
        unsigned c1 = s[1];
        if (c1 - 0x30 <= 9) {
            unsigned c2 = s[2];
            if (c2 - 0x30 <= 9) {
                unsigned c3 = s[3];
                if (c3 - 0x30 <= 9) {
                    r = 8;
                    if (pa != 0)
                        *pa = ((char)c1 - 0x30) * 100 + ((char)c2 - 0x30) * 10 + ((char)c3 - 0x30);
                    if (pb != 0) {
                        const unsigned char* t = e - 4;
                        *pb = (e[-4] << 24) | (t[1] << 16) | (t[2] << 8) | t[3];
                    }
                }
            }
        }
    }
    return r;
}

extern "C" int fn_803FCCAC(int a, int b, int c, int d, int e, int f, int g)
{
    return fn_803FC798(a, b, c, d, e, f, g);
}

extern "C" int validatehandle(RcHnd* h, RcFile** o1, RcHnd** o2)
{
    if (h == 0)
        return 1;
    int* q = (int*)h->m0;
    if (q[0] != 0x4d525453)
        return 1;
    *o2 = h;
    *o1 = (RcFile*)q;
    return 0;
}

extern "C" int inbetween(unsigned x, unsigned a, unsigned b)
{
    if (x <= a) {
        return b >= x && b < a;
    }
    return b >= x || b < a;
}

extern "C" void decbufferusage(RcFile* o, int n)
{
    fn_80400828(o->m4);
    int old = o->m58;
    int nw = old - n;
    o->m58 = nw;
    fn_8040084C(o->m4);
    if (old >= o->m50 && nw < o->m50) {
        o->m54 = 1;
        if (o->m44 == 1)
            FILESYS_priorityop(o->m180, o->m4C);
    }
}

extern "C" RcNd* getfreerequest(RcFile* o)
{
    RcNd* r = 0;
    fn_80400828(o->m4);
    RcNd* n = o->m74;
    if (n != 0) {
        r = n;
        o->m74 = n->mC;
        lbl_806EF478 += 0x100;
        if (lbl_806EF478 == 0)
            lbl_806EF478 = 0x100;
        n->m0 = ((unsigned char*)n)[3] | lbl_806EF478;
    }
    fn_8040084C(o->m4);
    return r;
}

extern "C" void queuerequest(RcFile* o, RcNd* n)
{
    n->m4 = 1;
    n->mC = 0;
    fn_80400828(o->m4);
    RcNd* t = o->m70;
    if (t == 0) {
        n->m8 = t;
    o->m68 = n;
    o->m6C = n;
    o->m70 = n;
    } else {
        n->m8 = t;
        o->m70->mC = n;
        o->m70 = n;
    }
    fn_8040084C(o->m4);
}

extern "C" RcSlot* locaterequest__FP15STREAMHEADERtagi(RcTab* t, int k)
{
    int idx = k & 0xff;
    if (idx >= t->m24)
        return 0;
    RcSlot* e = (RcSlot*)((char*)t->m20 + idx * 0x124);
    if (k == e->m0) {
        RcSlot* r = 0;
        if (e->m4 != 0)
            r = e;
        return r;
    }
    return 0;
}

extern "C" void freerequest(RcFile* o, RcNd* n)
{
    if (n == o->m68)
        o->m68 = n->mC;
    else
        n->m8->mC = n->mC;
    if (n == o->m70)
        o->m70 = n->m8;
    else
        n->mC->m8 = n->m8;
    if (n == o->m6C) {
        RcNd* t = n->mC;
        if (t == 0)
            t = n->m8;
        o->m6C = t;
    }
    n->m4 = 0;
    n->mC = o->m74;
    o->m74 = n;
}

/* near-miss: 1 slot; retail loads t->m28 after the count test and copies the bound (mr r11,r8) */
extern "C" int filterchunk(RcMapTab* t, const unsigned char* p)
{
    int k = (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
    int i;
    RcMap* e = t->m28;
    for (i = 0; i < t->m2C; i++) {
        if ((k & e->m0) == e->m4)
            return e->m8;
        e++;
    }
    return -2;
}

extern "C" void opencallback(int a, int b, RcFile* o)
{
    int t = FILESYS_completeop(*(int*)((char*)o + 0x180));
    *(int*)((char*)o + 0x178) = t;
    if (t != 0)
        restartstream(o, o->m4C);
}

extern "C" void closecallback(int a, int b, RcFile* o)
{
    FILESYS_completeop(o->m180);
    int h = FILESYS_open((char*)o + 0x78, 1, o->m4C, o);
    o->m180 = h;
    if (h != 0)
        FILESYS_callbackop(h, opencallback);
}

extern "C" void readcallback(int, int, RcFile* f)
{
    RcNd* p = f->m6C;
    int amt;
    int flag;
    int r;
    if (RCI(p, 0x10) == 1) {
        amt = f->m184;
        flag = f->m17C + amt >= RCI(p, 0x118);
    } else {
        amt = FILESYS_completeop(f->m180);
        flag = amt < f->m184;
    }
    f->m17C += amt;
    RCI(f, 0x64) += amt;
    r = parsechunks(f);
    if (p->m4 == 4) {
        startnextrequest(f, f->m4C);
    } else if (flag != 0 || r != 0) {
        fn_80400828(f->m4);
        if (p->m4 != 4)
            p->m4 = 3;
        fn_8040084C(f->m4);
        startnextrequest(f, f->m4C);
    } else {
        restartstream(f, f->m4C - 1);
    }
}

/* near-miss: 96.5%; callee-saved registers rotated (retail: a=r27 job=r28 cr=r29 empty=r30; ours a=r28 job=r29 cr=r30 empty=r27) */
extern "C" void startnextrequest(RcFile* o, int a)
{
    char* job = 0;
    fn_80400828(o->m4);
    int empty;
    RcNd* h = o->m6C;
    if (h == 0)
        empty = 1;
    else if (h->m4 != 1) {
        RcNd* nx = h->mC;
        if (nx == 0)
            empty = 1;
        else {
            o->m6C = nx;
            empty = 0;
        }
    } else
        empty = 0;
    if (empty != 0) {
        o->m44 = 0;
    } else {
        job = (char*)o->m6C;
        int cur = RCI(o, 0x60);
        RCI(job, 4) = 2;
        RCI(job, 0x120) = cur;
    }
    fn_8040084C(o->m4);
    if (empty == 0) {
        RCI(o, 0x64) = RCI(o, 0x60);
        if (RCI(job, 0x10) == 1) {
            o->m17C = 0;
        } else {
            o->m17C = RCI(job, 0x118);
            if (strcmp(job + 0x14, (char*)o + 0x78) != 0) {
                strcpy((char*)o + 0x78, job + 0x14);
                int k = o->m178;
                if (k == 0) {
                    int hd = FILESYS_open((char*)o + 0x78, 1, a, o);
                    o->m180 = hd;
                    if (hd != 0)
                        FILESYS_callbackop(hd, opencallback);
                } else {
                    int hd = FILESYS_close(k, a, o);
                    o->m180 = hd;
                    if (hd != 0)
                        FILESYS_callbackop(hd, closecallback);
                }
                return;
            }
        }
        restartstream(o, a);
    }
}

extern "C" int fn_803FD944(int a, int b, int c)
{
    int t1 = b * 0xc;
    int t2 = c << 4;
    int t3 = a * 0x124;
    t1 += 0x18c;
    return t3 + t1 + t2 + 0x20;
}

extern "C" void STREAM_setfilter(int h, int a, int b, int c, int d)
{
    RcFile* o;
    RcHnd* p;
    if (validatehandle((RcHnd*)h, &o, &p) == 0) {
        if (a > 0) {
            RcFile* q = o;
            if (a <= o->m2C) {
                if (a == o->m2C && (b | c) != 0)
                    return;
                if (d > 0 || d == -1 || d == -2) {
                    if (d <= o->m34) {
                        if (o->m44 == 0) {
                            char* tab = (char*)o->m28;
                            int off = a * 0xc - 0xc;
                            *(int*)(tab + off) = b;
                            tab += off;
                            ((RcMap*)tab)->m4 = c;
                            ((RcMap*)tab)->m8 = d;
                        }
                    }
                }
            }
        }
    }
}

extern "C" void STREAM_destroy(int h)
{
    RcFile* o;
    RcHnd* p;
    if (validatehandle((RcHnd*)h, &o, &p) == 0) {
        STREAM_kill(h);
        while (o->m44 == 1) {
            if (THREAD_iscurrent(0) != 0)
                fn_80400CE4(0);
            THREAD_yield(0);
        }
        o->m0 = 0;
        fn_804007FC(o->m4);
        if (o->m178 != 0)
            fn_803FEC9C(o->m178, 0x64);
    }
}

extern "C" void STREAM_setpriority(RcHnd* h, int a, int b)
{
    RcFile* o;
    RcHnd* p;
    if (validatehandle(h, &o, &p) == 0) {
        o->m48 = a;
        o->m4C = b;
    }
}

extern "C" void STREAM_setgreedylevel(int h, int v)
{
    RcFile* o;
    RcHnd* p;
    if (validatehandle((RcHnd*)h, &o, &p) == 0) {
        int old = o->m50;
        o->m50 = v;
        int a = o->m58 < v;
        int b = o->m58 < old;
        if (b != a)
            STREAM_setgreedystate(h, a);
    }
}

extern "C" void STREAM_setgreedystate(int h, int flag)
{
    RcFile* o;
    RcHnd* p;
    if (validatehandle((RcHnd*)h, &o, &p) == 0) {
        o->m54 = flag;
        if (flag != 0 && o->m44 == 1)
            FILESYS_priorityop(o->m180, o->m4C);
    }
}

extern "C" char* STREAM_taphandle(int h, int n)
{
    RcFile* o;
    RcHnd* p;
    if (validatehandle((RcHnd*)h, &o, &p) != 0 || n <= 0 || n > o->m34)
        return 0;
    return o->m30 + (n * 16 - 16);
}

extern "C" int STREAM_queuefile(RcHnd* h, const char* name, int a, int b)
{
    RcFile* o;
    RcHnd* p;
    int err = validatehandle(h, &o, &p);
    if (err != 0)
        return 0;
    RcNd* n = getfreerequest(o);
    if (n == 0)
        return 0;
    RCI(n, 0x10) = err;
    strncpy((char*)n + 0x14, name, 0xfe);
    RCI(n, 0x118) = a;
    RCI(n, 0x11c) = b;
    queuerequest(o, n);
    fn_80400828(o->m4);
    int was = o->m44;
    if (was == 0)
        o->m44 = 1;
    fn_8040084C(o->m4);
    if (was == 0) {
        if (o->m54 != 0)
            startnextrequest(o, o->m4C);
        else
            startnextrequest(o, o->m48);
    }
    return n->m0;
}

struct RcHndV {
    int m0, m4, m8;
    unsigned char* mC;
};
/* near-miss: 2 slots; the loop-back compare value is r0 in retail (or r0,r9,r10; cmpw r0,r28), r9 here */
extern "C" int STREAM_queuemem(RcHnd* h, unsigned char* data, int len, int tag)
{
    RcFile* o;
    RcHnd* p;
    if (validatehandle(h, &o, &p) != 0)
        return 0;
    RcNd* n = getfreerequest(o);
    if (n == 0)
        return 0;
    if (len == 0) {
        unsigned char* q = data;
        if (((q[3] << 24) | (q[2] << 16) | (q[1] << 8) | q[0]) != tag) {
            do {
                const unsigned char* t = q + 4;
                int sz = (t[3] << 24) | (t[2] << 16) | (t[1] << 8) | q[4];
                q += sz;
                len += sz;
            } while (((q[3] << 24) | (q[2] << 16) | (q[1] << 8) | q[0]) != tag);
        }
        const unsigned char* t = q + 4;
        len += (t[3] << 24) | (t[2] << 16) | (t[1] << 8) | q[4];
    }
    RCI(n, 0x114) = (int)data;
    RCI(n, 0x118) = len;
    RCI(n, 0x11c) = tag;
    RCI(n, 0x10) = 1;
    queuerequest(o, n);
    fn_80400828(o->m4);
    int was = o->m44;
    if (was == 0)
        o->m44 = 1;
    fn_8040084C(o->m4);
    if (was == 0)
        startnextrequest(o, 0);
    return n->m0;
}

extern "C" void freerequest(RcFile*, RcNd*);
struct RcFileV {
    char pad0[0x30];
    char* m30;
    int m34;
    char* m38;
    char* m3C;
    char pad40[4];
    int m44;
    char pad48[0x10];
    int m58;
    char* m5C;
    char* m60;
    char pad64[4];
    RcNd* m68;
    RcNd* m6C;
    RcNd* m70;
};
extern "C" void STREAM_cancelrequest(int, int);
struct RcSlotV {
    int m0, m4, m8;
    RcSlotV* mC;
    char pad10[0x110];
    char* m120;
};
struct RcMapE {
    int m0, m4, m8, mC;
};
extern "C" unsigned char* STREAM_get(RcHnd*);
extern "C" void STREAM_release(RcHnd*, char*);
/* near-miss: 99.3%; retail keeps lo/cur in r22/r26 (ours swapped) and holds the STREAM_get result in r31 across the handle reload */
extern "C" void STREAM_cancelrequest(int h, int key)
{
    RcFileV* o;
    RcMapE* e;
    char* lo = 0;
    char* cur = 0;
    char* hi = 0;
    int skip;
    if (validatehandle((RcHnd*)h, (RcFile**)&o, (RcHnd**)&e) == 0) {
        fn_80400828(o->pad0 + 4);
        RcSlotV* nd = (RcSlotV*)locaterequest__FP15STREAMHEADERtagi((RcTab*)o, key);
        if (nd == 0 || nd->m4 == 4)
            skip = 1;
        else if (nd->m4 == 1) {
            skip = 1;
            freerequest((RcFile*)o, (RcNd*)nd);
        } else {
            nd->m4 = 4;
            lo = (char*)((int*)o)[0x5c / 4];
            cur = lo;
            if ((RcNd*)nd != o->m68)
                cur = nd->m120;
            RcSlotV* nx = nd->mC;
            if (nx == 0 || nx->m4 == 1)
                hi = o->m60;
            else
                hi = nx->m120;
            skip = 0;
        }
        fn_8040084C(o->pad0 + 4);
        if (skip == 0) {
            int i;
            for (i = 0; i < o->m34; i++) {
                e = (RcMapE*)(o->m30 + i * 0x10);
                if (e->m8 > 0) {
                    if (inbetween((unsigned)lo, (unsigned)cur, e->mC) != 0) {
                        char* q = cur;
                        int tag = e->m4 << 24;
                        if (cur != hi) {
                            do {
                                if ((((unsigned char)q[3] << 24) | ((unsigned char)q[2] << 16) | ((unsigned char)q[1] << 8) | (unsigned char)q[0]) == -1)
                                    q = o->m3C;
                                else {
                                    char* t = q + 4;
                                    unsigned v = ((unsigned char)t[3] << 24) | ((unsigned char)t[2] << 16) | ((unsigned char)t[1] << 8) | (unsigned char)q[4];
                                    unsigned sz = v & 0xffffff;
                                    if ((int)(v & 0xff000000) == tag) {
                                        fn_80400828(o->pad0 + 4);
                                        e->m8 = e->m8 - sz;
                                        fn_8040084C(o->pad0 + 4);
                                        decbufferusage((RcFile*)o, sz);
                                        q[0] = -2;
                                        q[1] = -1;
                                        q[2] = -1;
                                        q[3] = -1;
                                        q[4] = sz;
                                        t[1] = sz >> 8;
                                        t[2] = sz >> 16;
                                        t[3] = sz >> 24;
                                    }
                                    q += sz;
                                }
                            } while (q != hi);
                        }
                    } else {
                        if (inbetween((unsigned)cur, (unsigned)hi, e->mC) != 0) {
                            do {
                                char* hh = (char*)STREAM_get((RcHnd*)e);
                                STREAM_release((RcHnd*)e, hh);
                                if (e->m8 <= 0)
                                    break;
                            } while (inbetween((unsigned)cur, (unsigned)hi, e->mC) != 0);
                        }
                    }
                }
            }
        }
    }
}

/* near-miss: 96.0%; retail reuses the reloaded handle (r3 / r8) across the loop bodies, ours reloads it from the stack in each body */
extern "C" void STREAM_kill(int h)
{
    RcFileV* o;
    RcHnd* p;
    unsigned sz = 0;
    if (validatehandle((RcHnd*)h, (RcFile**)&o, &p) == 0) {
        RcNd* nd = o->m70;
        if (nd != 0) {
            while ((unsigned)(nd->m4 - 1) <= 1) {
                STREAM_cancelrequest(h, nd->m0);
                nd = o->m70;
            }
            while (o->m68 != o->m6C)
                freerequest((RcFile*)o, o->m68);
            o->m6C->m4 = 4;
            int i;
            for (i = 0; i < o->m34; i++)
                RCI(o->m30, i * 0x10 + 8) = 0;
            decbufferusage((RcFile*)o, o->m58);
            char* q = o->m5C;
            if (q != o->m60) {
                do {
                    if ((((unsigned char)q[3] << 24) | ((unsigned char)q[2] << 16) | ((unsigned char)q[1] << 8) | (unsigned char)q[0]) == -1)
                        q = o->m3C;
                    else {
                        char* t = q + 4;
                        sz = (((unsigned char)t[3] << 24) | ((unsigned char)t[2] << 16) | ((unsigned char)t[1] << 8) | (unsigned char)q[4]) & 0xffffff;
                        q[0] = -2;
                        q[1] = -1;
                        q[2] = -1;
                        q[3] = -1;
                        q[4] = sz;
                        t[3] = sz >> 24;
                        q += sz;
                        t[1] = sz >> 8;
                        t[2] = sz >> 16;
                    }
                } while (q != o->m60);
            }
            if (o->m44 == 2) {
                if (q == o->m3C) {
                    o->m60 = o->m38;
                    o->m3C = o->m60;
                } else {
                    int pad = 0x20 - ((int)o->m60 & 0x1f);
                    if (pad == 0x20)
                        pad = 0;
                    char* r = o->m60 - sz;
                    sz += pad;
                    char* t2 = r + 4;
                    r[4] = sz;
                    t2[3] = sz >> 24;
                    t2[1] = sz >> 8;
                    t2[2] = sz >> 16;
                    o->m60 = o->m60 + pad;
                }
                o->m44 = 0;
            }
        }
    }
}

/* near-miss: 2 slots; retail schedules slwi before the mr r4,r9 copy of the handle in the first loop test */
extern "C" unsigned char* STREAM_get(RcHnd* hnd)
{
    RcFile* o;
    RcHndV* h;
    if (validatehandle(hnd, &o, (RcHnd**)&h) != 0)
        return 0;
    {
        if (h->m8 == 0)
            return 0;
        unsigned char* p = h->mC;
        const unsigned char* t = p + 4;
        unsigned sz = ((t[3] << 24) | (t[2] << 16) | (t[1] << 8) | p[4]) & 0xffffff;
        p[4] = sz;
        ((unsigned char*)t)[1] = sz >> 8;
        ((unsigned char*)t)[2] = sz >> 16;
        ((unsigned char*)t)[3] = sz >> 24;
        fn_80400828(o->m4);
        int rem = h->m8 - sz;
        h->m8 = rem;
        fn_8040084C(o->m4);
        if (rem > 0) {
            unsigned char* q = p + sz;
            int key = h->m4 << 24;
            if ((q[7] << 24) != key) {
                do {
                    if (((q[3] << 24) | (q[2] << 16) | (q[1] << 8) | q[0]) == -1)
                        q = (unsigned char*)o->pad3C;
                    else {
                        const unsigned char* t2 = q + 4;
                        sz = ((t2[3] << 24) | (t2[2] << 16) | (t2[1] << 8) | q[4]) & 0xffffff;
                        q += sz;
                    }
                } while ((q[7] << 24) != key);
            }
            h->mC = q;
        }
        return p;
    }
}

extern "C" void STREAM_release(RcHnd* h, char* p)
{
    RcFile* o;
    RcHnd* hh;
    if (validatehandle(h, &o, &hh) == 0 && (unsigned)p >= (unsigned)o->pad3C && (unsigned)p <= (unsigned)(o->m40 - 8)
        && (((unsigned char)p[3] << 24) | ((unsigned char)p[2] << 16) | ((unsigned char)p[1] << 8) | (unsigned char)p[0]) != -2) {
        p[0] = -2;
        const unsigned char* t = (unsigned char*)p + 4;
        p[1] = -1;
        p[2] = -1;
        p[3] = -1;
        decbufferusage(o, (t[3] << 24) | (t[2] << 16) | (t[1] << 8) | (unsigned char)p[4]);
        fn_80400828(o->m4);
        int v = o->m44;
        if (v == 2)
            o->m44 = 1;
        fn_8040084C(o->m4);
        if (v == 2) {
            if (o->m54 != 0)
                restartstream(o, o->m4C);
            else
                restartstream(o, o->m48);
        }
    }
}

extern "C" int fn_803FE914(RcHnd* h)
{
    RcFile* o;
    RcHnd* p;
    int r;
    if (validatehandle(h, &o, &p) == 0)
        r = p->m8;
    else
        r = 0;
    return r;
}

extern "C" int fn_803FE954(RcHnd* h)
{
    RcFile* o;
    RcHnd* p;
    int r;
    if (validatehandle(h, &o, &p) == 0)
        r = o->m44;
    else
        r = 0;
    return r;
}

extern "C" int STREAM_isendofstream(RcHnd* h)
{
    RcFile* o;
    RcHnd* p;
    if (validatehandle(h, &o, &p) != 0)
        return 0;
    int r = 0;
    if (o->m44 == 0)
        r = p->m8 == 0;
    return r;
}

extern "C" int STREAM_buffersize(RcHnd* h)
{
    RcFile* o;
    RcHnd* p;
    if (validatehandle(h, &o, &p) != 0)
        return 0;
    return o->m40 - o->m38;
}

extern "C" void fn_803FEA38(int h, int k, FsReqC* r)
{
    int n = FILESYS_completeop(h);
    r->h = 0;
    if (k == 1) {
        r->b += n;
        r->c += n;
        r->result += n;
        if (n < r->len)
            r->d = 0;
        else
            r->d -= n;
        if (r->d > 0) {
            if (r->d > 0x100000)
                r->len = 0x100000;
            else
                r->len = r->d;
            r->h = r->fn(r->a, r->b, r->c, r->len, r->e, (FsReqC*)r);
            if (r->h != 0)
                FILESYS_callbackop(r->h, (void (*)(int, int, RcFile*))fn_803FEA38);
            else
                r->d = r->h;
        }
    } else
        r->d = 0;
}
