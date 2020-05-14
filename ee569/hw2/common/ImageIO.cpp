/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#include <iostream>
#include <fstream>
#include <cstring>

#include "ImageIO.h"
#include "Image.h"

ImageIO::ImageIO() {}

ImageIO::~ImageIO() {}

Image *ImageIO::readRawImage(Image *image, const char *filename) const {
    if (image == nullptr) {
        return image;
    }

    std::streampos size;
    std::ifstream fInput;
    fInput.open(filename, std::ios::in | std::ios::binary);
    fInput.seekg(0, std::ios::end);
    size = fInput.tellg();
    fInput.seekg(0, std::ios::beg);

    auto puhBuffer = new PIXEL_TYPE[image->getSize()];
    fInput.read((char *) puhBuffer, size);
    std::cout << size << " bytes read from " << filename << '\n';
    fInput.close();

    image->readFromArray(puhBuffer);
    delete[] puhBuffer;
    return image;
}

void ImageIO::writeRawImage(const Image *image, const char *filename) const {
    if (image == nullptr || image->getImage() == nullptr) {
        std::cerr << "Image is null or not initialized and skip writing to the file: " << filename << '\n';
        return;
    }

    std::ofstream fOutput;
    fOutput.open(filename, std::ios::out | std::ios::binary);
    const auto puhBuffer = image->writeToArray();
    fOutput.write((char *) puhBuffer, image->getSize() * sizeof(PIXEL_TYPE));
    fOutput.close();
    std::cout << "Image has been written to the file: " << filename << '\n';

    char *phMetaFilename = new char[std::strlen(filename) + 5];
    std::strcpy(phMetaFilename, filename);
    std::strcat(phMetaFilename, ".meta");
    std::ofstream metafile;
    metafile.open(phMetaFilename);
    metafile << "WIDTH=" << image->getWidth() << '\n';
    metafile << "HEIGHT=" << image->getHeight() << '\n';
    metafile << "CHANNEL=" << image->getChannel() << '\n';
    metafile.close();
}
