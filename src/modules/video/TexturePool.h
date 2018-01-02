/**
 * @file
 */

#pragma once

#include "Texture.h"
#include <memory>

namespace video {

class TexturePool {
public:
	TexturePool();

	video::TexturePtr load(const char *name);

	bool init();
	void shutdown();
};

typedef std::shared_ptr<TexturePool> TexturePoolPtr;

}
