/*
 * Front-end overlay code, .frontend_codeoverlay 0x804F81E0-0x804FE34C (code-only unit;
 * placeholder TU). Define every function FE_OVERLAY (game/overlay.h); data extern by lbl_ name.
 */

#include "game/fe_804E02E0.h"

/* Q16-DECL-BEGIN */
struct Unk8019F8C0_Owner;
struct FeRec388 {
    char pad0[0x388];
    int m388;
    int m38C;
};
FeRec388* fn_801A4C20(Unk8019F8C0_Owner* o);
struct Unk8007FF8C_G;
float fn_80080110(Unk8007FF8C_G* g, unsigned i, int x);
void fn_800800C8(Unk8007FF8C_G* g, unsigned i, float f, int x);
struct UiS;
float fn_800DC77C(UiS* w);
struct Unk802CA5A0_Elem;
Unk802CA5A0_Elem* fn_802CA5A0(Unk802CA5A0_Elem* t, int i);
extern int lbl_805FD3F4[];
extern "C" int sprintf(char* d, const char* fmt, ...);
extern "C" char* strcpy(char* d, const char* s);

extern "C" int fn_804D1278(void* self);
extern "C" void fn_80123610(FeObj* w);
extern "C" int fn_800B57EC();
#define FE_LO30(X)     X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8) X(9) X(10) X(11) X(12) X(13) X(14) X(15) X(16) X(17)     X(18) X(19) X(20) X(21) X(22) X(23) X(24) X(25) X(26) X(27) X(28) X(29) X(30)
class FeObjR31 : public UiBase {
public:
    FE_LO30(FE_V0) virtual const void* v31(); virtual FeObj* v32(const char* a, int b); FE_HI(FE_V0)
};
struct FeCap : FeObj {
    char pad40[0x134 - 0x40];
    FeObjI* m134;
    char pad138[0x364 - 0x138];
    int m364;
    int m368;
    char pad36C[0x378 - 0x36C];
    int m378;
    char pad37C[0x4F0 - 0x37C];
    float m4F0[0x29];
    Unk8019F8C0_Owner* m594;
    char pad598[0x618 - 0x598];
    FeObjI* m618;
    char pad61C[0x630 - 0x61C];
    int m630;
    char pad634[0x650 - 0x634];
    FeObjS* m650;
    FeObjR31* m654;
    FeObjR31* m658;
    char pad65C[0x680 - 0x65C];
    FeObj* m680;
    FeObj* m684;
    char pad688[0x7F4 - 0x688];
    UiS* m7F4[14];
    char pad82C[0x870 - 0x82C];
    int m870;
    char pad874[0x1100 - 0x874];
    FeObj* m1100;
};
extern "C" {
void fn_804FA1AC(FeCap* self, int mode, int x);
int fn_804F9B84(FeCap* self);
int fn_804FA320(void* self, int v);
int fn_804FA950(FeCap* self);
int fn_804F9B14(FeCap* self, int i, int n);
int fn_804FAA00(FeCap* self, int k, int m, float x);
float fn_804FAB88(void* self, float* a, int i);
float fn_804FA098(FeS23* self, int i, float x);
float fn_804FA104(FeS23* self, int i, int flag, float x);
float fn_804FA7BC(void* self, float x);
}
struct FeTabE {
    int k;
    int thr;
    int a;
    int b;
};
extern FeTabE lbl_80691FC4[];
struct FeCap3 : FeObj {
    char pad40[0x740 - 0x40];
    int m740[19];
};
extern "C" void fn_804FA374(FeCap* self, int i, int a, int mode, int x, int sel);
extern "C" int fn_804FA284(void* self, int v);
extern "C" int fn_80403060(const unsigned short* s);
extern "C" void fn_805283FC(void);
extern "C" int fn_804F9D10(FeCap* self, int a);
extern "C" void fn_804F83C8(FeCap* self);
void fn_800DC580(UiS* w, float f);
extern const int lbl_805FD394[];
extern char lbl_805FB378[];
extern char lbl_805FB380[];
extern char lbl_805FB388[];
extern char lbl_805FB390[];
extern float lbl_805FB394;
extern "C" int strcmp(const char* a, const char* b);
void fn_800E633C(char* s);
extern "C" void Unicode2Ascii(char* dst, const void* src);
extern "C" int fn_804F9C64(void* l, int k);
extern const int lbl_805FD3B4[];
extern const int lbl_805FD3D4[];
extern "C" {
void fn_8013D0E0(FeObjI* w, const void* s);
void fn_8013C0A8(FeObjI* w, int a, int b, int c);
}
void fn_802AEBCC(void* a, void* b);
struct FeBigS : FeObj {
    char pad40[0x10EC - 0x40];
    FeObj* m10EC;
    FeObj* m10F0;
    FeObj* m10F4;
    FeObj* m10F8;
    FeObj* m10FC;
    FeObj* m1100;
};
extern "C" void fn_8040074C(void* p, int n);
extern const char lbl_8064B298[];
extern const char lbl_8064A550[];
extern const char lbl_8064A3B8[];
struct FeAr {
    int* m0;
    int m4;
    int m8;
    int mC;
    const void* vp;
    int m14;
};

inline void FeArBase(FeAr* a, int* buf, int cap)
{
    a->vp = lbl_8064B298;
    a->m0 = buf;
    a->m8 = cap;
    fn_8040074C(a->m0, cap * 4);
}

inline void FeArMid(FeAr* a)
{
    a->m4 = 0;
    a->mC = 0;
    a->vp = lbl_8064A550;
    fn_8040074C(a->m0, a->m8 * 4);
}

inline void FeArD2(FeAr* a)
{
    a->vp = lbl_8064A3B8;
    a->m4 = 0;
    a->mC = 0;
    a->m14 = 0;
}

inline void FeArDtor(FeAr* a)
{
    a->vp = lbl_8064A550;
}

extern "C" void fn_80538368(FeAr* a, int v);
struct FeSq : FeObj {
    char pad40[0x740 - 0x40];
    FeObj* a[9];
    FeObj* p0;
    FeObj* b[9];
    FeObj* p1;
    char pad790[0x844 - 0x790];
    FeObj* m844;
    FeObj* m848;
};
extern "C" FeObj* fn_800C8034(void* p);
class FeObjQ28 : public UiBase {
public:
    virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10();
    virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19(); virtual void v20();
    virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24(); virtual void v25();
    virtual void v26(); virtual void v27(); virtual void v28(); virtual void v29(); virtual void v30();
    virtual void v31(); virtual FeObjQ28* v32(const char* a, int b); virtual void v33();
    virtual void v34(); virtual void v35(); virtual void v36(); virtual void v37(); virtual void v38();
    virtual void v39(); virtual void v40(); virtual void v41(); virtual void v42(); virtual void v43();
    virtual void v44(); virtual void v45(); virtual void v46(); virtual void v47(); virtual void v48();
    virtual void v49(); virtual void v50(); virtual void v51(); virtual void v52(); virtual void v53();
    virtual void v54(); virtual void v55(); virtual void v56(); virtual void v57(); virtual void v58();
    virtual int v59(); virtual void v60(); virtual void v61(); virtual void v62(); virtual void v63();
    virtual void v64(); virtual void v65(); virtual void v66(); virtual void v67(int a, int b);
    virtual void v68(int a); virtual void v69(); virtual void v70(); virtual void v71();
    virtual void v72(); virtual void v73(); virtual void v74(); virtual void v75(); virtual void v76();
    virtual void v77(); virtual void v78(); virtual void v79(); virtual void v80(); virtual void v81();
    virtual void v82(); virtual void v83(); virtual void v84(); virtual void v85(); virtual void v86();
    virtual void v87(); virtual void v88(); virtual void v89(); virtual void v90(); virtual void v91();
    virtual void v92(); virtual void v93(); virtual void v94(); virtual void v95(); virtual void v96();
    virtual void v97(); virtual void v98(); virtual void v99(); virtual void v100();
    virtual void v101(); virtual void v102(); virtual void v103(); virtual void v104();
    virtual void v105(); virtual void v106(); virtual void v107(); virtual void v108();
    virtual void v109(); virtual void v110(); virtual void v111(); virtual void v112();
    virtual void v113(); virtual void v114(); virtual void v115(); virtual void v116();
    virtual void v117(); virtual void v118(); virtual void v119(); virtual void v120();
    virtual void v121(); virtual void v122(); virtual void v123(); virtual void v124();
    virtual void v125(); virtual void v126(); virtual void v127();
};
class FeSubQ31 {
public:
    char pad0[8];
    unsigned a : 1;
    unsigned b : 1;
    unsigned c : 1;
    unsigned rest : 29;
    char padC[0x24 - 0xC];
    virtual int s1(); virtual int s2(); virtual int s3(); virtual int s4(); virtual int s5(); virtual void s6(int a);
    virtual int s7(); virtual int s8(); virtual void s9(int a, int b); virtual int s10(); virtual int s11(); virtual int s12();
    virtual int s13(); virtual int s14(); virtual int s15(); virtual int s16(); virtual int s17(); virtual int s18();
    virtual int s19(); virtual void s20(int a, int b); virtual int s21(); virtual void s22(int a);
};
struct FeS23y : FeObj {
    char pad40[0x134 - 0x40];
    FeObjQ28* m134;
    char pad138[0x10EC - 0x138];
    FeObjQ28* m10EC;
    char pad10F0[0x10F4 - 0x10F0];
    FeObj* m10F4;
    FeObj* m10F8;
    FeObj* m10FC;
};
struct FeStQ32 {
    char pad0[0x4F8];
    int m4F8;
    int m4FC;
};
struct FeStQ33 {
    char pad0[0x608];
    int m608;
};
extern const char lbl_805FA3B8[];
extern "C" void fn_804D11BC();
extern "C" int fn_800B5CBC();
class FeObjQ36 : public UiBase {
public:
    virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10();
    virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19(); virtual void v20();
    virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24(); virtual void v25();
    virtual void v26(); virtual void v27(); virtual void v28(); virtual void v29(); virtual void v30();
    virtual void v31(); virtual void v32(); virtual void v33(); virtual void v34(); virtual void v35();
    virtual void v36(); virtual void v37(); virtual void v38(); virtual void v39(); virtual void v40();
    virtual void v41(); virtual void v42(); virtual void v43(); virtual void v44(); virtual void v45();
    virtual void v46(); virtual void v47(); virtual void v48(); virtual void v49(); virtual void v50();
    virtual void v51(int a, int b); virtual void v52(); virtual void v53(); virtual void v54();
    virtual void v55(); virtual void v56(); virtual void v57(); virtual void v58(); virtual void v59();
    virtual void v60(); virtual void v61(); virtual void v62(); virtual void v63(); virtual void v64();
    virtual void v65(); virtual void v66(); virtual void v67(); virtual void v68(); virtual void v69();
    virtual void v70(); virtual void v71(); virtual void v72(); virtual void v73(); virtual void v74();
    virtual void v75(); virtual void v76(); virtual void v77(); virtual void v78(); virtual void v79();
    virtual void v80(); virtual void v81(); virtual void v82(); virtual void v83(); virtual void v84();
    virtual void v85(); virtual void v86(); virtual void v87(); virtual void v88(); virtual void v89();
    virtual void v90(); virtual void v91(); virtual void v92(); virtual void v93(); virtual void v94();
    virtual void v95(); virtual void v96(); virtual void v97(); virtual void v98(); virtual void v99();
    virtual void v100(); virtual void v101(); virtual void v102(); virtual void v103();
    virtual void v104(); virtual void v105(); virtual void v106(); virtual void v107();
    virtual void v108(); virtual void v109(); virtual void v110(); virtual void v111();
    virtual void v112(); virtual void v113(); virtual void v114(); virtual void v115();
    virtual void v116(); virtual void v117(); virtual void v118(); virtual void v119();
    virtual void v120(); virtual void v121(); virtual void v122(); virtual void v123();
    virtual void v124(); virtual void v125(); virtual void v126(); virtual void v127();
};
class FeSubQ37 {
public:
    char pad0[0x24];
    virtual int s1(); virtual int s2(); virtual int s3(); virtual int s4(); virtual int s5(); virtual int s6();
    virtual int s7(); virtual void s8(int a); virtual int s9(); virtual int s10(); virtual int s11(); virtual int s12();
    virtual int s13(); virtual int s14(); virtual void s15();
};
struct FeS23z : FeObj {
    char pad40[0x630 - 0x40];
    int m630;
    char pad634[0x668 - 0x634];
    FeObjQ36* m668;
    char pad66C[0x86C - 0x66C];
    int m86C;
};
extern "C" int fn_80077BC4(void* g, int a);
class FeObjQ38 : public UiBase {
public:
    virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10();
    virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(int a); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24();
    virtual void v25(); virtual void v26(); virtual void v27(); virtual void v28(); virtual void v29();
    virtual void v30(); virtual void v31(); virtual FeObjQ38* v32(const char* a, int b);
    virtual void v33(); virtual void v34(); virtual void v35(); virtual void v36(); virtual void v37();
    virtual void v38(); virtual void v39(); virtual void v40(); virtual void v41(); virtual void v42();
    virtual void v43(); virtual void v44(); virtual void v45(); virtual void v46(); virtual void v47();
    virtual void v48(); virtual void v49(); virtual void v50(); virtual void v51(int a, int b);
    virtual void v52(); virtual void v53(); virtual void v54(); virtual void v55(); virtual void v56();
    virtual void v57(); virtual void v58(); virtual void v59(); virtual void v60(); virtual void v61();
    virtual void v62(); virtual void v63(); virtual void v64(); virtual void v65(); virtual void v66();
    virtual void v67(); virtual void v68(); virtual void v69(); virtual void v70(); virtual void v71();
    virtual void v72(); virtual void v73(); virtual void v74(); virtual void v75(); virtual void v76();
    virtual void v77(); virtual void v78(); virtual void v79(); virtual void v80(); virtual void v81();
    virtual void v82(); virtual void v83(); virtual void v84(); virtual void v85(); virtual void v86();
    virtual void v87(); virtual void v88(); virtual void v89(); virtual void v90(); virtual void v91();
    virtual void v92(); virtual void v93(); virtual void v94(); virtual void v95(); virtual void v96();
    virtual void v97(); virtual void v98(); virtual void v99(); virtual void v100();
    virtual void v101(); virtual void v102(); virtual void v103(); virtual void v104();
    virtual void v105(); virtual void v106(); virtual void v107(); virtual void v108();
    virtual void v109(); virtual void v110(); virtual void v111(); virtual void v112();
    virtual void v113(); virtual void v114(); virtual void v115(); virtual void v116();
    virtual void v117(); virtual void v118(); virtual void v119(); virtual void v120();
    virtual void v121(); virtual void v122(); virtual void v123(); virtual void v124();
    virtual void v125(); virtual void v126(); virtual void v127();
};
struct FeS23w : FeObj {
    char pad40[0x66C - 0x40];
    FeObjQ38* m66C;
    char pad670[0x82C - 0x670];
    FeObjQ38* m82C;
};
extern const char lbl_805FB3BC[];
extern float lbl_805FB3A8;
extern "C" void fn_804FA8F0(FeCap* self, int i, float x);
class FeSubQ44 {
public:
    char pad0[0x24];
    virtual int s1(); virtual int s2(); virtual int s3(); virtual int s4(); virtual int s5(); virtual void s6(int a);
};
struct FeS27z : FeObj {
    char pad40[0x740 - 0x40];
    int mA[9];
    FeObj* m764;
    int mB[9];
    FeObj* m78C;
};
extern const char lbl_805FB0A0[];
extern const char lbl_805FB0AC[];
extern const char lbl_805FB0B4[];
extern const char lbl_805FB0C0[];
extern const char lbl_805FB0C8[];
extern const char lbl_805FB0D0[];
extern const char lbl_805FB0D8[];
extern const char lbl_805FB0E4[];
extern const char lbl_805FB0EC[];
extern const char lbl_805FB098[];
class FeObjQ45 : public UiBase {
public:
    virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10();
    virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(int a); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24();
    virtual void v25(); virtual void v26(); virtual void v27(); virtual void v28(); virtual void v29();
    virtual void v30(); virtual void v31(); virtual FeObj* v32(const char* a, int b);
    virtual void v33(); virtual void v34(); virtual void v35(); virtual void v36(); virtual void v37();
    virtual void v38(); virtual void v39(); virtual void v40(); virtual void v41(); virtual void v42();
    virtual void v43(); virtual void v44(); virtual void v45(); virtual void v46(); virtual void v47();
    virtual void v48(); virtual void v49(); virtual void v50(); virtual void v51(); virtual void v52();
    virtual void v53(); virtual void v54(); virtual void v55(); virtual void v56(); virtual void v57();
    virtual void v58(); virtual int v59(); virtual int v60(); virtual void v61(); virtual void v62();
    virtual void v63(); virtual void v64(); virtual void v65(); virtual void v66(); virtual void v67();
    virtual void v68(); virtual void v69(); virtual void v70(); virtual void v71(); virtual void v72();
    virtual FeObjQ45* v73(int a, int b); virtual void v74(); virtual void v75(); virtual void v76();
    virtual void v77(); virtual void v78(); virtual void v79(); virtual void v80(); virtual void v81();
    virtual void v82(); virtual void v83(); virtual void v84(); virtual void v85(); virtual void v86();
    virtual void v87(); virtual void v88(); virtual void v89(); virtual void v90(); virtual void v91();
    virtual void v92(); virtual void v93(); virtual void v94(); virtual void v95(); virtual void v96();
    virtual void v97(); virtual void v98(); virtual void v99(); virtual void v100();
    virtual void v101(); virtual void v102(); virtual void v103(); virtual void v104();
    virtual void v105(); virtual void v106(); virtual void v107(); virtual void v108();
    virtual void v109(); virtual void v110(); virtual void v111(); virtual void v112();
    virtual void v113(); virtual void v114(); virtual void v115(); virtual void v116();
    virtual void v117(); virtual void v118(); virtual void v119(); virtual void v120();
    virtual void v121(); virtual void v122(); virtual void v123(); virtual void v124();
    virtual void v125(); virtual void v126(); virtual void v127();
};
class FeSubQ45 {
public:
    char pad0[0x24];
    virtual int s1(); virtual int s2(); virtual int s3(); virtual int s4(); virtual int s5(); virtual void s6(int a);
    virtual int s7(); virtual int s8(); virtual int s9(); virtual int s10(); virtual int s11();
};
struct FeFlag24 {
    char pad0[0x24];
    unsigned f;
};
struct FeW2E0 {
    char pad0[0x2E0];
    int m2E0;
};
struct FeS29 : FeObj {
    char pad40[0x134 - 0x40];
    FeObjI* m134;
    char pad138[0x618 - 0x138];
    FeObjQ45* m618;
    int m61C;
    char pad620[0x634 - 0x620];
    int m634;
    int m638;
    char pad63C[0x654 - 0x63C];
    FeFlag24* m654;
    FeFlag24* m658;
    char pad65C[0x668 - 0x65C];
    FeObj* m668;
    char pad66C[0x684 - 0x66C];
    FeObj* m684;
    char pad688[0x7F4 - 0x688];
    FeObjQ45* a[14];
    char pad82C[0x834 - 0x82C];
    FeObjQ45* m834;
    char pad838[0x860 - 0x838];
    int m860;
};
extern const char lbl_805FB2F4[];
extern "C" void fn_804F33C0(void* self, int a);
extern "C" void fn_8013D164(void* w, int v);
struct FeS30 : FeObj {
    char pad40[0x7B8 - 0x40];
    FeObj* m7B8;
    char pad7BC[0x7CC - 0x7BC];
    FeObj* m7CC;
    char pad7D0[0x7E0 - 0x7D0];
    FeObj* m7E0;
};
extern const char lbl_805FB46C[];
extern const char lbl_805FB47C[];
extern const char lbl_805FB48C[];
extern const char lbl_805FB49C[];
extern const char lbl_805FB4AC[];
extern const char lbl_805FB4BC[];
extern const char lbl_805FB4CC[];
extern const char lbl_805FB4DC[];
extern const char lbl_805FB4EC[];
extern const char lbl_805FB4FC[];
extern const char lbl_805FB50C[];
extern const char lbl_805FB51C[];
/* Q16-DECL-END */

extern "C" int fn_804FB7F4(FeS19* s);
extern "C" int fn_804FB84C(FeS19* s);

extern "C" FE_OVERLAY void fn_804F81E0(const unsigned short* s)
{
    FeObj* p = fn_800B7A94(UiGetMsgMgr(), "CREATEAPLAYER");
    ((FeObj1*)p)->v16(1);
    ((FeObj1*)p)->v8(1);
    if (fn_80403060(s) > 0) {
        FeObj* q = fn_800C8040(p->v32("FNAMETB", 1));
        ((FeObj1*)q)->v28((int)s);
        fn_804F83C8((FeCap*)p);
    }
}

extern "C" FE_OVERLAY void fn_804F82D4(const unsigned short* s)
{
    FeObj* p = fn_800B7A94(UiGetMsgMgr(), "CREATEAPLAYER");
    ((FeObj1*)p)->v16(1);
    ((FeObj1*)p)->v8(1);
    if (fn_80403060(s) > 0) {
        FeObj* q = fn_800C8040(p->v32("LNAMETB", 1));
        ((FeObj1*)q)->v28((int)s);
        fn_804F83C8((FeCap*)p);
    }
}

extern "C" FE_OVERLAY void fn_804F83C8(FeCap* self)
{
    if (self->m630 == 1 && fn_804F9D10(self, 0) != 0) {
        FeObj* p = fn_800B7A94(UiGetMsgMgr(), "POPUP");
        fn_80131F34(p, self, (FeCb)fn_805283FC, 0, 0x3A8E, 1, -1, 1, 0x26DE, 0, 0, 0, 0, 0);
        p->v86();
    }
}

extern "C" FE_OVERLAY void fn_804F84A0(void)
{
    FeObj* p = fn_800B7A94(UiGetMsgMgr(), "CREATEAPLAYER");
    ((FeObj1*)p)->v8(1);
    ((FeObj1*)p)->v16(1);
}

extern "C" FE_OVERLAY void fn_804F8534(void* p)
{
    FeCap* self = (FeCap*)p;
    if (self->m618 != 0) {
        int mode = 1;
        ((FeObj1*)self)->v16(0);
        if (self->m618->v60() == 0) {
            FeObjI* w = self->m134;
            fn_8013D0E0(w, ((FeObjR31*)fn_800C8040(self->v32("FNAMETB", 1)))->v31());
        } else {
            mode = 2;
            FeObjI* w = self->m134;
            fn_8013D0E0(w, ((FeObjR31*)fn_800C8040(self->v32("LNAMETB", 1)))->v31());
        }
        fn_8013C0A8(self->m134, 1, 1, mode);
    }
}

/* near-miss: loop bound compiles to cmpwi 105 / bgt where retail has cmpwi 106 / bge (2 slots differ) */
/* near-miss: loop bound compiles to cmpwi 105 / bgt where retail has cmpwi 106 / bge (2 slots differ) */
/* near-miss: loop bound compiles to cmpwi 105 / bgt where retail has cmpwi 106 / bge (2 slots differ) */
extern "C" FE_OVERLAY int fn_804F8668(void* self, int key, int thr, int* out)
{
    int i;
    int best = 0;
    for (i = 0; i < 0x6A && lbl_80691FC4[i].k != key; i++)
        ;
    if (i != 0x6A) {
        best = i;
        while (lbl_80691FC4[i].k == key) {
            if (lbl_80691FC4[i].thr <= thr) {
                best = i;
                i = best + 1;
            } else
                i++;
        }
    }
    *out = lbl_80691FC4[best].b;
    return lbl_80691FC4[best].a;
}

extern "C" FE_OVERLAY void fn_804F872C(FeCap* self)
{
    FeRec388* o = fn_801A4C20(self->m594);
    o->m388 = self->m368;
    o->m38C = self->m364;
}

extern "C" FE_OVERLAY void fn_804F876C(void* self, char* out)
{
    out[0] = 0;
    if (fn_800B555C()->m0 == 0x2E)
        strcpy(out, "DONEEDIT");
    else
        strcpy(out, "DONECREATE");
}

extern "C" FE_OVERLAY int fn_804F87F8(FeS21* s)
{
    if (s->m4A0 == s->m4B4 || s->m4A0 == s->m4C8 || s->m4A0 == s->m4DC)
        return 1;
    if (s->m4B4 == s->m4C8 || s->m4B4 == s->m4DC) {
        if (s->m4B4 != 0)
            return 1;
    }
    if (s->m4C8 == s->m4DC) {
        if (s->m4C8 != 0)
            return 1;
    }
    return 0;
}

extern "C" FE_OVERLAY int fn_804F885C(FeS22* self)
{
    if (self->m868 != lbl_805FD1A0[self->m3B4])
        return 1;
    if (self->m630 == 1 || fn_804FB7F4((FeS19*)self) != 0 || fn_804FB84C((FeS19*)self) != 0) {
        if (self->m864 != lbl_805FD2A8[self->m484])
            return 1;
        return 0;
    }
    return 0;
}

extern "C" FE_OVERLAY void fn_804F8900(FeS22* self)
{
    void* g = lbl_8061E978;
    char buf[64];
    if (self->m630 == 1 || fn_804FB7F4((FeS19*)self) != 0 || fn_804FB84C((FeS19*)self) != 0) {
        if (self->m864 != lbl_805FD2A8[self->m484]) {
            if (self->m864 >= 0) {
                sprintf(buf, "SIGP%04d", self->m864);
                fn_802AEBF0(g, buf);
            }
            self->m864 = lbl_805FD2A8[self->m484];
            if (self->m864 >= 0) {
                sprintf(buf, "SIGP%04d", self->m864);
                fn_802AEBCC(g, buf);
            }
        }
    }
    if (self->m868 != lbl_805FD1A0[self->m3B4]) {
        if (self->m868 >= 0) {
            sprintf(buf, "SIGB%04d", self->m868);
            fn_802AEBF0(g, buf);
        }
        self->m868 = lbl_805FD1A0[self->m3B4];
        if (self->m868 >= 0) {
            sprintf(buf, "SIGB%04d", self->m868);
            fn_802AEBCC(g, buf);
        }
    }
}

extern "C" FE_OVERLAY void fn_804F8A58(FeS22* self)
{
    void* g = lbl_8061E978;
    char buf[64];
    if (self->m864 >= 0) {
        fn_804089B4(buf, "SIGP%04d", self->m864);
        fn_802AEBF0(g, buf);
    }
    if (self->m868 >= 0) {
        fn_804089B4(buf, "SIGB%04d", self->m868);
        fn_802AEBF0(g, buf);
    }
}

extern "C" FE_OVERLAY void fn_804F8AE0(FeS22* self, int a)
{
    fn_800ED310(self, a);
    int k = ((FeSub*)((char*)fn_800C8058(((FeObjQ*)self->m604)->v33(10, 1)) + 0xDC))->s5();
    FeObj1* w = (FeObj1*)self->m82C[k];
    if (w != 0)
        w->v14(a);
}

extern "C" FE_OVERLAY int fn_804F8B84(FeS18* s, int* out)
{
    for (int i = 0; i < 25; i++) {
        if (s->e[i].a != 0) {
            *out = i;
            return 1;
        }
    }
    *out = 0;
    return 0;
}

extern "C" FE_OVERLAY int fn_804F9AAC(void* self, UiItemN* it)
{
    char* n = it->name;
    if (fn_8040B574(n, "FNAMETB") == 0 || fn_8040B574(n, "LNAMETB") == 0)
        fn_804F8534(self);
    return 0;
}

extern "C" FE_OVERLAY int fn_804F9B14(FeCap* self, int i, int n)
{
    int a[3] = { 110, 110, 140 };
    int b[3] = { 7, 7, 7 };
    int m = b[i];
    int r = a[i];
    int d = n - 0x42;
    if (d > 0)
        r += m * d;
    return r;
}

extern "C" FE_OVERLAY int fn_804F9B84(FeCap* self)
{
    int r;
    if (self->m630 == 1) {
        int a = ((FeSub*)((char*)self->m680 + 0xDC))->s11();
        int b = ((FeSub*)((char*)self->m684 + 0xDC))->s5();
        r = fn_804F9B14(self, b, a);
        r += fn_804FA950(self);
        self->m378 = r;
    } else
        r = self->m378;
    char buf[100];
    sprintf(buf, "%d", r);
    self->m650->v30(buf);
    return r;
}

extern "C" FE_OVERLAY int fn_804F9C64(void* l, int k)
{
    int r = 12;
    switch (k) {
    case 4:
        r = 12;
        break;
    case 5:
        r = 7;
        break;
    case 6:
        r = 13;
        break;
    case 7:
        r = 8;
        break;
    case 8:
        r = 16;
        break;
    case 9:
        r = 14;
        break;
    case 10:
        r = 17;
        break;
    case 11:
        r = 15;
        break;
    case 12:
        r = 21;
        break;
    case 13:
        r = 22;
        break;
    }
    return r;
}

/* near-miss: 99.3%; only self and the loop counter swap callee-saved registers (retail self=r29, i=r28). Declaration-order, loop-shape and ternary variants tried */
extern "C" FE_OVERLAY int fn_804F9D10(FeCap* self, int arg)
{
    float v;
    float k;
    char a[16];
    char b[16];
    const char* s1;
    const char* s2;
    const char* s3;
    const char* s4;
    if (self->m870 == 0) {
        if (fn_800B555C()->m0 == 0x2E && arg != 0) {
            Unk8007A4DC_Screen* sc = (Unk8007A4DC_Screen*)lbl_8061930C;
            int id = sc->fn_8007A6A0(0x31, arg);
            k = lbl_805FB394;
            for (int i = 2; i <= 0xD; i++) {
                int n;
                if ((unsigned)(i - 2) <= 1)
                    n = 10;
                else
                    n = fn_804F9C64(self, i);
                int idx = fn_804FA284(self, i);
                v = fn_80080110((Unk8007FF8C_G*)sc, id, n);
                if (v < lbl_80622220[idx] - lbl_805FB394 || v > lbl_80622220[idx] + lbl_80622240[idx] + lbl_805FB394)
                    goto found;
            }
        }
        s1 = lbl_805FB378;
        s2 = lbl_805FB380;
        s3 = lbl_805FB388;
        s4 = lbl_805FB390;
        Unicode2Ascii(a, self->m654->v31());
        Unicode2Ascii(b, self->m658->v31());
        fn_800E633C(a);
        fn_800E633C(b);
        if ((strcmp(a, s1) == 0 || strcmp(a, s2) == 0 || strcmp(a, s3) == 0) && strcmp(b, s4) == 0) {
        found:
            self->m870 = 1;
            fn_804F9B84(self);
            return 1;
        }
    }
    return 0;
}

extern "C" FE_OVERLAY void fn_804F9F14(FeCap* self, int x)
{
    fn_804FA1AC(self, 2, x);
    Unk8007FF8C_G* g = (Unk8007FF8C_G*)lbl_8061930C;
    int t = fn_804FA320(self, x);
    for (int i = 0; i <= 0x28; i++)
        self->m4F0[i] = fn_80080110(g, t, i);
    fn_804F9B84(self);
}

extern "C" FE_OVERLAY void fn_804F9F9C(FeCap* self, int b, int c)
{
    if (b != 0)
        fn_804FA1AC(self, 1, c);
    else
        fn_804FA1AC(self, 0, 0);
}

/* near-miss: retail keeps i in r4 and scales it per branch (no mr r10,r4); ours reuses r4 as a table pointer (6 slots differ) */
extern "C" FE_OVERLAY float fn_804FA098(FeS23* self, int i, float x)
{
    float r = 0.0f;
    if (self->m870 != 0) {
        r = lbl_80622260[i] + lbl_80622280[i] * x;
    } else {
        r = lbl_80622220[i] + lbl_80622240[i] * x;
    }
    return r;
}

/* near-miss: retail loads a[i] inside each branch and b[i] after the join; same register-reuse problem as fn_804FA098 */
extern "C" FE_OVERLAY float fn_804FA104(FeS23* self, int i, int flag, float x) { float r=0.0f; if (self->m870 != 0 && flag == 0) r=(x-lbl_80622260[i])/lbl_80622280[i]; else r=(x-lbl_80622220[i])/lbl_80622240[i]; if(r<0.0f) r=0.0f; else if(!(r<=1.0f)) r=1.0f; return r; }


extern "C" FE_OVERLAY void fn_804FA1AC(FeCap* self, int mode, int x)
{
    int sel = -1;
    if (x != 0) {
        sel = fn_804FA320(self, x);
        if (mode == 1)
            ((Unk8007A4DC_Screen*)lbl_8061930C)->fn_8007A808(0x31, x, sel);
    } else if (mode == 3) {
        int k = ((FeSub*)((char*)self->m684 + 0xDC))->s5();
        sel = lbl_805FD3F4[k];
    }
    for (int i = 0; i <= 0xD; i++)
        fn_804FA374(self, i, fn_804FA284(self, i), mode, x, sel);
}

extern "C" FE_OVERLAY int fn_804FA284(void* self, int v)
{
    switch (v) {
    case 0:
        return 0;
    case 1:
        return 1;
    case 2:
    case 3:
        return 2;
    case 4:
    case 5:
        return 3;
    case 6:
    case 7:
        return 4;
    case 8:
    case 9:
        return 5;
    case 10:
    case 11:
        return 6;
    case 12:
    case 13:
        return 7;
    }
    return 7;
}

extern "C" FE_OVERLAY int fn_804FA320(void* self, int v)
{
    for (int i = 0; i <= 0x18; i++) {
        if (v == fn_800765A8(i))
            return i;
    }
    return -1;
}

extern "C" FE_OVERLAY void fn_804FA374(FeCap* self, int i, int a, int mode, int x, int sel)
{
    float f = fn_800DC77C(self->m7F4[i]);
    int c1 = !(mode ^ 1);
    Unk8007FF8C_G* g = (Unk8007FF8C_G*)lbl_8061930C;
    float v = fn_804FA098((FeS23*)self, a, f);
    int c0 = !mode;
    float t;
    int n = 9;
    int gen = 1;
    switch (i) {
    case 0:
        gen = 0;
        if (c1) {
            fn_800800C8(g, sel, v, 0x12);
            fn_800800C8(g, sel, v, 0x2);
            fn_800800C8(g, sel, v, 0x3);
            fn_800800C8(g, sel, v, 0x4);
            fn_800800C8(g, sel, v, 0x9);
        } else if (c0) {
            self->m4F0[0x12] = v;
            self->m4F0[0x2] = v;
            self->m4F0[0x3] = v;
            self->m4F0[0x4] = v;
            self->m4F0[0x9] = v;
        } else {
            t = lbl_805FB3A8;
            if (mode == 3)
                t = fn_804FAB88(self, (float*)fn_802CA5A0((Unk802CA5A0_Elem*)((char*)lbl_8061E978 + 0x125E0), sel), 0x9);
            else
                t = fn_80080110(g, sel, 0x9);
            fn_804FA8F0(self, i, t);
        }
        break;
    case 1:
        gen = 0;
        if (c1) {
            fn_800800C8(g, sel, v, 0x0);
            fn_800800C8(g, sel, v, 0x1);
        } else if (c0) {
            self->m4F0[0x0] = v;
            self->m4F0[0x1] = v;
        } else {
            t = lbl_805FB3A8;
            if (mode == 3)
                t = fn_804FAB88(self, (float*)fn_802CA5A0((Unk802CA5A0_Elem*)((char*)lbl_8061E978 + 0x125E0), sel), 0x0);
            else
                t = fn_80080110(g, sel, 0x0);
            fn_804FA8F0(self, i, t);
        }
        break;
    case 2:
        gen = 0;
        if (c1) {
            fn_800800C8(g, sel, v, 0xA);
            fn_800800C8(g, sel, v, 0xB);
        } else if (c0) {
            self->m4F0[0xA] = v;
            self->m4F0[0xB] = v;
        } else {
            t = lbl_805FB3A8;
            if (mode == 3)
                t = fn_804FAB88(self, (float*)fn_802CA5A0((Unk802CA5A0_Elem*)((char*)lbl_8061E978 + 0x125E0), sel), 0xA);
            else
                t = fn_80080110(g, sel, 0xA);
            fn_804FA8F0(self, i, t);
        }
        break;
    case 3:
        gen = 0;
        if (c1) {
            fn_800800C8(g, sel, v, 0x5);
            fn_800800C8(g, sel, v, 0x6);
        } else if (c0) {
            self->m4F0[0x5] = v;
            self->m4F0[0x6] = v;
        } else {
            t = lbl_805FB3A8;
            if (mode == 3)
                t = fn_804FAB88(self, (float*)fn_802CA5A0((Unk802CA5A0_Elem*)((char*)lbl_8061E978 + 0x125E0), sel), 0x5);
            else
                t = fn_80080110(g, sel, 0x5);
            fn_804FA8F0(self, i, t);
        }
        break;
    default:
        n = fn_804F9C64(self, i);
        break;
    }
    if (gen != 0) {
        if (c1) {
            fn_800800C8(g, sel, v, n);
        } else if (c0) {
            self->m4F0[n] = v;
        } else {
            t = lbl_805FB3A8;
            if (mode == 3)
                t = fn_804FAB88(self, (float*)fn_802CA5A0((Unk802CA5A0_Elem*)((char*)lbl_8061E978 + 0x125E0), sel), n);
            else
                t = fn_80080110(g, sel, n);
            fn_804FA8F0(self, i, t);
        }
    }
}

extern "C" FE_OVERLAY float fn_804FA7BC(void* self, float x)
{
    Unk8007FF8C_G* g = (Unk8007FF8C_G*)lbl_8061930C;
    float a = fn_80080110(g, 0x18, 0xD);
    fn_800800C8(g, 0x18, x, 0xD);
    float b = fn_80080110(g, 0x18, 0xD);
    fn_800800C8(g, 0x18, a, 0xD);
    return b;
}

extern "C" FE_OVERLAY float fn_804FA840(FeS23* self, int i, float x)
{
    float t = fn_804FA098(self, i, x);
    if (!(t >= lbl_80622220[i]))
        t = lbl_80622220[i];
    else if (!(t <= lbl_80622220[i] + lbl_80622240[i]))
        t = lbl_80622220[i] + lbl_80622240[i];
    t = fn_804FA7BC(self, t);
    float r = fn_804FA104(self, i, 1, t);
    return r;
}

extern "C" FE_OVERLAY void fn_804FA8F0(FeCap* self, int i, float x)
{
    float t = fn_804FA104((FeS23*)self, fn_804FA284(self, i), 0, x);
    fn_800DC580(self->m7F4[i], t);
}

extern "C" FE_OVERLAY int fn_804FA950(FeCap* self)
{
    int sum = 0;
    int i = 0;
    float x = 0.0f;
    int m = ((FeSub*)((char*)self->m684 + 0xDC))->s5();
    do {
        int k = fn_804FA284(self, i);
        x = fn_800DC77C(self->m7F4[i]);
        i++;
        sum += fn_804FAA00(self, k, m, x);
    } while (i <= 0xD);
    return sum;
}

extern "C" FE_OVERLAY int fn_804FAA00(FeCap* self, int i, int sel, float x)
{
    float t = fn_804FA840((FeS23*)self, i, x);
    int v = 0;
    switch (sel) {
    case 0:
        v = lbl_805FD394[i];
        break;
    case 1:
        v = lbl_805FD3B4[i];
        break;
    case 2:
        v = lbl_805FD3D4[i];
        break;
    }
    return (int)((float)v * t);
}

extern "C" FE_OVERLAY void fn_804FAAD0(FeCap* self)
{
    int k = ((FeSub*)((char*)self->m684 + 0xDC))->s5();
    float* e = (float*)fn_802CA5A0((Unk802CA5A0_Elem*)((char*)lbl_8061E978 + 0x125E0), lbl_805FD3F4[k]);
    fn_804FA1AC(self, 3, 0);
    for (int i = 0; i <= 0x28; i++)
        self->m4F0[i] = fn_804FAB88(self, e, i);
    fn_804F9B84(self);
}

/* near-miss: retail forms the byte offsets i*8 / (i*8|4) with lfsx and loads 0.5f from a global before the sum (14 slots differ) */
extern "C" FE_OVERLAY float fn_804FAB88(void* self, float* a, int i)
{
    float x = a[i * 2];
    float y = a[i * 2 + 1];
    return (x + y) * 0.5f;
}

extern "C" FE_OVERLAY void fn_804FABC8(FeS23w* self, int kind, int arg2)
{
    ((FeSubQ37*)((char*)self->m66C + 0xDC))->s15();
    if (kind == 0 || kind == 0xA) {
        arg2 = 0;
        if (kind == 0)
            arg2 = 0xA;
        self->m66C->v51(0x2607, arg2);
        self->m82C->v32(lbl_805FB3BC, 1)->v16(0);
    } else {
        self->m82C->v32(lbl_805FB3BC, 1)->v16(1);
        if (arg2 == 0 || arg2 == 0xA)
            arg2 = kind;
        self->m66C->v51(0x2607, kind);
        if (kind != 1)
            self->m66C->v51(0x25BF, 1);
        if (kind != 2)
            self->m66C->v51(0x25B8, 2);
        if (kind != 3)
            self->m66C->v51(0x25B9, 3);
        if (kind != 4)
            self->m66C->v51(0x25BB, 4);
        if (kind != 5)
            self->m66C->v51(0x25BA, 5);
        if (kind != 6)
            self->m66C->v51(0x25BC, 6);
        if (kind != 7)
            self->m66C->v51(0x25BD, 7);
        if (kind != 8)
            self->m66C->v51(0x25BE, 8);
        if (kind != 11)
            self->m66C->v51(0x25CA, 11);
        if (kind != 12)
            self->m66C->v51(0x25C8, 12);
        if (kind != 13)
            self->m66C->v51(0x25CB, 13);
    }
    ((FeSubQ37*)((char*)self->m66C + 0xDC))->s8(arg2);
}

extern "C" FE_OVERLAY void fn_804FAF24(FeS23z* self, int arg)
{
    ((FeSubQ37*)((char*)self->m668 + 0xDC))->s15();
    if (self->m630 != 1 || (self->m86C != 0 && fn_80077BC4(lbl_8061930C, self->m86C) != 0)) {
        int sv = ((FeStQ32*)fn_800B555C())->m4FC;
        if (((Unk8007A4DC_Screen*)lbl_8061930C)->fn_8007A270(sv) != 0) {
            self->m668->v51(0x25C1, 0);
            self->m668->v51(0x3840, 0xA);
        } else {
            self->m668->v51(0x25BF, 1);
            self->m668->v51(0x25B8, 2);
            self->m668->v51(0x25B9, 3);
            self->m668->v51(0x25BB, 4);
            self->m668->v51(0x25BA, 5);
            self->m668->v51(0x25BC, 6);
            self->m668->v51(0x25BD, 7);
            self->m668->v51(0x25BE, 8);
        }
    } else {
        self->m668->v51(0x25C1, 0);
        self->m668->v51(0x25BF, 1);
        self->m668->v51(0x25B8, 2);
        self->m668->v51(0x25B9, 3);
        self->m668->v51(0x25BB, 4);
        self->m668->v51(0x25BA, 5);
        self->m668->v51(0x25BC, 6);
        self->m668->v51(0x25BD, 7);
        self->m668->v51(0x25BE, 8);
        self->m668->v51(0x3840, 0xA);
    }
    ((FeSubQ37*)((char*)self->m668 + 0xDC))->s8(arg);
}

extern "C" FE_OVERLAY void fn_804FB29C(FeCap* self_, int mode)
{
    FeS29* self = (FeS29*)self_;
    int a = 0;
    int b = 0;
    int c = 9;
    int s = self->m834->v60();
    switch (s) {
    case 1:
        a = 1;
        c = 0;
        break;
    case 2:
        a = 2;
        b = 3;
        c = 0xA;
        break;
    case 3:
        a = 4;
        b = 5;
        c = 0xC;
        break;
    case 4:
        a = 6;
        b = 7;
        c = 0xD;
        break;
    case 5:
        a = 8;
        b = 9;
        c = 0x10;
        break;
    case 6:
        a = 0xA;
        b = 0xB;
        c = 0x11;
        break;
    case 7:
        a = 0xC;
        b = 0xD;
        c = 0x15;
        break;
    }
    switch (mode) {
    case 0: {
        self->m860 = (self->m860 + 1) % 3;
        int n = self->m834->v59();
        for (int i = 2; i < n; i++) {
            ((FeSubQ45*)((char*)fn_800C8058(self->m834->v73(i, 0)->v32(lbl_805FB2F4, 1)) + 0xDC))->s6(self->m860);
            switch (i) {
            case 0:
                a = 0;
                break;
            case 1:
                a = 1;
                break;
            case 2:
                a = 2;
                b = 3;
                break;
            case 3:
                a = 4;
                b = 5;
                break;
            case 4:
                a = 6;
                b = 7;
                break;
            case 5:
                a = 8;
                b = 9;
                break;
            case 6:
                a = 0xA;
                b = 0xB;
                break;
            case 7:
                a = 0xC;
                b = 0xD;
                break;
            }
            if (self->m860 == 2) {
                self->a[a]->v16(0);
                self->a[b]->v16(1);
            } else if (self->m860 == 1) {
                self->a[a]->v16(1);
                self->a[b]->v16(0);
            } else {
                self->a[a]->v16(1);
                self->a[b]->v16(1);
            }
        }
        break;
    }
    case 1: {
        int k = ((FeSubQ45*)((char*)self->m684 + 0xDC))->s5();
        float* e = (float*)fn_802CA5A0((Unk802CA5A0_Elem*)((char*)lbl_8061E978 + 0x125E0), lbl_805FD3F4[k]);
        float f = fn_804FAB88(self, e, c);
        fn_804FA8F0(self_, a, f);
        if (s > 1)
            fn_804FA8F0(self_, b, f);
        fn_804F9B84(self_);
        break;
    }
    case 2:
        if (fn_804F87F8((FeS21*)self) == 0 || (((FeSubQ45*)((char*)self->m668 + 0xDC))->s11() != 0 && ((FeSubQ45*)((char*)self->m668 + 0xDC))->s11() != 10)) {
            fn_804F33C0(self, 1);
            self->m634 = 1;
        } else {
            fn_804F33C0(self, 2);
            self->m638 = 1;
        }
        break;
    case 3:
        if (self->m618 != 0 && self->m61C == 0) {
            int t = self->m618->v60();
            if (t == 0 && (self->m654->f & 0x40000000)) {
                fn_8013D0E0(self->m134, 0);
                fn_8013D164(self->m134, 0xA);
                ((FeW2E0*)self->m134)->m2E0 = (int)fn_804F81E0;
                fn_804F8534(self);
                break;
            }
            if (t == 1 && (self->m658->f & 0x40000000)) {
                fn_8013D0E0(self->m134, 0);
                fn_8013D164(self->m134, 0xD);
                ((FeW2E0*)self->m134)->m2E0 = (int)fn_804F82D4;
                fn_804F8534(self);
            }
        }
        break;
    }
}

/* Retail accepts both zero and ten, joining their shared true return. */
extern "C" FE_OVERLAY int fn_804FB7F4(FeS19* s)
{
    if(s->m630==0){
        int r=((Unk8007A4DC_Screen*)lbl_8061930C)->fn_8007A6A0(5,s->m86C);
        if(r==0)goto yes;
        if(r!=10)goto no;
        yes:return 1;
        no:return 0;
    }
    return 0;
}

/* near-miss: retail tests r == 10 with cmpwi/bne; ours folds to xori/subfic/adde (3 slots differ) */
extern "C" FE_OVERLAY int fn_804FB84C(FeS19* s) { if (s->m630 == 2) { int r=((Unk8007A4DC_Screen*)lbl_8061930C)->fn_8007A6A0(5,s->m86C); if (r==0) goto yes; if (r!=10) goto no; yes: return 1; no: return 0; } return 0; }


extern "C" FE_OVERLAY void fn_804FB8A4(FeS19* s, int a, int b)
{
    s->m630 = a;
    s->m86C = b;
    if (a == 1)
        fn_803043D4(1, b);
    else
        fn_803043D4(0, b);
}

extern "C" FE_OVERLAY int fn_804FB8EC(FeCap3* s, int x)
{
    if (x == s->m740[0] || x == s->m740[1] || x == s->m740[2] || x == s->m740[3] || x == s->m740[4] || x == s->m740[5]
        || x == s->m740[6] || x == s->m740[7] || x == s->m740[8] || x == s->m740[10] || x == s->m740[11]
        || x == s->m740[12] || x == s->m740[13] || x == s->m740[14] || x == s->m740[15] || x == s->m740[16]
        || x == s->m740[17] || x == s->m740[18])
        return 1;
    return 0;
}

extern "C" FE_OVERLAY void fn_804FB9D4(FeS27z* self, int sel)
{
    FeObj* q;
    FeObj* m;
    if (sel == self->mA[0]) {
        q = fn_800C8054(self->m764->v32(lbl_805FB0A0, 1));
        m = (FeObj*)self->mA[0];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mA[1]) {
        q = fn_800C8054(self->m764->v32(lbl_805FB0AC, 1));
        m = (FeObj*)self->mA[1];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mA[2]) {
        q = fn_800C8054(self->m764->v32(lbl_805FB0B4, 1));
        m = (FeObj*)self->mA[2];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mA[3]) {
        q = fn_800C8054(self->m764->v32(lbl_805FB0C0, 1));
        m = (FeObj*)self->mA[3];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mA[4]) {
        q = fn_800C8054(self->m764->v32(lbl_805FB0C8, 1));
        m = (FeObj*)self->mA[4];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mA[5]) {
        q = fn_800C8054(self->m764->v32(lbl_805FB0D0, 1));
        m = (FeObj*)self->mA[5];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mA[6]) {
        q = fn_800C8054(self->m764->v32(lbl_805FB0D8, 1));
        m = (FeObj*)self->mA[6];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mA[7]) {
        q = fn_800C8054(self->m764->v32(lbl_805FB0E4, 1));
        m = (FeObj*)self->mA[7];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mA[8]) {
        q = fn_800C8054(self->m764->v32(lbl_805FB0EC, 1));
        m = (FeObj*)self->mA[8];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mB[0]) {
        q = fn_800C8054(self->m78C->v32(lbl_805FB0A0, 1));
        m = (FeObj*)self->mB[0];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mB[1]) {
        q = fn_800C8054(self->m78C->v32(lbl_805FB0AC, 1));
        m = (FeObj*)self->mB[1];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mB[2]) {
        q = fn_800C8054(self->m78C->v32(lbl_805FB0B4, 1));
        m = (FeObj*)self->mB[2];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mB[3]) {
        q = fn_800C8054(self->m78C->v32(lbl_805FB0C0, 1));
        m = (FeObj*)self->mB[3];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mB[4]) {
        q = fn_800C8054(self->m78C->v32(lbl_805FB0C8, 1));
        m = (FeObj*)self->mB[4];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mB[5]) {
        q = fn_800C8054(self->m78C->v32(lbl_805FB0D0, 1));
        m = (FeObj*)self->mB[5];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mB[6]) {
        q = fn_800C8054(self->m78C->v32(lbl_805FB0D8, 1));
        m = (FeObj*)self->mB[6];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mB[7]) {
        q = fn_800C8054(self->m78C->v32(lbl_805FB0E4, 1));
        m = (FeObj*)self->mB[7];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    } else if (sel == self->mB[8]) {
        q = fn_800C8054(self->m78C->v32(lbl_805FB0EC, 1));
        m = (FeObj*)self->mB[8];
        ((FeSubQ44*)((char*)q + 0x98))->s6(((FeSubQ44*)((char*)fn_800C8058(m->v32(lbl_805FB098, 1)) + 0xDC))->s5());
    }
}

extern "C" FE_OVERLAY void fn_804FBF58(void* p)
{
    FeSq* self = (FeSq*)p;
    FeObj* r = 0;
    if (self->m844->m24 < 0) {
        if (self->a[0]->m24 & 0x10000000)
            r = fn_800C8034(self->p0->v32("TOPLEFTSQUARE", 1));
        else if (self->a[1]->m24 & 0x10000000)
            r = fn_800C8034(self->p0->v32("TOPSQUARE", 1));
        else if (self->a[2]->m24 & 0x10000000)
            r = fn_800C8034(self->p0->v32("TOPRIGHTSQUARE", 1));
        else if (self->a[3]->m24 & 0x10000000)
            r = fn_800C8034(self->p0->v32("LEFTSQUARE", 1));
        else if (self->a[4]->m24 & 0x10000000)
            r = fn_800C8034(self->p0->v32("CENTERSQUARE", 1));
        else if (self->a[5]->m24 & 0x10000000)
            r = fn_800C8034(self->p0->v32("RIGHTSQUARE", 1));
        else if (self->a[6]->m24 & 0x10000000)
            r = fn_800C8034(self->p0->v32("BOTTOMLSQUARE", 1));
        else if (self->a[7]->m24 & 0x10000000)
            r = fn_800C8034(self->p0->v32("BOTTOMSQUARE", 1));
        else if (self->a[8]->m24 & 0x10000000)
            r = fn_800C8034(self->p0->v32("BOTTOMRSQUARE", 1));
    } else if (self->m848->m24 < 0) {
        if (self->b[0]->m24 & 0x10000000)
            r = fn_800C8034(self->p1->v32("TOPLEFTSQUARE", 1));
        else if (self->b[1]->m24 & 0x10000000)
            r = fn_800C8034(self->p1->v32("TOPSQUARE", 1));
        else if (self->b[2]->m24 & 0x10000000)
            r = fn_800C8034(self->p1->v32("TOPRIGHTSQUARE", 1));
        else if (self->b[3]->m24 & 0x10000000)
            r = fn_800C8034(self->p1->v32("LEFTSQUARE", 1));
        else if (self->b[4]->m24 & 0x10000000)
            r = fn_800C8034(self->p1->v32("CENTERSQUARE", 1));
        else if (self->b[5]->m24 & 0x10000000)
            r = fn_800C8034(self->p1->v32("RIGHTSQUARE", 1));
        else if (self->b[6]->m24 & 0x10000000)
            r = fn_800C8034(self->p1->v32("BOTTOMLSQUARE", 1));
        else if (self->b[7]->m24 & 0x10000000)
            r = fn_800C8034(self->p1->v32("BOTTOMSQUARE", 1));
        else if (self->b[8]->m24 & 0x10000000)
            r = fn_800C8034(self->p1->v32("BOTTOMRSQUARE", 1));
    }
    if (r != 0) {
        ((FeObj1*)r)->v8(1);
        ((FeObj1*)r)->v16(1);
        ((FeObj1*)r)->v14(1);
        r->m24 &= 0xFEFFFFFF;
    }
}

extern "C" FE_OVERLAY int fn_804FD3C4(FeS20* s, int a)
{
    if (a == s->m844 || a == s->m848) {
        fn_804FC260(s);
        fn_804FBF58(s);
    }
    return 0;
}

extern char lbl_80680EB8[];
extern "C" FE_OVERLAY void fn_804FD418(FeS30* self)
{
    if (((FeSubQ45*)((char*)self->m7B8 + 0xDC))->s11() == 0) {
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB46C, 1)))->v16(0);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB47C, 1)))->v16(0);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB48C, 1)))->v16(0);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB49C, 1)))->v16(0);
    } else {
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB46C, 1)))->v16(1);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB47C, 1)))->v16(1);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB48C, 1)))->v16(1);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB49C, 1)))->v16(1);
    }
    if (((FeSubQ45*)((char*)self->m7CC + 0xDC))->s11() == 0) {
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB4AC, 1)))->v16(0);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB4BC, 1)))->v16(0);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB4CC, 1)))->v16(0);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB4DC, 1)))->v16(0);
    } else {
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB4AC, 1)))->v16(1);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB4BC, 1)))->v16(1);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB4CC, 1)))->v16(1);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB4DC, 1)))->v16(1);
    }
    if (((FeSubQ45*)((char*)self->m7E0 + 0xDC))->s11() == 0) {
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB4EC, 1)))->v16(0);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB4FC, 1)))->v16(0);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB50C, 1)))->v16(0);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB51C, 1)))->v16(0);
    } else {
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB4EC, 1)))->v16(1);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB4FC, 1)))->v16(1);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB50C, 1)))->v16(1);
        ((FeObjQ45*)fn_800C805C(self->v32(lbl_805FB51C, 1)))->v16(1);
    }
}

extern "C" FE_OVERLAY FeRawBig* fn_804FDB24(FeRawBig* self, int a, int b, int c, int d, int e, int f, int g, int h)
{
    fn_804D1070((FeScr6Raw*)self, a, b, c, d, e, f, g, h);
    self->m3C = lbl_80680EB8;
    self->m10EC = 0;
    self->m10F0 = 0;
    self->m10F4 = 0;
    self->m10F8 = 0;
    self->m10FC = 0;
    self->m1100 = 0;
    return self;
}

extern "C" FE_OVERLAY void fn_804FDB84(FeBigS* self)
{
    fn_804D10D8(self);
    self->m10EC = fn_800C805C(self->v32("LEAGUESG", 1));
    self->m10F4 = fn_800C8058(self->m10EC->v32("LEAGUETL", 1));
    self->m10F0 = fn_800C805C(self->v32("TEAMSG", 1));
    self->m10F8 = fn_800C8058(self->m10F0->v32("TEAMTL", 1));
    self->m10FC = fn_800C8054(self->m10F0->v32("TEAMSL", 1));
    self->m1100 = fn_800B7AB8(UiGetMsgMgr(), "BATPITCHINFO");
}

extern "C" FE_OVERLAY void fn_804FDCC8(FeS23y* self)
{
    int z = 0;
    fn_804D11BC();
    FeSubQ31* s1 = (FeSubQ31*)((char*)self->m10F8 + 0xDC);
    s1->c = z;
    s1->s22(0);
    FeSubQ31* s2 = (FeSubQ31*)((char*)self->m10FC + 0x98);
    s2->c = z;
    s2->s22(0);
    *(int*)((char*)self->m10F8 + 0xFC) = 1;
    *(int*)((char*)self->m10FC + 0xB8) = 1;
    ((FeSubQ31*)((char*)self->m10F4 + 0xDC))->s9(0, 0);
    ((FeSubQ31*)((char*)self->m10F4 + 0xDC))->s9(1, 2);
    ((FeSubQ31*)((char*)self->m10F4 + 0xDC))->s9(2, 4);
    ((FeSubQ31*)((char*)self->m10F4 + 0xDC))->s9(3, 7);
    ((FeSubQ31*)((char*)self->m10F4 + 0xDC))->s9(4, 4);
    FeStQ32* st = (FeStQ32*)((char*)fn_800B555C() + 8);
    int a = st->m4F8;
    int b = st->m4FC;
    if (a != -1) {
        int i = 0;
        int c = ((FeStQ32*)fn_800B555C())->m4FC;
        ((FeSubQ31*)((char*)self->m10F4 + 0xDC))->s6(a);
        ((FeSubQ31*)((char*)fn_800C8054(self->m10EC->v32(lbl_805FA3B8, 1)) + 0x98))->s6(a);
        ((FeSubQ31*)((char*)self->m10F8 + 0xDC))->s20(b, 0);
        ((FeSubQ31*)((char*)self->m10FC + 0x98))->s20(b, 0);
        ((FeObjQ28*)self)->v67(4, 0);
        int n = self->m134->v59();
        goto test;
    inc:
        i++;
    test:
        if (i >= n)
            goto done;
        self->m134->v68(i);
        if (fn_804D1278(self) != c)
            goto inc;
        return;
    done:
        self->m134->v68(0);
        return;
    }
    fn_800B555C();
    if (fn_800B57EC() != 0) {
        a = ((FeStQ33*)fn_800B555C())->m608;
        b = fn_8057CF60(fn_8007F7D0((Unk8007F198_B*)lbl_8061930C, 0), a);
    } else {
        b = fn_800B5CBC();
        a = lbl_8061930C->fn_80078A38(b);
    }
    ((FeSubQ31*)((char*)self->m10F4 + 0xDC))->s6(a);
    ((FeSubQ31*)((char*)fn_800C8054(self->m10EC->v32(lbl_805FA3B8, 1)) + 0x98))->s6(a);
    ((FeSubQ31*)((char*)self->m10F8 + 0xDC))->s20(b, 0);
    ((FeSubQ31*)((char*)self->m10FC + 0x98))->s20(b, 0);
    ((FeObjQ28*)self)->v67(4, 0);
}

extern "C" FE_OVERLAY void fn_804FE0F4(FeBigS* self, int kind)
{
    if (kind == 4) {
        ((FeObj1*)self->m10F0)->v8(0);
        ((FeObj1*)self->m10F0)->v16(0);
        FeAr a;
        int buf[0xCB2];
        FeArBase(&a, buf, 0xCB2);
        FeArMid(&a);
        FeArD2(&a);
        fn_80538368(&a, 0);
        ((FeObj1*)self)->v84((int)&a);
        FeArDtor(&a);
    } else {
        int m = 0;
        ((FeObj1*)self->m10F0)->v8(1);
        ((FeObj1*)self->m10F0)->v16(1);
        if (kind == 0) {
        } else if (kind == 1)
            m = 1;
        else if (kind == 2)
            m = 2;
        else if (kind == 3)
            m = 3;
        fn_800D0074((FeSub*)((char*)self->m10F8 + 0xDC), m, 0);
        fn_800D0074((FeSub*)((char*)self->m10FC + 0x98), m, 0);
        ((FeObj2*)self)->v67(5, 0);
    }
}

extern "C" FE_OVERLAY void fn_804FE2C4(FeCap* self)
{
    if (self->m134->v59() > 0) {
        fn_800B555C();
        int t = fn_800B57EC();
        int k = fn_804D1278(self);
        fn_80122D40(self->m1100, k, 1, 1, t ^ 1, 0);
    } else
        fn_80123610(self->m1100);
}
