#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include "tiffio.h"


typedef struct {
    BYTE r;
    BYTE g;
    BYTE b;
}R_G_B;


// read bytes
template <typename Type>
void read(std::ifstream& fp, Type& result, std::size_t size) {
    fp.read(reinterpret_cast<char*>(&result), size);
}

class BMP
{
public:
    const char* filename;
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER fileInfoHeader;
    RGBQUAD pallete[256];//по дефолту
    std::vector<std::vector<R_G_B>> rgb;//массив цветов (24bit)
    UINT8 buf;
    bool isBmp = false;
    bool isTiff = false;
    uint32 w, h;
    int linePadding; //размер отступа в конце каждой строки
    int colorsCount;
    int bitsOnColor;
    int maskValue;

    BMP() {};
    BMP(const char* filename)
    {
        this->filename = filename;
    }

    void readImgFile(const char*& filename);
    int getWidth() { return fileInfoHeader.biWidth; };
    int getHeight() { return fileInfoHeader.biHeight; };
    std::vector<std::vector<R_G_B>> getPixels() { return rgb; };
};

void BMP::readImgFile(const char*& filename)
{
    TIFF* tif = TIFFOpen(filename, "r");
    int k;
    int a[10];
    if (tif) 
    {
        TIFFGetField(tif, 256, &a[0]);
        isTiff = true;
        size_t npixels;
        uint32* raster;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
        npixels = w * h;
        raster = (uint32*)_TIFFmalloc(npixels * sizeof(uint32));
        rgb.assign(h, std::vector<R_G_B>(w));
        if (raster != NULL) {
            if (TIFFReadRGBAImage(tif, w, h, raster, 0)) {
                for(int i = 0; i < h; i++)
                    for (int j = 0; j < w; j++)
                    {
                        rgb[i][j].r = raster[i * w + j] & 0xff;
                        rgb[i][j].g = (raster[i * w + j] >> 8) & 0xff;
                        rgb[i][j].b = (raster[i * w + j] >> 16) & 0xff;
                    }
            }
            _TIFFfree(raster);
        }
        TIFFClose(tif);
    }
    else
    {
        //открытие файла
        std::ifstream fileStream(filename, std::ifstream::binary);
        if (!fileStream)
            std::cout << "Error opening file '" << filename << "'." << std::endl;
        else
        {
            //чтение заголовка
            read(fileStream, fileHeader.bfType, sizeof(fileHeader.bfType));
            read(fileStream, fileHeader.bfSize, sizeof(fileHeader.bfSize));
            read(fileStream, fileHeader.bfReserved1, sizeof(fileHeader.bfReserved1));
            read(fileStream, fileHeader.bfReserved2, sizeof(fileHeader.bfReserved2));
            read(fileStream, fileHeader.bfOffBits, sizeof(fileHeader.bfOffBits));

            if (fileHeader.bfType != 0x4D42)
            {
                std::cout << "Error: '" << filename << "' is not BMP file." << std::endl;
            }

            isBmp = true;

            // чтение информации изображения
            read(fileStream, fileInfoHeader.biSize, sizeof(fileInfoHeader.biSize));

            read(fileStream, fileInfoHeader.biWidth, sizeof(fileInfoHeader.biWidth));
            read(fileStream, fileInfoHeader.biHeight, sizeof(fileInfoHeader.biHeight));
            read(fileStream, fileInfoHeader.biPlanes, sizeof(fileInfoHeader.biPlanes));
            read(fileStream, fileInfoHeader.biBitCount, sizeof(fileInfoHeader.biBitCount));

            // получаем информацию о битности
            this->colorsCount = fileInfoHeader.biBitCount >> 3;
            if (colorsCount < 3) {
                colorsCount = 3;
            }

            this->bitsOnColor = fileInfoHeader.biBitCount / colorsCount;
            this->maskValue = (1 << bitsOnColor) - 1;

            read(fileStream, fileInfoHeader.biCompression, sizeof(fileInfoHeader.biCompression));
            read(fileStream, fileInfoHeader.biSizeImage, sizeof(fileInfoHeader.biSizeImage));
            read(fileStream, fileInfoHeader.biXPelsPerMeter, sizeof(fileInfoHeader.biXPelsPerMeter));
            read(fileStream, fileInfoHeader.biYPelsPerMeter, sizeof(fileInfoHeader.biYPelsPerMeter));
            read(fileStream, fileInfoHeader.biClrUsed, sizeof(fileInfoHeader.biClrUsed));
            read(fileStream, fileInfoHeader.biClrImportant, sizeof(fileInfoHeader.biClrImportant));
            //на 54 байте : читаем палитру
            read(fileStream, pallete, sizeof(pallete));
            // определение размера отступа в конце каждой строки
            this->linePadding = ((fileInfoHeader.biWidth * (fileInfoHeader.biBitCount / 8)) % 4) & 3;
            //this->linePadding = (fileInfoHeader.biWidth / 4 ) % 4; //для 8битовых

            //определения начала массива
            fileStream.close();
            std::ifstream fileStream(filename, std::ifstream::binary);
            fileStream.seekg(fileHeader.bfOffBits, std::ios_base::cur);

            //если bibitcount 4 политра максимум из 16 цветов, преобразуем в 24 бит(rgb)
            if (fileInfoHeader.biBitCount == 4)
            {
                std::vector<R_G_B> temp_rgb(fileInfoHeader.biWidth * fileInfoHeader.biHeight);
                for (int i = 0; i < temp_rgb.size() - 2; i += 2)
                {
                    fileStream.read((char*)&buf, sizeof(BYTE));
                    temp_rgb[i].r = pallete[buf / 16].rgbRed;
                    temp_rgb[i].g = pallete[buf / 16].rgbGreen;
                    temp_rgb[i].b = pallete[buf / 16].rgbBlue;
                    temp_rgb[i + 1].r = pallete[buf % 16].rgbRed;
                    temp_rgb[i + 1].g = pallete[buf % 16].rgbGreen;
                    temp_rgb[i + 1].b = pallete[buf % 16].rgbBlue;
                }
                rgb.assign(fileInfoHeader.biHeight, std::vector<R_G_B>(fileInfoHeader.biWidth));
                int k = 0;
                for (unsigned int i = 0; i < fileInfoHeader.biHeight; i++)
                    for (unsigned int j = 0; j < fileInfoHeader.biWidth; j++)
                    {
                        rgb[i][j] = temp_rgb[k];
                        k++;
                    }
            }
            if (fileInfoHeader.biBitCount == 8)
            {
                rgb.assign(fileInfoHeader.biHeight, std::vector<R_G_B>(fileInfoHeader.biWidth));
                for (unsigned int i = 0; i < fileInfoHeader.biHeight; i++)
                {
                    for (unsigned int j = 0; j < fileInfoHeader.biWidth; j++)
                    {
                        fileStream.read((char*)&buf, sizeof(BYTE));
                        rgb[i][j].r = pallete[buf].rgbRed;
                        rgb[i][j].g = pallete[buf].rgbGreen;
                        rgb[i][j].b = pallete[buf].rgbBlue;
                    }
                }
            }
            if (fileInfoHeader.biBitCount == 24)
            {
                // чтение массива для bibitcount 24 политрой не пользуемся
                unsigned char color[3];
                rgb.assign(fileInfoHeader.biHeight, std::vector<R_G_B>(fileInfoHeader.biWidth));
                for (unsigned int i = 0; i < fileInfoHeader.biHeight; i++)
                {
                    for (unsigned int j = 0; j < fileInfoHeader.biWidth; j++)
                    {
                        read(fileStream, color, fileInfoHeader.biBitCount / 8);
                        rgb[i][j].r = static_cast<BYTE>(color[2]);
                        rgb[i][j].g = static_cast<BYTE>(color[1]);
                        rgb[i][j].b = static_cast<BYTE>(color[0]);

                    }
                    fileStream.seekg(linePadding, std::ios_base::cur);//переход на след строку
                }
            }
        }
    }
    
}