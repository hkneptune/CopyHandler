#ifndef __LIBICTRANSLATE_H__
#define __LIBICTRANSLATE_H__

// import/export macros
#if defined(_WIN32) || defined(_WIN64)
	#ifdef LIBICTRANSLATE_EXPORTS
		/** \brief Import/export macros
		*
		*  These macros are being used throughout the whole code. They are meant to
		*  export symbols (if the LIBICTRANSLATE_EXPORTS is defined) from this library
		*  (also for importing (when LIBICTRANSLATE_EXPORTS macro is undefined) in other apps).
		*/
		#define LIBICTRANSLATE_API __declspec(dllexport)
	#else
		/** \brief Import/export macros
		*
		*  These macros are being used throughout the whole code. They are meant to
		*  export symbols (if the LIBICTRANSLATE_EXPORTS is defined) from this library
		*  (also for importing (when LIBICTRANSLATE_EXPORTS macro is undefined) in other apps).
		*/
		#define LIBICTRANSLATE_API __declspec(dllimport)
	#endif
#else
	/** \brief Import/export macros
	*
	*  These macros are being used throughout the whole code. They are meant to
	*  export symbols (if the LIBICTRANSLATE_EXPORTS is defined) from this library
	*  (also for importing (when LIBICTRANSLATE_EXPORTS macro is undefined) in other apps).
	*/
	#define LIBICTRANSLATE_API
#endif

/// Begins ch namespace
#define BEGIN_ICTRANSLATE_NAMESPACE namespace ictranslate {
/// Ends ch namespace
#define END_ICTRANSLATE_NAMESPACE }

#endif
