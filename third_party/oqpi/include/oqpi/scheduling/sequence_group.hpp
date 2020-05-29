#pragma once

#include <queue>

#include "oqpi/scheduling/task_group.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    // Builds a sequence of tasks as such:
    //
    // [T0] -> [T1] -> [T2] -> ... -> [Tn-1] -> [Tn]
    //
    // [Tn] waits on [Tn-1] completion which waits on [Tn-2] completion etc...
    //
    // This group is not thread safe, meaning the user has to ensure thread safety herself when
    // adding tasks to this kind of group.
    //
    template<typename _Scheduler, task_type _TaskType, typename _GroupContext>
    class sequence_group final
        : public task_group<_Scheduler, _TaskType, _GroupContext>
    {
    public:
        //------------------------------------------------------------------------------------------
        sequence_group(_Scheduler &sc, const std::string &name, task_priority priority)
            : task_group<_Scheduler, _TaskType, _GroupContext>(sc, name, priority)
        {}

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
                while (!empty())
                {
                    auto hTask = popTask();
                    hTask.executeSingleThreaded();
                }
            }
        }


    protected:
        //------------------------------------------------------------------------------------------
        virtual void addTaskImpl(const task_handle &hTask) override final
        {
            tasks_.push(hTask);
        }

        //------------------------------------------------------------------------------------------
        // Executes the current task
        virtual void executeImpl() override final
        {
            auto hTask = popTask();
            if (hTask.tryGrab())
            {
                hTask.execute();
            }
        }

        //------------------------------------------------------------------------------------------
        virtual void oneTaskDone() override final
        {
            if (!empty())
            {
                this->scheduler_.add(popTask());
            }
            else
            {
                this->notifyGroupDone();
            }
        }

    private:
        //------------------------------------------------------------------------------------------
        task_handle popTask()
        {
            task_handle hTask;
            if (oqpi_ensuref(!empty(), "Attempting to execute an empty sequence: %d", this->getUID()))
            {
                hTask = tasks_.front();
                tasks_.pop();
            }
            return hTask;
        }

    private:
        // Tasks of the sequence
        std::queue<task_handle> tasks_;
    };
    //----------------------------------------------------------------------------------------------
    

    //----------------------------------------------------------------------------------------------
    template<task_type _TaskType, typename _GroupContext, typename _Scheduler>
    inline auto make_sequence_group(_Scheduler &sc, const std::string &name, task_priority prio)
    {
        return make_task_group<sequence_group, _TaskType, _GroupContext>(sc, name, prio);
    }
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
