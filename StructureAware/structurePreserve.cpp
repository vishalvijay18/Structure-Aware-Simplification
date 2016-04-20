#include "structurePreserve.h"
#include <iostream>
#include <fstream>
#include <sstream>

#define MAX_ITERATION 600000
#define SHAPE_PRESERVE_MAX_ITERATION 100000
#define uParameter 0.5f

// Decimation flags
int shapePreserveDecimationFlag = 0;
int shapePreserveFlag = 0;
int decimationFlag = 0;
int modifyCost = 0;


/**
 * Calculate planar proxies equation
 */
void CalculatePlanarProxyEquations()
{
	//proxyEq.clear();
	for (auto f: faceList)
	{
		if (f->displayFlag == 1)
		{
			return;
		}

		int prox = f->proxy;
		if(proxyEq.find(prox) == proxyEq.end())
		{
			matrix4 *Eq = new matrix4(0.0);
			*(Eq) = *(Eq) + *(f->quadMat);
			proxyEq[prox] = Eq;
		}
	}
}
/**
 *  Update quadric of face
 */
void updateQuadricOfFace(face *f, float a, float b, float c, float d)
{
    if (isnan(a))
    {
        a = 0; b = 0; c = 0; d = 0;
    }

    if (f->quadMat != nullptr)
    	delete(f->quadMat);

    f->quadMat = new matrix4( a * a, a * b, a * c, a * d, a * b, b * b, b * c, b * d, a * c, b * c, c * c, c * d, a * d, b * d, c * d, d * d);
}
/**
 * Get vertices of given face
 *
 * @param face* : face of mesh
 * @result : vector of vertices
 *
 */
vector<vector3> getFaceCoordinates(face* f)
{
	vector<vector3> coords;
	w_edge *edg = f->edge;

	if (edg->left != f)
	{
    	edg = edg->right_next->left_prev;
    }

	coords.push_back(vector3(edg->start->x, edg->start->y, edg->start->z));
	coords.push_back(vector3(edg->end->x, edg->end->y, edg->end->z));
	coords.push_back(vector3(edg->left_next->end->x, edg->left_next->end->y, edg->left_next->end->z));

	return coords;
}

/**
 * Given a face, it will update its quadric value based on all its three vertices
 */
void computeQuadricForFace(face *f)
{
	vector<vector3> coords;
    if (f->displayFlag == 1)
    	return;

    coords = getFaceCoordinates(f);

    vector3 crossProduct = glm::normalize(glm::cross(coords[2] - coords[0], coords[1] - coords[0]));

    updateQuadricOfFace(f, crossProduct.x, crossProduct.y, crossProduct.z, -(coords[0].x * crossProduct.x + coords[0].y * crossProduct.y + coords[0].z * crossProduct.z));
}

/*
 * It will populate quadric of all  faces.
 */
void computeQuadricForAllFaces()
{
    for (auto f: faceList)
    {
        if (f->displayFlag == 1)
        	return;

        computeQuadricForFace(f);
    }
}
/**
 * For vertex quadric
 */
void computeQuadricForVertex(Vertex *ver)
{
    if (ver->displayFlag == 1)
    {
    	return;
    }

    if (ver->edge == nullptr)
    {
    	return;
   	}

    if (ver->quadMat != nullptr)
    {
    	delete(ver->quadMat);
    }

    if (!ver->proxies.empty())
    {
    	ver->proxies.clear();
    }

    ver->quadMat = new matrix4(0.0);
    w_edge *edg0 = ver->edge;
    w_edge *edg1 = edgeList[make_pair(indexMap[ver->edge->end] + 1,
                                                 indexMap[ver->edge->start] + 1)];
    w_edge *edg = edg0;
    do {
        if (edg->end == ver)
        {
        	if(modifyCost == 0)
        	{
            	*(ver->quadMat) = *(ver->quadMat) + *(edg->left->quadMat);
        	}
        	else
        	{
        		*(ver->quadMat) = *(ver->quadMat) + ((*(edg->left->quadMat)) * (1.0f - uParameter)) + ((*(proxyEq[edg->left->proxy])) * uParameter);
        	}

        	if(std::find(ver->proxies.begin(), ver->proxies.end(), edg->left->proxy) != ver->proxies.end())
        	{
    	    	// do nothing
        	}
        	else
        	{
        		ver->proxies.push_back(edg->left->proxy);
        	}

            	edg = edg->right_prev;
        }
        else
        {
        	if(modifyCost == 0)
        	{
            	*(ver->quadMat) = *(ver->quadMat) + *(edg->right->quadMat);
        	}
        	else
        	{
            	*(ver->quadMat) = *(ver->quadMat) + ((*(edg->right->quadMat)) * (1.0f - uParameter)) + ((*(proxyEq[edg->right->proxy])) * uParameter);
        	}

        	if(std::find(ver->proxies.begin(), ver->proxies.end(), edg->right->proxy) != ver->proxies.end())
        	{
    	    		// do nothing
            }
        	else
        	{
        		ver->proxies.push_back(edg->right->proxy);
        	}

        	edg = edg->left_prev;
        }
    } while (edg != edg0 && edg != edg1);
}

/**
 * Populate quadrics of all vertices
 */
void computeQuadricForAllVertices(void)
{
    int counter = 0;

    for (auto ver : vertexList)
    {
        if (ver->displayFlag == 1)
        	return;

        computeQuadricForVertex(ver);

        counter++;
    }
    //cout << counter << endl;
}
/**
 * Area computation of given face, which will be used in
 */
float computeFaceArea(face *f)
{
	vector<vector3> coords;

	coords = getFaceCoordinates(f);

    vector3 crossProduct = glm::cross(coords[2] - coords[0], coords[1] - coords[0]);

    float a = crossProduct.x;
    float b = crossProduct.y;
    float c = crossProduct.z;

    if (isnan(a))
    {
        a = 0; b = 0; c = 0;
    }

    float area = sqrt(a*a + b*b + c*c)/2.0;

    return area;
}

matrix4 *FindQuadricForOrthogonalPlane(face *f, w_edge *pair)
{
	vector<vector3> coords;

	coords = getFaceCoordinates(f);

    vector3 cross1 = glm::normalize(glm::cross(coords[2] - coords[0], coords[1] - coords[0]));

    //first we calculated cross1 which is normal of the face, then we use this cross1 and the pair edge to find normal to orthogonal plane
    vector3 ver1 = vector3(pair->start->x, pair->start->y, pair->start->z);
    vector3 ver2 = vector3(pair->end->x, pair->end->y, pair->end->z);
    vector3 cross = glm::normalize(glm::cross(cross1 - ver1, ver2 - ver1));

    float a = cross.x;
    float b = cross.y;
    float c = cross.z;
    float d = -(ver1.x * a + ver1.y * b + ver1.z * c);

    if (isnan(a))
    {
        a = 0; b = 0; c = 0; d = 0;
    }
    
    matrix4 *QOrtho = new matrix4(
            a * a, a * b, a * c, a * d, a * b, b * b, b * c, b * d, a * c, b * c, c * c, c * d, a * d, b * d, c * d, d * d
    );

    return QOrtho;
}

// Q for boundary edge
matrix4 *computeQuadricForBoundary(w_edge *pair)
{
	matrix4 *QBdry = new matrix4(0.0);

	if(pair->right->proxy != pair->left->proxy)
	{
		float area1 = computeFaceArea(pair->left);
    	float area2 = computeFaceArea(pair->right);

		matrix4 *QOrtho1 = FindQuadricForOrthogonalPlane(pair->left, pair);
		matrix4 *QOrtho2 = FindQuadricForOrthogonalPlane(pair->right, pair);

		*(QBdry) = ((*QOrtho1) * area1) + ((*QOrtho2) * area2);
	}

	return QBdry;
	
}
/**
 * If matrix is invertible, then use midpoint.
 * Utility method for computeVertexLocation
 */
vector4 computeIntermediateVertex(w_edge *pair)
{
	matrix4 Q = *(pair->start->quadMat) + *(pair->end->quadMat);

	vector4 v1 = vector4(pair->start->x, pair->start->y, pair->start->z, 1.0);
    vector4 v2 = vector4(pair->end->x, pair->end->y, pair->end->z, 1.0);
    vector4 vm = vector4((v1.x + v2.x) / 2, (v1.y + v2.y) / 2, (v1.z + v2.z) / 2, 1.0);

	if(pair->start->proxies.size() > pair->end->proxies.size())
		return v1;
	else if(pair->start->proxies.size() < pair->end->proxies.size())
		return v2;

    vector4 costv1 = v1 * Q * v1;
    vector4 costv2 = v2 * Q * v2;
    vector4 costvm = vm * Q * vm;

    float cost1 = costv1.x + costv1.y + costv1.z + costv1.w;
    float cost2 = costv2.x + costv2.y + costv2.z + costv2.w;
    float costm = costvm.x + costvm.y + costvm.z + costvm.w;

    if (cost1 <= cost2 && cost1 <= costm)
    	return v1;
    if (cost2 <= cost1 && cost2 <= costm)
    	return v2;

    	return vm;
}

/**
 *  This will compute final vertex location after edge decimation.
 *
 *  @param *edg : Edge to decimate
 *  @return vertex
 */
vector4 computeCandidateVertex(w_edge *edg)
{
    matrix4 Q = *(edg->start->quadMat) + *(edg->end->quadMat);

    //This algo didn't provide good results
    /*matrix4 Q1 = ((*(pair->left->K)) * 0.5f) + ((*(proxyEq[pair->left->proxy])) * 0.5f);
    matrix4 Q2 = ((*(pair->right->K)) * 0.5f) + ((*(proxyEq[pair->right->proxy])) * 0.5f);
    float area1 = computeFaceArea(pair->left);
    float area2 = computeFaceArea(pair->right);

    matrix4 Q = ((Q1 * area1 + Q2 * area2) * 1.0f) + ((*computeQuadricForBoundary(pair)) * 0.8f);*/
    
    if(edg->start->proxies.size() >=3 && edg->end->proxies.size() < 3)
    {
		return vector4(edg->start->x, edg->start->y, edg->start->z, 1.0);
    }
    else if(edg->end->proxies.size() >=3 && edg->start->proxies.size() < 3)
    {
		return vector4(edg->end->x, edg->end->y, edg->end->z, 1.0);;
    }

    matrix4 tmp = Q;
    tmp[3] = vector4(0.0f, 0.0f, 0.0f, 1.0f);
    matrix4 inverseQ = glm::inverse(tmp);

    if (isnan(inverseQ[0][0]))
    {
        //std::cout << "using intermediate vertex" << std::endl;
        return computeIntermediateVertex(edg);
    }
    else
    {
    	//cout << "Wrong inverse\n";
    	vector4 newV = vector4(0.0f, 0.0f, 0.0f, 1.0f) * inverseQ;
	
    	if(isnan(newV.x) || isnan(newV.y) || isnan(newV.z) || abs(newV.x) > 10.0*(abs(edg->start->x)) || abs(newV.y) > 10.0*(abs(edg->start->y)) || abs(newV.z) > 10.0*(abs(edg->start->z)))
    	{
    		//cout << newV.x << " " << newV.y << " " << newV.z << endl;
    		return computeIntermediateVertex(edg);
    	}
    	return newV;
    }
 }

/**
 * Given an edge, it will compute its decimation cost and set to appropriate
 */
void computePairCost(w_edge *edg)
{
    matrix4 Q = *(edg->start->quadMat) + *(edg->end->quadMat);
    //@todo This algorithm didn't provide good results
    /*matrix4 Q1 = ((*(pair->left->K)) * 0.5f) + ((*(proxyEq[pair->left->proxy])) * 0.5f);
    matrix4 Q2 = ((*(pair->right->K)) * 0.5f) + ((*(proxyEq[pair->right->proxy])) * 0.5f);

    matrix4 Q = ((Q1 * area1 + Q2 * area2) * 1.0f) + ((*computeQuadricForBoundary(pair)) * 0.8f);*/

    vector4 v = computeCandidateVertex(edg);
    vector4 costv = v * Q * v;
    edg->decimationCost = (costv.x + costv.y + costv.z + costv.w);

    if(isnan(edg->decimationCost))
	edg->decimationCost = 0;

    if(shapePreserveFlag == 1)
    {
    	if(edg->right->proxy != edg->left->proxy)
    	{
    		float area1 = computeFaceArea(edg->left);
    		float area2 = computeFaceArea(edg->right);
    		edg->decimationCost += (area1 + area2);
    	}
    }
    // @todo
    /*int mismatch = 0;
    if(pair->start->proxies.size() >= pair->end->proxies.size()) {
	for (auto i: pair->start->proxies) {
		if(std::find(pair->end->proxies.begin(), pair->end->proxies.end(), i) == pair->end->proxies.end())
			mismatch++;
	}	
    	//pair->cost += 10*mismatch;
    }
    else {
	for (auto i: pair->end->proxies) {
		if(std::find(pair->start->proxies.begin(), pair->start->proxies.end(), i) == pair->start->proxies.end())
			mismatch++;
	}	
    	//pair->cost += 10*mismatch;
    }*/


    //Graph connectivity
    if(shapePreserveFlag == 1)
    {
    	int mismatchStart = 0; int mismatchEnd = 0;
    	for (auto i: edg->start->proxies)
    	{

    		if(std::find(edg->end->proxies.begin(), edg->end->proxies.end(), i) == edg->end->proxies.end())
			mismatchStart++;
    	}

    	for (auto l: edg->end->proxies)
    	{
    		if(std::find(edg->start->proxies.begin(), edg->start->proxies.end(), l) == edg->start->proxies.end())
			mismatchEnd++;
    	}

    	if(mismatchStart > 0 && mismatchEnd > 0)
    	{
    		int min = (mismatchStart > mismatchEnd ? mismatchEnd : mismatchStart);
    		edg->decimationCost += 10*min;
    	}
    }

    //cout << "cost: " << pair->cost << endl;
}

/**
 *
 */

bool validateEdgeforCollapse(w_edge *edg)
{
    std::set<Vertex *> vertex_set;
    Vertex *start = edg->start;
    Vertex *end = edg->end;
    Vertex *up = edg->left_prev->start;
    Vertex *down = edg->right_next->end;

    w_edge *e0 = start->edge;
    w_edge *e1 = edgeList[make_pair(indexMap[start->edge->end] + 1, indexMap[start->edge->start] + 1)];
    w_edge *edge = e0;

    do {
        if (edge->end == start)
        {
            if (edge->start != end && edge->start != up && edge->start != down)
                vertex_set.insert(edge->start);

            edge = edge->right_prev;
        }
        else
        {
            if (edge->end != end && edge->end != up && edge->end != down)
                vertex_set.insert(edge->end);

            edge = edge->left_prev;
        }
    } while (edge != e0 && edge != e1);

    if(vertex_set.size() == 0)
    {
    	return false;
    }

    e0 = end->edge;
    e1 = edgeList[make_pair(indexMap[end->edge->end] + 1, indexMap[end->edge->start] + 1)];
    edge = e0;
    do {
        if (edge->end == end) {
            if (vertex_set.find(edge->start) != vertex_set.end())
                return false;
            edge = edge->right_prev;
        } else {
            if (vertex_set.find(edge->end) != vertex_set.end())
                return false;
            edge = edge->left_prev;
        }
    } while (edge != e0 && edge != e1);

    if(shapePreserveFlag == 1)
    {
    	if(proxyCount[edg->left->proxy] <= 2 || proxyCount[edg->right->proxy] <= 2)
    	{
    		int counting = 0;

    		for(auto c : proxyCount)
    		{
    			if(c.second > 2)
    				counting++;

    			if(counting >1)
    				return false;
    		}
    	}
    }

    if(shapePreserveDecimationFlag == 1)
    {
    	if(start->proxies.size() != end->proxies.size())
    		return false;

    	for (auto i: start->proxies)
    	{
    		if(std::find(end->proxies.begin(), end->proxies.end(), i) != end->proxies.end())
    		{
    	    		// do nothing
    		}
    		else
    		{
    			return false;
    		}
    	}
    }

    return true;
}

/**
 *  @param k =  number of edges choice
 *
 *  @return w_edge
 *  		Edge to decimate with least cost.
 */
w_edge *getKEdges(int k)
{
    srand((unsigned int)time(0));

    if (k > (int)edgeList.size())
        k = (int)edgeList.size();

    vector<w_edge *> choosenPairs;
    w_edge *edge;

    for (int i = 0; i < k; i++)
    {
        int v1, v2;
        int iterations = 0;
        do {
            iterations++;
            if (iterations >= MAX_ITERATION)
            {
                decimationFlag = 1;
                break;
            }
            v1 = rand() % vertexList.size() + 1;
            v2 = rand() % vertexList.size() + 1;

            if (v1 == v2)
            {
                iterations--;
                continue;
            }

            if (edgeList.find(make_pair(v1, v2)) != edgeList.end())
            {
                edge = edgeList[make_pair(v1, v2)];
                if (!validateEdgeforCollapse(edge))
                {
                   continue;
                }
                break;
            }
        } while (true);

        if (decimationFlag != 1)
        {
            computePairCost(edge);
            choosenPairs.push_back(edge);
        }
        else
        {
            break;
        }
    }

    w_edge *least_cost_pair;
    if (choosenPairs.empty())
    	return nullptr;

    least_cost_pair = choosenPairs[0];

    for (int i = 1; i < (int)choosenPairs.size(); i++)
    {
        if (choosenPairs[i]->decimationCost < least_cost_pair->decimationCost)
        {
            least_cost_pair = choosenPairs[i];
        }
    }

    // Return least cost pair
    return least_cost_pair;
}

/**
 *  Rearrange nearby faces of decimating edge.
 */
void collapseEdge(w_edge *candidateEdge)
{
    Vertex *v1 = candidateEdge->start;
    Vertex *v2 = candidateEdge->end;
    Vertex *v_up = candidateEdge->left_prev->start;
    Vertex *v_down = candidateEdge->right_next->end;

    int v1_index = indexMap[v1] + 1;
    int v2_index = indexMap[v2] + 1;
    int v_up_index = indexMap[v_up] + 1;
    int v_down_index = indexMap[v_down] + 1;

    vector4 v = computeCandidateVertex(candidateEdge);

    w_edge *edgel0 = candidateEdge->left_prev->right_next->left_prev;
    w_edge *edgel1 = candidateEdge->right_next->right_next->left_prev;
    w_edge *edger0 = candidateEdge->left_next->right_next->left_prev;
    w_edge *edger1 = candidateEdge->right_prev->right_next->left_prev;

    v1->x = v.x;
    v1->y = v.y;
    v1->z = v.z;
    v2->x = v.x;
    v2->y = v.y;
    v2->z = v.z;

    //cout << v.x << " " << v.y << " " << v.z << endl;

    v1->edge = edgel0;
    v_up->edge = edger0;
    v_down->edge = edgel1;

    edgel0->right = edger0->left;
    edgel1->right = edger1->left;
    edger0->right = edgel0->left;
    edger1->right = edgel1->left;

    candidateEdge->left->displayFlag = 1;
    candidateEdge->right->displayFlag = 1;

    proxyCount[candidateEdge->left->proxy] -= 1;
    proxyCount[candidateEdge->right->proxy] -= 1;
    //cout << proxyCount[pair->left->proxy] << " and " << proxyCount[pair->right->proxy] << endl;

    v2->displayFlag = 1;

    w_edge *e0 = edgeList[make_pair(v2_index, v1_index)];
    w_edge *e1 = candidateEdge;
    w_edge *edge = e0;
    do {
        if (edge->end == v2)
        {
            w_edge *next_edge = edge->right_prev;
            int start_index = indexMap[edge->start] + 1;
            edge->end = v1;
            edge->right_next->left_prev->start = v1;

            if (start_index == v_down_index)
            {
                delete edgeList[make_pair(v1_index, v_down_index)];
                edgeList[make_pair(v1_index, v_down_index)] = edge->right_next->left_prev;
                delete edge;
            }
            else if (start_index == v_up_index)
            {
                delete edgeList[make_pair(v_up_index, v1_index)];
                edgeList[make_pair(v_up_index, v1_index)] = edge;
                delete edge->right_next->left_prev;
            }
            else
            {
                edgeList[make_pair(start_index, v1_index)] = edge;
                edgeList[make_pair(v1_index, start_index)] = edge->right_next->left_prev;
            }

            edgeList.erase(make_pair(start_index, v2_index));
            edgeList.erase(make_pair(v2_index, start_index));
            edge = next_edge;
        }
        else
        {
            delete edgeList[make_pair(v1_index, v2_index)];
            delete edgeList[make_pair(v2_index, v1_index)];
            edgeList.erase(make_pair(v1_index, v2_index));
            edgeList.erase(make_pair(v2_index, v1_index));
            edge = edge->left_prev;
        }
    } while (edge != e0 && edge != e1);

    edgel0->right_next = edger0->left_next;
    edgel0->right_prev = edger0->left_prev;
    edgel1->right_next = edger1->left_next;
    edgel1->right_prev = edger1->left_prev;
    edger0->right_next = edgel0->left_next;
    edger0->right_prev = edgel0->left_prev;
    edger1->right_next = edgel1->left_next;
    edger1->right_prev = edgel1->left_prev;

    edgel0->left->edge = edgel0;
    edgel1->left->edge = edgel1;
    edger0->left->edge = edger0;
    edger1->left->edge = edger1;

    // Update proxies of new vertex
    for (auto prox: v2->proxies)
    {
		if(std::find(v1->proxies.begin(), v1->proxies.end(), prox) == v1->proxies.end())
			v1->proxies.push_back(prox);
    }

    v1->quadMat = new matrix4(*(v1->quadMat) + *(v2->quadMat));
    e0 = v1->edge;
    e1 = edgeList[make_pair(indexMap[v1->edge->end] + 1, indexMap[v1->edge->start] + 1)];
    edge = e0;
    do {
        if (edge->end == v1)
        {
            computeQuadricForFace(edge->left);
            
            if(shapePreserveFlag == 1)
            	modifyCost = 1;
        
            computeQuadricForVertex(edge->start);
		
            modifyCost = 0;
            edge = edge->right_prev;
        }
        else
        {
            computeQuadricForFace(edge->right);

            if(shapePreserveFlag == 1)
            	modifyCost = 1;

            computeQuadricForVertex(edge->end);

            modifyCost = 0;
            edge = edge->left_prev;
        }
    } while (edge != e0 && edge != e1);
}

w_edge* searchEdge(int v1, int v2)
{
	w_edge* edge;
	if (edgeList.find(make_pair(v1, v2)) != edgeList.end())
	{
		edge = edgeList[make_pair(v1, v2)];
		if (validateEdgeforCollapse(edge))
		{
			return edge;
		}
	}
	else if(edgeList.find(make_pair(v2, v1)) != edgeList.end())
	{
		edge = edgeList[make_pair(v2, v1)];
		if (validateEdgeforCollapse(edge))
		{
			return edge;
		}
	}

	return NULL;
}
/**
 * For testing and logging purpose
 */
void printCurrentInfo()
{
	cout << "Current Edge count: " << edgeList.size()/2 << endl;

	int faceCounter = 0;
	for( auto f: faceList)
	{
		if(f->displayFlag != 1)
			faceCounter++;
	}
	cout << "Current Face count: " << faceCounter << endl;
}
/**
 * For given v1 and v2, find correspondence from mapping file
 *
 */
bool checkMappingEdgeForDecimation(int v1, int v2)
{
	if(mapping.size() > 0)
	{
		if(mapping.find(v1) != mapping.end() && mapping.find(v2) != mapping.end())
		{
			int vpair1 = mapping[v1];
			int vpair2 = mapping[v2];
			
			w_edge *pair1 = searchEdge(vpair1,vpair2);
			
			if(pair1 != NULL)
			{
				collapseEdge(pair1);
				return true;			
			}
		}
		else if(revMapping.find(v1) != revMapping.end() && revMapping.find(v2) != revMapping.end())
		{
			int vpair1 = revMapping[v1];
			int vpair2 = revMapping[v2];
			
			w_edge *pair1 = searchEdge(vpair1,vpair2);
			
			if(pair1 != NULL)
			{
				collapseEdge(pair1);
				return true;			
			}
		}
	}

	return false;
}

/**
 * This is for testing and doing extreme simplification using GO button in our UI.
 */

void guaranteedShapePreserve(int k)
{
	decimationFlag = 0;
	shapePreserveDecimationFlag = 1;
	shapePreserveFlag = 1;

	computeQuadricForAllFaces();

	CalculatePlanarProxyEquations();

	computeQuadricForAllVertices();

	while(true)
	{
		if (edgeList.size() / 2 <= 6)
		{
			break;
		}

        w_edge *pair = getKEdges(k);

        if (pair == nullptr || decimationFlag == 1)
		{
			break;
		}

        int v1 = indexMap[pair->start] + 1;
		int v2 = indexMap[pair->end] + 1;       

		collapseEdge(pair);

		checkMappingEdgeForDecimation(v1, v2);
	}

	calculateFaceNormal();
    calculateVertexNormal();
    shapePreserveDecimationFlag = 0;
	shapePreserveFlag = 1;
	printCurrentInfo();
}

/**
 * This method is implementation of multiple choice decimation scheme.
 *
 * @params k = random size input for MCS algorithm
 * 		   target = number of edge to be deleted
 * @ return
 */
void multipleChoiceDecimation(int k, int target)
{
    decimationFlag = 0;
    shapePreserveFlag = 0;
    shapePreserveDecimationFlag = 0;

    computeQuadricForAllFaces();
    //CalculatePlanarProxyEquations();
    computeQuadricForAllVertices();

    for (int i = 0; i < target; i++)
    {
        if (edgeList.size() / 2 <= 6)
        	break;

        w_edge *pair = getKEdges(k);

        if (pair == nullptr || decimationFlag == 1)
        	break;

        int v1 = indexMap[pair->start] + 1;
        int v2 = indexMap[pair->end] + 1;

        collapseEdge(pair);
	/*if(checkMappingEdgeForDecimation(v1, v2))
		i++;  */
	}

    calculateFaceNormal();
    calculateVertexNormal();
    printCurrentInfo();
}
/**
 * Implementation of structure preservation decimation algorithm for
 * maintaining symmetries, corners and boundaries of given mesh.
 *
 * This algorithm will combine MCS approach (local quadric error) and proxy based quadric(global error)
 * to choose candidate edge. Also, include some rules to preserve boundaries and corners.
 *
 * @params k = Given random size for MCS algorithm
 * 		   target = number of edge to be deleted
 * @ return
 */
void shapePreserveDecimation(int k, int target)
{
	int i=0;
	int runCount=2;

	computeQuadricForAllFaces();

	CalculatePlanarProxyEquations();

	computeQuadricForAllVertices();

	decimationFlag = 0;
	shapePreserveDecimationFlag = 1;
	shapePreserveFlag =1;

	while(runCount--)
	{
		for (i; i < target; i++)
		{
			if (edgeList.size() / 2 <= 6)
				break;

			w_edge *pair = getKEdges(k);

			if (pair == nullptr || decimationFlag == 1)
				break;

			int v1 = indexMap[pair->start] + 1;
			int v2 = indexMap[pair->end] + 1;

			collapseEdge(pair);

			if(checkMappingEdgeForDecimation(v1, v2))
				i++;
		}

		shapePreserveDecimationFlag = 0;
		decimationFlag = 0;

	}

	shapePreserveFlag = 0;
	
	calculateFaceNormal();
    	calculateVertexNormal();
	printCurrentInfo();
}

