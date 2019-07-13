cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out_libs)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out_bin)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out_bin)
	set(LIB_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/libs/)
	include(FetchContent)
	FetchContent_Declare( al2o3 GIT_REPOSITORY https://github.com/DeanoC/al2o3 GIT_TAG master )
	FetchContent_GetProperties(al2o3)
	if(NOT al2o3_POPULATED)
		FetchContent_Populate(al2o3)
		add_subdirectory(${al2o3_SOURCE_DIR} ${al2o3_BINARY_DIR})
	endif()
endif ()

SET_MIN_VERSIONS()
set(LibName gfx_theforge)
project(${LibName})

set(Interface

		)

set(Src
		OS/Camera/FpsCameraController.cpp
		OS/Camera/GuiCameraController.cpp
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
		OS/Input/InputMappings.h
		OS/Input/InputSystem.cpp
		OS/Input/InputSystem.h
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
		OS/Logging/Log.cpp
		OS/Logging/Log.h
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
		)
if (WIN32)
	list(APPEND Src Renderer/Direct3D12/Direct3D12.cpp)
	list(APPEND Src Renderer/Direct3D12/Direct3D12Hooks.cpp)
	list(APPEND Src Renderer/Direct3D12/Direct3D12Hooks.h)
	list(APPEND Src Renderer/Direct3D12/Direct3D12MemoryAllocator.cpp)
	list(APPEND Src Renderer/Direct3D12/Direct3D12MemoryAllocator.h)
	list(APPEND Src Renderer/Direct3D12/Direct3D12Raytracing.cpp)
	list(APPEND Src Renderer/Direct3D12/Direct3D12ShaderReflection.cpp)
endif()

if(APPLE)
	list(APPEND Src Renderer/Metal/MetalMemoryAllocator.h)
	list(APPEND Src Renderer/Metal/MetalRaytracing.mm)
	list(APPEND Src Renderer/Metal/MetalRenderer.mm)
	list(APPEND Src Renderer/Metal/MetalShaderReflection.mm)
endif()

set(Deps
		al2o3_platform
		al2o3_memory
		al2o3_vfile
		)
ADD_LIB(${LibName} "${Interface}" "${Src}" "${Deps}")

