
#include <applications/CubeToEllipsoid/ecm.h>
#include <cstring>

ecm::ecm( double semiMinorAxis, double semiMajorAxis ) :
    m_semiMinorAxis{semiMinorAxis}, m_semiMajorAxis{semiMajorAxis} {
	if( !isValid() )
		throw std::runtime_error( "ECM: Wrong axes data.\n" );
}

static inline void checkEcm( const double ecm_x, const double ecm_y ) {
    if( ( ecm_x < -1.0 && ecm_x > 7.0 ) || ( ecm_y < -3.0 && ecm_y > 3.0 ) ||
		( ecm_x > 1.0 && ( ecm_y < -1.0 || ecm_y > 1.0 ) ) )
    	throw std::runtime_error( "ECM coordinates out of bounds.\n" );
}

static inline void checkFace( const int sn, const double sx, const double sy ) {
    if( sn < 0 || sn > 5 || sx < -1.0 || sx > +1.0 || sy < -1.0 || sy > +1.0 )
    	throw std::runtime_error( "ECM coordinates on face out of bounds.\n" );
}

bool ecm::operator==( const ecm& other ) const {
	return( omath::compareFloat( m_semiMinorAxis, other.m_semiMinorAxis ) &&
			omath::compareFloat( m_semiMajorAxis, other.m_semiMajorAxis ) );
}

bool ecm::operator!=( const ecm& other ) const {
	return !(*this == other);
}

bool ecm::isValid() const {
	return( m_semiMinorAxis > 0.0 && m_semiMajorAxis >= m_semiMinorAxis );
}

double ecm::getSemiMinorAxis() const {
	return m_semiMinorAxis;
}

double ecm::getSemiMajorAxis() const {
	return m_semiMajorAxis;
}

ecm::side_t ecm::ecmToSidenumber( const double ecm_x, const double ecm_y ) {
    checkEcm( ecm_x, ecm_y );
    side_t sidenumber;
    if( ecm_y > +1.0 )
        sidenumber = side_top;
    else if( ecm_y <= -1.0 )
        sidenumber = side_bottom;
    else if( ecm_x < +1.0 )
        sidenumber = side_front;
    else if( ecm_x < +2.0 )
        sidenumber = side_right;
    else if( ecm_x < +3.0 )
        sidenumber = side_back;
    else
        sidenumber = side_left;
    return sidenumber;
}

void ecm::ecmToSide( const double ecm_x, const double ecm_y, double &side_x, double &side_y, side_t &sidenumber ) {
    checkEcm( ecm_x, ecm_y );
    side_t sn;
    double sx, sy;
    if (ecm_y > +1.0) {
        sn = side_top;
        sx = ecm_x;
        sy = ecm_y - 2.0;
    } else if (ecm_y <= -1.0) {
        sn = side_bottom;
        if (ecm_y >= -1.0) {
            // special case: borders of sides 0..4
            if (ecm_x < 1.0) {
                sx = ecm_x;
                sy = +1.0;
            } else if (ecm_x < 3.0) {
                sx = +1.0;
                sy = -(ecm_x - 2.0);
            } else if (ecm_x < 5.0) {
                sx = -(ecm_x - 4.0);
                sy = -1.0;
            } else /* ecm_x <= 7.0 */ {
                sx = -1.0;
                sy = ecm_x - 6.0;
            }
        } else {
            sx = ecm_x;
            sy = ecm_y + 2.0;
        }
    } else {
        if( ecm_x < 1.0 )
            sn = side_front;
        else if( ecm_x < 3.0 )
            sn = side_right;
        else if( ecm_x < 5.0 )
            sn = side_back;
        else
            sn = side_left;
        sx = ecm_x - ( 2 * sn );
        sy = ecm_y;
    }
    side_x = sx;
    side_y = sy;
    sidenumber = sn;
    checkFace( sn, sx, sy );
}

void ecm::sideToEcm( const side_t sidenumber, const double side_x, const double side_y,
		double &ecm_x, double &ecm_y ) {
    checkFace( sidenumber, side_x, side_y );
    double ex, ey;
    if( sidenumber == side_top ) {
        ex = side_x;
        ey = side_y + 2.0f;
    } else if( sidenumber == side_bottom ) {
        ex = side_x;
        ey = side_y - 2.0f;
    } else {
        ex = side_x + ( 2 * sidenumber );
        ey = side_y;
    }
    ecm_x = ex;
    ecm_y = ey;
    checkEcm( ex, ey);
}

void ecm::ecmToGeocentric( const double ecm_x, const double ecm_y, double &geoc_lat, double &lon ) const {
    checkEcm( ecm_x, ecm_y );
    side_t sidenumber;
    double side_x, side_y;
    ecmToSide( ecm_x, ecm_y, side_x, side_y, sidenumber );
    /* Convert the side coordinates to the mu and nu angles as used by QSC.
     * This depends on the area of the cube face. */
    double nu = std::atan( omath::magnitude( omath::dvec2( side_x, side_y ) ) );
    double mu = std::atan2(side_y, side_x);
    int area;
    if (side_x >= 0.0 && side_x >= std::abs(side_y))
        area = 0;
    else if (side_y >= 0.0 && side_y >= std::abs(side_x)) {
        area = 1;
        mu -= omath::PI_OVER_TWO;
    } else if (side_x < 0.0 && -side_x >= std::abs(side_y)) {
        area = 2;
        mu = mu < 0.0 ? mu + M_PI : mu - M_PI;
    } else {
        area = 3;
        mu += omath::PI_OVER_TWO;
    }
    if( std::abs(mu) > omath::PI_OVER_FOUR )
    	throw std::runtime_error( "ECM: ecm to geocenrtic coord conversion error.\n" );

    /* Compute phi and theta for the area of definition.
     * The inverse projection is not described in the original paper, but some
     * good hints can be found here (as of 2011-12-14):
     * http://fits.gsfc.nasa.gov/fitsbits/saf.93/saf.9302
     * (search for "Message-Id: <9302181759.AA25477 at fits.cv.nrao.edu>") */
    double t = ( M_PI / 12.0 ) * std::tan( mu );
    double tantheta = std::sin(t) / (std::cos(t) - (1.0 / omath::SQRT2));
    double theta = std::atan(tantheta);
    double cosmu = std::cos(mu);
    double tannu = std::tan(nu);
    double cosphi = 1.0 - cosmu * cosmu * tannu * tannu * (1.0 - std::cos(std::atan(1.0 / std::cos(theta))));
    cosphi = omath::clamp(cosphi, -1.0, +1.0);

    /* Apply the result to the real area on the cube face.
     * For the top and bottom face, we can compute phi and lam directly.
     * For the other faces, we must use unit sphere cartesian coordinates
     * as an intermediate step. */
    omath::dvec2 geoc;
    if (sidenumber == side_top) {
        geoc[0] = omath::PI_OVER_TWO - std::acos(cosphi);
        if (area == 0)
            geoc[1] = theta + omath::PI_OVER_TWO;
        else if (area == 1)
            geoc[1] = (theta < 0.0 ? theta + M_PI : theta - M_PI);
        else if (area == 2)
            geoc[1] = theta - omath::PI_OVER_TWO;
        else /* area == 3 */
            geoc[1] = theta;
    } else if (sidenumber == side_bottom) {
        geoc[0] = acos(cosphi) - omath::PI_OVER_TWO;
        if (area == 0)
            geoc[1] = -theta + omath::PI_OVER_TWO;
        else if (area == 1)
            geoc[1] = -theta;
        else if (area == 2)
            geoc[1] = -theta - omath::PI_OVER_TWO;
        else /* area == 3 */
            geoc[1] = (theta < 0.0 ? -theta - M_PI : -theta + M_PI);
    } else {
        /* Compute phi and lambda via cartesian unit sphere coordinates q,r,s. */
        double q, r, s, t;
        q = cosphi;
        t = q * q;
        if (t >= 1.0)
            s = 0.0;
        else
            s = std::sqrt(1.0 - t) * std::sin(theta);
        t += s * s;
        if (t >= 1.0)
            r = 0.0;
        else
            r = std::sqrt(1.0 - t);
        /* Rotate q,r,s into the correct area. */
        if (area == 1) {
            t = r;
            r = -s;
            s = t;
        } else if (area == 2) {
            r = -r;
            s = -s;
        } else if (area == 3) {
            t = r;
            r = s;
            s = -t;
        }
        /* Rotate q,r,s into the correct cube face. */
        if (sidenumber == side_right) {
            t = q;
            q = -r;
            r = t;
        } else if (sidenumber == side_back) {
            q = -q;
            r = -r;
        } else if (sidenumber == side_left) {
            t = q;
            q = r;
            r = -t;
        }
        /* Now compute phi and lam from the unit sphere coordinates q,r,s. */
        geoc[0] = std::acos(-s) - omath::PI_OVER_TWO;
        geoc[1] = std::atan2(r, q);
    }
    if( ( ( geoc[0] < -omath::PI_OVER_TWO ) && ( geoc[0] > + omath::PI_OVER_TWO ) ) ||
    		geoc[1] < -M_PI || geoc[1] > M_PI )
    	throw std::runtime_error( "ECM: ecm to geocenrtic coord conversion error.\n" );
    geoc_lat = geoc[0];
    lon = geoc[1];
}

void ecm::ecmToGeodetic( double ecm_x, double ecm_y, double &geod_lat, double &lon ) const {
    checkEcm( ecm_x, ecm_y );
    double geoc_lat;
    ecmToGeocentric( ecm_x, ecm_y, geoc_lat, lon );
    geod_lat = geocentricToGeodetic(geoc_lat);
}

void ecm::ecmToCartesian( double ecm_x, double ecm_y, omath::dvec3 &cart ) const {
    checkEcm(ecm_x, ecm_y);
    double geoc_lat, lon;
    ecmToGeocentric(ecm_x, ecm_y, geoc_lat, lon);
    geocentricToCartesian(geoc_lat, lon, 0.0, cart);
}

void ecm::cartesianToGeocentric(const omath::dvec3 &cart, double &geoc_lat, double &lon) const {
	geoc_lat = std::acos( -cart[2] / omath::magnitude(cart)) - omath::PI_OVER_TWO;
    lon = std::atan2(cart[1], cart[0]);
}

void ecm::geocentricToCartesian(double geoc_lat, double lon, double geoc_alt, omath::dvec3 &cart) const {
    double sinlat, coslat;
    double sinlon, coslon;
    sincos(geoc_lat, &sinlat, &coslat);
    sincos(lon, &sinlon, &coslon);

    // From geocentric to unit-sphere-cartesian:
    omath::dvec3 c;
    c.x = coslat * coslon;
    c.y = coslat * sinlon;
    c.z = sinlat;

    // From sphere-cartesian to ellipsoid-cartesian:
    double a2 = m_semiMinorAxis * m_semiMinorAxis;
    double b2 = m_semiMajorAxis * m_semiMajorAxis;
    c /= std::sqrt(c.x * c.x / a2 + c.y * c.y / a2 + c.z * c.z / b2);

    // Apply altitude:
    if (geoc_alt < 0.0 || geoc_alt > 0.0)
        c += geoc_alt * omath::normalize(c);
    cart = c;
}

double ecm::geocentricToGeodetic(double geoc_lat) const {
    double t = m_semiMajorAxis / m_semiMinorAxis;
    double tanlat = std::tan(geoc_lat);
    double xa = m_semiMajorAxis / std::sqrt(tanlat * tanlat + t * t);
    double geod_lat = std::atan(std::sqrt(m_semiMinorAxis * m_semiMinorAxis - xa * xa) / (t * xa));
    if (geoc_lat < 0.0)
        geod_lat = -geod_lat;
    return geod_lat;
}

double ecm::geodeticToGeocentric(double geod_lat) const {
    double t = m_semiMajorAxis / m_semiMinorAxis;
    return std::atan((t * t) * std::tan(geod_lat));
}

void ecm::geodeticToCartesian(double geod_lat, double lon, double geod_alt, omath::dvec3 &cart) const {
    double sinlat, coslat;
    double sinlon, coslon;
    sincos(geod_lat, &sinlat, &coslat);
    sincos(lon, &sinlon, &coslon);

    double t = m_semiMajorAxis / m_semiMinorAxis;
    double e2 = (1.0 - t) * (1.0 + t);
    double N = m_semiMinorAxis / std::sqrt(1.0 - e2 * sinlat * sinlat);

    cart[0] = (N + geod_alt) * coslat * coslon;
    cart[1] = (N + geod_alt) * coslat * sinlon;
    cart[2]= (N * (1.0 - e2) + geod_alt) * sinlat;
}

void ecm::cartesianToGeodetic(const omath::dvec3 &cart, double &geod_lat, double &lon, double &geod_alt) const {
    // See http://en.wikipedia.org/wiki/Geodetic_system
    // in its version from 2009-05-13 under 'Convert ECEF to WGS-84'.
    // XXX: It is unclear if this single-step algorithm is useful for anything other than WGS84!
	// @todo Work with Geometry/Ellipsoid !
    const double a2 = m_semiMinorAxis * m_semiMinorAxis;
    const double b2 = m_semiMajorAxis * m_semiMajorAxis;
    const double z2 = cart[2] * cart[2];

    const double t = m_semiMajorAxis / m_semiMinorAxis;
    const double e2 = (1.0 - t) * (1.0 + t);
    const double ep2 = e2 / (t * t);
    const double r2 = cart[0] * cart[0] + cart[1] * cart[1];
    const double r = std::sqrt(r2);
    const double E2 = a2 - b2;
    const double F = 54.0 * b2 * z2;
    const double G = r2 + (1.0 - e2) * z2 - e2 * E2;
    const double c = (e2 * e2 * F * r2) / (G * G * G);
    const double s = std::cbrt(1.0 + c + std::sqrt(c * c + 2.0 * c));
    const double P = F / (3.0 * (s + 1.0 / s + 1.0) * (s + 1.0 / s + 1.0) * G * G);
    const double Q = std::sqrt(1.0 + 2.0 * e2 * e2 * P);
    const double ro_radicand = (a2 / 2.0) * (1.0 + 1.0 / Q) - ((1.0 - e2) * P * z2) / (Q * (1.0 + Q)) - P * r2 / 2.0;
    const double ro = -(e2 * P * r) / (1.0 + Q) + (ro_radicand >= 0.0 ? std::sqrt(ro_radicand) : 0.0);
    const double tmp = (r - e2 * ro) * (r - e2 * ro);
    const double U = std::sqrt(tmp + z2);
    const double V = std::sqrt(tmp + (1.0 - e2) * z2);
    const double zo = (b2 * cart[2]) / (m_semiMinorAxis * V);

    geod_alt = U * (1.0 - b2 / (m_semiMinorAxis * V));
    geod_lat = (r > 0.0 ? std::atan((cart[2] + ep2 * zo) / r) : omath::PI_OVER_TWO);
    lon = std::atan2(cart[1], cart[0]);
}

void ecm::geodeticNormal( const double geod_lat, const double lon, omath::dvec3 &normal) {
    double sinlat, coslat;
    double sinlon, coslon;
    sincos(geod_lat, &sinlat, &coslat);
    sincos(lon, &sinlon, &coslon);
    normal = normalize( omath::dvec3(coslat * coslon, coslat * sinlon, sinlat));
}

void ecm::quadToEcm( const side_t quad_side, const int quad_level, const int quad_x, const int quad_y,
		const double qx, const double qy, double &ecm_x, double &ecm_y ) {
    if( qx < 0.0 || qx > 1.0 || qy < 0.0 || qy > 1.0 )
    	throw std::runtime_error( "ECM: quad to ecm coord conversion error.\n" );
    int quads_in_level = (1 << quad_level);
    if( quad_x < 0 || quad_x > quads_in_level || quad_y < 0 ||
    	quad_y > quads_in_level || quad_side < 0 || quad_side > 6 )
    	throw std::runtime_error( "ECM: quad to ecm coord conversion error.\n" );
    sideToEcm( quad_side,
            2.0 * ((quad_x                        + qx) / quads_in_level) - 1.0,
            2.0 * (((quads_in_level - 1 - quad_y) + qy) / quads_in_level) - 1.0,
            ecm_x, ecm_y);
}

void ecm::quadToEcm( const side_t quad_side, const int quad_level, const int quad_x,
		const int quad_y, const int corner, double &ecm_x, double &ecm_y ) {
    omath::dvec2 qxy;
    if (corner == corner_tl)
        qxy = omath::dvec2(0.0, 1.0);
    else if (corner == corner_tr)
        qxy = omath::dvec2(1.0, 1.0);
    else if (corner == corner_br)
        qxy = omath::dvec2(1.0, 0.0);
    else
        qxy = omath::dvec2(0.0, 0.0);
    quadToEcm(quad_side, quad_level, quad_x, quad_y, qxy.x, qxy.y, ecm_x, ecm_y);
}

void ecm::quadPlane( const side_t quad_side, const int quad_level, const int quad_x, const int quad_y,
        const omath::dvec3 &quad_tl_cart, const omath::dvec3 &quad_tr_cart,
		const omath::dvec3 &quad_bl_cart, const omath::dvec3 &quad_br_cart,
		omath::dvec3 &plane_normal, double &plane_distance ) const {
    // The quad plane normal is the ellipsoid normal at the quad center.
    double quad_center_ecm_x, quad_center_ecm_y;
    quadToEcm(quad_side, quad_level, quad_x, quad_y, 0.5, 0.5, quad_center_ecm_x, quad_center_ecm_y);
    double quad_center_geod_lat, quad_center_lon;
    ecmToGeodetic(quad_center_ecm_x, quad_center_ecm_y, quad_center_geod_lat, quad_center_lon);
    omath::dvec3 quad_plane_normal;
    geodeticNormal(quad_center_geod_lat, quad_center_lon, quad_plane_normal );
    // The quad plane distance is the smallest distance of the four quad corners to the origin.
    double quad_tl_distance = omath::dot(quad_plane_normal, quad_tl_cart);
    double quad_tr_distance = omath::dot(quad_plane_normal, quad_tr_cart);
    double quad_bl_distance = omath::dot(quad_plane_normal, quad_bl_cart);
    double quad_br_distance = omath::dot(quad_plane_normal, quad_br_cart);
    double quad_plane_distance =
    		std::min( std::min( std::min( quad_tl_distance, quad_tr_distance ), quad_bl_distance ), quad_br_distance );
    if( quad_plane_distance < 0.0 )
    	throw std::runtime_error( "ECM: Distance of quad plane <0.\n" );
    // Return the result
    plane_normal = quad_plane_normal;
    plane_distance = quad_plane_distance;
}

double ecm::maxQuadPlaneDistanceEstimation( const side_t quad_side, const int quad_level,
		const int quad_x, const int quad_y, const omath::dvec3 &quad_plane_normal, double quad_plane_distance) const {
    // Use the distance at the quad center as the max distance estimation.
    double ecm_x, ecm_y;
    quadToEcm(quad_side, quad_level, quad_x, quad_y, 0.5, 0.5, ecm_x, ecm_y);
    omath::dvec3 cart;
    ecmToCartesian(ecm_x, ecm_y, cart );
    double estimation = omath::dot( omath::dvec3(quad_plane_normal), cart) - quad_plane_distance;
    if(estimation < 0.0)
    	throw std::runtime_error( "ECM: Distance estimation of quad plane < 0.\n" );
    return estimation;
}

void ecm::symmetryQuad( const side_t quad_side, const int quad_level, const int quad_x, const int quad_y,
        int &sym_quad_side, int &sym_quad_level, int &sym_quad_x, int &sym_quad_y,
        bool &mirror_x, bool &mirror_y, omath::mat3 &matrix ) {
    int sq_side = quad_side;
    int sq_level = quad_level;
    int sq_x = quad_x;
    int sq_y = quad_y;
    bool mx = false;
    bool my = false;
    omath::mat3 M{ 1.0f };

    if (sq_side == ecm::side_bottom)
        sq_side = ecm::side_top;
    else if (sq_side != ecm::side_top && sq_side != ecm::side_front)
        sq_side = ecm::side_front;

    int quads_in_level = (1 << sq_level);
    if (sq_x >= quads_in_level / 2) {
        sq_x = quads_in_level - 1 - sq_x;
        mx = true;
        M[1][1] = -1.0f;
    }
    if (sq_y >= quads_in_level / 2) {
        sq_y = quads_in_level - 1 - sq_y;
        my = true;
        if (quad_side <= 3)
            M[2][2] = -1.0f;
        else
            M[0][0] = -1.0f;
    }
    // correction for the destination side
    if (quad_side == ecm::side_right) {
    	omath::mat3 m{ { 0.0f, -1.0f,  0.0f }, { +1.0f,  0.0f,  0.0f }, { 0.0f,  0.0f, +1.0f } };
        M *= m;
    } else if (quad_side == ecm::side_back) {
    	omath::mat3 m{ { -1.0f,  0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f }, { 0.0f,  0.0f, +1.0f } };
        M *= m;
    } else if (quad_side == ecm::side_left) {
    	omath::mat3 m{ { 0.0f, +1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f,  0.0f, +1.0f } };
        M *= m;
    } else if (quad_side == ecm::side_bottom) {
    	omath::mat3 m{ { -1.0f,  0.0f,  0.0f }, { 0.0f, +1.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } };
        M *= m;
    }

    sym_quad_side = sq_side;
    sym_quad_level = sq_level;
    sym_quad_x = sq_x;
    sym_quad_y = sq_y;
    if (mirror_x)
    	mirror_x = mx;
    if (mirror_y)
        mirror_y = my;
    matrix = M;
}

static void goLeft( const ecm::side_t from_side, const omath::dvec2 &from_sqxy, const double d,
		ecm::side_t &side, omath::dvec2 &sqxy ) {
	if( from_side == ecm::side_top ) {
        side = ecm::side_left;
        sqxy.x = 1.0 - from_sqxy.y;
        sqxy.y = 1.0 - d;
    } else if (from_side == ecm::side_bottom) {
        side = ecm::side_left;
        sqxy.x = from_sqxy.x;
        sqxy.y = d;
    } else {
    	switch( from_side ) {
    		case ecm::side_front: side = ecm::side_left; break;
    		case ecm::side_right: side = ecm::side_front; break;
    		case ecm::side_back: side = ecm::side_right; break;
    		case ecm::side_left: side = ecm::side_front; break;
    		default: break;
    	}
        sqxy.x = 1.0 - d;
        sqxy.y = from_sqxy.y;
    }
}

static void goRight( const ecm::side_t from_side, const omath::dvec2 &from_sqxy, const double d,
		ecm::side_t &side, omath::dvec2& sqxy) {
    if (from_side == ecm::side_top) {
        side = ecm::side_right;
        sqxy.y = 1.0 - d;
        sqxy.x = from_sqxy.x;
    } else if (from_side == ecm::side_bottom) {
        side = ecm::side_right;
        sqxy.y = d;
        sqxy.x = 1.0 - from_sqxy.y;
    } else {
    	switch( from_side ) {
    		case ecm::side_left: side = ecm::side_front; break;
    		case ecm::side_front: side = ecm::side_right; break;
    		case ecm::side_right: side = ecm::side_back; break;
    		case ecm::side_back: side = ecm::side_left; break;
    		default: break;
    	}
        sqxy.x = d;
        sqxy.y = from_sqxy.y;
    }
}

static void goBottom( const ecm::side_t from_side, const omath::dvec2 &from_sqxy, const double d,
		ecm::side_t &side, omath::dvec2& sqxy) {
    if (from_side == ecm::side_top) {
        side = ecm::side_front;
        sqxy.x = from_sqxy.x;
        sqxy.y = 1.0 - d;
    } else if (from_side == ecm::side_bottom) {
        side = ecm::side_back;
        sqxy.x = 1.0 - from_sqxy.x;
        sqxy.y = d;
    } else if (from_side == ecm::side_front) {
        side = ecm::side_bottom;
        sqxy.x = from_sqxy.x;
        sqxy.y = 1.0 - d;
    } else if (from_side == ecm::side_right) {
        side = ecm::side_bottom;
        sqxy.x = 1.0 - d;
        sqxy.y = 1.0 - from_sqxy.x;
    } else if (from_side == ecm::side_back) {
        side = ecm::side_bottom;
        sqxy.x = 1.0 - from_sqxy.x;
        sqxy.y = d;
    } else {
    	if( from_side != ecm::side_left )
    		throw std::runtime_error( "ECM: Error going up.\n" );
        side = ecm::side_bottom;
        sqxy.x = d;
        sqxy.y = from_sqxy.x;
    }
}

static void goTop( const ecm::side_t from_side, const omath::dvec2 from_sqxy, const double d,
		ecm::side_t& side, omath::dvec2& sqxy ) {
    if (from_side == ecm::side_top) {
        side = ecm::side_back;
        sqxy.x = 1.0 - from_sqxy.x;
        sqxy.y = 1.0 - d;
    } else if (from_side == ecm::side_bottom) {
        side = ecm::side_front;
        sqxy.x = from_sqxy.x;
        sqxy.y = d;
    } else if (from_side == ecm::side_front) {
        side = ecm::side_top;
        sqxy.x = from_sqxy.x;
        sqxy.y = d;
    } else if (from_side == ecm::side_right) {
        side = ecm::side_top;
        sqxy.x = 1.0 - d;
        sqxy.y = from_sqxy.x;
    } else if (from_side == ecm::side_back) {
        side = ecm::side_top;
        sqxy.x = 1.0 - from_sqxy.x;
        sqxy.y = 1.0 - d;
    } else {
    	if( from_side != ecm::side_left )
    		throw std::runtime_error( "ECM: Error going up.\n" );
        side = ecm::side_top;
        sqxy.x = d;
        sqxy.y = 1.0 - from_sqxy.x;
    }
}

void ecm::quadBaseData( const side_t quad_side, const int quad_level, const int quad_x, const int quad_y,
        const omath::dvec3 &quad_tl_cart, const omath::dvec3 &quad_tr_cart, const omath::dvec3 &quad_bl_cart,
		const omath::dvec3 &quad_br_cart, const omath::dvec3 &quad_plane_normal, const double quad_plane_distance,
        const side_t quad_size, const int quad_overlap,
        float* offsets, float* normals, double* max_quad_plane_distance) const {
    const int quads_in_level = (1 << quad_level);
    if( quad_level <= 0 )
    	throw std::runtime_error( "ECM: Error getting quad base data.\n" );
    // Force use of symmetry quads
    if( quad_side != ecm::side_top || quad_side != ecm::side_front )
    	throw std::runtime_error( "ECM: Error getting quad base data.\n" );
    if( quad_x >= quads_in_level / 2 || quad_y >= quads_in_level / 2 )
    	throw std::runtime_error( "ECM: Error getting quad base data.\n" );

    *max_quad_plane_distance = -1.0;
    for( unsigned int y = 0; y < quad_size + 2 * quad_overlap; y++) {
        for( unsigned int x = 0; x < quad_size + 2 * quad_overlap; x++) {
            // Quad-relative coordinates.
        	omath::dvec2 qxy{ x - quad_overlap + 0.5, y - quad_overlap + 0.5 };
        	qxy /= static_cast<double>(quad_size);
            // Interpolated position (computed in the same way as in the vertex shader,
            // but in planetocentric coordinates with full precision).
            omath::dvec3 p_interp{ quad_bl_cart * (1.0 - qxy.x) * (1.0 - qxy.y) +
            					   quad_br_cart * (      qxy.x) * (1.0 - qxy.y) +
								   quad_tr_cart * (      qxy.x) * (      qxy.y) +
								   quad_tl_cart * (1.0 - qxy.x) * (      qxy.y) };
            // Side coordinates. These need to be adjusted for samples in the border area
            // since these belong to neighboring sides. Note that for the diagonal
            // directions (e.g. x==0,y==0 for bottom-left), we chose only one way to go
            // into the neighbor since the result of diagonal movement is ambiguous.
            side_t side = quad_side;
            omath::dvec2 sqxy{ (quad_x + qxy.x), ((quads_in_level - 1 - quad_y) + qxy.y) };
            sqxy /= quads_in_level;
            if( sqxy.x < 0.0)
            	goLeft( side, omath::clamp( sqxy, 0.0, 1.0 ), -sqxy.x, side, sqxy );
            else if( sqxy.x > 1.0)
                goRight(side, omath::clamp(sqxy, 0.0, 1.0), sqxy.x - 1.0, side, sqxy);
            else if( sqxy.y < 0.0)
                goBottom(side, omath::clamp(sqxy, 0.0, 1.0), -sqxy.y, side, sqxy);
            else if( sqxy.y > 1.0)
                goTop(side, omath::clamp(sqxy, 0.0, 1.0), sqxy.y - 1.0, side, sqxy);

            if( ( sqxy.x < 0.0 && sqxy.x > 1.0 ) || ( sqxy.y < 0.0 && sqxy.y > 1.0 ) )
            	throw std::runtime_error( "ECM: Error getting quad base data.\n" );
            /*
            if (qxy.x >= 0 && qxy.x <= 1.0 && qxy.y >= 0.0 && qxy.y <= 1.0) {
                assert(sqxy.x >= 0.0 && sqxy.x <= 0.5);
                assert(sqxy.y >= 0.5 && sqxy.y <= 1.0);
                assert(sqxy.x >= quad[2] / static_cast<double>((1 << quad[1])));
                assert(sqxy.x <= (quad[2] + 1) / static_cast<double>((1 << quad[1])));
                assert(sqxy.y <= 1.0 - quad[2] / static_cast<double>((1 << quad[1])));
                assert(sqxy.y >= (quad[2] + 1) / static_cast<double>((1 << quad[1])));
            }
            */
            omath::dvec2 sxy = 2.0 * sqxy - omath::dvec2(1.0, 1.0);
            // Cartesian coordinates in full precision.
            omath::dvec2 p_ecm;
            sideToEcm(side, sxy.x, sxy.y, p_ecm.x, p_ecm.y);
            omath::dvec3 p_cart;
            ecmToCartesian(p_ecm.x, p_ecm.y, p_cart);
            //if (x >= quad_overlap && y >= quad_overlap && x < quad_size + quad_overlap && y < quad_size + quad_overlap)
            //		assert(length(p_cart) >= length(p_interp));
            // The offset between interpolated and full precision coordinates.
            omath::dvec3 offset = p_cart - p_interp;
            // The ellipsoid normal.
            double geod_lat, lon;
            ecmToGeodetic(p_ecm.x, p_ecm.y, geod_lat, lon);
            omath::dvec3 normal;
            geodeticNormal(geod_lat, lon, normal);
            // The quad plane distance.
            double qpd = omath::dot(quad_plane_normal, p_cart) - quad_plane_distance;
            if( x >= quad_overlap && y >= quad_overlap && x < quad_size + quad_overlap && y < quad_size + quad_overlap )
            	if( qpd < 0.0 )
            		throw std::runtime_error( "ECM: Error getting quad base data.\n" );
            // Store the results.
            offsets[3 * (y * (quad_size + 2 * quad_overlap) + x) + 0] = offset.x;
            offsets[3 * (y * (quad_size + 2 * quad_overlap) + x) + 1] = offset.y;
            offsets[3 * (y * (quad_size + 2 * quad_overlap) + x) + 2] = offset.z;
            normals[2 * (y * (quad_size + 2 * quad_overlap) + x) + 0] = normal.x;
            normals[2 * (y * (quad_size + 2 * quad_overlap) + x) + 1] = normal.y;
            if( qpd > *max_quad_plane_distance )
                *max_quad_plane_distance = qpd;
        }
    }
}
