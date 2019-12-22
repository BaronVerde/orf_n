
#pragma once

#include "GridMesh.h"
#include "renderer/Program.h"
#include "scene/Renderable.h"
#include "renderer/Texture2D.h"
#include <string>

class HydroTile : public orf_n::Renderable {
public:
	HydroTile();

	virtual ~HydroTile();

	virtual void setup() override final;

	virtual void render() override final;

	virtual void cleanup() override final;

private:

	std::unique_ptr<orf_n::Program> m_program{nullptr};

	std::unique_ptr<terrain::GridMesh> m_gridMesh{nullptr};

	std::unique_ptr<orf_n::Texture2D> m_liftrateTexture{nullptr};

	uint32_t m_extent{256};

	// Liftrate per time interval
	// Values between 0.0 (no lift rate) and 1.0(m_maxLiftrate)
	const std::vector<float> m_profileEW{ 0.2f, 0.4f, 0.6f, 0.4f, 0.6f, 0.8f, 0.6f, 0.4f };

	const std::vector<float> m_profileNS{ 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.0f, 1.0f };

	// Multiplier for the liftrate. Will be applied to every position in the liftrate texture
	float m_maxLiftrate{1};

	// 8 bit grayscale png has sufficient resolution for this one, built from lerping EW and NS profile.
	// Values range between 0 and 100 and must be mapped to [0..m_maxLiftrate] during displacement.
	std::string m_liftrateFilename{ "liftrate.png" };

};
