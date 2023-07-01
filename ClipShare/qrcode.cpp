#include "qrcode.h"

QRCode::QRCode()
{

}

int QRCode::generate(const std::string text, const Path& path)
{
    if (!path.is_file())
    {
        return 0;
    }

    QRcode* qr = QRcode_encodeString(text.c_str(), 0, QR_ECLEVEL_H, QR_MODE_8, 1);
    const int scale = 10;
    const int size = qr->width * scale;

    std::vector<uint8_t> imageData(size * size * 3, 255);


    for (int y = 0; y < qr->width; ++y) {
        for (int x = 0; x < qr->width; ++x) {
            if (qr->data[y * qr->width + x] & 1) {
                for (int dy = 0; dy < scale; ++dy) {
                    for (int dx = 0; dx < scale; ++dx) {
                        imageData[((y * scale + dy) * size) + (x * scale + dx)] = 0;
                    }
                }
            }
        }
    }

    this->create_bitmap(path, size, imageData.data());
    QRcode_free(qr);

    return size;
}

void QRCode::paint(HWND hwnd, const Path& path)
{
    if (!path.is_file())
    {
        return;
    }

    HBITMAP hBitmap = (HBITMAP)LoadImageA(NULL, (LPCSTR)path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (!hBitmap)
    {
        return;
    }

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;


    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    HDC hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, hBitmap);
    
    BITMAP bm;
    GetObjectW(hBitmap, sizeof(bm), &bm);

    SetStretchBltMode(hdc, HALFTONE);
    StretchBlt(hdc, 0, 0, clientWidth, clientHeight, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    DeleteDC(hdcMem);
    DeleteObject(hBitmap);


    EndPaint(hwnd, &ps);
}

bool QRCode::create_bitmap(const Path& path, int size, uint8_t* data)
{
    std::ofstream file(path, std::ios::binary);
    if (!file)
    {
        return false;
    }

    int paddingSize = (4 - (size * 3) % 4) % 4;
    int dataSize = (size * 3 + paddingSize) * size;
    int fileSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + dataSize;

    BitmapFileHeader fileHeader;
    fileHeader.type = 0x4D42;
    fileHeader.fileSize = fileSize;
    fileHeader.reserved1 = 0;
    fileHeader.reserved2 = 0;
    fileHeader.dataOffset = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);

    BitmapInfoHeader infoHeader;
    infoHeader.size = sizeof(BitmapInfoHeader);
    infoHeader.width = size;
    infoHeader.height = size;
    infoHeader.planes = 1;
    infoHeader.bitCount = 24;
    infoHeader.compression = 0;
    infoHeader.dataSize = dataSize;
    infoHeader.horizontalResolution = 0;
    infoHeader.verticalResolution = 0;
    infoHeader.colorsUsed = 0;
    infoHeader.colorsImportant = 0;

    file.write(reinterpret_cast<char*>(&fileHeader), sizeof(BitmapFileHeader));
    file.write(reinterpret_cast<char*>(&infoHeader), sizeof(BitmapInfoHeader));

    int padding = 0;
    for (int y = size - 1; y >= 0; y--)
    {
        for (int x = 0; x < size; x++)
        {
            file.put(data[y * size + x]);
            file.put(data[y * size + x]);
            file.put(data[y * size + x]);
        }

        for (int p = 0; p < paddingSize; ++p)
        {
            file.put(0);
        }
    }

    file.close();
    return true;
}
