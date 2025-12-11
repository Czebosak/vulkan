#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera {
public:
    glm::vec3 position;
    glm::quat rotation;
    glm::mat4 projection;

    Camera();
    Camera(glm::vec3 position, glm::quat rotation, glm::mat4 projection);

    glm::mat4 get_matrix() const;
};
