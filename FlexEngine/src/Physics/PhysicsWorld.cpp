#include "stdafx.hpp"

#include "Physics/PhysicsWorld.hpp"

#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)

#include "Physics/PhysicsManager.hpp"
#include "GameContext.hpp"
#include "Cameras/BaseCamera.hpp"

namespace flex
{
	PhysicsWorld::PhysicsWorld()
	{
	}

	PhysicsWorld::~PhysicsWorld()
	{
	}

	void PhysicsWorld::Initialize(const GameContext& gameContext)
	{
		if (!m_World)
		{
			m_World = gameContext.physicsManager->CreateWorld();
			
			//m_World->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawAabb);
		}
	}

	void PhysicsWorld::Destroy()
	{
		if (m_World)
		{
			for (i32 i = m_World->getNumCollisionObjects() - 1; i >= 0; --i)
			{
				btCollisionObject* pObj = m_World->getCollisionObjectArray()[i];
				btRigidBody* pBody = btRigidBody::upcast(pObj);
				if (pBody && pBody->getMotionState())
				{
					delete pBody->getMotionState();
				}
				m_World->removeCollisionObject(pObj);
				delete pObj;
			}
			
			SafeDelete(m_World);
		}
	}

	void PhysicsWorld::Update(sec deltaSeconds)
	{
		if (m_World)
		{
			m_World->stepSimulation(deltaSeconds, MAX_SUBSTEPS);
		}
	}

	btDiscreteDynamicsWorld* PhysicsWorld::GetWorld()
	{
		return m_World;
	}

	bool PhysicsWorld::PickBody(const btVector3& rayFromWorld, const btVector3& rayToWorld)
	{
		btVector3 hitPos;
		float pickingDist;
		btRigidBody* pickedBody = nullptr;
		int savedState;
		btTypedConstraint* pickedConstraint = nullptr;
		Logger::LogInfo("click!");

		btCollisionWorld::ClosestRayResultCallback rayCallback(rayFromWorld, rayToWorld);
		m_World->rayTest(rayFromWorld, rayToWorld, rayCallback);
		if (rayCallback.hasHit())
		{
			Logger::LogInfo("Hit!");

			btVector3 pickPos = rayCallback.m_hitPointWorld;
			btRigidBody* body = (btRigidBody*)btRigidBody::upcast(rayCallback.m_collisionObject);
			if (body)
			{
				if (!(body->isStaticObject() || body->isKinematicObject()))
				{
					pickedBody = body;

					pickedBody->activate(true);
					pickedBody->clearForces();

					btVector3 localPivot = body->getCenterOfMassTransform().inverse() * pickPos;
					pickedBody->applyForce({ 0, 600, 0 }, localPivot);

					//savedState = pickedBody->getActivationState();
					//pickedBody->setActivationState(DISABLE_DEACTIVATION);

					//btPoint2PointConstraint* p2p = new btPoint2PointConstraint(*body, localPivot);
					//dynamicsWorld->addConstraint(p2p, true);
					//pickedConstraint = p2p;
					//btScalar mousePickClamping = 30.f;
					//p2p->m_setting.m_impulseClamp = mousePickClamping;
					//p2p->m_setting.m_tau = 0.001f;
				}
			}
			hitPos = pickPos;
			pickingDist = (pickPos - rayFromWorld).length();

			return true;
		}

		return false;
	}

	btVector3 PhysicsWorld::GetRayTo(const GameContext& gameContext, int x, int y)
	{
		btVector3 rayFrom = ToBtVec3(gameContext.camera->GetPosition());
		btVector3 rayForward = ToBtVec3(gameContext.camera->GetForward());
		float farPlane = gameContext.camera->GetZFar();
		rayForward *= farPlane;

		btVector3 vertical = ToBtVec3(gameContext.camera->GetUp());
		btVector3 horizontal = ToBtVec3(gameContext.camera->GetRight());

		float fov = gameContext.camera->GetFOV();
		float tanfov = tanf(0.5f * fov);

		horizontal *= 2.0f * farPlane * tanfov;
		vertical *= 2.0f * farPlane * tanfov;

		float frameBufferWidth = gameContext.window->GetFrameBufferSize().x;
		float frameBufferHeight = gameContext.window->GetFrameBufferSize().y;

		btScalar aspect = frameBufferWidth / frameBufferHeight;

		horizontal *= aspect;

		btVector3 rayToCenter = rayFrom + rayForward;
		btVector3 dHor = horizontal * 1.0f / frameBufferWidth;
		btVector3 dVert = vertical * 1.0f / frameBufferHeight;

		btVector3 rayTo = rayToCenter + 0.5f * horizontal + 0.5f * vertical;
		rayTo -= btScalar(x) * dHor;
		rayTo -= btScalar(y) * dVert;

		return rayTo;
	}
} // namespace flex