#pragma once

// If Windows/MSVC
#ifdef _MSC_VER 
    #define EXPORT_API __declspec(dllexport)
    #define IMPORT_API __declspec(dllimport)
// else GCC/CLANG
#else
    // TODO: for gcc/clang consider __attribute__((visibility("default")))
    // - https://stackoverflow.com/questions/52719364/how-to-use-the-attribute-visibilitydefault

    // untested
    #define EXPORT_API __attribute__((visibility("default")))
    #define IMPORT_API 

    #pragma warning Untested dynamic link import/export semantics (__attribute__((visibility("default"))))
#endif // End _MSC_VER


// Setting up PROFILER_API for import/export
// PROFILER_STATIC is defined via CMake if setting up as a static library

#ifdef PROFILER_DYNAMIC_LIBS
	// PROFILER_EXPORTS is defined by CMake if setting up as a dll/shared library
	#ifdef PROFILER_EXPORTS
		// exporting profiler
		#define PROFILER_API EXPORT_API
	#else
		// importing profiler
		#define PROFILER_API IMPORT_API
	#endif //End PROFILER_EXPORTS

#else // PROFILER_DYNAMIC_LIBS else PROFILER_STATIC
	// empty
	#define PROFILER_API 

#endif // End PROFILER_DYNAMIC_LIBS
