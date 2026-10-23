/*
 * EA SND 9.02.04 / csis (cflags_snd: -O2 -G0 -fno-strength-reduce -fno-strict-aliasing
 * -ffast-math -mps-float, C++),
 * .text 0x803A9830-0x803AB278 (code-only unit, 26 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 * Reference source: the port/mvp2005 branch (ported from mitsevox/mvp2005, CC0; see docs/DECISIONS.md).
 */

#include "snd/cmn/sndcmn.h"
#include <string.h>

extern unsigned short lbl_80625214[258];
extern float lbl_806095A0;
extern SNDSTREAMSTATE lbl_8069CADC;
extern const signed char lbl_80609630[6][6];
extern const float lbl_806095D8;
extern int lbl_80625448;
extern "C" int fn_803AB9F0(int h, int ch, float vol);

extern "C" void* SNDSTRMI_getrequestptr(int h);
extern "C" void SNDLINKI_remove(SNDLINKLIST* list, void* node);
extern "C" void SNDLINKI_push(SNDLINKLIST* list, void* node);
extern "C" void STREAM_release(int handle, void* p);
extern "C" int fn_803AA95C(int h);
/* near-miss: 60 vs 70 insns: retail keeps both arms of the `if (req->isgeneric)` tests (numframes load and the psamples loop) as separate code with different temporaries, presumably an endian-conversion macro that expands to the same value in both arms; ours merges the arms. temp-variable variants tried */
extern "C" void SNDSTRMI_parsedata(SNDSTREAMCHANNEL* pssc, int* hdr);
extern "C" void SNDSTRMI_parseheader(int h, int* hdr);
extern "C" int fn_803FE914(int handle);
extern "C" int fn_803FE954(int handle);
extern "C" void SNDSYS_entercritical();
extern "C" void SNDSYS_leavecritical();
extern "C" int SNDSTRMI_numcreated();
extern "C" int fn_803AAA3C(int h);
extern "C" void SNDSTRMI_service();
extern "C" void STREAM_destroy(int h);
extern "C" void STREAM_kill(int h);
struct RcHnd;
extern "C" unsigned char* STREAM_get(RcHnd* h);
extern "C" int SNDSTRMI_create(void* a, int b, void* c, void* d, int e, int f, int g);
extern void* lbl_8069CA44;
extern "C" int fn_803AA810(int h, int holdtime, void* a, void* b, int type);
extern "C" int STREAM_queuefile(int sh, void* a, void* b, int tag);
extern "C" int STREAM_queuemem(int sh, void* a, int b, int tag);
extern "C" void SNDLINKI_pushtail(SNDLINKLIST* list, void* node);
extern "C" void* SNDLINKI_pop(SNDLINKLIST* list);
extern "C" int fn_803B3EA8(int shandle, int ch, int level);

/* Average data rate of a sample format (bytes per second). */

extern "C" void StreamFader(void* data);

int iSNDsin(int angle)
{
    int quad = (angle >> 8) & 3;
    int idx = angle & 0xFF;
    if (quad == 0)
        return lbl_80625214[idx];
    if (quad == 1)
        return lbl_80625214[256 - idx];
    if (quad == 2)
        return -lbl_80625214[idx];
    return -lbl_80625214[256 - idx];
}

extern "C" void StreamRemoveFaders(SNDSTREAMCHANNEL* pssc)
{
    int i;
    for (i = 0; i < 6; i++) {
        Snd::StreamSourceChannelState* s = &pssc->sourceChannelState[i];
        if (s->fader.incrementPerUpdate != lbl_806095A0)
            Snd::Util::RemoveVariableTimerClient(&s->fader.variableTimerClient);
    }
}

/* near-miss: instruction set matches; callee-saved/scratch registers are permuted (retail: rep r0, chans r9, rate r3, mult r11); declaration-order permutations tried */
extern "C" int SNDSTRMI_calcdatarate(SNDSAMPLEFORMAT* ssf)
{
    int rep = ssf->samplerep;
    int chans = ssf->channels;
    int rate = ssf->samplerate * chans;
    int mult = 0;
    if (rep == 10)
        mult = 0x88;
    else if (rep == 4)
        mult = 0x33;
    else if (rep == 22)
        mult = 0x66;
    else if (rep == 7)
        mult = 0x200;
    else if (rep == 16 || rep == 23)
        return chans << 3;
    return (rate * mult) >> 8;
}

extern "C" int SNDSTRMI_destroyall()
{
    int i;
    for (i = 0; i < sndgs.sso.set.maxstreams; i++)
        fn_803AA95C(i);
    return 0;
}

SNDSTREAMCHANNEL* SNDSTRMI_getstreamptr(int sndstreamhandle)
{
    if (sndstreamhandle >= sndgs.sso.set.maxstreams || sndstreamhandle < 0)
        return 0;
    return lbl_8069CADC.pssc[sndstreamhandle];
}

/* SNDSTRMI_getrequestptr takes the handle (its definition in tu_803AB310 reads r3), so h stays live in r3 across the index. */
extern "C" void SNDSTRMI_removerequest(int h)
{
    SNDSTREAMCHANNEL* pssc = lbl_8069CADC.pssc[(unsigned char)h];
    void* req = SNDSTRMI_getrequestptr(h);
    SNDLINKI_remove(&pssc->allocatedrequests, req);
    SNDLINKI_push(&pssc->freerequests, req);
    if (pssc->parsingrequest == req)
        pssc->parsingrequest = 0;
}

extern "C" void SNDSTRMI_releasecallback(int* p)
{
    unsigned char* hdr = (unsigned char*)p[-1];
    STREAM_release(lbl_8069CADC.pssc[hdr[3]]->streamhandle, hdr);
}

extern "C" void SNDSTRMI_framescallback(int unused, unsigned int frames, SNDSTREAMCHANNEL* pssc)
{
    unsigned int leftover = 0;
    int iter = 0;
    for (;;) {
        iter++;
        Snd::SNDSTREAMREQUEST* r = (Snd::SNDSTREAMREQUEST*)pssc->allocatedrequests.phead;
        if (iter > 200) {
            SNDSTRMI_removerequest(r->sndrequesthandle);
            return;
        }
        if (frames > r->outstandingframes) {
            leftover = frames - r->outstandingframes;
            frames = frames - leftover;
        }
        r->currentframe += frames;
        r->outstandingframes -= frames;
        if (r->currentframe >= (unsigned int)r->totalframes)
            SNDSTRMI_removerequest(r->sndrequesthandle);
        if (leftover == 0)
            return;
        frames = leftover;
        leftover = 0;
    }
}

extern "C" void SNDSTRMI_parsedata(SNDSTREAMCHANNEL* pssc, int* hdr)
{
    SNDPACKET psp;
    Snd::SNDSTREAMREQUEST* req = (Snd::SNDSTREAMREQUEST*)pssc->parsingrequest;
    int* in = hdr;
    if (req->isgeneric != 0) {
        int t = in[2];
        psp.numframes = t;
    } else {
        psp.numframes = in[2];
    }
    int* data = in + 3;
    int* end = data + pssc->cursf.channels;
    int i;
    for (i = 0; i < pssc->cursf.channels; i++) {
        if (req->isgeneric != 0) {
            int t = data[i];
            psp.psamples[i] = (char*)end + t;
        } else {
            psp.psamples[i] = (char*)end + data[i];
        }
    }
    if (psp.numframes == 0) {
        STREAM_release(pssc->streamhandle, in);
    } else {
        ((int*)psp.psamples[0])[-1] = (int)in;
        in[0] = req->sndrequesthandle;
        req->outstandingframes += psp.numframes;
        psp.continuation = req->continuation;
        SNDPKTPLAY_submit(pssc->packetinstancehandle, &psp);
        req->continuation = 1;
    }
}

extern "C" void SNDSTRMI_startstream(SNDSTREAMCHANNEL*);
extern "C" int SNDSTRMI_parsechunk(int,int*);
extern "C" int SNDSTRMI_isheld(SNDSTREAMCHANNEL*);
extern "C" void SNDSTRMI_service()
{
    SNDSYS_entercritical();
    int i = 0;
    int limit = sndgs.sso.set.maxstreams;
    for (; i < limit; i++, limit = sndgs.sso.set.maxstreams) {
        SNDSTREAMCHANNEL* ch = lbl_8069CADC.pssc[i];
        if (!ch || !ch->allocatedrequests.items) continue;
        if (ch->state == 2) {
            if (SNDPKTPLAY_framesoutstanding(ch->packetinstancehandle) > 0) continue;
            ch->cursf = ch->newsf;
            ch->cursa = ch->newsa;
            ch->newsa.ptsdata[0] = 0;
            SNDPKTPLAY_stop(ch->packetinstancehandle);
            SNDSTRMI_startstream(ch);
        }
        if (SNDSTRMI_isheld(ch)) continue;
        int count;
        if (ch->state == 1) {
            count = SNDPKTPLAY_submitspace(ch->packetinstancehandle);
            if (count <= 0) continue;
        } else count = 8;
        int result = 0;
        do {
            count--;
            unsigned char* data = STREAM_get((RcHnd*)ch->streamhandle);
            if (data) result = SNDSTRMI_parsechunk(i,(int*)data);
        } while (result && count > 0);
    }
    SNDSYS_leavecritical();
}

extern "C" int SNDSTRMI_parsechunk(int h, int* hdr)
{
    SNDSTREAMCHANNEL* pssc = lbl_8069CADC.pssc[h];
    int tag = *hdr;
    if (tag == 0x5343446c) {
        SNDSTRMI_parsedata(pssc, hdr);
    } else if (tag == 0x5343486c) {
        SNDSTRMI_parseheader(h, hdr);
        return 0;
    } else {
        STREAM_release(pssc->streamhandle, hdr);
    }
    return 1;
}

extern "C" int SNDSTRMI_isheld(SNDSTREAMCHANNEL* pssc)
{
    Snd::SNDSTREAMREQUEST* r = (Snd::SNDSTREAMREQUEST*)pssc->parsingrequest;
    if (r != 0 && r->avgdatarate != 0) {
        if (r->holdtime < 0) {
        yes:
            return 1;
        }
        if (r->holdtime != 0) {
            unsigned int t = fn_803FE914(pssc->streamhandle);
            if (t > 0x3D0900)
                t = 0x3D0900;
            if (t * 1000 / r->avgdatarate >= (unsigned int)r->holdtime || fn_803FE954(pssc->streamhandle) == 2) {
                r->holdtime = 0;
                return 0;
            }
            if (pssc->freerequests.items > 0)
                goto yes;
            int st = fn_803FE954(pssc->streamhandle);
            if (st != 0)
                goto yes;
            r->holdtime = st;
        }
    }
    return 0;
}

extern "C" int SNDSTRMI_numcreated()
{
    int n = 0;
    int i;
    for (i = 0; i < sndgs.sso.set.maxstreams; i++)
        if (lbl_8069CADC.pssc[i] != 0)
            n++;
    return n;
}

extern "C" int fn_803AA810(int h, int holdtime, void* a, void* b, int type)
{
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);
    if (pssc == 0)
        return -8;
    if (pssc->freerequests.items == 0)
        return -13;
    SNDSYS_entercritical();
    if (type == 0)
        b = (void*)STREAM_queuefile(pssc->streamhandle, a, b, 0x6c454353);
    else if (type == 1)
        b = (void*)STREAM_queuemem(pssc->streamhandle, a, 0, 0x6c454353);
    if (b == 0) {
        SNDSYS_leavecritical();
        return -1;
    }
    Snd::SNDSTREAMREQUEST* r = (Snd::SNDSTREAMREQUEST*)SNDLINKI_pop(&pssc->freerequests);
    memset(r, 0, 0x28);
    SNDLINKI_pushtail(&pssc->allocatedrequests, r);
    r->streamrequestid = (int)b;
    pssc->lastsndrequesthandle += 0x100;
    if (pssc->lastsndrequesthandle < 0)
        pssc->lastsndrequesthandle = 0;
    r->sndrequesthandle = pssc->lastsndrequesthandle | h;
    r->holdtime = holdtime;
    int ret = r->sndrequesthandle;
    SNDSYS_leavecritical();
    return ret;
}

extern "C" void fn_803AA934(void* a, int b, void* c, void* d, int e)
{
    SNDSTRMI_create(a, b, c, d, e, 0, 0);
}

extern "C" int fn_803AA95C(int h)
{
    SNDSYS_entercritical();
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);
    if (pssc == 0) {
        SNDSYS_leavecritical();
        return -8;
    }
    fn_803AAA3C(h);
    StreamRemoveFaders(pssc);
    if (SNDSTRMI_numcreated() == 1) {
        iSNDserverremoveclient(SNDSTRMI_service);
        lbl_8069CA44 = 0;
    }
    SNDPKTPLAY_destroy(pssc->packetinstancehandle);
    int sh = pssc->streamhandle;
    int tap = pssc->fromtap;
    lbl_8069CADC.pssc[h] = 0;
    SNDSYS_leavecritical();
    if (tap == 0)
        STREAM_destroy(sh);
    return 0;
}

extern "C" void fn_803AAA18(int a, int b, void* c, void* d)
{
    fn_803AA810(a, b, c, d, 0);
}

extern "C" int fn_803AAA3C(int h)
{
    SNDSYS_entercritical();
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);
    if (pssc == 0) {
        SNDSYS_leavecritical();
        return -8;
    }
    if (pssc->shandle >= 0)
        SNDPKTPLAY_stop(pssc->packetinstancehandle);
    pssc->shandle = -1;
    if (pssc->fromtap == 0) {
        STREAM_kill(pssc->streamhandle);
        STREAM_get((RcHnd*)pssc->streamhandle);
    }
    if (pssc->newsa.ptsdata[0] != 0) {
        int i;
        for (i = 0; i < pssc->newsf.channels; i++)
            SNDMEMI_free(pssc->newsa.ptsdata[i]);
    }
    void* node;
again:
    node = SNDLINKI_pop(&pssc->allocatedrequests);
    if (node != 0) {
        SNDLINKI_push(&pssc->freerequests, node);
        goto again;
    }
    pssc->parsingrequest = 0;
    pssc->state = 0;
    memset(&pssc->cursf, 0, 4);
    memset(&pssc->newsf, 0, 4);
    memset(&pssc->cursa, 0, 0x68);
    memset(&pssc->newsa, 0, 0x68);
    SNDSYS_leavecritical();
    return 0;
}

extern "C" void StreamFader(void* data)
{
    Snd::StreamSourceChannelState* s = (Snd::StreamSourceChannelState*)data;
    s->vol += s->fader.incrementPerUpdate;
    if (s->fader.incrementPerUpdate < lbl_806095D8) {
        if (!(s->vol > s->fader.targetVol)) {
            s->vol = s->fader.targetVol;
            s->fader.incrementPerUpdate = lbl_806095D8;
        }
    } else {
        if (!(s->vol < s->fader.targetVol)) {
            s->vol = s->fader.targetVol;
            s->fader.incrementPerUpdate = lbl_806095D8;
        }
    }
    SNDSTREAMCHANNEL* pssc = (SNDSTREAMCHANNEL*)((char*)s - (0x13c + s->sourceChannel * 0x24));
    int voice = SNDVOICEI_get(pssc->shandle);
    if (voice >= 0) {
        CHANPUB* c = &sndgs.chan[voice];
        int idx = lbl_80609630[c->numchan - 1][s->sourceChannel];
        if (idx != -128) {
            int vv = c->voices[idx];
            CHANPUB* d = &sndgs.chan[vv];
            float v = s->vol;
            d->programmedVol = v;
            d->finalvol = v * Snd::gMasterVol;
            Snd::Hal::SetVol(vv);
            int i;
            for (i = 0; i < lbl_80625448; i++)
                SNDPLATFORM_setfxlevel(voice, i);
        }
    }
    if (s->fader.incrementPerUpdate == lbl_806095D8)
        Snd::Util::RemoveVariableTimerClient(&s->fader.variableTimerClient);
}

extern "C" int SNDSTRM_autovol(int h, int time, int targetVol)
{
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);
    if (pssc == 0)
        return -8;
    float target = targetVol / 127.0f;
    if (time <= 0)
        return fn_803AB9F0(h, -1, target);
    int i;
    for (i = 0; i <= 5; i++) {
        Snd::StreamSourceChannelState* s = &pssc->sourceChannelState[i];
        if (s->fader.incrementPerUpdate != 0.0f)
            Snd::Util::RemoveVariableTimerClient(&s->fader.variableTimerClient);
        s->fader.targetVol = target;
        s->fader.incrementPerUpdate = (target - s->vol) * Snd::gVariableTimerPeriod / time;
        if (s->fader.incrementPerUpdate != 0.0f) {
            s->fader.variableTimerClient.pClientFunc = StreamFader;
            s->fader.variableTimerClient.pClientData = s;
            Snd::Util::AddVariableTimerClient(&s->fader.variableTimerClient);
        }
    }
    return 0;
}

extern "C" void SNDSTRM_createtap(int a, void* b, int c, void* d, void* e, int f)
{
    SNDSTRMI_create(b, c, d, e, f, a, 1);
}

extern "C" int SNDSTRM_fxlevel(int h, int ch, int level)
{
    SNDSTREAMCHANNEL* pssc = SNDSTRMI_getstreamptr(h);
    if (pssc == 0)
        return -8;
    pssc->pFxVolume[ch].fxLevel = level / 127.0f;
    int voice = SNDVOICEI_get(pssc->shandle);
    if (voice >= 0) {
        int i;
        for (i = 0; i < pssc->cursf.channels; i++) {
            CHANPUB* c = &sndgs.chan[sndgs.chan[voice].voices[i]];
            c->pFxVolume[ch].fxLevel = pssc->pFxVolume[ch].fxLevel;
        }
        fn_803B3EA8(pssc->shandle, ch, level);
    }
    return 0;
}
