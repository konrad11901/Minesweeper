#pragma once

#include "GameState.h"

enum class FieldState {
    Undiscovered,
    Flagged,
    Unknown,
    Discovered
};

class Field {
public:
    Field(bool is_even, int neighbour_mines, D2D1_RECT_F rectangle);

    bool HandleLeftClick(GameState& game_state);
    void HandleRightClick(GameState& game_state);

    bool IsMined();
    bool HasNeighboursWithMines();

    void ResetGradient();

    void RenderField(bool hover, ID2D1DeviceContext6* device_context, ID2D1Bitmap1* bitmap, ID2D1SolidColorBrush* brush,
        ID2D1RadialGradientBrush* even_gradient_brush, ID2D1RadialGradientBrush* odd_gradient_brush);
private:
    bool is_even;
    int neighbour_mines; // -1 means that this field is mined
    FieldState field_state;
    D2D1_RECT_F rectangle;

    bool should_use_gradient;

    static constexpr D2D1_COLOR_F even_field_color = { .r = 0.53f, .g = 0.81f, .b = 0.98f, .a = 1.0f };
    static constexpr D2D1_COLOR_F odd_field_color = { .r = 0.69f, .g = 0.88f, .b = 0.9f, .a = 1.0f };
    static constexpr D2D1_COLOR_F even_field_hover_color = { .r = 0.53f, .g = 0.81f, .b = 0.98f, .a = 0.8f };
    static constexpr D2D1_COLOR_F odd_field_hover_color = { .r = 0.69f, .g = 0.88f, .b = 0.9f, .a = 0.8f };
    static constexpr D2D1_COLOR_F even_field_discovered_color = { .r = 0.75f, .g = 0.88f, .b = 0.49f, .a = 1.0f };
    static constexpr D2D1_COLOR_F odd_field_discovered_color = { .r = 0.65f, .g = 0.78f, .b = 0.4f, .a = 1.0f };

    static constexpr FLOAT BITMAP_FIELD_SIZE = 256.0f;
    static constexpr D2D1_RECT_F mine_src_rect = { .left = 0.0f, .top = 0.0f,
        .right = BITMAP_FIELD_SIZE - 1, .bottom = BITMAP_FIELD_SIZE - 1 };
    static constexpr D2D1_RECT_F flag_src_rect = { .left = BITMAP_FIELD_SIZE, .top = 0.0f,
        .right = BITMAP_FIELD_SIZE * 2 - 1, .bottom = BITMAP_FIELD_SIZE - 1 };
    static constexpr D2D1_RECT_F unknown_src_rect = { .left = BITMAP_FIELD_SIZE * 2, .top = 0.0f,
        .right = BITMAP_FIELD_SIZE * 3 - 1, .bottom = BITMAP_FIELD_SIZE - 1 };
    static constexpr int FIRST_DIGIT_FIELD = 3;
};