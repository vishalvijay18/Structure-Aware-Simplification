#ifndef WINGEDEDGE_H
#define WINGEDEDGE_H
#include "lib/glm.hpp"
#include "lib/gtc/matrix_transform.hpp"
#include "lib/gtc/type_ptr.hpp"

struct W_edge {
    struct Vertex *start, *end;
    struct Face *left, *right;
    struct W_edge *left_prev, *right_prev;
    struct W_edge *left_next, *right_next;
    float decimationCost;
    int mismatchCount;
};

struct Vertex {
    struct W_edge *edge;
    float x, y, z;
    int displayFlag;
    glm::mat4x4 *quadMat;
    std::vector<int> proxies;
};

struct Face {
    struct W_edge *edge;
    int displayFlag;
    glm::mat4x4 *quadMat;
    int proxy;
};

struct GLVector
{
	float x, y, z;
};

typedef struct Vertex vertex;
typedef struct Face face;
typedef struct W_edge w_edge;
typedef struct GLVector glvector;
typedef glm::mat4x4 matrix4;
typedef glm::vec3 vector3;
typedef glm::vec4 vector4;

#endif
