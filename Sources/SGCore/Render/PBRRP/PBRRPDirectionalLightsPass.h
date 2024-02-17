//
// Created by stuka on 26.11.2023.
//

#ifndef SUNGEARENGINE_PBRRPDIRECTIONALLIGHTSPASS_H
#define SUNGEARENGINE_PBRRPDIRECTIONALLIGHTSPASS_H

#include "SGCore/Render/IRenderPass.h"

namespace SGCore
{
    struct PBRRPDirectionalLightsPass : public IRenderPass
    {
        // todo: сделать рендер пассы с настройкой времени выполнения. то же самое сделать с системами
        
        void create(const Ref<IRenderPipeline>& parentRenderPipeline) final;

        void render(const Ref<Scene>& scene, const Ref<IRenderPipeline>& renderPipeline) final;

    private:
        Ref<SGUtils::TimerCallback> m_renderTimerCallback = MakeRef<SGUtils::TimerCallback>();
        SGUtils::Timer m_renderTimer { true };
    };
}

#endif //SUNGEARENGINE_PBRRPDIRECTIONALLIGHTSPASS_H
