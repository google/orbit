#pragma once

#include "oqpi/scheduling/context_container.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    class worker_base;
    class task_handle;
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    // Optional base class for worker contexts, should be inherited from virtually
    //
    class worker_context_base
    {
    public:
        worker_context_base(worker_base *pOwner)
            : pOwner_(pOwner)
        {}

    public:
        inline worker_base* owner() const { return pOwner_; }

    public:
        inline void onStart()                           {}
        inline void onStop()                            {}
        inline void onIdle()                            {}
        inline void onActive()                          {}
        inline void onPreExecute(const task_handle &)   {}
        inline void onPostExecute(const task_handle &)  {}

    private:
        worker_base *pOwner_;
    };
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    template<typename... _ContextList>
    using worker_context_container = context_container<worker_base, _ContextList...>;
    //----------------------------------------------------------------------------------------------
    using empty_worker_context = worker_context_container<>;
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
