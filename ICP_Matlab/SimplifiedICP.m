function [error, Reallignedsource] = SimplifiedICP(target,source)
    
    % This function rotates, translates and scales a 3D "source" of N*3 size (N points in N rows, 3 collumns for XYZ)
    % to fit a similar shaped  "target" again of N by 3 size
    %
    % The output shows the minimized value of dissimilarity measure in "error", the transformed source data set
    
    if(nargin < 2)
        display('require at least 2 arguments');
    end
    
    %**********---ICP---****************
    % Initially assign  high error value
    errortemp(1,:) = 100000;
    index=2;
    iteration = 1; % number of iterations after which loop will terminate
    threshold = 0.000001;
    % Initial calculation of alignment
    [errortemp(index,:),Reallignedsource,transform] = procrustes(target,source);
    % applying alignment
    source = transform.b*source*transform.T+repmat(transform.c(1,1:3),size(source,1),1);
    
    % max iterations or error > threshold
    while ((errortemp(index-1,:) - errortemp(index,:)) > threshold && iteration < 10)
        % display ('inside')
        % Update error and again calculate alignment and apply
        [errortemp(index+1,:),Reallignedsource,transform] = procrustes(target,source);
        source = transform.b*source*transform.T+repmat(transform.c(1,1:3),size(source,1),1);
        index=index+1;
        iteration =iteration+1;
    end
    
    display ('error')
    error = errortemp(index,:)
    
    
