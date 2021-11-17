#pragma once

namespace cppwin32
{
    static void write_namespace_0_h(std::string_view const& ns, cache::namespace_members const& members)
    {
        writer w;
        w.type_namespace = ns;

        {
            auto wrap = wrap_type_namespace(w, ns);

            w.write("#pragma region enums\n");
            w.write_each<write_enum>(members.enums);
            w.write("#pragma endregion enums\n\n");

            w.write("#pragma region forward_declarations\n");
            w.write_each<write_forward>(members.structs);
            w.write_each<write_forward>(members.interfaces);
            w.write("#pragma endregion forward_declarations\n\n");

            w.write("#pragma region delegates\n");
            write_delegates(w, members.delegates);
            w.write("#pragma endregion delegates\n\n");
        }
        {
            auto wrap = wrap_impl_namespace(w);

            w.write("#pragma region guids\n");
            w.write_each<write_guid>(members.interfaces);
            w.write("#pragma endregion guids\n\n");
        }

        write_close_file_guard(w);
        w.swap();

        write_preamble(w);
        write_open_file_guard(w, ns, '0');

        for (auto&& depends : w.depends)
        {
            auto guard = wrap_type_namespace(w, depends.first);
            w.write_each<write_forward>(depends.second);
        }

        w.save_header('0');
    }

    static void write_namespace_1_h(std::string_view const& ns, cache::namespace_members const& members)
    {
        writer w;
        w.type_namespace = ns;
        
        w.write("#include \"win32/impl/complex_structs.h\"\n");

        {
            auto wrap = wrap_type_namespace(w, ns);

            w.write("#pragma region interfaces\n");
            //write_interfaces(w, members.interfaces);
            w.write("#pragma endregion interfaces\n\n");
        }

        write_close_file_guard(w);
        w.swap();
        write_preamble(w);
        write_open_file_guard(w, ns, '1');

        for (auto&& depends : w.depends)
        {
            w.write_depends(depends.first, '0');
        }

        w.write_depends(w.type_namespace, '0');
        w.save_header('1');
    }

    static void write_namespace_2_h(std::string_view const& ns, cache::namespace_members const& members)
    {
        writer w;
        w.type_namespace = ns;

        w.write("#include \"win32/impl/complex_interfaces.h\"\n");

        {
            // No namespace
            w.write("#pragma region abi_methods\n");
            w.write_each<write_class_abi>(members.classes);
            w.write("#pragma endregion abi_methods\n\n");
        }

        write_close_file_guard(w);
        w.swap();
        write_preamble(w);
        write_open_file_guard(w, ns, '2');

        w.write_depends(w.type_namespace, '1');
        // Workaround for https://github.com/microsoft/cppwin32/issues/2
        for (auto&& extern_depends : w.extern_depends)
        {
            auto guard = wrap_type_namespace(w, extern_depends.first);
            w.write_each<write_extern_forward>(extern_depends.second);
        }
        w.save_header('2');
    }

    static void write_namespace_h(std::string_view const& ns, cache::namespace_members const& members)
    {
        writer w;
        w.type_namespace = ns;
        {
            auto wrap = wrap_type_namespace(w, ns);

            w.write("#pragma region methods\n");
            w.write_each<write_class>(members.classes);
            w.write("#pragma endregion methods\n\n");
        }

        write_close_file_guard(w);
        w.swap();
        write_preamble(w);
        write_open_file_guard(w, ns);
        write_version_assert(w);

        w.write_depends(w.type_namespace, '2');
        // Workaround for https://github.com/microsoft/cppwin32/issues/2
        for (auto&& extern_depends : w.extern_depends)
        {
            auto guard = wrap_type_namespace(w, extern_depends.first);
            w.write_each<write_extern_forward>(extern_depends.second);
        }
        w.save_header();
    }

    static void write_complex_structs_h(cache const& c)
    {
        writer w;

        type_dependency_graph graph;
        for (auto&& [ns, members] : c.namespaces())
        {
            for (auto&& s : members.structs)
            {
                graph.add_struct(s);
            }
        }

        graph.walk_graph([&](TypeDef const& type)
            {
                if (!is_nested(type))
                {
                    auto guard = wrap_type_namespace(w, type.TypeNamespace());
                    write_struct(w, type);
                }
            });

        write_close_file_guard(w);
        w.swap();

        write_preamble(w);
        write_open_file_guard(w, "complex_structs");

        for (auto&& depends : w.depends)
        {
            w.write_depends(depends.first, '0');
        }

        w.flush_to_file(settings.output_folder + "win32/impl/complex_structs.h");
    }

    static void write_complex_interfaces_h(cache const& c)
    {
        writer w;

        type_dependency_graph graph;
        for (auto&& [ns, members] : c.namespaces())
        {
            for (auto&& s : members.interfaces)
            {
                graph.add_interface(s);
            }
        }

        graph.walk_graph([&](TypeDef const& type)
            {
                if (!is_nested(type))
                {
                    auto guard = wrap_type_namespace(w, type.TypeNamespace());
                    write_interface(w, type);
                }
            });

        write_close_file_guard(w);
        w.swap();

        write_preamble(w);
        write_open_file_guard(w, "complex_interfaces");

        for (auto&& depends : w.depends)
        {
            w.write_depends(depends.first, '1');
        }
        // Workaround for https://github.com/microsoft/cppwin32/issues/2
        for (auto&& extern_depends : w.extern_depends)
        {
            auto guard = wrap_type_namespace(w, extern_depends.first);
            w.write_each<write_extern_forward>(extern_depends.second);
        }

        w.flush_to_file(settings.output_folder + "win32/impl/complex_interfaces.h");
    }
}