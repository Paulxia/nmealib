// upkit.cpp : main source file for upkit.exe
//

#include "stdafx.h"

#include "resource.h"

#include "MainDlg.h"

CAppModule _Module;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// Calling AtlInitCommonControls is not necessary to utilize picture,
	// static text, edit box, group box, button, check box, radio button, 
	// combo box, list box, or the horizontal and vertical scroll bars.
	// Calling AtlInitCommonControls with 0 is required to utilize the spin, 
	// progress, slider, list, tree, and tab controls.
	// Adding the ICC_DATE_CLASSES flag is required to initialize the 
	// date time picker and month calendar controls.
	// Add additional flags to support additoinal controls not mentioned above.
	AtlInitCommonControls(ICC_DATE_CLASSES);

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = 0;
	// BLOCK: Run application
	{
		CMainDlg dlgMain;
		nRet = dlgMain.DoModal();
	}

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
