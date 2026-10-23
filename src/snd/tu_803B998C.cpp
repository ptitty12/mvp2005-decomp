/*
 * EA SND 9.02.04 / csis (cflags_snd: -O2 -G0 -fno-strength-reduce -fno-strict-aliasing
 * -ffast-math -mps-float, C++),
 * .text 0x803B998C-0x803BA9DC (code-only unit, 43 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 * Reference source: the port/mvp2005 branch (ported from mitsevox/mvp2005, CC0; see docs/DECISIONS.md).
 */
#include "csis/csis_internal.h"
#include <cstring>

using namespace Csis;

extern ModList lbl_806AA89C;
extern EA::Allocator::IAllocator* lbl_806AA8A0;
extern char lbl_806AA8A4[0x18];
extern int lbl_80626DC0;
extern unsigned short lbl_80626DC4;

extern "C" {
void OSInitMutex(void*);
void OSLockMutex(void*);
void OSUnlockMutex(void*);
int fn_803B998C(FunctionHandle* h);
int fn_803B9B1C(ClassHandle* h);
int fn_803B9CAC(GlobalVariableHandle* h);
int fn_803B9DDC();
int fn_803B9E08();
void fn_803B9CFC(Class* c, void* data);
int ReleaseFast(Class* c);
int fn_803BA80C(GlobalVariableHandle* h, void* v);
void fn_803BA988(int a, int b);
}

extern "C" int fn_803B998C(FunctionHandle* h)
{
    HandleRaw* r = (HandleRaw*)h;
    if (r->key < 0)
        return r->key;
    FnEntry* e = (FnEntry*)r->p;
    if (e == 0)
        return -6;
    if (r->key == e->u.key)
        return 0;
    r->key = -3;
    r->p = 0;
    return -3;
}

struct SndErrObj {
    int pad0;
    int m4;
};

extern "C" void fn_803B99DC(SndErrObj* s)
{
    s->m4 = -3;
}

extern "C" int fn_803B99E8(ClassHandle* h, const InterfaceId* id)
{
    fn_803B9DDC();
    int r = h->SetFast(id);
    fn_803B9E08();
    return r;
}

Result ClassHandle::SetFast(const InterfaceId* id)
{
    int tried = 0;
retry:
    {
        Module* m;
        for (ModLink* l = lbl_806AA89C.head; l != 0; l = m->link.next) {
            m = (Module*)((char*)l - 0x20);
            FnEntry* tbl = m->cls;
            int n = m->nCls;
            if (m->crc == id->systemCrc || tried != 0) {
                for (int i = 0; i < n; i++) {
                    FnEntry* e = (FnEntry*)(i * 12 + (int)tbl);
                    if (id->interfaceCrc == e->u.k.crc && strcmp(id->pString, e->name) == 0) {
                        mpPrivate = e;
                        mKey = e->u.key;
                        return RESULT_OK;
                    }
                }
            }
        }
        if (tried != 0)
            return RESULT_ERR_NOTFOUND;
        tried = 1;
        goto retry;
    }
}

extern "C" int fn_803B9B1C(ClassHandle* h)
{
    HandleRaw* r = (HandleRaw*)h;
    if (r->key < 0)
        return r->key;
    FnEntry* e = (FnEntry*)r->p;
    if (e == 0)
        return -6;
    if (r->key == e->u.key)
        return 0;
    r->key = -3;
    r->p = 0;
    return -3;
}

extern "C" void fn_803B9B6C(SndErrObj* s)
{
    s->m4 = -3;
}

extern "C" int fn_803B9B78(GlobalVariableHandle* h, const InterfaceId* id)
{
    fn_803B9DDC();
    int r = h->SetFast(id);
    fn_803B9E08();
    return r;
}

Result GlobalVariableHandle::SetFast(const InterfaceId* id)
{
    int tried = 0;
retry:
    {
        Module* m;
        for (ModLink* l = lbl_806AA89C.head; l != 0; l = m->link.next) {
            m = (Module*)((char*)l - 0x20);
            GvEntry* tbl = m->gv;
            int n = m->nGv;
            if (m->crc == id->systemCrc || tried != 0) {
                for (int i = 0; i < n; i++) {
                    GvEntry* e = (GvEntry*)(i * 16 + (int)tbl);
                    if (id->interfaceCrc == e->u.k.crc && strcmp(id->pString, e->name) == 0) {
                        mpPrivate = e;
                        mKey = e->u.key;
                        return RESULT_OK;
                    }
                }
            }
        }
        if (tried != 0)
            return RESULT_ERR_NOTFOUND;
        tried = 1;
        goto retry;
    }
}

extern "C" int fn_803B9CAC(GlobalVariableHandle* h)
{
    HandleRaw* r = (HandleRaw*)h;
    if (r->key < 0)
        return r->key;
    GvEntry* e = (GvEntry*)r->p;
    if (e == 0)
        return -6;
    if (r->key == e->u.key)
        return 0;
    r->key = -3;
    r->p = 0;
    return -3;
}

extern "C" void fn_803B9CFC(Class* c, void* data)
{
    for (CNode* n = ((ClassObj*)c)->members; n != 0; n = n->next)
        ((void (*)(void*, void*))n->func)(data, n->data);
}

extern "C" int fn_803B9D50(EA::Allocator::IAllocator* a)
{
    lbl_806AA8A0 = a;
    return 0;
}

extern "C" int fn_803B9D60(EA::Allocator::IAllocator** pa)
{
    *pa = lbl_806AA8A0;
    if (lbl_806AA8A0 == 0)
        return -6;
    return 0;
}

extern "C" int Init()
{
    OSInitMutex(&lbl_806AA8A4);
    lbl_806AA89C.head = 0;
    lbl_80626DC0 = 1;
    return 0;
}

extern "C" int fn_803B9DC8()
{
    lbl_80626DC0 = 0;
    return 0;
}

extern "C" int fn_803B9DDC()
{
    OSLockMutex(&lbl_806AA8A4);
    return 0;
}

extern "C" int fn_803B9E08()
{
    OSUnlockMutex(&lbl_806AA8A4);
    return 0;
}

extern "C" int Subscribe(Module* m)
{
    int i;
    m->fn = (FnEntry*)((char*)m + 0x28);
    m->cls = m->fn + m->nFn;
    m->gv = (GvEntry*)(m->cls + m->nCls);
    for (i = 0; i < m->nFn; i++) {
        m->fn[i].name += (int)m;
        ++lbl_80626DC4;
        if (lbl_80626DC4 >= 0x8000)
            lbl_80626DC4 = 1;
        m->fn[i].u.k.gen = lbl_80626DC4;
    }
    for (i = 0; i < m->nCls; i++) {
        m->cls[i].name += (int)m;
        ++lbl_80626DC4;
        if (lbl_80626DC4 >= 0x8000)
            lbl_80626DC4 = 1;
        m->cls[i].u.k.gen = lbl_80626DC4;
    }
    for (i = 0; i < m->nGv; i++) {
        m->gv[i].name += (int)m;
        ++lbl_80626DC4;
        if (lbl_80626DC4 >= 0x8000)
            lbl_80626DC4 = 1;
        m->gv[i].u.k.gen = lbl_80626DC4;
    }
    ModLink* l = &m->link;
    l->next = lbl_806AA89C.head;
    l->prev = 0;
    if (lbl_806AA89C.head != 0)
        lbl_806AA89C.head->prev = l;
    lbl_806AA89C.head = l;
    return 0;
}

extern "C" int Unsubscribe(Module* m)
{
    int i;
    for (i = 0; i < m->nFn; i++)
        m->fn[i].u.key = -1;
    ModLink* l = &m->link;
    for (i = 0; i < m->nCls; i++)
        m->cls[i].u.key = -1;
    for (i = 0; i < m->nGv; i++)
        m->gv[i].u.key = -1;
    if (l == lbl_806AA89C.head)
        lbl_806AA89C.head = m->link.next;
    if (l->prev != 0)
        l->prev->next = m->link.next;
    if (m->link.next != 0)
        m->link.next->prev = l->prev;
    return 0;
}

Result Function::CallFast(FunctionHandle* pFunctionHandle, void* pParameters)
{
    int r = fn_803B998C(pFunctionHandle);
    if (r < 0)
        return (Result)r;
    CNode* n = ((FnEntry*)((HandleRaw*)pFunctionHandle)->p)->clients;
    if (n == 0)
        return RESULT_ERR_UNUSED;
    do {
        ((void (*)(void*, void*))n->func)(pParameters, n->data);
        n = n->next;
    } while (n != 0);
    return (Result)r;
}

extern "C" int fn_803BA118(FunctionHandle* h, FunctionClient* c)
{
    fn_803B9DDC();
    int r = Function::SubscribeFast(h, c);
    fn_803B9E08();
    return r;
}

Result Function::SubscribeFast(FunctionHandle* pFunctionHandle, FunctionClient* pFunctionClient)
{
    int r = fn_803B998C(pFunctionHandle);
    if (r >= 0) {
        FnEntry* e = (FnEntry*)((HandleRaw*)pFunctionHandle)->p;
        CNode* c = (CNode*)pFunctionClient;
        c->next = e->clients;
        c->prev = 0;
        if (e->clients != 0)
            e->clients->prev = c;
        e->clients = c;
        r = 0;
    }
    return (Result)r;
}

Result Function::UnsubscribeFast(FunctionHandle* pFunctionHandle, FunctionClient* pFunctionClient)
{
    int r = fn_803B998C(pFunctionHandle);
    if (r >= 0) {
        FnEntry* e = (FnEntry*)((HandleRaw*)pFunctionHandle)->p;
        CNode* c = (CNode*)pFunctionClient;
        if (c == e->clients)
            e->clients = c->next;
        if (c->prev != 0)
            c->prev->next = c->next;
        if (c->next != 0)
            c->next->prev = c->prev;
        r = 0;
    }
    return (Result)r;
}

extern "C" int fn_803BA240(ClassHandle* h, void* data, Class** out)
{
    fn_803B9DDC();
    int r = Class::CreateInstanceFast(h, data, out);
    fn_803B9E08();
    return r;
}

Result Class::CreateInstanceFast(ClassHandle* pClassHandle, void* pMemberData, Class** ppClass)
{
    *ppClass = 0;
    int r = fn_803B9B1C(pClassHandle);
    if (r >= 0) {
        ClassObj* o = (ClassObj*)lbl_806AA8A0->Alloc(0x10, EA::TagValuePair(0, 0));
        if (o == 0)
            return RESULT_ERR_MEM;
        o->members = 0;
        o->refCount = 1;
        o->dtors = 0;
        o->entry = (FnEntry*)((HandleRaw*)pClassHandle)->p;
        for (CNode* n = o->entry->clients; n != 0; n = n->next)
            ((void (*)(void*, void*, void*))n->func)(o, pMemberData, n->data);
        fn_803B9CFC((Class*)o, pMemberData);
        *ppClass = (Class*)o;
        r = 0;
    }
    return (Result)r;
}

extern "C" int fn_803BA378(Class* c)
{
    fn_803B9DDC();
    int r = ReleaseFast(c);
    fn_803B9E08();
    return r;
}

extern "C" int ReleaseFast(Class* c)
{
    ClassObj* o = (ClassObj*)c;
    CNode* n = o->dtors;
    if (n != 0) {
        do {
            CNode* nx = n->next;
            ((void (*)(void*, void*))n->func)(o, n->data);
            n = nx;
        } while (n != 0);
    }
    if (--o->refCount == 0)
        lbl_806AA8A0->Free(o, 0);
    return 0;
}

extern "C" int fn_803BA44C(Class* c, int* out)
{
    *out = ((ClassObj*)c)->refCount;
    return 0;
}

extern "C" int fn_803BA45C(Class* c)
{
    ((ClassObj*)c)->refCount++;
    return 0;
}

extern "C" int DecRef(Class* c)
{
    ClassObj* o = (ClassObj*)c;
    if (--o->refCount == 0)
        lbl_806AA8A0->Free(o, 0);
    return 0;
}

extern "C" int fn_803BA4D0(Class* c, void* data)
{
    fn_803B9DDC();
    int r = c->SetMemberDataFast(data);
    fn_803B9E08();
    return r;
}

Result Class::SetMemberDataFast(void* pMemberData)
{
    fn_803B9CFC(this, pMemberData);
    return RESULT_OK;
}

Result Class::SubscribeConstructorFast(ClassHandle* pClassHandle, ClassConstructorClient* pClient)
{
    int r = fn_803B9B1C(pClassHandle);
    if (r >= 0) {
        FnEntry* e = (FnEntry*)((HandleRaw*)pClassHandle)->p;
        CNode* c = (CNode*)pClient;
        c->next = e->clients;
        c->prev = 0;
        if (e->clients != 0)
            e->clients->prev = c;
        e->clients = c;
        r = 0;
    }
    return (Result)r;
}

Result Class::UnsubscribeConstructorFast(ClassHandle* pClassHandle, ClassConstructorClient* pClient)
{
    int r = fn_803B9B1C(pClassHandle);
    if (r >= 0) {
        FnEntry* e = (FnEntry*)((HandleRaw*)pClassHandle)->p;
        CNode* c = (CNode*)pClient;
        if (c == e->clients)
            e->clients = c->next;
        if (c->prev != 0)
            c->prev->next = c->next;
        if (c->next != 0)
            c->next->prev = c->prev;
        r = 0;
    }
    return (Result)r;
}

Result Class::SubscribeDestructorFast(ClassDestructorClient* pClient)
{
    ClassObj* o = (ClassObj*)this;
    CNode* c = (CNode*)pClient;
    c->next = o->dtors;
    c->prev = 0;
    if (o->dtors != 0)
        o->dtors->prev = c;
    o->dtors = c;
    o->refCount++;
    return RESULT_OK;
}

Result Class::UnsubscribeDestructorFast(ClassDestructorClient* pClient)
{
    ClassObj* o = (ClassObj*)this;
    CNode* c = (CNode*)pClient;
    if (c == o->dtors)
        o->dtors = c->next;
    if (c->prev != 0)
        c->prev->next = c->next;
    if (c->next != 0)
        c->next->prev = c->prev;
    if (--o->refCount == 0)
        lbl_806AA8A0->Free(o, 0);
    return RESULT_OK;
}

Result Class::SubscribeMemberDataFast(MemberDataClient* pClient)
{
    ClassObj* o = (ClassObj*)this;
    CNode* c = (CNode*)pClient;
    c->next = o->members;
    c->prev = 0;
    if (o->members != 0)
        o->members->prev = c;
    o->members = c;
    o->refCount++;
    return RESULT_OK;
}

Result Class::UnsubscribeMemberDataFast(MemberDataClient* pClient)
{
    ClassObj* o = (ClassObj*)this;
    CNode* c = (CNode*)pClient;
    if (c == o->members)
        o->members = c->next;
    if (c->prev != 0)
        c->prev->next = c->next;
    if (c->next != 0)
        c->next->prev = c->prev;
    if (--o->refCount == 0)
        lbl_806AA8A0->Free(o, 0);
    return RESULT_OK;
}

extern "C" int fn_803BA7C4(GlobalVariableHandle* h, void* v)
{
    fn_803B9DDC();
    int r = fn_803BA80C(h, v);
    fn_803B9E08();
    return r;
}

extern "C" int fn_803BA80C(GlobalVariableHandle* h, void* v)
{
    int r = fn_803B9CAC(h);
    if (r >= 0) {
        GvEntry* e = (GvEntry*)((HandleRaw*)h)->p;
        int nv = *(int*)v;
        if (nv == e->value)
            return 0;
        CNode* n = e->clients;
        e->value = nv;
        for (; n != 0; n = n->next)
            ((void (*)(void*, void*))n->func)(&e->value, n->data);
    }
    return r;
}

Result GlobalVariable::SubscribeFast(GlobalVariableHandle* pHandle, GlobalVariableClient* pClient)
{
    int r = fn_803B9CAC(pHandle);
    if (r >= 0) {
        GvEntry* e = (GvEntry*)((HandleRaw*)pHandle)->p;
        CNode* c = (CNode*)pClient;
        c->next = e->clients;
        c->prev = 0;
        if (e->clients != 0)
            e->clients->prev = c;
        e->clients = c;
        ((void (*)(void*, void*))c->func)(&e->value, c->data);
        r = 0;
    }
    return (Result)r;
}

Result GlobalVariable::UnsubscribeFast(GlobalVariableHandle* pHandle, GlobalVariableClient* pClient)
{
    int r = fn_803B9CAC(pHandle);
    if (r >= 0) {
        GvEntry* e = (GvEntry*)((HandleRaw*)pHandle)->p;
        CNode* c = (CNode*)pClient;
        if (c == e->clients)
            e->clients = c->next;
        if (c->prev != 0)
            c->prev->next = c->next;
        if (c->next != 0)
            c->next->prev = c->prev;
        r = 0;
    }
    return (Result)r;
}

extern "C" void fn_803BA988(int a, int b)
{
    if (b == 0xFFFF) {
        if (a != 0)
            lbl_806AA89C.head = 0;
    }
}

extern "C" void fn_803BA9B0()
{
    fn_803BA988(1, 0xFFFF);
}
