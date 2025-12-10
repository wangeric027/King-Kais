#include "tree.h"
#include <iostream>
#include <stack>
#include <glm/gtc/matrix_transform.hpp>


Tree::Tree() {
}


std::string Tree::createTree(){
    std::string treeString;
    std::string axiom = "FB";
    int iterations = 4 ;

    for (int i = 0; i < iterations; i++){
        std::string build;
        for (char character : axiom){
            switch(character){
                case 'F':
                    build += "FF";
                    break;
                case 'B':
                    build += "[lFB][rFB]";
                    break;
                default:
                    build += character;
            }
        }
        axiom = build;
    }

    treeString = axiom;
    std::cout << treeString << std::endl;
    return treeString;
}


std::vector<glm::mat4> Tree::createCTMList(std::string treeString){
    glm::mat4 currentCTM(1.0f);
    glm::vec3 currentAxis(0.0f, 1.0f, 0.0f);
    std::stack<glm::vec3> axisStack;
    std::stack<glm::mat4> ctmStack;


    std::vector<glm::mat4> resList;

    const float turnAngle = glm::radians(25.0f);
    const glm::vec3 rotAxis(1.0f, 0.0f, 1.0f); // rotate around Z


    for (char character : treeString){
        switch(character){
            case 'F':
                currentCTM = glm::translate(currentCTM, currentAxis);
                resList.push_back(currentCTM);
                break;
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
            break;
        }
    }
    return resList;
}
