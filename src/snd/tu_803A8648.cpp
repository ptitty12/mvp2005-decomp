/*
 * EA SND 9.02.04 / csis (cflags_snd: -O2 -G0 -fno-strength-reduce -fno-strict-aliasing
 * -ffast-math -mps-float, C++),
 * .text 0x803A8648-0x803A87D8 (code-only unit, 1 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 * Reference source: the port/mvp2005 branch (ported from mitsevox/mvp2005, CC0; see docs/DECISIONS.md).
 */
#include "snd/sndo.h"
#include "snd/cmn/sndcmn.h"

extern "C" {

void SNDSYS_entercritical();
void SNDSYS_leavecritical();
int SNDstop(int shandle);
int fn_803AD2F0(void* ppatch, void* attr);
void SNDBANKI_userdatacallback(void* attr, int type);
void SNDPLATFORM_memfree(int a, void* p);

int SNDbankremove(int bhandle)
{
    int i;
    if (bhandle == -1) {
        for (i = 0; i < sndgs.sso.set.maxbanks; i++) {
            SNDbankremove(i);
        }
        return 0;
    }
    if (SNDBANKI_valid(bhandle) != 0) {
        return -8;
    }
    SNDSYS_entercritical();
    BANKVER5* phdr = sndgs.banklist[bhandle].phdr;
    for (i = 0; i < sndgs.voicestotal; i++) {
        int o = i * 0x90;
        if (reinterpret_cast<CHANPUB*>(reinterpret_cast<char*>(sndgs.chan) + o)->bhandle == bhandle) {
            SNDstop(*reinterpret_cast<int*>(reinterpret_cast<char*>(sndgs.chan) + o));
        }
    }
    int patnum;
    TAGGEDPATCH* ppatch;
    void* cursor;
    for (patnum = 0; patnum < phdr->numpatches; patnum++) {
        char attr[0x100];
        void* cursor = SNDBANKI_getppatch(phdr, patnum);
        TAGGEDPATCH* ppatch = static_cast<TAGGEDPATCH*>(cursor);
        if (ppatch != NULL) {
            int more;
            cursor = reinterpret_cast<char*>(ppatch) + 4;
            do {
                more = fn_803AD2F0(&cursor, attr);
                SNDBANKI_userdatacallback(attr, 2);
            } while (more != 0);
        }
    }
    if (sndgs.banklist[bhandle].pspuram != NULL) {
        SNDPLATFORM_memfree(8, sndgs.banklist[bhandle].pspuram);
    }
    sndgs.banklist[bhandle].phdr = NULL;
    sndgs.banklist[bhandle].locked = 0;
    SNDSYS_leavecritical();
    return 0;
}

}
