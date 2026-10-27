#pragma once

class InputState
{
public:
	bool moveForward = false;
	bool moveBackward = false;
	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;
	bool jump = false;   // true only on the frame the jump key goes down

	bool toggleDebugCamera = false;
	bool debugCameraWasPressed = false;
};