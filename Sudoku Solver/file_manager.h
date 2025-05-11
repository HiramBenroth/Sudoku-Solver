#pragma once

#include <windows.h>
#include <commdlg.h>
#include <fstream>
#include <string>
#include <iostream>

class file_manager {
private:
    static const DWORD BufSize = MAX_PATH;
    WCHAR fileBuffer[BufSize];          // wide-char buffer for Unicode paths
    std::wifstream file;               // wide-char file stream
    std::wstring filename;             // stores the selected path

public:
    file_manager();
    ~file_manager();

    // Opens a Unicode file-open dialog; returns true if a file was selected and opened
    bool openDialog(HWND owner = nullptr);

    // Reads a wide string line from the opened file; returns empty wstring at EOF or error
    std::wstring readLine();

    // Returns true if end-of-file has been reached or if file not open
    bool eof() const;

    // Closes the file stream if open
    void close();
};
