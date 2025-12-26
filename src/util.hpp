#ifndef UTILS_HPP
#define UTILS_HPP

#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <memory>
#include <functional>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _linux_
#include <iconv.h>
#endif

// 字符串处理宏
#define CONCAT_INTERNAL(x, y) x##y
#define CONCAT(x, y) CONCAT_INTERNAL(x, y)
#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)

// defer 功能实现
namespace util{
struct _DeferBase{
    virtual ~_DeferBase() = default;
};

template<typename Defer>
struct _DeferImpl: public _DeferBase{
    _DeferImpl(Defer && defer) :_defer(std::move(defer)){}
    virtual ~_DeferImpl(){ _defer(); }
    Defer _defer;
};

struct _RaiiDefer{
    template<class Defer>
    _RaiiDefer(Defer && defer) : _base{ new _DeferImpl(std::move(defer))} {}
    ~_RaiiDefer(){ if (_base != nullptr) delete _base; }
    _DeferBase* _base = nullptr;
};
}
#define RAII_DEFER(_defer) util::_RaiiDefer CONCAT(_raii_defer_, __LINE__)([&](){_defer})


namespace util
{

template<typename... Args>
inline std::string format(const std::string& fmt, const Args&... args) {
    char buffer[2048];
    std::sprintf(buffer, fmt.c_str(), args...);
    return std::string(buffer);
}

inline std::wstring fromUtf8(const std::string & str) {
#ifdef _WIN32
    if (str.empty()) return std::wstring();
    int wlen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    if (wlen <= 0) return std::wstring();
    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], wlen);
    return wstr;
#elif defined(__linux__)
    // On Linux, convert UTF-8 to wide string using iconv
    if (str.empty()) return std::wstring();
    iconv_t cd = iconv_open("WCHAR_T", "UTF-8");
    if (cd == (iconv_t)-1) return std::wstring();

    size_t in_left = str.size();
    const char* in_ptr = str.data();
    std::wstring wstr;
    wchar_t out_buf[4096];
    char* out_ptr = (char*)out_buf;
    size_t out_left = sizeof(out_buf);

    // Reset the conversion state
    iconv(cd, nullptr, nullptr, nullptr, nullptr);

    while (in_left > 0) {
        size_t result = iconv(cd, &in_ptr, &in_left, &out_ptr, &out_left);
        if (result == (size_t)-1) {
            // Handle error or skip invalid sequence
            break;
        }
        size_t converted = (out_ptr - (char*)out_buf) / sizeof(wchar_t);
        wstr.append((const wchar_t*)out_buf, converted);
        out_ptr = (char*)out_buf;
        out_left = sizeof(out_buf);
    }
    iconv_close(cd);
    return wstr;
#else
    // Other platforms: assume no wide string conversion available
    return std::wstring();
#endif
}

inline std::string toUtf8(const std::wstring & str) {
#ifdef _WIN32
    if (str.empty()) return std::string();
    int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return std::string();
    std::string result(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], len, nullptr, nullptr);
    return result;
#elif defined(__linux__)
    if (str.empty()) return std::string();
    iconv_t cd = iconv_open("UTF-8", "WCHAR_T");
    if (cd == (iconv_t)-1) return std::string();

    size_t in_left = str.size() * sizeof(wchar_t);
    const char* in_ptr = reinterpret_cast<const char*>(str.data());
    char out_buf[4096];
    char* out_ptr = out_buf;
    size_t out_left = sizeof(out_buf);
    std::string result;

    iconv(cd, nullptr, nullptr, nullptr, nullptr);

    while (in_left > 0) {
        size_t res = iconv(cd, const_cast<char**>(&in_ptr), &in_left, &out_ptr, &out_left);
        if (res == (size_t)-1) break;
        result.append(out_buf, out_ptr - out_buf);
        out_ptr = out_buf;
        out_left = sizeof(out_buf);
    }
    if (out_ptr != out_buf) {
        result.append(out_buf, out_ptr - out_buf);
    }
    iconv_close(cd);
    return result;
#else
    return std::string();
#endif
}

inline std::wstring fromAnsi(const std::string & str) {
#ifdef _WIN32
    if (str.empty()) return std::wstring();
    int wlen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), nullptr, 0);
    if (wlen <= 0) return std::wstring();
    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), &wstr[0], wlen);
    return wstr;
#elif defined(__linux__)
    if (str.empty()) return std::wstring();
    iconv_t cd = iconv_open("WCHAR_T", "GBK//IGNORE");
    if (cd == (iconv_t)-1) return std::wstring();

    size_t in_left = str.size();
    const char* in_ptr = str.data();
    std::wstring wstr;
    wchar_t out_buf[4096];
    char* out_ptr = (char*)out_buf;
    size_t out_left = sizeof(out_buf);

    iconv(cd, nullptr, nullptr, nullptr, nullptr);

    while (in_left > 0) {
        size_t result = iconv(cd, (char**)&in_ptr, &in_left, &out_ptr, &out_left);
        if (result == (size_t)-1) {
            break;
        }
        size_t converted = (out_ptr - (char*)out_buf) / sizeof(wchar_t);
        wstr.append((const wchar_t*)out_buf, converted);
        out_ptr = (char*)out_buf;
        out_left = sizeof(out_buf);
    }
    iconv_close(cd);
    return wstr;
#else
    return std::wstring();
#endif
}

inline std::string toAnsi(const std::wstring & str){
#ifdef _WIN32
    if (str.empty()) return std::string();
    int len = WideCharToMultiByte(CP_ACP, 0, str.c_str(), (int)str.size(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return std::string();
    std::string result(len, 0);
    WideCharToMultiByte(CP_ACP, 0, str.c_str(), (int)str.size(), &result[0], len, nullptr, nullptr);
    return result;
#elif defined(__linux__)
    if (str.empty()) return std::string();
    iconv_t cd = iconv_open("GBK//IGNORE", "WCHAR_T");
    if (cd == (iconv_t)-1) return std::string();

    size_t in_left = str.size() * sizeof(wchar_t);
    const char* in_ptr = reinterpret_cast<const char*>(str.data());
    char out_buf[4096];
    char* out_ptr = out_buf;
    size_t out_left = sizeof(out_buf);
    std::string result;

    iconv(cd, nullptr, nullptr, nullptr, nullptr);

    while (in_left > 0) {
        size_t res = iconv(cd, const_cast<char**>(&in_ptr), &in_left, &out_ptr, &out_left);
        if (res == (size_t)-1) break;
        result.append(out_buf, out_ptr - out_buf);
        out_ptr = out_buf;
        out_left = sizeof(out_buf);
    }
    if (out_ptr != out_buf) {
        result.append(out_buf, out_ptr - out_buf);
    }
    iconv_close(cd);
    return result;
#else
    return std::string();
#endif
}

inline std::string ansi2utf8(const std::string & str){
    return toUtf8(fromAnsi(str));
}

inline std::string utf82ansi(const std::string & str){
    return toAnsi(fromUtf8(str));
}

inline std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

inline std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

inline bool startWith(const std::string& str, const std::string& prefix, bool caseSensitive = true) {
    if (prefix.empty()) return false;
    if (str.length() < prefix.length()) return false;

    std::string cmpStr = str.substr(0, prefix.length());
    std::string cmpPrefix = prefix;

    if (!caseSensitive) {
        cmpStr = toLower(cmpStr);
        cmpPrefix = toLower(cmpPrefix);
    }

    return (cmpStr == cmpPrefix);
}

inline bool startWith(const std::string& str, const std::regex& regex, bool caseSensitive) {
    std::string cmpStr = str;
    if (!caseSensitive) {
        cmpStr = toLower(cmpStr);
    }
    std::smatch match;
    return std::regex_search(cmpStr, match, regex) && match.position() == 0;
}

inline bool endWith(const std::string& str, const std::string& suffix, bool caseSensitive = true) {
    if (suffix.empty()) return false;
    if (str.length() < suffix.length()) return false;
    
    std::string cmpStr = str.substr(str.length() - suffix.length());
    std::string cmpSuffix = suffix;
    
    if (!caseSensitive) {
        cmpStr = toLower(cmpStr);
        cmpSuffix = toLower(cmpSuffix);
    }

    return (cmpStr == cmpSuffix);
}

inline bool endWith(const std::string& str, const std::regex& regex, bool caseSensitive = true) {
    std::string cmpStr = str;
    if (!caseSensitive) {
        cmpStr = toLower(cmpStr);
    }
    std::smatch match;
    // Search for the regex anywhere in the string
    auto searchStart = cmpStr.cbegin();
    while (std::regex_search(searchStart, cmpStr.cend(), match, regex)) {
        // Check if the match ends at the end of the string
        if ((match.position() + match.length()) == cmpStr.length()) {
            return true;
        }
        // Move search start forward to avoid infinite loop
        searchStart = match.suffix().first;
    }
    return false;
}


inline bool contain(const std::string& str, const std::string & target, bool caseSensitive = true) {
    if (target.empty()) return false;
    if (caseSensitive) {
        return str.find(target) != std::string::npos;
    } else {
        std::string lowerStr = toLower(str);
        std::string lowerTarget = toLower(target);
        return lowerStr.find(lowerTarget) != std::string::npos;
    }
}

inline bool contain(const std::string& str, const std::regex & regex) {
    return std::regex_search(str, regex);
}


inline std::string repleace(const std::string& str, const std::string& from, const std::string& to, bool all = false) {
    if (from.empty()) return str;
    std::string result = str;
    size_t pos = 0;
    if (all) {
        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.length(), to);
            pos += to.length();
        }
    } else {
        pos = result.find(from);
        if (pos != std::string::npos) {
            result.replace(pos, from.length(), to);
        }
    }
    return result;
}

inline std::string repleace(const std::string& str, const std::regex& regex, const std::string& to, bool all = false) {
    if (all) {
        return std::regex_replace(str, regex, to);
    } else {
        std::smatch match;
        if (std::regex_search(str, match, regex)) {
            std::string result = str;
            result.replace(match.position(), match.length(), to);
            return result;
        }
        return str;
    }
}

inline std::vector<std::string> split(const std::string& str, const std::string& delimiter, bool caseSensitive = false) {
    std::vector<std::string> tokens;
    if (delimiter.empty()) {
        tokens.push_back(str);
        return tokens;
    }

    std::string::size_type start = 0;
    std::string tempStr = caseSensitive ? str : toLower(str);
    std::string tempDelim = caseSensitive ? delimiter : toLower(delimiter);

    while (true) {
        std::string::size_type end = tempStr.find(tempDelim, start);
        if (end == std::string::npos) {
            tokens.push_back(str.substr(start));
            break;
        }
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    return tokens;
}

inline std::vector<std::string> split(const std::string& str, const std::regex& regex, bool caseSensitive = false) {
    std::vector<std::string> tokens;
    std::string s = caseSensitive ? str : toLower(str);
    std::sregex_token_iterator it(s.begin(), s.end(), regex, -1);
    std::sregex_token_iterator end;
    for (; it != end; ++it) {
        tokens.push_back(it->str());
    }
    return tokens;
}

inline std::string ltrim(const std::string& str){
    auto it = std::find_if(str.begin(), str.end(), [](unsigned char ch) { return !std::isspace(ch); });
    return std::string(it, str.end());
}

inline std::string rtrim(const std::string& str){
    auto it = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); });
    
    return std::string(str.begin(), it.base());
}

inline std::string trims(const std::string& str) { return ltrim(rtrim(str));}

template<typename Number>
inline bool toNumber(const std::string& str, Number& value) {
    try {
        size_t idx = 0;
        if (typeid(Number) == typeid(int)) {
            value = static_cast<Number>(std::stoi(str, &idx));
        } else if (typeid(Number) == typeid(size_t)) {
            value = static_cast<Number>(std::stoull(str, &idx));
        } else if (typeid(Number) == typeid(uint32_t)) {
            value = static_cast<Number>(std::stoul(str, &idx));
        } else if (typeid(Number) == typeid(int32_t)) {
            value = static_cast<Number>(std::stol(str, &idx));
        } else if (typeid(Number) == typeid(double)) {
            value = static_cast<Number>(std::stod(str, &idx));
        } else if (typeid(Number) == typeid(float)) {
            value = static_cast<Number>(std::stof(str, &idx));
        } else if (typeid(Number) == typeid(long)) {
            value = static_cast<Number>(std::stol(str, &idx));
        } else if (typeid(Number) == typeid(unsigned long)) {
            value = static_cast<Number>(std::stoul(str, &idx));
        } else if (typeid(Number) == typeid(long long)) {
            value = static_cast<Number>(std::stoll(str, &idx));
        } else if (typeid(Number) == typeid(unsigned long long)) {
            value = static_cast<Number>(std::stoull(str, &idx));
        } else {
            std::istringstream iss(str);
            iss >> value;
            return !iss.fail() && iss.eof();
        }
        return idx == str.size();
    } catch (...) {
        return false;
    }
}

} // namespace util

#endif /* UTILS_HPP */