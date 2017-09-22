#pragma once

#include "oqpi/synchronization/event.hpp"
#include "oqpi/scheduling/task_type.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    // This class is specialized for each task type.
    // It provides 2 functions: wait() and notify().
    // Inheriting from the right specialization will ensure that only waitable tasks instantiate
    // a synchronization object.
    template<task_type _TaskType, typename _EventType = manual_reset_event_interface<>>
    class notifier;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    // Nothing to notify or wait for, for a fire_and_forget task
    template<typename _EventType>
    class notifier<task_type::fire_and_forget, _EventType>
    {
    protected:
        //------------------------------------------------------------------------------------------
        notifier(task_uid) {};

        //------------------------------------------------------------------------------------------
        void wait() const { oqpi_checkf(false, "Can't wait on a fire_and_forget task"); }
        void notify() {}
    };
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    // Waitable tasks have a manual reset event to notify/wait on
    template<typename _EventType>
    class notifier<task_type::waitable, _EventType>
    {
        //------------------------------------------------------------------------------------------
        using self_type = notifier<task_type::waitable, _EventType>;

    protected:
        //------------------------------------------------------------------------------------------
        notifier(task_uid uid)
            : event_("Notifier/" + std::to_string(uid))
        {}

        //------------------------------------------------------------------------------------------
        // Movable
        notifier(self_type &&other)
            : event_(std::move(other.event_))
        {}
        //------------------------------------------------------------------------------------------
        self_type& operator =(self_type &&rhs)
        {
            if (this != &rhs)
            {
                event_ = std::move(rhs.event_);
            }
            return (*this);
        }

        //------------------------------------------------------------------------------------------
        void wait() const   { event_.wait();    }
        void notify()       { event_.notify();  }

    private:
        //------------------------------------------------------------------------------------------
        notifier(const self_type &rhs)              = delete;
        self_type& operator =(const self_type &rhs) = delete;

    private:
        // Done event
        _EventType event_;
    };
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
