
namespace winmd::reader
{
    inline std::pair<std::string_view, std::string_view> get_type_namespace_and_name(coded_index<TypeDefOrRef> const& type)
    {
        if (type.type() == TypeDefOrRef::TypeDef)
        {
            auto const def = type.TypeDef();
            return { def.TypeNamespace(), def.TypeName() };
        }
        else if (type.type() == TypeDefOrRef::TypeRef)
        {
            auto const ref = type.TypeRef();
            return { ref.TypeNamespace(), ref.TypeName() };
        }
        else
        {
            XLANG_ASSERT(false);
            return {};
        }
    }

    inline std::pair<std::string_view, std::string_view> get_base_class_namespace_and_name(TypeDef const& type)
    {
        return get_type_namespace_and_name(type.Extends());
    }

    inline auto extends_type(TypeDef type, std::string_view typeNamespace, std::string_view typeName)
    {
        return get_base_class_namespace_and_name(type) == std::pair(typeNamespace, typeName);
    }

    inline bool is_nested(TypeDef const& type)
    {
        const auto visibility = type.Flags().Visibility();
        return !(visibility == TypeVisibility::Public || visibility == TypeVisibility::NotPublic);
    }

    inline bool is_nested(TypeRef const& type)
    {
        return type.ResolutionScope().type() == ResolutionScope::TypeRef;
    }
}
