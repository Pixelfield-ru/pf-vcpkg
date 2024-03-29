//
// Created by ilya on 18.02.24.
//

#ifndef SUNGEARENGINE_PHYSICSWORLD3D_H
#define SUNGEARENGINE_PHYSICSWORLD3D_H

#include <thread>
#include <mutex>
#include <BulletCollision/CollisionDispatch/btCollisionConfiguration.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/BroadphaseCollision/btDispatcher.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletCollision/BroadphaseCollision/btBroadphaseInterface.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <LinearMath/btIDebugDraw.h>
#include <glm/glm.hpp>

#include "SGCore/Scene/Scene.h"
#include "SGCore/Main/CoreGlobals.h"
#include "PhysicsDebugDraw.h"
#include "SGUtils/Timer.h"
#include "SGCore/Scene/EntityComponentMember.h"
#include "SGCore/Threading/SafeObject.h"
#include "SGCore/Threading/FixedVector.h"
#include "SGCore/Scene/IParallelSystem.h"

namespace SGCore
{
    struct Transform;
    
    struct PhysicsWorld3D : public IParallelSystem<PhysicsWorld3D>
    {
        friend struct TransformationsUpdater;
        
        PhysicsWorld3D();

        void parallelUpdate(const double& dt, const double& fixedDt) noexcept final;

        void addBody(const Ref<btRigidBody>& rigidBody) noexcept;
        void removeBody(const Ref<btRigidBody>& rigidBody) noexcept;
        
        void update(const double& dt, const double& fixedDt) noexcept override;
        void onAddToScene() override;
        void onRemoveFromScene() override;
        
        auto& getCollisionConfig() noexcept
        {
            return m_collisionConfig;
        }
        
        auto& getCollisionDispatcher() noexcept
        {
            return m_collisionDispatcher;
        }
        
        auto& getOverlappingPairCache() noexcept
        {
            return m_overlappingPairCache;
        }
        
        auto& getSequentialImpulseConstraintSolver() noexcept
        {
            return m_sequentialImpulseConstraintSolver;
        }
        
        auto& getDynamicsWorld() noexcept
        {
            return m_dynamicsWorld;
        }
        
        auto& getDebugDraw() noexcept
        {
            return m_debugDraw;
        }
    
    private:
        Scope<btCollisionConfiguration> m_collisionConfig;
        Scope<btCollisionDispatcher> m_collisionDispatcher;
        Scope<btBroadphaseInterface> m_overlappingPairCache;
        Scope<btSequentialImpulseConstraintSolver> m_sequentialImpulseConstraintSolver;
        Scope<btDynamicsWorld> m_dynamicsWorld;
        Scope<PhysicsDebugDraw> m_debugDraw;
        
        std::mutex m_bodiesCountChangeMutex;
        
        SafeObject<std::vector<EntityComponentMember<glm::mat4>>> m_physicalMatricesVector;
    };
}

#endif // SUNGEARENGINE_PHYSICSWORLD3D_H
