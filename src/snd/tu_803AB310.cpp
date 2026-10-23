/*
 * EA SND 9.02.04 / csis (cflags_snd: -O2 -G0 -fno-strength-reduce -fno-strict-aliasing
 * -ffast-math -mps-float, C++),
 * .text 0x803AB310-0x803AC88C (code-only unit, 30 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 * Reference source: the port/mvp2005 branch (ported from mitsevox/mvp2005, CC0; see docs/DECISIONS.md).
 */
#include "snd/cmn/sndcmn.h"
#include <string.h>

extern "C" void SNDSYS_entercritical();
extern "C" void SNDSYS_leavecritical();
extern "C" void fn_803B4160(int shandle, int cutoff);
extern "C" void fn_803B445C(int shandle, int cutoff);
extern "C" void SNDPLATFORM_stop(int voice);
extern "C" int fn_803FD944(int a, int b, int c);
extern "C" void fn_803AA810(int h, int a, void* b, int c, int d);
extern "C" int fn_803FE914(int h);
extern "C" void STREAM_setgreedylevel(int h, int v);
extern "C" void SNDCTRL_timemult(int shandle, int v);

extern unsigned char lbl_8069C9E4[7];
extern int lbl_8062543C;
/* near-miss: all 62 insns; retail uses r30 for both the sndgs base and (after the last base use) the SNDMEMI_restore result (stb installed, then mr r30,r3); ours keeps the base in r31 and the result in r30 */
extern "C" int SNDSYS_restore();

extern const signed char lbl_80609630[6][6];
extern int lbl_80625448;

/* SNDSTRM_setvol: reference structure from the ported sstvol.c; MVP's build differs in detail. */

extern int lbl_8062544C;
extern int lbl_80625450;
extern "C" int SNDPLATFORM_outputcaps();
extern "C" int SNDSYS_getopts(SNDSYSOPTS* sso);

static inline void ErrNop(int) { }

static inline int ErrCode(int e) { switch (e) { case -18: case -10: case -7: break; } return 0; }

struct Unk8069CB5C {
    unsigned char mVal;
    char pad[3];
    int mFlag;
};
extern Unk8069CB5C lbl_8069CB5C[];
extern "C" int SNDSYS_setopts(SNDSYSOPTS* sso);

extern int lbl_8069CBA4;

struct Unk8069CB9C {
    int mVal;
    int mFlag;
};
extern Unk8069CB9C lbl_8069CB9C;
extern int lbl_80625440;

extern "C" int SNDPLATFORM_restore(int a);
extern "C" void SNDI_freespkrtable();
extern "C" void fn_803B12CC();
extern "C" int SNDMEMI_restore();

extern "C" Snd::SNDSTREAMREQUEST* SNDSTRMI_getrequestptr(int h)
{
    if (h < 0)
        return 0;
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr((unsigned char)h);
    if (pssc == 0)
        return 0;
    Snd::SNDSTREAMREQUEST* r = (Snd::SNDSTREAMREQUEST*)pssc->allocatedrequests.phead;
    while (r != 0 && r->sndrequesthandle != h)
        r = (Snd::SNDSTREAMREQUEST*)r->node.pnext;
    return r;
}

extern "C" int fn_803AB388(int h, int v)
{
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);
    if (pssc == 0)
        return -8;
    pssc->po.highpasscutoff = v;
    fn_803B4160(pssc->shandle, v);
    return 0;
}

extern "C" int SNDSTRM_modifyhold(int h, int v)
{
    SNDSYS_entercritical();
    int ret = -8;
    Snd::SNDSTREAMREQUEST* r = SNDSTRMI_getrequestptr(h);
    if (r != 0) {
        ret = 0;
        r->holdtime = (short)v;
    }
    SNDSYS_leavecritical();
    return ret;
}

extern "C" int fn_803AB430(int h, int v)
{
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);
    if (pssc == 0)
        return -8;
    pssc->po.lowpasscutoff = v;
    fn_803B445C(pssc->shandle, v);
    return 0;
}

extern "C" int fn_803AB480(int shandle)
{
    int setvoice;
    int voice = SNDVOICEI_get(shandle);
    if (voice >= 0) {
        setvoice = -1;
        while (iSNDpatchkey(voice, &setvoice))
            SNDPLATFORM_stop(setvoice);
    }
    return voice;
}

extern "C" int SNDSTRM_overheadtap(int maxrequests, int maxchunks)
{
    int n = lbl_80625448 * 4;
    n += 0x214;
    n += maxrequests * 0x28;
    return n + SNDPKTPLAY_overhead(maxchunks);
}

extern "C" int SNDSTRM_overhead(int a, int b)
{
    int n = SNDSTRM_overheadtap(a, b);
    return n + fn_803FD944(a + 2, 1, 1);
}

extern "C" int fn_803AB56C(int h, int v)
{
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);
    if (pssc == 0)
        return -8;
    if (v > 0x4000)
        v = 0x4000;
    pssc->po.pitchmult = v;
    SNDpitchmult(pssc->shandle, v);
    return 0;
}

extern "C" void fn_803AB5C8(int a, int b, int c)
{
    fn_803AA810(a, b, 0, c, 2);
}

extern "C" int SNDSTRM_drylevel(int h, int v)
{
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);
    if (pssc == 0)
        return -8;
    pssc->po.drylevel = v;
    SNDCTRL_drylevel(pssc->shandle, v);
    return 0;
}

extern "C" int SNDSTRM_requeststatus(int h, SNDREQUESTSTATUS* s)
{
    s->state = 0;
    s->currenttime = 0;
    s->timetoend = 0;
    s->timebuffered = 0;
    if (h < 0)
        return -8;
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr((unsigned char)h);
    if (pssc == 0)
        return -8;
    Snd::SNDSTREAMREQUEST* r = SNDSTRMI_getrequestptr(h);
    if (r != 0) {
        if (r->continuation != 0) {
            int rate;
            if (pssc->allocatedrequests.phead == (SNDLINKNODE*)r) {
                s->state = 2;
                rate = pssc->cursf.samplerate;
            } else {
                s->state = 1;
                rate = pssc->newsf.samplerate;
            }
            float f = 1000.0f / rate;
            s->currenttime = SNDI_ftoiround((float)(unsigned int)r->currentframe * f);
            s->timetoend = SNDI_ftoiround((float)(unsigned int)(r->totalframes - r->currentframe) * f);
            s->timebuffered = SNDI_ftoiround((float)(unsigned int)r->outstandingframes * f);
        } else {
            s->state = 0;
        }
    } else {
        s->state = 3;
    }
    return 0;
}

extern "C" int SNDSTRM_setgreedylevel(int h, int v)
{
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);
    if (pssc == 0)
        return -8;
    STREAM_setgreedylevel(pssc->streamhandle, v);
    return 0;
}

extern "C" int SNDSTRM_status(int h, SNDSTREAMSTATUS* s)
{
    s->timebuffered = 0;
    s->currentrequest = 0;
    s->outstandingrequests = 0;
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);
    if (pssc == 0)
        return -8;
    int n = pssc->allocatedrequests.items;
    s->outstandingrequests = n;
    if (n != 0) {
        Snd::SNDSTREAMREQUEST* r = (Snd::SNDSTREAMREQUEST*)pssc->allocatedrequests.phead;
        s->currentrequest = r->sndrequesthandle;
        if (pssc->cursf.samplerate != 0) {
            s->timebuffered = (unsigned int)SNDPKTPLAY_framesoutstanding(pssc->packetinstancehandle) * 1000 / pssc->cursf.samplerate;
            if (s->timebuffered == 0) {
                r = SNDSTRMI_getrequestptr(s->currentrequest);
                if (r->avgdatarate != 0) {
                    unsigned int t = fn_803FE914(pssc->streamhandle);
                    if (t > 0x3D0900)
                        t = 0x3D0900;
                    s->timebuffered = t * 1000 / r->avgdatarate;
                }
            }
        }
    }
    return 0;
}

extern "C" int SNDSTRM_timemult(int h, int v)
{
    if (v > 0x2000)
        v = 0x2000;
    else if (v < 0x800)
        v = 0x800;
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);
    if (pssc == 0)
        return -8;
    pssc->po.timemult = v;
    SNDCTRL_timemult(pssc->shandle, v);
    return 0;
}

extern "C" int fn_803AB9F0(int h, int sourceChannel, float volume)
{
    int i = 0;
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);

    if (pssc == 0) {
        return -8;
    }

    if (sourceChannel == -1) {
        for (; i < 6; i++) {
            Snd::StreamVolSetState(&pssc->sourceChannelState[i], volume);
        }
    } else {
        Snd::StreamVolSetState(&pssc->sourceChannelState[sourceChannel], volume);
    }

    int voice = SNDVOICEI_get(pssc->shandle);
    if (voice >= 0) {
        if (sourceChannel == -1) {
            CHANPUB* pVoice = &sndgs.chan[voice];
            bool volChanged = false;
            for (i = 0; i < pVoice->numchan; i++) {
                CHANPUB* pv = &sndgs.chan[pVoice->voices[i]];
                int sourceVoice = pVoice->voices[i];

                if (pv->programmedVol != volume) {
                    pv->programmedVol = volume;
                    Snd::Util::FastVol(pv);
                    volChanged = true;
                    Snd::Hal::SetVol(sourceVoice);
                }
            }
            if (volChanged) {
                for (i = 0; i < lbl_80625448; i++)
                    SNDPLATFORM_setfxlevel(voice, i);
            }
        } else {
            CHANPUB* pVoice = &sndgs.chan[voice];
            int channelIndex = lbl_80609630[pVoice->numchan - 1][sourceChannel];
            if (channelIndex == -0x80)
                goto done;

            CHANPUB* pv = &sndgs.chan[pVoice->voices[channelIndex]];
            int sourceVoice = pVoice->voices[channelIndex];

            pv->fadePer100Hz = 0.0f;
            if (pv->programmedVol != volume) {
                pv->programmedVol = volume;
                Snd::Util::FastVol(pv);
                Snd::Hal::SetVol(sourceVoice);
                for (i = 0; i < lbl_80625448; i++)
                    SNDPLATFORM_setfxlevel(voice, i);
            }
        }
    }

done:
    return 0;
}

extern "C" int CapOutputMode(int k, int* out)
{
    SNDSYSOPTS sso;
    int r = 0;
    *out = 0;
    int e = SNDSYS_getopts(&sso);
    switch (e) {
    case -18:
        e = 0;
        break;
    case -10:
        break;
    case -7:
        e = 0;
        break;
    }
    switch (k) {
    case 1:
        if (sso.cap.outputchannelsmax > 1)
            *out = k;
        break;
    case 0:
    case 8:
        *out = 1;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        break;
    }
    return 0;
}

extern "C" int SetOutputMode(int a)
{
    int tmp;
    CapOutputMode(a, &tmp);
    if (tmp == 0)
        return -5;
    if (lbl_8069C9E4[0] == 0) {
        lbl_80625440 = a;
    } else {
        lbl_8069CB9C.mVal = a;
        lbl_8069CB9C.mFlag = 1;
    }
    return 0;
}

/* near-miss: same instructions, callee-saved registers differ: retail has &sso in r30 and the value in r29 (k in r31); ours puts the value in r30 and &sso in r29. pointer-local forms (ps->) change the stores to 42(r29) so they are not usable */
extern "C" int fn_803ABDD0(int k, int v)
{
    SNDSYSOPTS sso;
    SNDSYSOPTS* ps = &sso;
    int e = SNDSYS_getopts(ps);
    switch (e) {
    case -18:
        e = 0;
        break;
    case -10:
        break;
    case -7:
        e = 0;
        break;
    }
    switch (k) {
    case 0:
        if (lbl_8069C9E4[0] == 0) {
            sso.set.voicesmaincpu = v;
        } else {
            lbl_8069CB5C[k].mVal = v;
            lbl_8069CB5C[k].mFlag = 1;
        }
        break;
    case 2:
        if (lbl_8069C9E4[0] == 0) {
            sso.set.voicesdsp = v;
        } else {
            lbl_8069CB5C[k].mVal = v;
            lbl_8069CB5C[k].mFlag = 1;
        }
        break;
    default:
        return -5;
    }
    if (lbl_8069C9E4[0] == 0) {
        e = SNDSYS_setopts(ps);
        switch (e) {
        case -18:
            e = 0;
            break;
        case -10:
            break;
        case -7:
            e = 0;
            break;
        }
    }
    return 0;
}

extern "C" int fn_803ABEDC()
{
    return -5;
}

extern "C" int fn_803AC11C()
{
    return lbl_8069C9E4[0] == 1;
}

extern "C" int fn_803AC134()
{
    SNDSYS_restore();
    lbl_8062543C = 0;
    return 0;
}

/* near-miss: same instructions, r30/r31 swapped for the second argument and &sso (retail a=r31, &sso=r30); dead error switch is `case -18: e = 0; break; case -10: break; case -7: e = 0; break;` */
extern "C" int fn_803AC164(int k, int a, int b)
{
    switch (k) {
    case 0:
        lbl_8062543C = a;
        lbl_8069CBA4 = b;
        break;
    case 2: {
        SNDSYSOPTS sso;
        SNDSYSOPTS* ps = &sso;
        int e = SNDSYS_getopts(ps);
        switch (e) {
        case -18:
            e = 0;
            break;
        case -10:
            break;
        case -7:
            e = 0;
            break;
        }
        ps->set.arampooladdr = a;
        ps->set.arampoolsize = b;
        e = SNDSYS_setopts(ps);
        switch (e) {
        case -18:
            e = 0;
            break;
        case -10:
            break;
        case -7:
            e = 0;
            break;
        }
        break;
    }
    default:
        return -5;
    }
    return 0;
}

extern "C" int SNDSYS_getopts(SNDSYSOPTS* sso)
{
    if (lbl_8062544C == 0) {
        sndgs.sso.set.maxbanks = 0x10;
        sndgs.sso.set.sndheapthreshold = 0x5A;
        sndgs.sso.set.stealequalpriorityvoices = 1;
        sndgs.sso.set.compatibility = 0;
        sndgs.sso.set.updateperiod = -1.0f;
        sndgs.prevset = sndgs.sso.set;
        lbl_80625450 = SNDPLATFORM_outputcaps();
        lbl_8062544C = 1;
    }
    *sso = sndgs.sso;
    return lbl_80625450;
}

extern "C" void SNDPLATFORM_outputset();

extern "C" int SNDSYS_setopts(SNDSYSOPTS* sso)
{
    sndgs.sso.set = sso->set;
    sndgs.sso.vec = sso->vec;
    SNDPLATFORM_outputset();
    sndgs.prevset = sndgs.sso.set;
    return 0;
}

extern "C" int SNDSYS_restore()
{
    if (fn_803AC11C() == 0)
        return -14;
    if (sndgs.aemsrestore != 0)
        sndgs.aemsrestore();
    if (sndgs.aemsstreamrestore != 0)
        sndgs.aemsstreamrestore();
    if (sndgs.eventrestore != 0)
        sndgs.eventrestore();
    if (sndgs.streamrestore != 0)
        sndgs.streamrestore();
    if (sndgs.midirestore != 0)
        sndgs.midirestore();
    SNDI_freespkrtable();
    if (sndgs.bankremove != 0)
        sndgs.bankremove(-1);
    SNDPLATFORM_restore(0);
    SNDSYS_entercritical();
    SNDMEMI_free(sndgs.chan);
    SNDMEMI_free(sndgs.banklist);
    SNDSYS_leavecritical();
    int r = SNDMEMI_restore();
    sndgs.installed = 0;
    fn_803B12CC();
    return r;
}

extern "C" int SNDSYSI_chanpubinit()
{
    int n = lbl_80625448;
    int nv = sndgs.voicestotal;
    int a = nv * 4;
    n = n * a;
    int size = nv * 0x90 + n;
    sndgs.chan = (CHANPUB*)SNDMEMI_allocz(size);
    memset(sndgs.chan, 0, size);
    int i;
    char* fx = (char*)(sndgs.chan + sndgs.voicestotal);
    for (i = 0; i < sndgs.voicestotal; i++) {
        sndgs.chan[i].pFxVolume = (FXVOLUME*)fx;
        int j;
        for (j = 0; j < lbl_80625448; j++)
            sndgs.chan[i].pFxVolume[j].fxLevel = 1.0f;
        fx += lbl_80625448 * 4;
    }
    return 0;
}

void iSNDserveraddclient(void (*pfunc)(void))
{
    sndgs.serverclient[sndgs.numserverclients] = pfunc;
    sndgs.numserverclients++;
}

void iSNDserverremoveclient(void (*pfunc)(void))
{
    int i;
    for (i = 0; i < sndgs.numserverclients; i++) {
        if (sndgs.serverclient[i] == pfunc) {
            sndgs.numserverclients--;
            for (; i < sndgs.numserverclients; i++)
                sndgs.serverclient[i] = sndgs.serverclient[i + 1];
            return;
        }
    }
}
