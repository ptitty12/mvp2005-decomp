#ifndef GAME_TU_80355A6C_H
#define GAME_TU_80355A6C_H

/*
 * Shared declarations for the game tail 0x80355A6C..0x8035B60C (range 28):
 * UI/menu object boilerplate (ctors that store a vtable, dtors with the
 * in-charge flag, forwarders to the base dtors). Vtables and the other data
 * are retail objects, extern by lbl_ name; classes here never define a virtual.
 */
#define PUT(o, v) (*(const void**)((char*)self + (o)) = (v))
#define F(T, p, o) (*(T*)((char*)(p) + (o)))

struct UiPtr0;
struct Unk803442CC { const void* m0; };

/* Slot container of the debug/menu lists (fn_80355A74 .. fn_80355BE4): records of 0x1C
 * bytes (m0, count m4), a virtual "slot is used" test at vtable +0x98, vptr at +0x10,
 * and an int list (data m14, count m18) behind it. */
struct Rec1C {
    int m0;
    int m4;
    int m8;
    int mC;
    int m10;
    int m14;
    int m18;
};
class Cont0 {
public:
    Rec1C* m0;
    unsigned m4;
    int m8;
    int mC;
    virtual int s1();
    virtual int s2();
    virtual int s3();
    virtual int s4();
    virtual int s5();
    virtual int s6();
    virtual int s7();
    virtual int s8();
    virtual int s9();
    virtual int s10();
    virtual int s11();
    virtual int s12();
    virtual int s13();
    virtual int s14();
    virtual int s15();
    virtual int s16();
    virtual int s17();
    virtual int s18();
    virtual int s19(int a);
};
struct IntVec {
    int* volatile m0;
    volatile unsigned m4;
};
class Cont : public Cont0 {
public:
    IntVec m14;
};
struct Unk803567AC {
    int m0;
    const void* m4;
    Unk803567AC();
};

/* screen/widget objects whose vptr is at +0x3C; only the slots the code calls are typed */
class VT3C {
public:
    char pad0[0x3C];
    virtual int s1();
    virtual int s2();
    virtual int s3();
    virtual int s4();
    virtual int s5();
    virtual int s6();
    virtual int s7();
    virtual int s8();
    virtual int s9();
    virtual int s10();
    virtual int s11();
    virtual int s12();
    virtual int s13();
    virtual int s14();
    virtual int s15();
    virtual int s16();
    virtual int s17();
    virtual int s18();
    virtual int s19();
    virtual int s20();
    virtual int s21();
    virtual int s22();
    virtual int s23();
    virtual int s24();
    virtual int s25();
    virtual int s26();
    virtual int s27();
    virtual int s28(char* a);
    virtual int s29(int a);
};

/* Small widget seen as plain data: three words, then the vptr at +0xC (ctor 0x800FEC00). */
struct UiWRaw {
    int m0;
    int m4;
    int m8;
    const void* mC;
    UiWRaw();
};

/* Message manager singleton (lbl_8061A32C), 0xD8 bytes, ctor fn_800B64BC; created lazily. */
void* operator new(unsigned int size, int heap);
struct UiMsg {
    char name[16];
};
struct UiMsgMgr {
    char pad0[0xD8];
    UiMsgMgr();
};
extern UiMsgMgr* lbl_8061A32C;
inline UiMsgMgr* UiGetMsgMgr()
{
    if (lbl_8061A32C == 0)
        lbl_8061A32C = new (0) UiMsgMgr;
    return lbl_8061A32C;
}

/* chain of three pointers read by the party-select helpers (fn_803571B4, fn_803571D8) */
struct Unk1CD0 {
    char pad0[0x1CD0];
    int m1CD0;
};
struct Unk14 {
    char pad0[0x14];
    Unk1CD0* m14;
};
struct Unk6F8 {
    char pad0[0x6F8];
    Unk14* m6F8;
};
extern float lbl_805C2CC8;

/* "Action" objects of the menu tables (vtables 0x8065D010..): m4 selects the mode,
 * m8 the target handle that fn_8011BC98 resolves (fn_8035799C). */
struct Unk704 {
    char pad0[0x704];
    int m704;
};
struct Act {
    int m0;
    int m4;
    void* m8;
};
struct Unk8017A930_T;
struct Unk8017DD64_T;
struct Unk80090880;
struct Scr700 : Unk6F8 {
    char pad6FC[4];
    void* m700;
};

/* 24-byte value (six words) copied out of lbl_805EBA10 by the list constructors */
struct Unk805EBA10 {
    int m0, m4, m8, mC, m10, m14;
};
extern Unk805EBA10 lbl_805EBA10;

/* retail data objects */
extern char lbl_805B955C[];
extern char lbl_805BE138[];
extern char lbl_805BEE80[];
extern char lbl_805BEE90[];
extern char lbl_805BEEA0[];
extern char lbl_805BEEAC[];
extern char lbl_805BEEB8[];
extern char lbl_805BEEC4[];
extern char lbl_805BEED4[];
extern char lbl_805C4544[];
extern char lbl_805C4554[];
extern char lbl_805C4570[];
extern char lbl_805C4584[];
extern char lbl_805C465C[];
extern char lbl_805C4668[];
extern char lbl_805C4674[];
extern char lbl_805C4688[];
extern char lbl_805C4698[];
extern char lbl_805C46A8[];
extern char lbl_805C46BC[];
extern char lbl_805C46D0[];
extern char lbl_805C46E8[];
extern char lbl_805C4700[];
extern char lbl_805C4718[];
extern char lbl_805C4730[];
extern char lbl_805C473C[];
extern char lbl_805C4750[];
extern char lbl_805C4770[];
extern char lbl_805C478C[];
extern char lbl_805C47A4[];
extern char lbl_805C47B4[];
extern char lbl_805C47D0[];
extern char lbl_805C47E8[];
extern char lbl_805C480C[];
extern char lbl_805C4830[];
extern char lbl_805C4838[];
extern char lbl_805C484C[];
extern char lbl_805C485C[];
extern char lbl_805C4870[];
extern char lbl_805C4884[];
extern char lbl_805C4898[];
extern char lbl_805C48A8[];
extern char lbl_805C48B8[];
extern char lbl_805C48CC[];
extern char lbl_805C48E4[];
extern char lbl_805C48F0[];
extern char lbl_805C4904[];
extern char lbl_805C4918[];
extern char lbl_805C4924[];
extern char lbl_805C4930[];
extern char lbl_805C4944[];
extern char lbl_805C495C[];
extern char lbl_805C4970[];
extern char lbl_805C4980[];
extern char lbl_805C4990[];
extern char lbl_805C49A0[];
extern char lbl_805C49B4[];
extern char lbl_805C49C4[];
extern char lbl_805C49CC[];
extern char lbl_805C49D4[];
extern char lbl_805C49E0[];
extern char lbl_805C49E8[];
extern char lbl_805C49F4[];
extern char lbl_805C4A00[];
extern char lbl_805C4A08[];
extern char lbl_805C4A14[];
extern char lbl_805C4A24[];
extern char lbl_805C4A38[];
extern char lbl_805C4A44[];
extern char lbl_805C4A54[];
extern char lbl_805C4A64[];
extern char lbl_805C4A6C[];
extern char lbl_805C4A7C[];
extern char lbl_805C4A84[];
extern char lbl_805C4A8C[];
extern char lbl_805C4A94[];
extern char lbl_805C4AA0[];
extern char lbl_805C4AAC[];
extern char lbl_805C5F50[];
extern char lbl_805C5FA8[];
extern char lbl_805C5FAC[];
extern char lbl_805C5FB0[];
extern char lbl_805C5FB4[];
extern char lbl_805CB03C[];
extern char lbl_805F31CC[];
extern char lbl_805F31EC[];
extern char lbl_805F320C[];
extern char lbl_805F321C[];
extern char lbl_8061BC34[];
extern char lbl_8061BC8C[];
extern char lbl_806204F0[];
extern char lbl_80620804[];
extern char lbl_80620808[];
extern char lbl_80649C48[];
extern char lbl_80649DA0[];
extern char lbl_80649DB8[];
extern char lbl_8064A550[];
extern char lbl_8064C028[];
extern char lbl_80650610[];
extern char lbl_80650628[];
extern char lbl_80651628[];
extern char lbl_80655E98[];
extern char lbl_80657308[];
extern char lbl_806573C8[];
extern char lbl_80658188[];
extern char lbl_80658248[];
extern char lbl_80658378[];
extern char lbl_80658D08[];
extern char lbl_806597A0[];
extern char lbl_80659A60[];
extern char lbl_8065A0A8[];
extern char lbl_8065A2B0[];
extern char lbl_8065B690[];
extern char lbl_8065B6A8[];
extern char lbl_8065B9C0[];
extern char lbl_8065BA00[];
extern char lbl_8065BA20[];
extern char lbl_8065BEF0[];
extern char lbl_8065CCE8[];
extern char lbl_8065D010[];
extern char lbl_8065D048[];
extern char lbl_8065D080[];
extern char lbl_8065D0B8[];
extern char lbl_8065D0F0[];
extern char lbl_8065D128[];
extern char lbl_8065D160[];
extern char lbl_8065D198[];
extern char lbl_8065D1D0[];
extern char lbl_8065D240[];
extern char lbl_8065D278[];
extern char lbl_8065D2B0[];
extern char lbl_8065D2E8[];
extern char lbl_8065D320[];
extern char lbl_8065D398[];
extern char lbl_8065D410[];
extern char lbl_8065D430[];
extern char lbl_8065D448[];
extern char lbl_8065D550[];
extern char lbl_8065D5A0[];
extern char lbl_8065D640[];
extern char lbl_8065D690[];
extern char lbl_8065D6E0[];
extern char lbl_8065D730[];
extern char lbl_8065D748[];
extern char lbl_8065D768[];
extern char lbl_8065D780[];
extern char lbl_8065D800[];
extern char lbl_8065D850[];
extern char lbl_8065D868[];
extern char lbl_8065EB00[];
extern char lbl_8065EC68[];
extern char lbl_8065ED60[];
extern char lbl_8065EDB8[];
extern char lbl_8065EEB0[];
extern char lbl_8065EEE8[];
extern char lbl_8065EF20[];
extern char lbl_8065F9B0[];
extern char lbl_80660100[];
extern char lbl_806604C8[];
extern char lbl_806604E0[];
extern char lbl_806604F8[];
extern char lbl_80660620[];
extern char lbl_80660638[];
extern char lbl_806662C8[];
extern char lbl_80666888[];
extern char lbl_806669C8[];
extern char lbl_80666A18[];
extern char lbl_80666DD0[];
extern char lbl_80666E20[];
extern char lbl_80666E70[];
extern char lbl_806841F8[];
extern char lbl_80691BEC[];
extern char lbl_80691CAC[];
extern char lbl_80691CB0[];
extern char lbl_80691CB4[];
extern char lbl_80691CB8[];
extern char lbl_80691CBC[];
extern char lbl_80691CC0[];
extern char lbl_80691CC4[];
extern char lbl_80691CDC[];
extern char lbl_80691CE0[];
extern char lbl_80691CE4[];
extern char lbl_80691CE8[];
extern char lbl_80691CEC[];
extern char lbl_80691CF0[];
extern char lbl_80691CF4[];
extern char lbl_80691D14[];


/* callees outside this range */
extern Unk80090880* lbl_8061977C;
extern Unk803442CC lbl_8068B87C;
extern float lbl_805CADB8;
extern float lbl_805CADBC;
extern int lbl_8062087C;
extern int lbl_8068B880;
void* fn_80090880(Unk80090880*, int);
extern "C" void fn_800AB308(void*, int);
extern "C" void fn_800B62FC(void*, int);
extern "C" void fn_800B7B4C(UiMsgMgr*, UiMsg*, void*);
extern "C" void fn_800C6808(void*, int);
extern "C" void fn_800D0D64(void*, int);
extern "C" void fn_800D0E24(void*, int);
extern "C" void fn_800D3A38(void*, int);
extern "C" void fn_800DE274(void*, void*, int);
extern "C" void fn_800EC704(void*, int);
extern "C" void fn_800ECF18(void*);
void fn_800FEB30(const char*, int);
int fn_800FF590(UiPtr0*);
extern "C" Unk704* fn_8011BC98(void*);
extern "C" int fn_8011BD0C(void*, int);
extern "C" int fn_8011BD3C(void*, int);
extern "C" int fn_8011BD94(void*, int);
extern "C" int fn_8011BE1C(void*, int);
extern "C" int fn_8011BF1C(void*, int);
extern "C" void fn_80134EC8(void*, int);
extern "C" void fn_8014D9C4(void*);
extern "C" void fn_80151048(void*, int);
void fn_8017045C();
void fn_80170488();
void fn_801704C0();
int fn_8017C974(Unk8017A930_T*);
void fn_8017CA14(Unk8017A930_T*, int);
extern "C" void fn_8017CD04(void*, int, int);
extern "C" void fn_8017D378(void*, int);
void fn_8017D640(Unk8017A930_T*, int, char*, int, int);
int fn_8017DD64(Unk8017DD64_T*, int);
extern "C" void fn_8017DF2C(void*, void*);
extern "C" int fn_801C1638(void*, int);
extern "C" void fn_802B9300(void*, int, int);
extern "C" void fn_802B96C4(void*, int);
extern "C" void fn_80328374(void*, int);
extern "C" void* fn_8032F19C(void*, const char*, void*, int);
extern "C" void fn_80330778(void*, int);
extern "C" void fn_80330ACC(void*, void*);
extern "C" void fn_8033CD28(void*, int);
extern "C" void fn_8033D14C(float, float, float, float, float);
Unk803442CC* fn_803445A8(Unk803442CC*);
extern "C" void fn_80377408(void*);
extern "C" void fn_80377488(void*);
extern "C" void fn_803785E4(void*);
extern "C" void fn_80378620(void*, int);
extern "C" void fn_8037936C(void*, void*);
extern "C" void fn_8037B424(void*, int);
extern "C" void fn_803DF74C(void*);
extern "C" int fn_803ED5E0(const void*, int, int, int, int);
extern "C" void MEM_free(int);
extern "C" void THREAD_createparam(void*, void (*)(void*), void*, int, int, int);
extern "C" void THREAD_destroy(void*);
extern "C" void THREAD_yield(int);
extern "C" void THREAD_setpriority(void*, int);
extern "C" void THREAD_waitexit(void*, int);
extern "C" int fn_804007D4(void*);
extern "C" void fn_804007FC(void*);
extern "C" void fn_80400828(void*);
extern "C" void fn_8040084C(void*);
extern "C" int fn_80403DE4(char*, const void*, ...);
extern "C" int fn_80409CF0(void (*)());
extern "C" void fn_8040B190(void*, const void*, int);
extern "C" void* fn_8040B320(void*, int, int, ...);
extern "C" char* fn_8040B61C(char*, const char*);
extern "C" char* fn_8040B830(char*, const char*, unsigned);
struct Unk54;

struct Unk803579D4 {
    int m0;
    int m4;
    int m8;
    int* mC;
    int* m10;
    int* m14;
};

struct Unk803596DC_V {
    int m0;
    int m4;
    int m8;
    void Clear() { m0 = 0; m4 = 0; m8 = 0; }
};
struct Unk803596DC {
    char m0;
    char pad1[0x3B];
    int m3C;
    int m40;
    Unk803596DC_V m44;
    Unk805EBA10 m50;
    Unk805EBA10 m68;
    Unk805EBA10 m80;
    int m98;
    int m9C;
    void Init()
    {
        m0 = 0;
        m3C = 0;
        m40 = 0;
        m44.Clear();
        m50 = lbl_805EBA10;
        m68 = lbl_805EBA10;
        m80 = lbl_805EBA10;
        m98 = 0;
        m9C = 0xae;
    }
};
struct Unk803597C4 {
    Unk803596DC m0[8];
};
inline void* operator new(unsigned int, void* p) { return p; }
void* fn_8035A148(void*);
/* reference-counted singleton of 0x24 bytes: vptr (data object lbl_8065EDB8), 0x1C bytes at +4,
 * count at +0x20 (fn_80359C58, fn_80359CF8, fn_80359D78, fn_80359D8C). */
struct Unk80359C58 {
    const void* m0;
    char m4[0x1C];
    int m20;
    Unk80359C58()
    {
        fn_8035A148(this);
        m0 = lbl_8065EDB8;
        fn_8040B320(m4, 0, 0x1C);
        m20 = 1;
        ((void* (*)(void*, int, int))fn_8040B320)(m4, 0, 0x1C);
        fn_804007D4(m4);
    }
};
class Unk80359D8C_VT {
public:
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual ~Unk80359D8C_VT();
};
class Unk80359FB0_VT {
public:
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual ~Unk80359FB0_VT();
};
extern Unk80359C58 lbl_8068B884;
extern int lbl_8068B8A8;
/* Thread object (0x334 bytes). Evidence: ctor fn_80359F30/fn_80359E94 (vptr lbl_8065ED60, m4 = 1, mC = 0x19000,
   memset of m14[0x318]), dtor fn_80359E30 (m330 flag, THREAD_destroy(m14)), setters fn_8035A028/fn_8035A09C. */
void* fn_8035A134(void*);
struct Unk80359E94 {
    const void* m0;
    int m4;
    int m8;
    int mC;
    int m10;
    char m14[0x318];
    int m32C;
    int m330;
    Unk80359E94()
    {
        fn_8035A134(this);
        m0 = lbl_8065ED60;
        m4 = 1;
        mC = 0x19000;
        m10 = 0;
        fn_8040B320(m14, 0, 0x318);
        m32C = 0;
        m330 = 0;
    }
};
extern Unk80359E94 lbl_8068B8AC;
extern int lbl_8068BBE0;
struct Unk80359A40 {
    int m0;
    int m4;
    float m8;
    int mC;
    int m10;
};
struct Unk806055AC {
    int m0;
    int m4;
    int m8;
};
extern Unk806055AC lbl_806055AC;
struct Unk80379324_V4 {
    float x, y, z, w;
};
extern Unk80379324_V4 lbl_8069429C;
extern float lbl_805CFFE4;
extern float lbl_805CFFE8;
extern float lbl_805CFFEC;
struct Unk8035A618_S {
    int m0;
    char pad4[0x5C];
    float m60;
    float m64;
    float m68;
    int m6C;
    float m70;
    float m74;
    float m78;
    Unk80379324_V4 m7C;
    float m8C;
    float m90;
    float m94;
};
struct Unk8035A900_E {
    Unk806055AC m0;
    Unk806055AC mC;
    int m18;
};
struct Unk8035A900 {
    int cnt;
    int m4;
    Unk8035A900_E* m8;
};
extern float lbl_805CFFF4;
extern float lbl_805D30A4;
struct Unk8035ABB0 {
    float m0;
    Unk806055AC m4;
    Unk80379324_V4 m10;
};
struct Unk8035AC10 {
    char pad0[0x10];
    Unk806055AC m10;
    int m1C;
};
struct Unk8035B1C8 {
    int m0;
    float m4;
    int m8;
    float mC;
    float m10;
    int m14;
    int m18;
    int m1C;
    int m20;
    float m24;
};
extern const float lbl_805C5FB8;
struct Unk8060558C {
    int m0;
    int m4;
};
extern Unk8060558C lbl_8060558C;
extern float lbl_805D30A8;
extern float lbl_805D30AC;

/* functions of this range */
void* fn_80355D08(void*);
void* fn_80355D44(void*);
void* fn_80355D80(void*);
void* fn_80355DBC(void*);
void* fn_80355DF8(void*);
void* fn_80355E34(void*);
void* fn_80355E70(void*);
void* fn_80355EAC(void*);
void* fn_80355EE8(void*);
void* fn_80355F24(void*);
void* fn_80355F60(void*);
void* fn_80355F9C(void*);
void fn_80355FD8(void*, int);
void fn_80355FF8(void*, int);
void fn_80356018(void*, int);
void fn_80356038(void*, int);
void fn_80356058(void*, int);
void fn_80356078(void*, int);
void fn_80356098(void*, int);
void fn_803560B8(void*, int);
void fn_803560D8(void*, int);
void fn_803560F8(void*, int);
void fn_80356130(void*, int);
void fn_80356150(void*, int);
void fn_80356170(void*, int);
void fn_80356190(void*, int);
void fn_803561B0(void*, int);
void fn_803561D0(void*, int);
void fn_803561F0(void*, int);
void fn_80356278(void*, int);
void fn_803562AC(void*, int);
void fn_803562CC(void*, int);
void fn_803562EC(void*, int);
void fn_8035630C(void*, int);
void fn_8035632C(void*, int);
void fn_8035634C(void*, int);
void fn_8035636C(void*, int);
void fn_8035638C(void*, int);
void fn_803563AC(void*, int);
void fn_803563CC(void*, int);
void fn_803563EC(void*, int);
void fn_8035640C(void*, int);
void fn_8035642C(void*, int);
void fn_8035644C(void*, int);
void fn_8035646C(void*, int);
void fn_8035649C(void*, int);
void fn_803564BC(void*, int);
void fn_803564DC(void*, int);
void fn_803564FC(void*, int);
void fn_80356578(void*, int);
void fn_80356598(void*, int);
void fn_803565B8(void*, int);
void fn_803565D8(void*, int);
void fn_803565F8(void*, int);
void fn_80356618(void*, int);
void fn_80356638(void*, int);
void fn_80356658(void*, int);
void fn_80356678(void*, int);
void fn_80356698(void*, int);
void fn_803566B8(void*, int);
void fn_803566D8(void*, int);
void fn_803566F8(void*, int);
void fn_80356718(void*, int);
void fn_80356738(void*, int);
void fn_80356758(void*, int);
void fn_80356778(void*, int);
extern "C" void* fn_80356798(void*);
void fn_803567C8(void*, int);
extern "C" void fn_8035682C(char* p);
extern "C" void fn_80356838(void*, int);
extern "C" void fn_8035686C(void*, int);
void fn_80356898(void*, int);
int fn_803568A0(void*);
void fn_803568A8();
void fn_803568AC();
void fn_803568B0(void*, int);
void fn_80356950(void*, int);
void fn_803569A4(void*, int);
void fn_80356A1C(void*, int);
void fn_80356A50();
void fn_80356A54();
void fn_80356A58();
void fn_80356A5C();
void fn_80356A60(void*, int);
void fn_80356A94(void*, int);
void fn_80356AC8(void*, int);
void fn_80356AFC(void*, int);
void fn_80356B30(void*, int);
void fn_80356B64(void*, int);
void fn_80356B98(void*, int);
int fn_80356BC8();
void fn_80356BD0(void*, int);
void fn_80356C04(void*, int);
void fn_80356C34(void*, int);
int fn_80356C98(void*);
void fn_80356CA0(void*, int);
void fn_80356CC0(void*, int);
extern "C" void fn_80356CE0(void*, int);
void fn_80356D14(void*, int);
void fn_80356D8C(void*, int);
void fn_80356E04(void*, int);
void fn_80356E38(void*, int);
int fn_80356E68();
int fn_80356E70();
int fn_80356E78();
int fn_80356E80();
void fn_80356F30(void*, int);
void fn_80356F68(void*, int);
void fn_80356F9C(void*, int);
void fn_80356FBC(void*, int);
void* fn_8035709C(void*);
int fn_80357104();
void fn_8035710C(void* self);
int fn_8035719C();
int fn_803571A4();
int fn_803571AC();
void fn_80357204(void*, int, int);
int fn_80357230(void*, VT3C*);
void fn_80357474(void*, int, int);
int fn_803574A0(void*, VT3C*);
void fn_80357514(void*, int, int);
int fn_80357540(void*, VT3C*);
void fn_8035757C(void*, void*, int);
int fn_803575E8(void*, VT3C*);
void fn_80357648(void*, int, int);
int fn_80357674(void*, VT3C*);
void fn_803576B0(void*, void*, int);
void fn_803576D8(void*, int, int);
int fn_80357704(void*, VT3C*);
void fn_80357740(void*, void*, int);
void fn_80357768(void*, int, int);
int fn_80357794(void*, VT3C*);
void fn_803577D0(void*, int, int);
int fn_803577FC(void*, VT3C*);
void fn_80357838(void*, void*, int);
void fn_80357860(void*, int, int);
int fn_8035788C(void*, VT3C*);
void fn_803578C8(void*, void*, int);
int fn_80357938(void*, VT3C*);
Unk704* fn_8035799C(Act* self);
void* fn_803579C0(void*);
extern "C" void fn_803579D4(Unk803579D4* o);
extern "C" void* fn_80357A5C(void*);
void* fn_80357A70(void*);
void fn_80357AAC(void*, int);
void fn_80357ADC();
void fn_80357AE0();
void fn_80357AE4();
void fn_80357AE8();
void* fn_80357AEC(void*);
void fn_80357B28(void*, int);
extern "C" void fn_80358484(void*, int);
void fn_803584A4(void*, int);
char* fn_803584E8(char*);
int fn_803584F0();
int fn_803584F8();
int fn_80358500();
void fn_80358508(void*, int);
int fn_80358510(void*);
int fn_80358518();
int fn_80358568();
int fn_80358570();
int fn_80358578();
int fn_80358580();
void* fn_80358588(void*);
void* fn_8035859C(void*);
void fn_8035864C(void*, int);
void fn_80358868(void*, int);
void fn_80358F54(void*, int);
void fn_80358F84(void* self);
void fn_8035929C(void*, int);
void fn_803592CC(void* self);
void fn_803595C4(void*, int);
void fn_803595E4(void*, int);
extern "C" void fn_80359604(void*, int);
extern "C" void fn_803596DC(Unk803596DC* s);
extern "C" void fn_80359910(int* self);
extern "C" void fn_80359948(void* p, int v);
void fn_803599C0(void*, int);
void fn_803599F4();
extern "C" void fn_80359A40(Unk80359A40* s);
void fn_80359A64(void*, int);
void fn_80359A98(void*, int);
void fn_80359B04(char* self);
void fn_80359B3C(int* p);
void fn_80359B48(void*, int);
extern "C" void fn_80359B78(void*, int);
extern "C" Unk803442CC* fn_80359BAC();
extern "C" Unk80359C58* fn_80359C58();
void fn_8035A020(void*, int);
extern "C" void fn_8035A09C(char* self, int unused);
void* fn_8035A134(void*);
void* fn_8035A148(void*);
Unk803442CC* fn_8035A15C(Unk803442CC* self);
void fn_8035A198(void* t);
int fn_8035A1F4();
int fn_8035A1FC();
int fn_8035A204();
int fn_8035A20C();
int fn_8035A2B0();
int fn_8035A2B8();
int fn_8035A2C0(void*);
int fn_8035A2C8(void*);
void fn_8035A2D0(void*, int);
extern "C" void fn_8035A304(void*, int);
extern "C" void fn_8035A324(void*, int);
extern "C" void fn_8035A344(void*, int);
void fn_8035A364(void*, int);
void fn_8035A384(void*, int);
void fn_8035A3A4(void*, int);
void fn_8035A3C4(void*, int);
void fn_8035A3E4(void*, int);
void fn_8035A404(void*, int);
void fn_8035A424(void*, int);
void fn_8035A444(void*, int);
void fn_8035A464(void*, int);
void fn_8035A484(void*, int);
void fn_8035A4A4(void*, int);
void fn_8035A4C4(void*, int);
void fn_8035A4E4(void*, int);
void fn_8035A504(void*, int);
void fn_8035A524(void*, int);
void fn_8035A544(void*, int);
void fn_8035A564(void*, int);
void fn_8035A584(void*, int);
extern "C" void fn_8035A5A4(void*, int);
void fn_8035A5C4(void*, int);
extern "C" void fn_8035A618(Unk8035A618_S* s);
void fn_8035A688(void*, int);
void fn_8035A7D8(void*, int);
extern "C" void fn_8035A900(Unk8035A900* s);
void fn_8035A994(void*, int);
void fn_8035A9C8(void*, int);
void fn_8035A9FC(void*, int);
void fn_8035AA30(void*, int);
void fn_8035AA64(void*, int);
void fn_8035AA98(void*, int);
void fn_8035AACC(void*, int);
extern "C" void* fn_8035AB00(void*);
void fn_8035AB14(void*, int);
void fn_8035AB44(void*, int);
extern "C" void fn_8035ABB0(Unk8035ABB0* s);
void* fn_8035AC10(Unk8035AC10* s, Unk8035AC10* o);
void fn_8035AD80();
void fn_8035AD84();
void fn_8035AD88(void*, int);
void fn_8035ADBC(void*, int);
void fn_8035ADF0(void*, int);
void fn_8035AE24(void*, int);
void fn_8035AE58(void*, int);
void fn_8035AE8C(void*, int);
void fn_8035AEC0(void*, int);
void fn_8035AEF4(void*, int);
int fn_8035AF28(void*);
void fn_8035AF30(void*, int);
void fn_8035AF64(void*, int);
void fn_8035AF98(void*, int);
void fn_8035AFCC(void*, int);
void fn_8035B000(void*, int);
void fn_8035B034(void*, int);
void fn_8035B068(void*, int);
void fn_8035B09C(void*, int);
void fn_8035B0D0(void*, int);
void fn_8035B104(void*, int);
void fn_8035B138(void*, int);
extern "C" void fn_8035B16C(char* self, int c);
extern "C" void fn_8035B1C8(Unk8035B1C8* s);
extern "C" int fn_8035B3BC(void*);
void fn_8035B404(void*, int);
void fn_8035B438(void*, int);
void fn_8035B46C(void*, int);
void fn_8035B4A0(void*, int);
void fn_8035B4D4(void*, int);
void fn_8035B508(void*, int);
extern "C" void* fn_8035B53C(char* self);
void fn_8035B570(void*, int);
void fn_8035B5A4(void*, int);
void fn_8035B5D8(void*, int);

#endif
