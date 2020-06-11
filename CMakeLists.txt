cmake_minimum_required(VERSION 3.5)
project(tramcontract VERSION 1.0)

find_package(eosio.cdt)

set(CMAKE_BUILD_TYPE "Release")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/helper.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/helper.cpp"
)

add_contract(tramcontract tramcontract ${CMAKE_CURRENT_SOURCE_DIR}/tramcontract.cpp ${CMAKE_CURRENT_BINARY_DIR}/helper.cpp)
target_compile_options( tramcontract.wasm PUBLIC -I=${EOSIO_CONTRACTS_DIR}/eosio.system/include )
target_compile_options( tramcontract.wasm PUBLIC -I=${EOSIO_CONTRACTS_DIR}/eosio.token/include )
