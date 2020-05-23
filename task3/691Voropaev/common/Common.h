#pragma once
// Comment this define to turn off DSA-style function calls.
#define __TRY_USE_DIRECT_STATE_ACCESS
#define __TRY_USE_INTERFACE_QUERY

// Convenient object manipulation
#ifdef __TRY_USE_DIRECT_STATE_ACCESS
#define USE_DSA GLEW_VERSION_4_5
#else
#define USE_DSA false
#endif

// Program introspection
#ifdef __TRY_USE_INTERFACE_QUERY
#define USE_INTERFACE_QUERY GLEW_VERSION_4_5
#else
#define USE_INTERFACE_QUERY false
#endif