/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803E3280-0x803E3C08 (code-only unit, 9 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

union PPCWGPipe {
    unsigned char u8;
    unsigned short u16;
    unsigned int u32;
    float f32;
};
#include "../../include/ealib/gx_fifo.h"

/* sprite definition: size, texel rectangle and pixel offset */
struct Unk803E3280_Sprite {
    char pad0[2];
    unsigned char m2;
    unsigned char m3;
    unsigned short m4;
    unsigned short m6;
    char pad8;
    char m9;
    char mA;
};

/* element of the sprite list drawn by fn_803E3280: stride 0xC */
struct Unk803E3280_Item {
    Unk803E3280_Sprite* m0;
    float m4;
    float m8;
};

struct Unk803E3AFC_Gfx {
    unsigned int m0;
    char pad4[4];
    float m8;
    char padC[4];
    float m10, m14, m18, m1C, m20, m24;
    char pad28[0x5C - 0x28];
    void* m5C;
};

struct Unk803E3AFC_Obj {
    char pad0[0x20];
    Unk803E3AFC_Gfx m20;
};

struct Unk803E3B3C_Cam {
    char pad0[0x158];
    char m158[0x48];
    int m1A0;
};

extern char lbl_806AAB20[];
extern Unk803E3B3C_Cam* lbl_806EFC28;
extern int lbl_806EFC2C;
extern int lbl_80627A78;

extern "C" int fn_803DB5A0(void* p);
extern "C" int fn_803DBD54(void* p);
extern "C" void fn_803DBD84(void* p, int chrg);
extern "C" void GXBegin(int prim, int fmt, unsigned short n);
extern "C" void fn_803E3280(Unk803E3AFC_Obj* o, Unk803E3280_Item* items, int count);
extern "C" void MEM_free(void* p);
extern "C" void GXSetProjection(void* m, int type);

/* near-miss: dataflow matches the original code but the float register allocation/schedule differs
 * (retail keeps 0.5f, 1.0f and both conversion constants in f28-f31; 137 slots differ). */
/* Draws `count` textured quads (GX_QUADS, vertex format 7: position xy, z, color, texcoord uv). */
extern "C" void fn_803E3280(Unk803E3AFC_Obj* o, Unk803E3280_Item* items, int count)
{
    Unk803E3AFC_Gfx* g = &o->m20;
    GXBegin(0x80, 7, count * 4);
    for (int i = 0; i < count; i++) {
        Unk803E3280_Sprite* sp = items->m0;
        float f9 = (float)sp->m9;
        float f4 = g->m1C;
        float f10 = (float)sp->mA * f4 + (items->m8 - 0.5f);
        float f0 = items->m4;
        items++;
        float f5 = g->m18;
        float x0 = f9 * f5 + (f0 - 0.5f);
        float f2 = g->m24;
        float f11 = (float)sp->m2 + 1.0f;
        float x1 = f11 * f5 + x0;
        float f1 = g->m20;
        float f12 = (float)sp->m3 + 1.0f;
        float y1 = f12 * f4 + f10;
        float f6 = g->m8;
        float f13 = (float)sp->m4 + g->m14;
        unsigned int col = g->m0;
        float u0 = (f13 - 0.5f) * f2;
        float f0_2 = (float)sp->m6 + g->m10;
        float v0 = (f0_2 - 0.5f) * f1;
        float u1 = ((f13 + f11) - 0.5f) * f2;
        float v1 = ((f0_2 + f12) - 0.5f) * f1;
        EALIB_GX_FIFO(PPCWGPipe).f32 = x0;
        EALIB_GX_FIFO(PPCWGPipe).f32 = f10;
        EALIB_GX_FIFO(PPCWGPipe).f32 = f6;
        EALIB_GX_FIFO(PPCWGPipe).u32 = col;
        EALIB_GX_FIFO(PPCWGPipe).f32 = u0;
        EALIB_GX_FIFO(PPCWGPipe).f32 = v0;
        EALIB_GX_FIFO(PPCWGPipe).f32 = x1;
        EALIB_GX_FIFO(PPCWGPipe).f32 = f10;
        EALIB_GX_FIFO(PPCWGPipe).f32 = f6;
        EALIB_GX_FIFO(PPCWGPipe).u32 = col;
        EALIB_GX_FIFO(PPCWGPipe).f32 = u1;
        EALIB_GX_FIFO(PPCWGPipe).f32 = v0;
        EALIB_GX_FIFO(PPCWGPipe).f32 = x1;
        EALIB_GX_FIFO(PPCWGPipe).f32 = y1;
        EALIB_GX_FIFO(PPCWGPipe).f32 = f6;
        EALIB_GX_FIFO(PPCWGPipe).u32 = col;
        EALIB_GX_FIFO(PPCWGPipe).f32 = u1;
        EALIB_GX_FIFO(PPCWGPipe).f32 = v1;
        EALIB_GX_FIFO(PPCWGPipe).f32 = x0;
        EALIB_GX_FIFO(PPCWGPipe).f32 = y1;
        EALIB_GX_FIFO(PPCWGPipe).f32 = f6;
        EALIB_GX_FIFO(PPCWGPipe).u32 = col;
        EALIB_GX_FIFO(PPCWGPipe).f32 = u0;
        EALIB_GX_FIFO(PPCWGPipe).f32 = v1;
    }
}

/* static initializer / destructor of the global at lbl_806AAB20 */
/* Texture header (data at +0x10, or at +0x10 + m10 when flag 0x10000000 is set in mC). */
struct Unk803E34ACHdr {
    unsigned char m0;
    char pad1[3];
    short m4;
    short m6;
    char pad8[4];
    unsigned int mC;
    int m10;
};
struct Unk803E34ACEnt {
    int val;
    unsigned char key;
    char pad5[3];
};
extern Unk803E34ACEnt lbl_8062C5B8[];
extern Unk803E34ACEnt lbl_8062C5F8[];
extern Unk803E34ACEnt lbl_8062C610[];
extern char lbl_8060C9B4[];
extern "C" {
Unk803E34ACHdr* SHAPE_clut(Unk803E34ACHdr* h);
void* fn_803ED604(void* tag, int size, int z);
void GXInitTexObj(void* obj, void* data, unsigned short w, unsigned short h, int fmt, int a, int b, int c);
void GXInitTexObjCI(void* obj, void* data, unsigned short w, unsigned short h, int fmt, int a, int b, int c, int tlut);
void GXInitTlutObj(void* obj, void* data, int fmt, unsigned short n);
}

extern "C" void createfont(Unk803E3AFC_Obj* o)
{
    int off = *(int*)((char*)o + 0x1C);
    Unk803E3AFC_Gfx* g = &o->m20;
    Unk803E34ACHdr* hdr = (Unk803E34ACHdr*)((char*)o + off);
    Unk803E34ACHdr* pal = SHAPE_clut(hdr);
    g->m18 = 1.0f;
    g->m1C = 1.0f;
    g->m8 = 0.0f;
    g->m24 = 1.0f / (float)hdr->m4;
    g->m20 = 1.0f / (float)hdr->m6;
    g->m14 = 0.0f;
    g->m10 = 0.0f;
    if (pal == 0) {
        unsigned int i = 0;
        if (lbl_8062C5B8[0].key != *((unsigned char*)o + off)) {
            do {
                i++;
                if (i > 7)
                    break;
            } while (lbl_8062C5B8[i].key != hdr->m0);
        }
        if (g->m5C == 0)
            g->m5C = fn_803ED604(lbl_8060C9B4, 0x50, 0);
        char* t = (char*)g->m5C;
        void* tx = t + 0x28;
        void* data = (hdr->mC & 0x10000000) ? (char*)hdr + hdr->m10 : (char*)hdr + 0x10;
        GXInitTexObj(tx, data, (unsigned short)hdr->m4, (unsigned short)hdr->m6, lbl_8062C5B8[i].val, 0, 0, 0);
    } else {
        unsigned int i = 0;
        int pk = pal->m0;
        if (lbl_8062C610[0].key != *((unsigned char*)o + off)) {
            do {
                i++;
                if (i > 1)
                    break;
            } while (lbl_8062C610[i].key != hdr->m0);
        }
        unsigned int j = 0;
        if (lbl_8062C5F8[0].key != (unsigned char)pk) {
            do {
                j++;
                if (j > 2)
                    break;
            } while (lbl_8062C5F8[j].key != (unsigned char)pk);
        }
        if (g->m5C == 0)
            g->m5C = fn_803ED604(lbl_8060C9B4, 0x50, 0);
        char* t = (char*)g->m5C;
        void* tx = t + 0x28;
        void* tl = t + 0x1C;
        void* data = (hdr->mC & 0x10000000) ? (char*)hdr + hdr->m10 : (char*)hdr + 0x10;
        GXInitTexObjCI(tx, data, (unsigned short)hdr->m4, (unsigned short)hdr->m6, lbl_8062C610[i].val, 0, 0, 0, 0);
        void* pdata = (pal->mC & 0x10000000) ? (char*)pal + pal->m10 : (char*)pal + 0x10;
        GXInitTlutObj(tl, pdata, lbl_8062C5F8[j].val, (unsigned short)pal->m4);
    }
    *(void**)((char*)g->m5C + 0x4C) = hdr;
    *(void**)((char*)g->m5C + 0x48) = pal;
}

extern "C" void fn_803E3A74(int initialize, int priority)
{
    if (priority == 0xFFFF) {
        if (initialize)
            fn_803DBD54(lbl_806AAB20);
        else
            fn_803DBD84(lbl_806AAB20, 2);
    }
}

extern "C" void fn_803E3AC8(Unk803E3AFC_Obj* o, Unk803E3280_Sprite* spr, float x, float y)
{
    Unk803E3280_Item it;
    it.m0 = spr;
    it.m4 = x;
    it.m8 = y;
    fn_803E3280(o, &it, 1);
}

extern "C" void destroyfont(Unk803E3AFC_Obj* o)
{
    Unk803E3AFC_Gfx* g = &o->m20;
    if (g->m5C) {
        MEM_free(g->m5C);
        g->m5C = 0;
    }
}

extern "C" void fn_803E3B3C()
{
    Unk803E3B3C_Cam* c = lbl_806EFC28;
    if (c->m1A0 == 0) {
        if (lbl_806EFC2C == 0)
            GXSetProjection(c->m158, 0);
        else
            GXSetProjection(c->m158, 1);
    }
    lbl_80627A78 = 0;
    fn_803DB5A0(lbl_806AAB20);
}

extern "C" void fn_803E3BB0()
{
    fn_803E3A74(1, 0xFFFF);
}

extern "C" void fn_803E3BDC()
{
    fn_803E3A74(0, 0xFFFF);
}
