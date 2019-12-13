
#include <resource/ResourceDescriptor.h>
#include <cstring>		// stof(), stoi()
#include <algorithm>	// remove_if()

namespace orf_n {

ResourceDescriptor::ResourceDescriptor( const std::string &filepath ) {
	m_descriptor.descriptorFile = filepath;
	load();
}

ResourceDescriptor::~ResourceDescriptor() {}

bool ResourceDescriptor::hasChanged() const {
	bool has{ m_descriptor.lastLoaded < getLastModified( m_descriptor.descriptorFile ) };
	Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::INFO, "Resource '" + m_descriptor.name + "' has " +
									 ( has ? "changed." : "not changed." ) );
	return has;
	// return m_lastLoaded < getLastModified( m_filepath ) };
}

void ResourceDescriptor::reload() {
	load();
}

const std::string &ResourceDescriptor::getResourceName() const {
	return m_descriptor.name;
}

const ResourceDescriptor::resourceType &ResourceDescriptor::getResourceType() const {
	return m_descriptor.type;
}

const std::string &ResourceDescriptor::getDataFile() const {
	return m_descriptor.dataFile;
}

const std::string &ResourceDescriptor::getDescriptorFile() const {
	return m_descriptor.descriptorFile;
}

int ResourceDescriptor::getAttributeIntValue( std::string &attributeName ) const {
	auto pos{ lookupAttribute( attributeName ) };
	if( pos->second.dataType != INT ) {
		std::string s{ "Attribute '" + attributeName + "' in descriptor '" + m_descriptor.name + "' is not an INT." };
		Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::ERROR, s );
		throw std::runtime_error( s );
	}
	return pos->second.intValue;
}

unsigned int ResourceDescriptor::getAttributeUIntValue( std::string &attributeName ) const {
	auto pos{ lookupAttribute( attributeName ) };
	if( pos->second.dataType != UINT ) {
		std::string s{ "Attribute '" + attributeName + "' in descriptor '" + m_descriptor.name + "' is not an UINT." };
		Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::ERROR, s );
		throw std::runtime_error( s );
	}
	return pos->second.uintValue;
}

float ResourceDescriptor::getAttributeFloatValue( std::string &attributeName ) const {
	auto pos{ lookupAttribute( attributeName ) };
	if( pos->second.dataType != FLOAT ) {
		std::string s{ "Attribute '" + attributeName + "' in descriptor '" + m_descriptor.name + "' is not a FLOAT." };
		Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::ERROR, s );
		throw std::runtime_error( s );
	}
	return pos->second.floatValue;
}

double ResourceDescriptor::getAttributeDoubleValue( std::string &attributeName ) const {
	auto pos{ lookupAttribute( attributeName ) };
	if( pos->second.dataType != DOUBLE ) {
		std::string s{ "Attribute '" + attributeName + "' in descriptor '" + m_descriptor.name + "' is not a DOUBLE." };
		Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::ERROR, s );
		throw std::runtime_error( s );
	}
	return pos->second.doubleValue;
}

std::string ResourceDescriptor::getAttributeStringValue( std::string &attributeName ) const {
	auto pos{ lookupAttribute( attributeName ) };
	if( pos->second.dataType != STRING ) {
		std::string s{ "Attribute '" + attributeName + "' in descriptor '" + m_descriptor.name + "' is not a STRING." };
		Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::ERROR, s );
		throw std::runtime_error( s );
	}
	return pos->second.stringValue;
}

std::map<std::string, ResourceDescriptor::attributeType>::const_iterator
ResourceDescriptor::lookupAttribute( std::string &name ) const {
	std::map<std::string, attributeType>::const_iterator pos{ m_descriptor.attributes.find( name ) };
	// auto pos{ m_descriptor.attributes.find( name ) };
	if( m_descriptor.attributes.end() == pos ) {
		std::string s{ "Could not find attribute '" + name + "' in descriptor '" + m_descriptor.name + "'." };
		Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::ERROR, s );
		throw std::runtime_error( s );
	}
	return pos;
}

/**
 * Utility function for remove_if()
 */
static inline bool unwanted( const unsigned char c ) {
	return ( c == ' ' || c == '\t' );
}

void ResourceDescriptor::load() {
	// file accessible ?
	std::ifstream file{ m_descriptor.descriptorFile, std::ios::binary  };
	if( file.is_open() ) {
		std::string s{ "Error opening resource descriptor file '" + m_descriptor.descriptorFile + "'." };
		Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::ERROR, s );
		throw std::runtime_error( s );
	}
	std::string thisLine;
	// build descriptor
	while( !file.eof() ) {
		getline( file, thisLine );
		// strip whitespace
		thisLine.erase( std::remove_if( thisLine.begin(), thisLine.end(), unwanted ), thisLine.end() );
		// continue to next line if a valid tag was found
		if( '#' != thisLine[0] && !thisLine.empty() ) {
			if( std::string::npos != thisLine.find( "Type" ) ) {
				std::string typestr{ thisLine.substr( thisLine.find(':')+1, thisLine.size() ) };
				if( 0 == typestr.compare( "vertexshader" ) ) {
					m_descriptor.type = VERTEX_SHADER;
					continue;
				} else if( 0 == typestr.compare( "texture1D" ) ) {
					m_descriptor.type = TEXTURE1D;
					continue;
				} else if( 0 == typestr.compare( "texture2D" ) ) {
					m_descriptor.type = TEXTURE2D;
					continue;
				} else if( 0 == typestr.compare( "mesh" ) ) {
					m_descriptor.type = VERTEX_MESH3D;
					continue;
				} else {
					std::string s{ "Error reading descriptor header " +
						m_descriptor.descriptorFile + ": unknown resource type" };
					Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::ERROR, s );
					throw std::runtime_error( s );
				}
			}
			if( std::string::npos != thisLine.find( "Name" ) ) {
				m_descriptor.name = thisLine.substr( thisLine.find(':')+1, thisLine.size() );
				continue;
			}
			if( std::string::npos != thisLine.find( "Source" ) ) {
				std::ifstream datafile{ m_descriptor.dataFile, std::ios::binary };
				if( !datafile.is_open() ) {
					std::string s{ "Resource data file '" + m_descriptor.dataFile + "' could not be opened." };
					Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::ERROR, s );
					throw std::runtime_error( s );
				} else
					datafile.close();
				m_descriptor.dataFile = thisLine.substr( thisLine.find(':')+1, thisLine.size() );
				continue;
			}

			if( std::string::npos != thisLine.find( "Attribute" ) ) {
				// cut off "Attribute:"
				std::string attributeStr{ thisLine.substr( thisLine.find(':')+1, thisLine.size() ) };
				attributeType att;
				// extract name
				att.name = attributeStr.substr( 0, attributeStr.find(';') );
				// cut off name;
				attributeStr = attributeStr.substr( attributeStr.find(';')+1, attributeStr.size() );
				// extract type
				std::string typeStr{ attributeStr.substr( 0, attributeStr.find(';') ) };
				if( 0 == strcmp( typeStr.c_str(), "int" ) )
					att.dataType = INT;
				else if( 0 == strcmp( typeStr.c_str(), "uint" ) )
					att.dataType = UINT;
				else if( 0 == strcmp( typeStr.c_str(), "float" ) )
					att.dataType = FLOAT;
				else if( 0 == strcmp( typeStr.c_str(), "double" ) )
					att.dataType = DOUBLE;
				else if( 0 == strcmp( typeStr.c_str(), "string" ) )
					att.dataType = STRING;
				else {
					std::string s{ "Unknown attribute data type in descriptor " + m_descriptor.descriptorFile };
					Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::ERROR, s );
					throw std::runtime_error( s );
				}
				// cut off type;
				attributeStr = attributeStr.substr( attributeStr.find(';')+1, attributeStr.size() );
				// extract value
				std::string valueStr{ attributeStr.substr( 0, attributeStr.find(';') ) };
				// convert to number
				switch( att.dataType ) {
				case INT :
					att.intValue = std::stoi( valueStr );
					break;
				case UINT :
					att.uintValue = std::stoi( valueStr );
					break;
				case FLOAT :
					att.floatValue = std::stof( valueStr );
					break;
				case DOUBLE :
					att.doubleValue = std::stod( valueStr );
					break;
				case STRING :
					att.stringValue = valueStr;
					break;
				}
				m_descriptor.attributes.insert( std::pair<std::string, attributeType>( att.name, att ) );
			}
		}
	}
	file.close();
	m_descriptor.lastLoaded = std::time( nullptr );
}

inline std::time_t ResourceDescriptor::getLastModified( const std::string &filename ) {
	struct stat attribs;
	if( 0 != stat( filename.c_str(), &attribs ) ) {
		std::string s{ "File '" + filename + "''s stats could not be read." };
		Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::ERROR, s );
		throw std::runtime_error( s );
	}
	return attribs.st_mtim.tv_sec;
}

}
