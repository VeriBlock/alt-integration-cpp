namespace altintegrationversion {

#define XSTR(x) #x
#define STR(x) XSTR(x)

const char* version =
#ifdef VBK_VERSION
    STR(VBK_VERSION)
#else
    "Undefined"
#endif
    ;

const char* built = __DATE__ " " __TIME__;

const char* os =
#if defined(__linux__) && !defined(__ANDROID__)
    "Linux"
#elif defined(__ANDROID)
    "Android"
#elif defined(__APPLE__)
    "Apple"
#elif defined(_WIN64)
    "Win64"
#elif defined(_WIN32)
    "Win32"
#else
    "Undefined"
#endif
    ;

const char* compiler =
#if defined(_MSC_VER)
    "Visual Studio " STR(_MSC_FULL_VER)
#elif defined(__clang__)
    "clang " STR(__clang_major__) "." STR(__clang_minor__) "." STR(
        __clang_patchlevel__)
#elif defined(__GNUC__)
    "gcc " STR(__GNUC__) "." STR(__GNUC_MINOR__)
#elif defined(__MINGW64__)
    "mingw-w64 64bit " STR(__MINGW64_VERSION_MAJOR) "." STR(
        __MINGW64_VERSION_MINOR)
#elif defined(__MINGW32__)
    "mingw 32bit " STR(__MINGW32_MAJOR_VERSION) "." STR(__MINGW32_MINOR_VERSION)
#else
    "Undefined"
#endif
    ;

const char* architecture =
#if defined(__i386__)
    "i386"
#elif defined(__x86_64__)
    "x86_64"
#elif defined(__arm__)
    "ARM"
#elif defined(__powerpc64__)
    "powerpc64"
#elif defined(__aarch64__)
    "aarch64"
#else
    "Undefined"
#endif
    ;
}  // namespace altintegrationversion