
#pragma once

/**
 * Basic map projection and quadtree definition; the basic Ellipsoid Cube Map model.
 * Maps an ellipsoid to a cube
 * @todo: use class geodetc, plane
 */

#include <omath/mat3.h>

/**
 * The ellipsoid is defined by its semi-major axis (a) and semi-minor axis (b).
 * The cube side length is 2a. By convention, the cube is axis-aligned
 * with the origin at its center. Its six sides are numbered as follows:
 * 0 is at x=+a (front)
 * 1 is at y=+a (right)
 * 2 is at x=-a (back)
 * 3 is at y=-a (left)
 * 4 is at z=+a (top)
 * 5 is at z=-a (bottom)
 * Also by convention, the four directions are numbered as follows:
 * up = 0, right = 1, bottom = 2, left = 3.
 * Ellipsoid Cube Map coordinates (ecx,ecy) use the following coordinate system:
 *
 * ecy
 * +3  +-------+
 *  |  |       |
 * +2  |   4   |
 *  |  |       |
 * +1  +-------+-------+-------+-------+
 *  |  |       |       |       |       |
 *  0  |   0   |   1   |   2   |   3   |
 *  |  |       |       |       |       |
 * -1  +-------+-------+-------+-------+
 *  |  |       |
 * -2  |   5   |
 *  |  |       |
 * -3  +-------+
 *    -1---0---1---2---3---4---5---6---7  ecx
 * 
 * For sides 0-3, the following applies: the lower bound for ecx,ecy for each side is
 * inclusive, the higher bound is exclusive. Thus, the cartesian point (-a,-a,0) belongs
 * to area 0, and (+a,-a,0) belongs to area 1.
 * For side 4, both lower bounds and upper bounds of ecx,ecy are exclusive (i.e. side 4
 * includes none of its edges).
 * For side 5, both lower bounds and upper bounds of ecx,ecy are inclusive (i.e. side 5
 * includes all of its edges).
 * Inside each cube side, the Quadrilateralized Spherical Cube projection is used to map
 * the unit sphere to the unit cube. This is an equal-area projection and therefore
 * minimizes sampling problems (both in terms of sampling errors and in terms of data
 * storage efficiency). This projection is described in this paper:
 * E.M. O'Neill and R.E. Laubscher, "Extended Studies of a Quadrilateralized Spherical Cube Earth Data Base",
 * Naval Environmental Prediction Research Facility Tech. Report NEPRF 3-76 (CSC), May 1976.
 *
 * Ellipsoid Cube Map Quadtree:
 * A quadtree on an Ellipsoid Cube Map is a group of 6 quadtrees (i.e. it has 6 root
 * nodes), one for each side of the cube, with proper neighboring relations as
 * given by the cube edges. For example, the level 0 quad that represents the
 * top side of the cuboid is a neighbor of the level 0 quads on the front, back,
 * left, and right sides of the cuboid; similar for higher quadtree levels.
 * A quadtree node always either has zero children or four children. By convention,
 * the four children of a quad are numbered as follows:
 * +---+---+
 * | 0 | 1 |
 * +---+---+
 * | 2 | 3 |
 * +---+---+
 */
class ecm {
public:
    /** Cube sides */
	typedef enum : unsigned int {
        side_front = 0,
        side_right = 1,
        side_back = 2,
        side_left = 3,
        side_top = 4,
        side_bottom = 5
    } side_t;

    /** Directions */
    enum direction {
        dir_up = 0,
        dir_right = 1,
        dir_down = 2,
        dir_left = 3
    };

    /** Quad corners */
    enum corner {
        corner_tl = 0,
        corner_tr = 1,
        corner_bl = 2,
        corner_br = 3
    };

public:
    ecm( const double semiMinorAxis, const double semiMajorAxis );

    /**
     * Return true if two ECMs have identical axes.
     */
    bool operator==( const ecm& other ) const;

    /**
     * Return true if two ECMs are not identical.
     */
    bool operator!=( const ecm& other ) const;

    /**
     * Return true if this ECM is valid.
     */
    bool isValid() const;

    /**
     * Return the semi-major axis.
     */
    double getSemiMinorAxis() const;

    /**
     * Return the semi-major axis.
     */
    double getSemiMajorAxis() const;

    /** Coordinate conversions */
    /**
     * Extract the cube side from ecm coordinates.
     */
    static side_t ecmToSidenumber( const double ecm_x, const double ecm_y );

    /**
     * Return the side-relative coordinates in reference parameters.
     */
    static void ecmToSide( const double ecm_x, const double ecm_y,
    						 double &side_x, double &side_y, side_t &sidenumber );

    /**
     * Return ECM coordinates for side-relative coordinates.
     */
    static void sideToEcm( const side_t sidenumber, const double side_x, const double side_y,
    						 double &ecm_x, double &ecm_y);

    /**
     * Convert ECM coordinates to geocentric latitude and longitude.
     */
    void ecmToGeocentric( const double ecm_x, const double ecm_y,
    						double &geoc_lat, double &lon ) const;

    /**
     * Convert ECM coordinates to geodetic latitude and longitude.
     */
    void ecmToGeodetic( const double ecm_x, const double ecm_y,
    					  double &geod_lat, double &lon ) const;

    /**
     * Convert ECM coordinates to planetocentric cartesian coordinates.
     */
    void ecmToCartesian( const double ecm_x, const double ecm_y, omath::dvec3 &cart ) const;

    /**
     * Convert planetocentric cartesian coordinates to geocentric latitude and longitude.
     */
    void cartesianToGeocentric( const omath::dvec3 &cart, double &geoc_lat, double &lon ) const;

    /**
     * Convert geocentric coordinates to planetocentric cartesian coordinates.
     */
    void geocentricToCartesian( const double geoc_lat, const double lon, const double geoc_alt,
    							  omath::dvec3 &cart ) const;

    /**
     * Convert geocentric latitude to geodetic latitude.
     * (Note that there is no difference between geocentric and geodetic longitude).
     */
    double geocentricToGeodetic( const double geoc_lat ) const;

    /**
     * Convert geodetic latitude to geocentric latitude.
     * (Note that there is no difference between geocentric and geodetic longitude).
     */
    double geodeticToGeocentric( const double geod_lat ) const;

    /**
     * Convert geodetic coordinates to planetocentric cartesian coordinates.
     */
    void geodeticToCartesian( const double geod_lat, const double lon, const double geod_alt,
    							omath::dvec3 &cart ) const;

    /**
     * Convert planetocentric cartesian coordinates to geodetic coordinates.
     */
    void cartesianToGeodetic( const omath::dvec3 &cart, double &geod_lat, double &lon, double &geod_alt ) const;

    /**
     * Return the ellipsoid surface normal at the geodetic latitude and longitude.
     * The returned normal has length 1.
     */
    static void geodeticNormal( const double geod_lat, const double lon, omath::dvec3 &normal );

    /** ECM quadtree quads */
    /**
     * Compute the ECM coordinates of a point on a quad.
     * Takes quadtree side and level as well as quad x and y coordinates and quad relative
     * x and y positions of the point on the quad. Returns resulting ecm x and y coordinates.
     */
    static void quadToEcm( const side_t quad_side, const int quad_level,
    		const int quad_x, const int quad_y, const double qx, const double qy,
			double &ecm_x, double &ecm_y );
    
    /**
     * Compute the ECM coordinates of a corner of a quad.
     * Takes quadtree side and level as well as quad x and y coordinates and quad's
     * corner number. Returns resulting ecm x and y coordinates.
     */
    static void quadToEcm( const side_t quad_side, const int quad_level,
    		const int quad_x, const int quad_y, const int corner,
			double &ecm_x, double &ecm_y );

    /**
     * Returns the quad plane E for the given quad in Hesse normal form:
     * E: plane_normal * x = plane_distance
     * Takes quadtree side and level as well as quad x and y coordinates and quad's corners
     * in cartesian coordinates. Returns normalized quad plane normal and quad plane distance.
     */
    void quadPlane( const side_t quad_side, const int quad_level, const int quad_x, const int quad_y,
    		const omath::dvec3 &quad_tl_cart, const omath::dvec3 &quad_tr_cart,
			const omath::dvec3 &quad_bl_cart, const omath::dvec3 &quad_br_cart,
			omath::dvec3 &plane_normal, double &plane_distance ) const;

    /**
     * Estimate the maximum distance of the quad plane to the ellipsoid surface.
     * This estimation might be bad, but can be computed quickly.
     * The correct value is computed by quad_base_data().
     * Takes quadtree side and level as well as quad x and y coordinates.
     * Returns normalized quad plane normal and quad plane distance.
     */
    double maxQuadPlaneDistanceEstimation(
    		const side_t quad_side, const int quad_level, const int quad_x, const int quad_y,
			const omath::dvec3 &quad_plane_normal, double quad_plane_distance ) const;

    /**
     * Quads in ECM quadtrees are symmetrical: the top and bottom sides of the cube are mirrors of each other,
     * and the four remaining sides can be rotated into each other. Furthermore, the four quarters
     * of each side can be mirrored into each other. This can be used to reduce the number of quads
     * for which base data needs to be computed with quad_base_data().
     * This function returns a symmetry quad for an original quad, and tells you how to transform
     * the base data computed by quad_base_data() for this symmetry quad so that it applies to the
     * original quad: mirror the texture coordinates according to the mirror flags, and apply the
     * column-major transformation matrix.
     * Takes quadtree side and level as well as quad x and y coordinates.
     * Returns symmetry quadtree side, level, x and y coordinates, mirror flags horizontal/vertical
     * and the transformation matrix.
     */
    static void symmetryQuad(
    		const side_t quad_side, const int quad_level, const int quad_x, const int quad_y,
            int &sym_quad_side, int &sym_quad_level, int &sym_quad_x, int &sym_quad_y,
            bool &mirror_x, bool &mirror_y, omath::mat3 &matrix );

    /**
     * Compute base data for a quad.
     * It is recommended that this expensive function is only called for symmetry quads (see summetry_quad()).
     * Offsets between positions interpolated from the quad corners and the real ellipsoid surface are stored in the
     * offsets array. Ellipsoid surface normals are stored in the normals array (but only the x and y components; z can
     * be computed from them since the normals have length 1).
     * Takes quadtree side and level as well as quad x and y coordinates and the cartesian
     * coordinates of the four corners as well as the normalized quad plane normal and distance and
     * the quad size in samples and an eventual overlap soize in samples.
     * @todo \param offsets           Storage space for (quad_size + 2*quad_overlap)^2 offset vectors (3 floats).
     * \param normals           Storage space for (quad_size + 2*quad_overlap)^2 normal vectors (2 floats, only x and y).
     * \param max_quad_plane_distance Pointer to the maximum distance of the quad plane to the ellipsoid surface.
     */
    void quadBaseData(
    		const side_t quad_side, const int quad_level, const int quad_x, const int quad_y,
            const omath::dvec3 &quad_tl_cart, const omath::dvec3 &quad_tr_cart,
			const omath::dvec3 &quad_bl_cart, const omath::dvec3 &quad_br_cart,
            const omath::dvec3 &quad_plane_normal, const double quad_plane_distance,
            const side_t quad_size, const int quad_overlap,
            float* offsets, float* normals, double* max_quad_plane_distance) const;

private:
    /**
     * Reference ellipsoid's axes
     * @todo: eventually store squares and minor/major quotient
     */
    double m_semiMinorAxis;

    double m_semiMajorAxis;

};
