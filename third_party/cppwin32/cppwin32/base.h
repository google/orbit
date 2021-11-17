#pragma once

#ifndef WIN32_BASE_H
#define WIN32_BASE_H

#include <array>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <string_view>
#include <type_traits>

#ifdef _DEBUG

#define WIN32_ASSERT _ASSERTE
#define WIN32_VERIFY WIN32_ASSERT
#define WIN32_VERIFY_(result, expression) WIN32_ASSERT(result == expression)

#else

#define WIN32_ASSERT(expression) ((void)0)
#define WIN32_VERIFY(expression) (void)(expression)
#define WIN32_VERIFY_(result, expression) (void)(expression)

#endif

#define WIN32_IMPL_SHIM(...) (*(abi_t<__VA_ARGS__>**)(this))

#ifdef __INTELLISENSE__
#define WIN32_IMPL_AUTO(...) __VA_ARGS__
#else
#define WIN32_IMPL_AUTO(...) auto
#endif

#ifndef WIN32_EXPORT
#define WIN32_EXPORT
#endif

#ifdef __IUnknown_INTERFACE_DEFINED__
#define WIN32_IMPL_IUNKNOWN_DEFINED
#endif

#ifdef _M_HYBRID
#define WIN32_IMPL_LINK(function, count) __pragma(comment(linker, "/alternatename:#WIN32_IMPL_" #function "@" #count "=#" #function "@" #count))
#elif _M_ARM64EC
#define WIN32_IMPL_LINK(function, count) __pragma(comment(linker, "/alternatename:#WIN32_IMPL_" #function "=#" #function))
#elif _M_IX86
#define WIN32_IMPL_LINK(function, count) __pragma(comment(linker, "/alternatename:_WIN32_IMPL_" #function "@" #count "=_" #function "@" #count))
#else
#define WIN32_IMPL_LINK(function, count) __pragma(comment(linker, "/alternatename:WIN32_IMPL_" #function "=" #function))
#endif

WIN32_EXPORT namespace win32
{
    struct hresult
    {
        int32_t value{};

        constexpr hresult() noexcept = default;

        constexpr hresult(int32_t const value) noexcept : value(value)
        {
        }

        constexpr operator int32_t() const noexcept
        {
            return value;
        }
    };

    struct guid
    {
        uint32_t Data1;
        uint16_t Data2;
        uint16_t Data3;
        uint8_t  Data4[8];

        guid() noexcept = default;

        constexpr guid(uint32_t const Data1, uint16_t const Data2, uint16_t const Data3, std::array<uint8_t, 8> const& Data4) noexcept :
            Data1(Data1),
            Data2(Data2),
            Data3(Data3),
            Data4{ Data4[0], Data4[1], Data4[2], Data4[3], Data4[4], Data4[5], Data4[6], Data4[7] }
        {
        }

#ifdef WIN32_IMPL_IUNKNOWN_DEFINED

        constexpr guid(GUID const& value) noexcept :
            Data1(value.Data1),
            Data2(value.Data2),
            Data3(value.Data3),
            Data4{ value.Data4[0], value.Data4[1], value.Data4[2], value.Data4[3], value.Data4[4], value.Data4[5], value.Data4[6], value.Data4[7] }
        {

        }

        operator GUID const& () const noexcept
        {
            return reinterpret_cast<GUID const&>(*this);
        }

#endif
    };

    inline bool operator==(guid const& left, guid const& right) noexcept
    {
        return !memcmp(&left, &right, sizeof(left));
    }

    inline bool operator!=(guid const& left, guid const& right) noexcept
    {
        return !(left == right);
    }

    inline bool operator<(guid const& left, guid const& right) noexcept
    {
        return memcmp(&left, &right, sizeof(left)) < 0;
    }
}

WIN32_EXPORT namespace win32::Windows::Win32
{
    struct IUnknown;
}

namespace win32::_impl_
{
#ifdef WIN32_IMPL_IUNKNOWN_DEFINED
    using guid_type = GUID;
#else
    using guid_type = guid;
#endif
}

WIN32_EXPORT namespace win32
{
    void check_hresult(hresult const result);
    hresult to_hresult() noexcept;

    struct take_ownership_from_abi_t {};
    inline constexpr take_ownership_from_abi_t take_ownership_from_abi{};

    template <typename T>
    struct com_ptr;
}

namespace win32::_impl_
{
    template <typename T, typename Enable = void>
    struct abi
    {
        using type = T;
    };

    template <typename T>
    struct abi<T, std::enable_if_t<std::is_enum_v<T>>>
    {
        using type = std::underlying_type_t<T>;
    };

    template <typename T>
    using abi_t = typename abi<T>::type;

    template <typename T, typename = std::void_t<>>
    struct default_interface
    {
        using type = T;
    };

    template <typename T>
#ifdef WIN32_IMPL_IUNKNOWN_DEFINED
#ifdef __clang__
    inline const guid guid_v{ __uuidof(T) };
#else
    inline constexpr guid guid_v{ __uuidof(T) };
#endif
#else
    inline constexpr guid guid_v{};
#endif

    template <typename T>
    constexpr auto to_underlying_type(T const value) noexcept
    {
        return static_cast<std::underlying_type_t<T>>(value);
    }

    template <typename, typename = std::void_t<>>
    struct is_implements : std::false_type {};

    template <typename T>
    struct is_implements<T, std::void_t<typename T::implements_type>> : std::true_type {};

    template <typename T>
    T empty_value() noexcept
    {
        if constexpr (std::is_base_of_v<Windows::Win32::IUnknown, T>)
        {
            return nullptr;
        }
        else
        {
            return {};
        }
    }

    template <typename D, typename I, typename Enable = void>
    struct produce_base;

    template <typename D, typename I>
    struct produce;

    template <typename D>
    struct produce<D, Windows::Win32::IUnknown> : produce_base<D, Windows::Win32::IUnknown>
    {
    };

    template <typename T>
    inline constexpr bool is_implements_v = is_implements<T>::value;

    template <typename T>
    struct wrapped_type
    {
        using type = T;
    };

    template <typename T>
    struct wrapped_type<com_ptr<T>>
    {
        using type = T;
    };

    template <typename T>
    using wrapped_type_t = typename wrapped_type<T>::type;
}

WIN32_EXPORT namespace win32
{
    template <typename T>
    using default_interface = typename _impl_::default_interface<T>::type;

    template <typename T>
    constexpr guid const& guid_of() noexcept
    {
        return _impl_::guid_v<default_interface<T>>;
    }

    template <typename... T>
    bool is_guid_of(guid const& id) noexcept
    {
        return ((id == guid_of<T>()) || ...);
    }
}

namespace win32::_impl_
{
    template <typename T>
    using com_ref = std::conditional_t<std::is_base_of_v<Windows::Win32::IUnknown, T>, T, com_ptr<T>>;

    template <typename T, std::enable_if_t<is_implements_v<T>, int> = 0>
    com_ref<T> wrap_as_result(void* result)
    {
        return { &static_cast<produce<T, typename default_interface<T>::type>*>(result)->shim(), take_ownership_from_abi };
    }

    template <typename T, std::enable_if_t<!is_implements_v<T>, int> = 0>
    com_ref<T> wrap_as_result(void* result)
    {
        return { result, take_ownership_from_abi };
    }

#ifdef WIN32_IMPL_IUNKNOWN_DEFINED
    template <typename T>
    struct is_com_interface : std::disjunction<std::is_base_of<Windows::Win32::IUnknown, T>, is_implements<T>, std::is_base_of<::IUnknown, T>> {};
#else
    template <typename T>
    struct is_com_interface : std::disjunction<std::is_base_of<Windows::Win32::IUnknown, T>, is_implements<T>> {};
#endif

    template <typename T>
    inline constexpr bool is_com_interface_v = is_com_interface<T>::value;

    // You must include <Unknwn.h> to use this overload.
    template <typename To, typename From, std::enable_if_t<!is_com_interface_v<To>, int> = 0>
    auto as(From* ptr);

    template <typename To, typename From, std::enable_if_t<is_com_interface_v<To>, int> = 0>
    com_ref<To> as(From* ptr)
    {
        if (!ptr)
        {
            return nullptr;
        }

        void* result{};
        check_hresult(ptr->QueryInterface(guid_of<To>(), &result));
        return wrap_as_result<To>(result);
    }

    // You must include <Unknwn.h> to use this overload.
    template <typename To, typename From, std::enable_if_t<!is_com_interface_v<To>, int> = 0>
    auto try_as(From* ptr) noexcept;

    template <typename To, typename From, std::enable_if_t<is_com_interface_v<To>, int> = 0>
    com_ref<To> try_as(From* ptr) noexcept
    {
        if (!ptr)
        {
            return nullptr;
        }

        void* result{};
        ptr->QueryInterface(guid_of<To>(), &result);
        return wrap_as_result<To>(result);
    }
}

WIN32_EXPORT namespace win32::Windows::Win32
{
    struct IUnknown;
}

WIN32_EXPORT namespace win32
{
    template <typename T, std::enable_if_t<!std::is_base_of_v<Windows::Win32::IUnknown, T>, int> = 0>
    auto get_abi(T const& object) noexcept
    {
        return reinterpret_cast<_impl_::abi_t<T> const&>(object);
    }

    template <typename T, std::enable_if_t<!std::is_base_of_v<Windows::Win32::IUnknown, T>, int> = 0>
    auto put_abi(T& object) noexcept
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            object = {};
        }

        return reinterpret_cast<_impl_::abi_t<T>*>(&object);
    }

    template <typename T, typename V, std::enable_if_t<!std::is_base_of_v<Windows::Win32::IUnknown, T>, int> = 0>
    void copy_from_abi(T& object, V&& value)
    {
        object = reinterpret_cast<T const&>(value);
    }

    template <typename T, typename V, std::enable_if_t<!std::is_base_of_v<Windows::Win32::IUnknown, T>, int> = 0>
    void copy_to_abi(T const& object, V& value)
    {
        reinterpret_cast<T&>(value) = object;
    }

    template <typename T, std::enable_if_t<!std::is_base_of_v<Windows::Win32::IUnknown, std::decay_t<T>> && !std::is_convertible_v<T, std::wstring_view>, int> = 0>
    auto detach_abi(T&& object)
    {
        _impl_::abi_t<T> result{};
        reinterpret_cast<T&>(result) = std::move(object);
        return result;
    }

    constexpr void* detach_abi(std::nullptr_t) noexcept
    {
        return nullptr;
    }

#ifdef WIN32_IMPL_IUNKNOWN_DEFINED

    inline ::IUnknown* get_unknown(Windows::Win32::IUnknown const& object) noexcept
    {
        return static_cast<::IUnknown*>(get_abi(object));
    }

#endif
}

WIN32_EXPORT namespace win32
{
    template <typename T>
    struct com_ptr
    {
        using type = _impl_::abi_t<T>;

        com_ptr(std::nullptr_t = nullptr) noexcept {}

        com_ptr(void* ptr, take_ownership_from_abi_t) noexcept : m_ptr(static_cast<type*>(ptr))
        {
        }

        com_ptr(com_ptr const& other) noexcept : m_ptr(other.m_ptr)
        {
            add_ref();
        }

        template <typename U>
        com_ptr(com_ptr<U> const& other) noexcept : m_ptr(other.m_ptr)
        {
            add_ref();
        }

        template <typename U>
        com_ptr(com_ptr<U>&& other) noexcept : m_ptr(std::exchange(other.m_ptr, {}))
        {
        }

        ~com_ptr() noexcept
        {
            release_ref();
        }

        com_ptr& operator=(com_ptr const& other) noexcept
        {
            copy_ref(other.m_ptr);
            return*this;
        }

        com_ptr& operator=(com_ptr&& other) noexcept
        {
            if (this != &other)
            {
                release_ref();
                m_ptr = std::exchange(other.m_ptr, {});
            }

            return*this;
        }

        template <typename U>
        com_ptr& operator=(com_ptr<U> const& other) noexcept
        {
            copy_ref(other.m_ptr);
            return*this;
        }

        template <typename U>
        com_ptr& operator=(com_ptr<U>&& other) noexcept
        {
            release_ref();
            m_ptr = std::exchange(other.m_ptr, {});
            return*this;
        }

        explicit operator bool() const noexcept
        {
            return m_ptr != nullptr;
        }

        auto operator->() const noexcept
        {
            return m_ptr;
        }

        T& operator*() const noexcept
        {
            return *m_ptr;
        }

        type* get() const noexcept
        {
            return m_ptr;
        }

        type** put() noexcept
        {
            WIN32_ASSERT(m_ptr == nullptr);
            return &m_ptr;
        }

        void** put_void() noexcept
        {
            return reinterpret_cast<void**>(put());
        }

        void attach(type* value) noexcept
        {
            release_ref();
            *put() = value;
        }

        type* detach() noexcept
        {
            return std::exchange(m_ptr, {});
        }

        friend void swap(com_ptr& left, com_ptr& right) noexcept
        {
            std::swap(left.m_ptr, right.m_ptr);
        }

        template <typename To>
        auto as() const
        {
            return _impl_::as<To>(m_ptr);
        }

        template <typename To>
        auto try_as() const noexcept
        {
            return _impl_::try_as<To>(m_ptr);
        }

        template <typename To>
        void as(To& to) const
        {
            to = as<_impl_::wrapped_type_t<To>>();
        }

        template <typename To>
        bool try_as(To& to) const noexcept
        {
            if constexpr (_impl_::is_com_interface_v<To> || !std::is_same_v<To, _impl_::wrapped_type_t<To>>)
            {
                to = try_as<_impl_::wrapped_type_t<To>>();
                return static_cast<bool>(to);
            }
            else
            {
                auto result = try_as<To>();
                to = result.has_value() ? result.value() : _impl_::empty_value<To>();
                return result.has_value();
            }
        }

        hresult as(guid const& id, void** result) const noexcept
        {
            return m_ptr->QueryInterface(id, result);
        }

        void copy_from(type* other) noexcept
        {
            copy_ref(other);
        }

        void copy_to(type** other) const noexcept
        {
            add_ref();
            *other = m_ptr;
        }

        template <typename F, typename...Args>
        bool try_capture(F function, Args&&...args)
        {
            return function(args..., guid_of<T>(), put_void()) >= 0;
        }

        template <typename O, typename M, typename...Args>
        bool try_capture(com_ptr<O> const& object, M method, Args&&...args)
        {
            return (object.get()->*(method))(args..., guid_of<T>(), put_void()) >= 0;
        }

        template <typename F, typename...Args>
        void capture(F function, Args&&...args)
        {
            check_hresult(function(args..., guid_of<T>(), put_void()));
        }

        template <typename O, typename M, typename...Args>
        void capture(com_ptr<O> const& object, M method, Args&&...args)
        {
            check_hresult((object.get()->*(method))(args..., guid_of<T>(), put_void()));
        }

    private:

        void copy_ref(type* other) noexcept
        {
            if (m_ptr != other)
            {
                release_ref();
                m_ptr = other;
                add_ref();
            }
        }

        void add_ref() const noexcept
        {
            if (m_ptr)
            {
                const_cast<std::remove_const_t<type>*>(m_ptr)->AddRef();
            }
        }

        void release_ref() noexcept
        {
            if (m_ptr)
            {
                unconditional_release_ref();
            }
        }

        __declspec(noinline) void unconditional_release_ref() noexcept
        {
            std::exchange(m_ptr, {})->Release();
        }

        template <typename U>
        friend struct com_ptr;

        type* m_ptr{};
    };
}

namespace win32::_impl_
{
    template <typename T>
    struct bind_out
    {
        bind_out(T& object) noexcept : object(object)
        {
        }

        T& object;

        operator void** () const noexcept
        {
            object = nullptr;
            return (void**)(&object);
        }

        template <typename R>
        operator R* () const noexcept
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                object = {};
            }

            return reinterpret_cast<R*>(&object);
        }
    };
}

namespace win32::_impl_
{
    template <typename pointer_t,
        typename close_fn_t,
        close_fn_t close_fn,
        typename pointer_storage_t = pointer_t,
        typename invalid_t = pointer_t,
        invalid_t invalid = invalid_t(),
        typename pointer_invalid_t = std::nullptr_t>
        struct resource_policy
    {
        using pointer_storage = pointer_storage_t;
        using pointer = pointer_t;
        using pointer_invalid = pointer_invalid_t;
        inline static pointer_storage invalid_value() noexcept { return (pointer)invalid; }
        inline static bool is_valid(pointer_storage value) noexcept { return static_cast<pointer>(value) != invalid_value(); }
        inline static void close(pointer_storage value) noexcept { std::invoke(close_fn, value); }
    };

    template <typename Policy>
    struct unique_storage
    {
    protected:
        using policy = Policy;
        using pointer_storage = typename policy::pointer_storage;
        using pointer = typename policy::pointer;
        using base_storage = unique_storage<policy>;

        unique_storage() noexcept
            : m_ptr(policy::invalid_value())
        {}

        explicit unique_storage(pointer_storage ptr) noexcept
            : m_ptr(ptr)
        {}

        unique_storage(unique_storage&& other) noexcept
            : m_ptr(std::move(other.m_ptr))
        {
            other.m_ptr = policy::invalid_value();
        }

        ~unique_storage() noexcept
        {
            if (policy::is_valid(m_ptr))
            {
                policy::close(m_ptr);
            }
        }

        void replace(unique_storage&& other) noexcept
        {
            reset(other.m_ptr);
            other.m_ptr = policy::invalid_value();
        }

    public:
        bool is_valid() const noexcept
        {
            return policy::is_valid(m_ptr);
        }

        void reset(pointer_storage ptr = policy::invalid_value()) noexcept
        {
            if (policy::is_valid(m_ptr))
            {
                policy::close(m_ptr);
            }
            m_ptr = ptr;
        }

        void reset(std::nullptr_t) noexcept
        {
            static_assert(std::is_same_v<typename policy::pointer_invalid, std::nullptr_t>, "reset(nullptr): valid only for handle types using nullptr as the invalid value");
        }

        pointer get() const noexcept
        {
            return static_cast<pointer>(m_ptr);
        }

        pointer_storage release() noexcept
        {
            auto ptr = m_ptr;
            m_ptr = policy::invalid_value();
            return ptr;
        }

        pointer_storage* addressof() noexcept
        {
            return &m_ptr;
        }

    private:
        pointer_storage m_ptr;
    };
}

WIN32_EXPORT namespace win32
{
    template <typename storage_t>
    struct unique_any_t : storage_t
    {
        using policy = typename storage_t::policy;
        using pointer_storage = typename policy::pointer_storage;
        using pointer = typename policy::pointer;

        unique_any_t(unique_any_t const&) = delete;
        unique_any_t& operator=(unique_any_t const&) = delete;

        unique_any_t() noexcept = default;

        template <typename Arg1, typename... Args>
        explicit unique_any_t(Arg1&& arg1, Args&&... args) noexcept
            : storage_t(std::forward<Arg1>(args), std::forward<Args>(args)...)
        {}

        explicit unique_any_t(std::nullptr_t) noexcept
        {
            static_assert(std::is_same_v<typename policy::pointer_invalid, std::nullptr_t>, "nullptr constructor: valid only for handle types using nullptr as the invalid value");
        }

        unique_any_t(unique_any_t&& other) noexcept
            : storage_t(std::move(other))
        {}

        unique_any_t& operator=(unique_any_t&& other) noexcept
        {
            if (this != std::addressof(other))
            {
                storage_t::replace(std::move(static_cast<typename storage_t::base_storage&>(other)));
            }
            return (*this);
        }

        unique_any_t& operator=(std::nullptr_t) noexcept
        {
            static_assert(std::is_same_v<typename policy::pointer_invalid, std::nullptr_t>, "nullptr assignment: valid only for handle types using nullptr as the invalid value");
            storage_t::reset();
            return (*this);
        }

        void swap(unique_any_t& other) noexcept
        {
            unique_any_t self(std::move(*this));
            operator=(std::move(other));
            other = std::move(self);
        }

        explicit operator bool() const noexcept
        {
            return storage_t::is_valid();
        }

        pointer_storage* put() noexcept
        {
            storage_t::reset();
            return storage_t::addressof();
        }

        pointer_storage* operator&() noexcept
        {
            return put();
        }

        pointer get() const noexcept
        {
            return storage_t::get();
        }
    };

    template <typename policy>
    void swap(unique_any_t<policy>& left, unique_any_t<policy>& right) noexcept
    {
        left.swap(right);
    }

    template <typename policy>
    bool operator==(const unique_any_t<policy>& left, const unique_any_t<policy>& right) noexcept
    {
        return (left.get() == right.get());
    }

    template <typename policy>
    bool operator==(const unique_any_t<policy>& left, std::nullptr_t) noexcept
    {
        static_assert(std::is_same_v<typename unique_any_t<policy>::policy::pointer_invalid, std::nullptr_t>, "the resource class does not use nullptr as an invalid value");
        return !left;
    }

    template <typename policy>
    bool operator==(std::nullptr_t, const unique_any_t<policy>& right) noexcept
    {
        static_assert(std::is_same_v<typename unique_any_t<policy>::policy::pointer_invalid, std::nullptr_t>, "the resource class does not use nullptr as an invalid value");
        return !right;
    }

    template <typename policy>
    bool operator!=(const unique_any_t<policy>& left, const unique_any_t<policy>& right) noexcept
    {
        return (!(left.get() == right.get()));
    }

    template <typename policy>
    bool operator!=(const unique_any_t<policy>& left, std::nullptr_t) noexcept
    {
        static_assert(std::is_same_v<typename unique_any_t<policy>::policy::pointer_invalid, std::nullptr_t>, "the resource class does not use nullptr as an invalid value");
        return !!left;
    }

    template <typename policy>
    bool operator!=(std::nullptr_t, const unique_any_t<policy>& right) noexcept
    {
        static_assert(std::is_same_v<typename unique_any_t<policy>::policy::pointer_invalid, std::nullptr_t>, "the resource class does not use nullptr as an invalid value");
        return !!right;
    }

    template <typename policy>
    bool operator<(const unique_any_t<policy>& left, const unique_any_t<policy>& right) noexcept
    {
        return (left.get() < right.get());
    }

    template <typename policy>
    bool operator>=(const unique_any_t<policy>& left, const unique_any_t<policy>& right) noexcept
    {
        return (!(left < right));
    }

    template <typename policy>
    bool operator>(const unique_any_t<policy>& left, const unique_any_t<policy>& right) noexcept
    {
        return (right < left);
    }

    template <typename policy>
    bool operator<=(const unique_any_t<policy>& left, const unique_any_t<policy>& right) noexcept
    {
        return (!(right < left));
    }

    template <typename pointer,
        typename close_fn_t,
        close_fn_t close_fn,
        typename pointer_storage = pointer,
        typename invalid_t = pointer,
        invalid_t invalid = invalid_t(),
        typename pointer_invalid = std::nullptr_t>
        using unique_any = unique_any_t<_impl_::unique_storage<_impl_::resource_policy<pointer, close_fn_t, close_fn, pointer_storage, invalid_t, invalid, pointer_invalid>>>;
}

// TODO: Hook this up to a real version
#define CPPWIN32_VERSION "0.0.0.1"

// WINRT_version is used by Microsoft to analyze C++/WinRT library adoption and inform future product decisions.
extern "C"
__declspec(selectany)
char const* const WIN32_version = "C++/Win32 version:" CPPWIN32_VERSION;

#ifdef _M_IX86
#pragma comment(linker, "/include:_WIN32_version")
#else
#pragma comment(linker, "/include:WIN32_version")
#endif

WIN32_EXPORT namespace win32
{
    template <size_t BaseSize, size_t ComponentSize>
    constexpr bool check_version(char const(&base)[BaseSize], char const(&component)[ComponentSize]) noexcept
    {
        if constexpr (BaseSize != ComponentSize)
        {
            return false;
        }

        for (size_t i = 0; i != BaseSize - 1; ++i)
        {
            if (base[i] != component[i])
            {
                return false;
            }
        }

        return true;
    }
}
#endif
