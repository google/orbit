// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <string>

#include "../compiler_check.hpp"
#include "../parser.hpp"

namespace krabs { namespace predicates {

    namespace adapters {

        /**
        * View adapter for counted_string strings
        */
        struct counted_string
        {
            using value_type = krabs::counted_string::value_type;
            using const_iterator = krabs::counted_string::const_iterator;

            collection_view<const_iterator> operator()(const property_info& propInfo) const
            {
                auto cs_ptr = reinterpret_cast<const krabs::counted_string*>(propInfo.pPropertyIndex_);
                return krabs::view(cs_ptr->string(), cs_ptr->length());
            }
        };
        
        /**
         * View adapter for fixed width and null-terminated strings
         */
        template <typename ElemT>
        struct generic_string
        {
            using value_type = ElemT;
            using const_iterator = const value_type*;

            collection_view<const_iterator> operator()(const property_info& propInfo) const
            {
                auto pString = reinterpret_cast<const value_type*>(propInfo.pPropertyIndex_);
                auto length = get_string_content_length(pString, propInfo.length_);

                return krabs::view(pString, length);
            }
        };

    } /* namespace adapters */

} /* namespace predicates */ } /* namespace krabs */