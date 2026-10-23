/*
 * EA SND 9.02.04 / csis (cflags_snd: -O2 -G0 -fno-strength-reduce -fno-strict-aliasing
 * -ffast-math -mps-float, C++),
 * .text 0x803A9204-0x803A9830 (code-only unit, 13 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 * Reference source: the port/mvp2005 branch (ported from mitsevox/mvp2005, CC0; see docs/DECISIONS.md).
 */

#include "snd/cmn/sndcmn.h"

extern unsigned char lbl_8069C9E4[7];
extern unsigned int lbl_8069CAB8[6];
extern const unsigned int lbl_806251F0[6];
extern int lbl_80625208;
extern void (*lbl_8062520C)();
extern void (*lbl_80625210)();
extern int lbl_8069CAD4;
extern const float lbl_80609598;

extern "C" int fn_803B0798();
extern "C" void iSNDserve();
extern "C" void fn_803AB480(int handle);
extern "C" void iSNDcalcvol(int voice);
extern "C" int fn_803AD2F0(void** pptp, SNDIPATCHHEADER* pph);
extern "C" int SNDPLATFORM_asyncresolvetimbre(SNDIPATCHHEADER* pph, int a, int b);
extern "C" void SNDBANKI_userdatacallback(SNDIPATCHHEADER* pph, int a);
extern "C" void fn_803B12D0(void* s, int chrg);
extern "C" void fn_803B12F8(void* s, int chrg);
extern "C" void fn_803A97DC(int initialize, int priority);

/* SNDplaysetdef: default SNDPLAYOPTS. */
extern "C" int fn_803A9204(SNDPLAYOPTS* pspo)
{
    pspo->pitchmult = 0x1000;
    pspo->timemult = 0x1000;
    pspo->highpasscutoff = 0;
    pspo->keynum = 0x3C;
    pspo->tempomult = 0x1000;
    pspo->lowpasscutoff = 0xFFFF;
    pspo->velocity = 0x7F;
    pspo->vol = 0x7F;
    pspo->drylevel = 0x7F;
    pspo->bend = 0x40;
    pspo->fxlevel0 = 0;
    pspo->azimuth = 0;
    pspo->elevation = 0;
    return 0;
}

extern "C" int SNDPROFILE_outputlatency()
{
    if (lbl_8069C9E4[0] == 0)
        return 0;
    return fn_803B0798();
}

/* Seeds the iSNDrandom state: six words of the seed table plus the given seed. */
extern "C" void SNDI_randomseed(unsigned int seed)
{
    int i;
    for (i = 0; i <= 5; i++)
        lbl_8069CAB8[i] = lbl_806251F0[i] + seed;
}

/* Six-word generator; intermediate stores preserve the later carry reloads. */
unsigned int iSNDrandom()
{
    unsigned c = 0;
    unsigned x = lbl_8069CAB8[5] + lbl_8069CAB8[4];
    if (x < lbl_8069CAB8[5] || x < lbl_8069CAB8[4])
        c = 1;
    unsigned o0 = lbl_8069CAB8[0];
    unsigned o3 = lbl_8069CAB8[3];
    unsigned o2 = lbl_8069CAB8[2];
    unsigned o1 = lbl_8069CAB8[1];
    lbl_8069CAB8[4] = x;
    x = x + o3 + c;
    ((volatile unsigned*)lbl_8069CAB8)[3] = x;
    x = x + o2 + (x < o3);
    ((volatile unsigned*)lbl_8069CAB8)[2] = x;
    x = x + o1 + (x < o2);
    ((volatile unsigned*)lbl_8069CAB8)[1] = x;
    x = x + o0 + (x < o1);
    lbl_8069CAB8[0] = x;
    if (++lbl_8069CAB8[5] == 0)
        if (++lbl_8069CAB8[4] == 0)
            if (++lbl_8069CAB8[3] == 0)
                if (++lbl_8069CAB8[2] == 0)
                    if (++lbl_8069CAB8[1] == 0)
                        x = ++lbl_8069CAB8[0];
    return x;
}

/* near-miss: all 32 insns, but retail loads r4 (a) before the addi r3 for &pph in the SNDPLATFORM_asyncresolvetimbre call; inline wrapper, local pointer and != / > 0 loop forms did not move it */
extern "C" int SNDBANKI_asyncresolvepatch(unsigned int rendermodes, void* ptp, int a, int b)
{
    SNDIPATCHHEADER pph;
    void* p = (char*)ptp + 4;
    int more;
    do {
        more = fn_803AD2F0(&p, &pph);
        if (pph.rendermode & rendermodes) {
            SNDPLATFORM_asyncresolvetimbre(&pph, a, b);
            SNDBANKI_userdatacallback(&pph, 0);
        }
    } while (more != 0);
    return 0;
}

/* Runs the variable-timer clients once the timer period changes. */
extern "C" void SNDSYSI_variabletimerservice()
{
    if (lbl_8069CAD4 != lbl_80625208) {
        lbl_80625208 = lbl_8069CAD4;
        Snd::VariableTimerClient* c = (Snd::VariableTimerClient*)Snd::gVariableTimerList.GetHead();
        while (c != 0) {
            Snd::VariableTimerClient* next = (Snd::VariableTimerClient*)c->ln.GetNext();
            c->pClientFunc(c->pClientData);
            c = next;
        }
    }
}

/* near-miss: 152 insns, same control flow as retail now (the fade clamp is `if (fade < Z) {if (!(v > t)) clamp} else {if (!(v < t)) clamp}`, as matched in StreamFader), but the zero constant (extern const float lbl_80609598) is hoisted out of the loop into f31/a saved register; retail reloads it with lis/lfs in each block. Literal 0.0f, extern float, local float and single-clamp goto forms tried */
extern "C" void SNDSYSI_100hzserver()
{
    int i = 0;
    sndgs.audiotick++;
    iSNDserve();
    for (i = 0; i < sndgs.numserverclients100hz; i++)
        sndgs.serverclient100hz[i]();
    for (i = 0; i < sndgs.voicestotal; i++) {
        CHANPUB* p = &sndgs.chan[i];
        if (p->status == 1) {
            if (sndgs.chan[i].handle >= 0 && p->ppitchlfo != 0) {
                if (++p->curpitchlfoentry >= p->pitchlfolength)
                    p->curpitchlfoentry = 0;
                p->detunepitch = 0;
                iSNDcalcpitch(i);
                SNDPLATFORM_setpitch(i);
            }
            int dirty = 0;
            if (p->pvollfo != 0) {
                dirty = 1;
                if (++p->curvollfoentry >= p->vollfolength)
                    p->curvollfoentry = 0;
            }
            if (p->fadePer100Hz != lbl_80609598) {
                dirty = 1;
                p->programmedVol += p->fadePer100Hz;
                if (p->fadePer100Hz < lbl_80609598) {
                    if (!(p->programmedVol > p->fadeTargetVol)) {
                        p->programmedVol = p->fadeTargetVol;
                        p->fadePer100Hz = lbl_80609598;
                    }
                } else {
                    if (!(p->programmedVol < p->fadeTargetVol)) {
                        p->programmedVol = p->fadeTargetVol;
                        p->fadePer100Hz = lbl_80609598;
                    }
                }
                if (p->programmedVol < lbl_80609598) {
                    fn_803AB480(p->handle);
                    continue;
                }
            }
            p->envduration--;
            if (p->envpertick != 0) {
                dirty = 1;
                p->envvol += p->envpertick;
            }
            if (p->envduration == 0) {
                if (++p->curenvelope >= p->numenvelopes) {
                    fn_803AB480(p->handle);
                    continue;
                }
                ENVELOPE* e = &p->paenvelope[p->curenvelope];
                p->envduration = e->duration;
                if (e->duration < 0)
                    p->envduration = 0x7FFFFFFF;
                p->envpertick = ((e->targetvol << 16) - p->envvol) / p->envduration;
            }
            if (dirty) {
                iSNDcalcvol(i);
                Snd::Hal::SetVol(i);
            }
        }
    }
}

extern "C" void fn_803A971C(void* s, int chrg)
{
    fn_803B12D0(s, chrg);
}

extern "C" void fn_803A973C(void* s, int chrg)
{
    fn_803B12F8(s, chrg);
}

/* SNDSYS_entercritical */
extern "C" void SNDSYS_entercritical()
{
    lbl_8062520C();
    sndgs.incritical++;
}

/* SNDSYS_leavecritical */
extern "C" void SNDSYS_leavecritical()
{
    sndgs.incritical--;
    lbl_80625210();
}

/* __static_initialization_and_destruction_0: Snd::gVariableTimerList(). */
extern "C" void fn_803A97DC(int initialize, int priority)
{
    if (priority == 0xFFFF && initialize != 0)
        *(void**)&Snd::gVariableTimerList = 0;
}

/* _GLOBAL_.I */
extern "C" void fn_803A9804()
{
    fn_803A97DC(1, 0xFFFF);
}
