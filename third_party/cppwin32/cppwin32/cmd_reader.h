#pragma once

#include <cassert>
#include <array>
#include <limits>
#include <cstdint>
#include <string>
#include <string_view>
#include <map>
#include <vector>
#include <set>
#include <filesystem>
#include <fstream>
#include <regex>
#include <Windows.h>
#include <shlwapi.h>
#include <XmlLite.h>

namespace cppwin32
{
    struct registry_key
    {
        HKEY handle{};

        registry_key(registry_key const&) = delete;
        registry_key& operator=(registry_key const&) = delete;

        explicit registry_key(HKEY handle) :
            handle(handle)
        {
        }

        ~registry_key() noexcept
        {
            if (handle)
            {
                RegCloseKey(handle);
            }
        }
    };

    template <typename T>
    struct com_ptr
    {
        T* ptr{};

        com_ptr(com_ptr const&) = delete;
        com_ptr& operator=(com_ptr const&) = delete;

        com_ptr() noexcept = default;

        ~com_ptr() noexcept
        {
            if (ptr)
            {
                ptr->Release();
            }
        }

        auto operator->() const noexcept
        {
            return ptr;
        }
    };

    static void check_xml(HRESULT result)
    {
        if (result < 0)
        {
            throw std::invalid_argument("Could not read the Windows SDK's Platform.xml");
        }
    }

    inline void add_files_from_xml(
        std::set<std::string>& files,
        std::string const& sdk_version,
        std::filesystem::path const& xml_path,
        std::filesystem::path const& sdk_path)
    {
        com_ptr<IStream> stream;

        check_xml(SHCreateStreamOnFileW(
            xml_path.c_str(),
            STGM_READ, &stream.ptr));

        com_ptr<IXmlReader> reader;

        check_xml(CreateXmlReader(
            __uuidof(IXmlReader),
            reinterpret_cast<void**>(&reader.ptr),
            nullptr));

        check_xml(reader->SetInput(stream.ptr));
        XmlNodeType node_type = XmlNodeType_None;

        while (S_OK == reader->Read(&node_type))
        {
            if (node_type != XmlNodeType_Element)
            {
                continue;
            }

            wchar_t const* value{ nullptr };
            check_xml(reader->GetLocalName(&value, nullptr));

            if (0 != wcscmp(value, L"ApiContract"))
            {
                continue;
            }

            auto path = sdk_path;
            path /= L"References";
            path /= sdk_version;

            check_xml(reader->MoveToAttributeByName(L"name", nullptr));
            check_xml(reader->GetValue(&value, nullptr));
            path /= value;

            check_xml(reader->MoveToAttributeByName(L"version", nullptr));
            check_xml(reader->GetValue(&value, nullptr));
            path /= value;

            check_xml(reader->MoveToAttributeByName(L"name", nullptr));
            check_xml(reader->GetValue(&value, nullptr));
            path /= value;

            path += L".winmd";
            files.insert(path.string());
        }
    }

    inline registry_key open_sdk()
    {
        HKEY key;

        if (0 != RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots",
            0,
            // https://task.ms/29349404 - The SDK sometimes stores the 64 bit location into KitsRoot10 which is wrong,
            // this breaks 64-bit cppwin32.exe, so work around this by forcing to use the WoW64 hive.
            KEY_READ | KEY_WOW64_32KEY,
            &key))
        {
            throw std::invalid_argument("Could not find the Windows SDK in the registry");
        }

        return registry_key{ key };
    }

    inline std::filesystem::path get_sdk_path()
    {
        auto key = open_sdk();

        DWORD path_size = 0;

        if (0 != RegQueryValueExW(
            key.handle,
            L"KitsRoot10",
            nullptr,
            nullptr,
            nullptr,
            &path_size))
        {
            throw std::invalid_argument("Could not find the Windows SDK path in the registry");
        }

        std::wstring root((path_size / sizeof(wchar_t)) - 1, L'?');

        RegQueryValueExW(
            key.handle,
            L"KitsRoot10",
            nullptr,
            nullptr,
            reinterpret_cast<BYTE*>(root.data()),
            &path_size);

        return root;
    }

    inline std::string get_module_path()
    {
        std::string path(100, '?');
        DWORD actual_size{};

        while (true)
        {
            actual_size = GetModuleFileNameA(nullptr, path.data(), 1 + static_cast<uint32_t>(path.size()));

            if (actual_size < 1 + path.size())
            {
                path.resize(actual_size);
                break;
            }
            else
            {
                path.resize(path.size() * 2, '?');
            }
        }

        return path;
    }

    inline std::string get_sdk_version()
    {
        auto module_path = get_module_path();
        std::regex rx(R"(((\d+)\.(\d+)\.(\d+)\.(\d+)))");
        std::cmatch match;
        auto sdk_path = get_sdk_path();

        if (std::regex_search(module_path.c_str(), match, rx))
        {
            auto path = sdk_path / "Platforms\\UAP" / match[1].str() / "Platform.xml";

            if (std::filesystem::exists(path))
            {
                return match[1].str();
            }
        }

        auto key = open_sdk();
        uint32_t index{};
        std::array<char, 100> subkey;
        std::array<unsigned long, 4> version_parts{};
        std::string result;

        while (0 == RegEnumKeyA(key.handle, index++, subkey.data(), static_cast<uint32_t>(subkey.size())))
        {
            if (!std::regex_match(subkey.data(), match, rx))
            {
                continue;
            }

            auto path = sdk_path / "Platforms\\UAP" / match[1].str() / "Platform.xml";
            if (!std::filesystem::exists(path))
            {
                continue;
            }

            char* next_part = subkey.data();
            bool force_newer = false;

            for (size_t i = 0; ; ++i)
            {
                auto version_part = strtoul(next_part, &next_part, 10);

                if ((version_part < version_parts[i]) && !force_newer)
                {
                    break;
                }
                else if (version_part > version_parts[i])
                {
                    // E.g. ensure something like '2.1' is considered newer than '1.2'
                    force_newer = true;
                }

                version_parts[i] = version_part;

                if (i == std::size(version_parts) - 1)
                {
                    result = subkey.data();
                    break;
                }

                if (!next_part)
                {
                    break;
                }

                ++next_part;
            }
        }

        if (result.empty())
        {
            throw std::invalid_argument("Could not find the Windows SDK");
        }

        return result;
    }

    [[noreturn]] inline void throw_invalid(std::string const& message)
    {
        throw std::invalid_argument(message);
    }

    template <typename...T>
    [[noreturn]] inline void throw_invalid(std::string message, T const&... args)
    {
        (message.append(args), ...);
        throw std::invalid_argument(message);
    }

    struct option
    {
        static constexpr uint32_t no_min = 0;
        static constexpr uint32_t no_max = UINT_MAX;

        std::string_view name;
        uint32_t min{ no_min };
        uint32_t max{ no_max };
        std::string_view arg{};
        std::string_view desc{};
    };

    struct reader
    {
        template <typename C, typename V, size_t numOptions>
        reader(C const argc, V const argv, const option(&options)[numOptions])
        {
#ifdef _DEBUG
            {
                std::set<std::string_view> unique;

                for (auto&& option : options)
                {
                    // If this assertion fails it means there are duplicate options.
                    assert(unique.insert(option.name).second);
                }
            }
#endif

            if (argc < 2)
            {
                return;
            }

            auto last{ std::end(options) };

            for (C i = 1; i < argc; ++i)
            {
                extract_option(argv[i], options, last);
            }

            for (auto&& option : options)
            {
                auto args = m_options.find(option.name);
                std::size_t const count = args == m_options.end() ? 0 : args->second.size();

                if (option.min == 0 && option.max == 0 && count > 0)
                {
                    throw_invalid("Option '", option.name, "' does not accept a value");
                }
                else if (option.max == option.min && count != option.max)
                {
                    throw_invalid("Option '", option.name, "' requires exactly ", std::to_string(option.max), " value(s)");
                }
                else if (count < option.min)
                {
                    throw_invalid("Option '", option.name, "' requires at least ", std::to_string(option.min), " value(s)");
                }
                else if (count > option.max)
                {
                    throw_invalid("Option '", option.name, "' accepts at most ", std::to_string(option.max), " value(s)");
                }
            }
        }

        explicit operator bool() const noexcept
        {
            return !m_options.empty();
        }

        bool exists(std::string_view const& name) const noexcept
        {
            return m_options.count(name);
        }

        auto const& values(std::string_view const& name) const noexcept
        {
            auto result = m_options.find(name);

            if (result == m_options.end())
            {
                static std::vector<std::string> empty{};
                return empty;
            }

            return result->second;
        }

        auto value(std::string_view const& name, std::string_view const& default_value = {}) const
        {
            auto result = m_options.find(name);

            if (result == m_options.end() || result->second.empty())
            {
                return std::string{ default_value };
            }

            return result->second.front();
        }

        template <typename F>
        auto files(std::string_view const& name, F directory_filter) const
        {
            std::set<std::string> files;

            auto add_directory = [&](auto&& path)
            {
                for (auto&& file : std::filesystem::directory_iterator(path))
                {
                    if (std::filesystem::is_regular_file(file))
                    {
                        auto filename = file.path().string();

                        if (directory_filter(filename))
                        {
                            files.insert(filename);
                        }
                    }
                }
            };

            for (auto&& path : values(name))
            {
                if (std::filesystem::is_directory(path))
                {
                    add_directory(std::filesystem::canonical(path));
                    continue;
                }

                if (std::filesystem::is_regular_file(path))
                {
                    files.insert(std::filesystem::canonical(path).string());
                    continue;
                }
                if (path == "local")
                {
                    std::array<char, 260> local{};
#ifdef _WIN64
                    ExpandEnvironmentStringsA("%windir%\\System32\\WinMetadata", local.data(), static_cast<uint32_t>(local.size()));
#else
                    ExpandEnvironmentStringsA("%windir%\\SysNative\\WinMetadata", local.data(), static_cast<uint32_t>(local.size()));
#endif
                    add_directory(local.data());
                    continue;
                }

                std::string sdk_version;

                if (path == "sdk" || path == "sdk+")
                {
                    sdk_version = get_sdk_version();
                }
                else
                {
                    std::regex rx(R"(((\d+)\.(\d+)\.(\d+)\.(\d+))\+?)");
                    std::smatch match;

                    if (std::regex_match(path, match, rx))
                    {
                        sdk_version = match[1].str();
                    }
                }

                throw_invalid("Path '", path, "' is not a file or directory");
            }

            return files;
        }

        auto files(std::string_view const& name) const
        {
            return files(name, [](auto&&) {return true; });
        }

    private:

        inline bool starts_with(std::string_view const& value, std::string_view const& match) noexcept
        {
            return 0 == value.compare(0, match.size(), match);
        }

        template<typename O>
        auto find(O const& options, std::string_view const& arg)
        {
            for (auto current = std::begin(options); current != std::end(options); ++current)
            {
                if (starts_with(current->name, arg))
                {
                    return current;
                }
            }

            return std::end(options);
        }

        std::map<std::string_view, std::vector<std::string>> m_options;

        template<typename O, typename L>
        void extract_option(std::string_view arg, O const& options, L& last)
        {
            if (arg[0] == '-' || arg[0] == '/')
            {
                arg.remove_prefix(1);
                last = find(options, arg);

                if (last == std::end(options))
                {
                    throw_invalid("Option '-", arg, "' is not supported");
                }

                m_options.try_emplace(last->name);
            }
            else if (arg[0] == '@')
            {
                arg.remove_prefix(1);
                extract_response_file(arg, options, last);
            }
            else if (last == std::end(options))
            {
                throw_invalid("Value '", arg, "' is not supported");
            }
            else
            {
                m_options[last->name].push_back(std::string{ arg });
            }
        }

        template<typename O, typename L>
        void extract_response_file(std::string_view const& arg, O const& options, L& last)
        {
            std::filesystem::path response_path{ std::string{ arg } };
            std::string extension = response_path.extension().generic_string();
            std::transform(extension.begin(), extension.end(), extension.begin(),
                [](auto c) { return static_cast<unsigned char>(::tolower(c)); });

            // Check if misuse of @ prefix, so if directory or metadata file instead of response file.
            if (is_directory(response_path) || extension == ".winmd")
            {
                throw_invalid("'@' is reserved for response files");
            }
            std::string line_buf;
            std::ifstream response_file(absolute(response_path));
            while (getline(response_file, line_buf))
            {
                size_t argc = 0;
                std::vector<std::string> argv;

                parse_command_line(line_buf.data(), argv, &argc);

                for (size_t i = 0; i < argc; i++)
                {
                    extract_option(argv[i], options, last);
                }
            }
        }

        template <typename Character>
        static void parse_command_line(Character* cmdstart, std::vector<std::string>& argv, size_t* argument_count)
        {

            std::string arg;
            bool copy_character;
            unsigned backslash_count;
            bool in_quotes;
            bool first_arg;

            Character* p = cmdstart;
            in_quotes = false;
            first_arg = true;
            *argument_count = 0;

            for (;;)
            {
                if (*p)
                {
                    while (*p == ' ' || *p == '\t')
                        ++p;
                }

                if (!first_arg)
                {
                    argv.emplace_back(arg);
                    arg.clear();
                    ++* argument_count;
                }

                if (*p == '\0')
                    break;

                for (;;)
                {
                    copy_character = true;

                    // Rules:
                    // 2N     backslashes   + " ==> N backslashes and begin/end quote
                    // 2N + 1 backslashes   + " ==> N backslashes + literal "
                    // N      backslashes       ==> N backslashes
                    backslash_count = 0;

                    while (*p == '\\')
                    {
                        ++p;
                        ++backslash_count;
                    }

                    if (*p == '"')
                    {
                        // if 2N backslashes before, start/end quote, otherwise
                        // copy literally:
                        if (backslash_count % 2 == 0)
                        {
                            if (in_quotes && p[1] == '"')
                            {
                                p++; // Double quote inside quoted string
                            }
                            else
                            {
                                // Skip first quote char and copy second:
                                copy_character = false;
                                in_quotes = !in_quotes;
                            }
                        }

                        backslash_count /= 2;
                    }

                    while (backslash_count--)
                    {
                        arg.push_back('\\');
                    }

                    if (*p == '\0' || (!in_quotes && (*p == ' ' || *p == '\t')))
                        break;

                    if (copy_character)
                    {
                        arg.push_back(*p);
                    }

                    ++p;
                }

                first_arg = false;
            }
        }
    };
}
