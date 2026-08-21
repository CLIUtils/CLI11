// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

// Build the full validator set into the library by default so consumers that
// define CLI11_ENABLE_EXTRA_VALIDATORS per-TU can still link. An explicit
// CMake-level choice (passed as a PUBLIC definition) is respected.
#if !defined(CLI11_ENABLE_EXTRA_VALIDATORS) && !defined(CLI11_DISABLE_EXTRA_VALIDATORS)
#define CLI11_ENABLE_EXTRA_VALIDATORS 1
#endif

// IWYU pragma: begin_keep

#include <CLI/impl/App_inl.hpp>
#include <CLI/impl/Argv_inl.hpp>
#include <CLI/impl/Completion_inl.hpp>
#include <CLI/impl/Config_inl.hpp>
#include <CLI/impl/Encoding_inl.hpp>
#include <CLI/impl/ExtraValidators_inl.hpp>
#include <CLI/impl/Formatter_inl.hpp>
#include <CLI/impl/Option_inl.hpp>
#include <CLI/impl/Split_inl.hpp>
#include <CLI/impl/StringTools_inl.hpp>
#include <CLI/impl/TypeTools_inl.hpp>
#include <CLI/impl/Validators_inl.hpp>

// IWYU pragma: end_keep

namespace CLI {
// Library consumers only see the declarations of these member templates, so
// the compiled library must provide the instantiations. Implicit instantiation
// emits only discardable weak symbols, which LTO can remove from a shared
// library; instantiate explicitly so the symbols always survive.
template Option *Option::ignore_case<App>(bool);
template Option *Option::ignore_underscore<App>(bool);
}  // namespace CLI
