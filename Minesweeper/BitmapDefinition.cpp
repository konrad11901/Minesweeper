#include "pch.h"
#include "BitmapDefinition.h"

BitmapDefinition::BitmapDefinition(PCWSTR uri) : uri(uri) {}

void BitmapDefinition::CreateDeviceIndependentResources(IWICImagingFactory* imaging_factory) {
    winrt::com_ptr<IWICBitmapDecoder> decoder;
    winrt::com_ptr<IWICBitmapFrameDecode> source;
    winrt::com_ptr<IWICStream> stream;

    winrt::check_hresult(imaging_factory->CreateDecoderFromFilename(
        uri,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        decoder.put()
    ));

    winrt::check_hresult(decoder->GetFrame(0, source.put()));

    winrt::check_hresult(imaging_factory->CreateFormatConverter(converter.put()));

    winrt::check_hresult(converter->Initialize(
        source.get(),
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.f,
        WICBitmapPaletteTypeMedianCut
    ));
}

void BitmapDefinition::CreateDeviceDependentResources(ID2D1DeviceContext6* device_context) {
    winrt::check_hresult(device_context->CreateBitmapFromWicBitmap(
        converter.get(),
        bitmap.put()
    ));
}

ID2D1Bitmap1* BitmapDefinition::GetBitmap() {
    return bitmap.get();
}