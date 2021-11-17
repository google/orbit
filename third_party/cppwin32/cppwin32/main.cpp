#include <winmd_reader.h>
#include "cmd_reader.h"
#include "settings.h"
#include "task_group.h"
#include "text_writer.h"
#include "type_dependency_graph.h"
#include "type_writers.h"
#include "code_writers.h"
#include "file_writers.h"
#include <unordered_set>

using namespace std::filesystem;

namespace cppwin32
{
    settings_type settings;

    struct usage_exception {};

    static constexpr option options[]
    {
        { "input", 0, option::no_max, "<spec>", "Windows metadata to include in projection" },
        { "reference", 0, option::no_max, "<spec>", "Windows metadata to reference from projection" },
        { "output", 0, 1, "<path>", "Location of generated projection and component templates" },
        { "verbose", 0, 0, {}, "Show detailed progress information" },
        { "pch", 0, 1, "<name>", "Specify name of precompiled header file (defaults to pch.h)" },
        { "include", 0, option::no_max, "<prefix>", "One or more prefixes to include in input" },
        { "exclude", 0, option::no_max, "<prefix>", "One or more prefixes to exclude from input" },
        { "base", 0, 0, {}, "Generate base.h unconditionally" },
        { "help", 0, option::no_max, {}, "Show detailed help with examples" },
        { "?", 0, option::no_max, {}, {} },
        { "library", 0, 1, "<prefix>", "Specify library prefix (defaults to win32)" },
        { "filter" }, // One or more prefixes to include in input (same as -include)
        { "license", 0, 0 }, // Generate license comment
        { "brackets", 0, 0 }, // Use angle brackets for #includes (defaults to quotes)
    };


    static void print_usage(writer& w)
    {
        static auto printColumns = [](writer& w, std::string_view const& col1, std::string_view const& col2)
        {
            w.write_printf("  %-20s%s\n", col1.data(), col2.data());
        };

        static auto printOption = [](writer& w, option const& opt)
        {
            if (opt.desc.empty())
            {
                return;
            }
            printColumns(w, w.write_temp("-% %", opt.name, opt.arg), opt.desc);
        };

        auto format = R"(
C++/Win32 v%
Copyright (c) Microsoft Corporation. All rights reserved.

  cppwin32.exe [options...]

Options:

%  ^@<path>             Response file containing command line options

Where <spec> is one or more of:

  path                Path to winmd file or recursively scanned folder
)";
        w.write(format, CPPWIN32_VERSION_STRING, bind_each(printOption, options));
    }

    static void process_args(reader const& args)
    {
        settings.verbose = args.exists("verbose");
        settings.fastabi = args.exists("fastabi");

        settings.input = args.files("input", database::is_database);
        settings.reference = args.files("reference", database::is_database);

        settings.component = args.exists("component");
        settings.base = args.exists("base");

        settings.license = args.exists("license");
        settings.brackets = args.exists("brackets");

        std::filesystem::path output_folder = args.value("output");
        std::filesystem::create_directories(output_folder / "win32/impl");
        settings.output_folder = std::filesystem::canonical(output_folder).string();
        settings.output_folder += '\\';

        for (auto&& include : args.values("include"))
        {
            settings.include.insert(include);
        }

        for (auto&& include : args.values("filter"))
        {
            settings.include.insert(include);
        }

        for (auto&& exclude : args.values("exclude"))
        {
            settings.exclude.insert(exclude);
        }

        if (settings.component)
        {
            settings.component_overwrite = args.exists("overwrite");
            settings.component_name = args.value("name");

            if (settings.component_name.empty())
            {
                // For compatibility with C++/WinRT 1.0, the component_name defaults to the *first*
                // input, hence the use of values() here that will return the args in input order.

                auto& values = args.values("input");

                if (!values.empty())
                {
                    settings.component_name = path(values[0]).filename().replace_extension().string();
                }
            }

            settings.component_pch = args.value("pch", "pch.h");
            settings.component_prefix = args.exists("prefix");
            settings.component_lib = args.value("library", "win32");
            settings.component_opt = args.exists("optimize");
            settings.component_ignore_velocity = args.exists("ignore_velocity");

            if (settings.component_pch == ".")
            {
                settings.component_pch.clear();
            }

            auto component = args.value("component");

            if (!component.empty())
            {
                create_directories(component);
                settings.component_folder = canonical(component).string();
                settings.component_folder += '\\';
            }
        }
    }

    static auto get_files_to_cache()
    {
        std::vector<std::string> files;
        files.insert(files.end(), settings.input.begin(), settings.input.end());
        files.insert(files.end(), settings.reference.begin(), settings.reference.end());
        return files;
    }

    static int run(int const argc, char* argv[])
    {
        int result{};
        writer w;

        try
        {
            auto const start_time = std::chrono::high_resolution_clock::now();

            reader args{ argc, argv, options };

            if (!args || args.exists("help") || args.exists("?"))
            {
                throw usage_exception{};
            }

            process_args(args);
            cache c{ get_files_to_cache() };
            task_group group;

            w.flush_to_console();

            for (auto&& [ns, members] : c.namespaces())
            {
                group.add([&, &ns = ns, &members = members]
                    {
                        write_namespace_0_h(ns, members);
                        write_namespace_1_h(ns, members);
                        write_namespace_2_h(ns, members);
                        write_namespace_h(ns, members);
                    });
            }
            group.add([&c] { write_complex_structs_h(c); });
            group.add([&c] { write_complex_interfaces_h(c); });

            std::filesystem::copy_file("base.h", settings.output_folder + "win32/" + "base.h", std::filesystem::copy_options::overwrite_existing);
        }
        catch (usage_exception const&)
        {
            print_usage(w);
        }
        catch (std::exception const& e)
        {
            w.write("cppwin32 : error %\n", e.what());
            result = 1;
        }

        w.flush_to_console(result == 0);
        return result;
    }
}

int main(int const argc, char* argv[])
{
    cppwin32::run(argc, argv);

    //// Hack prototype command line args for now
    //o.input = argv[1];
    //o.output_folder = std::filesystem::canonical(argv[2]);
    //std::filesystem::create_directories(o.output_folder);

    //winmd::reader::cache c{ o.input };

    //std::set<std::string_view> raii_helpers;

    //for (auto const& [ns, members] : c.namespaces())
    //{
    //    if (ns.empty())
    //    {
    //        continue;
    //    }
    //    writer w;
    //    w.write("#include \"base.h\"\n");
    //    w.type_namespace = ns;
    //    {
    //        auto wrap = wrap_type_namespace(w, ns);

    //        w.write("#pragma region enums\n");
    //        w.write_each<write_enum>(members.enums);
    //        w.write("#pragma endregion enums\n\n");

    //        w.write("#pragma region forward_declarations\n");
    //        w.write_each<write_forward>(members.structs);
    //        w.write_each<write_forward>(members.interfaces);
    //        w.write("#pragma endregion forward_declarations\n\n");

    //        w.write("#pragma region delegates\n");
    //        write_delegates(w, members.delegates);
    //        w.write("#pragma endregion delegates\n\n");
    //    }
    //    {
    //        auto wrap = wrap_impl_namespace(w);

    //        w.write("#pragma region guids\n");
    //        w.write_each<write_guid>(members.interfaces);
    //        w.write("#pragma endregion guids\n\n");

    //        //w.write("#pragma region consume\n");
    //        //w.write_each<write_consume>(members.interfaces);
    //        //w.write("#pragma endregion consume\n\n");
    //    }
    //    {
    //        auto wrap = wrap_type_namespace(w, ns);

    //        w.write("#pragma region structs\n");
    //        write_structs(w, members.structs);
    //        w.write("#pragma endregion structs\n\n");

    //        w.write("#pragma region interfaces\n");
    //        write_interfaces(w, members.interfaces);
    //        w.write("#pragma endregion interfaces\n\n");
    //    }
    //    {
    //        w.write("#pragma region abi_methods\n");
    //        w.write_each<write_class_abi>(members.classes);
    //        w.write("#pragma endregion abi_methods\n\n");
    //    }
    //    {
    //        auto wrap = wrap_type_namespace(w, ns);

    //        w.write("#pragma region raii_helpers\n");
    //        w.write_each<write_api_raii_helpers>(members.classes, raii_helpers);
    //        w.write("#pragma endregion raii_helpers\n\n");

    //        w.write("#pragma region methods\n");
    //        w.write_each<write_class>(members.classes);
    //        w.write("#pragma endregion methods\n\n");

    //        w.write("#pragma region enum_operators\n");
    //        w.write_each<write_enum_operators>(members.enums);
    //        w.write("#pragma endregion enum_operators\n\n");
    //    }
    //    {
    //        auto wrap = wrap_impl_namespace(w);

    //        //w.write("#pragma region consume_methods\n");
    //        //w.write_each<write_consume_definitions>(members.interfaces);
    //        //w.write("#pragma endregion consume_methods\n\n");
    //    }

    //    w.save_header(o.output_folder.string());

    //    std::filesystem::copy_file("base.h", o.output_folder / "base.h", std::filesystem::copy_options::overwrite_existing);
    //}
}
