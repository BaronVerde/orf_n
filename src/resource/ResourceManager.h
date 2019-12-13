
#pragma once

#include <map>
#include <string>

namespace orf_n {

class Resource;

/**
 * Loads ResourceDescriptors and Resources and keeps records of the loaded resources.
 * Can reload resources when their descriptors change and cache them for later use.
 * The key to a resource is the resource descriptor's filename.
 * @todo: cache, dependencies, resource id
 */
class ResourceManager {
public:
    /**
    * Create a new resource manager and set the cache size.
    */
	ResourceManager( unsigned int cacheSize = 0 );

	/**
	* Empty the cache and delete all the resources in it and the manager.
	* The resources kept by this manager can not be used any more.
	*/
	virtual ~ResourceManager();

	/**
	 * Load a resource descriptor with given filename and fills out records.
	 * If the resource is in the cache, it is transferred to the loaded resources.
	 */
	void loadResource( std::string &descriptorFile );

	/**
	 * Deletes a resource with given filename and updates records,
	 * but keeps it in the cache. Oldest cache entry is deleted if
	 * cache reaches its size limit.
	 */
	// void deleteResource( std::string &descriptorFile );

	/**
	 * Deletes a resource from given filename and updates records,
	 * If resource is in the cache it is deleted as well.
	 */
	void purgeResource( std::string &descriptorFile );

	/**
	 * Checks loaded resources if they have been updated.
	 * Then reloads these resources.
	 */
	void updateResources();

	//void performHouseKeeping();

private:
	/**
	 * Size of the cache of deleted resources.
	 */
	unsigned int m_cacheSize;

	/**
	 * Resource counter for reference.
	 */
	static unsigned int m_resourceCounter;

	/**
	 * Cache of deleted resources, mapped by descriptor filenames.
	 */
	std::map<std::string, Resource *> m_cachedResources;

	/**
	 * Map of loaded resources, mapped by descriptor filenames.
	 */
	std::map<std::string, Resource *> m_loadedResources;

};

}
