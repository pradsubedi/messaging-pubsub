add_executable(server server.c)
target_link_libraries(server messaging)

add_executable(client client.c)
target_link_libraries(client messaging)


find_program (BASH_PROGRAM bash)

if (BASH_PROGRAM)
  add_test (Test_one ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test_script.sh)
endif (BASH_PROGRAM)
