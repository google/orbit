#pragma once

#include <chrono>
#include "oqpi/platform.hpp"
#include "oqpi/error_handling.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    // Type definition of this platform thread implementation, this is needed by interface::thread
    // to be able to select the right implementation.
    using thread_impl = class win_thread;
    //----------------------------------------------------------------------------------------------
    class win_thread
    {
    protected:
        //------------------------------------------------------------------------------------------
        static uint32_t hardware_concurrency()
        {
            static const auto logicalProcessorCount = uint32_t(GetActiveProcessorCount(ALL_PROCESSOR_GROUPS));
            return logicalProcessorCount;
        }

    protected:
        //------------------------------------------------------------------------------------------
        // Default constructible
        win_thread() noexcept
            : handle_(nullptr)
            , id_(0)
        {}
        //------------------------------------------------------------------------------------------
        ~win_thread()
        {
            // The interface should have taken care of either joining or terminating the thread
            if (oqpi_failed(handle_ == nullptr && id_ == 0))
            {
                handle_ = nullptr;
                id_     = 0;
            }
        }
        //------------------------------------------------------------------------------------------

    protected:
        //------------------------------------------------------------------------------------------
        // Movable
        win_thread(win_thread &&other) noexcept
            : handle_(other.handle_)
            , id_(other.id_)
        {
            other.handle_   = nullptr;
            other.id_       = 0;
        }
        //------------------------------------------------------------------------------------------
        win_thread& operator =(win_thread &&rhs)
        {
            if (this != &rhs && oqpi_ensure(handle_ == nullptr && id_ == 0))
            {
                handle_     = rhs.handle_;
                id_         = rhs.id_;
                rhs.handle_ = nullptr;
                rhs.id_     = 0;
            }
            return (*this);
        }
        
    protected:
        //------------------------------------------------------------------------------------------
        template<typename _Launcher>
        bool create(const thread_attributes &attributes, void *pData)
        {
            // Specify if the handle can be inherited by child processes. NULL means it cannot.
            const auto lpThreadAttributes   = LPSECURITY_ATTRIBUTES{ nullptr };

            // Initial size of the thread's stack in bytes. 0 means use default
            const auto dwStackSize          = SIZE_T{ attributes.stackSize_ };

            // Thread function
            const auto lpStartAddress       = LPTHREAD_START_ROUTINE{ [](LPVOID pData) -> DWORD
            {
                auto upLauncher = std::unique_ptr<_Launcher>(static_cast<_Launcher*>(pData));
                if (oqpi_ensure(upLauncher != nullptr))
                {
                    (*upLauncher)();
                }
                return 0;
            } };

            // Data pointer
            const auto lpParameter          = LPVOID{ pData };

            // Creation flags
            const auto dwCreationFlags      = DWORD{ 0 };

            // This will receive the thread identifier
            const auto lpThreadId           = LPDWORD{ &id_ };
            
            handle_ = CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
            if (handle_ == nullptr)
            {
                oqpi_error("CreateThread failed with error code: %d", GetLastError());
                return false;
            }

            oqpi_check(id_ != 0ul);

            setPriority(attributes.priority_);
            setCoreAffinityMask(attributes.coreAffinityMask_);

            return true;
        }

    protected:
        //------------------------------------------------------------------------------------------
        // Platform specific types
        using native_handle_type    = HANDLE;
        using id                    = DWORD;

    protected:
        //------------------------------------------------------------------------------------------
        id                  getId()             const { return id_;                 }
        native_handle_type  getNativeHandle()   const { return handle_;             }
        bool                joinable()          const { return handle_ != nullptr;  }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        void join()
        {
            if (handle_ == nullptr)
            {
                oqpi_warning("Trying to join a non joinable thread.");
                return;
            }

            if (id_ == GetCurrentThreadId())
            {
                oqpi_warning("The current thread is the same as the thread attempted to join.");
                return;
            }

            const auto result = WaitForSingleObject(handle_, INFINITE);
            if (result != WAIT_FAILED)
            {
                handle_ = nullptr;
                id_     = 0;
            }
            else
            {
                oqpi_error("WaitForSingleObject failed with error code: %d", GetLastError());
            }
        }
        //------------------------------------------------------------------------------------------
        void detach()
        {
            if (handle_ == nullptr)
            {
                oqpi_warning("Trying to detach a non joinable thread.");
                return;
            }

            const auto result = CloseHandle(handle_);
            if (result == TRUE)
            {
                handle_ = nullptr;
                id_     = 0;
            }
            else
            {
                oqpi_error("CloseHandle failed with error code: %d", GetLastError());
            }
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        core_affinity getCoreAffinityMask() const
        {
            return get_core_affinity_mask(handle_);
        }
        //------------------------------------------------------------------------------------------
        void setCoreAffinityMask(core_affinity affinityMask)
        {
            set_core_affinity_mask(handle_, affinityMask);
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        thread_priority getPriority() const
        {
            return get_priority(handle_);
        }
        //------------------------------------------------------------------------------------------
        void setPriority(thread_priority priority)
        {
            set_priority(handle_, priority);
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        void setName(const char *name)
        {
            set_name(id_, name);
        }
        //------------------------------------------------------------------------------------------


    public:
        //------------------------------------------------------------------------------------------
        static void set_core_affinity_mask(native_handle_type handle, core_affinity affinityMask)
        {
            // Make sure the selected mask is valid
            oqpi_check(static_cast<uint64_t>(affinityMask) < (1ull << hardware_concurrency()) || affinityMask == core_affinity::all_cores);
            oqpi_verify(SetThreadAffinityMask(handle, static_cast<DWORD_PTR>(affinityMask)) != 0);
        }
        //------------------------------------------------------------------------------------------
        static core_affinity get_core_affinity_mask(native_handle_type handle)
        {
            // Nasty work-around as Win32 doesn't provide a GetThreadAffinityMask()
            auto testMask = DWORD_PTR{ 1 };
            auto dwPreviousAffinityMask = DWORD_PTR{ 0 };

            // Try every core one by one until one works or none are left
            // (usually the first one will work right away)
            while (testMask)
            {
                dwPreviousAffinityMask = SetThreadAffinityMask(handle, testMask);
                if (dwPreviousAffinityMask != 0)
                {
                    // Reset the previous value as if nothing happened
                    SetThreadAffinityMask(handle, dwPreviousAffinityMask);
                    break;
                }

                if (GetLastError() != ERROR_INVALID_PARAMETER)
                {
                    oqpi_warning("Unable to retrieve core affinity mask for thread %d", GetCurrentThreadId());
                    break;
                }
                testMask <<= 1;
            }

            return static_cast<core_affinity>(dwPreviousAffinityMask);
        }
        //------------------------------------------------------------------------------------------
        static void set_priority(native_handle_type handle, thread_priority priority)
        {
            SetThreadPriority(handle, win_thread_priority(priority));
        }
        //------------------------------------------------------------------------------------------
        static thread_priority get_priority(native_handle_type handle)
        {
            auto threadPriority = thread_priority::normal;

            const auto priority = GetThreadPriority(handle);
            if (priority != THREAD_PRIORITY_ERROR_RETURN)
            {
                for (int i = 0; i < int32_t(thread_priority::count); ++i)
                {
                    const auto p = static_cast<thread_priority>(i);
                    if (priority == win_thread_priority(p))
                    {
                        threadPriority = p;
                        break;
                    }
                }
            }
            else
            {
                oqpi_warning("Unable to retrieve thread priority for thread %d", GetCurrentThreadId());
            }

            return threadPriority;
        }
        //------------------------------------------------------------------------------------------
        static void set_name(id threadId, const char *name)
        {
            #pragma pack(push,8)
            struct THREADNAME_INFO
            {
                DWORD  dwType;       // Must be 0x1000.
                LPCSTR szName;       // Pointer to name (in user addr space).
                DWORD  dwThreadID;   // Thread ID (-1=caller thread).
                DWORD  dwFlags;      // Reserved for future use, must be zero.
            };
            #pragma pack(pop)

            THREADNAME_INFO info;
            info.dwType     = 0x1000;
            info.szName     = name;
            info.dwThreadID = threadId;
            info.dwFlags    = 0;

            __try
            {
                static const auto MS_VC_EXCEPTION = DWORD{ 0x406D1388 };
                RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
            }
        }
        //------------------------------------------------------------------------------------------


        //------------------------------------------------------------------------------------------
        static id get_current_thread_id()
        {
            return GetCurrentThreadId();
        }
        //------------------------------------------------------------------------------------------

    private:
        //------------------------------------------------------------------------------------------
        static int win_thread_priority(thread_priority prio)
        {
            static const int win_priority[] =
            {
                THREAD_PRIORITY_LOWEST,         // lowest
                THREAD_PRIORITY_BELOW_NORMAL,   // below_normal
                THREAD_PRIORITY_NORMAL,         // normal
                THREAD_PRIORITY_ABOVE_NORMAL,   // above_normal
                THREAD_PRIORITY_HIGHEST,        // highest
                THREAD_PRIORITY_TIME_CRITICAL,  // time_critical
            };
            static_assert(sizeof(win_priority) / sizeof(win_priority[0]) == size_t(thread_priority::count),
                "Thread priority count mismatch for Windows platform");

            return win_priority[static_cast<uint32_t>(prio)];
        }
        //------------------------------------------------------------------------------------------

    private:
        //------------------------------------------------------------------------------------------
        HANDLE handle_;
        DWORD  id_;
    };


    namespace this_thread {

        //------------------------------------------------------------------------------------------
        inline void set_name(const char *name)
        {
            win_thread::set_name(GetCurrentThreadId(), name);
        }
        //------------------------------------------------------------------------------------------
        template<typename _Rep, typename _Period>
        inline void sleep_for(const std::chrono::duration<_Rep, _Period>& relTime)
        {
            const auto dwMilliseconds = static_cast<DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(relTime).count());
            SleepEx(dwMilliseconds, TRUE);
        }
        //------------------------------------------------------------------------------------------
        inline uint32_t get_current_core()
        {
            // Retrieves the number of the processor the current thread was running on during
            // the call to this function.
            return GetCurrentProcessorNumber();
        }
        //------------------------------------------------------------------------------------------
        inline void yield() noexcept
        {
            // Causes the calling thread to yield execution to another thread that is ready to run
            // on the current processor. The operating system selects the next thread to be executed.
            SwitchToThread();
        }
        //------------------------------------------------------------------------------------------
        inline void set_priority(thread_priority threadPriority)
        {
            win_thread::set_priority(GetCurrentThread(), threadPriority);
        }
        //------------------------------------------------------------------------------------------
        inline void set_affinity_mask(core_affinity coreAffinityMask)
        {
            win_thread::set_core_affinity_mask(GetCurrentThread(), coreAffinityMask);
        }
        //------------------------------------------------------------------------------------------
        inline auto get_id()
        {
            return win_thread::get_current_thread_id();
        }
        //------------------------------------------------------------------------------------------

    } /*this_thread*/

} /*oqpi*/
