
add_library(backoffxx INTERFACE)

target_sources(backoffxx
  PRIVATE
    backoff.h
    backoffxx.h
    policies.h

  $<$<BOOL:${WIN32}>:
  >

  $<$<NOT:$<BOOL:${WIN32}>>:
  >
)

target_include_directories(backoffxx
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../
)

target_link_libraries(backoffxx
)

target_compile_definitions(backoffxx
  INTERFACE
    $<$<BOOL:${WIN32}>:
      _UNICODE
      UNICODE
      NOMINMAX
    >

    $<$<CONFIG:DEBUG>:
      _DEBUG
    >
)

get_target_property(backoffxx_FILES backoffxx SOURCES)
source_group("backoffxx" FILES ${backoffxx_FILES})

set_target_properties(backoffxx PROPERTIES
    VS_GLOBAL_RunCodeAnalysis true

    # Use visual studio core guidelines
    # Tweak as your wish.
    VS_GLOBAL_EnableMicrosoftCodeAnalysis true
    # VS_GLOBAL_CodeAnalysisRuleSet ${CMAKE_CURRENT_SOURCE_DIR}/foo.ruleset
    # VS_GLOBAL_CodeAnalysisRuleSet ${CMAKE_CURRENT_SOURCE_DIR}/foo.ruleset

    # Use clangtidy
    # Tweak as your wish.
    VS_GLOBAL_EnableClangTidyCodeAnalysis true
    # VS_GLOBAL_ClangTidyChecks -checks=-*,modernize-*,-modernize-use-trailing-return-type
)
