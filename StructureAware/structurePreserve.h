#ifndef STRUCTUREPRESERVE_H
#define STRUCTUREPRESERVE_H

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <math.h>
#include "wingedEdge.h"

using namespace std;

extern map<Vertex *, int> indexMap;
extern vector<Vertex *> vertexList;
extern vector<face *> faceList;
extern map<pair<int, int>, w_edge *> edgeList;
extern map<int, int> mapping;
extern map<int, int> revMapping;
extern map<int, glm::mat4x4 *> proxyEq;
extern map<int, int> proxyCount;
extern void calculateVertexNormal();
extern void calculateFaceNormal();

/**
 * This method is implementation of multiple choice decimation scheme.
 *
 * @params k = random size input for MCS algorithm
 * 		   target = number of edge to be deleted
 * @ return
 */
void multipleChoiceDecimation(int k, int target);

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
void shapePreserveDecimation(int, int);

/**
 * This is for testing and doing extreme simplification using GO button in our UI.
 */
 void guaranteedShapePreserve(int);

#endif
