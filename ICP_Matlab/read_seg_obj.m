    function [ v,f,parts] = read_obj_seg( filename, num_of_colored_parts)
        % Custom .obj reader to group symmetric parts of mesh 
        
        % Input : filename, number_of_colored_parts
        % Output : v, all vertices in N*3 array
        %          f, all faces in N*3 array
        %          parts: cell of cells of size 1*2 (containg different
        %          segmented part of mesh)
        %          parts{1,1} -> will contain index of segmented vertcies 
        %          parts{1,1}{1,1}->black colored
        %          parts{1,1}{1,2}->red colored
        %          parts{1,2} -> will conatin segmented faces corresponding to vertices
        %          parts{1,2}{1,1} -> black color faces
        %          parts{1,2}{1,2} -> red color faces
        
        fid = fopen(filename, 'r');
        v = [];
        f = [];
        vertexCount = 0;
        faceCount = 0;
        vertex_color_index = cell(1, num_of_colored_parts);
        face_color_index = cell(1, num_of_colored_parts);
        C = textscan(fid, '%s %s %s %s %s %s %s',1);
        while ~feof(fid)
            if strcmp(C{1}{1}, 'v')
                vertexCount=vertexCount+1;
                v1 = str2double(C{2}{1});  v2 = str2double(C{3}{1});  v3 = str2double(C{4}{1});
                r = str2double(C{5}{1});  g = str2double(C{6}{1});  b = str2double(C{7}{1});
                if(r==0 && g==0 && b==0)
                % black vertex
                vertex_color_index{1,1} = cat(1,vertex_color_index{1,1},vertexCount);
                elseif(r==1 && g==0 && b==0)
                 %red vertex
                vertex_color_index{1,2} = cat(1,vertex_color_index{1,2},vertexCount);
                else
                % anything else
                vertex_color_index{1,3} = cat(1,vertex_color_index{1,3},vertexCount);
                end
                v = cat(1, v, [v1,v2,v3]);

            elseif strcmp(C{1}{1}, 'f')
                fi1 = regexp(C{2}{1}, '\d+', 'match');
                fi2 = regexp(C{3}{1}, '\d+', 'match');
                fi3 = regexp(C{4}{1}, '\d+', 'match');

                v1 = str2num(fi1{1});
                v2 = str2num(fi2{1});
                v3 = str2num(fi3{1});

                faceCount = faceCount+1;
                if(ismember(v1,vertex_color_index{1,1}) && ismember(v2,vertex_color_index{1,1}) ...
                        && ismember(v3,vertex_color_index{1,1}))
                     % faces having black vertices 
                     face_color_index{1,1} = cat(1,face_color_index{1,1},faceCount);
                elseif(ismember(v1,vertex_color_index{1,2}) && ismember(v2,vertex_color_index{1,2}) ...
                        && ismember(v3,vertex_color_index{1,2}))
                       % faces having red vertices 
                     face_color_index{1,2} = cat(1,face_color_index{1,2},faceCount);
                else 
                    % anything else
                     face_color_index{1,3} = cat(1,face_color_index{1,3},faceCount);
                end   
               
                f = cat(1, f, [v1,v2,v3]);
             end
            C = textscan(fid, '%s %s %s %s %s %s %s', 1);
         end
        parts = {vertex_color_index,face_color_index};

        fclose(fid);

    end

