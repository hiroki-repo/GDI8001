// GDI8001.cpp : アプリケーションのエントリ ポイントを定義します。
//

#pragma warning(disable : 4996)

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include "framework.h"
#include "GDI8001.h"

#include <stdlib.h>
#include <time.h>

#include <shobjidl_core.h>

bool isbeepplayed = false;

#define SRATE    44100    //標本化周波数(1秒間のサンプル数)
#define F        2400     //周波数(1秒間の波形数)

WAVEFORMATEX wfe;
static HWAVEOUT hWaveOut;
static WAVEHDR whdr;
static LPBYTE lpWave;
int i, len;


void beepinit() {

    wfe.wFormatTag = WAVE_FORMAT_PCM;
    wfe.nChannels = 1;    //モノラル
    wfe.wBitsPerSample = 8;    //量子化ビット数
    wfe.nBlockAlign = wfe.nChannels * wfe.wBitsPerSample / 8;
    wfe.nSamplesPerSec = SRATE;    //標本化周波数
    wfe.nAvgBytesPerSec = wfe.nSamplesPerSec * wfe.nBlockAlign;

    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfe, 0, 0, CALLBACK_NULL);

    lpWave = (LPBYTE)calloc(wfe.nAvgBytesPerSec, 2);    //2秒分

    len = SRATE / F;    //波長
    for (i = 0; i < SRATE * 2; i++) {  //波形データ作成
        if (i % len < len / 2)    lpWave[i] = 128 + 64;
        else                 lpWave[i] = 128 - 64;
    }

    whdr.lpData = (LPSTR)lpWave;
    whdr.dwBufferLength = wfe.nAvgBytesPerSec * 2;
    whdr.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
    whdr.dwLoops = -1;

    waveOutPrepareHeader(hWaveOut, &whdr, sizeof(WAVEHDR));
}

void beep2400play() {
    if (isbeepplayed == false) {
        waveOutWrite(hWaveOut, &whdr, sizeof(WAVEHDR));
        isbeepplayed = true;
    }
}

void beep2400stop(){
    if (isbeepplayed == true) {
        waveOutReset(hWaveOut);
        isbeepplayed = false;
    }
}

FILE* cmtfile;
char FileName[MAX_PATH * 2];

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

#ifdef _ARM_
#pragma comment(lib,"bz80dll_arm.lib")
#else
#pragma comment(lib,"bz80dll.lib")
#endif
extern "C" __declspec(dllimport) void setz80memaccess(int (*tmp)(int, int, int));
extern "C" __declspec(dllimport) void Z80Init(void);
extern "C" __declspec(dllimport) void Z80Reset(void);
extern "C" __declspec(dllimport) int  Z80Run(void);
extern "C" __declspec(dllimport) void Z80DoIRQ(uint8 vector);
extern "C" __declspec(dllimport) void Z80DoNMI(void);
extern "C" __declspec(dllimport) int getz80regs();
extern "C" __declspec(dllimport) int getextz80regs(int tmp);


time_t timer;
struct tm local_time;

bool biosromenabled=false;

uint8 bios[0x6000];
uint8 memory[0x10000];
uint8 fontrom[0x800];

uint8 pc8001keybool[0x10];

HANDLE Z80Threadid = 0;
HANDLE BSThreadid = 0;
HANDLE BGThreadid = 0;
HANDLE RTIThreadid = 0;
int clockcount = 0;
bool videoenabled = false;
bool beepenabled = false;
uint8 uPD8251config[4];
uint8 upd8251configate = 0;
bool overrunerror = false;
bool rxdataready = false;
uint8 crtc2 = 0;

uint8 bgcolor = 0;
bool colorgraphicmode = false;
bool graphicdraw = false;
bool grpmode = false;
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

HDC hdc = 0;
HBITMAP hOldCBitmap;

int pc8001kb1p;

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
HPEN hPen;
HBRUSH hBackGround[256];
RECT rs;

uint8 ioporte6h = 0;

uint8 pc8001kmp[256] = { 255,255,255,255,255,255,255,255,131,255,255,255,255,23,255,255,134,135,132,255,255,255,255,255,255,255,255,151,255,133,255,255,150,255,255,144,128,130,129,130,129,255,255,255,255,255,22,255,96,97,98,99,100,101,102,103,112,113,255,255,255,255,255,255,255,33,34,35,36,37,38,39,48,49,50,51,52,53,54,55,64,65,66,67,68,69,70,71,80,81,82,255,255,132,255,255,0,1,2,3,4,5,6,7,16,17,18,19,20,21,255,255,145,146,147,148,149,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,114,115,116,87,117,118,32,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,83,84,85,86,255,255,255,119,255,255,133,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };

void rtcstrobe() { timezforch1.tm_sec = rtctimeforminus[0] % 60; timezforch1.tm_min = rtctimeforminus[1] % 60; timezforch1.tm_hour = rtctimeforminus[2] % 24; timezforch1.tm_mday = rtctimeforminus[3] % 32; timezforch1.tm_mon = (rtctimeforminus[4] - 1) % 12; timezforch1.tm_year = 71; timerforch123 = time(0) - mktime(timexforch1) + mktime(&timezforch1); timey = localtime(&timerforch123); rtctimeforminusck2[0] = timey->tm_sec; rtctimeforminusck2[1] = timey->tm_min; rtctimeforminusck2[2] = timey->tm_hour; rtctimeforminusck2[3] = timey->tm_mday; rtctimeforminusck2[4] = timey->tm_mon + 1; switch (pch & 0xF) { case 1:rtcpos = 0; rtctime[4] = rtctimeforminusck2[4] << 4; rtctime[3] = ((rtctimeforminusck2[3] / 10) << 4) + rtctimeforminusck2[3] % 10; rtctime[2] = ((rtctimeforminusck2[2] / 10) << 4) + rtctimeforminusck2[2] % 10; rtctime[1] = ((rtctimeforminusck2[1] / 10) << 4) + rtctimeforminusck2[1] % 10; rtctime[0] = ((rtctimeforminusck2[0] / 10) << 4) + rtctimeforminusck2[0] % 10; rtcdata = rtctime[0] & 0x01; break; case 2:for (int cnt = 0; cnt < 5; cnt++) { if (cnt != 4) { rtctime[cnt] = rtctimetmp[cnt]; } else { rtctime[cnt] = (rtctimetmp[cnt]-1); } }for (int cnt = 0; cnt < 5; cnt++) { if (cnt != 4) { rtctimeforminus[cnt] = (((rtctime[cnt] >> 4) * 10) + (rtctime[cnt] & 15)); } else { rtctimeforminus[cnt] = (rtctime[cnt] >> 4); } } break; } }
void rtcshift() {if(rtcpos<40){rtctimetmp[rtcpos>>3]|= (pch >> 3 & 1) << (rtcpos & 7);rtcpos+=1;rtcdata=(rtctime[rtcpos >> 3] >> (rtcpos & 7) & 1)? true :false; }}
void prtstrobe() {if (prtenable && prtenable0 != 0 && pch != 13){}prtenable0 = prtenable;}

bool cmtreseted = false;

int z80memaccess(int prm_0, int prm_1, int prm_2) {
    switch (prm_2){
    case 0:
        memory[prm_0 & 0xFFFF] = prm_1 & 0xFF;
        return 0;
        break;
    case 1:
        if ((prm_0 & 0xFFFF) < 0x6000 && biosromenabled == false) { return bios[prm_0 & 0xFFFF]; }
        else { return memory[prm_0 & 0xFFFF]; }
        break;
    case 2:
        switch (prm_0 & 0xFF) {
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
            cmtfile = fopen(FileName, "ab"); if (cmtfile != 0) { fputc(prm_1, cmtfile); fclose(cmtfile); }
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
            //if (prm_1 == 0x40) { cmtreseted = true; }
            return 0;
            if (upd8251configate == 1) { uPD8251config[3] = (prm_1 & 0xFF);/*=(upd8251config&0x00FFFFFF)|((_z80_data&0xFF)<<24)*/ if (uPD8251config[3] & 64) { uPD8251config[0] = 0; uPD8251config[1] = 0; uPD8251config[2] = 0; uPD8251config[3] = 0; } if (uPD8251config[3] & 16) { overrunerror = false; rxdataready = false; } }
            else { uPD8251config[0] = (prm_1 & 0xFF);/*upd8251config=(upd8251config&0xFFFFFF00)|((_z80_data&0xFF)<<0)*/ }
            upd8251configate++; if (upd8251configate >= 2) { upd8251configate = 0; }
            break;
        case 0x30:
        case 0x32:
        case 0x34:
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
        case 0x33:
        case 0x35:
        case 0x37:
        case 0x39:
        case 0x3B:
        case 0x3D:
        case 0x3F:
            crtc3 = prm_1;
                bgcolor = (prm_1 >> 5) & 0x7;
            colorgraphicmode = ((prm_1 >> 4) & 0x1) ? true : false;
            graphicdraw = ((prm_1 >> 3) & 0x1 )? true:false;
            grpmode = ((prm_1 >> 2) & 0x1) ? true : false;
            biosromenabled = ((prm_1 >> 1) & 0x1) ? true : false;
            romtype = ((prm_1 >> 0) & 0x1) ? true : false;
            break;
        case 0x40:
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
            uopout = (prm_1 >> 6) & 0x03;
            beepenabled = ((prm_1 >> 5) & 0x01)?true:false;
                crtcldsclkenable = (prm_1 >> 3) & 0x01;
                rtcclkenable = (prm_1 >> 2) & 0x01;
                rtcstbenable = (prm_1 >> 1) & 0x01;
                if (rtcstbenable == true) { rtcstrobe(); }
                else { if (rtcclkenable == true) { rtcshift(); } }
                prtenable = (prm_1 >> 0) & 0x01; prtstrobe();
            break;
        case 0x50:
        case 0x52:
        case 0x54:
        case 0x56:
        case 0x58:
        case 0x5A:
        case 0x5C:
        case 0x5E:
            uPD3301prm = prm_1;
                if (seq){
                    switch (seq--) {
                    case 1:
                        colorgraphicmode = ((prm_1 >> 4) & 0x1) ? true : false;
                        crtcactive = 0;
                        break;
                    case 3:
                        grpheight25 = ((prm_1 & 0x1f) < 9) ? true : false;
                        blinkingtime = ((prm_1 >> 6) & 3);
                        break;
                    case 6:
                        cursx = cursxtmp;
                        cursy = prm_1;
                        seq = 0;
                        break;
                    case 7:
                        cursxtmp = prm_1;
                        break;
                    }
                }
            break;
        case 0x51:
        case 0x53:
        case 0x55:
        case 0x57:
        case 0x59:
        case 0x5B:
        case 0x5D:
        case 0x5F:
            upd3301cmd = prm_1;
            switch (prm_1 & 0xF0){
            case 0x00:
                seq = 5;
                crtcactive = 0;
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
        }
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
		case 0xE6:
			ioporte6h = prm_1;
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
            if (cmtreseted == true) { cmtreseted = false; cmtseek = 0; return 0xff; }
            else { cmtfile = fopen(FileName, "rb"); if (cmtfile != 0) { struct _stat buf; int result = _stat(FileName, &buf); if (buf.st_size <= cmtseek) { cmtseek = 0; fclose(cmtfile); return 0xFF; } fseek(cmtfile, cmtseek++, SEEK_SET); ret = fgetc(cmtfile); fclose(cmtfile); return ret; } else { return 0xff; } }
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
            //return 0x07;
            return uPD8251config[1] | (( rxdataready ? 1 : 0 ) << 1) | ( ( ( uPD8251config[3] & 1 ) ? 1 : 0) << 0 );
            break;
        case 0x30:
        case 0x32:
        case 0x34:
        case 0x36:
        case 0x38:
        case 0x3A:
        case 0x3C:
        case 0x3E:
            return crtc2;
            break;
        case 0x31:
        case 0x33:
        case 0x35:
        case 0x37:
        case 0x39:
        case 0x3B:
        case 0x3D:
        case 0x3F:
            return ((bgcolor & 0x7) << 5) | ((colorgraphicmode ? 1 : 0) << 4) | ((graphicdraw ? 1 : 0) << 3) | ((grpmode ? 1 : 0) << 2) | ((biosromenabled ? 1 : 0) << 1) | ((romtype ? 1 : 0) << 0);
            break;
        case 0x40:
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
            return ((uipin & 3) << 6) | ((vbi ? 1 : 0) << 5) | ((rtcdata & 1) << 4) | ((fddconnected ? 1 : 0) << 3) | ((cmtdatard ? 1 : 0) << 2) | ((prtready ? 1 : 0) << 0);
            break;
        case 0x50:
        case 0x52:
        case 0x54:
        case 0x56:
        case 0x58:
        case 0x5A:
        case 0x5C:
        case 0x5E:
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
        case 0x53:
        case 0x55:
        case 0x57:
        case 0x59:
        case 0x5B:
        case 0x5D:
        case 0x5F:
            if (upd3301stat & 8) { return upd3301stat & (0xFF - 0x10); }
            else { return upd3301stat; }

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
            ret = dmaas[0] >> (((dmachiocnt >> 4) & 1) * 8); dmachiocnt ^= (1 << 4); return ret;
            break;
        case 0x65:
            ret = dmatc[0] >> (((dmachiocnt >> 5) & 1) * 8); dmachiocnt ^= (1 << 5); return ret;
            break;
        case 0x66:
            ret = dmaas[1] >> (((dmachiocnt >> 6) & 1) * 8); dmachiocnt ^= (1 << 6); return ret;
            break;
        case 0x67:
            ret = dmatc[1] >> (((dmachiocnt >> 7) & 1) * 8); dmachiocnt ^= (1 << 7); return ret;
            break;
        case 0x68:
            return dmamodestat | ((1 << 4) * ((clockcount % 19) > 9));
            break;
		case 0xE6:
			return ioporte6h;
			break;
        }
        return 0xff;
        break;
    }
    return 0;
}

HWND hwnd4mw;

uint8 z80irqid = 0;
uint8 z80irqfn = 0;

void RunZ80Infinity(LPVOID* arg4rz80) { while (true) { clockcount = 0; int clockcountinternal = 0; int z80timerbefore = time(NULL); while (clockcount < (graphicdraw ? 1830000 : 4000000)) { clockcountinternal = 0; while (clockcountinternal < (graphicdraw ? 183000 : 400000)) { if (z80irqid != 0) { if (z80irqid == 1) { Z80DoIRQ(z80irqfn); z80irqfn = 0; } else { Z80DoNMI(); } z80irqid = 0; } clockcountinternal += Z80Run(); vbi = vbi ? false : true; } clockcount += clockcountinternal; Sleep(100); } int z80timerint = time(NULL) - z80timerbefore; /*if (z80timerint < 1000) { Sleep(1000 - z80timerint); }*/ } }

void Z80INT(uint8 prm_0) { z80irqid = 1; z80irqfn = prm_0; }
void Z80NMI() { z80irqid = 2; }

void BeepService(LPVOID* arg4bs) { while (true) { if (beepenabled) { /*Beep(2400, 100);*/ beep2400play(); } else { beep2400stop(); } } }

void RTIService(LPVOID* arg4rtisv) { while (true) { if (ioporte6h & 1) { Z80INT(4); } Sleep(2); } }

int xsiz10times = 0;
int ysiz10times = 0;

uint8 color4draw = 0;

void SetPalette4emu(int prm_0) { color4draw = prm_0; }

void SetPset(int prm_0, int prm_1) {
    xsiz10times = pc8001widthflag ? 10 : 20; ysiz10times = grpheight25 ? 24 : 30;
    rs.left = (((prm_0 + 0) * xsiz10times) / 10);
    rs.top = (((prm_1 + 0) * ysiz10times) / 10);
    rs.right = (((prm_0 + 1) * xsiz10times) / 10);
    rs.bottom = (((prm_1 + 1) * ysiz10times) / 10);
    FillRect(hCDC, &rs, hBackGround[color4draw]);
}
void SetBox(int prm_0, int prm_1, int prm_2, int prm_3) {
    xsiz10times = pc8001widthflag ? 10 : 20; ysiz10times = grpheight25 ? 24 : 30;
    rs.left = (((prm_0 + 0) * xsiz10times) / 10);
    rs.top = (((prm_1 + 0) * ysiz10times) / 10);
    rs.right = (((prm_2 + 0) * xsiz10times) / 10);
    rs.bottom = (((prm_3 + 0) * ysiz10times) / 10);
    FillRect(hCDC, &rs, hBackGround[color4draw]);
}

void SetBGCL(){
    rs.left = 0;
    rs.top = 0;
    rs.right = 640;
    rs.bottom = 480;
    FillRect(hCDC, &rs, hBackGround[color4draw]);
}


void DrawFont(int prm_0, int prm_1, int prm_2) {
    for (int fonty = 0; fonty < 8; fonty++) {
        for (int fontx = 0; fontx < 8; fontx++) {
            if ((fontrom[prm_2 * 8 + fonty] << fontx) & 128) { SetPset(prm_0 + fontx, prm_1 + fonty); }
        }
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

void Drawbackground(LPVOID* arg4dbg) {
    while (true) {
        if ((graphicdraw==true)||(true)) {
            if (crtmodectrl == false) { SetPalette4emu(bgcolor); }else{ SetPalette4emu(8); }
            SetBGCL();
            for (int drawbacky = 0; drawbacky < (grpheight25 ? 25 : 20); drawbacky++){
                for (int drawbackx = 0; drawbackx < (pc8001widthflag ? 80 : 40); drawbackx++){
                uint8 char4show = z80memaccess(dmaas[2] + ((drawbackx * (pc8001widthflag ? 1 : 2)) + (drawbacky * 120)), 0, 1);
				attributetmp = -1; attributeold = -1; fontcolors = 0; grpcolors = 0; attributegcold = false;
				attributeold2 = -1; attributetmp2 = -1; attributeold3 = -1; attributetmp3 = -1; semigraphicenabled = false;
				for (int cnt = 0; cnt < 20; cnt++) {
					uint8 charattributetmp = z80memaccess(dmaas[2] + (((cnt * 2) + 81) + (drawbacky * 120)), 0, 1); if ((z80memaccess(dmaas[2] + (((cnt * 2) + 80) + (drawbacky * 120)), 0, 1) & 0x7F) != (64 | 32)) { if (charattributetmp & 8) { attributetmp = z80memaccess(dmaas[2] + (((cnt * 2) + 80) + (drawbacky * 120)), 0, 1) / (pc8001widthflag ? 1 : 2); if (attributetmp <= drawbackx && attributetmp > attributeold) { attributeold = attributetmp; fontcolors = (charattributetmp >> 5) & 7; grpcolors = (charattributetmp >> 5) & 7; if (charattributetmp & 16) { semigraphicenabled = true; } else { semigraphicenabled = false; } } } else { attributetmp3 = z80memaccess(dmaas[2] + (((cnt * 2) + 80) + (drawbacky * 120)), 0, 1) / (pc8001widthflag ? 1 : 2); if ((attributetmp3 <= drawbackx && attributetmp3 > attributeold3) || (((((charattributetmp & 128) ? true : false) != attributegcold) && (charattributetmp & 128)))) { charattribute = charattributetmp; attributeold3 = attributetmp3; attributegcold = (charattributetmp & 128) ? true : false; } } }
                }
				if (semigraphicenabled == true) { charattribute |= 128; }
				//grpcolors = 9;
                if (charattribute & 4) { if (crtmodectrl == false) { if (charattribute & 128) { SetPalette4emu(grpcolors); } else { SetPalette4emu(fontcolors); } } else { SetPalette4emu(9); } }else{ if (crtmodectrl == false) { SetPalette4emu(bgcolor); } else { SetPalette4emu(8); } }
                if ((cursx != -1 && cursy != -1)&& (cursx == (drawbackx * (pc8001widthflag ? 1 : 2)) && cursy == drawbacky)) { if (blinkai2 == false) {
                    if (charattribute & 4) { if (crtmodectrl == false) { SetPalette4emu(bgcolor); } else { SetPalette4emu(8); } }
                    else { if (crtmodectrl == false) { if (charattribute & 128) { SetPalette4emu(grpcolors); } else { SetPalette4emu(fontcolors); } } else { SetPalette4emu(9); } }
                    SetBox((((cursx / (pc8001widthflag ? 1 : 2)) + 0) * 8), ((cursy + 0) * 8), (((cursx / (pc8001widthflag ? 1 : 2)) + 1) * 8) - 0, ((cursy + 1) * 8) - 0);
                } }
                if (((blinkai == false) || ((charattribute & 2) == 0)) && ((charattribute & 1) == 0)) {
                    SetBox(((drawbackx + 0) * 8), ((drawbacky + 0) * 8), ((drawbackx + 1) * 8) - 0, ((drawbacky + 1) * 8) - 0);
                    if ((cursx == (drawbackx * (pc8001widthflag ? 1 : 2)) && cursy == drawbacky)&& (blinkai2 == false)) {
                        if (charattribute & 4) { if (crtmodectrl == false) { if (charattribute & 128) { SetPalette4emu(grpcolors); } else { SetPalette4emu(fontcolors); } } else { SetPalette4emu(9); } }
                        else { if (crtmodectrl == false) { SetPalette4emu(bgcolor); } else { SetPalette4emu(8); } }
                    }
                    else {
                        if (charattribute & 4) { if (crtmodectrl == false) { SetPalette4emu(bgcolor); } else { SetPalette4emu(8); } }
                        else { if (crtmodectrl == false) { if (charattribute & 128) { SetPalette4emu(grpcolors); } else { SetPalette4emu(fontcolors); } } else { SetPalette4emu(9); } }
                    }
					if ((charattribute & 128) || (attributegcold == true)) { for (int cnt = 0; cnt < 8; cnt++) { if ((char4show >> cnt) & 1) { SetBox(((drawbackx + 0) * 8) + (4 * ((cnt / 4) + 0)) - 0, ((drawbacky + 0) * 8) + (2 * ((cnt % 4) + 0)) - 0, ((drawbackx + 0) * 8) + (4 * ((cnt / 4) + 1)) - 0, ((drawbacky + 0) * 8) + (2 * ((cnt % 4) + 1)) - 0); } } }
					else { DrawFont(((drawbackx + 0) * 8), ((drawbacky + 0) * 8), char4show); }
					//SetPalette4emu(9); SetBox(((drawbackx + 0) * 8), ((drawbacky + 0) * 8), ((drawbackx + 1) * 8) - 0, ((drawbacky + 1) * 8) - 0);
					if (charattribute & 64) { SetBox(((drawbackx + 0) * 8) + 3, ((drawbacky + 0) * 8), ((drawbackx + 0) * 8) + 4, ((drawbacky + 1) * 8) - 1); }
                    if (charattribute & 32) { SetBox(((drawbackx + 0) * 8), ((drawbacky + 1) * 8) - 2, ((drawbackx + 1) * 8), ((drawbacky + 1) * 8) - 1); }
                    if (charattribute & 16) { SetBox(((drawbackx + 0) * 8), ((drawbacky + 0) * 8), ((drawbackx + 1) * 8), ((drawbacky + 0) * 8) + 1); }
                }
                }
            }
        } else { SetPalette4emu(8); SetBGCL(); }
        if (blinkingtime == blinkwaitisti) { blinkai2 = blinkai2 ? false : true; }
        if ((blinkingtime*2) == blinkwaitisti) { blinkai = blinkai ? false : true; blinkwaitisti = 0; }
        blinkwaitisti++;
        RECT rw4rend;
        GetClientRect(hwnd4mw, &rw4rend);

        if ((rw4rend.right != 0) && (rw4rend.bottom != 0)) { StretchBlt(hdc, 0, 0, rw4rend.right, rw4rend.bottom, hCDC, 0, 0, 640, 480, SRCCOPY); }
        Sleep(16);
    }
}

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    beepinit();

    Z80Init();
    setz80memaccess(z80memaccess);

    FILE *biosfile = fopen("n80basic.rom","rb");
    if (biosfile != 0) {
        fread(bios, 0x6000, 1, biosfile);
        fclose(biosfile);
    }

    FILE* fontfile = fopen("font.rom", "rb");
    if (fontfile != 0) {
        fread(fontrom, 0x800, 1, fontfile);
        fclose(fontfile);
    }

    time_t timer = time(NULL);
    timexforch1 = localtime(&timer);

    //rtctimeforminus

    timexforch1->tm_year -= 9;

    //timexforch1[0] = timexforch1charx->tm_sec, timexforch1charx->tm_min, timexforch1charx->tm_hour, timexforch1charx->tm_mday, timexforch1charx->tm_mon, timexforch1charx->tm_year - 9, 0, 0, 0;

    Z80Threadid = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RunZ80Infinity, 0, 0, 0);
    BSThreadid = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)BeepService, 0, 0, 0);
    BGThreadid = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Drawbackground, 0, 0, 0);
	RTIThreadid = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RTIService, 0, 0, 0);

    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GDI8001, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GDI8001));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
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
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GDI8001);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
      CW_USEDEFAULT, 0, 640, 540, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   hwnd4mw = hWnd;

   hdc = GetDC(hWnd);

   hCDC = CreateCompatibleDC(hdc);
   hCBitmap = CreateCompatibleBitmap(hdc, 640, 480);
   hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
   for (int cnt = 0; cnt < 256; cnt++) {
       if (cnt < 8) { hBackGround[cnt] = CreateSolidBrush(RGB(((cnt >> 1) & 1) * 255, ((cnt >> 2) & 1) * 255, ((cnt >> 0) & 1) * 255)); }
       else if (cnt == 8) { hBackGround[cnt] = CreateSolidBrush(RGB(0, 0, 0)); }
       else if (cnt == 9) { hBackGround[cnt] = CreateSolidBrush(RGB(255, 255, 255)); }
       else if ((cnt < 24) && (cnt > 15)) { hBackGround[cnt] = CreateSolidBrush(RGB(((cnt >> 1) & 1) * 230, ((cnt >> 2) & 1) * 230, ((cnt >> 0) & 1) * 230)); }
       else if (cnt == 24) { hBackGround[cnt] = CreateSolidBrush(RGB(0, 0, 0)); }
       else if (cnt == 25) { hBackGround[cnt] = CreateSolidBrush(RGB(230, 230, 230)); }
       else { hBackGround[cnt] = CreateSolidBrush(RGB(0, 0, 0)); }
   }

   hOldCBitmap = (HBITMAP)SelectObject(hCDC, hCBitmap);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

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
    case WM_KEYDOWN:
        if (wParam == 120) { 
            cmtreseted = true;
            cmtdatard = true;
            //初期化(これをしないとごみが入る)
            ZeroMemory(FileName, MAX_PATH * 2);
            //「ファイルを開く」ダイアログを表示
            uPD8251config[1] = 0x7;
            if (OpenDiaog(hWnd, "CMT File(*.cmt)\0*.cmt\0All Files(*.*)\0*.*\0\0",
                FileName, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY)) {
                //MessageBoxA(0, FileName,"A", 0);
            }
            return 0; }
        pc8001kb1p = pc8001kmp[wParam];
        if (pc8001kb1p!=255){ pc8001keybool[(pc8001kb1p >> 4) & 0xF] |= 1 << (pc8001kb1p & 0xF); }
        break;
    case WM_KEYUP:
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
                DestroyWindow(hWnd);
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
            Rectangle(hdc, 0, 0, 640, 480);  // 描画
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        waveOutUnprepareHeader(hWaveOut, &whdr, sizeof(WAVEHDR));
        waveOutClose(hWaveOut);
        free(lpWave);
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
