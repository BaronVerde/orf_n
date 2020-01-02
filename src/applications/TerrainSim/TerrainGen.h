
#pragma once

#include "omath/vec3.h"
#include <vector>

// I. river graph, flow data and node types

typedef enum { low = 0, medium, high } priorityIndex_t;
typedef enum { G = 0, F, E, DA, D, C, B, A, APLUS } riverTrajectory_t;
//typedef enum { narrowV, medium, wide } riverbedProfile_t;
typedef enum { bedrock, rocks, stones, gravel, sand, silt, clay } riverbed_t;

struct riverEdge_t {

};

struct riverNode_t {
	// y is elevation
	omath::vec3 position;
	priorityIndex_t priority;
	riverTrajectory_t riverType;
	float flow;
	uint32_t hortonStrahler;
	std::vector<riverEdge_t> edges;
};

struct junction_t {
	float connectionAngle;
	riverNode_t inflow1;
	riverNode_t inflow2;
	riverNode_t outflow;
};

typedef std::vector<riverNode_t> drainageNetwork_t;

static drainageNetwork_t network;
// initial candidate nodes are river mouths along the contour of the input domain
static drainageNetwork_t candidates;

static std::vector<omath::vec3> slope;
static std::vector<omath::vec3> contour;
static std::vector<omath::vec3> inputDomain;

// Returns node with highest priority (and lowest elevation if multiple nodes)
static riverNode_t &getHighestPriority( drainageNetwork_t &nodes ) {
	return nodes[0];
}

// delta controls length of network by limiting elevation differences
static riverNode_t &selectNodes( const float lowestElevation, const float deltaElevation ) {
	size_t size{ network.size() };
	candidates.clear();
	// create list of condidates within elevation range
	float lowest{ network[0].position.y };
	for( size_t i{1}; i < size; ++i ) {
		if( network[i].position.y >= lowestElevation && network[i].position.y <= lowestElevation + deltaElevation ) {
			candidates.push_back( network[i] );
			lowest = network[i].position.y < lowest ? network[i].position.y : lowest;
		}
	}
	// choose highest priority node
	return getHighestPriority( candidates );
}

static bool expandNode( const riverNode_t &nodeToExpand, const float slopeMagnitude ) {
	// ensure gradual elevation, each new node must be higher than its ancestor
	// create river slope map with the slope magnitude value (height variation), a local
	// parameter given by user or calculated procedurally
	// Horton-Strahler number s for a node: isLeaf then s=1,
	// else s = numChildren k, ecept when >=2 children are k, the s = k + 1
	// symmetric and asymmetric rule, probabilities for branching control length and branching of streams
	// cliff preventing condition: slope threshold k * distance(new node-node) > height difference between two nodes
	return true;
}

static bool updateNode() {
	return true;
}

static void computeVoronoiCells() {
	// compute voronoi cells from positions
	// determine water entries and outlets for each cell
	// watersheds for each cell, set of upstream connected cells
	// compute flow: 0.42 * area^0.69. Area = sum of connected cells in graph
	// simple rule takes into account evaporation and filtration
}

static void computeRidges() {
	// compute ridges: Voronoi cell ridges that don't intersect edges
	// crest height must be higher than adjacent river nodes, equal distance from each node
	// crest height = max(heights of adjacent nodes) + l(node positon) * distance to nodes,
	// which is equal for all nodes. l is[0..0.25] slope magnitude function.
	// describe if terrain will be plains, plateaus, valleys, or mountains
}

static void classifyNodes() {
	// assign classification for each edge based on slope and proximity to coast,
	// distance threshold (parameter or procedurally), from its two nodes,
	// junction types from lookup table. Geometry defined from in- and output flow
	// for seemless connection between edges.
	// mark rivermouth as delta if flow > threshold (idea: sediment freight),
	// braided if close to coasts (idea: if sediment rate is high is flow low),
	// meanders functionally
}

static void classifyRiver() {
	computeVoronoiCells();
	computeRidges();
	classifyNodes();
}

// II. Generate mesh

// build junctions and refine paths for a voronoi cell
static void refinePaths() {
	// cell has 1 outlet and 0..n entries -> n-1 confluences, set connection angle
	// do so for every entry
	// calculate winding path between junctions from perturbation, subpaths shorter than a parameter
}

static void computePrimitives() {
	// terrain primitives
}

static void generateModel() {
	// for every voronoi cell
	refinePaths();
	computePrimitives();
}
