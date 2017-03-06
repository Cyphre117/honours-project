#pragma once

#include "shader_program.h"
#include <glm.hpp>
#include <openvr.h>

// With help from: https://github.com/zecbmo/ViveSkyrim/blob/master/Source
// Because the openvr documentation is sparse...

class Controller
{
public:
	Controller();
	~Controller();

	void init( vr::TrackedDeviceIndex_t index, vr::IVRSystem* vr_system, const ShaderProgram& shader );
	void update();
	void draw();
	void handleEvent( vr::VREvent_t event );

	// Setters
	void setPose( vr::TrackedDevicePose_t pose ) { pose_ = pose; }

	// Getters
	bool isButtonDown( vr::EVRButtonId button ) const { return (state_.ulButtonPressed & vr::ButtonMaskFromId( button )) != 0; }
	bool isButtonPressed( vr::EVRButtonId button ) const { return ((state_.ulButtonPressed & vr::ButtonMaskFromId( button )) != 0) && ((prev_state_.ulButtonPressed & vr::ButtonMaskFromId( button )) == 0); }
	bool isButtonReleased( vr::EVRButtonId button ) const { return ((state_.ulButtonPressed & vr::ButtonMaskFromId( button )) == 0) && ((prev_state_.ulButtonPressed & vr::ButtonMaskFromId( button )) != 0); }

	glm::vec2 axis( vr::EVRButtonId button ) const;
	glm::vec2 prevAxis( vr::EVRButtonId button ) const;
	glm::vec2 axisDelta( vr::EVRButtonId button ) const;

	// Getters: pose attributes
	bool isPoseValid() const { return pose_.bPoseIsValid; }
	glm::mat4 deviceToAbsoluteTracking() const;
	glm::vec3 velocity() const { return glm::vec3( pose_.vVelocity.v[0], pose_.vVelocity.v[1], pose_.vVelocity.v[2] ); }
	glm::vec3 angularVelocity() const { return glm::vec3( pose_.vAngularVelocity.v[0], pose_.vAngularVelocity.v[1], pose_.vAngularVelocity.v[2] ); }
	vr::ETrackingResult trackingResult() const { return pose_.eTrackingResult; }

	// Getters: members
	bool isInitialised() const { return initialised_; }
	vr::TrackedDeviceIndex_t index() const { return index_; }
	vr::VRControllerState_t state() const { return state_; }
	vr::VRControllerState_t prevState() const { return prev_state_; }
	vr::TrackedDevicePose_t pose() const { return pose_; }

protected:
	bool initialised_;

	vr::IVRSystem* vr_system_;
	vr::TrackedDeviceIndex_t index_;

	// Stores button and axis information
	vr::VRControllerState_t state_;
	vr::VRControllerState_t prev_state_;

	// Stores location and orientation information
	vr::TrackedDevicePose_t pose_;

	ShaderProgram* shader_;
	GLuint vao_;
	GLuint model_mat_location_;
};
