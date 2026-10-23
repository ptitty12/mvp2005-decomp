/*
 * EA SND 9.02.04 / csis (cflags_snd: -O2 -G0 -fno-strength-reduce -fno-strict-aliasing
 * -ffast-math -mps-float, C++),
 * .text 0x803B3E18-0x803B3FE0 (code-only unit, 4 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 * Reference source: the port/mvp2005 branch (ported from mitsevox/mvp2005, CC0; see docs/DECISIONS.md).
 */

#include "snd/cmn/sndcmn.h"

struct DtorAt0 { const void* m; };

extern "C" void fn_803AC134(DtorAt0* s, int chrg);

extern unsigned char lbl_8060990A[];
extern int lbl_8069CBC0;

/* The per-layout table row is formed first, then offset by numchan*6; a channel pointer local keeps base + voice*0x90 in retail order. */
extern "C" void SetDefaultAzimuths(CHANPUB* c)
{
    const unsigned char* tab = lbl_8060990A + lbl_8069CBC0 * 0x24;
    const unsigned char* t = tab + c->numchan * 6;
    int i;
    for (i = 0; i < c->numchan; i++) {
        CHANPUB* ch = &sndgs.chan[c->voices[i]];
        ch->azimuth = t[i] << 8;
    }
}

extern "C" void fn_803B3E88(DtorAt0* s, int chrg)
{
    fn_803AC134(s, chrg);
}

extern "C" int fn_803B3EA8(int shandle, int ch, int level)
{
    int setvoice;
    int voice = SNDVOICEI_get(shandle);
    if (voice >= 0) {
        setvoice = -1;
        while (iSNDpatchkey(voice, &setvoice)) {
            sndgs.chan[setvoice].pFxVolume[ch].fxLevel = level / 127.0f;
            SNDPLATFORM_setfxlevel(setvoice, ch);
        }
    }
    return voice;
}

/* Big-endian n-byte read, sign-extended for 1-3 bytes; a plain while (rem-- != 0) gives retail's rotated loop. */
extern "C" int SNDI_getb(const unsigned char* p, int n)
{
    int v = 0;
    int rem = n;
    while (rem-- != 0)
        v = (v << 8) + *p++;
    if (n == 1 && v > 0x7f)
        v -= 0x100;
    else if (n == 2 && v > 0x7fff)
        v -= 0x10000;
    else if (n == 3 && v > 0x7fffff)
        v -= 0x1000000;
    return v;
}
