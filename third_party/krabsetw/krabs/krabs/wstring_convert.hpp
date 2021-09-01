// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <windows.h>

namespace krabs {

    /** <summary>
      * Converts std::wstring argument to std::string using UTF-8 codepage
      * Returns empty string if translation fails or input string is empty
      * </summary>
      */
    inline std::string from_wstring(const std::wstring& wstr, UINT codePage = CP_UTF8)
    {
        if (wstr.empty())
            return {};

        const auto requiredLen = WideCharToMultiByte(codePage, 0, wstr.data(), static_cast<int>(wstr.size()), 
            nullptr, 0, nullptr, nullptr);
        if (0 == requiredLen)
            return {};

        std::string result(requiredLen, 0);
        const auto convertedLen = WideCharToMultiByte(codePage, 0, wstr.data(), static_cast<int>(wstr.size()),
            &result[0], requiredLen, nullptr, nullptr);
        if (0 == convertedLen)
            return {};

        return result;
    }
}
