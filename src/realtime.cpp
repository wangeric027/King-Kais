#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"
#include <random>
#include "tree.h"
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "utils/stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "utils/tiny_obj_loader.h"




// ================== Rendering the Scene!

float objSpaceSphereRadius = 0.5f;
float moveSpeed = 1.0f;
float rotationAngle = glm::radians(7.5f);
const float CAMERA_DISTANCE = 2.0f;
const float CAMERA_HEIGHT = 2.5f;
bool hasPlayer = false;

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this

    m_keyMap[Qt::Key_Left] = false;
    m_keyMap[Qt::Key_Right] = false;
    m_keyMap[Qt::Key_Up] = false;
    m_keyMap[Qt::Key_Down] = false;
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    glDeleteVertexArrays(4, vaoList);
    glDeleteBuffers(4, vboList);
    glDeleteVertexArrays(1, &m_fullscreen_vao);
    m_fullscreen_vao = 0;
    glDeleteTextures(1, &depthTexture);
    depthTexture = 0;
    glDeleteTextures(1, &gPosition);
    gPosition = 0;
    glDeleteTextures(1, &gNormal);
    gNormal = 0;
    glDeleteTextures(1, &gDiffuse);
    gDiffuse = 0;
    glDeleteTextures(1, &gSpec);
    gSpec = 0;
    glDeleteFramebuffers(1, &gBuffer);
    gBuffer = 0;
    glDeleteProgram(geom_shader);
    geom_shader = 0;
    glDeleteProgram(deferred_shader);
    deferred_shader = 0;
    glDeleteProgram(depth_shader);
    depth_shader = 0;
    glDeleteProgram(geoBufferShader);
    geoBufferShader = 0;
    this->doneCurrent();
}


std::vector<float> Realtime::createGoku(){
    tinyobj::ObjReaderConfig config;
    config.mtl_search_path = "goku";      // or wherever your .mtl lives
    config.triangulate = true;        // make sure everything is triangles

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile("goku/Goku.obj", config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader error: " << reader.Error() << std::endl;
        }
        std::vector<float> empty;
        return empty;
    }

    if (!reader.Warning().empty()) {
        std::cerr << "TinyObjReader warning: " << reader.Warning() << std::endl;
    }

    const tinyobj::attrib_t &attrib = reader.GetAttrib();
    const std::vector<tinyobj::shape_t> &shapes = reader.GetShapes();
    const std::vector<tinyobj::material_t> &materials = reader.GetMaterials();

    // Our vertex layout: pos(3), normal(3), uv(2) = 8 floats
    std::vector<float> vertexData;

    for (const auto &shape : shapes) {
        for (const auto &idx : shape.mesh.indices) {
            // --- position ---
            if (idx.vertex_index < 0) continue; // skip invalid
            int vIdx = 3 * idx.vertex_index;
            float vx = attrib.vertices[vIdx + 0];
            float vy = attrib.vertices[vIdx + 1];
            float vz = attrib.vertices[vIdx + 2];

            // --- normal ---
            float nx = 0.f, ny = 0.f, nz = 1.f;
            if (idx.normal_index >= 0) {
                int nIdx = 3 * idx.normal_index;
                nx = attrib.normals[nIdx + 0];
                ny = attrib.normals[nIdx + 1];
                nz = attrib.normals[nIdx + 2];
            }

            // --- texcoord ---
            float u = 0.f, v = 0.f;
            if (idx.texcoord_index >= 0) {
                int tIdx = 2 * idx.texcoord_index;
                u = attrib.texcoords[tIdx + 0];
                v = attrib.texcoords[tIdx + 1];
            }

            vertexData.push_back(vx);
            vertexData.push_back(vy);
            vertexData.push_back(vz);

            vertexData.push_back(nx);
            vertexData.push_back(ny);
            vertexData.push_back(nz);

            vertexData.push_back(u);
            vertexData.push_back(v);
        }
    }

    return vertexData;
}




void Realtime::createImage(){

    //std::vector<glm::mat4> tree = Tree::createCTMList(Tree::createTree());
    glClearColor(0.0, 0, 0, 1.0);
    realtimeShapeList.clear();
    //generates list of buffers, one for each type of shape
    glGenBuffers(5, vboList);
    glGenVertexArrays(5, vaoList);

    SceneParser::parse(settings.sceneFilePath, metaData);

    sceneLightData = metaData.lights;
    globals = metaData.globalData;

    if (settings.shapeParameter1 < 2){
        settings.shapeParameter1 = 2;
    }
    if (settings.shapeParameter2 < 3){
        settings.shapeParameter2 = 3;
    }

    cam = Camera(this->width(), this->height(), metaData.cameraData, settings.nearPlane, settings.farPlane);

    for (int i = 0; i < 5; i++){
        //binds the VBO and inserts its corresponding vertex information
        glBindBuffer(GL_ARRAY_BUFFER, vboList[i]);
        std::vector<GLfloat> shapeVertexData;
        switch (i) {
            case 0: {
                cube.updateParams(settings.shapeParameter1);
                shapeVertexData = cube.generateShape();
            } break;
            case 1: {
                cone.updateParams(settings.shapeParameter1, settings.shapeParameter2);
                shapeVertexData = cone.generateShape();
            } break;
            case 2: {
                cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2);
                shapeVertexData = cylinder.generateShape();
            } break;
            case 3: {
                sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
                shapeVertexData = sphere.generateShape();
            } break;
            case 4:{
                shapeVertexData = createGoku();
            }
            break;
        }
        //Initialize our VBO with vertex info for a shape. Store the # of triangles in sizeList, since DrawArrays will need to know how many to draw
        sizeList[i] = shapeVertexData.size() / 8;
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * shapeVertexData.size(), shapeVertexData.data(), GL_STATIC_DRAW);
        //Creates my VAO and binds it
        glBindVertexArray(vaoList[i]);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), nullptr);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,  8 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,  8 * sizeof(GLfloat), reinterpret_cast<void*>(6 * sizeof(GLfloat)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }


    //At this point all vbos are initialized and so are all vbos and their sizes.
    // Just need to store each CTM with their shapeType
    SceneMaterial lastMat;
    for (RenderShapeData &shape: metaData.shapes){
        int enumAsInt = static_cast<int>(shape.primitive.type);
        RealtimeShapeInfo shapeInfo = RealtimeShapeInfo{enumAsInt, shape.ctm, shape.primitive.material, shape.name};
        glm::vec3 initialPos = shapeInfo.ctm[3];
        if (shapeInfo.groupName == "Player") {
            shapeInfo.shapeType = 4;
            shapeInfo.rigidBody.setMass(1.0f);
            shapeInfo.rigidBody.position = initialPos;
        } else if (shapeInfo.groupName == "Planet") {
            shapeInfo.rigidBody.setMass(10000000.f);
            shapeInfo.rigidBody.isStatic = true;
            shapeInfo.rigidBody.position = initialPos;
        } else {
            shapeInfo.rigidBody.setMass(0.0f);
            shapeInfo.rigidBody.position = initialPos;
        }
        std::cout << shapeInfo.groupName << std::endl;
        realtimeShapeList.push_back(shapeInfo);
        lastMat = shape.primitive.material;
    }

    for (RealtimeShapeInfo &shapeInfo: realtimeShapeList) {
        if (shapeInfo.groupName == "Player") {
            m_player = &shapeInfo;
            hasPlayer = true;
        }
        if (shapeInfo.groupName == "Planet") {
            m_planet= &shapeInfo;
        }
    }

    // for (glm::mat4 ctm : tree){
    //     RealtimeShapeInfo branch = RealtimeShapeInfo{2, ctm, lastMat, "branch"};
    //     realtimeShapeList.push_back(branch);
    // }

    createFBO();
    createBackground();
    createGoku();
}


void Realtime::createBackground(){
    float skyboxVertices[] = {
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
       -1.0f, 1.0f, -1.0f,
    };
    unsigned int skyboxIndices[]{
        1,2,6,
        6,5,1,
        0,4,7,
        7,3,0,
        4,5,6,
        6,7,4,
        0,3,2,
        2,1,0,
        0,1,5,
        5,4,0,
        3,7,6,
        6,2,3
    };

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(skyboxVertices),skyboxVertices,GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(skyboxIndices),skyboxIndices,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,3 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    std::string facesCubemap[6]{
        "sky/Epic_GloriousPink_Cam_2_Left+X.png",
        "sky/Epic_GloriousPink_Cam_3_Right-X.png",
        "sky/Epic_GloriousPink_Cam_4_Up+Y.png",
        "sky/Epic_GloriousPink_Cam_5_Down-Y.png",
        "sky/Epic_GloriousPink_Cam_0_Front+Z.png",
        "sky/Epic_GloriousPink_Cam_1_Back-Z.png"
    };



    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


    for (int i = 0; i < 6; i++){
        int width, height, nrChannels;
        unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
        if (data){
            stbi_set_flip_vertically_on_load(false);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "failed to load texture:" << data[i] << std::endl;
        }

    }
    skyboxShader = ShaderLoader::createShaderProgram("resources/shaders/skybox.vert", "resources/shaders/skybox.frag");
}

void Realtime::createFBO(){

    screen_width = size().width() * m_devicePixelRatio;
    screen_height = size().height() * m_devicePixelRatio;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    //depth
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture,0);

    //pos
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    //nromal
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    //diffuse
    glGenTextures(1, &gDiffuse);
    glBindTexture(GL_TEXTURE_2D, gDiffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gDiffuse, 0);

    //specular
    glGenTextures(1, &gSpec);
    glBindTexture(GL_TEXTURE_2D, gSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gSpec, 0);

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Realtime::initializeGL() {
    m_glInitialized = true;
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;
    // Allows OpenGL to draw objects appropriately on top of one another
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    default_fbo = 3;

    geom_shader = ShaderLoader::createShaderProgram("resources/shaders/gbuffer.vert", "resources/shaders/gbuffer.frag");
    deferred_shader = ShaderLoader::createShaderProgram("resources/shaders/deferred.vert", "resources/shaders/deferred.frag");
    depth_shader = ShaderLoader::createShaderProgram("resources/shaders/deferred.vert", "resources/shaders/depth.frag");
    geoBufferShader = ShaderLoader::createShaderProgram("resources/shaders/deferred.vert", "resources/shaders/geo.frag");

    //skybox shaders


    std::vector<GLfloat> fullscreen_quad_data =
        { //     POSITIONS    //
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f
        };
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),  reinterpret_cast<void*>(3* sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    createImage();



    /// 1. Load post-processing shader
    m_texture_shader = ShaderLoader::createShaderProgram(
        "resources/shaders/texture.vert",
        "resources/shaders/texture.frag"
        );

    /// 2. Create post-processing FBO
    glGenFramebuffers(1, &m_post_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_post_fbo);

    glGenTextures(1, &m_post_color);
    glBindTexture(GL_TEXTURE_2D, m_post_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_post_color, 0);

    glGenTextures(1, &m_post_depth);
    glBindTexture(GL_TEXTURE_2D, m_post_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_post_depth, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /// 3. Fullscreen quad for post-processing
    std::vector<GLfloat> pp_quad = {
        -1.f,  1.f, 0.f,   0.f, 1.f,
        -1.f, -1.f, 0.f,   0.f, 0.f,
        1.f, -1.f, 0.f,   1.f, 0.f,

        -1.f,  1.f, 0.f,   0.f, 1.f,
        1.f, -1.f, 0.f,   1.f, 0.f,
        1.f,  1.f, 0.f,   1.f, 1.f
    };

    glGenVertexArrays(1, &m_pp_vao);
    glGenBuffers(1, &m_pp_vbo);

    glBindVertexArray(m_pp_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_pp_vbo);
    glBufferData(GL_ARRAY_BUFFER, pp_quad.size() * sizeof(GLfloat), pp_quad.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3* sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /* ---------------- ADDED FOR POST-PROCESSING END ------------------ */
}

void Realtime::geometryPass(){

    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(geom_shader);
    for (RealtimeShapeInfo &realtimeShape : realtimeShapeList) { //binds



        glm::mat4 modelMVP = cam.getProjMatrix() * cam.getViewMatrix() * realtimeShape.ctm;
        glm::mat4 normalMatrix = inverse(transpose(glm::mat3(realtimeShape.ctm)));

        GLuint locDiffuseMap = glGetUniformLocation(geom_shader, "texture_diffuse1");
        GLuint locSpecMap    = glGetUniformLocation(geom_shader, "texture_specular1");

        GLuint materialPropertyTextures[2];
        glGenTextures(2, materialPropertyTextures);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, materialPropertyTextures[0]);
        float diffuse[4] = {realtimeShape.material.cDiffuse.r, realtimeShape.material.cDiffuse.g, realtimeShape.material.cDiffuse.b, realtimeShape.material.shininess };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1,0, GL_RGBA, GL_FLOAT, diffuse);
        glUniform1i(locDiffuseMap, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, materialPropertyTextures[1]);

        float specular[4] = {realtimeShape.material.cSpecular.r, realtimeShape.material.cSpecular.g, realtimeShape.material.cSpecular.b, realtimeShape.material.shininess};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1,0, GL_RGBA, GL_FLOAT, specular);
        glUniform1i(locSpecMap, 1);

        GLuint uniformShaderNormal = glGetUniformLocation(geom_shader, "normalMatrix");
        GLuint uniformShaderMVP= glGetUniformLocation(geom_shader, "modelMVP");
        GLuint uniformCTM = glGetUniformLocation(geom_shader, "ctm");
        glUniformMatrix4fv(uniformShaderNormal, 1, GL_FALSE, &normalMatrix[0][0]);
        glUniformMatrix4fv(uniformShaderMVP, 1, GL_FALSE, &modelMVP[0][0]);
        glUniformMatrix4fv(uniformCTM, 1, GL_FALSE, &realtimeShape.ctm[0][0]);

        glBindVertexArray(vaoList[realtimeShape.shapeType]);
        glDrawArrays(GL_TRIANGLES, 0, sizeList[realtimeShape.shapeType]);
    }


    glBindVertexArray(0);
    glUseProgram(0);
}


void Realtime::shadingPass(){

    glBindFramebuffer(GL_FRAMEBUFFER, m_post_fbo);
    glViewport(0, 0, screen_width, screen_height);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glDisable(GL_DEPTH_TEST);

    glUseProgram(deferred_shader);

    GLint locPos  = glGetUniformLocation(deferred_shader, "gPosition");
    GLint locNorm = glGetUniformLocation(deferred_shader, "gNormal");
    GLint locDiff = glGetUniformLocation(deferred_shader, "gDiffuse");
    GLint locSpec = glGetUniformLocation(deferred_shader, "gSpec");


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gDiffuse);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gSpec);

    glUniform1i(locPos,  0);
    glUniform1i(locNorm, 1);
    glUniform1i(locDiff, 2);
    glUniform1i(locSpec, 3);

    for (int i = 0; i < sceneLightData.size(); i++){
        SceneLightData sceneLight = sceneLightData[i];
        std::string posName = "posList[" + std::to_string(i) + "]";
        std::string dirName = "dirList[" + std::to_string(i) + "]";
        std::string colorName = "colorList[" + std::to_string(i) + "]";
        std::string attName = "attenuationList[" + std::to_string(i) + "]";
        std::string penName = "penumbraList[" + std::to_string(i) + "]";
        std::string angleName = "angleList[" + std::to_string(i) + "]";
        std::string typeName = "typeList[" + std::to_string(i) + "]";


        int uniformPos = glGetUniformLocation(deferred_shader, posName.c_str());
        int uniformDir = glGetUniformLocation(deferred_shader, dirName.c_str());
        int uniformColor = glGetUniformLocation(deferred_shader, colorName.c_str());
        int uniformAtt = glGetUniformLocation(deferred_shader, attName.c_str());
        int uniformPen = glGetUniformLocation(deferred_shader, penName.c_str());
        int uniformAngle = glGetUniformLocation(deferred_shader, angleName.c_str());

        int lightAsInt = static_cast<int>(sceneLight.type);
        int uniformType = glGetUniformLocation(deferred_shader, typeName.c_str());

        glUniform3f(uniformPos, sceneLight.pos[0], sceneLight.pos[1], sceneLight.pos[2]);
        glUniform3f(uniformDir, sceneLight.dir[0], sceneLight.dir[1], sceneLight.dir[2]);
        glUniform4f(uniformColor, sceneLight.color[0], sceneLight.color[1], sceneLight.color[2], sceneLight.color[3]);
        glUniform3f(uniformAtt, sceneLight.function[0], sceneLight.function[1], sceneLight.function[2]);
        glUniform1f(uniformPen, sceneLight.penumbra);
        glUniform1f(uniformAngle, sceneLight.angle);
        glUniform1i(uniformType, lightAsInt);
    }

    int uniformCount = glGetUniformLocation(deferred_shader, "lightCount");
    int uniformKA = glGetUniformLocation(deferred_shader, "ka");
    int uniformKS = glGetUniformLocation(deferred_shader, "ks");
    int uniformKD = glGetUniformLocation(deferred_shader, "kd");
    int uniformCamPos = glGetUniformLocation(deferred_shader, "camPos");
    glUniform1i(uniformCount, sceneLightData.size());
    glUniform1f(uniformKA, globals.ka);
    glUniform1f(uniformKS, globals.ks);
    glUniform1f(uniformKD, globals.kd);
    glUniform3f(uniformCamPos, cam.pos[0], cam.pos[1], cam.pos[2]);

    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::backgroundPass(){
    //glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
    glViewport(0, 0, screen_width, screen_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(skyboxShader);
    glDisable(GL_CULL_FACE);

    GLint shaderLoc = glGetUniformLocation(skyboxShader, "skybox");
    glUniform1i(shaderLoc, 0);

    glDepthFunc(GL_LEQUAL);
    glm::mat4 proj = cam.getProjMatrix();
    glm::mat4 view = cam.getViewMatrix();
    view = glm::mat4(glm::mat3(view));


    GLuint projLoc= glGetUniformLocation(skyboxShader, "projection");
    GLuint viewLoc = glGetUniformLocation(skyboxShader, "view");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

}

void Realtime::ppPass(){

//     /* ---------------- ADDED FOR POST-PROCESSING BEGIN ---------------- /

// // Draw shaded scene through your stylized post-process shader
glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
glViewport(0, 0, screen_width, screen_height);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glDisable(GL_DEPTH_TEST);

glUseProgram(m_texture_shader);

// Bind color texture from post-FBO
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, m_post_color);
glUniform1i(glGetUniformLocation(m_texture_shader, "colorTexture"), 0);

// Bind depth texture (if your shader uses it)
glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, m_post_depth);
glUniform1i(glGetUniformLocation(m_texture_shader, "depthTexture"), 1);

// Send stylization toggles
glUniform1i(glGetUniformLocation(m_texture_shader, "useGrayscale"), m_effects.grayscale);
glUniform1i(glGetUniformLocation(m_texture_shader, "useInvert"), m_effects.invert);
glUniform1i(glGetUniformLocation(m_texture_shader, "useEdgeDetection"), m_effects.edgeDetection);
glUniform1i(glGetUniformLocation(m_texture_shader, "useVignette"), m_effects.vignette);
glUniform1i(glGetUniformLocation(m_texture_shader, "useDepthVisualization"), m_effects.depthVisualization);
glUniform1i(glGetUniformLocation(m_texture_shader, "usePixelation"), m_effects.pixelation);

// Draw fullscreen quad
glBindVertexArray(m_pp_vao);
glDrawArrays(GL_TRIANGLES, 0, 6);
glBindVertexArray(0);
glBindTexture(GL_TEXTURE_2D, m_post_color);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

glBindTexture(GL_TEXTURE_2D, m_post_depth);
glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

glBindTexture(GL_TEXTURE_2D, 0);
// / ---------------- ADDED FOR POST-PROCESSING END ------------------ */

}



void Realtime::paintGL() {
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    backgroundPass();

    geometryPass();
    shadingPass();
    ppPass();


    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    glBindTexture(GL_TEXTURE_2D, m_post_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, m_post_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Students: anything requiring OpenGL calls when the program starts should be done here
}

void Realtime::sceneChanged() {
    createImage();
    update(); // asks for a PaintGL() call to occur
}

void Realtime::reLoad() {
    if (settings.shapeParameter1 < 2){
        settings.shapeParameter1 = 2;
    }
    if (settings.shapeParameter2 < 3){
        settings.shapeParameter2 = 3;
    }

    cam.setPlanes(settings.nearPlane, settings.farPlane);

    for (int i = 0; i < 5; i++){
        //binds the VBO and inserts its corresponding vertex information
        glBindBuffer(GL_ARRAY_BUFFER, vboList[i]);
        std::vector<GLfloat> shapeVertexData;
        switch (i) {
        case 0: {
            cube.updateParams(settings.shapeParameter1);
            shapeVertexData = cube.generateShape();
        } break;
        case 1: {
            cone.updateParams(settings.shapeParameter1, settings.shapeParameter2);
            shapeVertexData = cone.generateShape();
        } break;
        case 2: {
            cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2);
            shapeVertexData = cylinder.generateShape();
        } break;
        case 3: {
            sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
            shapeVertexData = sphere.generateShape();
        } break;
        case 4: {
            shapeVertexData = createGoku();

        }
        break;
        }
        //Initialize our VBO with vertex info for a shape. Store the # of triangles in sizeList, since DrawArrays will need to know how many to draw
        sizeList[i] = shapeVertexData.size() / 8;
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * shapeVertexData.size(), shapeVertexData.data(), GL_STATIC_DRAW);
        //Creates my VAO and binds it
        glBindVertexArray(vaoList[i]);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), nullptr);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,  8 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,  8 * sizeof(GLfloat), reinterpret_cast<void*>(6 * sizeof(GLfloat)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void Realtime::settingsChanged() {
    if (m_glInitialized) reLoad();
    update(); // asks for a PaintGL() call to occur

}

// ================== Camera Movement!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
    if (event->key() == Qt::Key_Z) { m_effects.depthVisualization ^= 1; update(); }
    if (event->key() == Qt::Key_E) { m_effects.edgeDetection    ^= 1; update(); }
    if (event->key() == Qt::Key_G) { m_effects.grayscale        ^= 1; update(); }
    if (event->key() == Qt::Key_I) { m_effects.invert           ^= 1; update(); }
    if (event->key() == Qt::Key_V) { m_effects.vignette         ^= 1; update(); }
    if (event->key() == Qt::Key_P) { m_effects.pixelation       ^= 1; update(); }
    if (event->key() == Qt::Key_C) { m_effects = PostProcessingEffects(); update(); }
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        cam.rotateCam(deltaX,deltaY);

        // Use deltaX and deltaY here to rotate

        update(); // asks for a PaintGL() call to occur
    }
}

glm::vec3 applyPlanetGravity(RigidBody& player, RigidBody& planet) {
    const float G = 0.000016f;
    glm::vec3 r_vec = planet.position - player.position;

    // 2. Calculate distance squared (r^2)
    float r_sq = glm::dot(r_vec, r_vec);

    // Prevent division by zero and excessive forces when too close
    const float MIN_DIST_SQ = 0.1f;
    if (r_sq < MIN_DIST_SQ) {
        r_sq = MIN_DIST_SQ;
    }

    // 3. Calculate the magnitude of the force (F = G * m1 * m2 / r^2)
    float F_mag = G * player.mass * planet.mass / r_sq;

    // 4. Calculate the unit direction vector (from player to planet)
    glm::vec3 r_hat = glm::normalize(r_vec);

    // 5. Calculate the final force vector
    glm::vec3 F = F_mag * r_hat;

    return F;
}

void handleCollision(RigidBody& player, RigidBody& planet, float worldRadius) {
    glm::vec3 r_vec = player.position - planet.position;
    float dist = glm::length(r_vec);

    const float epsilon = 0.1f;
    float targetDist = worldRadius + epsilon;

    if (dist < targetDist) {
        // Calculate the radial direction (from planet center to player)
        glm::vec3 radialDir = glm::normalize(r_vec);

        // Snap player to the correct distance from planet center
        player.position = planet.position + radialDir * targetDist;

        // Decompose velocity into radial and tangential components
        float radialVelocity = glm::dot(player.velocity, radialDir);
        glm::vec3 radialComponent = radialDir * radialVelocity;
        glm::vec3 tangentialComponent = player.velocity - radialComponent;

        // Only cancel the radial component if moving into the planet
        if (radialVelocity < 0.0f) {
            player.velocity = tangentialComponent;
        }

        // // Optional: Apply friction to tangential movement
        // float frictionCoefficient = 0.98f; // 0.98 = very little friction, 0.9 = more friction
        // player.velocity *= frictionCoefficient;
    }
}

void movePlayer(int key, RealtimeShapeInfo* player, RealtimeShapeInfo* planet) {
    // Extract the local Forward axis from the player's CTM (Z-axis column)
    glm::vec3 localForward = glm::normalize(glm::vec3(player->ctm[2][0], player->ctm[2][1], player->ctm[2][2]));

    glm::vec3 moveDirection(0.0f);

    switch (key) {
    case Qt::Key_Up: // Move Forward in local space
        moveDirection = localForward * moveSpeed;
        break;
    case Qt::Key_Down: // Move Backward in local space
        moveDirection = -localForward * moveSpeed;
        break;
    default:
        return;
    }

    glm::vec3 toPlanet = glm::normalize(player->rigidBody.position - planet->rigidBody.position);
    float radialVel = glm::dot(player->rigidBody.velocity, toPlanet);
    glm::vec3 radialComponent = toPlanet * radialVel;

    // Set tangential velocity directly, preserve radial (gravity) component
    glm::vec3 newTangentialVel = moveDirection * moveSpeed;
    player->rigidBody.velocity = radialComponent + newTangentialVel;
}

void rotatePlayer(int key, RealtimeShapeInfo* player) {
    glm::vec3 localUp = glm::normalize(glm::vec3(player->ctm[1][0], player->ctm[1][1], player->ctm[1][2]));

    switch (key) {
    case Qt::Key_Left:
        player->ctm = glm::rotate(player->ctm, -rotationAngle, localUp);
        break;
    case Qt::Key_Right:
        player->ctm = glm::rotate(player->ctm, rotationAngle, localUp);
        break;
    default:
        break;
    }
}

void stopPlayerMovement(RealtimeShapeInfo* player, RealtimeShapeInfo* planet) {
    glm::vec3 toPlanet = glm::normalize(player->rigidBody.position - planet->rigidBody.position);
    float radialVel = glm::dot(player->rigidBody.velocity, toPlanet);
    glm::vec3 radialComponent = toPlanet * radialVel;

    player->rigidBody.velocity = radialComponent;
}

void Realtime::alignToPlanetGravity() {
    glm::vec3 playerPos = m_player->rigidBody.position;
    glm::vec3 planetPos = m_planet->rigidBody.position;

    // 1. Calculate the required Up Vector (pointing away from planet)
    glm::vec3 upVectorRequired = glm::normalize(playerPos - planetPos);

    // 2. Extract current CTM and decompose scale
    glm::mat4 currentCTM = m_player->ctm;
    float scaleX = glm::length(glm::vec3(currentCTM[0]));
    float scaleY = glm::length(glm::vec3(currentCTM[1]));
    float scaleZ = glm::length(glm::vec3(currentCTM[2]));

    // Extract normalized current forward axis (without scale)
    glm::vec3 currentForward = glm::normalize(glm::vec3(currentCTM[2]));

    // 3. Project current forward onto the plane perpendicular to upVectorRequired
    glm::vec3 targetForward = currentForward - glm::dot(currentForward, upVectorRequired) * upVectorRequired;

    // Handle edge case where forward is parallel to up
    if (glm::length(targetForward) < 0.001f) {
        // Choose an arbitrary perpendicular direction
        glm::vec3 arbitrary = glm::abs(upVectorRequired.x) < 0.9f ?
                                  glm::vec3(1.0f, 0.0f, 0.0f) :
                                  glm::vec3(0.0f, 1.0f, 0.0f);
        targetForward = glm::cross(upVectorRequired, arbitrary);
    }
    targetForward = glm::normalize(targetForward);

    // 4. Build orthonormal basis using cross products
    // Right = Forward × Up
    glm::vec3 newRight = glm::normalize(glm::cross(targetForward, upVectorRequired));

    // Recompute Forward = Up × Right (ensures perfect orthogonality)
    glm::vec3 newForward = glm::normalize(glm::cross(upVectorRequired, newRight));

    // 5. Apply scale to the axes
    glm::vec3 scaledRight = newRight * scaleX;
    glm::vec3 scaledUp = upVectorRequired * scaleY;
    glm::vec3 scaledForward = newForward * scaleZ;

    // 6. Construct the new rotation matrix with proper basis vectors
    glm::mat4 newCTM = glm::mat4(1.0f);
    newCTM[0] = glm::vec4(scaledRight, 0.0f);      // X-axis (right)
    newCTM[1] = glm::vec4(scaledUp, 0.0f);         // Y-axis (up)
    newCTM[2] = glm::vec4(scaledForward, 0.0f);    // Z-axis (forward)
    newCTM[3] = currentCTM[3];                      // Preserve translation

    // 7. Verify the determinant is positive (correct winding order)
    glm::mat3 rotPart = glm::mat3(newCTM);
    float det = glm::determinant(rotPart);

    // If determinant is negative, we have a reflection - flip one axis
    if (det < 0.0f) {
        newCTM[0] = -newCTM[0];  // Flip right axis to fix winding
    }

    m_player->ctm = newCTM;
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    bool isMovingPlayer = false;
    m_elapsedTimer.restart();
    for (const auto &entry : m_keyMap) {
        Qt::Key key = entry.first;
        if (entry.second) {
            cam.move(static_cast<int>(key));
            if (hasPlayer){
                rotatePlayer(key, m_player);
                if (key == Qt::Key_Up || key == Qt::Key_Down) {
                    isMovingPlayer = true;
                    movePlayer(key, m_player, m_planet);
                }
            }
        }
    }
    if (settings.extraCredit1 && hasPlayer) {
        glm::vec3 F_planet = applyPlanetGravity(m_player->rigidBody, m_planet->rigidBody);

        m_player->rigidBody.applyForce(F_planet);
        for (auto &shapeData : realtimeShapeList)
        {
            if (!isMovingPlayer) {
                stopPlayerMovement(m_player, m_planet);
            }
            if (shapeData.rigidBody.isStatic) {
                continue;
            }

            shapeData.rigidBody.integrate(deltaTime);

            if (shapeData.groupName == "Player") {
                float sphereRadiusWorld = objSpaceSphereRadius * m_planet->ctm[0][0];
                handleCollision(shapeData.rigidBody, m_planet->rigidBody, sphereRadiusWorld);
                alignToPlanetGravity();
            }

            const glm::vec3& newPos = shapeData.rigidBody.position;

            shapeData.ctm[3][0] = newPos.x;
            shapeData.ctm[3][1] = newPos.y;
            shapeData.ctm[3][2] = newPos.z;
        }
        cam.followPlayer(m_player->ctm, CAMERA_DISTANCE, CAMERA_HEIGHT);
    }
    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int fixedWidth = viewport[2];
    int fixedHeight = viewport[3];
    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}

void Realtime::depthTest(){
    glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
    glViewport(0, 0, screen_width, screen_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(depth_shader);

    GLint depthLoc  = glGetUniformLocation(depth_shader, "depthTex");
    GLint nearLoc  = glGetUniformLocation(depth_shader, "near");
    GLint farLoc  = glGetUniformLocation(depth_shader, "far");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);


    glUniform1i(depthLoc,  0);
    glUniform1f(nearLoc,  settings.nearPlane);
    glUniform1f(farLoc,  settings.farPlane);


    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


void Realtime::geoTest(int buffer){
    glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
    glViewport(0, 0, screen_width, screen_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(geoBufferShader);

    GLint bufferLoc  = glGetUniformLocation(geoBufferShader, "buffer");
    GLint bufferTypeLoc  = glGetUniformLocation(geoBufferShader, "bufferType");

    glActiveTexture(GL_TEXTURE0);
    switch(buffer){
    case 0:
        glBindTexture(GL_TEXTURE_2D, gPosition);
        break;
    case 1:
        glBindTexture(GL_TEXTURE_2D, gNormal);
        break;
    case 2:
        glBindTexture(GL_TEXTURE_2D, gDiffuse);
        break;
    case 3:
        glBindTexture(GL_TEXTURE_2D, gSpec);
        break;
    }
    glUniform1i(bufferLoc,  0);
    glUniform1i(bufferTypeLoc,  buffer);

    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);

}
