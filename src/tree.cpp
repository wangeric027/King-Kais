#include "tree.h"
#include <iostream>
#include <stack>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/quaternion.hpp>

Tree::Tree() {
}


std::string Tree::createTree(){
    std::string treeString;
    std::string axiom = "FB";
    int iterations = 3;

    for (int i = 0; i < iterations; i++){
        std::string build;
        for (char character : axiom){
            switch(character){
            case 'F':
                build += "FFF";
                break;
            case 'B':
                build += "FF[lFB][rFB][bFB][xFB]";
                break;
            default:
                build += character;
            }
        }
        axiom = build;
    }

    treeString = axiom;
    return treeString;
}


std::vector<glm::mat4> Tree::createCTMList(std::string treeString, glm::vec3 posVec){
    glm::vec3 currentAxis(0.0f, 1.0f, 0.0f);
    std::stack<glm::vec3> axisStack;
    std::stack<glm::mat4> ctmStack;


    glm::mat4 currentCTM = glm::translate(glm::mat4(1.0f),
                                          1.5f* glm::normalize(posVec));


    std::vector<glm::mat4> resList;

    glm::vec3 rotAxis(glm::normalize(posVec));
    currentAxis = rotAxis;
    rotAxis = glm::vec3(1.0f,0.0f,1.0f);



    for (char character : treeString){
        float randomfloat = 20.0f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(60.0f-20.f)));
        float turnAngle = glm::radians(randomfloat);
        switch(character){
        case 'F':{
            currentCTM = glm::translate(currentCTM, currentAxis * 0.01f);
            glm::mat4 oriented = currentCTM;
            glm::vec3 up(0,1,0);
            glm::vec3 axis = glm::cross(up, currentAxis);
            float dotVal = glm::dot(up, currentAxis);
            dotVal = glm::clamp(dotVal, -1.0f, 1.0f);
            float angle = acos(dotVal);

            if (glm::length(axis) > 0.0001f) {
                axis = glm::normalize(axis);
                oriented = glm::rotate(oriented, angle, axis);
            }
            else {
                if (dotVal < 0.0f) {
                    oriented = glm::rotate(oriented, glm::pi<float>(), glm::vec3(1,0,0));
                }
            }
            oriented = glm::scale(oriented, glm::vec3(0.1f, 0.5f, 0.1f));

            resList.push_back(oriented);
            break;
        }
        case 'B':
            break;
        case '[':
            ctmStack.push(currentCTM);
            axisStack.push(currentAxis);
            break;
        case ']':
            currentAxis = axisStack.top();
            axisStack.pop();
            currentCTM = ctmStack.top();
            ctmStack.pop();
            break;
        case 'l':{
            glm::mat4 R = glm::rotate(glm::mat4(1.0f), turnAngle, rotAxis);
            currentAxis = glm::normalize(glm::vec3(R * glm::vec4(currentAxis, 0.0f)));
            currentCTM = glm::rotate(currentCTM, turnAngle, currentAxis);
        }
        break;
        case 'r':
        {
            // turn right
            glm::mat4 R = glm::rotate(glm::mat4(1.0f), -turnAngle, rotAxis);
            currentAxis = glm::normalize(glm::vec3(R * glm::vec4(currentAxis, 0.0f)));
            currentCTM = glm::rotate(currentCTM, turnAngle, currentAxis);
        }
        case 'b':{
            turnAngle += glm::radians(45.0f);
            glm::mat4 R = glm::rotate(glm::mat4(1.0f), turnAngle, rotAxis);
            currentAxis = glm::normalize(glm::vec3(R * glm::vec4(currentAxis, 0.0f)));
            currentCTM = glm::rotate(currentCTM, turnAngle, currentAxis);
            break;
        }
        case 'x':{
            turnAngle += glm::radians(45.0f);
            glm::mat4 R = glm::rotate(glm::mat4(1.0f), -turnAngle, rotAxis);
            currentAxis = glm::normalize(glm::vec3(R * glm::vec4(currentAxis, 0.0f)));
            currentCTM = glm::rotate(currentCTM, turnAngle, currentAxis);
        }
        break;

            break;
        }
    }
    return resList;
}
