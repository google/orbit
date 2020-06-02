#pragma once


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    // Helper class containing an arbitrary result of a task.
    // _ReturnType must be default constructible and copyable or movable.
    template<typename _ReturnType>
    class task_result
    {
    protected:
        task_result() = default;

    protected:
        template<typename _Func>
        void run(_Func &&f)
        {
            result_ = f();
        }

        _ReturnType getResult() const
        {
            return result_;
        }

    protected:
        _ReturnType result_;
    };
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    // Specialization for tasks returning void
    template<>
    class task_result<void>
    {
    protected:
        template<typename _Func>
        void run(_Func &&f)
        {
            f();
        }

        void getResult() const {}
    };
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
