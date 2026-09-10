
# Example drop shadow
if (NOT EMSCRIPTEN)
	add_executable(ExampleDropShadow example_dropshadow.cpp)
	target_include_directories(ExampleDropShadow
	  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	target_link_libraries(ExampleDropShadow PRIVATE CImgLib)

	add_executable(TestImageConversion test_image_conversion.cpp)
	target_link_libraries(TestImageConversion PRIVATE CImgLib)
	target_link_libraries(TestImageConversion PRIVATE StbImageLib)
	target_link_libraries(TestImageConversion PRIVATE BlueMarbleMapsLib)

	add_executable(TestResizePerformance test_resize_performance.cpp)
	target_link_libraries(TestResizePerformance PRIVATE CImgLib)
	target_link_libraries(TestResizePerformance PRIVATE StbImageLib)
	target_link_libraries(TestResizePerformance PRIVATE BlueMarbleMapsLib)

	add_executable(TestReflection test_reflection.cpp)
	target_link_libraries(TestReflection PRIVATE ReflectionCpp)
	target_link_libraries(TestReflection PRIVATE BlueMarbleMapsLib)

	add_executable(TestBasicReflection test_reflection_basic.cpp)
	target_link_libraries(TestBasicReflection PRIVATE ReflectionCpp)
	target_link_libraries(TestBasicReflection PRIVATE BlueMarbleMapsLib)

	add_executable(TestMyReflection test_my_reflection.cpp)
	target_link_libraries(TestMyReflection PRIVATE ReflectionCpp)
	target_link_libraries(TestMyReflection PRIVATE BlueMarbleMapsLib)

	add_executable(TestSharedFromThis test_shared_from_this.cpp)
	target_link_libraries(TestSharedFromThis PRIVATE BlueMarbleMapsLib)

	add_executable(TestSignal test_signal.cpp)
	target_link_libraries(TestSignal PRIVATE BlueMarbleMapsLib)

	add_executable(TestFiledatabase test_filedatabase.cpp)
	target_link_libraries(TestFiledatabase PRIVATE BlueMarbleMapsLib)

	add_executable(TestJson test_json.cpp)
	target_link_libraries(TestJson PRIVATE BlueMarbleMapsLib)

	# TCP Server/Client
	add_executable(TestTcpServer test_tcp.cpp)
	target_link_libraries(TestTcpServer PRIVATE BlueMarbleMapsLib)
	target_compile_definitions(TestTcpServer PRIVATE COMPILE_TCP_SERVER=true)
	add_executable(TestTcpClient test_tcp.cpp)
	target_link_libraries(TestTcpClient PRIVATE BlueMarbleMapsLib)
	target_compile_definitions(TestTcpClient PRIVATE COMPILE_TCP_SERVER=false)

	# UDP Server/Client
	add_executable(TestUdpServer test_udp.cpp)
	target_link_libraries(TestUdpServer PRIVATE BlueMarbleMapsLib)
	target_compile_definitions(TestUdpServer PRIVATE COMPILE_UDP_SERVER=true)
	add_executable(TestUdpClient test_udp.cpp)
	target_link_libraries(TestUdpClient PRIVATE BlueMarbleMapsLib)
	target_compile_definitions(TestUdpClient PRIVATE COMPILE_UDP_SERVER=false)

endif()

# TODO: only imgui example works with EMSCRIPTEN
add_subdirectory(imgui)
