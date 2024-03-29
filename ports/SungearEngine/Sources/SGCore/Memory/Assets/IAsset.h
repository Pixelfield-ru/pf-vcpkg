//
// Created by stuka on 07.05.2023.
//

#ifndef NATIVECORE_IASSET_H
#define NATIVECORE_IASSET_H

#include <iostream>
#include <memory>
#include <list>
#include <functional>
#include <filesystem>

#include "IAssetObserver.h"
#include "SGCore/Main/CoreGlobals.h"
#include "SGUtils/UniqueName.h"

namespace SGCore
{
    class IAssetObserver;

    class IAsset : public UniqueNameWrapper
    {
        friend class AssetManager;

    public:
        std::string m_name;

        virtual void load(const std::string& path) = 0;

        void addObserver(const std::shared_ptr<IAssetObserver>&) noexcept;
        void removeObserver(const std::shared_ptr<IAssetObserver>&) noexcept;

        void onModified();
        void onPathChanged();
        void onDeleted();
        void onRestored();

        long getLastModified() noexcept;
        [[nodiscard]] std::filesystem::path getPath() const noexcept;

    protected:
        long m_lastModified = -1;
        std::filesystem::path m_path;
        std::list<Weak<IAssetObserver>> m_observers;

    private:
        size_t m_assetTypeID = 0;
        
        template<typename InstanceT>
        requires(std::is_base_of_v<IAsset, InstanceT>)
        static Ref<InstanceT> createRefInstance() noexcept
        {
            return MakeRef<InstanceT>();
        }
    };
}

#endif //NATIVECORE_IASSET_H
