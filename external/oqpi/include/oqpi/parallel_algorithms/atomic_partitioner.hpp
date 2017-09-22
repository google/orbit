#pragma once

#include <atomic>

#include "oqpi/parallel_algorithms/base_partitioner.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    // Partitioner giving a fixed set of indices to each worker until no more indices are available
    //
    class atomic_partitioner
        : public base_partitioner
    {
    public:
        //------------------------------------------------------------------------------------------
        atomic_partitioner(int32_t firstIndex, int32_t lastIndex, int32_t indicesToGrab, int32_t maxBatches)
            : base_partitioner(firstIndex, lastIndex, maxBatches)
            , indicesToGrab_(indicesToGrab)
            , sharedIndex_(firstIndex)
        {}

        //------------------------------------------------------------------------------------------
        atomic_partitioner(int32_t elementsCount, int32_t indicesToGrab, int32_t maxBatches)
            : atomic_partitioner(0, elementsCount, indicesToGrab, maxBatches)
        {}

        //------------------------------------------------------------------------------------------
        atomic_partitioner(const atomic_partitioner &other)
            : base_partitioner(other)
            , indicesToGrab_(other.indicesToGrab_)
            , sharedIndex_(other.sharedIndex_.load())
        {}


        //------------------------------------------------------------------------------------------
        // Sets a range of indices for the caller to work on and returns true.
        // If no more indices are available returns false.
        // The range is warrantied to be returned to one and only one thread.
        //
        inline bool getNextValidRange(int32_t &firstIndex, int32_t &lastIndex)
        {
            while (true)
            {
                // Get a copy of where we're at in the array
                auto expectedIndex = sharedIndex_.load();
                // Compute how many indices we could grab
                const auto count = std::min<int32_t>(elementCount_ - expectedIndex, indicesToGrab_);
                if (count > 0)
                {
                    // There's still something to grab, try to do it, by being the one to increment the shared index.
                    if (sharedIndex_.compare_exchange_strong(expectedIndex, expectedIndex + count))
                    {
                        // We managed to grab the indices, still some work to do
                        firstIndex = expectedIndex;
                        lastIndex  = firstIndex + count;
                        return true;
                    }
                }
                else
                {
                    return false;
                }
            }
        }


        //------------------------------------------------------------------------------------------
        // Number of indices to grab at each run
        const int32_t           indicesToGrab_;
        // Shared index between all threads
        std::atomic<int32_t>    sharedIndex_;
    };
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
