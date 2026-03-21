
#ifndef UTILCOMMON_HPP
#define UTILCOMMON_HPP

#include <mutex>

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

struct RaiiDefer{
    template<class Defer>
    RaiiDefer(Defer && defer) : _base{ new _DeferImpl<Defer>(std::move(defer))} {}
    ~RaiiDefer(){ if (_base != nullptr) delete _base; }
    _DeferBase* _base = nullptr;
};
}
#define RAII_DEFER(_defer) util::RaiiDefer CONCAT(_raii_defer_, __LINE__)([&](){_defer})


// 单例模式实现
template<class Type>
class Singleton
{
public:
    using Ptr = std::shared_ptr<Type>; 
public:
    static Ptr instance(){
        if(m_instance == nullptr){
            std::lock_guard<std::mutex> lock(m_mut);
            if(m_instance == nullptr){
                m_instance = std::make_shared<Type>();
            }
        }
        return m_instance;
    }

    static void release(){
        m_instance = nullptr;
    }

private:

    static Ptr m_instance;
    static std::mutex m_mut;
};

template<class Type>
typename Singleton<Type>::Ptr Singleton<Type>::m_instance = nullptr;

template<class Type>
std::mutex Singleton<Type>::m_mut;

#endif /* UTILCOMMON_HPP */
