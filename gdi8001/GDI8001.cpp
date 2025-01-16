// GDI8001.cpp : アプリケーションのエントリ ポイントを定義します。
//

#pragma warning(disable : 4996)

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include "framework.h"
#include "GDI8001.h"
#include "Z80.h"

#include <stdlib.h>
#include <time.h>

//#include <shobjidl_core.h>
#include <CommCtrl.h>

extern bool is8mhz;
extern unsigned char crtcactive;

bool ispc80threadinrunningemulation = false;
bool isbeepenabledinthecool = false;
bool isbeepenabledinthecool2 = false;
UINT64 clockcount4beep = 0;
UINT64 clockcount4beepprev = 0;
UINT64 samplebase4beep = 44100*2000;
int samplebase4beeptiff = 0;
int howmanybeepstopped = 0;
int howmanybeepstopped_2 = 0;

extern bool beepenabled;
extern bool beepenabled2;

HWND HWNDfullscr;

bool isbeepplayed = false;
bool isbeepplayed2 = false;
bool bool4showwin = true;

//#define SRATE    44100    //標本化周波数(1秒間のサンプル数)
#define SRATE    55467    //標本化周波数(1秒間のサンプル数)
#define F        2400     //周波数(1秒間の波形数)

WAVEFORMATEX wfe;
static HWAVEOUT hWaveOut;
static WAVEHDR whdr;
static LPBYTE lpWave;
int i, len;

WAVEFORMATEX wfe_2;
static HWAVEOUT hWaveOut_2;
static WAVEHDR whdr_2;
static LPBYTE lpWave_2;
int i_2, len_2;

void beepinit() {

    wfe.wFormatTag = WAVE_FORMAT_PCM;
    wfe.nChannels = 2;    //ステレオ
    wfe.wBitsPerSample = 8;    //量子化ビット数
    wfe.nBlockAlign = wfe.nChannels * wfe.wBitsPerSample / 8;
    wfe.nSamplesPerSec = SRATE;    //標本化周波数
    wfe.nAvgBytesPerSec = wfe.nSamplesPerSec * wfe.nBlockAlign;

    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfe, 0, 0, CALLBACK_NULL);

    lpWave = (LPBYTE)calloc(wfe.nAvgBytesPerSec, 4);    //2秒分

    len = SRATE / F;    //波長
    for (i = 0; i < SRATE * 2; i++) {  //波形データ作成
        if (i % len < len / 2)    lpWave[(i * 2) + 0] = 128 + 64;
        else                 lpWave[(i * 2) + 0] = 128 - 64;
    }
    len_2 = SRATE / F;    //波長
    for (i = 0; i < SRATE * 2; i++) {  //波形データ作成
        lpWave[(i * 2) + 1] = 0;
    }

    whdr.lpData = (LPSTR)lpWave;
    whdr.dwBufferLength = wfe.nAvgBytesPerSec * 4;
    whdr.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
    whdr.dwLoops = -1;

    waveOutPrepareHeader(hWaveOut, &whdr, sizeof(WAVEHDR));

    wfe_2.wFormatTag = WAVE_FORMAT_PCM;
    wfe_2.nChannels = 2;    //ステレオ
    wfe_2.wBitsPerSample = 8;    //量子化ビット数
    wfe_2.nBlockAlign = wfe_2.nChannels * wfe_2.wBitsPerSample / 8;
    wfe_2.nSamplesPerSec = SRATE;    //標本化周波数
    wfe_2.nAvgBytesPerSec = wfe_2.nSamplesPerSec * wfe_2.nBlockAlign;

    waveOutOpen(&hWaveOut_2, WAVE_MAPPER, &wfe_2, 0, 0, CALLBACK_NULL);

    lpWave_2 = (LPBYTE)calloc(wfe_2.nAvgBytesPerSec, 4);    //2秒分

    len = SRATE / F;    //波長
    for (i_2 = 0; i_2 < SRATE * 2; i_2++) {  //波形データ作成
        lpWave_2[(i_2 * 2) + 0] = 0;
    }
    len_2 = SRATE / F;    //波長
    for (i_2 = 0; i_2 < SRATE * 2; i_2++) {  //波形データ作成
        lpWave_2[(i_2 * 2) + 1] = 0;
    }

    whdr_2.lpData = (LPSTR)lpWave_2;
    whdr_2.dwBufferLength = wfe_2.nAvgBytesPerSec * 4;
    whdr_2.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
    whdr_2.dwLoops = -1;

    waveOutPrepareHeader(hWaveOut_2, &whdr_2, sizeof(WAVEHDR));
}

void beeprestart() {
    if (isbeepplayed == true) {
        waveOutReset(hWaveOut);
        isbeepplayed = false;
    }
    waveOutUnprepareHeader(hWaveOut, &whdr, sizeof(WAVEHDR));

    //len = ((samplebase4beep / F) / (2000 +(((howmanybeepstopped>0) ? (18 * howmanybeepstopped) : 0))));    //波長
    len = (SRATE / F) + ((howmanybeepstopped > 0) ? ((((is8mhz ? 2 : 1) * (crtcactive ? 1830000 : 4000000)) * howmanybeepstopped) / 1000000) : 0);    //波長
    if (len <= 0) { len = SRATE / F; }
    for (i = 0; i < SRATE * 2; i++) {  //波形データ作成
        if (i % len < len / 2)    lpWave[(i * 2) + 0] = 128 + 64;
        else                 lpWave[(i * 2) + 0] = 128 - 64;
    }
    if (isbeepenabledinthecool2 == true) {
        len_2 = ((howmanybeepstopped_2 > 0) ? (1 * howmanybeepstopped_2) : 0);    //波長
        if (len_2 <= 0) { len_2 = SRATE / 1; }
        for (i_2 = 0; i_2 < SRATE * 2; i_2++) {  //波形データ作成
            if (i_2 % len_2 < len_2 / 2)    lpWave[(i_2 * 2) + 1] = 128 + 64;
            else                 lpWave[(i_2 * 2) + 1] = 128 - 64;
        }
    }
    else {
        len_2 = 1;
        for (i_2 = 0; i_2 < SRATE * 2; i_2++) {  //波形データ作成
            lpWave[(i_2 * 2) + 1] = 0;
        }
    }

    whdr.lpData = (LPSTR)lpWave;
    whdr.dwBufferLength = wfe.nAvgBytesPerSec * 4;
    whdr.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
    whdr.dwLoops = -1;

    waveOutPrepareHeader(hWaveOut, &whdr, sizeof(WAVEHDR));
}

void beep2restart() {
    if (isbeepplayed2 == true) {
        waveOutReset(hWaveOut_2);
        isbeepplayed2 = false;
    }
    if (howmanybeepstopped_2 == 0) { return; }
    waveOutUnprepareHeader(hWaveOut_2, &whdr_2, sizeof(WAVEHDR));

    if (isbeepenabledinthecool == true) {
        len = (SRATE / F) + ((howmanybeepstopped > 0) ? ((((is8mhz ? 2 : 1) * (crtcactive ? 1830000 : 4000000)) * howmanybeepstopped) / 1000000) : 0);    //波長
        if (len <= 0) { len = SRATE / F; }
        for (i = 0; i < SRATE * 2; i++) {  //波形データ作成
            if (i % len < len / 2)    lpWave_2[(i * 2) + 0] = 128 + 64;
            else                 lpWave_2[(i * 2) + 0] = 128 - 64;
        }
    }
    else {
        len = 1;
        for (i = 0; i < SRATE * 2; i++) {  //波形データ作成
            lpWave_2[(i * 2) + 0] = 0;
        }
    }
    len_2 = ((howmanybeepstopped_2 > 0) ? (1 * howmanybeepstopped_2) : 0);    //波長
    if (len_2 <= 0) { len_2 = SRATE / 1; }
    for (i_2 = 0; i_2 < SRATE * 2; i_2++) {  //波形データ作成
        if (i_2 % len_2 < len_2 / 2)    lpWave_2[(i_2 * 2) + 1] = 128 + 64;
        else                 lpWave_2[(i_2 * 2) + 1] = 128 - 64;
    }

    whdr_2.lpData = (LPSTR)lpWave_2;
    whdr_2.dwBufferLength = wfe_2.nAvgBytesPerSec * 4;
    whdr_2.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
    whdr_2.dwLoops = -1;

    waveOutPrepareHeader(hWaveOut_2, &whdr_2, sizeof(WAVEHDR));
}

void beep2400play() {
    if (isbeepplayed2 == true) {
        waveOutReset(hWaveOut_2);
        isbeepplayed2 = false;
    }
    if (isbeepplayed == false) {
        waveOutWrite(hWaveOut, &whdr, sizeof(WAVEHDR));
        isbeepplayed = true;
    }
}

void beep2400stop(){
    //if ((clockcount4beep - clockcount4beepprev) > F) { samplebase4beep = (((clockcount4beep - clockcount4beepprev) * ((UINT64)(SRATE))) / ((UINT64)((is8mhz ? 2 : 1) * (crtcactive ? (UINT64)1830000 : (UINT64)4000000)))) + samplebase4beeptiff; samplebase4beeptiff = samplebase4beep % F; } clockcount4beepprev = clockcount4beep;
    if (isbeepplayed == true) {
        waveOutReset(hWaveOut);
        isbeepplayed = false;
    }
    if (beepenabled2 == true) {
        if (isbeepplayed2 == false) {
            waveOutWrite(hWaveOut_2, &whdr_2, sizeof(WAVEHDR));
            isbeepplayed2 = true;
        }
    }
}
void beep2play() {
    if (howmanybeepstopped_2 == 0) { return; }
    if (isbeepplayed == true) {
        waveOutReset(hWaveOut);
        isbeepplayed = false;
    }
    if (isbeepplayed2 == false) {
        waveOutWrite(hWaveOut_2, &whdr_2, sizeof(WAVEHDR));
        isbeepplayed2 = true;
    }
}

void beep2stop() {
    if (isbeepplayed2 == true) {
        waveOutReset(hWaveOut_2);
        isbeepplayed2 = false;
    }
    if (beepenabled == true) {
        if (isbeepplayed == false) {
            waveOutWrite(hWaveOut, &whdr, sizeof(WAVEHDR));
            isbeepplayed = true;
        }
    }
}

FILE* cmtfile;
char FileName[MAX_PATH * 2];
char FileNameoffd[MAX_PATH * 2];

int cmtseek = 0;

wchar_t FilterW[4096];
wchar_t FileNameW[4096];

//------------------------------------------------------
//■関数 OpenDiaog
//■用途 「ファイルを開く」ダイアログを表示する
//■引数
//        hwnd       ...親ウインドウのハンドル
//        Filter     ...フィルター
//        FileName   ...ファイルのフルパス名(戻り値)
//        Flags      ...ダイアログのフラグ
//■戻り値
//      ファイルを選択  true   
//------------------------------------------------------
int OpenDiaog(HWND hwnd, LPCSTR Filter, char* FileName, DWORD Flags)
{
    OPENFILENAMEA OFN;

    ZeroMemory(&OFN, sizeof(OPENFILENAMEA));
    OFN.lStructSize = sizeof(OPENFILENAMEA);
    OFN.hwndOwner = hwnd;
    OFN.lpstrFilter = Filter;
    OFN.lpstrFile = FileName;
    OFN.nMaxFile = MAX_PATH * 2;
    OFN.Flags = Flags;
    OFN.lpstrTitle = "ファイルを開く";
    return (GetOpenFileNameA(&OFN));
}

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

#pragma warning(disable : 4996)

#if 0
#ifdef _ARM_
#pragma comment(lib,"bz80dll_arm.lib")
#else
#ifdef _M_ARM64
#pragma comment(lib,"bz80dll_arm64.lib")
#else
#ifdef _M_AMD64
#pragma comment(lib,"bz80dll_x64.lib")
#else
#pragma comment(lib,"bz80dll.lib")
#endif
#endif
#endif
extern "C" __declspec(dllimport) void setz80memaccess(int (*tmp)(int, int, int));
extern "C" __declspec(dllimport) void Z80Init(void);
extern "C" __declspec(dllimport) void Z80Reset(void);
extern "C" __declspec(dllimport) int  Z80Run(void);
extern "C" __declspec(dllimport) void Z80DoIRQ(uint8 vector);
extern "C" __declspec(dllimport) void Z80DoNMI(void);
extern "C" __declspec(dllimport) int getz80regs();
extern "C" __declspec(dllimport) int getextz80regs(int tmp);
#endif

extern UINT32 clockcount;

int (*GN80memaccess)(int, int, int);
class GocaineN80 : public Z80 {
public:
    int32_t load(uint16_t adr) { return GN80memaccess(adr, 0, 1); }
    int32_t loadpc(uint16_t adr) { return GN80memaccess(adr, 0, 1); }
    void store(uint16_t adr, uint8_t data) { GN80memaccess(adr, data, 0); }
    int32_t input(uint16_t adr) { return GN80memaccess(adr, 0, 3); }
    void output(uint16_t adr, uint8_t data) { clockcount4beep = ((UINT64)clockcount + (UINT64)clock); GN80memaccess(adr, data, 2); }
};

int (*GN8012memaccess)(int, int, int);
class GocaineN8012 : public Z80 {
public:
    int32_t load(uint16_t adr) { return GN8012memaccess(adr, 0, 1); }
    int32_t loadpc(uint16_t adr) { return GN8012memaccess(adr, 0, 1); }
    void store(uint16_t adr, uint8_t data) { GN8012memaccess(adr, data, 0); }
    int32_t input(uint16_t adr) { return GN8012memaccess(adr, 0, 3); }
    void output(uint16_t adr, uint8_t data) { GN8012memaccess(adr, data, 2); }
};

GocaineN80 GN80;
GocaineN8012 GN8012;

HMODULE bz80dll = 0;

void (*bz80_setz80memaccess)(int (*tmp)(int, int, int));
void (*bz80_Z80Init)(void);
void (*bz80_Z80Reset)(void);
int  (*bz80_Z80Run)(void);
void (*bz80_Z80DoIRQ)(uint8 vector);
void (*bz80_Z80DoNMI)(void);
int (*bz80_getz80regs)();
int (*bz80_getextz80regs)(int tmp);

extern "C" void setz80memaccess(int (*tmp)(int, int, int)) { if (bz80dll == 0) { GN80memaccess = tmp; } else { bz80_setz80memaccess(tmp); } }
extern "C" void Z80Init(void) { if (bz80dll == 0) { GN80.Reset(); } else { bz80_Z80Init(); } }
extern "C" void Z80Reset(void) { if (bz80dll == 0) { GN80.Reset(); } else { bz80_Z80Reset(); } }
extern "C" int  Z80Run(void) { if (bz80dll == 0) { return (GN80.Execute(1) + 1); } else { return bz80_Z80Run(); } }
extern "C" void Z80DoIRQ(uint8 vector) { if (bz80dll == 0) { GN80.INT(vector); } else { bz80_Z80DoIRQ(vector); } }
extern "C" void Z80DoNMI(void) { if (bz80dll == 0) { GN80.NMI(); } else { bz80_Z80DoNMI(); } }
extern "C" int getz80regs() { if (bz80dll == 0) { return 0; } else { return bz80_getz80regs(); } }
extern "C" int getextz80regs(int tmp) { if (bz80dll == 0) { return 0; } else { return bz80_getextz80regs(tmp); } }

time_t timer;
struct tm local_time;

bool biosromenabled=false;

uint8 bios[0x6000];
uint8 memory[0x10000];
uint8 fontrom[0x800];
uint8 gvram[3][0x4000];
uint8 n80rom[0x2000];
uint8 n88rom[0x8000];
uint8 fastestvram[0x1000];
uint8 kanjirom1[0x20000];
uint8 kanjirom2[0x20000];
uint16 kanjiromaddr1 = 0;
uint16 kanjiromaddr2 = 0;
uint8 erom[8][4][8192];
uint8 dicrom[0x80000];

uint8 dictromstat[2] = { 0,0xFF };

uint8 pc8001keybool[0x10];

uint8 showstatefor88grp = 0;

uint8 attributesize = 20;
bool ispc8801 = false;
bool rommode = false;
bool hiresgrpresol200 = false;
bool fastesttvramenabled = false;
uint8 eromsl = 0;
uint8 videooutputmode = 0;
bool palettemode = false;
bool gvramaccessmode = false;
bool soundintmask = false;
uint8 galuctrl = 0;
uint8 galuop = 0;
uint8 extendedromsel = 0xff;
uint8 textwindoffsetadru8 = 0;
uint8 palette512_8bt[8][4];

HANDLE Z80Threadid = 0;
HANDLE BSThreadid = 0;
HANDLE BS2Threadid = 0;
HANDLE BGThreadid = 0;
HANDLE RTIThreadid = 0;
HANDLE SERThreadid = 0;
HANDLE SERWThreadid = 0;
HANDLE FDDCZ80Threadid = 0;
UINT32 clockcount = 0;
int clockcountpc8012 = 0;
bool videoenabled = false;
bool beepenabled = true;
bool beepenabled2 = false;
uint8 uPD8251config[4];
uint8 upd8251configate = 0;
bool overrunerror = false;
bool rxdataready = false;
uint8 crtc2 = 0;

uint8 bgcolor = 0;
bool colorgraphicmode = false;
bool colorfullgraphicmode = false;
bool graphicdraw = true;
bool fullgraphicdraw = false;
uint8 gvramenabled = 0;
bool grpmode = false;
bool fullgrpmode = false;
bool romtype = false;

uint8 uipin = 0;
bool vbi = false;
bool rtcdata = false;
bool fddconnected = false;
bool cmtdatard = false;
bool prtready = false;

bool othercrtcio = false;
int upd31speclzsig;
int litepeninp;

uint8 upd3301stat = 0;
uint8 uPD3301prm = 0;
uint8 upd3301cmd = 0;

uint16 dmaas[4];
uint16 dmatc[4];
uint8 dmachiocnt = 0;
uint8 dmamodestat = 0;
uint8 dmaseq = 0;
uint8 crtcactive = 0;
int cursx = -1;
int cursy = -1;

bool grpheight25 = false;
uint8 blinkingtime = 0;
uint8 cursxtmp = 0;

uint8 uopout = 0;

uint8 ret = 0;

uint8 seq = 0;

uint8 crtc3 = 0;
uint8 bsmode = 0;

uint8 intmasklevel = 0xf;

HDC hdc = 0;
HDC hdcfullscr = 0;
HBITMAP hOldCBitmap;

uint8 cursortype = 0;
bool linespace = false;

int pc8001kb1p;

int chkedbb8 = 0;

bool crtcldsclkenable = false;
bool rtcclkenable = false;
bool rtcstbenable = false;
bool prtenable = false;
bool prtenable0 = false;

bool cassettemtstate = false;
uint8 cmtbinsnd = 0;
bool cmtdatawr = false;
bool crtmodectrl = false;
bool pc8001widthflag = false;

bool isharftoneenabled = false;

bool n80_8000 = false;

bool ttyconnected = false;

tm* timexforch1;
tm timezforch1;
time_t timerforch123;
uint8 rtctimeforminus[5];

uint8 pch = 0;

time_t* nptime_t;
tm* nptm;


int rtctimeforminusck[5] = { 6, 5, 4, 3, 1 };
int rtctimeforminusck3[5] = { 59, 59, 23, 31, 12 };
int rtctimeforminusck2[5];
tm* timey;

uint16 rtcpos = 0;
int rtctimetmp[5];
int rtctime[5];

HDC hCDC;
HBITMAP hCBitmap;
HDC hCDCfullscr;
HBITMAP hCBitmapfullscr;
HPEN hPen;
HBRUSH hBackGround[1280];
RECT rs;

HBITMAP hbDib;

BYTE* pBit;

uint8 ioporte6h = 0;

bool drawgrpbool = false;

uint8 pc8001kmp[256] = { 255,255,255,255,255,255,255,255,131,160,255,255,255,23,255,255,134,135,132,255,255,255,255,255,255,255,255,151,255,133,255,255,150,255,255,144,128,130,129,130,129,255,255,255,255,255,22,255,96,97,98,99,100,101,102,103,112,113,255,255,255,255,255,255,255,33,34,35,36,37,38,39,48,49,50,51,52,53,54,55,64,65,66,67,68,69,70,71,80,81,82,255,255,132,255,255,0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,20,145,146,147,148,149,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,114,115,116,87,117,118,32,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,83,84,85,86,255,255,255,119,255,255,133,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
int timexforch1tm_year4ml = 0;

int tofs = 0;

void rtcstrobe() { switch (pch & 0xF) { case 1:timerforch123 = time(0) + tofs; timey = localtime(&timerforch123); rtctimeforminusck2[0] = timey->tm_sec; rtctimeforminusck2[1] = timey->tm_min; rtctimeforminusck2[2] = timey->tm_hour; rtctimeforminusck2[3] = timey->tm_mday; rtctimeforminusck2[4] = timey->tm_mon + 1; rtcpos = 0; rtctime[4] = rtctimeforminusck2[4] << 4; rtctime[3] = ((rtctimeforminusck2[3] / 10) << 4) + rtctimeforminusck2[3] % 10; rtctime[2] = ((rtctimeforminusck2[2] / 10) << 4) + rtctimeforminusck2[2] % 10; rtctime[1] = ((rtctimeforminusck2[1] / 10) << 4) + rtctimeforminusck2[1] % 10; rtctime[0] = ((rtctimeforminusck2[0] / 10) << 4) + rtctimeforminusck2[0] % 10; rtcdata = rtctime[0] & 0x01; break; case 2: for (int cnt = 0; cnt < 5; cnt++) { if (cnt != 4) { rtctime[cnt] = rtctimetmp[cnt]; } else { rtctime[cnt] = (rtctimetmp[cnt] - 1); } }for (int cnt = 0; cnt < 5; cnt++) { if (cnt != 4) { rtctimeforminus[cnt] = (((rtctime[cnt] >> 4) * 10) + (rtctime[cnt] & 15)); } else { rtctimeforminus[cnt] = (rtctime[cnt] >> 4); } } timezforch1.tm_sec = rtctimeforminus[0] % 60; timezforch1.tm_min = rtctimeforminus[1] % 60; timezforch1.tm_hour = rtctimeforminus[2] % 24; timezforch1.tm_mday = rtctimeforminus[3] % 32; timezforch1.tm_mon = ((rtctimeforminus[4] - 1) % 12) + 1; timezforch1.tm_year = 71; tofs = mktime(&timezforch1) - time(0); break; } }
void rtcshift() { if (rtcpos < 40) { if ((pch >> 3 & 1)) { rtctimetmp[rtcpos >> 3] |= (pch >> 3 & 1) << (rtcpos & 7); } else { rtctimetmp[rtcpos >> 3] &= ~(1 << (rtcpos & 7)); }rtcpos += 1; rtcdata = (rtctime[rtcpos >> 3] >> (rtcpos & 7) & 1) ? true : false; }  }
void prtstrobe() {if (prtenable && prtenable0 != 0 && pch != 13){}prtenable0 = prtenable;}

bool cmtreseted = false;

UINT16 howmanypluginsloaded;

bool greenmonitor = false;

bool isenabledpcg = false;
bool crtcreverted = false;

bool is8mhz = false;

bool ispc8801mk2srormore = false;

uint8 crtcatsc = 0;

UINT8 arememorybankenabled = 0;

struct typeofpluginctx {
    UINT32 version;
    BOOL ispluginloaded;
    BOOL isexecutedontheemulator;
    UINT32 plugintype[24];
    int(*ptrofz80memaccess)(int,int,int);
    int(*uniquememaccess)(int, int, int);
} pluginctx[1024];

typedef BOOL typeofEmuInitialize(void);
BOOL(*EmuInitialize)(void);
typedef DWORD typeofEmuExecute(DWORD Addr, int NParams, ...);
DWORD(*EmuExecute)(DWORD Addr, int NParams, ...);

typedef DWORD typeofPeLdrLoadModule(LPCWSTR FileName);
DWORD(*PeLdrLoadModule)(LPCWSTR FileName);
typedef DWORD typeofPeLdrLoadModuleA(LPCSTR FileNameA);
DWORD(*PeLdrLoadModuleA)(LPCSTR FileNameA);
typedef DWORD typeofPeLdrGetModuleBase(DWORD Pe);
DWORD(*PeLdrGetModuleBase)(DWORD Pe);

typedef DWORD typeofPeLdrFindModuleByBase(DWORD Base);
DWORD(*PeLdrFindModuleByBase)(DWORD Base);
typedef FARPROC typeofPeLdrGetProcAddressA(DWORD Pe, LPCSTR Name);
FARPROC(*PeLdrGetProcAddressA)(DWORD Pe, LPCSTR Name);

FARPROC x86_GetProcAddress(DWORD Pe, LPCSTR Name) {
    return PeLdrGetProcAddressA(PeLdrFindModuleByBase(Pe), Name);
}

DWORD x86_LoadLibraryA(LPCSTR FileNameA) {
    return PeLdrLoadModuleA(FileNameA);
};
DWORD x86_LoadLibraryW(LPCWSTR FileName) {
    return PeLdrLoadModule(FileName);
};
DWORD x86_LoadLibraryAEx(LPCSTR FileNameA,HANDLE hFile,DWORD dwFlags) {
    return PeLdrLoadModuleA(FileNameA);
};
DWORD x86_LoadLibraryWEx(LPCWSTR FileName,HANDLE hFile,DWORD dwFlags) {
    return PeLdrLoadModule(FileName);
};

void WINAPI dialogtocheck(LPVOID showus) {
    char fddcommandstr[64];
    sprintf(fddcommandstr, "%02X\0", ((UINT32)showus));
    MessageBoxA(0, fddcommandstr, "A", 0);
    return;
}

UINT8 pcgtmp = 0;
UINT16 pcgaddr = 0;

UINT8 pcgcharram[0x400];

HBITMAP hbOld;
HDC hdcb;

DWORD palette32[256];

UINT8 serialchar[64];
UINT8 serialcharw[64];
bool serialstat = true;
bool serialstatw = true;

HANDLE cmtfileloc = 0;

UINT8 linecharnum = 0;
UINT8 upd3301intm = 0;

UINT8 rs232crate = 0;

UINT8 bankedmemory[4][0x8000];

void __stdcall serialdaemonx(void* prm_0) {
    while (true) {
        if (ttyconnected == true) {
            if (serialstatw == false) {
                WriteFile(cmtfileloc, &serialcharw, 1, 0, 0);
                serialstatw = true;
            }
        }
    }
}

union ALUFETCHBUF{
    UINT32 l;
    UINT8 c[4];
};

ALUFETCHBUF alutmp;
ALUFETCHBUF alucomp;

UINT8 fddcmemory[16384];
UINT8 fddcrom[2048];
bool isloadedfddcfirmware = false;

class i8255 {
private:
    uint8 modeio;
    uint8 bufferab[3];
    static int i8255phaccessx(int, int, int) {
        return 0xff;
    }
public:
    int (*i8255phaccess)(int,int,int);
    void init_i8255() {
        bufferab[0] = 0;
        bufferab[1] = 0;
        bufferab[2] = 0;
    }
    i8255() {
        i8255phaccess = i8255phaccessx;
        init_i8255();
    }
    ~i8255() {
    }
    int i8255memaccess(int prm_0, int prm_1, int prm_2) {
        switch (prm_2 & 1) {
        case 0:
            switch (prm_0 & 3) {
            case 0:
                if ((modeio & 0x80)) {
                    switch ((modeio >> 5) & 3) {
                    case 0:
                        if (!(modeio & 0x10)) {
                            bufferab[0] = prm_1;
                            i8255phaccess(0, bufferab[0], 0);
                        }
                        break;
                    case 1:
                        if (!(modeio & 0x10)) {
                            i8255phaccess(0, prm_1, 0);
                        }
                        break;
                    case 2:
                    case 3:
                        i8255phaccess(0, prm_1, 0);
                        break;
                    }
                }
                else {
                    i8255phaccess(0, prm_1, 0);
                }
                break;
            case 1:
                if ((modeio & 0x80)) {
                    if (!((modeio >> 6) & 1)) {
                        if (!(modeio & 2)) {
                            if (((modeio >> 2) & 1) == 0) {
                                bufferab[1] = prm_1;
                                i8255phaccess(1, bufferab[1], 0);
                            }
                            else {
                                i8255phaccess(1, prm_1, 0);
                            }
                        }
                    }
                    else {
                        i8255phaccess(1, prm_1, 0);
                    }
                }
                else {
                    i8255phaccess(1, prm_1, 0);
                }
                break;
            case 2:
                if ((modeio & 0x80)) {
                    if (!((modeio >> 6) & 1)) {
                        i8255phaccess(2, prm_1 & (((modeio & 1) ? 0 : 0xF) | ((modeio & 8) ? 0 : 0xF0)), 0);
                    }
                    else {
                        i8255phaccess(2, prm_1, 0);
                    }
                }
                else {
                    i8255phaccess(2, prm_1 | bufferab[2], 0);
                }
                break;
            case 3:
                modeio = prm_1;
                if (!(modeio & 0x80)) {
                    if (modeio & 1) {
                        bufferab[2] |= (1 << ((modeio >> 1) & 7));
                    }
                    else {
                        bufferab[2] &= ~(1 << ((modeio >> 1) & 7));
                    }
                    i8255phaccess(2, bufferab[2], 0);
                }
                break;
            }
            break;
        case 1:
            switch (prm_0 & 3) {
            case 0:
                if ((modeio & 0x80)) {
                    switch ((modeio >> 5) & 3) {
                    case 0:
                        if ((modeio & 0x10)) {
                            bufferab[0] = i8255phaccess(0, 0, 1);
                            return bufferab[0];
                        }
                        break;
                    case 1:
                        if ((modeio & 0x10)) {
                            return i8255phaccess(0, prm_1, 1);
                        }
                        break;
                    case 2:
                    case 3:
                        return i8255phaccess(0, prm_1, 1);
                        break;
                    }
                }
                else {
                    return i8255phaccess(0, prm_1, 1);
                }
                break;
            case 1:
                if ((modeio & 0x80)) {
                    if (!((modeio >> 6) & 1)) {
                        if ((modeio & 2)) {
                            if (((modeio >> 2) & 1) == 0) {
                                bufferab[1] = i8255phaccess(1, 0, 1);
                                return bufferab[1];
                            }
                            else {
                                return i8255phaccess(1, 0, 1);
                            }
                        }
                    }
                    else {
                        return i8255phaccess(1, 0, 1);
                    }
                }
                else {
                    return i8255phaccess(1, 0, 1);
                }
                break;
            case 2:
                if ((modeio & 0x80)) {
                    if (!((modeio >> 6) & 1)) {
                        return (i8255phaccess(2, 0, 1) & (((modeio & 1) ? 0xF : 0) | ((modeio & 8) ? 0xF0 : 0)));
                    }
                    else {
                        return i8255phaccess(2, 0, 1);
                    }
                }
                else {
                    return i8255phaccess(2, 0, 1) | bufferab[2];
                }
                break;
            case 3:
                return modeio;
                break;
            }
            break;
        }
        return 0xff;
    }
};

#ifndef _min
#define _min(a,b) (((a) < (b)) ? (a) : (b))
#endif

class i8272a {
private:
    uint8 i8272astatus;
    uint8 i8272datarate;
    uint8 statuspos;
    uint8 statussize;
    uint8 statcode[8];
    uint8 commandcode[32];
    uint8 commandpos;
    bool readwritephase;
    struct strct_Drive {
        uint32 cylinder;
        uint32 heada;
        uint32 recorda;
        uint32 numofwr;
        uint32 endoftrack;
        uint32 gaplen;
        uint32 datalen;
        uint32 diskpos;
        bool headsel;
        uint32 datalength;
        uint8 result;
        bool delaccess;
        bool crcerror;
    };
    strct_Drive Drive[4];
    uint8 i8272retstat;
    uint32 cylinderpcn;
    uint8 latestdisk;
    uint8 scancommandstat;
    bool isintpending;
    bool isnodmamode;
    bool iswaitedfortheexecevent;
    uint32 waitedexeceventtime;
    bool resultphase;

    static int fddphyaccessx(int prm_0, int prm_1, int prm_2, int prm_3, int prm_4) {
        return 0xff;
    }
public:
    struct strct_Diskstat {
        bool diskinserted;
        bool isprotected;
        bool motoractive;
        int (*fddphyaccess)(int,int,int,int,int);
    };
    strct_Diskstat Diskstat[4];
private:
    void get_i8272_param() {
        Drive[commandcode[1] & 3].headsel = ((commandcode[1] & 4) ? true : false);
        Drive[commandcode[1] & 3].cylinder = commandcode[2];
        Drive[commandcode[1] & 3].heada = commandcode[3];
        Drive[commandcode[1] & 3].recorda = commandcode[4];
        Drive[commandcode[1] & 3].numofwr = commandcode[5];
        Drive[commandcode[1] & 3].endoftrack = commandcode[6];
        Drive[commandcode[1] & 3].gaplen = commandcode[7];
        Drive[commandcode[1] & 3].datalen = commandcode[8];
        if ((commandcode[0] & 31) == 2) {
            Drive[commandcode[1] & 3].datalength = (0x80 << min(Drive[commandcode[1] & 3].numofwr, 7));
        }
        else {
            if (!(commandcode[0] & 16)) {
                if (Drive[commandcode[1] & 3].numofwr == 0) {
                    Drive[commandcode[1] & 3].datalength = Drive[commandcode[1] & 3].datalen;
                }
                else {
                    Drive[commandcode[1] & 3].datalength = ((Drive[latestdisk].numofwr & 7) ? (0x80 << (Drive[latestdisk].numofwr & 0x07)) : (_min(Drive[commandcode[1] & 3].datalen, 0x80)));
                }
            }
            else {
                Drive[commandcode[1] & 3].datalength = ((Drive[latestdisk].numofwr & 7) ? (0x80 << (Drive[latestdisk].numofwr & 0x07)) : (_min(Drive[commandcode[1] & 3].datalen, 0x80)));
            }
        }
        cylinderpcn = Drive[commandcode[1] & 3].cylinder;
        latestdisk = (commandcode[1] & 3);
        Drive[latestdisk].diskpos = 0;
        i8272astatus |= (1 << (latestdisk));
        i8272astatus |= 0x10;
        i8272astatus |= ((commandcode[0] & 1) ? 0 : 0x40);
        scancommandstat = 2;
        Drive[commandcode[1] & 3].result = (latestdisk & 3) | (Diskstat[latestdisk & 3].diskinserted ? 0 : 0x08) | ((Diskstat[latestdisk & 3].fddphyaccess == fddphyaccessx) ? 0x40 : 0);
    }
    void set_i8272_status() {
        i8272astatus &= ~(1 << (latestdisk));
        i8272astatus = 0x10 | 0x40 | 0x80;
        statussize = 7;
        statuspos = 6;
        statcode[0] = (commandcode[1] & 7) | (Diskstat[commandcode[1] & 3].diskinserted ? 0 : 0x08) | ((Diskstat[commandcode[1] & 3].fddphyaccess == fddphyaccessx && Drive[commandcode[1] & 3].delaccess == false && Drive[commandcode[1] & 3].crcerror == false) ? 0x40 : 0);//ST0
        statcode[1] = ((Diskstat[commandcode[1] & 3].isprotected && (commandcode[0] & 1)) ? 2 : 0) | (Diskstat[commandcode[1] & 3].diskinserted ? 0 : 0x01) | (Drive[commandcode[1] & 3].crcerror ? 0x20 : 0) | (Drive[commandcode[1] & 3].delaccess ? 0x80 : 0);//ST1
        statcode[2] = ((scancommandstat & 3) << 2) | (Drive[commandcode[1] & 3].crcerror ? 0x20 : 0) | (Drive[commandcode[1] & 3].delaccess ? 0x40 : 0);//ST2
        statcode[3] = Drive[commandcode[1] & 3].cylinder;//C
        statcode[4] = Drive[commandcode[1] & 3].heada;//H
        statcode[5] = Drive[commandcode[1] & 3].recorda;//R
        statcode[6] = Drive[commandcode[1] & 3].numofwr;//N
        Drive[commandcode[1] & 3].delaccess = false;
        Drive[commandcode[1] & 3].crcerror = false;
    }
    bool id_incr()
    {
        if ((commandcode[0] & 19) == 17) {
            // scan equal
            if ((Drive[commandcode[1] & 3].datalen & 0xff) == 0x02) {
                Drive[commandcode[1] & 3].recorda++;
            }
        }
        if (Drive[commandcode[1] & 3].recorda++ != Drive[commandcode[1] & 3].endoftrack) {
            return true;
        }
        Drive[commandcode[1] & 3].recorda = 1;
        if (commandcode[0] & 0x80) {
            Drive[commandcode[1] & 3].headsel = (Drive[commandcode[1] & 3].headsel ? false : true);
            Drive[commandcode[1] & 3].heada ^= 1;
            if (Drive[commandcode[1] & 3].heada & 1) {
                return true;
            }
        }
        Drive[commandcode[1] & 3].cylinder++;
        return false;
    }
    bool is_equal_scan(int prm_0,int prm_1) {
        switch (commandcode[0] & 31) {
        case 0x11:
            if (prm_0 != prm_1) { return false; }
            break;
        case 0x19:
            if (prm_0 > prm_1) { return false; }
            break;
        case 0x1d:
            if (prm_0 < prm_1) { return false; }
            break;
        }
        return true;
    }
public:
    void i8272a_waitforexec() {
        if (readwritephase == true) {
            Sleep(waitedexeceventtime);
            waitedexeceventtime = 0;
            i8272astatus |= 0x80;
        }
        else {
            if (iswaitedfortheexecevent == false) { return; }
            iswaitedfortheexecevent = false;
            Sleep(waitedexeceventtime);
            waitedexeceventtime = 0;
            i8272astatus = 0x10 | 0x20 | 0x80;
            if ((commandcode[0] & 31) == 2 || (commandcode[0] & 31) == 6 || (commandcode[0] & 31) == 12 || (commandcode[0] & 31) == 17 || (commandcode[0] & 31) == 25 || (commandcode[0] & 31) == 29) {
                i8272astatus |= 0x40;
            }
        }
    }
    void init_i8272a() {
        iswaitedfortheexecevent = false;
        //initialization of the i8272 fddc peripheral floppy parameters
        for (int cnt = 0; cnt < 4; cnt++) {
            Drive[cnt].cylinder = 0;
            Drive[cnt].heada = 0;
            Drive[cnt].recorda = 0;
            Drive[cnt].numofwr = 0;
            Drive[cnt].endoftrack = 0;
            Drive[cnt].gaplen = 0;
            Drive[cnt].datalen = 0;
            Drive[cnt].diskpos = 0;
            Drive[cnt].headsel = false;
            Drive[cnt].datalength = 0;
            Drive[cnt].result = 0;
            Drive[cnt].delaccess = false;
            Drive[cnt].crcerror = false;
        }
        commandpos = 0;
        for (int cnt = 0; cnt < 32; cnt++) {
            commandcode[cnt] = 0;
        }
        for (int cnt = 0; cnt < 8; cnt++) {
            statcode[cnt] = 0;
        }
        statussize = 0;
        readwritephase = false;
        isintpending = false;
        i8272astatus = 0x80;
        isnodmamode = true;
        waitedexeceventtime = 0;
        resultphase = false;
    }
    i8272a() {
        init_i8272a();
        for (int cnt = 0; cnt < 4; cnt++) {
            Diskstat[cnt].diskinserted = false;
            Diskstat[cnt].isprotected = false;
            Diskstat[cnt].motoractive = false;
            Diskstat[cnt].fddphyaccess = fddphyaccessx;
        }
    }
    ~i8272a() {}
    void removefdd(int prm_0) { Diskstat[prm_0 & 3].diskinserted = false; Diskstat[prm_0 & 3].isprotected = false; Diskstat[prm_0 & 3].motoractive = false; Diskstat[prm_0 & 3].fddphyaccess = fddphyaccessx; return; }
    bool is_int_pending() { if (isnodmamode == false) { isintpending = false; return false; } if (isintpending == true) { isintpending = false; return true; } return false; }
    UINT8 getlatestdisk() { return latestdisk; }
    void i8272a_sendtc() {
        if (readwritephase == true || statussize != 0 || (readwritephase == false && statussize == 7)) {
            set_i8272_status();
            commandpos = 0;
            readwritephase = false;
            isintpending = true;
        }
    }
    int i8272amemaccess(int prm_0, int prm_1, int prm_2) {
    uint8 commandsize[32] = {1,1,9,3,2,9,9,2,1,9,2,1,9,5,1,3,1,9,2,4,1,1,9,1,1,9,1,1,1,9,1,1};
        switch (prm_2) {
        case 0:
            switch (prm_0 & 1) {
            case 0:
                i8272datarate = prm_1 & 0xFF;
                break;
            case 1:
                if ((i8272astatus & 0xc0) == 0x80) {
                    i8272astatus &= ~0x80;
                    if (readwritephase == true) {
                        if ((i8272astatus & 0xc0) == 0x80) { i8272astatus &= ~0x80; }
                        if ((commandcode[0] & 31) == 13) {
                            if (Diskstat[commandcode[1] & 3].isprotected) {
                                i8272retstat = 0xff;
                                set_i8272_status();
                                commandpos = 0;
                                readwritephase = false;
                                isintpending = true;
                            }
                            else {
                                i8272retstat = 0xff;
                                switch (Drive[latestdisk].diskpos & 3) {
                                case 0x0:
                                    Drive[latestdisk].cylinder = prm_1;
                                    i8272astatus |= 0x20;
                                    break;
                                case 0x1:
                                    Drive[latestdisk].heada = prm_1;
                                    i8272astatus |= 0x20;
                                    break;
                                case 0x2:
                                    Drive[latestdisk].recorda = prm_1;
                                    i8272astatus |= 0x20;
                                    break;
                                case 0x3:
                                    Drive[latestdisk].numofwr = prm_1;
                                    for (int cnt = 0; cnt < (((Drive[latestdisk].numofwr & 7) ? (0x80 << (Drive[latestdisk].numofwr & 0x07)) : (0x80))); cnt++) {
                                        Diskstat[commandcode[1] & 3].fddphyaccess(cnt, commandcode[5], Drive[latestdisk].recorda, Drive[latestdisk].cylinder, (Drive[latestdisk].headsel ? 0x400 : 0) | (latestdisk << 8) | 0);
                                    }
                                    id_incr();
                                    i8272astatus |= 0x20;
                                    break;
                                }
                                Drive[latestdisk].diskpos++;
                                if (Drive[latestdisk].diskpos >= Drive[latestdisk].datalength) {
                                    i8272astatus |= 0x80;
                                    set_i8272_status();
                                    waitedexeceventtime = 2000;
                                    Drive[latestdisk].diskpos = 0;
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else {
                                    if (isnodmamode == true) {
                                        i8272astatus |= 0x80;
                                        isintpending = true;
                                    }
                                }
                            }
                            return i8272retstat;
                        }
                        else if (commandcode[0] & 16) {
                            if (is_equal_scan(Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, 0, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 1), prm_1)) {
                                scancommandstat &= ~2;
                            }
                            Drive[latestdisk].diskpos++;
                            if (Drive[latestdisk].diskpos >= Drive[latestdisk].datalength) {
                                id_incr();
                                Drive[commandcode[1] & 3].delaccess = false;
                                Drive[commandcode[1] & 3].crcerror = (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) & 2) ? true : false;
                                if (Drive[commandcode[1] & 3].crcerror == true) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else if (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) & 1) {
                                    Drive[commandcode[1] & 3].delaccess = true;
                                    if (commandcode[0] & 0x20) {
                                        set_i8272_status();
                                        commandpos = 0;
                                        readwritephase = false;
                                        isintpending = true;
                                    }
                                }
                                else {
                                    i8272astatus |= 0x80;
                                    waitedexeceventtime = 2000;
                                    Drive[latestdisk].diskpos = 0;
                                    isintpending = true;
                                }
                            }
                            else {
                                if (isnodmamode == true) {
                                    i8272astatus |= 0x80;
                                    isintpending = true;
                                }
                            }
                            return 0;
                        }
                        else 
                        {
                            if ((commandcode[0] & 1)) {
                                if (Diskstat[commandcode[1] & 3].isprotected) {
                                    i8272retstat = 0xff;
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else {
                                    i8272retstat = Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 0);
                                    Drive[latestdisk].diskpos++;
                                    if (Drive[latestdisk].diskpos >= Drive[latestdisk].datalength) {
                                        id_incr();
                                        Drive[commandcode[1] & 3].delaccess = false;
                                        Drive[commandcode[1] & 3].crcerror = (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) & 2) ? true : false;
                                        if (Drive[commandcode[1] & 3].crcerror == true) {
                                            set_i8272_status();
                                            commandpos = 0;
                                            readwritephase = false;
                                            isintpending = true;
                                        }
                                        else {
                                            if ((commandcode[0] & 31) == 9) {
                                                Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) | 1), Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 2);
                                            }
                                            else {
                                                Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) & ~1), Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 2);
                                            }
                                            i8272astatus |= 0x80;
                                            waitedexeceventtime = 2000;
                                            Drive[latestdisk].diskpos = 0;
                                            isintpending = true;
                                        }
                                    }
                                    else {
                                        if (isnodmamode == true) {
                                            i8272astatus |= 0x80;
                                            isintpending = true;
                                        }
                                    }
                                }
                                return i8272retstat;
                            }
                        }
                    }
                    else {
                        statussize = 0;
                        statuspos = 0;
                        commandcode[commandpos] = prm_1 & 0xFF; commandpos++;
                        if (commandsize[commandcode[0] & 31] >= 2) { i8272astatus = 0x10 | 0x80; }
                        if (commandsize[commandcode[0] & 31] >= 9 && commandpos > 1) { i8272astatus |= (1 << (commandcode[1] & 3)); }
                        if (commandsize[commandcode[0] & 31] <= commandpos) {
                            switch (commandcode[0] & 31) {
                            case 2://Read a Track
                                Drive[commandcode[1] & 3].delaccess = false;
                                Drive[commandcode[1] & 3].crcerror = (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[commandcode[1] & 3].diskpos, prm_1, Drive[commandcode[1] & 3].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[commandcode[1] & 3].headsel ? 0x400 : 0) | ((commandcode[1] & 3) << 8) | 3) & 2) ? true : false;
                                get_i8272_param();
                                i8272astatus &= ~(1 << (commandcode[1] & 3));
                                i8272astatus = 0x10 | 0x20 | 0x40 | 0x80;
                                Drive[commandcode[1] & 3].headsel = (Drive[commandcode[1] & 3].heada ? true : false);
                                if (Diskstat[commandcode[1] & 3].diskinserted == false) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else if (Drive[commandcode[1] & 3].crcerror == true) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else if (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) & 1) {
                                    Drive[commandcode[1] & 3].delaccess = true;
                                    if (commandcode[0] & 0x20) {
                                        set_i8272_status();
                                        commandpos = 0;
                                        readwritephase = false;
                                        isintpending = true;
                                    }
                                }
                                else {
                                    readwritephase = true;
                                    isintpending = true;
                                }
                                break;
                            case 3://specify
                                i8272astatus = 0x80;
                                isnodmamode = ((commandcode[2] & 0x01) ? true : false);
                                break;
                            case 4://Sense Drive status
                                i8272astatus = 0x10 | 0x40 | 0x80;
                                cylinderpcn = Drive[commandcode[1] & 3].cylinder;
                                latestdisk = (commandcode[1] & 3);
                                statussize = 1;
                                statcode[0] = (latestdisk & 3) | (Drive[commandcode[1] & 3].headsel ? 4 : 0) | ((cylinderpcn == 0) ? 0x10 : 0) | (Diskstat[commandcode[1] & 3].diskinserted ? 0x20 : 0) | (Diskstat[commandcode[1] & 3].isprotected ? 0x40 : 0) | (Diskstat[commandcode[1] & 3].motoractive ? 0 : 0x80) | 0x08;//ST3
                                isintpending = true;
                                break;
                            case 5://Write Data
                                Drive[commandcode[1] & 3].delaccess = false;
                                Drive[commandcode[1] & 3].crcerror = false;
                                get_i8272_param();
                                i8272astatus &= ~(1 << (commandcode[1] & 3));
                                i8272astatus = 0x10 | 0x20 | 0x80;
                                Drive[commandcode[1] & 3].headsel = (Drive[commandcode[1] & 3].heada ? true : false);
                                if (Diskstat[commandcode[1] & 3].diskinserted == false) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else {
                                    Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) & ~1), Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 2);
                                    readwritephase = true;
                                    isintpending = true;
                                }
                                break;
                            case 6://Read Data
                                Drive[commandcode[1] & 3].delaccess = false;
                                Drive[commandcode[1] & 3].crcerror = (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[commandcode[1] & 3].diskpos, prm_1, Drive[commandcode[1] & 3].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[commandcode[1] & 3].headsel ? 0x400 : 0) | ((commandcode[1] & 3) << 8) | 3) & 2) ? true : false;
                                get_i8272_param();
                                i8272astatus &= ~(1 << (commandcode[1] & 3));
                                i8272astatus = 0x10 | 0x20 | 0x40 | 0x80;
                                Drive[commandcode[1] & 3].headsel = (Drive[commandcode[1] & 3].heada ? true : false);
                                if (Diskstat[commandcode[1] & 3].diskinserted == false) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else if (Drive[commandcode[1] & 3].crcerror == true) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else if (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) & 1) {
                                    Drive[commandcode[1] & 3].delaccess = true;
                                    if (commandcode[0] & 0x20) {
                                        set_i8272_status();
                                        commandpos = 0;
                                        readwritephase = false;
                                        isintpending = true;
                                    }
                                }
                                else {
                                    readwritephase = true;
                                    isintpending = true;
                                }
                                break;
                            case 7://Recalibrate
                                i8272astatus = 0x80;
                                Drive[commandcode[1] & 3].cylinder = 0;
                                cylinderpcn = Drive[commandcode[1] & 3].cylinder;
                                latestdisk = (commandcode[1] & 3);
                                Drive[commandcode[1] & 3].result = (latestdisk & 3) | (Diskstat[latestdisk & 3].diskinserted ? 0 : 0x48) | ((Diskstat[latestdisk & 3].fddphyaccess == fddphyaccessx) ? 0x48 : 0) | 0x20;
                                isintpending = true;
                                i8272astatus &= ~(1 << (latestdisk));
                                break;
                            case 8://Sense Interrupt status
                                statussize = 0;
                                i8272astatus = 0x10 | 0x40 | 0x80;
                                for (int cnt = 0; cnt < 4; cnt++) {
                                    if (Drive[cnt & 3].result) {
                                        statussize = 2;
                                        statcode[0] = Drive[cnt & 3].result;//ST0
                                        statcode[1] = Drive[cnt & 3].cylinder;//Presented cylinder num
                                        Drive[cnt & 3].result = 0;
                                        isintpending = true;
                                        break;
                                    }
                                }
                                if (statussize == 0) {
                                    statussize = 1;
                                    statcode[0] = 0x80;//ST0
                                    isintpending = true;
                                }
                                break;
                            case 9://Write Deleted Data
                                Drive[commandcode[1] & 3].delaccess = false;
                                Drive[commandcode[1] & 3].crcerror = false;
                                get_i8272_param();
                                i8272astatus &= ~(1 << (commandcode[1] & 3));
                                i8272astatus = 0x10 | 0x20 | 0x80;
                                Drive[commandcode[1] & 3].headsel = (Drive[commandcode[1] & 3].heada ? true : false);
                                if (Diskstat[commandcode[1] & 3].diskinserted == false) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else {
                                    Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) | 1), Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 2);
                                    readwritephase = true;
                                    isintpending = true;
                                }
                                break;
                            case 10://Read ID
                                Drive[commandcode[1] & 3].headsel = ((commandcode[1] & 4) ? true : false);
                                cylinderpcn = Drive[commandcode[1] & 3].cylinder;
                                latestdisk = (commandcode[1] & 3);
                                set_i8272_status();
                                commandpos = 0;
                                isintpending = true;
                                break;
                            case 12://Read Deleted Data
                                Drive[commandcode[1] & 3].delaccess = false;
                                Drive[commandcode[1] & 3].crcerror = (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[commandcode[1] & 3].diskpos, prm_1, Drive[commandcode[1] & 3].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[commandcode[1] & 3].headsel ? 0x400 : 0) | ((commandcode[1] & 3) << 8) | 3) & 2) ? true : false;
                                get_i8272_param();
                                i8272astatus &= ~(1 << (commandcode[1] & 3));
                                i8272astatus = 0x10 | 0x20 | 0x40 | 0x80;
                                Drive[commandcode[1] & 3].headsel = (Drive[commandcode[1] & 3].heada ? true : false);
                                if (Diskstat[commandcode[1] & 3].diskinserted == false) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else if (Drive[commandcode[1] & 3].crcerror == true) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else {
                                    readwritephase = true;
                                    isintpending = true;
                                }
                                break;
                            case 13://Format Track
                                Drive[commandcode[1] & 3].delaccess = false;
                                Drive[commandcode[1] & 3].crcerror = (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[commandcode[1] & 3].diskpos, prm_1, Drive[commandcode[1] & 3].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[commandcode[1] & 3].headsel ? 0x400 : 0) | ((commandcode[1] & 3) << 8) | 3) & 2) ? true : false;
                                Drive[commandcode[1] & 3].headsel = ((commandcode[1] & 4) ? true : false);
                                Drive[commandcode[1] & 3].numofwr = commandcode[2];
                                Drive[commandcode[1] & 3].endoftrack = commandcode[3];
                                Drive[commandcode[1] & 3].gaplen = commandcode[4];
                                cylinderpcn = Drive[commandcode[1] & 3].cylinder;
                                latestdisk = (commandcode[1] & 3);
                                Drive[latestdisk].diskpos = 0;
                                i8272astatus |= (1 << (latestdisk));
                                i8272astatus |= 0x10;
                                i8272astatus |= ((commandcode[0] & 1) ? 0 : 0x40);
                                scancommandstat = 2;
                                Drive[commandcode[1] & 3].result = (latestdisk & 3) | (Diskstat[latestdisk & 3].diskinserted ? 0 : 0x08) | ((Diskstat[latestdisk & 3].fddphyaccess == fddphyaccessx) ? 0x40 : 0);
                                Drive[latestdisk].datalength = 4 * Drive[commandcode[1] & 3].endoftrack;
                                resultphase = true;
                                readwritephase = true;
                                i8272astatus |= 0x20;
                                isintpending = true;
                                //set_i8272_status();
                                break;
                            case 15://Seek
                                i8272astatus = 0x80;
                                Drive[commandcode[1] & 3].cylinder = commandcode[2];
                                cylinderpcn = Drive[commandcode[1] & 3].cylinder;
                                latestdisk = (commandcode[1] & 3);
                                Drive[commandcode[1] & 3].result = (latestdisk & 3) | (Diskstat[latestdisk & 3].diskinserted ? 0 : 0x48) | ((Diskstat[latestdisk & 3].fddphyaccess == fddphyaccessx) ? 0x48 : 0) | 0x20;
                                isintpending = true;
                                i8272astatus &= ~(1 << (latestdisk));
                                break;
                            case 17://Scan Equal
                                Drive[commandcode[1] & 3].delaccess = false;
                                Drive[commandcode[1] & 3].crcerror = (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[commandcode[1] & 3].diskpos, prm_1, Drive[commandcode[1] & 3].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[commandcode[1] & 3].headsel ? 0x400 : 0) | ((commandcode[1] & 3) << 8) | 3) & 2) ? true : false;
                                get_i8272_param();
                                i8272astatus = 0x10 | 0x20 | 0x40 | 0x80;
                                Drive[commandcode[1] & 3].headsel = (Drive[commandcode[1] & 3].heada ? true : false);
                                if (Diskstat[commandcode[1] & 3].diskinserted == false) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else if (Drive[commandcode[1] & 3].crcerror == true) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else if (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) & 1) {
                                    Drive[commandcode[1] & 3].delaccess = true;
                                    if (commandcode[0] & 0x20) {
                                        set_i8272_status();
                                        commandpos = 0;
                                        readwritephase = false;
                                        isintpending = true;
                                    }
                                }
                                else {
                                    readwritephase = true;
                                    isintpending = true;
                                }
                                break;
                            case 25://Scan Low or Equal
                                Drive[commandcode[1] & 3].delaccess = false;
                                Drive[commandcode[1] & 3].crcerror = (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[commandcode[1] & 3].diskpos, prm_1, Drive[commandcode[1] & 3].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[commandcode[1] & 3].headsel ? 0x400 : 0) | ((commandcode[1] & 3) << 8) | 3) & 2) ? true : false;
                                get_i8272_param();
                                i8272astatus = 0x10 | 0x20 | 0x40 | 0x80;
                                Drive[commandcode[1] & 3].headsel = (Drive[commandcode[1] & 3].heada ? true : false);
                                if (Diskstat[commandcode[1] & 3].diskinserted == false) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else if (Drive[commandcode[1] & 3].crcerror == true) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else if (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) & 1) {
                                    Drive[commandcode[1] & 3].delaccess = true;
                                    if (commandcode[0] & 0x20) {
                                        set_i8272_status();
                                        commandpos = 0;
                                        readwritephase = false;
                                        isintpending = true;
                                    }
                                }
                                else {
                                    readwritephase = true;
                                    isintpending = true;
                                }
                                break;
                            case 29://Scan High or Equal
                                Drive[commandcode[1] & 3].delaccess = false;
                                Drive[commandcode[1] & 3].crcerror = (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[commandcode[1] & 3].diskpos, prm_1, Drive[commandcode[1] & 3].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[commandcode[1] & 3].headsel ? 0x400 : 0) | ((commandcode[1] & 3) << 8) | 3) & 2) ? true : false;
                                get_i8272_param();
                                i8272astatus = 0x10 | 0x20 | 0x40 | 0x80;
                                Drive[commandcode[1] & 3].headsel = (Drive[commandcode[1] & 3].heada ? true : false);
                                if (Diskstat[commandcode[1] & 3].diskinserted == false) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else if (Drive[commandcode[1] & 3].crcerror == true) {
                                    set_i8272_status();
                                    commandpos = 0;
                                    readwritephase = false;
                                    isintpending = true;
                                }
                                else if (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) & 1) {
                                    Drive[commandcode[1] & 3].delaccess = true;
                                    if (commandcode[0] & 0x20) {
                                        set_i8272_status();
                                        commandpos = 0;
                                        readwritephase = false;
                                        isintpending = true;
                                    }
                                }
                                else {
                                    readwritephase = true;
                                    isintpending = true;
                                }
                                break;
                            default:
                                i8272astatus = 0x10 | 0x40 | 0x80;
                                statussize = 1;
                                statcode[0] = 0x80;//ST0
                                isintpending = true;
                                break;
                            }
                            if (statussize > 0) {
                                statuspos = statussize - 1;
                            }
                            else { statuspos = 0; }
                            if (readwritephase == false) {
                                commandpos = 0;
                            }
                            isintpending = true;
                        }
                    }
                }
                break;
            }
            return 0;
            break;
        case 1:
            switch (prm_0 & 1) {
            case 0:
                return i8272astatus;
                break;
            case 1:
                if ((i8272astatus & 0xc0) == 0xc0) {
                    i8272astatus &= ~0x80;
                    if (readwritephase == true) {
                        if ((commandcode[0] & 31) == 13) {
                            i8272astatus |= 0x80;
                            return 0xff;
                        }
                        else if (commandcode[0] & 16) {
                            i8272astatus |= 0x80;
                            return 0xff;
                        }
                        else {
                            if (!(commandcode[0] & 1)) {
                                i8272retstat = Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, 0, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 1);
                                Drive[latestdisk].diskpos++;
                                if (Drive[latestdisk].diskpos >= Drive[latestdisk].datalength) {
                                    id_incr();
                                    Drive[commandcode[1] & 3].delaccess = false;
                                    Drive[commandcode[1] & 3].crcerror = (Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) & 2) ? true : false;
                                    if (Drive[commandcode[1] & 3].crcerror == true) {
                                        set_i8272_status();
                                        commandpos = 0;
                                        readwritephase = false;
                                        isintpending = true;
                                    }
                                    else if ((Diskstat[commandcode[1] & 3].fddphyaccess(Drive[latestdisk].diskpos, prm_1, Drive[latestdisk].recorda, Drive[commandcode[1] & 3].cylinder, (Drive[latestdisk & 3].headsel ? 0x400 : 0) | (latestdisk << 8) | 3) & 1) && ((commandcode[0] & 31) != 12)) {
                                        Drive[commandcode[1] & 3].delaccess = true;
                                        if (commandcode[0] & 0x20) {
                                            set_i8272_status();
                                            commandpos = 0;
                                            readwritephase = false;
                                            isintpending = true;
                                        }
                                    }
                                    else {
                                        i8272astatus |= 0x80;
                                        waitedexeceventtime = 2000;
                                        Drive[latestdisk].diskpos = 0;
                                        isintpending = true;
                                    }
                                }
                                else {
                                    if (isnodmamode == true) {
                                        i8272astatus |= 0x80;
                                    }
                                    isintpending = true;
                                }
                                return i8272retstat;
                            }
                        }
                    }
                    else {
                        if (statussize == 0) {
                            i8272astatus |= 0x80;
                            return 0xff;
                        }
                        else {
                            i8272retstat = statcode[(statussize - 1) - statuspos];
                            if (statuspos > 0) { statuspos--; i8272astatus |= 0x80; }
                            else if (statuspos == 0) { statussize = 0; bool clearirq = true; if ((commandcode[0] & 31) == 0x08) { for (int cnt = 0; cnt < 4; cnt++) { if (Drive[cnt & 3].result) { clearirq = false; break; } } } if (clearirq == true) { isintpending = false; } i8272astatus = 0x80; }
                            return i8272retstat;
                        }
                    }
                    break;
                }
            }
            break;
        }
        return 0xff;
    }
};

typedef struct d88header {
    UINT8 diskname[17];
    UINT8 res0[9];
    UINT8 writeprotect;
    UINT8 disktype;
    UINT32 disksize;
    UINT32 trackoffs[164];
};

typedef struct d88secheader {
    UINT8 cylinder;
    UINT8 headerside;
    UINT8 sector;
    UINT8 sectorsize;
    UINT16 sectorspertrack;
    UINT8 dimensity;
    UINT8 isdeleted;
    UINT8 status;
    UINT8 res0[5];
    UINT16 sectordatasz;
};

typedef struct fddcontract4rw {
    d88header flpimgheader;
    d88secheader flpimgsecheadertmp;
    UINT32 track;
    UINT32 sector;
    UINT32 sectortmpl;
    UINT32 sectortmpl2;
    bool headerside;
    FILE* datafile;
    UINT32 pointerofsecheader;
    bool diskaccessing;
};

fddcontract4rw fdd[4];

i8272a GN8012_i8272;
i8255 GN8012_i8255;

int fddriveclose(int prm_0) {
    fdd[(prm_0) & 3].track = -1;
    fdd[(prm_0) & 3].sector = -1;
    fdd[(prm_0) & 3].sectortmpl = -1;
    fdd[(prm_0) & 3].sectortmpl2 = -1;
    fdd[(prm_0) & 3].headerside = false;
    fdd[(prm_0) & 3].diskaccessing = false;
    GN8012_i8272.Diskstat[(prm_0 & 3)].diskinserted = false;
    GN8012_i8272.Diskstat[(prm_0 & 3)].motoractive = false;
    if (fdd[(prm_0) & 3].datafile == 0) { return 0; }
    return fclose(fdd[(prm_0) & 3].datafile);
}
void fddriveload(int prm_0,const char* prm_1) {
    if (fdd[(prm_0) & 3].datafile != 0) { fddriveclose(prm_0); }
    fdd[(prm_0) & 3].track = -1;
    fdd[(prm_0) & 3].sector = -1;
    fdd[(prm_0) & 3].sectortmpl = -1;
    fdd[(prm_0) & 3].sectortmpl2 = -1;
    fdd[(prm_0) & 3].headerside = false;
    fdd[(prm_0) & 3].diskaccessing = false;
    fdd[(prm_0) & 3].datafile = fopen(prm_1, "rb+");
    if (fdd[(prm_0) & 3].datafile == 0) {
        fdd[(prm_0) & 3].datafile = fopen(prm_1, "wb+");
    }
    if (fdd[(prm_0) & 3].datafile == 0) { return; }
    GN8012_i8272.Diskstat[(prm_0 & 3)].diskinserted = true;
    GN8012_i8272.Diskstat[(prm_0 & 3)].motoractive = true;
    fseek(fdd[(prm_0) & 3].datafile, 0, SEEK_SET);
    fread(&(fdd[(prm_0) & 3].flpimgheader), sizeof(fdd[(prm_0) & 3].flpimgheader), 1, fdd[(prm_0) & 3].datafile);
    GN8012_i8272.Diskstat[(prm_0 & 3)].isprotected = (fdd[(prm_0) & 3].flpimgheader.writeprotect == 0x10) ? true : false;
    return;
}

UINT8 readbuf4fddrivebus[256 * 4];

int fddrivebus(int prm_0, int prm_1, int prm_2, int prm_3, int prm_4) {
    UINT32 sectortmp = -1;
    fdd[(prm_4 >> 8) & 3].diskaccessing = true;
    
    if (!(fdd[(prm_4 >> 8) & 3].track == prm_3 && fdd[(prm_4 >> 8) & 3].sector == prm_2 && fdd[(prm_4 >> 8) & 3].headerside == ((prm_4 & 0x400) ? true : false))) {
        for (int cnt2 = 0; cnt2 < 164; cnt2++) {
            if (fdd[(prm_4 >> 8) & 3].flpimgheader.trackoffs[cnt2] == 0) { break; }
            fseek(fdd[(prm_4 >> 8) & 3].datafile, fdd[(prm_4 >> 8) & 3].flpimgheader.trackoffs[cnt2], SEEK_SET);
            fread(&(fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp), 0x10, 1, fdd[(prm_4 >> 8) & 3].datafile);
            if (fdd[(prm_4 >> 8) & 3].flpimgheader.trackoffs[cnt2] != 0) {
                for (int cnt = 0; cnt < fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.sectorspertrack * ((fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.dimensity == 0x00) ? 2 : ((fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.dimensity == 0x40) ? 1 : ((fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.dimensity == 0x01) ? 1 : 4))); cnt++) {
                    fdd[(prm_4 >> 8) & 3].pointerofsecheader = fdd[(prm_4 >> 8) & 3].flpimgheader.trackoffs[cnt2] + (cnt * ((128 << fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.sectorsize) + 0x10));
                    fseek(fdd[(prm_4 >> 8) & 3].datafile, fdd[(prm_4 >> 8) & 3].pointerofsecheader, SEEK_SET);
                    fread(&(fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp), 0x10, 1, fdd[(prm_4 >> 8) & 3].datafile);
                    if ((fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.sector - 0) == prm_2 && fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.headerside == ((prm_4 & 0x400) ? 1 : 0)) { sectortmp = cnt; break; }
                }
                if ((cnt2 / 2) == prm_3 && sectortmp != -1) {
                    fdd[(prm_4 >> 8) & 3].sectortmpl = sectortmp;
                    fdd[(prm_4 >> 8) & 3].sectortmpl2 = cnt2;
                    break;
                }
                sectortmp = -1;
            }
        }
        if (sectortmp == -1) {
            if ((prm_4 & 3) == 0) {
                if (fdd[(prm_4 >> 8) & 3].flpimgheader.disksize == 0) { 
                    for (int cnt = 0; cnt < 17; cnt++) {
                        fdd[(prm_4 >> 8) & 3].flpimgheader.diskname[cnt] = 0;
                    }
                    for (int cnt = 0; cnt < 9; cnt++) {
                        fdd[(prm_4 >> 8) & 3].flpimgheader.res0[cnt] = 0;
                    }
                    fdd[(prm_4 >> 8) & 3].flpimgheader.writeprotect = 0;
                    fdd[(prm_4 >> 8) & 3].flpimgheader.disktype = 0;
                    fdd[(prm_4 >> 8) & 3].flpimgheader.disksize = 0x2b0;
                    for (int cnt = 0; cnt < 164; cnt++) {
                        fdd[(prm_4 >> 8) & 3].flpimgheader.trackoffs[cnt] = 0;
                    }
                    fseek(fdd[(prm_4 >> 8) & 3].datafile, 0, SEEK_SET);
                    fwrite(&fdd[(prm_4 >> 8) & 3].flpimgheader, sizeof(d88header), 1, fdd[(prm_4 >> 8) & 3].datafile);
                }
                if (fdd[(prm_4 >> 8) & 3].flpimgheader.trackoffs[(prm_3 * 2) + ((prm_4 & 0x400) ? 1 : 0)] == 0) {
                    fdd[(prm_4 >> 8) & 3].flpimgheader.trackoffs[(prm_3 * 2) + ((prm_4 & 0x400) ? 1 : 0)] = fdd[(prm_4 >> 8) & 3].flpimgheader.disksize;
                    sectortmp = 0;
                }
                d88secheader d88contenttmp;
                d88contenttmp.cylinder = (prm_3);
                d88contenttmp.headerside = ((prm_4 & 0x400) ? 1 : 0);
                d88contenttmp.sector = (prm_2);
                d88contenttmp.sectorsize = (0x1);
                d88contenttmp.sectorspertrack = (0x10);
                d88contenttmp.dimensity = (0);
                d88contenttmp.isdeleted = (0);
                d88contenttmp.status = (0);
                d88contenttmp.res0[0] = (0);
                d88contenttmp.res0[1] = (0);
                d88contenttmp.res0[2] = (0);
                d88contenttmp.res0[3] = (0);
                d88contenttmp.res0[4] = (0);
                d88contenttmp.sectordatasz = (256);
                fseek(fdd[(prm_4 >> 8) & 3].datafile, fdd[(prm_4 >> 8) & 3].flpimgheader.trackoffs[(prm_3 * 2) + ((prm_4 & 0x400) ? 1 : 0)] + ((256 + 0x10) * (prm_2 - 1)), SEEK_SET);
                fwrite(&d88contenttmp, 0x10, 1, fdd[(prm_4 >> 8) & 3].datafile);
                for (int cnt = 0; cnt < 256; cnt++) {
                    fputc(0xff, fdd[(prm_4 >> 8) & 3].datafile);
                }
                fdd[(prm_4 >> 8) & 3].flpimgheader.disksize += (256 + 0x10);
                fseek(fdd[(prm_4 >> 8) & 3].datafile, 0, SEEK_SET);
                fwrite(&fdd[(prm_4 >> 8) & 3].flpimgheader, sizeof(d88header), 1, fdd[(prm_4 >> 8) & 3].datafile);
                sectortmp = prm_2 - 1;
                fdd[(prm_4 >> 8) & 3].sectortmpl = sectortmp;
                fdd[(prm_4 >> 8) & 3].sectortmpl2 = ((prm_3 * 2) + ((prm_4 & 0x400) ? 1 : 0));
            }
            else if ((prm_4 & 3) == 3) {
                return 0x00;
            }
            else {
                return 0xff;
            }
        }
        fdd[(prm_4 >> 8) & 3].track = prm_3;
        fdd[(prm_4 >> 8) & 3].sector = prm_2;
        fdd[(prm_4 >> 8) & 3].headerside = ((prm_4 & 0x400) ? true : false);
        fseek(fdd[(prm_4 >> 8) & 3].datafile, fdd[(prm_4 >> 8) & 3].flpimgheader.trackoffs[fdd[(prm_4 >> 8) & 3].sectortmpl2] + (fdd[(prm_4 >> 8) & 3].sectortmpl * ((128 << fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.sectorsize) + 0x10)) + 0x10, SEEK_SET);
        fread(&readbuf4fddrivebus + (256 * ((prm_4 >> 8) & 3)), 256, 1, fdd[(prm_4 >> 8) & 3].datafile);
    }
    if (fdd[(prm_4 >> 8) & 3].sectortmpl == -1) { return 0xff; }
    switch (prm_4 & 3) {
    case 0:
        fseek(fdd[(prm_4 >> 8) & 3].datafile, fdd[(prm_4 >> 8) & 3].flpimgheader.trackoffs[fdd[(prm_4 >> 8) & 3].sectortmpl2] + (fdd[(prm_4 >> 8) & 3].sectortmpl * ((128 << fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.sectorsize) + 0x10)) + 0x10 + prm_0, SEEK_SET);
        fputc(prm_1, fdd[(prm_4 >> 8) & 3].datafile);
        readbuf4fddrivebus[(prm_0 & 0xFF) + (256 * ((prm_4 >> 8) & 3))] = prm_1 & 0xFF;
        break;
    case 1:
        //fseek(fdd[(prm_4 >> 8) & 3].datafile, fdd[(prm_4 >> 8) & 3].flpimgheader.trackoffs[fdd[(prm_4 >> 8) & 3].sectortmpl2] + (fdd[(prm_4 >> 8) & 3].sectortmpl * ((128 << fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.sectorsize) + 0x10)) + 0x10 + prm_0, SEEK_SET);
        return readbuf4fddrivebus[(prm_0 & 0xFF) + (256 * ((prm_4 >> 8) & 3))];
        break;
    case 2:
        if (fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.isdeleted == 0 && (prm_1 & 1)) {
            fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.isdeleted = 0x10;
            fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.status = 0x10;
            fseek(fdd[(prm_4 >> 8) & 3].datafile, fdd[(prm_4 >> 8) & 3].pointerofsecheader, SEEK_SET);
            fwrite(&(fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp), 0x10, 1, fdd[(prm_4 >> 8) & 3].datafile);
        }
        else if (fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.isdeleted == 0x10 && (prm_1 & 1) == 0) {
            fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.isdeleted = 0;
            fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.status = 0;
            fseek(fdd[(prm_4 >> 8) & 3].datafile, fdd[(prm_4 >> 8) & 3].pointerofsecheader, SEEK_SET);
            fwrite(&(fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp), 0x10, 1, fdd[(prm_4 >> 8) & 3].datafile);
        }
        break;
    case 3:
        return (fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.isdeleted ? 1 : 0) | (((fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.status == 0xa0) || (fdd[(prm_4 >> 8) & 3].flpimgsecheadertmp.status == 0xb0)) ? 2 : 0);
        break;
    }
    return 0xff;
}

i8255 GN80_i8255;
i8255 GN80_i8255_2;

UINT8 data4fddc[4];

int i8255mac_GN8012(int prm_0, int prm_1, int prm_2) {
    switch (prm_2 & 1) {
    case 0:
        switch (prm_0) {
        case 1:
            data4fddc[0] = prm_1 & 0xFF;
            break;
        case 2:
            data4fddc[2] = (data4fddc[2] & 0x0F) | ((prm_1 << 0) & 0xF0);
            break;
        }
        break;
    case 1:
        switch (prm_0) {
        case 0:
            return data4fddc[1] & 0xFF;
            break;
        case 2:
            return (data4fddc[2] >> 0) & 0x0F;
            break;
        }
        break;
    }
    return 0xff;
}
int i8255mac_GN80(int prm_0, int prm_1, int prm_2) {
    char recvcode4debug[64];
    switch (prm_2 & 1) {
    case 0:
        switch (prm_0) {
        case 1:
            data4fddc[1] = prm_1 & 0xFF;
            break;
        case 2:
            data4fddc[2] = (data4fddc[2] & 0xF0) | ((prm_1 >> 4) & 0xF);
            break;
        }
        break;
    case 1:
        switch (prm_0) {
        case 0:
            return data4fddc[0] & 0xFF;
            break;
        case 2:
            return (data4fddc[2] >> 4) & 0x0F;
            break;
        }
        break;
    }
    return 0xff;
}

void sendfdcmd28001(int prm_0) {
    while (true) { if ((i8255mac_GN8012(2, 0, 1) & 0x02)) { break; } Sleep(0); }
    i8255mac_GN8012(1, prm_0, 0);
    i8255mac_GN8012(2, 0x10, 0);
    while (true) { if (!(i8255mac_GN8012(2, 0, 1) & 0x02)) { break; } Sleep(0); }
    while (true) { if ((i8255mac_GN8012(2, 0, 1) & 0x04)) { break; } Sleep(0); }
    i8255mac_GN8012(2, 0x00, 0);
    while (true) { if (!(i8255mac_GN8012(2, 0, 1) & 0x04)) { break; } Sleep(0); }
}
int recvfdcmd28001() {
    int recvvaluetmp = 0;
    i8255mac_GN8012(2, 0x20, 0);
    while (true) { if ((i8255mac_GN8012(2, 0, 1) & 0x01)) { break; } Sleep(0); }
    i8255mac_GN8012(2, 0x00, 0);
    recvvaluetmp = i8255mac_GN8012(0, 0, 1);
    i8255mac_GN8012(2, 0x40, 0);
    while (true) { if (!(i8255mac_GN8012(2, 0, 1) & 0x01)) { break; } Sleep(0); }
    i8255mac_GN8012(2, 0x00, 0);
    return recvvaluetmp;
}

void sendfdcmd28001w(int prm_0) {
    while (true) { if ((i8255mac_GN8012(2, 0, 1) & 0x02)) { break; } Sleep(0); }
    i8255mac_GN8012(1, prm_0 & 0xFF, 0);
    i8255mac_GN8012(2, 0x10, 0);
    while (true) { if (!(i8255mac_GN8012(2, 0, 1) & 0x02)) { break; } Sleep(0); }
    while (true) { if ((i8255mac_GN8012(2, 0, 1) & 0x04)) { break; } Sleep(0); }
    i8255mac_GN8012(1, (prm_0 >> 8) & 0xFF, 0);
    i8255mac_GN8012(2, 0x00, 0);
    while (true) { if (!(i8255mac_GN8012(2, 0, 1) & 0x04)) { break; } Sleep(0); }
}
int recvfdcmd28001w() {
    int recvvaluetmp = 0;
    i8255mac_GN8012(2, 0x20, 0);
    while (true) { if ((i8255mac_GN8012(2, 0, 1) & 0x01)) { break; } Sleep(0); }
    i8255mac_GN8012(2, 0x00, 0);
    recvvaluetmp = i8255mac_GN8012(0, 0, 1) & 0xFF;
    i8255mac_GN8012(2, 0x40, 0);
    while (true) { if (!(i8255mac_GN8012(2, 0, 1) & 0x01)) { break; } Sleep(0); }
    recvvaluetmp |= (i8255mac_GN8012(0, 0, 1) << 8) & 0xFF00;
    i8255mac_GN8012(2, 0x00, 0);
    return recvvaluetmp;
}
int recvfdcmd28001c() {
    while (true) { if ((i8255mac_GN8012(2, 0, 1) & 0x08)) { break; } Sleep(0); }
    return recvfdcmd28001();
}

int fddcz80memaccess(int prm_0, int prm_1, int prm_2) {
    switch (prm_2) {
    case 0:
        if ((prm_0 >= 0x4000) && (prm_0 < 0x8000)) { fddcmemory[prm_0 & 0x3FFF] = (prm_1 & 0xFF); }
        break;
    case 1:
        if (prm_0 < 0x800) { return fddcrom[prm_0 & 0x7FF]; }
        if ((prm_0 >= 0x4000) && (prm_0 < 0x8000)) { return fddcmemory[prm_0 & 0x3FFF]; }
        break;
    case 2:
        switch (prm_0 & 0xFF) {
        case 0xf8:
            for (int cnt = 0; cnt < 4; cnt++) {
                GN8012_i8272.Diskstat[cnt].motoractive = (prm_1 & (1 << cnt)) ? true : false;
            }
            break;
        case 0xfa:
            return GN8012_i8272.i8272amemaccess(0, prm_1, prm_2 & 1);
            break;
        case 0xfb:
            return GN8012_i8272.i8272amemaccess(1, prm_1, prm_2 & 1);
            break;
        case 0xfc:
        case 0xfd:
        case 0xfe:
        case 0xff:
            return GN8012_i8255.i8255memaccess(prm_0 & 3, prm_1, prm_2 & 1);
            break;
        }
        break;
    case 3:
        switch (prm_0 & 0xFF) {
        case 0xf8:
            GN8012_i8272.i8272a_sendtc();
            return 0xff;
            break;
        case 0xfa:
            return GN8012_i8272.i8272amemaccess(0, prm_1, prm_2 & 1);
            break;
        case 0xfb:
            return GN8012_i8272.i8272amemaccess(1, prm_1, prm_2 & 1);
            break;
        case 0xfc:
        case 0xfd:
        case 0xfe:
        case 0xff:
            return GN8012_i8255.i8255memaccess(prm_0 & 3, prm_1, prm_2 & 1);
            break;
        }
        break;
    }
    return 0xff;
}

int z80memaccess(int prm_0, int prm_1, int prm_2) {
    switch (prm_2){
    case 0:
        if (((prm_0 & 0xFFFF) >= 0xC000 && (prm_0 & 0xFFFF) < 0x10000) && gvramenabled >= 1 && ispc8801 == true) { gvram[gvramenabled - 1][prm_0 & 0x3FFF] = prm_1 & 0xFF; return 0; }
        if (((prm_0 & 0xFFFF) >= 0xC000 && (prm_0 & 0xFFFF) < 0x10000) && (galuop & 0x80) && ispc8801 == true) { switch (galuop & 0x30) { case 0x00: for (int cnt = 0; cnt < 3; cnt++) { switch ((galuctrl >> cnt) & 0x11) { case 0x00: gvram[cnt][prm_0 & 0x3FFF] &= ~(prm_1 & 0xFF); break; case 0x01: gvram[cnt][prm_0 & 0x3FFF] |= (prm_1 & 0xFF); break; case 0x10: gvram[cnt][prm_0 & 0x3FFF] |= (prm_1 & 0xFF); break; } } break; case 0x10: gvram[0][prm_0 & 0x3FFF] = alutmp.c[0]; gvram[1][prm_0 & 0x3FFF] = alutmp.c[1]; gvram[2][prm_0 & 0x3FFF] = alutmp.c[2]; break; case 0x20: gvram[0][prm_0 & 0x3FFF] = alutmp.c[1]; break; case 0x30: gvram[1][prm_0 & 0x3FFF] = alutmp.c[0]; break; } return 0; }
        if ((prm_0 & 0xFFFF) >= 0xF000 && fastesttvramenabled == true && ispc8801 == true) { fastestvram[prm_0 & 0xFFF] = prm_1 & 0xFF; ispc8801mk2srormore=true; return 0; }
        if ((prm_0 & 0xFFFF) >= 0x8000 && (prm_0 & 0xFFFF) < 0x8400 && (rommode == false || biosromenabled == false) && ispc8801 == true && gvramenabled == 0) { if (fastesttvramenabled == true && (((prm_0 & 0x3ff) + (textwindoffsetadru8 << 8)) & 0xFFFF) >= 0xF000) { fastestvram[(((prm_0 & 0x3ff) + (textwindoffsetadru8 << 8)) & 0xFFFF) - 0xF000] = prm_1 & 0xFF; ispc8801mk2srormore=true; } else { memory[(((prm_0 & 0x3ff) + (textwindoffsetadru8 << 8)) & 0xFFFF)] = prm_1 & 0xFF; } return 0; }
        if (((prm_0 & 0xFFFF) >= 0x8000 && (prm_0 & 0xFFFF) < 0xC000) && gvramenabled >= 1 && ispc8801 == false) { gvram[gvramenabled - 1][prm_0 & 0x3FFF] = prm_1 & 0xFF; return 0; }
        if ((arememorybankenabled & 0xF0) && biosromenabled == false && ((prm_0 & 0xFFFF) < 0x8000)) { if (arememorybankenabled & 0x10) { bankedmemory[0][prm_0 & 0x7FFF] = prm_1 & 0xFF; } if (arememorybankenabled & 0x20) { bankedmemory[1][prm_0 & 0x7FFF] = prm_1 & 0xFF; } if (arememorybankenabled & 0x40) { bankedmemory[2][prm_0 & 0x7FFF] = prm_1 & 0xFF; } if (arememorybankenabled & 0x80) { bankedmemory[3][prm_0 & 0x7FFF] = prm_1 & 0xFF; } return 0; }
        memory[prm_0 & 0xFFFF] = prm_1 & 0xFF;
        return 0;
        break;
    case 1:
        if (((prm_0 & 0xFFFF) >= 0xC000 && (prm_0 & 0xFFFF) < 0x10000) && gvramenabled >= 1 && ispc8801 == true) { return gvram[gvramenabled - 1][prm_0 & 0x3FFF]; }
        if (((prm_0 & 0xFFFF) >= 0xC000 && (prm_0 & 0xFFFF) < 0x10000) && (galuop & 0x80) && ispc8801 == true) { ALUFETCHBUF wk; alutmp.l = (gvram[0][prm_0 & 0x3FFF] << (8 * 0)) | (gvram[1][prm_0 & 0x3FFF] << (8 * 1)) | (gvram[2][prm_0 & 0x3FFF] << (8 * 2)); wk.l = alucomp.l ^ alutmp.l; return wk.c[0] & wk.c[1] & wk.c[2]; }
        if ((dictromstat[1] & 1) == 0 && (((prm_0 & 0xC000) == 0xC000 && ispc8801 == true) || ((prm_0 & 0xC000) == 0x8000 && ispc8801 == false))) { return dicrom[(prm_0 & 0x3FFF) | ((dictromstat[0] & 0x1F) * 0x4000)]; }
        if ((prm_0 & 0xFFFF) >= 0xF000 && fastesttvramenabled == true && ispc8801 == true) { return fastestvram[prm_0 & 0xFFF]; }
        if ((prm_0 & 0xFFFF) >= 0x8000 && (prm_0 & 0xFFFF) < 0x8400 && (rommode == false || biosromenabled == false) && ispc8801 == true && gvramenabled == 0) { if (fastesttvramenabled == true && (((prm_0 & 0x3ff) + (textwindoffsetadru8 << 8)) & 0xFFFF) >= 0xF000) { return fastestvram[(((prm_0 & 0x3ff) + (textwindoffsetadru8 << 8)) & 0xFFFF) - 0xF000]; } else { return memory[(((prm_0 & 0x3ff) + (textwindoffsetadru8 << 8)) & 0xFFFF)]; } }
        if (((prm_0 & 0xFFFF) >= 0x8000 && (prm_0 & 0xFFFF) < 0xC000) && gvramenabled >= 1 && ispc8801 == false) { return gvram[gvramenabled - 1][prm_0 & 0x3FFF]; }
        if ((arememorybankenabled & 0x0F) && biosromenabled == false && ((prm_0 & 0xFFFF) < 0x8000)) { if (arememorybankenabled & 0x01) { return bankedmemory[0][prm_0 & 0x7FFF]; } if (arememorybankenabled & 0x02) { return bankedmemory[1][prm_0 & 0x7FFF]; } if (arememorybankenabled & 0x04) { return bankedmemory[2][prm_0 & 0x7FFF]; } if (arememorybankenabled & 0x08) { return bankedmemory[3][prm_0 & 0x7FFF]; } return 0xFF; }
        if ((prm_0 & 0xFFFF) < 0x6000 && biosromenabled == false && (rommode == true || ispc8801 == false)) { return bios[prm_0 & 0x7FFF]; }
        if ((prm_0 & 0xFFFF) < 0x8000 && biosromenabled == false && (rommode == true && ispc8801 == true)) { return n80rom[prm_0 & 0x1fff]; }
        if ((prm_0 & 0xFFFF) < 0x8000 && biosromenabled == false && (rommode == false && ispc8801 == true)) { 
            if ((extendedromsel == 0xff) || ((prm_0 & 0xFFFF) < 0x6000)) {
                return n88rom[prm_0 & 0x7FFF];
            }
            else if ((~extendedromsel) & 0x01) {
                return erom[0][eromsl & 3][prm_0 & 0x1FFF];
            }
            else if ((~extendedromsel) & 0x02) {
                return erom[1][eromsl & 3][prm_0 & 0x1FFF];
            }
            else if ((~extendedromsel) & 0x04) {
                return erom[2][eromsl & 3][prm_0 & 0x1FFF];
            }
            else if ((~extendedromsel) & 0x08) {
                return erom[3][eromsl & 3][prm_0 & 0x1FFF];
            }
            else if ((~extendedromsel) & 0x10) {
                return erom[4][eromsl & 3][prm_0 & 0x1FFF];
            }
            else if ((~extendedromsel) & 0x20) {
                return erom[5][eromsl & 3][prm_0 & 0x1FFF];
            }
            else if ((~extendedromsel) & 0x40) {
                return erom[6][eromsl & 3][prm_0 & 0x1FFF];
            }
            else if ((~extendedromsel) & 0x80) {
                return erom[7][eromsl & 3][prm_0 & 0x1FFF];
            }
        }
        if ((prm_0 & 0xFFFF) < 0x8000 && biosromenabled == false && romtype == true && ispc8801 == false) { return n80rom[prm_0 & 0x1fff]; }
        return memory[prm_0 & 0xFFFF];
        break;
    case 2:
        switch (prm_0 & 0xFF) {
        case 0x00:
            pcgtmp = prm_1;
            break;
        case 0x01:
            pcgaddr = (pcgaddr & 0xFF00) | ((prm_1 & 0xFF) << (8 * 0));
            break;
        case 0x02:
            pcgaddr = (pcgaddr & 0x00FF) | ((prm_1 & 0x03) << (8 * 1));
            if (prm_1 & 32) { pcgcharram[pcgaddr] = fontrom[0x400 + pcgaddr]; }
            else if (prm_1 & 16) { pcgcharram[pcgaddr] = pcgtmp; pcgtmp = 0; }
            break;
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
            pch = prm_1;
            break;
        case 0x20:
        case 0x22:
        case 0x24:
        case 0x26:
        case 0x28:
        case 0x2A:
        case 0x2C:
        case 0x2E:
            if (ttyconnected == false) {
                cmtfile = fopen(FileName, "ab"); if (cmtfile != 0) { fputc(prm_1, cmtfile); fclose(cmtfile); }
            }
            else { if (cmtfileloc != 0) { serialcharw[0] = prm_1; WriteFile(cmtfileloc, &serialcharw, 1, 0, 0); } }
            //if (uPD8251config[3] & 1) { cmtfile = fopen(FileName, "ab"); if (cmtfile != 0) { fputc(prm_1, cmtfile); fclose(cmtfile); } }
            break;
        case 0x21:
        case 0x23:
        case 0x25:
        case 0x27:
        case 0x29:
        case 0x2B:
        case 0x2D:
        case 0x2F:
        {
            bool configreseted = false;
            if (upd8251configate == 3) { uPD8251config[3] = (prm_1 & 0xFF);/*=(upd8251config&0x00FFFFFF)|((_z80_data&0xFF)<<24)*/ }
            else if (upd8251configate == 1) { uPD8251config[0] = (prm_1 & 0xFF); if (uPD8251config[0] & 0x40) { upd8251configate = 0; configreseted = true; return 0xff; } if (uPD8251config[0] & 0x10) { uPD8251config[1] &= ~0x38; overrunerror = false; rxdataready = false; } /*upd8251config=(upd8251config&0xFFFFFF00)|((_z80_data&0xFF)<<0)*/ }
            else if (upd8251configate == 0) { uPD8251config[2] = (prm_1 & 0xFF); configreseted = true; if (uPD8251config[2] & 3) { upd8251configate = 1; } else if (uPD8251config[2] & 0x80) { upd8251configate = 3; } else { upd8251configate = 2; } }
            if (configreseted == false) { upd8251configate++; if (upd8251configate == 2 || upd8251configate == 4) { upd8251configate = 1; } }
        }
            break;
        case 0x30:
        case 0x36:
        case 0x38:
        case 0x3A:
        case 0x3C:
        case 0x3E:
            cmtbinsnd = (prm_1 >> 4) & 0x3;
            cassettemtstate = (prm_1 >> 3) & 0x1;
            cmtdatawr = (prm_1 >> 2) & 0x1;
            crtmodectrl = (prm_1 >> 1) & 0x1;
            pc8001widthflag = (prm_1 >> 0) & 0x1;
            break;
        case 0x31:
        case 0x37:
        case 0x39:
        case 0x3B:
        case 0x3D:
        case 0x3F:
            crtc3 = prm_1;
            if (ispc8801 == true) {
                grpheight25 = ((prm_1 >> 5) & 0x1) ? true : false;
                colorfullgraphicmode = ((prm_1 >> 4) & 0x1) ? true : false;
                fullgraphicdraw = ((prm_1 >> 3) & 0x1) ? true : false;
                rommode = ((prm_1 >> 2) & 0x1) ? true : false;
                biosromenabled = ((prm_1 >> 1) & 0x1) ? true : false;
                hiresgrpresol200 = ((prm_1 >> 0) & 0x1) ? true : false;
            }
            else {
                bgcolor = (prm_1 >> 5) & 0x7;
                colorfullgraphicmode = ((prm_1 >> 4) & 0x1) ? true : false;
                fullgraphicdraw = ((prm_1 >> 3) & 0x1) ? true : false;
                fullgrpmode = ((prm_1 >> 2) & 0x1) ? true : false;
                biosromenabled = ((prm_1 >> 1) & 0x1) ? true : false;
                romtype = ((prm_1 >> 0) & 0x1) ? true : false;
            }
            break;
        case 0x32:
            soundintmask = ((prm_1 >> 7) & 0x1) ? true : false;
            gvramaccessmode = ((prm_1 >> 6) & 0x1) ? true : false;
            palettemode = ((prm_1 >> 5) & 0x1) ? true : false;
            fastesttvramenabled = ((prm_1 >> 4) & 0x1) ? false : true;
            videooutputmode = ((prm_1 >> 2) & 3);
            eromsl = ((prm_1 >> 0) & 3);
            break;
        case 0x34:
            galuctrl = prm_1 & 0x77;
            break;
        case 0x35:
            galuop = prm_1 & 0xb7;
            if (galuop & 0x80) { gvramenabled = 0; }
            alucomp.l = ((galuop & 1) ? 0x000000ff : 0) | ((galuop & 2) ? 0x0000ff00 : 0) | ((galuop & 4) ? 0x00ff0000 : 0);
            break;
        case 0x40:
            if (((prm_0 & 0x20) && beepenabled == false)/* || (!(prm_0 & 0x20) && beepenabled == true)*/) {
                isbeepenabledinthecool = true;
                if ((clockcount4beep - clockcount4beepprev) < F) { samplebase4beep = (((clockcount4beep - clockcount4beepprev) * ((UINT64)(SRATE))) / ((UINT64)((is8mhz ? 2 : 1) * (crtcactive ? (UINT64)1830000 : (UINT64)4000000)))) + samplebase4beeptiff; samplebase4beeptiff = samplebase4beep % F; clockcount4beepprev = clockcount4beep; }
            }
            else if ((!(prm_0 & 0x20) && beepenabled == true)) {
                howmanybeepstopped++;
            }
            if (ispc8801 == true) {
                if (((prm_0 & 0x80) && beepenabled2 == false)/* || (!(prm_0 & 0x80) && beepenabled2 == true)*/) {
                    isbeepenabledinthecool2 = true;
                }
                else if ((!(prm_0 & 0x80) && beepenabled2 == true)) {
                    howmanybeepstopped_2++;
                }
                beepenabled2 = ((prm_1 >> 7) & 0x01) ? true : false;
            }
            uopout = (prm_1 >> 6) & 0x03;
            beepenabled = ((prm_1 >> 5) & 0x01) ? true : false;
                crtcldsclkenable = (prm_1 >> 3) & 0x01;
                rtcclkenable = (prm_1 >> 2) & 0x01;
                rtcstbenable = (prm_1 >> 1) & 0x01;
                if (rtcstbenable == true) { rtcstrobe(); }
                else { if (rtcclkenable == true) { rtcshift(); } }
                prtenable = (prm_1 >> 0) & 0x01; prtstrobe();
            break;
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
            break;
        case 0x50:
            uPD3301prm = prm_1;
                if (seq){
                    switch (seq--) {
                    case 1:
                        crtcatsc = (prm_1 >> 5) & 7;
                        colorgraphicmode = ((prm_1 >> 4) & 0x1) ? true : false;
                        attributesize = _min((prm_1 & 0x1f), 20);
                        crtcactive = 0;
                        break;
                    case 3:
                        if (ispc8801 == false) {
                            grpheight25 = ((prm_1 & 0x1f) < 9) ? true : false;
                        }
                        cursortype = ((prm_1 >> 5) & 3);
                        linespace = (prm_1 & 0x80) ? true : false;
                        break;
                    case 4:
                        blinkingtime = ((prm_1 >> 6) & 3);
                        break;
                    case 5:
                        linecharnum = prm_1 & 0x7f;
                        break;
                    case 6:
                        cursx = cursxtmp;
                        cursy = prm_1;
                        seq = 0;
                        break;
                    case 7:
                        cursxtmp = prm_1;
                        break;
                    case 8:
                        upd3301stat = 0;
                        upd31speclzsig = 0;
                        seq = 0;
                        break;
                    }
                }
            break;
        case 0x51:
            upd3301cmd = prm_1;
            switch (prm_1 & 0xE0){
            case 0x00:
                seq = 5;
                upd3301stat &= ~16;
                upd3301stat |= 0x80;
                crtcactive = 0;
                break;
            case 0x20:
                upd3301stat &= ~8;
                upd3301stat |= 0x90;
                crtcactive = 1;
                crtcreverted = ((prm_1 & 1) ? true : false);
                break;
            case 0x40:
                if (!(prm_1 & 1)) {
                    upd3301stat = 0x80;
                    upd31speclzsig = 0;
                }
                upd3301intm = prm_1 & 3;
                break;
            case 0x60:
                if (((prm_1 & 0xF)) == 0) { upd3301stat = upd3301stat & (0xFF - 1); upd31speclzsig = 1; }
                break;
            case 0x80:
                if (((prm_1 & 0xF) & 0x01) == 0) {
                    cursx = -1;
                    cursy = -1;
                }
            if (((prm_1 & 0xF) & 0x01) == 1){
                seq = 7;
            }
            break;
            case 0xA0:
                upd3301stat &= ~0x06;
                break;
            case 0xC0:
                seq = 8;
                upd3301stat &= ~0x06;
                break;
        }
            break;
        case 0x52:
            bgcolor = (prm_1 >> 4) & 0x7;
            break;
        case 0x53:
            showstatefor88grp = prm_1;
            break;
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
            if (palettemode == false) {
                palette512_8bt[(prm_0 & 0xFF) - 0x54][0] = (((prm_1 & 1) ? 7 : 0) << (3 * 0)) | (((prm_1 & 2) ? 7 : 0) << (3 * 1));
                palette512_8bt[(prm_0 & 0xFF) - 0x54][1] = (((prm_1 & 4) ? 7 : 0) << (3 * 0));
            }
            else {
                palette512_8bt[(prm_0 & 0xFF) - 0x54][(prm_1 >> 6) & 3] = (prm_1 & 0x3F);
            }
            break;
        case 0x5C:
            gvramenabled = 1;
            galuop &= 0x7F;
            break;
        case 0x5D:
            gvramenabled = 2;
            galuop &= 0x7F;
            break;
        case 0x5E:
            gvramenabled = 3;
            galuop &= 0x7F;
            break;
        case 0x5F:
            gvramenabled = 0;
            galuop &= 0x7F;
            break;
        case 0x60:
            dmaas[0] = (dmaas[0] & (((dmachiocnt >> 0) & 1) ? 0x00FF : 0xFF00)) | ((prm_1 & 0xFF) << (((dmachiocnt >> 0) & 1) * 8)); dmachiocnt ^= (1 << 0);
            break;
        case 0x61:
            dmatc[0] = (dmatc[0] & (((dmachiocnt >> 1) & 1) ? 0x00FF : 0xFF00)) | ((prm_1 & 0xFF) << (((dmachiocnt >> 1) & 1) * 8)); dmachiocnt ^= (1 << 1);
            break;
        case 0x62:
            dmaas[1] = (dmaas[1] & (((dmachiocnt >> 2) & 1) ? 0x00FF : 0xFF00)) | ((prm_1 & 0xFF) << (((dmachiocnt >> 2) & 1) * 8)); dmachiocnt ^= (1 << 2);
            break;
        case 0x63:
            dmatc[1] = (dmatc[1] & (((dmachiocnt >> 3) & 1) ? 0x00FF : 0xFF00)) | ((prm_1 & 0xFF) << (((dmachiocnt >> 3) & 1) * 8)); dmachiocnt ^= (1 << 3);
            break;
        case 0x64:
            dmaas[2] = (dmaas[2] & (((dmachiocnt >> 4) & 1) ? 0x00FF : 0xFF00)) | ((prm_1 & 0xFF) << (((dmachiocnt >> 4) & 1) * 8)); dmachiocnt ^= (1 << 4);
            break;
        case 0x65:
            dmatc[2] = (dmatc[2] & (((dmachiocnt >> 5) & 1) ? 0x00FF : 0xFF00)) | ((prm_1 & 0xFF) << (((dmachiocnt >> 5) & 1) * 8)); dmachiocnt ^= (1 << 5);
            break;
        case 0x66:
            dmaas[3] = (dmaas[3] & (((dmachiocnt >> 6) & 1) ? 0x00FF : 0xFF00)) | ((prm_1 & 0xFF) << (((dmachiocnt >> 6) & 1) * 8)); dmachiocnt ^= (1 << 6);
            break;
        case 0x67:
            dmatc[3] = (dmatc[3] & (((dmachiocnt >> 7) & 1) ? 0x00FF : 0xFF00)) | ((prm_1 & 0xFF) << (((dmachiocnt >> 7) & 1) * 8)); dmachiocnt ^= (1 << 7);
            break;
        case 0x68:
            dmamodestat = prm_1;
            dmaseq = 2;
            break;
        case 0x6F:
            rs232crate = prm_1 & 0xF;
            break;
        case 0x70:
            textwindoffsetadru8 = prm_1;
            break;
        case 0x71:
            extendedromsel = prm_1;
            break;
        case 0x78:
            textwindoffsetadru8++;
            break;
        case 0xE2:
            arememorybankenabled = prm_1;
            break;
        case 0xE4:
            intmasklevel = prm_1 & 0x0F;
            break;
        case 0xE6:
            ioporte6h = prm_1;
            break;
        case 0xE8:
            kanjiromaddr1 = (kanjiromaddr1 & 0xFF00) | (prm_1 << (8 * 0));
            break;
        case 0xE9:
            kanjiromaddr1 = (kanjiromaddr1 & 0x00FF) | (prm_1 << (8 * 1));
            break;
        case 0xEC:
            kanjiromaddr2 = (kanjiromaddr2 & 0xFF00) | (prm_1 << (8 * 0));
            break;
        case 0xED:
            kanjiromaddr2 = (kanjiromaddr2 & 0x00FF) | (prm_1 << (8 * 1));
            break;
        case 0xF0:
        case 0xF1:
            if (((prm_1 & 0xFF) >= 0x20) && ((prm_0 & 1) == 0)) { return 0; }
            if (((prm_1 & 0xFF) & 0xFE) && ((prm_0 & 1) == 1)) { return 0; }
            dictromstat[prm_0 & 1] = prm_1 & 0xFF;
            return 0;
            break;
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
            return GN80_i8255_2.i8255memaccess(prm_0 & 3, prm_1, prm_2 & 1);
            break;
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            return GN80_i8255.i8255memaccess(prm_0 & 3, prm_1, prm_2 & 1);
            break;
        }
        break;
    case 3:
        switch (prm_0 & 0xFF) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            return ~pc8001keybool[prm_0 & 0xF];
            break;
        case 0x20:
        case 0x22:
        case 0x24:
        case 0x26:
        case 0x28:
        case 0x2A:
        case 0x2C:
        case 0x2E:
            //MessageBoxA(0, "A", "A", 0);
            if (ttyconnected == false) {
                if (cmtreseted == true) { cmtreseted = false; cmtseek = 0; return 0xff; }
                else { cmtfile = fopen(FileName, "rb"); if (cmtfile != 0) { struct _stat buf; int result = _stat(FileName, &buf); if (buf.st_size <= cmtseek) { cmtseek = 0; fclose(cmtfile); return 0xFF; } fseek(cmtfile, cmtseek++, SEEK_SET); ret = fgetc(cmtfile); fclose(cmtfile); return ret; } else { return 0xff; } }
            }
            else { while (true) { if (serialstat == true) { break; } } serialstat = false; if (rxdataready == false) { return 0xff; } else { ret = serialchar[0]; rxdataready = false; return ret; } }
            return 0xff;
            //if (uPD8251config[3] & 4) { if (cmtreseted == true) { cmtreseted = false; cmtseek = 0; return 0xff; } else { cmtfile = fopen(FileName, "rb"); if (cmtfile != 0) { fseek(cmtfile, cmtseek++, SEEK_SET); ret = fgetc(cmtfile); fclose(cmtfile); return ret; } } }
            return uPD8251config[2];
            break;
        case 0x21:
        case 0x23:
        case 0x25:
        case 0x27:
        case 0x29:
        case 0x2B:
        case 0x2D:
        case 0x2F:
            return uPD8251config[1] | (( rxdataready ? 1 : 0 ) << 1) | ( ( (uPD8251config[0] & 1 ) ? 1 : 0) << 0 ) | ((serialstatw ? 1 : 0) << 0) | ((cmtreseted ? 1 : 0) << 7) | 4;
            break;
        case 0x30:
            return crtc2;
            break;
        case 0x31:
            if (ispc8801 == true) {
                return bsmode;
            }
            else {
                return ((bgcolor & 0x7) << 5) | ((colorfullgraphicmode ? 1 : 0) << 4) | ((fullgraphicdraw ? 1 : 0) << 3) | ((fullgrpmode ? 1 : 0) << 2) | ((biosromenabled ? 1 : 0) << 1) | ((romtype ? 1 : 0) << 0);
            }
            break;
        case 0x32:
            return ((soundintmask ? 1 : 0) << 7) | ((gvramaccessmode ? 1 : 0) << 6) | ((palettemode ? 1 : 0) << 5) | ((fastesttvramenabled ? 0 : 1) << 4) | ((videooutputmode & 3) << 2) | ((eromsl & 3) << 0);
            break;
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
            return 0xff;
            break;
        case 0x40:
            return ((uipin & 3) << 6) | ((vbi ? 1 : 0) << 5) | ((rtcdata & 1) << 4) | ((fddconnected ? 0 : 1) << 3) | ((cmtdatard ? 1 : 0) << 2) | ((prtready ? 1 : 0) << 0);
            break;
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
            return 0xff;
            break;
        case 0x50:
            if (upd31speclzsig == 0) { return uPD3301prm; }
            else {
                switch (upd31speclzsig & 0xFF) {
                case 1:
                    ret = (litepeninp >> (((upd31speclzsig >> 8) & 0xFF) * 8));
                        break;
                }
                upd31speclzsig = (upd31speclzsig & 0xFFFF00FF) | ((((upd31speclzsig >> 8) & 0xFF) + 1) << 8);
                if (((upd31speclzsig >> 8)&0xFF) >= 2) { upd31speclzsig = 0; litepeninp = 0; }
                return ret;
                }

            break;
        case 0x51:
            if (upd3301stat & 8) { return upd3301stat & (0xFF - 0x10); }
            else { return upd3301stat; }

            break;
        case 0x5C:
            return ((gvramenabled == 0) ? 0 : (1 << (gvramenabled - 1)));
            break;
        case 0x60:
            ret = dmaas[0] >> (((dmachiocnt >> 0) & 1) * 8); dmachiocnt ^= (1 << 0); return ret;
            break;
        case 0x61:
            ret = dmatc[0] >> (((dmachiocnt >> 1) & 1) * 8); dmachiocnt ^= (1 << 1); return ret;
            break;
        case 0x62:
            ret = dmaas[1] >> (((dmachiocnt >> 2) & 1) * 8); dmachiocnt ^= (1 << 2); return ret;
            break;
        case 0x63:
            ret = dmatc[1] >> (((dmachiocnt >> 3) & 1) * 8); dmachiocnt ^= (1 << 3); return ret;
            break;
        case 0x64:
            ret = dmaas[2] >> (((dmachiocnt >> 4) & 1) * 8); dmachiocnt ^= (1 << 4); return ret;
            break;
        case 0x65:
            ret = dmatc[2] >> (((dmachiocnt >> 5) & 1) * 8); dmachiocnt ^= (1 << 5); return ret;
            break;
        case 0x66:
            ret = dmaas[3] >> (((dmachiocnt >> 6) & 1) * 8); dmachiocnt ^= (1 << 6); return ret;
            break;
        case 0x67:
            ret = dmatc[3] >> (((dmachiocnt >> 7) & 1) * 8); dmachiocnt ^= (1 << 7); return ret;
            break;
        case 0x68:
            return dmamodestat | ((1 << 4) * ((clockcount % 19) > 9));
            break;
        case 0x6E:
            return (is8mhz ? 0x7F : 0xFF);
            break;
        case 0x6F:
            return rs232crate & 0xF;
            break;
        case 0x70:
            return textwindoffsetadru8;
            break;
        case 0x71:
            return extendedromsel;
            break;
        case 0xE2:
            if (ispc8801 == true) {
                return ~arememorybankenabled;
            }
            else {
                return ~(arememorybankenabled & 0x0F);
            }
            break;
        case 0xE6:
            return ioporte6h;
            break;
        case 0xE8:
            return kanjirom1[(kanjiromaddr1 * 2) + 1];
            break;
        case 0xE9:
            return kanjirom1[(kanjiromaddr1 * 2) + 0];
            break;
        case 0xEC:
            return kanjirom2[(kanjiromaddr2 * 2) + 1];
            break;
        case 0xED:
            return kanjirom2[(kanjiromaddr2 * 2) + 0];
            break;
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
            return GN80_i8255_2.i8255memaccess(prm_0 & 3, prm_1, prm_2 & 1);
            break;
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            return GN80_i8255.i8255memaccess(prm_0 & 3, prm_1, prm_2 & 1);
            break;
        }
        //return 0xff;
        break;
    }
    BOOL isretvalfrompluginactive = false;
    UINT8 retvalfromplugin = 0;
    for (int cnt = 0; cnt < howmanypluginsloaded; cnt++) { for (int cnt4pluginctx = 0; cnt4pluginctx < 24; cnt4pluginctx++) { if ((pluginctx[cnt].ispluginloaded == true) && ((pluginctx[cnt].plugintype[cnt4pluginctx]) & 0x10000)) { if (pluginctx[cnt].isexecutedontheemulator == false) { retvalfromplugin |= pluginctx[cnt].uniquememaccess(prm_0, prm_1, prm_2); } else { retvalfromplugin |= EmuExecute((DWORD)&pluginctx[cnt].uniquememaccess, 3, prm_0, prm_1, prm_2); } isretvalfrompluginactive = true; } } }
    if (isretvalfrompluginactive == true) { return retvalfromplugin; }
    else { return 0xff; }
}

int crtcmemaccess(int prm_0, int prm_1, int prm_2) {
    switch (prm_2) {
    case 0:
        if ((prm_0 & 0xFFFF) >= 0xF000 && ispc8801mk2srormore == true && ispc8801 == true) { fastestvram[prm_0 & 0xFFF] = prm_1 & 0xFF; return fastestvram[prm_0 & 0xFFF]; }
        memory[prm_0 & 0xFFFF] = prm_1 & 0xFF;
        return 0;
        break;
    case 1:
        if ((prm_0 & 0xFFFF) >= 0xF000 && ispc8801mk2srormore == true && ispc8801 == true) { return fastestvram[prm_0 & 0xFFF]; }
        return memory[prm_0 & 0xFFFF];
        break;
    }
    return z80memaccess(prm_0, prm_1, prm_2);
}

COLORREF GetBrushColor(HBRUSH brush)
{
    LOGBRUSH lbr;
    if (GetObject(brush, sizeof(lbr), &lbr) != sizeof(lbr)) {
        // Not even a brush!
        return CLR_NONE;
    }
    if (lbr.lbStyle != BS_SOLID) {
        // Not a solid color brush.
        return CLR_NONE;
    }
    return lbr.lbColor;
}

HWND hwnd4mw;

uint8 z80irqid = 0;
uint8 z80irqfn = 0;
uint8 z80irqfnqueue[8];
uint8 z80irqfnqueuepos = 0;
uint8 z80irqfnqueuepos2 = 0;

uint8 z80irqmaxes = 8;

extern void DrawGrp();

void Z80INT(uint8 prm_0) { if (((intmasklevel & 0x8) == 0 && ((prm_0 / 2) >= (intmasklevel & 0x7))) && ispc8801 == true/* || ((intmasklevel & 0x8) && ((prm_0 / 2) > z80irqmaxes))*/) { return; } z80irqmaxes = prm_0 / 2; if (z80irqid >= 3 && z80irqid < 10) { z80irqid++; } else { if (z80irqid != 10) { z80irqid = 3; } } z80irqfnqueue[z80irqfnqueuepos] = prm_0; z80irqfnqueuepos = ((z80irqfnqueuepos + 1) & 0x7); }
void Z80NMI() { z80irqid = 2; }

void RunZ80Infinity(LPVOID* arg4rz80) { bool boolofwaitiinz80loop = true; UINT32 clockcounttemporary = 0; UINT32 clockcountold = 0; SYSTEMTIME st_st; SYSTEMTIME st_goal; int ststgoal16; while (true) { clockcountold = clockcount; int clockcountinternal = 0; int clockcountinternalold = 0; int z80timerbefore = time(NULL); while ((clockcount - clockcountold) < ((is8mhz ? 2 : 1) * (crtcactive ? 1830000 : 4000000))) { boolofwaitiinz80loop = (boolofwaitiinz80loop) ? false : true; clockcountinternalold = 0; clockcountinternal = 0; GetSystemTime(&st_st); UINT32 Z80Corepfclock = (crtcactive ? 1830000 : 4000000); ispc80threadinrunningemulation = true; isbeepenabledinthecool = false; isbeepenabledinthecool2 = false; while (clockcountinternal < ((is8mhz ? 2 : 1) * (Z80Corepfclock / 60))) { while ((clockcountinternal - clockcountinternalold) < ((is8mhz ? 2 : 1) * (Z80Corepfclock / 600))) { for (int clockcountinternal2 = 0; clockcountinternal2 < ((Z80Corepfclock / 3 / 600) * 2); clockcountinternal2) { vbi = false; if (z80irqid != 0) { if (z80irqid == 1) { Z80DoIRQ(z80irqfn); z80irqfn = 0; } else if (z80irqid >= 3 && z80irqid <= 10) { Z80DoIRQ(z80irqfnqueue[z80irqfnqueuepos2]); z80irqfnqueuepos2 = ((z80irqfnqueuepos2 + 1) & 0x7); for (int cnt = 0; cnt < 10000; cnt++) { clockcounttemporary = Z80Run(); clockcountinternal += clockcounttemporary; clockcountinternal2 += clockcounttemporary; clockcount += clockcounttemporary; } } else { Z80DoNMI(); } if (z80irqid > 3 && z80irqid <= 10) { z80irqid--; } else { z80irqid = 0; } } vbi = false; clockcounttemporary = Z80Run(); z80irqmaxes = 8; clockcountinternal += clockcounttemporary; clockcountinternal2 += clockcounttemporary; clockcount += clockcounttemporary; } vbi = true; if ((ioporte6h & 2) && ispc8801 == true) { if ((upd3301stat & 0x10) && !(upd3301intm & 1)) { upd3301stat |= 2; } Z80INT(2); z80irqmaxes = 8; } for (int clockcountinternal2 = 0; clockcountinternal2 < ((Z80Corepfclock / 3 / 600) * 1); clockcountinternal2) { if (z80irqid != 0) { if (z80irqid == 1) { Z80DoIRQ(z80irqfn); z80irqfn = 0; } else if (z80irqid >= 3 && z80irqid <= 10) { Z80DoIRQ(z80irqfnqueue[z80irqfnqueuepos2]); z80irqfnqueuepos2 = ((z80irqfnqueuepos2 + 1) & 0x7); for (int cnt = 0; cnt < 10000; cnt++) { clockcounttemporary = Z80Run(); clockcountinternal += clockcounttemporary; clockcountinternal2 += clockcounttemporary; clockcount += clockcounttemporary; } } else { Z80DoNMI(); } if (z80irqid > 3 && z80irqid <= 10) { z80irqid--; } else { z80irqid = 0; } } vbi = true; clockcounttemporary = Z80Run(); z80irqmaxes = 8; clockcountinternal += clockcounttemporary; clockcountinternal2 += clockcounttemporary; clockcount += clockcounttemporary; }  /*vbi = vbi ? false : true;*/ if (ioporte6h & 1) { Z80INT(4); } } clockcountinternalold = clockcountinternal; } ispc80threadinrunningemulation = false; /*clockcount += clockcountinternal; /*drawgrpbool = true;*/ if (beepenabled) { beeprestart(); beep2400play(); } if (beepenabled2 && ispc8801 == true) { beep2restart(); beep2play(); } howmanybeepstopped = 0; howmanybeepstopped_2 = 0; GetSystemTime(&st_goal); ststgoal16 = (st_goal.wMilliseconds) - (st_st.wMilliseconds); if (ststgoal16 < 0) { ststgoal16 += 1000; } if (ststgoal16 < (16 + (boolofwaitiinz80loop ? 1 : 0))) { Sleep((16 + (boolofwaitiinz80loop ? 1 : 0)) - ststgoal16); } }/*while (z80timerbefore == time(NULL)) {}*/ } }//UINT32 z80timemintab[2] = { 0, 0 }; SYSTEMTIME z80timeminta; while (true) { clockcount = 0; int clockcountinternal = 0; int z80timerbefore = time(NULL); while (clockcount < (graphicdraw ? 1830000 : 4000000)) { clockcountinternal = 0; GetSystemTime(&z80timeminta); z80timemintab[0] = (z80timeminta.wMilliseconds) + (time(NULL) * 1000); while (clockcountinternal < (graphicdraw ? 183000 : 400000)) { if (z80irqid != 0) { if (z80irqid == 1) { Z80DoIRQ(z80irqfn); z80irqfn = 0; } else { Z80DoNMI(); } z80irqid = 0; } clockcountinternal += Z80Run(); vbi = vbi ? false : true; } GetSystemTime(&z80timeminta); z80timemintab[1] = (z80timeminta.wMilliseconds) + (time(NULL) * 1000); clockcount += clockcountinternal; int timetowaitive = (z80timemintab[1] - z80timemintab[0]); /*if (timetowaitive < 0) { timetowaitive += 1000; }*/ if ((timetowaitive > 0) && (timetowaitive <= 100)) { Sleep(100 - timetowaitive); } else { Sleep(100); } } int z80timerint = time(NULL) - z80timerbefore; /*if (z80timerint < 1000) { Sleep(1000 - z80timerint); }*/ } }

void BeepService(LPVOID* arg4bs) { while (true) { if (beepenabled) { /*Beep(2400, 100);*/ beep2400play(); Sleep(16); } else { if (isbeepenabledinthecool == false) { beep2400stop(); } } if (beepenabled2) { /*Beep(2400, 100);*/ beep2play(); Sleep(16); } else { if (isbeepenabledinthecool2 == false) { beep2stop(); } } /*if (GN8012_i8272.is_int_pending()) { GN8012.INT(0); }*/ } }
void BeepService2(LPVOID* arg4bs) { while (true) { if (beepenabled2) { /*Beep(2400, 100);*/ beep2play(); Sleep(16); } else { if (isbeepenabledinthecool2 == false) { beep2stop(); } } /*if (GN8012_i8272.is_int_pending()) { GN8012.INT(0); }*/ } }
void PC8012Service(LPVOID* arg4bs) {
    while (FDDCZ80Threadid == 0) { Sleep(0); }
    if (isloadedfddcfirmware == true) {
        SYSTEMTIME st_st; SYSTEMTIME st_goal; int ststgoal16; while (true) { clockcountpc8012 = 0; int clockcountinternal = 0; int z80timerbefore = time(NULL); while (clockcountpc8012 < 4000000) { clockcountinternal = 0; GetSystemTime(&st_st); UINT32 Z80Corepfclock = 4000000 / 60; if (fddconnected == true) { while (true) { clockcountinternal += (GN8012.Execute(1) + 1); if (GN8012_i8272.is_int_pending()) { GN8012.INT(0); } if (clockcountinternal >= Z80Corepfclock) { break; } } } clockcountpc8012 += clockcountinternal; GetSystemTime(&st_goal); ststgoal16 = (st_goal.wMilliseconds) - (st_st.wMilliseconds); if (ststgoal16 < 0) { ststgoal16 += 1000; } if (ststgoal16 < 16) { Sleep(16 - ststgoal16); } } }
    }
    else {
        UINT8 amountofsec = 0;
        UINT8 driveid = 0;
        UINT8 track = 0;
        UINT8 sector = 0;

        UINT8 driveid2 = 0;
        UINT8 track2 = 0;
        UINT8 sector2 = 0;

        UINT8 drivemode = 0x0F;
        UINT16 transaddr = 0;
        UINT16 transsize = 0;
        UINT8 fddcommand = 0;
        UINT8 fddcommandold = 0;
        UINT16 recvsendtmp16;
        RegSet regtmp4du;
        bool readbufexist = false;
        bool iserroroccruuedondu = false;
        UINT16 transaddr2 = 0;
        UINT8 regid4rwreg4du = 0;
        while (true) {
            if (fddconnected == true) {
                fddcommandold = fddcommand;
                fddcommand = recvfdcmd28001c();
                //CreateThread(0, 0, (LPTHREAD_START_ROUTINE)dialogtocheck, ((LPVOID)fddcommand), 0, 0);
                switch (fddcommand) {
                case 0x00:
                    readbufexist = false;
                    iserroroccruuedondu = false;
                    fddcmemory[0x3f22] = 0xc9;
                    fddcmemory[0x3f43] = 0;
                    fddcmemory[0x3f44] = 0;
                    GN8012.GetRegSet(&regtmp4du);
                    regtmp4du.sp = 0x8000 - 2;
                    fddcmemory[0x3ffe] = 0x0c;
                    fddcmemory[0x3fff] = 0x00;
                    GN8012.SetRegSet(&regtmp4du);
                    break;
                case 0x01:
                    amountofsec = recvfdcmd28001();
                    driveid = recvfdcmd28001();
                    track = recvfdcmd28001();
                    sector = recvfdcmd28001();
                    for (int cnt = 0; cnt < (amountofsec * 256); cnt++) {
                        //if ((i8255mac_GN8012(2, 0, 1) & 0x08)) { break; }
                        fddcmemory[cnt] = recvfdcmd28001();
                    }
                    if (GN8012_i8272.Diskstat[driveid].isprotected == true) {
                        iserroroccruuedondu = true;
                    }
                    else {
                        iserroroccruuedondu = false;
                        for (int cnt = 0; cnt < (amountofsec * 256); cnt++) {
                            fddrivebus(cnt % 256, fddcmemory[cnt], (cnt / 256) + sector, ((drivemode & (1 << driveid)) ? (track / 2) : track), ((drivemode & (1 << driveid)) ? ((track % 2) ? 0x400 : 0) : 0) | ((driveid & 3) << 8) | 0);
                        }
                    }
                    break;
                case 0x02:
                    amountofsec = recvfdcmd28001();
                    driveid = recvfdcmd28001();
                    track = recvfdcmd28001();
                    sector = recvfdcmd28001();
                    if (GN8012_i8272.Diskstat[driveid].diskinserted == true) {
                        readbufexist = true;
                        iserroroccruuedondu = false;
                        for (int cnt = 0; cnt < amountofsec; cnt++) {
                            fddrivebus(0, 0, (cnt)+sector, ((drivemode & (1 << driveid)) ? (track / 2) : track), ((drivemode & (1 << driveid)) ? ((track % 2) ? 0x400 : 0) : 0) | ((driveid & 3) << 8) | 1);
                            memcpy(&fddcmemory[(cnt * 256) + 0x1000], readbuf4fddrivebus + (driveid * 256), 256);
                        }
                    }
                    else {
                        readbufexist = false;
                        iserroroccruuedondu = true;
                    }
                    break;
                case 0x03:
                    if (readbufexist == false) {
                        while (true) { if ((i8255mac_GN8012(2, 0, 1) & 0x08)) { break; } }
                    }
                    else {
                        for (int cnt = 0; cnt < (amountofsec * 256); cnt++) {
                            //if ((i8255mac_GN8012(2, 0, 1) & 0x08)) { break; }
                            sendfdcmd28001(fddcmemory[cnt + 0x1000]);
                        }
                        readbufexist = false;
                    }
                    break;
                case 0x04:
                    amountofsec = recvfdcmd28001();
                    driveid = recvfdcmd28001();
                    track = recvfdcmd28001();
                    sector = recvfdcmd28001();
                    driveid2 = recvfdcmd28001();
                    track2 = recvfdcmd28001();
                    sector2 = recvfdcmd28001();
                    if (GN8012_i8272.Diskstat[driveid].diskinserted == true) {
                        if (GN8012_i8272.Diskstat[driveid2].isprotected == true) {
                            iserroroccruuedondu = true;
                        }
                        else {
                            iserroroccruuedondu = false;
                            for (int cnt = 0; cnt < (amountofsec * 256); cnt++) {
                                fddrivebus(cnt % 256, fddrivebus(cnt % 256, 0, (cnt / 256) + sector, ((drivemode & (1 << driveid)) ? (track / 2) : track), ((drivemode & (1 << driveid)) ? ((track % 2) ? 0x400 : 0) : 0) | ((driveid & 3) << 8) | 1), (cnt / 256) + sector2, ((drivemode & (1 << driveid2)) ? (track2 / 2) : track2), ((drivemode & (1 << driveid)) ? ((track2 % 2) ? 0x400 : 0) : 0) | ((driveid2 & 3) << 8) | 0);
                            }
                            readbufexist = false;
                        }
                    }
                    else {
                        iserroroccruuedondu = true;
                    }
                    break;
                case 0x05:
                    driveid = recvfdcmd28001();
                    if (GN8012_i8272.Diskstat[driveid2].isprotected == true) {
                        iserroroccruuedondu = true;
                    }
                    else {
                        iserroroccruuedondu = false;
                        for (int cnt = 0; cnt < (0x10 * 80 * 256); cnt++) {
                            fddrivebus(cnt % 256, 0x00, (cnt / 256) % 0x10, ((drivemode & (1 << driveid)) ? (((cnt / 256) / 0x10) / 2) : ((cnt / 256) / 0x10)), ((drivemode & (1 << driveid)) ? ((((cnt / 256) / 0x10) % 2) ? 0x400 : 0) : 0) | ((driveid & 3) << 8) | 0);
                        }
                    }
                    break;
                case 0x06:
                    sendfdcmd28001(((readbufexist ? 0x40 : 0) | (iserroroccruuedondu ? 0x01 : 0)));
                    break;
                case 0x07:
                    sendfdcmd28001(0x0F | (GN8012_i8272.Diskstat[0].diskinserted ? 0x10 : 0) | (GN8012_i8272.Diskstat[1].diskinserted ? 0x20 : 0) | (GN8012_i8272.Diskstat[2].diskinserted ? 0x40 : 0) | (GN8012_i8272.Diskstat[3].diskinserted ? 0x80 : 0));
                    break;
                case 0x08:
                    sendfdcmd28001(0x80);
                    break;
                case 0x09:
                    transaddr = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    transsize = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    for (int cnt = 0; cnt < transsize; cnt++) {
                        sendfdcmd28001(fddcz80memaccess(transaddr + cnt, 0, 1));
                    }
                    break;
                case 0x0A:
                    fddcz80memaccess(0x00F7, recvfdcmd28001(), 2);
                    break;
                case 0x0B:
                    transaddr = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    transsize = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    for (int cnt = 0; cnt < transsize; cnt++) {
                        sendfdcmd28001(fddcz80memaccess(transaddr + cnt, 0, 1));
                    }
                    break;
                case 0x0C:
                    transaddr = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    transsize = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    for (int cnt = 0; cnt < transsize; cnt++) {
                        fddcz80memaccess(transaddr + cnt, recvfdcmd28001(), 0);
                    }
                    break;
                case 0x0D:
                    transaddr = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    GN8012.GetRegSet(&regtmp4du);
                    regtmp4du.pc = transaddr;
                    GN8012.SetRegSet(&regtmp4du);
                    while (regtmp4du.pc != 0x000C) {
                        GN8012.Execute(1);
                        if (GN8012_i8272.is_int_pending()) { GN8012.INT(0); }
                        GN8012.GetRegSet(&regtmp4du);
                    }
                    regtmp4du.sp = 0x8000 - 2;
                    fddcmemory[0x3ffe] = 0x0c;
                    fddcmemory[0x3fff] = 0x00;
                    GN8012.SetRegSet(&regtmp4du);
                    break;
                case 0x0E:
                    amountofsec = recvfdcmd28001();
                    driveid = recvfdcmd28001();
                    track = recvfdcmd28001();
                    sector = recvfdcmd28001();
                    transaddr = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    if (GN8012_i8272.Diskstat[driveid].diskinserted == true) {
                        iserroroccruuedondu = false;
                        for (int cnt = 0; cnt < amountofsec * 256; cnt++) {
                            fddcz80memaccess(transaddr + cnt, fddrivebus(cnt % 256, 0, (cnt / 256) + sector, ((drivemode & (1 << driveid)) ? (track / 2) : track), ((drivemode & (1 << driveid)) ? ((track % 2) ? 0x400 : 0) : 0) | ((driveid & 3) << 8) | 1), 0);
                        }
                    }
                    else {
                        iserroroccruuedondu = true;
                    }
                    break;
                case 0x0F:
                    amountofsec = recvfdcmd28001();
                    driveid = recvfdcmd28001();
                    track = recvfdcmd28001();
                    sector = recvfdcmd28001();
                    transaddr = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    if (GN8012_i8272.Diskstat[driveid].diskinserted == true) {
                        if (GN8012_i8272.Diskstat[driveid].isprotected == false) {
                            iserroroccruuedondu = false;
                            for (int cnt = 0; cnt < amountofsec * 256; cnt++) {
                                fddrivebus(cnt % 256, fddcz80memaccess(transaddr + cnt, 0, 1), (cnt / 256) + sector, ((drivemode & (1 << driveid)) ? (track / 2) : track), ((drivemode & (1 << driveid)) ? ((track % 2) ? 0x400 : 0) : 0) | ((driveid & 3) << 8) | 0);
                            }
                        }
                        else {
                            iserroroccruuedondu = true;
                        }
                    }
                    else {
                        iserroroccruuedondu = true;
                    }
                    break;
                case 0x11:
                    amountofsec = recvfdcmd28001();
                    driveid = recvfdcmd28001();
                    track = recvfdcmd28001();
                    sector = recvfdcmd28001();
                    for (int cnt = 0; cnt < (amountofsec * 128); cnt++) {
                        //if ((i8255mac_GN8012(2, 0, 1) & 0x08)) { break; }
                        recvsendtmp16 = recvfdcmd28001w();
                        fddcmemory[(cnt * 2) + 0] = (recvsendtmp16 >> (8 * 0)) & 0xFF;
                        fddcmemory[(cnt * 2) + 1] = (recvsendtmp16 >> (8 * 1)) & 0xFF;
                    }
                    if (GN8012_i8272.Diskstat[driveid].isprotected == true) {
                        iserroroccruuedondu = true;
                    }
                    else {
                        iserroroccruuedondu = false;
                        for (int cnt = 0; cnt < (amountofsec * 256); cnt++) {
                            fddrivebus(cnt % 256, fddcmemory[cnt], (cnt / 256) + sector, ((drivemode & (1 << driveid)) ? (track / 2) : track), ((drivemode & (1 << driveid)) ? ((track % 2) ? 0x400 : 0) : 0) | ((driveid & 3) << 8) | 0);
                        }
                    }
                    break;
                case 0x12:
                    if (readbufexist == false) {
                        while (true) { if ((i8255mac_GN8012(2, 0, 1) & 0x08)) { break; } }
                    }
                    else {
                        for (int cnt = 0; cnt < (amountofsec * 128); cnt++) {
                            //if ((i8255mac_GN8012(2, 0, 1) & 0x08)) { break; }
                            sendfdcmd28001w((fddcmemory[((cnt * 2) + 0) + 0x1000] & 0xFF) | ((fddcmemory[((cnt * 2) + 1) + 0x1000] & 0xFF) << 8));
                        }
                        readbufexist = false;
                    }
                    break;
                case 0x13:
                    sendfdcmd28001(0);
                    sendfdcmd28001(0);
                    sendfdcmd28001(0);
                    sendfdcmd28001(0);
                    sendfdcmd28001(track / 2);
                    sendfdcmd28001(track % 2);
                    sendfdcmd28001(sector);
                    sendfdcmd28001(256);
                    break;
                case 0x14:
                    driveid = recvfdcmd28001();
                    sendfdcmd28001((driveid & 3) | ((track == 0) ? 0x10 : 0) | ((track % 2) ? 0x04 : 0) | (GN8012_i8272.Diskstat[driveid].isprotected ? 0x40 : 0) | ((drivemode & (1 << driveid)) ? 0x08 : 0) | 0x20);
                    break;
                case 0x15:
                    transaddr = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    transsize = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    for (int cnt = 0; cnt < (transsize / 2); cnt++) {
                        //if ((i8255mac_GN8012(2, 0, 1) & 0x08)) { break; }
                        sendfdcmd28001w(((fddcz80memaccess(transaddr + ((cnt * 2) + 0), 0, 1) << (8 * 0)) & 0xFF) | ((fddcz80memaccess(transaddr + ((cnt * 2) + 1), 0, 1) << (8 * 1)) & 0xFF00));
                    }
                    break;
                case 0x16:
                    transaddr = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    transsize = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    for (int cnt = 0; cnt < (transsize / 2); cnt++) {
                        //if ((i8255mac_GN8012(2, 0, 1) & 0x08)) { break; }
                        recvsendtmp16 = recvfdcmd28001w();
                        fddcz80memaccess(transaddr + ((cnt * 2) + 0), ((recvsendtmp16 >> (8 * 0)) & 0xFF), 0);
                        fddcz80memaccess(transaddr + ((cnt * 2) + 1), ((recvsendtmp16 >> (8 * 1)) & 0xFF), 0);
                    }
                    break;
                case 0x17:
                    drivemode = recvfdcmd28001();
                    break;
                case 0x18:
                    sendfdcmd28001(drivemode);
                    break;
                case 0x1C:
                    transaddr = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    transaddr2 = (fddcmemory[0x3f43] & 0x00FF) | ((fddcmemory[0x3f44] << 8) & 0xFF00);
                    fddcz80memaccess(transaddr2, fddcmemory[0x3f45], 0);
                    fddcmemory[0x3f43] = (transaddr >> (8 * 0)) & 0xFF;
                    fddcmemory[0x3f44] = (transaddr >> (8 * 1)) & 0xFF;
                    fddcmemory[0x3f45] = fddcz80memaccess(transaddr, 0, 1);
                    fddcz80memaccess(transaddr, 0xcf, 0);
                    break;
                case 0x1D:
                    regid4rwreg4du = recvfdcmd28001();
                    transaddr = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    GN8012.GetRegSet(&regtmp4du);
                    switch (regid4rwreg4du) {
                    case 0x00:
                        regtmp4du.af = transaddr;
                        break;
                    case 0x01:
                        regtmp4du.bc = transaddr;
                        break;
                    case 0x02:
                        regtmp4du.de = transaddr;
                        break;
                    case 0x03:
                        regtmp4du.hl = transaddr;
                        break;
                    case 0x04:
                        regtmp4du.xaf = transaddr;
                        break;
                    case 0x05:
                        regtmp4du.xbc = transaddr;
                        break;
                    case 0x06:
                        regtmp4du.xde = transaddr;
                        break;
                    case 0x07:
                        regtmp4du.xhl = transaddr;
                        break;
                    case 0x08:
                        regtmp4du.ix = transaddr;
                        break;
                    case 0x09:
                        regtmp4du.iy = transaddr;
                        break;
                    case 0x0a:
                        GN8012.SetIntVec(transaddr);
                        break;
                    case 0x0b:
                        regtmp4du.pc = transaddr;
                        break;
                    case 0x0c:
                        regtmp4du.sp = transaddr;
                        break;
                    }
                    GN8012.SetRegSet(&regtmp4du);
                    break;
                case 0x1E:
                    regid4rwreg4du = recvfdcmd28001();
                    transaddr = ((recvfdcmd28001() << 8) & 0xFF00) | (recvfdcmd28001() & 0xFF);
                    GN8012.GetRegSet(&regtmp4du);
                    switch (regid4rwreg4du) {
                    case 0x00:
                        transaddr = regtmp4du.af;
                        break;
                    case 0x01:
                        transaddr = regtmp4du.bc;
                        break;
                    case 0x02:
                        transaddr = regtmp4du.de;
                        break;
                    case 0x03:
                        transaddr = regtmp4du.hl;
                        break;
                    case 0x04:
                        transaddr = regtmp4du.xaf;
                        break;
                    case 0x05:
                        transaddr = regtmp4du.xbc;
                        break;
                    case 0x06:
                        transaddr = regtmp4du.xde;
                        break;
                    case 0x07:
                        transaddr = regtmp4du.xhl;
                        break;
                    case 0x08:
                        transaddr = regtmp4du.ix;
                        break;
                    case 0x09:
                        transaddr = regtmp4du.iy;
                        break;
                    case 0x0a:
                        transaddr = 0;
                        break;
                    case 0x0b:
                        transaddr = regtmp4du.pc;
                        break;
                    case 0x0c:
                        transaddr = regtmp4du.sp;
                        break;
                    }
                    sendfdcmd28001((transaddr >> 8) & 0xFF);
                    sendfdcmd28001((transaddr >> 0) & 0xFF);
                    break;
                }
            }
            else {
                Sleep(16);
            }
        }
    }
}
void PC8012Diskwaiter(LPVOID* arg4bs) {
    while (true) {
        GN8012_i8272.i8272a_waitforexec();
        Sleep(0);
    }
}

void RTIService(LPVOID* arg4rtisv) { while (true) { /*if (ioporte6h & 1) { Z80INT(4); } Sleep(2);*/ } }

void __stdcall serialdaemon(void* prm_0) {
    COMSTAT tempcomstate;
    while (true) {
        if (ttyconnected == true) { if (rxdataready == false) { if (cmtfileloc != 0) { ClearCommError(cmtfileloc, 0, &tempcomstate); if (tempcomstate.cbInQue != 0) { ReadFile(cmtfileloc, &serialchar, 1, 0, 0); rxdataready = true; if ((ioporte6h & 4) && ispc8801 == true) { Z80INT(0); } } } serialstat = true; } }
    }
}

int xsiz10times = 0;
int ysiz10times = 0;

uint8 color4draw = 0;

UINT8 prevchar[256];

void myFillRect(HDC prm_0,const RECT* prm_2, HBRUSH prm_3) {
    if (pBit == 0) { return; }
    UINT32 colorpaletmp = GetBrushColor(prm_3);
    colorpaletmp = (((colorpaletmp >> (8 * 0)) & 0xFF) << (8 * 2)) | (((colorpaletmp >> (8 * 1)) & 0xFF) << (8 * 1)) | (((colorpaletmp >> (8 * 2)) & 0xFF) << (8 * 0));
    for (int cnt2 = prm_2->left; cnt2 < prm_2->right; cnt2++) {
        for (int cnt = prm_2->top; cnt < prm_2->bottom; cnt++) {
            if (cnt <= 479 && cnt2 <= 639 && cnt >= 0 && cnt2 >= 0) {
                (*(UINT32*)(&pBit[((479 - cnt) * (640 * 4)) + (cnt2 * 4)])) = colorpaletmp;
            }
        }
    }
}

void SetPalette4emu(int prm_0) { color4draw = prm_0 + (greenmonitor ? 128 : 0); }
void SetPalette4emu2(int prm_0) { color4draw = prm_0 + (greenmonitor ? 768 : 256); }

void SetPset2(int prm_0, int prm_1) {
    xsiz10times = colorfullgraphicmode ? ((ispc8801 == true) ? 100 : 200) : 100; ysiz10times = (hiresgrpresol200 == false && ispc8801 == true) ? 120 : ((colorfullgraphicmode == true && hiresgrpresol200 == true) ? 120 : 240);
    //xsiz10times = pc8001widthflag ? 10 : 20; ysiz10times = grpheight25 ? 24 : 30;
    //xsiz10times = pc8001widthflag ? 100 : 200; ysiz10times = grpheight25 ? 213 : 267;
    //xsiz10times = 10; ysiz10times = 10;
    if (colorfullgraphicmode == true && hiresgrpresol200 == true) {
        rs.left = (((prm_0 + 0) * xsiz10times) / 100);
        rs.top = ((((prm_1 * 2) + 0) * ysiz10times) / 100);
        rs.right = (((prm_0 + 1) * xsiz10times) / 100);
        rs.bottom = ((((prm_1 * 2) + 3) * ysiz10times) / 100);
    }
    else {
        rs.left = (((prm_0 + 0) * xsiz10times) / 100);
        rs.top = (((prm_1 + 0) * ysiz10times) / 100);
        rs.right = (((prm_0 + 1) * xsiz10times) / 100);
        rs.bottom = (((prm_1 + 1) * ysiz10times) / 100);
    }
    if (chkedbb8 >= 1) {
        DWORD basecolor1 = GetPixel(hCDC, rs.left, rs.top);
        DWORD basecolor2 = GetBrushColor(hBackGround[color4draw]);
        if (basecolor1 == GetBrushColor(hBackGround[32 + bgcolor])) { basecolor1 = basecolor2; }
        HBRUSH hbkgtmp = CreateSolidBrush((((((basecolor1 >> (8 * 0)) & 0xFF) + ((((basecolor1 >> (8 * 0)) & 0xFF) - ((basecolor2 >> (8 * 0)) & 0xFF)) / 2)) & 0xFF) << (8 * 0)) | (((((basecolor1 >> (8 * 1)) & 0xFF) + ((((basecolor1 >> (8 * 1)) & 0xFF) - ((basecolor2 >> (8 * 1)) & 0xFF)) / 2)) & 0xFF) << (8 * 1)) | (((((basecolor1 >> (8 * 2)) & 0xFF) + ((((basecolor1 >> (8 * 2)) & 0xFF) - ((basecolor2 >> (8 * 2)) & 0xFF)) / 2)) & 0xFF) << (8 * 2)));
        myFillRect(hCDC, &rs, hbkgtmp);
        DeleteObject(hbkgtmp);
    }
    else {
#if 0
        for (int cnt2 = 0; cnt2 < (rs.bottom - rs.top); cnt2++) {
            for (int cnt = 0; cnt < (rs.right - rs.left); cnt++) {
                pBit[((((479 - (rs.top + cnt2)) * 640) + (rs.left + cnt)) * 4) + 0] = ((palette32[color4draw] >> (8 * 3)) & 0xFF);
                pBit[((((479 - (rs.top + cnt2)) * 640) + (rs.left + cnt)) * 4) + 1] = ((palette32[color4draw] >> (8 * 2)) & 0xFF);
                pBit[((((479 - (rs.top + cnt2)) * 640) + (rs.left + cnt)) * 4) + 2] = ((palette32[color4draw] >> (8 * 1)) & 0xFF);
                pBit[((((479 - (rs.top + cnt2)) * 640) + (rs.left + cnt)) * 4) + 3] = ((palette32[color4draw] >> (8 * 0)) & 0xFF);
            }
        }
#endif
        myFillRect(hCDC, &rs, hBackGround[color4draw]);
    }
}
void SetBox2(int prm_0, int prm_1, int prm_2, int prm_3) {
    xsiz10times = colorfullgraphicmode ? ((ispc8801 == true) ? 100 : 200) : 100; ysiz10times = (hiresgrpresol200 == false && ispc8801 == true) ? 120 : ((colorfullgraphicmode == true && hiresgrpresol200 == true) ? 120 : 240);
    //xsiz10times = pc8001widthflag ? 10 : 20; ysiz10times = grpheight25 ? 24 : 30;
    //xsiz10times = pc8001widthflag ? 100 : 200; ysiz10times = grpheight25 ? 213 : 267;
    //xsiz10times = 10; ysiz10times = 10;
    rs.left = (((prm_0 + 0) * xsiz10times) / 100);
    rs.top = (((prm_1 + 0) * ysiz10times) / 100);
    rs.right = (((prm_2 + 0) * xsiz10times) / 100);
    rs.bottom = (((prm_3 + 0) * ysiz10times) / 100);
    if (chkedbb8 >= 1) {
        DWORD basecolor1 = GetPixel(hCDC, rs.left, rs.top);
        DWORD basecolor2 = GetBrushColor(hBackGround[color4draw]);
        if (basecolor1 == GetBrushColor(hBackGround[32 + bgcolor])) { basecolor1 = basecolor2; }
        HBRUSH hbkgtmp = CreateSolidBrush((((((basecolor1 >> (8 * 0)) & 0xFF) + ((((basecolor1 >> (8 * 0)) & 0xFF) - ((basecolor2 >> (8 * 0)) & 0xFF)) / 2)) & 0xFF) << (8 * 0)) | (((((basecolor1 >> (8 * 1)) & 0xFF) + ((((basecolor1 >> (8 * 1)) & 0xFF) - ((basecolor2 >> (8 * 1)) & 0xFF)) / 2)) & 0xFF) << (8 * 1)) | (((((basecolor1 >> (8 * 2)) & 0xFF) + ((((basecolor1 >> (8 * 2)) & 0xFF) - ((basecolor2 >> (8 * 2)) & 0xFF)) / 2)) & 0xFF) << (8 * 2)));
        if (colorfullgraphicmode == true && hiresgrpresol200 == true) {
            for (int cnt = prm_1; cnt < prm_3; cnt++) {
                rs.left = (((prm_0 + 0) * xsiz10times) / 100);
                rs.top = ((((cnt * 2) + 0) * ysiz10times) / 100);
                rs.right = (((prm_2 + 0) * xsiz10times) / 100);
                rs.bottom = ((((cnt * 2) + 3) * ysiz10times) / 100);
                myFillRect(hCDC, &rs, hbkgtmp);
            }
        }
        else {
            myFillRect(hCDC, &rs, hbkgtmp);
        }
        DeleteObject(hbkgtmp);
    }
    else {
#if 0
        for (int cnt2 = 0; cnt2 < (rs.bottom - rs.top); cnt2++) {
            for (int cnt = 0; cnt < (rs.right - rs.left); cnt++) {
                pBit[((((479 - (rs.top + cnt2)) * 640) + (rs.left + cnt)) * 4) + 0] = ((palette32[color4draw] >> (8 * 3)) & 0xFF);
                pBit[((((479 - (rs.top + cnt2)) * 640) + (rs.left + cnt)) * 4) + 1] = ((palette32[color4draw] >> (8 * 2)) & 0xFF);
                pBit[((((479 - (rs.top + cnt2)) * 640) + (rs.left + cnt)) * 4) + 2] = ((palette32[color4draw] >> (8 * 1)) & 0xFF);
                pBit[((((479 - (rs.top + cnt2)) * 640) + (rs.left + cnt)) * 4) + 3] = ((palette32[color4draw] >> (8 * 0)) & 0xFF);
            }
        }
#endif
        if (colorfullgraphicmode == true && hiresgrpresol200 == true) {
            for (int cnt = prm_1; cnt < prm_3; cnt++) {
                rs.left = (((prm_0 + 0) * xsiz10times) / 100);
                rs.top = ((((cnt * 2) + 0) * ysiz10times) / 100);
                rs.right = (((prm_2 + 0) * xsiz10times) / 100);
                rs.bottom = ((((cnt * 2) + 3) * ysiz10times) / 100);
                myFillRect(hCDC, &rs, hBackGround[color4draw]);
            }
        }
        else {
            myFillRect(hCDC, &rs, hBackGround[color4draw]);
        }
    }
}

void SetPset(int prm_0, int prm_1) {
    xsiz10times = pc8001widthflag ? 100 : 200; ysiz10times = grpheight25 ? 240 : 300;
    //xsiz10times = pc8001widthflag ? 10 : 20; ysiz10times = grpheight25 ? 24 : 30;
    //xsiz10times = pc8001widthflag ? 100 : 200; ysiz10times = grpheight25 ? 213 : 267;
    //xsiz10times = 10; ysiz10times = 10;
    rs.left = (((prm_0 + 0) * xsiz10times) / 100);
    rs.top = (((prm_1 + 0) * ysiz10times) / 100);
    rs.right = (((prm_0 + 1) * xsiz10times) / 100);
    rs.bottom = (((prm_1 + 1) * ysiz10times) / 100);
    if (chkedbb8 >= 1) {
        DWORD basecolor1 = GetPixel(hCDC, rs.left, rs.top);
        DWORD basecolor2 = GetBrushColor(hBackGround[color4draw]);
        if (basecolor1 == GetBrushColor(hBackGround[32 + bgcolor])) { basecolor1 = basecolor2; }
        HBRUSH hbkgtmp = CreateSolidBrush( ((( ((basecolor1 >> (8 * 0)) & 0xFF) + (( ((basecolor1 >> (8 * 0)) & 0xFF) - ((basecolor2 >> (8 * 0)) & 0xFF)) / 2)) & 0xFF) << (8 * 0)) | (((((basecolor1 >> (8 * 1)) & 0xFF) + ((((basecolor1 >> (8 * 1)) & 0xFF) - ((basecolor2 >> (8 * 1)) & 0xFF)) / 2)) & 0xFF) << (8 * 1)) | (((((basecolor1 >> (8 * 2)) & 0xFF) + ((((basecolor1 >> (8 * 2)) & 0xFF) - ((basecolor2 >> (8 * 2)) & 0xFF)) / 2)) & 0xFF) << (8 * 2)) );
        myFillRect(hCDC, &rs, hbkgtmp);
        DeleteObject(hbkgtmp);
    }
    else {
        myFillRect(hCDC, &rs, hBackGround[color4draw]);
    }
}
void SetBox(int prm_0, int prm_1, int prm_2, int prm_3) {
    xsiz10times = pc8001widthflag ? 100 : 200; ysiz10times = grpheight25 ? 240 : 300;
    //xsiz10times = pc8001widthflag ? 10 : 20; ysiz10times = grpheight25 ? 24 : 30;
    //xsiz10times = pc8001widthflag ? 100 : 200; ysiz10times = grpheight25 ? 213 : 267;
    //xsiz10times = 10; ysiz10times = 10;
    rs.left = (((prm_0 + 0) * xsiz10times) / 100);
    rs.top = (((prm_1 + 0) * ysiz10times) / 100);
    rs.right = (((prm_2 + 0) * xsiz10times) / 100);
    rs.bottom = (((prm_3 + 0) * ysiz10times) / 100);
    if (chkedbb8 >= 1) {
        DWORD basecolor1 = GetPixel(hCDC, rs.left, rs.top);
        DWORD basecolor2 = GetBrushColor(hBackGround[color4draw]);
        if (basecolor1 == GetBrushColor(hBackGround[32 + bgcolor])) { basecolor1 = basecolor2; }
        HBRUSH hbkgtmp = CreateSolidBrush((((((basecolor1 >> (8 * 0)) & 0xFF) + ((((basecolor1 >> (8 * 0)) & 0xFF) - ((basecolor2 >> (8 * 0)) & 0xFF)) / 2)) & 0xFF) << (8 * 0)) | (((((basecolor1 >> (8 * 1)) & 0xFF) + ((((basecolor1 >> (8 * 1)) & 0xFF) - ((basecolor2 >> (8 * 1)) & 0xFF)) / 2)) & 0xFF) << (8 * 1)) | (((((basecolor1 >> (8 * 2)) & 0xFF) + ((((basecolor1 >> (8 * 2)) & 0xFF) - ((basecolor2 >> (8 * 2)) & 0xFF)) / 2)) & 0xFF) << (8 * 2)));
        myFillRect(hCDC, &rs, hbkgtmp);
        DeleteObject(hbkgtmp);
    }
    else {
        myFillRect(hCDC, &rs, hBackGround[color4draw]);
    }
}

void SetBGCL(){
    rs.left = 0;
    rs.top = 0;
    rs.right = 640;
    rs.bottom = 480;
    myFillRect(hCDC, &rs, hBackGround[color4draw]);
}


void DrawFont_(int prm_0, int prm_1, int prm_2, bool prm_3) {
    for (int fonty = 0; fonty < 8; fonty++) {
        for (int fontx = 0; fontx < 8; fontx++) {
            if ((((isenabledpcg ? ((prm_2 < 128) ? fontrom[prm_2 * 8 + fonty] : pcgcharram[(prm_2 * 8 + fonty) - 0x400]) : fontrom[prm_2 * 8 + fonty]) << fontx) & 128) ^ (prm_3 ? 128 : 0)) { SetPset(prm_0 + fontx, prm_1 + fonty); }
        }
    }

}
void DrawFont(int prm_0, int prm_1, int prm_2) {
    for (int fonty = 0; fonty < 8; fonty++) {
        for (int fontx = 0; fontx < 8; fontx++) {
            if (((isenabledpcg ? ((prm_2 < 128) ? fontrom[prm_2 * 8 + fonty] : pcgcharram[(prm_2 * 8 + fonty)-0x400]) : fontrom[prm_2 * 8 + fonty]) << fontx) & 128) { SetPset(prm_0 + fontx, prm_1 + fonty); }
        }
    }

}
void DrawFontUS(int prm_0, int prm_1, int prm_2,int prm_3) {
    int fonty = 0;
    for (int fontx = 0; fontx < 8; fontx++) {
        if ((((isenabledpcg ? ((prm_3 < 128) ? fontrom[prm_3 * 8 + 7] : pcgcharram[(prm_3 * 8 + 7) - 0x400]) : fontrom[prm_3 * 8 + 7]) & (isenabledpcg ? ((prm_2 < 128) ? fontrom[prm_2 * 8 + fonty] : pcgcharram[(prm_2 * 8 + fonty) - 0x400]) : fontrom[prm_2 * 8 + fonty])) << fontx) & 128) { SetPset(prm_0 + fontx, prm_1 + fonty - 1); }
    }

}

int attributeold = -1;
int attributetmp = -1;
int attributeold2 = -1;
int attributetmp2 = -1;
int attributeold3 = -1;
int attributetmp3 = -1;
bool attributegcold = false;
uint8 charattribute = 0;
uint8 fontcolors = 0;
uint8 grpcolors = 0;
uint8 charattributeold = 0;
bool blinkai = false;
bool blinkai2 = false;
bool semigraphicenabled = false;
int blinkwaitisti = 0;
int blinkwaitisti2 = 0;

int lp_ggxy[2];
bool mousemvenabled = false;

uint8 graphiccodes[(80*25)][2];

uint8 colorbool[25];
uint8 charattributefinal = 0;

void DrawGrp() {
    charattribute = 0;
    if (ispc8801 == true) {
        for (int cnt = 0; cnt < 8; cnt++) { DeleteObject(hBackGround[cnt + 64]); DeleteObject(hBackGround[cnt + 64 + 128]); DeleteObject(hBackGround[cnt + 72]); DeleteObject(hBackGround[cnt + 72 + 128]); hBackGround[cnt + 64] = CreateSolidBrush(RGB(((palette512_8bt[cnt][0] >> 3) & 7) * 73 / 2, ((palette512_8bt[cnt][1] >> 0) & 7) * 73 / 2, ((palette512_8bt[cnt][0] >> 0) & 7) * 73 / 2)); hBackGround[cnt + 72] = CreateSolidBrush(RGB(((palette512_8bt[cnt][2] >> 3) & 7) * 73 / 2, ((palette512_8bt[cnt][3] >> 0) & 7) * 73 / 2, ((palette512_8bt[cnt][2] >> 0) & 7) * 73 / 2)); hBackGround[cnt + 64 + 128] = CreateSolidBrush(RGB(0, ((((((palette512_8bt[cnt][0] >> 3) & 7) + ((palette512_8bt[cnt][1] >> 0) & 7) + ((palette512_8bt[cnt][0] >> 0) & 7)) / 3) + 1) * 28) + 3, 0)); hBackGround[cnt + 72 + 128] = CreateSolidBrush(RGB(0, ((((((palette512_8bt[cnt][2] >> 3) & 7) + ((palette512_8bt[cnt][3] >> 0) & 7) + ((palette512_8bt[cnt][2] >> 0) & 7)) / 3) + 1) * 28), 0)); }
        for (int cnt = 0; cnt < 256; cnt++) {
            palette32[cnt] = GetBrushColor(hBackGround[cnt]);
        }
    }
    if ((graphicdraw == true)/* || (true)*/) {
        if (colorfullgraphicmode == false && fullgrpmode == false) {
            SetPalette4emu(32 + 8);
        }
        else {
            if (crtmodectrl == false) {
                SetPalette4emu(32 + bgcolor);
            }
            else { SetPalette4emu(32 + 8); }
        }
        SetBGCL();
        for (chkedbb8 = 0; chkedbb8 < (((dmatc[2] & 0x3FFF) >= 0xbb8) ? (((dmatc[2] & 0x3FFF) / 0xbb8) + 1) : 1); chkedbb8++) {
            bool breakdowndgp = false;
            UINT8 charattributetmp4backupforlater = 0;
            bool isattributerouleschanged = false;

            for (int drawbacky = 0; drawbacky < 25; drawbacky++) {
                semigraphicenabled = false;
                for (int drawbackx = 0; drawbackx < 80; drawbackx++) {
                    uint8 char4show = crtcmemaccess(dmaas[2] + (chkedbb8 * 0xbb8) + ((drawbackx)+(drawbacky * 120)), 0, 1);
                    attributetmp = -1; attributeold = -1; fontcolors = colorbool[drawbacky]; grpcolors = colorbool[drawbacky]; attributegcold = false;
                    attributeold2 = -1; attributetmp2 = -1; attributeold3 = -1; attributetmp3 = -1;
                    if (crtcatsc != 1) {
                        if (crtcatsc & 4) {
                            for (int cnt = 0; cnt < attributesize; cnt++) {
                                grpcolors = 7; fontcolors = 7;
                                if (dmatc[2] >= ((chkedbb8 * 0xbb8) + ((drawbackx)+(drawbacky * (80 + (attributesize * 2)))) + (cnt + 80))) { upd3301stat |= 8; }
                                if ((ioporte6h & 2) && ispc8801 == true) { if ((upd3301stat & 0x10) && !(upd3301intm & 2)) { upd3301stat |= 4; } Z80INT(2); } charattribute = crtcmemaccess(dmaas[2] + (chkedbb8 * 0xbb8) + (((cnt * 2) + 1) + (drawbacky * (80 + (attributesize * 2)))), 0, 1) & 0xFF;
                            }
                        }
                        else {
                            isattributerouleschanged = false;
                            uint8 charattributetmp = 0;
                            for (int cnt = 19; cnt >= 0; cnt--) {
                                if (dmatc[2] >= ((chkedbb8 * 0xbb8) + ((drawbackx)+(drawbacky * 120)) + (cnt + 80))) { upd3301stat |= 8; }
                                if ((crtcmemaccess(dmaas[2] + (chkedbb8 * 0xbb8) + (((cnt * 2) + 80) + (drawbacky * 120)), 0, 1) & 0x80)) { isattributerouleschanged = true; }
                                uint8 charattributetmp = crtcmemaccess(dmaas[2] + (chkedbb8 * 0xbb8) + (((cnt * 2) + 81) + (drawbacky * 120)), 0, 1); if ((crtcmemaccess(dmaas[2] + (chkedbb8 * 0xbb8) + (((cnt * 2) + 80) + (drawbacky * 120)), 0, 1) & 0x7F) != (64 | 32)) { if (charattributetmp & 8) { attributetmp = crtcmemaccess(dmaas[2] + (chkedbb8 * 0xbb8) + (((cnt * 2) + 80) + (drawbacky * 120)), 0, 1); if ((attributetmp == drawbackx && isattributerouleschanged == false) || (((attributetmp) > drawbackx) && isattributerouleschanged == true)) { attributeold = attributetmp; fontcolors = (charattributetmp >> 5) & 7; grpcolors = (charattributetmp >> 5) & 7; if (charattributetmp & 16) { semigraphicenabled = true; } else { charattribute = charattribute & 0x7F; semigraphicenabled = false; } } } else { attributetmp3 = crtcmemaccess(dmaas[2] + (chkedbb8 * 0xbb8) + (((cnt * 2) + 80) + (drawbacky * 120)), 0, 1); if ((attributetmp3 == drawbackx && isattributerouleschanged == false) || (((attributetmp3) > drawbackx) && isattributerouleschanged == true) || (((((charattributetmp & 128) ? true : false) != attributegcold) && (charattributetmp & 128)))) { charattribute = charattributetmp; attributeold3 = attributetmp3; attributegcold = (charattributetmp & 128) ? true : false; } } }
                            }
                        }
                        if (semigraphicenabled == true) { charattribute |= 128; }
                        graphiccodes[(80 * drawbacky) + drawbackx][0] = charattribute ^ (crtcreverted ? 4 : 0);
                        graphiccodes[(80 * drawbacky) + drawbackx][1] = grpcolors;
                        colorbool[drawbacky] = grpcolors;
                    }
                    else {
                        graphiccodes[(80 * drawbacky) + drawbackx][0] = 0;
                        graphiccodes[(80 * drawbacky) + drawbackx][1] = 7;
                        colorbool[drawbacky] = 0xFF;
                    }
                }
            }
            if (isattributerouleschanged == true) {
                charattributefinal = charattribute;
            }
            else { charattributefinal = 0; }
            if (fullgraphicdraw == true) {

                if (ispc8801 == true) {
                    for (int drawbacky = 0; drawbacky < ((grpheight25 || ((colorfullgraphicmode == true && hiresgrpresol200 == true) && ispc8801 == true)) ? 25 : 20); drawbacky++) {
                        for (int drawbackx = 0; drawbackx < (pc8001widthflag ? (linecharnum + 2) : ((linecharnum + 2) / 2)); drawbackx++) {
                            if (colorfullgraphicmode == true && hiresgrpresol200 == true) {
                                //SetPalette4emu2(((UINT32)palette512_8bt[0][0] & 0x3F) | (((UINT32)palette512_8bt[0][1] & 0x7) << 6));
                                SetPalette4emu(64);
                                SetBox2(((drawbackx + 0) * (8 * (pc8001widthflag ? 1 : 2))), ((drawbacky + 0) * (8 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))), ((drawbackx + 1) * (8 * (pc8001widthflag ? 1 : 2))) - 0, ((drawbacky + 1) * (8 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) - 0);
                                for (int cnt = 0; cnt < (8 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1)); cnt++) {
                                    for (int cnt2 = 0; cnt2 < (8 * (pc8001widthflag ? 1 : 2)); cnt2++) {
                                        if ((((showstatefor88grp & 2) ? 0 : (((gvram[0][((drawbackx * (pc8001widthflag ? 1 : 2)) + (cnt2 / 8)) + (drawbacky * (640 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (cnt * 80)] << (cnt2 % 8)) & 0x80) >> 7)) | ((showstatefor88grp & 4) ? 0 : (((gvram[1][((drawbackx * (pc8001widthflag ? 1 : 2)) + (cnt2 / 8)) + (drawbacky * (640 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (cnt * 80)] << (cnt2 % 8)) & 0x80) >> 6)) | ((showstatefor88grp & 8) ? 0 : (((gvram[2][((drawbackx * (pc8001widthflag ? 1 : 2)) + (cnt2 / 8)) + (drawbacky * (640 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (cnt * 80)] << (cnt2 % 8)) & 0x80) >> 5))) != 0) {
                                            //SetPalette4emu2(((UINT32)palette512_8bt[(((showstatefor88grp & 2) ? 0 : (((gvram[0][(drawbackx * 8) + (drawbacky * (640 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (cnt * 80)] << cnt2) & 0x80) >> 7)) | ((showstatefor88grp & 4) ? 0 : (((gvram[1][(drawbackx * 8) + (drawbacky * (640 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (cnt * 80)] << cnt2) & 0x80) >> 6)) | ((showstatefor88grp & 8) ? 0 : (((gvram[2][(drawbackx * 8) + (drawbacky * (640 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (cnt * 80)] << cnt2) & 0x80) >> 5)))][0] & 0x3F) | (((UINT32)palette512_8bt[(((showstatefor88grp & 2) ? 0 : (((gvram[0][(drawbackx * 8) + (drawbacky * (640 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (cnt * 80)] << cnt2) & 0x80) << 0)) | ((showstatefor88grp & 4) ? 0 : (((gvram[1][(drawbackx * 8) + (drawbacky * (640 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (cnt * 80)] << cnt2) & 0x80) << 1)) | ((showstatefor88grp & 8) ? 0 : (((gvram[2][(drawbackx * 8) + (drawbacky * (640 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (cnt * 80)] << cnt2) & 0x80) << 2)))][1] & 0x7) << 6));
                                            SetPalette4emu((((showstatefor88grp & 2) ? 0 : (((gvram[0][((drawbackx * (pc8001widthflag ? 1 : 2)) + (cnt2 / 8)) + (drawbacky * (640 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (cnt * 80)] << (cnt2 % 8)) & 0x80) >> 7)) | ((showstatefor88grp & 4) ? 0 : (((gvram[1][((drawbackx * (pc8001widthflag ? 1 : 2)) + (cnt2 / 8)) + (drawbacky * (640 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (cnt * 80)] << (cnt2 % 8)) & 0x80) >> 6)) | ((showstatefor88grp & 8) ? 0 : (((gvram[2][((drawbackx * (pc8001widthflag ? 1 : 2)) + (cnt2 / 8)) + (drawbacky * (640 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (cnt * 80)] << (cnt2 % 8)) & 0x80) >> 5))) + 64);
                                            SetPset2((drawbackx * (8 * (pc8001widthflag ? 1 : 2))) + cnt2, (drawbacky * (8 * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt);
                                        }
                                    }
                                }
                            }
                            else {
                                if (crtmodectrl == false) { SetPalette4emu((graphiccodes[(80 * (drawbacky / (linespace ? 2 : 1))) + (drawbackx * (pc8001widthflag ? 1 : 2))][1] & 7) + 32); }
                                else { SetPalette4emu(32 + 9); }
                                //SetPalette4emu2((((UINT32)palette512_8bt[(graphiccodes[(80 * (((((drawbacky * 10) / (grpheight25 ? 16 : 20)) / 10) + 0) / (linespace ? 2 : 1))) + (drawbackx * (pc8001widthflag ? 1 : 2))][1] & 7)][0] & 0x3F) << 0) | (((UINT32)palette512_8bt[(graphiccodes[(80 * (((((drawbacky * 10) / (grpheight25 ? 16 : 20)) / 10) + 0) / (linespace ? 2 : 1))) + (drawbackx * (pc8001widthflag ? 1 : 2))][1] & 7)][1] & 0x3F) << 6));
                                if (hiresgrpresol200 == true) {
                                    for (int cnt = 0; cnt < ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1)); cnt++) {
                                        for (int cnt2 = 0; cnt2 < (8 * (pc8001widthflag ? 1 : 2)); cnt2++) {
                                            if ((((gvram[2][((drawbackx * (pc8001widthflag ? 1 : 2)) + (cnt2 / 8)) + ((drawbacky * ((grpheight25 ? 640 : 800) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) - ((((drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt) >= 200) ? 16000 : 0)) + (cnt * 80)] << (cnt2 % 8)) & 0x80) && ((drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt) <  200) && ((showstatefor88grp & 8) == 0)) { SetPset2((drawbackx * (8 * (pc8001widthflag ? 1 : 2))) + cnt2, (drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt); }
                                            if ((((gvram[1][((drawbackx * (pc8001widthflag ? 1 : 2)) + (cnt2 / 8)) + ((drawbacky * ((grpheight25 ? 640 : 800) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) - ((((drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt) >= 200) ? 16000 : 0)) + (cnt * 80)] << (cnt2 % 8)) & 0x80) && ((drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt) <  200) && ((showstatefor88grp & 4) == 0)) { SetPset2((drawbackx * (8 * (pc8001widthflag ? 1 : 2))) + cnt2, (drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt); }
                                            if ((((gvram[0][((drawbackx * (pc8001widthflag ? 1 : 2)) + (cnt2 / 8)) + ((drawbacky * ((grpheight25 ? 640 : 800) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) - ((((drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt) >= 200) ? 16000 : 0)) + (cnt * 80)] << (cnt2 % 8)) & 0x80) && ((drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt) <  200) && ((showstatefor88grp & 2) == 0)) { SetPset2((drawbackx * (8 * (pc8001widthflag ? 1 : 2))) + cnt2, (drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt); }
                                        }
                                    }
                                }
                                else {
                                    for (int cnt = 0; cnt < ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1)); cnt++) {
                                        for (int cnt2 = 0; cnt2 < (8 * (pc8001widthflag ? 1 : 2)); cnt2++) {
                                                 if ((((gvram[0][((drawbackx * (pc8001widthflag ? 1 : 2)) + (cnt2 / 8)) + ((drawbacky * ((grpheight25 ? 640 : 800) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) - ((((drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt) >= 200) ? 16000 : 0)) + (cnt * 80)] << (cnt2 % 8)) & 0x80) && ((drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt) <  200) && ((showstatefor88grp & 2) == 0)) { SetPset2((drawbackx * (8 * (pc8001widthflag ? 1 : 2))) + cnt2, (drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt); }
                                            else if ((((gvram[1][((drawbackx * (pc8001widthflag ? 1 : 2)) + (cnt2 / 8)) + ((drawbacky * ((grpheight25 ? 640 : 800) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) - ((((drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt) >= 200) ? 16000 : 0)) + (cnt * 80)] << (cnt2 % 8)) & 0x80) && ((drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt) >= 200) && ((showstatefor88grp & 4) == 0)) { SetPset2((drawbackx * (8 * (pc8001widthflag ? 1 : 2))) + cnt2, (drawbacky * ((grpheight25 ? 8 : 10) * (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + cnt); }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else {
                    for (int drawbacky = 0; drawbacky < (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? (grpheight25 ? 50 : 40) : (grpheight25 ? 25 : 20)); drawbacky++) {
                        for (int drawbackx = 0; drawbackx < (colorfullgraphicmode ? 40 : 80); drawbackx++) {
                            //if (((chkedbb8 * 0xbb8) + ((drawbackx * (colorfullgraphicmode ? 2 : 1)) + (drawbacky * 120))) > (dmatc[2] & 0x3FFF)) { breakdowndgp = true; break; }
                            uint8 char4show = crtcmemaccess(dmaas[2] + (chkedbb8 * 0xbb8) + ((drawbackx * (colorfullgraphicmode ? 2 : 1)) + ((drawbacky / ((hiresgrpresol200 == false && ispc8801 == true) ? 2 : 1)) * 120)), 0, 1);
                            charattribute = (graphiccodes[(80 * (drawbacky / (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (drawbackx * (colorfullgraphicmode ? 2 : 1))][0]);
                            fontcolors = graphiccodes[(80 * (drawbacky / (((hiresgrpresol200 == false || (hiresgrpresol200 == true && colorfullgraphicmode == false)) && ispc8801 == true) ? 2 : 1))) + (drawbackx * (colorfullgraphicmode ? 2 : 1))][1] & 7; grpcolors = graphiccodes[(80 * (drawbacky / ((hiresgrpresol200 == false && ispc8801 == true) ? 2 : 1))) + (drawbackx * (colorfullgraphicmode ? 2 : 1))][1] & 7;
                            //grpcolors = 9;
                            if (fullgrpmode == true && colorfullgraphicmode == false) {
                                SetPalette4emu(32 + 8);
                                SetBox2(((drawbackx + 0) * 8), ((drawbacky + 0) * (grpheight25 ? 8 : 10)), ((drawbackx + 1) * 8) - 0, ((drawbacky + 1) * (grpheight25 ? 8 : 10)) - 0);
                            }
                            else if (fullgrpmode == false && colorfullgraphicmode == false) {
                                if (crtmodectrl == false) { SetPalette4emu(32 + bgcolor); }
                                else { SetPalette4emu(32 + 8); }
                                SetBox2(((drawbackx + 0) * 8), ((drawbacky + 0) * (grpheight25 ? 8 : 10)), ((drawbackx + 1) * 8) - 0, ((drawbacky + 1) * (grpheight25 ? 8 : 10)) - 0);
                            }
                            else if (colorfullgraphicmode == true) {
                                SetPalette4emu((fullgrpmode ? 1 : 0));
                                SetBox2(((drawbackx + 0) * 8), ((drawbacky + 0) * (grpheight25 ? 8 : 10)), ((drawbackx + 1) * 8) - 0, ((drawbacky + 1) * (grpheight25 ? 8 : 10)) - 0);
                            }
                            for (int cnt2 = 0; cnt2 < (grpheight25 ? 8 : 10); cnt2++) {
                                for (int cnt = 0; cnt < 8; cnt++) {
                                    if (crtmodectrl == false) { SetPalette4emu(32 + bgcolor); }
                                    else { SetPalette4emu(32 + 8); }
                                    if (colorfullgraphicmode == false) {
                                        if ((gvram[0][(drawbackx + (((drawbacky * (grpheight25 ? 8 : 10)) + cnt2) * 80))] << (cnt)) & 128) {
                                            if (fullgrpmode == false) {
                                                if (crtmodectrl == false) { if (charattribute & 128) { SetPalette4emu(32 + grpcolors); } else { SetPalette4emu(32 + fontcolors); } }
                                                else { SetPalette4emu(32 + 9); }
                                            }
                                            else {
                                                SetPalette4emu(32 + bgcolor);
                                            }
                                            SetPset2((drawbackx * 8) + cnt, (drawbacky * (grpheight25 ? 8 : 10)) + cnt2);
                                        }
                                    }
                                    else {
                                        UINT8 colortmp = (((gvram[0][(((drawbackx * 2) + (((drawbacky * (grpheight25 ? 8 : 10)) + cnt2) * 80))) + ((cnt % 8) / 4)] << (((cnt % 8) % 4) * 2)) & 192) >> 6) & 3;
                                        if (colortmp != 0) {
                                            if (colortmp <= 2) {
                                                SetPalette4emu((colortmp * 2) | (fullgrpmode ? 1 : 0));
                                            }
                                            SetPset2((drawbackx * 8) + cnt, (drawbacky * (grpheight25 ? 8 : 10)) + cnt2);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if ((showstatefor88grp & 1) == 0) {
                if (crtcactive == 1) {
                    breakdowndgp = false;
                    if (crtmodectrl == false) { SetPalette4emu(32 + bgcolor); }
                    else { SetPalette4emu(32 + 8); }
                    for (int drawbacky = 0; drawbacky < (grpheight25 ? 25 : 20); (linespace ? (drawbacky += 2) : (drawbacky++))) {
                        for (int drawbackx = 0; drawbackx < (pc8001widthflag ? (linecharnum + 2) : ((linecharnum + 2) / 2)); drawbackx++) {
                            if (((chkedbb8 * 0xbb8) + ((drawbackx * (pc8001widthflag ? 1 : 2)) + ((drawbacky / (linespace ? 2 : 1)) * 120))) > (dmatc[2] & 0x3FFF)) { breakdowndgp = true; break; }
                            uint8 char4show = crtcmemaccess(dmaas[2] + (chkedbb8 * 0xbb8) + ((drawbackx * (pc8001widthflag ? 1 : 2)) + ((drawbacky / (linespace ? 2 : 1)) * 120)), 0, 1);
                            attributegcold = false;
                            charattribute = (graphiccodes[(80 * (drawbacky / (linespace ? 2 : 1))) + (drawbackx * (pc8001widthflag ? 1 : 2))][0]);
                            fontcolors = graphiccodes[(80 * (drawbacky / (linespace ? 2 : 1))) + (drawbackx * (pc8001widthflag ? 1 : 2))][1] & 7; grpcolors = graphiccodes[(80 * (drawbacky / (linespace ? 2 : 1))) + (drawbackx * (pc8001widthflag ? 1 : 2))][1] & 7;
                            if (((blinkai == false) || ((charattribute & 2) == 0)) && ((charattribute & 1) == 0)) {
                                if (crtmodectrl == false) { if (charattribute & 128) { SetPalette4emu(grpcolors); } else { SetPalette4emu(fontcolors); } }
                                else { SetPalette4emu(9); }
                                bool rendreverted = ((charattribute & 4) ? true : false);
                                if ((((((cursx != -1 && cursy != -1) && (cursx == (drawbackx * (pc8001widthflag ? 1 : 2)) && cursy == drawbacky)) && ((blinkai2 == false) || (cursortype == 2))) ? true : false))) { rendreverted = (rendreverted ? false : true); }
                                if (linecharnum != 0){
                                    if ((charattribute & 128) || (attributegcold == true)) { for (int cnt = 0; cnt < 8; cnt++) { if (((char4show >> cnt) & 1) ^ (rendreverted ? 1 : 0)) { SetBox(((drawbackx + 0) * 8) + (4 * ((cnt / 4) + 0)) - 0, ((drawbacky + 0) * 8) + ((int)(2 * ((cnt % 4) + 0))) - 0, ((drawbackx + 0) * 8) + (4 * ((cnt / 4) + 1)) - 0, ((drawbacky + 0) * 8) + ((int)(2 * ((cnt % 4) + 1))) - 0); } } }
                                    else { /*DrawFontUS(((drawbackx + 0) * 8), ((drawbacky + 0) * 9), char4show, prevchar[drawbackx]);*/ DrawFont_(((drawbackx + 0) * 8), ((drawbacky + 0) * 8), char4show, rendreverted); prevchar[drawbackx] = char4show; }
                                    if ((cursortype & 2) == 0) {
                                        if (blinkai2 == false || (cursortype & 1) == 0) { SetBox(((drawbackx + 0) * 8), ((drawbacky + 0) * 8) + 6, ((drawbackx + 1) * 8) - 1, ((drawbacky + 1) * 8) - 1); }
                                    }
                                    //SetPalette4emu(9); SetBox(((drawbackx + 0) * 8), ((drawbacky + 0) * 8), ((drawbackx + 1) * 8) - 0, ((drawbacky + 1) * 8) - 0);
                                    if (charattribute & 64) { SetBox(((drawbackx + 0) * 8) + 3, ((drawbacky + 0) * 8), ((drawbackx + 0) * 8) + 4, ((drawbacky + 1) * 8) - 0); }
                                    if (charattribute & 32) { SetBox(((drawbackx + 0) * 8), ((drawbacky + 1) * 8) - 2, ((drawbackx + 1) * 8), ((drawbacky + 1) * 8) - 1); }
                                    if (charattribute & 16) { SetBox(((drawbackx + 0) * 8), ((drawbacky + 0) * 8), ((drawbackx + 1) * 8), ((drawbacky + 0) * 8) + 0); }
                                }
                            }
                            else {
                                if ((((((cursx != -1 && cursy != -1) && (cursx == (drawbackx * (pc8001widthflag ? 1 : 2)) && cursy == drawbacky)) && ((blinkai2 == false) || (cursortype == 2))) ? true : false))) { 
                                    if (crtmodectrl == false) { if (charattribute & 128) { SetPalette4emu(grpcolors); } else { SetPalette4emu(fontcolors); } }
                                    else { SetPalette4emu(9); }
                                    SetBox(((drawbackx + 0) * 8), ((drawbacky + 0) * 8), ((drawbackx + 1) * 8), ((drawbacky + 1) * 8));
                                }
                            }
                        }
                        if (breakdowndgp == true) { break; }
                    }
                    if (breakdowndgp == true) { break; }
                }
                else { upd3301stat |= 8; upd3301stat &= ~16; }
            }

        }
    }
    else { SetPalette4emu(8); SetBGCL(); }
    if ((blinkwaitisti % ((((4 - blinkingtime) * 1) * 2) + 1)) == 0) { blinkai2 = blinkai2 ? false : true; }
    if ((((4 - blinkingtime) * 2) * 2) <= blinkwaitisti) { blinkai = (blinkwaitisti2 != 0) ? false : true; blinkwaitisti2++; if (blinkwaitisti2 == 4) { blinkwaitisti2 = 0; } blinkwaitisti = 0; }
    blinkwaitisti++;
    RECT rw4rend;
    if (bool4showwin) {
        GetClientRect(hwnd4mw, &rw4rend);
        if ((rw4rend.right != 0) && (rw4rend.bottom != 0)) { SetStretchBltMode(hdc, (isharftoneenabled ? STRETCH_HALFTONE : COLORONCOLOR)); StretchBlt(hdc, 0, 0, rw4rend.right, rw4rend.bottom, hCDC, 0, 0, 640, 480, SRCCOPY); if (isharftoneenabled == true){ SetBrushOrgEx(hdc, 0, 0, NULL); } }
    }
    else {
        GetClientRect(HWNDfullscr, &rw4rend);
        if ((rw4rend.right != 0) && (rw4rend.bottom != 0)) { SetStretchBltMode(hdcfullscr, (isharftoneenabled ? STRETCH_HALFTONE : COLORONCOLOR)); StretchBlt(hdcfullscr, 0, 0, rw4rend.right, rw4rend.bottom, hCDC, 0, 0, 640, 480, SRCCOPY); if (isharftoneenabled == true){ SetBrushOrgEx(hdcfullscr, 0, 0, NULL); } }
    }
}

void Drawbackground(LPVOID* arg4dbg) {
    SYSTEMTIME st_st; SYSTEMTIME st_goal; int ststgoal16;
    while (true) {
        /*if (drawgrpbool == true) {
            DrawGrp();
            drawgrpbool = false;
        }*/
        GetSystemTime(&st_st);
        DrawGrp();
        GetSystemTime(&st_goal); ststgoal16 = (st_goal.wMilliseconds) - (st_st.wMilliseconds); if (ststgoal16 < 0) { ststgoal16 += 1000; } if (ststgoal16 < 17) { Sleep(17 - ststgoal16); }
        //Sleep(16);
    }
}

void ResetEmu() {
    attributesize = 20;
    biosromenabled = false;

    clockcount = 0;
    videoenabled = false;
    beepenabled = true;
    beepenabled2 = false;
    uPD8251config[0]=0;
    uPD8251config[1]&=~0x38;
    uPD8251config[2]=0;
    uPD8251config[3]=0;
    upd8251configate = 0;
    overrunerror = false;
    rxdataready = false;
    //crtc2 = 0;
    cursortype = 0;
    linespace = false;

    bgcolor = 0;
    colorgraphicmode = false;
    colorfullgraphicmode = false;
    graphicdraw = true;
    fullgraphicdraw = false;
    gvramenabled = 0;
    grpmode = false;
    fullgrpmode = false;
    romtype = false;

    uipin = 0;
    vbi = false;
    rtcdata = false;
    cmtdatard = false;
    prtready = false;

    othercrtcio = false;
    upd31speclzsig;
    litepeninp;

    upd3301stat = 0;
    uPD3301prm = 0;
    upd3301cmd = 0;

    dmaas[0]=0;
    dmaas[1]=0;
    dmaas[2]=0;
    dmaas[3]=0;
    dmatc[0]=0;
    dmatc[1]=0;
    dmatc[2]=0;
    dmatc[3]=0;
    dmachiocnt = 0;
    dmamodestat = 0;
    dmaseq = 0;
    crtcactive = 0;
    cursx = -1;
    cursy = -1;

    crtcreverted = false;

    grpheight25 = false;
    blinkingtime = 0;
    cursxtmp = 0;

    uopout = 0;

    ret = 0;

    seq = 0;

    crtc3 = 0;

    pc8001kb1p;

    crtcldsclkenable = false;
    rtcclkenable = false;
    rtcstbenable = false;
    prtenable = false;
    prtenable0 = false;

    cassettemtstate = false;
    cmtbinsnd = 0;
    cmtdatawr = false;
    crtmodectrl = false;
    pc8001widthflag = false;

    pch = 0;

    rtcpos = 0;

    ioporte6h = 0;

    mousemvenabled = false;
    rommode = false;
    showstatefor88grp = 0;
    hiresgrpresol200 = false;
    fastesttvramenabled = false;
    eromsl = 0;
    videooutputmode = 0;
    palettemode = false;
    gvramaccessmode = false;
    soundintmask = false;
    galuctrl = 0;
    galuop = 0;
    extendedromsel = 0xff;
    textwindoffsetadru8 = 0;
    kanjiromaddr1 = 0;
    kanjiromaddr2 = 0;

    linecharnum = 0;
    intmasklevel = 0xf;
    crtcatsc = 0;
    upd3301intm = 0;

    dictromstat[0] = 0;
    dictromstat[1] = 0xFF;

    arememorybankenabled = 0;
    rs232crate = 0;
    ispc8801mk2srormore = false;
    for (int cnt = 0; cnt < 25; cnt++) { colorbool[cnt] = 0xff; }

    if (cmtfileloc != 0) { CloseHandle(cmtfileloc); }

    //memset(memory, 0, 65536);
    //memset(fddcmemory, 0, 16384);
    //memset(bankedmemory, 0, 0x8000 * 4);
    if (ispc8801 == false) {
        memset(memory, 0, 32768);
    }

    GN8012_i8255.init_i8255();
    //GN8012_i8272.init_i8272a();
    GN8012.Reset();

    GN80_i8255.init_i8255();
    GN80_i8255_2.init_i8255();
    z80irqmaxes = 8;

    Z80Init();
}

void checkthefddaccess(HWND hWnd) {
    HMENU hWnd_Menu = GetMenu(hWnd);
    while (true) {
        for (int cnt = 0; cnt < 4; cnt++) {
            if (fdd[cnt].diskaccessing == true) {
                fdd[cnt].diskaccessing = false;
                CheckMenuItem(hWnd_Menu, ID_32781 + cnt, MF_BYCOMMAND | MF_CHECKED);
            }
            else {
                CheckMenuItem(hWnd_Menu, ID_32781 + cnt, MF_BYCOMMAND | MF_UNCHECKED);
            }
        }
        Sleep(1000);
    };
}

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名
WCHAR szWindowClass2[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    for (int cnt = 0; cnt < 25; cnt++) { colorbool[cnt] = 0xff; }
    bsmode = 0xc0;

    beepinit();

    GN8012memaccess = fddcz80memaccess;
    GN8012.Reset();
    GN8012_i8272.init_i8272a();
    GN8012_i8255.init_i8255();
    GN8012_i8255.i8255phaccess = i8255mac_GN8012;

    GN80_i8255.init_i8255();
    GN80_i8255.i8255phaccess = i8255mac_GN80;
    GN80_i8255_2.init_i8255();

    for (int cnt = 0; cnt < 4; cnt++) {
        GN8012_i8272.Diskstat[cnt].fddphyaccess = fddrivebus;
    }

    bz80dll = LoadLibraryA("bz80dll.dll");

    if (bz80dll != 0) {
        bz80_setz80memaccess = (void (*)(int (*tmp)(int, int, int)))GetProcAddress(bz80dll,"setz80memaccess");
        bz80_Z80Init = (void (*)(void))GetProcAddress(bz80dll, "Z80Init");
        bz80_Z80Reset = (void (*)(void))GetProcAddress(bz80dll, "Z80Reset");
        bz80_Z80Run = (int  (*)(void))GetProcAddress(bz80dll, "Z80Run");
        bz80_Z80DoIRQ = (void (*)(uint8 vector))GetProcAddress(bz80dll, "Z80DoIRQ");
        bz80_Z80DoNMI = (void (*)(void))GetProcAddress(bz80dll, "Z80DoNMI");
        bz80_getz80regs = (int (*)())GetProcAddress(bz80dll, "getz80regs");
        bz80_getextz80regs = (int (*)(int tmp))GetProcAddress(bz80dll, "getextz80regs");
    }

    Z80Init();
    setz80memaccess(z80memaccess);

    FILE* checkexist4x86plugin=fopen("peldr.nt.dll", "rb");
    if (checkexist4x86plugin != 0) {
        HMODULE peldr4runtheplugin = LoadLibraryA("peldr.nt.dll"); 
        PeLdrLoadModule = (typeofPeLdrLoadModule*)GetProcAddress(peldr4runtheplugin,"PeLdrLoadModule");
        PeLdrLoadModuleA = (typeofPeLdrLoadModuleA*)GetProcAddress(peldr4runtheplugin, "PeLdrLoadModuleA");
        PeLdrGetProcAddressA = (typeofPeLdrGetProcAddressA*)GetProcAddress(peldr4runtheplugin, "PeLdrGetProcAddressA");
        PeLdrGetModuleBase = (typeofPeLdrGetModuleBase*)GetProcAddress(peldr4runtheplugin, "PeLdrGetModuleBase");
        PeLdrFindModuleByBase = (typeofPeLdrFindModuleByBase*)GetProcAddress(peldr4runtheplugin, "PeLdrFindModuleByBase");
        fclose(checkexist4x86plugin);
    }
    checkexist4x86plugin = fopen("np21w_emu.dll", "rb");
    if (checkexist4x86plugin != 0) {
        HMODULE emu4runtheplugin = LoadLibraryA("np21w_emu.dll");
        EmuInitialize = (typeofEmuInitialize*)GetProcAddress(emu4runtheplugin,"EmuInitialize");
        EmuExecute= (typeofEmuExecute*)GetProcAddress(emu4runtheplugin, "EmuExecute");
        fclose(checkexist4x86plugin);
    }

    WIN32_FIND_DATAA pluginlister;
    HANDLE handleofpluginlister;
    typedef void typeoftheinitplugin(void*);
    void (*initplugin)(void*);
    handleofpluginlister = FindFirstFileA("plugins\\*.dll",&pluginlister);
    if (handleofpluginlister != INVALID_HANDLE_VALUE) {
        for (int cnt = 0; cnt < 1024; cnt++) {
            pluginctx[cnt].ptrofz80memaccess = z80memaccess;
            pluginctx[cnt].version = 0x00000001;
            if ((pluginlister.dwFileAttributes & 0x10) == 0) {
                HMODULE handleofplugindll = LoadLibraryA(pluginlister.cFileName);
                if (handleofplugindll == 0) {
                    pluginctx[cnt].isexecutedontheemulator = true;
                    handleofplugindll = (HMODULE)x86_LoadLibraryA(pluginlister.cFileName);
                    initplugin = (typeoftheinitplugin*)x86_GetProcAddress((DWORD)handleofplugindll, "InitPlugin");
                    EmuExecute((DWORD)&initplugin, 1, &pluginctx[cnt]);
                }
                else {
                    pluginctx[cnt].isexecutedontheemulator = false;
                    initplugin = (typeoftheinitplugin*)GetProcAddress(handleofplugindll, "InitPlugin");
                    initplugin(&pluginctx[cnt]);
                }
            }
            if (FindNextFileA(handleofpluginlister, &pluginlister) == false) { break; }
        }
    }

    for (int cnt = 0; cnt < 16; cnt++) { memset(erom[cnt % 4][cnt / 4], 0xff, 0x2000); }


    FILE* biosfile = fopen("n88basic.rom", "rb");
    if (biosfile != 0) {
        ispc8801 = true;
        fread(n88rom, 0x8000, 1, biosfile);
        fread(n80rom, 0x2000, 1, biosfile);
        fseek(biosfile, 0x2000, SEEK_CUR);
        fread(erom[0][0], 0x2000, 1, biosfile);
        fread(erom[0][1], 0x2000, 1, biosfile);
        fread(erom[0][2], 0x2000, 1, biosfile);
        fread(erom[0][3], 0x2000, 1, biosfile);
        fseek(biosfile, 0x2000, SEEK_CUR);
        fread(bios, 0x6000, 1, biosfile);
        fclose(biosfile);
        FILE* biosfile = fopen("n80basic.rom", "rb");
        if (biosfile != 0) {
            fread(bios, 0x6000, 1, biosfile);
            fread(n80rom, 0x2000, 1, biosfile);
            fclose(biosfile);
            n80_8000 = true;
        }
        biosfile = fopen("n88_0.rom", "rb");
        if (biosfile != 0) {
            fread(erom[0][0], 0x2000, 1, biosfile);
            fclose(biosfile);
        }
        biosfile = fopen("n88_1.rom", "rb");
        if (biosfile != 0) {
            fread(erom[0][1], 0x2000, 1, biosfile);
            fclose(biosfile);
        }
        biosfile = fopen("n88_2.rom", "rb");
        if (biosfile != 0) {
            fread(erom[0][2], 0x2000, 1, biosfile);
            fclose(biosfile);
        }
        biosfile = fopen("n88_3.rom", "rb");
        if (biosfile != 0) {
            fread(erom[0][3], 0x2000, 1, biosfile);
            fclose(biosfile);
        }
        crtc2 = 0xd2;
        bsmode = 0xeb;
    }
    else {
        FILE* biosfile = fopen("nbasic.rom", "rb");
        if (biosfile != 0) {
            fread(bios, 0x6000, 1, biosfile);
            fread(n80rom, 0x2000, 1, biosfile);
            fclose(biosfile);
            FILE* biosfile = fopen("n80.rom", "rb");
            if (biosfile != 0) {
                fread(n80rom, 0x2000, 1, biosfile);
                fclose(biosfile);
            }
            else { n80_8000 = true; }
        }
        else {
            FILE* biosfile = fopen("n80basic.rom", "rb");
            if (biosfile != 0) {
                fread(bios, 0x6000, 1, biosfile);
                fclose(biosfile);
            }
        }
    }
    biosfile = fopen("dict.rom", "rb");
    if (biosfile != 0) {
        fread(dicrom, 0x80000, 1, biosfile);
        fclose(biosfile);
    }
    isloadedfddcfirmware = false;
    fddconnected = true;
    FILE* fddbiosfile = fopen("n80s31.rom", "rb");
    if (fddbiosfile != 0) {
        fread(fddcrom, 0x800, 1, fddbiosfile);
        fclose(fddbiosfile);
        fddconnected = true;
        isloadedfddcfirmware = true;
    }

    FILE* fontfile = fopen("font.rom", "rb");
    if (fontfile != 0) {
        fread(fontrom, 0x800, 1, fontfile);
        fclose(fontfile);
    }
    fontfile = fopen("kfont1.rom", "rb");
    if (fontfile != 0) {
        fread(kanjirom1, 0x20000, 1, fontfile);
        fclose(fontfile);
    }
    fontfile = fopen("kfont2.rom", "rb");
    if (fontfile != 0) {
        fread(kanjirom2, 0x20000, 1, fontfile);
        fclose(fontfile);
    }

    time_t timer = time(NULL);
    timexforch1 = localtime(&timer);

    //rtctimeforminus

    timexforch1tm_year4ml = (timexforch1->tm_year - 121);
    //timexforch1->tm_year -= (9+timexforch1tm_year4ml);//9;

    //timexforch1[0] = timexforch1charx->tm_sec, timexforch1charx->tm_min, timexforch1charx->tm_hour, timexforch1charx->tm_mday, timexforch1charx->tm_mon, timexforch1charx->tm_year - 9, 0, 0, 0;

    FDDCZ80Threadid = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)PC8012Service, 0, 0, 0);
    Z80Threadid = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RunZ80Infinity, 0, 0, 0);
    BSThreadid = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)BeepService, 0, 0, 0);
    /*if (ispc8801 == true) {
        BS2Threadid = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)BeepService2, 0, 0, 0);
    }*/
    BGThreadid = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Drawbackground, 0, 0, 0);
	//RTIThreadid = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RTIService, 0, 0, 0);
    SERThreadid = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)serialdaemon, 0, 0, 0);
    //SERWThreadid = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)serialdaemonx, 0, 0, 0);
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)PC8012Diskwaiter, 0, 0, 0);

    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GDI8001, szWindowClass, MAX_LOADSTRING);
    wcscat(szWindowClass2, szWindowClass);
    wcscat(szWindowClass2, L"_fullscr");
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GDI8001));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, 0, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GDI8001));
    wcex.hCursor        = LoadCursor(0, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GDI8001);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    WNDCLASSEXW wcex2;

    wcex2.cbSize = sizeof(WNDCLASSEX);

    wcex2.style = CS_HREDRAW | CS_VREDRAW;
    wcex2.lpfnWndProc = WndProc;
    wcex2.cbClsExtra = 0;
    wcex2.cbWndExtra = 0;
    wcex2.hInstance = hInstance;
    wcex2.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GDI8001));
    wcex2.hCursor = LoadCursor(0, IDC_ARROW);
    wcex2.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex2.lpszMenuName = 0;//MAKEINTRESOURCEW(IDC_GDI8001);
    wcex2.lpszClassName = szWindowClass2;
    wcex2.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    RegisterClassExW(&wcex2);

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 780, 640, 0, 0, hInstance, 0);
   HWNDfullscr = CreateWindowW(szWindowClass2, 0, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_MAXIMIZE, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0, 0, hInstance, 0);
   ShowWindow(HWNDfullscr, 5);
   UpdateWindow(HWNDfullscr);
   SetWindowPos(HWNDfullscr, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
   ShowWindow(HWNDfullscr, 0);
   UpdateWindow(HWNDfullscr);

   if (!hWnd)
   {
      return FALSE;
   }

   hwnd4mw = hWnd;

   hdc = GetDC(hWnd);
   hdcfullscr = GetDC(HWNDfullscr);

   BITMAPINFOHEADER *pbi;
   pbi = (LPBITMAPINFOHEADER)GlobalAlloc(GPTR, sizeof(BITMAPINFOHEADER));
   pbi->biBitCount = 32;

   pbi->biSize = sizeof(BITMAPINFOHEADER);
   pbi->biWidth = 640;
   pbi->biHeight = 480;
   pbi->biPlanes = 1;

   //hdcb = CreateCompatibleDC(hdc);
   hbDib = CreateDIBSection(hdc, (BITMAPINFO*)pbi, DIB_RGB_COLORS, (void**)&pBit, NULL, 0);
   hCDC = CreateCompatibleDC(hdc);
   //hCBitmap = CreateCompatibleBitmap(hdc, 640, 480);
   hCBitmap = 0;
   hCDCfullscr = CreateCompatibleDC(hdcfullscr);
   hbOld = (HBITMAP)SelectObject(hCDC, hbDib);
   hCBitmapfullscr = CreateCompatibleBitmap(hdcfullscr, 640, 480);
   hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
   for (int cnt = 0; cnt < 1280; cnt++) {
       if (cnt < 8) { hBackGround[cnt] = CreateSolidBrush(RGB(((cnt >> 1) & 1) * 255, ((cnt >> 2) & 1) * 255, ((cnt >> 0) & 1) * 255)); }
       else if (cnt == 8) { hBackGround[cnt] = CreateSolidBrush(RGB(0, 0, 0)); }
       else if (cnt == 9) { hBackGround[cnt] = CreateSolidBrush(RGB(255, 255, 255)); }
       else if ((cnt < 24) && (cnt > 15)) { hBackGround[cnt] = CreateSolidBrush(RGB(((cnt >> 1) & 1) * 230, ((cnt >> 2) & 1) * 230, ((cnt >> 0) & 1) * 230)); }
       else if (cnt == 24) { hBackGround[cnt] = CreateSolidBrush(RGB(0, 0, 0)); }
       else if (cnt == 25) { hBackGround[cnt] = CreateSolidBrush(RGB(230, 230, 230)); }
       else if ((cnt > 31) && (cnt < 40)) { hBackGround[cnt] = CreateSolidBrush(RGB(((((cnt - 32) >> 1) & 1) * 253) + 1, ((((cnt - 32) >> 2) & 1) * 253) + 1, ((((cnt - 32) >> 0) & 1) * 253) + 1)); }
       else if (cnt == 40) { hBackGround[cnt] = CreateSolidBrush(RGB(1, 1, 1)); }
       else if (cnt == 41) { hBackGround[cnt] = CreateSolidBrush(RGB(253, 253, 253)); }
       else if ((cnt >= (0 + 128)) && (cnt < (8 + 128))) { hBackGround[cnt] = CreateSolidBrush(RGB(0,((cnt - (0 + 127))*28)+3,0)); }
       else if (cnt == (8 + 128)) { hBackGround[cnt] = CreateSolidBrush(RGB(0, 31, 0)); }
       else if (cnt == (9 + 128)) { hBackGround[cnt] = CreateSolidBrush(RGB(0, 255, 0)); }
       else if ((cnt < (24 + 128)) && (cnt > (15 + 128))) { hBackGround[cnt] = CreateSolidBrush(RGB(0, ((cnt - (16 + 127)) * 25) + 5, 0)); }
       else if (cnt == (24 + 128)) { hBackGround[cnt] = CreateSolidBrush(RGB(0, 30, 0)); }
       else if (cnt == (25 + 128)) { hBackGround[cnt] = CreateSolidBrush(RGB(0, 230, 0)); }
       else if ((cnt > (31 + 128)) && (cnt < (40 + 128))) { hBackGround[cnt] = CreateSolidBrush(RGB(1, ((cnt - (32 + 127)) * 28) + 1, 1)); }
       else if (cnt == (40 + 128)) { hBackGround[cnt] = CreateSolidBrush(RGB(1, 28 + 1, 1)); }
       else if (cnt == (41 + 128)) { hBackGround[cnt] = CreateSolidBrush(RGB(0, 253, 0)); }
       else if (cnt < (128 + 128)) { hBackGround[cnt] = CreateSolidBrush(RGB(0, 0, 0)); }
       else if (cnt > 255 && cnt < 768) { hBackGround[cnt] = CreateSolidBrush(RGB(((((cnt - 256) >> 3) & 7) * 31), ((((cnt - 256) >> 6) & 7) * 31), ((((cnt - 256) >> 0) & 7) * 31))); }
       else if (cnt > 767 && cnt < 1280) { hBackGround[cnt] = CreateSolidBrush(RGB(0, (((((((cnt - 768) >> 3) & 7) + 1) * 28) + (((((cnt - 768) >> 6) & 7) + 1) * 28) + (((((cnt - 768) >> 0) & 7) + 1) * 28)) / 3), 0)); }
       else { hBackGround[cnt] = CreateSolidBrush(RGB(0, 33, 0)); }
   }
   for (int cnt = 0; cnt < 256; cnt++) {
       palette32[cnt] = GetBrushColor(hBackGround[cnt]);
   }

   hOldCBitmap = (HBITMAP)SelectObject(hCDC, hCBitmap);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   CreateThread(0, 0, (LPTHREAD_START_ROUTINE)checkthefddaccess, (LPVOID)hWnd, 0, 0);
   if (ispc8801 == true) {
       ModifyMenuA(GetSubMenu(GetMenu(hWnd),3), ID_DIPSW_N80, MF_STRING, ID_DIPSW_N80, "N88 / N BASIC");
   }
   CheckMenuItem(GetMenu(hWnd), ID_DIPSW_N80, MF_BYCOMMAND | ((crtc2 & 1) ? MF_CHECKED : MF_UNCHECKED));
   CheckMenuItem(GetMenu(hWnd), ID_DIPSW_STANDARD, MF_BYCOMMAND | (!(bsmode & 0x40) ? MF_CHECKED : MF_UNCHECKED));
   CheckMenuItem(GetMenu(hWnd), ID_DIPSW_V1, MF_BYCOMMAND | (!(bsmode & 0x80) ? MF_CHECKED : MF_UNCHECKED));
   CheckMenuItem(GetMenu(hWnd), ID_DIPSW_4MHZ, MF_BYCOMMAND | (is8mhz ? MF_CHECKED : MF_UNCHECKED));

   return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_MOUSEMOVE:
        if (mousemvenabled == false) { return 0; }
    case WM_LBUTTONDOWN:
        mousemvenabled = true;
        RECT rw4rend;
        if (bool4showwin) {
            GetClientRect(hwnd4mw, &rw4rend);
        }
        else {
            GetClientRect(HWNDfullscr, &rw4rend);
        }
        lp_ggxy[0] = (((((lParam >> (16 * 0)) & 0xFFFF) * (pc8001widthflag ? 80 : 40)) / rw4rend.right) + (vbi ? 0x6 : 0x36));
        lp_ggxy[1] = ((((lParam >> (16 * 1)) & 0xFFFF) * (grpheight25 ? 25 : 20)) / rw4rend.bottom);
        litepeninp = ((((vbi?0:1) << 7) | (lp_ggxy[0] & 127)) << 0) | ((lp_ggxy[1] & 63) << 8);
        upd3301stat |= 1;
        //rw4rend.right, rw4rend.bottom
        break;
    case WM_LBUTTONUP:
        mousemvenabled = false;
        break;
    case WM_KEYDOWN:
        if (wParam == 120) {
            ttyconnected = false;
            cmtreseted = true;
            cmtdatard = true;
            serialstat = true;
            serialstatw = true;
            //初期化(これをしないとごみが入る)
            ZeroMemory(FileName, MAX_PATH * 2);
            //「ファイルを開く」ダイアログを表示
            uPD8251config[1] = 0x7;
            if (OpenDiaog(hWnd, "CMT File(*.cmt)\0*.cmt\0All Files(*.*)\0*.*\0\0",
                FileName, OFN_PATHMUSTEXIST | /*OFN_FILEMUSTEXIST | */OFN_HIDEREADONLY)) {
                //MessageBoxA(0, FileName,"A", 0);
            }
            if (cmtfileloc != 0) { CloseHandle(cmtfileloc); }
            return 0; }
        if (wParam == 123) {
            if (bool4showwin) {
                ShowWindow(hwnd4mw, 0);
                ShowWindow(HWNDfullscr, 5);
            }else {
                ShowWindow(hwnd4mw, 5);
                ShowWindow(HWNDfullscr, 0);
            }
            UpdateWindow(hwnd4mw);
            UpdateWindow(HWNDfullscr);
            bool4showwin = bool4showwin ? false : true;
            return 0;
        }
        else if (wParam == 122) {
            ResetEmu();
            return 0;
        }
		if (wParam == 37 || wParam == 40) {
			pc8001kb1p = pc8001kmp[16];
			if (pc8001kb1p != 255) { pc8001keybool[(pc8001kb1p >> 4) & 0xF] |= 1 << (pc8001kb1p & 0xF); }
		}
        pc8001kb1p = pc8001kmp[wParam];
        if (pc8001kb1p!=255){ pc8001keybool[(pc8001kb1p >> 4) & 0xF] |= 1 << (pc8001kb1p & 0xF); }
        break;
    case WM_KEYUP:
		if (wParam == 37 || wParam == 40) {
			pc8001kb1p = pc8001kmp[16];
			if (pc8001kb1p != 255) { pc8001keybool[(pc8001kb1p >> 4) & 0xF] &= ~(1 << (pc8001kb1p & 0xF)); }
		}
        pc8001kb1p = pc8001kmp[wParam];
        if (pc8001kb1p != 255) { pc8001keybool[(pc8001kb1p >> 4) & 0xF] &= ~( 1 << (pc8001kb1p & 0xF) ); }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                for (int cnt = 0; cnt < 4; cnt++) {
                    fddriveclose(cnt);
                }
                DestroyWindow(hWnd);
                break;
            case IDM_HFT:
                isharftoneenabled = (isharftoneenabled ? false : true);
                break;
            case IDM_GREENDSP:
                greenmonitor = (greenmonitor ? false : true);
                break;
            case IDM_PCG:
                for (int cnt = 0; cnt < 0x400; cnt++) { pcgcharram[cnt] = rand(); }
                isenabledpcg = (isenabledpcg ? false : true);
                break;
            case ID_DIPSW_N80:
                crtc2 ^= 1;
                CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | ((crtc2 & 1) ? MF_CHECKED : MF_UNCHECKED));
                break;
            case ID_DIPSW_STANDARD:
                bsmode ^= 0x40;
                CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | (!(bsmode & 0x40) ? MF_CHECKED : MF_UNCHECKED));
                break;
            case ID_DIPSW_V1:
                bsmode ^= 0x80;
                CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | (!(bsmode & 0x80) ? MF_CHECKED : MF_UNCHECKED));
                break;
            case ID_DIPSW_4MHZ:
                is8mhz = is8mhz ? false : true;
                CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | (is8mhz ? MF_CHECKED : MF_UNCHECKED));
                break;
            case ID_32778:
                ttyconnected = false;
                cmtreseted = true;
                cmtdatard = true;
                serialstat = true;
                serialstatw = true;
                //初期化(これをしないとごみが入る)
                ZeroMemory(FileName, MAX_PATH * 2);
                //「ファイルを開く」ダイアログを表示
                uPD8251config[1] = 0x7;
                if (OpenDiaog(hWnd, "CMT File(*.cmt)\0*.cmt\0All Files(*.*)\0*.*\0\0",
                    FileName, OFN_PATHMUSTEXIST | /*OFN_FILEMUSTEXIST | */OFN_HIDEREADONLY)) {
                    //MessageBoxA(0, FileName,"A", 0);
                }
                if (cmtfileloc != 0) { CloseHandle(cmtfileloc); }
                break;
            case ID_32779:
                cmtreseted = true;
                cmtdatard = true;
                if (cmtfileloc != 0) { CloseHandle(cmtfileloc); }
                cmtfileloc = CreateFileA("\\\\.\\COM1", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                memcpy(FileName, "\\\\.\\COM1", 9);
                serialstat = false;
                serialstatw = true;
                ttyconnected = true;
                rxdataready = false;
                break;
            case ID_32780:
                ResetEmu();
                break;
            case ID_32781:
                //初期化(これをしないとごみが入る)
                ZeroMemory(FileNameoffd, MAX_PATH * 2);
                //「ファイルを開く」ダイアログを表示
                if (OpenDiaog(hWnd, "D88 File(*.d88)\0*.d88\0All Files(*.*)\0*.*\0\0",
                    FileNameoffd, OFN_PATHMUSTEXIST | /*OFN_FILEMUSTEXIST | */OFN_HIDEREADONLY)) {
                    //MessageBoxA(0, FileName,"A", 0);
                }
                if (strlen(FileNameoffd) == 0) {
                    fddriveclose(0);
                }
                else {
                    fddriveload(0, FileNameoffd);
                }
                break;
            case ID_32782:
                //初期化(これをしないとごみが入る)
                ZeroMemory(FileNameoffd, MAX_PATH * 2);
                //「ファイルを開く」ダイアログを表示
                if (OpenDiaog(hWnd, "D88 File(*.d88)\0*.d88\0All Files(*.*)\0*.*\0\0",
                    FileNameoffd, OFN_PATHMUSTEXIST | /*OFN_FILEMUSTEXIST | */OFN_HIDEREADONLY)) {
                    //MessageBoxA(0, FileName,"A", 0);
                }
                if (strlen(FileNameoffd) == 0) {
                    fddriveclose(1);
                }
                else {
                    fddriveload(1, FileNameoffd);
                }
                break;
            case ID_32783:
                //初期化(これをしないとごみが入る)
                ZeroMemory(FileNameoffd, MAX_PATH * 2);
                //「ファイルを開く」ダイアログを表示
                if (OpenDiaog(hWnd, "D88 File(*.d88)\0*.d88\0All Files(*.*)\0*.*\0\0",
                    FileNameoffd, OFN_PATHMUSTEXIST | /*OFN_FILEMUSTEXIST | */OFN_HIDEREADONLY)) {
                    //MessageBoxA(0, FileName,"A", 0);
                }
                if (strlen(FileNameoffd) == 0) {
                    fddriveclose(2);
                }
                else {
                    fddriveload(2, FileNameoffd);
                }
                break;
            case ID_32784:
                //初期化(これをしないとごみが入る)
                ZeroMemory(FileNameoffd, MAX_PATH * 2);
                //「ファイルを開く」ダイアログを表示
                if (OpenDiaog(hWnd, "D88 File(*.d88)\0*.d88\0All Files(*.*)\0*.*\0\0",
                    FileNameoffd, OFN_PATHMUSTEXIST | /*OFN_FILEMUSTEXIST | */OFN_HIDEREADONLY)) {
                    //MessageBoxA(0, FileName,"A", 0);
                }
                if (strlen(FileNameoffd) == 0) {
                    fddriveclose(3);
                }
                else {
                    fddriveload(3, FileNameoffd);
                }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: HDC を使用する描画コードをここに追加してください...
            //Rectangle(hdc, 0, 0, 640, 480);  // 描画
            RECT rw4rend;
            GetClientRect(hWnd, &rw4rend);
            if ((rw4rend.right != 0) && (rw4rend.bottom != 0)) { SetStretchBltMode(hdc, (isharftoneenabled ? STRETCH_HALFTONE : COLORONCOLOR)); StretchBlt(hdc, 0, 0, rw4rend.right, rw4rend.bottom, hCDC, 0, 0, 640, 480, SRCCOPY); if (isharftoneenabled == true) { SetBrushOrgEx(hdc, 0, 0, NULL); } }
            EndPaint(hWnd, &ps);
    }
        break;
    case WM_DESTROY:
        waveOutUnprepareHeader(hWaveOut, &whdr, sizeof(WAVEHDR));
        waveOutClose(hWaveOut);
        free(lpWave);
        for (int cnt = 0; cnt < 4; cnt++) {
            fddriveclose(cnt);
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


EXTERN_C BOOL WINAPI _imp__IsDebuggerPresent(void)
{
  return FALSE;
}