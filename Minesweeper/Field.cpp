#include "pch.h"
#include "Field.h"

Field::Field(bool is_even, int neighbour_mines, D2D1_RECT_F rectangle) : is_even(is_even),
    neighbour_mines(neighbour_mines), rectangle(rectangle), field_state(FieldState::Undiscovered), should_use_gradient(false) {}

bool Field::HandleLeftClick(GameState& game_state) {
    if (field_state != FieldState::Undiscovered) {
        return false;
    }

    field_state = FieldState::Discovered;
    game_state.OnFieldDiscover(IsMined());

    if (!IsMined()) {
        should_use_gradient = true;
    }

    return true;
}

void Field::HandleRightClick(GameState& game_state) {
    switch (field_state) {
    case FieldState::Undiscovered:
        field_state = FieldState::Flagged;
        game_state.OnFieldFlag();
        break;
    case FieldState::Flagged:
        field_state = FieldState::Unknown;
        game_state.OnFieldUnflag();
        break;
    case FieldState::Unknown:
        field_state = FieldState::Undiscovered;
        break;
    default:
        break;
    }
}

bool Field::IsMined() {
    return neighbour_mines == -1;
}

bool Field::HasNeighboursWithMines() {
    return neighbour_mines != 0;
}

void Field::ResetGradient() {
    should_use_gradient = false;
}

void Field::RenderField(bool hover, ID2D1DeviceContext6* device_context, ID2D1Bitmap1* bitmap, ID2D1SolidColorBrush* brush,
    ID2D1RadialGradientBrush* even_gradient_brush, ID2D1RadialGradientBrush* odd_gradient_brush) {
    if (should_use_gradient) {
        device_context->FillRectangle(rectangle, is_even ? even_gradient_brush : odd_gradient_brush);
    }
    else {
        if (field_state == FieldState::Discovered) {
            brush->SetColor(is_even ? even_field_discovered_color : odd_field_discovered_color);
        }
        else if (hover) {
            brush->SetColor(is_even ? even_field_hover_color : odd_field_hover_color);
        }
        else {
            brush->SetColor(is_even ? even_field_color : odd_field_color);
        }
        device_context->FillRectangle(rectangle, brush);
    }

    if (field_state == FieldState::Discovered) {
        if (IsMined()) {
            device_context->DrawBitmap(bitmap, rectangle, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, mine_src_rect);
        }
        else if (HasNeighboursWithMines()) {
            auto digit_src_rect = D2D1::RectF(
                (FIRST_DIGIT_FIELD + neighbour_mines - 1) * BITMAP_FIELD_SIZE,
                0.0f,
                (FIRST_DIGIT_FIELD + neighbour_mines) * BITMAP_FIELD_SIZE - 1.0f,
                BITMAP_FIELD_SIZE - 1.0f);
            device_context->DrawBitmap(bitmap, rectangle, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, digit_src_rect);
        }
    }
    else if (field_state == FieldState::Flagged) {
        device_context->DrawBitmap(bitmap, rectangle, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, flag_src_rect);
    }
    else if (field_state == FieldState::Unknown) {
        device_context->DrawBitmap(bitmap, rectangle, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, unknown_src_rect);
    }
}
