#pragma once

namespace cppwin32
{
    using namespace winmd::reader;

    template <typename...T> struct visit_overload : T... { using T::operator()...; };

    template <typename V, typename...C>
    auto call(V&& variant, C&&...call)
    {
        return std::visit(visit_overload<C...>{ std::forward<C>(call)... }, std::forward<V>(variant));
    }

    struct type_name
    {
        std::string_view name;
        std::string_view name_space;

        explicit type_name(TypeDef const& type) :
            name(type.TypeName()),
            name_space(type.TypeNamespace())
        {
        }

        explicit type_name(TypeRef const& type) :
            name(type.TypeName()),
            name_space(type.TypeNamespace())
        {
        }

        explicit type_name(coded_index<TypeDefOrRef> const& type)
        {
            auto const& [type_namespace, type_name] = get_type_namespace_and_name(type);
            name_space = type_namespace;
            name = type_name;
        }
    };

    bool operator==(type_name const& left, type_name const& right)
    {
        return left.name == right.name && left.name_space == right.name_space;
    }

    bool operator==(type_name const& left, std::string_view const& right)
    {
        if (left.name.size() + 1 + left.name_space.size() != right.size())
        {
            return false;
        }

        if (right[left.name_space.size()] != '.')
        {
            return false;
        }

        if (0 != right.compare(left.name_space.size() + 1, left.name.size(), left.name))
        {
            return false;
        }

        return 0 == right.compare(0, left.name_space.size(), left.name_space);
    }

    struct method_signature
    {
        explicit method_signature(MethodDef const& method) :
            m_method(method),
            m_signature(method.Signature())
        {
            auto params = method.ParamList();

            if (m_signature.ReturnType() && params.first != params.second && params.first.Sequence() == 0)
            {
                m_return = params.first;
                ++params.first;
            }

            for (uint32_t i{}; i != size(m_signature.Params()); ++i)
            {
                m_params.emplace_back(params.first + i, &m_signature.Params().first[i]);
            }
        }

        std::vector<std::pair<Param, ParamSig const*>>& params()
        {
            return m_params;
        }

        std::vector<std::pair<Param, ParamSig const*>> const& params() const
        {
            return m_params;
        }

        auto const& return_signature() const
        {
            return m_signature.ReturnType();
        }

        auto return_param_name() const
        {
            std::string_view name;

            if (m_return && !m_return.Name().empty())
            {
                name = m_return.Name();
            }
            else
            {
                name = "win32_impl_result";
            }

            return name;
        }

        auto return_param() const
        {
            return m_return;
        }

        MethodDef const& method() const
        {
            return m_method;
        }

    private:

        MethodDef m_method;
        MethodDefSig m_signature;
        std::vector<std::pair<Param, ParamSig const*>> m_params;
        Param m_return;
    };

    enum class param_category
    {
        enum_type,
        struct_type,
        array_type,
        fundamental_type,
        interface_type,
        delegate_type,
        generic_type
    };

    inline param_category get_category(coded_index<TypeDefOrRef> const& type, TypeDef* signature_type = nullptr)
    {
        TypeDef type_def;
        if (type.type() == TypeDefOrRef::TypeDef)
        {
            type_def = type.TypeDef();
        }
        else
        {
            auto type_ref = type.TypeRef();
            if (type_name(type_ref) == "System.Guid")
            {
                return param_category::struct_type;
            }
            type_def = find_required(type_ref);
        }

        if (signature_type)
        {
            *signature_type = type_def;
        }

        switch (get_category(type_def))
        {
        case category::interface_type:
            return param_category::interface_type;
        case category::enum_type:
            return param_category::enum_type;
        case category::struct_type:
            return param_category::struct_type;
        case category::delegate_type:
            return param_category::delegate_type;
        default:
            return param_category::generic_type;
        }
    }

    inline param_category get_category(TypeSig const& signature, TypeDef* signature_type = nullptr)
    {
        if (signature.is_szarray())
        {
            return param_category::array_type;
        }

        if (signature.element_type() == ElementType::Class)
        {
            return param_category::interface_type;
        }

        param_category result{};

        call(signature.Type(),
            [&](ElementType type)
            {
                result = param_category::fundamental_type;
            },
            [&](coded_index<TypeDefOrRef> const& type)
            {
                result = get_category(type, signature_type);
            },
                [&](auto&&)
            {
                result = param_category::generic_type;
            });
        return result;
    }

    inline bool is_com_interface(TypeDef const& type)
    {
        if (type.TypeName() == "IUnknown")
        {
            return true;
        }

        for (auto&& base : type.InterfaceImpl())
        {
            auto base_type = find(base.Interface());
            if (base_type && is_com_interface(base_type))
            {
                return true;
            }
        }
        return false;
    }

    inline bool is_union(TypeDef const& type)
    {
        return type.Flags().Layout() == TypeLayout::ExplicitLayout;
    }

    inline bool is_nested(coded_index<TypeDefOrRef> const& type)
    {
        if (type.type() == TypeDefOrRef::TypeDef)
        {
            return is_nested(type.TypeDef());
        }
        else
        {
            XLANG_ASSERT(type.type() == TypeDefOrRef::TypeRef);
            return is_nested(type.TypeRef());
        }
    }

    MethodDef get_delegate_method(TypeDef const& type)
    {
        MethodDef invoke;
        for (auto&& method : type.MethodList())
        {
            if (method.Name() == "Invoke")
            {
                invoke = method;
                break;
            }
        }
        return invoke;
    }

    coded_index<TypeDefOrRef> get_base_interface(TypeDef const& type)
    {
        auto bases = type.InterfaceImpl();
        if (!empty(bases))
        {
            XLANG_ASSERT(size(bases) == 1);
            return bases.first.Interface();
        }
        return {};
    }
}