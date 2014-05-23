INCLUDE_DIRECTORIES(../orchid)
INCLUDE_DIRECTORIES(/usr/local/include)
LINK_DIRECTORIES(/usr/local/lib)
INCLUDE_DIRECTORIES(/usr/include)
LINK_DIRECTORIES(/usr/lib)

ADD_EXECUTABLE(client test_client.cpp)
ADD_EXECUTABLE(server test_server.cpp)

ADD_EXECUTABLE(client1 test_client1.cpp)
ADD_EXECUTABLE(server1 test_server1.cpp)

ADD_EXECUTABLE(client2 test_client2.cpp)
ADD_EXECUTABLE(server2 test_server2.cpp)

ADD_EXECUTABLE(chan test_chan.cpp)

ADD_EXECUTABLE(benchmark_orchid_client benchmark_orchid_client.cpp)
ADD_EXECUTABLE(benchmark_orchid_server benchmark_orchid_server.cpp)

ADD_EXECUTABLE(benchmark_asio_client benchmark_asio_client.cpp)
ADD_EXECUTABLE(benchmark_asio_server benchmark_asio_server.cpp)

ADD_EXECUTABLE(chat_client chat_client.cpp)
ADD_EXECUTABLE(chat_server chat_server.cpp)

ADD_EXECUTABLE(worker test_worker.cpp)

ADD_EXECUTABLE(hello_world hello_world.cpp)


TARGET_LINK_LIBRARIES(hello_world boost_context boost_system boost_thread)

TARGET_LINK_LIBRARIES(client boost_context boost_system boost_thread pthread)
TARGET_LINK_LIBRARIES(server boost_context boost_system boost_thread pthread)

TARGET_LINK_LIBRARIES(client1 boost_context boost_system boost_thread pthread)
TARGET_LINK_LIBRARIES(server1 boost_context boost_system boost_thread pthread)

TARGET_LINK_LIBRARIES(client2 boost_context boost_system boost_thread pthread)
TARGET_LINK_LIBRARIES(server2 boost_context boost_system boost_thread pthread)

TARGET_LINK_LIBRARIES(chan boost_context boost_system boost_thread pthread)
TARGET_LINK_LIBRARIES(chat_client boost_context boost_system boost_thread pthread)
TARGET_LINK_LIBRARIES(chat_server boost_context boost_system boost_thread pthread)
TARGET_LINK_LIBRARIES(worker boost_context boost_system boost_thread pthread)


