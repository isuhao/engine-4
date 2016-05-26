/**
 * @file
 */

#include "MeshShader.h"

namespace frontend {

bool MeshShader::setup() {
	if (!loadProgram("shaders/mesh")) {
		return false;
	}

	if (!hasAttribute("a_pos")) {
		Log::error("no attribute a_pos found");
	}
	if (!hasAttribute("a_texcoords")) {
		Log::error("no attribute a_texcoords found");
	}
	if (!hasAttribute("a_norm")) {
		Log::error("no attribute a_norm found");
	}
	if (!hasUniform("u_projection")) {
		Log::error("no uniform u_projection found");
	}
	if (!hasUniform("u_model")) {
		Log::error("no uniform u_model found");
	}
	if (!hasUniform("u_view")) {
		Log::error("no uniform u_view found");
	}
	if (!hasUniform("u_texture")) {
		Log::error("no uniform u_texture found");
	}
	if (!hasUniform("u_viewdistance")) {
		Log::error("no uniform u_viewdistance found");
	}
	if (!hasUniform("u_fogrange")) {
		Log::error("no uniform u_fogrange found");
	}

	return true;
}

}
