
#pragma once

#include <base/Logbook.h>
#include <map>
#include <string>
#include <ctime>		// time_t
#include <sys/stat.h>	// struct stat and stat() for file date
#include <map>
#include "../../extern/glad/glad.h"

namespace orf_n {

/**
 * Resource descriptors contain the descriptive elements of a resource class
 * in text format. These can be texture type, size, etc. for textures; uniforms and
 * their default values for shaders, etc. Elements are stored in Attributes.
 * They are loaded from a given pathname together with a timespace.
 * Should the descriptor file be touched after being loaded, it can be reloaded.
 * @todo: load different descriptor types and build descriptor structs.
 */
class ResourceDescriptor {
public:

	typedef enum {
		VERTEX_SHADER = 0, TESS_CONTROL_SHADER, TESS_EVAL_SHADER, GEOMETRY_SHADER, FRAGMENT_SHADER, COMPUTE_SHADER,
		TEXTURE1D, TEXTURE2D, TEXTURECUBE,
		VERTEX_MESH3D
	} resourceType;

	typedef enum {
		INT = 0, UINT, FLOAT, DOUBLE, STRING
	} attributeDataType;

	typedef struct {
		std::string name;
		attributeDataType dataType;
		GLint intValue;
		GLuint uintValue;
		GLfloat floatValue;
		GLdouble doubleValue;
		std::string stringValue;
	} attributeType;

	/**
	 * Defines the descriptor. The map holds the attributes by their names.
	 */
	typedef struct {
		resourceType type;
		std::string name;
		std::string dataFile;		// path to data file
		std::string descriptorFile; // path to descriptor file
		time_t lastLoaded;			// time stamp of last descriptor loading op.
		std::map<std::string, attributeType> attributes;
	} descriptorType;

	/**
	 * Creates a new resource descriptor. Loads the file and assigns descriptor.
	 * @param name Path to the descriptor file.
	 */
	ResourceDescriptor( const std::string &filepath );

	virtual ~ResourceDescriptor();

	/**
	 * Has the resource descriptor changed since last loaded ?
	 * Compares file modification date with last loaded data.
	 */
	bool hasChanged() const;

	/**
	 * Reloads the descriptor file.
	 */
	void reload();

	/**
	 * Get name of resource.
	 */
	const std::string &getResourceName() const;

	/**
	 * Get type of resource.
	 */
	const resourceType &getResourceType() const;

	/**
	 * Get name of data file.
	 */
	const std::string &getDataFile() const;

	/**
	 * Get name of descriptor file.
	 */
	const std::string &getDescriptorFile() const;

	/**
	 * Getter functions to get attribute value by attribute name.
	 */
	int getAttributeIntValue( std::string &attributeName ) const;

	unsigned int getAttributeUIntValue( std::string &attributeName ) const;

	float getAttributeFloatValue( std::string &attributeName ) const;

	double getAttributeDoubleValue( std::string &attributeName ) const;

	std::string getAttributeStringValue( std::string &attributeName ) const;

private:

	/**
	 * For ease of handling, the descriptor can be accessed publicly.
	 */
	descriptorType m_descriptor;

	/**
	 * Uitility function to load a descriptor. Used by ctor and reload().
	 */
	void load();

	/**
	 * Utility function to get the modified time stamp of a file.
	 * e.g. for comparison with lastLoaded.
	 * Throws an error of file is not found.
	 */
	static inline std::time_t getLastModified( const std::string &filename );

	/**
	 * Helper function for attribute value lookup.
	 */
	std::map<std::string, attributeType>::const_iterator lookupAttribute( std::string &name ) const;

};

}
