/**
 * @file
 */

#pragma once

#include "video/Shader.h"

namespace frontend {

class ColorShader : public video::Shader {
public:
	bool init();
};

typedef std::shared_ptr<ColorShader> ColorShaderPtr;

}
