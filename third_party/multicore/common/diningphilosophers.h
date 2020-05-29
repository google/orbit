//---------------------------------------------------------
// For conditions of distribution and use, see
// https://github.com/preshing/cpp11-on-multicore/blob/master/LICENSE
//---------------------------------------------------------

#ifndef __CPP11OM_DINING_PHILOSOPHERS_H__
#define __CPP11OM_DINING_PHILOSOPHERS_H__

#include <cassert>
#include <mutex>
#include <vector>
#include <array>
#include <algorithm>
#include <utility>
#include <atomic>
#include "sema.h"
#include "bitfield.h"


//---------------------------------------------------------
// DiningPhiloHelpers
//---------------------------------------------------------
namespace DiningPhiloHelpers
{
    inline int left(int index, int numPhilos)
    {
        return (index > 0 ? index : numPhilos) - 1;
    }

    inline int right(int index, int numPhilos)
    {
        ++index;
        return index < numPhilos ? index : 0;
    }
}


//---------------------------------------------------------
// DiningPhilosophers
//---------------------------------------------------------
class DiningPhilosophers
{
private:
    int m_numPhilos;

    // "Box office" using a mutex.
    // m_status keeps track of the status of each philosopher (thread).
    // 0: Philosopher is thinking
    // 1: Philosopher is eating
    // 2+: Philosopher is waiting and must not eat before his/her direct neighbors if they have a lower status.
    std::mutex m_mutex;
    std::vector<int> m_status;

    // "Bouncers"
    // Can't use std::vector<DefaultSemaphoreType> because DefaultSemaphoreType is not copiable/movable.
    std::unique_ptr<DefaultSemaphoreType[]> m_sema;

    int left(int index) const { return DiningPhiloHelpers::left(index, m_numPhilos); }
    int right(int index) const { return DiningPhiloHelpers::right(index, m_numPhilos); }

    int neighbor(int index, int step) const
    {
        assert(step >= 0 && step < m_numPhilos);
        index += step;
        if (index >= m_numPhilos)
            index -= m_numPhilos;
        return index;
    }

    // We call tryAdjustStatus after a philosopher finishes eating.
    // It fans outward (in the direction of step), trying to decrement the status of each neighbor (to target).
    bool tryAdjustStatus(int philoIndex, int target, int step)
    {
        // Should not already have the target status.
        assert(m_status[philoIndex] != target);
        if (m_status[philoIndex] == target + 1)
        {
            // Decrementing this status will bring it to target.
            // Make sure the next neighbor doesn't prevent it.
            int n = neighbor(philoIndex, step);
            assert(m_status[n] != target + 1);  // No two neighbors should have equal status > 0.
            if (m_status[n] != target)
            {
                // Decrement it.
                m_status[philoIndex] = target;
                // If neighbor's status is exactly 1 greater, continue visiting.
                if (m_status[n] > target)
                    tryAdjustStatus(n, target + 1, step);
                return true;
            }
        }
        return false;
    }

public:
    DiningPhilosophers(int numPhilos) : m_numPhilos(numPhilos)
    {
        m_status.resize(numPhilos);
        m_sema = std::unique_ptr<DefaultSemaphoreType[]>(new DefaultSemaphoreType[numPhilos]);
    }

    void beginEating(int philoIndex)
    {
        int maxNeighborStatus; // Initialized inside lock

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            assert(m_status[philoIndex] == 0);  // Must have been thinking
            // Establish order relative to direct neighbors.
            maxNeighborStatus = std::max(m_status[left(philoIndex)], m_status[right(philoIndex)]);
            m_status[philoIndex] = maxNeighborStatus + 1;
            // Sanity check.
            for (int i = 0; i < m_numPhilos; i++)
                assert(m_status[i] >= 0 && m_status[i] <= m_numPhilos);
        }

        if (maxNeighborStatus > 0)
            m_sema[philoIndex].wait();  // Neighbor has priority; must wait
    }

    void endEating(int philoIndex)
    {
        int stepFirst = 1;
        int firstNeighbor = neighbor(philoIndex, 1);
        int secondNeighbor = neighbor(philoIndex, m_numPhilos - 1);
        bool firstWillEat;  // Initialized inside lock
        bool secondWillEat; // Initialized inside lock

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            assert(m_status[philoIndex] == 1);  // Must have been eating
            m_status[philoIndex] = 0;
            // Choose which neighbor to visit first based on priority
            if (m_status[firstNeighbor] > m_status[secondNeighbor])
            {
                std::swap(firstNeighbor, secondNeighbor);
                stepFirst = m_numPhilos - stepFirst;
            }
            // Adjust neighbor statuses.
            firstWillEat = tryAdjustStatus(firstNeighbor, 1, stepFirst);
            secondWillEat = tryAdjustStatus(secondNeighbor, 1, m_numPhilos - stepFirst);
            // Sanity check.
            for (int i = 0; i < m_numPhilos; i++)
                assert(m_status[i] >= 0 && m_status[i] <= m_numPhilos);
        }

        if (firstWillEat)
            m_sema[firstNeighbor].signal(); // Release waiting neighbor
        if (secondWillEat)
            m_sema[secondNeighbor].signal(); // Release waiting neighbor
    }
};


//---------------------------------------------------------
// LockReducedDiningPhilosophers
// Version of DiningPhilosophers with a lock-free box office.
//---------------------------------------------------------
class LockReducedDiningPhilosophers
{
private:
    int m_numPhilos;

    // Lock-free "box office".
    // AllStatus::philos keeps track of the status of each philosopher (thread).
    // 0: Philosopher is thinking
    // 1: Philosopher is eating
    // 2+: Philosopher is waiting and must not eat before his/her direct neighbors if they have a lower status.
    typedef uint32_t IntType;
    BEGIN_BITFIELD_TYPE(AllStatus, IntType)
        ADD_BITFIELD_ARRAY(philos, 0, 4, 8)     // 8 array elements, 4 bits each
    END_BITFIELD_TYPE()
    std::atomic<IntType> m_allStatus;

    // "Bouncers"
    // Can't use std::vector<DefaultSemaphoreType> because DefaultSemaphoreType is not copiable/movable.
    std::unique_ptr<DefaultSemaphoreType[]> m_sema;

    int left(int index) const { return DiningPhiloHelpers::left(index, m_numPhilos); }
    int right(int index) const { return DiningPhiloHelpers::right(index, m_numPhilos); }

    int neighbor(int index, int step) const
    {
        assert(step >= 0 && step < m_numPhilos);
        index += step;
        if (index >= m_numPhilos)
            index -= m_numPhilos;
        return index;
    }

    // We call tryAdjustStatus after a philosopher finishes eating.
    // It fans outward (in the direction of step), trying to decrement the status of each neighbor (to target).
    bool tryAdjustStatus(AllStatus& allStatus, int philoIndex, int target, int step) const
    {
        // Should not already have the target status.
        assert(allStatus.philos[philoIndex] != target);
        if (allStatus.philos[philoIndex] == target + 1)
        {
            // Decrementing this status will bring it to target.
            // Make sure the next neighbor doesn't prevent it.
            int n = neighbor(philoIndex, step);
            assert(allStatus.philos[n] != target + 1);    // No two neighbors should have equal status > 0.
            if (allStatus.philos[n] != target)
            {
                // Decrement it.
                allStatus.philos[philoIndex] = target;
                // If neighbor's status is exactly 1 greater, continue visiting.
                if (allStatus.philos[n] > IntType(target))
                    tryAdjustStatus(allStatus, n, target + 1, step);
                return true;
            }
        }
        return false;
    }

public:
    LockReducedDiningPhilosophers(int numPhilos)
    : m_numPhilos(numPhilos)
    , m_allStatus(0)
    {
        assert(IntType(numPhilos) <= AllStatus().philos.maximum());
        assert(numPhilos < AllStatus().philos.numItems());
        m_sema = std::unique_ptr<DefaultSemaphoreType[]>(new DefaultSemaphoreType[numPhilos]);
    }

    void beginEating(int philoIndex)
    {
        int maxNeighborStatus; // Initialized inside CAS loop.

        AllStatus oldStatus = m_allStatus.load(std::memory_order_relaxed);
        for (;;)    // Begin CAS loop
        {
            assert(oldStatus.philos[philoIndex] == 0);    // Must have been thinking
            // Establish order relative to direct neighbors.
            maxNeighborStatus = std::max(oldStatus.philos[left(philoIndex)], oldStatus.philos[right(philoIndex)]);
            AllStatus newStatus(oldStatus);
            newStatus.philos[philoIndex] = maxNeighborStatus + 1;
            // Sanity check.
            for (int i = 0; i < m_numPhilos; i++)
                assert(newStatus.philos[i] <= IntType(m_numPhilos));
            // CAS until successful. On failure, oldStatus will be updated with the latest value.
            if (m_allStatus.compare_exchange_strong(oldStatus, newStatus, std::memory_order_relaxed))
                break;
        }

        if (maxNeighborStatus > 0)
            m_sema[philoIndex].wait();  // Neighbor has priority; must wait
    }

    void endEating(int philoIndex)
    {
        int stepFirst = 1;
        int firstNeighbor = neighbor(philoIndex, 1);
        int secondNeighbor = neighbor(philoIndex, m_numPhilos - 1);
        bool firstWillEat;  // Initialized inside CAS loop.
        bool secondWillEat; // Initialized inside CAS loop.

        AllStatus oldStatus = m_allStatus.load(std::memory_order_relaxed);
        for (;;)    // Begin CAS loop
        {
            assert(oldStatus.philos[philoIndex] == 1);    // Must have been eating
            AllStatus newStatus(oldStatus);
            newStatus.philos[philoIndex] = 0;
            // Choose which neighbor to visit first based on priority
            if (newStatus.philos[firstNeighbor] > newStatus.philos[secondNeighbor])
            {
                std::swap(firstNeighbor, secondNeighbor);
                stepFirst = m_numPhilos - stepFirst;
            }
            // Adjust neighbor statuses.
            firstWillEat = tryAdjustStatus(newStatus, firstNeighbor, 1, stepFirst);
            secondWillEat = tryAdjustStatus(newStatus, secondNeighbor, 1, m_numPhilos - stepFirst);
            // Sanity check.
            for (int i = 0; i < m_numPhilos; i++)
                assert(newStatus.philos[i] <= IntType(m_numPhilos));
            // CAS until successful. On failure, oldStatus will be updated with the latest value.
            if (m_allStatus.compare_exchange_strong(oldStatus, newStatus, std::memory_order_relaxed))
                break;
        }

        if (firstWillEat)
            m_sema[firstNeighbor].signal(); // Release waiting neighbor
        if (secondWillEat)
            m_sema[secondNeighbor].signal(); // Release waiting neighbor
    }
};


typedef LockReducedDiningPhilosophers DefaultDiningPhilosophersType;


#endif // __CPP11OM_DINING_PHILOSOPHERS_H__
