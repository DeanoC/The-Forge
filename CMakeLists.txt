cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
get_directory_property(hasParent PARENT_DIRECTORY)
if (NOT hasParent)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out_libs)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out_bin)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out_bin)
	set(LIB_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/libs/)
	include(FetchContent)
	FetchContent_Declare(al2o3 GIT_REPOSITORY https://github.com/DeanoC/al2o3 GIT_TAG master)
	FetchContent_GetProperties(al2o3)
	if (NOT al2o3_POPULATED)
		FetchContent_Populate(al2o3)
		add_subdirectory(${al2o3_SOURCE_DIR} ${al2o3_BINARY_DIR})
	endif ()
endif ()

SET_MIN_VERSIONS()
set(LibName gfx_theforge)
project(${LibName})

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
		OS/Core/PlatformEvents.cpp
		OS/Core/RingBuffer.h
		OS/Core/ThreadSystem.cpp
		OS/Core/ThreadSystem.h
		OS/Core/Timer.cpp
		OS/Image/Image.cpp
		OS/Image/Image.h
		OS/Image/ImageEnums.h
		OS/Interfaces/IApp.h
		OS/Interfaces/ICameraController.h
		OS/Interfaces/IFileSystem.h
		OS/Interfaces/ILog.h
		OS/Interfaces/IMemory.h
		OS/Interfaces/IMiddleware.h
		OS/Interfaces/IOperatingSystem.h
		OS/Interfaces/IPlatformEvents.h
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
		)
if (WIN32)
	list(APPEND Src Renderer/Direct3D12/Direct3D12.cpp)
	list(APPEND Src Renderer/Direct3D12/Direct3D12Hooks.cpp)
	list(APPEND Src Renderer/Direct3D12/Direct3D12Hooks.h)
	list(APPEND Src Renderer/Direct3D12/Direct3D12MemoryAllocator.cpp)
	list(APPEND Src Renderer/Direct3D12/Direct3D12MemoryAllocator.h)
	list(APPEND Src Renderer/Direct3D12/Direct3D12Raytracing.cpp)
	list(APPEND Src Renderer/Direct3D12/Direct3D12ShaderReflection.cpp)
	list(APPEND Src OS/Windows/WindowsFileSystem.cpp)
	list(APPEND Src OS/Windows/WindowsThread.cpp)
	list(APPEND Src OS/Windows/WindowsTime.cpp)
endif ()

if (APPLE)
	list(APPEND Src Renderer/Metal/MetalMemoryAllocator.h)
	list(APPEND Src Renderer/Metal/MetalRaytracing.mm)
	list(APPEND Src Renderer/Metal/MetalRenderer.mm)
	list(APPEND Src Renderer/Metal/MetalShaderReflection.mm)

	list(APPEND Src OS/macOs/macOSFileSystem.mm)
	list(APPEND Src OS/macOs/macOSThread.cpp)
	list(APPEND Src OS/macOs/macOSTime.cpp)
endif ()

set(Deps
		al2o3_platform
		al2o3_memory
		tiny_imageformat
		)
ADD_LIB(${LibName} "${Interface}" "${Src}" "${Deps}")
if (APPLE)
	add_compile_definitions(${LibName} METAL)
	target_compile_options(${LibName} PRIVATE -fobjc-arc -ObjC++)

	set_source_files_properties(src/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputMac.mm
			PROPERTIES COMPILE_FLAGS -fno-objc-arc)

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
	target_compile_definitions(${LibName} PRIVATE DIRECT3D12)
	target_link_libraries(${LibName} PRIVATE
			winmm
			${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/OpenSource/winpixeventruntime/bin/x64/WinPixEventRuntime.lib)

	configure_file(
			${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/OpenSource/winpixeventruntime/bin/x64/WinPixEventRuntime.dll
			${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/WinPixEventRuntime.dll COPYONLY)

endif()