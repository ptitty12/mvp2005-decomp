/*
 * Game code, .text 0x80317B88-0x8031B354 (code-only unit; original file and TU
 * boundaries unknown, placeholder). Every data object is extern by its lbl_ name.
 */
/* chooser.h declares these two with C++ linkage; here they are plain C symbols. */
#define floor fn_80410634_cxx
#define fn_8037C7FC fn_8037C7FC_cxx
#include "game/tu_802A6334.h"
#undef floor
#undef fn_8037C7FC
#include "game/tu_80317B88.h"

extern const char lbl_805E89D4[];
extern const char lbl_805E89E0[];
extern const char lbl_805E89EC[];
extern const char lbl_805E89F0[];
extern const char lbl_805E89F8[];
extern const char lbl_805E8A00[];
extern const char lbl_805E8A0C[];
extern const char lbl_805E79A4[];
extern const char lbl_805E9CC8[];
extern const float lbl_805E9FA4;
extern const float lbl_805E9FA8;
extern const float lbl_805E9FAC;
extern const void* const lbl_806658C0[];
extern const void* const lbl_80664008[];
extern const void* const lbl_80665650[];
extern const void* const lbl_80665828[];
extern const void* const lbl_80665860[];
extern const void* const lbl_806659F8[];
extern const void* const lbl_80693CAC[];
extern char lbl_806EF9DD;
extern int lbl_8061FF54[];
extern int lbl_8061FF64;
extern int lbl_8061FF6C;
extern int lbl_8061FF74;
extern FxCtx* lbl_8061FEDC;
struct FxV2b {
    float x;
    float y;
};
struct FxVx {
    float x;
    float y;
    float z;
    float w;
};
/* 0x98-byte particle record seen from fn_803185A0: two vertex arrays at 0x8 and 0x48. */
struct FxQb {
    int m0;
    int m4;
    FxVx a8[4];
    FxVx a48[4];
    int m88;
    int m8C;
    int m90;
    int m94;
};
extern int lbl_8061FF78[];
extern int lbl_8062C290;
extern const float lbl_805EA540;
extern const float lbl_805EA544;
extern const float lbl_805EA564;
extern const float lbl_805E9C90;
extern const float lbl_805E9C94;
extern const float lbl_805E9C98;
extern const float lbl_805E9C9C;
extern const char lbl_805E7988[];
extern const char* lbl_8061FF8C[];
extern int lbl_8061FF88;
extern FxCfg7 lbl_80691480;
extern const float lbl_805EA568;

extern "C" {
void fn_80328374(void* p, int a);
void OnWindowClose(void* p);
void fn_803E6B74(void* p);
void fn_803E6C28(void* p, int n);
int fn_803E69D8(void* p, const char* name, void* data, int size, int count);
int fn_803E6950(void* p, const char* name);
void fn_803E67A0(void* p, void* q);
int fn_803E68B0(void* p, int n);
int fn_803DBDD8(void* p, int n);
int fn_803DBE18(void* p, int n);
int fn_803DBE38(void* p, int n);
int fn_803DBE20(void* p, int n);
int fn_803DBE68(void* p, int n);
int fn_803DBE78(void* p, int n);
void fn_803DBD54(void* p);
void fn_803E6660(void* p);
int fn_8040B574(const char* a, const char* b);
void fn_8040B61C(void* p, int a);
void* fn_803774A8(unsigned size, int heap);
void fn_8035F77C(void* p);
void fn_80328234(void* p, int a, int b);
int fn_803E9B98(const void* a, void* b);
void* fn_8037C7FC();
void* fn_8037CF5C(void* c, unsigned hash, void* obj);
void fn_80377488(void* p);
float sinf(float x);
float cosf(float x);
int fn_8014DDAC(void* p);
void fn_803181E4(int a, int b, int c, float x1, float x2, float x3, float x4, int d, int e, int f, float x5, int g, int h, int s0, int s1);
void fn_80377408(void* p);
void fn_803E6708(void* p, int a);
void fn_803DBD84(void* p, int a);
}

extern "C" {
// PROTOS-BEGIN
void fn_80317B88(Fx* self, int a);
void fn_80317F30(Fx* self, int a);
void fn_80317F38(void* self, FxV2f* origin, FxV2f* p, float a);
void fn_80318090(void* self, FxQuad* obj, float a);
void fn_80318140(int a, int b, int c, float x1, float x2, float x3, float x4, int d, void* e, int f, float x5, int g, int h, int s0, int s1);
void fn_8031883C(Fx* self);
void fn_80318890(Fx* self);
void fn_803188C4(Fx* self);
void fn_803188F0(Fx* self);
void fn_803188FC(Fx* self, int id);
void fn_803191AC(Fx* self, int tex);
FxList* fn_80319218(FxList* self);
void fn_8031925C(Unk802A6334* self);
Unk802A6334_Node* fn_80319310(Unk802A6334* self, void* obj, int key);
FxP* fn_80319330(Unk802A6334* self, int key);
void fn_80319360(Unk802A6334* self);
void fn_803193F0(FxReg* self, int a);
void fn_80319424(FxReg* self);
int fn_8031945C(FxRegV* self);
int fn_80319490(void* self, int* tbl, int val, int a, int b, int c);
int fn_803194F4(FxReg* self, int a, int b, int c, int d);
FxCfg* fn_80319534(FxCfg* self);
FxCfg* fn_8031984C();
void* fn_8031987C(const char* s);
void fn_803198B8(FxCfg7* c, int on);
FxB* fn_80319A58(FxB* self, int n);
void fn_80319B7C(FxB* self, int a);
void fn_80319C2C(FxB* self, int id);
void fn_80319CD4();
void fn_80319D20();
void fn_8031A760(FxB* self, int tex);
FxC* fn_8031A7CC(FxC* self, int a);
void fn_8031A8B0(FxC* self, int a);
int fn_8031A8E0(const char* name);
void fn_8031AB8C(FxRec* r, int x, int y);
void fn_8031AC74();
void fn_8031B0E8(FxRec* self, int a);
void fn_8031B0F0(FxC* self, const char* name, int val);
int fn_8031B170(FxC* self, int** pp);
void fn_8031B228(FxC* self, int i);
void fn_8031B250(FxC* self, int i, int a, float f, FxSrc* b);
void fn_8031B27C(FxC* self, int i);
void fn_8031B2A8(FxC* self, int i);
void fn_8031A3D0(FxB* self, int arg);
void fn_803185A0(Fx* self, int tex, FxV2b* pos, FxV2b* uv, int a7, void* col, int p9, int p10);
void fn_8031A948(const char* text, unsigned len, FxRec* arr, int* count);
void fn_80317C94(Fx* self);
void fn_80318F3C(Fx* self, FxPart* pp);
void fn_803195A8(FxCfg* self);
// PROTOS-END
void fn_80319D78(FxB* self);
void fn_80319938(FxCfg7* cfg, int on);
}

// FUNCS-BEGIN

void fn_80317B88(Fx* self, int a) {
    self->m18 = lbl_806658C0;
    if (self->m13C) { fn_80377488(self->m13C); self->m13C = 0; }
    if (self->m140) { fn_80377488(self->m140); self->m140 = 0; }
    if (self->m144) { fn_80377488(self->m144); self->m144 = 0; }
    for (int i = 0; i < 6; i++) {
        if (self->m148[i]) { fn_80377488(self->m148[i]); self->m148[i] = 0; }
    }
    if (self->m30) fn_80377488(self->m30);
    if (self->m34) fn_80377488(self->m34);
    if (self->m38) fn_80377488(self->m38);
    lbl_8061FF44 = 0;
    fn_803E6708(self->mB8, 2);
    fn_803DBD84(self->m7C, 2);
    fn_80328374(self, a);
}

extern const float lbl_805E9C88;
extern const float lbl_805E9C8C;
extern "C" {
int fn_803DBE44(void* p, int n);
}
void fn_80317C94(Fx* self)
{
    char tmp[60];
    char* h = self->mB8;
    fn_803DBD54(tmp);
    char* m = self->m7C;
    fn_803E67A0(h, (char*)self + 0x3C);
    OnWindowClose(h);
    fn_803E6950(h, lbl_805E89D4);
    fn_803E69D8(h, lbl_805E89D4, lbl_8061FF54, 4, 1);
    fn_803E69D8(h, lbl_805E89E0, self->m30, 0xC, 0x4B0);
    fn_803E69D8(h, lbl_805E89EC, self->m34, 8, 0x4B0);
    fn_803E69D8(h, lbl_805E89F0, self->m38, 4, 0x4B0);
    fn_803DBDD8(m, 0x90);
    fn_803DBE18(m, 1);
    fn_803DBE38(m, 1);
    fn_803DBE20(m, 0);
    fn_803DBE68(m, 1);
    fn_803DBE78(m, 1);
    fn_803DBE44(m, 0);
    fn_803E69D8(h, lbl_805E89F8, m, 0x3C, 1);
    fn_803E69D8(h, lbl_805E8A00, (void*)lbl_8062C290, 0x40, 1);
    fn_803E68B0(h, 0x90);
    fn_803E6B74(h);
    for (int i = 0; i < self->mF4; i++) {
        int off = i * 0x98;
        for (int k = 0; k < 4; k++) {
            int o = k * 16;
            char* b0 = (char*)(off + (int)self->m13C) + 0x10;
            *(float*)(b0 + o) = lbl_805E9C88;
            char* b1 = (char*)(off + (int)self->m13C) + 0x14;
            *(float*)(b1 + o) = lbl_805E9C8C;
            char* b2 = (char*)(off + (int)self->m13C) + 0x50;
            *(float*)(b2 + o) = lbl_805E9C8C;
            char* b3 = (char*)(off + (int)self->m13C) + 0x54;
            *(float*)(b3 + o) = lbl_805E9C8C;
        }
    }
    for (int i = 0; i < *(int*)((char*)self + 0xF8); i++) {
        int off = i * 0x98;
        for (int k = 0; k < 4; k++) {
            int o = k * 16;
            char* b0 = (char*)(off + (int)self->m140) + 0x10;
            *(float*)(b0 + o) = lbl_805E9C88;
            char* b1 = (char*)(off + (int)self->m140) + 0x14;
            *(float*)(b1 + o) = lbl_805E9C8C;
            char* b2 = (char*)(off + (int)self->m140) + 0x50;
            *(float*)(b2 + o) = lbl_805E9C8C;
            char* b3 = (char*)(off + (int)self->m140) + 0x54;
            *(float*)(b3 + o) = lbl_805E9C8C;
        }
    }
    fn_8031883C(self);
    fn_803DBD84(tmp, 2);
}

void fn_80317F30(Fx* self, int a) { self->m24 = a; }

/* near-miss: rotate-about-point; retail builds the vectors through inline struct helpers (different stack layout and float slots) */
void fn_80317F38(void* self, FxV2f* origin, FxV2f* p, float a) {
    FxV2f q = {0, 0};
    q.x = p->x;
    q.y = p->y;
    float ang = a;
    FxV2f d;
    d.x = q.x - origin->x;
    d.y = q.y - origin->y;
    FxV2f d2 = d;
    if (!(ang >= lbl_805E9C90)) ang = lbl_805E9C90;
    if (!(ang <= lbl_805E9C94)) ang = lbl_805E9C94;
    if (!(ang <= lbl_805E9C98)) ang = ang - lbl_805E9C94;
    float s = sinf(ang);
    float c = cosf(ang);
    FxV2f r;
    r.x = d2.x * c - d2.y * s;
    r.y = d2.x * s + d2.y * c;
    r.x = r.x + origin->x;
    r.y = r.y + origin->y;
    p->y = r.y;
    p->x = r.x;
}

void fn_80318090(void* self, FxQuad* obj, float a) {
    FxV2f mid;
    mid.x = (obj->p[0].x + obj->p[1].x) * lbl_805E9C9C;
    mid.y = (obj->p[0].y + obj->p[3].y) * lbl_805E9C9C;
    fn_80317F38(self, &mid, (FxV2f*)&obj->p[0], a);
    fn_80317F38(self, &mid, (FxV2f*)&obj->p[1], a);
    fn_80317F38(self, &mid, (FxV2f*)&obj->p[2], a);
    fn_80317F38(self, &mid, (FxV2f*)&obj->p[3], a);
}

void fn_80318140(int a, int b, int c, float x1, float x2, float x3, float x4, int d, void* e, int f, float x5, int g, int h, int s0, int s1) {
    fn_803181E4(a, b, c, x1, x2, x3, x4, d, fn_8014DDAC(e), f, x5, g, h, s0, s1);
}

extern "C" void fn_80317240(float* x, float* y, int a, int b);
/* near-miss 95.6%: 103/103 insns, 34 slots differ: retail keeps the frame 8 bytes larger (hole at 0x10) and one more callee-saved register; vertex loop address registers differ */
void fn_803185A0(Fx* self, int tex, FxV2b* pos, FxV2b* uv, int a7, void* col, int p9, int p10)
{
    if (self->m118 < self->mF4) {
        FxQb* q = (FxQb*)&self->m13C[self->m118];
        q->m90 = self->m2C;
        self->m118 = self->m118 + 1;
        int w = lbl_8061FEDC->m28 != 0 ? lbl_8061FEDC->m2C : lbl_8061FEDC->m14;
        int h = lbl_8061FEDC->m28 != 0 ? lbl_8061FEDC->m30 : lbl_8061FEDC->m18;
        q->m94 = 0;
        q->m4 = fn_8014DDAC(col);
        q->m0 = tex;
        q->m8C = lbl_8061FF50;
        q->m88 = a7;
        lbl_8061FF50 = lbl_8061FF50 + 1;
        FxVx* d8 = q->a8;
        FxVx* d48 = q->a48;
        for (int i = 0; i < 4; i++) {
            float x = pos[i].x;
            float y = pos[i].y;
            fn_80317240(&x, &y, p9, p10);
            d8->x = x * (float)w;
            d8->y = y * (float)h;
            d8++;
            d48->x = uv[i].x;
            d48->y = uv[i].y;
            d48++;
        }
    }
}

void fn_8031883C(Fx* self) {
    if (self->m24 != 0) fn_803188F0(self);
    else fn_803188C4(self);
    lbl_8061FF50 = 0;
    self->m24 = 0;
}

void fn_80318890(Fx* self) {
    fn_803188C4(self);
    fn_803188F0(self);
}

void fn_803188C4(Fx* self) {
    for (int i = 0; i < 6; i++) self->m124[i] = 0;
    self->m118 = 0;
    self->m120 = 0;
}

void fn_803188F0(Fx* self) { self->m11C = 0; }

/* near-miss: three erase-by-id loops; only the count/element register assignment differs (r9/r11 swapped) */
void fn_803188FC(Fx* self, int id) {
    int i = 0;
    while (i < self->m118) {
        FxPart* d = &self->m13C[i];
        if (d->m90 == id) {
            int n = --self->m118;
            FxPart* s = (FxPart*)(n * 0x98 + (int)self->m13C);
            *d = *s;
        } else {
            i++;
        }
    }
    i = 0;
    while (i < self->m11C) {
        FxPart* d = &self->m140[i];
        if (d->m90 == id) {
            int n = --self->m11C;
            FxPart* s = (FxPart*)(n * 0x98 + (int)self->m140);
            *d = *s;
        } else {
            i++;
        }
    }
    i = 0;
    while (i < self->m120) {
        FxPart24* d = &self->m144[i];
        if (d->m20 == id) {
            int n = --self->m120;
            FxPart24* s = (FxPart24*)(n * 0x24 + (int)self->m144);
            *d = *s;
        } else {
            i++;
        }
    }
    self->m124[id] = 0;
    lbl_8061FF50 = 0;
    self->m24 = 0;
}

void fn_80318F3C(Fx* self, FxPart* pp)
{
    FxQb* p = (FxQb*)pp;
    int i = self->m28 * 6;
    ((FxV3*)self->m30)[i].x = p->a8[0].x;
    ((FxV3*)self->m30)[i].y = p->a8[0].y;
    ((FxV3*)self->m30)[i].z = p->a8[0].z;
    ((FxV2b*)self->m34)[i].x = p->a48[0].x;
    ((FxV2b*)self->m34)[i].y = p->a48[0].y;
    ((int*)self->m38)[i] = p->m4;
    i++;
    ((FxV3*)self->m30)[i].x = p->a8[1].x;
    ((FxV3*)self->m30)[i].y = p->a8[1].y;
    ((FxV3*)self->m30)[i].z = p->a8[1].z;
    ((FxV2b*)self->m34)[i].x = p->a48[1].x;
    ((FxV2b*)self->m34)[i].y = p->a48[1].y;
    ((int*)self->m38)[i] = p->m4;
    i++;
    ((FxV3*)self->m30)[i].x = p->a8[3].x;
    ((FxV3*)self->m30)[i].y = p->a8[3].y;
    ((FxV3*)self->m30)[i].z = p->a8[3].z;
    ((FxV2b*)self->m34)[i].x = p->a48[3].x;
    ((FxV2b*)self->m34)[i].y = p->a48[3].y;
    ((int*)self->m38)[i] = p->m4;
    i++;
    ((FxV3*)self->m30)[i].x = p->a8[0].x;
    ((FxV3*)self->m30)[i].y = p->a8[0].y;
    ((FxV3*)self->m30)[i].z = p->a8[0].z;
    ((FxV2b*)self->m34)[i].x = p->a48[0].x;
    ((FxV2b*)self->m34)[i].y = p->a48[0].y;
    ((int*)self->m38)[i] = p->m4;
    i++;
    ((FxV3*)self->m30)[i].x = p->a8[3].x;
    ((FxV3*)self->m30)[i].y = p->a8[3].y;
    ((FxV3*)self->m30)[i].z = p->a8[3].z;
    ((FxV2b*)self->m34)[i].x = p->a48[3].x;
    ((FxV2b*)self->m34)[i].y = p->a48[3].y;
    ((int*)self->m38)[i] = p->m4;
    i++;
    ((FxV3*)self->m30)[i].x = p->a8[2].x;
    ((FxV3*)self->m30)[i].y = p->a8[2].y;
    ((FxV3*)self->m30)[i].z = p->a8[2].z;
    ((FxV2b*)self->m34)[i].x = p->a48[2].x;
    ((FxV2b*)self->m34)[i].y = p->a48[2].y;
    ((int*)self->m38)[i] = p->m4;
}

/* Texture-state refresh: rebuilds the vertex description (fn_803191AC, fn_8031A760). */
void fn_803191AC(Fx* self, int tex) {
    char* r = self->mB8;
    OnWindowClose(r);
    fn_803E69D8(r, lbl_805E8A0C, (void*)tex, 0x58, -1);
    fn_803E6B74(r);
    fn_803E6C28(r, self->m28 * 6);
}

FxList* fn_80319218(FxList* self) {
    self->m1C = lbl_80664008;
    new ((void*)self) Unk802A6334(0, 5);
    return self;
}

void fn_8031925C(Unk802A6334* self) {
    Unk802A6334_Node* n = self->mC;
    int first = 0;
    while (n) {
        FxP* p = (FxP*)n->m0;
        if (first == 0) {
            first = 1;
            p->s2();
        }
        if (p->m4 == 0x19) p->s5(lbl_806EF9DD);
        else p->s4();
        n = n->m8;
    }
}

Unk802A6334_Node* fn_80319310(Unk802A6334* self, void* obj, int key) {
    return self->fn_802A6458(obj, key);
}

FxP* fn_80319330(Unk802A6334* self, int key) {
    for (Unk802A6334_Node* n = self->mC; n; n = n->m8) {
        if (((FxP*)n->m0)->m4 == key) return (FxP*)n->m0;
    }
    return 0;
}

void fn_80319360(Unk802A6334* self) {
    Unk802A6334_Node* n;
    while ((n = self->mC) != 0) {
        FxP* p = (FxP*)n->m0;
        self->fn_802A63F8(n);
        if (p) delete p;
    }
}

// fn_803193CC
FxReg::FxReg() {
    m4 = lbl_80665828;
    m0 = 0;
    lbl_8061FF64 = (int)this;
}

void fn_803193F0(FxReg* self, int a) {
    self->m4 = lbl_80665828;
    if (a & 1) fn_80377408(self);
}

void fn_80319424(FxReg* self) { self->m0 = (int)fn_8031987C(lbl_805E9CC8); }

int fn_8031945C(FxRegV* self) { return self->s5(); }

int fn_80319490(void* self, int* tbl, int val, int a, int b, int c) {
    int k = 0;
    if (a) k = 2;
    if (b) k = 1;
    if (c) k = 3;
    for (int i = 0; i < 3; i++) {
        char* base = (char*)tbl + 4;
        int off = i * 4 + k * 12;
        if (val > *(int*)(base + off)) return i;
    }
    return 3;
}

int fn_803194F4(FxReg* self, int a, int b, int c, int d) {
    return fn_80319490(self, (int*)self->m0, a, b, c, d);
}

/* m34 is written after m40 in source; that gives retail's store order. */
FxCfg* fn_80319534(FxCfg* self) {
    new ((void*)self) Unk8037B3E8(0xCED26CE8U);
    self->m0 = lbl_80665860;
    self->m38 = 1;
    self->m3C = 1;
    self->m40 = 1;
    self->m34 = 0;
    self->m44 = 0;
    self->m48 = 1;
    self->m4C = 1;
    self->m50 = 1;
    self->m54 = 1;
    self->m58 = 1;
    return self;
}

void fn_803195A8(FxCfg* self)
{
    Unk8037B3E8* b = (Unk8037B3E8*)self;
    b->fn_8037B7E4("lod");
    const char* rng = "0,1000,1";
    b->fn_8037B494("PLAYER_LOD_0_INGAME_MIN_IMPORTANCE", (int*)((char*)self + 0x4), rng, 0);
    b->fn_8037B494("PLAYER_LOD_1_INGAME_MIN_IMPORTANCE", (int*)((char*)self + 0x8), rng, 0);
    b->fn_8037B494("PLAYER_LOD_2_INGAME_MIN_IMPORTANCE", (int*)((char*)self + 0xC), rng, 0);
    b->fn_8037B494("PLAYER_LOD_0_OUTOFGAME_MIN_IMPORTANCE", (int*)((char*)self + 0x10), rng, 0);
    b->fn_8037B494("PLAYER_LOD_1_OUTOFGAME_MIN_IMPORTANCE", (int*)((char*)self + 0x14), rng, 0);
    b->fn_8037B494("PLAYER_LOD_2_OUTOFGAME_MIN_IMPORTANCE", (int*)((char*)self + 0x18), rng, 0);
    b->fn_8037B494("PLAYER_LOD_0_HRD_MIN_IMPORTANCE", (int*)((char*)self + 0x1C), rng, 0);
    b->fn_8037B494("PLAYER_LOD_1_HRD_MIN_IMPORTANCE", (int*)((char*)self + 0x20), rng, 0);
    b->fn_8037B494("PLAYER_LOD_2_HRD_MIN_IMPORTANCE", (int*)((char*)self + 0x24), rng, 0);
    b->fn_8037B494("PLAYER_LOD_0_JUMBOTRON_MIN_IMPORTANCE", (int*)((char*)self + 0x28), rng, 0);
    b->fn_8037B494("PLAYER_LOD_1_JUMBOTRON_MIN_IMPORTANCE", (int*)((char*)self + 0x2C), rng, 0);
    b->fn_8037B494("PLAYER_LOD_2_JUMBOTRON_MIN_IMPORTANCE", (int*)((char*)self + 0x30), rng, 0);
    b->fn_8037B458("ENABLE_HELMET_ENVIRONMENT_MAPS", (int*)((char*)self + 0x38), 0, 0);
    b->fn_8037B458("ENABLE_DUST_CLOUD_EFFECT", (int*)((char*)self + 0x3C), 0, 0);
    b->fn_8037B458("ENABLE_FRONTEND_MOVIES", (int*)((char*)self + 0x40), 0, 0);
    b->fn_8037B494("SCREEN_RES_INDEX", (int*)((char*)self + 0x34), 0, 0);
    b->fn_8037B458("ENABLE_LIVE_JUMBOTRONS", (int*)((char*)self + 0x44), 0, 0);
    b->fn_8037B458("ENABLE_PLAYER_SHADOWS", (int*)((char*)self + 0x48), 0, 0);
    b->fn_8037B458("DISPLAY_CROWDS", (int*)((char*)self + 0x4C), 0, 0);
    b->fn_8037B458("DISPLAY_UMPIRES", (int*)((char*)self + 0x50), 0, 0);
    b->fn_8037B458("DISPLAY_PIP_WINDOWS", (int*)((char*)self + 0x54), 0, 0);
    b->fn_8037B458("DISPLAY_CATCHER_IN_BATTER_PITCHER_VIEW", (int*)((char*)self + 0x58), 0, 0);
}

FxCfg* fn_8031984C() { return fn_80319534((FxCfg*)operator new(0x5C, lbl_806EF9B4)); }

void* fn_8031987C(const char* s) { return fn_8037CF5C(fn_8037C7FC(), 0xCED26CE8, (void*)s); }

void fn_803198B8(FxCfg7* c, int on) {
    fn_80319938(c, on);
    if (on) lbl_80691480 = *c;
    lbl_8061FF6C = on;
}

extern "C" {
void fn_803D9C68(int o, int v);
void fn_803D9C8C(int o, int v);
void fn_803D9CB0(int o, float v);
void fn_803D9CD4(int o, float v);
void fn_803D9CF8(int o, int* c);
void fn_803D9D24(int o, int v);
}
void fn_80319938(FxCfg7* c, int on)
{
    float k = lbl_805E9FA4;
    fn_803D9C68(lbl_8061FEDC->m38->m4 + 0x10, on ? 1 : 0);
    fn_803D9C8C(lbl_8061FEDC->m38->m4 + 0x10, on ? 5 : 0);
    fn_803D9CB0(lbl_8061FEDC->m38->m4 + 0x10, *(float*)&c->mC * k);
    fn_803D9CD4(lbl_8061FEDC->m38->m4 + 0x10, *(float*)&c->m10 * k);
    struct { int v; } col;
    col.v = (c->m0 << 24) | ((c->m4 << 16) & 0xFF0000) | ((c->m8 << 8) & 0xFF00);
    fn_803D9CF8(lbl_8061FEDC->m38->m4 + 0x10, &col.v);
    fn_803D9D24(lbl_8061FEDC->m38->m4 + 0x10, 1);
}

FxB* fn_80319A58(FxB* self, int n) {
    char* sub = (char*)self + 0x38;
    int k = n - 1;
    int zero = 0;
    char tmp[8];
    fn_80328234(self, 0x13, 0);
    self->m18 = lbl_80665650;
    self->m38 = lbl_80693CAC;
    *(int*)(sub + 4) = zero;
    *(int*)(sub + 0x14) = fn_803E9B98(lbl_805E79A4, tmp);
    *(int*)(sub + 0x10) = 1;
    fn_803DBD54(self->m78);
    fn_803E6660(self->mB4);
    lbl_8061FF74 = (int)self;
    self->m28 = zero;
    self->mF0 = n;
    FxPart* arr = (FxPart*)fn_803774A8(n * 0x98, 0);
    FxPart* q = arr;
    if (n != 0) {
        do {
            fn_8035F77C(q);
            q++;
        } while (k-- != 0);
    }
    int n6 = n * 6;
    self->mF8 = arr;
    self->mF4 = 0;
    self->m24 = 0;
    self->m2C = (char*)fn_803774A8(n * 0x48, 0);
    self->m30 = (char*)fn_803774A8(n6 * 8, 0);
    self->m34 = (char*)fn_803774A8(n6 * 4, 0);
    self->mF4 = 0;
    fn_80319D78(self);
    return self;
}

void fn_80319B7C(FxB* self, int a) {
    self->m18 = lbl_80665650;
    if (self->mF8) { fn_80377488(self->mF8); self->mF8 = 0; }
    if (self->m2C) fn_80377488(self->m2C);
    if (self->m30) fn_80377488(self->m30);
    if (self->m34) fn_80377488(self->m34);
    lbl_8061FF74 = 0;
    fn_803E6708(self->mB4, 2);
    fn_803DBD84(self->m78, 2);
    fn_80328374(self, a);
}

void fn_80319C2C(FxB* self, int id) {
    int i = 0;
    while (i < self->mF4) {
        FxPart* d = &self->mF8[i];
        if (d->m90 == id) {
            int n = --self->mF4;
            FxPart* s = (FxPart*)(n * 0x98 + (int)self->mF8);
            *d = *s;
        } else {
            i++;
        }
    }
}

void fn_80319CD4() {
    if (lbl_8061FF74 == 0) lbl_8061FF74 = (int)fn_80319A58((FxB*)operator new(0xFC, 0), 0x5D);
}

void fn_80319D20() {
    FxBV* o = (FxBV*)lbl_8061FF74;
    if (o) {
        delete o;
        lbl_8061FF74 = 0;
    }
}

extern int lbl_8061FE90;
extern "C" {
int fn_80328648(int a);
void fn_8031259C(FxCtx* c);
void fn_803125D0(FxCtx* c);
void fn_8031A4F0(FxB* self, FxPart* p);
}
void fn_80319D78(FxB* self)
{
    char* h = self->mB4;
    char* m = self->m78;
    fn_803E67A0(h, (char*)self + 0x38);
    OnWindowClose(h);
    fn_803E6950(h, lbl_805E89D4);
    fn_803E69D8(h, lbl_805E89D4, lbl_8061FF78, 4, 1);
    fn_803E69D8(h, lbl_805E89E0, self->m2C, 0xC, 0x4B0);
    fn_803E69D8(h, lbl_805E89EC, self->m30, 8, 0x4B0);
    fn_803E69D8(h, lbl_805E89F0, self->m34, 4, 0x4B0);
    fn_803DBDD8(m, 0x90);
    fn_803DBE18(m, 1);
    fn_803DBE38(m, 1);
    fn_803DBE20(m, 0);
    fn_803DBE68(m, 1);
    fn_803DBE78(m, 1);
    fn_803E69D8(h, lbl_805E89F8, m, 0x3C, 1);
    fn_803E69D8(h, lbl_805E8A00, (void*)lbl_8062C290, 0x40, 1);
    fn_803E68B0(h, 0x90);
    fn_803E6B74(h);
    for (int i = 0; i < self->mF0; i++) {
        int off = i * 0x98;
        for (int k = 0; k < 4; k++) {
            int o = k * 16;
            char* b0 = (char*)(off + (int)self->mF8) + 0x10;
            *(float*)(b0 + o) = lbl_805E9FA8;
            char* b1 = (char*)(off + (int)self->mF8) + 0x14;
            *(float*)(b1 + o) = lbl_805E9FAC;
            char* b2 = (char*)(off + (int)self->mF8) + 0x50;
            *(float*)(b2 + o) = lbl_805E9FAC;
            char* b3 = (char*)(off + (int)self->mF8) + 0x54;
            *(float*)(b3 + o) = lbl_805E9FAC;
        }
    }
    self->mF4 = 0;
}

void fn_8031A3D0(FxB* self, int arg)
{
    if (lbl_8061FE90 != 0) {
        self->mF4 = 0;
        return;
    }
    int lo = 0;
    int hi = 4;
    int tex = 0;
    if (fn_80328648(arg) - 1 > 0) {
        lo = 5;
        hi = 9;
        tex = 5;
    }
    FxCtx* ctx = lbl_8061FEDC;
    fn_8031259C(ctx);
    int n = self->mF4;
    FxPart* p = self->mF8;
    int key = p->m0;
    while (n > 0) {
        while (n > 0 && key == p->m0) {
            if (p->m90 >= lo && p->m90 <= hi) {
                fn_8031A4F0(self, p);
                self->m24 = self->m24 + 1;
            }
            p++;
            n--;
        }
        if (self->m24 != 0) {
            fn_8031A760(self, key);
            self->m24 = 0;
        }
        if (n > 0)
            key = p->m0;
    }
    fn_803125D0(ctx);
    fn_80319C2C((FxB*)lbl_8061FF74, tex);
}

void fn_8031A4F0(FxB* self, FxPart* pp)
{
    FxQb* p = (FxQb*)pp;
    int i = self->m24 * 6;
    ((FxV3*)self->m2C)[i].x = p->a8[0].x;
    ((FxV3*)self->m2C)[i].y = p->a8[0].y;
    ((FxV3*)self->m2C)[i].z = p->a8[0].z;
    ((FxV2b*)self->m30)[i].x = p->a48[0].x;
    ((FxV2b*)self->m30)[i].y = p->a48[0].y;
    ((int*)self->m34)[i] = p->m4;
    i++;
    ((FxV3*)self->m2C)[i].x = p->a8[1].x;
    ((FxV3*)self->m2C)[i].y = p->a8[1].y;
    ((FxV3*)self->m2C)[i].z = p->a8[1].z;
    ((FxV2b*)self->m30)[i].x = p->a48[1].x;
    ((FxV2b*)self->m30)[i].y = p->a48[1].y;
    ((int*)self->m34)[i] = p->m4;
    i++;
    ((FxV3*)self->m2C)[i].x = p->a8[3].x;
    ((FxV3*)self->m2C)[i].y = p->a8[3].y;
    ((FxV3*)self->m2C)[i].z = p->a8[3].z;
    ((FxV2b*)self->m30)[i].x = p->a48[3].x;
    ((FxV2b*)self->m30)[i].y = p->a48[3].y;
    ((int*)self->m34)[i] = p->m4;
    i++;
    ((FxV3*)self->m2C)[i].x = p->a8[0].x;
    ((FxV3*)self->m2C)[i].y = p->a8[0].y;
    ((FxV3*)self->m2C)[i].z = p->a8[0].z;
    ((FxV2b*)self->m30)[i].x = p->a48[0].x;
    ((FxV2b*)self->m30)[i].y = p->a48[0].y;
    ((int*)self->m34)[i] = p->m4;
    i++;
    ((FxV3*)self->m2C)[i].x = p->a8[3].x;
    ((FxV3*)self->m2C)[i].y = p->a8[3].y;
    ((FxV3*)self->m2C)[i].z = p->a8[3].z;
    ((FxV2b*)self->m30)[i].x = p->a48[3].x;
    ((FxV2b*)self->m30)[i].y = p->a48[3].y;
    ((int*)self->m34)[i] = p->m4;
    i++;
    ((FxV3*)self->m2C)[i].x = p->a8[2].x;
    ((FxV3*)self->m2C)[i].y = p->a8[2].y;
    ((FxV3*)self->m2C)[i].z = p->a8[2].z;
    ((FxV2b*)self->m30)[i].x = p->a48[2].x;
    ((FxV2b*)self->m30)[i].y = p->a48[2].y;
    ((int*)self->m34)[i] = p->m4;
}

void fn_8031A760(FxB* self, int tex) {
    char* r = self->mB4;
    OnWindowClose(r);
    fn_803E69D8(r, lbl_805E8A0C, (void*)tex, 0x58, -1);
    fn_803E6B74(r);
    fn_803E6C28(r, self->m24 * 6);
}

/* near-miss: retail keeps li/lis/lfs inside the item loops; ours hoists them */
FxC* fn_8031A7CC(FxC* self, int a) {
    fn_80328234(self, 0xD, a);
    self->m70 = 0;
    self->m18 = lbl_806659F8;
    self->m24 = 0;
    for (int i = 0; i < 5; i++) {
        self->a74[i].m0 = 0;
        self->a74[i].m4 = lbl_805EA540;
        self->a74[i].mC = lbl_805EA544;
        self->a74[i].m8 = lbl_805EA544;
        self->a74[i].m10 = 1;
    }
    for (int i = 0; i < 5; i++) {
        self->aD8[i].m0 = 0;
        self->aD8[i].m4 = lbl_805EA540;
        self->aD8[i].mC = lbl_805EA544;
        self->aD8[i].m8 = lbl_805EA544;
        self->aD8[i].m10 = 1;
    }
    self->m14 = 0;
    fn_8040B61C(self, lbl_8061FF88);
    return self;
}

void fn_8031A8B0(FxC* self, int a) {
    self->m18 = lbl_806659F8;
    fn_80328374(self, a);
}

/* Name/value lookup in lbl_8061FF8C (0x6A entries), -1 if absent. */
int fn_8031A8E0(const char* name) {
    for (int i = 0; i <= 0x69; i++) {
        if (fn_8040B574(name, lbl_8061FF8C[i]) == 0) return i;
    }
    return -1;
}

extern "C" {
void fn_80329AEC(const char** pp, const char** out, unsigned* pn, int delim);
void fn_80329C4C(const char** pp, unsigned* pn, int* out, int lo, int hi, int delim);
int fn_80329B98(const char** pp, unsigned* pn, float* out, int delim);
}
extern "C" char* strcpy(char* d, const char* s);
/* near-miss 97.6%: 145/145 insns; retail keeps the record pointer in r31 and &sp14 in r30 (ours the other way round), and stores sp10 before sp8 */
void fn_8031A948(const char* text, unsigned len, FxRec* arr, int* count)
{
    const char* sp8;
    const char* spC;
    unsigned sp10;
    sp10 = len;
    sp8 = text;
    spC = 0;
    const char* sp14;
    int sp18;
    int k = 7;
    do {
        fn_80329AEC(&sp8, &spC, &sp10, 0x2C);
    } while (--k != 0);
    fn_80329AEC(&sp8, &spC, &sp10, 0xA);
    int i = 0;
    fn_80329C4C(&sp8, &sp10, count, 0, 0x50, 0xA);
    if (i < *count) {
        do {
            fn_80329AEC(&sp8, &sp14, &sp10, 0x2C);
            strcpy(arr[i].name, sp14);
            fn_80329C4C(&sp8, &sp10, &sp18, 0, 1, 0x2C);
            int v = 1;
            if (sp18 == 0)
                v = 0;
            arr[i].m20 = v;
            fn_80329AEC(&sp8, &sp14, &sp10, 0x2C);
            arr[i].m24 = fn_8031A8E0(sp14);
            fn_80329C4C(&sp8, &sp10, &arr[i].m28, 0, 1000000, 0x2C);
            if (arr[i].m28 == 0)
                arr[i].m28 = 1;
            fn_80329C4C(&sp8, &sp10, &arr[i].m2C, 0, 1000000, 0x2C);
            if (arr[i].m2C == 0)
                arr[i].m2C = 1;
            fn_80329B98(&sp8, &sp10, &arr[i].m30, 0x2C);
            fn_80329B98(&sp8, &sp10, &arr[i].m34, 0x2C);
            fn_80329AEC(&sp8, &sp14, &sp10, 0xA);
            int kind;
            switch (*sp14) {
            case 'l':
                kind = 0;
                break;
            case 'p':
                kind = 1;
                break;
            default:
            case 'o':
                kind = 2;
                break;
            }
            arr[i].m38 = kind;
            i++;
        } while (i < *count);
    }
}

/* near-miss: retail loads both float temps before the first store */
void fn_8031AB8C(FxRec* r, int x, int y) {
    float fx = (float)x / (float)r->m28;
    float fy = (float)y / (float)r->m2C;
    if (r->m3C) {
        r->m3C[0] = fx;
        r->m3C[1] = fy;
    }
}

// fn_8031AC48
/* The name byte is read inside an inline: that read is not an unchanging MEM, so
 * sched keeps it after the int stores, as in retail. */
inline void FxRecName(FxRec* r, const char* s) { r->name[0] = s[0]; }
FxRec::FxRec() { // fn_8031AC48
    m28 = 0;
    m2C = 0;
    m38 = -1;
    m3C = 0;
    FxRecName(this, lbl_805E7988);
}

void fn_8031AC74() {}

void fn_8031B0E8(FxRec* self, int a) { self->m3C = (float*)a; }

void fn_8031B0F0(FxC* self, const char* name, int val) {
    for (int i = 0; i < self->m70; i++) {
        if (fn_8040B574((char*)self->m24 + i * 0x40, name) == 0) fn_8031B0E8((FxRec*)((char*)self->m24 + i * 0x40), val);
    }
}

/* near-miss: retail keeps the output cursor in a stack slot (address taken) */
int fn_8031B170(FxC* self, int** pp) {
    int cnt = 0;
    for (int k = 0; k < 5; k++) {
        if (self->a74[k].m10 == 0) cnt++;
    }
    int* p = *pp;
    *p = cnt;
    char* cur = (char*)(p + 1);
    for (int k = 0; k < 5; k++) {
        if (self->a74[k].m10 == 0) {
            *(FxItem*)cur = self->a74[k];
            cur += 0x14;
        }
    }
    *pp = (int*)cur;
    return 1;
}

void fn_8031B228(FxC* self, int i) {
    int off = i * 12 + 0x28;
    FxSlot* s = (FxSlot*)((char*)self + off);
    s->m0 = 0;
    s->m4 = 0;
    s->m8 = lbl_805EA564;
}

void fn_8031B250(FxC* self, int i, int a, float f, FxSrc* b) {
    int off = i * 12 + 0x28;
    FxSlot* s = (FxSlot*)((char*)self + off);
    s->m0 = a;
    s->m4 = b;
    s->m8 = f;
}

void fn_8031B27C(FxC* self, int i) {
    int off = i * 12 + 0x28;
    *(FxSlot*)((char*)self + off) = self->a28[1];
}

/* near-miss: slot address computed with add+displacement in retail, addi+lwzx here */
void fn_8031B2A8(FxC* self, int i) {
    FxSrc* src = self->a28[i].m4;
    for (int k = 0; k < 5; k++) {
        self->aD8[k].m0 = 0;
        self->aD8[k].m4 = lbl_805EA568;
        self->aD8[k].m10 = 1;
    }
    if (src) {
        int n = src->m0;
        if (n > 5) n = 5;
        for (int j = 0; j < n; j++) self->aD8[j] = (&src->m4)[j];
    }
}

extern const float lbl_805E9FBC;
inline FxVx Make80319F68(const FxVx& v) { FxVx r;r.x=v.x;r.y=v.y;r.z=v.z;r.w=v.w;return r; }
inline FxVx Convert80319F68(const FxV3& v) { FxVx t;t.x=v.x;t.y=v.y;t.z=v.z;t.w=lbl_805E9FBC;return Make80319F68(t); }
inline FxVx UV80319F68(const float& x,const float& y) { FxVx t;t.x=x;t.y=y;t.z=lbl_805E9FBC;t.w=lbl_805E9FBC;return Make80319F68(t); }
extern "C" void fn_80319F68(FxB* self,int texture,FxVx* rect,const FxV3* a,const FxV3* b,const FxV3* c,const FxV3* d,const int* color) {
 int rgba=*color;
 if(self->mF4<self->mF0) {
    rect->z+=rect->x;rect->w+=rect->y;
    FxQb* q=(FxQb*)&self->mF8[self->mF4];
    q->a8[0]=Convert80319F68(*a);
    q->a8[1]=Convert80319F68(*b);
    q->a8[2]=Convert80319F68(*c);
    q->a8[3]=Convert80319F68(*d);
    q->a48[0]=UV80319F68(rect->x,rect->y);
    q->a48[1]=UV80319F68(rect->z,rect->y);
    q->a48[2]=UV80319F68(rect->x,rect->w);
    q->a48[3]=UV80319F68(rect->z,rect->w);
    q->m0=texture;q->m4=rgba;q->m90=self->m28;self->mF4++;
 }
}
