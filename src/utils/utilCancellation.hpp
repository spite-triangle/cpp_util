#ifndef UTILCANCELLATION_HPP
#define UTILCANCELLATION_HPP

#include <mutex>
#include <vector>
#include <atomic>
#include <memory>
#include <functional>
#include <unordered_map>
#include <condition_variable>


namespace util
{

class CancellationTokenSource;


/* 用于传递的 token */
class CancellationToken : public std::enable_shared_from_this<CancellationToken> {
public:
    using ptr = std::shared_ptr<CancellationToken>;
    using Callback = std::function<void()>;

    /* 用于处理回调的销毁 */
    class CallbackDisposer {
    public:
        using ptr = std::shared_ptr<CallbackDisposer>;
    public:
        CallbackDisposer(CancellationToken::ptr token, size_t index, bool autoDispose = false)
            : m_token(std::move(token)), m_index(index), m_autoDispose(autoDispose) {}

        ~CallbackDisposer() {
            if (m_autoDispose) dispose();
        }

        void dispose() {
            if (m_token) {
                m_token->dispose(m_index);
            }
        }
    private:
        bool m_autoDispose;
        size_t m_index;
        CancellationToken::ptr m_token;
    };

public:
    CancellationToken() noexcept = default;
    ~CancellationToken() {
        dispose();
    }

    bool isCancellationRequested() const noexcept {
        return m_cancelled.load();
    }

    CallbackDisposer::ptr onCancellationRequested(Callback cb, bool autoDispose = false) {
        if (isCancellationRequested()) {
            cb();
            return std::make_shared<CallbackDisposer>(ptr(), -1);
        }

        std::lock_guard<std::mutex> guard(m_callbacksMutex);
        if (m_cancelled.load()) {
            cb();
            return std::make_shared<CallbackDisposer>(ptr(), -1);
        } else {
            auto index = m_nCallbackIndex++;
            m_callbacks.emplace(index, [this,cb, index](){
                cb();
                dispose(index);
            });
            return std::make_shared<CallbackDisposer>(shared_from_this(), index, autoDispose);
        }
    }

    void dispose() {
        std::lock_guard<std::mutex> guard(m_callbacksMutex);
        m_callbacks.clear();
    }

private:
    void dispose(size_t index) {
        std::lock_guard<std::mutex> guard(m_callbacksMutex);
        auto it = m_callbacks.find(index);
        if(it != m_callbacks.end()){
            m_callbacks.erase(it);
        }
    }

    void cancel() {
        bool expected = false;
        if (m_cancelled.compare_exchange_strong(expected, true)) {
            std::unordered_map<size_t, Callback> copy;
            {
                std::lock_guard<std::mutex> guard(m_callbacksMutex);
                copy = std::move(m_callbacks);
                m_callbacks.clear();
            }

            for (auto& f : copy) {
                if (f.second) {
                    f.second();
                }
            }
        }
    }

private:
    friend class CancellationTokenSource;
    size_t m_nCallbackIndex{0};
    std::mutex m_callbacksMutex;
    std::atomic<bool> m_cancelled{false};
    std::unordered_map<size_t, Callback> m_callbacks;
};


/* 管理 token 的源 */
class CancellationTokenSource {
public:
    using ptr = std::shared_ptr<CancellationTokenSource>;

    CancellationTokenSource() noexcept = default;

    explicit CancellationTokenSource(CancellationToken::ptr parent)
        : m_parent(std::move(parent)) {
        if (m_parent) {
            m_parent->onCancellationRequested([this]() { this->cancel(); });
        }
    }

    ~CancellationTokenSource() {
         dispose();
    }

    CancellationToken::ptr token() {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (!m_token) {
            m_token = std::make_shared<CancellationToken>();
        }
        return m_token;
    }

    void cancel() {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (!m_token) {
            m_token = std::make_shared<CancellationToken>();
            m_token->m_cancelled.store(true);
            return;
        }
        m_token->cancel();
    }

    void dispose(bool bCancel = false) {
        if(bCancel) {
            cancel();
        }

        std::lock_guard<std::mutex> guard(m_mutex);
        if (m_token) {
            m_token->dispose();
        }
    }

private:
    std::mutex m_mutex;
    std::atomic<bool> m_disposed{false};
    CancellationToken::ptr m_token;
    CancellationToken::ptr m_parent;
};

/* 等待多个 token 结束 */
class CancellationTokenPool {
public:
    using ptr = std::shared_ptr<CancellationTokenPool>;
public:
    CancellationTokenPool() = default;
    ~CancellationTokenPool() {
        dispose();
    }

    CancellationToken::ptr token() {
        return m_source.token();
    }

    void add(const CancellationToken::ptr& token) {
        if (!token) return;

        std::lock_guard<std::mutex> guard(m_mutex);
        if (m_isDone) {
            return;
        }

        m_nCount++;
        if (token->isCancellationRequested()) {
            m_nCount--;
            check();
            return;
        }

        auto disposer = token->onCancellationRequested([this]() {
            std::lock_guard<std::mutex> guard2(m_mutex);
            m_nCount--;
            check();
        });
        m_disposers.emplace_back(std::move(disposer));
    }

    void dispose() {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_isDone = true;
        m_source.dispose();
        for (auto & item : m_disposers)
        {
            if(item) item->dispose();
        }
    }

private:
    void check() {
        if (!m_isDone && m_nCount == 0) {
            m_isDone = true;
            m_source.cancel();
        }
    }
private:
    size_t m_nCount{0};
    bool m_isDone = false;
    std::mutex m_mutex;
    CancellationTokenSource m_source;
    std::vector<CancellationToken::CallbackDisposer::ptr> m_disposers;
};



} // namespace util


#endif /* UTILCANCELLATION_HPP */
