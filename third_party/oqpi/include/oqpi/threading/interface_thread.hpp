#pragma once

#include <tuple>
#include <memory>
#include <type_traits>

#include "oqpi/empty_layer.hpp"
#include "oqpi/threading/thread_attributes.hpp"


namespace oqpi { namespace itfc {

    //----------------------------------------------------------------------------------------------
    // Thread interface, all thread implementation need to comply to this interface
    template
    <
        // Platform specific implementation for a thread
          typename _Impl
        // Augmentation layer, needs to be templated and inherit from the implementation
        , template<typename> typename _Layer = empty_layer
    >
    class thread
        : public std::conditional<is_empty_layer<_Layer>::value, _Impl, _Layer<_Impl>>::type
    {
    public:
        //------------------------------------------------------------------------------------------
        // Whether the thread has augmented layer(s) or not
        static constexpr auto is_lean   = is_empty_layer<_Layer>::value;
        // The platform specific implementation
        using thread_impl               = _Impl;
        // The actual base type taking into account the presence or absence of augmentation layer(s)
        using base_type                 = typename std::conditional<is_lean, thread_impl, _Layer<thread_impl>>::type;
        // The actual type
        using self_type                 = thread<thread_impl, _Layer>;
        
    public:
        //------------------------------------------------------------------------------------------
        // Returns the number of hardware threads (logical cores)
        static unsigned int hardware_concurrency() { return thread_impl::hardware_concurrency(); }

    public:
        //------------------------------------------------------------------------------------------
        // Default constructible, constructs a non-joinable thread
        thread() = default;

        //------------------------------------------------------------------------------------------
        // Constructor from any callable object, creates a thread and runs the passed function
        // passing it the any arguments it needs. The arguments have to be provided.
        // See thread_attributes for more info on how to configure the thread.
        template<typename _Func, typename... _Args>
        explicit thread(const thread_attributes &attributes, _Func &&func, _Args &&...args)
        {
            launch(attributes, std::tuple<std::decay_t<_Func>, std::decay_t<_Args>...>(std::forward<_Func>(func), std::forward<_Args>(args)...));
        }

        //------------------------------------------------------------------------------------------
        // Creates a thread specifying only its name. Uses default thread configuration.
        template<typename _Func, typename... _Args>
        explicit thread(const std::string &name, _Func &&func, _Args &&...args)
            : thread(thread_attributes(name), std::forward<_Func>(func), std::forward<_Args>(args)...)
        {}

        //------------------------------------------------------------------------------------------
        // Kills the thread if it's still joinable on destruction
        ~thread() noexcept
        {
            if (joinable())
            {
                std::terminate();
            }
        }

    public:
        //------------------------------------------------------------------------------------------
        // Movable
        thread(self_type &&other)
            : base_type(std::move(other))
        {}
        //------------------------------------------------------------------------------------------
        self_type& operator =(self_type &&rhs)
        {
            if (this != &rhs)
            {
                if (joinable())
                {
                    std::terminate();
                }
                thread_impl::operator =(std::move(rhs));
            }
            return (*this);
        }

    private:
        //------------------------------------------------------------------------------------------
        // Not copyable
        thread(const self_type &)                   = delete;
        self_type& operator =(const self_type &)    = delete;


    public:
        //------------------------------------------------------------------------------------------
        // Forward native type  definitions
        using id                 = typename thread_impl::id;
        using native_handle_type = typename thread_impl::native_handle_type;

    public:
        //------------------------------------------------------------------------------------------
        // Public interface that needs to be implemented by the thread implementation
        id                  getId()                                     const   { return thread_impl::getId();                       }
        native_handle_type  getNativeHandle()                           const   { return thread_impl::getNativeHandle();             }

        bool                joinable()                                  const   { return thread_impl::joinable();                    }
        void                join()                                              { return thread_impl::join();                        }
        void                detach()                                            { return thread_impl::detach();                      }
                                        
        void                setCoreAffinityMask(core_affinity affinity)         { return thread_impl::setCoreAffinityMask(affinity); }
        core_affinity       getCoreAffinityMask()                       const   { return thread_impl::getCoreAffinityMask();         }

        void                setPriority(thread_priority priority)               { return thread_impl::setPriority(priority);         }
        thread_priority     getPriority()                               const   { return thread_impl::getPriority();                 }

    public:
        //------------------------------------------------------------------------------------------
        // Helpers
        void setCoreAffinity(int32_t coreNumber)
        {
            setCoreAffinityMask(core_affinity(1 << coreNumber));
        }

    private:
        //------------------------------------------------------------------------------------------
        // Intermediary structure used to launch a thread. It is templated by the tuple holding
        // the function as well as the needed parameters to pass to the said function.
        template<typename _Tuple>
        struct launcher
        {
            launcher(const thread_attributes &attributes, _Tuple &&t)
                : attributes_(attributes)
                , tuple_(std::move(t))
            {}

            inline void operator()()
            {
                thread_impl::set_name(thread_impl::get_current_thread_id(), attributes_.name_.c_str());
                run(std::make_integer_sequence<size_t, std::tuple_size<_Tuple>::value>());
            }

            template<size_t... _Indices>
            void run(std::integer_sequence<size_t, _Indices...>)
            {
                std::invoke(std::move(std::get<_Indices>(tuple_))...);
            }

            const thread_attributes attributes_;
            _Tuple tuple_;
        };

        //------------------------------------------------------------------------------------------
        // This function is responsible for the actual thread creation and launch.
        // The template tuple packs the function to call alongside its needed parameters.
        template<typename _Tuple>
        void launch(const thread_attributes &attributes, _Tuple &&tuple)
        {
            // Actual type of the launcher
            using launcher_t = launcher<_Tuple>;
            // Create a launcher on the heap to be able to pass it to the thread.
            // The unique_ptr will make sure that the resource will be freed if anything goes wrong.
            auto upLauncher = std::make_unique<launcher_t>(attributes, std::forward<_Tuple>(tuple));

            // Actually creates the thread. Passed a pointer to the launcher as user data.
            // The implementation should take the ownership of the launcher if the thread creation succeeds.
            if (thread_impl::template create<launcher_t>(attributes, upLauncher.get()))
            {
                // If the thread creation succeeded, we transfer the ownership of the launcher to it.
                upLauncher.release();
            }
        }
    };

} /*itfc*/ } /*oqpi*/