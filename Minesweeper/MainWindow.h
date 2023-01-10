#pragma once

#include "Minesweeper.h"

class MainWindow {
public:
    MainWindow();
    void InitializeWindow(HINSTANCE instance, INT cmd_show);
    void RunMessageLoop();
private:
    HWND hwnd;
    Minesweeper minesweeper;
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK SizeDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    static int x_size;
    static int y_size;
    static int mine_count;
};
