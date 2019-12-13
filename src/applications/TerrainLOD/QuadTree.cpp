
#include <applications/TerrainLOD/Node.h>
#include <applications/TerrainLOD/QuadTree.h>
#include <applications/TerrainLOD/TerrainTile.h>
#include <base/Logbook.h>
#include <sstream>

namespace terrain {

QuadTree::QuadTree( const TerrainTile *const terrainTile ) :
		m_terrainTile{ terrainTile } {
	if( m_terrainTile->getHeightMap()->getExtent().x > 65535 || m_terrainTile->getHeightMap()->getExtent().y > 65535 ) {
		std::string s{ "Heightmap too large (>65535) for the quad tree." };
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::ERROR, s );
		throw std::runtime_error( s );
	}

	// Determine how many nodes will we use, and the size of the top (root) tree node.
	m_rasterSizeX = m_terrainTile->getHeightMap()->getExtent().x;
	m_rasterSizeZ = m_terrainTile->getHeightMap()->getExtent().y;
	int totalNodeCount = 0;
	m_topNodeSize = LEAF_NODE_SIZE;
	for( int i{ 0 }; i < NUMBER_OF_LOD_LEVELS; i++ ) {
		if( i != 0 )
			m_topNodeSize *= 2;
		int nodeCountX = ( m_rasterSizeX - 1 ) / m_topNodeSize + 1;
		int nodeCountZ = ( m_rasterSizeZ- 1 ) / m_topNodeSize + 1;
		totalNodeCount += nodeCountX * nodeCountZ;
	}
	// Initialize the tree memory, create tree nodes, and extract min/max Ys (heights)
	m_allNodes = new Node[totalNodeCount];
	int nodeCounter = 0;
	m_topNodeCountX = ( m_rasterSizeX - 1 ) / m_topNodeSize + 1;
	m_topNodeCountZ = ( m_rasterSizeZ - 1 ) / m_topNodeSize + 1;
	m_topLevelNodes = new Node**[m_topNodeCountZ];
	for( int z{ 0 }; z < m_topNodeCountZ; ++z ) {
		m_topLevelNodes[z] = new Node*[m_topNodeCountX];
		for( int x{ 0 }; x < m_topNodeCountX; ++x ) {
			m_topLevelNodes[z][x] = &m_allNodes[nodeCounter];
			nodeCounter++;
			m_topLevelNodes[z][x]->create( x * m_topNodeSize, z * m_topNodeSize,
					m_topNodeSize, 0, m_terrainTile, m_allNodes, nodeCounter );
		}
	}
	m_nodeCount = nodeCounter;
	if( m_nodeCount != totalNodeCount ) {
		std::ostringstream s;
		s << "Node counter (" << m_nodeCount << ") does not equal pre-calculated node count ("
		  << totalNodeCount << ").";
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::ERROR, s.str() );
		throw std::runtime_error( s.str() );
	}

	// Debug output
	std::ostringstream s;
	// Quad tree summary
	float sizeInMemory{ (float)m_nodeCount * ( sizeof( Node ) + sizeof( orf_n::AABB ) ) };
	s << "Quad tree created " << m_nodeCount << " Nodes; size in memory: " <<
			( sizeInMemory / 1024.0f ) << "kB.\n\t" << m_topNodeCountX << '*' <<
			m_topNodeCountZ << " top nodes.";
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s.str() );
	// Debug: List of all Nodes
	/*for( int i{ 0 }; i < m_nodeCount; ++i ) {
		s.str( std::string() );
		Node *n{ &m_allNodes[i] };
		s << "Node " << i << " Level " << n->getLevel() << " BB " << *n->getBoundingBox();
		if( n->isLeaf() )
			s << "; is leaf node.";
		else {
			s << "; child lvls: " << n->getUpperLeft()->getLevel() << '/' <<
					n->getUpperRight()->getLevel() << '/' << n->getLowerLeft()->getLevel() <<
					'/' << n->getLowerRight()->getLevel();
		}
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s.str() );
	}*/

}

QuadTree::~QuadTree() {
	if( m_allNodes != nullptr )
		delete[] m_allNodes;
	if( m_topLevelNodes != nullptr ) {
		for( int y{ 0 }; y < m_topNodeCountZ; ++y )
			delete[] m_topLevelNodes[y];
		delete[] m_topLevelNodes;
	}
}

const Node *QuadTree::getNodes() const {
	return m_allNodes;
}

int QuadTree::getNodeCount() const {
	return m_nodeCount;
}

void QuadTree::lodSelect( LODSelection *lodSelection ) const {
	for( int z{ 0 }; z < m_topNodeCountZ; ++z )
		for( int x{ 0 }; x < m_topNodeCountX; ++x )
			m_topLevelNodes[z][x]->lodSelect( lodSelection, false );
}

}
