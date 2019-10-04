/*
 * Copyright (c) 2018-2019 Confetti Interactive Inc.
 *
 * This file is part of The-Forge
 * (see https://github.com/ConfettiFX/The-Forge).
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
*/

//--------------------------------------------------------------------------------------------
// NOTE: Make sure this is the last include in a .cpp file!
//       Never include this file from a header!!  If you must use the mem manager from a
//       header (which should be in rare cases, and usually only in core Forge source), 
//       define "IMEMORY_FROM_HEADER" before including it.
//--------------------------------------------------------------------------------------------

#ifndef IMEMORY_H
#define IMEMORY_H
#include "al2o3_memory/memory.h"

template <typename T, typename... Args>
static T* conf_placement_new(void* ptr, Args... args)
{
	return new (ptr) T(args...);
}

static void* conf_malloc_internal(size_t size, const char *f, int l, const char *sf)
{
	return MEMORY_MALLOC(size); // TODO tracking info
}


template <typename T, typename... Args>
static T* conf_new_internal(const char *f, int l, const char *sf, Args... args)
{
	Memory_TrackerPushNextSrcLoc(f, l, sf);
	T* ptr = (T*)Memory_GlobalAllocator.malloc(sizeof(T));
	return new (ptr) T(args...);
}

template <typename T>
static void conf_delete_internal(T* ptr)
{
	if (ptr)
	{
		ptr->~T();
		MEMORY_FREE(ptr);
	}
}

#define conf_malloc(size) MEMORY_MALLOC(size)
#define conf_memalign(align,size) MEMORY_AALLOC(size, align)

#define conf_calloc(count,size) MEMORY_CALLOC(count, size)
#define conf_realloc(ptr,size) MEMORY_REALLOC(ptr, size)
#define conf_free(ptr) MEMORY_FREE(ptr)
#define conf_new(ObjectType, ...) conf_new_internal<ObjectType>(__FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define conf_delete(ptr) conf_delete_internal(ptr)

#endif 

#ifdef IMEMORY_FROM_HEADER
#undef IMEMORY_FROM_HEADER
#endif