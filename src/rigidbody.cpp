#include "RigidBody.h"
#include <cmath> // for powf

RigidBody::RigidBody(float m,
                     const glm::vec3& pos,
                     const glm::vec3& vel,
                     float damping,
                     bool isStatic)
    : position(pos),
    velocity(vel),
    mass(m),
    linearDamping(damping),
    forceAccum(0.0f),
    isStatic(false)
{}

void RigidBody::setMass(float m) {
    mass = m;
    invMass = (m > 0.f) ? 1.f / m : 0.f;
    isStatic = (m == 0.f); // mass 0 means static
}

void RigidBody::applyForce(const glm::vec3& force) {
    if (!isStatic) {
        forceAccum += force;
    }
}

void RigidBody::applyImpulse(const glm::vec3& impulse) {
    if (invMass <= 0.0f) return;
    velocity += impulse * invMass;
}

void RigidBody::integrate(float dt) {
    if (isStatic) return;

    glm::vec3 accel = forceAccum * invMass;


    velocity += accel * dt;

    if (linearDamping != 0.0f) {
        velocity *= powf(1.0f - linearDamping, dt);
    }

    position += velocity * dt;

    clearForces();
}

void RigidBody::clearForces() {
    forceAccum = glm::vec3(0.0f);
}

