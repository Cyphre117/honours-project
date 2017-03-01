#pragma once
#include <openvr.h>
#include <glm.hpp>
#include <string>
#include <GL/glew.h>

class VRSystem
{
public:
	static VRSystem* get();
	~VRSystem();

	void processVREvents();
	void bindEyeTexture( vr::EVREye eye );
	void resolveEyeTextures();
	void submitEyeTextures();

	// Getters
	bool hasInputFocus() { return !vr_system_->IsInputFocusCapturedByAnotherProcess(); }
	uint32_t renderTargetWidth() { return render_target_width_; }
	uint32_t renderTargetHeight() { return render_target_height_; }
	glm::mat4 projectionMartix( vr::Hmd_Eye eye );
	glm::mat4 eyePoseMatrix( vr::Hmd_Eye eye );

	// Helpers
	std::string getDeviceString( vr::TrackedDeviceIndex_t device, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* error );
	static glm::mat4 convertHMDmat3ToGLMMat4( vr::HmdMatrix34_t matrix );
	static glm::mat4 convertHMDmat4ToGLMmat4( vr::HmdMatrix44_t matrix );

private:
	VRSystem();
	static VRSystem* self_;
	bool init();

	vr::IVRSystem* vr_system_;

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
	} EyeBuffers[2]; // 0 is left, 1, is right
};