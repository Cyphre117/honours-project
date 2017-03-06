#pragma once
#include <glm.hpp>
#include <openvr.h>

glm::mat4 convertHMDmat3ToGLMMat4( const vr::HmdMatrix34_t& matrix );
glm::mat4 convertHMDmat4ToGLMmat4( const vr::HmdMatrix44_t& matrix );