#pragma once

#include <atomic>

#include "oqpi/parallel_algorithms/base_partitioner.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    // Partitioner dividing a set of indices into fixed size batches and giving one batch to each 
    // worker.
    //
    class simple_partitioner
        : public base_partitioner
    {
    public:
        simple_partitioner(int32_t firstIndex, int32_t lastIndex, int32_t maxBatches)
            : base_partitioner      (firstIndex, lastIndex, maxBatches)
            , nbElementsPerBatch_   ((elementCount_ >= maxBatches) ? (elementCount_ / batchCount_) : 1)
            , remainder_            ((elementCount_ >= maxBatches) ? (elementCount_ % batchCount_) : 0)
            , batchIndex_           (0)
        {}

        simple_partitioner(int32_t elementsCount, int32_t maxBatches)
            : simple_partitioner(0, elementsCount, maxBatches)
        {}

        simple_partitioner(const simple_partitioner &other)
            : base_partitioner      (other)
            , nbElementsPerBatch_   (other.nbElementsPerBatch_)
            , remainder_            (other.remainder_)
            , batchIndex_           (other.batchIndex_.load())
        {}

        inline bool getNextValidRange(int32_t &fromIndex, int32_t &toIndex)
        {
            const auto batchIndex = batchIndex_++;
            if (batchIndex >= batchCount_)
            {
                // All batches have been processed
                return false;
            }
            
            // Return the range [fromIndex; toIndex[
            fromIndex = firstIndexOfBatch(batchIndex);
            toIndex   = lastIndexOfBatch(batchIndex);

            return true;
        }

    private:
        inline int32_t firstIndexOfBatch(int32_t batchIndex) const
        {
            // The first index of the first batch is firstIndex_.
            // The first index of all other batches is the last index of the previous batch.
            return (batchIndex > 0) ? lastIndexOfBatch(batchIndex - 1) : firstIndex_;
        }

        inline int32_t lastIndexOfBatch(int32_t batchIndex) const
        {
            // If there's a remainder, each batch with batchIndex < remainder will process one extra element.
            // So each batch has to be offset by at most remainder_ number of elements.
            const auto offset = (batchIndex < remainder_) ? batchIndex + 1 : remainder_;
            return firstIndex_ + ((batchIndex + 1) * nbElementsPerBatch_) + offset;
        }

    private:
        // Minimum number of elements each batch should have (can be more if there is a remainder).
        const int32_t           nbElementsPerBatch_;
        // If elementCount_ is not divisible by batchCount_, this holds the remainder of that division.
        const int32_t           remainder_;
        // Each worker increments this atomic and is given the corresponding range, until it reaches batchCount_.
        std::atomic<int32_t>    batchIndex_;
    };

} /*oqpi*/
