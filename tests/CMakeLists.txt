CPMAddPackage(
  NAME googletest
  GITHUB_REPOSITORY google/googletest
  GIT_TAG release-1.11.0
  OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
)

add_executable(backoffxx_test)

target_sources(backoffxx_test
  PRIVATE
    backoff_test.cpp
    policies_test.cpp
)

apply_backoffxx_compile_conf(backoffxx_test)

target_link_libraries(backoffxx_test
  PRIVATE
    gtest_main
    backoffxx
)

get_target_property(test_FILES backoffxx_test SOURCES)
source_group("tests" FILES ${test_FILES})

include(GoogleTest)
gtest_discover_tests(backoffxx_test)
