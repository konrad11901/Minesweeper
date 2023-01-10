#include "pch.h"
#include "MainWindow.h"

// Enable more modern visual styles
// https://xiongzh.com/2021/04/26/win32-enable-visual-styles/
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

INT WINAPI wWinMain(_In_ [[maybe_unused]] HINSTANCE instance,
    _In_opt_ [[maybe_unused]] HINSTANCE prev_instance,
    _In_ [[maybe_unused]] PWSTR cmd_line,
    _In_ [[maybe_unused]] INT cmd_show) {
    MainWindow main_window;

    main_window.InitializeWindow(instance, cmd_show);
    main_window.RunMessageLoop();

    return 0;
}
