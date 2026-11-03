/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803DA4EC-0x803DBFF4 (code-only unit, 32 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

extern "C" {
void fn_80377408(void* p); /* __builtin_delete */
void GXSetVtxAttrFmt(int fmt, int attr, int cnt, int type, int frac);
void GXSetCoPlanar(unsigned char enable);
void GXGetVtxDescv(void* list);
void GXSetVtxDescv(void* list);
void GXGetCullMode(int* mode);
void GXSetCullMode(int mode);
void GXClearVtxDesc(void);
void GXSetVtxDesc(int attr, int type);
void GXBegin(int a, int b, int c);
void GXSetZMode(int a, int b, int c);
void GXSetColorUpdate(int a);
}

union PPCWGPipe {
    unsigned char u8;
    unsigned short u16;
    unsigned int u32;
    float f32;
};
extern volatile PPCWGPipe GXWGFifo;

inline void WriteVec3I(float* v, int k)
{
    float x = v[k];
    float y = v[k + 1];
    float z = v[k + 2];
    GXWGFifo.f32 = x;
    GXWGFifo.f32 = y;
    GXWGFifo.f32 = z;
}

struct Unk803DA4ECVtxDesc {
    int a;
    int b;
};

/* Raster-state record (0x3C bytes, owned through the sized delete at lbl_8062C66C).
   Evidence: default init fn_803DBE8C, accessors fn_803DBE20..fn_803DBF40. */
struct Unk803DA4EC {
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
    int m30;
    int m34;
    int m38;
};

struct Unk803DA4EC3 {
    int m0;
    int m4;
    int m8;
    int mC;
};

extern Unk803DA4EC3 lbl_80627A98;
extern void (*lbl_8062C66C)(void* p, int size);
extern int lbl_80627A8C;
extern int lbl_80627A90;
extern int lbl_80627A7C;
extern int lbl_80627A80;
extern int lbl_80627A84;
extern int lbl_80627A88;
extern Unk803DA4EC lbl_80627AA8;
extern int lbl_80627AAC;

extern "C" void fn_803DBED8(void* self, int c);
extern "C" void fn_803DBD84(Unk803DA4EC* self, int c);
extern "C" Unk803DA4EC* fn_803DBE8C(Unk803DA4EC* self);

extern "C" void fn_803DB4CC()
{
    Unk803DA4EC* st = &lbl_80627AA8;
    int a = lbl_80627A80;
    int b = lbl_80627A84;
    if (st->m28 != 0)
        a = st->m28 == 2;
    if (st->m2C != 0)
        b = st->m2C == 1;
    if (st->m34 != 0)
        GXSetCullMode(a ? 1 : 2);
    else
        GXSetCullMode(0);
    int v = lbl_80627AAC;
    int e = 0;
    if (b)
        e = 1;
    GXSetZMode(1, v, e);
    int f = 0;
    if (lbl_80627A88)
        f = 1;
    GXSetColorUpdate(f);
}

extern "C" void fn_803DB9C0(int flags, int mode)
{
    if (flags == lbl_80627A8C && mode == lbl_80627A90)
        return;
    if (mode != -1) {
        GXClearVtxDesc();
        if (flags & 0x1)
            GXSetVtxDesc(0, 1);
        if (flags & 0x2)
            GXSetVtxDesc(1, 1);
        if (flags & 0x4)
            GXSetVtxDesc(2, 1);
        if (flags & 0x8)
            GXSetVtxDesc(3, 1);
        if (flags & 0x10)
            GXSetVtxDesc(4, 1);
        if (flags & 0x20)
            GXSetVtxDesc(5, 1);
        if (flags & 0x40)
            GXSetVtxDesc(6, 1);
        if (flags & 0x80)
            GXSetVtxDesc(7, 1);
        if (flags & 0x100)
            GXSetVtxDesc(8, 1);
        if (flags & 0x200)
            GXSetVtxDesc(9, mode);
        if (flags & 0x400)
            GXSetVtxDesc(10, mode);
        if (flags & 0x800)
            GXSetVtxDesc(25, mode);
        if (flags & 0x1000)
            GXSetVtxDesc(11, mode);
        if (flags & 0x2000)
            GXSetVtxDesc(12, mode);
        if (flags & 0x4000)
            GXSetVtxDesc(13, mode);
        if (flags & 0x8000)
            GXSetVtxDesc(14, mode);
        if (flags & 0x10000)
            GXSetVtxDesc(15, mode);
        if (flags & 0x20000)
            GXSetVtxDesc(16, mode);
        if (flags & 0x40000)
            GXSetVtxDesc(17, mode);
        if (flags & 0x80000)
            GXSetVtxDesc(18, mode);
        if (flags & 0x100000)
            GXSetVtxDesc(19, mode);
        if (flags & 0x200000)
            GXSetVtxDesc(20, mode);
    }
    lbl_80627A8C = flags;
    lbl_80627A90 = mode;
}

extern "C" void DisableCoPlanarZMode();
extern "C" void fn_803DBBE0(float* m)
{
    int savedFlags = lbl_80627A8C;
    int savedMode = lbl_80627A90;
    Unk803DA4ECVtxDesc list[27];
    GXGetVtxDescv(list);
    fn_803DB9C0(0x200, 1);
    GXSetVtxAttrFmt(5, 9, 1, 4, 0);
    if (lbl_80627A7C != 0)
        DisableCoPlanarZMode();
    int cull;
    GXGetCullMode(&cull);
    GXSetCullMode(3);
    GXBegin(0x90, 5, 3);
    WriteVec3I(m, 0);
    WriteVec3I(m, 3);
    WriteVec3I(m, 6);
    GXSetCullMode(cull);
    GXSetVtxDescv(list);
    fn_803DB9C0(savedFlags, savedMode);
    GXSetCoPlanar(1);
    lbl_80627A7C = 1;
}

extern "C" void fn_803DBCF0(int a, int b)
{
    if (b == 0xFFFF && a != 0) {
        lbl_80627A98.m0 = 4;
        lbl_80627A98.m4 = 4;
        lbl_80627A98.m8 = 1;
    }
}

extern "C" void fn_803DBD28(Unk803DA4EC* p)
{
    if (p)
        fn_803DBD84(p, 3);
}

extern "C" Unk803DA4EC* fn_803DBD54(Unk803DA4EC* self)
{
    fn_803DBE8C(self);
    return self;
}

extern "C" void fn_803DBD84(Unk803DA4EC* self, int c)
{
    fn_803DBED8(self, 2);
    if (c & 1)
        lbl_8062C66C(self, 0x3C);
}

extern "C" int fn_803DBDD8(int* out, int v)
{
    if (v < 0) {
        if (v >= -4) {
            *out = 0xB8;
            return 0;
        }
    }
    *out = v;
    return 1;
}

extern "C" int fn_803DBE08(int* src, int* dst)
{
    *dst = *src;
    return 1;
}

extern "C" int fn_803DBE18()
{
    return 0;
}

extern "C" int fn_803DBE20(Unk803DA4EC* o, int v)
{
    o->m34 = v;
    return 1;
}

extern "C" int fn_803DBE2C(Unk803DA4EC* o, int v)
{
    o->m4 = v;
    return 1;
}

extern "C" int fn_803DBE38(Unk803DA4EC* o, int v)
{
    o->m8 = v;
    return 1;
}

extern "C" int fn_803DBE44(Unk803DA4EC* o, int v)
{
    o->m30 = v;
    return 1;
}

extern "C" int fn_803DBE50(Unk803DA4EC* o, int v)
{
    o->mC = v;
    return 1;
}

extern "C" int fn_803DBE5C(Unk803DA4EC* o, int v)
{
    o->m10 = v;
    return 1;
}

extern "C" int fn_803DBE68()
{
    return 0;
}

extern "C" int fn_803DBE70()
{
    return 0;
}

extern "C" int fn_803DBE78(Unk803DA4EC* o, int v)
{
    o->m14 = v;
    return 1;
}

extern "C" int fn_803DBE84()
{
    return 0;
}

extern "C" Unk803DA4EC* fn_803DBE8C(Unk803DA4EC* self)
{
    self->m0 = 0x90;
    self->m4 = 3;
    self->mC = 0x10;
    self->m10 = 4;
    self->m8 = 0;
    self->m14 = 0;
    self->m34 = 0;
    self->m30 = 1;
    self->m28 = 0;
    self->m38 = 1;
    self->m2C = 0;
    return self;
}

extern "C" void fn_803DBED8(void* self, int c)
{
    if (c & 1)
        fn_80377408(self);
}

extern "C" void fn_803DBF00(Unk803DA4EC* o, int v)
{
    if (v != 0)
        o->m2C = 1;
    else
        o->m2C = 2;
}

extern "C" void fn_803DBF20(Unk803DA4EC* o, int v)
{
    if (v == 0)
        o->m28 = 2;
    else
        o->m28 = 1;
}

extern "C" void fn_803DBF40(Unk803DA4EC* o, int a, int b, int c, int d)
{
    o->m8 = 6;
    o->m18 = a;
    o->m1C = b;
    o->m20 = c;
    o->m24 = d;
}

extern "C" void fn_803DBF5C(int* a, int* b)
{
    *a = lbl_80627A8C;
    *b = lbl_80627A90;
}

extern "C" void fn_803DBF78(int fmt, int attr, int cnt, int type, int frac)
{
    GXSetVtxAttrFmt(fmt, attr, cnt, type, frac);
}

extern "C" void DisableCoPlanarZMode()
{
    GXSetCoPlanar(0);
    lbl_80627A7C = 0;
}

extern "C" void fn_803DBFC8()
{
    fn_803DBCF0(1, 0xFFFF);
}

/*
 * Not attempted (over the size limit, the original code stays linked):
 * fn_803DA4EC (0x640 bytes), fn_803DAB2C (0x9A0 bytes), fn_803DB5A0 (0x420 bytes, raster-state diff
 * against lbl_80627AA8 using the Unk803DA4EC record above).
 */
