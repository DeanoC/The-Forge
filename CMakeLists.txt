cmake_minimum_required(VERSION 3.12.4)
get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
	option(unittests "unittests" OFF)
	get_filename_component(_PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
	set_property(GLOBAL PROPERTY GLOBAL_FETCHDEPS_BASE ${_PARENT_DIR}/al2o3 )
	include(FetchContent)
	FetchContent_Declare( al2o3 GIT_REPOSITORY https://github.com/DeanoC/al2o3 GIT_TAG master )
	FetchContent_GetProperties(al2o3)
	if(NOT al2o3_POPULATED)
		FetchContent_Populate(al2o3)
		add_subdirectory(${al2o3_SOURCE_DIR} ${al2o3_BINARY_DIR})
	endif()
	INIT_AL2O3(${CMAKE_CURRENT_SOURCE_DIR})
endif ()

#set(USE_D3D12 TRUE)
#set(USE_VULKAN TRUE)
#set(USE_D3D11 TRUE)

# windows supports USE_VULKAN, USE_D3D11 and USE_D3D12
# defaults to USE_D3D12
if (WIN32)
	if(NOT USE_VULKAN AND NOT USE_D3D11)
		set(USE_D3D12 TRUE)
	endif()

	if(USE_VULKAN)
		find_package(Vulkan REQUIRED)
	endif()
endif()
# TODO vulkan on metal support
if( APPLE)
	set(USE_METAL TRUE)
	set(USE_VULKAN FALSE)
endif()

SET_MIN_VERSIONS()
set(LibName gfx_theforge)
project(${LibName})

set(Deps
		al2o3_platform
		al2o3_memory
		tiny_imageformat
		)

set(Interface
		enums.h
		resourceloader.h
		resourceloaders_structs.h
		shaderreflection.h
		structs.h
		theforge.h
		)

set(Src
		theforge.cpp
		OS/Core/Atomics.h
		OS/Core/Compiler.h
		OS/Core/DLL.h
		OS/Core/FileSystem.cpp
		OS/Core/GPUConfig.h
		OS/Core/RingBuffer.h
		OS/Core/ThreadSystem.cpp
		OS/Core/ThreadSystem.h
		OS/Core/Timer.cpp
		OS/Image/Image.cpp
		OS/Image/Image.h
		OS/Interfaces/IApp.h
		OS/Interfaces/IFileSystem.h
		OS/Interfaces/ILog.h
		OS/Interfaces/IMemory.h
		OS/Interfaces/IMiddleware.h
		OS/Interfaces/IOperatingSystem.h
		OS/Interfaces/IProfiler.h
		OS/Interfaces/IThread.h
		OS/Interfaces/ITime.h
		OS/Math/MathTypes.h
		OS/MemoryTracking/MemoryTracking.cpp
		OS/MemoryTracking/NoMemoryDefines.h
		Renderer/CommonShaderReflection.cpp
		Renderer/GpuProfiler.cpp
		Renderer/GpuProfiler.h
		Renderer/IRay.h
		Renderer/IRenderer.h
		Renderer/IShaderReflection.h
		Renderer/ResourceLoader.cpp
		Renderer/ResourceLoader.h

		ThirdParty/OpenSource/EASTL/allocator_forge.cpp
		ThirdParty/OpenSource/EASTL/assert.cpp
		ThirdParty/OpenSource/EASTL/hashtable.cpp
		ThirdParty/OpenSource/EASTL/numeric_limits.cpp
		ThirdParty/OpenSource/EASTL/red_black_tree.cpp
		ThirdParty/OpenSource/EASTL/string.cpp
		ThirdParty/OpenSource/EASTL/thread_support.cpp
		ThirdParty/OpenSource/EASTL/EAStdC/EASprintf.cpp

		ThirdParty/OpenSource/basis_universal/transcoder/basisu_file_headers.h
		ThirdParty/OpenSource/basis_universal/transcoder/basisu_global_selector_cb.h
		ThirdParty/OpenSource/basis_universal/transcoder/basisu_global_selector_palette.h
		ThirdParty/OpenSource/basis_universal/transcoder/basisu_transcoder.cpp
		ThirdParty/OpenSource/basis_universal/transcoder/basisu_transcoder.h
		ThirdParty/OpenSource/basis_universal/transcoder/basisu_transcoder_internal.h
		ThirdParty/OpenSource/basis_universal/transcoder/basisu_transcoder_tables_bc7_m6.inc
		ThirdParty/OpenSource/basis_universal/transcoder/basisu_transcoder_tables_dxt1_5.inc
		ThirdParty/OpenSource/basis_universal/transcoder/basisu_transcoder_tables_dxt1_6.inc
		)
if (WIN32)
	if(USE_D3D11)
		list(APPEND Src Renderer/Direct3D11/Direct3D11.cpp)
		list(APPEND Src Renderer/Direct3D11/Direct3D11Raytracing.cpp)
		list(APPEND Src Renderer/Direct3D11/Direct3D11ShaderReflection.cpp)
	endif()

	if(USE_D3D12)
		list(APPEND Src Renderer/Direct3D12/Direct3D12.cpp)
		list(APPEND Src Renderer/Direct3D12/Direct3D12Hooks.cpp)
		list(APPEND Src Renderer/Direct3D12/Direct3D12Hooks.h)
		list(APPEND Src Renderer/Direct3D12/Direct3D12MemoryAllocator.cpp)
		list(APPEND Src Renderer/Direct3D12/Direct3D12MemoryAllocator.h)
		list(APPEND Src Renderer/Direct3D12/Direct3D12Raytracing.cpp)
		list(APPEND Src Renderer/Direct3D12/Direct3D12ShaderReflection.cpp)
	endif()

	list(APPEND Src OS/Windows/WindowsFileSystem.cpp)
	list(APPEND Src OS/Windows/WindowsThread.cpp)
	list(APPEND Src OS/Windows/WindowsTime.cpp)
endif ()

if(USE_VULKAN)
	list(APPEND Src Renderer/Vulkan/Vulkan.cpp)
	list(APPEND Src Renderer/Vulkan/VulkanRaytracing.cpp)
	list(APPEND Src Renderer/Vulkan/VulkanShaderReflection.cpp)
	list(APPEND Src Renderer/Vulkan/VulkanCapsBuilder.h)
	list(APPEND Src Tools/SpirvTools/SpirvTools.cpp)

	list(APPEND Deps gfx_spirvtools)
	list(APPEND Deps gfx_spirvcross)
endif()

if (APPLE)
	if( USE_METAL )
		list(APPEND Src Renderer/Metal/MetalMemoryAllocator.h)
		list(APPEND Src Renderer/Metal/MetalCapBuilder.h)
		list(APPEND Src Renderer/Metal/MetalRaytracing.mm)
		list(APPEND Src Renderer/Metal/MetalRenderer.mm)
		list(APPEND Src Renderer/Metal/MetalShaderReflection.mm)
	endif()

	list(APPEND Src OS/macOs/macOSFileSystem.mm)
	list(APPEND Src OS/macOs/macOSThread.cpp)
	list(APPEND Src OS/macOs/macOSTime.cpp)
endif ()


ADD_LIB(${LibName} "${Interface}" "${Src}" "${Deps}")

if(USE_VULKAN)
	add_compile_definitions(${LibName} VULKAN)
	target_include_directories(${LibName} PRIVATE Vulkan::Vulkan)
	target_link_libraries(${LibName} PRIVATE Vulkan::Vulkan)
endif()

if (APPLE)
	add_compile_definitions(${LibName} METAL)
	target_compile_options(${LibName} PRIVATE -fobjc-arc -ObjC++)

	target_link_libraries(${LibName} PRIVATE
			stdc++
			objc
			"-framework Foundation"
			"-framework Cocoa"
			"-framework IOKit"
			"-framework Metal"
			"-framework MetalKit"
			"-framework MetalPerformanceShaders"
			"-framework QuartzCore"
			)
endif ()

if(WIN32)
	if(USE_D3D12)
		target_compile_definitions(${LibName} PRIVATE DIRECT3D12)
	endif()

	if(USE_D3D11)
		target_compile_definitions(${LibName} PRIVATE DIRECT3D11)
	endif()

	target_link_libraries(${LibName} PRIVATE
			winmm
			${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/OpenSource/winpixeventruntime/bin/WinPixEventRuntime.lib)

	configure_file(
			${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/OpenSource/winpixeventruntime/bin/WinPixEventRuntime.dll
			${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/WinPixEventRuntime.dll COPYONLY)

endif()
