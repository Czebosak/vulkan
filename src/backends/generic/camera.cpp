#include "camera.hpp"

#include <fmt/core.h>

Camera::Camera() {}

Camera::Camera(glm::vec3 position, glm::quat rotation, glm::mat4 projection)
: position(position), rotation(rotation), projection(projection) {
	// invert the Y direction on projection matrix so that we are more similar
	// to opengl and gltf axis
	// projection[1][1] *= -1;
}

glm::mat4 Camera::get_matrix() const {
    glm::mat4 translation_mat = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotation_mat    = glm::mat4_cast(rotation);

    glm::mat4 mat = glm::inverse(translation_mat * rotation_mat);

    return projection * mat;
}
