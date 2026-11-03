/*
 * EA shared library code (ProDG, cflags_ealib: -O2 -G0),
 * .text 0x803D8234-0x803D9C34 (code-only unit, 38 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 */

extern "C" {
void fn_80377408(void* p); /* __builtin_delete */
void GXInvalidateVtxCache(void);
void GXInvalidateTexAll(void);
void VISetPostRetraceCallback(int cb);
void fn_803DB4CC(void);
void GXSetDither(int a);
void GXDrawDone(void);
void DCInvalidateRange(void);
void GXRedirectWriteGatherPipe(int a);
void GXRestoreWriteGatherPipe(void);
void GXEnableBreakPt(int a);
void GXDisableBreakPt(void);
void GXFlush(void);
int GXGetCPUFifo(void);
void GXGetFifoPtrs(int h, int* a, int* b);
int OSDisableInterrupts(void);
void OSRestoreInterrupts(int level);
}

struct lbl_Node;

/* Object at +8 of the owner: current node at +8, list head at +0xC. */
struct lbl_Impl {
    char pad0[0x8];
    lbl_Node* mCur;
    lbl_Node* mHead;
};

class lbl_Owner {
public:
    char pad0[0x8];
    lbl_Impl* m8;
    const void* mC;
    void fn_803DA098(lbl_Node* node);
};

union PPCWGPipe {
    unsigned char u8;
    unsigned short u16;
    unsigned int u32;
    float f32;
    double f64;
};
#include "../../include/ealib/gx_fifo.h"
/* fn_803D83BC still needs the symbol view to preserve its nearer scheduling. */
extern volatile PPCWGPipe GXWGFifo;

/* Render device object (size at least 0xF0): sub-objects at +0x10 and +0x38, owner at +0xEC. */
struct Unk803D8234Dev {
    int m0;
    int m4;
    int m8;
    const void* mC;
    char pad10[0x54 - 0x10];
    float m54;
    float m58;
    int m5C;
    char pad60[0xDC - 0x60];
    int mDC;
    char padE0[4];
    int mE4;
    int mE8;
    int mEC;
};

/* Ring-buffer record (12 bytes). */
struct Unk803D8234Rec {
    int m0;
    int m4;
    void (*m8)(int);
};

extern const void* lbl_80686770[];
extern int lbl_806272E8;
extern int lbl_80627A84;
extern int lbl_80627A80;
extern int lbl_806279B8;
extern int lbl_80627310[];
extern int lbl_8062731C;
extern int lbl_80627320;
extern int lbl_80627350;
extern volatile int lbl_80627348;
extern volatile int lbl_8062734C;
extern Unk803D8234Rec lbl_806273B4[];

extern "C" void fn_803D999C(void);

extern int lbl_80627380[];
extern int lbl_80627374[];
extern int lbl_80627324;
extern int lbl_806272D0;
extern volatile int lbl_80627304;
extern short lbl_806272E0;
/* GX draw-sync callback fn803D9454 (registered at803D8F24) publishes samples to render readers. */
extern volatile short lbl_806E9938[];
extern volatile short lbl_806EB938[];
extern volatile unsigned char lbl_806ED938[];
extern const void* lbl_80686740[];
extern const void* lbl_806867E8[];
extern const void* lbl_806867B8[];
extern const void* lbl_806867D0[];
extern const void* lbl_806867A0[];
extern int lbl_806272EC;
extern int lbl_8062C2B4;
extern int fn_803D81FC();
extern void fn_803D81A0(int unused, int v);

extern "C" {
void* fn_803D9BE0(struct Unk803D8234Light* sub, void* owner);
void fn_803DD73C(void* o, int c);
void* fn_803DD7C0(void* o);
void MEM_free(void* o);
void* fn_803D9A24(struct Unk803D8234Sub* sub, void* owner);
void GXReadPixMetric(int* a, int* b, int* c, int* d, int* e, int* f);
void fn_803D9664(void);
int fn_803D96FC(Unk803D8234Rec* a, Unk803D8234Rec* b);
void fn_803D9774(Unk803D8234Rec* r, int a, int b);
void fn_803D96CC(Unk803D8234Rec* r);
void fn_803D9C34(void* o, int c);
void fn_803D9AD0(struct Unk803D8234Sub* self, int c);
void fn_803D95FC(int a, int b);
int fn_803D98AC();
void fn_803E2D18(void);
void fn_803D9E70(void* o, int a, int b, unsigned short c, unsigned short d, unsigned short e, unsigned short f, int g, int h);
void fn_803D9ECC(void* o, int a, int b, int c, int d);
void fn_803D8828(void);
void GXSetZMode(int a, int b, int c);
void GXSetColorUpdate(int a);
void GXSetDrawSync(int a);
int OSReceiveMessage(void* q, void* msg, int flags);
void GXCopyDisp(int dest, int clear);
}

/* Light-like sub-object (vptr at +0x1C). */
struct Unk803D8234Light {
    int m0;
    float m4;
    float m8;
    int mC;
    int m10;
    int m14;
    void* m18;
    const void* m1C;
    void* m20;
    int m24;
};
struct Unk803D8234Sub {
    int m0;
    int m4;
    int m8;
    int mC;
    const void* m10;
    int m14;
    void* m18;
    float m1C;
    float m20;
    int m24;
    int m28;
    int m2C;
    int m30;
    int m34;
    int m38;
    int m3C;
    char pad40[0x80 - 0x40];
    int m80;
    int pad84;
    int m88;
    int m8C;
    int m90;
    int m94;
    float m98;
    int m9C;
    int mA0;
    int mA4;
    int mA8;
    int mAC;
    int mB0;
};
/* Second view of the render device for its destructor. */
struct Unk803D8234Buf {
    void* m0;
    int m4;
    int m8;
    int mC;
};
struct Unk803D8234DevB {
    int m0;
    int m4;
    lbl_Impl* m8;
    const void* mC;
    char pad10[0x74 - 0x10];
    int m74;
    char pad78[0xB4 - 0x78];
    void* mB4;
    void* mB8;
    char padBC[0xC4 - 0xBC];
    int mC4;
};
extern int lbl_80627300;
void fn_80364A50(void* p, int n);
extern void (*lbl_8062C66C)(void* p, int size);

extern "C" void fn_803D8234(Unk803D8234DevB* self, int c)
{
    self->mC = lbl_80686740;
    Unk803D8234Buf* b = (Unk803D8234Buf*)lbl_80627310[0];
    if (b) {
        lbl_8062C66C(b->m0, b->m8);
        b->m8 = 0;
        fn_80364A50(b, 0x10);
        lbl_80627310[0] = 0;
    }
    b = (Unk803D8234Buf*)lbl_80627310[1];
    if (b) {
        lbl_8062C66C(b->m0, b->m8);
        b->m8 = 0;
        fn_80364A50(b, 0x10);
        lbl_80627310[1] = 0;
    }
    if (self->mB8) {
        lbl_8062C66C(self->mB8, self->m74);
        self->mB8 = 0;
    }
    if (self->mB4) {
        lbl_8062C66C(self->mB4, self->mC4);
        self->mB4 = 0;
    }
    lbl_80627300 = 0;
    fn_803D9AD0((Unk803D8234Sub*)((char*)self + 0x38), 2);
    fn_803D9C34((char*)self + 0x10, 2);
    self->mC = lbl_80686770;
    while (self->m8->mHead != 0)
        ((lbl_Owner*)self)->fn_803DA098(self->m8->mHead);
    if (c & 1)
        lbl_8062C66C(self, 0xF0);
}

struct Unk803D83BCObj {
    char pad0[0x1C];
    float m1C;
    float m20;
    char pad24[0x94 - 0x24];
    int m94;
    float m98;
};
extern int lbl_80627A78;
extern "C" {
void fn_803DE3B4(int* a, int* b);
void fn_803DE374(int a, int b);
void fn_803DD794(int a);
void fn_803DE1BC(float* m);
void fn_803DE300(float* m, int a);
void fn_803DE3D0(int a);
void fn_803DDCD4();
void GXSetNumChans(int a);
void GXSetNumTexGens(int n);
void GXSetNumTevStages(int n);
void fn_803DB9C0(int a, int b);
void fn_803DBF78(int a, int b, int c, int d, int e);
void GXSetCullMode(int m);
void C_MTXOrtho(float* m, float t, float b, float l, float r, float n, float f);
void GXSetProjection(float* m, int type);
void GXSetCurrentMtx(int idx);
void GXSetChanCtrl(int a, int b, int c, int d, int e, int f, int g);
void GXSetBlendMode(int a, int b, int c, int d);
void GXSetAlphaCompare(int a, int b, int c, int d, int e);
void GXSetZCompLoc(int a);
void GXBegin(int a, int b, int c);
}

/* near-miss (96.9%): fullscreen alpha quad (unsigned conversion of the fade value, ortho setup, four GX vertices).
   Retail keeps 0.0 in f0 and loads self->m1C/m20 into f13/f12 around the volatile FIFO stores; ours picks other
   float registers. Tried const float locals for 0.0/1.0 and the colour in place or in a new variable. */
extern "C" void fn_803D83BC(Unk803D83BCObj* self)
{
    unsigned a = (unsigned)(self->m98 * 224.0f);
    if (a != 0) {
        float m1[16];
        char pad[0x28];
        float m2[16];
        int s0;
        int s1;
        a |= 0xFFFFFF00;
        fn_803DE3B4(&s0, &s1);
        fn_803DE374(0, 0);
        fn_803DD794(self->m94);
        fn_803DE1BC(m1);
        fn_803DE300(m1, 0);
        fn_803DE3D0(1);
        fn_803DDCD4();
        GXSetNumChans(1);
        GXSetNumTexGens(1);
        GXSetNumTevStages(1);
        fn_803DB9C0(0x5200, 1);
        fn_803DBF78(5, 9, 1, 4, 0);
        fn_803DBF78(5, 0xB, 1, 5, 0);
        fn_803DBF78(5, 0xD, 1, 4, 0);
        GXSetCullMode(0);
        C_MTXOrtho(m2, 0.0f, self->m20, 0.0f, self->m1C, 0.0f, -1.0f);
        GXSetProjection(m2, 1);
        GXSetCurrentMtx(0x3C);
        GXSetChanCtrl(4, 0, 0, 1, 0, 2, 2);
        GXSetBlendMode(1, 4, 5, 3);
        GXSetAlphaCompare(7, 0, 1, 7, 0);
        GXSetZCompLoc(0);
        GXSetZMode(0, 7, 0);
        GXBegin(0x80, 5, 4);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u32 = a;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = self->m1C;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u32 = a;
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = self->m1C;
        GXWGFifo.f32 = self->m20;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u32 = a;
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = self->m20;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u32 = a;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = 1.0f;
        fn_803DE374(s0, s1);
        lbl_80627A78 = 0;
    }
}

struct Unk803D8640Info {
    char pad0[4];
    unsigned short m4;
    unsigned short m6;
};
struct Unk803D8640Ring {
    int m0;
    int m4;
    int m8;
    int mC;
};
struct Unk803D8640Dev {
    char pad0[0x54];
    float m54;
    float m58;
    char pad5C[0xB8 - 0x5C];
    int mB8;
    int mBC;
    char padC0[0xCC - 0xC0];
    int mCC;
    float mD0;
    int mD4;
    int mD8;
    int mDC;
};
extern volatile int lbl_806272D4;
extern unsigned char lbl_80627308;
extern int lbl_80627390[];
/* Completion flags are shared with the retrace callback fn_803D8828 and
   draw-sync callback fn_803D9454; publish the reset before enabling completion. */
extern "C" void fn_803D8640(Unk803D8640Dev* self)
{
    fn_803E2D18();
    if (self->mCC != 0) {
        if (self->mD0 > 0.0f) {
            if (self->mD4 != 0)
                fn_803D83BC((Unk803D83BCObj*)((char*)self + 0x38));
            char* sub = (char*)self + 0x10;
            unsigned short w = ((Unk803D8640Info*)fn_803DD7C0((void*)self->mCC))->m4;
            unsigned short h = ((Unk803D8640Info*)fn_803DD7C0((void*)self->mCC))->m6;
            unsigned short fx = (int)self->m54;
            unsigned short fy = (int)self->m58;
            int flag = 0;
            if (self->mD8 != 0)
                flag = 1;
            fn_803D9E70(sub, 0, 0, fx, fy, w, h, 4, flag);
            fn_803D9ECC(sub, self->mCC, 0, 0, 0);
            if (self->mBC == 0)
                self->mD4 = 1;
        } else {
            self->mD4 = 0;
        }
    } else {
        self->mD4 = 0;
    }
    int msg;
    GXSetZMode(1, 3, 1);
    GXSetColorUpdate(1);
    lbl_806272E8 = self->mDC;
    OSReceiveMessage(lbl_80627390, &msg, 1);
    lbl_806272D4 = 0;
    lbl_80627304 = 1;
    GXSetDrawSync(0x8000);
    fn_803D95FC(0, (int)fn_803D8828);
    GXCopyDisp(self->mB8, lbl_80627308);
    GXFlush();
    lbl_8062731C ^= 1;
    Unk803D8640Ring* r = (Unk803D8640Ring*)fn_803D98AC();
    int used = r->m4 - r->m0;
    if (used > r->mC)
        r->mC = used;
    r->m4 = r->m0;
    lbl_806272D0--;
    lbl_806279B8++;
}

extern "C" void fn_803D9110(int a, int b)
{
    if (b == 0xFFFF && a != 0) {
        lbl_80627374[0] = 4;
        lbl_80627374[1] = 4;
        lbl_80627374[2] = 1;
        lbl_80627380[0] = 3;
        lbl_80627380[1] = 3;
        lbl_80627380[2] = 1;
    }
}

extern "C" Unk803D8234Dev* fn_803D9160(Unk803D8234Dev* self, int a, int b, int c)
{
    self->m0 = b;
    self->mC = lbl_80686770;
    self->m4 = a;
    self->m8 = c;
    return self;
}

extern "C" void fn_803D9180(lbl_Owner* self, int c)
{
    self->mC = lbl_80686770;
    while (self->m8->mHead != 0)
        self->fn_803DA098(self->m8->mHead);
    if (c & 1)
        fn_80377408(self);
}

extern "C" void* fn_803D91F8(Unk803D8234Dev* self, int owner)
{
    char* sub10 = (char*)self + 0x10;
    char* sub38 = (char*)self + 0x38;
    self->m8 = (int)sub38;
    self->m4 = 0;
    self->mC = lbl_80686740;
    self->m0 = (int)sub10;
    fn_803D9BE0((Unk803D8234Light*)sub10, self);
    fn_803D9A24((Unk803D8234Sub*)sub38, self);
    self->mEC = owner;
    return self;
}

extern "C" void fn_803D9268(Unk803D8234Sub* self)
{
    lbl_80627A80 = self->mA8;
    lbl_80627A84 = self->mAC;
    fn_803DB4CC();
    if (lbl_80627324) {
        int v = 0;
        if (self->mB0)
            v = 1;
        GXSetDither(v);
    }
}

extern "C" void fn_803D92D4(Unk803D8234Dev* dev)
{
    Unk803D8234Sub* s = (Unk803D8234Sub*)((char*)dev + 0x38);
    lbl_806272D0 = lbl_806272D0 + 1;
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    fn_803D81A0(fn_803D81FC() + 4, (int)dev);
    lbl_80627A80 = s->mA8;
    lbl_80627A84 = s->mAC;
    fn_803DB4CC();
    if (lbl_80627324) {
        int v = 0;
        if (s->mB0)
            v = 1;
        GXSetDither(v);
    }
    lbl_8062C2B4 = 0;
}

extern "C" void fn_803D9454(unsigned short x)
{
    if (x == 0x8000) {
        lbl_80627304 = 0;
    } else {
        lbl_806272E0 = x;
        if ((x & 0xF000) == 0x4000) {
            int a;
            int b;
            int c;
            int d;
            int e;
            int f;
            int idx = x & 0xFFF;
            GXReadPixMetric(&a, &b, &c, &d, &e, &f);
            lbl_806E9938[idx] = (unsigned short)d + (unsigned short)b;
            lbl_806EB938[idx] = (unsigned short)c + (unsigned short)a;
            lbl_806ED938[idx] = 1;
            fn_803D9664();
        }
    }
}

extern "C" void fn_803D95C4()
{
    fn_803D96CC(&lbl_806273B4[lbl_8062734C]);
}

extern "C" void fn_803D95FC(int a, int b)
{
    fn_803D9774(&lbl_806273B4[lbl_80627348], a, b);
    lbl_80627348 = lbl_80627348 + 1;
    if (lbl_80627348 > 0x7F)
        lbl_80627348 = 0;
}

extern "C" void fn_803D9664()
{
    int n = lbl_8062734C + 1;
    if (n > 0x7F)
        n = 0;
    if (fn_803D96FC(&lbl_806273B4[lbl_8062734C], &lbl_806273B4[n]))
        lbl_8062734C = n;
}

extern "C" void fn_803D96CC(Unk803D8234Rec* r)
{
    r->m8(r->m4);
}

extern "C" int fn_803D96FC(Unk803D8234Rec* a, Unk803D8234Rec* b)
{
    if (a->m0 == 0)
        return 0;
    int v = b->m0;
    if (v != 0) {
        GXEnableBreakPt(v);
        lbl_80627350 = 1;
    } else {
        GXDisableBreakPt();
        lbl_80627350 = v;
    }
    a->m0 = 0;
    return 1;
}

extern "C" void fn_803D9774(Unk803D8234Rec* r, int a, int b)
{
    int x;
    int y;
    GXFlush();
    GXGetFifoPtrs(GXGetCPUFifo(), &x, &y);
    while (r->m0 != 0)
        ;
    r->m4 = a;
    r->m8 = (void (*)(int))b;
    int ints = OSDisableInterrupts();
    r->m0 = y;
    if (lbl_80627350 == 0) {
        lbl_80627350 = 1;
        GXEnableBreakPt(y);
    }
    OSRestoreInterrupts(ints);
}

extern "C" void fn_803D9804(Unk803D8234Dev* self, float a, float b)
{
    self->m54 = a;
    self->m58 = b;
}

extern "C" void fn_803D9810(Unk803D8234Dev* self, float* a, float* b)
{
    *a = self->m54;
    *b = self->m58;
}

extern "C" int fn_803D9824(Unk803D8234Dev* self)
{
    return self->m5C;
}

extern "C" void fn_803D982C(Unk803D8234Dev* self, int v)
{
    self->mDC = v;
    lbl_806272E8 = v;
}

extern "C" int fn_803D983C(Unk803D8234Dev* self, int v)
{
    self->mE4 = v;
    lbl_80627A84 = v;
    fn_803DB4CC();
    return 1;
}

extern "C" int fn_803D986C(Unk803D8234Dev* self, int* out)
{
    *out = self->mE4;
    return 1;
}

extern "C" void fn_803D987C()
{
    GXDrawDone();
    lbl_806279B8 += 5;
}

extern "C" int fn_803D98AC()
{
    return lbl_80627310[lbl_8062731C];
}

extern "C" void fn_803D98C8(int x)
{
    DCInvalidateRange();
    if (lbl_80627320)
        fn_803D999C();
    lbl_80627320 = 1;
    GXRedirectWriteGatherPipe(x);
}

extern "C" void fn_803D9918(const double* p, unsigned n)
{
    while (n > 31) {
        double a = p[0];
        double b = p[1];
        double c = p[2];
        double d = p[3];
        EALIB_GX_FIFO(PPCWGPipe).f64 = a;
        EALIB_GX_FIFO(PPCWGPipe).f64 = b;
        EALIB_GX_FIFO(PPCWGPipe).f64 = c;
        EALIB_GX_FIFO(PPCWGPipe).f64 = d;
        p += 4;
        n -= 32;
    }
    while (n > 7) {
        double a = p[0];
        p++;
        n -= 8;
        EALIB_GX_FIFO(PPCWGPipe).f64 = a;
    }
    if (n != 0) {
        const unsigned char* q = (const unsigned char*)p;
        do {
            unsigned char c = *q;
            n--;
            q++;
            EALIB_GX_FIFO(PPCWGPipe).u8 = c;
        } while (n != 0);
    }
}

extern "C" void fn_803D999C()
{
    GXRestoreWriteGatherPipe();
    lbl_80627320 = 0;
}

extern "C" Unk803D8234Dev* fn_803D99C8(char* self, int a)
{
    *(const void**)(self + 0x10) = lbl_806867D0;
    *(int*)(self + 4) = a;
    *(int*)(self + 0) = 0;
    *(int*)(self + 8) = 0;
    *(int*)(self + 0xC) = 0;
    return (Unk803D8234Dev*)self;
}

extern "C" void fn_803D99F0(char* self, int c)
{
    *(const void**)(self + 0x10) = lbl_806867D0;
    if (c & 1)
        fn_80377408(self);
}

extern "C" void* fn_803D9A24(Unk803D8234Sub* self, void* owner)
{
    self->m0 = 0;
    self->m4 = (int)owner;
    self->m8 = 0;
    self->mC = 0;
    self->m10 = lbl_806867A0;
    self->m14 = 0;
    self->m18 = owner;
    self->m1C = 640.0f;
    self->m20 = 448.0f;
    self->m24 = 0x18;
    self->m28 = 0x18;
    self->m2C = -1;
    self->m30 = -1;
    self->m34 = -1;
    self->m38 = -1;
    self->m3C = 0;
    self->m80 = 0;
    self->m88 = 0;
    self->m8C = 0x40000;
    self->m90 = 0;
    self->m94 = 0;
    self->m98 = 0.0f;
    self->m9C = 0;
    self->mA0 = 1;
    self->mA4 = 1;
    self->mA8 = 0;
    self->mAC = 1;
    self->mB0 = 1;
    return self;
}

extern "C" void fn_803D9AD0(Unk803D8234Sub* self, int c)
{
    self->m10 = lbl_806867A0;
    if (lbl_806272EC) {
        lbl_806272EC = lbl_806272EC - 1;
        VISetPostRetraceCallback(0);
    }
    if (self->m94) {
        MEM_free(fn_803DD7C0((void*)self->m94));
        if (self->m94)
            fn_803DD73C((void*)self->m94, 3);
    }
    self->m10 = lbl_806867D0;
    if (c & 1)
        fn_80377408(self);
}

inline void LightInit(Unk803D8234Light* self, void* owner)
{
    self->m1C = lbl_806867E8;
    self->m0 = 0;
    self->m4 = 0.0f;
    self->m8 = 0.0f;
    self->mC = 0;
    self->m10 = 0;
    self->m14 = 1;
    self->m18 = owner;
}

extern "C" Unk803D8234Light* fn_803D9B6C(Unk803D8234Light* self, void* owner)
{
    self->m0 = 0;
    self->m1C = lbl_806867E8;
    self->m4 = 0.0f;
    self->m8 = 0.0f;
    self->mC = 0;
    self->m10 = 0;
    self->m14 = 1;
    self->m18 = owner;
    return self;
}

extern "C" void fn_803D9BAC(Unk803D8234Light* self, int c)
{
    self->m1C = lbl_806867E8;
    if (c & 1)
        fn_80377408(self);
}

extern "C" void* fn_803D9BE0(Unk803D8234Light* self, void* owner)
{
    LightInit(self, owner);
    self->m1C = lbl_806867B8;
    self->m20 = owner;
    self->m24 = 0;
    return self;
}

/*
 * Not matched yet: SetupFrameBuffers (510 instructions); fn_803D8828 and fn_803D9378 read the time base (mftb);
 * fn_803D9528 ends in a trap word after the fatal-error call. The original code stays linked for these.
 * fn_803D9918 uses the shared FIFO view for the retail signed displacement.
 */
