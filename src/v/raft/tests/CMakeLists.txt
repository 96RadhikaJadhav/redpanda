find_package(Seastar REQUIRED)
rp_test(
  UNIT_TEST
  BINARY_NAME jitter_tests
  SOURCES jitter_tests.cc
  LIBRARIES Seastar::seastar_testing raft
  DEFINITIONS SEASTAR_TESTING_MAIN
 )