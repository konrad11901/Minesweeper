#include "pch.h"
#include "Minesweeper.h"
#include "RandomGenerator.h"

Minesweeper::Minesweeper() : hovered_x(-1), hovered_y(-1), center(), dpi(1.0f), transformation(), field_bitmap_def(L"Assets\\Field.png") {
    InitializeGame(15, 15, 40);
}

void Minesweeper::CreateDeviceIndependentResources() {
    D2D1_FACTORY_OPTIONS options{};

    // Initialize the Direct2D Factory.
    winrt::check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, d2d_factory.put()));

    winrt::check_hresult(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));

    // Initialize the Windows Imaging Component (WIC) Factory.
    winrt::check_hresult(CoCreateInstance(
        CLSID_WICImagingFactory2,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(imaging_factory.put())
    ));

    winrt::com_ptr<IDWriteTextFormat> format;
    winrt::check_hresult(DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory7),
        reinterpret_cast<IUnknown**>(write_factory.put())
    ));
    winrt::check_hresult(write_factory->CreateTextFormat(
        L"Times New Roman",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        50.0f,
        L"en-us",
        format.put()
    ));
    format.as(text_format);

    winrt::check_hresult(text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
    winrt::check_hresult(text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

    field_bitmap_def.CreateDeviceIndependentResources(imaging_factory.get());
}

void Minesweeper::CreateDeviceDependentResources() {
    // This flag adds support for surfaces with a different color channel ordering
    // than the API default. It is required for compatibility with Direct2D.
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    // This array defines the set of DirectX hardware feature levels this app will support.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11DeviceContext> context;

    auto hr = D3D11CreateDevice(
        nullptr,                    // Specify nullptr to use the default adapter.
        D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
        0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
        creationFlags,              // Set Direct2D compatibility flag.
        featureLevels,              // List of feature levels this app can support.
        ARRAYSIZE(featureLevels),   // Size of the list above.
        D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Runtime apps.
        device.put(),               // Returns the Direct3D device created.
        nullptr,                    // Don't store feature level of device created.
        context.put()               // Returns the device immediate context.
    );

    if (FAILED(hr))
    {
        // If the initialization fails, fall back to the WARP device.
        // For more information on WARP, see: 
        // http://go.microsoft.com/fwlink/?LinkId=286690
        winrt::check_hresult(D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
            0,
            creationFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            device.put(),
            nullptr,
            context.put()
        ));
    }

    // Store pointers to the Direct3D 11.1 API device and immediate context.
    device.as(d3d_device);
    context.as(d3d_context);

    // Create the Direct2D device object and a corresponding context.
    winrt::com_ptr<IDXGIDevice4> dxgi_device;
    d3d_device.as(dxgi_device);

    winrt::check_hresult(d2d_factory->CreateDevice(dxgi_device.get(), d2d_device.put()));
    winrt::check_hresult(d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2d_context.put()));

    field_bitmap_def.CreateDeviceDependentResources(d2d_context.get());

    winrt::check_hresult(d2d_context->CreateSolidColorBrush(background_color, main_brush.put()));

    winrt::check_hresult(d2d_context->CreateEffect(CLSID_D2D13DPerspectiveTransform, perspective_transform_effect.put()));
    winrt::check_hresult(d2d_context->CreateEffect(CLSID_D2D1ColorMatrix, color_matrix_effect.put()));

    winrt::com_ptr<ID2D1GradientStopCollection> even_rad_stops, odd_rad_stops;
    winrt::check_hresult(d2d_context->CreateGradientStopCollection(even_rad_stops_data, 3, even_rad_stops.put()));
    winrt::check_hresult(d2d_context->CreateRadialGradientBrush(
        D2D1::RadialGradientBrushProperties(D2D1::Point2F(0, 0), D2D1::Point2F(0, 0), 150, 150),
        even_rad_stops.get(), even_gradient_brush.put()));
    winrt::check_hresult(d2d_context->CreateGradientStopCollection(odd_rad_stops_data, 3, odd_rad_stops.put()));
    winrt::check_hresult(d2d_context->CreateRadialGradientBrush(
        D2D1::RadialGradientBrushProperties(D2D1::Point2F(0, 0), D2D1::Point2F(0, 0), 150, 150),
        odd_rad_stops.get(), odd_gradient_brush.put()));
}

void Minesweeper::CreateWindowSizeDependentResources(HWND hwnd) {
    using D2D1::Matrix3x2F;

    d2d_context->SetTarget(nullptr);
    d2d_target_bitmap = nullptr;

    if (swap_chain) {
        // If the swap chain already exists, resize it.
        auto hr = swap_chain->ResizeBuffers(
            2,
            0,
            0,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            0
        );
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            HandleDeviceLost(hwnd);

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else {
            winrt::check_hresult(hr);
        }
    }
    else {
        winrt::com_ptr<IDXGIDevice4> dxgi_device;
        d3d_device.as(dxgi_device);

        winrt::com_ptr<IDXGIAdapter> dxgi_adapter;
        winrt::check_hresult(dxgi_device->GetAdapter(dxgi_adapter.put()));

        winrt::com_ptr<IDXGIFactory7> dxgi_factory;
        winrt::check_hresult(dxgi_adapter->GetParent(IID_PPV_ARGS(dxgi_factory.put())));

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

        swapChainDesc.Width = 0;
        swapChainDesc.Height = 0;
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = 0;

        winrt::com_ptr<IDXGISwapChain1> swap_chain1;
        dxgi_factory->CreateSwapChainForHwnd(d3d_device.get(), hwnd, &swapChainDesc, nullptr, nullptr, swap_chain1.put());
        swap_chain1.as(swap_chain);
    }

    winrt::com_ptr<IDXGISurface2> dxgi_back_buffer;
    winrt::check_hresult(swap_chain->GetBuffer(0, IID_PPV_ARGS(dxgi_back_buffer.put())));

    // Get screen DPI
    dpi = (FLOAT)GetDpiForWindow(hwnd);

    // Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
    D2D1_BITMAP_PROPERTIES1 bitmapProperties =
        D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), dpi, dpi);

    d2d_context->CreateBitmapFromDxgiSurface(dxgi_back_buffer.get(), &bitmapProperties, d2d_target_bitmap.put());
    d2d_context->SetTarget(d2d_target_bitmap.get());
    d2d_context->SetDpi(dpi, dpi);

    center = D2D1::Point2F(d2d_context->GetSize().width / 2.0f, d2d_context->GetSize().height / 2.0f);

    RebuildBoardBitmap();
}

std::size_t Minesweeper::GetBoardXSize() {
    return fields.size();
}

std::size_t Minesweeper::GetBoardYSize() {
    return fields.empty() ? 0 : fields[0].size();
}

void Minesweeper::HandleDeviceLost(HWND hwnd) {
    swap_chain = nullptr;

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources(hwnd);
}

D2D1_MATRIX_5X4_F Minesweeper::GetMatrixForGrayscaleAnimation(double time) {
    float ratio = time < 2000.f ? time / 2000.0f : 1.0f;
    return D2D1::Matrix5x4F(
        1.0f - 0.701f * ratio, 0.299f * ratio, 0.299f * ratio, 0.0f,
        0.587f * ratio, 1.0f - 0.413f * ratio, 0.587f * ratio, 0.0f,
        0.114f * ratio, 0.114f * ratio, 1.0f - 0.886f * ratio, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f);
}

void Minesweeper::InitializeGame(int x_size, int y_size, int mines_count) {
    if (x_size <= 0 || y_size <= 0 || mines_count < 0) {
        return;
    }

    fields.clear();
    fields.reserve(x_size);
    FLOAT x_begin = x_size / 2.0f * -FIELD_SIZE;
    FLOAT y_begin = y_size / 2.0f * -FIELD_SIZE;

    for (int x = 0; x < x_size; x++) {
        std::vector<Field> fields_column;
        fields_column.reserve(y_size);
        for (int y = 0; y < y_size; y++) {
            auto rect = D2D1::RectF(
                x_begin + x * FIELD_SIZE,
                y_begin + y * FIELD_SIZE,
                x_begin + (x + 1) * FIELD_SIZE - 1.0f,
                y_begin + (y + 1) * FIELD_SIZE - 1.0f);
            fields_column.emplace_back(((x + y) % 2 == 0), 0, rect);
        }
        fields.push_back(fields_column);
    }

    game_state = GameState(mines_count, x_size * y_size);
    entry_animation_state = AnimationState::Scheduled;
    discover_animation_state = AnimationState::Ended;
    mine_animation_state = AnimationState::Ended;

    RebuildBoardBitmap();
}

void Minesweeper::DisableFieldGradient() {
    for (auto& fields_column : fields) {
        for (auto& field : fields_column) {
            field.ResetGradient();
        }
    }
}

void Minesweeper::GenerateBoard() {
    auto x_size = GetBoardXSize(), y_size = GetBoardYSize();
    std::vector<std::vector<int>> field_values(x_size, std::vector<int>(y_size));
    RandomGenerator random_generator;

    for (int mine = 0; mine < game_state.GetRemainingMines(); mine++) {
        std::size_t mine_x, mine_y;
        do {
            mine_x = random_generator.GetRandomNumber(0, static_cast<std::size_t>(x_size) - 1);
            mine_y = random_generator.GetRandomNumber(0, static_cast<std::size_t>(y_size) - 1);
        } while (field_values[mine_x][mine_y] < 0
            || (std::abs(hovered_x - static_cast<int>(mine_x)) <= 1
            && std::abs(hovered_y - static_cast<int>(mine_y)) <= 1));

        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                if (x == 0 && y == 0) {
                    field_values[mine_x][mine_y] = -1;
                }
                else {
                    if (mine_x + x < 0 || mine_x + x >= x_size || mine_y + y < 0 || mine_y + y >= y_size
                        || field_values[mine_x + x][mine_y + y] < 0) {
                        continue;
                    }
                    field_values[mine_x + x][mine_y + y]++;
                }
            }
        }
    }

    fields.clear();
    fields.reserve(x_size);
    FLOAT x_begin = x_size / 2.0f * -FIELD_SIZE;
    FLOAT y_begin = y_size / 2.0f * -FIELD_SIZE;

    for (int x = 0; x < x_size; x++) {
        std::vector<Field> fields_column;
        fields_column.reserve(y_size);
        for (int y = 0; y < y_size; y++) {
            auto rect = D2D1::RectF(
                x_begin + x * FIELD_SIZE,
                y_begin + y * FIELD_SIZE,
                x_begin + (x + 1) * FIELD_SIZE - 1.0f,
                y_begin + (y + 1) * FIELD_SIZE - 1.0f);
            fields_column.emplace_back(((x + y) % 2 == 0), field_values[x][y], rect);
        }
        fields.push_back(fields_column);
    }

    game_state.OnBoardGenerate();
}

void Minesweeper::RevealNeighbours() {
    std::queue<std::pair<std::size_t, std::size_t>> queue;
    std::vector<std::vector<bool>> visited(GetBoardXSize(), std::vector<bool>(GetBoardYSize()));

    visited[hovered_x][hovered_y] = true;
    queue.emplace(hovered_x, hovered_y);

    while (!queue.empty()) {
        auto field = queue.front();
        queue.pop();

        fields[field.first][field.second].HandleLeftClick(game_state);

        if (!fields[field.first][field.second].HasNeighboursWithMines()) {
            for (int x = -1; x <= 1; x++) {
                for (int y = -1; y <= 1; y++) {
                    if (field.first + x < 0 || field.first + x >= GetBoardXSize()
                        || field.second + y < 0 || field.second + y >= GetBoardYSize()) {
                        continue;
                    }
                    if (!visited[field.first + x][field.second + y]) {
                        visited[field.first + x][field.second + y] = true;
                        queue.emplace(field.first + x, field.second + y);
                    }
                }
            }
        }
    }
}

bool Minesweeper::IsValidFieldHovered() {
    return (hovered_x >= 0 && hovered_x < GetBoardXSize() && hovered_y >= 0 && hovered_y < GetBoardYSize());
}

void Minesweeper::RenderTexts() {
    main_brush->SetColor(font_color);

    std::wstring top_text;
    switch (game_state.GetGameStatus()) {
    case GameStatus::Init:
    case GameStatus::Ongoing:
        top_text = L"Remaining mines: ";
        top_text += std::to_wstring(game_state.GetRemainingMines());
        break;
    case GameStatus::Won:
        top_text = L"You won!";
        break;
    case GameStatus::Lost:
        top_text = L"You lost!";
        break;
    }

    d2d_context->DrawText(
        top_text.c_str(),
        static_cast<UINT32>(top_text.length()),
        text_format.get(),
        D2D1::RectF(0.0f, 16.0f, d2d_context->GetSize().width, 100.0f),
        main_brush.get());

    if (game_state.GetGameStatus() == GameStatus::Won || game_state.GetGameStatus() == GameStatus::Lost) {
        std::wstring bottom_text(L"Press R to restart");

        d2d_context->DrawText(
            bottom_text.c_str(),
            static_cast<UINT32>(bottom_text.length()),
            text_format.get(),
            D2D1::RectF(
                0.0f,
                d2d_context->GetSize().height - 116.0f,
                d2d_context->GetSize().width,
                d2d_context->GetSize().height - 16.0f),
            main_brush.get());
    }
}

void Minesweeper::RenderBoard() {
    d2d_context->SetTarget(d2d_target_bitmap.get());
    d2d_context->BeginDraw();
    d2d_context->Clear(background_color);
    d2d_context->SetTransform(D2D1::Matrix3x2F::Identity());

    if (entry_animation_state != AnimationState::Ended) {
        if (entry_animation_state == AnimationState::Scheduled) {
            entry_animation_timer.StartCounter();
            entry_animation_state = AnimationState::Ongoing;
        }

        auto time = entry_animation_timer.GetTime();
        RenderBoardWithEntryAnimation(time);

        if (time >= 500) {
            entry_animation_state = AnimationState::Ended;
        }
    }
    else if (mine_animation_state != AnimationState::Ended) {
        if (mine_animation_state == AnimationState::Scheduled) {
            mine_animation_timer.StartCounter();
            mine_animation_state = AnimationState::Ongoing;
        }

        auto time = mine_animation_timer.GetTime();
        RenderBoardWithGrayscaleAnimation(time);
    }
    else {
        d2d_context->DrawImage(
            board_bitmap.get(),
            D2D1::Point2F(
                (d2d_context->GetSize().width - FIELD_SIZE * GetBoardXSize()) / 2.0f,
                (d2d_context->GetSize().height - FIELD_SIZE * GetBoardYSize()) / 2.0f
            )
        );
    }
    RenderTexts();

    winrt::check_hresult(d2d_context->EndDraw());
}

void Minesweeper::RenderBoardWithEntryAnimation(double time) {
    float ratio = time < 500.0f ? 1.0f - time / 500.0f : 0.0f;

    perspective_transform_effect->SetInput(0, board_bitmap.get());
    perspective_transform_effect->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP_PERSPECTIVE_ORIGIN,
        D2D1::Vector2F(0.0f, ratio * 600.0f));
    perspective_transform_effect->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP_ROTATION,
        D2D1::Vector3F(0.0f, ratio * 60.0f, 0.0f));
    d2d_context->DrawImage(
        perspective_transform_effect.get(),
        D2D1::Point2F(
            (d2d_context->GetSize().width - FIELD_SIZE * GetBoardXSize()) / 2.0f,
            (d2d_context->GetSize().height - FIELD_SIZE * GetBoardYSize()) / 2.0f
        )
    );
}

void Minesweeper::RenderBoardWithGrayscaleAnimation(double time) {
    color_matrix_effect->SetInput(0, board_bitmap.get());
    color_matrix_effect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, GetMatrixForGrayscaleAnimation(time));
    d2d_context->DrawImage(
        color_matrix_effect.get(),
        D2D1::Point2F(
            (d2d_context->GetSize().width - FIELD_SIZE * GetBoardXSize()) / 2.0f,
            (d2d_context->GetSize().height - FIELD_SIZE * GetBoardYSize()) / 2.0f
        )
    );
}

void Minesweeper::RenderFields() {
    d2d_context->SetTarget(board_bitmap.get());
    d2d_context->BeginDraw();
    d2d_context->Clear(background_color);
    d2d_context->SetTransform(transformation);

    auto time = discover_animation_timer.GetTime();
    if (discover_animation_state == AnimationState::Ongoing) {
        if (time >= 1000) {
            discover_animation_state = AnimationState::Ended;
            DisableFieldGradient();
        }
        else {
            even_gradient_brush->SetRadiusX(GetBoardXSize() * FIELD_SIZE * static_cast<FLOAT>(time) / 500.0f);
            even_gradient_brush->SetRadiusY(GetBoardYSize() * FIELD_SIZE * static_cast<FLOAT>(time) / 500.0f);
            odd_gradient_brush->SetRadiusX(GetBoardXSize() * FIELD_SIZE * static_cast<FLOAT>(time) / 500.0f);
            odd_gradient_brush->SetRadiusY(GetBoardYSize() * FIELD_SIZE * static_cast<FLOAT>(time) / 500.0f);
        }
    }
    else if (discover_animation_state == AnimationState::Scheduled) {
        time = 0;
        discover_animation_timer.StartCounter();
        discover_animation_state = AnimationState::Ongoing;
    }

    for (std::size_t x = 0; x < fields.size(); x++) {
        for (std::size_t y = 0; y < fields[0].size(); y++) {
            fields[x][y].RenderField((x == hovered_x && y == hovered_y), d2d_context.get(),
                field_bitmap_def.GetBitmap(), main_brush.get(), even_gradient_brush.get(), odd_gradient_brush.get());
        }
    }

    winrt::check_hresult(d2d_context->EndDraw());
}

void Minesweeper::RebuildBoardBitmap() {
    if (!d2d_context) {
        return;
    }

    board_bitmap = nullptr;

    D2D1_SIZE_U bitmap_size_in_pixels = D2D1::SizeU(
        static_cast<UINT32>((FIELD_SIZE * GetBoardXSize()) / 96.0f * dpi),
        static_cast<UINT32>((FIELD_SIZE * GetBoardYSize()) / 96.0f * dpi)
    );

    winrt::check_hresult(d2d_context->CreateBitmap(
        bitmap_size_in_pixels,
        nullptr,
        0,
        D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET,
            D2D1::PixelFormat(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                D2D1_ALPHA_MODE_PREMULTIPLIED
            ),
            dpi,
            dpi
        ),
        board_bitmap.put()
    ));
    transformation = D2D1::Matrix3x2F::Translation((FIELD_SIZE * GetBoardXSize()) / 2.0f, (FIELD_SIZE * GetBoardYSize()) / 2.0f);
}

void Minesweeper::OnRender(HWND hwnd) {
    RenderFields();

    RenderBoard();

    DXGI_PRESENT_PARAMETERS parameters = { 0 };
    parameters.DirtyRectsCount = 0;
    parameters.pDirtyRects = nullptr;
    parameters.pScrollRect = nullptr;
    parameters.pScrollOffset = nullptr;

    auto hr = swap_chain->Present1(1, 0, &parameters);
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        // If the device was removed for any reason, a new device and swap chain will need to be created.
        HandleDeviceLost(hwnd);
    }
    else {
        winrt::check_hresult(hr);
    }
}

void Minesweeper::OnResize(UINT width, UINT height, HWND hwnd) {
    CreateWindowSizeDependentResources(hwnd);
}

void Minesweeper::OnMouseMove(HWND hwnd, const D2D1_POINT_2L& mouse_pos) {
    if (game_state.GetGameStatus() != GameStatus::Ongoing && game_state.GetGameStatus() != GameStatus::Init) {
        return;
    }

    FLOAT x_begin = center.x - GetBoardXSize() / 2.0f * FIELD_SIZE;
    FLOAT y_begin = center.y - GetBoardYSize() / 2.0f * FIELD_SIZE;

    FLOAT x_pos_diff = (mouse_pos.x * 96 / dpi) - x_begin;
    FLOAT y_pos_diff = (mouse_pos.y * 96 / dpi) - y_begin;

    int column_no = x_pos_diff > 0 ? static_cast<int>(x_pos_diff / FIELD_SIZE) : -1;
    int row_no = y_pos_diff > 0 ? static_cast<int>(y_pos_diff / FIELD_SIZE) : -1;

    if (hovered_x != column_no || hovered_y != row_no) {
        hovered_x = column_no;
        hovered_y = row_no;
        InvalidateRect(hwnd, nullptr, FALSE);
    }    
}

void Minesweeper::OnLeftButtonClick(HWND hwnd) {
    if (!IsValidFieldHovered() || (game_state.GetGameStatus() != GameStatus::Ongoing && game_state.GetGameStatus() != GameStatus::Init)) {
        return;
    }

    if (game_state.GetGameStatus() == GameStatus::Init) {
        GenerateBoard();
    }

    if (discover_animation_state != AnimationState::Ended) {
        DisableFieldGradient();
    }

    if (!fields[hovered_x][hovered_y].HandleLeftClick(game_state)) {
        return;
    }

    if (!fields[hovered_x][hovered_y].HasNeighboursWithMines()) {
        RevealNeighbours();
    }

    even_gradient_brush->SetCenter(D2D1::Point2F(
        FIELD_SIZE * (hovered_x + 0.5f - GetBoardXSize() / 2.0f),
        FIELD_SIZE * (hovered_y + 0.5f - GetBoardYSize() / 2.0f)));
    discover_animation_state = AnimationState::Scheduled;

    if (game_state.GetGameStatus() == GameStatus::Lost) {
        mine_animation_state = AnimationState::Scheduled;
    }

    InvalidateRect(hwnd, nullptr, FALSE);
}

void Minesweeper::OnRightButtonClick(HWND hwnd) {
    if (!IsValidFieldHovered() || (game_state.GetGameStatus() != GameStatus::Ongoing && game_state.GetGameStatus() != GameStatus::Init)) {
        return;
    }

    fields[hovered_x][hovered_y].HandleRightClick(game_state);
    InvalidateRect(hwnd, nullptr, FALSE);
}
