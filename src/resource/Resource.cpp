
#include <resource/Resource.h>

namespace orf_n {

Resource::Resource( std::string &descriptorFile ) {
	// load the descriptor
	m_descriptor = new ResourceDescriptor( descriptorFile );
	std::string s{ "Resource descriptor'" + m_descriptor->getDescriptorFile() + "' loaded." };
	Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::INFO, s );
	// load the data
	m_data = loadBinaryFile( m_descriptor->getDataFile(), &m_dataSize );
}

Resource::~Resource() {
	if( m_data != nullptr )
		delete[] m_data;
	if( m_descriptor != nullptr )
		delete m_descriptor;
}

unsigned char *Resource::getData( unsigned int *size ) const {
	*size = m_dataSize;
	return m_data;
}

bool Resource::update() {
	bool success{ false };
	if( m_descriptor->hasChanged() ) {
		delete[] m_data;
		m_dataSize = 0;
		m_descriptor->reload();
		std::string s{ "Resource descriptor'" + m_descriptor->getDescriptorFile() + "' reloaded." };
		Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::INFO, s );
		m_data = loadBinaryFile( m_descriptor->getDataFile(), &m_dataSize );
		success = true;
	}
	return success;
}

// static
unsigned char *Resource::loadBinaryFile( const std::string &file, unsigned int *size ) {
    std::ifstream fs( file.c_str(), std::ios::binary | std::ios::ate );
    if( !fs ) {
    	std::string s{ "Could not open file '" + file + "' for reading." };
    	Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::ERROR, "Loaded file '" + file + "'." );
    	throw std::runtime_error( s );
    }
    *size = (unsigned int)fs.tellg();
    unsigned char *data = new unsigned char[*size + 1];
    fs.seekg(0);
    fs.read( (char*) data, *size );
    fs.close();
    data[*size] = 0;
    Logbook::getInstance().logMsg( Logbook::RESOURCE, Logbook::INFO, "Loaded file '" + file + "'." );
    return data;
}

}
