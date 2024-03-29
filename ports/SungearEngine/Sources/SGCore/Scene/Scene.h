#ifndef SUNGEARENGINE_SCENE_H
#define SUNGEARENGINE_SCENE_H

#include <vector>
#include <list>
#include <memory>
#include <map>
#include <variant>
#include <type_traits>
#include <entt/entt.hpp>
#include <sgcore_export.h>

#include "Layer.h"
#include "ISystem.h"
#include "SGUtils/Utils.h"
#include "SGUtils/UniqueNamesManager.h"
#include "SGUtils/Event.h"
#include "SGUtils/EventListener.h"
#include "SGUtils/TypeTraits.h"

namespace SGCore
{
    struct XMLDocument;
    
    class SGCORE_EXPORT Scene : public std::enable_shared_from_this<Scene>
    {
    public:
        Scene();
        
        std::string m_name;

        void createDefaultSystems();

        static Ref<Scene> getCurrentScene() noexcept;
        
        void update(const double& dt, const double& fixedDt);
        void fixedUpdate(const double& dt, const double& fixedDt);

        template<typename SystemT>
        // requires(std::is_base_of_v<ISystem, SystemT>)
        Ref<SystemT> getSystem()
        {
            for(auto& system : m_systems)
            {
                if(SG_INSTANCEOF(system.get(), SystemT))
                {
                    return std::static_pointer_cast<SystemT>(system);
                }
            }

            return nullptr;
        }
        
        template<typename SystemT>
        // requires(std::is_base_of_v<SystemT, ISystem>)
        std::vector<Ref<SystemT>> getSystems()
        {
            std::vector<Ref<SystemT>> foundSystems;
            
            for(auto& system : m_systems)
            {
                if(SG_INSTANCEOF(system.get(), SystemT))
                {
                    foundSystems.push_back(std::static_pointer_cast<SystemT>(system));
                }
            }
            
            return foundSystems;
        }

        void addSystem(const Ref<ISystem>& system) noexcept;
        const std::vector<Ref<ISystem>>& getAllSystems() noexcept;

        auto getUniqueNamesManager() const noexcept
        {
            return m_uniqueNamesManager;
        }

        // size_t createBaseEntity() noexcept;

        auto& getECSRegistry() noexcept
        {
            return m_ecsRegistry;
        }
        
        static void addScene(const Ref<Scene>& scene) noexcept;
        static Ref<Scene> getScene(const std::string& sceneName) noexcept;
        static void setCurrentScene(const std::string& sceneName) noexcept;
        
        static const auto& getScenes() noexcept
        {
            return m_scenes;
        }
        
        Layer createLayer(const std::string& name) noexcept;
        
        void setUIXMLDocument(const Ref<XMLDocument>& xmlDocument) noexcept;
        Weak<XMLDocument> getUIXMLDocument() const noexcept;
        void reloadUI() noexcept;
        
        double getUpdateFunctionExecutionTime() const noexcept;
        double getFixedUpdateFunctionExecutionTime() const noexcept;

    private:
        double m_update_executionTime = 0.0;
        double m_fixedUpdate_executionTime = 0.0;
        
        entt::basic_registry<highp_entity> m_ecsRegistry;

        Ref<UniqueNamesManager> m_uniqueNamesManager = MakeRef<UniqueNamesManager>();

        std::vector<Ref<ISystem>> m_systems;
        std::unordered_map<std::string, Layer> m_layers;

        size_t m_maxLayersCount = 0;
        
        Weak<XMLDocument> m_UIXMLDocument;
        
        static inline Ref<Scene> m_currentScene;
        static inline std::vector<Ref<Scene>> m_scenes;
    };
}

#endif //SUNGEARENGINE_SCENE_H