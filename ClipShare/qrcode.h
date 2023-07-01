#pragma once
#include "stdafx.h"
#include "path.h"

#pragma pack(push, 1)
struct BitmapFileHeader {
    uint16_t type;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;
};

struct BitmapInfoHeader {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t dataSize;
    int32_t horizontalResolution;
    int32_t verticalResolution;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
};
#pragma pack(pop)

class QRCode {
public:
    QRCode();

    int generate(const std::string text, const Path& outfile);

    void paint(HWND hwnd, const Path& path);
private:
    bool create_bitmap(const Path& path, int size, uint8_t* data);
};

