#pragma once

#include "Field.h"
#include "BitmapDefinition.h"
#include "GameState.h"
#include "Timer.h"
#include "AnimationState.h"

class Minesweeper {
public:
    Minesweeper();

    void InitializeGame(int x_size, int y_size, int mines_count);
    void CreateDeviceDependentResources();
    void CreateDeviceIndependentResources();
    void CreateWindowSizeDependentResources(HWND hwnd);

    void OnRender(HWND hwnd);
    void OnResize(UINT width, UINT height, HWND hwnd);
    void OnMouseMove(HWND hwnd, const D2D1_POINT_2L& mouse_pos);
    void OnLeftButtonClick(HWND hwnd);
    void OnRightButtonClick(HWND hwnd);
private:
    // Direct3D objects.
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

    D2D1::Matrix3x2F transformation;
    D2D1_POINT_2F center;
    FLOAT dpi;
    int hovered_x, hovered_y;
    std::vector<std::vector<Field>> fields;
    GameState game_state;

    std::size_t GetBoardXSize();
    std::size_t GetBoardYSize();
    bool IsValidFieldHovered();

    void DisableFieldGradient();
    void GenerateBoard();
    void RevealNeighbours();
    void RebuildBoardBitmap();
    void RenderTexts();
    void RenderBoard();
    void RenderFields();
    void HandleDeviceLost(HWND hwnd);

    // Consts
    static constexpr int FIELD_SIZE = 40;
    static constexpr D2D1_COLOR_F background_color = { .r = 0.41f, .g = 0.41f, .b = 0.41f, .a = 1.0f };
    static constexpr D2D1_COLOR_F font_color = { .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f };

    static constexpr D2D1_GRADIENT_STOP even_rad_stops_data[] = {
        {.position = 0.0f, .color = {.r = 0.75f, .g = 0.88f, .b = 0.49f, .a = 1.0f } },
        {.position = 0.5f, .color = {.r = 0.75f, .g = 0.88f, .b = 0.49f, .a = 1.0f } },
        {.position = 1.0f, .color = {.r = 0.53f, .g = 0.81f, .b = 0.98f, .a = 1.0f } }
    };
    static constexpr D2D1_GRADIENT_STOP odd_rad_stops_data[] = {
        {.position = 0.0f, .color = {.r = 0.65f, .g = 0.78f, .b = 0.4f, .a = 1.0f } },
        {.position = 0.5f, .color = {.r = 0.65f, .g = 0.78f, .b = 0.4f, .a = 1.0f } },
        {.position = 1.0f, .color = {.r = 0.69f, .g = 0.88f, .b = 0.9f, .a = 1.0f } }
    };
};