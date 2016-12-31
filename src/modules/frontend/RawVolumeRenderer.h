#pragma once

#include "voxel/polyvox/RawVolume.h"
#include "video/VertexBuffer.h"
#include "video/Camera.h"
#include "FrontendShaders.h"
#include "voxel/polyvox/Mesh.h"
#include "frontend/ShapeRenderer.h"
#include "frontend/Shadow.h"
#include "video/UniformBuffer.h"
#include "video/ShapeBuilder.h"
#include "video/Texture.h"
#include "video/DepthBuffer.h"

namespace frontend {

/**
 * @brief Handles the shaders, vertex buffers and rendering of a voxel::RawVolume
 *
 * @sa voxel::RawVolume
 */
class RawVolumeRenderer {
protected:
	voxel::RawVolume* _rawVolume;
	voxel::Mesh* _mesh;

	video::ShapeBuilder _shapeBuilder;
	frontend::ShapeRenderer _shapeRenderer;

	video::VertexBuffer _vertexBuffer;
	shader::ShadowmapShader& _shadowMapShader;
	shader::WorldShader& _worldShader;
	video::DepthBuffer _depthBuffer;
	frontend::Shadow _shadow;

	video::TexturePtr _whiteTexture;;

	int32_t _vertexBufferIndex = -1;
	int32_t _indexBufferIndex = -1;

	int32_t _aabbMeshIndex = -1;
	int32_t _gridMeshIndexXYNear = -1;
	int32_t _gridMeshIndexXYFar = -1;
	int32_t _gridMeshIndexXZNear = -1;
	int32_t _gridMeshIndexXZFar = -1;
	int32_t _gridMeshIndexYZNear = -1;
	int32_t _gridMeshIndexYZFar = -1;
	glm::vec3 _diffuseColor = glm::vec3(1.0, 1.0, 1.0);
	glm::vec3 _ambientColor = glm::vec3(0.2, 0.2, 0.2);
	glm::vec3 _sunDirection;

	bool _renderAABB;
	bool _renderGrid;
	bool _renderWireframe;
public:
	RawVolumeRenderer(bool renderAABB = false, bool renderWireframe = false, bool renderGrid = false);

	void render(const video::Camera& camera);

	/**
	 * @brief Updates the vertex buffers manually
	 * @sa extract()
	 */
	bool update(const std::vector<voxel::VoxelVertex>& vertices, const std::vector<voxel::IndexType>& indices);

	/**
	 * @brief Reextract the whole volume region and updates the vertex buffers.
	 * @sa update()
	 */
	bool extract();

	/**
	 * @param[in,out] volume The RawVolume pointer
	 * @return The old volume that was managed by the class, @c nullptr if there was none
	 *
	 * @sa volume()
	 */
	voxel::RawVolume* setVolume(voxel::RawVolume* volume);

	const voxel::Mesh* mesh() const;

	size_t numVertices() const;
	const voxel::VoxelVertex* vertices() const;

	size_t numIndices() const;
	const voxel::IndexType* indices() const;

	/**
	 * @sa setVolume()
	 */
	voxel::RawVolume* volume();

	bool renderAABB() const;
	void setRenderAABB(bool renderAABB);

	bool renderGrid() const;
	void setRenderGrid(bool renderGrid);

	bool renderWireframe() const;
	void setRenderWireframe(bool renderWireframe);

	void setAmbientColor(const glm::vec3& color);
	void setSunDirection(const glm::vec3& sunDirection);

	/**
	 * @sa shutdown()
	 */
	bool init();

	bool onResize(const glm::ivec2& position, const glm::ivec2& dimension);
	/**
	 * @return the managed voxel::RawVolume instance pointer, or @c nullptr if there is none set.
	 *
	 * @sa init()
	 */
	voxel::RawVolume* shutdown();
};

inline void RawVolumeRenderer::setSunDirection(const glm::vec3& sunDirection) {
	_sunDirection = sunDirection;
}

inline void RawVolumeRenderer::setAmbientColor(const glm::vec3& color) {
	_ambientColor = color;
}

inline voxel::RawVolume* RawVolumeRenderer::volume() {
	return _rawVolume;
}

inline bool RawVolumeRenderer::renderAABB() const {
	return _renderAABB;
}

inline bool RawVolumeRenderer::renderGrid() const {
	return _renderGrid;
}

inline bool RawVolumeRenderer::renderWireframe() const {
	return _renderWireframe;
}

inline void RawVolumeRenderer::setRenderAABB(bool renderAABB) {
	_renderAABB = renderAABB;
}

inline void RawVolumeRenderer::setRenderGrid(bool renderGrid) {
	_renderGrid = renderGrid;
}

inline void RawVolumeRenderer::setRenderWireframe(bool renderWireframe) {
	_renderWireframe = renderWireframe;
}

inline const voxel::Mesh* RawVolumeRenderer::mesh() const {
	return _mesh;
}

}
