#include "pch.h"
#include "MainWindow.h"
#include "resource.h"

int MainWindow::x_size = 15;
int MainWindow::y_size = 15;
int MainWindow::mine_count = 40;

MainWindow::MainWindow() : hwnd(nullptr), minesweeper(x_size, y_size, mine_count) {}

void MainWindow::InitializeWindow(HINSTANCE instance, INT cmd_show) {
    minesweeper.CreateDeviceIndependentResources();

    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWindow::WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = instance;
    wcex.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON));
    wcex.hCursor = LoadCursor(nullptr, IDI_APPLICATION);
    wcex.lpszClassName = L"Minesweeper";

    winrt::check_bool(RegisterClassEx(&wcex));

    hwnd = CreateWindowEx(
        0,                      // Optional window styles
        L"Minesweeper",         // Window class
        L"Minesweeper",         // Window text
        WS_OVERLAPPEDWINDOW,    // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        nullptr,    // Parent window    
        nullptr,    // Menu
        instance,   // Instance handle
        this        // Additional application data
    );

    winrt::check_pointer(hwnd);

    minesweeper.CreateDeviceDependentResources();
    minesweeper.CreateWindowSizeDependentResources(hwnd);

    ShowWindow(hwnd, cmd_show);
}

void MainWindow::RunMessageLoop() {
    MSG msg;

    do {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message != WM_QUIT) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else {
            if (minesweeper.IsAnimationOngoing()) {
                minesweeper.OnRender(hwnd);
            }
        }
    } while (msg.message != WM_QUIT);
}

LRESULT MainWindow::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    if (message == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        MainWindow* main_window = (MainWindow*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(main_window)
        );

        result = 1;
    }
    else {
        MainWindow* main_window = reinterpret_cast<MainWindow*>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
            )));

        bool wasHandled = false;

        if (main_window) {
            switch (message) {
            case WM_SIZE:
            {
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                main_window->minesweeper.OnResize(width, height, hwnd);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_DISPLAYCHANGE:
            {
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_PAINT:
            {
                main_window->minesweeper.OnRender(hwnd);
                ValidateRect(hwnd, nullptr);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_DESTROY:
            {
                PostQuitMessage(0);
            }
            result = 1;
            wasHandled = true;
            break;

            case WM_MOUSEMOVE:
            {
                main_window->minesweeper.OnMouseMove(hwnd, D2D1::Point2L(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_LBUTTONDOWN:
            {
                main_window->minesweeper.OnLeftButtonClick(hwnd);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_RBUTTONDOWN:
            {
                main_window->minesweeper.OnRightButtonClick(hwnd);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_KEYDOWN:
            {
                if (wParam == 'R') {
                    main_window->minesweeper.InitializeGame(x_size, y_size, mine_count);
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                else if (wParam == 'C') {
                    if (DialogBox(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDD_DIALOG1), hwnd, SizeDialogProc) == IDOK) {
                        main_window->minesweeper.InitializeGame(x_size, y_size, mine_count);
                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                }
            }
            result = 0;
            wasHandled = true;
            break;

            }
        }

        if (!wasHandled) {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}

INT_PTR MainWindow::SizeDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        SetDlgItemInt(hwnd, IDC_EDITX, x_size, FALSE);
        SetDlgItemInt(hwnd, IDC_EDITY, y_size, FALSE);
        SetDlgItemInt(hwnd, IDC_EDITM, mine_count, FALSE);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        {
            int tmp_x_size = GetDlgItemInt(hwnd, IDC_EDITX, nullptr, FALSE);
            int tmp_y_size = GetDlgItemInt(hwnd, IDC_EDITY, nullptr, FALSE);
            int tmp_mine_count = GetDlgItemInt(hwnd, IDC_EDITM, nullptr, FALSE);

            if (tmp_x_size == 0 || tmp_y_size == 0) {
                MessageBox(hwnd, L"Board dimensions must be nonzero numbers!", L"Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }
            if (tmp_mine_count > tmp_x_size * tmp_y_size) {
                MessageBox(hwnd, L"Mine count is higher than the number of fields!", L"Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            x_size = tmp_x_size;
            y_size = tmp_y_size;
            mine_count = tmp_mine_count;
        }
        case IDCANCEL:
            EndDialog(hwnd, wParam);
            return TRUE;
        }
    }

    return FALSE;
}
