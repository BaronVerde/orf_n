
#include <base/Logbook.h>
#include <resource/Resource.h>
#include <resource/ResourceManager.h>
#include <future>
#include <iostream>

namespace orf_n {

unsigned int ResourceManager::m_resourceCounter = 0;

ResourceManager::ResourceManager( unsigned int cacheSize ) :
	m_cacheSize{ cacheSize } {
	if( m_cacheSize > 0 ) {
		std::string s{ "Resource cache not yet supported. No caching will take place yet." };
		Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::WARNING, s );
	}
}

ResourceManager::~ResourceManager() {
	if( m_cachedResources.size() > 0 ) {
        for( auto r : m_cachedResources )
			delete r.second;
		m_cachedResources.clear();
	}
	if( m_loadedResources.size() > 0 ) {
		for( std::pair<std::string, Resource *> r : m_loadedResources )
			delete r.second;
		m_loadedResources.clear();
	}
}

void ResourceManager::loadResource( std::string &descriptorFile ) {
	// already loaded ?
	if( m_loadedResources.find( descriptorFile ) != m_loadedResources.end() ) {
        Resource *newRes{ new Resource( descriptorFile ) };
        m_loadedResources.emplace( descriptorFile, newRes );
	}
}

void ResourceManager::purgeResource( std::string &descriptorFile ) {
    //auto r{ m_loadedResources.find( descriptorFile ) };
    std::map<std::string, Resource *>::const_iterator r = m_loadedResources.find( descriptorFile );
    if( r != m_loadedResources.end() ) {
        delete r->second;
        m_loadedResources.erase( r );
        std::string s{ "Resource '" + descriptorFile + "' purged." };
		Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::INFO, s );
    }
}

void ResourceManager::updateResources() {
    //for( auto r : m_loadedResources ) {
	std::string s{ "Resource '" };
    for( std::pair<std::string, Resource *> r : m_loadedResources ) {
        if( r.second->update() )
        	s.append( r.first + "' reloaded." );
        else
        	s.append( r.first + "' unchanged, not reloaded." );
		Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::INFO, s );
    }
}

} // namespace orf_n
