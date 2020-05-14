% Name: Chenying Wang
% Email: chenying.wang@usc.edu
% USC ID: ****-****-**
% Date: Friday, February 14, 2020
function E = detectEdge(oriImg, edgeImg, width, height)
    currentFolder = pwd;
    opts=edgesTrain();                % default options (good settings)
    opts.modelDir='models/';          % model will be in models/forest
    opts.modelFnm='modelBsds';        % model name
    opts.nPos=5e5; opts.nNeg=5e5;     % decrease to speedup training
    opts.useParfor=0;                 % parallelize if sufficient memory

    tic, model=edgesTrain(opts); toc; % will load model if already trained

    model.opts.multiscale=0;          % for top accuracy set multiscale=1
    model.opts.sharpen=2;             % for top speed set sharpen=0
    model.opts.nTreesEval=4;          % for top speed set nTreesEval=1
    model.opts.nThreads=4;            % max number threads for evaluation
    model.opts.nms=0;                 % set to true to enable nms

    cd(currentFolder);
    oriImgFile = fopen(oriImg, 'rb');
    oriImgData = fread(oriImgFile, inf, 'uint8');
    fclose(oriImgFile);
    I = permute(reshape(oriImgData, 3, width, height), [3, 2, 1]) / 255;
    tic, E=edgesDetect(I,model); toc

    edgeImgFile = fopen(edgeImg, 'wb');
    edgeImgData = uint8(E' * 255);
    fwrite(edgeImgFile, edgeImgData, 'uint8');
    fclose(edgeImgFile);
