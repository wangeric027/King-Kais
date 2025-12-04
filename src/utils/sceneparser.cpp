#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>


bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }
    renderData.cameraData = fileReader.getCameraData();
    renderData.globalData = fileReader.getGlobalData();


    SceneNode* root = fileReader.getRootNode();
    renderData.shapes.clear();
    glm::mat4 ctm(1.0f);
    traverseSceneGraph(renderData, ctm, root);
    return true;
}

void SceneParser::traverseSceneGraph(RenderData &renderData, glm::mat4 ctm, SceneNode* node){
    if (node->transformations.size() != 0) { //applies transformation to ctm
        for (SceneTransformation* transform : node->transformations){
            switch (transform->type) {
            case TransformationType::TRANSFORMATION_TRANSLATE: {
                glm::mat4 translationVector = glm::translate(transform->translate);
                ctm = ctm * translationVector;
                break;
            }
            case TransformationType::TRANSFORMATION_ROTATE: {
                glm::mat4 rotationVector = glm::rotate(transform->angle, transform->rotate);
                ctm = ctm * rotationVector;
                break;
            }
            case TransformationType::TRANSFORMATION_SCALE: {
                glm::mat4 scaleVector = glm::scale(transform->scale);
                ctm = ctm * scaleVector;
                break;
            }
            case TransformationType::TRANSFORMATION_MATRIX: {
                ctm = ctm * transform->matrix;
                break;
            }
            default: {
                break;
            }
            }
        }
    }
    if (node->primitives.size() != 0) { //stores the shape and its ctm
        for (ScenePrimitive* shape : node->primitives){
            RenderShapeData renderShapeData = {*shape, ctm};
            renderData.shapes.push_back(renderShapeData);
        }
    }

    if (node->lights.size() != 0) { //lights
        for (SceneLight* light : node->lights){
            SceneLightData renderLight;
            renderLight = {light->id,
                           light->type,
                           light->color,
                           light->function,
                           ctm * glm::vec4{0,0,0,1},
                           ctm * light->dir,
                           light->penumbra,
                           light->angle};
            renderData.lights.push_back(renderLight);
        }
    }

    for (int i = 0; i < node->children.size(); i++){ //recurse on each child
        traverseSceneGraph(renderData, ctm, node->children[i]);
    }
}
