#include "lab.hpp"

// ******* Function Member Implementation *******

// ***** Shape function members *****
Shape::Shape() : mColour{0, 0, 0}
{}

void Shape::setColour(Colour const& col)
{
    mColour = col;
}

Colour Shape::getColour() const
{
    return mColour;
}

void Shape::setMaterial(std::shared_ptr<Material> const& material)
{
    mMaterial = material;
}

std::shared_ptr<Material> Shape::getMaterial() const
{
    return mMaterial;
}

// ***** Camera function members *****
Camera::Camera() :
	mEye{ 0.0f, 0.0f, 500.0f },
	mLookAt{ 0.0f },
	mUp{ 0.0f, 1.0f, 0.0f },
	mU{ 1.0f, 0.0f, 0.0f },
	mV{ 0.0f, 1.0f, 0.0f },
	mW{ 0.0f, 0.0f, 1.0f }
{}

void Camera::setEye(atlas::math::Point const& eye)
{
	mEye = eye;
}

void Camera::setLookAt(atlas::math::Point const& lookAt)
{
	mLookAt = lookAt;
}

void Camera::setUpVector(atlas::math::Vector const& up)
{
	mUp = up;
}

void Camera::computeUVW()
{
	mW = glm::normalize(mEye - mLookAt);
	mU = glm::normalize(glm::cross(mUp, mW));
	mV = glm::cross(mW, mU);

	if (areEqual(mEye.x, mLookAt.x) && areEqual(mEye.z, mLookAt.z) &&
		mEye.y > mLookAt.y)
	{
		mU = { 0.0f, 0.0f, 1.0f };
		mV = { 1.0f, 0.0f, 0.0f };
		mW = { 0.0f, 1.0f, 0.0f };
	}

	if (areEqual(mEye.x, mLookAt.x) && areEqual(mEye.z, mLookAt.z) &&
		mEye.y < mLookAt.y)
	{
		mU = { 1.0f, 0.0f, 0.0f };
		mV = { 0.0f, 0.0f, 1.0f };
		mW = { 0.0f, -1.0f, 0.0f };
	}
}

// ***** Sampler function members *****
Sampler::Sampler(int numSamples, int numSets) :
    mNumSamples{numSamples}, mNumSets{numSets}, mCount{0}, mJump{0}
{
    mSamples.reserve(mNumSets * mNumSamples);
    setupShuffledIndeces();
}

int Sampler::getNumSamples() const
{
    return mNumSamples;
}

void Sampler::setupShuffledIndeces()
{
    mShuffledIndeces.reserve(mNumSamples * mNumSets);
    std::vector<int> indices;

    std::random_device d;
    std::mt19937 generator(d());

    for (int j = 0; j < mNumSamples; ++j)
    {
        indices.push_back(j);
    }

    for (int p = 0; p < mNumSets; ++p)
    {
        std::shuffle(indices.begin(), indices.end(), generator);

        for (int j = 0; j < mNumSamples; ++j)
        {
            mShuffledIndeces.push_back(indices[j]);
        }
    }
}

atlas::math::Point Sampler::sampleUnitSquare()
{
    if (mCount % mNumSamples == 0)
    {
        atlas::math::Random<int> engine;
        mJump = (engine.getRandomMax() % mNumSets) * mNumSamples;
    }

    return mSamples[mJump + mShuffledIndeces[mJump + mCount++ % mNumSamples]];
}



// ***** BRDF function members *****

Lambertian::Lambertian() : kd{ 1 }, cd{ 1 }
{}

Lambertian::Lambertian(float k, float c) : kd{ k }, cd{c}
{}

void Lambertian::set_kd(float ka) {
	kd = ka;
}
void Lambertian::set_cd(Colour c) {
	cd = c;
}

Colour Lambertian::fn([[maybe_unused]] ShadeRec const& sr,
	[[maybe_unused]] atlas::math::Vector const& reflected,
	[[maybe_unused]] atlas::math::Vector const& incoming) const {
	return kd * cd / (float) M_PI;
}
Colour Lambertian::rho([[maybe_unused]] ShadeRec const& sr,
	[[maybe_unused]] atlas::math::Vector const& reflected) const {
	return kd * cd;
}



// ***** Material function members *****

Matte::Matte() :
	Material(),
	ambient_brdf(new Lambertian),
	diffuse_brdf(new Lambertian)
{}

void Matte::set_ka(const float ka) {
	ambient_brdf->set_kd(ka);
}

void Matte::set_kd(const float kd) {
	diffuse_brdf->set_kd(kd);
}

void Matte::set_cd(const Colour& c) {
	ambient_brdf->set_cd(c);
	diffuse_brdf->set_cd(c);
}

Colour Matte::shade(ShadeRec& sr) {
	atlas::math::Vector wo = -sr.ray.d;
	Colour L = ambient_brdf->rho(sr, wo) * sr.world->ambient->L(sr);
	size_t numLights = sr.world->lights.size();

	for (int j = 0; j < numLights; j++) {
		atlas::math::Vector wi = sr.world->lights[j]->getDirection(sr);
		float ndotwi = glm::dot(sr.normal, wi);

		if (ndotwi > 0.0f)
			L += diffuse_brdf->fn(sr, wo, wi) * sr.world->lights[j]->L(sr) * ndotwi;
	}
	return L;
}




// ***** Light function members *****
Light::Light() :
	mColour{ 1,1,1 },
	mRadiance{ 1 },
	mShadows{ false }
{}

Colour Light::L([[maybe_unused]] ShadeRec& sr)
{
    return Colour{0.0f};
}

void Light::scaleRadiance([[maybe_unused]] float b)
{}

void Light::setColour([[maybe_unused]] Colour const& c)
{}

// ***** Ambient function members *****

Ambient::Ambient() 
	: Light()
{
	scaleRadiance({ 1.0 });
	setColour({ 1,1,1 });
}

void Ambient::scaleRadiance(float b) {
	mRadiance = b;
}

void Ambient::setColour(Colour const& c) {
	mColour = c;
}

atlas::math::Vector Ambient::getDirection([[maybe_unused]] ShadeRec& sr) {
	return (atlas::math::Vector({ 0,0,0 }));
}

Colour Ambient::L([[maybe_unused]] ShadeRec& sr) {
	return mRadiance * mColour;
}

// ***** PointLight function members *****

PointLight::PointLight()
	: Light(), mLocation{0,0,0}
{
	scaleRadiance({ 1.0 });
	setColour({ 1,1,1 });
}

void PointLight::scaleRadiance(float b) {
	mRadiance = b;
}

void PointLight::setColour(Colour const& c) {
	mColour = c;
}

atlas::math::Vector PointLight::getDirection(ShadeRec& sr) {
	return glm::normalize(mLocation - sr.hit_point);
}

Colour PointLight::L([[maybe_unused]] ShadeRec& sr) {
	return mRadiance * mColour;
}

void PointLight::setLocation(atlas::math::Point location) {
	mLocation = location;
}

// ***** Plane function members *****

Plane::Plane(atlas::math::Point point, Normal normal) :
	mPoint{ point }, mNormal{ normal }
{}

bool Plane::hit(atlas::math::Ray<atlas::math::Vector> const& ray,
	ShadeRec& sr) const
{
	float t{ std::numeric_limits<float>::max() };
	bool intersect{ intersectRay(ray, t) };

	// update ShadeRec info about new closest hit
	if (intersect && t < sr.t)
	{
		sr.normal = mNormal;
		sr.ray = ray;
		sr.color = mColour;
		sr.t = t;
		sr.material = mMaterial;
	}

	return intersect;

}

bool Plane::intersectRay(atlas::math::Ray<atlas::math::Vector> const& ray,
	float& tMin) const {
	float denom{ glm::dot(mNormal, ray.d) };

	if (std::fabs(denom) > 0.0001f) {
		tMin = glm::dot(mPoint - ray.o, mNormal) / denom;
		if (tMin >= 0) return true;
	}

	return false;
}

// ***** Triangle function members *****

Triangle::Triangle(atlas::math::Point a, atlas::math::Point b, atlas::math::Point c) :
	mA{ a }, mB{ b }, mC{c}
{}

bool Triangle::hit(atlas::math::Ray<atlas::math::Vector> const& ray,
	ShadeRec& sr) const
{
	float t{ std::numeric_limits<float>::max() };
	bool intersect{ intersectRay(ray, t) };

	// update ShadeRec info about new closest hit
	if (intersect && t < sr.t)
	{
		sr.normal = glm::cross((mA - mB), (mA - mC));
		sr.ray = ray;
		sr.color = mColour;
		sr.t = t;
		sr.material = mMaterial;
	}

	return intersect;
}

bool Triangle::intersectRay(atlas::math::Ray<atlas::math::Vector> const& ray,
	float& tMin) const {
	atlas::math::Vector normal = glm::cross(mA - mB, mA - mC);
	float denom{ glm::dot(normal, ray.d) };

	if (std::fabs(denom) > 0.0001f) {
		tMin = glm::dot(mA - ray.o, normal) / denom;
		if (tMin >= 0) {
			atlas::math::Vector P = ray.o + tMin * ray.d;

			atlas::math::Vector e1 = mB - mA;
			atlas::math::Vector e2 = mC - mB;
			atlas::math::Vector e3 = mA - mC;

			atlas::math::Vector c1 = P - mA;
			atlas::math::Vector c2 = P - mB;
			atlas::math::Vector c3 = P - mC;

			if (glm::dot(normal, glm::cross(e1, c1)) > 0 &&
				glm::dot(normal, glm::cross(e2, c2)) > 0 &&
				glm::dot(normal, glm::cross(e3, c3)) > 0)
				return true;
		}
	}

	return false;
}

// ***** Sphere function members *****
Sphere::Sphere(atlas::math::Point center, float radius) :
    mCentre{center}, mRadius{radius}, mRadiusSqr{radius * radius}
{}

bool Sphere::hit(atlas::math::Ray<atlas::math::Vector> const& ray,
                 ShadeRec& sr) const
{
    atlas::math::Vector tmp = ray.o - mCentre;
    float t{std::numeric_limits<float>::max()};
    bool intersect{intersectRay(ray, t)};

    // update ShadeRec info about new closest hit
    if (intersect && t < sr.t)
    {
        sr.normal   = (tmp + t * ray.d) / mRadius;
        sr.ray      = ray;
        sr.color    = mColour;
        sr.t        = t;
        sr.material = mMaterial;
    }

    return intersect;
}

bool Sphere::intersectRay(atlas::math::Ray<atlas::math::Vector> const& ray,
                          float& tMin) const
{
    const auto tmp{ray.o - mCentre};
    const auto a{glm::dot(ray.d, ray.d)};
    const auto b{2.0f * glm::dot(ray.d, tmp)};
    const auto c{glm::dot(tmp, tmp) - mRadiusSqr};
    const auto disc{(b * b) - (4.0f * a * c)};

    if (atlas::core::geq(disc, 0.0f))
    {
        const float kEpsilon{0.01f};
        const float e{std::sqrt(disc)};
        const float denom{2.0f * a};

        // Look at the negative root first
        float t = (-b - e) / denom;
        if (atlas::core::geq(t, kEpsilon))
        {
            tMin = t;
            return true;
        }

        // Now the positive root
        t = (-b + e);
        if (atlas::core::geq(t, kEpsilon))
        {
            tMin = t;
            return true;
        }
    }

    return false;
}

// ***** Pinhole function members *****
Pinhole::Pinhole() : Camera{}, mDistance{ 750.0f }, mZoom{ 1.0f }
{}

void Pinhole::setDistance(float distance)
{
	mDistance = distance;
}

void Pinhole::setZoom(float zoom)
{
	mZoom = zoom;
}

atlas::math::Vector Pinhole::rayDirection(atlas::math::Point const& p) const
{
	const auto dir = p.x * mU + p.y * mV - (mDistance * mZoom) * mW;
	return glm::normalize(dir);
}

void Pinhole::renderScene(std::shared_ptr<World> world) const
{
	using atlas::math::Point;
	using atlas::math::Ray;
	using atlas::math::Vector;

	float max_r{ 1 };
	float max_g{ 1 };
	float max_b{ 1 };

	Point samplePoint{}, pixelPoint{};
	Ray<atlas::math::Vector> ray{};

	ray.o = mEye;
	float avg{ 1.0f / world->sampler->getNumSamples() };

	for (int r{ 0 }; r < world->height; ++r)
	{
		for (int c{ 0 }; c < world->width; ++c)
		{
			Colour pixelAverage{ 0, 0, 0 };

			for (int j = 0; j < world->sampler->getNumSamples(); ++j)
			{
				ShadeRec trace_data{};
				trace_data.world = world;
				trace_data.t = std::numeric_limits<float>::max();
				samplePoint = world->sampler->sampleUnitSquare();
				pixelPoint.x = c - 0.5f * world->width + samplePoint.x;
				pixelPoint.y = r - 0.5f * world->height + samplePoint.y;
				ray.d = rayDirection(pixelPoint);
				bool hit{};

				for (auto obj : world->scene)
				{
					hit |= obj->hit(ray, trace_data);
				}

				if (hit)
				{
					if (trace_data.material != NULL)
						pixelAverage += trace_data.material->shade(trace_data);
				}
			}

			float pix_r = pixelAverage.r * avg;
			float pix_g = pixelAverage.g * avg;
			float pix_b = pixelAverage.b * avg;

			if (pix_r > max_r)
				max_r = pix_r;
			if (pix_g > max_g)
				max_g = pix_g;
			if (pix_b > max_b)
				max_b = pix_b;

			world->image.push_back({ pix_r,
									pix_g,
									pix_b });
		}
	}

	int temp = 0;
	for (Colour col : world->image) {
		world->image[temp] = { col[0] / max_r, col[1] / max_g, col[2] / max_b };
		temp++;
	}
}

// ***** Regular function members *****
Regular::Regular(int numSamples, int numSets) : Sampler{numSamples, numSets}
{
    generateSamples();
}

void Regular::generateSamples()
{
    int n = static_cast<int>(glm::sqrt(static_cast<float>(mNumSamples)));

    for (int j = 0; j < mNumSets; ++j)
    {
        for (int p = 0; p < n; ++p)
        {
            for (int q = 0; q < n; ++q)
            {
                mSamples.push_back(
                    atlas::math::Point{(q + 0.5f) / n, (p + 0.5f) / n, 0.0f});
            }
        }
    }
}

// ***** Random function members *****
Random::Random(int numSamples, int numSets) : Sampler{numSamples, numSets}
{
    generateSamples();
}

void Random::generateSamples()
{
    atlas::math::Random<float> engine;
    for (int p = 0; p < mNumSets; ++p)
    {
        for (int q = 0; q < mNumSamples; ++q)
        {
            mSamples.push_back(atlas::math::Point{
                engine.getRandomOne(), engine.getRandomOne(), 0.0f});
        }
    }
}

// ***** Jitter function members *****

Jitter::Jitter(int numSamples, int numSets) : Sampler{ numSamples, numSets }
{
	generateSamples();
}

void Jitter::generateSamples()
{
	int n = static_cast<int>(glm::sqrt(static_cast<float>(mNumSamples)));

	for (int j = 0; j < mNumSets; ++j)
	{
		for (int p = 0; p < n; ++p)
		{
			for (int q = 0; q < n; ++q)
			{
				float rx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float ry = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				mSamples.push_back(
					atlas::math::Point{ (q + rx) / n, (p + ry) / n, 0.0f });
			}
		}
	}
}

// ******* Driver Code *******

int main()
{
    std::shared_ptr<World> world{std::make_shared<World>()};

    world->width      = 600;
    world->height     = 600;
    world->background = {0, 0, 0};
    world->sampler    = std::make_shared<Jitter>(4, 83);

	std::shared_ptr<Ambient> ambient{ std::make_shared<Ambient>() };
	ambient->scaleRadiance(1.5f);
	ambient->setColour({ 1,1,1 });
	world->ambient = ambient;

	std::shared_ptr<PointLight> pointlight{ std::make_shared<PointLight>() };
	pointlight->setLocation({ -300,150,150 });
	pointlight->scaleRadiance(1.5f);
	ambient->setColour({ 1,1,1 });

	world->lights.push_back(pointlight);

	std::shared_ptr<Matte> matte0{ std::make_shared<Matte>() };
	matte0->set_ka(25);
	matte0->set_kd(65);
	matte0->set_cd({1,0,0});

	std::shared_ptr<Matte> matte1{std::make_shared<Matte>() };
	matte1->set_ka(25);
	matte1->set_kd(65);
	matte1->set_cd({ 0,0,1 });

	std::shared_ptr<Matte> matte2{ std::make_shared<Matte>() };
	matte2->set_ka(25);
	matte2->set_kd(65);
	matte2->set_cd({ 0,1,0 });

	std::shared_ptr<Matte> matte3{ std::make_shared<Matte>() };
	matte3->set_ka(25);
	matte3->set_kd(65);
	matte3->set_cd({ 1,1,1 });

	std::shared_ptr<Matte> matte4{ std::make_shared<Matte>() };
	matte4->set_ka(25);
	matte4->set_kd(65);
	matte4->set_cd({ 0,0,0 });

    world->scene.push_back(
        std::make_shared<Sphere>(atlas::math::Point{0, 0, -600}, 128.0f));
    world->scene[0]->setColour({1, 0, 0});
	world->scene[0]->setMaterial(matte0);

    world->scene.push_back(
        std::make_shared<Sphere>(atlas::math::Point{128, 32, -700}, 64.0f));
    world->scene[1]->setColour({0, 0, 1});
	world->scene[1]->setMaterial(matte1);

    world->scene.push_back(
        std::make_shared<Sphere>(atlas::math::Point{-128, 32, -700}, 64.0f));
    world->scene[2]->setColour({0, 1, 0});
	world->scene[2]->setMaterial(matte2);
	
	world->scene.push_back(
		std::make_shared<Plane>(atlas::math::Point{ 0, 0, -800 }, atlas::math::Point{ 0, 2, 1 }));
	world->scene[3]->setColour({0.2, 0.2, 0.2});
	world->scene[3]->setMaterial(matte3);

	world->scene.push_back(
		std::make_shared<Plane>(atlas::math::Point{ 0, 0, -800 }, atlas::math::Point{ 0, -2, 1 }));
	world->scene[4]->setColour({ 0.1, 0.1, 0.1 });
	world->scene[4]->setMaterial(matte3);

	world->scene.push_back(
		std::make_shared<Triangle>(atlas::math::Point{ -50,0,-200 },
			atlas::math::Point{ 50,0,-200 },
			atlas::math::Point{ 0,50,-200 }));
	world->scene[5]->setColour({ 0, 0, 0});
	world->scene[5]->setMaterial(matte4);

	// set up camera
	Pinhole camera{};

	// change camera position here
	camera.setEye({ 0, 0, 1 });
	// 150.0f, 150.0f, 500.0f
	
	camera.computeUVW();

	camera.renderScene(world);

    saveToFile("raytrace.bmp", world->width, world->height, world->image);

    return 0;
}

void saveToFile(std::string const& filename,
                std::size_t width,
                std::size_t height,
                std::vector<Colour> const& image)
{
    std::vector<unsigned char> data(image.size() * 3);

    for (std::size_t i{0}, k{0}; i < image.size(); ++i, k += 3)
    {
        Colour pixel = image[i];
        data[k + 0]  = static_cast<unsigned char>(pixel.r * 255);
        data[k + 1]  = static_cast<unsigned char>(pixel.g * 255);
        data[k + 2]  = static_cast<unsigned char>(pixel.b * 255);
    }

    stbi_write_bmp(filename.c_str(),
                   static_cast<int>(width),
                   static_cast<int>(height),
                   3,
                   data.data());
}