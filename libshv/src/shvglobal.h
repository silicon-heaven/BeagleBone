#ifndef SHV_GLOBAL_H
#define	SHV_GLOBAL_H

#  if defined _WIN32 || defined _WIN64
#    define MY_DECL_EXPORT     __declspec(dllexport)
#    define MY_DECL_IMPORT     __declspec(dllimport)
#  else
#    define MY_DECL_EXPORT     __attribute__((visibility("default")))
#    define MY_DECL_IMPORT     __attribute__((visibility("default")))
#    define MY_DECL_HIDDEN     __attribute__((visibility("hidden")))
#  endif

/// Declaration of macros required for exporting symbols
/// into shared libraries
#if defined(SHV_BUILD_DLL)
//#warning "EXPORT"
#  define SHV_DECL_EXPORT MY_DECL_EXPORT
#else
//#warning "IMPORT"
#  define SHV_DECL_EXPORT MY_DECL_IMPORT
#endif

#endif
