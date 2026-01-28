#ifndef CASHMERE_EXPORT_H
#define CASHMERE_EXPORT_H

#ifdef CASHMERE_STATIC_DEFINE
#  define CASHMERE_EXPORT
#  define CASHMERE_NO_EXPORT
#else
#  ifndef CASHMERE_EXPORT
#    ifdef cashmere_EXPORTS
        /* We are building this library */
#      define CASHMERE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define CASHMERE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef CASHMERE_NO_EXPORT
#    define CASHMERE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef CASHMERE_DEPRECATED
#  define CASHMERE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef CASHMERE_DEPRECATED_EXPORT
#  define CASHMERE_DEPRECATED_EXPORT CASHMERE_EXPORT CASHMERE_DEPRECATED
#endif

#ifndef CASHMERE_DEPRECATED_NO_EXPORT
#  define CASHMERE_DEPRECATED_NO_EXPORT CASHMERE_NO_EXPORT CASHMERE_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CASHMERE_NO_DEPRECATED
#    define CASHMERE_NO_DEPRECATED
#  endif
#endif

#endif /* CASHMERE_EXPORT_H */
