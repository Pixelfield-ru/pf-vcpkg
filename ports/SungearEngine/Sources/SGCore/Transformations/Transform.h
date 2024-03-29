//
// Created by stuka on 03.02.2024.
//

#ifndef SUNGEARENGINE_TRANSFORM_H
#define SUNGEARENGINE_TRANSFORM_H

#include <atomic>
#include "TransformBase.h"
#include "SGCore/Main/CoreGlobals.h"

namespace SGCore
{
    // sizeof(Transform) = 767
    struct Transform
    {
        TransformBase m_finalTransform;
        TransformBase m_ownTransform;

        // will transform follow parent entity`s translation, rotation and scale
        // x - follow translation
        // y - follow rotation
        // z - follow scale
        glm::vec<3, bool, glm::highp> m_followParentTRS { true, true, true };
        // glm::bvec3 m_lastFollowParentTRS = glm::vec3 { false };
        
        bool m_transformChanged = false;
    };
}

#endif //SUNGEARENGINE_TRANSFORM_H
