#pragma once
#include <openvr.h>
#include <glm.hpp>
#include <string>
#include <GL/glew.h>
#include "shader_program.h"
#include "controller.h"

class VRSystem
{
public:
	static VRSystem* get();
	~VRSystem();

	void processVREvents();
	void manageDevices();
	void updatePoses();
	void drawControllers( vr::EVREye eye );
	void bindEyeTexture( vr::EVREye eye );
	void blitEyeTextures();
	void submitEyeTextures();

	/* GETTERS */
	uint32_t renderTargetWidth() { return render_target_width_; }
	uint32_t renderTargetHeight() { return render_target_height_; }
	glm::mat4 projectionMartix( vr::Hmd_Eye eye );
	glm::mat4 eyePoseMatrix( vr::Hmd_Eye eye );
	glm::mat4 viewMatrix( vr::Hmd_Eye eye ) { return eyePoseMatrix( eye ) * deviceTransform( vr::k_unTrackedDeviceIndex_Hmd ); }
	
	// Returns NULL if the controller is not ready
	Controller* leftControler() { return left_controller_.isInitialised() ? &left_controller_ : nullptr; }
	Controller* rightControler() { return right_controller_.isInitialised() ? &right_controller_ : nullptr; }

	std::string getDeviceString( vr::TrackedDeviceIndex_t device, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* error );

	inline vr::TrackedDevicePose_t devicePose( uint32_t device ) { return poses_[device]; }
	inline glm::mat4 deviceTransform( uint32_t device ) { return transforms_[device]; }
	inline bool hasInputFocus() { return !vr_system_->IsInputFocusCapturedByAnotherProcess(); }
	inline GLuint renderEyeTexture( vr::Hmd_Eye eye ) { return eye_buffers_[(eye == vr::Eye_Left ? 0 : 1)].render_frame_buffer; }
	inline GLuint resolveEyeTexture( vr::Hmd_Eye eye ) { return eye_buffers_[(eye == vr::Eye_Left ? 0 : 1)].resolve_frame_buffer; }

private:
	VRSystem();
	static VRSystem* self_;
	bool init();

	vr::IVRSystem* vr_system_;

	ShaderProgram controller_shader_;
	GLint controller_shader_modl_mat_locaton_;
	GLint controller_shader_view_mat_locaton_;
	GLint controller_shader_proj_mat_locaton_;

	Controller left_controller_;
	Controller right_controller_;

	vr::TrackedDevicePose_t poses_[vr::k_unMaxTrackedDeviceCount];
	glm::mat4 transforms_[vr::k_unMaxTrackedDeviceCount];

	uint32_t render_target_width_;
	uint32_t render_target_height_;
	float near_clip_plane_;
	float far_clip_plane_;

	struct EyeFrameBufferDesc {
		// Rendering is done into the 'render_frame_buffer'
		// The result is then copied into the 'resolve_frame_buffer' which is sent to the HMD
		GLuint render_depth;
		GLuint render_texture;
		GLuint render_frame_buffer;
		GLuint resolve_texture;
		GLuint resolve_frame_buffer;
	} eye_buffers_[2]; // 0 is left, 1, is right
};