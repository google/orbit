#pragma once

#include "oqpi/empty_layer.hpp"


namespace oqpi { namespace itfc {

    //----------------------------------------------------------------------------------------------
    template
    <
        // Platform specific implementation for semaphores
          typename _Impl
        // Augmentation layer, needs to be templated and inherit from the implementation
        , template<typename> typename _Layer = empty_layer
    >
    class semaphore
        : public std::conditional<is_empty_layer<_Layer>::value, _Impl, _Layer<_Impl>>::type
    {
    public:
        //------------------------------------------------------------------------------------------
        // Whether the event has augmented layer(s) or not
        static constexpr auto is_lean = is_empty_layer<_Layer>::value;
        // The platform specific implementation
        using semaphore_impl = _Impl;
        // The actual base type taking into account the presence or absence of augmentation layer(s)
        using base_type = typename std::conditional<is_lean, semaphore_impl, _Layer<semaphore_impl>>::type;
        // The actual type
        using self_type = semaphore<semaphore_impl, _Layer>;

    public:
        //------------------------------------------------------------------------------------------
        explicit semaphore(const std::string &name = "", int32_t initCount = 0, int32_t maxCount = 0x7fffffff)
            : base_type(initCount, maxCount, name)
        {}

    public:
        //------------------------------------------------------------------------------------------
        // Movable
        semaphore(self_type &&rhs)
            : base_type(std::move(rhs))
        {}

        //------------------------------------------------------------------------------------------
        inline self_type& operator =(self_type &&rhs)
        {
            if (this != &rhs)
            {
                base_type::operator =(std::move(rhs));
            }
            return (*this);
        }

        //------------------------------------------------------------------------------------------
        // Not copyable
        semaphore(const self_type &)                    = delete;
        self_type& operator =(const self_type &)    = delete;

    public:
        //------------------------------------------------------------------------------------------
        // User interface
        void notify(int32_t count)  { return base_type::notify(count);  }
        void notifyOne()            { return notify(1);                 }
        void notifyAll()            { return base_type::notifyAll();    }
        bool tryWait()              { return base_type::tryWait();      }
        void wait()                 { return base_type::wait();         }

        //------------------------------------------------------------------------------------------
        template<typename _Rep, typename _Period>
        inline bool waitFor(const std::chrono::duration<_Rep, _Period>& relTime) const
        {
            return base_type::waitFor(relTime);
        }
        template<typename _Clock, typename _Duration>
        inline bool waitUntil(const std::chrono::time_point<_Clock, _Duration>& absTime) const
        {
            return waitFor(absTime - _Clock::now());
        }
    };
    //----------------------------------------------------------------------------------------------

} /*itfc*/ } /*oqpi*/
