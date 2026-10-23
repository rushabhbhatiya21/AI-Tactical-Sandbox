#pragma once
#include "ICamera.h"
#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera : public ICamera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Camera constructor with vectors
    Camera(
        glm::vec3 position = glm::vec3(0.f),
        glm::vec3 up = glm::vec3(0.f, 1.f, 0.f),
        float yaw = YAW,
        float pitch = PITCH
    );


    // Camera constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw = YAW, float pitch = PITCH);

    glm::mat4 getViewMatrix() const override;
    glm::mat4 getProjectionMatrix() const override;
    glm::vec3 getPosition() const override;
    glm::mat4 calculate_lookAt_matrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp) const;
    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void processKeyboard(Camera_Movement direction, float deltaTime);
    void processMouseScroll(float yoffset);
    void setAspectRatio(float ratio);

private:
    float aspectRatio = 16.0f / 9.0f;
    float nearPlane = 0.1f;
    float farPlane = 200.0f;

    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();
};