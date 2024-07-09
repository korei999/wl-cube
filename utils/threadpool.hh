#pragma once

#include <condition_variable>
#include <functional>
#include <deque>

class ThreadPool
{
public:
    ThreadPool(size_t nThreads) { this->start(nThreads); }
    ~ThreadPool() { this->stop(); }

    void
    submit(const std::function<void()>& job)
    {
        {
            std::unique_lock lock(this->mtxQ);
            this->qTasks.emplace_back(std::move(job));
        }
        this->cndMtx.notify_one();
    }

    void
    wait()
    {
        if (this->busy())
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

    void
    start(size_t nThreads = std::thread::hardware_concurrency())
    {
        for (size_t i = 0; i < nThreads; i++)
            aThreads.emplace_back(std::thread(&ThreadPool::loop, this));
    }

private:
    bool bDone = false;
    std::atomic<int> activeTasks = 0;
    std::mutex mtxQ, mtxWait;
    std::condition_variable cndMtx, cndWait;
    std::vector<std::thread> aThreads;
    std::deque<std::function<void()>> qTasks;

    void
    loop()
    {
        while (!this->bDone)
        {
            std::function<void()> job;
            {
                std::unique_lock lock(this->mtxQ);

                this->cndMtx.wait(lock, [this]{ return !this->qTasks.empty() || this->bDone; });

                if (this->bDone) return;

                job = this->qTasks.front();
                this->qTasks.pop_front();
                this->activeTasks++; /* increment before unlocking mtxQ to avoid 0 tasks and 0 q possibility */
            }
            job();
            this->activeTasks--;

            if (!this->busy())
                this->cndWait.notify_one(); /* signal for the `wait()` */
        }
    }
};
