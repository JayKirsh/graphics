#pragma once

#include <atlas/core/Float.hpp>
#include <atlas/math/Math.hpp>
#include <atlas/math/Random.hpp>
#include <atlas/math/Ray.hpp>

#include <fmt/printf.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include <limits>
#include <memory>
#include <vector>

#define M_PI 3.14159265358979323846;

using atlas::core::areEqual;

using Colour = atlas::math::Vector;
using Normal = atlas::math::Vector;

void saveToFile(std::string const& filename,
                std::size_t width,
                std::size_t height,
                std::vector<Colour> const& image);

// Declarations
class BRDF;
class Camera;
class Material;
class Light;
class Shape;
class Sampler;

struct World
{
	std::size_t width{ 0 }, height{ 0 };
	Colour background{0,0,0};
    std::shared_ptr<Sampler> sampler;
    std::vector<std::shared_ptr<Shape>> scene;
    std::vector<Colour> image;
    std::vector<std::shared_ptr<Light>> lights;
    std::shared_ptr<Light> ambient;
};

struct ShadeRec
{
    Colour color;
	atlas::math::Vector hit_point;
    float t;
    atlas::math::Normal normal;
    atlas::math::Ray<atlas::math::Vector> ray;
    std::shared_ptr<Material> material;
    std::shared_ptr<World> world;
};


// Abstract classes defining the interfaces for concrete entities

class Camera
{
public:
	Camera();

	virtual ~Camera() = default;

	virtual void renderScene(std::shared_ptr<World> world) const = 0;

	void setEye(atlas::math::Point const& eye);

	void setLookAt(atlas::math::Point const& lookAt);

	void setUpVector(atlas::math::Vector const& up);

	void computeUVW();

protected:
	atlas::math::Point mEye;
	atlas::math::Point mLookAt;
	atlas::math::Point mUp;
	atlas::math::Vector mU, mV, mW;
};

class Sampler
{
public:
    Sampler(int numSamples, int numSets);
    virtual ~Sampler() = default;

    int getNumSamples() const;

    void setupShuffledIndeces();

    virtual void generateSamples() = 0;

    atlas::math::Point sampleUnitSquare();

protected:
    std::vector<atlas::math::Point> mSamples;
    std::vector<int> mShuffledIndeces;

    int mNumSamples;
    int mNumSets;
    unsigned long mCount;
    int mJump;
};

class Shape
{
public:
    Shape();
    virtual ~Shape() = default;

    // if t computed is less than the t in sr, it and the color should be
    // updated in sr
    virtual bool hit(atlas::math::Ray<atlas::math::Vector> const& ray,
                     ShadeRec& sr) const = 0;

    void setColour(Colour const& col);

    Colour getColour() const;

    void setMaterial(std::shared_ptr<Material> const& material);

    std::shared_ptr<Material> getMaterial() const;

protected:
    virtual bool intersectRay(atlas::math::Ray<atlas::math::Vector> const& ray,
                              float& tMin) const = 0;

    Colour mColour;
    std::shared_ptr<Material> mMaterial;
};



// BRDFS



class BRDF
{
public:
    virtual ~BRDF() = default;

    virtual Colour fn(ShadeRec const& sr,
                      atlas::math::Vector const& reflected,
                      atlas::math::Vector const& incoming) const = 0;
    virtual Colour rho(ShadeRec const& sr,
                       atlas::math::Vector const& reflected) const = 0;
};


class Lambertian : public BRDF {
	public:
		Lambertian();
		Lambertian(float k, float c);
		virtual Colour fn(ShadeRec const& sr,
			atlas::math::Vector const& reflected,
			atlas::math::Vector const& incoming) const;
		virtual Colour rho(ShadeRec const& sr,
			atlas::math::Vector const& reflected) const;

		void set_kd(float ka);
		void set_cd(Colour c);

	private:
		float kd;
		Colour cd;
};




// MATERIALS



class Material
{
public:
    virtual ~Material() = default;

    virtual Colour shade(ShadeRec& sr) = 0;
};

class Matte : public Material {
	public:
		Matte();

		void set_ka(const float k);
		void set_kd(const float k);
		void set_cd(const Colour& c);
		virtual Colour shade(ShadeRec& sr);
	private:
		std::shared_ptr<Lambertian> ambient_brdf;
		std::shared_ptr<Lambertian> diffuse_brdf;
};

// LIGHTS


class Light
{
public:
	Light();
    virtual atlas::math::Vector getDirection(ShadeRec& sr) = 0;
    virtual Colour L(ShadeRec& sr);

    void scaleRadiance(float b);
    void setColour(Colour const& c);

protected:
    Colour mColour;
    float mRadiance;
	bool mShadows;
};

class Ambient : public Light
{
public:
	Ambient();
	atlas::math::Vector getDirection(ShadeRec& sr);
	Colour L(ShadeRec& sr);

	void scaleRadiance(float b);
	void setColour(Colour const& c);
};

class PointLight : public Light
{
public:
	PointLight();
	atlas::math::Vector getDirection(ShadeRec& sr);
	Colour L(ShadeRec& sr);

	void scaleRadiance(float b);
	void setColour(Colour const& c);
	void setLocation(atlas::math::Point location);

private:
	atlas::math::Point mLocation;
};


// CAMERAS


class Pinhole : public Camera
{
public:
	Pinhole();

	void setDistance(float distance);
	void setZoom(float zoom);

	atlas::math::Vector rayDirection(atlas::math::Point const& p) const;
	void renderScene(std::shared_ptr<World> world) const;

private:
	float mDistance;
	float mZoom;
};


// SHAPES



class Plane : public Shape
{
public:
	Plane(atlas::math::Point point, Normal normal);

	bool hit(atlas::math::Ray<atlas::math::Vector> const& ray, ShadeRec& trace_data) const;

protected:
	bool intersectRay(atlas::math::Ray<atlas::math::Vector> const& ray,
		float& tMin) const;

	atlas::math::Point mPoint;
	Normal mNormal;

};

class Triangle : public Shape
{
public:
	Triangle(atlas::math::Point a, atlas::math::Point b, atlas::math::Point c);

	bool hit(atlas::math::Ray<atlas::math::Vector> const& ray, ShadeRec& trace_data) const;

protected:
	bool intersectRay(atlas::math::Ray<atlas::math::Vector> const& ray,
		float& tMin) const;

	atlas::math::Point mA;
	atlas::math::Point mB;
	atlas::math::Point mC;
};

class Sphere : public Shape
{
public:
    Sphere(atlas::math::Point center, float radius);

    bool hit(atlas::math::Ray<atlas::math::Vector> const& ray,
             ShadeRec& sr) const;

private:
    bool intersectRay(atlas::math::Ray<atlas::math::Vector> const& ray,
                      float& tMin) const;

    atlas::math::Point mCentre;
    float mRadius;
    float mRadiusSqr;
};



// SAMPLES



class Regular : public Sampler
{
public:
    Regular(int numSamples, int numSets);

    void generateSamples();
};

class Random : public Sampler
{
public:
    Random(int numSamples, int numSets);

    void generateSamples();
};

class Jitter : public Sampler
{
public:
	Jitter(int numSamples, int numSets);

	void generateSamples();
};
