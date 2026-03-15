#include "Camera.h"
#include <cmath>

// ============================================================
// Construction
// ============================================================

Camera::Camera()
    : width(800.0f),
    height(600.0f),
    znear(1.0f),
    fov(30.0f),
    Obs(0.0f, 0.0f, 0.0f),
    Ref(0.0f, 0.0f, 0.0f),
    Vert(0.0f, 0.0f, 1.0f),
    alpha(0.0f),
    beta(0.0f),
    dist(1500.0f)
{
    Update();
}

// ============================================================
// Camera update
// ============================================================

void Camera::Update() {
    Obs = glm::vec3(
        Ref.x + dist * std::cos(alpha) * std::cos(beta),
        Ref.y + dist * std::cos(alpha) * std::sin(beta),
        Ref.z + dist * std::sin(alpha)
    );

    setView();
    setProjection(glm::infinitePerspective(fov, width / height, znear));
}

// ============================================================
// Frustum tests
// ============================================================

bool Camera::isPointInFrustum(const glm::vec3& point) const {
    glm::vec4 clipSpace = projection * view * glm::vec4(point, 1.0f);

    if (clipSpace.w == 0.0f) {
        return false;
    }

    glm::vec3 ndc = glm::vec3(clipSpace) / clipSpace.w;

    return ndc.x >= -1.0f && ndc.x <= 1.0f &&
        ndc.y >= -1.0f && ndc.y <= 1.0f &&
        ndc.z >= -1.0f && ndc.z <= 1.0f;
}

bool Camera::isBoxInFrustum(const glm::vec3& min, const glm::vec3& max) {
    Plane planes[6];
    extractFrustumPlanes(planes);

    for (int i = 0; i < 6; ++i) {
        const Plane& p = planes[i];

        glm::vec3 positiveVertex(
            (p.normal.x >= 0.0f) ? max.x : min.x,
            (p.normal.y >= 0.0f) ? max.y : min.y,
            (p.normal.z >= 0.0f) ? max.z : min.z
        );

        float distance = glm::dot(p.normal, positiveVertex) + p.d;
        if (distance < 0.0f) {
            return false;
        }
    }

    return true;
}

// ============================================================
// Setters / getters
// ============================================================

void Camera::setObs(float x, float y, float z) {
    setObs(glm::vec3(x, y, z));
}

void Camera::setObs(const glm::vec3& other) {
    Obs = other;
}

glm::vec3 Camera::getObs() const {
    return Obs;
}

void Camera::setRef(float x, float y, float z) {
    setRef(glm::vec3(x, y, z));
}

void Camera::setRef(const glm::vec3& other) {
    Ref = other;
}

glm::vec3 Camera::getRef() const {
    return Ref;
}

void Camera::setVert(float x, float y, float z) {
    setVert(glm::vec3(x, y, z));
}

void Camera::setVert(const glm::vec3& other) {
    Vert = other;
}

glm::vec3 Camera::getVert() const {
    return Vert;
}

void Camera::setView() {
    view = glm::lookAt(Obs, Ref, Vert);
}

glm::mat4 Camera::getView() const {
    return view;
}

void Camera::setProjection(const glm::mat4& proj) {
    projection = proj;
}

glm::mat4 Camera::getProjection() const {
    return projection;
}

// ============================================================
// Orbit center movement
// ============================================================

void Camera::MoveForward(float delta) {
    glm::vec3 dir = glm::normalize(Ref - Obs);
    Ref += dir * delta;
}

void Camera::MoveBackward(float delta) {
    glm::vec3 dir = glm::normalize(Ref - Obs);
    Ref -= dir * delta;
}

void Camera::MoveLeft(float delta) {
    glm::vec3 right = glm::normalize(glm::cross(Ref - Obs, Vert));
    Ref += right * delta;
}

void Camera::MoveRight(float delta) {
    glm::vec3 right = glm::normalize(glm::cross(Ref - Obs, Vert));
    Ref -= right * delta;
}

float& Camera::widthR() { return width; }
float& Camera::heightR() { return height; }
float& Camera::distR() { return dist; }
float& Camera::alphaR() { return alpha; }
float& Camera::betaR() { return beta; }

// ============================================================
// Internal helpers
// ============================================================

void Camera::normalizePlane(Plane& plane) {
    float len = glm::length(plane.normal);
    if (len == 0.0f) { return; }
    plane.normal /= len;
    plane.d /= len;
}

void Camera::extractFrustumPlanes(Plane planes[6]) {
    glm::mat4 m = projection * view;

    // Left
    planes[0].normal.x = m[0][3] + m[0][0];
    planes[0].normal.y = m[1][3] + m[1][0];
    planes[0].normal.z = m[2][3] + m[2][0];
    planes[0].d = m[3][3] + m[3][0];

    // Right
    planes[1].normal.x = m[0][3] - m[0][0];
    planes[1].normal.y = m[1][3] - m[1][0];
    planes[1].normal.z = m[2][3] - m[2][0];
    planes[1].d = m[3][3] - m[3][0];

    // Bottom
    planes[2].normal.x = m[0][3] + m[0][1];
    planes[2].normal.y = m[1][3] + m[1][1];
    planes[2].normal.z = m[2][3] + m[2][1];
    planes[2].d = m[3][3] + m[3][1];

    // Top
    planes[3].normal.x = m[0][3] - m[0][1];
    planes[3].normal.y = m[1][3] - m[1][1];
    planes[3].normal.z = m[2][3] - m[2][1];
    planes[3].d = m[3][3] - m[3][1];

    // Near
    planes[4].normal.x = m[0][3] + m[0][2];
    planes[4].normal.y = m[1][3] + m[1][2];
    planes[4].normal.z = m[2][3] + m[2][2];
    planes[4].d = m[3][3] + m[3][2];

    // Far
    planes[5].normal.x = m[0][3] - m[0][2];
    planes[5].normal.y = m[1][3] - m[1][2];
    planes[5].normal.z = m[2][3] - m[2][2];
    planes[5].d = m[3][3] - m[3][2];

    for (int i = 0; i < 6; ++i) {
        normalizePlane(planes[i]);
    }
}