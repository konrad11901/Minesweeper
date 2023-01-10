#pragma once

class BitmapDefinition {
public:
    BitmapDefinition(PCWSTR uri);
    void CreateDeviceIndependentResources(IWICImagingFactory* imaging_factory);
    void CreateDeviceDependentResources(ID2D1DeviceContext6* device_context);
    ID2D1Bitmap1* GetBitmap();
private:
    PCWSTR uri;
    winrt::com_ptr<IWICFormatConverter> converter;
    winrt::com_ptr<ID2D1Bitmap1> bitmap;
};