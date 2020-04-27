
/**
 * Base class for renderaböle objects.
 * Defines the life cycle of an object.
 */

#pragma once

#include <omath/mat4.h>
#include <string>
#include <map>

namespace orf_n {

class scene;

/**
 * Abstract class for the life cycle of a renderable object with 5 stages.
 * setup(), render() and cleanup() must be overridden.
 */
class renderable {
public:
	/**
	 * Create a new renderable object.
	 * @param name Name of object. No special meaning.
	 */
	renderable( const std::string &name );

	virtual ~renderable();

	virtual const std::string &get_name() const final;

	virtual void set_scene( scene *scene ) final;

	virtual void setup() = 0;

	virtual void prepareFrame();

	virtual void render() = 0;

	virtual void endFrame();

	virtual void cleanup() = 0;

protected:
	scene *m_scene;

	std::string m_name;

};

}
