#pragma once

#include "oqpi/platform.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    // Forward declaration of this platform semaphore implementation
    using semaphore_impl = class win_semaphore;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    class win_semaphore
    {
    protected:
        explicit win_semaphore(int32_t initCount, int32_t maxCount, const std::string &)
            : initCount_(initCount)
            , maxCount_(maxCount)
            , handle_(nullptr)
        {
            handle_ = CreateSemaphoreA(nullptr, initCount_, maxCount_, nullptr);

            oqpi_check(handle_ != nullptr);
            oqpi_check(GetLastError() != ERROR_ALREADY_EXISTS);
        }

        ~win_semaphore()
        {
            if (handle_)
            {
                CloseHandle(handle_);
                handle_ = nullptr;
            }
        }

        win_semaphore(win_semaphore &&other)
            : handle_(other.handle_)
        {
            other.handle_ = nullptr;
        }

        win_semaphore& operator =(win_semaphore &&rhs)
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
        void notify(int32_t count)
        {
            auto previousCount = LONG{ 0 };
            oqpi_verify(ReleaseSemaphore(handle_, LONG{ count }, &previousCount) != 0);
        }

        void notifyAll()
        {
            notify(maxCount_);
        }

        bool tryWait()
        {
            return internalWait(0, TRUE);
        }

        void wait() const
        {
            internalWait(INFINITE, TRUE);
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
        win_semaphore(const win_semaphore &)                = delete;
        win_semaphore& operator =(const win_semaphore &)    = delete;

    private:
        LONG    initCount_;
        LONG    maxCount_;
        HANDLE  handle_;
    };
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
