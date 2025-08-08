#pragma once

//
// 3D Camera
//
#define CAM_MOVE_FORWARD	0x1
#define CAM_MOVE_BACKWARD	0x2
#define CAM_MOVE_LEFT		0x3
#define CAM_MOVE_RIGHT		0x4


typedef struct _CameraInfo {

	DWORD dwCamMovementFlag;

	glm::vec3 v3Eye;
	glm::vec3 v3Center;
	glm::vec3 v3Up;
	glm::quat qCamOrientation;

	glm::vec2 v2CurrentMousePos;
	glm::vec2 v2OldMousePos;

	bool bMoveMouse = false;

	float fMouseSpeedFactor = 1.0f;

	float fAccFactor = 0.5f;

}CameraInfo;

void InitCamera(CameraInfo& camInfo, glm::vec3 eye, glm::vec3 center, glm::vec3 up);
glm::mat4 GetCameraViewMatrix(CameraInfo& camInfo);
void UpdateCamera(CameraInfo& camInfo);


