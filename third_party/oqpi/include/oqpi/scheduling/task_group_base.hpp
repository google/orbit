#pragma once

#include <memory>
#include "oqpi/scheduling/task_base.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    class task_handle;
    //----------------------------------------------------------------------------------------------
    class task_group_base;
    //----------------------------------------------------------------------------------------------
    using task_group_sptr = std::shared_ptr<task_group_base>;
    using task_group_wptr = std::weak_ptr<task_group_base>;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    class task_group_base
        : public task_base
        , public std::enable_shared_from_this<task_group_base>
    {
    public:
        task_group_base(task_priority priority)
            : task_base(priority)
        {}

        virtual ~task_group_base() = default;

    public:
        virtual void addTask(task_handle hTask) = 0;
        virtual void oneTaskDone()              = 0;
        virtual bool empty()        const       = 0;
    };
    //----------------------------------------------------------------------------------------------
    

    //----------------------------------------------------------------------------------------------
    // Declared here as a workaround to the circular dependency between task_base and
    // task_group_base.
    inline void task_base::notifyParent()
    {
        if (spParentGroup_)
        {
            spParentGroup_->oneTaskDone();
            spParentGroup_.reset();
        }
    }
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
