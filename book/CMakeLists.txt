# Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
# under NSF AWARD 1414736 and by the respective contributors.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

set(book_sources README.md SUMMARY.md)

file(
  GLOB book_chapters
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  chapters/*.md)
add_custom_target(cli_book SOURCES ${book_sources} ${book_chapters})
