#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdint>

struct BaseParticle
{
	sf::Vector2f	mPos;
	sf::Time		mLifetime;
	sf::Color		mColor;
	float			mSize;
};

template <typename Particle, typename SpawnParams>
class ParticleSystem : public sf::Drawable, public sf::Transformable
{
public:
	ParticleSystem(uint32_t count) :
		mParticles(count),
		mVertices(sf::Quads, count * 4)
	{

	}

	void SetEmitterPosition(sf::Vector2f position)
	{
		mEmitterPos = position;
	}

	void Update(sf::Time elapsed)
	{
		for (std::size_t i = 0; i < mNumActive;)
		{
			Particle& p = mParticles[i];

			// update the particle lifetime
			UpdateBloodParticle(p, elapsed);

			float halfSize = p.mSize / 2.0f;

			// update the position of the corresponding vertex
			mVertices[0 + i * 4].position = p.mPos + sf::Vector2f(-halfSize, -halfSize);
			mVertices[1 + i * 4].position = p.mPos + sf::Vector2f(halfSize, -halfSize);
			mVertices[2 + i * 4].position = p.mPos + sf::Vector2f(halfSize, halfSize);
			mVertices[3 + i * 4].position = p.mPos + sf::Vector2f(-halfSize, halfSize);

			// update the alpha (transparency) of the particle according to its lifetime
			mVertices[0 + i * 4].color = p.mColor;
			mVertices[1 + i * 4].color = p.mColor;
			mVertices[2 + i * 4].color = p.mColor;
			mVertices[3 + i * 4].color = p.mColor;

			// if the particle is dead, move to the end
			if (p.mLifetime <= sf::Time::Zero)
			{
				mNumActive--;
				std::swap(mParticles[i], mParticles[mNumActive]);
			}
			else
			{
				i++;
			}
		}

		// ensure we only draw the active particles
		mVertices.resize(mNumActive * 4);
	}

	uint32_t GetNumActive() const
	{
		return mNumActive;
	}

	typedef void(*SpawnInitFunction)(Particle& particle, SpawnParams const& params);

	void Spawn(uint32_t count, SpawnParams const& params, SpawnInitFunction initCallback)
	{
		count = std::min(mParticles.size() - mNumActive, count);
		mVertices.resize((mNumActive + count) * 4);

		for (uint32_t i = 0; i < count; ++i)
		{
			uint32_t index = i + mNumActive;

			if (initCallback)
			{
				initCallback(mParticles[index], params);
			}

			float halfSize = mParticles[index].mSize / 2.0f;

			// reset the position of the corresponding vertex
			mVertices[0 + index * 4].position = mEmitterPos + sf::Vector2f(-halfSize, -halfSize);
			mVertices[1 + index * 4].position = mEmitterPos + sf::Vector2f(halfSize, -halfSize);
			mVertices[2 + index * 4].position = mEmitterPos + sf::Vector2f(halfSize, halfSize);
			mVertices[3 + index * 4].position = mEmitterPos + sf::Vector2f(-halfSize, halfSize);
		}

		mNumActive += count;
	}

private:

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		// apply the transform
		states.transform *= getTransform();

		// our particles don't use a texture
		states.texture = NULL;

		// draw the vertex array
		target.draw(mVertices, states);
	}

private:

	std::vector<Particle>	mParticles;
	sf::VertexArray			mVertices;

	sf::Vector2f	mEmitterPos	= sf::Vector2f(0.0f, 0.0f);
	sf::Time		mLifetime	= sf::seconds(3);
	uint32_t		mNumActive	= 0;
};	



