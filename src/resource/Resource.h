
#pragma once

#include <resource/ResourceDescriptor.h>

namespace orf_n {

/**
 * A class to load resources from disk, with mechanisms for automatic
 * updates when resources' descriptors have changed.
 */
class Resource {
public:
	/**
	 * Load the resource's data from a given descriptor.
	 */
	Resource( std::string &descriptorFile );

	virtual ~Resource();

	/**
	 *  Checks if the descriptor file was modified, if so deletes the data, reloads
	 *  descriptor and then loads the data.
	 *  Returns true if the data has changed and was reloaded.
	 */
	bool update();

	/**
	 * Returns pointer to the data and size in a parameter.
	 */
	unsigned char *getData( unsigned int *size ) const;

private:

	/**
	 * Pointer to resource descriptor.
	 * @todo Public for easier access.
	 */
	ResourceDescriptor *m_descriptor{ nullptr };

	unsigned char *m_data{ nullptr };

	/**
	 * Size of data in bytes.
	 */
	unsigned int m_dataSize;

	/**
	 * Utility function to load the content of a file.
	 * Attention ! Returns a pointer to data allocated with new char[].
	 * Do not forget to delete[] !
	 * @param file the name of a file.
	 * @param[out] size returns the size of the file's content in bytes.
	 * @return the file's content.
	 */
	static unsigned char *loadBinaryFile( const std::string &file, unsigned int *size );

};

}
