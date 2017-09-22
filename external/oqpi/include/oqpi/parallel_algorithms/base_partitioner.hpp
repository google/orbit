#pragma once


namespace oqpi {

    //------------------------------------------------------------------------------------------
    // Base class for the majority of partitioners.
    // Keeps first and last indexes and calculates the total number of elements in that range.
    //
    class base_partitioner
    {
    protected:
        //------------------------------------------------------------------------------------------
        base_partitioner(int32_t firstIndex, int32_t lastIndex, int32_t maxBatches)
            : firstIndex_   (firstIndex)
            , lastIndex_    (lastIndex)
            , elementCount_ (lastIndex - firstIndex)
            , batchCount_   ((elementCount_ < maxBatches) ? elementCount_ : maxBatches)
        {}

        //------------------------------------------------------------------------------------------
        base_partitioner(int32_t elementsCount, int32_t maxBatches)
            : base_partitioner(0, elementsCount, maxBatches)
        {}

        //------------------------------------------------------------------------------------------
        base_partitioner(const base_partitioner &other)
            : firstIndex_   (other.firstIndex_)
            , lastIndex_    (other.lastIndex_)
            , elementCount_ (other.elementCount_)
            , batchCount_   (other.batchCount_)
        {}

    public:
        //------------------------------------------------------------------------------------------
        inline int32_t isValid() const
        {
            return elementCount_ > 0;
        }

        //------------------------------------------------------------------------------------------
        inline int32_t batchCount() const
        {
            return batchCount_;
        }

        //------------------------------------------------------------------------------------------
        inline int32_t elementCount() const
        {
            return elementCount_;
        }

    protected:
        const int32_t firstIndex_;
        const int32_t lastIndex_;
        const int32_t elementCount_;
        const int32_t batchCount_;
    };

} /*oqpi*/
