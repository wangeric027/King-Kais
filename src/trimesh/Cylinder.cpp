#include "Cylinder.h"
#include <glm/gtc/constants.hpp>

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}


// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cylinder::makeCapTile(glm::vec3 topLeft,
                           glm::vec3 topRight,
                           glm::vec3 bottomLeft,
                           glm::vec3 bottomRight, bool top) {


    glm::vec3 normal = glm::cross(topLeft - bottomRight, bottomLeft - bottomRight);
    normal = glm::normalize(normal);
    if (top) {
        normal = -normal;
        insertVec3(m_vertexData, topLeft);
        insertVec3(m_vertexData, normal);
        insertUVCap(m_vertexData, topLeft,  top);
        insertVec3(m_vertexData, topRight);
        insertVec3(m_vertexData, normal);
        insertUVCap(m_vertexData, topRight, top);
        insertVec3(m_vertexData, bottomRight);
        insertVec3(m_vertexData, normal);
        insertUVCap(m_vertexData, bottomRight, top);
        insertVec3(m_vertexData, topLeft);
        insertVec3(m_vertexData, normal);
        insertUVCap(m_vertexData, topLeft,  top);
        insertVec3(m_vertexData, bottomRight);
        insertVec3(m_vertexData, normal);
        insertUVCap(m_vertexData, bottomRight, top);
        insertVec3(m_vertexData, bottomLeft);
        insertVec3(m_vertexData, normal);
        insertUVCap(m_vertexData, bottomLeft, top);
        return;
    }
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertUVCap(m_vertexData, topLeft,  top);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal);
    insertUVCap(m_vertexData, bottomLeft, top);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    insertUVCap(m_vertexData, bottomRight, top);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertUVCap(m_vertexData, topLeft,  top);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    insertUVCap(m_vertexData, bottomRight, top);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal);
    insertUVCap(m_vertexData, topRight, top);
}


void Cylinder::makeSlopeTile(glm::vec3 topLeft,
                             glm::vec3 topRight,
                             glm::vec3 bottomLeft,
                             glm::vec3 bottomRight) {

    if (topLeft == topRight){
        glm::vec BLN = calcNorm(bottomLeft);
        glm::vec BRN = calcNorm(bottomRight);
        glm::vec TLN = glm::normalize((BLN + BRN)/2.0f);
        insertVec3(m_vertexData, topLeft);
        insertVec3(m_vertexData, TLN);
        insertUVSide(m_vertexData, topLeft);
        insertVec3(m_vertexData, bottomLeft);
        insertVec3(m_vertexData, BLN);
        insertUVSide(m_vertexData, bottomLeft);
        insertVec3(m_vertexData, bottomRight);
        insertVec3(m_vertexData, BRN);
        insertUVSide(m_vertexData, bottomRight);
        return;
    }

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, calcNorm(topLeft));
    insertUVSide(m_vertexData, topLeft);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, calcNorm(bottomLeft));
    insertUVSide(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, calcNorm(bottomRight));
    insertUVSide(m_vertexData, bottomRight);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, calcNorm(topLeft));
    insertUVSide(m_vertexData, topLeft);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, calcNorm(bottomRight));
    insertUVSide(m_vertexData, bottomRight);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, calcNorm(topRight));
    insertUVSide(m_vertexData, topRight);
}

glm::vec3 Cylinder::calcNorm(glm::vec3& pt) {



    float xNorm = (2 * pt.x);
    float yNorm = 0;
    float zNorm = (2 * pt.z);

    return glm::normalize(glm::vec3{ xNorm, yNorm, zNorm });
}

void Cylinder::makeCapSlice(float currentTheta, float nextTheta, bool top){

    std::vector<glm::vec3> listOfTopLefts;
    std::vector<glm::vec3> listOfTopRights;
    std::vector<glm::vec3> listOfBottomLefts;
    std::vector<glm::vec3> listOfBottomRights;

    float radius = -0.5f;
    if (top) {
        radius = 0.5f;
    }

    float radiusStep = 0.5f / m_param1;
    for (int i = 0; i < m_param1; i ++){
        float TLx = radiusStep * i * glm::cos(currentTheta);
        float TLy = radius;
        float TLz = radiusStep * i *  glm::sin(currentTheta);
        listOfTopLefts.push_back(glm::vec3{TLx, TLy, TLz});

        float TRx = radiusStep * i * glm::cos(nextTheta);
        float TRy = radius;
        float TRz = radiusStep * i * glm::sin(nextTheta);
        listOfTopRights.push_back(glm::vec3{TRx, TRy, TRz});

        float BLx = radiusStep * (i + 1) * glm::cos(currentTheta);
        float BLy = radius;
        float BLz = radiusStep * (i + 1) * glm::sin(currentTheta);
        listOfBottomLefts.push_back(glm::vec3{BLx, BLy, BLz});

        float BRx = radiusStep * (i + 1) * glm::cos(nextTheta);
        float BRy = radius;
        float BRz = radiusStep * (i + 1) * glm::sin(nextTheta);
        listOfBottomRights.push_back(glm::vec3{BRx, BRy, BRz});
    }

    for (int k = 0; k < listOfBottomLefts.size(); k ++){
        makeCapTile(listOfTopLefts[k], listOfTopRights[k], listOfBottomLefts[k], listOfBottomRights[k], top);
    }
    // Task 8: create a slice of the cap face using your
    //         make tile function(s)
    // Note: think about how param 1 comes into play here!
}

void Cylinder::makeSlopeSlice(float currentTheta, float nextTheta){
    // Task 9: create a single sloped face using your make
    //         tile function(s)
    // Note: think about how param 1 comes into play here!

    std::vector<glm::vec3> listOfTopLefts;
    std::vector<glm::vec3> listOfTopRights;
    std::vector<glm::vec3> listOfBottomLefts;
    std::vector<glm::vec3> listOfBottomRights;
    float heightStep = 1.0f / m_param1;


    for (int i = 0; i < m_param1; i ++){

        float TLx = 0.5 * glm::cos(currentTheta);
        float TLy = -0.5 + heightStep * (i + 1);
        float TLz = -0.5 * glm::sin(currentTheta);
        listOfTopLefts.push_back(glm::vec3{TLx, TLy, TLz});

        float TRx = 0.5 * glm::cos(nextTheta);
        float TRy = -0.5 + heightStep * (i + 1);
        float TRz = -0.5 *  glm::sin(nextTheta);
        listOfTopRights.push_back(glm::vec3{TRx, TRy, TRz});

        float BLx = 0.5 * glm::cos(currentTheta);
        float BLy = -0.5 + heightStep * i;
        float BLz = -0.5 * glm::sin(currentTheta);
        listOfBottomLefts.push_back(glm::vec3{BLx, BLy, BLz});

        float BRx = 0.5 * glm::cos(nextTheta);
        float BRy = -0.5 + heightStep * i;
        float BRz = -0.5 * glm::sin(nextTheta);
        listOfBottomRights.push_back(glm::vec3{BRx, BRy, BRz});
    }

    for (int k = 0; k < listOfBottomLefts.size(); k ++){
        makeSlopeTile(listOfTopLefts[k], listOfTopRights[k], listOfBottomLefts[k], listOfBottomRights[k]);
    }

}

void Cylinder::makeWedge(float currentTheta, float nextTheta) {
    makeCapSlice(currentTheta, nextTheta, true);
    makeCapSlice(currentTheta, nextTheta, false);
    makeSlopeSlice(currentTheta, nextTheta);
}

void Cylinder::setVertexData() {
    float thetaStep = glm::radians(360.f / m_param2);
    for (int i = 0; i < m_param2; i++){
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
    // Task 10: create a full cone using the makeWedge() function you
    //          just implemented
    // Note: think about how param 2 comes into play here!
}


void Cylinder::insertUVCap(std::vector<float> &data, glm::vec3 position, bool top){
    float u = position[0];
    float v = position[2];
    if (top){
        v = -v;
    }
    u += 0.5f;
    v += 0.5f;

    data.push_back(u);
    data.push_back(v);
}

void Cylinder::insertUVSide(std::vector<float> &data, glm::vec3 position){
    float u = 0;
    float theta = atan2(position[2], position[0]);
    if (theta < 0){
        u = -theta/(2.0 * glm::pi<float>());
    } else {
        u = 1 - theta/(2.0 * glm::pi<float>());
    }
    float v = position[1] + 0.5f; //y value
    data.push_back(u);
    data.push_back(v);
}

