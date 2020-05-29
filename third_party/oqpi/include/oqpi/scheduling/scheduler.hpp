#pragma once

#include <vector>
#include <atomic>

#include "oqpi/scheduling/worker.hpp"
#include "oqpi/scheduling/task_handle.hpp"
#include "oqpi/scheduling/worker_context.hpp"
#include "oqpi/scheduling/task_group_base.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    // The scheduler holds several task queues (one queue per priority).
    // It also holds a list of workers. It assigns tasks to workers according to priority
    // rules. A worker can be assigned to one to several priorities. The scheduler will always
    // check the queue with the highest priority first then go down to the lowest if and only if 
    // the highest priority queues are empty.
    //
    template<template<typename> class _TaskQueueType>
    class scheduler
    {
    public:
        static_assert(std::is_default_constructible<_TaskQueueType<task_handle>>::value, "_TaskQueueType must be default constructible.");

    public:
        using self_type = scheduler<_TaskQueueType>;

    public:
        scheduler()
            : running_(false)
        {
            std::memset(&workersPerPrio_[0], 0, sizeof(workersPerPrio_));
        }

        ~scheduler()
        {
            if (running_.load())
            {
                stop();
            }
        }

        scheduler(const self_type &)               = delete;
        self_type& operator= (const self_type &)    = delete;

    public:
        //------------------------------------------------------------------------------------------
        // Creates the workers with a user defined context
        template<typename _Thread, typename _Notifier, typename _WorkerContext>
        void registerWorker(const worker_config &config)
        {
            for (int prio = 0; prio < PRIO_COUNT; ++prio)
            {
                if (can_work_on_priority(config.workerPrio, task_priority(prio)))
                {
                    workersPerPrio_[prio] += config.count;
                }
            }

            using worker_type = worker<_Thread, _Notifier, self_type, _WorkerContext>;
            for (int i = 0; i < config.count; ++i)
            {
                workers_.emplace_back(std::make_unique<worker_type>(*this, i, config));
            }
        }
        //------------------------------------------------------------------------------------------
        template<typename _Thread, typename _Notifier, typename _WorkerContext, int N>
        void registerWorkers(const worker_config(&configs)[N])
        {
            for (const auto &config : configs)
            {
                registerWorker<_Thread, _Notifier, _WorkerContext>(config);
            }
        }

        //------------------------------------------------------------------------------------------
        // Creates the workers with a default empty context
        template<typename _Thread, typename _Notifier>
        void registerWorker(const worker_config &config)
        {
            registerWorker<_Thread, _Notifier, empty_worker_context>(config);
        }
        //------------------------------------------------------------------------------------------
        template<typename _Thread, typename _Notifier, int N>
        void registerWorkers(const worker_config(&configs)[N])
        {
            registerWorkers<_Thread, _Notifier, empty_worker_context>(configs);
        }

        //------------------------------------------------------------------------------------------
        // Check if the config is valid and start the workers
        void start()
        {
            if (oqpi_ensuref(running_.load() == false, "Scheduler already started."))
            {
                for (int prio = 0; prio < PRIO_COUNT; ++prio)
                {
                    oqpi_checkf(workersPerPrio_[prio] > 0, "No worker for priority %d", prio);
                }

                running_.store(true);

                for (auto &upWorker : workers_)
                {
                    upWorker->start();
                }
            }
        }

        //------------------------------------------------------------------------------------------
        // Stops the workers and waits on them
        void stop()
        {
            running_.store(false);

            for (auto &upWorker : workers_)
            {
                upWorker->stop();
            }

            wakeUpAllWorkers();

            for (auto &upWorker : workers_)
            {
                upWorker->join();
            }
        }

        //------------------------------------------------------------------------------------------
        // Number of workers registered no matter the priority
        int workersTotalCount() const
        {
            return int(workers_.size());
        }
        //------------------------------------------------------------------------------------------
        // Number of workers registered for the specified priority.
        // Note that the sum of workers for each priority does not necessarily equal the workers
        // total count as the same worker can be registered for several priorities.
        int workersCount(task_priority prio) const
        {
            oqpi_checkf(prio < task_priority::count, "Invalid priority: %d", int(prio));
            return workersPerPrio_[int(prio)];
        }

        //------------------------------------------------------------------------------------------
        // Pushes a task handle in the queue and returns the same passed handle.
        // The handle is warrantied to be valid at the return of this function, even if the task
        // completed in the meantime.
        // It also signals all workers able to work on the task's priority.
        task_handle add(task_handle hTask)
        {
            if (hTask.isValid() && !hTask.isGrabbed() && !hTask.isDone())
            {
                const auto priority = resolveTaskPriority(hTask);
                pendingTasks_[int(priority)].push(hTask);
                wakeUpWorkersWithPriority(priority);
            }
            return hTask;
        }

    private:
        //------------------------------------------------------------------------------------------
        // Get the actual priority of the task, task items can be set to inherit so they take the
        // priority of their owning group
        task_priority resolveTaskPriority(const task_handle &hTask)
        {
            task_priority priority = hTask.getPriority();

            if (priority == task_priority::inherit)
            {
                auto pParentGroup = hTask.getParentGroup().get();
                do 
                {
                    if (oqpi_failedf(pParentGroup != nullptr, "One parent group is invalid for this task: %d", hTask.getUID()))
                    {
                         priority = task_priority::normal;
                        break;
                    }

                    priority     = pParentGroup->getPriority();
                    pParentGroup = pParentGroup->getParentGroup().get();

                } while (priority == task_priority::inherit);
            }

            return priority;
        }

        //------------------------------------------------------------------------------------------
        // Retrieve a task to work on
        task_handle waitForNextTask(worker_base &w)
        {
            const auto pumpTask = [this, &w](task_handle &hTask)
            {
                if (!running_.load())
                {
                    return true;
                }

                for (auto prio = 0; prio < PRIO_COUNT; ++prio)
                {
                    if (w.canWorkOnPriority(task_priority(prio)))
                    {
                        while (pendingTasks_[prio].tryPop(hTask))
                        {
                            // We got a task, try to grab it to ensure that we can work on it
                            // Note that a task_group can be done without being grabbed when calling activeWait
                            if (hTask.tryGrab() && !hTask.isDone())
                            {
                                // We got the go to start working on the current task
                                return true;
                            }
                            else
                            {
                                // The task has already been grabbed by someone else
                                hTask.reset();
                            }
                        }
                    }
                }

                return false;
            };

            // Before checking if we have a task decrement the semaphore count until it reaches 0
            while (w.tryWait());

            task_handle hTask;
            while (!pumpTask(hTask))
            {
                w.wait();
                while (w.tryWait());
            }

            return hTask;
        }

    public:
        //------------------------------------------------------------------------------------------
        // Called by worker threads when they are available, this function blocks on a semaphore.
        // Once it receives a token it proceeds to getting a valid task from the queue.
        void signalAvailableWorker(worker_base &w)
        {
            // Loop until we find a task to work on
            while (true)
            {
                // Grab the next task
                task_handle hTask = waitForNextTask(w);

                // We could have been waken up to stop
                if (!running_.load())
                {
                    return;
                }

                // Make sure it's runnable
                if (oqpi_ensure(hTask.isValid() && hTask.isGrabbed()))
                {
                    // Make sure it's not done (activeWait on groups can leave a task done without being grabbed)
                    if (!hTask.isDone())
                    {
                        // Assign it to the available worker
                        w.assign(std::move(hTask));

                        // We got a task! See ya!
                        break;
                    }
                }
            }
        }

    private:
        //------------------------------------------------------------------------------------------
        // Signal all workers
        void wakeUpAllWorkers()
        {
            for (auto &upWorker : workers_)
            {
                upWorker->notify();
            }

        }
        // Signal only the workers of the specified priority
        void wakeUpWorkersWithPriority(task_priority prio)
        {
            for (auto &upWorker : workers_)
            {
                if (upWorker->canWorkOnPriority(prio))
                {
                    upWorker->notify();
                }
            }
        }

    private:
        static const auto PRIO_COUNT = int32_t(task_priority::count);

        std::vector<worker_uptr>    workers_;
        int32_t                     workersPerPrio_[PRIO_COUNT];
        _TaskQueueType<task_handle> pendingTasks_[PRIO_COUNT];
        std::atomic<bool>           running_;
    };
    //----------------------------------------------------------------------------------------------

 } /*oqpi*/
