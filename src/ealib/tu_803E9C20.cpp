/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803E9C20-0x803EAF50 (code-only unit, 17 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

/* 0x14-byte array holder: m4 = capacity (ints), m8 = storage, m10 = length. */
struct Unk803EA068 {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
};

extern int lbl_8062C680;
extern void (*lbl_8062C66C)(void* p, int size);
extern void* (*lbl_8062C668)(int size, const char* tag);
extern char lbl_8060D944[];
extern char lbl_806AABD0[];
extern char lbl_8062C688[];
extern char lbl_8062C6A8[];
extern char lbl_8060DAA8[];

extern "C" int strlen(const char* s);
extern "C" int strcmp(const char* a, const char* b);
extern "C" char* strcpy(char* d, const char* s);
extern "C" void fn_803EA1E0(Unk803EA068* o);
extern "C" void fn_803E9C20(void* o, const char* s, void* v);
extern "C" void RenderSingleDrawTexture();
extern "C" void* fn_803DA440(void* self, void (*fn)(), void* a, void* b, void* c);

/* Open-addressing string hash table: m0 = live entries, m4 = slot count, m8 = slot array. An entry is the value
 * word followed by the NUL-terminated key (allocated strlen + 5 bytes). */
struct Unk803E9E30_Ent {
    int m0;
    char key[1];
};

struct Unk803E9E30_Tbl {
    int m0;
    unsigned int m4;
    Unk803E9E30_Ent** m8;
};

/* near-miss: retail strength-reduces idx*4 into a second counter (addi r31,r31,4; li r31,0 / li r30,0 at the wrap); ours
 * recomputes slwi each pass (about 2-4 slots). */
extern "C" void fn_803E9E30(Unk803E9E30_Tbl* t, const char* name)
{
    if (t->m8 == 0)
        return;
    unsigned int h = 0;
    for (int i = 0; name[i] != 0; i++)
        h ^= (h << 5) ^ name[i];
    unsigned int idx = h % t->m4;
    unsigned int probes = 0;
    for (;;) {
        Unk803E9E30_Ent* e = t->m8[idx];
        if (e && strcmp(name, e->key) == 0) {
            Unk803E9E30_Ent* hit = t->m8[idx];
            if (hit == 0)
                return;
            lbl_8062C66C(hit, strlen(hit->key) + 5);
            t->m8[idx] = 0;
            t->m0--;
            return;
        }
        if (probes > t->m4)
            return;
        idx++;
        if (idx >= t->m4)
            idx = 0;
        probes++;
    }
}

/* near-miss: same as fn_803E9E30. */
extern "C" int fn_803E9F50(Unk803E9E30_Tbl* t, const char* name, int* found)
{
    if (t->m8 == 0) {
        *found = 0;
        return 0;
    }
    unsigned int h = 0;
    for (int i = 0; name[i] != 0; i++)
        h ^= (h << 5) ^ name[i];
    unsigned int idx = h % t->m4;
    unsigned int probes = 0;
    for (;;) {
        Unk803E9E30_Ent* e = t->m8[idx];
        if (e && strcmp(name, e->key) == 0) {
            Unk803E9E30_Ent* hit = t->m8[idx];
            if (hit) {
                *found = 1;
                return hit->m0;
            }
        }
        if (probes > t->m4)
            break;
        idx++;
        if (idx >= t->m4)
            idx = 0;
        probes++;
    }
    *found = 0;
    return 0;
}

extern "C" Unk803EA068* fn_803EA068(Unk803EA068* o)
{
    o->m0 = 0;
    o->m4 = lbl_8062C680;
    o->m8 = 0;
    o->m10 = 0;
    return o;
}

extern "C" void fn_803EA08C(Unk803EA068* o)
{
    fn_803EA1E0(o);
    if (o->m8) {
        lbl_8062C66C((void*)o->m8, o->m4 << 2);
        o->m8 = 0;
    }
}

extern "C" void fn_803EA0E4(Unk803EA068* o, int chrg)
{
    fn_803EA1E0(o);
    if (o->m8) {
        lbl_8062C66C((void*)o->m8, o->m4 << 2);
        o->m8 = 0;
    }
    if (chrg & 1)
        lbl_8062C66C(o, 0x14);
}

extern "C" char* fn_803EA160(void* o, const char* s, int v)
{
    int* p = (int*)lbl_8062C668(strlen(s) + 5, lbl_8060D944);
    char* name = (char*)(p + 1);
    *p = v;
    strcpy(name, s);
    fn_803E9C20(o, s, p);
    return name;
}

extern "C" void fn_803EA1E0(Unk803EA068* o)
{
    if (o->m8 != 0) {
        unsigned int i = 0;
        if (i < (unsigned int)o->m4) {
            do {
                char* e = ((char**)o->m8)[i];
                if (e != 0)
                    lbl_8062C66C(e, strlen(e + 4) + 5);
                ((int*)o->m8)[i] = 0;
                i++;
            } while (i < (unsigned int)o->m4);
        }
    }
    o->m0 = 0;
    if (o->m10 != 0) {
        do {
            int* n = (int*)o->m10;
            o->m10 = n[2];
            lbl_8062C66C(n, 0xc);
        } while (o->m10 != 0);
    }
    o->m10 = 0;
}

extern "C" int fn_803EA2C4(unsigned char* t, int bit)
{
    int i = bit / 8;
    unsigned char b = *(unsigned char*)(((int)t) + i + 2);
    if ((b >> (bit & 7)) & 1)
        return 1;
    return 0;
}

extern "C" int fn_803EA320(int n);

extern "C" int fn_803EA2FC(unsigned short* p)
{
    return fn_803EA320(*p);
}

extern "C" int fn_803EA320(int n)
{
    return (n + 7) / 8 + 2;
}

struct Unk80373AD8Param {
    void* m0;
    void* m4;
};

struct Unk803E3C08 {
    Unk80373AD8Param* m0;
    unsigned char* m4;
    int* m8;
    int pad0C;
    int* m10;
    int* m14;
    int m18;
    int* m1C;
    int* m20;
    unsigned char* m24;
    void* m28;
    int m2C;
    int m30[8];
    int m50[24];
    int mB0;
    int mB4;
    int mB8;
    int mBC;
};

struct Unk80623E94 {
    int m0;
    void* m4;
    int m8;
    int pad0C[10];
    int m34;
    int pad38[3];
    int m44;
    int pad48;
    int m4C;
    int pad50[7];
    int m6C;
    int m70;
};

struct Unk80373AD8Mtx {
    float f[16];
};

extern "C" void fn_803DFBAC(Unk80373AD8Mtx* in, Unk80373AD8Mtx* out);

extern "C" void GXLoadPosMtxImm(Unk80373AD8Mtx* m, int id);

extern "C" void GXSetCurrentMtx(int id);

extern "C" void GXSetChanCtrl(int a, int b, int c, int d, int e, int f, int g);

extern "C" void GXSetNumChans(int n);

extern "C" int fn_803DB5A0(void* state);

extern "C" void fn_803DB9C0(int a, int b);

extern "C" void fn_803DD794(void* tex);

extern "C" void fn_803DDCD4(void);

union PPCWGPipe {
    unsigned char u8;
    unsigned short u16;
    unsigned int u32;
    float f32;
};

#include "../../include/ealib/gx_fifo.h"

extern "C" void GXCallDisplayList(void* list, int nbytes);

struct Unk803DBE08Out {
    int m0;
    int m4;
    inline Unk803DBE08Out(void* state);
    inline void Begin(int n);
    inline void StoreHdr(int per);
};

extern unsigned char* lbl_8062C660;

extern int lbl_8062C664;

extern "C" void DCStoreRange(void* p, int n);

extern "C" void fn_803DA188(int n);

extern "C" int fn_803DBE08(void* state, Unk803DBE08Out* out);
extern "C" void GXBegin(int a,int b,int c);
inline Unk803DBE08Out::Unk803DBE08Out(void* state)
{
    fn_803DBE08(state, this);
}

inline void Unk803DBE08Out::StoreHdr(int per)
{
    *lbl_8062C660 = (unsigned char)m0;
    *(unsigned short*)(lbl_8062C660 + 1) = per;
}

inline void Unk803DBE08Out::Begin(int n)
{
    GXBegin(0x90, 0, 0);
    EALIB_GX_FIFO(PPCWGPipe).u8 = (unsigned char)m0;
    EALIB_GX_FIFO(PPCWGPipe).u16 = n;
}

extern "C" void fn_803EA500(Unk803E3C08* a, Unk80623E94* b)
{
    Unk80373AD8Mtx mtx;
    if (a->mB0) {
        b->m4 = a->m0[6].m4;
        mtx = *(Unk80373AD8Mtx*)a->m0[1].m4;
        fn_803DFBAC(&mtx, &mtx);
        GXLoadPosMtxImm(&mtx, 0);
        GXSetCurrentMtx(0);
        if (b->m70) {
            GXSetChanCtrl(4, 0, 1, 1, 0, 2, 2);
            GXSetChanCtrl(5, 0, 1, 1, 0, 0, 2);
        } else {
            GXSetNumChans(1);
            GXSetChanCtrl(4, 0, 0, 0, 0, 2, 2);
            GXSetChanCtrl(5, 0, 0, 0, 0, 0, 2);
        }
        fn_803DB5A0(b->m4);
        if (b->m6C == 0) {
            fn_803DB9C0(b->m8, 3);
        }
    }
    if (a->m30[0]) {
        fn_803DD794(a->m0[5].m4);
    }
    fn_803DDCD4();
    if (a->mBC == 0) {
        void* dl = a->m28;
        int dlsz = a->m2C;
        if (a->mB8) {
            EALIB_GX_FIFO(PPCWGPipe).u8 = 0x40;
            EALIB_GX_FIFO(PPCWGPipe).u32 = (unsigned int)dl;
            EALIB_GX_FIFO(PPCWGPipe).u32 = dlsz;
        } else {
            GXCallDisplayList(dl, dlsz);
        }
    } else {
        Unk803DBE08Out v(b->m4);
        int n = *(int*)a->m0[0].m4;
        if (lbl_8062C660) {
            int per = (lbl_8062C664 - 3) / b->m0;
            per = per / 12 * 12;
            fn_803DB9C0(b->m8, 2);
            if (*(int*)lbl_8062C660 == -1) {
                v.StoreHdr(per);
                unsigned char* p = lbl_8062C660 + 3;
                unsigned char i = 0;
                while (i < per) {
                    int next = i + 1;
                    for (int j = 0; j < b->m0; j++) {
                        *p++ = i;
                    }
                    i = next;
                }
                for (; p < lbl_8062C660 + lbl_8062C664; p++) {
                    *p = 0;
                }
                DCStoreRange(lbl_8062C660, lbl_8062C664);
            }
            int sz = (per * b->m0 + 0x22) / 32 * 32;
            int first = 0;
            while (n >= per) {
                if (first) {
                    EALIB_GX_FIFO(PPCWGPipe).u8 = 0x40;
                    EALIB_GX_FIFO(PPCWGPipe).u32 = (unsigned int)lbl_8062C660;
                    EALIB_GX_FIFO(PPCWGPipe).u32 = sz;
                } else {
                    GXCallDisplayList(lbl_8062C660, sz);
                    first = 1;
                }
                int chunk = per;
                if (v.m0 == 0x98) {
                    chunk = per - 2;
                }
                fn_803DA188(chunk);
                n -= chunk;
            }
            unsigned i;
            v.Begin(n);
            if (b->m0 == 4) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i | (i << 8) | (i << 16) | (i << 24);
                }
            } else if (b->m0 == 3) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u16 = i | (i << 8);
                    EALIB_GX_FIFO(PPCWGPipe).u8 = i;
                }
            } else if (b->m0 == 2) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u16 = i | (i << 8);
                }
            } else if (b->m0 == 1) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u8 = i;
                }
            } else {
                for (i = 0; i < n; i++) {
                    for (int j = 0; j < b->m0; j++) {
                        EALIB_GX_FIFO(PPCWGPipe).u8 = i;
                    }
                }
            }
            n *= b->m0;
            n += 3;
            while (n & 0x1f) {
                n++;
                EALIB_GX_FIFO(PPCWGPipe).u8 = 0;
            }
        } else {
            unsigned i;
            v.Begin(n);
            if (b->m0 == 4) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i | (i << 16);
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i | (i << 16);
                }
            } else if (b->m0 == 3) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i | (i << 16);
                    EALIB_GX_FIFO(PPCWGPipe).u16 = i;
                }
            } else if (b->m0 == 2) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u32 = i | (i << 16);
                }
            } else if (b->m0 == 1) {
                for (i = 0; i < n; i++) {
                    EALIB_GX_FIFO(PPCWGPipe).u16 = i;
                }
            } else {
                for (i = 0; i < n; i++) {
                    for (int j = 0; j < b->m0; j++) {
                        EALIB_GX_FIFO(PPCWGPipe).u16 = i;
                    }
                }
            }
            n = (n + n) * b->m0;
            n += 3;
            while (n & 0x1f) {
                n++;
                EALIB_GX_FIFO(PPCWGPipe).u8 = 0;
            }
        }
    }
}


/* static initializer: registers the node lbl_806AABD0 */
extern "C" void fn_803EAEC4(int initialize, int priority)
{
    if (priority == 0xFFFF && initialize)
        fn_803DA440(lbl_806AABD0, RenderSingleDrawTexture, lbl_8062C688, lbl_8062C6A8, lbl_8060DAA8);
}

extern "C" void fn_803EAF24()
{
    fn_803EAEC4(1, 0xFFFF);
}

