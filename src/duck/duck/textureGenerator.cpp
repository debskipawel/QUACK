#include "textureGenerator.h"

using namespace mini::gk2;

TextureGenerator::TextureGenerator(unsigned octaves, float persistance)
	: m_octaves(octaves), m_persistance(persistance)
{ }

float TextureGenerator::Interpolate(float a, float b, float t)
{
	return b * t + a * (1 - t);
}

float TextureGenerator::Noise1(int x, int y)
{
	auto n = static_cast<unsigned>(x + y * 73);
	n = (n << 13) ^ n;
	return (1.0f - ((n * (n * n * 37731 + 789223) + 1376312019) & 0x7fffffff) / 2147483647.0f);
}

float TextureGenerator::SmoothNoise1(int x, int y)
{
	auto corners = (Noise1(x - 1, y - 1) + Noise1(x + 1, y - 1) + Noise1(x - 1, y + 1) + Noise1(x + 1, y + 1)) / 16;
	auto sides = (Noise1(x - 1, y) + Noise1(x + 1, y) + Noise1(x, y - 1) + Noise1(x, y + 1)) / 8;
	auto center = Noise1(x, y) / 4;
	return corners + sides + center;
}

float TextureGenerator::InterpolatedNoise1(float x, float y)
{
	auto ix = static_cast<int>(x);
	auto iy = static_cast<int>(y);
	auto v1 = SmoothNoise1(ix, iy);
	auto v2 = SmoothNoise1(ix + 1, iy);
	auto v3 = SmoothNoise1(ix, iy + 1);
	auto v4 = SmoothNoise1(ix + 1, iy + 1);
	auto fx = x - ix;
	v1 = Interpolate(v1, v2, fx);
	v3 = Interpolate(v3, v4, fx);
	return Interpolate(v1, v3, y - iy);
}

float TextureGenerator::Noise2D(float x, float y) const
{
	auto sum = 0.f;
	auto amplitude = 1.f;
	auto frequency = 1.f;
	for (auto i = 0U; i < m_octaves; ++i, amplitude *= m_persistance, frequency *= 2)
		sum += InterpolatedNoise1(x * frequency, y * frequency) * amplitude;
	return sum;
}

float TextureGenerator::Wood(float x, float y) const
{
	auto g = Noise2D(x, y) * 30.f;
	return g - static_cast<int>(g);
}