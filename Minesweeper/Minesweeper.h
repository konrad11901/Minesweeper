#pragma once

#include "Field.h"
#include "BitmapDefinition.h"
#include "GameState.h"
#include "Timer.h"
#include "AnimationState.h"

class Minesweeper {
public:
    Minesweeper(int x_size, int y_size, int mine_count);

    void InitializeGame(int x_size, int y_size, int mine_count);
    void CreateDeviceDependentResources();
    void CreateDeviceIndependentResources();
    void CreateWindowSizeDependentResources(HWND hwnd);

    void OnRender(HWND hwnd);
    void OnResize(UINT width, UINT height, HWND hwnd);
    void OnMouseMove(HWND hwnd, const D2D1_POINT_2L& mouse_pos);
    void OnLeftButtonClick(HWND hwnd);
    void OnRightButtonClick(HWND hwnd);

    bool IsAnimationOngoing();
private:
    // Direct3D objects.
    // Since this game uses Direct2D effects, we need to initialize Direct2D device context
    // instead of render target. To initialize Direct2D device context, we need a Direct3D device
    // and DXGI swap chain, and we need to render to a bitmap.
    // Source: https://learn.microsoft.com/en-us/windows/win32/direct2d/devices-and-device-contexts
    // Alternate source: https://katyscode.wordpress.com/2013/01/23/migrating-existing-direct2d-applications-to-use-direct2d-1-1-functionality-in-windows-7/
    winrt::com_ptr<ID3D11Device5> d3d_device;
    winrt::com_ptr<ID3D11DeviceContext4> d3d_context;
    winrt::com_ptr<IDXGISwapChain4> swap_chain;

    // Direct2D objects.
    winrt::com_ptr<ID2D1Factory7> d2d_factory;
    winrt::com_ptr<ID2D1Device6> d2d_device;
    winrt::com_ptr<ID2D1DeviceContext6> d2d_context;
    winrt::com_ptr<ID2D1Bitmap1> d2d_target_bitmap;
    winrt::com_ptr<ID2D1Bitmap1> board_bitmap;
    winrt::com_ptr<ID2D1Effect> perspective_transform_effect;
    winrt::com_ptr<ID2D1Effect> color_matrix_effect;
    winrt::com_ptr<ID2D1SolidColorBrush> main_brush;
    winrt::com_ptr<ID2D1RadialGradientBrush> even_gradient_brush, odd_gradient_brush;

    // DirectWrite objects.
    winrt::com_ptr<IDWriteFactory7> write_factory;
    winrt::com_ptr<IDWriteTextFormat3> text_format;

    // WIC objects.
    winrt::com_ptr<IWICImagingFactory2> imaging_factory;
    BitmapDefinition field_bitmap_def;

    // Animation-related objects.
    Timer entry_animation_timer;
    AnimationState entry_animation_state;
    Timer discover_animation_timer;
    AnimationState discover_animation_state;
    Timer mine_animation_timer;
    AnimationState mine_animation_state;

    D2D1::Matrix3x2F transformation;
    D2D1_POINT_2F center;
    FLOAT dpi;
    int hovered_x, hovered_y;
    std::vector<std::vector<Field>> fields;
    GameState game_state;

    std::size_t GetBoardXSize();
    std::size_t GetBoardYSize();
    bool IsValidFieldHovered();
    bool ShouldRegenerateMine(std::size_t mine_x, std::size_t mine_y);

    void DisableFieldGradient();
    void GenerateBoard();
    void RevealNeighbours();
    void RebuildBoardBitmap();
    void RenderTexts();
    void RenderBoard();
    void RenderBoardWithEntryAnimation(double time);
    void RenderBoardWithGrayscaleAnimation(double time);
    void RenderFields();
    void HandleDeviceLost(HWND hwnd);

    D2D1_MATRIX_5X4_F GetMatrixForGrayscaleAnimation(double time);

    // Consts
    static constexpr int FIELD_SIZE = 40;
    static constexpr D2D1_COLOR_F BACKGROUND_COLOR = { .r = 0.41f, .g = 0.41f, .b = 0.41f, .a = 1.0f };
    static constexpr D2D1_COLOR_F FONT_COLOR = { .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f };

    static constexpr D2D1_GRADIENT_STOP EVEN_RAD_STOPS_DATA[] = {
        {.position = 0.0f, .color = {.r = 0.75f, .g = 0.88f, .b = 0.49f, .a = 1.0f } },
        {.position = 0.5f, .color = {.r = 0.75f, .g = 0.88f, .b = 0.49f, .a = 1.0f } },
        {.position = 1.0f, .color = {.r = 0.53f, .g = 0.81f, .b = 0.98f, .a = 1.0f } }
    };
    static constexpr D2D1_GRADIENT_STOP ODD_RAD_STOPS_DATA[] = {
        {.position = 0.0f, .color = {.r = 0.65f, .g = 0.78f, .b = 0.4f, .a = 1.0f } },
        {.position = 0.5f, .color = {.r = 0.65f, .g = 0.78f, .b = 0.4f, .a = 1.0f } },
        {.position = 1.0f, .color = {.r = 0.69f, .g = 0.88f, .b = 0.9f, .a = 1.0f } }
    };
    
    static constexpr PCWSTR FIELD_BITMAP_PATH = L"Assets\\Field.png";
    static constexpr const WCHAR* FONT_COLLECTION = L"Times New Roman";
    static constexpr const WCHAR* LOCALE_NAME = L"en-us";

    static constexpr double ENTRY_ANIMATION_TIME = 500.0;
    static constexpr double REVEAL_ANIMATION_TIME = 1000.0;
    static constexpr double MINE_ANIMATION_TIME = 2000.0;

    static constexpr const WCHAR* REMAINING_MINES_TEXT = L"Remaining mines: ";
    static constexpr const WCHAR* WIN_TEXT = L"You won!";
    static constexpr const WCHAR* LOSE_TEXT = L"You lost!";
    static constexpr const WCHAR* RESTART_TEXT = L"Press R to restart";

    static constexpr FLOAT BASE_DPI = 96.0f;
    static constexpr FLOAT TEXT_RECT_HEIGHT = 100.0f;
    static constexpr FLOAT TEXT_RECT_OFFSET = 16.0f;
};