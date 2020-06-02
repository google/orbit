#pragma once

#include "oqpi/platform.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    // Forward declaration of this platform semaphore implementation
    using event_manual_reset_policy_impl = struct win_event_manual_reset_policy;
    //----------------------------------------------------------------------------------------------
    template<typename _ResetPolicy> class win_event;
    //----------------------------------------------------------------------------------------------
    template<typename _ResetPolicy>
    using event_impl = class win_event<_ResetPolicy>;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    template<typename _ResetPolicy>
    class win_event
        : public _ResetPolicy
    {
    protected:
        explicit win_event(const std::string &)
            : handle_(nullptr)
        {
            const auto bManualReset  = BOOL{ _ResetPolicy::is_manual_reset_enabled() };
            const auto bInitialState = BOOL{ FALSE };
            handle_ = CreateEventA(nullptr, bManualReset, bInitialState, nullptr);

            oqpi_check(handle_ != nullptr);
            oqpi_check(GetLastError() != ERROR_ALREADY_EXISTS);
        }

        win_event(win_event &&rhs)
            : handle_(rhs.handle_)
        {
            rhs.handle_ = nullptr;
        }

        ~win_event()
        {
            if (handle_)
            {
                CloseHandle(handle_);
                handle_ = nullptr;
            }
        }

        win_event& operator =(win_event &&rhs)
        {
            if (this != &rhs)
            {
                handle_ = rhs.handle_;
                rhs.handle_ = nullptr;
            }
            return (*this);
        }

    protected:
        // User interface
        void notify()
        {
            oqpi_verify(SetEvent(handle_) == TRUE);
        }

        void wait() const
        {
            internalWait(INFINITE, TRUE);
        }

        void reset()
        {
            _ResetPolicy::reset(handle_);
        }

        template<typename _Rep, typename _Period>
        bool waitFor(const std::chrono::duration<_Rep, _Period>& relTime) const
        {
            const auto dwMilliseconds = DWORD(std::chrono::duration_cast<std::chrono::milliseconds>(relTime).count());
            return internalWait(dwMilliseconds, TRUE);
        }

    private:
        bool internalWait(DWORD dwMilliseconds, BOOL bAlertable) const
        {
            const auto result = WaitForSingleObjectEx(handle_, dwMilliseconds, bAlertable);
            if (oqpi_failed(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT))
            {
                oqpi_error("WaitForSingleObjectEx failed with error code 0x%x", GetLastError());
            }
            return (result == WAIT_OBJECT_0);
        }

    private:
        // Not copyable
        win_event(const win_event &)                = delete;
        win_event& operator =(const win_event &)    = delete;

    private:
        HANDLE handle_;
    };
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    struct win_event_manual_reset_policy
    {
        static bool is_manual_reset_enabled()
        {
            return true;
        }

        void reset(HANDLE &handle)
        {
            oqpi_verify(ResetEvent(handle) != FALSE);
        }
    };
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
