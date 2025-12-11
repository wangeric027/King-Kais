#include "Cube.h"
#include <iostream>

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight, int face) {
    glm::vec3 norm1 = glm::normalize(glm::cross(topLeft - bottomRight, bottomLeft - bottomRight));
    glm::vec3 norm2 = glm::normalize(glm::cross(topRight - bottomRight, topLeft - bottomRight));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, norm1);
    insertUV(m_vertexData, topLeft, face);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, norm1);
    insertUV(m_vertexData, bottomLeft, face);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, norm1);
    insertUV(m_vertexData, bottomRight, face);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, norm2);
    insertUV(m_vertexData, topLeft, face);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, norm2);
    insertUV(m_vertexData, bottomRight, face);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, norm2);
    insertUV(m_vertexData, topRight, face);
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight, int face) {
    // Task 3: create a single side of the cube out of the 4
    //         given points and makeTile()
    // Note: think about how param 1 affects the number of triangles on
    //       the face of the cube

    std::vector<glm::vec3> listOfTopLefts;
    std::vector<glm::vec3> listOfTopRights;
    std::vector<glm::vec3> listOfBottomLefts;
    std::vector<glm::vec3> listOfBottomRights;

    glm::vec3 topUnit = (topRight-topLeft)/ float(m_param1);
    glm::vec3 bottomUnit = (bottomLeft - topLeft)/ float(m_param1);
    for (int j = 0; j < m_param1; j ++){
        for (int i = 0; i < m_param1; i++){
            listOfTopLefts.push_back(topLeft + float(i) * topUnit + float(j) * bottomUnit);
            listOfTopRights.push_back(topLeft + float(i + 1) * topUnit + float(j) * bottomUnit);
            listOfBottomLefts.push_back(topLeft + float(i) * topUnit + float(j + 1) * bottomUnit);
            listOfBottomRights.push_back(topLeft + float(i + 1) * topUnit + float(j + 1) * bottomUnit);
        }
    }

    for (int k = 0; k < m_param1 * m_param1; k++){
        makeTile(listOfTopLefts[k], listOfTopRights[k], listOfBottomLefts[k], listOfBottomRights[k], face);
    }
}

void Cube::setVertexData() {
    // Uncomment these lines for Task 2, then comment them out for Task 3:

    // makeTile(glm::vec3(-0.5f,  0.5f, 0.5f),
    //          glm::vec3( 0.5f,  0.5f, 0.5f),
    //          glm::vec3(-0.5f, -0.5f, 0.5f),
    //          glm::vec3( 0.5f, -0.5f, 0.5f));

    // Uncomment these lines for Task 3:

    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f), 0);

    makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f), 1);

    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f,  0.5f), 2);

    makeFace(glm::vec3( 0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f), 3);

    makeFace(glm::vec3(-0.5f, 0.5f, -0.5f),
             glm::vec3( 0.5f, 0.5f, -0.5f),
             glm::vec3(-0.5f, 0.5f,  0.5f),
             glm::vec3( 0.5f, 0.5f,  0.5f), 4);

    makeFace(glm::vec3(-0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f), 5);


    // Task 4: Use the makeFace() function to make all 6 sides of the cube
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}


void Cube::insertUV(std::vector<float> &data, glm::vec3 position, int face) {
    float u = 0.f;
    float v = 0.f;

    switch (face) {
    case 0:
        u = position.x;
        v = position.y;
        break;
    case 1:
        u = -position.x;
        v = position.y;
        break;
    case 2:
        u = position.z;
        v = position.y;
        break;
    case 3:
        u = -position.z;
        v = position.y;
        break;
    case 4:
        u = position.x;
        v = -position.z;
        break;
    case 5:
        u = position.x;
        v = position.z;
        break;
    default:
        u = 0.f;
        v = 0.f;
        break;
    }

    u += 0.5f;
    v += 0.5f;

    data.push_back(u);
    data.push_back(v);
}
