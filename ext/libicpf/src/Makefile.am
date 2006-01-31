lib_LTLIBRARIES = libicpf.la
libicpf_la_SOURCES = callback.cpp exception.cpp \
			str.cpp log.cpp dumpctx.cpp cfg.cpp str_help.cpp \
			crc32.cpp file.cpp module.cpp conv.cpp crypt.cpp rijndael-alg-fst.c \
			rijndael-api-fst.c sha256.c

# set the include path found by configure
INCLUDES = $(all_includes)

# the library search path.
libicpf_la_LDFLAGS = $(all_libraries) 
libicpf_la_LIBADD = -ldl -lpthread
dist_include_HEADERS = callback.h exception.h str.h dumpctx.h cfg.h module.h \
			file.h log.h cfg.h str_help.h crc32.h libicpf.h gen_types.h \
			conv.h crypt.h err_codes.h gen_types.h macros.h mutex.h \
			rijndael-alg-fst.h rijndael-api-fst.h sha256.h
AM_CFLAGS = -Wall -Wshadow -DLIBICPF_EXPORTS
AM_CXXFLAGS = -Wall -Wshadow -DLIBICPF_EXPORTS
