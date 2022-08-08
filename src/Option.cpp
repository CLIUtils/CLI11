// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <CLI/impl/Option_inl.hpp>
#include <CLI/App.hpp>

template class CLI::OptionBase<CLI::OptionDefaults>;
template class CLI::OptionBase<CLI::Option>;

template void CLI::OptionBase<CLI::OptionDefaults>::copy_to(CLI::Option *) const;
