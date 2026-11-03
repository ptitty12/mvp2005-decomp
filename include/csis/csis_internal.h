/*
 * Csis (EA common sound interface) internal layouts, as used by the library code in
 * src/snd/tu_803B7DA0.cpp (tail) and src/snd/tu_803B998C.cpp.
 * Evidence: offsets read from Subscribe (module registration), the Valid/SetFast
 * handle code at 0x803B998C..0x803B9D00 and the Subscribe/Unsubscribe lists at
 * 0x803BA160..0x803BA90C. The public interface is include/csis/csis.h.
 */
#ifndef CSIS_INTERNAL_H
#define CSIS_INTERNAL_H 1

#include "csis/csis.h"

namespace Csis {

/* Client list node: FunctionClient / ClassConstructorClient / ClassDestructorClient
 * all start with reserved[2] = {next, prev}. */
struct CNode {
    CNode* next;
    CNode* prev;
    void* func;
    void* data;
};

struct KeyHalves {
    short crc;
    unsigned short gen;
};

union KeyU {
    int key;
    KeyHalves k;
};

/* Function / class table entry (0xC bytes). */
struct FnEntry {
    CNode* clients; /* +0 */
    const char* name; /* +4, relocated against the module base on registration */
    KeyU u;           /* +8 */
};

/* Global variable table entry (0x10 bytes). */
struct GvEntry {
    CNode* clients; /* +0 */
    int value;      /* +4 */
    const char* name; /* +8 */
    KeyU u;           /* +0xC */
};

struct ModLink {
    ModLink* next;
    ModLink* prev;
};

/* lbl_806AA89C: head of the registered-module list (a struct in the original code: its
 * stores alias the module fields). */
struct ModList {
    ModLink* head;
};

/* A registered interface module (header 0x28 bytes, tables follow). */
struct Module {
    char pad0[0xA];
    unsigned short nFn;  /* +0xA */
    unsigned short nCls; /* +0xC */
    unsigned short nGv;  /* +0xE */
    short crc;           /* +0x10 */
    short pad12;
    FnEntry* fn;  /* +0x14 */
    FnEntry* cls; /* +0x18 */
    GvEntry* gv;  /* +0x1C */
    ModLink link; /* +0x20 */
};

/* Layout of FunctionHandle / ClassHandle / GlobalVariableHandle (all 8 bytes). */
struct HandleRaw {
    void* p;
    int key;
};

/* Layout of a live Csis::Class instance (0x10 bytes). */
struct ClassObj {
    FnEntry* entry; /* +0 */
    int refCount;   /* +4 */
    CNode* members; /* +8: member data clients */
    CNode* dtors;   /* +0xC: destructor clients */
};

} // namespace Csis

#endif
