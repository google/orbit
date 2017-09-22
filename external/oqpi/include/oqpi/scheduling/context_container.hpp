#pragma once

#include "oqpi/platform.hpp"

#if OQPI_PLATFORM_WIN
#   pragma warning (push)
#   pragma warning (disable : 4100)
// https://connect.microsoft.com/VisualStudio/feedback/details/1031958/unreferenced-formal-parameter-warning-c4100-checks-code-after-variadics-expansion
#endif


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    class task_base;
    //----------------------------------------------------------------------------------------------
    class task_handle;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    template<typename _OwnerType, typename... _ContextList>
    class context_container
        : public _ContextList...
    {
    public:
        using self_type = context_container<_OwnerType, _ContextList...>;

    public:
        template<typename... _Args>
        context_container(_OwnerType *pOwner, _Args &&...args)
            : _ContextList(pOwner, std::forward<_Args>(args)...)...
        {}

        //------------------------------------------------------------------------------------------
        // Worker context
        inline void worker_onStart()
        {
            worker_on_start<self_type, _ContextList...>(*this);
        }

        inline void worker_onStop()
        {
            worker_on_stop<self_type, _ContextList...>(*this);
        }

        inline void worker_onIdle()
        {
            worker_on_idle<self_type, _ContextList...>(*this);
        }

        inline void worker_onActive()
        {
            worker_on_active<self_type, _ContextList...>(*this);
        }

        inline void worker_onPreExecute(const task_handle &hTask)
        {
            worker_on_pre_execute<self_type, _ContextList...>(*this, hTask);
        }

        inline void worker_onPostExecute(const task_handle &hTask)
        {
            worker_on_post_execute<self_type, _ContextList...>(*this, hTask);
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Group context
        inline void group_onAddedToGroup(const task_group_sptr &spParentGroup)
        {
            group_on_added_to_group<self_type, _ContextList...>(*this, spParentGroup);
        }

        inline void group_onTaskAdded(const task_handle &hTask)
        {
            group_on_task_added<self_type, _ContextList...>(*this, hTask);
        }

        inline void group_onPreExecute()
        {
            group_on_pre_execute<self_type, _ContextList...>(*this);
        }

        inline void group_onPostExecute()
        {
            group_on_post_execute<self_type, _ContextList...>(*this);
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Task context
        inline void task_onAddedToGroup(const task_group_sptr &spParentGroup)
        {
            task_on_added_to_group<self_type, _ContextList...>(*this, spParentGroup);
        }

        inline void task_onPreExecute()
        {
            task_on_pre_execute<self_type, _ContextList...>(*this);
        }

        inline void task_onPostExecute()
        {
            task_on_post_execute<self_type, _ContextList...>(*this);
        }
        //------------------------------------------------------------------------------------------

    private:
        //------------------------------------------------------------------------------------------
        // Worker Context
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void worker_on_start(_ContextContainer &contextContainer)
        {
            contextContainer._Context::onStart();
            worker_on_start<_ContextContainer, _TContextList...>(contextContainer);
        }
        template<typename _ContextContainer> inline static void worker_on_start(_ContextContainer &) {} // EoR (End of Recursion)
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void worker_on_stop(_ContextContainer &contextContainer)
        {
            contextContainer._Context::onStop();
            worker_on_stop<_ContextContainer, _TContextList...>(contextContainer);
        }
        template<typename _ContextContainer> inline static void worker_on_stop(_ContextContainer &) {} // EoR
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void worker_on_idle(_ContextContainer &contextContainer)
        {
            contextContainer._Context::onIdle();
            worker_on_idle<_ContextContainer, _TContextList...>(contextContainer);
        }
        template<typename _ContextContainer> inline static void worker_on_idle(_ContextContainer &) {} // EoR
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void worker_on_active(_ContextContainer &contextContainer)
        {
            contextContainer._Context::onActive();
            worker_on_active<_ContextContainer, _TContextList...>(contextContainer);
        }
        template<typename _ContextContainer> inline static void worker_on_active(_ContextContainer &) {} // EoR
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void worker_on_pre_execute(_ContextContainer &contextContainer, const task_handle &hTask)
        {
            contextContainer._Context::onPreExecute(hTask);
            worker_on_pre_execute<_ContextContainer, _TContextList...>(contextContainer, hTask);
        }
        template<typename _ContextContainer> inline static void worker_on_pre_execute(_ContextContainer &, const task_handle &) {} // EoR
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void worker_on_post_execute(_ContextContainer &contextContainer, const task_handle &hTask)
        {
            contextContainer._Context::onPostExecute(hTask);
            worker_on_post_execute<_ContextContainer, _TContextList...>(contextContainer, hTask);
        }
        template<typename _ContextContainer> inline static void worker_on_post_execute(_ContextContainer &, const task_handle &) {} // EoR
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Group Context
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void group_on_added_to_group(_ContextContainer &contextContainer, const task_group_sptr &spParentGroup)
        {
            contextContainer._Context::onAddedToGroup(spParentGroup);
            group_on_added_to_group<_ContextContainer, _TContextList...>(contextContainer, spParentGroup);
        }
        template<typename _ContextContainer> inline static void group_on_added_to_group(_ContextContainer &, const task_group_sptr &) {} // EoR
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void group_on_task_added(_ContextContainer &contextContainer, const task_handle &hTask)
        {
            contextContainer._Context::onTaskAdded(hTask);
            group_on_task_added<_ContextContainer, _TContextList...>(contextContainer, hTask);
        }
        template<typename _ContextContainer> inline static void group_on_task_added(_ContextContainer &, const task_handle &) {} // EoR
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void group_on_pre_execute(_ContextContainer &contextContainer)
        {
            contextContainer._Context::onPreExecute();
            group_on_pre_execute<_ContextContainer, _TContextList...>(contextContainer);
        }
        template<typename _ContextContainer> inline static void group_on_pre_execute(_ContextContainer &) {} // EoR
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void group_on_post_execute(_ContextContainer &contextContainer)
        {
            contextContainer._Context::onPostExecute();
            group_on_post_execute<_ContextContainer, _TContextList...>(contextContainer);
        }
        template<typename _ContextContainer> inline static void group_on_post_execute(_ContextContainer &) {} // EoR
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        // Task Context
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void task_on_added_to_group(_ContextContainer &contextContainer, const task_group_sptr &spParentGroup)
        {
            contextContainer._Context::onAddedToGroup(spParentGroup);
            task_on_added_to_group<_ContextContainer, _TContextList...>(contextContainer, spParentGroup);
        }
        template<typename _ContextContainer> inline static void task_on_added_to_group(_ContextContainer &, const task_group_sptr &) {}
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void task_on_pre_execute(_ContextContainer &contextContainer)
        {
            contextContainer._Context::onPreExecute();
            task_on_pre_execute<_ContextContainer, _TContextList...>(contextContainer);
        }
        template<typename _ContextContainer> inline static void task_on_pre_execute(_ContextContainer &) {} // EoR
        //------------------------------------------------------------------------------------------
        template<typename _ContextContainer, typename _Context, typename... _TContextList>
        inline static void task_on_post_execute(_ContextContainer &contextContainer)
        {
            contextContainer._Context::onPostExecute();
            task_on_post_execute<_ContextContainer, _TContextList...>(contextContainer);
        }
        template<typename _ContextContainer> inline static void task_on_post_execute(_ContextContainer &) {} // EoR
        //------------------------------------------------------------------------------------------
    };
    //----------------------------------------------------------------------------------------------

} /*oqpi*/

#if OQPI_PLATFORM_WIN
#   pragma warning (pop)
#endif
