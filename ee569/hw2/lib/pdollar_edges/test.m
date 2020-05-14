clear;

data = load('./img/Dogs_GT.mat', 'groundTruth');
%[thrs, cntR, sumR, cntP, sumP, V] = edgesEvalImg(E, groundTruth);
size(data.groundTruth{1, 1}.Boundaries)
