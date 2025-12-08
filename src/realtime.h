#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <iostream>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>
#include "camera.h"
#include "trimesh/Cone.h"
#include "trimesh/Cube.h"
#include "trimesh/Sphere.h"
#include "trimesh/Cylinder.h"
#include "rigidbody.h"


struct RealtimeShapeInfo {
    int shapeType; // 0 = Cube, 1 = Cone, 2 = Cylinder, 3 = Sphere
    glm::mat4 ctm;
    SceneMaterial material;
    std::string groupName;
    RigidBody rigidBody;
};

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);
    void reLoad();

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    void createFBO();

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    double m_devicePixelRatio;

    GLuint geom_shader;
    GLuint deferred_shader;
    GLuint depth_shader;
    GLuint fogShader;
    GLuint geoBufferShader;

    int numTriangles;

    std::vector<RealtimeShapeInfo> realtimeShapeList;
    RealtimeShapeInfo* m_planet;
    RealtimeShapeInfo* m_player;
    std::vector<SceneLightData> sceneLightData;
    SceneGlobalData globals;

    GLuint default_fbo = 2;
    GLuint gBuffer;
    GLuint depthTexture, gPosition, gNormal, gDiffuse, gSpec;
    GLuint vboList[4];
    GLuint vaoList[4];
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;
    Cube cube;
    Sphere sphere;
    Cylinder cylinder;
    Cone cone;
    RenderData metaData;
    int sizeList[4];
    void createImage();
    void geometryPass();
    void shadingPass();
    void depthTest();
    void fogTest();
    void geoTest(int buffer);
    void alignToPlanetGravity();

    glm::mat4 projMatrix;
    glm::mat4 viewMatrix;
    glm::vec3 camPos;

    int screen_width;
    int screen_height;

    Camera cam;

    bool m_glInitialized = false;
};
