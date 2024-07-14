#pragma once

#include <condition_variable>
#include <functional>
#include <deque>
#include <future>

class ThreadPool
{
public:
    ThreadPool(size_t threadCount) { this->start(threadCount); }
    ~ThreadPool() { this->stop(); }

    // void
    // submit(const std::function<void()> task)
    // {
    //     {
    //         std::unique_lock lock(this->mtxQ);
    //         this->qTasks.emplace_back(std::move(task));
    //         this->activeTasks++; /* decrement after completing the task */
    //     }
    //     this->cndMtx.notify_one();
    // }

    template<typename Fn, typename... Args>
    void
    submit(Fn&& f, Args&&... args)
    {
        {
            std::unique_lock lock(this->mtxQ);
            this->qTasks.emplace_back([=]{ f(std::forward<Args>(args)...); });
            this->activeTasks++;
        }
        this->cndMtx.notify_one();
    }

    template<typename Fn, typename... Args>
    auto
    future(Fn&& f, Args&&... args)
    {
        auto task {
            std::make_shared<std::packaged_task<std::invoke_result_t<Fn, Args...>()>>(
                [&f, &args...]() { return f(std::forward<Args>(args)...); }
            )
        };

        {
            std::unique_lock lock(this->mtxQ);
            this->qTasks.emplace_back([task]{ (*task)(); });
            this->activeTasks++;
        }
        this->cndMtx.notify_one();

        return task->get_future();
    }

    void
    wait()
    {
        while (this->busy())
        {
            std::unique_lock lock(this->mtxWait);
            cndWait.wait(lock);
        }
    }

    void
    stop()
    {
        if (!this->bDone)
        {
            this->bDone = true;
            this->cndMtx.notify_all();
            for (std::thread& t : this->aThreads)
                t.join();
        }
    }

    bool
    busy()
    {
        bool bPoolBusy;
        {
            std::unique_lock lock(this->mtxQ);
            bPoolBusy = !this->qTasks.empty();
        }
        return bPoolBusy || this->activeTasks > 0;
    }

private:
    bool bDone = false;
    std::atomic<int> activeTasks = 0;
    std::mutex mtxQ, mtxWait;
    std::condition_variable cndMtx, cndWait;
    std::vector<std::thread> aThreads;
    std::deque<std::function<void()>> qTasks;

    void
    start(size_t threadCount = std::thread::hardware_concurrency())
    {
        for (size_t i = 0; i < threadCount; i++)
            aThreads.emplace_back(std::thread(&ThreadPool::loop, this));
    }

    void
    loop()
    {
        while (!this->bDone)
        {
            std::function<void()> task;
            {
                std::unique_lock lock(this->mtxQ);

                this->cndMtx.wait(lock, [this]{ return !this->qTasks.empty() || this->bDone; });

                if (this->bDone) return;

                task = this->qTasks.front();
                this->qTasks.pop_front();
            }
            task();
            this->activeTasks--;

            if (!this->busy())
                this->cndWait.notify_one(); /* signal for the `wait()` */
        }
    }
};
