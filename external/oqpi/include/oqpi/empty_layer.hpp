#pragma once

namespace oqpi {

    // Defines an empty layer for augmented interfaces
    template<typename>
    struct empty_layer {};

    // Determines if a given layer is empty: default case, not empty
    template<template<typename> typename _Layer>
    struct is_empty_layer : public std::false_type {};

    // Determines if a given layer is empty: empty!
    template<>
    struct is_empty_layer<empty_layer> : public std::true_type {};

} /*oqpi*/
