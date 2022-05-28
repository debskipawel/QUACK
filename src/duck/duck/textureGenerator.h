#pragma once
namespace mini
{
	namespace gk2
	{
		/***************** NEW *****************/
		//2D smooth noise function generator

		class TextureGenerator
		{
		public:
			TextureGenerator(unsigned octaves, float persistance);

			float Noise2D(float x, float y) const;
			float Wood(float x, float y) const;

		private:
			unsigned m_octaves;
			float m_persistance;

			static float Noise1(int x, int y);
			static float SmoothNoise1(int x, int y);
			static float InterpolatedNoise1(float x, float y);
			static float Interpolate(float a, float b, float t);
		};
	}
}