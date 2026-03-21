#ifndef UTILCONTAINER_HPP
#define UTILCONTAINER_HPP

#include <atomic>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "utilOptional.hpp"

namespace util
{

/* 线程安全的链式队列 */
template <typename T>
class SafeLinkQueue
{
public:
    struct Node
    {
        T data;
        std::unique_ptr<Node> next;
    };

public:
    // NOTE - 提前创建一个空节点，用来标记队列为空
    SafeLinkQueue() : m_head(new Node()), m_pTail(m_head.get()),m_size{0} {}
    ~SafeLinkQueue() = default;

    SafeLinkQueue(const SafeLinkQueue &other) = delete;
    SafeLinkQueue &operator=(const SafeLinkQueue &other) = delete;

    size_t size(){
        return m_size.load();
    }

    optional<T> tryPop()
    {
        std::unique_ptr<Node> old_head;
        {
            std::lock_guard<std::mutex> head_lock(m_mutHead);
            if (m_head.get() == tail())
            {
                return nullopt;
            }
            old_head = std::move(pop());
        }
        return old_head ? old_head->data : optional<T>();
    }

    T popWait()
    {
        std::unique_lock<std::mutex> head_lock(m_mutHead);

        m_condData.wait(head_lock, [&]{ return m_head.get() != tail(); });

        std::unique_ptr<Node> old_head = pop();
        return old_head->data;
    }

    T popWait(std::chrono::microseconds timeout)
    {
        std::unique_lock<std::mutex> head_lock(m_mutHead);

        auto status = m_condData.wait_for(head_lock, timeout, [&]{ return m_head.get() != tail(); });
        if (!status) {
            throw util::TimeoutError("queue pop wait timeout");
        }

        std::unique_ptr<Node> old_head = pop();
        return old_head->data;
    }

    template<class U>
    void push(U && data)
    {
        std::unique_ptr<Node> p(new Node());
        auto pData = p.get();

        {
            std::lock_guard<std::mutex> tail_lock(m_mutTail);
            m_pTail->data = std::forward<U>(data);
            m_pTail->next = std::move(p);
            m_pTail = pData;
            ++m_size;
        }

        m_condData.notify_one();
    }

    void clear()
    {
        std::lock(m_mutHead, m_mutTail);
        std::lock_guard<std::mutex> head_lock(m_mutHead, std::adopt_lock);
        std::lock_guard<std::mutex> tail_lock(m_mutTail, std::adopt_lock);

        m_head.reset(new Node());
        m_pTail = m_head.get();
        m_size.store(0);
    }

private:
    const Node* tail()
    {
        std::lock_guard<std::mutex> tail_lock(m_mutTail);
        return m_pTail;
    }

    std::unique_ptr<Node> pop(){
        --m_size;
        std::unique_ptr<Node> old_head = std::move(m_head);
        m_head = std::move(old_head->next);
        return old_head;  // NRVO or move will be applied automatically
    }

private:
    std::atomic_size_t m_size;

    std::mutex m_mutHead;
    std::unique_ptr<Node> m_head;

    std::mutex m_mutTail;
    // NOTE - 由于使用的是 unique_ptr 来管理节点，一个对象只能放入一个 unique_ptr 中，因此队尾就直接使用指针
    Node *m_pTail;

    std::condition_variable m_condData;
};
} // namespace util

#endif /* UTILCONTAINER_HPP */
