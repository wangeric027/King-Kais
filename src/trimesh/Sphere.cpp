#include "Sphere.h"
#include <algorithm>
#include <glm/gtc/constants.hpp>

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(topLeft));
    insertUV(m_vertexData, topLeft);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, glm::normalize(bottomLeft));
    insertUV(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(bottomRight));
    insertUV(m_vertexData, bottomRight);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(topLeft));
    insertUV(m_vertexData, topLeft);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(bottomRight));
    insertUV(m_vertexData, bottomRight);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, glm::normalize(topRight));
    insertUV(m_vertexData, topRight);
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    // Task 6: create a single wedge of the sphere using the
    //         makeTile() function you implemented in Task 5
    // Note: think about how param 1 comes into play here!

    std::vector<glm::vec3> listOfTopLefts;
    std::vector<glm::vec3> listOfTopRights;
    std::vector<glm::vec3> listOfBottomLefts;
    std::vector<glm::vec3> listOfBottomRights;

    float phiStep = glm::radians(180.f/m_param1);
    for (int i = 0; i < m_param1; i ++){
        float TLx = 0.5f * glm::sin(phiStep * i) * glm::cos(currentTheta);
        float TLy = 0.5f * glm::cos(phiStep * i);
        float TLz = -0.5f * glm::sin(phiStep * i) * glm::sin(currentTheta);
        listOfTopLefts.push_back(glm::vec3{TLx, TLy, TLz});

        float TRx = 0.5f * glm::sin(phiStep * i) * glm::cos(nextTheta);
        float TRy = 0.5f * glm::cos(phiStep * i);
        float TRz = -0.5f * glm::sin(phiStep * i) * glm::sin(nextTheta);
        listOfTopRights.push_back(glm::vec3{TRx, TRy, TRz});

        float BLx = 0.5f * glm::sin(phiStep * (i + 1)) * glm::cos(currentTheta);
        float BLy = 0.5f * glm::cos(phiStep * (i + 1));
        float BLz = -0.5f * glm::sin(phiStep * (i + 1)) * glm::sin(currentTheta);
        listOfBottomLefts.push_back(glm::vec3{BLx, BLy, BLz});

        float BRx = 0.5f * glm::sin(phiStep * (i +1)) * glm::cos(nextTheta);
        float BRy = 0.5f * glm::cos(phiStep * (i + 1));
        float BRz = -0.5f * glm::sin(phiStep * (i + 1)) * glm::sin(nextTheta);
        listOfBottomRights.push_back(glm::vec3{BRx, BRy, BRz});
    }

    for (int k = 0; k < listOfBottomLefts.size(); k ++){
        makeTile(listOfTopLefts[k], listOfTopRights[k], listOfBottomLefts[k], listOfBottomRights[k]);
    }
}

void Sphere::makeSphere() {
    float thetaStep = glm::radians(360.f / m_param2);
    for (int i = 0; i < m_param2; i++){
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
    // Task 7: create a full sphere using the makeWedge() function you
    //         implemented in Task 6
    // Note: think about how param 2 comes into play here!
}

void Sphere::setVertexData() {
    // Uncomment these lines to make a wedge for Task 6, then comment them out for Task 7:


    // makeWedge(currentTheta, nextTheta);

    // Uncomment these lines to make sphere for Task 7:

    makeSphere();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}



void Sphere::insertUV(std::vector<float> &data, glm::vec3 position){
    float u = 0;
    float v = 0;
    float phi = glm::asin(position[1] / 0.5f);
    v = phi/glm::pi<float>() + 0.5f;
    if (v == 0 || v == 1){
        u = 0.5f;
        data.push_back(u);
        data.push_back(v);
        return;
    }
    float theta = atan2(position[2], position[0]);
    if (theta < 0){
        u = -theta/(2.0 * glm::pi<float>());
    } else {
        u = 1 - theta/(2.0 * glm::pi<float>());
    }

    data.push_back(u);
    data.push_back(v);

}
