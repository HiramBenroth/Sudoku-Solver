#include "framework.h"
#include "file_manager.h"
#include "Sudoku Solver.h"
#include "board.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

void delay(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

#define MAX_LOADSTRING 100
#define ID_BUTTON_LOAD 101
#define ID_BUTTON_STEP 102
#define ID_BUTTON_RUN  103
#define ID_EDIT_SPEED  104

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
board* puzzle = nullptr; 
int changes = 0;                                // Tracks number of changes (pencil marks or candidates set)
int step = 0;                                   // Tracks next step type (e.g., Naked Single, Hidden Single)
bool is_running = false;                        // Tracks if the solver is running or paused
int timer_interval = 500;                       // Timer interval in milliseconds (default 500ms)

// Forward declarations
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Using's
using std::vector;
using std::string;

// This is for the board, to update it when necessary.
static vector<string> board_data(9);  // Holds the current board state (rows of numbers)
static bool board_loaded = false;     // Flag to indicate if board data is loaded

// Helper: convert wide string to UTF-8 narrow string
static std::string narrow(const std::wstring& w) {
    if (w.empty()) return {};
    int size_needed = WideCharToMultiByte(
        CP_UTF8, 0,
        w.data(), (int)w.size(),
        nullptr, 0,
        nullptr, nullptr
    );
    std::string s(size_needed, '\0');
    WideCharToMultiByte(
        CP_UTF8, 0,
        w.data(), (int)w.size(),
        &s[0], size_needed,
        nullptr, nullptr
    );
    return s;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SUDOKUSOLVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SUDOKUSOLVER));
    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

// Registers the window class
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SUDOKUSOLVER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SUDOKUSOLVER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// Saves instance handle and creates main window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;
    HWND hWnd = CreateWindowW(
        szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 600, 650, // Increased height to accommodate text box
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Create buttons
    CreateWindowW(L"BUTTON", L"Load", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        460, 50, 100, 40, hWnd, (HMENU)ID_BUTTON_LOAD, hInstance, nullptr);
    CreateWindowW(L"BUTTON", L"Step", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        460, 110, 100, 40, hWnd, (HMENU)ID_BUTTON_STEP, hInstance, nullptr);
    CreateWindowW(L"BUTTON", L"Run", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        460, 170, 100, 40, hWnd, (HMENU)ID_BUTTON_RUN, hInstance, nullptr);
    // Create text box for timer speed
    CreateWindowW(L"EDIT", L"500", WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
        460, 210, 100, 30, hWnd, (HMENU)ID_EDIT_SPEED, hInstance, nullptr);

    return TRUE;
}

// Main window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case ID_BUTTON_LOAD: {
            file_manager fm;
            std::string boardData[9];
            if (fm.openDialog(hWnd)) {
                for (int i = 0; i < 9; i++) {
                    std::wstring wline = fm.readLine();
                    if (wline.empty()) {
                        MessageBox(hWnd, L"File has fewer than 9 lines", L"Error", MB_OK | MB_ICONERROR);
                        return 0;
                    }
                    std::string line = narrow(wline);
                    boardData[i] = line;
                }
            }
            else {
                MessageBox(hWnd, L"Failed to open file", L"Error", MB_OK | MB_ICONERROR);
                return 0;
            }
            if (puzzle != nullptr) {
                delete puzzle;
                puzzle = nullptr;
            }
            try {
                puzzle = new board(boardData);
                board_data = puzzle->getBoard();
                board_loaded = true;
                changes = 0; // Reset changes
                step = 1;    // Reset step
                is_running = false; // Ensure solver is paused
                timer_interval = 500; // Reset timer interval
                SetWindowTextW(GetDlgItem(hWnd, ID_BUTTON_RUN), L"Run"); // Reset button text
                SetWindowTextW(GetDlgItem(hWnd, ID_EDIT_SPEED), L"500"); // Reset speed text box
                InvalidateRect(hWnd, nullptr, TRUE);
            }
            catch (const std::exception& e) {
                MessageBoxA(hWnd, e.what(), "Error", MB_OK | MB_ICONERROR);
                puzzle = nullptr;
                board_loaded = false;
            }
            break;
        }
        case ID_BUTTON_STEP:
            if (puzzle != nullptr) {
                try {
                    vector<int> step_changes = puzzle->step();
                    board_data = puzzle->getBoard();
                    changes = step_changes[0]; // Update global changes
                    step = step_changes[1];    // Update global step
                    InvalidateRect(hWnd, nullptr, TRUE);
                }
                catch (const std::exception& e) {
                    MessageBoxA(hWnd, e.what(), "Error", MB_OK | MB_ICONERROR);
                }
            }
            break;
        case ID_BUTTON_RUN:
            if (puzzle != nullptr && !puzzle->checkSolution()) {
                is_running = !is_running; // Toggle running state
                if (is_running) {
                    // Read and validate timer interval from text box
                    WCHAR buffer[32];
                    GetWindowTextW(GetDlgItem(hWnd, ID_EDIT_SPEED), buffer, 32);
                    int input_interval = _wtoi(buffer);
                    if (input_interval > 0) {
                        timer_interval = input_interval;
                    } else {
                        timer_interval = 500; // Default if invalid
                        SetWindowTextW(GetDlgItem(hWnd, ID_EDIT_SPEED), L"500");
                    }
                    // Start the timer with specified interval
                    SetTimer(hWnd, 1, timer_interval, nullptr);
                    SetWindowTextW(GetDlgItem(hWnd, ID_BUTTON_RUN), L"Pause"); // Update button text
                } else {
                    // Stop the timer
                    KillTimer(hWnd, 1);
                    SetWindowTextW(GetDlgItem(hWnd, ID_BUTTON_RUN), L"Run"); // Update button text
                }
            }
            break;
        case IDM_EXIT:
            if (puzzle != nullptr) {
                delete puzzle;
                puzzle = nullptr;
            }
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_TIMER:
        if (puzzle != nullptr && !puzzle->checkSolution() && is_running) {
            try {
                vector<int> step_changes = puzzle->step();
                board_data = puzzle->getBoard();
                changes = step_changes[0]; // Update global changes
                step = step_changes[1];    // Update global step
                InvalidateRect(hWnd, nullptr, TRUE);
                UpdateWindow(hWnd); // Force immediate repaint
            }
            catch (const std::exception& e) {
                KillTimer(hWnd, 1); // Stop the timer on error
                is_running = false; // Set to paused
                SetWindowTextW(GetDlgItem(hWnd, ID_BUTTON_RUN), L"Run"); // Reset button text
                MessageBoxA(hWnd, e.what(), "Error", MB_OK | MB_ICONERROR);
            }
        }
        else {
            KillTimer(hWnd, 1); // Stop the timer when solved or paused
            is_running = false; // Set to paused
            SetWindowTextW(GetDlgItem(hWnd, ID_BUTTON_RUN), L"Run"); // Reset button text
            if (puzzle != nullptr && puzzle->checkSolution()) {
                MessageBox(hWnd, L"Puzzle was solved.", L"SOLVED", MB_OK | MB_ICONINFORMATION);
            }
        }
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Draw welcome message
        LPCWSTR msg = L"Welcome to Sudoku Solver!";
        RECT textRect = { 0, 0, 600, 30 };
        DrawTextW(hdc, msg, -1, &textRect, DT_CENTER | DT_TOP | DT_SINGLELINE);

        // Grid constants
        int xOffset = 20, yOffset = 50; // Adjusted yOffset to make room for title
        int cellSize = 40, gridSize = 9;

        // Draw grid lines
        for (int i = 0; i <= gridSize; ++i) {
            HPEN pen = CreatePen(PS_SOLID, (i % 3 == 0) ? 3 : 1, RGB(0, 0, 0));
            HPEN old = (HPEN)SelectObject(hdc, pen);
            // Vertical lines
            MoveToEx(hdc, xOffset + i * cellSize, yOffset, NULL);
            LineTo(hdc, xOffset + i * cellSize, yOffset + gridSize * cellSize);
            // Horizontal lines
            MoveToEx(hdc, xOffset, yOffset + i * cellSize, NULL);
            LineTo(hdc, xOffset + gridSize * cellSize, yOffset + i * cellSize);
            SelectObject(hdc, old);
            DeleteObject(pen);
        }

        // Fonts for numbers
        HFONT bigFont = CreateFontW(
            24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI"
        );
        HFONT smallFont = CreateFontW(
            10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI"
        );
        HFONT textFont = CreateFontW(
            16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI"
        );
        HFONT oldFont = (HFONT)SelectObject(hdc, bigFont); // Default to big font

        // Draw numbers if loaded
        if (board_loaded) {
            for (int r = 0; r < 9; ++r) {
                const string& row = board_data[r];
                int c = 0;
                for (size_t i = 0; i < row.length() && c < 9;) {
                    if (row[i] == '{') {
                        ++i;
                        std::string cellText;
                        while (i < row.length() && row[i] != '}') {
                            if (isdigit(row[i])) {
                                cellText += row[i];
                            }
                            ++i;
                        }
                        ++i; // Skip '}'

                        // Calculate cell position
                        int x = xOffset + c * cellSize;
                        int y = yOffset + r * cellSize;

                        if (cellText.length() == 1) {
                            // Single number (given or last candidate): draw centered with big font
                            std::wstring wcellText(cellText.begin(), cellText.end());
                            RECT cellRect = { x, y, x + cellSize, y + cellSize };
                            SelectObject(hdc, bigFont); // Ensure big font is selected
                            DrawTextW(hdc, wcellText.c_str(), -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        }
                        else if (cellText.length() > 1) {
                            // Multiple candidates: draw in 3x3 keypad layout with small font
                            SelectObject(hdc, smallFont);
                            // Define 3x3 subgrid positions (13x13 subcells, adjusted to fit)
                            struct Subcell { int left, top, right, bottom; };
                            Subcell subcells[9] = {
                                { x + 2,  y + 2,  x + 15, y + 15 }, // 1: Top-left
                                { x + 15, y + 2,  x + 28, y + 15 }, // 2: Top-center
                                { x + 28, y + 2,  x + 38, y + 15 }, // 3: Top-right
                                { x + 2,  y + 15, x + 15, y + 28 }, // 4: Middle-left
                                { x + 15, y + 15, x + 28, y + 28 }, // 5: Middle-center
                                { x + 28, y + 15, x + 38, y + 28 }, // 6: Middle-right
                                { x + 2,  y + 28, x + 15, y + 38 }, // 7: Bottom-left
                                { x + 15, y + 28, x + 28, y + 38 }, // 8: Bottom-center
                                { x + 28, y + 28, x + 38, y + 38 }  // 9: Bottom-right
                            };

                            // Draw each candidate in its corresponding subcell
                            for (char digit : cellText) {
                                int num = digit - '0'; // Convert '1' to 1, etc.
                                if (num >= 1 && num <= 9) {
                                    int index = num - 1; // Map 1->0, 2->1, ..., 9->8
                                    std::wstring candidate(1, digit);
                                    RECT subcellRect = {
                                        subcells[index].left,
                                        subcells[index].top,
                                        subcells[index].right,
                                        subcells[index].bottom
                                    };
                                    DrawTextW(hdc, candidate.c_str(), -1, &subcellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                                }
                            }
                        }
                        ++c;
                    }
                    else {
                        ++i;
                    }
                }
            }
        }

        // Draw changes and step information below the board
        SelectObject(hdc, textFont);
        int infoY = yOffset + gridSize * cellSize + 20; // Position below the grid
        std::wstring changesText = L"Changes Made: " + std::to_wstring(changes);
        std::wstring stepText = L"Next Step Solve: ";
        if (step == 0) {
            stepText += L"None";
        } else if (step == 1) {
            stepText += L"Naked Single";
        } else if (step == 2) {
            stepText += L"Hidden Single";
        } else {
            stepText += L"Pointing Pair";
        }

        RECT changesRect = { xOffset, infoY, xOffset + 360, infoY + 20 };
        RECT stepRect = { xOffset, infoY + 20, xOffset + 360, infoY + 40 };
        DrawTextW(hdc, changesText.c_str(), -1, &changesRect, DT_LEFT | DT_TOP | DT_SINGLELINE);
        DrawTextW(hdc, stepText.c_str(), -1, &stepRect, DT_LEFT | DT_TOP | DT_SINGLELINE);

        SelectObject(hdc, oldFont);
        DeleteObject(bigFont);
        DeleteObject(smallFont);
        DeleteObject(textFont);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        if (puzzle != nullptr) {
            delete puzzle;
            puzzle = nullptr;
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// About dialog
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}