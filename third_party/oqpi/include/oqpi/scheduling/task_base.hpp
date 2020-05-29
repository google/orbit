#pragma once

#include <atomic>
#include <memory>
#include <string>
#include "oqpi/scheduling/task_type.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    class task_group_base;
    //----------------------------------------------------------------------------------------------
    class task_base;
    //----------------------------------------------------------------------------------------------
    using task_group_sptr = std::shared_ptr<task_group_base>;
    //----------------------------------------------------------------------------------------------
    using task_uptr = std::unique_ptr<task_base>;
    using task_sptr = std::shared_ptr<task_base>;
    using task_wptr = std::weak_ptr<task_base>;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    // Base class for all kind of tasks, unit tasks as well as groups
    class task_base
    {
    public:
        //------------------------------------------------------------------------------------------
        // Constructor
        task_base(task_priority priority)
            : uid_(uid_provider())
            , spParentGroup_(nullptr)
            , priority_(priority)
            , grabbed_(false)
            , done_(false)
        {}

        //------------------------------------------------------------------------------------------
        // Used as a base class for the whole task hierarchy
        virtual ~task_base() = default;

        //------------------------------------------------------------------------------------------
        // Can be moved
        task_base(task_base &&other)
            : uid_(other.uid_)
            , spParentGroup_(std::move(other.spParentGroup_))
            , priority_(other.priority_)
            , grabbed_(other.grabbed_.load())
            , done_(other.done_.load())
        {}

        //------------------------------------------------------------------------------------------
        task_base& operator =(task_base &&rhs)
        {
            if (this != &rhs)
            {
                uid_            = rhs.uid_;
                spParentGroup_  = std::move(rhs.spParentGroup_);
                priority_       = rhs.priority_;
                grabbed_        = rhs.grabbed_.load();
                done_           = rhs.done_.load();

                rhs.uid_        = invalid_task_uid;
            }
            return (*this);
        }

        //------------------------------------------------------------------------------------------
        // Copy disabled
        task_base(const task_base &)                = delete;
        task_base& operator =(const task_base &)    = delete;

    public:
        //------------------------------------------------------------------------------------------
        // Interface
        virtual void execute()                  = 0;
        virtual void executeSingleThreaded()    = 0;
        virtual void wait() const               = 0;
        virtual void activeWait()               = 0;

    protected:
        virtual void onParentGroupSet()         = 0;

    public:
        //------------------------------------------------------------------------------------------
        // Accessors
        inline task_uid getUID() const
        {
            return uid_;
        }

        inline void setParentGroup(const task_group_sptr &spParentGroup)
        {
            spParentGroup_ = spParentGroup;
            onParentGroupSet();
        }

        inline const task_group_sptr& getParentGroup() const
        {
            return spParentGroup_;
        }

        inline task_priority getPriority() const
        {
            return priority_;
        }

        inline bool tryGrab()
        {
            bool expected = false;
            return grabbed_.compare_exchange_strong(expected, true);
        }

        inline bool isGrabbed() const
        {
            return grabbed_.load();
        }

        inline bool isDone() const
        {
            return done_.load();
        }

        inline void setDone()
        {
            done_.store(true);
        }

        inline void notifyParent();

    protected:
        //------------------------------------------------------------------------------------------
        // The unique id of this task
        task_uid            uid_;
        // Optional parent group
        task_group_sptr     spParentGroup_;
        // Relative priority of the task
        task_priority       priority_;
        // Token that has to be acquired by anyone before executing the task
        std::atomic<bool>   grabbed_;
        // Flag flipped once the task execution is done
        std::atomic<bool>   done_;

    private:
        //------------------------------------------------------------------------------------------
        static inline task_uid uid_provider()
        {
            // Starts at 1 as 0 is the invalid value
            static std::atomic<task_uid> uid_generator(1);
            return uid_generator.fetch_add(1);
        }
    };

} /*oqpi*/
