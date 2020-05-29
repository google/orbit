#pragma once

#include <string>
#include <limits>
#include <cstdint>

namespace oqpi {

    //----------------------------------------------------------------------------------------------
    enum class thread_priority
    {
        lowest,
        below_normal,
        normal,
        above_normal,
        highest,
        time_critical,

        count
    };
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    enum core_affinity : uint32_t
    {
        core0 = 1,
        core1 = core0 << 1,
        core2 = core1 << 1,
        core3 = core2 << 1,
        core4 = core3 << 1,
        core5 = core4 << 1,
        core6 = core5 << 1,
        core7 = core6 << 1,

        all_cores = (std::numeric_limits<uint32_t>::max)()
    };
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    struct thread_attributes
    {
        // Thread's name that will appear in various debug tools
        std::string     name_;
        // The maximal stack size of the thread, 0 will use the system's default value
        uint32_t		stackSize_;
        // Specifies which cores this thread is allowed to run on.
        core_affinity   coreAffinityMask_;
        // The higher the priority the bigger the time slices this thread will be given in the 
        // underlying OS scheduler.
        thread_priority priority_;
        // Whether or not to launch the thread detached
        bool            detached_;

        // Constructor with default values, the name should always be specified
        thread_attributes
        (
            const std::string name,
            uint32_t stackSize              = 0u,
            core_affinity coreAffinityMask  = core_affinity::all_cores,
            thread_priority priority        = thread_priority::normal,
            bool detached                   = false
        )
            : name_             (name)
            , stackSize_        (stackSize)
            , coreAffinityMask_ (coreAffinityMask)
            , priority_         (priority)
            , detached_         (detached)
        {}
    };
    //----------------------------------------------------------------------------------------------

}
