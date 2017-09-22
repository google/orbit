#pragma once

#include "oqpi/error_handling.hpp"
#include "oqpi/scheduling/context_container.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    class task_group_base;
    class task_handle;
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    // Optional base class for group contexts, should be inherited from virtually
    //
    class group_context_base
    {
    public:
        group_context_base(task_group_base *pOwner, std::string name)
            : pOwner_(pOwner)
        {}

        task_group_base* owner() const
        {
            oqpi_check(pOwner_);
            return pOwner_;
        }

        inline void onAddedToGroup(const task_group_sptr &) {};
        inline void onTaskAdded(const task_handle &) {}
        inline void onPreExecute()  {}
        inline void onPostExecute() {}

    private:
        task_group_base *pOwner_;
    };
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    template<typename... _ContextList>
    using group_context_container = context_container<task_group_base, _ContextList...>;
    //----------------------------------------------------------------------------------------------
    using empty_group_context = group_context_container<>;
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
