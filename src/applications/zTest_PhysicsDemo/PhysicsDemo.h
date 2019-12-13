
#pragma once

#include <geometry/OBB.h>
#include <physics/Particle.h>
#include <physics/PhysicsSystem.h>
#include <renderer/DrawPrimitives.h>
#include <scene/Renderable.h>

class PhysicsDemo : public orf_n::Renderable {
public:
	PhysicsDemo();

	virtual ~PhysicsDemo();

	virtual void setup() override final;

	virtual void render() override final;

	virtual void cleanup() override final;

private:
	static const uint32_t NUM_PARTCLES{ 100 };

	orf_n::PhysicsSystem m_system{};

	const orf_n::DrawPrimitives &m_drawPrimitives{ orf_n::DrawPrimitives::getInstance() };

};
