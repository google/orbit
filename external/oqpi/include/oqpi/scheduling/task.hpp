#pragma once

#include "oqpi/scheduling/task_base.hpp"
#include "oqpi/scheduling/task_result.hpp"
#include "oqpi/scheduling/task_notifier.hpp"


namespace oqpi {

    template<task_type _TaskType, typename _EventType, typename _TaskContext, typename _Func>
    class task final
        : public task_base
        , public task_result<typename std::result_of<_Func()>::type>
        , public _TaskContext
        , public notifier<_TaskType, _EventType>
    {
        //------------------------------------------------------------------------------------------
        using self_type         = task<_TaskType, _EventType, _TaskContext, _Func>;
        using return_type       = typename std::result_of<_Func()>::type;
        using task_result_type  = task_result<return_type>;
        using notifier_type     = notifier<_TaskType, _EventType>;

    public:
        //------------------------------------------------------------------------------------------
        task(const std::string &name, task_priority priority, _Func func)
            : task_base(priority)
            , _TaskContext(this, name)
            , notifier_type(task_base::getUID())
            , func_(std::move(func))
        {}

        //------------------------------------------------------------------------------------------
        // Movable
        task(self_type &&other)
            : task_base(std::move(other))
            , _TaskContext(std::move(other))
            , notifier_type(std::move(other))
            , func_(std::move(other.func_))
        {}

        //------------------------------------------------------------------------------------------
        self_type& operator =(self_type &&rhs)
        {
            if (this != &rhs)
            {
                task_base::operator =(std::move(rhs));
                _TaskContext::operator =(std::move(rhs));
                notifier_type::operator =(std::move(rhs));
                func_ = std::move(rhs.func_);
            }
            return (*this);
        }
        
        //------------------------------------------------------------------------------------------
        // Not copyable
        task(const self_type &)                     = delete;
        self_type& operator =(const self_type &)    = delete;

    public:
        //------------------------------------------------------------------------------------------
        virtual void execute() override final
        {
            if (oqpi_ensuref(task_base::isGrabbed(), "Trying to execute an ungrabbed task: %d", task_base::getUID()))
            {
                invoke();
                task_base::notifyParent();
            }
        }

        //------------------------------------------------------------------------------------------
        virtual void executeSingleThreaded() override final
        {
            if (task_base::tryGrab())
            {
                invoke();
                // We are single threaded meaning that our parent (if any) is running this task
                // in its executeSingleThreaded function, so no need to notify it.
            }
        }

        //------------------------------------------------------------------------------------------
        virtual void wait() const override final
        {
            notifier_type::wait();
        }

        //------------------------------------------------------------------------------------------
        virtual void activeWait() override final
        {
            if (task_base::tryGrab())
            {
                execute();
            }
            else
            {
                wait();
            }
        }

        //------------------------------------------------------------------------------------------
        virtual void onParentGroupSet() override final
        {
            _TaskContext::task_onAddedToGroup(this->spParentGroup_);
        }

        //------------------------------------------------------------------------------------------
        return_type getResult() const
        {
            oqpi_checkf(task_base::isDone(), "Trying to get the result of an unfinished task: %d", task_base::getUID());
            return task_result_type::getResult();
        }

        //------------------------------------------------------------------------------------------
        return_type waitForResult() const
        {
            wait();
            return getResult();
        }

    private:
        //------------------------------------------------------------------------------------------
        inline void invoke()
        {
            // Run the preExecute code of the context
            _TaskContext::task_onPreExecute();
            // Run the task itself
            task_result_type::run(func_);
            // Flag the task as done
            task_base::setDone();
            // Run the postExecute code of the context
            _TaskContext::task_onPostExecute();
            // Signal that the task is done
            notifier_type::notify();
        }

    private:
        _Func func_;
    };
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    // Type     : user defined
    // Context  : user defined
    template<task_type _TaskType, typename _EventType, typename _TaskContext, typename _Func, typename... _Args>
    inline auto make_task(const std::string &name, task_priority priority, _Func &&func, _Args &&...args)
    {
        const auto f = [func = std::forward<_Func>(func), &args...]
        {
            return func(std::forward<_Args>(args)...);
        };

        using task_type = task<_TaskType, _EventType, _TaskContext, std::decay_t<decltype(f)>>;
        return std::make_shared<task_type>
        (
            name,
            priority,
            std::move(f)
        );
    }
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
