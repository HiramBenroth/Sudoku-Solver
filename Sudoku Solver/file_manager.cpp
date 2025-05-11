// file_manager.cpp
#include "file_manager.h"

file_manager::file_manager() {
    // Initialize buffer with zeros
    ZeroMemory(fileBuffer, sizeof(fileBuffer));
}

file_manager::~file_manager() {
    close();
}

bool file_manager::openDialog(HWND owner) {
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFile = fileBuffer;
    ofn.nMaxFile = BufSize;
    ofn.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = L"Select a file to open";
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = L"txt";

    // Show the Unicode dialog
    if (GetOpenFileNameW(&ofn) == TRUE) {
        filename = fileBuffer;  // assign WCHAR buffer to std::wstring
        file.open(filename);    // open as wide-char wifstream
        if (file.is_open()) {
            std::wcout << L"Opened file: " << filename << std::endl;
            return true;
        }
        else {
            std::wcerr << L"Error: Could not open file '" << filename << L"'" << std::endl;
        }
    }
    else {
        DWORD err = CommDlgExtendedError();
        if (err != 0) {
            std::wcerr << L"GetOpenFileNameW failed, error code: " << err << std::endl;
        }
    }
    return false;
}

std::wstring file_manager::readLine() {
    std::wstring line;
    if (file.is_open() && std::getline(file, line)) {
        return line;
    }
    return std::wstring();
}

bool file_manager::eof() const {
    return !file.is_open() || file.eof();
}

void file_manager::close() {
    if (file.is_open()) {
        file.close();
    }
}
