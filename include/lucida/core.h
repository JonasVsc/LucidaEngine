#pragma once

#include <stdio.h>
#include <assert.h>

typedef enum LeResult { 
	LE_SUCCESS = 0, 
	LE_FAIL = -1 
} LeResult;

#ifdef DEBUG
	#define LE_LOG(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__)
#else
	#define LE_LOG(msg)
#endif  // DEBUG


#define LE_CHECK(f)												\
{																\
	LeResult res = (f);											\
	if (res != LE_SUCCESS)										\
	{															\
		fprintf(stderr, "Fatal error");							\
		assert(res == LE_SUCCESS);								\
	}															\
}


