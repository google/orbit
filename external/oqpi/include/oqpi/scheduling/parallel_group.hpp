#pragma once

#include <vector>
#include <atomic>

#include "oqpi/scheduling/task_group.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    // Builds a fork of tasks as such:
    //
    //            /----[T0]----\
    //           / ----[T1]---- \
    // (fork) o--- ----[T2]---- ---o (join)
    //           \ ---- .. ---- /
    //            \----[Tn]----/
    //
    // This group is NOT thread safe! Meaning it does not allow multiple threads to concurrently add
    // tasks to it.
    //
    template<typename _Scheduler, task_type _TaskType, typename _GroupContext>
    class parallel_group final
        : public task_group<_Scheduler, _TaskType, _GroupContext>
    {
    public:
        //------------------------------------------------------------------------------------------
        parallel_group(_Scheduler &sc, const std::string &name, task_priority priority, int32_t taskCount = 0, int32_t maxSimultaneousTasks = 0)
            : task_group<_Scheduler, _TaskType, _GroupContext>(sc, name, priority)
            , activeTasksCount_(0)
            , maxSimultaneousTasks_(maxSimultaneousTasks)
            , currentTaskIndex_(1)
        {
            tasks_.reserve(taskCount);
        }

    public:
        //------------------------------------------------------------------------------------------
        virtual bool empty() const override final
        {
            return tasks_.empty();
        }

        //------------------------------------------------------------------------------------------
        // For debug purposes
        virtual void executeSingleThreadedImpl() override final
        {
            if (task_base::tryGrab())
            {
                const auto taskCount = tasks_.size();
                if (oqpi_ensure(taskCount > 0))
                {
                    oqpi_check(taskCount == activeTasksCount_);
                    for (size_t i = 0; i < taskCount; ++i)
                    {
                        tasks_[i].executeSingleThreaded();
                    }
                }
                tasks_.clear();
            }
        }

        //------------------------------------------------------------------------------------------
        virtual void activeWait() override final
        {
            for (auto &hTask : tasks_)
            {
                if (hTask.tryGrab())
                {
                    hTask.execute();
                }
            }

            this->wait();
        }

    protected:
        //------------------------------------------------------------------------------------------
        virtual void addTaskImpl(const task_handle &hTask) override final
        {
            tasks_.emplace_back(hTask);
            activeTasksCount_.fetch_add(1);
        }

        //------------------------------------------------------------------------------------------
        virtual void executeImpl() override final
        {
            const auto taskCount = tasks_.size();
            if (oqpi_ensuref(taskCount > 0, "Trying to execute an empty group"))
            {
                size_t i = 0;
                int32_t scheduledTasks = 0;

                while ((i = currentTaskIndex_.fetch_add(1)) < taskCount)
                {
                    if (!tasks_[i].isGrabbed() && !tasks_[i].isDone())
                    {
                        this->scheduler_.add(tasks_[i]);
                        if (maxSimultaneousTasks_ > 0 && ++scheduledTasks >= maxSimultaneousTasks_-1)
                        {
                            break;
                        }
                    }
                }

                if (tasks_[0].tryGrab())
                {
                    tasks_[0].execute();
                }
            }
        }

        //------------------------------------------------------------------------------------------
        virtual void oneTaskDone() override final
        {
            const auto previousTaskCount = activeTasksCount_.fetch_sub(1);
            if (previousTaskCount == 1)
            {
                this->notifyGroupDone();
            }
            else if (maxSimultaneousTasks_ > 0)
            {
                const auto taskCount = tasks_.size();
                size_t i = 0;
                while ((i = currentTaskIndex_.fetch_add(1)) < taskCount)
                {
                    if (!tasks_[i].isGrabbed() && !tasks_[i].isDone())
                    {
                        this->scheduler_.add(tasks_[i]);
                        break;
                    }
                }
            }
        }

    protected:
        // Number of tasks still running or yet to be run
        std::atomic<size_t>         activeTasksCount_;
        // Tasks of the fork
        std::vector<task_handle>    tasks_;
        // Number of maximum tasks this group is allowed to run in parallel
        const int32_t               maxSimultaneousTasks_;
        // Index of the next task to be scheduled
        std::atomic<size_t>         currentTaskIndex_;
    };
    //----------------------------------------------------------------------------------------------

    
    //----------------------------------------------------------------------------------------------
    template<task_type _TaskType, typename _GroupContext, typename _Scheduler>
    inline auto make_parallel_group(_Scheduler &sc, const std::string &name, task_priority prio, int32_t taskCount = 0, int32_t maxSimultaneousTasks = 0)
    {
        return make_task_group<parallel_group, _TaskType, _GroupContext>(sc, name, prio, taskCount, maxSimultaneousTasks);
    }
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
