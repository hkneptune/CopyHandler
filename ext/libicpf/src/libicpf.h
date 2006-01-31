#ifndef __LIBICPF_H__
#define __LIBICPF_H__

/** \brief Allows unicode handling throughout the files
 *
 * It means that the engine would process fs object's paths
 * in the better standard - if possible then with current ANSI code page (one-byte
 * chars) or with UNICODE strings (with 2-byte per char strings).
 * \note if this is disabled (#undef) then the data saved by ie. unicode windows
 * system may not be readable (because we won't use the iconv package on linux
 * systems nor use any of the unicode functions in windows systems).
 * Recommended setting is 'defined'.
 */
#define ALLOW_UNICODE

/** \brief Enables use of encryption throughout this library.
 *
 * Enabling this macro enables usage of the encryption in some modules.
 */
#define USE_ENCRYPTION

// import/export macros
#ifdef _WIN32
        #ifdef LIBICPF_EXPORTS
                /** \brief Import/export macros
                *
                *  These macros are being used throughout the whole code. They are meant to
                *  export symbols (if the LIBICPF_EXPORTS is defined) from this library
                *  (also for importing (when LIBICPF_EXPORTS macro is undefined) in other apps).
                */
                #define LIBICPF_API __declspec(dllexport)
        #else
                /** \brief Import/export macros
                *
                *  These macros are being used throughout the whole code. They are meant to
                *  export symbols (if the LIBICPF_EXPORTS is defined) from this library
                *  (also for importing (when LIBICPF_EXPORTS macro is undefined) in other apps).
                */
                #define LIBICPF_API __declspec(dllimport)
        #endif
#else
        /** \brief Import/export macros
        *
        *  These macros are being used throughout the whole code. They are meant to
        *  export symbols (if the LIBICPF_EXPORTS is defined) from this library
        *  (also for importing (when LIBICPF_EXPORTS macro is undefined) in other apps).
        */
        #define LIBICPF_API
#endif

/// Begins ch namespace
#define BEGIN_ICPF_NAMESPACE namespace icpf {
/// Ends ch namespace
#define END_ICPF_NAMESPACE };

#endif
