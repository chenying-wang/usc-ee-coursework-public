/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
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

    FILE *fInput;
    if (!(fInput = fopen(filename, "rb"))) {
        std::cerr << "Cannot open input file:" << filename  << '\n';
        return image;
    }
    auto puhBuffer = new uint8_t[image->getSize()];
    auto size = fread(puhBuffer, image->getDepth(), image->getSize(), fInput);
	fclose(fInput);
    std::cout << size << " bytes read from " << filename << '\n';
    image->readFromArray(puhBuffer);
    delete[] puhBuffer;
    return image;
}

void ImageIO::writeRawImage(const Image *image, const char *filename) const {
    if (image == nullptr || image->getImage() == nullptr) {
        std::cerr << "Image is null or not initialized and skip writing to the file: " << filename << '\n';
        return;
    }

    FILE *fOutput;
    if (!(fOutput = fopen(filename, "wb"))) {
        std::cerr << "Cannot open output file: " << filename << '\n';
        return;
    }

    const auto puhBuffer = image->writeToArray();
    fwrite(puhBuffer, image->getDepth(), image->getSize(), fOutput);
    fclose(fOutput);
    delete[] puhBuffer;
    std::cout << "Image has been written to the file: " << filename << '\n';

    char *phMetaFilename = new char[std::strlen(filename) + 5];
    std::strcpy(phMetaFilename, filename);
    std::strcat(phMetaFilename, ".meta");
    std::ofstream metafile;
    metafile.open (phMetaFilename);
    metafile << "WIDTH=" << image->getWidth() << '\n';
    metafile << "HEIGHT=" << image->getHeight() << '\n';
    metafile << "CHANNEL=" << image->getChannel() << '\n';
    metafile.close();
}
