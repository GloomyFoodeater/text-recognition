#pragma once

#include "resource.h"
#include "RichEdit.h"
#include "preprocessing.h"
#include "segmentation.h"
#include "recognition.h"
#include "postprocessing.h"
#include <fstream>
#include <codecvt>
#include <commdlg.h>
#define MAX_LOADSTRING  100
#define MIN_WND_WIDHT   800		// Min width of the main window
#define MIN_WND_HEIGHT  600		// Min height of the main window
#define S_PBOX			0.44	// Scale of the picture box
#define S_TEXT			0.44	// Scale of the text field
#define S_OPTIONS		0.12	// Scale of the panel
#define BTN_WIDTH		150		// Width of buttons
#define BTN_HEIGHT		40		// Height of buttons 
#define BTN_GAP			20		// Height of buttons' gaps
#define WIDTH(r) ((r).right - (r).left)
#define HEIGHT(r) ((r).bottom - (r).top)
#define TXTFLD_STYLE	WS_BORDER | WS_CHILD | WS_VISIBLE |\
						WS_VSCROLL | ES_LEFT | ES_MULTILINE |\
						ES_AUTOVSCROLL | ES_READONLY
#define PBOX_STYLE		WS_VISIBLE | WS_CHILD | SS_WHITERECT |\
						SS_NOTIFY | SS_BITMAP | SS_CENTERIMAGE

// Function prototypes
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void                onCreate();
void                onSize();
void                onLoad();
void				onUnload();
void				onExtract();
void				onSave();
void				onAbout();
wstring				loadString(UINT);
vector<Mat>			loadTemplates(UINT, UINT);
vector<wstring>		loadDictionary(wstring);
void				rewriteDictionary(const vector<wstring>&, wstring);
void				setDefaultImage();
void				setDefaultText();
