#ifndef SMFPARSER_H
#define SMFPARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <math.h>
#include <cfloat>
#include "wingedEdge.h"

using namespace std;

extern map<Vertex *, int> indexMap;
extern vector<Vertex *> vertexList;    
extern vector<face *> faceList;       
extern map<pair<int, int>, w_edge *> edgeList;
extern map<int, int> mapping;
extern map<int, int> revMapping;
extern int meshImportFlag;
extern map<int, int> proxyCount;
extern std::map<face *, GLVector*> faceNormalMap;
extern std::map<Vertex *, GLVector*> vertexNormalMap;
extern void calculateVertexNormal();
extern void calculateFaceNormal();

GLVector* findVertexNormal(Vertex* );
/**
 *  for reading .smf file.
 *
 *  @param name of file
 *  @return void
 */
void loadMesh(string);

/**
 *  method for loading mapping file generated in pre-processing step.
 *
 *  @param  mapping filename
 *  @return void
 */
void loadMappingFile(string);
/**
 * For saving .SMF file.
 *
 * @param filename
 * @return void
 */
void saveMesh();

/**
 * Helper function for reading a .smf file.
 *
 */
string exec(const char*);

#endif
