% Name: Chenying Wang
% Email: chenying.wang@usc.edu
% USC ID: ****-****-**
% Date: Friday, January 24, 2020
clear;

addpath('../lib/tut_bm3d');

MAX_PIXEL_VALUE = 255;
WIDTH = 320;
HEIGHT = 320;

noisyImageFilename = '../img/Corn_noisy.raw';
oriImageFilename = '../img/Corn_gray.raw';
outputImageFilename = '../img/result/bm3d.raw';

noisyImageFile = fopen(noisyImageFilename, 'rb');
noisyData = fread(noisyImageFile, inf, 'uint8');
fclose(noisyImageFile);

oriImageFile = fopen(oriImageFilename, 'rb');
oriData = fread(oriImageFile, inf, 'uint8');
fclose(oriImageFile);

z = reshape(noisyData, WIDTH, HEIGHT)' / MAX_PIXEL_VALUE;
y = reshape(oriData, WIDTH, HEIGHT)' / MAX_PIXEL_VALUE;

clear noisyData oriData;

sigma = std(y - z, 0, 'all');
y_est = BM3D(z, sigma, 'refilter');

psnr = getPSNR(y, y_est);
psnr_cropped = getCroppedPSNR(y, y_est, [16, 16]);

fprintf("PSNR: %.4fdB\n", psnr);
fprintf("Cropped PSNR: %.4fdB\n", psnr_cropped);

outputImageFile = fopen(outputImageFilename,'wb');
outputData = uint8(y_est' * 255);
count = fwrite(outputImageFile, outputData, 'uint8');
fclose(outputImageFile);
