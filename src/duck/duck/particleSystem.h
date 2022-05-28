#pragma once
#include <DirectXMath.h>
#include <vector>
#include <random>
#include <d3d11.h>

namespace mini
{
	namespace gk2
	{
		struct ParticleVertex
		{
			DirectX::XMFLOAT3 Pos;
			float Age;
			float Angle;
			float Size;
			static const D3D11_INPUT_ELEMENT_DESC Layout[4];

			ParticleVertex() : Pos(0.0f, 0.0f, 0.0f), Age(0.0f), Angle(0.0f), Size(0.0f) { }
		};

		struct ParticleVelocities
		{
			DirectX::XMFLOAT3 Velocity;
			float AngularVelocity;

			ParticleVelocities() : Velocity(0.0f, 0.0f, 0.0f), AngularVelocity(0.0f) { }
		};

		struct Particle
		{
			ParticleVertex Vertex;
			ParticleVelocities Velocities;
		};

		class ParticleSystem
		{
		public:
			ParticleSystem() = default;

			ParticleSystem(ParticleSystem&& other) = default;

			ParticleSystem(DirectX::XMFLOAT3 emmiterPosition);

			ParticleSystem& operator=(ParticleSystem&& other) = default;

			std::vector<ParticleVertex> Update(float dt, DirectX::XMFLOAT4 cameraPosition);

			size_t particlesCount() const { return m_particles.size(); }
			static const int MAX_PARTICLES;		//maximal number of particles in the system

		private:
			static const DirectX::XMFLOAT3 EMITTER_DIR;	//mean direction of particles' velocity
			static const float TIME_TO_LIVE;	//time of particle's life in seconds
			static const float EMISSION_RATE;	//number of particles to be born per second
			static const float MAX_ANGLE;		//maximal angle declination from mean direction
			static const float MIN_VELOCITY;	//minimal value of particle's velocity
			static const float MAX_VELOCITY;	//maximal value of particle's velocity
			static const float PARTICLE_SIZE;	//initial size of a particle
			static const float PARTICLE_SCALE;	//size += size*scale*dtime
			static const float MIN_ANGLE_VEL;	//minimal rotation speed
			static const float MAX_ANGLE_VEL;	//maximal rotation speed

			DirectX::XMFLOAT3 m_emitterPos;
			float m_particlesToCreate;

			std::vector<Particle> m_particles;

			std::default_random_engine m_random;

			DirectX::XMFLOAT3 RandomVelocity();
			Particle RandomParticle();
			static void UpdateParticle(Particle& p, float dt);
			std::vector<ParticleVertex> GetParticleVerts(DirectX::XMFLOAT4 cameraPosition);
		};
	}
}