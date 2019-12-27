
if(MSVC)

    set(
        book_sources
        README.md
        SUMMARY.md
        chapters/advanced-topics.md
        chapters/an-advanced-example.md
        chapters/basics.md
        chapters/config.md
        chapters/flags.md
        chapters/formatting.md
        chapters/installation.md
        chapters/internals.md
        chapters/options.md
        chapters/subcommands.md
        chapters/toolkits.md
        chapters/validators.md
    )

    add_custom_target(cli_book SOURCES ${book_sources})

endif(MSVC)
