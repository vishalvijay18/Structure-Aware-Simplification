    function [ mapping ] = runner(mesh_file)
        %   This is custom utility written for generating correspondence between 
        %   symmetric part of meshs.
        %   For our experiment we are using manual segmentation and assigning
        %   color to vertex/faces using meshlab.
        
        %   Example 1 : Table with two legs will have 3 segments or more  
        %           however both symmetric legs should be in black and red color
        %           respecitvely.
        %   Example 2 : Table with four symmetric legs.
        %               Run this program twice, giving black color to
        %               source and red color to target in each run.
        %   Strict assumption  : Black color vertices are source and Red color vertices are target

        %   Input   : mesh_file (name of mesh file)
        %   Output  : mesh_file.txt (under meshes folder, which will be used in decimation framework)
       
        %   Running example
        %   runner('meshs/newbigTable-catmull.obj'); 
        %   Output : newbigTable-catmull-mapping.txt under meshs folder
        
        if(nargin < 1)
            display ('minimum 1 argument.');
        end
            mapping = [];
            % read mesh using custom reader 
            [v,f,parts] = read_seg_obj(mesh_file, 3);
                % get source information
                sourceV = v(parts{1,1}{1,1},:);
                sourceF = f(parts{1,2}{1,1},:);

                % get target information
                targetV=v(parts{1,1}{1,2},:);
                targetF=f(parts{1,2}{1,2},:);
                % We are establishing mapping only between symmetric
                % parts, so this is mandotory step to preserve actual numbering
                % as per original mesh file 
                sourceSpit = min(sourceF(:))-1;
                targetSplit = min(targetF(:))-1;
                sourceF = sourceF - sourceSpit;
                targetF = targetF - targetSplit; 

                % calling our simplified ICP (Iterative closest pair) method
                [error,Reallignedsource] = SimplifiedICP(targetV,sourceV);

                % nearest point finder after transformation 
                dt = triangulation(targetF,targetV);
                xi = nearestNeighbor(dt, Reallignedsource);  
                 for i = 1:size(Reallignedsource,1)
                    temp(i,1) = sourceSpit + i;
                    temp(i,2) = xi(i) + targetSplit;
                 end
            mapping = cat(1,mapping,temp);
            filename = strcat(strsplit(mesh_file,'.'),'-mapping.txt');
            dlmwrite(filename{1,1},mapping,'delimiter','\t');
    end

