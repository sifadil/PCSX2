#include "Global.h"
#include <math.h>
#include <Dbt.h>
#include <stdio.h>

// For escape timer, so as not to break GSDX+DX9.
#include <time.h>

#define PADdefs
#include "PS2Etypes.h"
#include "PS2Edefs.h"

#include "Config.h"
#include "InputManager.h"
#include "DeviceEnumerator.h"
#include "WndProcEater.h"
#include "KeyboardQueue.h"
#include "svnrev.h"
#include "resource.h"

#ifdef _DEBUG
#include "crtdbg.h"
#endif

// Used to prevent reading input and cleaning up input devices at the same time.
// Only an issue when not reading input in GS thread and disabling devices due to
// lost focus.
CRITICAL_SECTION readInputCriticalSection;

HINSTANCE hInst;
HWND hWnd;

// Used to toggle mouse listening.
u8 miceEnabled;

// 2 when both pads are initialized, 1 for one pad, etc.
int openCount = 0;

int activeWindow = 0;

int bufSize = 0;
unsigned char outBuf[50];
unsigned char inBuf[50];

#define MODE_DIGITAL 0x41
#define MODE_ANALOG 0x73
#define MODE_DS2_NATIVE 0x79

int IsWindowMaximized (HWND hWnd) {
	RECT rect;
	if (GetWindowRect(hWnd, &rect)) {
		POINT p;
		p.x = rect.left;
		p.y = rect.top;
		MONITORINFO info;
		memset(&info, 0, sizeof(info));
		info.cbSize = sizeof(info);
		HMONITOR hMonitor;
		if ((hMonitor = MonitorFromPoint(p, MONITOR_DEFAULTTOPRIMARY)) &&
			GetMonitorInfo(hMonitor, &info) &&
			memcmp(&info.rcMonitor, &rect, sizeof(rect)) == 0) {
				return 1;
		}
	}
	return 0;
}

void DEBUG_NEW_SET() {
	if (config.debug) {
		HANDLE hFile = CreateFileA("logs\\padLog.txt", GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0);
		if (hFile != INVALID_HANDLE_VALUE) {
			int i;
			char temp[1500];
			char *end = temp;
			for (i=0; i<bufSize; i++) {
				sprintf(end, "%02X ", inBuf[i]);
				end = strchr(end, 0);
			}
			end++[0] = '\n';
			for (i=0; i<bufSize; i++) {
				sprintf(end, "%02X ", outBuf[i]);
				end = strchr(end, 0);
			}
			end++[0] = '\n';
			end++[0] = '\n';
			DWORD junk;
			WriteFile(hFile, temp, end-temp, &junk, 0);
			CloseHandle(hFile);;
		}
	}
	bufSize = 0;
}

inline void DEBUG_IN(unsigned char c) {
	if (bufSize < sizeof(inBuf)-1) inBuf[bufSize] = c;
}
inline void DEBUG_OUT(unsigned char c) {
	if (bufSize < sizeof(outBuf)-1) outBuf[bufSize++] = c;
}

struct Stick {
	int horiz;
	int vert;
};

struct ButtonSum {
	int buttons[12];
	Stick sticks[3];
};


struct PadFreezeData {
	// Digital / Analog / DS2 Native
	u8 mode;

	u8 modeLock;

	// In config mode
	u8 config;

	u8 vibrate[8];
	u8 umask[2];
};

class Pad : public PadFreezeData {
public:
	ButtonSum sum, lockedSum;

	int lockedState;

	// Vibration indices.
	u8 vibrateI[2];

	// Last vibration value.  Only used so as not to call vibration
	// functions when old and new values are both 0.
	u8 vibrateVal[2];

	// Used to keep track of which pads I'm running.
	// Note that initialized pads *can* be disabled.
	// I keep track of state of non-disabled non-initialized
	// pads, but should never be asked for their state.
	u8 initialized;
} pads[2][4];

// Active slots for each port.
int slots[2];
// Which ports we're running on.
int portInitialized[2];

// Force value to be from 0 to 255.
u8 Cap (int i) {
	if (i<0) return 0;
	if (i>255) return 255;
	return (u8) i;
}

// RefreshEnabledDevices() enables everything that can potentially
// be bound to, as well as the "Ignore keyboard" device.
//
// This enables everything that input should be read from while the
// emulator is running.  Takes into account  mouse and focus state
// and which devices have bindings for enabled pads.  Releases
// keyboards if window is not focused.  Releases game devices if
// background monitoring is not checked.
// And releases games if not focused and config.background is not set.
void UpdateEnabledDevices(int updateList = 0) {
	// Enable all devices I might want.  Can ignore the rest.
	RefreshEnabledDevices(updateList);
	// Figure out which pads I'm getting input for.
	int padsEnabled[2][4];
	for (int port = 0; port<2; port++) {
		for (int slot = 0; slot<4; slot++) {
			padsEnabled[port][slot] = pads[port][slot].initialized && config.padConfigs[port][slot].type != DisabledPad;
		}
	}
	for (int i=0; i<dm->numDevices; i++) {
		Device *dev = dm->devices[i];

		if (!dev->enabled) continue;
		if (!dev->attached) {
			dm->DisableDevice(i);
			continue;
		}

		// Disable ignore keyboard if don't have focus or there are no keys to ignore.
		if (dev->api == IGNORE_KEYBOARD) {
			if ((!config.vistaVolume && (config.keyboardApi == NO_API || !dev->pads[0][0].numBindings)) || !activeWindow) {
				dm->DisableDevice(i);
			}
			continue;
		}
		// Keep for PCSX2 keyboard shotcuts, unless unfocused.
		if (dev->type == KEYBOARD) {
			if (!activeWindow) dm->DisableDevice(i);
		}
		// Keep for cursor hiding consistency, unless unfocused.
		// miceEnabled tracks state of mouse enable/disable button, not if mouse API is set to disabled.
		else if (dev->type == MOUSE) {
			if (!miceEnabled || !activeWindow) dm->DisableDevice(i);
		}
		else if (!activeWindow && !config.background) dm->DisableDevice(i);
		else {
			int numActiveBindings = 0;
			for (int port=0; port<2; port++) {
				for (int slot=0; slot<4; slot++) {
					if (padsEnabled[port][slot]) {
						numActiveBindings += dev->pads[port][slot].numBindings + dev->pads[port][slot].numFFBindings;
					}
				}
			}
			if (!numActiveBindings)
				dm->DisableDevice(i);
		}
	}
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, void* lpvReserved) {
	hInst = hInstance;
	if (fdwReason == DLL_PROCESS_ATTACH) {
		InitializeCriticalSection(&readInputCriticalSection);
		DisableThreadLibraryCalls(hInstance);
	}
	else if (fdwReason == DLL_PROCESS_DETACH) {
		DeleteCriticalSection(&readInputCriticalSection);
		while (openCount)
			PADclose();
		PADshutdown();
	}
	return 1;
}

void AddForce(ButtonSum *sum, u8 cmd, int delta = 255) {
	if (!delta) return;
	if (cmd<0x14) {
		sum->buttons[cmd-0x10] += delta;
	}
	// D-pad.  Command numbering is based on ordering of digital values.
	else if (cmd < 0x18) {
		if (cmd == 0x14) {
			sum->sticks[0].vert -= delta;
		}
		else if (cmd == 0x15) {
			sum->sticks[0].horiz += delta;
		}
		else if (cmd == 0x16) {
			sum->sticks[0].vert += delta;
		}
		else if (cmd == 0x17) {
			sum->sticks[0].horiz -= delta;
		}
	}
	else if (cmd < 0x20) {
		sum->buttons[cmd-0x10-4] += delta;
	}
	// Left stick.
	else if (cmd < 0x24) {
		if (cmd == 32) {
			sum->sticks[2].vert -= delta;
		}
		else if (cmd == 33) {
			sum->sticks[2].horiz += delta;
		}
		else if (cmd == 34) {
			sum->sticks[2].vert += delta;
		}
		else if (cmd == 35) {
			sum->sticks[2].horiz -= delta;
		}
	}
	// Right stick.
	else if (cmd < 0x28) {
		if (cmd == 36) {
			sum->sticks[1].vert -= delta;
		}
		else if (cmd == 37) {
			sum->sticks[1].horiz += delta;
		}
		else if (cmd == 38) {
			sum->sticks[1].vert += delta;
		}
		else if (cmd == 39) {
			sum->sticks[1].horiz -= delta;
		}
	}
}

void ProcessButtonBinding(Binding *b, ButtonSum *sum, int value) {
	int sensitivity = b->sensitivity;
	if (sensitivity < 0) {
		sensitivity = -sensitivity;
		value = (1<<16)-value;
	}
	if (value > 0) {
		AddForce(sum, b->command, (int)((((sensitivity*(255*(__int64)value)) + BASE_SENSITIVITY/2)/BASE_SENSITIVITY + FULLY_DOWN/2)/FULLY_DOWN));
	}
}

// Restricts d-pad/analog stick values to be from -255 to 255 and button values to be from 0 to 255.
// With D-pad in DS2 native mode, the negative and positive ranges are both independently from 0 to 255,
// which is why I use 9 bits of all sticks.  For left and right sticks, I have to remove a bit before sending.
void CapSum(ButtonSum *sum) {
	int i;
	for (i=0; i<3; i++) {
		int div = max(abs(sum->sticks[i].horiz), abs(sum->sticks[i].vert));
		if (div > 255) {
			sum->sticks[i].horiz = sum->sticks[i].horiz * 255 / div;
			sum->sticks[i].vert = sum->sticks[i].vert * 255 / div;
		}
	}
	for (i=0; i<12; i++) {
		sum->buttons[i] = Cap(sum->buttons[i]);
	}
}

// Counters for when to next update pad state.
// Read all devices at once, so don't need to read them again
// for pad 2 immediately after pad 1.  3rd counter is for
// when neither pad is being read, so still respond to
// key press info requests.
int summed[3] = {0, 0, 0};

#define LOCK_DIRECTION 2
#define LOCK_BUTTONS 4
#define LOCK_BOTH 1


void Update(int pad) {
	if ((unsigned int)pad > 2) return;
	if (summed[pad] > 0) {
		summed[pad]--;
		return;
	}
	int i;
	ButtonSum s[2][4];
	u8 lockStateChanged[2][4];
	memset(lockStateChanged, 0, sizeof(lockStateChanged));

	for (i=0; i<8; i++) {
		s[i&1][i>>1] = pads[i&1][i>>1].lockedSum;
	}
	InitInfo info = {
		0, hWnd, hWnd, 0
	};

	if (!config.GSThreadUpdates) {
		EnterCriticalSection(&readInputCriticalSection);
	}
	dm->Update(&info);
	static int turbo = 0;
	turbo++;
	for (i=0; i<dm->numDevices; i++) {
		Device *dev = dm->devices[i];
		// Skip both disabled devices and inactive enabled devices.
		// Shouldn't be any of the latter, in general, but just in case...
		if (!dev->virtualControlState) continue;
		for (int port=0; port<2; port++) {
			for (int slot=0; slot<4; slot++) {
				if (config.padConfigs[port][slot].type == DisabledPad || !pads[port][slot].initialized) continue;
				for (int j=0; j<dev->pads[port][slot].numBindings; j++) {
					Binding *b = dev->pads[port][slot].bindings+j;
					int cmd = b->command;
					int state = dev->virtualControlState[b->controlIndex];
					if (!(turbo & b->turbo)) {
						if (cmd > 0x0F && cmd != 0x28) {
							ProcessButtonBinding(b, &s[port][slot], state);
						}
						else if ((state>>15) && !(dev->oldVirtualControlState[b->controlIndex]>>15)) {
							if (cmd == 0x0F) {
								miceEnabled = !miceEnabled;
								UpdateEnabledDevices();
							}
							else if (cmd == 0x0C) {
								lockStateChanged[port][slot] |= LOCK_BUTTONS;
							}
							else if (cmd == 0x0E) {
								lockStateChanged[port][slot] |= LOCK_DIRECTION;
							}
							else if (cmd == 0x0D) {
								lockStateChanged[port][slot] |= LOCK_BOTH;
							}
							else if (cmd == 0x28) {
								if (!pads[port][slot].modeLock) {
									if (pads[port][slot].mode != MODE_DIGITAL)
										pads[port][slot].mode = MODE_DIGITAL;
									else
										pads[port][slot].mode = MODE_ANALOG;
								}
							}
						}
					}
				}
			}
		}
	}
	dm->PostRead();

	if (!config.GSThreadUpdates) {
		LeaveCriticalSection(&readInputCriticalSection);
	}

	for (int port=0; port<2; port++) {
		for (int slot=0; slot<4; slot++) {
			if (config.padConfigs[port][slot].type == DisabledPad || !pads[port][slot].initialized) continue;
			if (config.padConfigs[port][slot].type == GuitarPad) {
				if (!config.GH2) {
					s[port][slot].sticks[1].vert = -s[port][slot].sticks[1].vert;
				}
				// GH2 hack.
				else if (config.GH2) {
					const unsigned int oldIdList[5] = {ID_R2, ID_CIRCLE, ID_TRIANGLE, ID_CROSS, ID_SQUARE};
					const unsigned int idList[5] = {ID_L2, ID_L1, ID_R1, ID_R2, ID_CROSS};
					int values[5];
					int i;
					for (i=0; i<5; i++) {
						int id = oldIdList[i] - 0x1104;
						values[i] = s[port][slot].buttons[id];
						s[port][slot].buttons[id] = 0;
					}
					s[port][slot].buttons[ID_TRIANGLE-0x1104] = values[1];
					for (i=0; i<5; i++) {
						int id = idList[i] - 0x1104;
						s[port][slot].buttons[id] = values[i];
					}
					if (abs(s[port][slot].sticks[0].vert) <= 48) {
						for (int i=0; i<5; i++) {
							unsigned int id = idList[i] - 0x1104;
							if (pads[port][slot].sum.buttons[id] < s[port][slot].buttons[id]) {
								s[port][slot].buttons[id] = pads[port][slot].sum.buttons[id];
							}
						}
					}
					else if (abs(pads[port][slot].sum.sticks[0].vert) <= 48) {
						for (int i=0; i<5; i++) {
							unsigned int id = idList[i] - 0x1104;
							if (pads[port][slot].sum.buttons[id]) {
								s[port][slot].buttons[id] = 0;
							}
						}
					}
				}
			}

			if (pads[port][slot].mode == 0x41) {
				s[port][slot].sticks[0].horiz +=
					s[port][slot].sticks[1].horiz +
					s[port][slot].sticks[2].horiz;
				s[port][slot].sticks[0].vert +=
					s[port][slot].sticks[1].vert +
					s[port][slot].sticks[2].vert;
			}

			CapSum(&s[port][slot]);
			if (lockStateChanged[port][slot]) {
				if (lockStateChanged[port][slot] & LOCK_BOTH) {
					if (pads[port][slot].lockedState != (LOCK_DIRECTION | LOCK_BUTTONS)) {
						// Enable the one that's not enabled.
						lockStateChanged[port][slot] ^= pads[port][slot].lockedState^(LOCK_DIRECTION | LOCK_BUTTONS);
					}
					else {
						// Disable both
						lockStateChanged[port][slot] ^= LOCK_DIRECTION | LOCK_BUTTONS;
					}
				}
				if (lockStateChanged[port][slot] & LOCK_DIRECTION) {
					if (pads[port][slot].lockedState & LOCK_DIRECTION) {
						memset(pads[port][slot].lockedSum.sticks, 0, sizeof(pads[port][slot].lockedSum.sticks));
					}
					else {
						memcpy(pads[port][slot].lockedSum.sticks, s[port][slot].sticks, sizeof(pads[port][slot].lockedSum.sticks));
					}
					pads[port][slot].lockedState ^= LOCK_DIRECTION;
				}
				if (lockStateChanged[port][slot] & LOCK_BUTTONS) {
					if (pads[port][slot].lockedState & LOCK_BUTTONS) {
						memset(pads[port][slot].lockedSum.buttons, 0, sizeof(pads[port][slot].lockedSum.buttons));
					}
					else {
						memcpy(pads[port][slot].lockedSum.buttons, s[port][slot].buttons, sizeof(pads[port][slot].lockedSum.buttons));
					}
					pads[port][slot].lockedState ^= LOCK_BUTTONS;
				}
				for (i=0; i<sizeof(pads[port][slot].lockedSum)/4; i++) {
					if (((int*)&pads[port][slot].lockedSum)[i]) break;
				}
				if (i==sizeof(pads[port][slot].lockedSum)/4) {
					pads[port][slot].lockedState = 0;
				}
			}
		}
	}
	for (i=0; i<8; i++) {
		pads[i&1][i>>1].sum = s[i&1][i>>1];
	}
	summed[0] = 1;
	summed[1] = 1;
	summed[2] = 2;
	summed[pad]--;
}

void CALLBACK PADupdate(int pad) {
	if (config.GSThreadUpdates) Update(pad);
}

inline void SetVibrate(int port, int slot, int motor, u8 val) {
	if (val || pads[port][slot].vibrateVal[motor]) {
		dm->SetEffect(port,slot, motor, val);
		pads[port][slot].vibrateVal[motor] = val;
	}
}

u32 CALLBACK PS2EgetLibType(void) {
	ps2e = 1;
	return PS2E_LT_PAD;
}

#define VERSION ((0<<8) | 9 | (11<<24))

u32 CALLBACK PS2EgetLibVersion2(u32 type) {
	ps2e = 1;
	if (type == PS2E_LT_PAD)
		return (PS2E_PAD_VERSION<<16) | VERSION;
	return 0;
}

// Used in about and config screens.
void GetNameAndVersionString(wchar_t *out) {
#ifdef _DEBUG
	wsprintfW(out, L"LilyPad Debug %i.%i.%i (r%i)", (VERSION>>8)&0xFF, VERSION&0xFF, (VERSION>>24)&0xFF, SVN_REV);
#elif (_MSC_VER != 1400)
	wsprintfW(out, L"LilyPad svn %i.%i.%i (r%i)", (VERSION>>8)&0xFF, VERSION&0xFF, (VERSION>>24)&0xFF, SVN_REV);
#else
	wsprintfW(out, L"LilyPad %i.%i.%i", (VERSION>>8)&0xFF, VERSION&0xFF, (VERSION>>24)&0xFF, SVN_REV);
#endif
}

char* CALLBACK PSEgetLibName() {
#ifdef _DEBUG
	static char version[50];
	sprintf(version, "LilyPad Debug (r%i)", SVN_REV);
	return version;
#else
	#if (_MSC_VER != 1400)
		static char version[50];
		sprintf(version, "LilyPad svn (r%i)", SVN_REV);
		return version;
	#endif
	return "LilyPad";
#endif
}

char* CALLBACK PS2EgetLibName(void) {
	ps2e = 1;
	return PSEgetLibName();
}

//void CALLBACK PADgsDriverInfo(GSdriverInfo *info) {
//	info=info;
//}

void CALLBACK PADshutdown() {
	for (int i=0; i<8; i++)
		pads[i&1][i>>1].initialized = 0;
	UnloadConfigs();
}

inline void StopVibrate() {
	for (int i=0; i<8; i++) {
		SetVibrate(i&1, i>>1, 0, 0);
		SetVibrate(i&1, i>>1, 1, 0);
	}
}

inline void ResetVibrate(int port, int slot) {
	SetVibrate(port, slot, 0, 0);
	SetVibrate(port, slot, 1, 0);
	((int*)(pads[port][slot].vibrate))[0] = 0xFFFFFF5A;
	((int*)(pads[port][slot].vibrate))[1] = 0xFFFFFFFF;
}

void ResetPad(int port, int slot) {
	memset(&pads[port][slot], 0, sizeof(pads[0][0]));
	pads[port][slot].mode = MODE_DIGITAL;
	pads[port][slot].umask[0] = pads[port][slot].umask[1] = 0xFF;
	ResetVibrate(port, slot);
	if (config.padConfigs[port][slot].autoAnalog) {
		pads[port][slot].mode = MODE_ANALOG;
	}
}


struct QueryInfo {
	u8 port;
	u8 slot;
	u8 lastByte;
	u8 currentCommand;
	u8 numBytes;
	u8 queryDone;
	u8 response[42];
} query = {0,0,0,0, 0,0xFF, 0xF3};

int saveStateIndex = 0;

s32 CALLBACK PADinit(u32 flags) {
	// Note:  Won't load settings if already loaded.
	if (LoadSettings() < 0) {
		return -1;
	}
	int pad = (flags & 3);
	if (pad == 3) {
		if (PADinit(1)) return -1;
		return PADinit(2);
	}
	#ifdef _DEBUG
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag( tmpFlag );
	#endif
	pad --;

	ResetPad(pad, 0);

	pads[pad][0].initialized = 1;
	memset(slots, 0, sizeof(slots));

	query.lastByte = 1;
	query.numBytes = 0;
	ClearKeyQueue();
	// Just in case, when resuming emulation.
	QueueKeyEvent(VK_SHIFT, KEYRELEASE);
	QueueKeyEvent(VK_MENU, KEYRELEASE);
	QueueKeyEvent(VK_CONTROL, KEYRELEASE);
	return 0;
}



// Note to self:  Has to be a define for the sizeof() to work right.
// Note to self 2: All are the same size, anyways, except for longer full DS2 response
//   and shorter digital mode response.
#define SET_RESULT(a) { \
	memcpy(query.response+2, a, sizeof(a)); \
	query.numBytes = 2+sizeof(a); \
}

#define SET_FINAL_RESULT(a) {			  \
	memcpy(query.response+2, a, sizeof(a));\
	query.numBytes = 2+sizeof(a);		  \
	query.queryDone = 1;			  \
}

static const u8 ConfigExit[7] = {0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//static const u8 ConfigExit[7] = {0x5A, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

static const u8 noclue[7] =			{0x5A, 0x00, 0x00, 0x02, 0x00, 0x00, 0x5A};
static u8 queryMaskMode[7] =    {0x5A, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x5A};
//static const u8 DSNonNativeMode[7] = {0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const u8 setMode[7] =         {0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// DS2
static const u8 queryModelDS2[7] =      {0x5A, 0x03, 0x02, 0x00, 0x02, 0x01, 0x00};
// DS1
static const u8 queryModelDS1[7] =      {0x5A, 0x01, 0x02, 0x00, 0x02, 0x01, 0x00};

static const u8 queryAct[2][7] =    {{0x5A, 0x00, 0x00, 0x01, 0x02, 0x00, 0x0A},
									 {0x5A, 0x00, 0x00, 0x01, 0x01, 0x01, 0x14}};

static const u8 queryComb[7] =       {0x5A, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00};

static const u8 queryMode[7] =		{0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


static const u8 setNativeMode[7] =  {0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A};

// Implements a couple of the hacks, also responsible for monitoring device addition/removal and focus
// changes.
ExtraWndProcResult HackWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *output) {
	switch (uMsg) {
		case WM_SETTEXT:
			if (config.saveStateTitle) {
				wchar_t text[200];
				int len;
				if (IsWindowUnicode(hWnd)) {
					len = wcslen((wchar_t*) lParam);
					if (len < sizeof(text)/sizeof(wchar_t)) wcscpy(text, (wchar_t*) lParam);
				}
				else {
					len = MultiByteToWideChar(CP_ACP, 0, (char*) lParam, -1, text, sizeof(text)/sizeof(wchar_t));
				}
				if (len > 0 && len < 150 && !wcsstr(text, L" | State ")) {
					wsprintfW(text+len, L" | State %i", saveStateIndex);
					SetWindowText(hWnd, text);
					return NO_WND_PROC;
				}
			}
			break;
		case WM_DEVICECHANGE:
			if (wParam == DBT_DEVNODES_CHANGED) {
				// Need to do this when not reading input from gs thread.
				// Checking for that case not worth the effort.
				EnterCriticalSection(&readInputCriticalSection);
				UpdateEnabledDevices(1);
				LeaveCriticalSection(&readInputCriticalSection);
			}
			break;
		case WM_ACTIVATEAPP:
			// Release any buttons PCSX2 may think are down when
			// losing/gaining focus.
			QueueKeyEvent(VK_SHIFT, KEYRELEASE);
			QueueKeyEvent(VK_MENU, KEYRELEASE);
			QueueKeyEvent(VK_CONTROL, KEYRELEASE);

			// Need to do this when not reading input from gs thread.
			// Checking for that case not worth the effort.
			EnterCriticalSection(&readInputCriticalSection);
			if (!wParam) {
				activeWindow = 0;
				UpdateEnabledDevices();
			}
			else {
				activeWindow = 1;
				UpdateEnabledDevices();
			}
			LeaveCriticalSection(&readInputCriticalSection);
			break;
		case WM_CLOSE:
			if (config.closeHacks & 1) {
				QueueKeyEvent(VK_ESCAPE, KEYPRESS);
				return NO_WND_PROC;
			}
			else if (config.closeHacks & 2) {
				ExitProcess(0);
				return NO_WND_PROC;
			}
			break;
		case WM_SYSCOMMAND:
			if ((wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER) && config.disableScreenSaver)
				return NO_WND_PROC;
			break;
		case WM_DESTROY:
			QueueKeyEvent(VK_ESCAPE, KEYPRESS);
			break;
		default:
			break;
	}
	return CONTINUE_BLISSFULLY;
}

// All that's needed to force hiding the cursor in the proper thread.
// Could have a special case elsewhere, but this make sure it's called
// only once, rather than repeatedly.
ExtraWndProcResult HideCursorProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *output) {
	ShowCursor(0);
	return CONTINUE_BLISSFULLY_AND_RELEASE_PROC;
}

char restoreFullScreen = 0;
DWORD WINAPI MaximizeWindowThreadProc(void *lpParameter) {
	Sleep(100);
	keybd_event(VK_LMENU, MapVirtualKey(VK_LMENU, MAPVK_VK_TO_VSC), 0, 0);
	keybd_event(VK_RETURN, MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC), 0, 0);
	Sleep(10);
	keybd_event(VK_RETURN, MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
	keybd_event(VK_LMENU, MapVirtualKey(VK_LMENU, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
	return 0;
}

s32 CALLBACK PADopen(void *pDsp) {
	if (openCount++) return 0;

	// Not really needed, shouldn't do anything.
	if (LoadSettings()) return -1;
	miceEnabled = !config.mouseUnfocus;
	if (!hWnd) {
		if (IsWindow((HWND)pDsp)) {
			hWnd = (HWND) pDsp;
		}
		else if (pDsp && !IsBadReadPtr(pDsp, 4) && IsWindow(*(HWND*) pDsp)) {
			hWnd = *(HWND*) pDsp;
		}
		else {
			openCount = 0;
			return -1;
		}
		while (GetWindowLong (hWnd, GWL_STYLE) & WS_CHILD)
			hWnd = GetParent (hWnd);
		// Implements most hacks, as well as enabling/disabling mouse
		// capture when focus changes.
		if (!EatWndProc(hWnd, HackWndProc)) {
			openCount = 0;
			return -1;
		}
		if (config.forceHide) {
			EatWndProc(hWnd, HideCursorProc);
		}
	}

	if (restoreFullScreen) {
		if (!IsWindowMaximized(hWnd)) {
			HANDLE hThread = CreateThread(0, 0, MaximizeWindowThreadProc, hWnd, 0, 0);
			if (hThread) CloseHandle(hThread);
		}
		restoreFullScreen = 0;
	}
	for (int port=0; port<2; port++) {
		for (int slot=0; slot<4; slot++) {
			memset(&pads[port][slot].sum, 0, sizeof(pads[port][slot].sum));
			memset(&pads[port][slot].lockedSum, 0, sizeof(pads[port][slot].lockedSum));
			pads[port][slot].lockedState = 0;
		}
	}

	// I'd really rather use this line, but GetActiveWindow() does not have complete specs.
	// It *seems* to return null when no window from this thread has focus, but the
	// Microsoft specs seem to imply it returns the window from this thread that would have focus,
	// if any window did (topmost in this thread?).  Which isn't what I want, and doesn't seem
	// to be what it actually does.
	// activeWindow = GetActiveWindow() == hWnd;

	activeWindow = (GetAncestor(hWnd, GA_ROOT) == GetAncestor(GetForegroundWindow(), GA_ROOT));
	UpdateEnabledDevices();
	return 0;
}

void CALLBACK PADclose() {
	if (openCount && !--openCount) {
		dm->ReleaseInput();
		ReleaseEatenProc();
		hWnd = 0;
		ClearKeyQueue();
	}
}

u8 CALLBACK PADstartPoll(int pad) {
	DEBUG_NEW_SET();
	pad--;
	if ((unsigned int)pad <= 1) {
		query.queryDone = 0;
		query.port = pad;
		query.slot = slots[query.port];
		query.numBytes = 2;
		query.lastByte = 0;
		DEBUG_IN(pad);
		DEBUG_OUT(0xFF);
		return 0xFF;
	}
	else {
		query.queryDone = 1;
		query.numBytes = 0;
		query.lastByte = 1;
		DEBUG_IN(pad);
		DEBUG_OUT(0);
		return 0;
	}
}

u8 CALLBACK PADpoll(u8 value) {
	DEBUG_IN(value);
	if (query.lastByte+1 >= query.numBytes) {
		DEBUG_OUT(0);
		return 0;
	}
	if (query.lastByte && query.queryDone) {
		DEBUG_OUT(query.response[1+query.lastByte]);
		return query.response[++query.lastByte];
	}
	/*
	{
		query.numBytes = 35;
		u8 test[35] = {0xFF, 0x80, 0x5A,
			0x73, 0x5A, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80,
			0x73, 0x5A, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80,
			0x73, 0x5A, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80,
			0x73, 0x5A, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80
		};
		memcpy(query.response, test, sizeof(test));
		DEBUG_OUT(query.response[1+query.lastByte]);
		return query.response[++query.lastByte];
	}//*/
	int i;
	Pad *pad = &pads[query.port][query.slot];
	if (query.lastByte == 0) {
		query.lastByte++;
		query.currentCommand = value;
		switch(value) {
		// CONFIG_MODE
		case 0x43:
			if (pad->config) {
				// In config mode.  Might not actually be leaving it.
				SET_RESULT(ConfigExit);
				DEBUG_OUT(0xF3);
				return 0xF3;
			}
		// READ_DATA_AND_VIBRATE
		case 0x42:
			query.response[2] = 0x5A;
			{
				if (!config.GSThreadUpdates) {
					Update(query.port);
				}
				ButtonSum *sum = &pad->sum;

				u8 b1 = 0xFF, b2 = 0xFF;
				for (i = 0; i<4; i++) {
					b1 -= (sum->buttons[i]>=128) << i;
				}
				for (i = 0; i<8; i++) {
					b2 -= (sum->buttons[i+4]>=128) << i;
				}
				if (config.padConfigs[query.port][query.slot].type == GuitarPad && !config.GH2) {
					sum->sticks[0].horiz = -255;
					// Not sure about this.  Forces wammy to be from 0 to 0x7F.
					// if (sum->sticks[2].vert > 0) sum->sticks[2].vert = 0;
				}
				b1 -= ((sum->sticks[0].vert<=-128) << 4);
				b1 -= ((sum->sticks[0].horiz>=128) << 5);
				b1 -= ((sum->sticks[0].vert>=128) << 6);
				b1 -= ((sum->sticks[0].horiz<=-128) << 7);
				query.response[3] = b1;
				query.response[4] = b2;

				query.numBytes = 5;
				if (pad->mode != MODE_DIGITAL) {
					query.response[5] = Cap((sum->sticks[1].horiz+255)/2);
					query.response[6] = Cap((sum->sticks[1].vert+255)/2);
					query.response[7] = Cap((sum->sticks[2].horiz+255)/2);
					query.response[8] = Cap((sum->sticks[2].vert+255)/2);

					query.numBytes = 9;
					if (pad->mode != MODE_ANALOG) {
						// Good idea?  No clue.
						//query.response[3] &= pad->mask[0];
						//query.response[4] &= pad->mask[1];

						// Each value is from -255 to 255, so have to use cap to convert
						// negative values to 0.
						query.response[9] = Cap(sum->sticks[0].horiz);
						query.response[10] = Cap(-sum->sticks[0].horiz);
						query.response[11] = Cap(-sum->sticks[0].vert);
						query.response[12] = Cap(sum->sticks[0].vert);

						// No need to cap these, already done int CapSum().
						query.response[13] = (unsigned char) sum->buttons[8];
						query.response[14] = (unsigned char) sum->buttons[9];
						query.response[15] = (unsigned char) sum->buttons[10];
						query.response[16] = (unsigned char) sum->buttons[11];
						query.response[17] = (unsigned char) sum->buttons[6];
						query.response[18] = (unsigned char) sum->buttons[7];
						query.response[19] = (unsigned char) sum->buttons[4];
						query.response[20] = (unsigned char) sum->buttons[5];
						query.numBytes = 21;
					}
				}
			}

			query.lastByte=1;
			DEBUG_OUT(pad->mode);
			return pad->mode;
		// SET_VREF_PARAM
		case 0x40:
			SET_FINAL_RESULT(noclue);
			break;
		// QUERY_DS2_ANALOG_MODE
		case 0x41:
			if (pad->mode == MODE_DIGITAL) {
				queryMaskMode[1] = queryMaskMode[2] = queryMaskMode[3] = 0;
				queryMaskMode[6] = 0x00;
			}
			else {
				queryMaskMode[1] = pad->umask[0];
				queryMaskMode[2] = pad->umask[1];
				queryMaskMode[3] = 0x03;
				// Not entirely sure about this.
				//queryMaskMode[3] = 0x01 | (pad->mode == MODE_DS2_NATIVE)*2;
				queryMaskMode[6] = 0x5A;
			}
			SET_FINAL_RESULT(queryMaskMode);
			break;
		// SET_MODE_AND_LOCK
		case 0x44:
			SET_RESULT(setMode);
			ResetVibrate(query.port, query.slot);
			break;
		// QUERY_MODEL_AND_MODE
		case 0x45:
			if (config.padConfigs[query.port][query.slot].type != GuitarPad || config.GH2) SET_FINAL_RESULT(queryModelDS2)
			else SET_FINAL_RESULT(queryModelDS1);
			query.response[5] = pad->mode != MODE_DIGITAL;
			break;
		// QUERY_ACT
		case 0x46:
			SET_RESULT(queryAct[0]);
			break;
		// QUERY_COMB
		case 0x47:
			SET_FINAL_RESULT(queryComb);
			break;
		// QUERY_MODE
		case 0x4C:
			SET_RESULT(queryMode);
			break;
		// VIBRATION_TOGGLE
		case 0x4D:
			memcpy(query.response+2, pad->vibrate, 7);
			query.numBytes = 9;
			ResetVibrate(query.port, query.slot);
			break;
		// SET_DS2_NATIVE_MODE
		case 0x4F:
			SET_RESULT(setNativeMode);
			break;
		default:
			query.numBytes = 0;
			query.queryDone = 1;
			break;
		}
		DEBUG_OUT(0xF3);
		return 0xF3;
	}
	else {
		query.lastByte++;
		switch (query.currentCommand) {
			// READ_DATA_AND_VIBRATE
			case 0x42:
				if (query.lastByte == pad->vibrateI[0]) {
					SetVibrate(query.port, query.slot, 1, 255*(0!=value));
				}
				else if (query.lastByte == pad->vibrateI[1]) {
					SetVibrate(query.port, query.slot, 0, value);
				}
				break;
			// CONFIG_MODE
			case 0x43:
				if (query.lastByte == 3) {
					query.queryDone = 1;
					pad->config = value;
				}
				break;
			// SET_MODE_AND_LOCK
			case 0x44:
				if (query.lastByte == 3 && value < 2) {
					static const u8 modes[2] = {MODE_DIGITAL, MODE_ANALOG};
					pad->mode = modes[value];
				}
				else if (query.lastByte == 4) {
					if (value == 3) {
						pad->modeLock = 3;
					}
					else {
						pad->modeLock = 0;
						if (pad->mode == MODE_DIGITAL && config.padConfigs[query.port][query.slot].autoAnalog) {
							pad->mode = MODE_ANALOG;
						}
					}
					query.queryDone = 1;
				}
				break;
			// QUERY_ACT
			case 0x46:
				if (query.lastByte == 3) {
					if (value<2) SET_RESULT(queryAct[value])
					// bunch of 0's
					// else SET_RESULT(setMode);
					query.queryDone = 1;
				}
				break;
			// QUERY_MODE
			case 0x4C:
				if (query.lastByte == 3 && value<2) {
					query.response[6] = 4+value*3;
					query.queryDone = 1;
				}
				// bunch of 0's
				//else data = setMode;
				break;
			// VIBRATION_TOGGLE
			case 0x4D:
				if (query.lastByte>=3) {
					if (value == 0) {
						pad->vibrateI[0] = (u8)query.lastByte;
					}
					else if (value == 1) {
						pad->vibrateI[1] = (u8)query.lastByte;
					}
					pad->vibrate[query.lastByte-2] = value;
				}
				break;
			case 0x4F:
				if (query.lastByte == 3 || query.lastByte == 4) {
					pad->umask[query.lastByte-3] = value;
				}
				else if (query.lastByte == 5) {
					if (!(value & 1)) {
						pad->mode = MODE_DIGITAL;
					}
					else if (!(value & 2)) {
						pad->mode = MODE_ANALOG;
					}
					else {
						pad->mode = MODE_DS2_NATIVE;
					}
				}
				break;
			default:
				DEBUG_OUT(0);
				return 0;
		}
		DEBUG_OUT(query.response[query.lastByte]);
		return query.response[query.lastByte];
	}
}

// returns: 1 if supports pad1
//			2 if supports pad2
//			3 if both are supported
u32 CALLBACK PADquery() {
	return 3;
}

//void CALLBACK PADgsDriverInfo(GSdriverInfo *info) {
//}

INT_PTR CALLBACK AboutDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_INITDIALOG) {
		wchar_t idString[100];
		GetNameAndVersionString(idString);
		SetDlgItemTextW(hWndDlg, IDC_VERSION, idString);
	}
	else if (uMsg == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)) {
		EndDialog(hWndDlg, 0);
		return 1;
	}
	return 0;
}


void CALLBACK PADabout() {
	DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), 0, AboutDialogProc);
}

s32 CALLBACK PADtest() {
	return 0;
}

DWORD WINAPI RenameWindowThreadProc(void *lpParameter) {
	wchar_t newTitle[200];
	if (hWnd) {
		int len = GetWindowTextW(hWnd, newTitle, 200);
		if (len > 0 && len < 199) {
			wchar_t *end;
			if (end = wcsstr(newTitle, L" | State ")) *end = 0;
			SetWindowTextW(hWnd, newTitle);
		}
	}
	return 0;
}

keyEvent* CALLBACK PADkeyEvent() {
	if (!config.GSThreadUpdates) {
		Update(2);
	}
	static char shiftDown = 0;
	static char altDown = 0;
	static keyEvent ev;
	if (!GetQueuedKeyEvent(&ev)) return 0;
	if ((ev.key == VK_ESCAPE || (int)ev.key == -2) && ev.evt == KEYPRESS && config.escapeFullscreenHack) {
		static int t;
		if ((int)ev.key != -2 && IsWindowMaximized(hWnd)) {
			t = timeGetTime();
			QueueKeyEvent(-2, KEYPRESS);
			HANDLE hThread = CreateThread(0, 0, MaximizeWindowThreadProc, 0, 0, 0);
			if (hThread) CloseHandle(hThread);
			restoreFullScreen = 1;
			return 0;
		}
		if (ev.key != VK_ESCAPE) {
			if (timeGetTime() - t < 1000) {
				QueueKeyEvent(-2, KEYPRESS);
				return 0;
			}
		}
		ev.key = VK_ESCAPE;
	}

	if (ev.key == VK_F2 && ev.evt == KEYPRESS) {
		saveStateIndex += 1 - 2*shiftDown;
		saveStateIndex = (saveStateIndex+10)%10;
		if (config.saveStateTitle) {
			// GSDX only checks its window's message queue at certain points or something, so
			// have to do this in another thread to prevent deadlock.
			HANDLE hThread = CreateThread(0, 0, RenameWindowThreadProc, 0, 0, 0);
			if (hThread) CloseHandle(hThread);
		}
	}

	// So don't change skip mode on alt-F4.
	if (ev.key == VK_F4 && altDown) {
		return 0;
	}

	if (ev.key == VK_LSHIFT || ev.key == VK_RSHIFT || ev.key == VK_SHIFT) {
		ev.key = VK_SHIFT;
		shiftDown = (ev.evt == KEYPRESS);
	}
	else if (ev.key == VK_LCONTROL || ev.key == VK_RCONTROL) {
		ev.key = VK_CONTROL;
	}
	else if (ev.key == VK_LMENU || ev.key == VK_RMENU || ev.key == VK_SHIFT) {
		ev.key = VK_MENU;
		altDown = (ev.evt == KEYPRESS);
	}
	return &ev;
}

#define PAD_SAVE_STATE_VERSION	0

struct PadPluginFreezeData {
	char format[8];
	// Currently all different versions are incompatible.
	// May split into major/minor with some compatibility rules.
	u32 version;
	// So when loading, know which plugin's settings I'm loading.
	// Not a big deal.  Use a static variable when saving to figure it out.
	u8 port;
	// active slot for port
	u8 slot;
	// Currently only use padData[0].  Save room for all 4 slots for simplicity.
	PadFreezeData padData[4];
};

s32 CALLBACK PADfreeze(int mode, freezeData *data) {
	if (mode == FREEZE_SIZE) {
		data->size = sizeof(PadPluginFreezeData);
	}
	else if (mode == FREEZE_LOAD) {
		if (data->size < sizeof(PadPluginFreezeData)) return 0;
		PadPluginFreezeData &pdata = *(PadPluginFreezeData*)(data->data);
		if (pdata.version != PAD_SAVE_STATE_VERSION || strcmp(pdata.format, "PadMode")) {
			return 0;
		}
		StopVibrate();
		int port = pdata.port;
		for (int slot=0; slot<4; slot++) {
			u8 mode = pads[port][slot].mode = pdata.padData[slot].mode;
			if (mode != MODE_DIGITAL && mode != MODE_ANALOG && mode != MODE_DS2_NATIVE) {
				ResetPad(port, slot);
				continue;
			}
			pads[port][slot].config = pdata.padData[slot].config;
			pads[port][slot].modeLock = pdata.padData[slot].modeLock;
			memcpy(pads[port][slot].umask, pdata.padData[slot].umask, sizeof(pads[port][slot].umask));

			slots[port] = slot;
			// Means I only have to have one chunk of code to parse vibrate info.
			// Other plugins don't store it exactly, but think it's technically correct
			// to do so, though I could be wrong.
			PADstartPoll(port+1);
			PADpoll(0x4D);
			for (int j=0; j<7; j++) {
				PADpoll(pdata.padData[slot].vibrate[j]);
			}
		}
		slots[port] = pdata.slot;
	}
	else if (mode == FREEZE_SAVE) {
		if (data->size != sizeof(PadPluginFreezeData)) return 0;
		PadPluginFreezeData &pdata = *(PadPluginFreezeData*)(data->data);
		static int nextPort = 0;
		if (!portInitialized[nextPort]) nextPort ^= 1;
		int port = nextPort;
		if (!portInitialized[nextPort^1]) nextPort = 0;
		else nextPort ^= 1;


		memset(&pdata, 0, sizeof(pdata));
		strcpy(pdata.format, "PadMode");
		pdata.version = PAD_SAVE_STATE_VERSION;
		pdata.port = port;
		pdata.slot = slots[port];
		for (int slot=0; slot<4; slot++) {
			pdata.padData[slot] = pads[port][slot];
		}
	}
	else return -1;
	return 0;
 }

u32 CALLBACK PADreadPort1 (PadDataS* pads) {
	PADstartPoll(1);
	PADpoll(0x42);
	memcpy(pads, query.response+1, 7);
	pads->controllerType = pads[0].controllerType>>4;
	memset (pads+7, 0, sizeof(PadDataS)-7);
	return 0;
}

u32 CALLBACK PADreadPort2 (PadDataS* pads) {
	PADstartPoll(2);
	PADpoll(0x42);
	memcpy(pads, query.response+1, 7);
	pads->controllerType = pads->controllerType>>4;
	memset (pads+7, 0, sizeof(PadDataS)-7);
	return 0;
}

u32 CALLBACK PSEgetLibType() {
	return 8;
}

u32 CALLBACK PSEgetLibVersion() {
	return (VERSION & 0xFFFFFF);
}

// Little funkiness to handle rounding floating points to ints without the C runtime.
// Unfortunately, means I can't use /GL optimization option when NO_CRT is defined.
#ifdef NO_CRT
extern "C" long _cdecl _ftol();
extern "C" long _cdecl _ftol2_sse() {
	return _ftol();
}
extern "C" long _cdecl _ftol2() {
	return _ftol();
}
#endif
