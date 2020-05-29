#pragma once

#include <mutex>
#include <queue>

template <typename T>
class concurrent_queue
{
    using lock_t = std::lock_guard<std::mutex>;

public:
    void push(T &&t)
    {
        lock_t __l(mutex_);
        queue_.emplace(std::move(t));
    }

    void push(const T &t)
    {
        lock_t __l(mutex_);
        queue_.emplace(t);
    }

    bool tryPop(typename std::queue<T>::reference v)
    {
        lock_t __l(mutex_);
        if (!queue_.empty())
        {
            v = std::move(queue_.front());
            queue_.pop();
            return true;
        }
        return false;
    }

    bool empty() const
    {
        return queue_.empty();
    }

private:
    std::mutex      mutex_;
    std::queue<T>   queue_;
};
