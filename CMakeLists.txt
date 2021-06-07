cmake_minimum_required(VERSION 3.10)

project(dect-scanner)

add_compile_options(-Wall -O2)

#set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_package(Boost "1.35" COMPONENTS filesystem thread system)
if(NOT Boost_FOUND)
	message(FATAL_ERROR "Boost required to compile dect-scanner")
endif()

# Debian 10: custom gr-osmosdr with bladerf support
include_directories(/opt/bladeRF/include)
link_directories(/opt/bladeRF/lib)

add_executable(dect-scanner
	src/dect2/api.h
	src/dect2/dect2_common.h
	src/dect2/packet_decoder.h
	src/dect2/packet_decoder_impl.h
	src/dect2/packet_decoder_impl.cxx
	src/dect2/packet_receiver.h
	src/dect2/packet_receiver_impl.h
	src/dect2/packet_receiver_impl.cxx
	src/dect2/phase_diff.h
	src/dect2/phase_diff_impl.h
	src/dect2/phase_diff_impl.cxx
	src/logging.cxx
	src/main.cxx
)
target_link_libraries(dect-scanner
	-pthread
	gnuradio-osmosdr
	gnuradio-blocks
	gnuradio-fft
	gnuradio-filter
	gnuradio-uhd
	gnuradio-runtime
	gnuradio-pmt
	log4cpp
	uhd
	boost_thread
	boost_system
)

install(TARGETS dect-scanner RUNTIME DESTINATION bin)
