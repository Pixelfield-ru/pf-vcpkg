//
// Created by stuka on 02.05.2023.
//

#pragma once

#ifndef NATIVECORE_TRANSFORMCOMPONENT_H
#define NATIVECORE_TRANSFORMCOMPONENT_H

#include <glm/glm.hpp>

#include "SGUtils/Math/MathUtils.h"
#include "SGUtils/Utils.h"
#include "SGUtils/Event.h"
#include "SGUtils/Math/AABB.h"

namespace SGCore
{
    // sizeof(TransformBase) == 382
    // todo: make quaternion transformations
    struct TransformBase
    {
        friend struct TransformationsUpdater;

        bool m_blockTranslation = false;
        bool m_blockRotation = false;
        bool m_blockScale = false;

        AABB m_aabb;
        
        glm::vec3 m_position { 0.0 };
        glm::vec3 m_rotation { 0.0 };
        glm::vec3 m_scale { 1.0 };

        glm::vec3 m_left = MathUtils::left3;
        glm::vec3 m_forward = MathUtils::forward3;
        glm::vec3 m_up = MathUtils::up3;

        glm::mat4 m_translationMatrix = glm::mat4(1);
        glm::mat4 m_rotationMatrix = glm::mat4(1);
        glm::mat4 m_scaleMatrix = glm::mat4(1);

        bool m_positionChanged = false;
        bool m_rotationChanged = false;
        bool m_scaleChanged = false;

        glm::mat4 m_modelMatrix = glm::mat4(1);

    // private:
        glm::vec3 m_lastPosition { 0.0 };
        glm::vec3 m_lastRotation { 0.0 };
        glm::vec3 m_lastScale = glm::vec3(0);
    };
}

#endif //NATIVECORE_TRANSFORMCOMPONENT_H
