# Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
# under NSF AWARD 1414736 and by the respective contributors.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

include("${CMAKE_CURRENT_LIST_DIR}/CLI11Targets.cmake")

# Give the module target its short name. ALIAS of an imported target needs
# CMake 3.18+; modules need 3.28+ anyway, but keep this file loadable on the
# project minimum (3.14) when the module target is absent.
if(TARGET CLI11::CLI11_Module
   AND NOT TARGET CLI11::Module
   AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
  add_library(CLI11::Module ALIAS CLI11::CLI11_Module)
endif()
