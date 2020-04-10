
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky

#include <cstdio>
#include <cstring>
#include <random>
#include "omath/vec3.h"
#include <cmath>
#include <chrono>
#include <fstream>
#include <iostream>

#define WIDTH 512
#define HEIGHT 512
//#define DOME 1

const float kInfinity = std::numeric_limits<float>::max();

class Atmosphere {
public:
	Atmosphere(
			omath::vec3 sd = omath::vec3(0, 1, 0),
			float er = 6360e3f, float ar = 6420e3f,
			float hr = 7994.0f, float hm = 1200.0f ) :
        sunDirection(sd), earthRadius(er), atmosphereRadius(ar), Hr(hr), Hm(hm) {}

    omath::vec3 computeIncidentLight(
    		const omath::vec3& orig, const omath::vec3& dir, float tmin, float tmax ) const;

    // The sun direction (normalized)
    omath::vec3 sunDirection;
    // In the paper this is usually Rg or Re (radius ground, eart)
    float earthRadius;
    // In the paper this is usually R or Ra (radius atmosphere)
    float atmosphereRadius;
    // Thickness of the atmosphere if density was uniform (Hr)
    float Hr;
    // Same as above but for Mie scattering (Hm)
    float Hm;

    // Raleigh and Mie coefficients
    static const omath::vec3 betaR;
    static const omath::vec3 betaM;

};

const omath::vec3 Atmosphere::betaR( 3.8e-6f, 13.5e-6f, 33.1e-6f );
const omath::vec3 Atmosphere::betaM( 10e-6f ); // orig: 21e-6f

bool solveQuadratic( float a, float b, float c, float& x1, float& x2 ) {
	if (b == 0) {
		// Handle special case where the the two vector ray.dir and V are perpendicular
		// with V = ray.orig - sphere.centre
        if (a == 0)
        	return false;
        x1 = 0;
        x2 = sqrtf(-c / a);
        return true;
    }
	float discr = b * b - 4 * a * c;
	if (discr < 0)
		return false;
	float q = (b < 0.f) ? -0.5f * (b - sqrtf(discr)) : -0.5f * (b + sqrtf(discr));
    x1 = q / a;
    x2 = c / q;
    return true;
}

// simple intersection routine
bool raySphereIntersect( const omath::vec3& orig, const omath::vec3& dir, const float& radius, float& t0, float& t1 ) {
    // They ray dir is normalized so A = 1
    float A = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    float B = 2 * (dir.x * orig.x + dir.y * orig.y + dir.z * orig.z);
    float C = orig.x * orig.x + orig.y * orig.y + orig.z * orig.z - radius * radius;
    if (!solveQuadratic(A, B, C, t0, t1))
    	return false;
    if (t0 > t1)
    	std::swap(t0, t1);
    return true;
}

// Color of the sky for a camera ray. Raymarch along primary ray from camera to atmosphere exit
// or planet intersection. For each sample cast a light ray and march along
omath::vec3 Atmosphere::computeIncidentLight( const omath::vec3& orig, const omath::vec3& dir, float tmin, float tmax ) const {
	float t0, t1;
	if (!raySphereIntersect(orig, dir, atmosphereRadius, t0, t1) || t1 < 0)
		return 0;
    if (t0 > tmin && t0 > 0)
    	tmin = t0;
    if (t1 < tmax)
    	tmax = t1;
    uint32_t numSamples = 16;
    uint32_t numSamplesLight = 8;
    float segmentLength = (tmax - tmin) / (float)numSamples;
    float tCurrent = tmin;
    omath::vec3 sumR(0), sumM(0); // mie and rayleigh contribution
    float opticalDepthR = 0, opticalDepthM = 0;
    float mu = dot(dir, sunDirection); // mu in the paper which is the cosine of the angle between the sun direction and the ray direction
    float phaseR = 3.f / (16.f * (float)M_PI) * (1 + mu * mu);
    float g = 0.76f;
    float phaseM = 3.f / (8.f * (float)M_PI) * ((1.f - g * g) * (1.f + mu * mu)) /
    		((2.f + g * g) * powf(1.f + g * g - 2.f * g * mu, 1.5f));
    for (uint32_t i = 0; i < numSamples; ++i) {
        omath::vec3 samplePosition = orig + (tCurrent + segmentLength * 0.5f) * dir;
        float height = omath::magnitude( samplePosition ) - earthRadius;
        // compute optical depth for light
        float hr = expf(-height / Hr) * segmentLength;
        float hm = expf(-height / Hm) * segmentLength;
        opticalDepthR += hr;
        opticalDepthM += hm;
        // light optical depth
        float t0Light, t1Light;
        raySphereIntersect(samplePosition, sunDirection, atmosphereRadius, t0Light, t1Light);
        float segmentLengthLight = t1Light / (float)numSamplesLight, tCurrentLight = 0;
        float opticalDepthLightR = 0, opticalDepthLightM = 0;
        uint32_t j;
        for (j = 0; j < numSamplesLight; ++j) {
            omath::vec3 samplePositionLight = samplePosition + (tCurrentLight + segmentLengthLight * 0.5f) * sunDirection;
            float heightLight = omath::magnitude( samplePositionLight ) - earthRadius;
            if (heightLight < 0)
            	break;
            opticalDepthLightR += expf(-heightLight / Hr) * segmentLengthLight;
            opticalDepthLightM += expf(-heightLight / Hm) * segmentLengthLight;
            tCurrentLight += segmentLengthLight;
        }
        if (j == numSamplesLight) {
            omath::vec3 tau = betaR * (opticalDepthR + opticalDepthLightR) + betaM * 1.1f * (opticalDepthM + opticalDepthLightM);
            omath::vec3 attenuation(expf(-tau.x), expf(-tau.y), expf(-tau.z));
            sumR += attenuation * hr;
            sumM += attenuation * hm;
        }
        tCurrent += segmentLength;
    }
    // magic number 20
    return omath::vec3( sumR * betaR * phaseR + sumM * betaM * phaseM ) * 20.0f;
}

void renderSkydome( const omath::vec3& sunDir, const char *filename ) {
	Atmosphere atmosphere(sunDir);
	auto t0 = std::chrono::high_resolution_clock::now();
#if DOME
	// Render fisheye
	const unsigned width = 512, height = 512;
	omath::vec3 *image = new omath::vec3[width * height], *p = image;
	// @todo memset( image, 0x0, sizeof( omath::vec3 ) * width * height );
	for (unsigned j = 0; j < height; ++j) {
		float y = 2.f * ((float)j + 0.5f) / float(height - 1) - 1.f;
		for (unsigned i = 0; i < width; ++i, ++p) {
			float x = 2.f * ((float)i + 0.5f) / float(width - 1) - 1.f;
			float z2 = x * x + y * y;
			if (z2 <= 1) {
				float phi = std::atan2(y, x);
				float theta = std::acos(1 - z2);
				omath::vec3 dir(sinf(theta) * cosf(phi), cosf(theta), sinf(theta) * sinf(phi));
				// 1 meter above sea level
				*p = atmosphere.computeIncidentLight(omath::vec3(0, atmosphere.earthRadius + 1, 0), dir, 0, kInfinity);
			}
		}
	}
#else
	// Render normal camera
	const unsigned width = 640, height = 480;
	omath::vec3 *image = new omath::vec3[width * height], *p = image;
	// @todo memset( mage, 0x0, sizeof(omath::vec3) * width * height );
	float aspectRatio = width / float(height);
	float fov = 65;
	float angle = tanf(fov * (float)M_PI / 180.0f * 0.5f);
	unsigned numPixelSamples = 4;
	omath::vec3 orig( 0.0f, atmosphere.earthRadius + 1000.0f, 30000.0f ); // camera position
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 1); // to generate random floats in the range [0:1]
	for (unsigned y = 0; y < height; ++y) {
		for (unsigned x = 0; x < width; ++x, ++p) {
			for (unsigned m = 0; m < numPixelSamples; ++m) {
				for (unsigned n = 0; n < numPixelSamples; ++n) {
					float rayx = (2 * (x + (m + distribution(generator)) / (float)numPixelSamples) /
							float(width) - 1) * aspectRatio * angle;
					float rayy = (1 - (y + (n + distribution(generator)) / (float)numPixelSamples) /
							float(height) * 2) * angle;
					omath::vec3 dir(rayx, rayy, -1);
					dir = omath::normalize(dir);
					// Does the ray intersect the planetory body? If the ray intersects the Earth body and that
					// the intersection is ahead of us, then the ray intersects the planet in 2 points, t0 and t1.
					// But we only want to comupute the atmosphere between t=0 and t=t0 (where the ray hits the Earth first).
					// If the viewing ray doesn't hit the Earth, or course the ray is then bounded to the range [0:INF].
					// In the method computeIncidentLight() we then compute where this primary ray intersects the atmosphere
					// and we limit the max t range  of the ray to the point where it leaves the atmosphere.
					float t0, t1, tMax = kInfinity;
					if (raySphereIntersect(orig, dir, atmosphere.earthRadius, t0, t1) && t1 > 0)
						tMax = std::max(0.f, t0);
					//The *viewing or camera ray* is bounded to the range [0:tMax]
					*p += atmosphere.computeIncidentLight(orig, dir, 0, tMax);
				}
			}
			*p *= 1.f / (float)( numPixelSamples * numPixelSamples );
		}
	}
#endif
	std::cout << ((std::chrono::duration<float>)(std::chrono::high_resolution_clock::now() - t0)).count() << " seconds" << std::endl;
	// Save result to a PPM image (keep these flags if you compile under Windows)
	std::ofstream ofs(filename, std::ios::out | std::ios::binary);
	ofs << "P6\n" << width << " " << height << "\n255\n";
	p = image;
	for (unsigned j = 0; j < height; ++j) {
		for (unsigned i = 0; i < width; ++i, ++p) {
#if DOME
			// Apply tone mapping function
			(*p)[0] = (*p)[0] < 1.413f ? powf((*p)[0] * 0.38317f, 1.0f / 2.2f) : 1.0f - expf(-(*p)[0]);
			(*p)[1] = (*p)[1] < 1.413f ? powf((*p)[1] * 0.38317f, 1.0f / 2.2f) : 1.0f - expf(-(*p)[1]);
			(*p)[2] = (*p)[2] < 1.413f ? powf((*p)[2] * 0.38317f, 1.0f / 2.2f) : 1.0f - expf(-(*p)[2]);
#endif
			ofs << (unsigned char)(std::min(1.f, (*p)[0]) * 255) <<
					(unsigned char)(std::min(1.f, (*p)[1]) * 255) <<
					(unsigned char)(std::min(1.f, (*p)[2]) * 255);
		}
	}
	ofs.close();
	delete[] image;
}

int main() {
	puts( "Startup ..." );
	//volume_ramp( 0.5f );
#if DOME
	// Render a sequence of images (sunrise to sunset)
	unsigned nangles = 32;
	for (unsigned i = 0; i < nangles; ++i) {
		char filename[1024];
		sprintf(filename, "./skydome.%04d.ppm", i);
		float angle = (float)i / float(nangles - 1) * (float)M_PI * 0.6f;
		printf( "Rendering image %d, angle = %0.2f\n", i, angle * 180 / M_PI );
		renderSkydome( omath::vec3( 0.0f, cosf(angle), -sinf(angle) ), filename );
	}
	// Render one single image
#else
	float angle = (float)M_PI * 0.45f;
	omath::vec3 sunDir( 0.0f, cosf(angle), -sinf(angle) );
	std::cout << "Sun direction: " << sunDir << std::endl;
	renderSkydome( sunDir, "./skydome.ppm" );
#endif

	puts( "... ending" );
	return EXIT_SUCCESS;
}

/*
// Demonstrates absorption coefficient
void volume_ramp( const float &sigma_a ) {
	float volume_size{ 20.0f };
	omath::vec3 *buffer = new omath::vec3[WIDTH * HEIGHT];
	omath::vec3 *transmittance = buffer;
	const float w = (float)(WIDTH-1);
	// Do only one row
	for( unsigned int x = 0; x < WIDTH; ++x ) {
		float l{ (float)x / w * volume_size };
		*(transmittance++) = expf( -sigma_a * l );
	}
	// Copy rows
	for( unsigned int y = 0; y < HEIGHT; ++y )
		memcpy( buffer + y * WIDTH, buffer, sizeof(omath::vec3) * WIDTH );
	save_to_ppm( buffer, "ramp.ppm" );
	delete [] buffer;
}

static void save_to_ppm( const omath::vec3* const colors, const char* filename ) {
	FILE* f = fopen( filename, "w" );
	fprintf( f, "P3\n %d %d\n255\n", WIDTH, HEIGHT );
	for( unsigned int i = 0; i < HEIGHT; ++i )
		for( unsigned int j = 0; j < WIDTH; ++j ) {
			const omath::vec3 c{ colors[i * WIDTH + j] * 255.0f };
			fprintf( f, "%d %d %d\n", (uint8_t)c.x, (uint8_t)c.y, (uint8_t)c.z );
		}
	fclose( f );
}
*/
