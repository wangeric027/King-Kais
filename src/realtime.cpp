#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"

// ================== Rendering the Scene!

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
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here

    this->doneCurrent();
}




void Realtime::createImage(){
    glClearColor(0.0, 0, 0, 0.0);
    realtimeShapeList.clear();
    //generates list of buffers, one for each type of shape
    glGenBuffers(4, vboList);
    glGenVertexArrays(4, vaoList);


    RenderData metaData;
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

    for (int i = 0; i < 4; i++){
        //binds the VBO and inserts its corresponding vertex information
        glBindBuffer(GL_ARRAY_BUFFER, vboList[i]);
        std::vector<GLfloat> shapeVertexData;
        switch (i) {
            case 0: {
                Cube cube;
                cube.updateParams(settings.shapeParameter1);
                shapeVertexData = cube.generateShape();
            } break;
            case 1: {
                Cone cone;
                cone.updateParams(settings.shapeParameter1, settings.shapeParameter2);
                shapeVertexData = cone.generateShape();
            } break;
            case 2: {
                Cylinder cylinder;
                cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2);
                shapeVertexData = cylinder.generateShape();
            } break;
            case 3: {
                Sphere sphere;
                sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
                shapeVertexData = sphere.generateShape();
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
    for (RenderShapeData &shape: metaData.shapes){
        int enumAsInt = static_cast<int>(shape.primitive.type);
        RealtimeShapeInfo shapeInfo = RealtimeShapeInfo{enumAsInt, shape.ctm, shape.primitive.material};
        realtimeShapeList.push_back(shapeInfo);
    }


    //we now have a list of all shapes with their ctm, and vertices

    createFBO();

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

    geom_shader = ShaderLoader::createShaderProgram("resources/shaders/gbuffer.vert", "resources/shaders/gbuffer.frag");
    deferred_shader = ShaderLoader::createShaderProgram("resources/shaders/deferred.vert", "resources/shaders/deferred.frag");
    depth_shader = ShaderLoader::createShaderProgram("resources/shaders/deferred.vert", "resources/shaders/depth.frag");
    geoBufferShader = ShaderLoader::createShaderProgram("resources/shaders/deferred.vert", "resources/shaders/geo.frag");


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

    glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
    glViewport(0, 0, screen_width, screen_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

void Realtime::paintGL() {
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    geometryPass();
    //shadingPass();

    //depthTest();
    //fogTest();
    geoTest(3);

    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
}

void Realtime::sceneChanged() {
    createImage();
    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    if (m_glInitialized) createImage();
    update(); // asks for a PaintGL() call to occur

}

// ================== Camera Movement!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;

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

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();
    for (const auto &entry : m_keyMap) {
        Qt::Key key = entry.first;
        if (entry.second){
            cam.move(static_cast<int>(key));
        }
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
