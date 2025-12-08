#include <stdexcept>
#include "camera.h"

Camera::Camera(int width, int height, SceneCameraData renderData, float nearPlaneData, float farPlaneData){
    SceneCameraData camData = renderData;
    pos = glm::vec3(camData.pos); //world space
    look = glm::vec3(camData.look); //cam space
    up = glm::vec3(camData.up); //cam space
    heightAngle = camData.heightAngle;
    aperture = camData.aperture;
    focalLength = camData.focalLength;
    nearPlane = nearPlaneData;
    farPlane = farPlaneData;
    c_width = width;
    c_height = height;
    calculateVectors();
    initViewMatrices();
    initProjMatrix();

}

void Camera::initViewMatrices() {
    glm::vec4 col0(u[0], v[0], w[0], 0);
    glm::vec4 col1(u[1], v[1], w[1], 0);
    glm::vec4 col2(u[2], v[2], w[2], 0);
    glm::vec4 col3(0,0,0,1);
    glm::mat4 rotation(col0,col1,col2,col3);
    glm::vec4 colx(1, 0, 0, 0);
    glm::vec4 coly(0,1, 0, 0);
    glm::vec4 colz(0, 0, 1, 0);
    glm::vec4 colw(-pos[0],-pos[1],-pos[2],1);
    glm::mat4 translation(colx,coly,colz,colw);
    viewMatrix = rotation * translation;
    inverseViewMatrix = glm::inverse(viewMatrix);
}

void Camera::initProjMatrix(){
    glm::vec4 col0(1.0f/(farPlane * glm::tan(getWidthAngle()/2.0f)), 0, 0, 0);
    glm::vec4 col1(0,1.0f/(farPlane * glm::tan(getHeightAngle()/2.0f)), 0, 0);
    glm::vec4 col2(0, 0, (1.0f/farPlane), 0);
    glm::vec4 col3(0,0,0,1.0f);
    glm::mat4 perspective(col0,col1,col2,col3);

    float c = -nearPlane / farPlane;
    glm::vec4 colx(1.0f, 0, 0, 0);
    glm::vec4 coly(0,1.0f, 0, 0);
    glm::vec4 colz(0, 0, 1.0f/(1.0f + c), -1);
    glm::vec4 colw(0,0, -c/(1.0f + c), 0);
    glm::mat4 unhinge(colx,coly,colz,colw);

    glm::vec4 cola(1, 0, 0, 0);
    glm::vec4 colb(0,1, 0, 0);
    glm::vec4 colc(0, 0, -2, 0);
    glm::vec4 cold(0,0,-1,1);
    glm::mat4 shift(cola,colb,colc,cold);
    projMatrix = shift * unhinge * perspective;
}

void Camera::setPlanes(float nearPlaneData, float farPlaneData) {
    nearPlane = nearPlaneData;
    farPlane = farPlaneData;
    initProjMatrix();
}

glm::mat4 Camera::getViewMatrix() const {
    return viewMatrix;
}

glm::mat4 Camera::getInverseViewMatrix() const {
    return inverseViewMatrix;
}

glm::mat4 Camera::getProjMatrix() const {
    return projMatrix;
}

float Camera::getAspectRatio() const {
    return float(c_width ) / float(c_height);
}

float Camera::getHeightAngle() const {
    return heightAngle;
}

float Camera::getWidthAngle() const {
    return 2 * glm::atan(glm::tan(heightAngle/2) * getAspectRatio());
}

void Camera::calculateVectors(){
    w = glm::normalize(-look); //axes in terms of world space
    v = glm::normalize(up - (glm::dot(up, w)* w));
    u = glm::cross(v,w);
}

void Camera::move(int key){
    glm::vec3 forward = glm::normalize(look);
    glm::vec3 right   = glm::normalize(glm::cross(look, up));
    glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);

    glm::vec3 delta(0.f);

    switch (key) {
    case 87:        // forward
        delta += forward;
        break;
    case 83:        // backward
        delta -= forward;
        break;
    case 65:        // left (strafe)
        delta -= right;
        break;
    case 68:        // right (strafe)
        delta += right;
        break;
    case 32:    // up in world space
        delta += worldUp;
        break;
    case 16777249:  // down in world space
        delta -= worldUp;
        break;
    default:
        break;             // ignore other keys
    }
    pos += delta * 0.1f;
    calculateVectors();
    initViewMatrices();
}


void Camera::rotateCam(float deltaX, float deltaY){

    glm::vec3 col0(glm::cos(deltaX * 0.01),0 , -glm::sin(deltaX * 0.01));
    glm::vec3 col1(0, 1, 0);
    glm::vec3 col2(glm::sin(deltaX * 0.01), 0, glm::cos(deltaX* 0.01));
    glm::mat3 rotateX(col0,col1,col2);

    look = rotateX * look;

    glm::vec3 perpVector = glm::normalize(glm::cross(look, up));

    float cos = glm::cos(deltaY * 0.01);                   // theta in radians
    float sin = glm::sin(deltaY * 0.01);

    glm::vec3 colA(
        cos + perpVector.x * perpVector.x * (1.0f - cos),
        perpVector.y * perpVector.x * (1.0f - cos) + perpVector.z * sin,
        perpVector.z * perpVector.x * (1.0f - cos) - perpVector.y * sin
        );
    glm::vec3 colB(
        u.x * perpVector.y * (1.0f - cos) - perpVector.z * sin,
        cos + perpVector.y * perpVector.y * (1.0f - cos),
        perpVector.z * perpVector.y * (1.0f - cos) + perpVector.x * sin
        );
    glm::vec3 colC(
        perpVector.x * perpVector.z * (1.0f - cos) + perpVector.y * sin,
        perpVector.y * perpVector.z * (1.0f - cos) - perpVector.x * sin,
        cos + perpVector.z * perpVector.z * (1.0f - cos)
        );

    glm::mat3 rotateAxis(colA, colB, colC);

    look = rotateAxis * look;


    calculateVectors();
    initViewMatrices();

}
