
#ifndef GFX_EXPORT_H
#define GFX_EXPORT_H

#ifdef GFX_STATIC_DEFINE
#  define GFX_EXPORT
#  define GFX_NO_EXPORT
#else
#  ifndef GFX_EXPORT
#    ifdef gfx_EXPORTS
        /* We are building this library */
#      define GFX_EXPORT 
#    else
        /* We are using this library */
#      define GFX_EXPORT 
#    endif
#  endif

#  ifndef GFX_NO_EXPORT
#    define GFX_NO_EXPORT 
#  endif
#endif

#ifndef GFX_DEPRECATED
#  define GFX_DEPRECATED __declspec(deprecated)
#endif

#ifndef GFX_DEPRECATED_EXPORT
#  define GFX_DEPRECATED_EXPORT GFX_EXPORT GFX_DEPRECATED
#endif

#ifndef GFX_DEPRECATED_NO_EXPORT
#  define GFX_DEPRECATED_NO_EXPORT GFX_NO_EXPORT GFX_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GFX_NO_DEPRECATED
#    define GFX_NO_DEPRECATED
#  endif
#endif

#endif /* GFX_EXPORT_H */
