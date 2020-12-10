clear;

%% image_sets_1
ori_img = imread('./data/image_sets_1/house.bmp');
images = ["house_distor1", "house_distor2", "house_distor3", "house_distor4", "house_distor5"];

for image = images
    img = imread('./data/image_sets_1/' + image + '.bmp');
    [mssim, ssim_map] = ssim(img, ori_img);
    fprintf('Image: %s, MSSIM: %f\n', image, mssim);
end

%% image_sets_2
images = ["house_distor6", "house_distor7", "house_distor8", "house_distor9", "house_distor10"];
for image = images
    img = imread('./data/image_sets_2/' + image + '.bmp');
    [mssim, ssim_map] = ssim(img, ori_img);
    fprintf('Image: %s, MSSIM: %f\n', image, mssim);
end

%% image_sets_3
images = ["buildings", "airplane", "boat"];
for image = images
    img = im2gray(imread('./data/image_sets_3/' + image + '.jpg'));
    ori_img = imread('./data/image_sets_3/' + image + '_ori.bmp');
    [mssim, ssim_map] = ssim(img, ori_img);
    fprintf('Image: %s, MSSIM: %f\n', image, mssim);
    imwrite(max(ssim_map, 0) .^ 4, './out/' + image + '_ssim_map.png');
end
