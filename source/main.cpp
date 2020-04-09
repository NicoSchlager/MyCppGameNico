#include <SFML/Graphics.hpp>
#include <windows.h>
#include <ctime>
#include <cstdlib>
#include <unordered_map>
#include "ParticleSystem.h"
#include <Windows.h>
#include <mmsystem.h>
#include<iostream>
#include <mciapi.h>

#pragma comment(lib, "Winmm.lib")
#if defined( NDEBUG )

#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-system.lib")


#else // defined( NDEBUG )

#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-system-d.lib")

#endif // defined( NDEBUG )

#define UNUSED(x) (void)(x)

#define PI_VALUE (3.142f)

typedef sf::Vector2f Float2;
typedef sf::Vector3f Float3;
typedef LONGLONG _Longlong;

//////////////////////////////////////////////////////////////////////////
// Utility functions

void DebugLog(const char* msg, ...)
{
	char temp[4096];

	va_list ap;
	va_start(ap, msg);
	vsnprintf_s(temp, 4096 - 1, msg, ap);
	va_end(ap);

	OutputDebugStringA(temp);
}

//////////////////////////////////////////////////////////////////////////

float Clamp(float x, float lower, float upper)
{
	return fmin(upper, fmax(x, lower));
}

//////////////////////////////////////////////////////////////////////////

float LengthSq(Float2 v)
{
	return ((v.x * v.x) + (v.y * v.y));
}

//////////////////////////////////////////////////////////////////////////

float Length(Float2 v)
{
	return sqrt((v.x * v.x) + (v.y * v.y));
}

//////////////////////////////////////////////////////////////////////////

Float2 Normalized(Float2 v)
{
	float len = Length(v);
	return Float2(v.x / len, v.y / len);
}

//////////////////////////////////////////////////////////////////////////

float Dot(Float2 v1, Float2 v2)
{
	return (v1.x * v2.x) + (v1.y * v2.y);
}

//////////////////////////////////////////////////////////////////////////

Float2 Rotate(Float2 v, float angleRadians)
{
	return Float2(
		v.x * cos(angleRadians) - v.y * sin(angleRadians),
		v.x * sin(angleRadians) + v.y * cos(angleRadians)
		);
}

//////////////////////////////////////////////////////////////////////////

float RadiansToDegrees(float deg)
{
	return deg * (180.0f / PI_VALUE);
}

//////////////////////////////////////////////////////////////////////////

float DegreesToRadians(float deg)
{
	return deg * (PI_VALUE / 180.0f);
}

//////////////////////////////////////////////////////////////////////////

float ToAngle(Float2 v)
{
	return atan2f(v.y, v.x);
}

//////////////////////////////////////////////////////////////////////////

// Returns a random value between 0.0f - 1.0f
float FRand()
{
	return ((float)std::rand() / (float)RAND_MAX);
}

//Returns randomly a 1 or -1
float FRandSign()
{
	if (FRand() > 0.5f) {
		return 1;
	}
	else {
		return -1;
	}
}

//lerps linear between a and b, 0<=t<=1
float FLerp(float a, float b, float t) {
	return (1 - t) * a + t * b;
}

Float2 F2Lerp(Float2 a, Float2 b, float t) {
	return (1 - t) * a + t * b;
}

//////////////////////////////////////////////////////////////////////////

// Test intersection between a line and circle
bool TestLineToCircle(Float2 p1, Float2 p2, Float2 circleCenter, float circleRadius)
{
	Float2 d = p2 - p1;
	Float2 f = circleCenter - p1;

	// find the closest point between the line and the circle center
	Float2 du = Normalized(d);
	float proj = Dot(f, du);

	Float2 closest;

	if (proj < 0.0f)
	{
		closest = p1;
	}
	else if (proj > Length(d))
	{
		closest = p2;
	}
	else
	{
		Float2 projV = du * proj;
		closest = projV + p1;
	}

	Float2 closestDiff = circleCenter - closest;
	float closestLen = Length(closestDiff);

	if (closestLen > circleRadius)
	{
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Game resources

struct Resources
{
	sf::Texture mBackgroundTex;
	sf::Texture mPlayerTex;
	sf::Texture mEnemyTex;
	sf::Font	mMainFont;
	sf::Shader	mPostProShader;
	sf::Texture	mFlashlight;
};

//////////////////////////////////////////////////////////////////////////

bool LoadResources(Resources& r)
{
	bool success = true;

	success &= r.mBackgroundTex.loadFromFile("../assets/patchygrass_1.jpg");
	success &= r.mPlayerTex.loadFromFile("../assets/player.png");
	success &= r.mEnemyTex.loadFromFile("../assets/enemy.png");
	success &= r.mMainFont.loadFromFile("../assets/RussoOne-Regular.ttf");
	success &= r.mPostProShader.loadFromFile("../assets/shaders/postpro.vert", "../assets/shaders/postpro.frag");
	success &= r.mFlashlight.loadFromFile("../assets/flashlight.png");

	return success;
}

//////////////////////////////////////////////////////////////////////////
// Particle systems

//////////////////////////////////////////////////////////////////////////
// Blood

struct BloodParticle : public BaseParticle
{
	Float2		mVelocity = Float2(0.0f, 0.0f);

	sf::Time	mInitialLifetime;
};

struct BloodParticleSpawnParams
{
	Float2	mPos = Float2(0.0f, 0.0f);
	Float2	mVelocity = Float2(0.0f, 0.0f);

	float	mSpread = 0.0f;		// cone angle for emissions
};

ParticleSystem<BloodParticle, BloodParticleSpawnParams> gParticlesBlood(5000);

//////////////////////////////////////////////////////////////////////////

void InitBloodParticle(BloodParticle& particle, BloodParticleSpawnParams const& params)
{
	// give a random velocity and lifetime to the particle
	float angle = (std::rand() % 360) * 3.14f / 180.f;
	float speed = 0.0f + FRand() * 500.f;

	particle.mVelocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
	particle.mInitialLifetime = particle.mLifetime = sf::milliseconds((std::rand() % 500) + 20000);
	particle.mSize = FRand() * 20.0f;
	particle.mColor = sf::Color(128 + (std::rand() % 128), 0 + (std::rand() % 30), 0 + (std::rand() % 30), 255);

	// skew velocity in direction of shot
	Float2 dir = Normalized(params.mVelocity);
	dir = Rotate(dir, DegreesToRadians(params.mSpread * (FRand() - 0.5f)));

	particle.mVelocity = (dir * speed);
	particle.mPos = params.mPos;
}

//////////////////////////////////////////////////////////////////////////

void UpdateBloodParticle(BloodParticle& particle, sf::Time elapsed)
{
	particle.mLifetime -= elapsed;

	float a = std::max(particle.mLifetime / particle.mInitialLifetime, 0.0f);
	particle.mColor.a = static_cast<sf::Uint8>(a * 255);

	particle.mPos += particle.mVelocity * elapsed.asSeconds();

	sf::Time age = particle.mInitialLifetime - particle.mLifetime;

	if (age.asSeconds() > 0.2f)
	{
		particle.mVelocity = Float2(0.0f, 0.0f);
	}
}

//////////////////////////////////////////////////////////////////////////

void SpawnBloodParticles(Float2 pos, Float2 dir, uint32_t quantity, float spreadAngle)
{
	BloodParticleSpawnParams params = {};

	params.mPos = pos;
	params.mVelocity = dir;
	params.mSpread = spreadAngle;

	gParticlesBlood.Spawn(quantity, params, InitBloodParticle);
}

//////////////////////////////////////////////////////////////////////////

void UpdateAndRenderAllParticles(sf::RenderTexture* renderTex, float dt)
{
	sf::Time elapsed = sf::seconds(dt);
	gParticlesBlood.Update(elapsed);

	renderTex->draw(gParticlesBlood);
}

//////////////////////////////////////////////////////////////////////////
// GAMEPLAY code

struct Bullet
{
	Bullet() {};
	Float2	mPos = Float2(0.0f, 0.0f);
	Float2	mVelocity = Float2(0.0f, 0.0f);

	float	mLife = 0.0f;
};

struct Enemy
{
	Enemy() {};
	Float2 mPos = Float2(0.0f, 0.0f);
	Float2 mVelocity = Float2(0.0f, 0.0f);
	Float2 Knockback = Float2(0.0f, 0.0f);

	float mHealth = 0.0f;
	float mLastHit = 0.0f;
};

struct GameState
{
	Float2 mPlayerStartPos = Float2(0.0f, 0.0f);
	Float2 mPlayerPos = Float2(0.0f, 0.0f);
	Float2 mPlayerDir = Float2(0.0f, 0.0f);
	float playerCurSpeed = 0;
	Float2 mMouseWorldPos = Float2(0.0f, 0.0f);
	float mPlayerFireCooldown = 0.0f;
	int32_t mPlayerScore = 0;
	int32_t mPlayer1Total = 0;
	int32_t mPlayer2Total = 0;
	bool player1 = true;

	float mEnemySpawnCounter = 2.0f;
	float mobstacleSpawnCounter = 0.0f;
	float mEnemySpawnDifficulty = 0.1f;
	int32_t mEnemyCount = 0;


	float roundTimer = 0;
	int32_t roundNumber = 1;
	int32_t maxRounds = 2;
	int32_t obstacleCount = 4;

	std::vector<Float2> obstaclePos{ 30 }; //obstacle object pool
	std::unordered_map<Bullet*, bool> mBullets; //bullet object pool
	std::unordered_map<Enemy*, bool> mEnemies; //enemy object pool

	GameState() {

		/*mPlayerPos = Float2(0.0f, 0.0f);
		mPlayerDir = Float2(0.0f, 0.0f);
		mMouseWorldPos = Float2(0.0f, 0.0f);

		mPlayerFireCooldown = 0.0f;
		mEnemySpawnCounter = 0.0f;
		mEnemySpawnDifficulty = 0.1f;

		mEnemyCount = 0;

		mPlayerScore = 0;*/


		//EnemyCreation
		for (size_t i = 0; i < 30; i++) {
			Enemy* enemy = new Enemy();
			mEnemies.insert({ enemy, false });
		}

		//BulletCreation
		for (size_t i = 0; i < 20; i++) {
			Bullet* bullet = new Bullet();
			mBullets.insert({ bullet, false });
		}
		obstaclePos[0] = Float2(400.0f, 200.0f);
		obstaclePos[1] = Float2(-200.0f, 200.0f);
		obstaclePos[2] = Float2(100.0f, -200.0f);
		obstaclePos[3] = Float2(-350, -150.0f);
		obstacleCount = 4;
		//ObstacleCreation
		for (size_t i = 4; i < obstaclePos.size(); i++) {
			obstaclePos[i].x = 10000;
			obstaclePos[i].y = 10000;
		}

		
	}

	void AddScore(int val) {
		mPlayerScore += val;
		/*if (player1) mPlayer1Total = mPlayerScore > mPlayer1Total ? mPlayerScore : mPlayer1Total;
		else mPlayer2Total = mPlayerScore > mPlayer2Total ? mPlayerScore : mPlayer2Total;*/
	}

	void Reset() {
		
		mPlayerPos = Float2(0.0f, 0.0f);
		mPlayerDir = Float2(0.0f, 0.0f);
		mMouseWorldPos = Float2(0.0f, 0.0f);

		mPlayerFireCooldown = 0.0f;
		mEnemySpawnCounter = 2.0f;
		mEnemySpawnDifficulty = 0.1f;

		if (!player1) { 
			mPlayer2Total += mPlayerScore;
			roundNumber++; 
		}
		else {
			mPlayer1Total += mPlayerScore;
		}

		roundTimer = 0;

		mEnemyCount = 0;
		player1 = !player1;
		mPlayerScore = 0;

		for (auto iter = mEnemies.begin(); iter != mEnemies.end(); iter++)
		{
			iter->first->mPos = { 10000, 10000 };
			iter->second = false;
		}

		obstaclePos[0] = Float2(400.0f, 200.0f);
		obstaclePos[1] = Float2(-200.0f, 200.0f);
		obstaclePos[2] = Float2(100.0f, -200.0f);
		obstaclePos[3] = Float2(-350, -150.0f);
		obstacleCount = 4;
		//ObstacleCreation
		for (size_t i = 4; i < obstaclePos.size(); i++) {
			obstaclePos[i].x = 10000;
			obstaclePos[i].y = 10000;
		}
	}

	void HardReset() {

		mPlayerPos = Float2(0.0f, 0.0f);
		mPlayerDir = Float2(0.0f, 0.0f);
		mMouseWorldPos = Float2(0.0f, 0.0f);
		mPlayerFireCooldown = 0.0f;
		mEnemySpawnCounter = 2.0f;
		mEnemySpawnDifficulty = 0.1f;

		mPlayer1Total = 0;
		mPlayer2Total = 0;
		player1 = true;
		roundTimer = 0;
		roundNumber = 1;



		if (!player1) { roundNumber++; }

		roundTimer = 0;

		mEnemyCount = 0;
		mPlayerScore = 0;

		for (auto iter = mEnemies.begin(); iter != mEnemies.end(); iter++)
		{
			iter->first->mPos = { 10000, 10000 };
			iter->second = false;
		}
		obstaclePos[0] = Float2(400.0f, 200.0f);
		obstaclePos[1] = Float2(-200.0f, 200.0f);
		obstaclePos[2] = Float2(100.0f, -200.0f);
		obstaclePos[3] = Float2(-350, -150.0f);
		obstacleCount = 4;
		//ObstacleCreation
		for (size_t i = 4; i < obstaclePos.size(); i++) {
			obstaclePos[i] = Float2(10000, 10000);
		}
	}
	~GameState() { 
		//EnemyDeletion
		for (auto iter = mEnemies.begin(); iter != mEnemies.end(); iter++) {
			delete iter->first;
		}
		//BulletDeletion
		for (auto iter = mBullets.begin(); iter != mBullets.end(); iter++) {
			delete iter->first;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// Gameplay parameters

static Float2 sWindowSize = Float2(1280.0f, 720.0f);

static const float sPlayerSpeed = 200.0f;
static const float sPlayerFireDelay = 0.10f;
static const float sPlayerBulletSpeed = 1000.0f;
static const float sPlayerBulletLife = 0.5f;
static const float sPlayerBulletSize = 4.0f;

static const float sEnemyTowardsPlayerForce = 10000.0f;
static const float sEnemyMinPlayerDistance = 80.0f;
static const float sEnemySpawnTime = 3.0f;
static const float sEnemyMaxVelocity = 100.0f;
static const float sEnemyKnockbackDecay = 0.8f;

//////////////////////////////////////////////////////////////////////////

Float2 EnemyMovementForce(GameState* gameState, Enemy* enemy)
{
	Float2 enemyToPlayer = gameState->mPlayerPos - enemy->mPos;
	Float2 force = Float2(0.0f, 0.0f);

	// head towards player
	if (Length(enemyToPlayer) > sEnemyMinPlayerDistance)
	{
		force += Normalized(enemyToPlayer) * sEnemyTowardsPlayerForce;
	}
	else
	{
		// don't get too close!
		if (Length(enemyToPlayer) > FLT_MIN)
		{
			force += -Normalized(enemyToPlayer) * powf(sEnemyMinPlayerDistance - Length(enemyToPlayer), 3.0f);
		}
		else
		{
			force += Float2(0.0f, 1.0f) * powf(sEnemyMinPlayerDistance, 3.0f);
		}
	}

	// keep away from other enemies
	for (auto iter = gameState->mEnemies.begin(); iter != gameState->mEnemies.end(); iter++)
	{
		// ignore self
		if (enemy != iter->first && iter->second == true)
		{
			Float2 enemyToEnemy = enemy->mPos - iter->first->mPos;

			if (Length(enemyToEnemy) < 100.0f)
			{
				force += Normalized(enemyToEnemy) * (100.0f - Length(enemyToEnemy)) * 1000.0f;
			}
		}
	}
	for (auto iter = gameState->obstaclePos.begin(); iter != gameState->obstaclePos.end(); iter++)
	{
		Float2 pos = { iter->x, iter->y };
		Float2 enemyToObj = enemy->mPos - pos;

		if (Length(enemyToObj) < 100.0f)
		{
			force += Normalized(enemyToObj) * (100.0f - Length(enemyToObj)) * 1000.0f;
		}

	}

	return force;
}

//////////////////////////////////////////////////////////////////////////

void HitEnemy(Enemy* e, Bullet* b)
{
	e->mHealth -= 10.0f;

	// knockback
	Float2 knockback = Normalized(b->mVelocity) * 100.0f;
	e->Knockback += knockback;

	// flash enemy
	e->mLastHit = 0.0f;
}

//////////////////////////////////////////////////////////////////////////

void UpdateAndRenderLevel(Resources* resources, sf::RenderTexture* renderTex)
{
	Float2 texSize = (Float2)resources->mBackgroundTex.getSize();

	sf::Sprite s(resources->mBackgroundTex);

	s.setOrigin(texSize / 2.0f);
	s.setColor(sf::Color(80, 80, 80));
	s.setPosition(sf::Vector2f(0.0f, 0.0f));

	sf::View view(sf::Vector2f(0.0f, 0.0f), sWindowSize);
	renderTex->setView(view);



	renderTex->draw(s);
}

//////////////////////////////////////////////////////////////////////////

void UpdateAndRenderPlayer(GameState* gameState, Resources* resources, sf::RenderTexture* renderTex, float dt)
{
	Float2 playerVelocity = Float2(0.0f, 0.0f);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		playerVelocity.y -= 1.0f;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		playerVelocity.y += 1.0f;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		playerVelocity.x += 1.0f;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		playerVelocity.x -= 1.0f;
	}

	// ensure the player doesn't go faster diagonally
	if (LengthSq(playerVelocity) > 1.0f)
	{
		playerVelocity = Normalized(playerVelocity);
	}
	for (auto iter = gameState->obstaclePos.begin(); iter != gameState->obstaclePos.end(); iter++)
	{
		Float2 pos = { iter->x, iter->y };
		Float2 playerToObs = gameState->mPlayerPos - pos;

		if (Length(playerToObs) < 60.0f)
		{
			gameState->Reset();
			break;
		}
	}
	// move the player
	gameState->mPlayerPos += playerVelocity * sPlayerSpeed * dt;

	// point the player at the mouse cursor
	Float2 playerToMouse = gameState->mMouseWorldPos - gameState->mPlayerPos;
	gameState->mPlayerDir = Normalized(playerToMouse);

	// draw the player
	sf::Sprite playerSprite(resources->mPlayerTex);

	playerSprite.setOrigin((Float2)resources->mPlayerTex.getSize() * 0.5f);
	playerSprite.setPosition(gameState->mPlayerPos.x, gameState->mPlayerPos.y);
	playerSprite.setRotation(RadiansToDegrees(ToAngle(gameState->mPlayerDir)));

	renderTex->draw(playerSprite);

	// player firing

	gameState->mPlayerFireCooldown -= dt;

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{

		if (gameState->mPlayerFireCooldown <= 0.0f)
		{
			for (auto iter = gameState->mBullets.begin(); iter != gameState->mBullets.end(); iter++) {
				if (!iter->second) {
					iter->second = true;
					gameState->mPlayerFireCooldown = sPlayerFireDelay;

					iter->first->mLife = sPlayerBulletLife;
					iter->first->mPos = gameState->mPlayerPos;
					iter->first->mVelocity = gameState->mPlayerDir * sPlayerBulletSpeed;
					break;
				}
			}

		}
	}
}

//////////////////////////////////////////////////////////////////////////

void UpdateAndRenderBullets(GameState* gameState, sf::RenderTexture* renderTex, float dt)
{
	// update and draw player bullets
	sf::CircleShape bulletShape(sPlayerBulletSize);
	bulletShape.setOrigin(sPlayerBulletSize / 2.0f, sPlayerBulletSize / 2.0f);

	for (auto it = gameState->mBullets.begin(); it != gameState->mBullets.end(); it++)
	{
		if (it->second) {
			Bullet* bullet = it->first;
			Float2 oldPos = bullet->mPos;
			Float2 newPos = oldPos + bullet->mVelocity * dt;

			bullet->mLife -= dt;

			bool expireBullet = false;

			if (bullet->mLife <= 0.0f)
			{
				expireBullet = true;
			}
			else
			{
				// only player shooting at the moment so just test against enemies
				for (auto iter = gameState->mEnemies.begin(); iter != gameState->mEnemies.end(); iter++)
				{
					// ignore inactive enemies
					if (iter->second == true) {

						Float2 enemyPos = iter->first->mPos;
						float enemyRadius = 20.0f;

						if (TestLineToCircle(oldPos, newPos, enemyPos, enemyRadius))
						{
							HitEnemy(iter->first, bullet);
							expireBullet = true;
							break;
						}
					}
				}
				for (auto iter = gameState->obstaclePos.begin(); iter != gameState->obstaclePos.end(); iter++)
				{
					Float2 pos = { iter->x, iter->y };
					Float2 bulletToObs = bullet->mPos - pos;

					if (Length(bulletToObs) < 40.0f)
					{
						expireBullet = true;
						break;
					}
				}

				bullet->mPos = newPos;

				bulletShape.setPosition(bullet->mPos.x, bullet->mPos.y);
				renderTex->draw(bulletShape);
			}

			if (expireBullet)
			{
				// bullet has expired so swap it with the end of the list and reduce the count by one
				it->second = false;
				it->first->mPos = { 10000,10000 };
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void UpdateAndRenderEnemies(GameState* gameState, Resources* resources, sf::RenderTexture* renderTex, float dt)
{
	for (auto iter = gameState->mEnemies.begin(); iter != gameState->mEnemies.end(); iter++)
	{
		if (iter->second) {
			Enemy* enemy = iter->first;
			if (enemy->mHealth <= 0.0f)
			{
				// splat away from player because we don't know where the damage came from here
				Float2 splatDir = enemy->mPos - gameState->mPlayerPos;
				SpawnBloodParticles(enemy->mPos, Normalized(splatDir), 100, 50.0f);

				iter->second = false;
				iter->first->mPos = { 10000, 10000 };

				gameState->AddScore(10);

				continue;
			}
			else
			{
				if (enemy->mLastHit == 0.0f)
				{
					Float2 splatDir = enemy->mPos - gameState->mPlayerPos;
					SpawnBloodParticles(enemy->mPos, Normalized(splatDir), std::rand() % 10, 20.0f);
				}

				// process movement

				enemy->mVelocity += EnemyMovementForce(gameState, enemy) * dt;

				if (LengthSq(enemy->mVelocity) > (sEnemyMaxVelocity * sEnemyMaxVelocity))
				{
					enemy->mVelocity = Normalized(enemy->mVelocity) * sEnemyMaxVelocity;
				}

				enemy->mVelocity += enemy->Knockback;
				enemy->Knockback *= sEnemyKnockbackDecay;

				if (TestLineToCircle(enemy->mPos, enemy->mPos + enemy->mVelocity * dt, gameState->mPlayerPos, 70.f))
				{
					gameState->Reset();
				}

				enemy->mPos += enemy->mVelocity * dt;



				// Render them

				sf::Sprite playerSprite(resources->mEnemyTex);

				playerSprite.setOrigin((Float2)resources->mEnemyTex.getSize() * 0.5f);
				playerSprite.setPosition(enemy->mPos.x, enemy->mPos.y);

				// flash for a 50ms when hit
				sf::Color color = (enemy->mLastHit < 0.05f) ? sf::Color::Red : sf::Color::Magenta;
				playerSprite.setColor(color);

				enemy->mLastHit += dt;

				renderTex->draw(playerSprite);
			}
		}
	}

	// spawn any enemies?
	gameState->mEnemySpawnCounter -= dt;


	if (gameState->mEnemySpawnCounter < 0.0f)
	{
		gameState->mEnemySpawnDifficulty += 0.1f; //linear Difficulty;
		gameState->mEnemySpawnDifficulty = Clamp(gameState->mEnemySpawnDifficulty, 0, sEnemySpawnTime - 1.0f);
		gameState->mEnemySpawnCounter = sEnemySpawnTime - gameState->mEnemySpawnDifficulty;

		Enemy* enemy;
		for (auto iter = gameState->mEnemies.begin(); iter != gameState->mEnemies.end(); iter++) {
			if (!iter->second) {
				enemy = iter->first;
				iter->second = true;
				enemy->mHealth = 100.0f;
				enemy->mPos = Float2(0.0f, 0.0f);
				// TODO: random offscreen pos

				Float2 s = (Float2)renderTex->getSize();
				Float2 sh = s / 2.0f;
				Float2 randomOffscreen{ 0,0 };

				do {
					randomOffscreen.x = Clamp(0 + (FRandSign() * s.x * FRand()), -sh.x * 1.1f, sh.x * 1.1f);
					randomOffscreen.y = Clamp(0 + (FRandSign() * s.y * FRand()), -sh.y * 1.1f, sh.y * 1.1f);
				} while (randomOffscreen.x != sh.x * 1.1f && randomOffscreen.x != -sh.x * 1.1f && randomOffscreen.y != sh.y * 1.1f && randomOffscreen.y != -sh.y * 1.1f);

				enemy->mPos = randomOffscreen;

				enemy->mLastHit = 1.0f;
				break;
			}
		}

	}
}

//////////////////////////////////////////////////////////////////////////

void UpdateAndRenderPPFX(Resources* resources, sf::RenderTexture* renderTex, sf::RenderWindow* window)
{

	resources->mPostProShader.setParameter("texture", renderTex->getTexture());

	// Take RenderTexture as a texture and convert to Sprite to draw back to window with post pro shader applied.

	sf::Sprite fullScreenSprite = sf::Sprite(renderTex->getTexture());

	Float2 vpSize = window->getView().getSize();
	fullScreenSprite.setOrigin((float)vpSize.x / 2, (float)vpSize.y / 2);

	// Flip Vertically
	fullScreenSprite.setScale(sf::Vector2f(1.0f, -1.0f));

	window->draw(fullScreenSprite, &resources->mPostProShader);
}

//////////////////////////////////////////////////////////////////////////

void UpdateAndRenderUI(GameState* gameState, Resources* resources, sf::RenderWindow* window)
{
	Float2 vpSize = window->getView().getSize();

	// draw UI
	sf::Text text;
	text.setFont(resources->mMainFont);
	text.setStyle(sf::Text::Bold);

	text.setPosition(window->getView().getCenter().x - vpSize.x / 3, window->getView().getCenter().y - 100);
	text.setCharacterSize(100);
	text.setColor(sf::Color::Red);

	if (gameState->roundNumber > gameState->maxRounds) {
		if (gameState->mPlayer1Total > gameState->mPlayer2Total)
			text.setString("Player1 WON!!!! " + std::to_string((_Longlong)gameState->mPlayer1Total));
		else if (gameState->mPlayer1Total < gameState->mPlayer2Total)
			text.setString("Player2 WON!!!! " + std::to_string((_Longlong)gameState->mPlayer2Total));
		else if (gameState->mPlayer1Total == gameState->mPlayer2Total)
			text.setString("DRAW!!!! " + std::to_string((_Longlong)gameState->mPlayer1Total));
		if (gameState->roundTimer < 0.5f) {
			sf::Color fade{ sf::Color::Red.r ,sf::Color::Red.g,sf::Color::Red.b, (sf::Uint8)FLerp(0.0f,255.0f,gameState->roundTimer * 2.0f) };
			text.setColor(fade);
		}
		if (gameState->roundTimer > 1.0f) {
			sf::Color fade{ sf::Color::Red.r ,sf::Color::Red.g,sf::Color::Red.b, (sf::Uint8)FLerp(255.0f,0.0f,(gameState->roundTimer - 1.0f) * 2.0f) };
			text.setColor(fade);
		}
		window->draw(text);
	}
	else if (gameState->roundTimer < 1.5f) {
		if (gameState->roundTimer < 0.5f) {
			sf::Color fade{ sf::Color::Red.r ,sf::Color::Red.g,sf::Color::Red.b, (sf::Uint8)FLerp(0.0f,255.0f,gameState->roundTimer * 2.0f) };
			text.setColor(fade);
		}
		if (gameState->roundTimer > 1.0f) {
			sf::Color fade{ sf::Color::Red.r ,sf::Color::Red.g,sf::Color::Red.b, (sf::Uint8)FLerp(255.0f,0.0f,(gameState->roundTimer - 1.0f) * 2.0f) };
			text.setColor(fade);
		}

		if (gameState->player1)
			text.setString("Round " + std::to_string(gameState->roundNumber) + " || Player 1");
		else
			text.setString("Round " + std::to_string(gameState->roundNumber) + " || Player 2");
		window->draw(text);
	}
	





	text.setCharacterSize(32);

	text.setString("P1 Total Score: " + std::to_string((_Longlong)gameState->mPlayer1Total));
	text.setPosition(-vpSize.x / 2.0f, -vpSize.y / 2.0f - 10);
	window->draw(text);
	text.setString("P2 Total Score: " + std::to_string((_Longlong)gameState->mPlayer2Total));
	text.setPosition(vpSize.x / 2.0f - text.getLocalBounds().width, -vpSize.y / 2.0f - 10);
	window->draw(text);
	//text.setString("TOP LEFT");
	//text.setPosition(-vpSize.x/2.0f, -vpSize.y/2.0f - 10);
	//window->draw(text);
	//
	//text.setString("TOP RIGHT");
	//text.setPosition(vpSize.x/2.0f - text.getLocalBounds().width, -vpSize.y/2.0f - 10);
	//window->draw(text);
	//
	//text.setString("BOTTOM LEFT");
	//text.setPosition(-vpSize.x/2.0f, vpSize.y/2.0f - 32);
	//window->draw(text);
	//
	//text.setString("BOTTOM RIGHT");
	//text.setPosition(vpSize.x/2.0f - text.getLocalBounds().width, vpSize.y/2.0f - 32);
	//window->draw(text);
	text.setPosition(window->getView().getCenter().x, window->getView().getCenter().y - vpSize.y / 2.0f - 10);
	if (gameState->player1)
		text.setString("P1: " + std::to_string((_Longlong)gameState->mPlayerScore));
	else
		text.setString("P2: " + std::to_string((_Longlong)gameState->mPlayerScore));

	window->draw(text);
	

}

	

	

	

void UpdateAndRenderObstacles(GameState* gameState, Resources* resources, sf::RenderTexture* renderTex, float dt)
{
	Float2 texSize = (Float2)resources->mEnemyTex.getSize();
	gameState->mobstacleSpawnCounter += dt;
	if (gameState->mobstacleSpawnCounter > 3.0f) {

		gameState->mobstacleSpawnCounter = 0;
		Float2 s = (Float2)renderTex->getSize();
		Float2 sh = s / 2.0f;
		Float2 randomOnscreen{ 0,0 };
		do {
			randomOnscreen.x = 0 + (FRandSign() * sh.x * FRand());
			randomOnscreen.y = 0 + (FRandSign() * sh.y * FRand());
		} while (LengthSq(randomOnscreen - gameState->mPlayerPos) < 4000 && LengthSq(randomOnscreen - gameState->mPlayerStartPos) < 4000);
		//check if not at player or player start position

		gameState->obstaclePos[gameState->obstacleCount++] = randomOnscreen;
	}

	sf::Sprite s(resources->mEnemyTex);

	for (int i = 0; i < gameState->obstacleCount; i++) {
		s.setOrigin((Float2)resources->mEnemyTex.getSize() * 0.5f);
		s.setColor(sf::Color::Black);
		s.setPosition(gameState->obstaclePos[i]);
		renderTex->draw(s);
	}


}
void UpdateAndRenderFlashlight(GameState* gameState, Resources* resources, sf::RenderTexture* renderTex)
{

	sf::Sprite s(resources->mFlashlight);

	s.setOrigin((Float2)resources->mFlashlight.getSize() * 0.5f);
	//s.setColor(sf::Color::Black);
	s.setPosition(gameState->mPlayerPos);
	s.setRotation(RadiansToDegrees(ToAngle(gameState->mPlayerDir)));
	renderTex->draw(s);


}

//////////////////////////////////////////////////////////////////////////

void UpdateAndRender(GameState* gameState, Resources* resources, sf::RenderTexture* renderTex, sf::RenderWindow* window, float dt)
{

	dt = (dt > 0.01666f ? 0.01666f : dt);

	gameState->mMouseWorldPos = renderTex->mapPixelToCoords(sf::Mouse::getPosition(*window));

	UpdateAndRenderLevel(resources, renderTex);
	UpdateAndRenderAllParticles(renderTex, dt);
	UpdateAndRenderPlayer(gameState, resources, renderTex, dt);
	UpdateAndRenderBullets(gameState, renderTex, dt);
	UpdateAndRenderEnemies(gameState, resources, renderTex, dt);
	UpdateAndRenderObstacles(gameState, resources, renderTex, dt);
	UpdateAndRenderFlashlight(gameState, resources, renderTex);
	UpdateAndRenderPPFX(resources, renderTex, window);
	UpdateAndRenderUI(gameState, resources, window);

}

//////////////////////////////////////////////////////////////////////////

int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
	)
{
	UNUSED(hInstance);
	UNUSED(hPrevInstance);
	UNUSED(lpCmdLine);
	UNUSED(nCmdShow);

	sf::RenderWindow window(sf::VideoMode((uint32_t)sWindowSize.x, (uint32_t)sWindowSize.y), "Nico Schlager");
	window.setVerticalSyncEnabled(true);

	sf::View view(Float2(0.0f, 0.0f), sWindowSize);
	window.setView(view);

	// initialise gamesate to 0
	GameState gameState;

	Resources resources;

	if (!LoadResources(resources))
	{
		DebugLog("Failed to load resources!\n");

		return -1;
	}

	sf::Clock clock;
	srand(static_cast <unsigned> (time(0)));

	sf::RenderTexture renderTex;
	renderTex.create((uint32_t)sWindowSize.x, (uint32_t)sWindowSize.y);
	//bool played = false;
	while (window.isOpen())
	{
		sf::Event event;


		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			else if (event.type == sf::Event::Resized)
			{
				sWindowSize = Float2((float)event.size.width, (float)event.size.height);

				view.setSize(sWindowSize);
				window.setView(view);
			}
		}

		sf::Time frameTime = clock.getElapsedTime();
		clock.restart();

		window.clear();
		gameState.roundTimer += frameTime.asSeconds();
		UpdateAndRender(&gameState, &resources, &renderTex, &window, frameTime.asSeconds());


		if (gameState.roundNumber > gameState.maxRounds && gameState.roundTimer > 1.5f) {
			gameState.HardReset();
		}
		window.display();
	}

	return 0;
}