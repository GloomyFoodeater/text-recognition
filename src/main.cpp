#include "framework.h"
#include "main.h"

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// Descriptors
HWND hMainWnd, // Main window
hPctBox, // Picture box to load image
hTxtFld, // Text field for recognized text
hBtnLoad, hBtnUnload, hBtnExtract, hBtnSave, // Buttons
hCbDrawBounds, hCbPostProcess; // Checkboxes

Bitmap* image = nullptr; // Image to extract text from
Bounds* bounds = nullptr;
vector<Mat> templates; // Templates to match with
wstring labels; // Templates' labels
vector<wstring> dict; // Dictionary for postprocessing

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Init GDI+
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Ok)
		return FALSE;

	// Init COM
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (!SUCCEEDED(hr))
	{
		GdiplusShutdown(gdiplusToken);
		return FALSE;
	}

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_TEXTRECOGNITION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEXTRECOGNITION));

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// Close COM, GDI+
	CoUninitialize();
	GdiplusShutdown(gdiplusToken);

	return (int)msg.wParam;
}

// Register main window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEXTRECOGNITION));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TEXTRECOGNITION);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

// Initialise app's instance and create main window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	hMainWnd = CreateWindowW(szWindowClass, szTitle,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		nullptr, nullptr, hInstance, nullptr);

	if (!hMainWnd)
		return FALSE;

	ShowWindow(hMainWnd, nCmdShow);
	UpdateWindow(hMainWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		hMainWnd = hWnd;
		onCreate();
		break;
	}
	case WM_SIZE:
	{
		onSize();
		break;
	}
	case WM_GETMINMAXINFO:
	{
		// Set min size of the window
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = MIN_WND_WIDHT;
		lpMMI->ptMinTrackSize.y = MIN_WND_HEIGHT;
		break;
	}
	case WM_CTLCOLORSTATIC:
	{
		int id = GetDlgCtrlID((HWND)lParam);
		if (id == IDC_CB_DRAW || id == IDC_CB_POSTPROC || id == IDC_PCT_BOX)
			return (LRESULT)GetStockObject(WHITE_BRUSH);
		break;
	}
	case WM_COMMAND:
	{
		int id = LOWORD(wParam);
		int code = HIWORD(wParam);
		switch (id)
		{
		case IDM_ABOUT:
			onAbout();
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_OPEN:
			onLoad();
			break;
		case IDM_SAVE:
			onSave();
			break;
		case IDM_CLOSE:
			onUnload();
			break;
		case IDC_BTN_LOAD:
			if (code == BN_CLICKED)
				onLoad();
			break;
		case IDC_BTN_UNLOAD:
			if (code == BN_CLICKED)
				onUnload();
			break;
		case IDC_BTN_EXTRACT:
			if (code == BN_CLICKED)
				onExtract();
			break;
		case IDC_BTN_SAVE:
			if (code == BN_CLICKED)
				onSave();
			break;
		case IDC_PCT_BOX:
			if (code == STN_DBLCLK)
				onLoad();
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_DESTROY:
		onUnload();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void redrawImage(const Bounds* bounds = nullptr)
{
	if (image)
	{
		RECT r;
		GetClientRect(hPctBox, &r);
		int w = image->GetWidth(), h = image->GetHeight();

		// Image must fit biggest size of picture box
		double scaleX = WIDTH(r) / (double)w;
		double scaleY = HEIGHT(r) / (double)h;
		double scale = min(scaleX, scaleY);
		if (scale < 1)
		{
			// Picture doesn't fit
			w *= scale;
			h *= scale;
		}
		else
			scale = 1; // Picture fits and there are white spaces

		// Scale image
		Bitmap output(w, h);
		Graphics g(&output);
		g.ScaleTransform(scale, scale); // Set scale matrix
		g.DrawImage(image, Rect(0, 0, image->GetWidth(), image->GetHeight()));

		// Draw bounds
		if (bounds && IsDlgButtonChecked(hMainWnd, IDC_CB_DRAW))
		{
			Pen letterPen(Color(255, 0, 255, 0), 1);
			for (const auto& line : *bounds)
				for (const auto& word : line)
					for (const auto& letter : word)
						g.DrawRectangle(&letterPen, letter);
		}

		// Reset image on static control
		output.SetPixel(0, 0, Color(255, 255, 255)); // SS_CENTERIMAGE
													 // fills background with (0, 0) px
		HBITMAP hBmp;
		if (output.GetHBITMAP(0, &hBmp) == Ok) // Extract handle
			SendMessageW(hPctBox, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
		else
		{
			onUnload();
		}
	}
}

// WM_CREATE handler: creates windows, loads recognition data
void onCreate()
{
	// Load recognition data
	labels = loadString(IDS_LABELS);
	templates = loadTemplates(IDB_BITMAP1, labels.size());
	dict = loadDictionary(L"dictionary.txt");
	rewriteDictionary(dict, L"dictionary.txt"); // Write sorted

	// Create picture box
	hPctBox = CreateWindowW(L"STATIC", nullptr, PBOX_STYLE,
		0, 0, 0, 0, hMainWnd, (HMENU)IDC_PCT_BOX, hInst, nullptr);
	setDefaultImage();

	// Create text field
	LoadLibraryW(L"Msftedit.dll");
	hTxtFld = CreateWindowExW(0, MSFTEDIT_CLASS, nullptr, TXTFLD_STYLE,
		0, 0, 0, 0, hMainWnd, nullptr, hInst, nullptr);
	setDefaultText();

	// Create buttons
	hBtnLoad = CreateWindowW(L"BUTTON", L"Открыть",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		0, 0, 0, 0, hMainWnd, (HMENU)IDC_BTN_LOAD, hInst, nullptr);
	hBtnUnload = CreateWindowW(L"BUTTON", L"Закрыть",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		0, 0, 0, 0, hMainWnd, (HMENU)IDC_BTN_UNLOAD, hInst, nullptr);
	hBtnExtract = CreateWindowW(L"BUTTON", L"Извлечь текст",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		0, 0, 0, 0, hMainWnd, (HMENU)IDC_BTN_EXTRACT, hInst, nullptr);
	hBtnSave = CreateWindowW(L"BUTTON", L"Сохранить",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		0, 0, 0, 0, hMainWnd, (HMENU)IDC_BTN_SAVE, hInst, nullptr);

	// Create checkboxes
	hCbDrawBounds = CreateWindowW(L"BUTTON", L"Рисовать границы",
		WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
		0, 0, 0, 0, hMainWnd, (HMENU)IDC_CB_DRAW, hInst, nullptr);
	hCbPostProcess = CreateWindowW(L"BUTTON", L"Постобработка",
		WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
		0, 0, 0, 0, hMainWnd, (HMENU)IDC_CB_POSTPROC, hInst, nullptr);
	SendMessageW(hCbPostProcess, BM_SETCHECK, BST_CHECKED, 0);
}

// WM_SIZE handler: moves windows
void onSize()
{
	// Initialise coordinates
	RECT r;
	GetClientRect(hMainWnd, &r);
	int x, y = 0, w, h = HEIGHT(r);
	int dw = max((BTN_WIDTH - S_OPTIONS * WIDTH(r)) / 2, 0);

	// Move picture box
	x = 0;
	w = (int)(S_PBOX * WIDTH(r)) - dw;
	MoveWindow(hPctBox, x, y, w, h, TRUE);

	// Move text field
	x += w;
	w = (int)(S_TEXT * WIDTH(r)) - dw;
	MoveWindow(hTxtFld, x, y, w, h, TRUE);

	// Move buttons
	x += w + (WIDTH(r) - x - w - BTN_WIDTH) / 2;
	y = BTN_GAP;
	w = BTN_WIDTH;
	h = BTN_HEIGHT;
	MoveWindow(hBtnLoad, x, y, w, h, TRUE);
	y += BTN_GAP + h;
	MoveWindow(hBtnUnload, x, y, w, h, TRUE);
	y += BTN_GAP + h;
	MoveWindow(hBtnExtract, x, y, w, h, TRUE);
	y += BTN_GAP + h;
	MoveWindow(hBtnSave, x, y, w, h, TRUE);
	y += BTN_GAP + h + h;
	MoveWindow(hCbDrawBounds, x, y, w, h, TRUE);
	y += BTN_GAP + h;
	MoveWindow(hCbPostProcess, x, y, w, h, TRUE);

	redrawImage(bounds);
}

// hBtnLoad click handler: call open dialog and loads image
void onLoad()
{
	// Init input and output structure for dialog
	OPENFILENAME ofn = { 0 };
	wchar_t szFilename[256] = L"";
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrFilter = L"Изображения (*.bmp, *.jpg, *.png)\0*.bmp;*.jpg;*.png\0";
	ofn.lpstrTitle = L"Открыть";
	ofn.lpstrFile = szFilename;
	ofn.nMaxFile = 256;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Open file dialog and get file name
	if (GetOpenFileName(&ofn))
	{
		onUnload(); // Reset text and image
		image = new Bitmap(ofn.lpstrFile); // Load image
		redrawImage();
	}
}

// hBtnUnload click handler: reset text and image and free memory
void onUnload()
{
	if (image)
	{
		// Load default image and text
		setDefaultImage();
		setDefaultText();

		// Reset pointers
		delete image;
		image = nullptr;
		if (bounds)
		{
			delete bounds;
			bounds = nullptr;
		}
	}

}

// hBtnExtract click handler: recognises text from image into rich edit
void onExtract()
{
	if (image)
	{
		wstring text;
		try
		{
			auto mat = getGrayscale(*image);
			threshold(mat, 175); // Preprocess
			if (bounds)
			{
				delete bounds;
				bounds = nullptr;
			}
			bounds = new Bounds(getBounds(mat)); // Segmentation
			// Optional output
			if (IsDlgButtonChecked(hMainWnd, IDC_CB_DRAW))
				redrawImage(bounds);
			else
				redrawImage(nullptr);
			text = recognise(mat, *bounds, labels, templates); // Recognition
			if (IsDlgButtonChecked(hMainWnd, IDC_CB_POSTPROC))
				text = correctText(text, dict); // Postprocessing
		}
		catch (exception& e)
		{
			text = L"Ошибка при распознавании...";
		}
		SendMessageW(hTxtFld, WM_SETTEXT, 0, (LPARAM)text.c_str()); // Output
	}
}

// hBtnSave click handler: saves text from rich edit to file
void onSave()
{
	if (image)
	{
		// Init input and output structure for dialog
		OPENFILENAME ofn = { 0 };
		wchar_t szFilename[256] = L"";
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hMainWnd;
		ofn.lpstrFilter = L"Текстовые файлы (*.txt)\0*.txt\0";
		ofn.lpstrTitle = L"Сохранить как";
		ofn.lpstrFile = szFilename;
		ofn.nMaxFile = 256;
		ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
		ofn.lpstrDefExt = L"txt";

		// Open file dialog
		if (GetSaveFileName(&ofn))
		{
			// Create stream and set conversion
			wofstream fs(ofn.lpstrFile);
			fs.imbue(locale(locale::empty(), new codecvt_utf8<wchar_t>));

			// Get text from field
			WCHAR* buffer = 0;
			int nLen = GetWindowTextLength(hTxtFld);
			if (nLen >= 0)
			{
				buffer = new WCHAR[nLen + 1];
				buffer[nLen] = L'\0';
				if (GetWindowText(hTxtFld, buffer, nLen + 1))
				{
					// Save to file without '\r'
					wstring text(buffer);
					text.erase(remove(text.begin(), text.end(), L'\r'), text.end());
					fs << text;
				}
				delete[] buffer;
			}
			fs.close();
		}
	}
}

// IDM_ABOUT handle: calls helper
void onAbout()
{
	wstring x = L"Данное приложение предназначено для распознавания печатного текста шрифта Arial с фотографий формата BMP, JPG, PNG.";
	x += L"\n\nЭтап препроцессинга фотографии состоит только из бинаризации изображения, поэтому не гарантируется правильное распознавание текста с фотографий с различными искажениями : шумами, перепадами освещения, поворотами и т.д.";
	x += L"\n\nПриложение опускает этап детектирования текста, полагая, что вся фотография состоит из цельного текста структуры : строки->слова->буквы.";
	x += L"\n\nСегментация цельного текста основана на методе проекционных профилей : проекционный профиль - массив сумм яркостей пикселей / числа пикселей переднего плана, вычисленная в определённом столбце или строке в зависимости от вида профиля.";
	x += L"\n\nБлок классификации использует метод сопоставления с шаблоном, который попиксельно сравнивает часть фотографии с заданными шаблонами.Фотографии шаблонов являются ресурсами приложения.";
	x += L"\n\nПостпроцессинг делает все буквы строчными и использует метрику Левенштейна для замены слов с орфографическими ошибками на корректные.Словарь составляется из файла dictionary.txt";
	MessageBoxW(hMainWnd, x.c_str(), L"Справка", MB_OK);
}

// Load string from resource
wstring loadString(UINT ID)
{
	// Get pointer to readonly resource
	WCHAR* pBuf = nullptr;
	int len = LoadStringW(hInst, ID, reinterpret_cast<LPWSTR>(&pBuf), 0);

	// Type conversion
	if (len)
		return wstring(pBuf, len);
	else
		return wstring();
}

// Load sequential bitmaps from resources and converts to binary matrix
vector<Mat> loadTemplates(UINT startId, UINT templatesCount)
{
	vector<Mat> templates(templatesCount);

	// Resources must have sequential identifiers
	for (UINT i = 0; i < templatesCount; i++)
	{
		Bitmap* pBmp = Bitmap::FromResource(hInst, MAKEINTRESOURCE(startId + i));
		templates[i] = getGrayscale(*pBmp); // Convert to grayscale
		threshold(templates[i], 175); // Binarize image
		delete pBmp;
	}
	return templates;
}

// Load words from utf8 file and sorts them in lexicographic order
vector<wstring> loadDictionary(wstring fileName)
{
	// Create stream and set conversion
	wifstream ifs(fileName);
	ifs.imbue(locale(locale::empty(), new codecvt_utf8<wchar_t>));

	// Read file
	vector<wstring> dict;
	wstring word;
	while (ifs >> word)
		dict.push_back(word);
	ifs.close();

	// Sort strings
	sort(dict.begin(), dict.end());

	return dict;
}

// Writes dictionary to file in utf8
void rewriteDictionary(const vector<wstring>& dict, wstring fileName)
{
	// Create stream and set conversion
	wofstream ofs(fileName);
	ofs.imbue(locale(locale::empty(), new codecvt_utf8<wchar_t>));

	for (auto& word : dict)
		ofs << word << endl;
	ofs.close();
}

// Set default image
void setDefaultImage()
{
	// Load default image
	Bitmap* pBmp = Bitmap::FromResource(hInst,
		MAKEINTRESOURCE(IDB_BITMAP1 + templates.size()));

	// Get handler
	HBITMAP hBmp;
	if (pBmp->GetHBITMAP(0, &hBmp) == Ok) // Extract handle
		SendMessageW(hPctBox, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp); // Set image
	delete pBmp; // Free memory
}

// Set default text and font
void setDefaultText()
{
	// Set font
	HFONT hFont = CreateFontW(20, 0, 0, 0,
		FW_NORMAL, FALSE, FALSE, FALSE, RUSSIAN_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		VARIABLE_PITCH, L"Arial");
	SendMessageW(hTxtFld, WM_SETFONT, (WPARAM)hFont, (LPARAM)FALSE);
	DeleteObject(hFont); // Free memory

	// Set text
	WCHAR defaultText[] = L"Извлечённый текст будет отображён здесь...";
	SendMessageW(hTxtFld, WM_SETTEXT, 0, (LPARAM)defaultText);
}
