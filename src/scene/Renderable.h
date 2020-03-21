
/**
 * Base class for renderaböle objects.
 * Defines the life cycle of an object.
 */

#pragma once

#include <omath/mat4.h>
#include <string>
#include <map>

namespace orf_n {

class Scene;

/**
 * Abstract class for the life cycle of a renderable object with 5 stages.
 * setup(), render() and cleanup() must be overridden.
 */
class Renderable {
public:
	/**
	 * Create a new renderable object.
	 * @param name Name of object. No special meaning.
	 */
	Renderable( const std::string &name );

	virtual ~Renderable();

	virtual const std::string &getName() const final;

	virtual void setScene( Scene *scene ) final;

	virtual void setup() = 0;

	virtual void prepareFrame();

	virtual void render() = 0;

	virtual void endFrame();

	virtual void cleanup() = 0;

protected:
	Scene *m_scene;

	std::string m_name;

};

}
