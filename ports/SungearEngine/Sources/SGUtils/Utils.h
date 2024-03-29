#ifndef SUNGEARENGINE_UTILS_H
#define SUNGEARENGINE_UTILS_H

#define SG_INSTANCEOF(data, type) SGUtils::Utils::instanceof<type>(data)

#define SG_MAY_NORETURN __declspec(noreturn)

#define SG_CTOR(cls) cls() = default;
#define SG_COPY_CTOR(cls) cls(const cls&) = default;
#define SG_MOVE_CTOR(cls) cls(cls&& cls) noexcept = default;

#define SG_NO_CTOR(cls) cls() = delete;
#define SG_NO_COPY(cls) cls(const cls&) = delete;
#define SG_NO_MOVE(cls) cls(cls&& cls) noexcept = delete;

#define SG_CURRENT_LOCATION_STR SGUtils::Utils::sourceLocationToString(std::source_location::current())

#define SG_STRINGIFY(n) #n

#ifdef _MSC_VER
#define SG_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define SG_NOINLINE __attribute__((noinline))
#endif

#ifdef _MSC_VEC
#define SG_CDECL __cdecl
#elif defined(__GNUC__)
#define SG_CDECL __attribute__((cdecl))
#endif

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <source_location>
#include <chrono>
#include <locale>
#include <codecvt>
#include <memory>

namespace SGCore
{
    static std::chrono::high_resolution_clock::time_point now() noexcept
    {
        return std::chrono::high_resolution_clock::now();
    }
    
    template<typename ScalarT, typename TimeValT>
    static ScalarT timeDiff(const std::chrono::high_resolution_clock::time_point& begin,
                            const std::chrono::high_resolution_clock::time_point& end) noexcept
    {
        std::chrono::duration<ScalarT, TimeValT> timeSpan = end - begin;
        
        return timeSpan.count();
    }
}

namespace SGUtils
{
    class Utils
    {
    public:
        // stores type of T
        template<typename T>
        struct TypeWrapper { using type = T; };

        template<typename T>
        struct WeakCompare
        {
            bool operator()(const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs) const
            {
                auto lptr = lhs.lock(), rptr = rhs.lock();
                if (!rptr) return false; // nothing after expired pointer
                if (!lptr) return true;  // every not expired after expired pointer

                return lptr.get() < rptr.get();
            }
        };

        /**
         * Example
         * forTypes<InTypes...>([](auto t) { using type = typename decltype(t)::type; });
         * @tparam InTypes Types through which iteration will occur.
         * @tparam Func Type of function.
         * @param f Function.
         */
        template <typename... InTypes, typename Func>
        static void forTypes(Func&& func)
        {
            (func(TypeWrapper<InTypes>()), ...);
        }

        template<typename Base, typename T>
        static bool instanceof(const T* ptr)
        {
            return dynamic_cast<const Base*>(ptr) != nullptr;
        }

        static void splitString(const std::string& str, char delim, std::vector<std::string>& words) noexcept
        {
            std::stringstream ss(str);
            std::string word;
            while(std::getline(ss, word, delim)) {
                words.push_back(word);
            }
        }

        static std::string replaceAll(const std::string& str, const std::string& from, const std::string& to) noexcept
        {
            std::string resString = str;

            size_t start_pos = 0;
            while((start_pos = resString.find(from, start_pos)) != std::string::npos) {
                resString.replace(start_pos, from.length(), to);
                start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
            }

            return resString;
        }

        static std::string replaceFirst(const std::string& str, const std::string& from, const std::string& to) noexcept
        {
            std::string resString = str;

            size_t start_pos = 0;
            start_pos = resString.find(from, start_pos);
            if(start_pos != std::string::npos) {
                resString.replace(start_pos, from.length(), to);
            }

            return resString;
        }

        static std::string trim(const std::string& str,
                                const std::string& whitespace = " \t")
        {
            const auto strBegin = str.find_first_not_of(whitespace);
            if(strBegin == std::string::npos)
            {
                return ""; // no content
            }
            const auto strEnd = str.find_last_not_of(whitespace);
            const auto strRange = strEnd - strBegin + 1;
            return str.substr(strBegin, strRange);
        }

        static std::string reduce(const std::string& str,
                                  const std::string& fill = " ",
                                  const std::string& whitespace = " \t")
        {
            // trim first
            auto result = trim(str, whitespace);
            // replace sub ranges
            auto beginSpace = result.find_first_of(whitespace);
            while (beginSpace != std::string::npos)
            {
                const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
                const auto range = endSpace - beginSpace;
                result.replace(beginSpace, range, fill);
                const auto newStart = beginSpace + fill.length();
                beginSpace = result.find_first_of(whitespace, newStart);
            }
            return result;
        }

        static std::string toString(const std::vector<std::string>& vec) noexcept
        {
            std::string str;

            for(auto& s : vec)
            {
                str += s;
            }

            return str;
        }

        static std::string toString(const std::vector<std::string>::iterator& begin, const std::vector<std::string>::iterator& end) noexcept
        {
            std::string str;

            auto it = begin;

            while(it != end)
            {
                str += *it;
                ++it;
            }

            return str;
        }

        static std::string sourceLocationToString(const std::source_location& location) noexcept
        {
            return "\tFile: " + std::string(location.file_name()) + "\n"
                                                                    "\tFunction: " +
                   std::string(location.function_name()) + "\n" + "\tLine: " +
                   std::to_string(location.line()) + "\n" + "\tColumn: " +
                   std::to_string(location.column()) + "\n";
        }
        
        template <typename T>
        static std::string toUTF8(const std::basic_string<T, std::char_traits<T>, std::allocator<T>>& source)
        {
            std::string result;
            std::wstring_convert<std::codecvt_utf8_utf16<T>, T> convertor;
            result = convertor.to_bytes(source);
            return result;
        }
        
        template <typename T>
        static void fromUTF8(const std::string& source, std::basic_string<T, std::char_traits<T>, std::allocator<T>>& result)
        {
            std::wstring_convert<std::codecvt_utf8_utf16<T>, T> convertor;
            result = convertor.from_bytes(source);
        }
        
        template <typename T>
        static std::basic_string<T, std::char_traits<T>, std::allocator<T>> fromUTF8(const std::string& source)
        {
            std::wstring_convert<std::codecvt_utf8_utf16<T>, T> convertor;
            return convertor.from_bytes(source);
        }

        static long long getTimeMilliseconds() noexcept
        {
            using namespace std::chrono;
            auto timepoint = system_clock::now();
            auto coarse = system_clock::to_time_t(timepoint);
            auto fine = time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count();

            return fine;
        }

        static long long getTimeMicros() noexcept
        {
            using namespace std::chrono;
            auto timepoint = system_clock::now();
            auto coarse = system_clock::to_time_t(timepoint);
            auto fine = time_point_cast<std::chrono::microseconds>(timepoint).time_since_epoch().count();

            return fine;
        }

        static long long getTimeNanos() noexcept
        {
            using namespace std::chrono;
            auto timepoint = system_clock::now();
            auto coarse = system_clock::to_time_t(timepoint);
            auto fine = time_point_cast<std::chrono::nanoseconds>(timepoint).time_since_epoch().count();

            return fine;
        }
    };
}


#endif //SUNGEARENGINE_UTILS_H
