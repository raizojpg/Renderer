#pragma once
#include <GL/glew.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

struct Plane {
    glm::vec3 normal;
    float d;
};

class Camera {
public:
    Camera();

    void Update();

    bool isPointInFrustum(const glm::vec3& point) const;
    bool isBoxInFrustum(const glm::vec3& min, const glm::vec3& max);

    void MoveForward(float delta);
    void MoveBackward(float delta);
    void MoveLeft(float delta);
    void MoveRight(float delta);

    void setObs(float x, float y, float z);
    void setObs(const glm::vec3& other);
    glm::vec3 getObs() const;

    void setRef(float x, float y, float z);
    void setRef(const glm::vec3& other);
    glm::vec3 getRef() const;

    void setVert(float x, float y, float z);
    void setVert(const glm::vec3& other);
    glm::vec3 getVert() const;

    void setView();
    glm::mat4 getView() const;

    void setProjection(const glm::mat4& proj);
    glm::mat4 getProjection() const;

    float& widthR();
    float& heightR();
    float& distR();
    float& alphaR();
    float& betaR();

private:
    void normalizePlane(Plane& plane);
    void extractFrustumPlanes(Plane planes[6]);

private:
    float width;
    float height;
    float znear;
    float fov;

    float alpha;
    float beta;
    float dist;

    glm::vec3 Obs;
    glm::vec3 Ref;
    glm::vec3 Vert;

    glm::mat4 view;
    glm::mat4 projection;
};