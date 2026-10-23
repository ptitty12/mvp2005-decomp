/*
 * EA SND 9.02.04 / csis (cflags_snd: -O2 -G0 -fno-strength-reduce -fno-strict-aliasing
 * -ffast-math -mps-float, C++),
 * .text 0x803A6D80-0x803A7BEC (code-only unit, 19 functions; TU boundaries approximate, placeholder).
 * Every data object is extern by its lbl_ name.
 * Reference source: the port/mvp2005 branch (ported from mitsevox/mvp2005, CC0; see docs/DECISIONS.md).
 */
#include "snd/sndo.h"
#include "snd/cmn/sndcmn.h"
#undef override
#include "snd/extern/aemsdef.h"
#include "snd/cmn/saemsi.h"

int SNDAEMSI_playerresetoutputs(AemsDef::PLAYERSTATE* pplayerstate);

static inline AemsDef::SNDAEMSSTREAMCHANNEL* GetStreamChannel(int handle)
{
    AemsDef::SNDAEMSSTREAMCHANNEL* pchan = NULL;
    AemsDef::SNDAEMSSTREAMCHANNEL* p = &sndaems.pstreamchannel[handle & 0x7F];
    if (handle == p->handle) {
        pchan = p;
    }
    return pchan;
}

extern AemsDef::PLAYERPLAYFN* lbl_806250BC[4];
extern AemsDef::PLAYERSTOPFN* lbl_806250CC[4];
extern AemsDef::PLAYERPAUSEFN* lbl_806250DC[4];
extern AemsDef::PLAYERUNPAUSEFN* lbl_806250EC[4];
extern AemsDef::PLAYERUPDATEFN* lbl_806250FC[4];
extern int (*lbl_8069CA34)();
extern AemsDef::PLAYERINTERFACEFN* lbl_806251B0[9];

/* View of sndaems whose flag and skip counter are volatile in the original code (reloaded after each store). */
struct SndAemsVolatileView {
    char pad0[9];
    volatile signed char timerUpdatesSkipped;
    char pad0A[2];
    volatile int instreamsynctask;
    char pad10[0x20 - 0x10];
    void* clientData;
};
#define SNDAEMS_V (*reinterpret_cast<SndAemsVolatileView*>(&sndaems))

struct StreamListHead {
    CListDNode* phead;
};

extern "C" {
void SNDSYS_entercritical();
void SNDSYS_leavecritical();
int fn_803AAA18(int sndstrmhandle, int arg, const char* filename, int offset);
void fn_803AAA3C(int sndstrmhandle);
AemsDef::PLAYERHANDLE* SNDAEMSI_playerplaystream(AemsDef::PLAYERHANDLE* out, AemsDef::PLAYERSTATE* ps, AemsDef::SAMPLEENTRY* se);
int SNDAEMSI_playerupdatestream(AemsDef::PLAYERSTATE* ps);
int fn_803A7ADC();
void SNDAEMSI_streamsystemtask();
void fn_803AC7F0(void (*fn)());
void fn_803AC7C4(void (*fn)());
int SNDSTRM_destroy(int sndstrmhandle);
int fn_803AA934(SNDPLAYOPTS* spo, int a, int b, void* mem, int size);
int SNDplaysetdef(SNDPLAYOPTS* spo);
int SNDSTRM_overhead(int a, int b);
void SNDAEMSI_playerunpausestream(AemsDef::PLAYERSTATE* ps);
int SNDSTRM_pitchmult(int handle, int value);
void SNDSTRM_timemult(int handle, int value);
int fn_803AB9F0(int handle, int arg, float v);
void fn_803AAB7C(int handle, int arg, float v);
void SNDSTRM_fxlevel(int handle, int arg, int value);
void fn_803AB430(int handle, int value);
void fn_803AB388(int handle, int value);
void SNDSTRM_drylevel(int handle, int value);

int SNDAEMSI_streamgethandle(int priority)
{
    AemsDef::SNDAEMSSTREAMCHANNEL* pchan;
    CListDNode* head = sndaems.freestreams.GetHead();
    if (head != NULL) {
        CListDNode* next = head->GetNext();
        reinterpret_cast<StreamListHead*>(&sndaems.freestreams)->phead = next;
        if (next != NULL) {
            next->SetPrev(NULL);
        }
    }
    pchan = reinterpret_cast<AemsDef::SNDAEMSSTREAMCHANNEL*>(head);
    if (pchan != NULL) {
        sndaems.allocatedstreams.Push(&pchan->ln);
    } else {
        unsigned bestts = 0xFFFFFFFF;
        int bestprio = priority - 1;
        AemsDef::SNDAEMSSTREAMCHANNEL* p = reinterpret_cast<AemsDef::SNDAEMSSTREAMCHANNEL*>(sndaems.allocatedstreams.GetHead());
        for (; p != NULL; p = reinterpret_cast<AemsDef::SNDAEMSSTREAMCHANNEL*>(p->ln.GetNext())) {
            if (p->priority < bestprio) {
                bestprio = p->priority;
                bestts = p->timestamp;
                pchan = p;
            } else if (p->priority == bestprio && p->timestamp < bestts) {
                bestts = p->timestamp;
                pchan = p;
            }
        }
        if (pchan == NULL) {
            return -9;
        }
        fn_803AAA3C(pchan->sndstrmhandle);
    }
    int h = (pchan->handle & 0x3FFFFFFF) + 0x80;
    pchan->handle = h;
    pchan->timestamp = sndgs.audiotick;
    pchan->priority = priority;
    return h;
}

void fn_803A6E90(int handle, int value)
{
    SNDSTRM_pitchmult(handle, SNDI_clipint32(value, 0, 0xFFFF));
}

void fn_803A6ED8(int handle, int value)
{
    if (value > 0x2000) {
        value = 0x2000;
    } else if (value < 0x800) {
        value = 0x800;
    }
    SNDSTRM_timemult(handle, value);
}

void SNDAEMSI_streamvol(int handle, int value)
{
    fn_803AB9F0(handle, -1, SNDI_clipint32(value, 0, 0x7FFF) * (1.0f / 32767.0f));
}

void SNDAEMSI_streamazimuth(int handle, int value)
{
    SNDSTRMI_getstreamptr(handle);
    fn_803AAB7C(handle, -1, static_cast<unsigned short>(value) * (360.0f / 65535.0f));
}

void SNDAEMSI_streamfxwet0(int handle, int value)
{
    SNDSTRM_fxlevel(handle, 0, SNDI_clipint32(value, 0, 0x7FFF) >> 8);
}

void fn_803A7038(int handle, int value)
{
    fn_803AB430(handle, SNDI_clipint32(value, 0, 0xFFFF));
}

void fn_803A7080(int handle, int value)
{
    fn_803AB388(handle, SNDI_clipint32(value, 0, 0xFFFF));
}

void SNDAEMSI_streamdrylevel(int handle, int value)
{
    SNDSTRM_drylevel(handle, SNDI_clipint32(value, 0, 0x7FFF) >> 8);
}

void fn_803A7108()
{
}

AemsDef::PLAYERHANDLE* SNDAEMSI_playerplaystream(AemsDef::PLAYERHANDLE* out, AemsDef::PLAYERSTATE* ps, AemsDef::SAMPLEENTRY* se)
{
    int in[12];
    int h = SNDAEMSI_streamgethandle(se->priority);
    if (h >= 0) {
        AemsDef::PLAYERINPUTSTATE* pin;
        int i;
        AemsDef::SNDAEMSSTREAMCHANNEL* pchan = GetStreamChannel(h);
        pchan->psampleentry = se;
        pchan->filename = ps->settings.pModuleBank->streamfilepath;
        pchan->fileoffset = ps->settings.pModuleBank->streamfileoffset;
        pchan->state = 0;
        in[0] = 0x1000;
        in[1] = 0x1000;
        in[2] = 0x7FFF;
        in[3] = 0;
        in[4] = 0;
        in[5] = 0;
        in[6] = 0xFFFF;
        in[7] = 0;
        in[8] = 0x7FFF;
        pin = reinterpret_cast<AemsDef::PLAYERINPUTSTATE*>(&ps[1]);
        for (i = 0; i < ps->settings.numinputs; i++) {
            in[pin->settings.type] = pin->value;
            pin++;
        }
        for (i = 0; i <= 8; i++) {
            if (pchan->input[i] != in[i]) {
                lbl_806251B0[i](pchan->sndstrmhandle, in[i]);
                pchan->input[i] = in[i];
            }
        }
        if (ps->settings.updateoutputs != 0) {
            AemsDef::PLAYEROUTPUTS* pout = reinterpret_cast<AemsDef::PLAYEROUTPUTS*>(pin);
            pout->timeleft = 0x7FFFFFFF;
            pout->timecurr = 0;
        }
    }
    out->u.streamhandle = h;
    return out;
}

void SNDAEMSI_playerstopstream(AemsDef::PLAYERSTATE* ps)
{
    AemsDef::SNDAEMSSTREAMCHANNEL* pchan = GetStreamChannel(ps->settings.handle.u.streamhandle);
    if (pchan != NULL) {
        fn_803AAA3C(pchan->sndstrmhandle);
        sndaems.allocatedstreams.Remove(&pchan->ln);
        sndaems.freestreams.Push(&pchan->ln);
        if (ps->settings.prevplaycontrol[0] == 2) {
            SNDAEMSI_playerunpausestream(ps);
        }
    }
}

void SNDAEMSI_playerpausestream(AemsDef::PLAYERHANDLE h)
{
    AemsDef::SNDAEMSSTREAMCHANNEL* pchan = GetStreamChannel(h.u.streamhandle);
    if (pchan != NULL) {
        fn_803AB9F0(pchan->sndstrmhandle, -1, 0.0f);
        SNDSTRM_pitchmult(pchan->sndstrmhandle, 0);
    }
}

void SNDAEMSI_playerunpausestream(AemsDef::PLAYERSTATE* ps)
{
    AemsDef::SNDAEMSSTREAMCHANNEL* pchan = GetStreamChannel(ps->settings.handle.u.streamhandle);
    if (pchan != NULL) {
        AemsDef::PLAYERINPUTSTATE* pin = reinterpret_cast<AemsDef::PLAYERINPUTSTATE*>(&ps[1]);
        int i;
        for (i = 0; i < ps->settings.numinputs; i++, pin++) {
            int type = pin->settings.type;
            if (type == 0 || type == 2) {
                pchan->input[type] = pin->value;
            }
        }
        fn_803A6E90(pchan->sndstrmhandle, pchan->input[0]);
        SNDAEMSI_streamvol(pchan->sndstrmhandle, pchan->input[2]);
    }
}

/* near-miss: control flow and calls as in the original code, but the original keeps pchan,
   pss and ps in r27/r26/r28 (7 saved registers, ours uses 8) and converts the sample rate with
   a signed int-to-float; the free path is laid out before the update block there. */
int SNDAEMSI_playerupdatestream(AemsDef::PLAYERSTATE* ps)
{
    AemsDef::SNDAEMSSTREAMCHANNEL* pchan = GetStreamChannel(ps->settings.handle.u.streamhandle);
    if (pchan != NULL) {
        SNDSTREAMCHANNEL* pss = SNDSTRMI_getstreamptr(pchan->sndstrmhandle);
        if (pchan->state == 1 && pss->allocatedrequests.items == 0 && pchan->psampleentry->type == 1) {
            sndaems.allocatedstreams.Remove(&pchan->ln);
            sndaems.freestreams.Push(&pchan->ln);
        } else {
            AemsDef::PLAYERINPUTSTATE* pin = reinterpret_cast<AemsDef::PLAYERINPUTSTATE*>(&ps[1]);
            int i;
            for (i = 0; i < ps->settings.numinputs; i++, pin++) {
                int type = pin->settings.type;
                if (pchan->input[type] != pin->value) {
                    lbl_806251B0[type](pchan->sndstrmhandle, pin->value);
                    pchan->input[type] = pin->value;
                }
            }
            if (ps->settings.updateoutputs != 0) {
                AemsDef::PLAYEROUTPUTS* pout = reinterpret_cast<AemsDef::PLAYEROUTPUTS*>(pin);
                if (pchan->state == 1) {
                    unsigned short samplerate = pss->cursf.samplerate;
                    Snd::SNDSTREAMREQUEST* req = reinterpret_cast<Snd::SNDSTREAMREQUEST*>(pss->allocatedrequests.phead);
                    if (samplerate != 0) {
                        float inverse = 1000.0f / static_cast<float>(samplerate);
                        if (req != NULL) {
                            if (pchan->leadinrequesthandle == req->sndrequesthandle) {
                                pchan->leadinsamples = req->totalframes;
                                pout->timecurr = SNDI_ftoifast(static_cast<float>(req->currentframe) * inverse);
                            } else {
                                pout->timeleft = SNDI_ftoifast(static_cast<float>(req->totalframes - req->currentframe) * inverse);
                                pout->timecurr = SNDI_ftoifast(static_cast<float>(pchan->leadinsamples + req->currentframe) * inverse);
                            }
                        }
                    }
                } else {
                    pout->timeleft = 0x7FFFFFFF;
                    pout->timecurr = 0;
                }
            }
            return 1;
        }
    }
    return SNDAEMSI_playerresetoutputs(ps);
}

void SNDAEMSI_streamsystemtask()
{
    sndaems.instreamsynctask = true;
    SNDSYS_entercritical();
    AemsDef::SNDAEMSSTREAMCHANNEL* pchan = reinterpret_cast<AemsDef::SNDAEMSSTREAMCHANNEL*>(sndaems.allocatedstreams.GetHead());
    for (; pchan != NULL; pchan = reinterpret_cast<AemsDef::SNDAEMSSTREAMCHANNEL*>(pchan->ln.GetNext())) {
        if (pchan->state == 0) {
            pchan->leadinsamples = 0;
            pchan->leadinrequesthandle = -1;
            if (pchan->psampleentry->type == 1) {
                SNDSYS_leavecritical();
                fn_803AAA18(pchan->sndstrmhandle, 0, pchan->filename, pchan->fileoffset + pchan->psampleentry->u.loopedstream.leadinoffset);
                SNDSYS_entercritical();
            } else {
                SNDSYS_leavecritical();
                pchan->leadinrequesthandle = fn_803AAA18(pchan->sndstrmhandle, 1000, pchan->filename, pchan->fileoffset + pchan->psampleentry->u.loopedstream.leadinoffset);
                SNDSYS_entercritical();
            }
            pchan->state = 1;
        }
        if (pchan->state == 1 && pchan->psampleentry->type == 2) {
            if (SNDSTRMI_getstreamptr(pchan->sndstrmhandle)->freerequests.items > 0) {
                SNDSYS_leavecritical();
                fn_803AAA18(pchan->sndstrmhandle, 0, pchan->filename, pchan->fileoffset + pchan->psampleentry->u.loopedstream.loopoffset);
                SNDSYS_entercritical();
            }
        }
    }
    SNDAEMS_V.instreamsynctask = false;
    while (SNDAEMS_V.timerUpdatesSkipped > 0) {
        SNDAEMSI_timerupdate(SNDAEMS_V.clientData);
        SNDAEMS_V.timerUpdatesSkipped--;
    }
    SNDSYS_leavecritical();
}

int SNDAEMS_streaminit(void* pmem, int memsize, int count)
{
    SNDPLAYOPTS spo;
    int i;
    fn_803A7ADC();
    lbl_8069CA34 = fn_803A7ADC;
    lbl_806250BC[2] = lbl_806250BC[1] = reinterpret_cast<AemsDef::PLAYERPLAYFN*>(SNDAEMSI_playerplaystream);
    lbl_806250CC[2] = lbl_806250CC[1] = reinterpret_cast<AemsDef::PLAYERSTOPFN*>(SNDAEMSI_playerstopstream);
    lbl_806250DC[2] = lbl_806250DC[1] = reinterpret_cast<AemsDef::PLAYERPAUSEFN*>(SNDAEMSI_playerpausestream);
    lbl_806250EC[2] = lbl_806250EC[1] = reinterpret_cast<AemsDef::PLAYERUNPAUSEFN*>(SNDAEMSI_playerunpausestream);
    lbl_806250FC[2] = lbl_806250FC[1] = reinterpret_cast<AemsDef::PLAYERUPDATEFN*>(SNDAEMSI_playerupdatestream);
    SNDSYS_entercritical();
    sndaems.pstreamchannel = reinterpret_cast<AemsDef::SNDAEMSSTREAMCHANNEL*>(pmem);
    sndaems.streamchannels = count;
    memsize = (memsize - count * 0x60) / count;
    pmem = reinterpret_cast<char*>(pmem) + count * 0x60;
    SNDplaysetdef(&spo);
    spo.fxlevel0 = 0;
    memsize &= 0x7FFFFFFC;
    for (i = 0; i < count; i++) {
        AemsDef::SNDAEMSSTREAMCHANNEL* pchan = &sndaems.pstreamchannel[i];
        sndaems.freestreams.Push(&pchan->ln);
        pchan->sndstrmhandle = fn_803AA934(&spo, 3, 0x2D, pmem, memsize);
        pmem = reinterpret_cast<char*>(pmem) + memsize;
        pchan->handle = i;
        pchan->input[0] = 0x1000;
        pchan->input[1] = 0x1000;
        pchan->input[2] = 0x7FFF;
        pchan->input[3] = 0;
        pchan->input[4] = 0;
        pchan->input[5] = 0;
        pchan->input[6] = 0xFFFF;
        pchan->input[7] = 0;
        pchan->input[8] = 0x7FFF;
    }
    fn_803AC7C4(SNDAEMSI_streamsystemtask);
    SNDSYS_leavecritical();
    return memsize - SNDSTRM_overhead(3, 0x2D);
}

int fn_803A7ADC()
{
    if (sndaems.pstreamchannel == NULL) {
        return 0;
    }
    int i;
    fn_803AC7F0(SNDAEMSI_streamsystemtask);
    for (i = 0; i < sndaems.streamchannels; i++) {
        AemsDef::SNDAEMSSTREAMCHANNEL* p = &sndaems.pstreamchannel[i];
        SNDSTRM_destroy(p->sndstrmhandle);
    }
    sndaems.freestreams = CListDStack();
    sndaems.allocatedstreams = CListDStack();
    sndaems.streamchannels = 0;
    sndaems.pstreamchannel = NULL;
    return 0;
}

}
