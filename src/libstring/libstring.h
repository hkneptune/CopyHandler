// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBSTRING_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LIBSTRING_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#if defined(_WIN32) || defined(_WIN64)
	#ifdef LIBSTRING_EXPORTS
		#define LIBSTRING_API __declspec(dllexport)
	#else
		#define LIBSTRING_API __declspec(dllimport)
	#endif
#else
	/** \brief Import/export macros
	*
	*  These macros are being used throughout the whole code. They are meant to
	*  export symbols (if the LIBSTRING_EXPORTS is defined) from this library
	*  (also for importing (when LIBSTRING_EXPORTS macro is undefined) in other apps).
	*/
	#define LIBSTRING_API
#endif
