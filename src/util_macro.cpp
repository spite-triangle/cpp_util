#include "doctest.h"

#include "util.hpp"


// Defer类实现
class DeferBase {
public:
    virtual ~DeferBase() = default;
};

template<typename F, typename... Args>
class DeferImpl : public DeferBase {
    F func;
    std::tuple<Args...> args;
public:
    DeferImpl(F&& f, Args&&... a)
        : func(std::forward<F>(f)), args(std::forward<Args>(a)...) {}
    ~DeferImpl() override {
        std::apply(func, args);
    }
};

class Defer {
    DeferBase* ptr;
public:
    template<typename F, typename... Args>
    Defer(F&& f, Args&&... args) {
        ptr = new DeferImpl<std::decay_t<F>, std::decay_t<Args>...>(std::forward<F>(f), std::forward<Args>(args)...);
    }
    ~Defer() {
        delete ptr;
    }
    Defer(const Defer&) = delete;
    Defer& operator=(const Defer&) = delete;
    Defer(Defer&& other) noexcept : ptr(other.ptr) { other.ptr = nullptr; }
    Defer& operator=(Defer&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }
};

TEST_CASE("defer"){
    printf("begin\n");
    RAII_DEFER(
        printf("defer\n");
    );
    printf("end\n");

    /*
    打印顺序:
      begin
      end
      defer
    */
}