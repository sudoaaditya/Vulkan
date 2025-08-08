#include "vk.h"


void InitCamera(CameraInfo& camInfo, glm::vec3 eye, glm::vec3 center, glm::vec3 up) {

	camInfo.v3Eye = eye;
	camInfo.v3Center = center;
	camInfo.v3Up = up;

	camInfo.qCamOrientation = glm::lookAt(eye, center, up);
}

glm::mat4 GetCameraViewMatrix(CameraInfo& camInfo) {

	glm::mat4 translate = glm::translate(glm::mat4(1.0f), -camInfo.v3Eye);
	glm::mat4 rotate = glm::mat4_cast(camInfo.qCamOrientation);

	return rotate * translate;
}

glm::vec3 CalculateAcceleration(CameraInfo& camInfo) {

	glm::mat4 m4Cam = glm::mat4_cast(camInfo.qCamOrientation);
	
	glm::vec3 Forward, Up, Right;

	glm::vec3 Acc = glm::vec3(0.0f);

	Right = glm::vec3(m4Cam[0][0], m4Cam[1][0], m4Cam[2][0]);
	Up = glm::vec3(m4Cam[0][1], m4Cam[1][1], m4Cam[2][1]);
	Forward = -glm::vec3(m4Cam[0][2], m4Cam[1][2], m4Cam[2][2]);

	switch (camInfo.dwCamMovementFlag) {
		
	case CAM_MOVE_FORWARD:
		Acc = Forward;
		break;

	case CAM_MOVE_BACKWARD:
		Acc = -Forward;
		break;

	case CAM_MOVE_LEFT:
		Acc = -Right;
		break;

	case CAM_MOVE_RIGHT:
		Acc = Right;
		break;
	}

	return Acc;
}

glm::vec3 CalculateVelocity(CameraInfo& camInfo) {

	glm::vec3 acc = CalculateAcceleration(camInfo);

	return acc * camInfo.fAccFactor;
}

void SetUpVector(CameraInfo &camInfo)
{
	glm::mat4 View = GetCameraViewMatrix(camInfo);

	glm::vec3 Forward = glm::vec3(View[0][2], View[1][2], View[2][2]);

	camInfo.qCamOrientation = glm::lookAt(camInfo.v3Eye, camInfo.v3Eye - Forward, camInfo.v3Up);

}


void CalcCameraOrientation(CameraInfo& camInfo)
{
	glm::vec2 DeltaMouse = camInfo.v2CurrentMousePos - camInfo.v2OldMousePos;

	glm::quat rotateQuat = glm::quat(glm::vec3(camInfo.fMouseSpeedFactor * DeltaMouse.y, camInfo.fMouseSpeedFactor * DeltaMouse.x, 0.0f));

	camInfo.qCamOrientation = glm::normalize(rotateQuat * camInfo.qCamOrientation);

	SetUpVector(camInfo);
}



void UpdateCamera(CameraInfo& camInfo) {

	if (camInfo.bMoveMouse) {
		CalcCameraOrientation(camInfo);
		camInfo.v2OldMousePos = camInfo.v2CurrentMousePos;
	}
	else {
		glm::vec3 velocity = CalculateVelocity(camInfo);
		camInfo.v3Eye += velocity;
	}
}

