/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <fstream>
#include <string>

#include "ImageIO.h"
#include "Image.h"

ImageIO::ImageIO() {}

ImageIO::~ImageIO() {}

Image *ImageIO::readRawImage(Image *image, const char *filename) const {
    if (image == nullptr || filename == nullptr) {
        return image;
    }

    std::streampos size;
    std::ifstream fInput;
    fInput.open(filename, std::ios::in | std::ios::binary);
    if (!fInput.is_open()) {
        std::cerr << "Error: Failed to open file: " << filename << '\n';
        fInput.close();
        return image;
    }
    fInput.seekg(0, std::ios::end);
    size = fInput.tellg();
    fInput.seekg(0, std::ios::beg);

    auto puhBuffer = new PIXEL_TYPE[image->getSize()];
    fInput.read((char *) puhBuffer, size);
#ifdef __VERBOSE
    std::cout << size << " bytes read from " << filename << '\n';
#endif
    fInput.close();

    image->readFromArray(puhBuffer);
    delete[] puhBuffer;
    return image;
}

void ImageIO::writeRawImage(const Image *image, const char *filename) const {
    if (image == nullptr || image->getImage() == nullptr || filename == nullptr) {
        std::cerr << "Image is null or not initialized and skip writing to the file: " << filename << '\n';
        return;
    }

    std::ofstream fOutput;
    fOutput.open(filename, std::ios::out | std::ios::binary);
    if (!fOutput.is_open()) {
        std::cerr << "Error: Failed to open file: " << filename << '\n';
        return;
    }
    const auto puhBuffer = image->writeToArray();
    fOutput.write((char *) puhBuffer, image->getSize() * sizeof(PIXEL_TYPE));
    fOutput.close();
#ifdef __VERBOSE
    std::cout << "Image has been written to the file: " << filename << '\n';
#endif

    std::string strMetaFilename = filename;
    std::ofstream metafile;
    metafile.open(strMetaFilename.append(".meta"));
    if (!metafile.is_open()) {
        std::cerr << "Error: Failed to open file: " << filename << '\n';
        return;
    }
    metafile << "WIDTH=" << image->getWidth() << '\n';
    metafile << "HEIGHT=" << image->getHeight() << '\n';
    metafile << "CHANNEL=" << image->getChannel() << '\n';
    metafile.close();
}

void ImageIO::appendToRawImage(const Image *image, const char *filename) const {
    if (image == nullptr || image->getImage() == nullptr || filename == nullptr) {
        std::cerr << "Image is null or not initialized and skip writing to the file: " << filename << '\n';
        return;
    }

    std::ofstream fOutput;
    fOutput.open(filename, std::ios::app | std::ios::binary);
    if (!fOutput.is_open()) {
        std::cerr << "Error: Failed to open file: " << filename << '\n';
        return;
    }
    const auto puhBuffer = image->writeToArray();
    fOutput.write((char *) puhBuffer, image->getSize() * sizeof(PIXEL_TYPE));
    fOutput.close();
    std::cout << "Image has been appended to the file: " << filename << '\n';
}

void ImageIO::clean(const char *filename) const {
    std::ofstream fOutput;
    fOutput.open(filename, std::ios::trunc | std::ios::binary);
    if (!fOutput.is_open()) {
        std::cerr << "Error: Failed to open file: " << filename << '\n';
        return;
    }
    fOutput.close();
}
