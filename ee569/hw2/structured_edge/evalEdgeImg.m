% Name: Chenying Wang
% Email: chenying.wang@usc.edu
% USC ID: ****-****-**
% Date: Friday, February 14, 2020
function optimalDec = evalEdgeImg(edgeImg, groundTruth, width, height, thrs)
    edgeImgFile = fopen(edgeImg, 'rb');
    edgeImgData = fread(edgeImgFile, inf, 'uint8');
    edgeImgData = reshape(edgeImgData, width, height)' / 255;
    data = load(groundTruth, 'groundTruth');
    fclose(edgeImgFile);

    prms = struct('thrs', thrs);
    fprintf('%s:\n', edgeImg);
    optimalDec = zeros(length(data.groundTruth), 4);
    for i = 1:length(data.groundTruth)
        gt = data.groundTruth{i};
        data_gt = struct('groundTruth', {{gt}});
        [thrs, cntR, sumR, cntP, sumP, ~] = edgesEvalImg(edgeImgData, data_gt, prms);
        R = cntR ./ sumR;
        P = cntP ./ sumP;
        F = 2 * R .* P ./ (R + P);
        maxF = max(F);
        decision = [thrs F P R];
        optimalDec(i, :) = decision(F == maxF, :);
        fprintf("GT: %d, Threshold = %f, F = %f, Recall = %f, Precision = %f\n", i, optimalDec(i, :));
    end
