#pragma once

#include <winmd_reader.h>
#include "text_writer.h"
#include "helpers.h"
#include "settings.h"

namespace cppwin32
{
    using namespace winmd::reader;

    template <typename First, typename... Rest>
    auto get_impl_name(First const& first, Rest const&... rest)
    {
        std::string result;
        result.reserve(std::string_view(first).size() + ((1 + std::string_view(rest).size()), ...));
        ((result += first), (((result += '_') += rest), ...));
        std::transform(result.begin(), result.end(), result.begin(), [](char c)
            {
                return c == '.' ? '_' : c;
            });
        return result;
    }

    struct writer : writer_base<writer>
    {
        using writer_base<writer>::write;

        struct depends_compare
        {
            bool operator()(TypeDef const& left, TypeDef const& right) const
            {
                return left.TypeName() < right.TypeName();
            }
            bool operator()(TypeRef const& left, TypeRef const& right) const
            {
                return left.TypeName() < right.TypeName();
            }
        };

        std::string type_namespace;
        bool abi_types{};
        bool full_namespace{};
        bool consume_types{};
        std::map<std::string_view, std::set<TypeDef, depends_compare>> depends;
        std::map<std::string_view, std::set<TypeRef, depends_compare>> extern_depends;

        template<typename T>
        struct member_value_guard
        {
            writer* const owner;
            T writer::* const member;
            T const previous;
            explicit member_value_guard(writer* arg, T writer::* ptr, T value) :
                owner(arg), member(ptr), previous(std::exchange(owner->*member, value))
            {
            }

            ~member_value_guard()
            {
                owner->*member = previous;
            }

            member_value_guard(member_value_guard const&) = delete;
            member_value_guard& operator=(member_value_guard const&) = delete;
        };

        [[nodiscard]] auto push_abi_types(bool value)
        {
            return member_value_guard(this, &writer::abi_types, value);
        }

        [[nodiscard]] auto push_full_namespace(bool value)
        {
            return member_value_guard(this, &writer::full_namespace, value);
        }

        [[nodiscard]] auto push_consume_types(bool value)
        {
            return member_value_guard(this, &writer::consume_types, value);
        }

        void write_root_include(std::string_view const& include)
        {
            auto format = R"(#include %win32/%.h%
)";

            write(format,
                settings.brackets ? '<' : '\"',
                include,
                settings.brackets ? '>' : '\"');
        }

        void add_depends(TypeDef const& type)
        {
            auto ns = type.TypeNamespace();

            if (ns != type_namespace)
            {
                depends[ns].insert(type);
            }
        }

        void add_extern_depends(TypeRef const& type)
        {
            auto ns = type.TypeNamespace();
            XLANG_ASSERT(ns != type_namespace);
            extern_depends[ns].insert(type);
        }

        void write_depends(std::string_view const& ns, char impl = 0)
        {
            if (impl)
            {
                write_root_include(write_temp("impl/%.%", ns, impl));
            }
            else
            {
                write_root_include(ns);
            }
        }

        template <typename T>
        void write_value(T value)
        {
            char buffer[128];
            char* first = buffer;
            char* last = std::end(buffer);

            if constexpr (std::numeric_limits<T>::is_integer)
            {
                int base = 10;
                if constexpr (!std::numeric_limits<T>::is_signed)
                {
                    *first++ = '0';
                    *first++ = 'x';
                    base = 16;
                }
                auto end = std::to_chars(first, last, value, base).ptr;
                write(std::string_view{ buffer, static_cast<size_t>(end - buffer) });
            }
            else
            {
                static_assert(std::is_same_v<float, T> || std::is_same_v<double, T>);
                *first++ = '0';
                *first++ = 'x';
                auto end = std::to_chars(first, last, value, std::chars_format::hex).ptr;
                // Put the leading '-' in front of '0x'
                if (*first == '-')
                {
                    std::rotate(buffer, first, first + 1);
                }
                write(std::string_view{ buffer, static_cast<size_t>(end - buffer) });
            }
        }

        void write_code(std::string_view const& value)
        {
            for (auto&& c : value)
            {
                if (c == '.')
                {
                    write("::");
                }
                else
                {
                    write(c);
                }
            }
        }

        void write(ConstantType type)
        {
            switch (type)
            {
            case ConstantType::UInt8:
                write("uint8_t");
                break;
            case ConstantType::Int8:
                write("int8_t");
                break;
            case ConstantType::UInt16:
                write("uint16_t");
                break;
            case ConstantType::Int16:
                write("int16_t");
                break;
            case ConstantType::UInt32:
                write("uint32_t");
                break;
            case ConstantType::Int32:
                write("int32_t");
                break;
            case ConstantType::UInt64:
                write("uint64_t");
                break;
            case ConstantType::Int64:
                write("int64_t");
                break;
            case ConstantType::Float32:
                write("float");
                break;
            case ConstantType::Float64:
                write("double");
                break;
            case ConstantType::String:
                write("wchar_t const*");
                break;
            default:
                throw_invalid("Invalid ConstantType");
                break;
            }
        }

        void write(Constant const& value)
        {
            switch (value.Type())
            {
            case ConstantType::UInt8:
                write_value(value.ValueUInt8());
                break;
            case ConstantType::Int8:
                write_value(value.ValueInt8());
                break;
            case ConstantType::UInt16:
                write_value(value.ValueUInt16());
                break;
            case ConstantType::Int16:
                write_value(value.ValueInt16());
                break;
            case ConstantType::Int32:
                write_value(value.ValueInt32());
                break;
            case ConstantType::UInt32:
                write_value(value.ValueUInt32());
                break;
            case ConstantType::Int64:
                write_value(value.ValueInt64());
                break;
            case ConstantType::UInt64:
                write_value(value.ValueUInt64());
                break;
            case ConstantType::Float32:
                write_value(value.ValueFloat32());
                break;
            case ConstantType::Float64:
                write_value(value.ValueFloat64());
                break;
            case ConstantType::String:
                write(R"(L"%")", value.ValueString());
                break;
            default:
                throw std::invalid_argument("Unexpected constant type");
            }
        }

        void write(std::u16string_view const& str)
        {
            auto const data = reinterpret_cast<wchar_t const*>(str.data());
            auto const size = ::WideCharToMultiByte(CP_UTF8, 0, data, static_cast<int32_t>(str.size()), nullptr, 0, nullptr, nullptr);
            if (size == 0)
            {
                return;
            }
            std::string result(size, '?');
            ::WideCharToMultiByte(CP_UTF8, 0, data, static_cast<int32_t>(str.size()), result.data(), size, nullptr, nullptr);
            write(result);
        }

        void write(TypeDef const& type)
        {
            add_depends(type);
            if (is_nested(type))
            {
                write(type.TypeName());
            }
            else
            {
                if (full_namespace)
                {
                    write("win32::");
                }
                write("@::%", type.TypeNamespace(), type.TypeName());
            }
        }

        void write(TypeRef const& type)
        {
            if (type.TypeNamespace() == "System" && type.TypeName() == "Guid")
            {
                write("::win32::guid");
            }
            else if (is_nested(type))
            {
                write(type.TypeName());
            }
            else
            {
                auto type_def = find(type);
                if (type_def)
                {
                    write(type_def);
                    return;
                }
                add_extern_depends(type);
                if (full_namespace)
                {
                    write("win32::");
                }
                write("@::%", type.TypeNamespace(), type.TypeName());
            }
        }

        void write(coded_index<TypeDefOrRef> const& type)
        {
            switch (type.type())
            {
            case TypeDefOrRef::TypeDef:
                write(type.TypeDef());
                break;
            case TypeDefOrRef::TypeRef:
                write(type.TypeRef());
                break;
            default:
                throw std::invalid_argument("Unexpected TypeDefOrRef type");
            }
        }

        void write(TypeSig const& signature)
        {
            call(signature.Type(),
                [&](ElementType type)
                {
                    switch (type)
                    {
                    case ElementType::Boolean:
                        write("bool");
                        break;
                    case ElementType::Char:
                        write("char");
                        break;
                    case ElementType::I1:
                        write("int8_t");
                        break;
                    case ElementType::U1:
                        write("uint8_t");
                        break;
                    case ElementType::I2:
                        write("int16_t");
                        break;
                    case ElementType::U2:
                        write("uint16_t");
                        break;
                    case ElementType::I4:
                        write("int32_t");
                        break;
                    case ElementType::U4:
                        write("uint32_t");
                        break;
                    case ElementType::I8:
                        write("int64_t");
                        break;
                    case ElementType::U8:
                        write("uint64_t");
                        break;
                    case ElementType::R4:
                        write("float");
                        break;
                    case ElementType::R8:
                        write("double");
                        break;
                    case ElementType::U:
                        write("size_t");
                        break;
                    case ElementType::I:
                        write("intptr_t");
                        break;
                    case ElementType::Void:
                        write("void");
                        break;
                    default:
                        throw std::invalid_argument("Invalid TypeSig type");
                    }
                    for (int i = 0; i < signature.ptr_count(); ++i)
                    {
                        write('*');
                    }
                },
                [&](coded_index<TypeDefOrRef> const& type)
                {
                    write(type);
                    for (int i = 0; i < signature.ptr_count(); ++i)
                    {
                        write('*');
                    }
                    if (signature.element_type() == ElementType::Class)
                    {
                        write('*');
                    }
                },
                [&](auto&& type)
                {
                    throw std::invalid_argument("Invalid TypeSig type");
                });
        }

        void write(RetTypeSig const& value)
        {
            if (value)
            {
                write(value.Type());
            }
            else
            {
                write("void");
            }
        }

        void save_header(char impl = 0)
        {
            auto filename{ settings.output_folder + "win32/" };
            if (impl)
            {
                filename += "impl/";
            }

            filename += type_namespace;

            if (impl)
            {
                filename += '.';
                filename += impl;
            }

            filename += ".h";
            flush_to_file(filename);
        }

        void save_cpp()
        {
            auto filename{ settings.output_folder + "win32/impl/" };
            filename += type_namespace;
            filename += ".cpp";
            flush_to_file(filename);
        }
    };
}
