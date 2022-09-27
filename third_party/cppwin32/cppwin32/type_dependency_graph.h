#pragma once

#include <vector>
#include <winmd_reader.h>
#include "helpers.h"
// For tracking "hard" dependencies between types that require a definition, not a forward declaration

namespace cppwin32
{
    using namespace winmd::reader;
    struct type_dependency_graph
    {
        type_dependency_graph() = default;
        explicit type_dependency_graph(std::string_view type_namespace)
            : type_namespace(type_namespace)
        {}

        enum class walk_state
        {
            not_started,
            walking,
            complete
        };

        struct node
        {
            std::vector<TypeDef> edges;
            walk_state state = walk_state::not_started;

            void add_edge(TypeDef const& edge)
            {
                if (std::find(edges.begin(), edges.end(), edge) == edges.end())
                {
                    edges.push_back(edge);
                }
            }
        };

        std::map<TypeDef, node> graph;
        using value_type = std::map<TypeDef, node>::value_type;
        std::string_view type_namespace;

        template <typename Callback>
        void walk_graph(Callback c)
        {
            auto eligible = [](value_type const& v) { return v.second.state == walk_state::not_started; };
            for (auto it = std::find_if(graph.begin(), graph.end(), eligible)
                ; it != graph.end()
                ; it = std::find_if(graph.begin(), graph.end(), eligible))
            {
                visit(*it, c);
            }
        }

        void reset_walk_state()
        {
            for (auto& value : graph)
            {
                value.second.state = walk_state::not_started;
            }
        }

        void add_struct(TypeDef const& type)
        {
            auto [it, inserted] = graph.insert({ type, {} });
            if (!inserted) return;

            for (auto&& field : type.FieldList())
            {
                auto const& signature = field.Signature();
                if (auto const field_type = std::get_if<coded_index<TypeDefOrRef>>(&signature.Type().Type()))
                {
                    if (signature.Type().ptr_count() == 0 || is_nested(*field_type))
                    {
                        auto field_type_def = find(*field_type);
                        if (field_type_def && get_category(field_type_def) == category::struct_type)
                        {
                            it->second.add_edge(field_type_def);
                            add_struct(field_type_def);
                        }
                    }
                }
            }
        }

        void add_delegate(TypeDef const& type)
        {
            auto [it, inserted] = graph.insert({ type, {} });
            if (!inserted) return;

            method_signature method_signature{ get_delegate_method(type) };
            auto add_param = [this, current = it](TypeSig const& type)
            {
                auto index = std::get_if<coded_index<TypeDefOrRef>>(&type.Type());
                if (index)
                {
                    auto param_type_def = find(*index);
                    if (param_type_def && get_category(param_type_def) == category::delegate_type)
                    {
                        if (type_namespace.empty() || type_namespace == param_type_def.TypeName())
                        {
                            current->second.add_edge(param_type_def);
                            add_delegate(param_type_def);
                        }
                    }
                }
            };

            const RetTypeSig& return_signature = method_signature.return_signature();
            // "RetTypeSig" has "operator bool()" that checks if "Type()" returns a valid type.
            // Without checking, calling "Type()" could be dereferencing a std::nullopt.
            if(return_signature) {
                add_param(return_signature.Type());
            }

            for (auto const& [param, param_sig] : method_signature.params())
            {
                add_param(param_sig->Type());
            }
        }

        void add_interface(TypeDef const& type)
        {
            auto [it, inserted] = graph.insert({ type, {} });
            if (!inserted) return;

            auto const base_index = get_base_interface(type);
            if (base_index)
            {
                auto const base_type = find(base_index);
                if (base_type
                    && (type_namespace.empty() || type_namespace == base_type.TypeNamespace()))
                {
                    it->second.add_edge(base_type);
                    add_interface(base_type);
                }
            }
        }

    private:

        template<typename Callback>
        void visit(value_type& v, Callback c)
        {
#ifdef _DEBUG
            auto type_name = v.first.TypeName();
#endif
            if (v.second.state == walk_state::complete) return;
            if (v.second.state == walk_state::walking) throw std::invalid_argument("Cyclic dependency graph encountered at type " + std::string(v.first.TypeNamespace()) + "." + std::string(v.first.TypeName()));

            v.second.state = walk_state::walking;
            for (auto&& edge : v.second.edges)
            {
                auto it = graph.find(edge);
                XLANG_ASSERT(it != graph.end());
                visit(*it, c);
            }
            v.second.state = walk_state::complete;
            c(v.first);
        }
    };
}

