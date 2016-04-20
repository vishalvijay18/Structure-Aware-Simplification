#include "smfParser.h"

/**
 * function to create new vertex
 *
 * @param x, y, z coordinates
 * @return vertex with specified x,y,z coordinates
 *
 */
vertex *newVertex(float x, float y, float z)
{
	vertex *temp = new vertex;
	temp->x = x;
	temp->y = y;
	temp->z = z;
	temp->edge = NULL;
	temp->displayFlag = 0;
	temp->quadMat = NULL;

	return temp;
}

/**
 * Function to create new edge
 * @params
 * 			*start = pointer to start vertex
 * 			*end = pointer to end vertex
 * @return
 * 			new edge with specified start and end vertex
 */
w_edge *newWEdge(vertex *start, vertex *end)
{
	w_edge* edge = new w_edge;
	edge->start = start;
	edge->end = end;
	edge->left = NULL;
	edge->right = NULL;
	edge->left_prev = NULL;
	edge->left_next = NULL;
	edge->right_prev = NULL;
	edge->right_next = NULL;
	edge->decimationCost = 0;

	return edge;
}

//function to create new wEdge with face structure passed as argument
w_edge *newWEdge(vertex *start, vertex *end, face* left)
{
    w_edge* edge = newWEdge(start, end);
	edge->left = left;

    return edge;
}

//function to create new face object
face *newFace(w_edge *edge)
{
	face* temp = new face;
	temp->edge = edge;
	temp->displayFlag = 0;
	temp->quadMat = NULL;

	return temp;
}

//function to create new glvector
GLVector* newGLVector(float x, float y, float z)
{
	GLVector* temp = new GLVector;
	temp->x = x;
	temp->y = y;
	temp->z = z;
	return temp;
}

//Checks if equal, with relaxed precision
/**
 *  checking face normal direction (with relaxed precision)
 *  @return
 *  		true : if almost same direction
 *  		false : otherwise
 */
bool faceNormalDirectionValidity(GLVector *v1, GLVector *v2)
{
	//using cross product
	//float x = ((v1->y)*(v2->z)) - ((v1->z)*(v2->y));
    	//float y = ((v2->x)*(v1->z)) - ((v1->x)*(v2->z));
    	//float z = ((v1->x)*(v2->y)) - ((v1->y)*(v2->x));	
	//if(x==0 && y==0 && z==0)
	//return true;
	
	//using dot product
	float modV1 = sqrt((v1->x * v1->x) + (v1->y * v1->y) + (v1->z * v1->z));
	float modV2 = sqrt((v2->x * v2->x) + (v2->y * v2->y) + (v2->z * v2->z));

	float cosTheta = ((v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z))/(modV1 * modV2);

	if(cosTheta > 0.866)
	return true;

	return false;
}

// DFS for planar proxies
void DFSUtil(face* f, map<face*, bool> &visited, int proxy)
{
    // Mark the current node as visited and print it
	visited[f] = true;
 	f->proxy = proxy;

	w_edge * faceEdgeArray[3];
	w_edge *edge = f->edge;
    	if (edge->left != f) edge = edge->right_next->left_prev;

	faceEdgeArray[0] = edge;
	faceEdgeArray[1] = edge->left_next;
	faceEdgeArray[2] = edge->left_prev;

	for(int k=0; k<3; k++)
	{
		if(faceEdgeArray[k]->left == f)
		{
			face* rightface = faceEdgeArray[k]->right;
			
			if(!visited[rightface])
			{
				GLVector *normal1 = faceNormalMap[f];
				GLVector *normal2 = faceNormalMap[rightface];

				if(faceNormalDirectionValidity(normal1, normal2))
				{
					DFSUtil(rightface, visited, proxy);
				}
			}
		}
		else
		{
			face* leftface = faceEdgeArray[k]->left;
			
			if(!visited[leftface])
			{
				GLVector *normal1 = faceNormalMap[f];
				GLVector *normal2 = faceNormalMap[leftface];

				if(faceNormalDirectionValidity(normal1, normal2))
				{
					DFSUtil(leftface, visited, proxy);
				}
			}
		}
	}
}

/**
 * Depth first search implementation for finding nearby proxy plane.
 */
void DFS()
{
	// Mark all the vertices as not visited
	map<face*, bool> visited;
	bool allVisited = false;
	face* fCurrent;
	
	for (auto f : faceList)
        visited[f] = false;
     	
	fCurrent = *faceList.begin();
	visited[fCurrent] = true;

        int proxy = 1;
        // Call the recursive helper function to print DFS traversal
        while(!allVisited)
	{
        	DFSUtil(fCurrent, visited, proxy);
		
		int noIfound = 1;

		for (auto i : faceList)
        	{
			if(visited[i] == false)			
			{
				fCurrent = i;
				noIfound = 0;
			}
		}
	
		if(noIfound == 1)
		allVisited = true;

		proxy++;
	}

	cout << "Proxy: " << proxy-1 << endl;
}
/**
 * For saving .SMF file.
 *
 * @param filename
 * @return void
 */
void saveMesh()
{
        ofstream outFile("output.smf", ofstream::out);

        if (!outFile.is_open()) {
            cout << "Failed" << endl;
            return;
        }

        int numVertices, numFaces;
        int vertexAdjust = 0;
        int faceAdjust = 0;
        vector<int> prefix;

        for (auto vertex1 : vertexList)
        {
            if (vertex1->displayFlag == 1)
            	vertexAdjust++;

            prefix.push_back(vertexAdjust);
        }

        for (auto f : faceList)
            if (f->displayFlag == 1) faceAdjust++;

        numVertices = vertexList.size() - vertexAdjust;
        numFaces = faceList.size() - faceAdjust;
        outFile << "# " << numVertices << " " << numFaces << endl;

        for (auto v1 : vertexList)
        {
            if (v1->displayFlag == 1)
            	continue;

            outFile << "v " << v1->x << " " << v1->y << " " << v1->z << endl;
        }

        for (auto f : faceList)
        {
            if (f->displayFlag == 1)
            	continue;

            w_edge *edg = f->edge;
            w_edge *edg1 = edg;

            outFile << "f";

            do {
                outFile << " " << indexMap[edg1->start] + 1 - prefix[indexMap[edg1->start]];      // The faces data

                if (edg1->right == f)
                    edg1 = edg1->right_next;
                else
                    edg1 = edg1->left_next;

            } while (edg1 != edg);

            outFile << endl;
        }

        outFile.close();

        cout << "File saved." << endl;
 }

/*
 *   populating proxy plane of each face
 */
void calculateProxy()
{
	proxyCount.clear();	
	DFS();

	for(auto f : faceList) {
		if (proxyCount.find(f->proxy) != proxyCount.end())
			proxyCount[f->proxy] += 1;
		else
			proxyCount[f->proxy] = 1;
		
		//cout << f->proxy << " " << proxyCount[f->proxy] << endl;
    	}
}


// calculating face normal in unnormalized
GLVector *faceNormalFromVertexUnnormalized(face* f, Vertex *v1, Vertex *v2, Vertex *v3)
{
	float x = ((v2->y - v1->y)*(v3->z - v1->z)) - ((v2->z - v1->z)*(v3->y - v1->y));
    float y = ((v3->x - v1->x)*(v2->z - v1->z)) - ((v3->z - v1->z)*(v2->x - v1->x));
    float z = ((v2->x - v1->x)*(v3->y - v1->y)) - ((v2->y - v1->y)*(v3->x - v1->x));

	return newGLVector(x, y, z);
}

// calculating face normals
void calculateFaceNormal()
{
	std::map<face*, GLVector*>::iterator itr4;

	for(itr4 = faceNormalMap.begin(); itr4 != faceNormalMap.end(); itr4++)
	{
    		// found it - delete it
    		delete itr4->second;
    		faceNormalMap.erase(itr4);
	}
	
	for(face* itr : faceList)
	{
		face* f = itr;

		if (f->displayFlag == 1) continue;

		w_edge *edge = f->edge;
    		if (edge->left != f) edge = edge->right_next->left_prev;             // Get the three points in the plane
    		Vertex *vertex1 = edge->start;
    		Vertex *vertex2 = edge->end;
    		Vertex *vertex3 = edge->left_next->end;
		/*
		*create face normals
		*/
		GLVector *vector1;
		vector1 = faceNormalFromVertexUnnormalized(f, vertex1, vertex2, vertex3);
		faceNormalMap.insert(std::make_pair(f, vector1));
	}

}

// updating the vertexNormal in vertexNormalMap
void updateVertexNormal(face* f, Vertex* v1, Vertex* v2, Vertex* v3)
{
	GLVector* vertexNormal;
	GLVector* faceNormal;
	
	if(vertexNormalMap.find(v1) != vertexNormalMap.end())
	{
		vertexNormal = vertexNormalMap[v1];
	}
	else
	{	
		vertexNormal = newGLVector(0.0, 0.0, 0.0);
		vertexNormalMap.insert(std::make_pair(v1, vertexNormal));
	}
	
	if(faceNormalMap.find(f) != faceNormalMap.end())
	{
		faceNormal = faceNormalMap[f];		
		float normalLength = sqrtf((faceNormal->x * faceNormal->x) + (faceNormal->y * faceNormal->y) + (faceNormal->z * faceNormal->z));

		float vector1X = v2->x - v1->x;
		float vector1Y = v2->y - v1->y;
		float vector1Z = v2->z - v1->z;

		float vector2X = v3->x - v1->x;
		float vector2Y = v3->y - v1->y;
		float vector2Z = v3->z - v1->z;

		float vector1Length = sqrtf((vector1X * vector1X) + (vector1Y * vector1Y) + (vector1Z * vector1Z));
		float vector2Length = sqrtf((vector2X * vector2X) + (vector2Y * vector2Y) + (vector2Z * vector2Z));

		float sin_alpha = normalLength/(vector1Length*vector2Length);

		float normalX = faceNormal->x/normalLength;
		float normalY = faceNormal->y/normalLength;
		float normalZ = faceNormal->z/normalLength;

		normalX = normalX * asin(sin_alpha);
		normalY = normalY * asin(sin_alpha);
		normalZ = normalZ * asin(sin_alpha);

		//updating the entry in vertexNormal
		vertexNormal->x = vertexNormal->x + normalX;
		vertexNormal->y = vertexNormal->y + normalY;
		vertexNormal->z = vertexNormal->z + normalZ;
	}
	else
	{	
		cout << "error in updateVertexNormal()";
	}
}

// function calculating vertex normal for all vertices
void calculateVertexNormal()
{
	//clear old vertexNormalData
	std::map<Vertex*, GLVector*>::iterator itr5;
	for(itr5 = vertexNormalMap.begin(); itr5 != vertexNormalMap.end(); itr5++)
	{
    		// found it - delete it
    		delete itr5->second;
    		vertexNormalMap.erase(itr5);
	}

	for(face* itr : faceList)
	{
		face* f = itr;

		if (f->displayFlag == 1) continue;
	
    		w_edge *edge = f->edge;
    		if (edge->left != f) edge = edge->right_next->left_prev;             // Get the three points in the plane
    		Vertex *vertex1 = edge->start;
    		Vertex *vertex2 = edge->end;
    		Vertex *vertex3 = edge->left_next->end;

		updateVertexNormal(f, vertex1, vertex2, vertex3);
		updateVertexNormal(f, vertex2, vertex3, vertex1);
		updateVertexNormal(f, vertex3, vertex1, vertex2);
	}
}

// function returning vertex normal of a vertex
GLVector* findVertexNormal(Vertex *v)
{
	if(vertexNormalMap.find(v) != vertexNormalMap.end())
		return vertexNormalMap[v];
	else
		cout << "error finding vertex normal\n";

	return NULL;
}

/**
 * Utility method
 */
void processStringForInt(string &s, char delim, vector<int> &vec)
{
    stringstream ss(s);
    string str;

    while (getline(ss, str, delim)) {
        if (str.compare("") != 0)
            vec.push_back((int)stof(str));
    }

}

/**
 * clear all tables on load
 */
void clearData()
{
    vertexList.clear();
    faceList.clear();
    edgeList.clear();
}

/**
 *  for reading .smf file.
 *
 *  @param name of file
 *  @return void
 */
void loadMesh(string fileName)
{
	//need to remove 'newline' which is passed in output from linux command
	fileName.erase(std::remove(fileName.begin(), fileName.end(), '\n'),
			fileName.end());

	int count = 1;
	string inputType;
	int numVertices, numFaces;

	ifstream fileInput(fileName.c_str());

	if (!fileInput.is_open()) {
		cout << "File open issue." << endl;
		return;
	}

	clearData();

	//reading number of vertices and number of faces
	fileInput >> inputType;
	fileInput >> numVertices >> numFaces;

	while (count <= numVertices) {
		float x, y, z;

		fileInput >> inputType;
		fileInput >> x >> y >> z;

		vertex *vertex1 = newVertex(x, y, z);      // Add new vertex
		vertexList.push_back(vertex1);
		indexMap[vertex1] = (vertexList.size() - 1);

		count++;
	}

	count = 1;

	while (count <= numFaces) {
		//vertices indexes
		int v1, v2, v3;
		vector<int> verticesIndex;

		fileInput >> inputType;
		fileInput >> v1 >> v2 >> v3;

		verticesIndex.push_back(v1);
		verticesIndex.push_back(v2);
		verticesIndex.push_back(v3);

		face *faceNew = newFace(NULL);
		faceList.push_back(faceNew);

		for (int i = 0; i < 3; i++) {
			Vertex *startV = vertexList[verticesIndex[i] - 1];
			Vertex *endV = vertexList[verticesIndex[(i + 1) % 3] - 1];

			w_edge *edgeNew = newWEdge(startV, endV);

			pair<int, int> p(verticesIndex[i], verticesIndex[(i + 1) % 3]);

			edgeList[p] = edgeNew;

			faceNew->edge = edgeNew;
			edgeNew->left = faceNew;
			startV->edge = edgeNew;
		}

		for (int i = 0; i < 3; i++) {

			pair<int, int> E(verticesIndex[i], verticesIndex[(i + 1) % 3]);
			pair<int, int> Eprev(verticesIndex[(i - 1 + 3) % 3], verticesIndex[i]);
			pair<int, int> Enext(verticesIndex[(i + 1) % 3], verticesIndex[(i + 2) % 3]);
			pair<int, int> Ereverse(verticesIndex[(i + 1) % 3], verticesIndex[i]);

			w_edge *edge = edgeList.find(E)->second;
			w_edge *edge_prev = edgeList.find(Eprev)->second;
			w_edge *edge_next = edgeList.find(Enext)->second;

			edge->left_next = edge_next;
			edge->left_prev = edge_prev;

			if (edgeList.find(Ereverse) != edgeList.end()) {
				w_edge *edge2 = edgeList.find(Ereverse)->second;

				edge->right = edge2->left;
				edge->right_next = edge2->left_next;
				edge->right_prev = edge2->left_prev;

				edge2->right = edge->left;
				edge2->right_next = edge->left_next;
				edge2->right_prev = edge->left_prev;
			}
		}

		count++;
	}

	fileInput.close();

	calculateFaceNormal();
	calculateVertexNormal();
	calculateProxy();

	cout << "Import Summary: Edge count - " << edgeList.size() / 2
			<< " Face count - " << faceList.size() << endl;
}

/**
 *  method for loading mapping file generated in pre-processing step.
 *
 *  @param  mapping filename
 *  @return void
 */
void loadMappingFile(string fileName)
{
	mapping.clear();
	revMapping.clear();
	
	fileName.erase(std::remove(fileName.begin(), fileName.end(), '\n'), fileName.end());

        ifstream infile(fileName.c_str());

        if (!infile.is_open()) {
            cout << "File open issue." << endl;
            return;
        }

    	string line;
    	while (getline(infile, line)) {
		if (line.find_first_not_of(' ') == string::npos) continue;    // If is blank line
		vector<int> index;

		processStringForInt(line, ' ', index);

        int size = (int)index.size();
		mapping[index[0]] = index[1];
		revMapping[index[1]] = index[0];
    	}
}
