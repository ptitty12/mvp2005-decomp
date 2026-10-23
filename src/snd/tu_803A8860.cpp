/*
 * EA SND 9.02.04 / csis (cflags_snd: -O2 -G0 -fno-strength-reduce -fno-strict-aliasing
 * -ffast-math -mps-float, C++),
 * .text 0x803A8860-0x803A8BDC (code-only unit, 5 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 * Reference source: the port/mvp2005 branch (ported from mitsevox/mvp2005, CC0; see docs/DECISIONS.md).
 */
#include "csis/csis.h"
#include "snd/sndo.h"
#include "snd/cmn/sndcmn.h"

/* Per-interface settings block of an AEMS module (MVP layout, older than nfsmw's): three
 * function handles at +8, +0x10 and +0x18, flags at +0x30/+0x31. */
struct AemsFnHandles {
    char pad0[8];
    char fh8[8];
    char fh10[8];
    char fh18[8];
    char pad20[0x10];
    unsigned char m30;
    unsigned char m31;
};

/* State object (0x24 bytes) allocated by A1cConstructorCallBack. */
struct AemsIfaceState {
    AemsFnHandles* psettings;
    Csis::FunctionClient fnclient;
    Csis::ClassDestructorClient dtorclient;
};

/* Interface table entry (0x54 bytes): class/function handles, the constructor client and the
 * relocatable interface ids. */
struct AemsIface {
    char classHandle[8];
    char fh8[8];
    char fh10[8];
    char fh18[8];
    Csis::ClassConstructorClient ctor;
    unsigned char f30;
    unsigned char f31;
    unsigned char f32;
    char pad33;
    Csis::InterfaceId id34;
    Csis::InterfaceId id3C;
    Csis::InterfaceId id44;
    Csis::InterfaceId id4C;
};

struct AemsIfaceTable {
    char pad0[0xA];
    unsigned short count;
    AemsIface* entries;
};

extern AemsIfaceTable* lbl_806251E4;
extern EA::Allocator::IAllocator* lbl_806251E8;

extern "C" {

int fn_803BA45C(Csis::Class* c);
void DecRef(Csis::Class* c);
int fn_803B9D60(EA::Allocator::IAllocator** p);
int fn_803B9DDC();
int fn_803B9E08();
void A1cDestructorCallBack(Csis::Class* c, void* data);

void fn_803A8860(Csis::Parameter* params, void* data)
{
    AemsIfaceState* st = static_cast<AemsIfaceState*>(data);
    Csis::Function::CallFast(reinterpret_cast<Csis::FunctionHandle*>(st->psettings->fh10), params);
}

void A1cDestructorCallBack(Csis::Class* c, void* data)
{
    AemsIfaceState* st = static_cast<AemsIfaceState*>(data);
    AemsFnHandles* s = st->psettings;
    Csis::Function::CallFast(reinterpret_cast<Csis::FunctionHandle*>(s->fh18), NULL);
    if (s->m31 != 0) {
        fn_803BA45C(c);
        c->UnsubscribeMemberDataFast(&st->fnclient);
    }
    fn_803BA45C(c);
    c->UnsubscribeDestructorFast(&st->dtorclient);
    lbl_806251E8->Free(st, 0);
}

void A1cConstructorCallBack(Csis::Class* c, void* arg, AemsFnHandles* s)
{
    AemsIfaceState* st = static_cast<AemsIfaceState*>(lbl_806251E8->Alloc(0x24, EA::TagValuePair(0, 0)));
    if (st != NULL) {
        if (s->m30 != 0) {
            Csis::Function::CallFast(reinterpret_cast<Csis::FunctionHandle*>(s->fh8), NULL);
        }
        if (s->m31 != 0) {
            Csis::Function::CallFast(reinterpret_cast<Csis::FunctionHandle*>(s->fh10), arg);
        }
        st->psettings = s;
        if (s->m31 != 0) {
            st->fnclient.pClientData = st;
            st->fnclient.pClientFunc = fn_803A8860;
            c->SubscribeMemberDataFast(&st->fnclient);
            DecRef(c);
        }
        st->dtorclient.pClientData = st;
        st->dtorclient.pClientFunc = reinterpret_cast<void (*)(Csis::Class*, void*)>(A1cDestructorCallBack);
        c->SubscribeDestructorFast(&st->dtorclient);
        DecRef(c);
    }
}


int fn_803A8A20(AemsIfaceTable* t)
{
    int i = 0;
    AemsIface* base = reinterpret_cast<AemsIface*>(reinterpret_cast<char*>(t) + 0x10);
    lbl_806251E4 = t;
    t->entries = base;
    fn_803B9D60(&lbl_806251E8);
    fn_803B9DDC();
    for (; i < lbl_806251E4->count; i++) {
        AemsIface* e = reinterpret_cast<AemsIface*>(i * 0x54 + reinterpret_cast<int>(base));
        e->id34.pString = reinterpret_cast<const char*>(reinterpret_cast<int>(e->id34.pString) + reinterpret_cast<int>(t));
        if (reinterpret_cast<Csis::ClassHandle*>(e->classHandle)->SetFast(&e->id34) < 0) {
            continue;
        }
        if (e->f30 != 0) {
            e->id3C.pString = reinterpret_cast<const char*>(reinterpret_cast<int>(e->id3C.pString) + reinterpret_cast<int>(t));
            reinterpret_cast<Csis::FunctionHandle*>(e->fh8)->SetFast(&e->id3C);
        }
        if (e->f31 != 0) {
            e->id44.pString = reinterpret_cast<const char*>(reinterpret_cast<int>(e->id44.pString) + reinterpret_cast<int>(t));
            reinterpret_cast<Csis::FunctionHandle*>(e->fh10)->SetFast(&e->id44);
        }
        if (e->f32 != 0) {
            e->id4C.pString = reinterpret_cast<const char*>(reinterpret_cast<int>(e->id4C.pString) + reinterpret_cast<int>(t));
            reinterpret_cast<Csis::FunctionHandle*>(e->fh18)->SetFast(&e->id4C);
        }
        e->ctor.pClientFunc = reinterpret_cast<void (*)(Csis::Class*, Csis::Parameter*, void*)>(A1cConstructorCallBack);
        e->ctor.pClientData = e;
        Csis::Class::SubscribeConstructorFast(reinterpret_cast<Csis::ClassHandle*>(e->classHandle), &e->ctor);
    }
    fn_803B9E08();
    return 0;
}

int fn_803A8B4C()
{
    if (lbl_806251E4 == NULL) {
        return -4;
    }
    AemsIface* entries = lbl_806251E4->entries;
    int i = 0;
    fn_803B9DDC();
    for (; i < lbl_806251E4->count; i++) {
        Csis::Class::UnsubscribeConstructorFast(reinterpret_cast<Csis::ClassHandle*>(entries[i].classHandle), &entries[i].ctor);
    }
    fn_803B9E08();
    lbl_806251E4 = NULL;
    lbl_806251E8 = NULL;
    return 0;
}

}
