/*
 * EA shared library code that uses small data (cflags_ealib_sda: -O2, default -G8):
 * VP6/MAD video codecs and SPCH, or RealCore,
 * .text 0x803F9350-0x803FAE94 (code-only unit, 37 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */


union RcH {
    int i;
    struct {
        unsigned short g;
        unsigned char x;
        unsigned char idx;
    } s;
};

struct RcJob {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    int m18;
    int m1C;
    int m20;
    int m24;
    int m28;
    int m2C;
};

struct RcSysJ {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    RcJob* m18;
    char* m1C;
    int m20;
    int m24;
};

struct RcBuf14 {
    int m0, m4, m8, mC, m10;
};

extern RcJob* lbl_806ADE64[3];
extern RcJob* lbl_806ADE6C[3];
extern RcJob* lbl_806ADE70[3];
extern RcSysJ lbl_806ADE4C;
extern char lbl_806ADE74[];
extern RcBuf14 lbl_806ADE8C;
extern int lbl_806EEAA8;
extern int lbl_806EEAAC;
extern unsigned short lbl_806EEA90;
extern char* lbl_806ADE68[3];
extern char lbl_8060E700[];
extern char lbl_8060E748[];

extern "C" int REAL_abortmessage(const char*, ...);
extern "C" void OSLockMutex(void*);
extern "C" void OSUnlockMutex(void*);
extern "C" void MEM_fill(void*, int, int);
extern "C" int FILESYS_completeop(int);
extern "C" void iFILE_ExecCommand(int);
extern "C" int THREAD_iscurrent(int);
extern "C" int fn_80400CE4(int);
extern "C" void THREAD_yield(int);
extern "C" void fn_8040074C(void*, int);
extern "C" void MEM_copy(void*, void*, int);
extern "C" int OPVALID(int);
extern "C" RcJob* fn_803FA94C();
extern "C" int reservehandle(RcJob*);
extern "C" void fn_803FA948(RcJob*);
extern "C" char* strncpy(char*, const char*, unsigned);

extern "C" void* memset(void*, int, unsigned, ...);
extern "C" void iFILE_addbigreadcallback(int, int, RcJob*);
struct RcB30x {
    int w[12];
};
/* Object whose vptr sits at +0x44 (data-only base, then the virtuals). */
struct RcPad44 {
    char pad[0x44];
};
struct RcObj44 : RcPad44 {
    virtual void v0();
};

extern "C" void fn_803FADD0(int, int);

extern RcJob* lbl_806ADE60[3];
extern char lbl_8060E790[];
extern "C" int FILESYS_initadr(int, int, int, void*);
extern "C" int fn_803F9434();
extern "C" int fn_803F9464();

extern "C" int fn_803F963C(int a, int b, int c);

extern "C" void OSInitMutex(void*);
extern "C" void initfiledev(void);

extern "C" int strncmp(const char*, const char*, unsigned);
extern char lbl_8060E7AC[];
typedef void* (*RcAllocFn)(const char*, int, int);

extern "C" void fn_803F9350(RcObj44* o, int* a, RcB30x* b, int c)
{
    o->v0();
    *(int*)((char*)o + 8) = a[0];
    *(RcB30x*)((char*)o + 0xc) = *b;
    *(int*)((char*)o + 0x3c) = c;
    *(int*)((char*)o + 0x40) = 0;
}

extern "C" int OPVALID(int h)
{
    int r = 0;
    if (h != 0)
        r = *(unsigned short*)&h == *(unsigned short*)((char*)lbl_806ADE64[0] + ((unsigned char*)&h)[3] * 0x30);
    return r;
}

extern "C" int fn_803F9434()
{
    REAL_abortmessage(lbl_8060E700);
    return 0;
}

extern "C" int fn_803F9464()
{
    REAL_abortmessage(lbl_8060E748);
    return 0;
}

extern "C" int FILESYS_init(int a, int b, int c)
{
    RcBuf14* q = &lbl_806ADE8C;
    if (q->m4 == 0) {
        q->m4 = (int)fn_803F9434;
        q->m8 = (int)fn_803F9464;
    }
    if (lbl_806ADE4C.m0 == 0) {
        void* mem = ((void* (*)(const char*, int, int))q->m4)(lbl_8060E790, fn_803F963C(a, b, c), 0);
        return FILESYS_initadr(a, b, c, mem);
    }
    return 0;
}

extern "C" int FILESYS_initadr(int a, int b, int c, void* mem)
{
    if (a == 0)
        a = 0x18;
    if (b == 0)
        b = 0x800;
    if (c == 0)
        c = 0xa;
    RcBuf14* q = &lbl_806ADE8C;
    if (q->m4 == 0) {
        q->m4 = (int)fn_803F9434;
        q->m8 = (int)fn_803F9464;
    }
    if (lbl_806ADE4C.m0 == 0) {
        lbl_806ADE4C.m18 = (RcJob*)mem;
        lbl_806ADE4C.m8 = 0xff;
        lbl_806ADE4C.m0 = c;
        lbl_806ADE4C.m4 = a;
        OSInitMutex(lbl_806ADE74);
        MEM_fill(lbl_806ADE4C.m18, 0, fn_803F963C(a, b, c));
        lbl_806ADE4C.m1C = (char*)lbl_806ADE4C.m18 + lbl_806ADE4C.m0 * 0x30;
        initfiledev();
        return 1;
    }
    return 0;
}

extern "C" int fn_803F963C(int a, int b, int c)
{
    if (a == 0)
        a = 0x18;
    if (c == 0)
        c = 0xa;
    return c * 0x30 + a * 0x144;
}

extern "C" int FILESYS_opstatus(int h)
{
    int r;
    if (OPVALID(h) != 0)
        r = (lbl_806ADE64[0] + ((unsigned char*)&h)[3])->m8;
    else
        r = -3;
    return r;
}

extern "C" void FILESYS_callbackop(int h, void (*cb)(int, int, int))
{
    RcJob* j = lbl_806ADE4C.m18 + ((unsigned char*)&h)[3];
    int m8 = j->m8;
    j->m28 = (int)cb;
    if (m8 != 0) {
        lbl_806ADE4C.m10++;
        cb(h, *(volatile int*)&j->m8, j->m14);
        lbl_806ADE4C.m10--;
    }
}

extern "C" void FILESYS_priorityop(int h, int prio)
{
    RcJob* j = lbl_806ADE4C.m18 + ((unsigned char*)&h)[3];
    if (OPVALID(h) != 0) {
        OSLockMutex(lbl_806ADE74);
        int old = j->m10;
        j->m10 = prio;
        if (lbl_806ADE4C.mC > 1 && (int)j != lbl_806ADE4C.m14 && j->m8 == 0 && old != prio) {
            RcJob* p = (RcJob*)lbl_806ADE4C.m20;
            RcJob* t = 0;
            while (p != 0 && p != j) {
                t = p;
                p = (RcJob*)p->m2C;
            }
            if (p == 0) {
                OSUnlockMutex(lbl_806ADE74);
                return;
            }
            if (t != 0)
                t->m2C = j->m2C;
            else
                lbl_806ADE6C[0] = (RcJob*)j->m2C;
            t = 0;
            RcJob* c = lbl_806ADE6C[0];
            j->m2C = (int)c;
            if (c != 0) {
                int pr = j->m10;
                if (c->m10 <= pr) {
                    do {
                        t = (RcJob*)j->m2C;
                        c = (RcJob*)t->m2C;
                        j->m2C = (int)c;
                    } while (c != 0 && c->m10 <= pr);
                }
            }
            if (t != 0)
                t->m2C = (int)j;
            else
                lbl_806ADE6C[0] = j;
        }
        OSUnlockMutex(lbl_806ADE74);
    }
}

extern "C" void FILESYS_cancelop(int h)
{
    int state = 0;
    OSLockMutex(lbl_806ADE74);
    if (OPVALID(h) != 0 && ((unsigned char*)&h)[2] != 3 && ((unsigned char*)&h)[2] != 10) {
        RcJob* j = lbl_806ADE4C.m18 + ((unsigned char*)&h)[3];
        if (lbl_806ADE4C.m14 == (int)j) {
            j->m4 = 1;
            state = 1;
        } else {
            if (j->m8 != 1) {
                RcJob* p = (RcJob*)lbl_806ADE4C.m20;
                RcJob* t = 0;
                while (p != 0 && p != j) {
                    t = p;
                    p = (RcJob*)p->m2C;
                }
                if (p == 0) {
                    OSUnlockMutex(lbl_806ADE74);
                    return;
                }
                if (t != 0)
                    t->m2C = j->m2C;
                else
                    lbl_806ADE6C[0] = (RcJob*)j->m2C;
                state = 2;
                lbl_806ADE4C.mC = lbl_806ADE4C.mC - 1;
            }
            j->m8 = -1;
        }
        if ((state != 1 || *((unsigned char*)j + 2) != 4) && state == 2) {
            if (j->m28 != 0)
                ((void (*)(int, int, int))j->m28)(j->m0, -1, j->m14);
        }
    }
    OSUnlockMutex(lbl_806ADE74);
}

extern "C" int FILESYS_waitop(int h)
{
    RcJob* j = lbl_806ADE64[0] + ((unsigned char*)&h)[3];
    if (OPVALID(h) == 0)
        return -3;
    for (;;) {
        if (THREAD_iscurrent(0) != 0)
            fn_80400CE4(0);
        else
            THREAD_yield(10);
        if (OPVALID(h) == 0)
            return -3;
        if (((volatile RcJob*)j)->m8 != 0)
            return ((volatile RcJob*)j)->m8;
    }
}

extern "C" void fn_803FAA38(void*);
extern "C" void fn_803FAB1C(void*);
struct RcM104 {
    char pad[0x104];
    int m104;
};
extern "C" int FILESYS_completeop(int h)
{
    int ret = 0;
    RcJob* j = lbl_806ADE64[0] + ((unsigned char*)&h)[3];
    switch (j->m8) {
    case 0:
        break;
    case 1:
        switch (*((unsigned char*)j + 2)) {
        case 2:
        case 9:
            ret = j->m24;
            break;
        case 3:
        case 7:
        case 10:
            ret = j->m8;
            break;
        case 6:
        case 8:
            ret = j->m18;
            break;
        case 4:
            ret = j->m1C;
            break;
        case 5: {
            RcM104* o = (RcM104*)j->m24;
            ret = j->m1C;
            int t = ret + j->m18;
            if (t > o->m104)
                o->m104 = t;
            break;
        }
        case 11:
            ret = 1;
            break;
        }
        break;
    case -1:
    case -2: {
        unsigned char t = *((unsigned char*)j + 2);
        if (t == 2 || t == 9)
            fn_803FAB1C((void*)j->m24);
        break;
    }
    }
    fn_803FAA38(j);
    return ret;
}

extern "C" int FILESYS_exists(const char* name, int a, int b)
{
    RcJob* j = fn_803FA94C();
    j->m14 = b;
    j->m10 = a;
    *((unsigned char*)j + 2) = 8;
    j->m18 = 1;
    j->m24 = reservehandle(j);
    if (j->m24 == 0) {
        j->mC = 2;
        fn_803FA948(j);
    }
    strncpy((char*)j->m24 + 5, name, 0xff);
    iFILE_ExecCommand((int)j);
    return j->m0;
}

extern "C" int FILESYS_open(const char* name, int a, int b, int c)
{
    RcJob* j = fn_803FA94C();
    j->m14 = c;
    j->m18 = a;
    j->m10 = b;
    *((unsigned char*)j + 2) = 2;
    j->m24 = reservehandle(j);
    if (j->m24 == 0) {
        j->mC = 2;
        fn_803FA948(j);
    }
    strncpy((char*)j->m24 + 5, name, 0xff);
    iFILE_ExecCommand((int)j);
    return j->m0;
}

extern "C" int FILESYS_close(int x, int a, int b)
{
    RcJob* j = fn_803FA94C();
    RcJob* p = lbl_806ADE70[0];
    *((unsigned char*)j + 2) = 3;
    j->m10 = a;
    j->m14 = b;
    j->m24 = x;
    while (p != 0) {
        if (p->mC == x) {
            j->mC = 3;
            fn_803FA948(j);
            break;
        }
        p = (RcJob*)p->m14;
    }
    iFILE_ExecCommand((int)j);
    return j->m0;
}

extern "C" int FILESYS_read(RcJob* x, int a, int b, int c, int d, int e)
{
    RcJob* j = fn_803FA94C();
    *((unsigned char*)j + 2) = 4;
    j->m14 = e;
    j->m10 = d;
    if (x == 0) {
        j->mC = 6;
        fn_803FA948(j);
    }
    j->m24 = (int)x;
    int lim = ((int*)x)[0x104 / 4];
    if (a + c > lim)
        c = lim - a;
    j->m1C = c;
    j->m20 = b;
    j->m18 = a;
    iFILE_ExecCommand((int)j);
    return j->m0;
}

extern "C" int FILESYS_write(RcJob* x, int a, int b, int c, int d, int e)
{
    RcJob* j = fn_803FA94C();
    *((unsigned char*)j + 2) = 5;
    j->m14 = e;
    if (x == 0) {
        j->mC = 6;
        fn_803FA948(j);
    }
    j->m24 = (int)x;
    j->m10 = d;
    j->m1C = c;
    j->m18 = a;
    j->m20 = b;
    iFILE_ExecCommand((int)j);
    return j->m0;
}

extern "C" int FILESYS_size(RcJob* x, int a, int b)
{
    RcJob* j = fn_803FA94C();
    *((unsigned char*)j + 2) = 6;
    j->m14 = b;
    j->m10 = a;
    if (x == 0) {
        j->mC = 6;
        fn_803FA948(j);
    }
    j->m24 = (int)x;
    iFILE_ExecCommand((int)j);
    return j->m0;
}

extern "C" int fn_803FC300(int);
extern "C" int fn_803FC360(int);
extern void (*lbl_806ADE94[3])(int);
extern char lbl_8060E79C[];
extern "C" void iFILE_addbigreadcallback(int h, int unused, RcJob* req)
{
    RcJob* j = lbl_806ADE4C.m18 + ((unsigned char*)&h)[3];
    int prio = j->m10;
    int o = j->m24;
    req->mC = o;
    ((RcJob*)req->m10)->m24 = o;
    FILESYS_completeop(h);
    if (fn_803FC300(req->m0) == 0)
        lbl_806ADE94[0](req->m0);
    int len = fn_803FC360(req->m0);
    if (len > req->m4) {
        RcBuf14* q = &lbl_806ADE8C;
        char* a = (char*)((void* (*)(char*, int, int))q->m4)(lbl_8060E79C, len, req->m8 ^ 0x100);
        MEM_copy(a, (void*)req->m0, req->m4);
        ((void (*)(int))q->m8)(req->m0);
        char* b = (char*)((void* (*)(char*, int, int))q->m4)(lbl_8060E7AC, len, req->m8);
        MEM_copy(b, a, req->m4);
        ((void (*)(char*))q->m8)(a);
        req->m0 = (int)b;
        h = FILESYS_read((RcJob*)req->mC, req->m4, (int)b + req->m4, len - req->m4, prio, (int)req);
        FILESYS_callbackop(h, (void (*)(int, int, int))iFILE_addbigreadcallback);
        req->m4 = len;
    } else {
        req->m14 = lbl_806ADE4C.m24;
        lbl_806ADE4C.m24 = (int)req;
        iFILE_ExecCommand(req->m10);
    }
}

extern "C" void iFILE_addbigopencallback(int h, int k, RcJob* req)
{
    int sav = (lbl_806ADE64[0] + ((unsigned char*)&h)[3])->m10;
    int x = FILESYS_completeop(h);
    if (k != 1) {
        ((RcJob*)req->m10)->mC = 4;
        iFILE_ExecCommand(req->m10);
    } else {
        h = FILESYS_read((RcJob*)x, 0, req->m0, 0x800, sav, (int)req);
        FILESYS_callbackop(h, (void (*)(int, int, int))iFILE_addbigreadcallback);
    }
}

/* near-miss: 98.0%; retail keeps the list cursor in r31 and the job in r29 (ours swapped) */
extern "C" int FILESYS_addbig(const char* name, int a, int b, int c)
{
    RcJob* p = lbl_806ADE70[0];
    RcJob* j = fn_803FA94C();
    *((unsigned char*)j + 2) = 9;
    j->m14 = c;
    j->m10 = b;
    while (p != 0) {
        if (strncmp(name, (char*)p->mC + 5, 0xff) == 0) {
            j->m24 = p->mC;
            iFILE_ExecCommand((int)j);
            return (int)j;
        }
        p = (RcJob*)p->m14;
    }
    RcBuf14* q = &lbl_806ADE8C;
    RcJob* n = (RcJob*)((void* (*)(const char*, int, int))q->m4)(name, 0x18, a);
    n->m4 = 0x800;
    n->m10 = (int)j;
    n->m0 = (int)((void* (*)(char*, int, int))q->m4)(lbl_8060E7AC, 0x800, a);
    n->m8 = a;
    FILESYS_callbackop(FILESYS_open(name, 1, b, (int)n), (void (*)(int, int, int))iFILE_addbigopencallback);
    return j->m0;
}

extern "C" void iFILE_delbigclosecallback(int a, int b, int p)
{
    FILESYS_completeop(a);
    iFILE_ExecCommand(p);
}

extern "C" int FILESYS_atomic(int (*cb)(int, int), int unused, int ctx, int arg)
{
    RcSysJ* s = &lbl_806ADE4C;
    int saved = s->m8;
    s->m8 = ctx;
    int r = cb(ctx, arg);
    s->m8 = saved;
    iFILE_ExecCommand(0);
    return r;
}

extern "C" void fn_803FA948()
{
}

extern "C" RcJob* fn_803FA94C()
{
    int i = 0;
    OSLockMutex(lbl_806ADE74);
    for (; i < lbl_806ADE4C.m0; i++) {
        if (*((unsigned char*)(lbl_806ADE4C.m18 + i) + 2) == 0) {
            *((unsigned char*)(lbl_806ADE4C.m18 + i) + 2) = 1;
            *((unsigned char*)(lbl_806ADE4C.m18 + i) + 3) = i;
            lbl_806EEA90++;
            if (lbl_806EEA90 == 0)
                lbl_806EEA90 = 1;
            *(unsigned short*)(lbl_806ADE4C.m18 + i) = lbl_806EEA90;
            break;
        }
    }
    OSUnlockMutex(lbl_806ADE74);
    if (i == lbl_806ADE4C.m0)
        for (;;)
            ;
    return lbl_806ADE64[0] + i;
}

extern "C" void fn_803FAA38(void* p)
{
    OSLockMutex(lbl_806ADE74);
    MEM_fill(p, 0, 0x30);
    OSUnlockMutex(lbl_806ADE74);
}

extern "C" int reservehandle(RcJob*)
{
    int i = 0;
    OSLockMutex(lbl_806ADE74);
    for (; i < lbl_806ADE4C.m4; i++) {
        if (*((unsigned char*)lbl_806ADE4C.m1C + i * 0x144 + 4) == 0) {
            *((unsigned char*)lbl_806ADE4C.m1C + i * 0x144 + 4) = 1;
            break;
        }
    }
    OSUnlockMutex(lbl_806ADE74);
    if (i == lbl_806ADE4C.m4)
        for (;;)
            ;
    return (int)(lbl_806ADE68[0] + i * 0x144);
}

extern "C" void fn_803FAB1C(void* p)
{
    OSLockMutex(lbl_806ADE74);
    MEM_fill(p, 0, 0x144);
    OSUnlockMutex(lbl_806ADE74);
}

extern "C" char* strchr(const char*, int);
extern "C" int strcmp(const char*, const char*);
extern "C" char* strcpy(char*, const char*);
extern "C" int fn_803FF058(const char*, int, void*);
extern "C" int getfilesize(void*);
extern "C" void closefile(void*);
extern "C" int fn_803FC3E8(int, char*, int, int*, int*);
extern "C" int FILESYS_bypassqueuefileinfo(const char* path, int flags, int* out)
{
    char name[0x100];
    char rest[0x100];
    char obj[0x148];
    int tmp;
    int val;
    int found = 0;
    int kind;
    *out = 0;
    rest[0] = 0;
    if (strchr(path, 0x7c) != 0) {
        if (path[0] == 0x7c)
            kind = 2;
        else {
            int n = strchr(path, 0x7c) - path;
            kind = 4;
            strncpy(name, path, n);
            name[n] = 0;
        }
        strcpy(rest, strchr(path, 0x7c) + 1);
    } else {
        kind = 1;
        if ((flags & 0x81) == 1) {
            kind = 3;
            strcpy(rest, path);
        }
    }
    if (kind & 1) {
        if (fn_803FF058(path, flags, obj) != 0) {
            found = 1;
            *out = getfilesize(obj);
            closefile(obj);
        }
    }
    if (found == 0 && (kind & 6)) {
        RcJob* p = lbl_806ADE70[0];
        while (p != 0 && found == 0) {
            if ((kind & 4) == 0 || strcmp((char*)p->mC + 5, name) == 0) {
                if (fn_803FC3E8(p->m0, rest, 0, &tmp, &val) != 0) {
                    found = 1;
                    *out = val;
                }
            }
            p = (RcJob*)p->m14;
        }
    }
    return found;
}

extern "C" int FILESYS_getopts(int* p)
{
    int n = p[0];
    if (n > 0x14)
        fn_8040074C(p, n);
    lbl_806ADE8C.mC = lbl_806EEAAC;
    lbl_806ADE8C.m10 = lbl_806EEAA8;
    MEM_copy(p, &lbl_806ADE8C, n);
    return 1;
}

extern "C" int FILESYS_setopts(int* p)
{
    if (p[0] <= 0x13)
        fn_8040074C(&lbl_806ADE8C, 0x14);
    MEM_copy(&lbl_806ADE8C, p, p[0]);
    lbl_806EEAA8 = lbl_806ADE8C.m10;
    lbl_806EEAAC = lbl_806ADE8C.mC;
    return 1;
}

/* near-miss: 99.1%; retail loads lbl_806EEAAC into r11 and lbl_806EEAA8 into r0, ours swap those temporaries */
extern "C" void fn_803FADD0(int a, int b)
{
    if (b == 0xffff && a != 0) {
        RcBuf14 s;
        RcBuf14* sp = &s;
        memset(sp, 0, 0x14);
        int x = lbl_806EEAAC;
        int y = lbl_806EEAA8;
        s.mC = x;
        s.m10 = y;
        s.m0 = 0x14;
        lbl_806ADE8C = s;
    }
}

extern "C" void fn_803FAE68()
{
    fn_803FADD0(1, 0xffff);
}
