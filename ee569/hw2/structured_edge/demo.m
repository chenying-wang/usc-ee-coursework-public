% Name: Chenying Wang
% Email: chenying.wang@usc.edu
% USC ID: ****-****-**
% Date: Friday, February 14, 2020
WIDTH = 481;
HEIGHT = 321;

INPUT_IMAGE_GALLERY = './img/Gallery.raw';
EDGE_IMAGE_GALLERY = './img/Gallery_se.raw';
FINAL_EDGE_IMAGE_GALLERY = './img/Gallery_se_final.raw';
GROUND_TRUTH_FILE_GALLERY = './img/Gallery_GT.mat';

edge_Gallery = detectEdge(INPUT_IMAGE_GALLERY, EDGE_IMAGE_GALLERY, WIDTH, HEIGHT);
evalEdgeImg(EDGE_IMAGE_GALLERY, GROUND_TRUTH_FILE_GALLERY, WIDTH, HEIGHT, 99);

edgeImgFile = fopen(FINAL_EDGE_IMAGE_GALLERY, 'wb');
edgeImgData = uint8((edge_Gallery > 0.19)' * 255);
fwrite(edgeImgFile, edgeImgData, 'uint8');
fclose(edgeImgFile);

INPUT_IMAGE_DOGS = './img/Dogs.raw';
EDGE_IMAGE_DOGS = './img/Dogs_se.raw';
FINAL_EDGE_IMAGE_DOGS = './img/Dogs_se_final.raw';
GROUND_TRUTH_FILE_DOGS = './img/Dogs_GT.mat';

edge_Dogs = detectEdge(INPUT_IMAGE_DOGS, EDGE_IMAGE_DOGS, WIDTH, HEIGHT);
evalEdgeImg(EDGE_IMAGE_DOGS, GROUND_TRUTH_FILE_DOGS, WIDTH, HEIGHT, 99);

edgeImgFile = fopen(FINAL_EDGE_IMAGE_DOGS, 'wb');
edgeImgData = uint8((edge_Dogs > 0.18)' * 255);
fwrite(edgeImgFile, edgeImgData, 'uint8');
fclose(edgeImgFile);
