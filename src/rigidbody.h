#pragma once
#include <glm/glm.hpp>

class RigidBody {
public:
    // Constructors

    RigidBody(float mass = 1.0f,
              const glm::vec3& position = glm::vec3(0.0f),
              const glm::vec3& velocity = glm::vec3(0.0f),
              float linearDamping = 0.0f,
              bool isStatic = false);

    void setMass(float m);

    void applyForce(const glm::vec3& force);

    void applyImpulse(const glm::vec3& impulse);

    void integrate(float dt);

    void clearForces();

    glm::vec3 position;
    glm::vec3 velocity;

    float mass;
    float invMass;
    bool isStatic;

    float linearDamping;

private:
    glm::vec3 forceAccum;
};
