#ifndef BITMAP_HPP
#define BITMAP_HPP

#include <fstream>
#include <vector>

#include "Vector3.hpp"

namespace GraphGenerator
{
#pragma pack(push, 1)
    struct BmpHeader
    {
        std::uint16_t type;
        std::uint32_t size;
        std::uint16_t reserved1;
        std::uint16_t reserved2;
        std::uint32_t offset;
    };

    struct BmpInfoHeader
    {
        std::uint32_t size;
        std::int32_t  width;
        std::int32_t  height;
        std::uint16_t planes;
        std::uint16_t bitCount;
        std::uint32_t compression;
        std::uint32_t imageSize;
        std::int32_t  xResolution;
        std::int32_t  yResolution;
        std::uint32_t colorsUsed;
        std::uint32_t colorsImportant;
    };
#pragma pack(pop)

    void writeToFiles(std::vector<std::vector<Vector3>> colorGraph, std::ofstream& stream)
    {
        auto height = colorGraph.size();
        auto width = colorGraph.size();
        auto padding = (4 - (width * 3) % 4) % 4;

        BmpHeader header;
        header.type = 0x4D42;
        header.reserved1 = 0;
        header.reserved2 = 0;
        header.offset = sizeof(BmpHeader) + sizeof(BmpInfoHeader);

        BmpInfoHeader infoHeader;
        infoHeader.size = sizeof(BmpInfoHeader);
        infoHeader.width = width;
        infoHeader.height = height;
        infoHeader.planes = 1;
        infoHeader.bitCount = 24;
        infoHeader.compression = 0;
        infoHeader.imageSize = (width * 3 + padding) * height;
        infoHeader.xResolution = 0;
        infoHeader.yResolution = 0;
        infoHeader.colorsUsed = 0;
        infoHeader.colorsImportant = 0;

        header.size = header.offset + infoHeader.imageSize;

        stream.write(reinterpret_cast<char*>(&header), sizeof(BmpHeader));
        stream.write(reinterpret_cast<char*>(&infoHeader), sizeof(BmpInfoHeader));

        const std::uint8_t paddingBuffer[3] = { 0, 0, 0 };
        for (auto i = 0; i < height; i++)
        {
            for (auto j = 0; j < width; j++)
            {
                char red = colorGraph[i][j].x * 255;
                char green = colorGraph[i][j].y * 255;
                char blue = colorGraph[i][j].z * 255;
                stream.write(&blue, 1);
                stream.write(&green, 1);
                stream.write(&red, 1);
            }
            stream.write(reinterpret_cast<const char*>(paddingBuffer), padding);
        }
    }
}

#endif // !BITMAP_HPP