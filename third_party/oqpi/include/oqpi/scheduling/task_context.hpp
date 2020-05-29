#pragma once

#include "oqpi/scheduling/context_container.hpp"
#include "oqpi/scheduling/task_base.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    class task_base;
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    // Optional base class for task contexts, should be inherited from virtually
    //
    class task_context_base
    {
    public:
        task_context_base(task_base *pOwner, const std::string &)
            : pOwner_(pOwner)
        {}

    public:
        inline task_base* owner() const { return pOwner_; }

    public:
        inline void onAddedToGroup(const task_group_sptr &) {}
        inline void onPreExecute() {}
        inline void onPostExecute() {}

    private:
        task_base *pOwner_;
    };
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    template<typename... _ContextList>
    using task_context_container = context_container<task_base, _ContextList...>;
    //----------------------------------------------------------------------------------------------
    using empty_task_context = task_context_container<>;
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
