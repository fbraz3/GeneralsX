/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// TheSuperHackers @build feliwir 15/04/2025 Simple allocator implementation useful for tools
#include "always.h"
#include <stdlib.h>
#include <stdio.h> // Added for printf

#if defined(DISABLE_GAMEMEMORY)
#pragma message("DISABLE_GAMEMEMORY is DEFINED")
#else
#pragma message("DISABLE_GAMEMEMORY is NOT DEFINED")
#endif

#ifdef _OPERATOR_NEW_DEFINED_
#pragma message("_OPERATOR_NEW_DEFINED_ is DEFINED")

void *operator new(size_t size)
{
	fprintf(stderr, "[wwallocstub] operator new called (size=%zu)\n", size); fflush(stderr);
	void* p = malloc(size);
	fprintf(stderr, "[wwallocstub] operator new returning %p\n", p); fflush(stderr);
	return p;
}

void *operator new[](size_t size)
{
	return malloc(size);
}

void operator delete(void *p)
{
	free(p);
}

void operator delete[](void *p)
{
	free(p);
}

void* operator new(size_t size, const char * fname, int)
{
	return malloc(size);
}

void operator delete(void * p, const char *, int)
{
	free(p);
}

void* operator new[](size_t size, const char * fname, int)
{
	return malloc(size);
}

void operator delete[](void * p, const char *, int)
{
	free(p);
}

#endif

void* createW3DMemPool(const char *poolName, int allocationSize)
{
	fprintf(stderr, "[wwallocstub] createW3DMemPool called for %s\n", poolName); fflush(stderr);
	return NULL;
}

void* allocateFromW3DMemPool(void* pool, int allocationSize)
{
	fprintf(stderr, "[wwallocstub] allocateFromW3DMemPool called (size=%d)\n", allocationSize); fflush(stderr);
	return malloc(allocationSize);
}

void* allocateFromW3DMemPool(void* pool, int allocationSize, const char* msg, int unused)
{
	fprintf(stderr, "[wwallocstub] allocateFromW3DMemPool (debug) called (size=%d, msg=%s)\n", allocationSize, msg); fflush(stderr);
	return malloc(allocationSize);
}

void freeFromW3DMemPool(void* pool, void* p)
{
	free(p);
}
