//
// Created by ilya on 20.03.24.
//

#ifndef SUNGEARENGINE_DYNAMICLIBRARY_H
#define SUNGEARENGINE_DYNAMICLIBRARY_H

#include "SGUtils/CrashHandler/Platform.h"

#include <functional>

#ifdef PLATFORM_OS_LINUX
#include <dlfcn.h>
#elif defined(PLATFORM_OS_WINDOWS)
#include <windows.h>
#endif

#ifdef PLATFORM_OS_WINDOWS
#define DL_POSTFIX ".dll"
#elif defined(PLATFORM_OS_LINUX)
#define DL_POSTFIX ".so"
#endif

namespace SGCore
{
    struct DynamicLibrary
    {
        #ifdef PLATFORM_OS_LINUX
        using native_handler_t = void*;
        #elif defined(PLATFORM_OS_WINDOWS)
        using native_handler_t = HINSTANCE;
        #endif

        ~DynamicLibrary()
        {
            unload();
        }
        
        void load(const std::string& pluginDLPath, std::string& err) noexcept
        {
            unload();
            
            #ifdef PLATFORM_OS_LINUX
            m_nativeHandler = dlopen(pluginDLPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
            if(!m_nativeHandler)
            {
                err = dlerror();
            }
            #elif defined(PLATFORM_OS_WINDOWS)
            m_nativeHandler = LoadLibrary(pluginDLPath.c_str());
            std::cout << "m_nativeHandler " << m_nativeHandler << std::endl;
            if(!m_nativeHandler)
            {
                err = getLastWindowsError();
            }
            #endif
        }
        
        void unload()
        {
            #ifdef PLATFORM_OS_LINUX
            if(m_nativeHandler)
            {
                dlclose(m_nativeHandler);
                m_nativeHandler = nullptr;
            }
            #elif defined(PLATFORM_OS_WINDOWS)
            if(m_nativeHandler)
            {
                FreeLibrary(m_nativeHandler);
                m_nativeHandler = nullptr;
            }
            #endif
        }
        
        template<typename FuncT>
        std::function<FuncT> loadFunction(const char* funcName, std::string& err) noexcept
        {
            if(!m_nativeHandler)
            {
                err = "The function cannot be loaded: the library is not loaded.";
                
                return nullptr;
            }
            
            return DLFuncLoader<FuncT>()(m_nativeHandler, funcName, err);
        }
        
        [[nodiscard]] native_handler_t getNativeHandler() noexcept
        {
            return m_nativeHandler;
        }
        
    private:
        #ifdef PLATFORM_OS_WINDOWS
        static std::string getLastWindowsError()
        {
            DWORD errorMessageID = GetLastError();

            LPSTR messageBuffer = nullptr;
            // Ask Win32 to give us the string version of that message ID.
            // The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                         NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
            // Copy the error message into a std::string.
            std::string message = std::string(messageBuffer, size) + " (Code '" + std::to_string(errorMessageID) + "').";
            // Free the Win32's string's buffer.
            LocalFree(messageBuffer);

            return message;
        }
        #endif

        template<typename> struct DLFuncLoader;
        
        template<typename Result, typename... Args>
        struct DLFuncLoader<Result(Args...)>
        {
            std::function<Result(Args...)> operator()(native_handler_t nativeHandler, const char* funcName, std::string& err) const noexcept
            {
                #ifdef PLATFORM_OS_LINUX
                using func_t = Result(*)(Args...);
                #elif defined(PLATFORM_OS_WINDOWS)
                using func_t = Result(__cdecl *)(Args...);
                #endif
                
                func_t func = nullptr;
                
                #ifdef PLATFORM_OS_LINUX
                func = (func_t) dlsym(nativeHandler, funcName);
                if(!func)
                {
                    err = dlerror();
                }
                #elif defined(PLATFORM_OS_WINDOWS)
                func = (func_t) GetProcAddress(nativeHandler, funcName);
                if(!func)
                {
                    err = DynamicLibrary::getLastWindowsError();
                }
                #endif
                
                return func;
            }
        };

        native_handler_t m_nativeHandler = nullptr;
    };
}

#endif //SUNGEARENGINE_DYNAMICLIBRARY_H
