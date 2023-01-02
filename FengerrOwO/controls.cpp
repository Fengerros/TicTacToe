#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <iostream>

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}


glm::vec3 position = glm::vec3(1.5f, 5.0f, 3.0f);
float horizontalAngle = 3.14;
float verticalAngle = -1.10f;
float initialFoV = 45.0f;

float speedNormal = 3.0f;
float speedSprint = 8.0f;
float speed = 0.f;

float mouseSpeed = 0.003f;



void computeMatricesFromInputs(GLFWwindow* window, float width, float height) {

	static double lastTime = glfwGetTime();

	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	glfwSetCursorPos(window, width / 2, height / 2);

	horizontalAngle += mouseSpeed * float(width / 2 - xpos);
	verticalAngle += mouseSpeed * float(height / 2 - ypos);

	if (verticalAngle > 1.57f) verticalAngle = 1.57f;
	if (verticalAngle < -1.57f) verticalAngle = -1.57f;

	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	glm::vec3 upCamera = glm::cross(right, direction);

	glm::vec3 upWorld = glm::vec3(0.0f, 1.0f, 0.0f);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		position += upWorld * deltaTime * speed;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		position -= upWorld * deltaTime * speed;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
		speed = speedNormal;
	}
	else
	{
		speed = speedSprint;
	}

	float FoV = initialFoV;

	ProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	ViewMatrix = glm::lookAt(
		position,
		position + direction,
		upWorld                  
	);

	lastTime = currentTime;
}

glm::vec3 getPlayerDirection() {
	return glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
}

glm::vec3 getPlayerPosition() {
	return position;
}