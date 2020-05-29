#pragma once

#include "oqpi/synchronization/event.hpp"
#include "oqpi/scheduling/task.hpp"
#include "oqpi/scheduling/scheduler.hpp"
#include "oqpi/scheduling/task_type.hpp"
#include "oqpi/scheduling/task_handle.hpp"
#include "oqpi/scheduling/task_context.hpp"
#include "oqpi/scheduling/group_context.hpp"
#include "oqpi/scheduling/parallel_group.hpp"
#include "oqpi/scheduling/sequence_group.hpp"
#include "oqpi/parallel_algorithms/parallel_for.hpp"
#include "oqpi/concurrent_queue.hpp"


namespace oqpi {

    template
    <
          typename _Scheduler           = scheduler<concurrent_queue>
        // The context to use for making groups
        , typename _DefaultGroupContext = empty_group_context
        // The context to use for making tasks
        , typename _DefaultTaskContext  = empty_task_context
        // Type of events used for notification when a task is done
        , typename _EventType           = manual_reset_event_interface<>
    >
    struct helpers
    {
        //------------------------------------------------------------------------------------------
        using self_type = helpers<_Scheduler, _DefaultGroupContext, _DefaultTaskContext, _EventType>;

        //------------------------------------------------------------------------------------------
        static constexpr auto default_priority = task_priority::normal;


        //------------------------------------------------------------------------------------------
        // Static instance
        static _Scheduler scheduler_;
        static _Scheduler& scheduler() { return scheduler_; }


        //------------------------------------------------------------------------------------------
        // Start the scheduler with a default workers configuration
        inline static void start_default_scheduler()
        {
            // Use the default thread and semaphore (without any layer)
            using default_thread    = thread_interface<>;
            using default_semaphore = semaphore_interface<>;

            auto config = oqpi::worker_config{};
            // Let the workers roam on all cores.
            config.threadAttributes.coreAffinityMask_   = oqpi::core_affinity::all_cores;
            // The worker's id will be appended to the thread's name.
            config.threadAttributes.name_               = "oqpi::worker_";
            // Set the worker's thread priority to a high value.
            config.threadAttributes.priority_           = oqpi::thread_priority::highest;
            // Workers can work on any task priority.
            config.workerPrio                           = oqpi::worker_priority::wprio_any;
            // Start as many workers as there are cores.
            config.count                                = default_thread::hardware_concurrency();

            scheduler_.registerWorker<default_thread, default_semaphore>(config);
            // Fire it up!
            scheduler_.start();
        }

        //------------------------------------------------------------------------------------------
        inline static void stop_scheduler()
        {
            scheduler_.stop();
        }


        //------------------------------------------------------------------------------------------
        // Add a task to the scheduler
        inline static task_handle schedule_task(const task_handle &hTask)
        {
            return scheduler_.add(hTask);
        }
        //------------------------------------------------------------------------------------------
        inline static task_handle schedule_task(task_handle &&hTask)
        {
            return scheduler_.add(std::move(hTask));
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Creates AND adds a task to the scheduler, any callable object can be passed along.
        // All the following overloads end up calling this function.
        //
        // Type     : user defined
        // Context  : user defined
        // Priority : user defined
        template<task_type _TaskType, typename _TaskContext, typename _Func, typename... _Args>
        inline static task_handle schedule_task(const std::string &name, task_priority prio, _Func &&f, _Args &&...args)
        {
            auto spTask = self_type::make_task<_TaskType, _TaskContext>(name, prio, std::forward<_Func>(f), std::forward<_Args>(args)...);
            return self_type::schedule_task(std::move(spTask));
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Creates a waitable task and schedules it right away
        // 
        // Type     : waitable
        // Context  : user defined
        // Priority : user defined
        template<typename _TaskContext, typename _Func, typename... _Args>
        inline static task_handle schedule_task(const std::string &name, task_priority prio, _Func &&f, _Args &&...args)
        {
            return self_type::schedule_task<task_type::waitable, _TaskContext>(name, prio, std::forward<_Func>(f), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : waitable
        // Context  : default
        // Priority : user defined
        template<typename _Func, typename... _Args>
        inline static task_handle schedule_task(const std::string &name, task_priority prio, _Func &&f, _Args &&...args)
        {
            return self_type::schedule_task<_DefaultTaskContext>(name, prio, std::forward<_Func>(f), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : waitable
        // Context  : user defined
        // Priority : default
        template<typename _TaskContext, typename _Func, typename... _Args>
        inline static task_handle schedule_task(const std::string &name, _Func &&f, _Args &&...args)
        {
            return self_type::schedule_task<_TaskContext>(name, default_priority, std::forward<_Func>(f), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : waitable
        // Context  : default
        // Priority : default
        template<typename _Func, typename... _Args>
        inline static task_handle schedule_task(const std::string &name, _Func &&f, _Args &&...args)
        {
            return self_type::schedule_task(name, default_priority, std::forward<_Func>(f), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Type     : fire_and_forget
        // Context  : user defined
        // Priority : user defined
        template<typename _TaskContext, typename _Func, typename... _Args>
        inline static void fire_and_forget_task(const std::string &name, task_priority prio, _Func &&f, _Args &&...args)
        {
            self_type::schedule_task<task_type::fire_and_forget, _TaskContext>(name, prio, std::forward<_Func>(f), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : fire_and_forget
        // Context  : default
        // Priority : user defined
        template<typename _Func, typename... _Args>
        inline static void fire_and_forget_task(const std::string &name, task_priority prio, _Func &&f, _Args &&...args)
        {
            self_type::fire_and_forget_task<_DefaultTaskContext>(name, prio, std::forward<_Func>(f), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : fire_and_forget
        // Context  : user defined
        // Priority : default
        template<typename _TaskContext, typename _Func, typename... _Args>
        inline static void fire_and_forget_task(const std::string &name, _Func &&f, _Args &&...args)
        {
            self_type::fire_and_forget_task<_TaskContext>(name, default_priority, std::forward<_Func>(f), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : fire_and_forget
        // Context  : default
        // Priority : default
        template<typename _Func, typename... _Args>
        inline static void fire_and_forget_task(const std::string &name, _Func &&f, _Args &&...args)
        {
            self_type::fire_and_forget_task(name, default_priority, std::forward<_Func>(f), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------




        //------------------------------------------------------------------------------------------
        // Create a task, the task is NOT added to the scheduler
        //
        // Type     : user defined
        // Context  : user defined
        // Priority : user defined
        template<task_type _TaskType, typename _TaskContext, typename _Func, typename... _Args>
        inline static auto make_task(const std::string &name, task_priority priority, _Func &&func, _Args &&...args)
        {
            return oqpi::make_task<_TaskType, _EventType, _TaskContext>(name, priority, std::forward<_Func>(func), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : user defined
        // Context  : default
        // Priority : user defined
        template<task_type _TaskType, typename _Func, typename... _Args>
        inline static auto make_task(const std::string &name, task_priority priority, _Func &&func, _Args &&...args)
        {
            return self_type::make_task<_TaskType, _DefaultTaskContext>(name, priority, std::forward<_Func>(func), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : user defined
        // Context  : user defined
        // Priority : default
        template<task_type _TaskType, typename _TaskContext, typename _Func, typename... _Args>
        inline static auto make_task(const std::string &name, _Func &&func, _Args &&...args)
        {
            return self_type::make_task<_TaskType, _TaskContext>(name, default_priority, std::forward<_Func>(func), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : user defined
        // Context  : default
        // Priority : default
        template<task_type _TaskType, typename _Func, typename... _Args>
        inline static auto make_task(const std::string &name, _Func &&func, _Args &&...args)
        {
            return self_type::make_task<_TaskType>(name, default_priority, std::forward<_Func>(func), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Creates a waitable task, the task is NOT added to the scheduler
        //
        // Type     : waitable
        // Context  : user defined
        // Priority : user defined
        template<typename _TaskContext, typename _Func, typename... _Args>
        inline static auto make_task(const std::string &name, task_priority priority, _Func &&func, _Args &&...args)
        {
            return self_type::make_task<task_type::waitable, _TaskContext>(name, priority, std::forward<_Func>(func), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : waitable
        // Context  : default
        // Priority : user defined
        template<typename _Func, typename... _Args>
        inline static auto make_task(const std::string &name, task_priority priority, _Func &&func, _Args &&...args)
        {
            return self_type::make_task<_DefaultTaskContext>(name, priority, std::forward<_Func>(func), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : waitable
        // Context  : user defined
        // Priority : default
        template<typename _TaskContext, typename _Func, typename... _Args>
        inline static auto make_task(const std::string &name, _Func &&func, _Args &&...args)
        {
            return self_type::make_task<_TaskContext>(name, default_priority, std::forward<_Func>(func), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : waitable
        // Context  : default
        // Priority : default
        template<typename _Func, typename... _Args>
        inline static auto make_task(const std::string &name, _Func &&func, _Args &&...args)
        {
            return self_type::make_task(name, default_priority, std::forward<_Func>(func), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Creates a fire and forget task, the task is NOT added to the scheduler
        //
        // Type     : fire_and_forget
        // Context  : user defined
        // Priority : user defined
        template<typename _TaskContext, typename _Func, typename... _Args>
        inline static auto make_task_item(const std::string &name, task_priority priority, _Func &&func, _Args &&...args)
        {
            return self_type::make_task<task_type::fire_and_forget, _TaskContext, _Func>(name, priority, std::forward<_Func>(func), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : fire_and_forget
        // Context  : default
        // Priority : user defined
        template<typename _Func, typename... _Args>
        inline static auto make_task_item(const std::string &name, task_priority priority, _Func &&func, _Args &&...args)
        {
            return self_type::make_task_item<_DefaultTaskContext>(name, priority, std::forward<_Func>(func), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : fire_and_forget
        // Context  : user defined
        // Priority : default
        template<typename _TaskContext, typename _Func, typename... _Args>
        inline static auto make_task_item(const std::string &name, _Func &&func, _Args &&...args)
        {
            return self_type::make_task_item<_TaskContext, _Func>(name, default_priority, std::forward<_Func>(func), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------
        // Type     : fire_and_forget
        // Context  : default
        // Priority : default
        template<typename _Func, typename... _Args>
        inline static auto make_task_item(const std::string &name, _Func &&func, _Args &&...args)
        {
            return self_type::make_task_item(name, default_priority, std::forward<_Func>(func), std::forward<_Args>(args)...);
        }
        //------------------------------------------------------------------------------------------



        
        //------------------------------------------------------------------------------------------
        // Create a parallel group, the group is NOT added to the scheduler
        //
        // Type     : user defined
        // Context  : user defined
        template<task_type _TaskType, typename _GroupContext>
        inline static auto make_parallel_group(const std::string &name, task_priority prio = default_priority, int32_t taskCount = 0, int32_t maxSimultaneousTasks = 0)
        {
            return oqpi::make_parallel_group<_TaskType, _GroupContext>(scheduler_, name, prio, taskCount, maxSimultaneousTasks);
        }
        //------------------------------------------------------------------------------------------
        // Type     : user defined
        // Context  : default
        template<task_type _TaskType>
        inline static auto make_parallel_group(const std::string &name, task_priority prio = default_priority, int32_t taskCount = 0, int32_t maxSimultaneousTasks = 0)
        {
            return self_type::make_parallel_group<_TaskType, _DefaultGroupContext>(name, prio, taskCount, maxSimultaneousTasks);
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Creates a sequence of tasks, the group is NOT added to the scheduler
        //
        // Type     : user defined
        // Context  : user defined
        template<task_type _TaskType, typename _GroupContext>
        inline static auto make_sequence_group(const std::string &name, task_priority prio = default_priority)
        {
            return oqpi::make_sequence_group<_TaskType, _GroupContext>(scheduler_, name, prio);
        }
        //------------------------------------------------------------------------------------------
        // Type     : user defined
        // Context  : default
        template<task_type _TaskType>
        inline static auto make_sequence_group(const std::string &name, task_priority prio = default_priority)
        {
            return self_type::make_sequence_group<_TaskType, _DefaultGroupContext>(name, prio);
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Group Context    : user defined
        // Task Context     : user defined
        template<task_type _TaskType, typename _GroupContext, typename _TaskContext, typename _Func, typename _Partitioner>
        inline static auto make_parallel_for_task_group(const std::string &name, const _Partitioner &partitioner, task_priority prio, _Func &&func)
        {
            return oqpi::make_parallel_for_task_group<_TaskType, _EventType, _GroupContext, _TaskContext>(scheduler_, name, partitioner, prio, std::forward<_Func>(func));
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : default
        // Task Context     : default
        template<task_type _TaskType, typename _Func, typename _Partitioner>
        inline static auto make_parallel_for_task_group(const std::string &name, const _Partitioner &partitioner, task_priority prio, _Func &&func)
        {
            return self_type::make_parallel_for_task_group<_TaskType, _DefaultGroupContext, _DefaultTaskContext>(name, partitioner, prio, std::forward<_Func>(func));
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Group Context    : user defined
        // Task Context     : user defined
        // Partitioner      : user defined
        // Priority         : user defined
        template<typename _GroupContext, typename _TaskContext, typename _Func, typename _Partitioner>
        inline static void parallel_for(const std::string &name, const _Partitioner &partitioner, task_priority prio, _Func &&func)
        {
            oqpi::parallel_for<_EventType, _GroupContext, _TaskContext>(scheduler_, name, partitioner, prio, std::forward<_Func>(func));
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : user defined
        // Task Context     : user defined
        // Partitioner      : simple_partitioner
        // Priority         : normal
        template<typename _GroupContext, typename _TaskContext, typename _Func>
        inline static void parallel_for(const std::string &name, int32_t firstIndex, int32_t lastIndex, _Func &&func)
        {
            const auto priority     = default_priority;
            const auto partitioner  = oqpi::simple_partitioner(firstIndex, lastIndex, scheduler_.workersCount(priority));
            self_type::parallel_for<_GroupContext, _TaskContext>(name, partitioner, priority, std::forward<_Func>(func));
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : user defined
        // Task Context     : user defined
        // Partitioner      : simple_partitioner
        // Priority         : normal
        template<typename _GroupContext, typename _TaskContext, typename _Func>
        inline static void parallel_for(const std::string &name, int32_t elementCount, _Func &&func)
        {
            self_type::parallel_for<_GroupContext, _TaskContext>(name, 0, elementCount, std::forward<_Func>(func));
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : default
        // Task Context     : default
        // Partitioner      : user defined
        // Priority         : user defined
        template<typename _Func, typename _Partitioner>
        inline static void parallel_for(const std::string &name, const _Partitioner &partitioner, task_priority prio, _Func &&func)
        {
            self_type::parallel_for<_DefaultGroupContext, _DefaultTaskContext>(name, partitioner, prio, std::forward<_Func>(func));
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : default
        // Task Context     : default
        // Partitioner      : simple_partitioner
        // Priority         : normal
        template<typename _Func>
        inline static void parallel_for(const std::string &name, int32_t firstIndex, int32_t lastIndex, _Func &&func)
        {
            self_type::parallel_for<_DefaultGroupContext, _DefaultTaskContext>(name, firstIndex, lastIndex, std::forward<_Func>(func));
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : default
        // Task Context     : default
        // Partitioner      : simple_partitioner
        // Priority         : normal
        template<typename _Func>
        inline static void parallel_for(const std::string &name, int32_t elementCount, _Func &&func)
        {
            self_type::parallel_for(name, 0, elementCount, std::forward<_Func>(func));
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Group Context    : user defined
        // Task Context     : user defined
        // Partitioner      : user defined
        // Priority         : user defined
        template<typename _GroupContext, typename _TaskContext, typename _Func, typename _Container, typename _Partitioner>
        inline static void parallel_for_each(const std::string &name, _Container &container, const _Partitioner &partitioner, task_priority prio, _Func &&func)
        {
            self_type::parallel_for<_GroupContext, _TaskContext>(name, partitioner, prio,
                [&container, func = std::forward<_Func>(func)](int32_t elementIndex)
            {
                func(container[elementIndex]);
            });
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : default
        // Task Context     : default
        // Partitioner      : user defined
        // Priority         : user defined
        template<typename _Func, typename _Container, typename _Partitioner>
        inline static void parallel_for_each(const std::string &name, _Container &container, const _Partitioner &partitioner, task_priority prio, _Func &&func)
        {
            self_type::parallel_for_each<_DefaultGroupContext, _DefaultTaskContext>(name, container, partitioner, prio, std::forward<_Func>(func));
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : default
        // Task Context     : default
        // Partitioner      : simple_partitioner
        // Priority         : normal
        template<typename _Func, typename _Container>
        inline static void parallel_for_each(const std::string &name, _Container &container, _Func &&func)
        {
            self_type::parallel_for(name, 0, int32_t(container.size()),
                [&container, func = std::forward<_Func>(func)](int32_t elementIndex)
            {
                func(container[elementIndex]);
            });
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Creates a sequence of tasks and schedule it right away
        //
        // Group Context    : user defined
        // Priority         : user defined
        template<task_type _TaskType, typename _GroupContext, typename... _TaskHandles>
        inline static auto sequence_tasks(const std::string &name, task_priority prio, _TaskHandles &&...taskHandles)
        {
            auto spSequence = self_type::make_sequence_group<_TaskType, _GroupContext>(name, prio);
            self_type::add_to_group(spSequence, std::forward<_TaskHandles>(taskHandles)...);
            return self_type::schedule_task(task_handle(std::move(spSequence)));
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : default
        // Priority         : user defined
        template<task_type _TaskType, typename... _TaskHandles>
        inline static auto sequence_tasks(const std::string &name, task_priority prio, _TaskHandles &&...taskHandles)
        {
            return self_type::sequence_tasks<_TaskType, _DefaultGroupContext>(name, prio, std::forward<_TaskHandles>(taskHandles)...);
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : user defined
        // Priority         : default
        template<task_type _TaskType, typename _GroupContext, typename... _TaskHandles>
        inline static auto sequence_tasks(const std::string &name, _TaskHandles &&...taskHandles)
        {
            return self_type::sequence_tasks<_TaskType, _GroupContext>(name, default_priority, std::forward<_TaskHandles>(taskHandles)...);
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : default
        // Priority         : default
        template<task_type _TaskType, typename... _TaskHandles>
        inline static auto sequence_tasks(const std::string &name, _TaskHandles &&...taskHandles)
        {
            return self_type::sequence_tasks<_TaskType>(name, default_priority, std::forward<_TaskHandles>(taskHandles)...);
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Creates a fork of tasks (parallele_group) and schedules it right away
        //
        // Group Context    : user defined
        // Priority         : user defined
        template<task_type _TaskType, typename _GroupContext, typename... _TaskHandles>
        inline static auto fork_tasks(const std::string &name, task_priority prio, _TaskHandles &&...taskHandles)
        {
            auto spFork = self_type::make_parallel_group<_TaskType, _GroupContext>(name, prio, sizeof...(taskHandles));
            self_type::add_to_group(spFork, std::forward<_TaskHandles>(taskHandles)...);
            return self_type::schedule_task(task_handle(std::move(spFork)));
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : default
        // Priority         : user defined
        template<task_type _TaskType, typename... _TaskHandles>
        inline static auto fork_tasks(const std::string &name, task_priority prio, _TaskHandles &&...taskHandles)
        {
            return self_type::fork_tasks<_TaskType, _DefaultGroupContext>(name, prio, std::forward<_TaskHandles>(taskHandles)...);
        }
        //------------------------------------------------------------------------------------------
        // Group Context    : default
        // Priority         : default
        template<task_type _TaskType, typename... _TaskHandles>
        inline static auto fork_tasks(const std::string &name, _TaskHandles &&...taskHandles)
        {
            return self_type::fork_tasks<_TaskType, _DefaultGroupContext>(name, default_priority, std::forward<_TaskHandles>(taskHandles)...);
        }
        //------------------------------------------------------------------------------------------


    private:
        //------------------------------------------------------------------------------------------
        // Recursively adds tasks to a group
        template<typename _GroupType, typename... _TaskHandles>
        inline static void add_to_group(const _GroupType &spGroup, task_handle &&taskHandle, _TaskHandles &&...taskHandles)
        {
            add_to_group(spGroup, std::forward<task_handle>(taskHandle));
            add_to_group(spGroup, std::forward<_TaskHandles>(taskHandles)...);
        }
        //------------------------------------------------------------------------------------------
        template<typename _GroupType>
        inline static void add_to_group(const _GroupType &spGroup, task_handle &&taskHandle)
        {
            spGroup->addTask(std::forward<task_handle>(taskHandle));
        }
    };

    //----------------------------------------------------------------------------------------------
    template<typename _Scheduler, typename _DefaultGroupContext, typename _DefaultTaskContext, typename _EventType>
    _Scheduler helpers<_Scheduler, _DefaultGroupContext, _DefaultTaskContext, _EventType>::scheduler_;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    using default_helpers = helpers<>;
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
