#pragma once

#include <glm/glm.hpp>
#include "utils/sceneparser.h"



// A class representing a virtual camera.

// Feel free to make your own design choices for Camera class, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class Camera {


public:
    // Returns the view matrix for the current camera settings.
    // You might also want to define another function that return the inverse of the view matrix.

    Camera(int width, int height, SceneCameraData renderData, float nearPlaneData, float farPlaneData);
    Camera() = default;

    glm::vec3 pos; //camera axes
    glm::vec3 look;
    glm::vec3 up;
    glm::vec3 u; //camera coords in world
    glm::vec3 v;
    glm::vec3 w;
    int c_width;
    int c_height;


    float getAspectRatio() const;
    float getHeightAngle() const;
    float getWidthAngle() const;
    glm::mat4 getViewMatrix() const;
    glm::mat4 getInverseViewMatrix() const;
    glm::mat4 getProjMatrix() const;
    void move(int key);
    void rotateCam(float deltaX, float deltaY);
private:
    void calculateVectors();
    float aspectRatio;
    float heightAngle;
    float widthAngle;
    float focalLength;
    float aperture;
    float nearPlane;
    float farPlane;
    glm::mat4 viewMatrix;
    glm::mat4 inverseViewMatrix;
    glm::mat4 projMatrix;
    void initViewMatrices();
    void initProjMatrix();

};
