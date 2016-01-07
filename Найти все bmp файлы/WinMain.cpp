// найти все bmp файлы в системе размером более 50х50 пикселей.
// загрузить пути к ним в вектор или список.

#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <io.h>

#include "resource.h"

#define MIN_SIZE_BMP_WIDTH 50 
#define MIN_SIZE_BMP_HEIGHT 50

BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
void SearchFiles(char* path);
void SearchLogicalDrive();

HWND hStart, hStop, hPicture;
std::vector<std::wstring> PachBmp;
HBITMAP *hBmp;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
}

BOOL CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const unsigned int MY_TIMER_1 = 10;

	switch (message)
	{
	case WM_CLOSE:
		delete[] hBmp;
		EndDialog(hWnd, 0);
		return TRUE;

	case WM_INITDIALOG:
	{
		hStart = GetDlgItem(hWnd, IDC_START);
		hStop = GetDlgItem(hWnd, IDC_STOP);
		hPicture = GetDlgItem(hWnd, IDC_PICTURE);

		LONG styles = GetWindowLong(hPicture, GWL_STYLE);
		SetWindowLong(hPicture, GWL_STYLE, styles | SS_BITMAP);

		if (IDCANCEL != MessageBox(0, TEXT("Подождите идет поиск..."), TEXT(""), MB_OKCANCEL | MB_ICONINFORMATION)){

			SearchLogicalDrive();
			//SearchFiles("D:");

			hBmp = new HBITMAP[PachBmp.size()];
			for (unsigned int i = 0; i < PachBmp.size(); i++){
				*(hBmp + i) = (HBITMAP)LoadImage(NULL, PachBmp[i].c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			}
		}
		return TRUE;
	}

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_START)
		{
			SetTimer(hWnd, MY_TIMER_1, 1000, 0);
			EnableWindow(hStart, 0);
			EnableWindow(hStop, 1);
			SetFocus(hStop);
		}
		else if (LOWORD(wParam) == IDC_STOP)
		{
			KillTimer(hWnd, MY_TIMER_1);
			EnableWindow(hStart, 1);
			EnableWindow(hStop, 0);
			SetFocus(hStart);
		}
		return TRUE;

	case WM_TIMER:
	{
		static int index = 0;
		index++;
		if (index == PachBmp.size())
			index = 0;
		SendMessage(hPicture, STM_SETIMAGE, WPARAM(IMAGE_BITMAP), LPARAM(hBmp[index]));
	}
		return TRUE;
	}
	return FALSE;
}

void SearchLogicalDrive() {

	char str[3];
	str[1] = ':';
	str[2] = '\0';
	int gld; // битовая маска логических дисков
	gld = GetLogicalDrives();
	for (int i = 0, md = 1; i < gld; ++i, md *= 2)
	{
		if (gld & md)
		{
			str[0] = i + 65;
			SearchFiles(str);
		}
	}
}

void SearchFiles(char* path) {
	char *find = new char[MAX_PATH];
	wchar_t *str = new wchar_t[MAX_PATH];

	strcpy_s(find, MAX_PATH, path);
	strcat_s(find, MAX_PATH, "\\*.*");

	_finddata_t *f = new _finddata_t;
	int handle = _findfirst(find, f);
	int result = handle;
	while (result != -1)
	{
		if (strcmp(f->name, ".") == 0 || strcmp(f->name, "..") == 0)
		{
			result = _findnext(handle, f);
			continue;
		}
		if (f->attrib & _A_SUBDIR)
		{
			strcpy_s(find, MAX_PATH, path);
			strcat_s(find, MAX_PATH, "\\");
			strcat_s(find, MAX_PATH, f->name);
			SearchFiles(find);
		}
		else {
			if (!_strnicmp(&(f->name[strlen(f->name) - 4]), ".bmp", 4)){
				char tmp[MAX_PATH];
				strcpy_s(tmp, MAX_PATH, path);
				strcat_s(tmp, MAX_PATH, "\\");
				strcat_s(tmp, MAX_PATH, f->name);
				std::ifstream input(tmp, std::ios::in | std::ios::binary);
				if (input){
					LONG biWidth, biHeight;
					input.seekg(sizeof(BITMAPFILEHEADER) + sizeof(DWORD));
					input.read((char*)&biWidth, sizeof(biWidth));
					input.read((char*)&biHeight, sizeof(biHeight));
					if (biWidth > MIN_SIZE_BMP_WIDTH && biHeight > MIN_SIZE_BMP_HEIGHT)
					 {
						MultiByteToWideChar(CP_ACP, 0, path, -1, str, MAX_PATH);
						PachBmp.push_back(str);
						PachBmp[PachBmp.size() - 1] += L"\\";
						MultiByteToWideChar(CP_ACP, 0, f->name, -1, str, MAX_PATH);
						PachBmp[PachBmp.size() - 1] += str;
					}
				}
				input.close();
			}
		}

		result = _findnext(handle, f);
	}
	_findclose(handle);
	delete[] find;
	delete f;
	delete[] str;
}