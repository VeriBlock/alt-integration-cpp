//
//          Tracy profiler
//         ----------------
//
// For fast integration, compile and
// link with this source file (and none
// other) in your executable (or in the
// main DLL / shared object on multi-DLL
// projects).
//

// Define TRACY_ENABLE to enable profiler.


/*** Start of inlined file: TracySystem.cpp ***/
#if defined _MSC_VER || defined __CYGWIN__ || defined _WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# ifndef NOMINMAX
#  define NOMINMAX
# endif
#endif
#ifdef _MSC_VER
#  pragma warning(disable:4996)
#endif
#if defined _WIN32 || defined __CYGWIN__
#  include <windows.h>
#  include <malloc.h>
#else
#  include <pthread.h>
#  include <string.h>
#  include <unistd.h>
#endif

#ifdef __linux__
#  ifdef __ANDROID__
#    include <sys/types.h>
#  else
#    include <sys/syscall.h>
#  endif
#  include <fcntl.h>
#elif defined __FreeBSD__
#  include <sys/thr.h>
#elif defined __NetBSD__ || defined __DragonFly__
#  include <sys/lwp.h>
#endif

#ifdef __MINGW32__
#  define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>


/*** Start of inlined file: TracySystem.hpp ***/
#ifndef __TRACYSYSTEM_HPP__
#define __TRACYSYSTEM_HPP__

#include <cstdint>


/*** Start of inlined file: TracyApi.h ***/
#ifndef __TRACYAPI_H__
#define __TRACYAPI_H__

#if defined _WIN32 || defined __CYGWIN__
#  if defined TRACY_EXPORTS
#    define TRACY_API __declspec(dllexport)
#  elif defined TRACY_IMPORTS
#    define TRACY_API __declspec(dllimport)
#  else
#    define TRACY_API
#  endif
#else
#  define TRACY_API __attribute__((visibility("default")))
#endif

#endif    // __TRACYAPI_H__

/*** End of inlined file: TracyApi.h ***/

namespace tracy
{

namespace detail
{
TRACY_API uint64_t GetThreadHandleImpl();
}

#ifdef TRACY_ENABLE
TRACY_API uint64_t GetThreadHandle();
#else
static inline uint64_t GetThreadHandle()
{
    return detail::GetThreadHandleImpl();
}
#endif

TRACY_API void SetThreadName( const char* name );
TRACY_API const char* GetThreadName( uint64_t id );

TRACY_API const char* GetEnvVar(const char* name);

}

#endif

/*** End of inlined file: TracySystem.hpp ***/

#if defined _WIN32 || defined __CYGWIN__
extern "C" typedef HRESULT (WINAPI *t_SetThreadDescription)( HANDLE, PCWSTR );
extern "C" typedef HRESULT (WINAPI *t_GetThreadDescription)( HANDLE, PWSTR* );
#endif

#ifdef TRACY_ENABLE
#  include <atomic>

/*** Start of inlined file: TracyAlloc.hpp ***/
#ifndef __TRACYALLOC_HPP__
#define __TRACYALLOC_HPP__

#include <stdlib.h>

#ifdef TRACY_ENABLE


/*** Start of inlined file: TracyForceInline.hpp ***/
#ifndef __TRACYFORCEINLINE_HPP__
#define __TRACYFORCEINLINE_HPP__

#if defined(__GNUC__)
#  define tracy_force_inline __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#  define tracy_force_inline __forceinline
#else
#  define tracy_force_inline inline
#endif

#if defined(__GNUC__)
#  define tracy_no_inline __attribute__((noinline))
#elif defined(_MSC_VER)
#  define tracy_no_inline __declspec(noinline)
#else
#  define tracy_no_inline
#endif

#endif

/*** End of inlined file: TracyForceInline.hpp ***/


/*** Start of inlined file: tracy_rpmalloc.hpp ***/
#include <cstddef>

namespace tracy
{

#if defined(__clang__) || defined(__GNUC__)
# define RPMALLOC_EXPORT __attribute__((visibility("default")))
# define RPMALLOC_ALLOCATOR
# define RPMALLOC_ATTRIB_MALLOC __attribute__((__malloc__))
# if defined(__clang_major__) && (__clang_major__ < 4)
# define RPMALLOC_ATTRIB_ALLOC_SIZE(size)
# define RPMALLOC_ATTRIB_ALLOC_SIZE2(count, size)
# else
# define RPMALLOC_ATTRIB_ALLOC_SIZE(size) __attribute__((alloc_size(size)))
# define RPMALLOC_ATTRIB_ALLOC_SIZE2(count, size)  __attribute__((alloc_size(count, size)))
# endif
# define RPMALLOC_CDECL
#elif defined(_MSC_VER)
# define RPMALLOC_EXPORT
# define RPMALLOC_ALLOCATOR __declspec(allocator) __declspec(restrict)
# define RPMALLOC_ATTRIB_MALLOC
# define RPMALLOC_ATTRIB_ALLOC_SIZE(size)
# define RPMALLOC_ATTRIB_ALLOC_SIZE2(count,size)
# define RPMALLOC_CDECL __cdecl
#else
# define RPMALLOC_EXPORT
# define RPMALLOC_ALLOCATOR
# define RPMALLOC_ATTRIB_MALLOC
# define RPMALLOC_ATTRIB_ALLOC_SIZE(size)
# define RPMALLOC_ATTRIB_ALLOC_SIZE2(count,size)
# define RPMALLOC_CDECL
#endif

//! Define RPMALLOC_CONFIGURABLE to enable configuring sizes
#ifndef RPMALLOC_CONFIGURABLE
#define RPMALLOC_CONFIGURABLE 0
#endif

//! Flag to rpaligned_realloc to not preserve content in reallocation
#define RPMALLOC_NO_PRESERVE    1

typedef struct rpmalloc_global_statistics_t {
	//! Current amount of virtual memory mapped, all of which might not have been committed (only if ENABLE_STATISTICS=1)
	size_t mapped;
	//! Peak amount of virtual memory mapped, all of which might not have been committed (only if ENABLE_STATISTICS=1)
	size_t mapped_peak;
	//! Current amount of memory in global caches for small and medium sizes (<32KiB)
	size_t cached;
	//! Current amount of memory allocated in huge allocations, i.e larger than LARGE_SIZE_LIMIT which is 2MiB by default (only if ENABLE_STATISTICS=1)
	size_t huge_alloc;
	//! Peak amount of memory allocated in huge allocations, i.e larger than LARGE_SIZE_LIMIT which is 2MiB by default (only if ENABLE_STATISTICS=1)
	size_t huge_alloc_peak;
	//! Total amount of memory mapped since initialization (only if ENABLE_STATISTICS=1)
	size_t mapped_total;
	//! Total amount of memory unmapped since initialization  (only if ENABLE_STATISTICS=1)
	size_t unmapped_total;
} rpmalloc_global_statistics_t;

typedef struct rpmalloc_thread_statistics_t {
	//! Current number of bytes available in thread size class caches for small and medium sizes (<32KiB)
	size_t sizecache;
	//! Current number of bytes available in thread span caches for small and medium sizes (<32KiB)
	size_t spancache;
	//! Total number of bytes transitioned from thread cache to global cache (only if ENABLE_STATISTICS=1)
	size_t thread_to_global;
	//! Total number of bytes transitioned from global cache to thread cache (only if ENABLE_STATISTICS=1)
	size_t global_to_thread;
	//! Per span count statistics (only if ENABLE_STATISTICS=1)
	struct {
		//! Currently used number of spans
		size_t current;
		//! High water mark of spans used
		size_t peak;
		//! Number of spans transitioned to global cache
		size_t to_global;
		//! Number of spans transitioned from global cache
		size_t from_global;
		//! Number of spans transitioned to thread cache
		size_t to_cache;
		//! Number of spans transitioned from thread cache
		size_t from_cache;
		//! Number of spans transitioned to reserved state
		size_t to_reserved;
		//! Number of spans transitioned from reserved state
		size_t from_reserved;
		//! Number of raw memory map calls (not hitting the reserve spans but resulting in actual OS mmap calls)
		size_t map_calls;
	} span_use[32];
	//! Per size class statistics (only if ENABLE_STATISTICS=1)
	struct {
		//! Current number of allocations
		size_t alloc_current;
		//! Peak number of allocations
		size_t alloc_peak;
		//! Total number of allocations
		size_t alloc_total;
		//! Total number of frees
		size_t free_total;
		//! Number of spans transitioned to cache
		size_t spans_to_cache;
		//! Number of spans transitioned from cache
		size_t spans_from_cache;
		//! Number of spans transitioned from reserved state
		size_t spans_from_reserved;
		//! Number of raw memory map calls (not hitting the reserve spans but resulting in actual OS mmap calls)
		size_t map_calls;
	} size_use[128];
} rpmalloc_thread_statistics_t;

typedef struct rpmalloc_config_t {
	//! Map memory pages for the given number of bytes. The returned address MUST be
	//  aligned to the rpmalloc span size, which will always be a power of two.
	//  Optionally the function can store an alignment offset in the offset variable
	//  in case it performs alignment and the returned pointer is offset from the
	//  actual start of the memory region due to this alignment. The alignment offset
	//  will be passed to the memory unmap function. The alignment offset MUST NOT be
	//  larger than 65535 (storable in an uint16_t), if it is you must use natural
	//  alignment to shift it into 16 bits. If you set a memory_map function, you
	//  must also set a memory_unmap function or else the default implementation will
	//  be used for both.
	void* (*memory_map)(size_t size, size_t* offset);
	//! Unmap the memory pages starting at address and spanning the given number of bytes.
	//  If release is set to non-zero, the unmap is for an entire span range as returned by
	//  a previous call to memory_map and that the entire range should be released. The
	//  release argument holds the size of the entire span range. If release is set to 0,
	//  the unmap is a partial decommit of a subset of the mapped memory range.
	//  If you set a memory_unmap function, you must also set a memory_map function or
	//  else the default implementation will be used for both.
	void (*memory_unmap)(void* address, size_t size, size_t offset, size_t release);
	//! Size of memory pages. The page size MUST be a power of two. All memory mapping
	//  requests to memory_map will be made with size set to a multiple of the page size.
	//  Used if RPMALLOC_CONFIGURABLE is defined to 1, otherwise system page size is used.
	size_t page_size;
	//! Size of a span of memory blocks. MUST be a power of two, and in [4096,262144]
	//  range (unless 0 - set to 0 to use the default span size). Used if RPMALLOC_CONFIGURABLE
	//  is defined to 1.
	size_t span_size;
	//! Number of spans to map at each request to map new virtual memory blocks. This can
	//  be used to minimize the system call overhead at the cost of virtual memory address
	//  space. The extra mapped pages will not be written until actually used, so physical
	//  committed memory should not be affected in the default implementation. Will be
	//  aligned to a multiple of spans that match memory page size in case of huge pages.
	size_t span_map_count;
	//! Enable use of large/huge pages. If this flag is set to non-zero and page size is
	//  zero, the allocator will try to enable huge pages and auto detect the configuration.
	//  If this is set to non-zero and page_size is also non-zero, the allocator will
	//  assume huge pages have been configured and enabled prior to initializing the
	//  allocator.
	//  For Windows, see https://docs.microsoft.com/en-us/windows/desktop/memory/large-page-support
	//  For Linux, see https://www.kernel.org/doc/Documentation/vm/hugetlbpage.txt
	int enable_huge_pages;
} rpmalloc_config_t;

//! Initialize allocator with default configuration
TRACY_API int
rpmalloc_initialize(void);

//! Initialize allocator with given configuration
RPMALLOC_EXPORT int
rpmalloc_initialize_config(const rpmalloc_config_t* config);

//! Get allocator configuration
RPMALLOC_EXPORT const rpmalloc_config_t*
rpmalloc_config(void);

//! Finalize allocator
TRACY_API void
rpmalloc_finalize(void);

//! Initialize allocator for calling thread
TRACY_API void
rpmalloc_thread_initialize(void);

//! Finalize allocator for calling thread
TRACY_API void
rpmalloc_thread_finalize(void);

//! Perform deferred deallocations pending for the calling thread heap
RPMALLOC_EXPORT void
rpmalloc_thread_collect(void);

//! Query if allocator is initialized for calling thread
RPMALLOC_EXPORT int
rpmalloc_is_thread_initialized(void);

//! Get per-thread statistics
RPMALLOC_EXPORT void
rpmalloc_thread_statistics(rpmalloc_thread_statistics_t* stats);

//! Get global statistics
RPMALLOC_EXPORT void
rpmalloc_global_statistics(rpmalloc_global_statistics_t* stats);

//! Dump all statistics in human readable format to file (should be a FILE*)
RPMALLOC_EXPORT void
rpmalloc_dump_statistics(void* file);

//! Allocate a memory block of at least the given size
TRACY_API RPMALLOC_ALLOCATOR void*
rpmalloc(size_t size) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE(1);

//! Free the given memory block
TRACY_API void
rpfree(void* ptr);

//! Allocate a memory block of at least the given size and zero initialize it
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR void*
rpcalloc(size_t num, size_t size) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2);

//! Reallocate the given block to at least the given size
TRACY_API RPMALLOC_ALLOCATOR void*
rprealloc(void* ptr, size_t size) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE(2);

//! Reallocate the given block to at least the given size and alignment,
//  with optional control flags (see RPMALLOC_NO_PRESERVE).
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR void*
rpaligned_realloc(void* ptr, size_t alignment, size_t size, size_t oldsize, unsigned int flags) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE(3);

//! Allocate a memory block of at least the given size and alignment.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR void*
rpaligned_alloc(size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE(2);

//! Allocate a memory block of at least the given size and alignment.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR void*
rpmemalign(size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE(2);

//! Allocate a memory block of at least the given size and alignment.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size (default 64KiB)
RPMALLOC_EXPORT int
rpposix_memalign(void **memptr, size_t alignment, size_t size);

//! Query the usable size of the given memory block (from given pointer to the end of block)
RPMALLOC_EXPORT size_t
rpmalloc_usable_size(void* ptr);

}

/*** End of inlined file: tracy_rpmalloc.hpp ***/

#endif

namespace tracy
{

#ifdef TRACY_ENABLE
TRACY_API void InitRpmalloc();
#endif

static inline void* tracy_malloc( size_t size )
{
#ifdef TRACY_ENABLE
    InitRpmalloc();
    return rpmalloc( size );
#else
    return malloc( size );
#endif
}

static inline void* tracy_malloc_fast( size_t size )
{
#ifdef TRACY_ENABLE
    return rpmalloc( size );
#else
    return malloc( size );
#endif
}

static inline void tracy_free( void* ptr )
{
#ifdef TRACY_ENABLE
    InitRpmalloc();
    rpfree( ptr );
#else
    free( ptr );
#endif
}

static inline void tracy_free_fast( void* ptr )
{
#ifdef TRACY_ENABLE
    rpfree( ptr );
#else
    free( ptr );
#endif
}

static inline void* tracy_realloc( void* ptr, size_t size )
{
#ifdef TRACY_ENABLE
    InitRpmalloc();
    return rprealloc( ptr, size );
#else
    return realloc( ptr, size );
#endif
}

}

#endif

/*** End of inlined file: TracyAlloc.hpp ***/


#endif

namespace tracy
{

namespace detail
{

TRACY_API uint64_t GetThreadHandleImpl()
{
#if defined _WIN32 || defined __CYGWIN__
    static_assert( sizeof( decltype( GetCurrentThreadId() ) ) <= sizeof( uint64_t ), "Thread handle too big to fit in protocol" );
    return uint64_t( GetCurrentThreadId() );
#elif defined __APPLE__
    uint64_t id;
    pthread_threadid_np( pthread_self(), &id );
    return id;
#elif defined __ANDROID__
    return (uint64_t)gettid();
#elif defined __linux__
    return (uint64_t)syscall( SYS_gettid );
#elif defined __FreeBSD__
    long id;
    thr_self( &id );
    return id;
#elif defined __NetBSD__
    return _lwp_self();
#elif defined __DragonFly__
    return lwp_gettid();
#elif defined __OpenBSD__
    return getthrid();
#else
    static_assert( sizeof( decltype( pthread_self() ) ) <= sizeof( uint64_t ), "Thread handle too big to fit in protocol" );
    return uint64_t( pthread_self() );
#endif

}

}

#ifdef TRACY_ENABLE
struct ThreadNameData
{
    uint64_t id;
    const char* name;
    ThreadNameData* next;
};
std::atomic<ThreadNameData*>& GetThreadNameData();
#endif

#ifdef _MSC_VER
#  pragma pack( push, 8 )
struct THREADNAME_INFO
{
    DWORD dwType;
    LPCSTR szName;
    DWORD dwThreadID;
    DWORD dwFlags;
};
#  pragma pack(pop)

void ThreadNameMsvcMagic( const THREADNAME_INFO& info )
{
    __try
    {
        RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
}
#endif

TRACY_API void SetThreadName( const char* name )
{
#if defined _WIN32 || defined __CYGWIN__
    static auto _SetThreadDescription = (t_SetThreadDescription)GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "SetThreadDescription" );
    if( _SetThreadDescription )
    {
        wchar_t buf[256];
        mbstowcs( buf, name, 256 );
        _SetThreadDescription( GetCurrentThread(), buf );
    }
    else
    {
#  if defined _MSC_VER
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = name;
        info.dwThreadID = GetCurrentThreadId();
        info.dwFlags = 0;
        ThreadNameMsvcMagic( info );
#  endif
    }
#elif defined _GNU_SOURCE && !defined __EMSCRIPTEN__ && !defined __CYGWIN__
    {
        const auto sz = strlen( name );
        if( sz <= 15 )
        {
            pthread_setname_np( pthread_self(), name );
        }
        else
        {
            char buf[16];
            memcpy( buf, name, 15 );
            buf[15] = '\0';
            pthread_setname_np( pthread_self(), buf );
        }
    }
#endif
#ifdef TRACY_ENABLE
    {
        const auto sz = strlen( name );
        char* buf = (char*)tracy_malloc( sz+1 );
        memcpy( buf, name, sz );
        buf[sz] = '\0';
        auto data = (ThreadNameData*)tracy_malloc_fast( sizeof( ThreadNameData ) );
        data->id = detail::GetThreadHandleImpl();
        data->name = buf;
        data->next = GetThreadNameData().load( std::memory_order_relaxed );
        while( !GetThreadNameData().compare_exchange_weak( data->next, data, std::memory_order_release, std::memory_order_relaxed ) ) {}
    }
#endif
}

TRACY_API const char* GetThreadName( uint64_t id )
{
    static char buf[256];
#ifdef TRACY_ENABLE
    auto ptr = GetThreadNameData().load( std::memory_order_relaxed );
    while( ptr )
    {
        if( ptr->id == id )
        {
            return ptr->name;
        }
        ptr = ptr->next;
    }
#else
#  if defined _WIN32 || defined __CYGWIN__
    static auto _GetThreadDescription = (t_GetThreadDescription)GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "GetThreadDescription" );
    if( _GetThreadDescription )
    {
        auto hnd = OpenThread( THREAD_QUERY_LIMITED_INFORMATION, FALSE, (DWORD)id );
        if( hnd != 0 )
        {
            PWSTR tmp;
            _GetThreadDescription( hnd, &tmp );
            auto ret = wcstombs( buf, tmp, 256 );
            CloseHandle( hnd );
            if( ret != 0 )
            {
                return buf;
            }
        }
    }
#  elif defined __linux__
    int cs, fd;
    char path[32];
#   ifdef __ANDROID__
    int tid = gettid();
#   else
    int tid = (int) syscall( SYS_gettid );
#   endif
    snprintf( path, sizeof( path ), "/proc/self/task/%d/comm", tid );
    sprintf( buf, "%" PRIu64, id );
#   ifndef __ANDROID__
    pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &cs );
#   endif
    if ( ( fd = open( path, O_RDONLY ) ) > 0) {
        int len = read( fd, buf, 255 );
        if( len > 0 )
        {
            buf[len] = 0;
            if( len > 1 && buf[len-1] == '\n' )
            {
                buf[len-1] = 0;
            }
        }
        close( fd );
    }
#   ifndef __ANDROID__
    pthread_setcancelstate( cs, 0 );
#   endif
    return buf;
#  endif
#endif
    sprintf( buf, "%" PRIu64, id );
    return buf;
}

TRACY_API const char* GetEnvVar( const char* name )
{
#if defined _WIN32
    // unfortunately getenv() on Windows is just fundamentally broken.  It caches the entire
    // environment block once on startup, then never refreshes it again.  If any environment
    // strings are added or modified after startup of the CRT, those changes will not be
    // seen by getenv().  This removes the possibility of an app using this SDK from
    // programmatically setting any of the behaviour controlling envvars here.
    //
    // To work around this, we'll instead go directly to the Win32 environment strings APIs
    // to get the current value.
    static char buffer[1024];
    DWORD const kBufferSize = DWORD(sizeof(buffer) / sizeof(buffer[0]));
    DWORD count = GetEnvironmentVariableA(name, buffer, kBufferSize);

    if( count == 0 )
        return nullptr;

    if( count >= kBufferSize )
    {
        char* buf = reinterpret_cast<char*>(_alloca(count + 1));
        count = GetEnvironmentVariableA(name, buf, count + 1);
        memcpy(buffer, buf, kBufferSize);
        buffer[kBufferSize - 1] = 0;
    }

    return buffer;
#else
    return getenv(name);
#endif
}

}

#ifdef __cplusplus
extern "C" {
#endif

TRACY_API void ___tracy_set_thread_name( const char* name ) { tracy::SetThreadName( name ); }

#ifdef __cplusplus
}
#endif

/*** End of inlined file: TracySystem.cpp ***/

#ifdef TRACY_ENABLE

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif


/*** Start of inlined file: tracy_lz4.cpp ***/
/*-************************************
*  Tuning parameters
**************************************/
/*
 * LZ4_HEAPMODE :
 * Select how default compression functions will allocate memory for their hash table,
 * in memory stack (0:default, fastest), or in memory heap (1:requires malloc()).
 */
#ifndef LZ4_HEAPMODE
#  define LZ4_HEAPMODE 0
#endif

/*
 * LZ4_ACCELERATION_DEFAULT :
 * Select "acceleration" for LZ4_compress_fast() when parameter value <= 0
 */
#define LZ4_ACCELERATION_DEFAULT 1
/*
 * LZ4_ACCELERATION_MAX :
 * Any "acceleration" value higher than this threshold
 * get treated as LZ4_ACCELERATION_MAX instead (fix #876)
 */
#define LZ4_ACCELERATION_MAX 65537

/*-************************************
*  CPU Feature Detection
**************************************/
/* LZ4_FORCE_MEMORY_ACCESS
 * By default, access to unaligned memory is controlled by `memcpy()`, which is safe and portable.
 * Unfortunately, on some target/compiler combinations, the generated assembly is sub-optimal.
 * The below switch allow to select different access method for improved performance.
 * Method 0 (default) : use `memcpy()`. Safe and portable.
 * Method 1 : `__packed` statement. It depends on compiler extension (ie, not portable).
 *            This method is safe if your compiler supports it, and *generally* as fast or faster than `memcpy`.
 * Method 2 : direct access. This method is portable but violate C standard.
 *            It can generate buggy code on targets which assembly generation depends on alignment.
 *            But in some circumstances, it's the only known way to get the most performance (ie GCC + ARMv6)
 * See https://fastcompression.blogspot.fr/2015/08/accessing-unaligned-memory.html for details.
 * Prefer these methods in priority order (0 > 1 > 2)
 */
#ifndef LZ4_FORCE_MEMORY_ACCESS   /* can be defined externally */
#  if defined(__GNUC__) && \
  ( defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) \
  || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__) )
#    define LZ4_FORCE_MEMORY_ACCESS 2
#  elif (defined(__INTEL_COMPILER) && !defined(_WIN32)) || defined(__GNUC__)
#    define LZ4_FORCE_MEMORY_ACCESS 1
#  endif
#endif

/*
 * LZ4_FORCE_SW_BITCOUNT
 * Define this parameter if your target system or compiler does not support hardware bit count
 */
#if defined(_MSC_VER) && defined(_WIN32_WCE)   /* Visual Studio for WinCE doesn't support Hardware bit count */
#  undef  LZ4_FORCE_SW_BITCOUNT  /* avoid double def */
#  define LZ4_FORCE_SW_BITCOUNT
#endif

/*-************************************
*  Dependency
**************************************/
/*
 * LZ4_SRC_INCLUDED:
 * Amalgamation flag, whether lz4.c is included
 */
#ifndef LZ4_SRC_INCLUDED
#  define LZ4_SRC_INCLUDED 1
#endif

#ifndef LZ4_STATIC_LINKING_ONLY
#define LZ4_STATIC_LINKING_ONLY
#endif

#ifndef LZ4_DISABLE_DEPRECATE_WARNINGS
#define LZ4_DISABLE_DEPRECATE_WARNINGS /* due to LZ4_decompress_safe_withPrefix64k */
#endif

#define LZ4_STATIC_LINKING_ONLY  /* LZ4_DISTANCE_MAX */

/*** Start of inlined file: tracy_lz4.hpp ***/
#ifndef TRACY_LZ4_H_2983827168210
#define TRACY_LZ4_H_2983827168210

/* --- Dependency --- */
#include <cstddef>   /* size_t */
#include <cstdint>

/**
  Introduction

  LZ4 is lossless compression algorithm, providing compression speed >500 MB/s per core,
  scalable with multi-cores CPU. It features an extremely fast decoder, with speed in
  multiple GB/s per core, typically reaching RAM speed limits on multi-core systems.

  The LZ4 compression library provides in-memory compression and decompression functions.
  It gives full buffer control to user.
  Compression can be done in:
    - a single step (described as Simple Functions)
    - a single step, reusing a context (described in Advanced Functions)
    - unbounded multiple steps (described as Streaming compression)

  lz4.h generates and decodes LZ4-compressed blocks (doc/lz4_Block_format.md).
  Decompressing such a compressed block requires additional metadata.
  Exact metadata depends on exact decompression function.
  For the typical case of LZ4_decompress_safe(),
  metadata includes block's compressed size, and maximum bound of decompressed size.
  Each application is free to encode and pass such metadata in whichever way it wants.

  lz4.h only handle blocks, it can not generate Frames.

  Blocks are different from Frames (doc/lz4_Frame_format.md).
  Frames bundle both blocks and metadata in a specified manner.
  Embedding metadata is required for compressed data to be self-contained and portable.
  Frame format is delivered through a companion API, declared in lz4frame.h.
  The `lz4` CLI can only manage frames.
*/

/*^***************************************************************
*  Export parameters
*****************************************************************/
/*
*  LZ4_DLL_EXPORT :
*  Enable exporting of functions when building a Windows DLL
*  LZ4LIB_VISIBILITY :
*  Control library symbols visibility.
*/
#ifndef LZ4LIB_VISIBILITY
#  if defined(__GNUC__) && (__GNUC__ >= 4)
#    define LZ4LIB_VISIBILITY __attribute__ ((visibility ("default")))
#  else
#    define LZ4LIB_VISIBILITY
#  endif
#endif
#if defined(LZ4_DLL_EXPORT) && (LZ4_DLL_EXPORT==1)
#  define LZ4LIB_API __declspec(dllexport) LZ4LIB_VISIBILITY
#elif defined(LZ4_DLL_IMPORT) && (LZ4_DLL_IMPORT==1)
#  define LZ4LIB_API __declspec(dllimport) LZ4LIB_VISIBILITY /* It isn't required but allows to generate better code, saving a function pointer load from the IAT and an indirect jump.*/
#else
#  define LZ4LIB_API LZ4LIB_VISIBILITY
#endif

/*------   Version   ------*/
#define LZ4_VERSION_MAJOR    1    /* for breaking interface changes  */
#define LZ4_VERSION_MINOR    9    /* for new (non-breaking) interface capabilities */
#define LZ4_VERSION_RELEASE  3    /* for tweaks, bug-fixes, or development */

#define LZ4_VERSION_NUMBER (LZ4_VERSION_MAJOR *100*100 + LZ4_VERSION_MINOR *100 + LZ4_VERSION_RELEASE)

#define LZ4_LIB_VERSION LZ4_VERSION_MAJOR.LZ4_VERSION_MINOR.LZ4_VERSION_RELEASE
#define LZ4_QUOTE(str) #str
#define LZ4_EXPAND_AND_QUOTE(str) LZ4_QUOTE(str)
#define LZ4_VERSION_STRING LZ4_EXPAND_AND_QUOTE(LZ4_LIB_VERSION)

namespace tracy
{

LZ4LIB_API int LZ4_versionNumber (void);  /**< library version number; useful to check dll version */
LZ4LIB_API const char* LZ4_versionString (void);   /**< library version string; useful to check dll version */

/*-************************************
*  Tuning parameter
**************************************/
/*!
 * LZ4_MEMORY_USAGE :
 * Memory usage formula : N->2^N Bytes (examples : 10 -> 1KB; 12 -> 4KB ; 16 -> 64KB; 20 -> 1MB; etc.)
 * Increasing memory usage improves compression ratio.
 * Reduced memory usage may improve speed, thanks to better cache locality.
 * Default value is 14, for 16KB, which nicely fits into Intel x86 L1 cache
 */
#ifndef LZ4_MEMORY_USAGE
# define LZ4_MEMORY_USAGE 14
#endif

/*-************************************
*  Simple Functions
**************************************/
/*! LZ4_compress_default() :
 *  Compresses 'srcSize' bytes from buffer 'src'
 *  into already allocated 'dst' buffer of size 'dstCapacity'.
 *  Compression is guaranteed to succeed if 'dstCapacity' >= LZ4_compressBound(srcSize).
 *  It also runs faster, so it's a recommended setting.
 *  If the function cannot compress 'src' into a more limited 'dst' budget,
 *  compression stops *immediately*, and the function result is zero.
 *  In which case, 'dst' content is undefined (invalid).
 *      srcSize : max supported value is LZ4_MAX_INPUT_SIZE.
 *      dstCapacity : size of buffer 'dst' (which must be already allocated)
 *     @return  : the number of bytes written into buffer 'dst' (necessarily <= dstCapacity)
 *                or 0 if compression fails
 * Note : This function is protected against buffer overflow scenarios (never writes outside 'dst' buffer, nor read outside 'source' buffer).
 */
LZ4LIB_API int LZ4_compress_default(const char* src, char* dst, int srcSize, int dstCapacity);

/*! LZ4_decompress_safe() :
 *  compressedSize : is the exact complete size of the compressed block.
 *  dstCapacity : is the size of destination buffer (which must be already allocated), presumed an upper bound of decompressed size.
 * @return : the number of bytes decompressed into destination buffer (necessarily <= dstCapacity)
 *           If destination buffer is not large enough, decoding will stop and output an error code (negative value).
 *           If the source stream is detected malformed, the function will stop decoding and return a negative result.
 * Note 1 : This function is protected against malicious data packets :
 *          it will never writes outside 'dst' buffer, nor read outside 'source' buffer,
 *          even if the compressed block is maliciously modified to order the decoder to do these actions.
 *          In such case, the decoder stops immediately, and considers the compressed block malformed.
 * Note 2 : compressedSize and dstCapacity must be provided to the function, the compressed block does not contain them.
 *          The implementation is free to send / store / derive this information in whichever way is most beneficial.
 *          If there is a need for a different format which bundles together both compressed data and its metadata, consider looking at lz4frame.h instead.
 */
LZ4LIB_API int LZ4_decompress_safe (const char* src, char* dst, int compressedSize, int dstCapacity);

/*-************************************
*  Advanced Functions
**************************************/
#define LZ4_MAX_INPUT_SIZE        0x7E000000   /* 2 113 929 216 bytes */
#define LZ4_COMPRESSBOUND(isize)  ((unsigned)(isize) > (unsigned)LZ4_MAX_INPUT_SIZE ? 0 : (isize) + ((isize)/255) + 16)

/*! LZ4_compressBound() :
    Provides the maximum size that LZ4 compression may output in a "worst case" scenario (input data not compressible)
    This function is primarily useful for memory allocation purposes (destination buffer size).
    Macro LZ4_COMPRESSBOUND() is also provided for compilation-time evaluation (stack memory allocation for example).
    Note that LZ4_compress_default() compresses faster when dstCapacity is >= LZ4_compressBound(srcSize)
        inputSize  : max supported value is LZ4_MAX_INPUT_SIZE
        return : maximum output size in a "worst case" scenario
              or 0, if input size is incorrect (too large or negative)
*/
LZ4LIB_API int LZ4_compressBound(int inputSize);

/*! LZ4_compress_fast() :
    Same as LZ4_compress_default(), but allows selection of "acceleration" factor.
    The larger the acceleration value, the faster the algorithm, but also the lesser the compression.
    It's a trade-off. It can be fine tuned, with each successive value providing roughly +~3% to speed.
    An acceleration value of "1" is the same as regular LZ4_compress_default()
    Values <= 0 will be replaced by LZ4_ACCELERATION_DEFAULT (currently == 1, see lz4.c).
    Values > LZ4_ACCELERATION_MAX will be replaced by LZ4_ACCELERATION_MAX (currently == 65537, see lz4.c).
*/
LZ4LIB_API int LZ4_compress_fast (const char* src, char* dst, int srcSize, int dstCapacity, int acceleration);

/*! LZ4_compress_fast_extState() :
 *  Same as LZ4_compress_fast(), using an externally allocated memory space for its state.
 *  Use LZ4_sizeofState() to know how much memory must be allocated,
 *  and allocate it on 8-bytes boundaries (using `malloc()` typically).
 *  Then, provide this buffer as `void* state` to compression function.
 */
LZ4LIB_API int LZ4_sizeofState(void);
LZ4LIB_API int LZ4_compress_fast_extState (void* state, const char* src, char* dst, int srcSize, int dstCapacity, int acceleration);

/*! LZ4_compress_destSize() :
 *  Reverse the logic : compresses as much data as possible from 'src' buffer
 *  into already allocated buffer 'dst', of size >= 'targetDestSize'.
 *  This function either compresses the entire 'src' content into 'dst' if it's large enough,
 *  or fill 'dst' buffer completely with as much data as possible from 'src'.
 *  note: acceleration parameter is fixed to "default".
 *
 * *srcSizePtr : will be modified to indicate how many bytes where read from 'src' to fill 'dst'.
 *               New value is necessarily <= input value.
 * @return : Nb bytes written into 'dst' (necessarily <= targetDestSize)
 *           or 0 if compression fails.
 *
 * Note : from v1.8.2 to v1.9.1, this function had a bug (fixed un v1.9.2+):
 *        the produced compressed content could, in specific circumstances,
 *        require to be decompressed into a destination buffer larger
 *        by at least 1 byte than the content to decompress.
 *        If an application uses `LZ4_compress_destSize()`,
 *        it's highly recommended to update liblz4 to v1.9.2 or better.
 *        If this can't be done or ensured,
 *        the receiving decompression function should provide
 *        a dstCapacity which is > decompressedSize, by at least 1 byte.
 *        See https://github.com/lz4/lz4/issues/859 for details
 */
LZ4LIB_API int LZ4_compress_destSize (const char* src, char* dst, int* srcSizePtr, int targetDstSize);

/*! LZ4_decompress_safe_partial() :
 *  Decompress an LZ4 compressed block, of size 'srcSize' at position 'src',
 *  into destination buffer 'dst' of size 'dstCapacity'.
 *  Up to 'targetOutputSize' bytes will be decoded.
 *  The function stops decoding on reaching this objective.
 *  This can be useful to boost performance
 *  whenever only the beginning of a block is required.
 *
 * @return : the number of bytes decoded in `dst` (necessarily <= targetOutputSize)
 *           If source stream is detected malformed, function returns a negative result.
 *
 *  Note 1 : @return can be < targetOutputSize, if compressed block contains less data.
 *
 *  Note 2 : targetOutputSize must be <= dstCapacity
 *
 *  Note 3 : this function effectively stops decoding on reaching targetOutputSize,
 *           so dstCapacity is kind of redundant.
 *           This is because in older versions of this function,
 *           decoding operation would still write complete sequences.
 *           Therefore, there was no guarantee that it would stop writing at exactly targetOutputSize,
 *           it could write more bytes, though only up to dstCapacity.
 *           Some "margin" used to be required for this operation to work properly.
 *           Thankfully, this is no longer necessary.
 *           The function nonetheless keeps the same signature, in an effort to preserve API compatibility.
 *
 *  Note 4 : If srcSize is the exact size of the block,
 *           then targetOutputSize can be any value,
 *           including larger than the block's decompressed size.
 *           The function will, at most, generate block's decompressed size.
 *
 *  Note 5 : If srcSize is _larger_ than block's compressed size,
 *           then targetOutputSize **MUST** be <= block's decompressed size.
 *           Otherwise, *silent corruption will occur*.
 */
LZ4LIB_API int LZ4_decompress_safe_partial (const char* src, char* dst, int srcSize, int targetOutputSize, int dstCapacity);

/*-*********************************************
*  Streaming Compression Functions
***********************************************/
typedef union LZ4_stream_u LZ4_stream_t;  /* incomplete type (defined later) */

LZ4LIB_API LZ4_stream_t* LZ4_createStream(void);
LZ4LIB_API int           LZ4_freeStream (LZ4_stream_t* streamPtr);

/*! LZ4_resetStream_fast() : v1.9.0+
 *  Use this to prepare an LZ4_stream_t for a new chain of dependent blocks
 *  (e.g., LZ4_compress_fast_continue()).
 *
 *  An LZ4_stream_t must be initialized once before usage.
 *  This is automatically done when created by LZ4_createStream().
 *  However, should the LZ4_stream_t be simply declared on stack (for example),
 *  it's necessary to initialize it first, using LZ4_initStream().
 *
 *  After init, start any new stream with LZ4_resetStream_fast().
 *  A same LZ4_stream_t can be re-used multiple times consecutively
 *  and compress multiple streams,
 *  provided that it starts each new stream with LZ4_resetStream_fast().
 *
 *  LZ4_resetStream_fast() is much faster than LZ4_initStream(),
 *  but is not compatible with memory regions containing garbage data.
 *
 *  Note: it's only useful to call LZ4_resetStream_fast()
 *        in the context of streaming compression.
 *        The *extState* functions perform their own resets.
 *        Invoking LZ4_resetStream_fast() before is redundant, and even counterproductive.
 */
LZ4LIB_API void LZ4_resetStream_fast (LZ4_stream_t* streamPtr);

/*! LZ4_loadDict() :
 *  Use this function to reference a static dictionary into LZ4_stream_t.
 *  The dictionary must remain available during compression.
 *  LZ4_loadDict() triggers a reset, so any previous data will be forgotten.
 *  The same dictionary will have to be loaded on decompression side for successful decoding.
 *  Dictionary are useful for better compression of small data (KB range).
 *  While LZ4 accept any input as dictionary,
 *  results are generally better when using Zstandard's Dictionary Builder.
 *  Loading a size of 0 is allowed, and is the same as reset.
 * @return : loaded dictionary size, in bytes (necessarily <= 64 KB)
 */
LZ4LIB_API int LZ4_loadDict (LZ4_stream_t* streamPtr, const char* dictionary, int dictSize);

/*! LZ4_compress_fast_continue() :
 *  Compress 'src' content using data from previously compressed blocks, for better compression ratio.
 * 'dst' buffer must be already allocated.
 *  If dstCapacity >= LZ4_compressBound(srcSize), compression is guaranteed to succeed, and runs faster.
 *
 * @return : size of compressed block
 *           or 0 if there is an error (typically, cannot fit into 'dst').
 *
 *  Note 1 : Each invocation to LZ4_compress_fast_continue() generates a new block.
 *           Each block has precise boundaries.
 *           Each block must be decompressed separately, calling LZ4_decompress_*() with relevant metadata.
 *           It's not possible to append blocks together and expect a single invocation of LZ4_decompress_*() to decompress them together.
 *
 *  Note 2 : The previous 64KB of source data is __assumed__ to remain present, unmodified, at same address in memory !
 *
 *  Note 3 : When input is structured as a double-buffer, each buffer can have any size, including < 64 KB.
 *           Make sure that buffers are separated, by at least one byte.
 *           This construction ensures that each block only depends on previous block.
 *
 *  Note 4 : If input buffer is a ring-buffer, it can have any size, including < 64 KB.
 *
 *  Note 5 : After an error, the stream status is undefined (invalid), it can only be reset or freed.
 */
LZ4LIB_API int LZ4_compress_fast_continue (LZ4_stream_t* streamPtr, const char* src, char* dst, int srcSize, int dstCapacity, int acceleration);

/*! LZ4_saveDict() :
 *  If last 64KB data cannot be guaranteed to remain available at its current memory location,
 *  save it into a safer place (char* safeBuffer).
 *  This is schematically equivalent to a memcpy() followed by LZ4_loadDict(),
 *  but is much faster, because LZ4_saveDict() doesn't need to rebuild tables.
 * @return : saved dictionary size in bytes (necessarily <= maxDictSize), or 0 if error.
 */
LZ4LIB_API int LZ4_saveDict (LZ4_stream_t* streamPtr, char* safeBuffer, int maxDictSize);

/*-**********************************************
*  Streaming Decompression Functions
*  Bufferless synchronous API
************************************************/
typedef union LZ4_streamDecode_u LZ4_streamDecode_t;   /* tracking context */

/*! LZ4_createStreamDecode() and LZ4_freeStreamDecode() :
 *  creation / destruction of streaming decompression tracking context.
 *  A tracking context can be re-used multiple times.
 */
LZ4LIB_API LZ4_streamDecode_t* LZ4_createStreamDecode(void);
LZ4LIB_API int                 LZ4_freeStreamDecode (LZ4_streamDecode_t* LZ4_stream);

/*! LZ4_setStreamDecode() :
 *  An LZ4_streamDecode_t context can be allocated once and re-used multiple times.
 *  Use this function to start decompression of a new stream of blocks.
 *  A dictionary can optionally be set. Use NULL or size 0 for a reset order.
 *  Dictionary is presumed stable : it must remain accessible and unmodified during next decompression.
 * @return : 1 if OK, 0 if error
 */
LZ4LIB_API int LZ4_setStreamDecode (LZ4_streamDecode_t* LZ4_streamDecode, const char* dictionary, int dictSize);

/*! LZ4_decoderRingBufferSize() : v1.8.2+
 *  Note : in a ring buffer scenario (optional),
 *  blocks are presumed decompressed next to each other
 *  up to the moment there is not enough remaining space for next block (remainingSize < maxBlockSize),
 *  at which stage it resumes from beginning of ring buffer.
 *  When setting such a ring buffer for streaming decompression,
 *  provides the minimum size of this ring buffer
 *  to be compatible with any source respecting maxBlockSize condition.
 * @return : minimum ring buffer size,
 *           or 0 if there is an error (invalid maxBlockSize).
 */
LZ4LIB_API int LZ4_decoderRingBufferSize(int maxBlockSize);
#define LZ4_DECODER_RING_BUFFER_SIZE(maxBlockSize) (65536 + 14 + (maxBlockSize))  /* for static allocation; maxBlockSize presumed valid */

/*! LZ4_decompress_*_continue() :
 *  These decoding functions allow decompression of consecutive blocks in "streaming" mode.
 *  A block is an unsplittable entity, it must be presented entirely to a decompression function.
 *  Decompression functions only accepts one block at a time.
 *  The last 64KB of previously decoded data *must* remain available and unmodified at the memory position where they were decoded.
 *  If less than 64KB of data has been decoded, all the data must be present.
 *
 *  Special : if decompression side sets a ring buffer, it must respect one of the following conditions :
 *  - Decompression buffer size is _at least_ LZ4_decoderRingBufferSize(maxBlockSize).
 *    maxBlockSize is the maximum size of any single block. It can have any value > 16 bytes.
 *    In which case, encoding and decoding buffers do not need to be synchronized.
 *    Actually, data can be produced by any source compliant with LZ4 format specification, and respecting maxBlockSize.
 *  - Synchronized mode :
 *    Decompression buffer size is _exactly_ the same as compression buffer size,
 *    and follows exactly same update rule (block boundaries at same positions),
 *    and decoding function is provided with exact decompressed size of each block (exception for last block of the stream),
 *    _then_ decoding & encoding ring buffer can have any size, including small ones ( < 64 KB).
 *  - Decompression buffer is larger than encoding buffer, by a minimum of maxBlockSize more bytes.
 *    In which case, encoding and decoding buffers do not need to be synchronized,
 *    and encoding ring buffer can have any size, including small ones ( < 64 KB).
 *
 *  Whenever these conditions are not possible,
 *  save the last 64KB of decoded data into a safe buffer where it can't be modified during decompression,
 *  then indicate where this data is saved using LZ4_setStreamDecode(), before decompressing next block.
*/
LZ4LIB_API int LZ4_decompress_safe_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* src, char* dst, int srcSize, int dstCapacity);

/*! LZ4_decompress_*_usingDict() :
 *  These decoding functions work the same as
 *  a combination of LZ4_setStreamDecode() followed by LZ4_decompress_*_continue()
 *  They are stand-alone, and don't need an LZ4_streamDecode_t structure.
 *  Dictionary is presumed stable : it must remain accessible and unmodified during decompression.
 *  Performance tip : Decompression speed can be substantially increased
 *                    when dst == dictStart + dictSize.
 */
LZ4LIB_API int LZ4_decompress_safe_usingDict (const char* src, char* dst, int srcSize, int dstCapcity, const char* dictStart, int dictSize);

}

#endif /* LZ4_H_2983827168210 */

/*^*************************************
 * !!!!!!   STATIC LINKING ONLY   !!!!!!
 ***************************************/

/*-****************************************************************************
 * Experimental section
 *
 * Symbols declared in this section must be considered unstable. Their
 * signatures or semantics may change, or they may be removed altogether in the
 * future. They are therefore only safe to depend on when the caller is
 * statically linked against the library.
 *
 * To protect against unsafe usage, not only are the declarations guarded,
 * the definitions are hidden by default
 * when building LZ4 as a shared/dynamic library.
 *
 * In order to access these declarations,
 * define LZ4_STATIC_LINKING_ONLY in your application
 * before including LZ4's headers.
 *
 * In order to make their implementations accessible dynamically, you must
 * define LZ4_PUBLISH_STATIC_FUNCTIONS when building the LZ4 library.
 ******************************************************************************/

#ifdef LZ4_STATIC_LINKING_ONLY

#ifndef TRACY_LZ4_STATIC_3504398509
#define TRACY_LZ4_STATIC_3504398509

#ifdef LZ4_PUBLISH_STATIC_FUNCTIONS
#define LZ4LIB_STATIC_API LZ4LIB_API
#else
#define LZ4LIB_STATIC_API
#endif

namespace tracy
{

/*! LZ4_compress_fast_extState_fastReset() :
 *  A variant of LZ4_compress_fast_extState().
 *
 *  Using this variant avoids an expensive initialization step.
 *  It is only safe to call if the state buffer is known to be correctly initialized already
 *  (see above comment on LZ4_resetStream_fast() for a definition of "correctly initialized").
 *  From a high level, the difference is that
 *  this function initializes the provided state with a call to something like LZ4_resetStream_fast()
 *  while LZ4_compress_fast_extState() starts with a call to LZ4_resetStream().
 */
LZ4LIB_STATIC_API int LZ4_compress_fast_extState_fastReset (void* state, const char* src, char* dst, int srcSize, int dstCapacity, int acceleration);

/*! LZ4_attach_dictionary() :
 *  This is an experimental API that allows
 *  efficient use of a static dictionary many times.
 *
 *  Rather than re-loading the dictionary buffer into a working context before
 *  each compression, or copying a pre-loaded dictionary's LZ4_stream_t into a
 *  working LZ4_stream_t, this function introduces a no-copy setup mechanism,
 *  in which the working stream references the dictionary stream in-place.
 *
 *  Several assumptions are made about the state of the dictionary stream.
 *  Currently, only streams which have been prepared by LZ4_loadDict() should
 *  be expected to work.
 *
 *  Alternatively, the provided dictionaryStream may be NULL,
 *  in which case any existing dictionary stream is unset.
 *
 *  If a dictionary is provided, it replaces any pre-existing stream history.
 *  The dictionary contents are the only history that can be referenced and
 *  logically immediately precede the data compressed in the first subsequent
 *  compression call.
 *
 *  The dictionary will only remain attached to the working stream through the
 *  first compression call, at the end of which it is cleared. The dictionary
 *  stream (and source buffer) must remain in-place / accessible / unchanged
 *  through the completion of the first compression call on the stream.
 */
LZ4LIB_STATIC_API void LZ4_attach_dictionary(LZ4_stream_t* workingStream, const LZ4_stream_t* dictionaryStream);

/*! In-place compression and decompression
 *
 * It's possible to have input and output sharing the same buffer,
 * for highly contrained memory environments.
 * In both cases, it requires input to lay at the end of the buffer,
 * and decompression to start at beginning of the buffer.
 * Buffer size must feature some margin, hence be larger than final size.
 *
 * |<------------------------buffer--------------------------------->|
 *                             |<-----------compressed data--------->|
 * |<-----------decompressed size------------------>|
 *                                                  |<----margin---->|
 *
 * This technique is more useful for decompression,
 * since decompressed size is typically larger,
 * and margin is short.
 *
 * In-place decompression will work inside any buffer
 * which size is >= LZ4_DECOMPRESS_INPLACE_BUFFER_SIZE(decompressedSize).
 * This presumes that decompressedSize > compressedSize.
 * Otherwise, it means compression actually expanded data,
 * and it would be more efficient to store such data with a flag indicating it's not compressed.
 * This can happen when data is not compressible (already compressed, or encrypted).
 *
 * For in-place compression, margin is larger, as it must be able to cope with both
 * history preservation, requiring input data to remain unmodified up to LZ4_DISTANCE_MAX,
 * and data expansion, which can happen when input is not compressible.
 * As a consequence, buffer size requirements are much higher,
 * and memory savings offered by in-place compression are more limited.
 *
 * There are ways to limit this cost for compression :
 * - Reduce history size, by modifying LZ4_DISTANCE_MAX.
 *   Note that it is a compile-time constant, so all compressions will apply this limit.
 *   Lower values will reduce compression ratio, except when input_size < LZ4_DISTANCE_MAX,
 *   so it's a reasonable trick when inputs are known to be small.
 * - Require the compressor to deliver a "maximum compressed size".
 *   This is the `dstCapacity` parameter in `LZ4_compress*()`.
 *   When this size is < LZ4_COMPRESSBOUND(inputSize), then compression can fail,
 *   in which case, the return code will be 0 (zero).
 *   The caller must be ready for these cases to happen,
 *   and typically design a backup scheme to send data uncompressed.
 * The combination of both techniques can significantly reduce
 * the amount of margin required for in-place compression.
 *
 * In-place compression can work in any buffer
 * which size is >= (maxCompressedSize)
 * with maxCompressedSize == LZ4_COMPRESSBOUND(srcSize) for guaranteed compression success.
 * LZ4_COMPRESS_INPLACE_BUFFER_SIZE() depends on both maxCompressedSize and LZ4_DISTANCE_MAX,
 * so it's possible to reduce memory requirements by playing with them.
 */

#define LZ4_DECOMPRESS_INPLACE_MARGIN(compressedSize)          (((compressedSize) >> 8) + 32)
#define LZ4_DECOMPRESS_INPLACE_BUFFER_SIZE(decompressedSize)   ((decompressedSize) + LZ4_DECOMPRESS_INPLACE_MARGIN(decompressedSize))  /**< note: presumes that compressedSize < decompressedSize. note2: margin is overestimated a bit, since it could use compressedSize instead */

#ifndef LZ4_DISTANCE_MAX   /* history window size; can be user-defined at compile time */
#  define LZ4_DISTANCE_MAX 65535   /* set to maximum value by default */
#endif

#define LZ4_COMPRESS_INPLACE_MARGIN                           (LZ4_DISTANCE_MAX + 32)   /* LZ4_DISTANCE_MAX can be safely replaced by srcSize when it's smaller */
#define LZ4_COMPRESS_INPLACE_BUFFER_SIZE(maxCompressedSize)   ((maxCompressedSize) + LZ4_COMPRESS_INPLACE_MARGIN)  /**< maxCompressedSize is generally LZ4_COMPRESSBOUND(inputSize), but can be set to any lower value, with the risk that compression can fail (return code 0(zero)) */

}

#endif   /* LZ4_STATIC_3504398509 */
#endif   /* LZ4_STATIC_LINKING_ONLY */

#ifndef TRACY_LZ4_H_98237428734687
#define TRACY_LZ4_H_98237428734687

namespace tracy
{

/*-************************************************************
 *  Private Definitions
 **************************************************************
 * Do not use these definitions directly.
 * They are only exposed to allow static allocation of `LZ4_stream_t` and `LZ4_streamDecode_t`.
 * Accessing members will expose user code to API and/or ABI break in future versions of the library.
 **************************************************************/
#define LZ4_HASHLOG   (LZ4_MEMORY_USAGE-2)
#define LZ4_HASHTABLESIZE (1 << LZ4_MEMORY_USAGE)
#define LZ4_HASH_SIZE_U32 (1 << LZ4_HASHLOG)       /* required as macro for static allocation */

#if defined(__cplusplus) || (defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* C99 */)
  typedef  int8_t  LZ4_i8;
  typedef uint8_t  LZ4_byte;
  typedef uint16_t LZ4_u16;
  typedef uint32_t LZ4_u32;
#else
  typedef   signed char  LZ4_i8;
  typedef unsigned char  LZ4_byte;
  typedef unsigned short LZ4_u16;
  typedef unsigned int   LZ4_u32;
#endif

typedef struct LZ4_stream_t_internal LZ4_stream_t_internal;
struct LZ4_stream_t_internal {
    LZ4_u32 hashTable[LZ4_HASH_SIZE_U32];
    LZ4_u32 currentOffset;
    LZ4_u32 tableType;
    const LZ4_byte* dictionary;
    const LZ4_stream_t_internal* dictCtx;
    LZ4_u32 dictSize;
};

typedef struct {
    const LZ4_byte* externalDict;
    size_t extDictSize;
    const LZ4_byte* prefixEnd;
    size_t prefixSize;
} LZ4_streamDecode_t_internal;

/*! LZ4_stream_t :
 *  Do not use below internal definitions directly !
 *  Declare or allocate an LZ4_stream_t instead.
 *  LZ4_stream_t can also be created using LZ4_createStream(), which is recommended.
 *  The structure definition can be convenient for static allocation
 *  (on stack, or as part of larger structure).
 *  Init this structure with LZ4_initStream() before first use.
 *  note : only use this definition in association with static linking !
 *  this definition is not API/ABI safe, and may change in future versions.
 */
#define LZ4_STREAMSIZE       16416  /* static size, for inter-version compatibility */
#define LZ4_STREAMSIZE_VOIDP (LZ4_STREAMSIZE / sizeof(void*))
union LZ4_stream_u {
    void* table[LZ4_STREAMSIZE_VOIDP];
    LZ4_stream_t_internal internal_donotuse;
}; /* previously typedef'd to LZ4_stream_t */

/*! LZ4_initStream() : v1.9.0+
 *  An LZ4_stream_t structure must be initialized at least once.
 *  This is automatically done when invoking LZ4_createStream(),
 *  but it's not when the structure is simply declared on stack (for example).
 *
 *  Use LZ4_initStream() to properly initialize a newly declared LZ4_stream_t.
 *  It can also initialize any arbitrary buffer of sufficient size,
 *  and will @return a pointer of proper type upon initialization.
 *
 *  Note : initialization fails if size and alignment conditions are not respected.
 *         In which case, the function will @return NULL.
 *  Note2: An LZ4_stream_t structure guarantees correct alignment and size.
 *  Note3: Before v1.9.0, use LZ4_resetStream() instead
 */
LZ4LIB_API LZ4_stream_t* LZ4_initStream (void* buffer, size_t size);

/*! LZ4_streamDecode_t :
 *  information structure to track an LZ4 stream during decompression.
 *  init this structure  using LZ4_setStreamDecode() before first use.
 *  note : only use in association with static linking !
 *         this definition is not API/ABI safe,
 *         and may change in a future version !
 */
#define LZ4_STREAMDECODESIZE_U64 (4 + ((sizeof(void*)==16) ? 2 : 0) /*AS-400*/ )
#define LZ4_STREAMDECODESIZE     (LZ4_STREAMDECODESIZE_U64 * sizeof(unsigned long long))
union LZ4_streamDecode_u {
    unsigned long long table[LZ4_STREAMDECODESIZE_U64];
    LZ4_streamDecode_t_internal internal_donotuse;
} ;   /* previously typedef'd to LZ4_streamDecode_t */

/*-************************************
*  Obsolete Functions
**************************************/

/*! Deprecation warnings
 *
 *  Deprecated functions make the compiler generate a warning when invoked.
 *  This is meant to invite users to update their source code.
 *  Should deprecation warnings be a problem, it is generally possible to disable them,
 *  typically with -Wno-deprecated-declarations for gcc
 *  or _CRT_SECURE_NO_WARNINGS in Visual.
 *
 *  Another method is to define LZ4_DISABLE_DEPRECATE_WARNINGS
 *  before including the header file.
 */
#ifdef LZ4_DISABLE_DEPRECATE_WARNINGS
#  define LZ4_DEPRECATED(message)   /* disable deprecation warnings */
#else
#  if defined (__cplusplus) && (__cplusplus >= 201402) /* C++14 or greater */
#    define LZ4_DEPRECATED(message) [[deprecated(message)]]
#  elif defined(_MSC_VER)
#    define LZ4_DEPRECATED(message) __declspec(deprecated(message))
#  elif defined(__clang__) || (defined(__GNUC__) && (__GNUC__ * 10 + __GNUC_MINOR__ >= 45))
#    define LZ4_DEPRECATED(message) __attribute__((deprecated(message)))
#  elif defined(__GNUC__) && (__GNUC__ * 10 + __GNUC_MINOR__ >= 31)
#    define LZ4_DEPRECATED(message) __attribute__((deprecated))
#  else
#    pragma message("WARNING: LZ4_DEPRECATED needs custom implementation for this compiler")
#    define LZ4_DEPRECATED(message)   /* disabled */
#  endif
#endif /* LZ4_DISABLE_DEPRECATE_WARNINGS */

/*! Obsolete compression functions (since v1.7.3) */
LZ4_DEPRECATED("use LZ4_compress_default() instead")       LZ4LIB_API int LZ4_compress               (const char* src, char* dest, int srcSize);
LZ4_DEPRECATED("use LZ4_compress_default() instead")       LZ4LIB_API int LZ4_compress_limitedOutput (const char* src, char* dest, int srcSize, int maxOutputSize);
LZ4_DEPRECATED("use LZ4_compress_fast_extState() instead") LZ4LIB_API int LZ4_compress_withState               (void* state, const char* source, char* dest, int inputSize);
LZ4_DEPRECATED("use LZ4_compress_fast_extState() instead") LZ4LIB_API int LZ4_compress_limitedOutput_withState (void* state, const char* source, char* dest, int inputSize, int maxOutputSize);
LZ4_DEPRECATED("use LZ4_compress_fast_continue() instead") LZ4LIB_API int LZ4_compress_continue                (LZ4_stream_t* LZ4_streamPtr, const char* source, char* dest, int inputSize);
LZ4_DEPRECATED("use LZ4_compress_fast_continue() instead") LZ4LIB_API int LZ4_compress_limitedOutput_continue  (LZ4_stream_t* LZ4_streamPtr, const char* source, char* dest, int inputSize, int maxOutputSize);

/*! Obsolete decompression functions (since v1.8.0) */
LZ4_DEPRECATED("use LZ4_decompress_fast() instead") LZ4LIB_API int LZ4_uncompress (const char* source, char* dest, int outputSize);
LZ4_DEPRECATED("use LZ4_decompress_safe() instead") LZ4LIB_API int LZ4_uncompress_unknownOutputSize (const char* source, char* dest, int isize, int maxOutputSize);

/* Obsolete streaming functions (since v1.7.0)
 * degraded functionality; do not use!
 *
 * In order to perform streaming compression, these functions depended on data
 * that is no longer tracked in the state. They have been preserved as well as
 * possible: using them will still produce a correct output. However, they don't
 * actually retain any history between compression calls. The compression ratio
 * achieved will therefore be no better than compressing each chunk
 * independently.
 */
LZ4_DEPRECATED("Use LZ4_createStream() instead") LZ4LIB_API void* LZ4_create (char* inputBuffer);
LZ4_DEPRECATED("Use LZ4_createStream() instead") LZ4LIB_API int   LZ4_sizeofStreamState(void);
LZ4_DEPRECATED("Use LZ4_resetStream() instead")  LZ4LIB_API int   LZ4_resetStreamState(void* state, char* inputBuffer);
LZ4_DEPRECATED("Use LZ4_saveDict() instead")     LZ4LIB_API char* LZ4_slideInputBuffer (void* state);

/*! Obsolete streaming decoding functions (since v1.7.0) */
LZ4_DEPRECATED("use LZ4_decompress_safe_usingDict() instead") LZ4LIB_API int LZ4_decompress_safe_withPrefix64k (const char* src, char* dst, int compressedSize, int maxDstSize);
LZ4_DEPRECATED("use LZ4_decompress_fast_usingDict() instead") LZ4LIB_API int LZ4_decompress_fast_withPrefix64k (const char* src, char* dst, int originalSize);

/*! Obsolete LZ4_decompress_fast variants (since v1.9.0) :
 *  These functions used to be faster than LZ4_decompress_safe(),
 *  but this is no longer the case. They are now slower.
 *  This is because LZ4_decompress_fast() doesn't know the input size,
 *  and therefore must progress more cautiously into the input buffer to not read beyond the end of block.
 *  On top of that `LZ4_decompress_fast()` is not protected vs malformed or malicious inputs, making it a security liability.
 *  As a consequence, LZ4_decompress_fast() is strongly discouraged, and deprecated.
 *
 *  The last remaining LZ4_decompress_fast() specificity is that
 *  it can decompress a block without knowing its compressed size.
 *  Such functionality can be achieved in a more secure manner
 *  by employing LZ4_decompress_safe_partial().
 *
 *  Parameters:
 *  originalSize : is the uncompressed size to regenerate.
 *                 `dst` must be already allocated, its size must be >= 'originalSize' bytes.
 * @return : number of bytes read from source buffer (== compressed size).
 *           The function expects to finish at block's end exactly.
 *           If the source stream is detected malformed, the function stops decoding and returns a negative result.
 *  note : LZ4_decompress_fast*() requires originalSize. Thanks to this information, it never writes past the output buffer.
 *         However, since it doesn't know its 'src' size, it may read an unknown amount of input, past input buffer bounds.
 *         Also, since match offsets are not validated, match reads from 'src' may underflow too.
 *         These issues never happen if input (compressed) data is correct.
 *         But they may happen if input data is invalid (error or intentional tampering).
 *         As a consequence, use these functions in trusted environments with trusted data **only**.
 */
LZ4_DEPRECATED("This function is deprecated and unsafe. Consider using LZ4_decompress_safe() instead")
LZ4LIB_API int LZ4_decompress_fast (const char* src, char* dst, int originalSize);
LZ4_DEPRECATED("This function is deprecated and unsafe. Consider using LZ4_decompress_safe_continue() instead")
LZ4LIB_API int LZ4_decompress_fast_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* src, char* dst, int originalSize);
LZ4_DEPRECATED("This function is deprecated and unsafe. Consider using LZ4_decompress_safe_usingDict() instead")
LZ4LIB_API int LZ4_decompress_fast_usingDict (const char* src, char* dst, int originalSize, const char* dictStart, int dictSize);

/*! LZ4_resetStream() :
 *  An LZ4_stream_t structure must be initialized at least once.
 *  This is done with LZ4_initStream(), or LZ4_resetStream().
 *  Consider switching to LZ4_initStream(),
 *  invoking LZ4_resetStream() will trigger deprecation warnings in the future.
 */
LZ4LIB_API void LZ4_resetStream (LZ4_stream_t* streamPtr);

}

#endif /* LZ4_H_98237428734687 */

/*** End of inlined file: tracy_lz4.hpp ***/


/* see also "memory routines" below */

/*-************************************
*  Compiler Options
**************************************/
#if defined(_MSC_VER) && (_MSC_VER >= 1400)  /* Visual Studio 2005+ */
#  include <intrin.h>               /* only present in VS2005+ */
#  pragma warning(disable : 4127)   /* disable: C4127: conditional expression is constant */
#endif  /* _MSC_VER */

#ifndef LZ4_FORCE_INLINE
#  ifdef _MSC_VER    /* Visual Studio */
#    define LZ4_FORCE_INLINE static __forceinline
#  else
#    if defined (__cplusplus) || defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L   /* C99 */
#      ifdef __GNUC__
#        define LZ4_FORCE_INLINE static inline __attribute__((always_inline))
#      else
#        define LZ4_FORCE_INLINE static inline
#      endif
#    else
#      define LZ4_FORCE_INLINE static
#    endif /* __STDC_VERSION__ */
#  endif  /* _MSC_VER */
#endif /* LZ4_FORCE_INLINE */

/* LZ4_FORCE_O2 and LZ4_FORCE_INLINE
 * gcc on ppc64le generates an unrolled SIMDized loop for LZ4_wildCopy8,
 * together with a simple 8-byte copy loop as a fall-back path.
 * However, this optimization hurts the decompression speed by >30%,
 * because the execution does not go to the optimized loop
 * for typical compressible data, and all of the preamble checks
 * before going to the fall-back path become useless overhead.
 * This optimization happens only with the -O3 flag, and -O2 generates
 * a simple 8-byte copy loop.
 * With gcc on ppc64le, all of the LZ4_decompress_* and LZ4_wildCopy8
 * functions are annotated with __attribute__((optimize("O2"))),
 * and also LZ4_wildCopy8 is forcibly inlined, so that the O2 attribute
 * of LZ4_wildCopy8 does not affect the compression speed.
 */
#if defined(__PPC64__) && defined(__LITTLE_ENDIAN__) && defined(__GNUC__) && !defined(__clang__)
#  define LZ4_FORCE_O2  __attribute__((optimize("O2")))
#  undef LZ4_FORCE_INLINE
#  define LZ4_FORCE_INLINE  static __inline __attribute__((optimize("O2"),always_inline))
#else
#  define LZ4_FORCE_O2
#endif

#if (defined(__GNUC__) && (__GNUC__ >= 3)) || (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 800)) || defined(__clang__)
#  define expect(expr,value)    (__builtin_expect ((expr),(value)) )
#else
#  define expect(expr,value)    (expr)
#endif

#ifndef likely
#define likely(expr)     expect((expr) != 0, 1)
#endif
#ifndef unlikely
#define unlikely(expr)   expect((expr) != 0, 0)
#endif

/* Should the alignment test prove unreliable, for some reason,
 * it can be disabled by setting LZ4_ALIGN_TEST to 0 */
#ifndef LZ4_ALIGN_TEST  /* can be externally provided */
# define LZ4_ALIGN_TEST 1
#endif

/*-************************************
*  Memory routines
**************************************/
#ifdef LZ4_USER_MEMORY_FUNCTIONS
/* memory management functions can be customized by user project.
 * Below functions must exist somewhere in the Project
 * and be available at link time */
void* LZ4_malloc(size_t s);
void* LZ4_calloc(size_t n, size_t s);
void  LZ4_free(void* p);
# define ALLOC(s)          LZ4_malloc(s)
# define ALLOC_AND_ZERO(s) LZ4_calloc(1,s)
# define FREEMEM(p)        LZ4_free(p)
#else
# include <stdlib.h>   /* malloc, calloc, free */
# define ALLOC(s)          malloc(s)
# define ALLOC_AND_ZERO(s) calloc(1,s)
# define FREEMEM(p)        free(p)
#endif

#include <string.h>   /* memset, memcpy */
#define MEM_INIT(p,v,s)   memset((p),(v),(s))

/*-************************************
*  Common Constants
**************************************/
#define MINMATCH 4

#define WILDCOPYLENGTH 8
#define LASTLITERALS   5   /* see ../doc/lz4_Block_format.md#parsing-restrictions */
#define MFLIMIT       12   /* see ../doc/lz4_Block_format.md#parsing-restrictions */
#define MATCH_SAFEGUARD_DISTANCE  ((2*WILDCOPYLENGTH) - MINMATCH)   /* ensure it's possible to write 2 x wildcopyLength without overflowing output buffer */
#define FASTLOOP_SAFE_DISTANCE 64
static const int LZ4_minLength = (MFLIMIT+1);

#define KB *(1 <<10)
#define MB *(1 <<20)
#define GB *(1U<<30)

#define LZ4_DISTANCE_ABSOLUTE_MAX 65535
#if (LZ4_DISTANCE_MAX > LZ4_DISTANCE_ABSOLUTE_MAX)   /* max supported by LZ4 format */
#  error "LZ4_DISTANCE_MAX is too big : must be <= 65535"
#endif

#define ML_BITS  4
#define ML_MASK  ((1U<<ML_BITS)-1)
#define RUN_BITS (8-ML_BITS)
#define RUN_MASK ((1U<<RUN_BITS)-1)

/*-************************************
*  Error detection
**************************************/
#if defined(LZ4_DEBUG) && (LZ4_DEBUG>=1)
#  include <cassert>
#else
#  ifndef assert
#    define assert(condition) ((void)0)
#  endif
#endif

#define LZ4_STATIC_ASSERT(c)   { enum { LZ4_static_assert = 1/(int)(!!(c)) }; }   /* use after variable declarations */

#if defined(LZ4_DEBUG) && (LZ4_DEBUG>=2)
#  include <stdio.h>
   static int g_debuglog_enable = 1;
#  define DEBUGLOG(l, ...) {                          \
        if ((g_debuglog_enable) && (l<=LZ4_DEBUG)) {  \
            fprintf(stderr, __FILE__ ": ");           \
            fprintf(stderr, __VA_ARGS__);             \
            fprintf(stderr, " \n");                   \
    }   }
#else
#  define DEBUGLOG(l, ...) {}    /* disabled */
#endif

static int LZ4_isAligned(const void* ptr, size_t alignment)
{
    return ((size_t)ptr & (alignment -1)) == 0;
}

/*-************************************
*  Types
**************************************/
#include <climits>
#if defined(__cplusplus) || (defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* C99 */)
# include <cstdint>
  typedef  uint8_t BYTE;
  typedef uint16_t U16;
  typedef uint32_t U32;
  typedef  int32_t S32;
  typedef uint64_t U64;
  typedef uintptr_t uptrval;
#else
# if UINT_MAX != 4294967295UL
#   error "LZ4 code (when not C++ or C99) assumes that sizeof(int) == 4"
# endif
  typedef unsigned char       BYTE;
  typedef unsigned short      U16;
  typedef unsigned int        U32;
  typedef   signed int        S32;
  typedef unsigned long long  U64;
  typedef size_t              uptrval;   /* generally true, except OpenVMS-64 */
#endif

#if defined(__x86_64__)
  typedef U64    reg_t;   /* 64-bits in x32 mode */
#else
  typedef size_t reg_t;   /* 32-bits in x32 mode */
#endif

typedef enum {
    notLimited = 0,
    limitedOutput = 1,
    fillOutput = 2
} limitedOutput_directive;

namespace tracy
{

/*-************************************
*  Reading and writing into memory
**************************************/

/**
 * LZ4 relies on memcpy with a constant size being inlined. In freestanding
 * environments, the compiler can't assume the implementation of memcpy() is
 * standard compliant, so it can't apply its specialized memcpy() inlining
 * logic. When possible, use __builtin_memcpy() to tell the compiler to analyze
 * memcpy() as if it were standard compliant, so it can inline it in freestanding
 * environments. This is needed when decompressing the Linux Kernel, for example.
 */
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define LZ4_memcpy(dst, src, size) __builtin_memcpy(dst, src, size)
#else
#define LZ4_memcpy(dst, src, size) memcpy(dst, src, size)
#endif

static unsigned LZ4_isLittleEndian(void)
{
    const union { U32 u; BYTE c[4]; } one = { 1 };   /* don't use static : performance detrimental */
    return one.c[0];
}

#if defined(LZ4_FORCE_MEMORY_ACCESS) && (LZ4_FORCE_MEMORY_ACCESS==2)
/* lie to the compiler about data alignment; use with caution */

static U16 LZ4_read16(const void* memPtr) { return *(const U16*) memPtr; }
static U32 LZ4_read32(const void* memPtr) { return *(const U32*) memPtr; }
static reg_t LZ4_read_ARCH(const void* memPtr) { return *(const reg_t*) memPtr; }

static void LZ4_write16(void* memPtr, U16 value) { *(U16*)memPtr = value; }
static void LZ4_write32(void* memPtr, U32 value) { *(U32*)memPtr = value; }

#elif defined(LZ4_FORCE_MEMORY_ACCESS) && (LZ4_FORCE_MEMORY_ACCESS==1)

/* __pack instructions are safer, but compiler specific, hence potentially problematic for some compilers */
/* currently only defined for gcc and icc */
typedef union { U16 u16; U32 u32; reg_t uArch; } __attribute__((packed)) unalign;

static U16 LZ4_read16(const void* ptr) { return ((const unalign*)ptr)->u16; }
static U32 LZ4_read32(const void* ptr) { return ((const unalign*)ptr)->u32; }
static reg_t LZ4_read_ARCH(const void* ptr) { return ((const unalign*)ptr)->uArch; }

static void LZ4_write16(void* memPtr, U16 value) { ((unalign*)memPtr)->u16 = value; }
static void LZ4_write32(void* memPtr, U32 value) { ((unalign*)memPtr)->u32 = value; }

#else  /* safe and portable access using memcpy() */

static U16 LZ4_read16(const void* memPtr)
{
    U16 val; LZ4_memcpy(&val, memPtr, sizeof(val)); return val;
}

static U32 LZ4_read32(const void* memPtr)
{
    U32 val; LZ4_memcpy(&val, memPtr, sizeof(val)); return val;
}

static reg_t LZ4_read_ARCH(const void* memPtr)
{
    reg_t val; LZ4_memcpy(&val, memPtr, sizeof(val)); return val;
}

static void LZ4_write16(void* memPtr, U16 value)
{
    LZ4_memcpy(memPtr, &value, sizeof(value));
}

static void LZ4_write32(void* memPtr, U32 value)
{
    LZ4_memcpy(memPtr, &value, sizeof(value));
}

#endif /* LZ4_FORCE_MEMORY_ACCESS */

static U16 LZ4_readLE16(const void* memPtr)
{
    if (LZ4_isLittleEndian()) {
        return LZ4_read16(memPtr);
    } else {
        const BYTE* p = (const BYTE*)memPtr;
        return (U16)((U16)p[0] + (p[1]<<8));
    }
}

static void LZ4_writeLE16(void* memPtr, U16 value)
{
    if (LZ4_isLittleEndian()) {
        LZ4_write16(memPtr, value);
    } else {
        BYTE* p = (BYTE*)memPtr;
        p[0] = (BYTE) value;
        p[1] = (BYTE)(value>>8);
    }
}

/* customized variant of memcpy, which can overwrite up to 8 bytes beyond dstEnd */
LZ4_FORCE_INLINE
void LZ4_wildCopy8(void* dstPtr, const void* srcPtr, void* dstEnd)
{
    BYTE* d = (BYTE*)dstPtr;
    const BYTE* s = (const BYTE*)srcPtr;
    BYTE* const e = (BYTE*)dstEnd;

    do { LZ4_memcpy(d,s,8); d+=8; s+=8; } while (d<e);
}

static const unsigned inc32table[8] = {0, 1, 2,  1,  0,  4, 4, 4};
static const int      dec64table[8] = {0, 0, 0, -1, -4,  1, 2, 3};

#ifndef LZ4_FAST_DEC_LOOP
#  if defined __i386__ || defined _M_IX86 || defined __x86_64__ || defined _M_X64
#    define LZ4_FAST_DEC_LOOP 1
#  elif defined(__aarch64__) && !defined(__clang__)
     /* On aarch64, we disable this optimization for clang because on certain
      * mobile chipsets, performance is reduced with clang. For information
      * refer to https://github.com/lz4/lz4/pull/707 */
#    define LZ4_FAST_DEC_LOOP 1
#  else
#    define LZ4_FAST_DEC_LOOP 0
#  endif
#endif

#if LZ4_FAST_DEC_LOOP

LZ4_FORCE_INLINE void
LZ4_memcpy_using_offset_base(BYTE* dstPtr, const BYTE* srcPtr, BYTE* dstEnd, const size_t offset)
{
    assert(srcPtr + offset == dstPtr);
    if (offset < 8) {
        LZ4_write32(dstPtr, 0);   /* silence an msan warning when offset==0 */
        dstPtr[0] = srcPtr[0];
        dstPtr[1] = srcPtr[1];
        dstPtr[2] = srcPtr[2];
        dstPtr[3] = srcPtr[3];
        srcPtr += inc32table[offset];
        LZ4_memcpy(dstPtr+4, srcPtr, 4);
        srcPtr -= dec64table[offset];
        dstPtr += 8;
    } else {
        LZ4_memcpy(dstPtr, srcPtr, 8);
        dstPtr += 8;
        srcPtr += 8;
    }

    LZ4_wildCopy8(dstPtr, srcPtr, dstEnd);
}

/* customized variant of memcpy, which can overwrite up to 32 bytes beyond dstEnd
 * this version copies two times 16 bytes (instead of one time 32 bytes)
 * because it must be compatible with offsets >= 16. */
LZ4_FORCE_INLINE void
LZ4_wildCopy32(void* dstPtr, const void* srcPtr, void* dstEnd)
{
    BYTE* d = (BYTE*)dstPtr;
    const BYTE* s = (const BYTE*)srcPtr;
    BYTE* const e = (BYTE*)dstEnd;

    do { LZ4_memcpy(d,s,16); LZ4_memcpy(d+16,s+16,16); d+=32; s+=32; } while (d<e);
}

/* LZ4_memcpy_using_offset()  presumes :
 * - dstEnd >= dstPtr + MINMATCH
 * - there is at least 8 bytes available to write after dstEnd */
LZ4_FORCE_INLINE void
LZ4_memcpy_using_offset(BYTE* dstPtr, const BYTE* srcPtr, BYTE* dstEnd, const size_t offset)
{
    BYTE v[8];

    assert(dstEnd >= dstPtr + MINMATCH);

    switch(offset) {
    case 1:
        MEM_INIT(v, *srcPtr, 8);
        break;
    case 2:
        LZ4_memcpy(v, srcPtr, 2);
        LZ4_memcpy(&v[2], srcPtr, 2);
        LZ4_memcpy(&v[4], v, 4);
        break;
    case 4:
        LZ4_memcpy(v, srcPtr, 4);
        LZ4_memcpy(&v[4], srcPtr, 4);
        break;
    default:
        LZ4_memcpy_using_offset_base(dstPtr, srcPtr, dstEnd, offset);
        return;
    }

    LZ4_memcpy(dstPtr, v, 8);
    dstPtr += 8;
    while (dstPtr < dstEnd) {
        LZ4_memcpy(dstPtr, v, 8);
        dstPtr += 8;
    }
}
#endif

/*-************************************
*  Common functions
**************************************/
LZ4_FORCE_INLINE unsigned LZ4_NbCommonBytes (reg_t val)
{
    assert(val != 0);
    if (LZ4_isLittleEndian()) {
        if (sizeof(val) == 8) {
#       if defined(_MSC_VER) && (_MSC_VER >= 1800) && defined(_M_AMD64) && !defined(LZ4_FORCE_SW_BITCOUNT)
            /* x64 CPUS without BMI support interpret `TZCNT` as `REP BSF` */
            return (unsigned)_tzcnt_u64(val) >> 3;
#       elif defined(_MSC_VER) && defined(_WIN64) && !defined(LZ4_FORCE_SW_BITCOUNT)
            unsigned long r = 0;
            _BitScanForward64(&r, (U64)val);
            return (unsigned)r >> 3;
#       elif (defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 3) || \
                            ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))))) && \
                                        !defined(LZ4_FORCE_SW_BITCOUNT)
            return (unsigned)__builtin_ctzll((U64)val) >> 3;
#       else
            const U64 m = 0x0101010101010101ULL;
            val ^= val - 1;
            return (unsigned)(((U64)((val & (m - 1)) * m)) >> 56);
#       endif
        } else /* 32 bits */ {
#       if defined(_MSC_VER) && (_MSC_VER >= 1400) && !defined(LZ4_FORCE_SW_BITCOUNT)
            unsigned long r;
            _BitScanForward(&r, (U32)val);
            return (unsigned)r >> 3;
#       elif (defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 3) || \
                            ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))))) && \
                        !defined(__TINYC__) && !defined(LZ4_FORCE_SW_BITCOUNT)
            return (unsigned)__builtin_ctz((U32)val) >> 3;
#       else
            const U32 m = 0x01010101;
            return (unsigned)((((val - 1) ^ val) & (m - 1)) * m) >> 24;
#       endif
        }
    } else   /* Big Endian CPU */ {
        if (sizeof(val)==8) {
#       if (defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 3) || \
                            ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))))) && \
                        !defined(__TINYC__) && !defined(LZ4_FORCE_SW_BITCOUNT)
            return (unsigned)__builtin_clzll((U64)val) >> 3;
#       else
#if 1
            /* this method is probably faster,
             * but adds a 128 bytes lookup table */
            static const unsigned char ctz7_tab[128] = {
                7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            };
            U64 const mask = 0x0101010101010101ULL;
            U64 const t = (((val >> 8) - mask) | val) & mask;
            return ctz7_tab[(t * 0x0080402010080402ULL) >> 57];
#else
            /* this method doesn't consume memory space like the previous one,
             * but it contains several branches,
             * that may end up slowing execution */
            static const U32 by32 = sizeof(val)*4;  /* 32 on 64 bits (goal), 16 on 32 bits.
            Just to avoid some static analyzer complaining about shift by 32 on 32-bits target.
            Note that this code path is never triggered in 32-bits mode. */
            unsigned r;
            if (!(val>>by32)) { r=4; } else { r=0; val>>=by32; }
            if (!(val>>16)) { r+=2; val>>=8; } else { val>>=24; }
            r += (!val);
            return r;
#endif
#       endif
        } else /* 32 bits */ {
#       if (defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 3) || \
                            ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))))) && \
                                        !defined(LZ4_FORCE_SW_BITCOUNT)
            return (unsigned)__builtin_clz((U32)val) >> 3;
#       else
            val >>= 8;
            val = ((((val + 0x00FFFF00) | 0x00FFFFFF) + val) |
              (val + 0x00FF0000)) >> 24;
            return (unsigned)val ^ 3;
#       endif
        }
    }
}

#define STEPSIZE sizeof(reg_t)
LZ4_FORCE_INLINE
unsigned LZ4_count(const BYTE* pIn, const BYTE* pMatch, const BYTE* pInLimit)
{
    const BYTE* const pStart = pIn;

    if (likely(pIn < pInLimit-(STEPSIZE-1))) {
        reg_t const diff = LZ4_read_ARCH(pMatch) ^ LZ4_read_ARCH(pIn);
        if (!diff) {
            pIn+=STEPSIZE; pMatch+=STEPSIZE;
        } else {
            return LZ4_NbCommonBytes(diff);
    }   }

    while (likely(pIn < pInLimit-(STEPSIZE-1))) {
        reg_t const diff = LZ4_read_ARCH(pMatch) ^ LZ4_read_ARCH(pIn);
        if (!diff) { pIn+=STEPSIZE; pMatch+=STEPSIZE; continue; }
        pIn += LZ4_NbCommonBytes(diff);
        return (unsigned)(pIn - pStart);
    }

    if ((STEPSIZE==8) && (pIn<(pInLimit-3)) && (LZ4_read32(pMatch) == LZ4_read32(pIn))) { pIn+=4; pMatch+=4; }
    if ((pIn<(pInLimit-1)) && (LZ4_read16(pMatch) == LZ4_read16(pIn))) { pIn+=2; pMatch+=2; }
    if ((pIn<pInLimit) && (*pMatch == *pIn)) pIn++;
    return (unsigned)(pIn - pStart);
}

#ifndef LZ4_COMMONDEFS_ONLY
/*-************************************
*  Local Constants
**************************************/
static const int LZ4_64Klimit = ((64 KB) + (MFLIMIT-1));
static const U32 LZ4_skipTrigger = 6;  /* Increase this value ==> compression run slower on incompressible data */

/*-************************************
*  Local Structures and types
**************************************/
typedef enum { clearedTable = 0, byPtr, byU32, byU16 } tableType_t;

/**
 * This enum distinguishes several different modes of accessing previous
 * content in the stream.
 *
 * - noDict        : There is no preceding content.
 * - withPrefix64k : Table entries up to ctx->dictSize before the current blob
 *                   blob being compressed are valid and refer to the preceding
 *                   content (of length ctx->dictSize), which is available
 *                   contiguously preceding in memory the content currently
 *                   being compressed.
 * - usingExtDict  : Like withPrefix64k, but the preceding content is somewhere
 *                   else in memory, starting at ctx->dictionary with length
 *                   ctx->dictSize.
 * - usingDictCtx  : Like usingExtDict, but everything concerning the preceding
 *                   content is in a separate context, pointed to by
 *                   ctx->dictCtx. ctx->dictionary, ctx->dictSize, and table
 *                   entries in the current context that refer to positions
 *                   preceding the beginning of the current compression are
 *                   ignored. Instead, ctx->dictCtx->dictionary and ctx->dictCtx
 *                   ->dictSize describe the location and size of the preceding
 *                   content, and matches are found by looking in the ctx
 *                   ->dictCtx->hashTable.
 */
typedef enum { noDict = 0, withPrefix64k, usingExtDict, usingDictCtx } dict_directive;
typedef enum { noDictIssue = 0, dictSmall } dictIssue_directive;

/*-************************************
*  Local Utils
**************************************/
int LZ4_versionNumber (void) { return LZ4_VERSION_NUMBER; }
const char* LZ4_versionString(void) { return LZ4_VERSION_STRING; }
int LZ4_compressBound(int isize)  { return LZ4_COMPRESSBOUND(isize); }
int LZ4_sizeofState(void) { return LZ4_STREAMSIZE; }

/*-************************************
*  Internal Definitions used in Tests
**************************************/

int LZ4_compress_forceExtDict (LZ4_stream_t* LZ4_dict, const char* source, char* dest, int srcSize);

int LZ4_decompress_safe_forceExtDict(const char* source, char* dest,
                                     int compressedSize, int maxOutputSize,
                                     const void* dictStart, size_t dictSize);

/*-******************************
*  Compression functions
********************************/
LZ4_FORCE_INLINE U32 LZ4_hash4(U32 sequence, tableType_t const tableType)
{
    if (tableType == byU16)
        return ((sequence * 2654435761U) >> ((MINMATCH*8)-(LZ4_HASHLOG+1)));
    else
        return ((sequence * 2654435761U) >> ((MINMATCH*8)-LZ4_HASHLOG));
}

LZ4_FORCE_INLINE U32 LZ4_hash5(U64 sequence, tableType_t const tableType)
{
    const U32 hashLog = (tableType == byU16) ? LZ4_HASHLOG+1 : LZ4_HASHLOG;
    if (LZ4_isLittleEndian()) {
        const U64 prime5bytes = 889523592379ULL;
        return (U32)(((sequence << 24) * prime5bytes) >> (64 - hashLog));
    } else {
        const U64 prime8bytes = 11400714785074694791ULL;
        return (U32)(((sequence >> 24) * prime8bytes) >> (64 - hashLog));
    }
}

LZ4_FORCE_INLINE U32 LZ4_hashPosition(const void* const p, tableType_t const tableType)
{
    if ((sizeof(reg_t)==8) && (tableType != byU16)) return LZ4_hash5(LZ4_read_ARCH(p), tableType);
    return LZ4_hash4(LZ4_read32(p), tableType);
}

LZ4_FORCE_INLINE void LZ4_clearHash(U32 h, void* tableBase, tableType_t const tableType)
{
    switch (tableType)
    {
    default: /* fallthrough */
    case clearedTable: { /* illegal! */ assert(0); return; }
    case byPtr: { const BYTE** hashTable = (const BYTE**)tableBase; hashTable[h] = NULL; return; }
    case byU32: { U32* hashTable = (U32*) tableBase; hashTable[h] = 0; return; }
    case byU16: { U16* hashTable = (U16*) tableBase; hashTable[h] = 0; return; }
    }
}

LZ4_FORCE_INLINE void LZ4_putIndexOnHash(U32 idx, U32 h, void* tableBase, tableType_t const tableType)
{
    switch (tableType)
    {
    default: /* fallthrough */
    case clearedTable: /* fallthrough */
    case byPtr: { /* illegal! */ assert(0); return; }
    case byU32: { U32* hashTable = (U32*) tableBase; hashTable[h] = idx; return; }
    case byU16: { U16* hashTable = (U16*) tableBase; assert(idx < 65536); hashTable[h] = (U16)idx; return; }
    }
}

LZ4_FORCE_INLINE void LZ4_putPositionOnHash(const BYTE* p, U32 h,
                                  void* tableBase, tableType_t const tableType,
                            const BYTE* srcBase)
{
    switch (tableType)
    {
    case clearedTable: { /* illegal! */ assert(0); return; }
    case byPtr: { const BYTE** hashTable = (const BYTE**)tableBase; hashTable[h] = p; return; }
    case byU32: { U32* hashTable = (U32*) tableBase; hashTable[h] = (U32)(p-srcBase); return; }
    case byU16: { U16* hashTable = (U16*) tableBase; hashTable[h] = (U16)(p-srcBase); return; }
    }
}

LZ4_FORCE_INLINE void LZ4_putPosition(const BYTE* p, void* tableBase, tableType_t tableType, const BYTE* srcBase)
{
    U32 const h = LZ4_hashPosition(p, tableType);
    LZ4_putPositionOnHash(p, h, tableBase, tableType, srcBase);
}

/* LZ4_getIndexOnHash() :
 * Index of match position registered in hash table.
 * hash position must be calculated by using base+index, or dictBase+index.
 * Assumption 1 : only valid if tableType == byU32 or byU16.
 * Assumption 2 : h is presumed valid (within limits of hash table)
 */
LZ4_FORCE_INLINE U32 LZ4_getIndexOnHash(U32 h, const void* tableBase, tableType_t tableType)
{
    LZ4_STATIC_ASSERT(LZ4_MEMORY_USAGE > 2);
    if (tableType == byU32) {
        const U32* const hashTable = (const U32*) tableBase;
        assert(h < (1U << (LZ4_MEMORY_USAGE-2)));
        return hashTable[h];
    }
    if (tableType == byU16) {
        const U16* const hashTable = (const U16*) tableBase;
        assert(h < (1U << (LZ4_MEMORY_USAGE-1)));
        return hashTable[h];
    }
    assert(0); return 0;  /* forbidden case */
}

static const BYTE* LZ4_getPositionOnHash(U32 h, const void* tableBase, tableType_t tableType, const BYTE* srcBase)
{
    if (tableType == byPtr) { const BYTE* const* hashTable = (const BYTE* const*) tableBase; return hashTable[h]; }
    if (tableType == byU32) { const U32* const hashTable = (const U32*) tableBase; return hashTable[h] + srcBase; }
    { const U16* const hashTable = (const U16*) tableBase; return hashTable[h] + srcBase; }   /* default, to ensure a return */
}

LZ4_FORCE_INLINE const BYTE*
LZ4_getPosition(const BYTE* p,
                const void* tableBase, tableType_t tableType,
                const BYTE* srcBase)
{
    U32 const h = LZ4_hashPosition(p, tableType);
    return LZ4_getPositionOnHash(h, tableBase, tableType, srcBase);
}

LZ4_FORCE_INLINE void
LZ4_prepareTable(LZ4_stream_t_internal* const cctx,
           const int inputSize,
           const tableType_t tableType) {
    /* If the table hasn't been used, it's guaranteed to be zeroed out, and is
     * therefore safe to use no matter what mode we're in. Otherwise, we figure
     * out if it's safe to leave as is or whether it needs to be reset.
     */
    if ((tableType_t)cctx->tableType != clearedTable) {
        assert(inputSize >= 0);
        if ((tableType_t)cctx->tableType != tableType
          || ((tableType == byU16) && cctx->currentOffset + (unsigned)inputSize >= 0xFFFFU)
          || ((tableType == byU32) && cctx->currentOffset > 1 GB)
          || tableType == byPtr
          || inputSize >= 4 KB)
        {
            DEBUGLOG(4, "LZ4_prepareTable: Resetting table in %p", cctx);
            MEM_INIT(cctx->hashTable, 0, LZ4_HASHTABLESIZE);
            cctx->currentOffset = 0;
            cctx->tableType = (U32)clearedTable;
        } else {
            DEBUGLOG(4, "LZ4_prepareTable: Re-use hash table (no reset)");
        }
    }

    /* Adding a gap, so all previous entries are > LZ4_DISTANCE_MAX back, is faster
     * than compressing without a gap. However, compressing with
     * currentOffset == 0 is faster still, so we preserve that case.
     */
    if (cctx->currentOffset != 0 && tableType == byU32) {
        DEBUGLOG(5, "LZ4_prepareTable: adding 64KB to currentOffset");
        cctx->currentOffset += 64 KB;
    }

    /* Finally, clear history */
    cctx->dictCtx = NULL;
    cctx->dictionary = NULL;
    cctx->dictSize = 0;
}

/** LZ4_compress_generic() :
 *  inlined, to ensure branches are decided at compilation time.
 *  Presumed already validated at this stage:
 *  - source != NULL
 *  - inputSize > 0
 */
LZ4_FORCE_INLINE int LZ4_compress_generic_validated(
                 LZ4_stream_t_internal* const cctx,
                 const char* const source,
                 char* const dest,
                 const int inputSize,
                 int *inputConsumed, /* only written when outputDirective == fillOutput */
                 const int maxOutputSize,
                 const limitedOutput_directive outputDirective,
                 const tableType_t tableType,
                 const dict_directive dictDirective,
                 const dictIssue_directive dictIssue,
                 const int acceleration)
{
    int result;
    const BYTE* ip = (const BYTE*) source;

    U32 const startIndex = cctx->currentOffset;
    const BYTE* base = (const BYTE*) source - startIndex;
    const BYTE* lowLimit;

    const LZ4_stream_t_internal* dictCtx = (const LZ4_stream_t_internal*) cctx->dictCtx;
    const BYTE* const dictionary =
        dictDirective == usingDictCtx ? dictCtx->dictionary : cctx->dictionary;
    const U32 dictSize =
        dictDirective == usingDictCtx ? dictCtx->dictSize : cctx->dictSize;
    const U32 dictDelta = (dictDirective == usingDictCtx) ? startIndex - dictCtx->currentOffset : 0;   /* make indexes in dictCtx comparable with index in current context */

    int const maybe_extMem = (dictDirective == usingExtDict) || (dictDirective == usingDictCtx);
    U32 const prefixIdxLimit = startIndex - dictSize;   /* used when dictDirective == dictSmall */
    const BYTE* const dictEnd = dictionary ? dictionary + dictSize : dictionary;
    const BYTE* anchor = (const BYTE*) source;
    const BYTE* const iend = ip + inputSize;
    const BYTE* const mflimitPlusOne = iend - MFLIMIT + 1;
    const BYTE* const matchlimit = iend - LASTLITERALS;

    /* the dictCtx currentOffset is indexed on the start of the dictionary,
     * while a dictionary in the current context precedes the currentOffset */
    const BYTE* dictBase = !dictionary ? NULL : (dictDirective == usingDictCtx) ?
                            dictionary + dictSize - dictCtx->currentOffset :
                            dictionary + dictSize - startIndex;

    BYTE* op = (BYTE*) dest;
    BYTE* const olimit = op + maxOutputSize;

    U32 offset = 0;
    U32 forwardH;

    DEBUGLOG(5, "LZ4_compress_generic_validated: srcSize=%i, tableType=%u", inputSize, tableType);
    assert(ip != NULL);
    /* If init conditions are not met, we don't have to mark stream
     * as having dirty context, since no action was taken yet */
    if (outputDirective == fillOutput && maxOutputSize < 1) { return 0; } /* Impossible to store anything */
    if ((tableType == byU16) && (inputSize>=LZ4_64Klimit)) { return 0; }  /* Size too large (not within 64K limit) */
    if (tableType==byPtr) assert(dictDirective==noDict);      /* only supported use case with byPtr */
    assert(acceleration >= 1);

    lowLimit = (const BYTE*)source - (dictDirective == withPrefix64k ? dictSize : 0);

    /* Update context state */
    if (dictDirective == usingDictCtx) {
        /* Subsequent linked blocks can't use the dictionary. */
        /* Instead, they use the block we just compressed. */
        cctx->dictCtx = NULL;
        cctx->dictSize = (U32)inputSize;
    } else {
        cctx->dictSize += (U32)inputSize;
    }
    cctx->currentOffset += (U32)inputSize;
    cctx->tableType = (U32)tableType;

    if (inputSize<LZ4_minLength) goto _last_literals;        /* Input too small, no compression (all literals) */

    /* First Byte */
    LZ4_putPosition(ip, cctx->hashTable, tableType, base);
    ip++; forwardH = LZ4_hashPosition(ip, tableType);

    /* Main Loop */
    for ( ; ; ) {
        const BYTE* match;
        BYTE* token;
        const BYTE* filledIp;

        /* Find a match */
        if (tableType == byPtr) {
            const BYTE* forwardIp = ip;
            int step = 1;
            int searchMatchNb = acceleration << LZ4_skipTrigger;
            do {
                U32 const h = forwardH;
                ip = forwardIp;
                forwardIp += step;
                step = (searchMatchNb++ >> LZ4_skipTrigger);

                if (unlikely(forwardIp > mflimitPlusOne)) goto _last_literals;
                assert(ip < mflimitPlusOne);

                match = LZ4_getPositionOnHash(h, cctx->hashTable, tableType, base);
                forwardH = LZ4_hashPosition(forwardIp, tableType);
                LZ4_putPositionOnHash(ip, h, cctx->hashTable, tableType, base);

            } while ( (match+LZ4_DISTANCE_MAX < ip)
                   || (LZ4_read32(match) != LZ4_read32(ip)) );

        } else {   /* byU32, byU16 */

            const BYTE* forwardIp = ip;
            int step = 1;
            int searchMatchNb = acceleration << LZ4_skipTrigger;
            do {
                U32 const h = forwardH;
                U32 const current = (U32)(forwardIp - base);
                U32 matchIndex = LZ4_getIndexOnHash(h, cctx->hashTable, tableType);
                assert(matchIndex <= current);
                assert(forwardIp - base < (ptrdiff_t)(2 GB - 1));
                ip = forwardIp;
                forwardIp += step;
                step = (searchMatchNb++ >> LZ4_skipTrigger);

                if (unlikely(forwardIp > mflimitPlusOne)) goto _last_literals;
                assert(ip < mflimitPlusOne);

                if (dictDirective == usingDictCtx) {
                    if (matchIndex < startIndex) {
                        /* there was no match, try the dictionary */
                        assert(tableType == byU32);
                        matchIndex = LZ4_getIndexOnHash(h, dictCtx->hashTable, byU32);
                        match = dictBase + matchIndex;
                        matchIndex += dictDelta;   /* make dictCtx index comparable with current context */
                        lowLimit = dictionary;
                    } else {
                        match = base + matchIndex;
                        lowLimit = (const BYTE*)source;
                    }
                } else if (dictDirective==usingExtDict) {
                    if (matchIndex < startIndex) {
                        DEBUGLOG(7, "extDict candidate: matchIndex=%5u  <  startIndex=%5u", matchIndex, startIndex);
                        assert(startIndex - matchIndex >= MINMATCH);
                        match = dictBase + matchIndex;
                        lowLimit = dictionary;
                    } else {
                        match = base + matchIndex;
                        lowLimit = (const BYTE*)source;
                    }
                } else {   /* single continuous memory segment */
                    match = base + matchIndex;
                }
                forwardH = LZ4_hashPosition(forwardIp, tableType);
                LZ4_putIndexOnHash(current, h, cctx->hashTable, tableType);

                DEBUGLOG(7, "candidate at pos=%u  (offset=%u \n", matchIndex, current - matchIndex);
                if ((dictIssue == dictSmall) && (matchIndex < prefixIdxLimit)) { continue; }    /* match outside of valid area */
                assert(matchIndex < current);
                if ( ((tableType != byU16) || (LZ4_DISTANCE_MAX < LZ4_DISTANCE_ABSOLUTE_MAX))
                  && (matchIndex+LZ4_DISTANCE_MAX < current)) {
                    continue;
                } /* too far */
                assert((current - matchIndex) <= LZ4_DISTANCE_MAX);  /* match now expected within distance */

                if (LZ4_read32(match) == LZ4_read32(ip)) {
                    if (maybe_extMem) offset = current - matchIndex;
                    break;   /* match found */
                }

            } while(1);
        }

        /* Catch up */
        filledIp = ip;
        while (((ip>anchor) & (match > lowLimit)) && (unlikely(ip[-1]==match[-1]))) { ip--; match--; }

        /* Encode Literals */
        {   unsigned const litLength = (unsigned)(ip - anchor);
            token = op++;
            if ((outputDirective == limitedOutput) &&  /* Check output buffer overflow */
                (unlikely(op + litLength + (2 + 1 + LASTLITERALS) + (litLength/255) > olimit)) ) {
                return 0;   /* cannot compress within `dst` budget. Stored indexes in hash table are nonetheless fine */
            }
            if ((outputDirective == fillOutput) &&
                (unlikely(op + (litLength+240)/255 /* litlen */ + litLength /* literals */ + 2 /* offset */ + 1 /* token */ + MFLIMIT - MINMATCH /* min last literals so last match is <= end - MFLIMIT */ > olimit))) {
                op--;
                goto _last_literals;
            }
            if (litLength >= RUN_MASK) {
                int len = (int)(litLength - RUN_MASK);
                *token = (RUN_MASK<<ML_BITS);
                for(; len >= 255 ; len-=255) *op++ = 255;
                *op++ = (BYTE)len;
            }
            else *token = (BYTE)(litLength<<ML_BITS);

            /* Copy Literals */
            LZ4_wildCopy8(op, anchor, op+litLength);
            op+=litLength;
            DEBUGLOG(6, "seq.start:%i, literals=%u, match.start:%i",
                        (int)(anchor-(const BYTE*)source), litLength, (int)(ip-(const BYTE*)source));
        }

_next_match:
        /* at this stage, the following variables must be correctly set :
         * - ip : at start of LZ operation
         * - match : at start of previous pattern occurence; can be within current prefix, or within extDict
         * - offset : if maybe_ext_memSegment==1 (constant)
         * - lowLimit : must be == dictionary to mean "match is within extDict"; must be == source otherwise
         * - token and *token : position to write 4-bits for match length; higher 4-bits for literal length supposed already written
         */

        if ((outputDirective == fillOutput) &&
            (op + 2 /* offset */ + 1 /* token */ + MFLIMIT - MINMATCH /* min last literals so last match is <= end - MFLIMIT */ > olimit)) {
            /* the match was too close to the end, rewind and go to last literals */
            op = token;
            goto _last_literals;
        }

        /* Encode Offset */
        if (maybe_extMem) {   /* static test */
            DEBUGLOG(6, "             with offset=%u  (ext if > %i)", offset, (int)(ip - (const BYTE*)source));
            assert(offset <= LZ4_DISTANCE_MAX && offset > 0);
            LZ4_writeLE16(op, (U16)offset); op+=2;
        } else  {
            DEBUGLOG(6, "             with offset=%u  (same segment)", (U32)(ip - match));
            assert(ip-match <= LZ4_DISTANCE_MAX);
            LZ4_writeLE16(op, (U16)(ip - match)); op+=2;
        }

        /* Encode MatchLength */
        {   unsigned matchCode;

            if ( (dictDirective==usingExtDict || dictDirective==usingDictCtx)
              && (lowLimit==dictionary) /* match within extDict */ ) {
                const BYTE* limit = ip + (dictEnd-match);
                assert(dictEnd > match);
                if (limit > matchlimit) limit = matchlimit;
                matchCode = LZ4_count(ip+MINMATCH, match+MINMATCH, limit);
                ip += (size_t)matchCode + MINMATCH;
                if (ip==limit) {
                    unsigned const more = LZ4_count(limit, (const BYTE*)source, matchlimit);
                    matchCode += more;
                    ip += more;
                }
                DEBUGLOG(6, "             with matchLength=%u starting in extDict", matchCode+MINMATCH);
            } else {
                matchCode = LZ4_count(ip+MINMATCH, match+MINMATCH, matchlimit);
                ip += (size_t)matchCode + MINMATCH;
                DEBUGLOG(6, "             with matchLength=%u", matchCode+MINMATCH);
            }

            if ((outputDirective) &&    /* Check output buffer overflow */
                (unlikely(op + (1 + LASTLITERALS) + (matchCode+240)/255 > olimit)) ) {
                if (outputDirective == fillOutput) {
                    /* Match description too long : reduce it */
                    U32 newMatchCode = 15 /* in token */ - 1 /* to avoid needing a zero byte */ + ((U32)(olimit - op) - 1 - LASTLITERALS) * 255;
                    ip -= matchCode - newMatchCode;
                    assert(newMatchCode < matchCode);
                    matchCode = newMatchCode;
                    if (unlikely(ip <= filledIp)) {
                        /* We have already filled up to filledIp so if ip ends up less than filledIp
                         * we have positions in the hash table beyond the current position. This is
                         * a problem if we reuse the hash table. So we have to remove these positions
                         * from the hash table.
                         */
                        const BYTE* ptr;
                        DEBUGLOG(5, "Clearing %u positions", (U32)(filledIp - ip));
                        for (ptr = ip; ptr <= filledIp; ++ptr) {
                            U32 const h = LZ4_hashPosition(ptr, tableType);
                            LZ4_clearHash(h, cctx->hashTable, tableType);
                        }
                    }
                } else {
                    assert(outputDirective == limitedOutput);
                    return 0;   /* cannot compress within `dst` budget. Stored indexes in hash table are nonetheless fine */
                }
            }
            if (matchCode >= ML_MASK) {
                *token += ML_MASK;
                matchCode -= ML_MASK;
                LZ4_write32(op, 0xFFFFFFFF);
                while (matchCode >= 4*255) {
                    op+=4;
                    LZ4_write32(op, 0xFFFFFFFF);
                    matchCode -= 4*255;
                }
                op += matchCode / 255;
                *op++ = (BYTE)(matchCode % 255);
            } else
                *token += (BYTE)(matchCode);
        }
        /* Ensure we have enough space for the last literals. */
        assert(!(outputDirective == fillOutput && op + 1 + LASTLITERALS > olimit));

        anchor = ip;

        /* Test end of chunk */
        if (ip >= mflimitPlusOne) break;

        /* Fill table */
        LZ4_putPosition(ip-2, cctx->hashTable, tableType, base);

        /* Test next position */
        if (tableType == byPtr) {

            match = LZ4_getPosition(ip, cctx->hashTable, tableType, base);
            LZ4_putPosition(ip, cctx->hashTable, tableType, base);
            if ( (match+LZ4_DISTANCE_MAX >= ip)
              && (LZ4_read32(match) == LZ4_read32(ip)) )
            { token=op++; *token=0; goto _next_match; }

        } else {   /* byU32, byU16 */

            U32 const h = LZ4_hashPosition(ip, tableType);
            U32 const current = (U32)(ip-base);
            U32 matchIndex = LZ4_getIndexOnHash(h, cctx->hashTable, tableType);
            assert(matchIndex < current);
            if (dictDirective == usingDictCtx) {
                if (matchIndex < startIndex) {
                    /* there was no match, try the dictionary */
                    matchIndex = LZ4_getIndexOnHash(h, dictCtx->hashTable, byU32);
                    match = dictBase + matchIndex;
                    lowLimit = dictionary;   /* required for match length counter */
                    matchIndex += dictDelta;
                } else {
                    match = base + matchIndex;
                    lowLimit = (const BYTE*)source;  /* required for match length counter */
                }
            } else if (dictDirective==usingExtDict) {
                if (matchIndex < startIndex) {
                    match = dictBase + matchIndex;
                    lowLimit = dictionary;   /* required for match length counter */
                } else {
                    match = base + matchIndex;
                    lowLimit = (const BYTE*)source;   /* required for match length counter */
                }
            } else {   /* single memory segment */
                match = base + matchIndex;
            }
            LZ4_putIndexOnHash(current, h, cctx->hashTable, tableType);
            assert(matchIndex < current);
            if ( ((dictIssue==dictSmall) ? (matchIndex >= prefixIdxLimit) : 1)
              && (((tableType==byU16) && (LZ4_DISTANCE_MAX == LZ4_DISTANCE_ABSOLUTE_MAX)) ? 1 : (matchIndex+LZ4_DISTANCE_MAX >= current))
              && (LZ4_read32(match) == LZ4_read32(ip)) ) {
                token=op++;
                *token=0;
                if (maybe_extMem) offset = current - matchIndex;
                DEBUGLOG(6, "seq.start:%i, literals=%u, match.start:%i",
                            (int)(anchor-(const BYTE*)source), 0, (int)(ip-(const BYTE*)source));
                goto _next_match;
            }
        }

        /* Prepare next loop */
        forwardH = LZ4_hashPosition(++ip, tableType);

    }

_last_literals:
    /* Encode Last Literals */
    {   size_t lastRun = (size_t)(iend - anchor);
        if ( (outputDirective) &&  /* Check output buffer overflow */
            (op + lastRun + 1 + ((lastRun+255-RUN_MASK)/255) > olimit)) {
            if (outputDirective == fillOutput) {
                /* adapt lastRun to fill 'dst' */
                assert(olimit >= op);
                lastRun  = (size_t)(olimit-op) - 1/*token*/;
                lastRun -= (lastRun + 256 - RUN_MASK) / 256;  /*additional length tokens*/
            } else {
                assert(outputDirective == limitedOutput);
                return 0;   /* cannot compress within `dst` budget. Stored indexes in hash table are nonetheless fine */
            }
        }
        DEBUGLOG(6, "Final literal run : %i literals", (int)lastRun);
        if (lastRun >= RUN_MASK) {
            size_t accumulator = lastRun - RUN_MASK;
            *op++ = RUN_MASK << ML_BITS;
            for(; accumulator >= 255 ; accumulator-=255) *op++ = 255;
            *op++ = (BYTE) accumulator;
        } else {
            *op++ = (BYTE)(lastRun<<ML_BITS);
        }
        LZ4_memcpy(op, anchor, lastRun);
        ip = anchor + lastRun;
        op += lastRun;
    }

    if (outputDirective == fillOutput) {
        *inputConsumed = (int) (((const char*)ip)-source);
    }
    result = (int)(((char*)op) - dest);
    assert(result > 0);
    DEBUGLOG(5, "LZ4_compress_generic: compressed %i bytes into %i bytes", inputSize, result);
    return result;
}

/** LZ4_compress_generic() :
 *  inlined, to ensure branches are decided at compilation time;
 *  takes care of src == (NULL, 0)
 *  and forward the rest to LZ4_compress_generic_validated */
LZ4_FORCE_INLINE int LZ4_compress_generic(
                 LZ4_stream_t_internal* const cctx,
                 const char* const src,
                 char* const dst,
                 const int srcSize,
                 int *inputConsumed, /* only written when outputDirective == fillOutput */
                 const int dstCapacity,
                 const limitedOutput_directive outputDirective,
                 const tableType_t tableType,
                 const dict_directive dictDirective,
                 const dictIssue_directive dictIssue,
                 const int acceleration)
{
    DEBUGLOG(5, "LZ4_compress_generic: srcSize=%i, dstCapacity=%i",
                srcSize, dstCapacity);

    if ((U32)srcSize > (U32)LZ4_MAX_INPUT_SIZE) { return 0; }  /* Unsupported srcSize, too large (or negative) */
    if (srcSize == 0) {   /* src == NULL supported if srcSize == 0 */
        if (outputDirective != notLimited && dstCapacity <= 0) return 0;  /* no output, can't write anything */
        DEBUGLOG(5, "Generating an empty block");
        assert(outputDirective == notLimited || dstCapacity >= 1);
        assert(dst != NULL);
        dst[0] = 0;
        if (outputDirective == fillOutput) {
            assert (inputConsumed != NULL);
            *inputConsumed = 0;
        }
        return 1;
    }
    assert(src != NULL);

    return LZ4_compress_generic_validated(cctx, src, dst, srcSize,
                inputConsumed, /* only written into if outputDirective == fillOutput */
                dstCapacity, outputDirective,
                tableType, dictDirective, dictIssue, acceleration);
}

int LZ4_compress_fast_extState(void* state, const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration)
{
    LZ4_stream_t_internal* const ctx = & LZ4_initStream(state, sizeof(LZ4_stream_t)) -> internal_donotuse;
    assert(ctx != NULL);
    if (acceleration < 1) acceleration = LZ4_ACCELERATION_DEFAULT;
    if (acceleration > LZ4_ACCELERATION_MAX) acceleration = LZ4_ACCELERATION_MAX;
    if (maxOutputSize >= LZ4_compressBound(inputSize)) {
        if (inputSize < LZ4_64Klimit) {
            return LZ4_compress_generic(ctx, source, dest, inputSize, NULL, 0, notLimited, byU16, noDict, noDictIssue, acceleration);
        } else {
            const tableType_t tableType = ((sizeof(void*)==4) && ((uptrval)source > LZ4_DISTANCE_MAX)) ? byPtr : byU32;
            return LZ4_compress_generic(ctx, source, dest, inputSize, NULL, 0, notLimited, tableType, noDict, noDictIssue, acceleration);
        }
    } else {
        if (inputSize < LZ4_64Klimit) {
            return LZ4_compress_generic(ctx, source, dest, inputSize, NULL, maxOutputSize, limitedOutput, byU16, noDict, noDictIssue, acceleration);
        } else {
            const tableType_t tableType = ((sizeof(void*)==4) && ((uptrval)source > LZ4_DISTANCE_MAX)) ? byPtr : byU32;
            return LZ4_compress_generic(ctx, source, dest, inputSize, NULL, maxOutputSize, limitedOutput, tableType, noDict, noDictIssue, acceleration);
        }
    }
}

/**
 * LZ4_compress_fast_extState_fastReset() :
 * A variant of LZ4_compress_fast_extState().
 *
 * Using this variant avoids an expensive initialization step. It is only safe
 * to call if the state buffer is known to be correctly initialized already
 * (see comment in lz4.h on LZ4_resetStream_fast() for a definition of
 * "correctly initialized").
 */
int LZ4_compress_fast_extState_fastReset(void* state, const char* src, char* dst, int srcSize, int dstCapacity, int acceleration)
{
    LZ4_stream_t_internal* ctx = &((LZ4_stream_t*)state)->internal_donotuse;
    if (acceleration < 1) acceleration = LZ4_ACCELERATION_DEFAULT;
    if (acceleration > LZ4_ACCELERATION_MAX) acceleration = LZ4_ACCELERATION_MAX;

    if (dstCapacity >= LZ4_compressBound(srcSize)) {
        if (srcSize < LZ4_64Klimit) {
            const tableType_t tableType = byU16;
            LZ4_prepareTable(ctx, srcSize, tableType);
            if (ctx->currentOffset) {
                return LZ4_compress_generic(ctx, src, dst, srcSize, NULL, 0, notLimited, tableType, noDict, dictSmall, acceleration);
            } else {
                return LZ4_compress_generic(ctx, src, dst, srcSize, NULL, 0, notLimited, tableType, noDict, noDictIssue, acceleration);
            }
        } else {
            const tableType_t tableType = ((sizeof(void*)==4) && ((uptrval)src > LZ4_DISTANCE_MAX)) ? byPtr : byU32;
            LZ4_prepareTable(ctx, srcSize, tableType);
            return LZ4_compress_generic(ctx, src, dst, srcSize, NULL, 0, notLimited, tableType, noDict, noDictIssue, acceleration);
        }
    } else {
        if (srcSize < LZ4_64Klimit) {
            const tableType_t tableType = byU16;
            LZ4_prepareTable(ctx, srcSize, tableType);
            if (ctx->currentOffset) {
                return LZ4_compress_generic(ctx, src, dst, srcSize, NULL, dstCapacity, limitedOutput, tableType, noDict, dictSmall, acceleration);
            } else {
                return LZ4_compress_generic(ctx, src, dst, srcSize, NULL, dstCapacity, limitedOutput, tableType, noDict, noDictIssue, acceleration);
            }
        } else {
            const tableType_t tableType = ((sizeof(void*)==4) && ((uptrval)src > LZ4_DISTANCE_MAX)) ? byPtr : byU32;
            LZ4_prepareTable(ctx, srcSize, tableType);
            return LZ4_compress_generic(ctx, src, dst, srcSize, NULL, dstCapacity, limitedOutput, tableType, noDict, noDictIssue, acceleration);
        }
    }
}

int LZ4_compress_fast(const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration)
{
    int result;
#if (LZ4_HEAPMODE)
    LZ4_stream_t* ctxPtr = ALLOC(sizeof(LZ4_stream_t));   /* malloc-calloc always properly aligned */
    if (ctxPtr == NULL) return 0;
#else
    LZ4_stream_t ctx;
    LZ4_stream_t* const ctxPtr = &ctx;
#endif
    result = LZ4_compress_fast_extState(ctxPtr, source, dest, inputSize, maxOutputSize, acceleration);

#if (LZ4_HEAPMODE)
    FREEMEM(ctxPtr);
#endif
    return result;
}

int LZ4_compress_default(const char* src, char* dst, int srcSize, int maxOutputSize)
{
    return LZ4_compress_fast(src, dst, srcSize, maxOutputSize, 1);
}

/* Note!: This function leaves the stream in an unclean/broken state!
 * It is not safe to subsequently use the same state with a _fastReset() or
 * _continue() call without resetting it. */
static int LZ4_compress_destSize_extState (LZ4_stream_t* state, const char* src, char* dst, int* srcSizePtr, int targetDstSize)
{
    void* const s = LZ4_initStream(state, sizeof (*state));
    assert(s != NULL); (void)s;

    if (targetDstSize >= LZ4_compressBound(*srcSizePtr)) {  /* compression success is guaranteed */
        return LZ4_compress_fast_extState(state, src, dst, *srcSizePtr, targetDstSize, 1);
    } else {
        if (*srcSizePtr < LZ4_64Klimit) {
            return LZ4_compress_generic(&state->internal_donotuse, src, dst, *srcSizePtr, srcSizePtr, targetDstSize, fillOutput, byU16, noDict, noDictIssue, 1);
        } else {
            tableType_t const addrMode = ((sizeof(void*)==4) && ((uptrval)src > LZ4_DISTANCE_MAX)) ? byPtr : byU32;
            return LZ4_compress_generic(&state->internal_donotuse, src, dst, *srcSizePtr, srcSizePtr, targetDstSize, fillOutput, addrMode, noDict, noDictIssue, 1);
    }   }
}

int LZ4_compress_destSize(const char* src, char* dst, int* srcSizePtr, int targetDstSize)
{
#if (LZ4_HEAPMODE)
    LZ4_stream_t* ctx = (LZ4_stream_t*)ALLOC(sizeof(LZ4_stream_t));   /* malloc-calloc always properly aligned */
    if (ctx == NULL) return 0;
#else
    LZ4_stream_t ctxBody;
    LZ4_stream_t* ctx = &ctxBody;
#endif

    int result = LZ4_compress_destSize_extState(ctx, src, dst, srcSizePtr, targetDstSize);

#if (LZ4_HEAPMODE)
    FREEMEM(ctx);
#endif
    return result;
}

/*-******************************
*  Streaming functions
********************************/

LZ4_stream_t* LZ4_createStream(void)
{
    LZ4_stream_t* const lz4s = (LZ4_stream_t*)ALLOC(sizeof(LZ4_stream_t));
    LZ4_STATIC_ASSERT(LZ4_STREAMSIZE >= sizeof(LZ4_stream_t_internal));    /* A compilation error here means LZ4_STREAMSIZE is not large enough */
    DEBUGLOG(4, "LZ4_createStream %p", lz4s);
    if (lz4s == NULL) return NULL;
    LZ4_initStream(lz4s, sizeof(*lz4s));
    return lz4s;
}

static size_t LZ4_stream_t_alignment(void)
{
#if LZ4_ALIGN_TEST
    typedef struct { char c; LZ4_stream_t t; } t_a;
    return sizeof(t_a) - sizeof(LZ4_stream_t);
#else
    return 1;  /* effectively disabled */
#endif
}

LZ4_stream_t* LZ4_initStream (void* buffer, size_t size)
{
    DEBUGLOG(5, "LZ4_initStream");
    if (buffer == NULL) { return NULL; }
    if (size < sizeof(LZ4_stream_t)) { return NULL; }
    if (!LZ4_isAligned(buffer, LZ4_stream_t_alignment())) return NULL;
    MEM_INIT(buffer, 0, sizeof(LZ4_stream_t_internal));
    return (LZ4_stream_t*)buffer;
}

/* resetStream is now deprecated,
 * prefer initStream() which is more general */
void LZ4_resetStream (LZ4_stream_t* LZ4_stream)
{
    DEBUGLOG(5, "LZ4_resetStream (ctx:%p)", LZ4_stream);
    MEM_INIT(LZ4_stream, 0, sizeof(LZ4_stream_t_internal));
}

void LZ4_resetStream_fast(LZ4_stream_t* ctx) {
    LZ4_prepareTable(&(ctx->internal_donotuse), 0, byU32);
}

int LZ4_freeStream (LZ4_stream_t* LZ4_stream)
{
    if (!LZ4_stream) return 0;   /* support free on NULL */
    DEBUGLOG(5, "LZ4_freeStream %p", LZ4_stream);
    FREEMEM(LZ4_stream);
    return (0);
}

#define HASH_UNIT sizeof(reg_t)
int LZ4_loadDict (LZ4_stream_t* LZ4_dict, const char* dictionary, int dictSize)
{
    LZ4_stream_t_internal* dict = &LZ4_dict->internal_donotuse;
    const tableType_t tableType = byU32;
    const BYTE* p = (const BYTE*)dictionary;
    const BYTE* const dictEnd = p + dictSize;
    const BYTE* base;

    DEBUGLOG(4, "LZ4_loadDict (%i bytes from %p into %p)", dictSize, dictionary, LZ4_dict);

    /* It's necessary to reset the context,
     * and not just continue it with prepareTable()
     * to avoid any risk of generating overflowing matchIndex
     * when compressing using this dictionary */
    LZ4_resetStream(LZ4_dict);

    /* We always increment the offset by 64 KB, since, if the dict is longer,
     * we truncate it to the last 64k, and if it's shorter, we still want to
     * advance by a whole window length so we can provide the guarantee that
     * there are only valid offsets in the window, which allows an optimization
     * in LZ4_compress_fast_continue() where it uses noDictIssue even when the
     * dictionary isn't a full 64k. */
    dict->currentOffset += 64 KB;

    if (dictSize < (int)HASH_UNIT) {
        return 0;
    }

    if ((dictEnd - p) > 64 KB) p = dictEnd - 64 KB;
    base = dictEnd - dict->currentOffset;
    dict->dictionary = p;
    dict->dictSize = (U32)(dictEnd - p);
    dict->tableType = (U32)tableType;

    while (p <= dictEnd-HASH_UNIT) {
        LZ4_putPosition(p, dict->hashTable, tableType, base);
        p+=3;
    }

    return (int)dict->dictSize;
}

void LZ4_attach_dictionary(LZ4_stream_t* workingStream, const LZ4_stream_t* dictionaryStream) {
    const LZ4_stream_t_internal* dictCtx = dictionaryStream == NULL ? NULL :
        &(dictionaryStream->internal_donotuse);

    DEBUGLOG(4, "LZ4_attach_dictionary (%p, %p, size %u)",
             workingStream, dictionaryStream,
             dictCtx != NULL ? dictCtx->dictSize : 0);

    if (dictCtx != NULL) {
        /* If the current offset is zero, we will never look in the
         * external dictionary context, since there is no value a table
         * entry can take that indicate a miss. In that case, we need
         * to bump the offset to something non-zero.
         */
        if (workingStream->internal_donotuse.currentOffset == 0) {
            workingStream->internal_donotuse.currentOffset = 64 KB;
        }

        /* Don't actually attach an empty dictionary.
         */
        if (dictCtx->dictSize == 0) {
            dictCtx = NULL;
        }
    }
    workingStream->internal_donotuse.dictCtx = dictCtx;
}

static void LZ4_renormDictT(LZ4_stream_t_internal* LZ4_dict, int nextSize)
{
    assert(nextSize >= 0);
    if (LZ4_dict->currentOffset + (unsigned)nextSize > 0x80000000) {   /* potential ptrdiff_t overflow (32-bits mode) */
        /* rescale hash table */
        U32 const delta = LZ4_dict->currentOffset - 64 KB;
        const BYTE* dictEnd = LZ4_dict->dictionary + LZ4_dict->dictSize;
        int i;
        DEBUGLOG(4, "LZ4_renormDictT");
        for (i=0; i<LZ4_HASH_SIZE_U32; i++) {
            if (LZ4_dict->hashTable[i] < delta) LZ4_dict->hashTable[i]=0;
            else LZ4_dict->hashTable[i] -= delta;
        }
        LZ4_dict->currentOffset = 64 KB;
        if (LZ4_dict->dictSize > 64 KB) LZ4_dict->dictSize = 64 KB;
        LZ4_dict->dictionary = dictEnd - LZ4_dict->dictSize;
    }
}

int LZ4_compress_fast_continue (LZ4_stream_t* LZ4_stream,
                                const char* source, char* dest,
                                int inputSize, int maxOutputSize,
                                int acceleration)
{
    const tableType_t tableType = byU32;
    LZ4_stream_t_internal* streamPtr = &LZ4_stream->internal_donotuse;
    const BYTE* dictEnd = streamPtr->dictionary + streamPtr->dictSize;

    DEBUGLOG(5, "LZ4_compress_fast_continue (inputSize=%i)", inputSize);

    LZ4_renormDictT(streamPtr, inputSize);   /* avoid index overflow */
    if (acceleration < 1) acceleration = LZ4_ACCELERATION_DEFAULT;
    if (acceleration > LZ4_ACCELERATION_MAX) acceleration = LZ4_ACCELERATION_MAX;

    /* invalidate tiny dictionaries */
    if ( (streamPtr->dictSize-1 < 4-1)   /* intentional underflow */
      && (dictEnd != (const BYTE*)source) ) {
        DEBUGLOG(5, "LZ4_compress_fast_continue: dictSize(%u) at addr:%p is too small", streamPtr->dictSize, streamPtr->dictionary);
        streamPtr->dictSize = 0;
        streamPtr->dictionary = (const BYTE*)source;
        dictEnd = (const BYTE*)source;
    }

    /* Check overlapping input/dictionary space */
    {   const BYTE* sourceEnd = (const BYTE*) source + inputSize;
        if ((sourceEnd > streamPtr->dictionary) && (sourceEnd < dictEnd)) {
            streamPtr->dictSize = (U32)(dictEnd - sourceEnd);
            if (streamPtr->dictSize > 64 KB) streamPtr->dictSize = 64 KB;
            if (streamPtr->dictSize < 4) streamPtr->dictSize = 0;
            streamPtr->dictionary = dictEnd - streamPtr->dictSize;
        }
    }

    /* prefix mode : source data follows dictionary */
    if (dictEnd == (const BYTE*)source) {
        if ((streamPtr->dictSize < 64 KB) && (streamPtr->dictSize < streamPtr->currentOffset))
            return LZ4_compress_generic(streamPtr, source, dest, inputSize, NULL, maxOutputSize, limitedOutput, tableType, withPrefix64k, dictSmall, acceleration);
        else
            return LZ4_compress_generic(streamPtr, source, dest, inputSize, NULL, maxOutputSize, limitedOutput, tableType, withPrefix64k, noDictIssue, acceleration);
    }

    /* external dictionary mode */
    {   int result;
        if (streamPtr->dictCtx) {
            /* We depend here on the fact that dictCtx'es (produced by
             * LZ4_loadDict) guarantee that their tables contain no references
             * to offsets between dictCtx->currentOffset - 64 KB and
             * dictCtx->currentOffset - dictCtx->dictSize. This makes it safe
             * to use noDictIssue even when the dict isn't a full 64 KB.
             */
            if (inputSize > 4 KB) {
                /* For compressing large blobs, it is faster to pay the setup
                 * cost to copy the dictionary's tables into the active context,
                 * so that the compression loop is only looking into one table.
                 */
                LZ4_memcpy(streamPtr, streamPtr->dictCtx, sizeof(*streamPtr));
                result = LZ4_compress_generic(streamPtr, source, dest, inputSize, NULL, maxOutputSize, limitedOutput, tableType, usingExtDict, noDictIssue, acceleration);
            } else {
                result = LZ4_compress_generic(streamPtr, source, dest, inputSize, NULL, maxOutputSize, limitedOutput, tableType, usingDictCtx, noDictIssue, acceleration);
            }
        } else {
            if ((streamPtr->dictSize < 64 KB) && (streamPtr->dictSize < streamPtr->currentOffset)) {
                result = LZ4_compress_generic(streamPtr, source, dest, inputSize, NULL, maxOutputSize, limitedOutput, tableType, usingExtDict, dictSmall, acceleration);
            } else {
                result = LZ4_compress_generic(streamPtr, source, dest, inputSize, NULL, maxOutputSize, limitedOutput, tableType, usingExtDict, noDictIssue, acceleration);
            }
        }
        streamPtr->dictionary = (const BYTE*)source;
        streamPtr->dictSize = (U32)inputSize;
        return result;
    }
}

/* Hidden debug function, to force-test external dictionary mode */
int LZ4_compress_forceExtDict (LZ4_stream_t* LZ4_dict, const char* source, char* dest, int srcSize)
{
    LZ4_stream_t_internal* streamPtr = &LZ4_dict->internal_donotuse;
    int result;

    LZ4_renormDictT(streamPtr, srcSize);

    if ((streamPtr->dictSize < 64 KB) && (streamPtr->dictSize < streamPtr->currentOffset)) {
        result = LZ4_compress_generic(streamPtr, source, dest, srcSize, NULL, 0, notLimited, byU32, usingExtDict, dictSmall, 1);
    } else {
        result = LZ4_compress_generic(streamPtr, source, dest, srcSize, NULL, 0, notLimited, byU32, usingExtDict, noDictIssue, 1);
    }

    streamPtr->dictionary = (const BYTE*)source;
    streamPtr->dictSize = (U32)srcSize;

    return result;
}

/*! LZ4_saveDict() :
 *  If previously compressed data block is not guaranteed to remain available at its memory location,
 *  save it into a safer place (char* safeBuffer).
 *  Note : you don't need to call LZ4_loadDict() afterwards,
 *         dictionary is immediately usable, you can therefore call LZ4_compress_fast_continue().
 *  Return : saved dictionary size in bytes (necessarily <= dictSize), or 0 if error.
 */
int LZ4_saveDict (LZ4_stream_t* LZ4_dict, char* safeBuffer, int dictSize)
{
    LZ4_stream_t_internal* const dict = &LZ4_dict->internal_donotuse;
    const BYTE* const previousDictEnd = dict->dictionary + dict->dictSize;

    if ((U32)dictSize > 64 KB) { dictSize = 64 KB; } /* useless to define a dictionary > 64 KB */
    if ((U32)dictSize > dict->dictSize) { dictSize = (int)dict->dictSize; }

    if (safeBuffer == NULL) assert(dictSize == 0);
    if (dictSize > 0)
        memmove(safeBuffer, previousDictEnd - dictSize, dictSize);

    dict->dictionary = (const BYTE*)safeBuffer;
    dict->dictSize = (U32)dictSize;

    return dictSize;
}

/*-*******************************
 *  Decompression functions
 ********************************/

typedef enum { endOnOutputSize = 0, endOnInputSize = 1 } endCondition_directive;
typedef enum { decode_full_block = 0, partial_decode = 1 } earlyEnd_directive;

#undef MIN
#define MIN(a,b)    ( (a) < (b) ? (a) : (b) )

/* Read the variable-length literal or match length.
 *
 * ip - pointer to use as input.
 * lencheck - end ip.  Return an error if ip advances >= lencheck.
 * loop_check - check ip >= lencheck in body of loop.  Returns loop_error if so.
 * initial_check - check ip >= lencheck before start of loop.  Returns initial_error if so.
 * error (output) - error code.  Should be set to 0 before call.
 */
typedef enum { loop_error = -2, initial_error = -1, ok = 0 } variable_length_error;
LZ4_FORCE_INLINE unsigned
read_variable_length(const BYTE**ip, const BYTE* lencheck,
                     int loop_check, int initial_check,
                     variable_length_error* error)
{
    U32 length = 0;
    U32 s;
    if (initial_check && unlikely((*ip) >= lencheck)) {    /* overflow detection */
        *error = initial_error;
        return length;
    }
    do {
        s = **ip;
        (*ip)++;
        length += s;
        if (loop_check && unlikely((*ip) >= lencheck)) {    /* overflow detection */
            *error = loop_error;
            return length;
        }
    } while (s==255);

    return length;
}

/*! LZ4_decompress_generic() :
 *  This generic decompression function covers all use cases.
 *  It shall be instantiated several times, using different sets of directives.
 *  Note that it is important for performance that this function really get inlined,
 *  in order to remove useless branches during compilation optimization.
 */
LZ4_FORCE_INLINE int
LZ4_decompress_generic(
                 const char* const src,
                 char* const dst,
                 int srcSize,
                 int outputSize,         /* If endOnInput==endOnInputSize, this value is `dstCapacity` */

                 endCondition_directive endOnInput,   /* endOnOutputSize, endOnInputSize */
                 earlyEnd_directive partialDecoding,  /* full, partial */
                 dict_directive dict,                 /* noDict, withPrefix64k, usingExtDict */
                 const BYTE* const lowPrefix,  /* always <= dst, == dst when no prefix */
                 const BYTE* const dictStart,  /* only if dict==usingExtDict */
                 const size_t dictSize         /* note : = 0 if noDict */
                 )
{
    if (src == NULL) { return -1; }

    {   const BYTE* ip = (const BYTE*) src;
        const BYTE* const iend = ip + srcSize;

        BYTE* op = (BYTE*) dst;
        BYTE* const oend = op + outputSize;
        BYTE* cpy;

        const BYTE* const dictEnd = (dictStart == NULL) ? NULL : dictStart + dictSize;

        const int safeDecode = (endOnInput==endOnInputSize);
        const int checkOffset = ((safeDecode) && (dictSize < (int)(64 KB)));

        /* Set up the "end" pointers for the shortcut. */
        const BYTE* const shortiend = iend - (endOnInput ? 14 : 8) /*maxLL*/ - 2 /*offset*/;
        const BYTE* const shortoend = oend - (endOnInput ? 14 : 8) /*maxLL*/ - 18 /*maxML*/;

        const BYTE* match;
        size_t offset;
        unsigned token;
        size_t length;

        DEBUGLOG(5, "LZ4_decompress_generic (srcSize:%i, dstSize:%i)", srcSize, outputSize);

        /* Special cases */
        assert(lowPrefix <= op);
        if ((endOnInput) && (unlikely(outputSize==0))) {
            /* Empty output buffer */
            if (partialDecoding) return 0;
            return ((srcSize==1) && (*ip==0)) ? 0 : -1;
        }
        if ((!endOnInput) && (unlikely(outputSize==0))) { return (*ip==0 ? 1 : -1); }
        if ((endOnInput) && unlikely(srcSize==0)) { return -1; }

	/* Currently the fast loop shows a regression on qualcomm arm chips. */
#if LZ4_FAST_DEC_LOOP
        if ((oend - op) < FASTLOOP_SAFE_DISTANCE) {
            DEBUGLOG(6, "skip fast decode loop");
            goto safe_decode;
        }

        /* Fast loop : decode sequences as long as output < iend-FASTLOOP_SAFE_DISTANCE */
        while (1) {
            /* Main fastloop assertion: We can always wildcopy FASTLOOP_SAFE_DISTANCE */
            assert(oend - op >= FASTLOOP_SAFE_DISTANCE);
            if (endOnInput) { assert(ip < iend); }
            token = *ip++;
            length = token >> ML_BITS;  /* literal length */

            assert(!endOnInput || ip <= iend); /* ip < iend before the increment */

            /* decode literal length */
            if (length == RUN_MASK) {
                variable_length_error error = ok;
                length += read_variable_length(&ip, iend-RUN_MASK, (int)endOnInput, (int)endOnInput, &error);
                if (error == initial_error) { goto _output_error; }
                if ((safeDecode) && unlikely((uptrval)(op)+length<(uptrval)(op))) { goto _output_error; } /* overflow detection */
                if ((safeDecode) && unlikely((uptrval)(ip)+length<(uptrval)(ip))) { goto _output_error; } /* overflow detection */

                /* copy literals */
                cpy = op+length;
                LZ4_STATIC_ASSERT(MFLIMIT >= WILDCOPYLENGTH);
                if (endOnInput) {  /* LZ4_decompress_safe() */
                    if ((cpy>oend-32) || (ip+length>iend-32)) { goto safe_literal_copy; }
                    LZ4_wildCopy32(op, ip, cpy);
                } else {   /* LZ4_decompress_fast() */
                    if (cpy>oend-8) { goto safe_literal_copy; }
                    LZ4_wildCopy8(op, ip, cpy); /* LZ4_decompress_fast() cannot copy more than 8 bytes at a time :
                                                 * it doesn't know input length, and only relies on end-of-block properties */
                }
                ip += length; op = cpy;
            } else {
                cpy = op+length;
                if (endOnInput) {  /* LZ4_decompress_safe() */
                    DEBUGLOG(7, "copy %u bytes in a 16-bytes stripe", (unsigned)length);
                    /* We don't need to check oend, since we check it once for each loop below */
                    if (ip > iend-(16 + 1/*max lit + offset + nextToken*/)) { goto safe_literal_copy; }
                    /* Literals can only be 14, but hope compilers optimize if we copy by a register size */
                    LZ4_memcpy(op, ip, 16);
                } else {  /* LZ4_decompress_fast() */
                    /* LZ4_decompress_fast() cannot copy more than 8 bytes at a time :
                     * it doesn't know input length, and relies on end-of-block properties */
                    LZ4_memcpy(op, ip, 8);
                    if (length > 8) { LZ4_memcpy(op+8, ip+8, 8); }
                }
                ip += length; op = cpy;
            }

            /* get offset */
            offset = LZ4_readLE16(ip); ip+=2;
            match = op - offset;
            assert(match <= op);

            /* get matchlength */
            length = token & ML_MASK;

            if (length == ML_MASK) {
                variable_length_error error = ok;
                if ((checkOffset) && (unlikely(match + dictSize < lowPrefix))) { goto _output_error; } /* Error : offset outside buffers */
                length += read_variable_length(&ip, iend - LASTLITERALS + 1, (int)endOnInput, 0, &error);
                if (error != ok) { goto _output_error; }
                if ((safeDecode) && unlikely((uptrval)(op)+length<(uptrval)op)) { goto _output_error; } /* overflow detection */
                length += MINMATCH;
                if (op + length >= oend - FASTLOOP_SAFE_DISTANCE) {
                    goto safe_match_copy;
                }
            } else {
                length += MINMATCH;
                if (op + length >= oend - FASTLOOP_SAFE_DISTANCE) {
                    goto safe_match_copy;
                }

                /* Fastpath check: Avoids a branch in LZ4_wildCopy32 if true */
                if ((dict == withPrefix64k) || (match >= lowPrefix)) {
                    if (offset >= 8) {
                        assert(match >= lowPrefix);
                        assert(match <= op);
                        assert(op + 18 <= oend);

                        LZ4_memcpy(op, match, 8);
                        LZ4_memcpy(op+8, match+8, 8);
                        LZ4_memcpy(op+16, match+16, 2);
                        op += length;
                        continue;
            }   }   }

            if (checkOffset && (unlikely(match + dictSize < lowPrefix))) { goto _output_error; } /* Error : offset outside buffers */
            /* match starting within external dictionary */
            if ((dict==usingExtDict) && (match < lowPrefix)) {
                if (unlikely(op+length > oend-LASTLITERALS)) {
                    if (partialDecoding) {
                        DEBUGLOG(7, "partialDecoding: dictionary match, close to dstEnd");
                        length = MIN(length, (size_t)(oend-op));
                    } else {
                        goto _output_error;  /* end-of-block condition violated */
                }   }

                if (length <= (size_t)(lowPrefix-match)) {
                    /* match fits entirely within external dictionary : just copy */
                    memmove(op, dictEnd - (lowPrefix-match), length);
                    op += length;
                } else {
                    /* match stretches into both external dictionary and current block */
                    size_t const copySize = (size_t)(lowPrefix - match);
                    size_t const restSize = length - copySize;
                    LZ4_memcpy(op, dictEnd - copySize, copySize);
                    op += copySize;
                    if (restSize > (size_t)(op - lowPrefix)) {  /* overlap copy */
                        BYTE* const endOfMatch = op + restSize;
                        const BYTE* copyFrom = lowPrefix;
                        while (op < endOfMatch) { *op++ = *copyFrom++; }
                    } else {
                        LZ4_memcpy(op, lowPrefix, restSize);
                        op += restSize;
                }   }
                continue;
            }

            /* copy match within block */
            cpy = op + length;

            assert((op <= oend) && (oend-op >= 32));
            if (unlikely(offset<16)) {
                LZ4_memcpy_using_offset(op, match, cpy, offset);
            } else {
                LZ4_wildCopy32(op, match, cpy);
            }

            op = cpy;   /* wildcopy correction */
        }
    safe_decode:
#endif

        /* Main Loop : decode remaining sequences where output < FASTLOOP_SAFE_DISTANCE */
        while (1) {
            token = *ip++;
            length = token >> ML_BITS;  /* literal length */

            assert(!endOnInput || ip <= iend); /* ip < iend before the increment */

            /* A two-stage shortcut for the most common case:
             * 1) If the literal length is 0..14, and there is enough space,
             * enter the shortcut and copy 16 bytes on behalf of the literals
             * (in the fast mode, only 8 bytes can be safely copied this way).
             * 2) Further if the match length is 4..18, copy 18 bytes in a similar
             * manner; but we ensure that there's enough space in the output for
             * those 18 bytes earlier, upon entering the shortcut (in other words,
             * there is a combined check for both stages).
             */
            if ( (endOnInput ? length != RUN_MASK : length <= 8)
                /* strictly "less than" on input, to re-enter the loop with at least one byte */
              && likely((endOnInput ? ip < shortiend : 1) & (op <= shortoend)) ) {
                /* Copy the literals */
                LZ4_memcpy(op, ip, endOnInput ? 16 : 8);
                op += length; ip += length;

                /* The second stage: prepare for match copying, decode full info.
                 * If it doesn't work out, the info won't be wasted. */
                length = token & ML_MASK; /* match length */
                offset = LZ4_readLE16(ip); ip += 2;
                match = op - offset;
                assert(match <= op); /* check overflow */

                /* Do not deal with overlapping matches. */
                if ( (length != ML_MASK)
                  && (offset >= 8)
                  && (dict==withPrefix64k || match >= lowPrefix) ) {
                    /* Copy the match. */
                    LZ4_memcpy(op + 0, match + 0, 8);
                    LZ4_memcpy(op + 8, match + 8, 8);
                    LZ4_memcpy(op +16, match +16, 2);
                    op += length + MINMATCH;
                    /* Both stages worked, load the next token. */
                    continue;
                }

                /* The second stage didn't work out, but the info is ready.
                 * Propel it right to the point of match copying. */
                goto _copy_match;
            }

            /* decode literal length */
            if (length == RUN_MASK) {
                variable_length_error error = ok;
                length += read_variable_length(&ip, iend-RUN_MASK, (int)endOnInput, (int)endOnInput, &error);
                if (error == initial_error) { goto _output_error; }
                if ((safeDecode) && unlikely((uptrval)(op)+length<(uptrval)(op))) { goto _output_error; } /* overflow detection */
                if ((safeDecode) && unlikely((uptrval)(ip)+length<(uptrval)(ip))) { goto _output_error; } /* overflow detection */
            }

            /* copy literals */
            cpy = op+length;
#if LZ4_FAST_DEC_LOOP
        safe_literal_copy:
#endif
            LZ4_STATIC_ASSERT(MFLIMIT >= WILDCOPYLENGTH);
            if ( ((endOnInput) && ((cpy>oend-MFLIMIT) || (ip+length>iend-(2+1+LASTLITERALS))) )
              || ((!endOnInput) && (cpy>oend-WILDCOPYLENGTH)) )
            {
                /* We've either hit the input parsing restriction or the output parsing restriction.
                 * In the normal scenario, decoding a full block, it must be the last sequence,
                 * otherwise it's an error (invalid input or dimensions).
                 * In partialDecoding scenario, it's necessary to ensure there is no buffer overflow.
                 */
                if (partialDecoding) {
                    /* Since we are partial decoding we may be in this block because of the output parsing
                     * restriction, which is not valid since the output buffer is allowed to be undersized.
                     */
                    assert(endOnInput);
                    DEBUGLOG(7, "partialDecoding: copying literals, close to input or output end")
                    DEBUGLOG(7, "partialDecoding: literal length = %u", (unsigned)length);
                    DEBUGLOG(7, "partialDecoding: remaining space in dstBuffer : %i", (int)(oend - op));
                    DEBUGLOG(7, "partialDecoding: remaining space in srcBuffer : %i", (int)(iend - ip));
                    /* Finishing in the middle of a literals segment,
                     * due to lack of input.
                     */
                    if (ip+length > iend) {
                        length = (size_t)(iend-ip);
                        cpy = op + length;
                    }
                    /* Finishing in the middle of a literals segment,
                     * due to lack of output space.
                     */
                    if (cpy > oend) {
                        cpy = oend;
                        assert(op<=oend);
                        length = (size_t)(oend-op);
                    }
                } else {
                    /* We must be on the last sequence because of the parsing limitations so check
                     * that we exactly regenerate the original size (must be exact when !endOnInput).
                     */
                    if ((!endOnInput) && (cpy != oend)) { goto _output_error; }
                     /* We must be on the last sequence (or invalid) because of the parsing limitations
                      * so check that we exactly consume the input and don't overrun the output buffer.
                      */
                    if ((endOnInput) && ((ip+length != iend) || (cpy > oend))) {
                        DEBUGLOG(6, "should have been last run of literals")
                        DEBUGLOG(6, "ip(%p) + length(%i) = %p != iend (%p)", ip, (int)length, ip+length, iend);
                        DEBUGLOG(6, "or cpy(%p) > oend(%p)", cpy, oend);
                        goto _output_error;
                    }
                }
                memmove(op, ip, length);  /* supports overlapping memory regions; only matters for in-place decompression scenarios */
                ip += length;
                op += length;
                /* Necessarily EOF when !partialDecoding.
                 * When partialDecoding, it is EOF if we've either
                 * filled the output buffer or
                 * can't proceed with reading an offset for following match.
                 */
                if (!partialDecoding || (cpy == oend) || (ip >= (iend-2))) {
                    break;
                }
            } else {
                LZ4_wildCopy8(op, ip, cpy);   /* may overwrite up to WILDCOPYLENGTH beyond cpy */
                ip += length; op = cpy;
            }

            /* get offset */
            offset = LZ4_readLE16(ip); ip+=2;
            match = op - offset;

            /* get matchlength */
            length = token & ML_MASK;

    _copy_match:
            if (length == ML_MASK) {
              variable_length_error error = ok;
              length += read_variable_length(&ip, iend - LASTLITERALS + 1, (int)endOnInput, 0, &error);
              if (error != ok) goto _output_error;
                if ((safeDecode) && unlikely((uptrval)(op)+length<(uptrval)op)) goto _output_error;   /* overflow detection */
            }
            length += MINMATCH;

#if LZ4_FAST_DEC_LOOP
        safe_match_copy:
#endif
            if ((checkOffset) && (unlikely(match + dictSize < lowPrefix))) goto _output_error;   /* Error : offset outside buffers */
            /* match starting within external dictionary */
            if ((dict==usingExtDict) && (match < lowPrefix)) {
                if (unlikely(op+length > oend-LASTLITERALS)) {
                    if (partialDecoding) length = MIN(length, (size_t)(oend-op));
                    else goto _output_error;   /* doesn't respect parsing restriction */
                }

                if (length <= (size_t)(lowPrefix-match)) {
                    /* match fits entirely within external dictionary : just copy */
                    memmove(op, dictEnd - (lowPrefix-match), length);
                    op += length;
                } else {
                    /* match stretches into both external dictionary and current block */
                    size_t const copySize = (size_t)(lowPrefix - match);
                    size_t const restSize = length - copySize;
                    LZ4_memcpy(op, dictEnd - copySize, copySize);
                    op += copySize;
                    if (restSize > (size_t)(op - lowPrefix)) {  /* overlap copy */
                        BYTE* const endOfMatch = op + restSize;
                        const BYTE* copyFrom = lowPrefix;
                        while (op < endOfMatch) *op++ = *copyFrom++;
                    } else {
                        LZ4_memcpy(op, lowPrefix, restSize);
                        op += restSize;
                }   }
                continue;
            }
            assert(match >= lowPrefix);

            /* copy match within block */
            cpy = op + length;

            /* partialDecoding : may end anywhere within the block */
            assert(op<=oend);
            if (partialDecoding && (cpy > oend-MATCH_SAFEGUARD_DISTANCE)) {
                size_t const mlen = MIN(length, (size_t)(oend-op));
                const BYTE* const matchEnd = match + mlen;
                BYTE* const copyEnd = op + mlen;
                if (matchEnd > op) {   /* overlap copy */
                    while (op < copyEnd) { *op++ = *match++; }
                } else {
                    LZ4_memcpy(op, match, mlen);
                }
                op = copyEnd;
                if (op == oend) { break; }
                continue;
            }

            if (unlikely(offset<8)) {
                LZ4_write32(op, 0);   /* silence msan warning when offset==0 */
                op[0] = match[0];
                op[1] = match[1];
                op[2] = match[2];
                op[3] = match[3];
                match += inc32table[offset];
                LZ4_memcpy(op+4, match, 4);
                match -= dec64table[offset];
            } else {
                LZ4_memcpy(op, match, 8);
                match += 8;
            }
            op += 8;

            if (unlikely(cpy > oend-MATCH_SAFEGUARD_DISTANCE)) {
                BYTE* const oCopyLimit = oend - (WILDCOPYLENGTH-1);
                if (cpy > oend-LASTLITERALS) { goto _output_error; } /* Error : last LASTLITERALS bytes must be literals (uncompressed) */
                if (op < oCopyLimit) {
                    LZ4_wildCopy8(op, match, oCopyLimit);
                    match += oCopyLimit - op;
                    op = oCopyLimit;
                }
                while (op < cpy) { *op++ = *match++; }
            } else {
                LZ4_memcpy(op, match, 8);
                if (length > 16)  { LZ4_wildCopy8(op+8, match+8, cpy); }
            }
            op = cpy;   /* wildcopy correction */
        }

        /* end of decoding */
        if (endOnInput) {
            DEBUGLOG(5, "decoded %i bytes", (int) (((char*)op)-dst));
           return (int) (((char*)op)-dst);     /* Nb of output bytes decoded */
       } else {
           return (int) (((const char*)ip)-src);   /* Nb of input bytes read */
       }

        /* Overflow error detected */
    _output_error:
        return (int) (-(((const char*)ip)-src))-1;
    }
}

/*===== Instantiate the API decoding functions. =====*/

LZ4_FORCE_O2
int LZ4_decompress_safe(const char* source, char* dest, int compressedSize, int maxDecompressedSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxDecompressedSize,
                                  endOnInputSize, decode_full_block, noDict,
                                  (BYTE*)dest, NULL, 0);
}

LZ4_FORCE_O2
int LZ4_decompress_safe_partial(const char* src, char* dst, int compressedSize, int targetOutputSize, int dstCapacity)
{
    dstCapacity = MIN(targetOutputSize, dstCapacity);
    return LZ4_decompress_generic(src, dst, compressedSize, dstCapacity,
                                  endOnInputSize, partial_decode,
                                  noDict, (BYTE*)dst, NULL, 0);
}

LZ4_FORCE_O2
int LZ4_decompress_fast(const char* source, char* dest, int originalSize)
{
    return LZ4_decompress_generic(source, dest, 0, originalSize,
                                  endOnOutputSize, decode_full_block, withPrefix64k,
                                  (BYTE*)dest - 64 KB, NULL, 0);
}

/*===== Instantiate a few more decoding cases, used more than once. =====*/

LZ4_FORCE_O2 /* Exported, an obsolete API function. */
int LZ4_decompress_safe_withPrefix64k(const char* source, char* dest, int compressedSize, int maxOutputSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize,
                                  endOnInputSize, decode_full_block, withPrefix64k,
                                  (BYTE*)dest - 64 KB, NULL, 0);
}

/* Another obsolete API function, paired with the previous one. */
int LZ4_decompress_fast_withPrefix64k(const char* source, char* dest, int originalSize)
{
    /* LZ4_decompress_fast doesn't validate match offsets,
     * and thus serves well with any prefixed dictionary. */
    return LZ4_decompress_fast(source, dest, originalSize);
}

LZ4_FORCE_O2
static int LZ4_decompress_safe_withSmallPrefix(const char* source, char* dest, int compressedSize, int maxOutputSize,
                                               size_t prefixSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize,
                                  endOnInputSize, decode_full_block, noDict,
                                  (BYTE*)dest-prefixSize, NULL, 0);
}

LZ4_FORCE_O2
int LZ4_decompress_safe_forceExtDict(const char* source, char* dest,
                                     int compressedSize, int maxOutputSize,
                                     const void* dictStart, size_t dictSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize,
                                  endOnInputSize, decode_full_block, usingExtDict,
                                  (BYTE*)dest, (const BYTE*)dictStart, dictSize);
}

LZ4_FORCE_O2
static int LZ4_decompress_fast_extDict(const char* source, char* dest, int originalSize,
                                       const void* dictStart, size_t dictSize)
{
    return LZ4_decompress_generic(source, dest, 0, originalSize,
                                  endOnOutputSize, decode_full_block, usingExtDict,
                                  (BYTE*)dest, (const BYTE*)dictStart, dictSize);
}

/* The "double dictionary" mode, for use with e.g. ring buffers: the first part
 * of the dictionary is passed as prefix, and the second via dictStart + dictSize.
 * These routines are used only once, in LZ4_decompress_*_continue().
 */
LZ4_FORCE_INLINE
int LZ4_decompress_safe_doubleDict(const char* source, char* dest, int compressedSize, int maxOutputSize,
                                   size_t prefixSize, const void* dictStart, size_t dictSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize,
                                  endOnInputSize, decode_full_block, usingExtDict,
                                  (BYTE*)dest-prefixSize, (const BYTE*)dictStart, dictSize);
}

LZ4_FORCE_INLINE
int LZ4_decompress_fast_doubleDict(const char* source, char* dest, int originalSize,
                                   size_t prefixSize, const void* dictStart, size_t dictSize)
{
    return LZ4_decompress_generic(source, dest, 0, originalSize,
                                  endOnOutputSize, decode_full_block, usingExtDict,
                                  (BYTE*)dest-prefixSize, (const BYTE*)dictStart, dictSize);
}

/*===== streaming decompression functions =====*/

LZ4_streamDecode_t* LZ4_createStreamDecode(void)
{
    LZ4_streamDecode_t* lz4s = (LZ4_streamDecode_t*) ALLOC_AND_ZERO(sizeof(LZ4_streamDecode_t));
    LZ4_STATIC_ASSERT(LZ4_STREAMDECODESIZE >= sizeof(LZ4_streamDecode_t_internal));    /* A compilation error here means LZ4_STREAMDECODESIZE is not large enough */
    return lz4s;
}

int LZ4_freeStreamDecode (LZ4_streamDecode_t* LZ4_stream)
{
    if (LZ4_stream == NULL) { return 0; }  /* support free on NULL */
    FREEMEM(LZ4_stream);
    return 0;
}

/*! LZ4_setStreamDecode() :
 *  Use this function to instruct where to find the dictionary.
 *  This function is not necessary if previous data is still available where it was decoded.
 *  Loading a size of 0 is allowed (same effect as no dictionary).
 * @return : 1 if OK, 0 if error
 */
int LZ4_setStreamDecode (LZ4_streamDecode_t* LZ4_streamDecode, const char* dictionary, int dictSize)
{
    LZ4_streamDecode_t_internal* lz4sd = &LZ4_streamDecode->internal_donotuse;
    lz4sd->prefixSize = (size_t) dictSize;
    lz4sd->prefixEnd = (const BYTE*) dictionary + dictSize;
    lz4sd->externalDict = NULL;
    lz4sd->extDictSize  = 0;
    return 1;
}

/*! LZ4_decoderRingBufferSize() :
 *  when setting a ring buffer for streaming decompression (optional scenario),
 *  provides the minimum size of this ring buffer
 *  to be compatible with any source respecting maxBlockSize condition.
 *  Note : in a ring buffer scenario,
 *  blocks are presumed decompressed next to each other.
 *  When not enough space remains for next block (remainingSize < maxBlockSize),
 *  decoding resumes from beginning of ring buffer.
 * @return : minimum ring buffer size,
 *           or 0 if there is an error (invalid maxBlockSize).
 */
int LZ4_decoderRingBufferSize(int maxBlockSize)
{
    if (maxBlockSize < 0) return 0;
    if (maxBlockSize > LZ4_MAX_INPUT_SIZE) return 0;
    if (maxBlockSize < 16) maxBlockSize = 16;
    return LZ4_DECODER_RING_BUFFER_SIZE(maxBlockSize);
}

/*
*_continue() :
    These decoding functions allow decompression of multiple blocks in "streaming" mode.
    Previously decoded blocks must still be available at the memory position where they were decoded.
    If it's not possible, save the relevant part of decoded data into a safe buffer,
    and indicate where it stands using LZ4_setStreamDecode()
*/
LZ4_FORCE_O2
int LZ4_decompress_safe_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int compressedSize, int maxOutputSize)
{
    LZ4_streamDecode_t_internal* lz4sd = &LZ4_streamDecode->internal_donotuse;
    int result;

    if (lz4sd->prefixSize == 0) {
        /* The first call, no dictionary yet. */
        assert(lz4sd->extDictSize == 0);
        result = LZ4_decompress_safe(source, dest, compressedSize, maxOutputSize);
        if (result <= 0) return result;
        lz4sd->prefixSize = (size_t)result;
        lz4sd->prefixEnd = (BYTE*)dest + result;
    } else if (lz4sd->prefixEnd == (BYTE*)dest) {
        /* They're rolling the current segment. */
        if (lz4sd->prefixSize >= 64 KB - 1)
            result = LZ4_decompress_safe_withPrefix64k(source, dest, compressedSize, maxOutputSize);
        else if (lz4sd->extDictSize == 0)
            result = LZ4_decompress_safe_withSmallPrefix(source, dest, compressedSize, maxOutputSize,
                                                         lz4sd->prefixSize);
        else
            result = LZ4_decompress_safe_doubleDict(source, dest, compressedSize, maxOutputSize,
                                                    lz4sd->prefixSize, lz4sd->externalDict, lz4sd->extDictSize);
        if (result <= 0) return result;
        lz4sd->prefixSize += (size_t)result;
        lz4sd->prefixEnd  += result;
    } else {
        /* The buffer wraps around, or they're switching to another buffer. */
        lz4sd->extDictSize = lz4sd->prefixSize;
        lz4sd->externalDict = lz4sd->prefixEnd - lz4sd->extDictSize;
        result = LZ4_decompress_safe_forceExtDict(source, dest, compressedSize, maxOutputSize,
                                                  lz4sd->externalDict, lz4sd->extDictSize);
        if (result <= 0) return result;
        lz4sd->prefixSize = (size_t)result;
        lz4sd->prefixEnd  = (BYTE*)dest + result;
    }

    return result;
}

LZ4_FORCE_O2
int LZ4_decompress_fast_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int originalSize)
{
    LZ4_streamDecode_t_internal* lz4sd = &LZ4_streamDecode->internal_donotuse;
    int result;
    assert(originalSize >= 0);

    if (lz4sd->prefixSize == 0) {
        assert(lz4sd->extDictSize == 0);
        result = LZ4_decompress_fast(source, dest, originalSize);
        if (result <= 0) return result;
        lz4sd->prefixSize = (size_t)originalSize;
        lz4sd->prefixEnd = (BYTE*)dest + originalSize;
    } else if (lz4sd->prefixEnd == (BYTE*)dest) {
        if (lz4sd->prefixSize >= 64 KB - 1 || lz4sd->extDictSize == 0)
            result = LZ4_decompress_fast(source, dest, originalSize);
        else
            result = LZ4_decompress_fast_doubleDict(source, dest, originalSize,
                                                    lz4sd->prefixSize, lz4sd->externalDict, lz4sd->extDictSize);
        if (result <= 0) return result;
        lz4sd->prefixSize += (size_t)originalSize;
        lz4sd->prefixEnd  += originalSize;
    } else {
        lz4sd->extDictSize = lz4sd->prefixSize;
        lz4sd->externalDict = lz4sd->prefixEnd - lz4sd->extDictSize;
        result = LZ4_decompress_fast_extDict(source, dest, originalSize,
                                             lz4sd->externalDict, lz4sd->extDictSize);
        if (result <= 0) return result;
        lz4sd->prefixSize = (size_t)originalSize;
        lz4sd->prefixEnd  = (BYTE*)dest + originalSize;
    }

    return result;
}

/*
Advanced decoding functions :
*_usingDict() :
    These decoding functions work the same as "_continue" ones,
    the dictionary must be explicitly provided within parameters
*/

int LZ4_decompress_safe_usingDict(const char* source, char* dest, int compressedSize, int maxOutputSize, const char* dictStart, int dictSize)
{
    if (dictSize==0)
        return LZ4_decompress_safe(source, dest, compressedSize, maxOutputSize);
    if (dictStart+dictSize == dest) {
        if (dictSize >= 64 KB - 1) {
            return LZ4_decompress_safe_withPrefix64k(source, dest, compressedSize, maxOutputSize);
        }
        assert(dictSize >= 0);
        return LZ4_decompress_safe_withSmallPrefix(source, dest, compressedSize, maxOutputSize, (size_t)dictSize);
    }
    assert(dictSize >= 0);
    return LZ4_decompress_safe_forceExtDict(source, dest, compressedSize, maxOutputSize, dictStart, (size_t)dictSize);
}

int LZ4_decompress_fast_usingDict(const char* source, char* dest, int originalSize, const char* dictStart, int dictSize)
{
    if (dictSize==0 || dictStart+dictSize == dest)
        return LZ4_decompress_fast(source, dest, originalSize);
    assert(dictSize >= 0);
    return LZ4_decompress_fast_extDict(source, dest, originalSize, dictStart, (size_t)dictSize);
}

/*=*************************************************
*  Obsolete Functions
***************************************************/
/* obsolete compression functions */
int LZ4_compress_limitedOutput(const char* source, char* dest, int inputSize, int maxOutputSize)
{
    return LZ4_compress_default(source, dest, inputSize, maxOutputSize);
}
int LZ4_compress(const char* src, char* dest, int srcSize)
{
    return LZ4_compress_default(src, dest, srcSize, LZ4_compressBound(srcSize));
}
int LZ4_compress_limitedOutput_withState (void* state, const char* src, char* dst, int srcSize, int dstSize)
{
    return LZ4_compress_fast_extState(state, src, dst, srcSize, dstSize, 1);
}
int LZ4_compress_withState (void* state, const char* src, char* dst, int srcSize)
{
    return LZ4_compress_fast_extState(state, src, dst, srcSize, LZ4_compressBound(srcSize), 1);
}
int LZ4_compress_limitedOutput_continue (LZ4_stream_t* LZ4_stream, const char* src, char* dst, int srcSize, int dstCapacity)
{
    return LZ4_compress_fast_continue(LZ4_stream, src, dst, srcSize, dstCapacity, 1);
}
int LZ4_compress_continue (LZ4_stream_t* LZ4_stream, const char* source, char* dest, int inputSize)
{
    return LZ4_compress_fast_continue(LZ4_stream, source, dest, inputSize, LZ4_compressBound(inputSize), 1);
}

/*
These decompression functions are deprecated and should no longer be used.
They are only provided here for compatibility with older user programs.
- LZ4_uncompress is totally equivalent to LZ4_decompress_fast
- LZ4_uncompress_unknownOutputSize is totally equivalent to LZ4_decompress_safe
*/
int LZ4_uncompress (const char* source, char* dest, int outputSize)
{
    return LZ4_decompress_fast(source, dest, outputSize);
}
int LZ4_uncompress_unknownOutputSize (const char* source, char* dest, int isize, int maxOutputSize)
{
    return LZ4_decompress_safe(source, dest, isize, maxOutputSize);
}

/* Obsolete Streaming functions */

int LZ4_sizeofStreamState(void) { return LZ4_STREAMSIZE; }

int LZ4_resetStreamState(void* state, char* inputBuffer)
{
    (void)inputBuffer;
    LZ4_resetStream((LZ4_stream_t*)state);
    return 0;
}

void* LZ4_create (char* inputBuffer)
{
    (void)inputBuffer;
    return LZ4_createStream();
}

char* LZ4_slideInputBuffer (void* state)
{
    /* avoid const char * -> char * conversion warning */
    return (char *)(uptrval)((LZ4_stream_t*)state)->internal_donotuse.dictionary;
}

#endif   /* LZ4_COMMONDEFS_ONLY */

}

/*** End of inlined file: tracy_lz4.cpp ***/


/*** Start of inlined file: TracyProfiler.cpp ***/
#ifdef TRACY_ENABLE

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <winsock2.h>
#  include <windows.h>
#  include <tlhelp32.h>
#  include <inttypes.h>
#  include <intrin.h>
#else
#  include <sys/time.h>
#  include <sys/param.h>
#endif

#ifdef __CYGWIN__
#  include <windows.h>
#  include <unistd.h>
#  include <tlhelp32.h>
#endif

#ifdef _GNU_SOURCE
#  include <errno.h>
#endif

#ifdef __linux__
#  include <dirent.h>
#  include <signal.h>
#  include <pthread.h>
#  include <sys/types.h>
#  include <sys/syscall.h>
#endif

#if defined __APPLE__ || defined BSD
#  include <sys/types.h>
#  include <sys/sysctl.h>
#endif

#if defined __APPLE__
#  include "TargetConditionals.h"
#  include <mach-o/dyld.h>
#endif

#ifdef __ANDROID__
#  include <sys/mman.h>
#  include <stdio.h>
#  include <cstdint>
#  include <algorithm>
#  include <vector>
#endif

#include <algorithm>
#include <cassert>
#include <atomic>
#include <chrono>
#include <limits>
#include <new>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <thread>


/*** Start of inlined file: TracyAlign.hpp ***/
#ifndef __TRACYALIGN_HPP__
#define __TRACYALIGN_HPP__

#include <string.h>

namespace tracy
{

template<typename T>
tracy_force_inline T MemRead( const void* ptr )
{
    T val;
    memcpy( &val, ptr, sizeof( T ) );
    return val;
}

template<typename T>
tracy_force_inline void MemWrite( void* ptr, T val )
{
    memcpy( ptr, &val, sizeof( T ) );
}

}

#endif

/*** End of inlined file: TracyAlign.hpp ***/


/*** Start of inlined file: TracySocket.hpp ***/
#ifndef __TRACYSOCKET_HPP__
#define __TRACYSOCKET_HPP__

#include <atomic>
#include <cstdint>

struct addrinfo;
struct sockaddr;

namespace tracy
{

#ifdef _WIN32
void InitWinSock();
#endif

class Socket
{
public:
    Socket();
    Socket( int sock );
    ~Socket();

    bool Connect( const char* addr, uint16_t port );
    bool ConnectBlocking( const char* addr, uint16_t port );
    void Close();

    int Send( const void* buf, int len );
    int GetSendBufSize();

    int ReadUpTo( void* buf, int len, int timeout );
    bool Read( void* buf, int len, int timeout );

    template<typename ShouldExit>
    bool Read( void* buf, int len, int timeout, ShouldExit exitCb )
    {
        auto cbuf = (char*)buf;
        while( len > 0 )
        {
            if( exitCb() ) return false;
            if( !ReadImpl( cbuf, len, timeout ) ) return false;
        }
        return true;
    }

    bool ReadRaw( void* buf, int len, int timeout );
    bool HasData();
    bool IsValid() const;

    Socket( const Socket& ) = delete;
    Socket( Socket&& ) = delete;
    Socket& operator=( const Socket& ) = delete;
    Socket& operator=( Socket&& ) = delete;

private:
    int RecvBuffered( void* buf, int len, int timeout );
    int Recv( void* buf, int len, int timeout );

    bool ReadImpl( char*& buf, int& len, int timeout );

    char* m_buf;
    char* m_bufPtr;
    std::atomic<int> m_sock;
    int m_bufLeft;

    struct addrinfo *m_res;
    struct addrinfo *m_ptr;
    int m_connSock;
};

class ListenSocket
{
public:
    ListenSocket();
    ~ListenSocket();

    bool Listen( uint16_t port, int backlog );
    Socket* Accept();
    void Close();

    ListenSocket( const ListenSocket& ) = delete;
    ListenSocket( ListenSocket&& ) = delete;
    ListenSocket& operator=( const ListenSocket& ) = delete;
    ListenSocket& operator=( ListenSocket&& ) = delete;

private:
    int m_sock;
};

class UdpBroadcast
{
public:
    UdpBroadcast();
    ~UdpBroadcast();

    bool Open( const char* addr, uint16_t port );
    void Close();

    int Send( uint16_t port, const void* data, int len );

    UdpBroadcast( const UdpBroadcast& ) = delete;
    UdpBroadcast( UdpBroadcast&& ) = delete;
    UdpBroadcast& operator=( const UdpBroadcast& ) = delete;
    UdpBroadcast& operator=( UdpBroadcast&& ) = delete;

private:
    int m_sock;
    uint32_t m_addr;
};

class IpAddress
{
public:
    IpAddress();
    ~IpAddress();

    void Set( const struct sockaddr& addr );

    uint32_t GetNumber() const { return m_number; }
    const char* GetText() const { return m_text; }

    IpAddress( const IpAddress& ) = delete;
    IpAddress( IpAddress&& ) = delete;
    IpAddress& operator=( const IpAddress& ) = delete;
    IpAddress& operator=( IpAddress&& ) = delete;

private:
    uint32_t m_number;
    char m_text[17];
};

class UdpListen
{
public:
    UdpListen();
    ~UdpListen();

    bool Listen( uint16_t port );
    void Close();

    const char* Read( size_t& len, IpAddress& addr, int timeout );

    UdpListen( const UdpListen& ) = delete;
    UdpListen( UdpListen&& ) = delete;
    UdpListen& operator=( const UdpListen& ) = delete;
    UdpListen& operator=( UdpListen&& ) = delete;

private:
    int m_sock;
};

}

#endif

/*** End of inlined file: TracySocket.hpp ***/


/*** Start of inlined file: TracyYield.hpp ***/
#ifndef __TRACYYIELD_HPP__
#define __TRACYYIELD_HPP__

#if defined __SSE2__ || defined _M_AMD64 || _M_IX86_FP == 2
#  include <emmintrin.h>
#else
#  include <thread>
#endif

namespace tracy
{

static tracy_force_inline void YieldThread()
{
#if defined __SSE2__ || defined _M_AMD64 || _M_IX86_FP == 2
    _mm_pause();
#else
    std::this_thread::yield();
#endif
}

}

#endif

/*** End of inlined file: TracyYield.hpp ***/


/*** Start of inlined file: TracyCallstack.hpp ***/
#ifndef __TRACYCALLSTACK_HPP__
#define __TRACYCALLSTACK_HPP__


/*** Start of inlined file: TracyCallstack.h ***/
#ifndef __TRACYCALLSTACK_H__
#define __TRACYCALLSTACK_H__

#if !defined _WIN32 && !defined __CYGWIN__
#  include <sys/param.h>
#endif

#if defined _WIN32 || defined __CYGWIN__
#  define TRACY_HAS_CALLSTACK 1
#elif defined __ANDROID__
#  if !defined __arm__ || __ANDROID_API__ >= 21
#    define TRACY_HAS_CALLSTACK 2
#  else
#    define TRACY_HAS_CALLSTACK 5
#  endif
#elif defined __linux
#  if defined _GNU_SOURCE && defined __GLIBC__
#    define TRACY_HAS_CALLSTACK 3
#  else
#    define TRACY_HAS_CALLSTACK 2
#  endif
#elif defined __APPLE__
#  define TRACY_HAS_CALLSTACK 4
#elif defined BSD
#  define TRACY_HAS_CALLSTACK 6
#endif

#endif

/*** End of inlined file: TracyCallstack.h ***/

#if TRACY_HAS_CALLSTACK == 2 || TRACY_HAS_CALLSTACK == 5
#  include <unwind.h>
#elif TRACY_HAS_CALLSTACK >= 3
#  include <execinfo.h>
#endif

#ifdef TRACY_HAS_CALLSTACK

#include <cassert>
#include <cstdint>

namespace tracy
{

struct CallstackSymbolData
{
    const char* file;
    uint32_t line;
    bool needFree;
    uint64_t symAddr;
};

struct CallstackEntry
{
    const char* name;
    const char* file;
    uint32_t line;
    uint32_t symLen;
    uint64_t symAddr;
};

struct CallstackEntryData
{
    const CallstackEntry* data;
    uint8_t size;
    const char* imageName;
};

CallstackSymbolData DecodeSymbolAddress( uint64_t ptr );
CallstackSymbolData DecodeCodeAddress( uint64_t ptr );
const char* DecodeCallstackPtrFast( uint64_t ptr );
CallstackEntryData DecodeCallstackPtr( uint64_t ptr );
void InitCallstack();

#if TRACY_HAS_CALLSTACK == 1

TRACY_API uintptr_t* CallTrace( int depth );

static tracy_force_inline void* Callstack( int depth )
{
    assert( depth >= 1 && depth < 63 );
    return CallTrace( depth );
}

#elif TRACY_HAS_CALLSTACK == 2 || TRACY_HAS_CALLSTACK == 5

struct BacktraceState
{
    void** current;
    void** end;
};

static _Unwind_Reason_Code tracy_unwind_callback( struct _Unwind_Context* ctx, void* arg )
{
    auto state = (BacktraceState*)arg;
    uintptr_t pc = _Unwind_GetIP( ctx );
    if( pc )
    {
        if( state->current == state->end ) return _URC_END_OF_STACK;
        *state->current++ = (void*)pc;
    }
    return _URC_NO_REASON;
}

static tracy_force_inline void* Callstack( int depth )
{
    assert( depth >= 1 && depth < 63 );

    auto trace = (uintptr_t*)tracy_malloc( ( 1 + depth ) * sizeof( uintptr_t ) );
    BacktraceState state = { (void**)(trace+1), (void**)(trace+1+depth) };
    _Unwind_Backtrace( tracy_unwind_callback, &state );

    *trace = (uintptr_t*)state.current - trace + 1;

    return trace;
}

#elif TRACY_HAS_CALLSTACK == 3 || TRACY_HAS_CALLSTACK == 4 || TRACY_HAS_CALLSTACK == 6

static tracy_force_inline void* Callstack( int depth )
{
    assert( depth >= 1 );

    auto trace = (uintptr_t*)tracy_malloc( ( 1 + (size_t)depth ) * sizeof( uintptr_t ) );
    const auto num = (size_t)backtrace( (void**)(trace+1), depth );
    *trace = num;

    return trace;
}

#endif

}

#endif

#endif

/*** End of inlined file: TracyCallstack.hpp ***/


/*** Start of inlined file: TracyDxt1.hpp ***/
#ifndef __TRACYDXT1_HPP__
#define __TRACYDXT1_HPP__

namespace tracy
{

void CompressImageDxt1( const char* src, char* dst, int w, int h );

}

#endif

/*** End of inlined file: TracyDxt1.hpp ***/


/*** Start of inlined file: TracyScoped.hpp ***/
#ifndef __TRACYSCOPED_HPP__
#define __TRACYSCOPED_HPP__

#include <limits>
#include <cstdint>
#include <string.h>


/*** Start of inlined file: TracyProfiler.hpp ***/
#ifndef __TRACYPROFILER_HPP__
#define __TRACYPROFILER_HPP__

#include <cassert>
#include <atomic>
#include <cstdint>
#include <string.h>
#include <time.h>


/*** Start of inlined file: tracy_concurrentqueue.h ***/
// Provides a C++11 implementation of a multi-producer, multi-consumer lock-free queue.
// An overview, including benchmark results, is provided here:
//     http://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++
// The full design is also described in excruciating detail at:
//    http://moodycamel.com/blog/2014/detailed-design-of-a-lock-free-queue

// Simplified BSD license:
// Copyright (c) 2013-2016, Cameron Desrochers.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// - Redistributions of source code must retain the above copyright notice, this list of
// conditions and the following disclaimer.
// - Redistributions in binary form must reproduce the above copyright notice, this list of
// conditions and the following disclaimer in the documentation and/or other materials
// provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#if defined(__GNUC__)
// Disable -Wconversion warnings (spuriously triggered when Traits::size_t and
// Traits::index_t are set to < 32 bits, causing integer promotion, causing warnings
// upon assigning any computed values)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#if defined(__APPLE__)
#include "TargetConditionals.h"
#endif

#include <atomic>		// Requires C++11. Sorry VS2010.
#include <cassert>
#include <cstddef>              // for max_align_t
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <algorithm>
#include <utility>
#include <limits>
#include <climits>		// for CHAR_BIT
#include <array>
#include <thread>		// partly for __WINPTHREADS_VERSION if on MinGW-w64 w/ POSIX threading

namespace tracy
{

// Compiler-specific likely/unlikely hints
namespace moodycamel { namespace details {
#if defined(__GNUC__)
	inline bool cqLikely(bool x) { return __builtin_expect((x), true); }
	inline bool cqUnlikely(bool x) { return __builtin_expect((x), false); }
#else
	inline bool cqLikely(bool x) { return x; }
	inline bool cqUnlikely(bool x) { return x; }
#endif
} }

namespace
{
    // to avoid MSVC warning 4127: conditional expression is constant
    template <bool>
    struct compile_time_condition
    {
        static const bool value = false;
    };
    template <>
    struct compile_time_condition<true>
    {
        static const bool value = true;
    };
}

namespace moodycamel {
namespace details {
	template<typename T>
	struct const_numeric_max {
		static_assert(std::is_integral<T>::value, "const_numeric_max can only be used with integers");
		static const T value = std::numeric_limits<T>::is_signed
			? (static_cast<T>(1) << (sizeof(T) * CHAR_BIT - 1)) - static_cast<T>(1)
			: static_cast<T>(-1);
	};

#if defined(__GLIBCXX__)
	typedef ::max_align_t std_max_align_t;      // libstdc++ forgot to add it to std:: for a while
#else
	typedef std::max_align_t std_max_align_t;   // Others (e.g. MSVC) insist it can *only* be accessed via std::
#endif

	// Some platforms have incorrectly set max_align_t to a type with <8 bytes alignment even while supporting
	// 8-byte aligned scalar values (*cough* 32-bit iOS). Work around this with our own union. See issue #64.
	typedef union {
		std_max_align_t x;
		long long y;
		void* z;
	} max_align_t;
}

// Default traits for the ConcurrentQueue. To change some of the
// traits without re-implementing all of them, inherit from this
// struct and shadow the declarations you wish to be different;
// since the traits are used as a template type parameter, the
// shadowed declarations will be used where defined, and the defaults
// otherwise.
struct ConcurrentQueueDefaultTraits
{
	// General-purpose size type. std::size_t is strongly recommended.
	typedef std::size_t size_t;

	// The type used for the enqueue and dequeue indices. Must be at least as
	// large as size_t. Should be significantly larger than the number of elements
	// you expect to hold at once, especially if you have a high turnover rate;
	// for example, on 32-bit x86, if you expect to have over a hundred million
	// elements or pump several million elements through your queue in a very
	// short space of time, using a 32-bit type *may* trigger a race condition.
	// A 64-bit int type is recommended in that case, and in practice will
	// prevent a race condition no matter the usage of the queue. Note that
	// whether the queue is lock-free with a 64-int type depends on the whether
	// std::atomic<std::uint64_t> is lock-free, which is platform-specific.
	typedef std::size_t index_t;

	// Internally, all elements are enqueued and dequeued from multi-element
	// blocks; this is the smallest controllable unit. If you expect few elements
	// but many producers, a smaller block size should be favoured. For few producers
	// and/or many elements, a larger block size is preferred. A sane default
	// is provided. Must be a power of 2.
	static const size_t BLOCK_SIZE = 64*1024;

	// For explicit producers (i.e. when using a producer token), the block is
	// checked for being empty by iterating through a list of flags, one per element.
	// For large block sizes, this is too inefficient, and switching to an atomic
	// counter-based approach is faster. The switch is made for block sizes strictly
	// larger than this threshold.
	static const size_t EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD = 32;

	// How many full blocks can be expected for a single explicit producer? This should
	// reflect that number's maximum for optimal performance. Must be a power of 2.
	static const size_t EXPLICIT_INITIAL_INDEX_SIZE = 32;

	// Controls the number of items that an explicit consumer (i.e. one with a token)
	// must consume before it causes all consumers to rotate and move on to the next
	// internal queue.
	static const std::uint32_t EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE = 256;

	// The maximum number of elements (inclusive) that can be enqueued to a sub-queue.
	// Enqueue operations that would cause this limit to be surpassed will fail. Note
	// that this limit is enforced at the block level (for performance reasons), i.e.
	// it's rounded up to the nearest block size.
	static const size_t MAX_SUBQUEUE_SIZE = details::const_numeric_max<size_t>::value;

	// Memory allocation can be customized if needed.
	// malloc should return nullptr on failure, and handle alignment like std::malloc.
#if defined(malloc) || defined(free)
	// Gah, this is 2015, stop defining macros that break standard code already!
	// Work around malloc/free being special macros:
	static inline void* WORKAROUND_malloc(size_t size) { return malloc(size); }
	static inline void WORKAROUND_free(void* ptr) { return free(ptr); }
	static inline void* (malloc)(size_t size) { return WORKAROUND_malloc(size); }
	static inline void (free)(void* ptr) { return WORKAROUND_free(ptr); }
#else
	static inline void* malloc(size_t size) { return tracy::tracy_malloc(size); }
	static inline void free(void* ptr) { return tracy::tracy_free(ptr); }
#endif
};

// When producing or consuming many elements, the most efficient way is to:
//    1) Use one of the bulk-operation methods of the queue with a token
//    2) Failing that, use the bulk-operation methods without a token
//    3) Failing that, create a token and use that with the single-item methods
//    4) Failing that, use the single-parameter methods of the queue
// Having said that, don't create tokens willy-nilly -- ideally there should be
// a maximum of one token per thread (of each kind).
struct ProducerToken;
struct ConsumerToken;

template<typename T, typename Traits> class ConcurrentQueue;

namespace details
{
	struct ConcurrentQueueProducerTypelessBase
	{
		ConcurrentQueueProducerTypelessBase* next;
		std::atomic<bool> inactive;
		ProducerToken* token;
        uint64_t threadId;

		ConcurrentQueueProducerTypelessBase()
			: next(nullptr), inactive(false), token(nullptr), threadId(0)
		{
		}
	};

	template<typename T>
	static inline bool circular_less_than(T a, T b)
	{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4554)
#endif
		static_assert(std::is_integral<T>::value && !std::numeric_limits<T>::is_signed, "circular_less_than is intended to be used only with unsigned integer types");
		return static_cast<T>(a - b) > static_cast<T>(static_cast<T>(1) << static_cast<T>(sizeof(T) * CHAR_BIT - 1));
#ifdef _MSC_VER
#pragma warning(pop)
#endif
	}

	template<typename U>
	static inline char* align_for(char* ptr)
	{
		const std::size_t alignment = std::alignment_of<U>::value;
		return ptr + (alignment - (reinterpret_cast<std::uintptr_t>(ptr) % alignment)) % alignment;
	}

	template<typename T>
	static inline T ceil_to_pow_2(T x)
	{
		static_assert(std::is_integral<T>::value && !std::numeric_limits<T>::is_signed, "ceil_to_pow_2 is intended to be used only with unsigned integer types");

		// Adapted from http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		--x;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		for (std::size_t i = 1; i < sizeof(T); i <<= 1) {
			x |= x >> (i << 3);
		}
		++x;
		return x;
	}

	template<typename T>
	static inline void swap_relaxed(std::atomic<T>& left, std::atomic<T>& right)
	{
		T temp = std::move(left.load(std::memory_order_relaxed));
		left.store(std::move(right.load(std::memory_order_relaxed)), std::memory_order_relaxed);
		right.store(std::move(temp), std::memory_order_relaxed);
	}

	template<typename T>
	static inline T const& nomove(T const& x)
	{
		return x;
	}

	template<bool Enable>
	struct nomove_if
	{
		template<typename T>
		static inline T const& eval(T const& x)
		{
			return x;
		}
	};

	template<>
	struct nomove_if<false>
	{
		template<typename U>
		static inline auto eval(U&& x)
			-> decltype(std::forward<U>(x))
		{
			return std::forward<U>(x);
		}
	};

	template<typename It>
	static inline auto deref_noexcept(It& it) noexcept -> decltype(*it)
	{
		return *it;
	}

#if defined(__clang__) || !defined(__GNUC__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
	template<typename T> struct is_trivially_destructible : std::is_trivially_destructible<T> { };
#else
	template<typename T> struct is_trivially_destructible : std::has_trivial_destructor<T> { };
#endif

	template<typename T> struct static_is_lock_free_num { enum { value = 0 }; };
	template<> struct static_is_lock_free_num<signed char> { enum { value = ATOMIC_CHAR_LOCK_FREE }; };
	template<> struct static_is_lock_free_num<short> { enum { value = ATOMIC_SHORT_LOCK_FREE }; };
	template<> struct static_is_lock_free_num<int> { enum { value = ATOMIC_INT_LOCK_FREE }; };
	template<> struct static_is_lock_free_num<long> { enum { value = ATOMIC_LONG_LOCK_FREE }; };
	template<> struct static_is_lock_free_num<long long> { enum { value = ATOMIC_LLONG_LOCK_FREE }; };
	template<typename T> struct static_is_lock_free : static_is_lock_free_num<typename std::make_signed<T>::type> {  };
	template<> struct static_is_lock_free<bool> { enum { value = ATOMIC_BOOL_LOCK_FREE }; };
	template<typename U> struct static_is_lock_free<U*> { enum { value = ATOMIC_POINTER_LOCK_FREE }; };
}

struct ProducerToken
{
	template<typename T, typename Traits>
	explicit ProducerToken(ConcurrentQueue<T, Traits>& queue);

	ProducerToken(ProducerToken&& other) noexcept
		: producer(other.producer)
	{
		other.producer = nullptr;
		if (producer != nullptr) {
			producer->token = this;
		}
	}

	inline ProducerToken& operator=(ProducerToken&& other) noexcept
	{
		swap(other);
		return *this;
	}

	void swap(ProducerToken& other) noexcept
	{
		std::swap(producer, other.producer);
		if (producer != nullptr) {
			producer->token = this;
		}
		if (other.producer != nullptr) {
			other.producer->token = &other;
		}
	}

	// A token is always valid unless:
	//     1) Memory allocation failed during construction
	//     2) It was moved via the move constructor
	//        (Note: assignment does a swap, leaving both potentially valid)
	//     3) The associated queue was destroyed
	// Note that if valid() returns true, that only indicates
	// that the token is valid for use with a specific queue,
	// but not which one; that's up to the user to track.
	inline bool valid() const { return producer != nullptr; }

	~ProducerToken()
	{
		if (producer != nullptr) {
			producer->token = nullptr;
			producer->inactive.store(true, std::memory_order_release);
		}
	}

	// Disable copying and assignment
	ProducerToken(ProducerToken const&) = delete;
	ProducerToken& operator=(ProducerToken const&) = delete;

private:
	template<typename T, typename Traits> friend class ConcurrentQueue;

protected:
	details::ConcurrentQueueProducerTypelessBase* producer;
};

struct ConsumerToken
{
	template<typename T, typename Traits>
	explicit ConsumerToken(ConcurrentQueue<T, Traits>& q);

	ConsumerToken(ConsumerToken&& other) noexcept
		: initialOffset(other.initialOffset), lastKnownGlobalOffset(other.lastKnownGlobalOffset), itemsConsumedFromCurrent(other.itemsConsumedFromCurrent), currentProducer(other.currentProducer), desiredProducer(other.desiredProducer)
	{
	}

	inline ConsumerToken& operator=(ConsumerToken&& other) noexcept
	{
		swap(other);
		return *this;
	}

	void swap(ConsumerToken& other) noexcept
	{
		std::swap(initialOffset, other.initialOffset);
		std::swap(lastKnownGlobalOffset, other.lastKnownGlobalOffset);
		std::swap(itemsConsumedFromCurrent, other.itemsConsumedFromCurrent);
		std::swap(currentProducer, other.currentProducer);
		std::swap(desiredProducer, other.desiredProducer);
	}

	// Disable copying and assignment
	ConsumerToken(ConsumerToken const&) = delete;
	ConsumerToken& operator=(ConsumerToken const&) = delete;

private:
	template<typename T, typename Traits> friend class ConcurrentQueue;

private: // but shared with ConcurrentQueue
	std::uint32_t initialOffset;
	std::uint32_t lastKnownGlobalOffset;
	std::uint32_t itemsConsumedFromCurrent;
	details::ConcurrentQueueProducerTypelessBase* currentProducer;
	details::ConcurrentQueueProducerTypelessBase* desiredProducer;
};

template<typename T, typename Traits = ConcurrentQueueDefaultTraits>
class ConcurrentQueue
{
public:
    struct ExplicitProducer;

	typedef moodycamel::ProducerToken producer_token_t;
	typedef moodycamel::ConsumerToken consumer_token_t;

	typedef typename Traits::index_t index_t;
	typedef typename Traits::size_t size_t;

	static const size_t BLOCK_SIZE = static_cast<size_t>(Traits::BLOCK_SIZE);
	static const size_t EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD = static_cast<size_t>(Traits::EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD);
	static const size_t EXPLICIT_INITIAL_INDEX_SIZE = static_cast<size_t>(Traits::EXPLICIT_INITIAL_INDEX_SIZE);
	static const std::uint32_t EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE = static_cast<std::uint32_t>(Traits::EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE);
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4307)		// + integral constant overflow (that's what the ternary expression is for!)
#pragma warning(disable: 4309)		// static_cast: Truncation of constant value
#endif
	static const size_t MAX_SUBQUEUE_SIZE = (details::const_numeric_max<size_t>::value - static_cast<size_t>(Traits::MAX_SUBQUEUE_SIZE) < BLOCK_SIZE) ? details::const_numeric_max<size_t>::value : ((static_cast<size_t>(Traits::MAX_SUBQUEUE_SIZE) + (BLOCK_SIZE - 1)) / BLOCK_SIZE * BLOCK_SIZE);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

	static_assert(!std::numeric_limits<size_t>::is_signed && std::is_integral<size_t>::value, "Traits::size_t must be an unsigned integral type");
	static_assert(!std::numeric_limits<index_t>::is_signed && std::is_integral<index_t>::value, "Traits::index_t must be an unsigned integral type");
	static_assert(sizeof(index_t) >= sizeof(size_t), "Traits::index_t must be at least as wide as Traits::size_t");
	static_assert((BLOCK_SIZE > 1) && !(BLOCK_SIZE & (BLOCK_SIZE - 1)), "Traits::BLOCK_SIZE must be a power of 2 (and at least 2)");
	static_assert((EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD > 1) && !(EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD & (EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD - 1)), "Traits::EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD must be a power of 2 (and greater than 1)");
	static_assert((EXPLICIT_INITIAL_INDEX_SIZE > 1) && !(EXPLICIT_INITIAL_INDEX_SIZE & (EXPLICIT_INITIAL_INDEX_SIZE - 1)), "Traits::EXPLICIT_INITIAL_INDEX_SIZE must be a power of 2 (and greater than 1)");

public:
	// Creates a queue with at least `capacity` element slots; note that the
	// actual number of elements that can be inserted without additional memory
	// allocation depends on the number of producers and the block size (e.g. if
	// the block size is equal to `capacity`, only a single block will be allocated
	// up-front, which means only a single producer will be able to enqueue elements
	// without an extra allocation -- blocks aren't shared between producers).
	// This method is not thread safe -- it is up to the user to ensure that the
	// queue is fully constructed before it starts being used by other threads (this
	// includes making the memory effects of construction visible, possibly with a
	// memory barrier).
	explicit ConcurrentQueue(size_t capacity = 6 * BLOCK_SIZE)
		: producerListTail(nullptr),
		producerCount(0),
		initialBlockPoolIndex(0),
		nextExplicitConsumerId(0),
		globalExplicitConsumerOffset(0)
	{
		populate_initial_block_list(capacity / BLOCK_SIZE + ((capacity & (BLOCK_SIZE - 1)) == 0 ? 0 : 1));
	}

	// Computes the correct amount of pre-allocated blocks for you based
	// on the minimum number of elements you want available at any given
	// time, and the maximum concurrent number of each type of producer.
	ConcurrentQueue(size_t minCapacity, size_t maxExplicitProducers)
		: producerListTail(nullptr),
		producerCount(0),
		initialBlockPoolIndex(0),
		nextExplicitConsumerId(0),
		globalExplicitConsumerOffset(0)
	{
		size_t blocks = (((minCapacity + BLOCK_SIZE - 1) / BLOCK_SIZE) - 1) * (maxExplicitProducers + 1) + 2 * (maxExplicitProducers);
		populate_initial_block_list(blocks);
	}

	// Note: The queue should not be accessed concurrently while it's
	// being deleted. It's up to the user to synchronize this.
	// This method is not thread safe.
	~ConcurrentQueue()
	{
		// Destroy producers
		auto ptr = producerListTail.load(std::memory_order_relaxed);
		while (ptr != nullptr) {
			auto next = ptr->next_prod();
			if (ptr->token != nullptr) {
				ptr->token->producer = nullptr;
			}
			destroy(ptr);
			ptr = next;
		}

		// Destroy global free list
		auto block = freeList.head_unsafe();
		while (block != nullptr) {
			auto next = block->freeListNext.load(std::memory_order_relaxed);
			if (block->dynamicallyAllocated) {
				destroy(block);
			}
			block = next;
		}

		// Destroy initial free list
		destroy_array(initialBlockPool, initialBlockPoolSize);
	}

	// Disable copying and copy assignment
	ConcurrentQueue(ConcurrentQueue const&) = delete;
    ConcurrentQueue(ConcurrentQueue&& other) = delete;
	ConcurrentQueue& operator=(ConcurrentQueue const&) = delete;
    ConcurrentQueue& operator=(ConcurrentQueue&& other) = delete;

public:
    tracy_force_inline T* enqueue_begin(producer_token_t const& token, index_t& currentTailIndex)
    {
        return static_cast<ExplicitProducer*>(token.producer)->ConcurrentQueue::ExplicitProducer::enqueue_begin(currentTailIndex);
    }

	template<class NotifyThread, class ProcessData>
    size_t try_dequeue_bulk_single(consumer_token_t& token, NotifyThread notifyThread, ProcessData processData )
    {
        if (token.desiredProducer == nullptr || token.lastKnownGlobalOffset != globalExplicitConsumerOffset.load(std::memory_order_relaxed)) {
            if (!update_current_producer_after_rotation(token)) {
                return 0;
            }
        }

        size_t count = static_cast<ProducerBase*>(token.currentProducer)->dequeue_bulk(notifyThread, processData);
        token.itemsConsumedFromCurrent += static_cast<std::uint32_t>(count);

        auto tail = producerListTail.load(std::memory_order_acquire);
        auto ptr = static_cast<ProducerBase*>(token.currentProducer)->next_prod();
        if (ptr == nullptr) {
            ptr = tail;
        }
        if( count == 0 )
        {
            while (ptr != static_cast<ProducerBase*>(token.currentProducer)) {
                auto dequeued = ptr->dequeue_bulk(notifyThread, processData);
                if (dequeued != 0) {
                    token.currentProducer = ptr;
                    token.itemsConsumedFromCurrent = static_cast<std::uint32_t>(dequeued);
                    return dequeued;
                }
                ptr = ptr->next_prod();
                if (ptr == nullptr) {
                    ptr = tail;
                }
            }
            return 0;
        }
        else
        {
            token.currentProducer = ptr;
            token.itemsConsumedFromCurrent = 0;
            return count;
        }
    }

	// Returns an estimate of the total number of elements currently in the queue. This
	// estimate is only accurate if the queue has completely stabilized before it is called
	// (i.e. all enqueue and dequeue operations have completed and their memory effects are
	// visible on the calling thread, and no further operations start while this method is
	// being called).
	// Thread-safe.
	size_t size_approx() const
	{
		size_t size = 0;
		for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
			size += ptr->size_approx();
		}
		return size;
	}

	// Returns true if the underlying atomic variables used by
	// the queue are lock-free (they should be on most platforms).
	// Thread-safe.
	static bool is_lock_free()
	{
		return
			details::static_is_lock_free<bool>::value == 2 &&
			details::static_is_lock_free<size_t>::value == 2 &&
			details::static_is_lock_free<std::uint32_t>::value == 2 &&
			details::static_is_lock_free<index_t>::value == 2 &&
			details::static_is_lock_free<void*>::value == 2;
	}

private:
	friend struct ProducerToken;
	friend struct ConsumerToken;
	friend struct ExplicitProducer;

	///////////////////////////////
	// Queue methods
	///////////////////////////////

	inline bool update_current_producer_after_rotation(consumer_token_t& token)
	{
		// Ah, there's been a rotation, figure out where we should be!
		auto tail = producerListTail.load(std::memory_order_acquire);
		if (token.desiredProducer == nullptr && tail == nullptr) {
			return false;
		}
		auto prodCount = producerCount.load(std::memory_order_relaxed);
		auto globalOffset = globalExplicitConsumerOffset.load(std::memory_order_relaxed);
		if (details::cqUnlikely(token.desiredProducer == nullptr)) {
			// Aha, first time we're dequeueing anything.
			// Figure out our local position
			// Note: offset is from start, not end, but we're traversing from end -- subtract from count first
			std::uint32_t offset = prodCount - 1 - (token.initialOffset % prodCount);
			token.desiredProducer = tail;
			for (std::uint32_t i = 0; i != offset; ++i) {
				token.desiredProducer = static_cast<ProducerBase*>(token.desiredProducer)->next_prod();
				if (token.desiredProducer == nullptr) {
					token.desiredProducer = tail;
				}
			}
		}

		std::uint32_t delta = globalOffset - token.lastKnownGlobalOffset;
		if (delta >= prodCount) {
			delta = delta % prodCount;
		}
		for (std::uint32_t i = 0; i != delta; ++i) {
			token.desiredProducer = static_cast<ProducerBase*>(token.desiredProducer)->next_prod();
			if (token.desiredProducer == nullptr) {
				token.desiredProducer = tail;
			}
		}

		token.lastKnownGlobalOffset = globalOffset;
		token.currentProducer = token.desiredProducer;
		token.itemsConsumedFromCurrent = 0;
		return true;
	}

	///////////////////////////
	// Free list
	///////////////////////////

	template <typename N>
	struct FreeListNode
	{
		FreeListNode() : freeListRefs(0), freeListNext(nullptr) { }

		std::atomic<std::uint32_t> freeListRefs;
		std::atomic<N*> freeListNext;
	};

	// A simple CAS-based lock-free free list. Not the fastest thing in the world under heavy contention, but
	// simple and correct (assuming nodes are never freed until after the free list is destroyed), and fairly
	// speedy under low contention.
	template<typename N>		// N must inherit FreeListNode or have the same fields (and initialization of them)
	struct FreeList
	{
		FreeList() : freeListHead(nullptr) { }
		FreeList(FreeList&& other) : freeListHead(other.freeListHead.load(std::memory_order_relaxed)) { other.freeListHead.store(nullptr, std::memory_order_relaxed); }
		void swap(FreeList& other) { details::swap_relaxed(freeListHead, other.freeListHead); }

		FreeList(FreeList const&) = delete;
		FreeList& operator=(FreeList const&) = delete;

		inline void add(N* node)
		{
			// We know that the should-be-on-freelist bit is 0 at this point, so it's safe to
			// set it using a fetch_add
			if (node->freeListRefs.fetch_add(SHOULD_BE_ON_FREELIST, std::memory_order_acq_rel) == 0) {
				// Oh look! We were the last ones referencing this node, and we know
				// we want to add it to the free list, so let's do it!
		 		add_knowing_refcount_is_zero(node);
			}
		}

		inline N* try_get()
		{
			auto head = freeListHead.load(std::memory_order_acquire);
			while (head != nullptr) {
				auto prevHead = head;
				auto refs = head->freeListRefs.load(std::memory_order_relaxed);
				if ((refs & REFS_MASK) == 0 || !head->freeListRefs.compare_exchange_strong(refs, refs + 1, std::memory_order_acquire, std::memory_order_relaxed)) {
					head = freeListHead.load(std::memory_order_acquire);
					continue;
				}

				// Good, reference count has been incremented (it wasn't at zero), which means we can read the
				// next and not worry about it changing between now and the time we do the CAS
				auto next = head->freeListNext.load(std::memory_order_relaxed);
				if (freeListHead.compare_exchange_strong(head, next, std::memory_order_acquire, std::memory_order_relaxed)) {
					// Yay, got the node. This means it was on the list, which means shouldBeOnFreeList must be false no
					// matter the refcount (because nobody else knows it's been taken off yet, it can't have been put back on).
					assert((head->freeListRefs.load(std::memory_order_relaxed) & SHOULD_BE_ON_FREELIST) == 0);

					// Decrease refcount twice, once for our ref, and once for the list's ref
					head->freeListRefs.fetch_sub(2, std::memory_order_release);
					return head;
				}

				// OK, the head must have changed on us, but we still need to decrease the refcount we increased.
				// Note that we don't need to release any memory effects, but we do need to ensure that the reference
				// count decrement happens-after the CAS on the head.
				refs = prevHead->freeListRefs.fetch_sub(1, std::memory_order_acq_rel);
				if (refs == SHOULD_BE_ON_FREELIST + 1) {
					add_knowing_refcount_is_zero(prevHead);
				}
			}

			return nullptr;
		}

		// Useful for traversing the list when there's no contention (e.g. to destroy remaining nodes)
		N* head_unsafe() const { return freeListHead.load(std::memory_order_relaxed); }

	private:
		inline void add_knowing_refcount_is_zero(N* node)
		{
			// Since the refcount is zero, and nobody can increase it once it's zero (except us, and we run
			// only one copy of this method per node at a time, i.e. the single thread case), then we know
			// we can safely change the next pointer of the node; however, once the refcount is back above
			// zero, then other threads could increase it (happens under heavy contention, when the refcount
			// goes to zero in between a load and a refcount increment of a node in try_get, then back up to
			// something non-zero, then the refcount increment is done by the other thread) -- so, if the CAS
			// to add the node to the actual list fails, decrease the refcount and leave the add operation to
			// the next thread who puts the refcount back at zero (which could be us, hence the loop).
			auto head = freeListHead.load(std::memory_order_relaxed);
			while (true) {
				node->freeListNext.store(head, std::memory_order_relaxed);
				node->freeListRefs.store(1, std::memory_order_release);
				if (!freeListHead.compare_exchange_strong(head, node, std::memory_order_release, std::memory_order_relaxed)) {
					// Hmm, the add failed, but we can only try again when the refcount goes back to zero
					if (node->freeListRefs.fetch_add(SHOULD_BE_ON_FREELIST - 1, std::memory_order_release) == 1) {
						continue;
					}
				}
				return;
			}
		}

	private:
		// Implemented like a stack, but where node order doesn't matter (nodes are inserted out of order under contention)
		std::atomic<N*> freeListHead;

	static const std::uint32_t REFS_MASK = 0x7FFFFFFF;
	static const std::uint32_t SHOULD_BE_ON_FREELIST = 0x80000000;
	};

	///////////////////////////
	// Block
	///////////////////////////

	struct Block
	{
		Block()
			: next(nullptr), elementsCompletelyDequeued(0), freeListRefs(0), freeListNext(nullptr), shouldBeOnFreeList(false), dynamicallyAllocated(true)
		{
		}

		inline bool is_empty() const
		{
			if (compile_time_condition<BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD>::value) {
				// Check flags
				for (size_t i = 0; i < BLOCK_SIZE; ++i) {
					if (!emptyFlags[i].load(std::memory_order_relaxed)) {
						return false;
					}
				}

				// Aha, empty; make sure we have all other memory effects that happened before the empty flags were set
				std::atomic_thread_fence(std::memory_order_acquire);
				return true;
			}
			else {
				// Check counter
				if (elementsCompletelyDequeued.load(std::memory_order_relaxed) == BLOCK_SIZE) {
					std::atomic_thread_fence(std::memory_order_acquire);
					return true;
				}
				assert(elementsCompletelyDequeued.load(std::memory_order_relaxed) <= BLOCK_SIZE);
				return false;
			}
		}

		// Returns true if the block is now empty (does not apply in explicit context)
		inline bool set_empty(index_t i)
		{
			if (BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
				// Set flag
				assert(!emptyFlags[BLOCK_SIZE - 1 - static_cast<size_t>(i & static_cast<index_t>(BLOCK_SIZE - 1))].load(std::memory_order_relaxed));
				emptyFlags[BLOCK_SIZE - 1 - static_cast<size_t>(i & static_cast<index_t>(BLOCK_SIZE - 1))].store(true, std::memory_order_release);
				return false;
			}
			else {
				// Increment counter
				auto prevVal = elementsCompletelyDequeued.fetch_add(1, std::memory_order_release);
				assert(prevVal < BLOCK_SIZE);
				return prevVal == BLOCK_SIZE - 1;
			}
		}

		// Sets multiple contiguous item statuses to 'empty' (assumes no wrapping and count > 0).
		// Returns true if the block is now empty (does not apply in explicit context).
		inline bool set_many_empty(index_t i, size_t count)
		{
			if (compile_time_condition<BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD>::value) {
				// Set flags
				std::atomic_thread_fence(std::memory_order_release);
				i = BLOCK_SIZE - 1 - static_cast<size_t>(i & static_cast<index_t>(BLOCK_SIZE - 1)) - count + 1;
				for (size_t j = 0; j != count; ++j) {
					assert(!emptyFlags[i + j].load(std::memory_order_relaxed));
					emptyFlags[i + j].store(true, std::memory_order_relaxed);
				}
				return false;
			}
			else {
				// Increment counter
				auto prevVal = elementsCompletelyDequeued.fetch_add(count, std::memory_order_release);
				assert(prevVal + count <= BLOCK_SIZE);
				return prevVal + count == BLOCK_SIZE;
			}
		}

		inline void set_all_empty()
		{
			if (BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
				// Set all flags
				for (size_t i = 0; i != BLOCK_SIZE; ++i) {
					emptyFlags[i].store(true, std::memory_order_relaxed);
				}
			}
			else {
				// Reset counter
				elementsCompletelyDequeued.store(BLOCK_SIZE, std::memory_order_relaxed);
			}
		}

		inline void reset_empty()
		{
			if (compile_time_condition<BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD>::value) {
				// Reset flags
				for (size_t i = 0; i != BLOCK_SIZE; ++i) {
					emptyFlags[i].store(false, std::memory_order_relaxed);
				}
			}
			else {
				// Reset counter
				elementsCompletelyDequeued.store(0, std::memory_order_relaxed);
			}
		}

		inline T* operator[](index_t idx) noexcept { return static_cast<T*>(static_cast<void*>(elements)) + static_cast<size_t>(idx & static_cast<index_t>(BLOCK_SIZE - 1)); }
		inline T const* operator[](index_t idx) const noexcept { return static_cast<T const*>(static_cast<void const*>(elements)) + static_cast<size_t>(idx & static_cast<index_t>(BLOCK_SIZE - 1)); }

	private:
		// IMPORTANT: This must be the first member in Block, so that if T depends on the alignment of
		// addresses returned by malloc, that alignment will be preserved. Apparently clang actually
		// generates code that uses this assumption for AVX instructions in some cases. Ideally, we
		// should also align Block to the alignment of T in case it's higher than malloc's 16-byte
		// alignment, but this is hard to do in a cross-platform way. Assert for this case:
		static_assert(std::alignment_of<T>::value <= std::alignment_of<details::max_align_t>::value, "The queue does not support super-aligned types at this time");
		// Additionally, we need the alignment of Block itself to be a multiple of max_align_t since
		// otherwise the appropriate padding will not be added at the end of Block in order to make
		// arrays of Blocks all be properly aligned (not just the first one). We use a union to force
		// this.
		union {
			char elements[sizeof(T) * BLOCK_SIZE];
			details::max_align_t dummy;
		};
	public:
		Block* next;
		std::atomic<size_t> elementsCompletelyDequeued;
		std::atomic<bool> emptyFlags[BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD ? BLOCK_SIZE : 1];
	public:
		std::atomic<std::uint32_t> freeListRefs;
		std::atomic<Block*> freeListNext;
		std::atomic<bool> shouldBeOnFreeList;
		bool dynamicallyAllocated;		// Perhaps a better name for this would be 'isNotPartOfInitialBlockPool'
	};
	static_assert(std::alignment_of<Block>::value >= std::alignment_of<details::max_align_t>::value, "Internal error: Blocks must be at least as aligned as the type they are wrapping");

	///////////////////////////
	// Producer base
	///////////////////////////

	struct ProducerBase : public details::ConcurrentQueueProducerTypelessBase
	{
		ProducerBase(ConcurrentQueue* parent_) :
			tailIndex(0),
			headIndex(0),
			dequeueOptimisticCount(0),
			dequeueOvercommit(0),
			tailBlock(nullptr),
			parent(parent_)
		{
		}

		virtual ~ProducerBase() { };

		template<class NotifyThread, class ProcessData>
		inline size_t dequeue_bulk(NotifyThread notifyThread, ProcessData processData)
		{
			return static_cast<ExplicitProducer*>(this)->dequeue_bulk(notifyThread, processData);
		}

		inline ProducerBase* next_prod() const { return static_cast<ProducerBase*>(next); }

		inline size_t size_approx() const
		{
			auto tail = tailIndex.load(std::memory_order_relaxed);
			auto head = headIndex.load(std::memory_order_relaxed);
			return details::circular_less_than(head, tail) ? static_cast<size_t>(tail - head) : 0;
		}

		inline index_t getTail() const { return tailIndex.load(std::memory_order_relaxed); }
	protected:
		std::atomic<index_t> tailIndex;		// Where to enqueue to next
		std::atomic<index_t> headIndex;		// Where to dequeue from next

		std::atomic<index_t> dequeueOptimisticCount;
		std::atomic<index_t> dequeueOvercommit;

		Block* tailBlock;

	public:
		ConcurrentQueue* parent;
	};

    public:
	///////////////////////////
	// Explicit queue
	///////////////////////////
	struct ExplicitProducer : public ProducerBase
	{
		explicit ExplicitProducer(ConcurrentQueue* _parent) :
			ProducerBase(_parent),
			blockIndex(nullptr),
			pr_blockIndexSlotsUsed(0),
			pr_blockIndexSize(EXPLICIT_INITIAL_INDEX_SIZE >> 1),
			pr_blockIndexFront(0),
			pr_blockIndexEntries(nullptr),
			pr_blockIndexRaw(nullptr)
		{
			size_t poolBasedIndexSize = details::ceil_to_pow_2(_parent->initialBlockPoolSize) >> 1;
			if (poolBasedIndexSize > pr_blockIndexSize) {
				pr_blockIndexSize = poolBasedIndexSize;
			}

			new_block_index(0);		// This creates an index with double the number of current entries, i.e. EXPLICIT_INITIAL_INDEX_SIZE
		}

		~ExplicitProducer()
		{
			// Destruct any elements not yet dequeued.
			// Since we're in the destructor, we can assume all elements
			// are either completely dequeued or completely not (no halfways).
			if (this->tailBlock != nullptr) {		// Note this means there must be a block index too
				// First find the block that's partially dequeued, if any
				Block* halfDequeuedBlock = nullptr;
				if ((this->headIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1)) != 0) {
					// The head's not on a block boundary, meaning a block somewhere is partially dequeued
					// (or the head block is the tail block and was fully dequeued, but the head/tail are still not on a boundary)
					size_t i = (pr_blockIndexFront - pr_blockIndexSlotsUsed) & (pr_blockIndexSize - 1);
					while (details::circular_less_than<index_t>(pr_blockIndexEntries[i].base + BLOCK_SIZE, this->headIndex.load(std::memory_order_relaxed))) {
						i = (i + 1) & (pr_blockIndexSize - 1);
					}
					assert(details::circular_less_than<index_t>(pr_blockIndexEntries[i].base, this->headIndex.load(std::memory_order_relaxed)));
					halfDequeuedBlock = pr_blockIndexEntries[i].block;
				}

				// Start at the head block (note the first line in the loop gives us the head from the tail on the first iteration)
				auto block = this->tailBlock;
				do {
					block = block->next;
					if (block->ConcurrentQueue::Block::is_empty()) {
						continue;
					}

					size_t i = 0;	// Offset into block
					if (block == halfDequeuedBlock) {
						i = static_cast<size_t>(this->headIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1));
					}

					// Walk through all the items in the block; if this is the tail block, we need to stop when we reach the tail index
					auto lastValidIndex = (this->tailIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1)) == 0 ? BLOCK_SIZE : static_cast<size_t>(this->tailIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1));
					while (i != BLOCK_SIZE && (block != this->tailBlock || i != lastValidIndex)) {
						(*block)[i++]->~T();
					}
				} while (block != this->tailBlock);
			}

			// Destroy all blocks that we own
			if (this->tailBlock != nullptr) {
				auto block = this->tailBlock;
				do {
					auto nextBlock = block->next;
					if (block->dynamicallyAllocated) {
						destroy(block);
					}
					else {
						this->parent->add_block_to_free_list(block);
					}
					block = nextBlock;
				} while (block != this->tailBlock);
			}

			// Destroy the block indices
			auto header = static_cast<BlockIndexHeader*>(pr_blockIndexRaw);
			while (header != nullptr) {
				auto prev = static_cast<BlockIndexHeader*>(header->prev);
				header->~BlockIndexHeader();
				(Traits::free)(header);
				header = prev;
			}
		}

        inline void enqueue_begin_alloc(index_t currentTailIndex)
        {
            // We reached the end of a block, start a new one
            if (this->tailBlock != nullptr && this->tailBlock->next->ConcurrentQueue::Block::is_empty()) {
                // We can re-use the block ahead of us, it's empty!
                this->tailBlock = this->tailBlock->next;
                this->tailBlock->ConcurrentQueue::Block::reset_empty();

                // We'll put the block on the block index (guaranteed to be room since we're conceptually removing the
                // last block from it first -- except instead of removing then adding, we can just overwrite).
                // Note that there must be a valid block index here, since even if allocation failed in the ctor,
                // it would have been re-attempted when adding the first block to the queue; since there is such
                // a block, a block index must have been successfully allocated.
            }
            else {
                // We're going to need a new block; check that the block index has room
                if (pr_blockIndexRaw == nullptr || pr_blockIndexSlotsUsed == pr_blockIndexSize) {
                    // Hmm, the circular block index is already full -- we'll need
                    // to allocate a new index. Note pr_blockIndexRaw can only be nullptr if
                    // the initial allocation failed in the constructor.
                    new_block_index(pr_blockIndexSlotsUsed);
                }

                // Insert a new block in the circular linked list
                auto newBlock = this->parent->ConcurrentQueue::requisition_block();
                newBlock->ConcurrentQueue::Block::reset_empty();
                if (this->tailBlock == nullptr) {
                    newBlock->next = newBlock;
                }
                else {
                    newBlock->next = this->tailBlock->next;
                    this->tailBlock->next = newBlock;
                }
                this->tailBlock = newBlock;
                ++pr_blockIndexSlotsUsed;
            }

            // Add block to block index
            auto& entry = blockIndex.load(std::memory_order_relaxed)->entries[pr_blockIndexFront];
            entry.base = currentTailIndex;
            entry.block = this->tailBlock;
            blockIndex.load(std::memory_order_relaxed)->front.store(pr_blockIndexFront, std::memory_order_release);
            pr_blockIndexFront = (pr_blockIndexFront + 1) & (pr_blockIndexSize - 1);
        }

        tracy_force_inline T* enqueue_begin(index_t& currentTailIndex)
        {
            currentTailIndex = this->tailIndex.load(std::memory_order_relaxed);
            if (details::cqUnlikely((currentTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0)) {
                this->enqueue_begin_alloc(currentTailIndex);
            }
            return (*this->tailBlock)[currentTailIndex];
        }

        tracy_force_inline std::atomic<index_t>& get_tail_index()
        {
            return this->tailIndex;
        }

		template<class NotifyThread, class ProcessData>
		size_t dequeue_bulk(NotifyThread notifyThread, ProcessData processData)
		{
			auto tail = this->tailIndex.load(std::memory_order_relaxed);
			auto overcommit = this->dequeueOvercommit.load(std::memory_order_relaxed);
			auto desiredCount = static_cast<size_t>(tail - (this->dequeueOptimisticCount.load(std::memory_order_relaxed) - overcommit));
			if (details::circular_less_than<size_t>(0, desiredCount)) {
				desiredCount = desiredCount < 8192 ? desiredCount : 8192;
				std::atomic_thread_fence(std::memory_order_acquire);

				auto myDequeueCount = this->dequeueOptimisticCount.fetch_add(desiredCount, std::memory_order_relaxed);
				assert(overcommit <= myDequeueCount);

				tail = this->tailIndex.load(std::memory_order_acquire);
				auto actualCount = static_cast<size_t>(tail - (myDequeueCount - overcommit));
				if (details::circular_less_than<size_t>(0, actualCount)) {
					actualCount = desiredCount < actualCount ? desiredCount : actualCount;
					if (actualCount < desiredCount) {
						this->dequeueOvercommit.fetch_add(desiredCount - actualCount, std::memory_order_release);
					}

					// Get the first index. Note that since there's guaranteed to be at least actualCount elements, this
					// will never exceed tail.
					auto firstIndex = this->headIndex.fetch_add(actualCount, std::memory_order_acq_rel);

					// Determine which block the first element is in
					auto localBlockIndex = blockIndex.load(std::memory_order_acquire);
					auto localBlockIndexHead = localBlockIndex->front.load(std::memory_order_acquire);

					auto headBase = localBlockIndex->entries[localBlockIndexHead].base;
					auto firstBlockBaseIndex = firstIndex & ~static_cast<index_t>(BLOCK_SIZE - 1);
					auto offset = static_cast<size_t>(static_cast<typename std::make_signed<index_t>::type>(firstBlockBaseIndex - headBase) / BLOCK_SIZE);
					auto indexIndex = (localBlockIndexHead + offset) & (localBlockIndex->size - 1);

					notifyThread( this->threadId );

					// Iterate the blocks and dequeue
					auto index = firstIndex;
					do {
						auto firstIndexInBlock = index;
						auto endIndex = (index & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
						endIndex = details::circular_less_than<index_t>(firstIndex + static_cast<index_t>(actualCount), endIndex) ? firstIndex + static_cast<index_t>(actualCount) : endIndex;
						auto block = localBlockIndex->entries[indexIndex].block;

						const auto sz = endIndex - index;
						processData( (*block)[index], sz );
						index += sz;

						block->ConcurrentQueue::Block::set_many_empty(firstIndexInBlock, static_cast<size_t>(endIndex - firstIndexInBlock));
						indexIndex = (indexIndex + 1) & (localBlockIndex->size - 1);
					} while (index != firstIndex + actualCount);

					return actualCount;
				}
				else {
					// Wasn't anything to dequeue after all; make the effective dequeue count eventually consistent
					this->dequeueOvercommit.fetch_add(desiredCount, std::memory_order_release);
				}
			}

			return 0;
		}

	private:
		struct BlockIndexEntry
		{
			index_t base;
			Block* block;
		};

		struct BlockIndexHeader
		{
			size_t size;
			std::atomic<size_t> front;		// Current slot (not next, like pr_blockIndexFront)
			BlockIndexEntry* entries;
			void* prev;
		};

		bool new_block_index(size_t numberOfFilledSlotsToExpose)
		{
			auto prevBlockSizeMask = pr_blockIndexSize - 1;

			// Create the new block
			pr_blockIndexSize <<= 1;
			auto newRawPtr = static_cast<char*>((Traits::malloc)(sizeof(BlockIndexHeader) + std::alignment_of<BlockIndexEntry>::value - 1 + sizeof(BlockIndexEntry) * pr_blockIndexSize));
			if (newRawPtr == nullptr) {
				pr_blockIndexSize >>= 1;		// Reset to allow graceful retry
				return false;
			}

			auto newBlockIndexEntries = reinterpret_cast<BlockIndexEntry*>(details::align_for<BlockIndexEntry>(newRawPtr + sizeof(BlockIndexHeader)));

			// Copy in all the old indices, if any
			size_t j = 0;
			if (pr_blockIndexSlotsUsed != 0) {
				auto i = (pr_blockIndexFront - pr_blockIndexSlotsUsed) & prevBlockSizeMask;
				do {
					newBlockIndexEntries[j++] = pr_blockIndexEntries[i];
					i = (i + 1) & prevBlockSizeMask;
				} while (i != pr_blockIndexFront);
			}

			// Update everything
			auto header = new (newRawPtr) BlockIndexHeader;
			header->size = pr_blockIndexSize;
			header->front.store(numberOfFilledSlotsToExpose - 1, std::memory_order_relaxed);
			header->entries = newBlockIndexEntries;
			header->prev = pr_blockIndexRaw;		// we link the new block to the old one so we can free it later

			pr_blockIndexFront = j;
			pr_blockIndexEntries = newBlockIndexEntries;
			pr_blockIndexRaw = newRawPtr;
			blockIndex.store(header, std::memory_order_release);

			return true;
		}

	private:
		std::atomic<BlockIndexHeader*> blockIndex;

		// To be used by producer only -- consumer must use the ones in referenced by blockIndex
		size_t pr_blockIndexSlotsUsed;
		size_t pr_blockIndexSize;
		size_t pr_blockIndexFront;		// Next slot (not current)
		BlockIndexEntry* pr_blockIndexEntries;
		void* pr_blockIndexRaw;
	};

    ExplicitProducer* get_explicit_producer(producer_token_t const& token)
    {
        return static_cast<ExplicitProducer*>(token.producer);
    }

    private:

	//////////////////////////////////
	// Block pool manipulation
	//////////////////////////////////

	void populate_initial_block_list(size_t blockCount)
	{
		initialBlockPoolSize = blockCount;
		if (initialBlockPoolSize == 0) {
			initialBlockPool = nullptr;
			return;
		}

		initialBlockPool = create_array<Block>(blockCount);
		if (initialBlockPool == nullptr) {
			initialBlockPoolSize = 0;
		}
		for (size_t i = 0; i < initialBlockPoolSize; ++i) {
			initialBlockPool[i].dynamicallyAllocated = false;
		}
	}

	inline Block* try_get_block_from_initial_pool()
	{
		if (initialBlockPoolIndex.load(std::memory_order_relaxed) >= initialBlockPoolSize) {
			return nullptr;
		}

		auto index = initialBlockPoolIndex.fetch_add(1, std::memory_order_relaxed);

		return index < initialBlockPoolSize ? (initialBlockPool + index) : nullptr;
	}

	inline void add_block_to_free_list(Block* block)
	{
		freeList.add(block);
	}

	inline void add_blocks_to_free_list(Block* block)
	{
		while (block != nullptr) {
			auto next = block->next;
			add_block_to_free_list(block);
			block = next;
		}
	}

	inline Block* try_get_block_from_free_list()
	{
		return freeList.try_get();
	}

	// Gets a free block from one of the memory pools, or allocates a new one (if applicable)
	Block* requisition_block()
	{
		auto block = try_get_block_from_initial_pool();
		if (block != nullptr) {
			return block;
		}

		block = try_get_block_from_free_list();
		if (block != nullptr) {
			return block;
		}

		return create<Block>();
	}

	//////////////////////////////////
	// Producer list manipulation
	//////////////////////////////////

	ProducerBase* recycle_or_create_producer()
	{
		bool recycled;
		return recycle_or_create_producer(recycled);
	}

    ProducerBase* recycle_or_create_producer(bool& recycled)
    {
        // Try to re-use one first
        for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
            if (ptr->inactive.load(std::memory_order_relaxed)) {
                if( ptr->size_approx() == 0 )
                {
                    bool expected = true;
                    if (ptr->inactive.compare_exchange_strong(expected, /* desired */ false, std::memory_order_acquire, std::memory_order_relaxed)) {
                        // We caught one! It's been marked as activated, the caller can have it
                        recycled = true;
                        return ptr;
                    }
                }
            }
        }

        recycled = false;
        return add_producer(static_cast<ProducerBase*>(create<ExplicitProducer>(this)));
    }

	ProducerBase* add_producer(ProducerBase* producer)
	{
		// Handle failed memory allocation
		if (producer == nullptr) {
			return nullptr;
		}

		producerCount.fetch_add(1, std::memory_order_relaxed);

		// Add it to the lock-free list
		auto prevTail = producerListTail.load(std::memory_order_relaxed);
		do {
			producer->next = prevTail;
		} while (!producerListTail.compare_exchange_weak(prevTail, producer, std::memory_order_release, std::memory_order_relaxed));

		return producer;
	}

	void reown_producers()
	{
		// After another instance is moved-into/swapped-with this one, all the
		// producers we stole still think their parents are the other queue.
		// So fix them up!
		for (auto ptr = producerListTail.load(std::memory_order_relaxed); ptr != nullptr; ptr = ptr->next_prod()) {
			ptr->parent = this;
		}
	}

	//////////////////////////////////
	// Utility functions
	//////////////////////////////////

	template<typename U>
	static inline U* create_array(size_t count)
	{
		assert(count > 0);
		return static_cast<U*>((Traits::malloc)(sizeof(U) * count));
	}

	template<typename U>
	static inline void destroy_array(U* p, size_t count)
	{
		((void)count);
		if (p != nullptr) {
			assert(count > 0);
			(Traits::free)(p);
		}
	}

	template<typename U>
	static inline U* create()
	{
		auto p = (Traits::malloc)(sizeof(U));
		return new (p) U;
	}

	template<typename U, typename A1>
	static inline U* create(A1&& a1)
	{
		auto p = (Traits::malloc)(sizeof(U));
		return new (p) U(std::forward<A1>(a1));
	}

	template<typename U>
	static inline void destroy(U* p)
	{
		if (p != nullptr) {
			p->~U();
		}
		(Traits::free)(p);
	}

private:
	std::atomic<ProducerBase*> producerListTail;
	std::atomic<std::uint32_t> producerCount;

	std::atomic<size_t> initialBlockPoolIndex;
	Block* initialBlockPool;
	size_t initialBlockPoolSize;

	FreeList<Block> freeList;

	std::atomic<std::uint32_t> nextExplicitConsumerId;
	std::atomic<std::uint32_t> globalExplicitConsumerOffset;
};

template<typename T, typename Traits>
ProducerToken::ProducerToken(ConcurrentQueue<T, Traits>& queue)
	: producer(queue.recycle_or_create_producer())
{
	if (producer != nullptr) {
		producer->token = this;
        producer->threadId = detail::GetThreadHandleImpl();
	}
}

template<typename T, typename Traits>
ConsumerToken::ConsumerToken(ConcurrentQueue<T, Traits>& queue)
	: itemsConsumedFromCurrent(0), currentProducer(nullptr), desiredProducer(nullptr)
{
	initialOffset = queue.nextExplicitConsumerId.fetch_add(1, std::memory_order_release);
	lastKnownGlobalOffset = static_cast<std::uint32_t>(-1);
}

template<typename T, typename Traits>
inline void swap(ConcurrentQueue<T, Traits>& a, ConcurrentQueue<T, Traits>& b) noexcept
{
	a.swap(b);
}

inline void swap(ProducerToken& a, ProducerToken& b) noexcept
{
	a.swap(b);
}

inline void swap(ConsumerToken& a, ConsumerToken& b) noexcept
{
	a.swap(b);
}

}

} /* namespace tracy */

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

/*** End of inlined file: tracy_concurrentqueue.h ***/


/*** Start of inlined file: TracySysTime.hpp ***/
#ifndef __TRACYSYSTIME_HPP__
#define __TRACYSYSTIME_HPP__

#if defined _WIN32 || defined __CYGWIN__ || defined __linux__ || defined __APPLE__
#  define TRACY_HAS_SYSTIME
#else
#  include <sys/param.h>
#endif

#ifdef BSD
#  define TRACY_HAS_SYSTIME
#endif

#ifdef TRACY_HAS_SYSTIME

#include <cstdint>

namespace tracy
{

class SysTime
{
public:
    SysTime();
    float Get();

    void ReadTimes();

private:
    uint64_t idle, used;
};

}
#endif

#endif

/*** End of inlined file: TracySysTime.hpp ***/


/*** Start of inlined file: TracyFastVector.hpp ***/
#ifndef __TRACYFASTVECTOR_HPP__
#define __TRACYFASTVECTOR_HPP__

#include <cassert>
#include <cstddef>

namespace tracy
{

template<typename T>
class FastVector
{
public:
    using iterator = T*;
    using const_iterator = const T*;

    FastVector( size_t capacity )
        : m_ptr( (T*)tracy_malloc( sizeof( T ) * capacity ) )
        , m_write( m_ptr )
        , m_end( m_ptr + capacity )
    {
        assert( capacity != 0 );
    }

    FastVector( const FastVector& ) = delete;
    FastVector( FastVector&& ) = delete;

    ~FastVector()
    {
        tracy_free( m_ptr );
    }

    FastVector& operator=( const FastVector& ) = delete;
    FastVector& operator=( FastVector&& ) = delete;

    bool empty() const { return m_ptr == m_write; }
    size_t size() const { return m_write - m_ptr; }

    T* data() { return m_ptr; }
    const T* data() const { return m_ptr; };

    T* begin() { return m_ptr; }
    const T* begin() const { return m_ptr; }
    T* end() { return m_write; }
    const T* end() const { return m_write; }

    T& front() { assert( !empty() ); return m_ptr[0]; }
    const T& front() const { assert( !empty() ); return m_ptr[0]; }

    T& back() { assert( !empty() ); return m_write[-1]; }
    const T& back() const { assert( !empty() ); return m_write[-1]; }

    T& operator[]( size_t idx ) { return m_ptr[idx]; }
    const T& operator[]( size_t idx ) const { return m_ptr[idx]; }

    T* push_next()
    {
        if( m_write == m_end ) AllocMore();
        return m_write++;
    }

    T* prepare_next()
    {
        if( m_write == m_end ) AllocMore();
        return m_write;
    }

    void commit_next()
    {
        m_write++;
    }

    void clear()
    {
        m_write = m_ptr;
    }

    void swap( FastVector& vec )
    {
        const auto ptr1 = m_ptr;
        const auto ptr2 = vec.m_ptr;
        const auto write1 = m_write;
        const auto write2 = vec.m_write;
        const auto end1 = m_end;
        const auto end2 = vec.m_end;

        m_ptr = ptr2;
        vec.m_ptr = ptr1;
        m_write = write2;
        vec.m_write = write1;
        m_end = end2;
        vec.m_end = end1;
    }

private:
    tracy_no_inline void AllocMore()
    {
        const auto cap = size_t( m_end - m_ptr ) * 2;
        const auto size = size_t( m_write - m_ptr );
        T* ptr = (T*)tracy_malloc( sizeof( T ) * cap );
        memcpy( ptr, m_ptr, size * sizeof( T ) );
        tracy_free_fast( m_ptr );
        m_ptr = ptr;
        m_write = m_ptr + size;
        m_end = m_ptr + cap;
    }

    T* m_ptr;
    T* m_write;
    T* m_end;
};

}

#endif

/*** End of inlined file: TracyFastVector.hpp ***/


/*** Start of inlined file: TracyQueue.hpp ***/
#ifndef __TRACYQUEUE_HPP__
#define __TRACYQUEUE_HPP__

#include <cstdint>

namespace tracy
{

enum class QueueType : uint8_t
{
    ZoneText,
    ZoneName,
    Message,
    MessageColor,
    MessageCallstack,
    MessageColorCallstack,
    MessageAppInfo,
    ZoneBeginAllocSrcLoc,
    ZoneBeginAllocSrcLocCallstack,
    CallstackSerial,
    Callstack,
    CallstackAlloc,
    CallstackSample,
    FrameImage,
    ZoneBegin,
    ZoneBeginCallstack,
    ZoneEnd,
    LockWait,
    LockObtain,
    LockRelease,
    LockSharedWait,
    LockSharedObtain,
    LockSharedRelease,
    LockName,
    MemAlloc,
    MemAllocNamed,
    MemFree,
    MemFreeNamed,
    MemAllocCallstack,
    MemAllocCallstackNamed,
    MemFreeCallstack,
    MemFreeCallstackNamed,
    GpuZoneBegin,
    GpuZoneBeginCallstack,
    GpuZoneBeginAllocSrcLoc,
    GpuZoneBeginAllocSrcLocCallstack,
    GpuZoneEnd,
    GpuZoneBeginSerial,
    GpuZoneBeginCallstackSerial,
    GpuZoneBeginAllocSrcLocSerial,
    GpuZoneBeginAllocSrcLocCallstackSerial,
    GpuZoneEndSerial,
    PlotData,
    ContextSwitch,
    ThreadWakeup,
    GpuTime,
    GpuContextName,
    Terminate,
    KeepAlive,
    ThreadContext,
    GpuCalibration,
    Crash,
    CrashReport,
    ZoneValidation,
    ZoneColor,
    ZoneValue,
    FrameMarkMsg,
    FrameMarkMsgStart,
    FrameMarkMsgEnd,
    SourceLocation,
    LockAnnounce,
    LockTerminate,
    LockMark,
    MessageLiteral,
    MessageLiteralColor,
    MessageLiteralCallstack,
    MessageLiteralColorCallstack,
    GpuNewContext,
    CallstackFrameSize,
    CallstackFrame,
    SymbolInformation,
    CodeInformation,
    SysTimeReport,
    TidToPid,
    HwSampleCpuCycle,
    HwSampleInstructionRetired,
    HwSampleCacheReference,
    HwSampleCacheMiss,
    HwSampleBranchRetired,
    HwSampleBranchMiss,
    PlotConfig,
    ParamSetup,
    AckServerQueryNoop,
    AckSourceCodeNotAvailable,
    CpuTopology,
    SingleStringData,
    SecondStringData,
    MemNamePayload,
    StringData,
    ThreadName,
    PlotName,
    SourceLocationPayload,
    CallstackPayload,
    CallstackAllocPayload,
    FrameName,
    FrameImageData,
    ExternalName,
    ExternalThreadName,
    SymbolCode,
    SourceCode,
    NUM_TYPES
};

#pragma pack( 1 )

struct QueueThreadContext
{
    uint64_t thread;
};

struct QueueZoneBeginLean
{
    int64_t time;
};

struct QueueZoneBegin : public QueueZoneBeginLean
{
    uint64_t srcloc;    // ptr
};

struct QueueZoneEnd
{
    int64_t time;
};

struct QueueZoneValidation
{
    uint32_t id;
};

struct QueueZoneColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct QueueZoneValue
{
    uint64_t value;
};

struct QueueStringTransfer
{
    uint64_t ptr;
};

struct QueueFrameMark
{
    int64_t time;
    uint64_t name;      // ptr
};

struct QueueFrameImage
{
    uint32_t frame;
    uint16_t w;
    uint16_t h;
    uint8_t flip;
};

struct QueueFrameImageFat : public QueueFrameImage
{
    uint64_t image;     // ptr
};

struct QueueSourceLocation
{
    uint64_t name;
    uint64_t function;  // ptr
    uint64_t file;      // ptr
    uint32_t line;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct QueueZoneTextFat
{
    uint64_t text;      // ptr
    uint16_t size;
};

enum class LockType : uint8_t
{
    Lockable,
    SharedLockable
};

struct QueueLockAnnounce
{
    uint32_t id;
    int64_t time;
    uint64_t lckloc;    // ptr
    LockType type;
};

struct QueueLockTerminate
{
    uint32_t id;
    int64_t time;
};

struct QueueLockWait
{
    uint64_t thread;
    uint32_t id;
    int64_t time;
};

struct QueueLockObtain
{
    uint64_t thread;
    uint32_t id;
    int64_t time;
};

struct QueueLockRelease
{
    uint64_t thread;
    uint32_t id;
    int64_t time;
};

struct QueueLockMark
{
    uint64_t thread;
    uint32_t id;
    uint64_t srcloc;    // ptr
};

struct QueueLockName
{
    uint32_t id;
};

struct QueueLockNameFat : public QueueLockName
{
    uint64_t name;      // ptr
    uint16_t size;
};

enum class PlotDataType : uint8_t
{
    Float,
    Double,
    Int
};

struct QueuePlotData
{
    uint64_t name;      // ptr
    int64_t time;
    PlotDataType type;
    union
    {
        double d;
        float f;
        int64_t i;
    } data;
};

struct QueueMessage
{
    int64_t time;
};

struct QueueMessageColor : public QueueMessage
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct QueueMessageLiteral : public QueueMessage
{
    uint64_t text;      // ptr
};

struct QueueMessageColorLiteral : public QueueMessageColor
{
    uint64_t text;      // ptr
};

struct QueueMessageFat : public QueueMessage
{
    uint64_t text;      // ptr
    uint16_t size;
};

struct QueueMessageColorFat : public QueueMessageColor
{
    uint64_t text;      // ptr
    uint16_t size;
};

// Don't change order, only add new entries at the end, this is also used on trace dumps!
enum class GpuContextType : uint8_t
{
    Invalid,
    OpenGl,
    Vulkan,
    OpenCL,
    Direct3D12,
    Direct3D11
};

enum GpuContextFlags : uint8_t
{
    GpuContextCalibration   = 1 << 0
};

struct QueueGpuNewContext
{
    int64_t cpuTime;
    int64_t gpuTime;
    uint64_t thread;
    float period;
    uint8_t context;
    GpuContextFlags flags;
    GpuContextType type;
};

struct QueueGpuZoneBeginLean
{
    int64_t cpuTime;
    uint64_t thread;
    uint16_t queryId;
    uint8_t context;
};

struct QueueGpuZoneBegin : public QueueGpuZoneBeginLean
{
    uint64_t srcloc;
};

struct QueueGpuZoneEnd
{
    int64_t cpuTime;
    uint64_t thread;
    uint16_t queryId;
    uint8_t context;
};

struct QueueGpuTime
{
    int64_t gpuTime;
    uint16_t queryId;
    uint8_t context;
};

struct QueueGpuCalibration
{
    int64_t gpuTime;
    int64_t cpuTime;
    int64_t cpuDelta;
    uint8_t context;
};

struct QueueGpuContextName
{
    uint8_t context;
};

struct QueueGpuContextNameFat : public QueueGpuContextName
{
    uint64_t ptr;
    uint16_t size;
};

struct QueueMemNamePayload
{
    uint64_t name;
};

struct QueueMemAlloc
{
    int64_t time;
    uint64_t thread;
    uint64_t ptr;
    char size[6];
};

struct QueueMemFree
{
    int64_t time;
    uint64_t thread;
    uint64_t ptr;
};

struct QueueCallstackFat
{
    uint64_t ptr;
};

struct QueueCallstackAllocFat
{
    uint64_t ptr;
    uint64_t nativePtr;
};

struct QueueCallstackSample
{
    int64_t time;
    uint64_t thread;
};

struct QueueCallstackSampleFat : public QueueCallstackSample
{
    uint64_t ptr;
};

struct QueueCallstackFrameSize
{
    uint64_t ptr;
    uint8_t size;
};

struct QueueCallstackFrame
{
    uint32_t line;
    uint64_t symAddr;
    uint32_t symLen;
};

struct QueueSymbolInformation
{
    uint32_t line;
    uint64_t symAddr;
};

struct QueueCodeInformation
{
    uint64_t ptr;
    uint32_t line;
    uint64_t symAddr;
};

struct QueueCrashReport
{
    int64_t time;
    uint64_t text;      // ptr
};

struct QueueSysTime
{
    int64_t time;
    float sysTime;
};

struct QueueContextSwitch
{
    int64_t time;
    uint64_t oldThread;
    uint64_t newThread;
    uint8_t cpu;
    uint8_t reason;
    uint8_t state;
};

struct QueueThreadWakeup
{
    int64_t time;
    uint64_t thread;
};

struct QueueTidToPid
{
    uint64_t tid;
    uint64_t pid;
};

struct QueueHwSample
{
    uint64_t ip;
    int64_t time;
};

enum class PlotFormatType : uint8_t
{
    Number,
    Memory,
    Percentage
};

struct QueuePlotConfig
{
    uint64_t name;      // ptr
    uint8_t type;
};

struct QueueParamSetup
{
    uint32_t idx;
    uint64_t name;      // ptr
    uint8_t isBool;
    int32_t val;
};

struct QueueCpuTopology
{
    uint32_t package;
    uint32_t core;
    uint32_t thread;
};

struct QueueHeader
{
    union
    {
        QueueType type;
        uint8_t idx;
    };
};

struct QueueItem
{
    QueueHeader hdr;
    union
    {
        QueueThreadContext threadCtx;
        QueueZoneBegin zoneBegin;
        QueueZoneBeginLean zoneBeginLean;
        QueueZoneEnd zoneEnd;
        QueueZoneValidation zoneValidation;
        QueueZoneColor zoneColor;
        QueueZoneValue zoneValue;
        QueueStringTransfer stringTransfer;
        QueueFrameMark frameMark;
        QueueFrameImage frameImage;
        QueueFrameImageFat frameImageFat;
        QueueSourceLocation srcloc;
        QueueZoneTextFat zoneTextFat;
        QueueLockAnnounce lockAnnounce;
        QueueLockTerminate lockTerminate;
        QueueLockWait lockWait;
        QueueLockObtain lockObtain;
        QueueLockRelease lockRelease;
        QueueLockMark lockMark;
        QueueLockName lockName;
        QueueLockNameFat lockNameFat;
        QueuePlotData plotData;
        QueueMessage message;
        QueueMessageColor messageColor;
        QueueMessageLiteral messageLiteral;
        QueueMessageColorLiteral messageColorLiteral;
        QueueMessageFat messageFat;
        QueueMessageColorFat messageColorFat;
        QueueGpuNewContext gpuNewContext;
        QueueGpuZoneBegin gpuZoneBegin;
        QueueGpuZoneBeginLean gpuZoneBeginLean;
        QueueGpuZoneEnd gpuZoneEnd;
        QueueGpuTime gpuTime;
        QueueGpuCalibration gpuCalibration;
        QueueGpuContextName gpuContextName;
        QueueGpuContextNameFat gpuContextNameFat;
        QueueMemAlloc memAlloc;
        QueueMemFree memFree;
        QueueMemNamePayload memName;
        QueueCallstackFat callstackFat;
        QueueCallstackAllocFat callstackAllocFat;
        QueueCallstackSample callstackSample;
        QueueCallstackSampleFat callstackSampleFat;
        QueueCallstackFrameSize callstackFrameSize;
        QueueCallstackFrame callstackFrame;
        QueueSymbolInformation symbolInformation;
        QueueCodeInformation codeInformation;
        QueueCrashReport crashReport;
        QueueSysTime sysTime;
        QueueContextSwitch contextSwitch;
        QueueThreadWakeup threadWakeup;
        QueueTidToPid tidToPid;
        QueueHwSample hwSample;
        QueuePlotConfig plotConfig;
        QueueParamSetup paramSetup;
        QueueCpuTopology cpuTopology;
    };
};
#pragma pack()

enum { QueueItemSize = sizeof( QueueItem ) };

static constexpr size_t QueueDataSize[] = {
    sizeof( QueueHeader ),                                  // zone text
    sizeof( QueueHeader ),                                  // zone name
    sizeof( QueueHeader ) + sizeof( QueueMessage ),
    sizeof( QueueHeader ) + sizeof( QueueMessageColor ),
    sizeof( QueueHeader ) + sizeof( QueueMessage ),         // callstack
    sizeof( QueueHeader ) + sizeof( QueueMessageColor ),    // callstack
    sizeof( QueueHeader ) + sizeof( QueueMessage ),         // app info
    sizeof( QueueHeader ) + sizeof( QueueZoneBeginLean ),   // allocated source location
    sizeof( QueueHeader ) + sizeof( QueueZoneBeginLean ),   // allocated source location, callstack
    sizeof( QueueHeader ),                                  // callstack memory
    sizeof( QueueHeader ),                                  // callstack
    sizeof( QueueHeader ),                                  // callstack alloc
    sizeof( QueueHeader ) + sizeof( QueueCallstackSample ),
    sizeof( QueueHeader ) + sizeof( QueueFrameImage ),
    sizeof( QueueHeader ) + sizeof( QueueZoneBegin ),
    sizeof( QueueHeader ) + sizeof( QueueZoneBegin ),       // callstack
    sizeof( QueueHeader ) + sizeof( QueueZoneEnd ),
    sizeof( QueueHeader ) + sizeof( QueueLockWait ),
    sizeof( QueueHeader ) + sizeof( QueueLockObtain ),
    sizeof( QueueHeader ) + sizeof( QueueLockRelease ),
    sizeof( QueueHeader ) + sizeof( QueueLockWait ),        // shared
    sizeof( QueueHeader ) + sizeof( QueueLockObtain ),      // shared
    sizeof( QueueHeader ) + sizeof( QueueLockRelease ),     // shared
    sizeof( QueueHeader ) + sizeof( QueueLockName ),
    sizeof( QueueHeader ) + sizeof( QueueMemAlloc ),
    sizeof( QueueHeader ) + sizeof( QueueMemAlloc ),        // named
    sizeof( QueueHeader ) + sizeof( QueueMemFree ),
    sizeof( QueueHeader ) + sizeof( QueueMemFree ),         // named
    sizeof( QueueHeader ) + sizeof( QueueMemAlloc ),        // callstack
    sizeof( QueueHeader ) + sizeof( QueueMemAlloc ),        // callstack, named
    sizeof( QueueHeader ) + sizeof( QueueMemFree ),         // callstack
    sizeof( QueueHeader ) + sizeof( QueueMemFree ),         // callstack, named
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBegin ),
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBegin ),    // callstack
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBeginLean ),// allocated source location
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBeginLean ),// allocated source location, callstack
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneEnd ),
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBegin ),    // serial
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBegin ),    // serial, callstack
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBeginLean ),// serial, allocated source location
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneBeginLean ),// serial, allocated source location, callstack
    sizeof( QueueHeader ) + sizeof( QueueGpuZoneEnd ),      // serial
    sizeof( QueueHeader ) + sizeof( QueuePlotData ),
    sizeof( QueueHeader ) + sizeof( QueueContextSwitch ),
    sizeof( QueueHeader ) + sizeof( QueueThreadWakeup ),
    sizeof( QueueHeader ) + sizeof( QueueGpuTime ),
    sizeof( QueueHeader ) + sizeof( QueueGpuContextName ),
    // above items must be first
    sizeof( QueueHeader ),                                  // terminate
    sizeof( QueueHeader ),                                  // keep alive
    sizeof( QueueHeader ) + sizeof( QueueThreadContext ),
    sizeof( QueueHeader ) + sizeof( QueueGpuCalibration ),
    sizeof( QueueHeader ),                                  // crash
    sizeof( QueueHeader ) + sizeof( QueueCrashReport ),
    sizeof( QueueHeader ) + sizeof( QueueZoneValidation ),
    sizeof( QueueHeader ) + sizeof( QueueZoneColor ),
    sizeof( QueueHeader ) + sizeof( QueueZoneValue ),
    sizeof( QueueHeader ) + sizeof( QueueFrameMark ),       // continuous frames
    sizeof( QueueHeader ) + sizeof( QueueFrameMark ),       // start
    sizeof( QueueHeader ) + sizeof( QueueFrameMark ),       // end
    sizeof( QueueHeader ) + sizeof( QueueSourceLocation ),
    sizeof( QueueHeader ) + sizeof( QueueLockAnnounce ),
    sizeof( QueueHeader ) + sizeof( QueueLockTerminate ),
    sizeof( QueueHeader ) + sizeof( QueueLockMark ),
    sizeof( QueueHeader ) + sizeof( QueueMessageLiteral ),
    sizeof( QueueHeader ) + sizeof( QueueMessageColorLiteral ),
    sizeof( QueueHeader ) + sizeof( QueueMessageLiteral ),  // callstack
    sizeof( QueueHeader ) + sizeof( QueueMessageColorLiteral ), // callstack
    sizeof( QueueHeader ) + sizeof( QueueGpuNewContext ),
    sizeof( QueueHeader ) + sizeof( QueueCallstackFrameSize ),
    sizeof( QueueHeader ) + sizeof( QueueCallstackFrame ),
    sizeof( QueueHeader ) + sizeof( QueueSymbolInformation ),
    sizeof( QueueHeader ) + sizeof( QueueCodeInformation ),
    sizeof( QueueHeader ) + sizeof( QueueSysTime ),
    sizeof( QueueHeader ) + sizeof( QueueTidToPid ),
    sizeof( QueueHeader ) + sizeof( QueueHwSample ),        // cpu cycle
    sizeof( QueueHeader ) + sizeof( QueueHwSample ),        // instruction retired
    sizeof( QueueHeader ) + sizeof( QueueHwSample ),        // cache reference
    sizeof( QueueHeader ) + sizeof( QueueHwSample ),        // cache miss
    sizeof( QueueHeader ) + sizeof( QueueHwSample ),        // branch retired
    sizeof( QueueHeader ) + sizeof( QueueHwSample ),        // branch miss
    sizeof( QueueHeader ) + sizeof( QueuePlotConfig ),
    sizeof( QueueHeader ) + sizeof( QueueParamSetup ),
    sizeof( QueueHeader ),                                  // server query acknowledgement
    sizeof( QueueHeader ),                                  // source code not available
    sizeof( QueueHeader ) + sizeof( QueueCpuTopology ),
    sizeof( QueueHeader ),                                  // single string data
    sizeof( QueueHeader ),                                  // second string data
    sizeof( QueueHeader ) + sizeof( QueueMemNamePayload ),
    // keep all QueueStringTransfer below
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // string data
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // thread name
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // plot name
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // allocated source location payload
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // callstack payload
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // callstack alloc payload
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // frame name
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // frame image data
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // external name
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // external thread name
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // symbol code
    sizeof( QueueHeader ) + sizeof( QueueStringTransfer ),  // source code
};

static_assert( QueueItemSize == 32, "Queue item size not 32 bytes" );
static_assert( sizeof( QueueDataSize ) / sizeof( size_t ) == (uint8_t)QueueType::NUM_TYPES, "QueueDataSize mismatch" );
static_assert( sizeof( void* ) <= sizeof( uint64_t ), "Pointer size > 8 bytes" );
static_assert( sizeof( void* ) == sizeof( uintptr_t ), "Pointer size != uintptr_t" );

}

#endif

/*** End of inlined file: TracyQueue.hpp ***/


/*** Start of inlined file: TracyMutex.hpp ***/
#ifndef __TRACYMUTEX_HPP__
#define __TRACYMUTEX_HPP__

#if defined _MSC_VER

#  include <shared_mutex>

namespace tracy
{
using TracyMutex = std::shared_mutex;
}

#else

#include <mutex>

namespace tracy
{
using TracyMutex = std::mutex;
}

#endif

#endif

/*** End of inlined file: TracyMutex.hpp ***/


/*** Start of inlined file: TracyProtocol.hpp ***/
#ifndef __TRACYPROTOCOL_HPP__
#define __TRACYPROTOCOL_HPP__

#include <limits>
#include <cstdint>

namespace tracy
{

constexpr unsigned Lz4CompressBound( unsigned isize ) { return isize + ( isize / 255 ) + 16; }

enum : uint32_t { ProtocolVersion = 48 };
enum : uint16_t { BroadcastVersion = 2 };

using lz4sz_t = uint32_t;

enum { TargetFrameSize = 256 * 1024 };
enum { LZ4Size = Lz4CompressBound( TargetFrameSize ) };
static_assert( LZ4Size <= std::numeric_limits<lz4sz_t>::max(), "LZ4Size greater than lz4sz_t" );
static_assert( TargetFrameSize * 2 >= 64 * 1024, "Not enough space for LZ4 stream buffer" );

enum { HandshakeShibbolethSize = 8 };
static const char HandshakeShibboleth[HandshakeShibbolethSize] = { 'T', 'r', 'a', 'c', 'y', 'P', 'r', 'f' };

enum HandshakeStatus : uint8_t
{
    HandshakePending,
    HandshakeWelcome,
    HandshakeProtocolMismatch,
    HandshakeNotAvailable,
    HandshakeDropped
};

enum { WelcomeMessageProgramNameSize = 64 };
enum { WelcomeMessageHostInfoSize = 1024 };

#pragma pack( 1 )

// Must increase left query space after handling!
enum ServerQuery : uint8_t
{
    ServerQueryTerminate,
    ServerQueryString,
    ServerQueryThreadString,
    ServerQuerySourceLocation,
    ServerQueryPlotName,
    ServerQueryCallstackFrame,
    ServerQueryFrameName,
    ServerQueryDisconnect,
    ServerQueryExternalName,
    ServerQueryParameter,
    ServerQuerySymbol,
    ServerQuerySymbolCode,
    ServerQueryCodeLocation,
    ServerQuerySourceCode,
    ServerQueryDataTransfer,
    ServerQueryDataTransferPart
};

struct ServerQueryPacket
{
    ServerQuery type;
    uint64_t ptr;
    uint32_t extra;
};

enum { ServerQueryPacketSize = sizeof( ServerQueryPacket ) };

enum CpuArchitecture : uint8_t
{
    CpuArchUnknown,
    CpuArchX86,
    CpuArchX64,
    CpuArchArm32,
    CpuArchArm64
};

struct WelcomeFlag
{
    enum _t : uint8_t
    {
        OnDemand        = 1 << 0,
        IsApple         = 1 << 1,
        CodeTransfer    = 1 << 2,
        CombineSamples  = 1 << 3,
    };
};

struct WelcomeMessage
{
    double timerMul;
    int64_t initBegin;
    int64_t initEnd;
    uint64_t delay;
    uint64_t resolution;
    uint64_t epoch;
    uint64_t exectime;
    uint64_t pid;
    int64_t samplingPeriod;
    uint8_t flags;
    uint8_t cpuArch;
    char cpuManufacturer[12];
    uint32_t cpuId;
    char programName[WelcomeMessageProgramNameSize];
    char hostInfo[WelcomeMessageHostInfoSize];
};

enum { WelcomeMessageSize = sizeof( WelcomeMessage ) };

struct OnDemandPayloadMessage
{
    uint64_t frames;
    uint64_t currentTime;
};

enum { OnDemandPayloadMessageSize = sizeof( OnDemandPayloadMessage ) };

struct BroadcastMessage
{
    uint16_t broadcastVersion;
    uint16_t listenPort;
    uint32_t protocolVersion;
    int32_t activeTime;        // in seconds
    char programName[WelcomeMessageProgramNameSize];
};

enum { BroadcastMessageSize = sizeof( BroadcastMessage ) };

#pragma pack()

}

#endif

/*** End of inlined file: TracyProtocol.hpp ***/

#if defined _WIN32 || defined __CYGWIN__
#  include <intrin.h>
#endif
#ifdef __APPLE__
#  include <TargetConditionals.h>
#  include <mach/mach_time.h>
#endif

#if !defined TRACY_TIMER_FALLBACK && ( defined _WIN32 || defined __CYGWIN__ || ( defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64 ) || ( defined TARGET_OS_IOS && TARGET_OS_IOS == 1 ) )
#  define TRACY_HW_TIMER
#endif

#if !defined TRACY_HW_TIMER
#  include <chrono>
#endif

#ifndef TracyConcat
#  define TracyConcat(x,y) TracyConcatIndirect(x,y)
#endif
#ifndef TracyConcatIndirect
#  define TracyConcatIndirect(x,y) x##y
#endif

namespace tracy
{
#if defined(TRACY_DELAYED_INIT) && defined(TRACY_MANUAL_LIFETIME)
TRACY_API void StartupProfiler();
TRACY_API void ShutdownProfiler();
#endif

class GpuCtx;
class Profiler;
class Socket;
class UdpBroadcast;

struct GpuCtxWrapper
{
    GpuCtx* ptr;
};

TRACY_API moodycamel::ConcurrentQueue<QueueItem>::ExplicitProducer* GetToken();
TRACY_API Profiler& GetProfiler();
TRACY_API std::atomic<uint32_t>& GetLockCounter();
TRACY_API std::atomic<uint8_t>& GetGpuCtxCounter();
TRACY_API GpuCtxWrapper& GetGpuCtx();
TRACY_API uint64_t GetThreadHandle();
TRACY_API bool ProfilerAvailable();
TRACY_API int64_t GetFrequencyQpc();

struct SourceLocationData
{
    const char* name;
    const char* function;
    const char* file;
    uint32_t line;
    uint32_t color;
};

#ifdef TRACY_ON_DEMAND
struct LuaZoneState
{
    uint32_t counter;
    bool active;
};
#endif

#define TracyLfqPrepare( _type ) \
    moodycamel::ConcurrentQueueDefaultTraits::index_t __magic; \
    auto __token = GetToken(); \
    auto& __tail = __token->get_tail_index(); \
    auto item = __token->enqueue_begin( __magic ); \
    MemWrite( &item->hdr.type, _type );

#define TracyLfqCommit \
    __tail.store( __magic + 1, std::memory_order_release );

#define TracyLfqPrepareC( _type ) \
    tracy::moodycamel::ConcurrentQueueDefaultTraits::index_t __magic; \
    auto __token = tracy::GetToken(); \
    auto& __tail = __token->get_tail_index(); \
    auto item = __token->enqueue_begin( __magic ); \
    tracy::MemWrite( &item->hdr.type, _type );

#define TracyLfqCommitC \
    __tail.store( __magic + 1, std::memory_order_release );

typedef void(*ParameterCallback)( uint32_t idx, int32_t val );

class Profiler
{
    struct FrameImageQueueItem
    {
        void* image;
        uint32_t frame;
        uint16_t w;
        uint16_t h;
        uint8_t offset;
        bool flip;
    };

public:
    Profiler();
    ~Profiler();

    void SpawnWorkerThreads();

    static tracy_force_inline int64_t GetTime()
    {
#ifdef TRACY_HW_TIMER
#  if defined TARGET_OS_IOS && TARGET_OS_IOS == 1
        return mach_absolute_time();
#  elif defined _WIN32 || defined __CYGWIN__
#    ifdef TRACY_TIMER_QPC
        return GetTimeQpc();
#    else
        return int64_t( __rdtsc() );
#    endif
#  elif defined __i386 || defined _M_IX86
        uint32_t eax, edx;
        asm volatile ( "rdtsc" : "=a" (eax), "=d" (edx) );
        return ( uint64_t( edx ) << 32 ) + uint64_t( eax );
#  elif defined __x86_64__ || defined _M_X64
        uint64_t rax, rdx;
        asm volatile ( "rdtsc" : "=a" (rax), "=d" (rdx) );
        return (int64_t)(( rdx << 32 ) + rax);
#  else
#    error "TRACY_HW_TIMER detection logic needs fixing"
#  endif
#else
#  if defined __linux__ && defined CLOCK_MONOTONIC_RAW
        struct timespec ts;
        clock_gettime( CLOCK_MONOTONIC_RAW, &ts );
        return int64_t( ts.tv_sec ) * 1000000000ll + int64_t( ts.tv_nsec );
#  else
        return std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count();
#  endif
#endif
    }

    tracy_force_inline uint32_t GetNextZoneId()
    {
        return m_zoneId.fetch_add( 1, std::memory_order_relaxed );
    }

    static tracy_force_inline QueueItem* QueueSerial()
    {
        auto& p = GetProfiler();
        p.m_serialLock.lock();
        return p.m_serialQueue.prepare_next();
    }

    static tracy_force_inline QueueItem* QueueSerialCallstack( void* ptr )
    {
        auto& p = GetProfiler();
        p.m_serialLock.lock();
        p.SendCallstackSerial( ptr );
        return p.m_serialQueue.prepare_next();
    }

    static tracy_force_inline void QueueSerialFinish()
    {
        auto& p = GetProfiler();
        p.m_serialQueue.commit_next();
        p.m_serialLock.unlock();
    }

    static tracy_force_inline void SendFrameMark( const char* name )
    {
        if( !name ) GetProfiler().m_frameCount.fetch_add( 1, std::memory_order_relaxed );
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        TracyLfqPrepare( QueueType::FrameMarkMsg );
        MemWrite( &item->frameMark.time, GetTime() );
        MemWrite( &item->frameMark.name, uint64_t( name ) );
        TracyLfqCommit;
    }

    static tracy_force_inline void SendFrameMark( const char* name, QueueType type )
    {
        assert( type == QueueType::FrameMarkMsgStart || type == QueueType::FrameMarkMsgEnd );
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        auto item = QueueSerial();
        MemWrite( &item->hdr.type, type );
        MemWrite( &item->frameMark.time, GetTime() );
        MemWrite( &item->frameMark.name, uint64_t( name ) );
        QueueSerialFinish();
    }

    static tracy_force_inline void SendFrameImage( const void* image, uint16_t w, uint16_t h, uint8_t offset, bool flip )
    {
#ifndef TRACY_NO_FRAME_IMAGE
        auto& profiler = GetProfiler();
        assert( profiler.m_frameCount.load( std::memory_order_relaxed ) < std::numeric_limits<uint32_t>::max() );
#  ifdef TRACY_ON_DEMAND
        if( !profiler.IsConnected() ) return;
#  endif
        const auto sz = size_t( w ) * size_t( h ) * 4;
        auto ptr = (char*)tracy_malloc( sz );
        memcpy( ptr, image, sz );

        profiler.m_fiLock.lock();
        auto fi = profiler.m_fiQueue.prepare_next();
        fi->image = ptr;
        fi->frame = uint32_t( profiler.m_frameCount.load( std::memory_order_relaxed ) - offset );
        fi->w = w;
        fi->h = h;
        fi->flip = flip;
        profiler.m_fiQueue.commit_next();
        profiler.m_fiLock.unlock();
#endif
    }

    static tracy_force_inline void PlotData( const char* name, int64_t val )
    {
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        TracyLfqPrepare( QueueType::PlotData );
        MemWrite( &item->plotData.name, (uint64_t)name );
        MemWrite( &item->plotData.time, GetTime() );
        MemWrite( &item->plotData.type, PlotDataType::Int );
        MemWrite( &item->plotData.data.i, val );
        TracyLfqCommit;
    }

    static tracy_force_inline void PlotData( const char* name, float val )
    {
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        TracyLfqPrepare( QueueType::PlotData );
        MemWrite( &item->plotData.name, (uint64_t)name );
        MemWrite( &item->plotData.time, GetTime() );
        MemWrite( &item->plotData.type, PlotDataType::Float );
        MemWrite( &item->plotData.data.f, val );
        TracyLfqCommit;
    }

    static tracy_force_inline void PlotData( const char* name, double val )
    {
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        TracyLfqPrepare( QueueType::PlotData );
        MemWrite( &item->plotData.name, (uint64_t)name );
        MemWrite( &item->plotData.time, GetTime() );
        MemWrite( &item->plotData.type, PlotDataType::Double );
        MemWrite( &item->plotData.data.d, val );
        TracyLfqCommit;
    }

    static tracy_force_inline void ConfigurePlot( const char* name, PlotFormatType type )
    {
        TracyLfqPrepare( QueueType::PlotConfig );
        MemWrite( &item->plotConfig.name, (uint64_t)name );
        MemWrite( &item->plotConfig.type, (uint8_t)type );

#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif

        TracyLfqCommit;
    }

    static tracy_force_inline void Message( const char* txt, size_t size, int callstack )
    {
        assert( size < std::numeric_limits<uint16_t>::max() );
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        if( callstack != 0 )
        {
            tracy::GetProfiler().SendCallstack( callstack );
        }

        TracyLfqPrepare( callstack == 0 ? QueueType::Message : QueueType::MessageCallstack );
        auto ptr = (char*)tracy_malloc( size );
        memcpy( ptr, txt, size );
        MemWrite( &item->messageFat.time, GetTime() );
        MemWrite( &item->messageFat.text, (uint64_t)ptr );
        MemWrite( &item->messageFat.size, (uint16_t)size );
        TracyLfqCommit;
    }

    static tracy_force_inline void Message( const char* txt, int callstack )
    {
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        if( callstack != 0 )
        {
            tracy::GetProfiler().SendCallstack( callstack );
        }

        TracyLfqPrepare( callstack == 0 ? QueueType::MessageLiteral : QueueType::MessageLiteralCallstack );
        MemWrite( &item->messageLiteral.time, GetTime() );
        MemWrite( &item->messageLiteral.text, (uint64_t)txt );
        TracyLfqCommit;
    }

    static tracy_force_inline void MessageColor( const char* txt, size_t size, uint32_t color, int callstack )
    {
        assert( size < std::numeric_limits<uint16_t>::max() );
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        if( callstack != 0 )
        {
            tracy::GetProfiler().SendCallstack( callstack );
        }

        TracyLfqPrepare( callstack == 0 ? QueueType::MessageColor : QueueType::MessageColorCallstack );
        auto ptr = (char*)tracy_malloc( size );
        memcpy( ptr, txt, size );
        MemWrite( &item->messageColorFat.time, GetTime() );
        MemWrite( &item->messageColorFat.text, (uint64_t)ptr );
        MemWrite( &item->messageColorFat.r, uint8_t( ( color       ) & 0xFF ) );
        MemWrite( &item->messageColorFat.g, uint8_t( ( color >> 8  ) & 0xFF ) );
        MemWrite( &item->messageColorFat.b, uint8_t( ( color >> 16 ) & 0xFF ) );
        MemWrite( &item->messageColorFat.size, (uint16_t)size );
        TracyLfqCommit;
    }

    static tracy_force_inline void MessageColor( const char* txt, uint32_t color, int callstack )
    {
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        if( callstack != 0 )
        {
            tracy::GetProfiler().SendCallstack( callstack );
        }

        TracyLfqPrepare( callstack == 0 ? QueueType::MessageLiteralColor : QueueType::MessageLiteralColorCallstack );
        MemWrite( &item->messageColorLiteral.time, GetTime() );
        MemWrite( &item->messageColorLiteral.text, (uint64_t)txt );
        MemWrite( &item->messageColorLiteral.r, uint8_t( ( color       ) & 0xFF ) );
        MemWrite( &item->messageColorLiteral.g, uint8_t( ( color >> 8  ) & 0xFF ) );
        MemWrite( &item->messageColorLiteral.b, uint8_t( ( color >> 16 ) & 0xFF ) );
        TracyLfqCommit;
    }

    static tracy_force_inline void MessageAppInfo( const char* txt, size_t size )
    {
        assert( size < std::numeric_limits<uint16_t>::max() );
        auto ptr = (char*)tracy_malloc( size );
        memcpy( ptr, txt, size );
        TracyLfqPrepare( QueueType::MessageAppInfo );
        MemWrite( &item->messageFat.time, GetTime() );
        MemWrite( &item->messageFat.text, (uint64_t)ptr );
        MemWrite( &item->messageFat.size, (uint16_t)size );

#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif

        TracyLfqCommit;
    }

    static tracy_force_inline void MemAlloc( const void* ptr, size_t size, bool secure )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        const auto thread = GetThreadHandle();

        GetProfiler().m_serialLock.lock();
        SendMemAlloc( QueueType::MemAlloc, thread, ptr, size );
        GetProfiler().m_serialLock.unlock();
    }

    static tracy_force_inline void MemFree( const void* ptr, bool secure )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        const auto thread = GetThreadHandle();

        GetProfiler().m_serialLock.lock();
        SendMemFree( QueueType::MemFree, thread, ptr );
        GetProfiler().m_serialLock.unlock();
    }

    static tracy_force_inline void MemAllocCallstack( const void* ptr, size_t size, int depth, bool secure )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_HAS_CALLSTACK
        auto& profiler = GetProfiler();
#  ifdef TRACY_ON_DEMAND
        if( !profiler.IsConnected() ) return;
#  endif
        const auto thread = GetThreadHandle();

        auto callstack = Callstack( depth );

        profiler.m_serialLock.lock();
        SendCallstackSerial( callstack );
        SendMemAlloc( QueueType::MemAllocCallstack, thread, ptr, size );
        profiler.m_serialLock.unlock();
#else
        MemAlloc( ptr, size, secure );
#endif
    }

    static tracy_force_inline void MemFreeCallstack( const void* ptr, int depth, bool secure )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_HAS_CALLSTACK
        auto& profiler = GetProfiler();
#  ifdef TRACY_ON_DEMAND
        if( !profiler.IsConnected() ) return;
#  endif
        const auto thread = GetThreadHandle();

        auto callstack = Callstack( depth );

        profiler.m_serialLock.lock();
        SendCallstackSerial( callstack );
        SendMemFree( QueueType::MemFreeCallstack, thread, ptr );
        profiler.m_serialLock.unlock();
#else
        MemFree( ptr, secure );
#endif
    }

    static tracy_force_inline void MemAllocNamed( const void* ptr, size_t size, bool secure, const char* name )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        const auto thread = GetThreadHandle();

        GetProfiler().m_serialLock.lock();
        SendMemName( name );
        SendMemAlloc( QueueType::MemAllocNamed, thread, ptr, size );
        GetProfiler().m_serialLock.unlock();
    }

    static tracy_force_inline void MemFreeNamed( const void* ptr, bool secure, const char* name )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) return;
#endif
        const auto thread = GetThreadHandle();

        GetProfiler().m_serialLock.lock();
        SendMemName( name );
        SendMemFree( QueueType::MemFreeNamed, thread, ptr );
        GetProfiler().m_serialLock.unlock();
    }

    static tracy_force_inline void MemAllocCallstackNamed( const void* ptr, size_t size, int depth, bool secure, const char* name )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_HAS_CALLSTACK
        auto& profiler = GetProfiler();
#  ifdef TRACY_ON_DEMAND
        if( !profiler.IsConnected() ) return;
#  endif
        const auto thread = GetThreadHandle();

        auto callstack = Callstack( depth );

        profiler.m_serialLock.lock();
        SendCallstackSerial( callstack );
        SendMemName( name );
        SendMemAlloc( QueueType::MemAllocCallstackNamed, thread, ptr, size );
        profiler.m_serialLock.unlock();
#else
        MemAlloc( ptr, size, secure );
#endif
    }

    static tracy_force_inline void MemFreeCallstackNamed( const void* ptr, int depth, bool secure, const char* name )
    {
        if( secure && !ProfilerAvailable() ) return;
#ifdef TRACY_HAS_CALLSTACK
        auto& profiler = GetProfiler();
#  ifdef TRACY_ON_DEMAND
        if( !profiler.IsConnected() ) return;
#  endif
        const auto thread = GetThreadHandle();

        auto callstack = Callstack( depth );

        profiler.m_serialLock.lock();
        SendCallstackSerial( callstack );
        SendMemName( name );
        SendMemFree( QueueType::MemFreeCallstackNamed, thread, ptr );
        profiler.m_serialLock.unlock();
#else
        MemFree( ptr, secure );
#endif
    }

    static tracy_force_inline void SendCallstack( int depth )
    {
#ifdef TRACY_HAS_CALLSTACK
        auto ptr = Callstack( depth );
        TracyLfqPrepare( QueueType::Callstack );
        MemWrite( &item->callstackFat.ptr, (uint64_t)ptr );
        TracyLfqCommit;
#endif
    }

    static tracy_force_inline void ParameterRegister( ParameterCallback cb ) { GetProfiler().m_paramCallback = cb; }
    static tracy_force_inline void ParameterSetup( uint32_t idx, const char* name, bool isBool, int32_t val )
    {
        TracyLfqPrepare( QueueType::ParamSetup );
        tracy::MemWrite( &item->paramSetup.idx, idx );
        tracy::MemWrite( &item->paramSetup.name, (uint64_t)name );
        tracy::MemWrite( &item->paramSetup.isBool, (uint8_t)isBool );
        tracy::MemWrite( &item->paramSetup.val, val );

#ifdef TRACY_ON_DEMAND
        GetProfiler().DeferItem( *item );
#endif

        TracyLfqCommit;
    }

    void SendCallstack( int depth, const char* skipBefore );
    static void CutCallstack( void* callstack, const char* skipBefore );

    static bool ShouldExit();

    tracy_force_inline bool IsConnected() const
    {
        return m_isConnected.load( std::memory_order_acquire );
    }

#ifdef TRACY_ON_DEMAND
    tracy_force_inline uint64_t ConnectionId() const
    {
        return m_connectionId.load( std::memory_order_acquire );
    }

    tracy_force_inline void DeferItem( const QueueItem& item )
    {
        m_deferredLock.lock();
        auto dst = m_deferredQueue.push_next();
        memcpy( dst, &item, sizeof( item ) );
        m_deferredLock.unlock();
    }
#endif

    void RequestShutdown() { m_shutdown.store( true, std::memory_order_relaxed ); m_shutdownManual.store( true, std::memory_order_relaxed ); }
    bool HasShutdownFinished() const { return m_shutdownFinished.load( std::memory_order_relaxed ); }

    void SendString( uint64_t str, const char* ptr, QueueType type ) { SendString( str, ptr, strlen( ptr ), type ); }
    void SendString( uint64_t str, const char* ptr, size_t len, QueueType type );
    void SendSingleString( const char* ptr ) { SendSingleString( ptr, strlen( ptr ) ); }
    void SendSingleString( const char* ptr, size_t len );
    void SendSecondString( const char* ptr ) { SendSecondString( ptr, strlen( ptr ) ); }
    void SendSecondString( const char* ptr, size_t len );

    // Allocated source location data layout:
    //  2b  payload size
    //  4b  color
    //  4b  source line
    //  fsz function name
    //  1b  null terminator
    //  ssz source file name
    //  1b  null terminator
    //  nsz zone name (optional)

    static tracy_force_inline uint64_t AllocSourceLocation( uint32_t line, const char* source, const char* function )
    {
        return AllocSourceLocation( line, source, function, nullptr, 0 );
    }

    static tracy_force_inline uint64_t AllocSourceLocation( uint32_t line, const char* source, const char* function, const char* name, size_t nameSz )
    {
        return AllocSourceLocation( line, source, strlen(source), function, strlen(function), name, nameSz );
    }

    static tracy_force_inline uint64_t AllocSourceLocation( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz )
    {
        return AllocSourceLocation( line, source, sourceSz, function, functionSz, nullptr, 0 );
    }

    static tracy_force_inline uint64_t AllocSourceLocation( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz, const char* name, size_t nameSz )
    {
        const auto sz32 = uint32_t( 2 + 4 + 4 + functionSz + 1 + sourceSz + 1 + nameSz );
        assert( sz32 <= std::numeric_limits<uint16_t>::max() );
        const auto sz = uint16_t( sz32 );
        auto ptr = (char*)tracy_malloc( sz );
        memcpy( ptr, &sz, 2 );
        memset( ptr + 2, 0, 4 );
        memcpy( ptr + 6, &line, 4 );
        memcpy( ptr + 10, function, functionSz );
        ptr[10 + functionSz] = '\0';
        memcpy( ptr + 10 + functionSz + 1, source, sourceSz );
        ptr[10 + functionSz + 1 + sourceSz] = '\0';
        if( nameSz != 0 )
        {
            memcpy( ptr + 10 + functionSz + 1 + sourceSz + 1, name, nameSz );
        }
        return uint64_t( ptr );
    }

private:
    enum class DequeueStatus { DataDequeued, ConnectionLost, QueueEmpty };

    static void LaunchWorker( void* ptr ) { ((Profiler*)ptr)->Worker(); }
    void Worker();

#ifndef TRACY_NO_FRAME_IMAGE
    static void LaunchCompressWorker( void* ptr ) { ((Profiler*)ptr)->CompressWorker(); }
    void CompressWorker();
#endif

    void ClearQueues( tracy::moodycamel::ConsumerToken& token );
    void ClearSerial();
    DequeueStatus Dequeue( tracy::moodycamel::ConsumerToken& token );
    DequeueStatus DequeueContextSwitches( tracy::moodycamel::ConsumerToken& token, int64_t& timeStop );
    DequeueStatus DequeueSerial();
    bool CommitData();

    tracy_force_inline bool AppendData( const void* data, size_t len )
    {
        const auto ret = NeedDataSize( len );
        AppendDataUnsafe( data, len );
        return ret;
    }

    tracy_force_inline bool NeedDataSize( size_t len )
    {
        assert( len <= TargetFrameSize );
        bool ret = true;
        if( m_bufferOffset - m_bufferStart + (int)len > TargetFrameSize )
        {
            ret = CommitData();
        }
        return ret;
    }

    tracy_force_inline void AppendDataUnsafe( const void* data, size_t len )
    {
        memcpy( m_buffer + m_bufferOffset, data, len );
        m_bufferOffset += int( len );
    }

    bool SendData( const char* data, size_t len );
    void SendLongString( uint64_t ptr, const char* str, size_t len, QueueType type );
    void SendSourceLocation( uint64_t ptr );
    void SendSourceLocationPayload( uint64_t ptr );
    void SendCallstackPayload( uint64_t ptr );
    void SendCallstackPayload64( uint64_t ptr );
    void SendCallstackAlloc( uint64_t ptr );
    void SendCallstackFrame( uint64_t ptr );
    void SendCodeLocation( uint64_t ptr );

    bool HandleServerQuery();
    void HandleDisconnect();
    void HandleParameter( uint64_t payload );
    void HandleSymbolQuery( uint64_t symbol );
    void HandleSymbolCodeQuery( uint64_t symbol, uint32_t size );
    void HandleSourceCodeQuery();

    void AckServerQuery();
    void AckSourceCodeNotAvailable();

    void CalibrateTimer();
    void CalibrateDelay();
    void ReportTopology();

    static tracy_force_inline void SendCallstackSerial( void* ptr )
    {
#ifdef TRACY_HAS_CALLSTACK
        auto item = GetProfiler().m_serialQueue.prepare_next();
        MemWrite( &item->hdr.type, QueueType::CallstackSerial );
        MemWrite( &item->callstackFat.ptr, (uint64_t)ptr );
        GetProfiler().m_serialQueue.commit_next();
#endif
    }

    static tracy_force_inline void SendMemAlloc( QueueType type, const uint64_t thread, const void* ptr, size_t size )
    {
        assert( type == QueueType::MemAlloc || type == QueueType::MemAllocCallstack || type == QueueType::MemAllocNamed || type == QueueType::MemAllocCallstackNamed );

        auto item = GetProfiler().m_serialQueue.prepare_next();
        MemWrite( &item->hdr.type, type );
        MemWrite( &item->memAlloc.time, GetTime() );
        MemWrite( &item->memAlloc.thread, thread );
        MemWrite( &item->memAlloc.ptr, (uint64_t)ptr );
        if( compile_time_condition<sizeof( size ) == 4>::value )
        {
            memcpy( &item->memAlloc.size, &size, 4 );
            memset( &item->memAlloc.size + 4, 0, 2 );
        }
        else
        {
            assert( sizeof( size ) == 8 );
            memcpy( &item->memAlloc.size, &size, 4 );
            memcpy( ((char*)&item->memAlloc.size)+4, ((char*)&size)+4, 2 );
        }
        GetProfiler().m_serialQueue.commit_next();
    }

    static tracy_force_inline void SendMemFree( QueueType type, const uint64_t thread, const void* ptr )
    {
        assert( type == QueueType::MemFree || type == QueueType::MemFreeCallstack || type == QueueType::MemFreeNamed || type == QueueType::MemFreeCallstackNamed );

        auto item = GetProfiler().m_serialQueue.prepare_next();
        MemWrite( &item->hdr.type, type );
        MemWrite( &item->memFree.time, GetTime() );
        MemWrite( &item->memFree.thread, thread );
        MemWrite( &item->memFree.ptr, (uint64_t)ptr );
        GetProfiler().m_serialQueue.commit_next();
    }

    static tracy_force_inline void SendMemName( const char* name )
    {
        assert( name );
        auto item = GetProfiler().m_serialQueue.prepare_next();
        MemWrite( &item->hdr.type, QueueType::MemNamePayload );
        MemWrite( &item->memName.name, (uint64_t)name );
        GetProfiler().m_serialQueue.commit_next();
    }

#if ( defined _WIN32 || defined __CYGWIN__ ) && defined TRACY_TIMER_QPC
    static int64_t GetTimeQpc();
#endif

    double m_timerMul;
    uint64_t m_resolution;
    uint64_t m_delay;
    std::atomic<int64_t> m_timeBegin;
    uint64_t m_mainThread;
    uint64_t m_epoch, m_exectime;
    std::atomic<bool> m_shutdown;
    std::atomic<bool> m_shutdownManual;
    std::atomic<bool> m_shutdownFinished;
    Socket* m_sock;
    UdpBroadcast* m_broadcast;
    bool m_noExit;
    uint32_t m_userPort;
    std::atomic<uint32_t> m_zoneId;
    int64_t m_samplingPeriod;

    uint64_t m_threadCtx;
    int64_t m_refTimeThread;
    int64_t m_refTimeSerial;
    int64_t m_refTimeCtx;
    int64_t m_refTimeGpu;

    void* m_stream;     // LZ4_stream_t*
    char* m_buffer;
    int m_bufferOffset;
    int m_bufferStart;

    char* m_lz4Buf;

    FastVector<QueueItem> m_serialQueue, m_serialDequeue;
    TracyMutex m_serialLock;

#ifndef TRACY_NO_FRAME_IMAGE
    FastVector<FrameImageQueueItem> m_fiQueue, m_fiDequeue;
    TracyMutex m_fiLock;
#endif

    std::atomic<uint64_t> m_frameCount;
    std::atomic<bool> m_isConnected;
#ifdef TRACY_ON_DEMAND
    std::atomic<uint64_t> m_connectionId;

    TracyMutex m_deferredLock;
    FastVector<QueueItem> m_deferredQueue;
#endif

#ifdef TRACY_HAS_SYSTIME
    void ProcessSysTime();

    SysTime m_sysTime;
    uint64_t m_sysTimeLast = 0;
#else
    void ProcessSysTime() {}
#endif

    ParameterCallback m_paramCallback;

    char* m_queryData;
    char* m_queryDataPtr;

#if defined _WIN32 || defined __CYGWIN__
    void* m_exceptionHandler;
#endif
#ifdef __linux__
    struct {
        struct sigaction pwr, ill, fpe, segv, pipe, bus, abrt;
    } m_prevSignal;
#endif
    bool m_crashHandlerInstalled;
};

}

#endif

/*** End of inlined file: TracyProfiler.hpp ***/

namespace tracy
{

class ScopedZone
{
public:
    ScopedZone( const ScopedZone& ) = delete;
    ScopedZone( ScopedZone&& ) = delete;
    ScopedZone& operator=( const ScopedZone& ) = delete;
    ScopedZone& operator=( ScopedZone&& ) = delete;

    tracy_force_inline ScopedZone( const SourceLocationData* srcloc, bool is_active = true )
#ifdef TRACY_ON_DEMAND
        : m_active( is_active && GetProfiler().IsConnected() )
#else
        : m_active( is_active )
#endif
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        m_connectionId = GetProfiler().ConnectionId();
#endif
        TracyLfqPrepare( QueueType::ZoneBegin );
        MemWrite( &item->zoneBegin.time, Profiler::GetTime() );
        MemWrite( &item->zoneBegin.srcloc, (uint64_t)srcloc );
        TracyLfqCommit;
    }

    tracy_force_inline ScopedZone( const SourceLocationData* srcloc, int depth, bool is_active = true )
#ifdef TRACY_ON_DEMAND
        : m_active( is_active && GetProfiler().IsConnected() )
#else
        : m_active( is_active )
#endif
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        m_connectionId = GetProfiler().ConnectionId();
#endif
        GetProfiler().SendCallstack( depth );

        TracyLfqPrepare( QueueType::ZoneBeginCallstack );
        MemWrite( &item->zoneBegin.time, Profiler::GetTime() );
        MemWrite( &item->zoneBegin.srcloc, (uint64_t)srcloc );
        TracyLfqCommit;
    }

    tracy_force_inline ScopedZone( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz, const char* name, size_t nameSz, bool is_active = true )
#ifdef TRACY_ON_DEMAND
        : m_active( is_active && GetProfiler().IsConnected() )
#else
        : m_active( is_active )
#endif
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        m_connectionId = GetProfiler().ConnectionId();
#endif
        TracyLfqPrepare( QueueType::ZoneBeginAllocSrcLoc );
        const auto srcloc = Profiler::AllocSourceLocation( line, source, sourceSz, function, functionSz, name, nameSz );
        MemWrite( &item->zoneBegin.time, Profiler::GetTime() );
        MemWrite( &item->zoneBegin.srcloc, srcloc );
        TracyLfqCommit;
    }

    tracy_force_inline ScopedZone( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz, const char* name, size_t nameSz, int depth, bool is_active = true )
#ifdef TRACY_ON_DEMAND
        : m_active( is_active && GetProfiler().IsConnected() )
#else
        : m_active( is_active )
#endif
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        m_connectionId = GetProfiler().ConnectionId();
#endif
        GetProfiler().SendCallstack( depth );

        TracyLfqPrepare( QueueType::ZoneBeginAllocSrcLocCallstack );
        const auto srcloc = Profiler::AllocSourceLocation( line, source, sourceSz, function, functionSz, name, nameSz );
        MemWrite( &item->zoneBegin.time, Profiler::GetTime() );
        MemWrite( &item->zoneBegin.srcloc, srcloc );
        TracyLfqCommit;
    }

    tracy_force_inline ~ScopedZone()
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        if( GetProfiler().ConnectionId() != m_connectionId ) return;
#endif
        TracyLfqPrepare( QueueType::ZoneEnd );
        MemWrite( &item->zoneEnd.time, Profiler::GetTime() );
        TracyLfqCommit;
    }

    tracy_force_inline void Text( const char* txt, size_t size )
    {
        assert( size < std::numeric_limits<uint16_t>::max() );
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        if( GetProfiler().ConnectionId() != m_connectionId ) return;
#endif
        auto ptr = (char*)tracy_malloc( size );
        memcpy( ptr, txt, size );
        TracyLfqPrepare( QueueType::ZoneText );
        MemWrite( &item->zoneTextFat.text, (uint64_t)ptr );
        MemWrite( &item->zoneTextFat.size, (uint16_t)size );
        TracyLfqCommit;
    }

    tracy_force_inline void Name( const char* txt, size_t size )
    {
        assert( size < std::numeric_limits<uint16_t>::max() );
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        if( GetProfiler().ConnectionId() != m_connectionId ) return;
#endif
        auto ptr = (char*)tracy_malloc( size );
        memcpy( ptr, txt, size );
        TracyLfqPrepare( QueueType::ZoneName );
        MemWrite( &item->zoneTextFat.text, (uint64_t)ptr );
        MemWrite( &item->zoneTextFat.size, (uint16_t)size );
        TracyLfqCommit;
    }

    tracy_force_inline void Color( uint32_t color )
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        if( GetProfiler().ConnectionId() != m_connectionId ) return;
#endif
        TracyLfqPrepare( QueueType::ZoneColor );
        MemWrite( &item->zoneColor.r, uint8_t( ( color       ) & 0xFF ) );
        MemWrite( &item->zoneColor.g, uint8_t( ( color >> 8  ) & 0xFF ) );
        MemWrite( &item->zoneColor.b, uint8_t( ( color >> 16 ) & 0xFF ) );
        TracyLfqCommit;
    }

    tracy_force_inline void Value( uint64_t value )
    {
        if( !m_active ) return;
#ifdef TRACY_ON_DEMAND
        if( GetProfiler().ConnectionId() != m_connectionId ) return;
#endif
        TracyLfqPrepare( QueueType::ZoneValue );
        MemWrite( &item->zoneValue.value, value );
        TracyLfqCommit;
    }

    tracy_force_inline bool IsActive() const { return m_active; }

private:
    const bool m_active;

#ifdef TRACY_ON_DEMAND
    uint64_t m_connectionId;
#endif
};

}

#endif

/*** End of inlined file: TracyScoped.hpp ***/


/*** Start of inlined file: TracyThread.hpp ***/
#ifndef __TRACYTHREAD_HPP__
#define __TRACYTHREAD_HPP__

#if defined _WIN32 || defined __CYGWIN__
#  include <windows.h>
#else
#  include <pthread.h>
#endif

#ifdef TRACY_MANUAL_LIFETIME

#endif

namespace tracy
{

class ThreadExitHandler
{
public:
    ~ThreadExitHandler()
    {
#ifdef TRACY_MANUAL_LIFETIME
        rpmalloc_thread_finalize();
#endif
    }
};

#if defined _WIN32 || defined __CYGWIN__

class Thread
{
public:
    Thread( void(*func)( void* ptr ), void* ptr )
        : m_func( func )
        , m_ptr( ptr )
        , m_hnd( CreateThread( nullptr, 0, Launch, this, 0, nullptr ) )
    {}

    ~Thread()
    {
        WaitForSingleObject( m_hnd, INFINITE );
        CloseHandle( m_hnd );
    }

    HANDLE Handle() const { return m_hnd; }

private:
    static DWORD WINAPI Launch( void* ptr ) { ((Thread*)ptr)->m_func( ((Thread*)ptr)->m_ptr ); return 0; }

    void(*m_func)( void* ptr );
    void* m_ptr;
    HANDLE m_hnd;
};

#else

class Thread
{
public:
    Thread( void(*func)( void* ptr ), void* ptr )
        : m_func( func )
        , m_ptr( ptr )
    {
        pthread_create( &m_thread, nullptr, Launch, this );
    }

    ~Thread()
    {
        pthread_join( m_thread, nullptr );
    }

    pthread_t Handle() const { return m_thread; }

private:
    static void* Launch( void* ptr ) { ((Thread*)ptr)->m_func( ((Thread*)ptr)->m_ptr ); return nullptr; }
    void(*m_func)( void* ptr );
    void* m_ptr;
    pthread_t m_thread;
};

#endif

}

#endif

/*** End of inlined file: TracyThread.hpp ***/


/*** Start of inlined file: TracyArmCpuTable.hpp ***/
namespace tracy
{

#if defined __linux__ && defined __ARM_ARCH

static const char* DecodeArmImplementer( uint32_t v )
{
    static char buf[16];
    switch( v )
    {
    case 0x41: return "ARM";
    case 0x42: return "Broadcom";
    case 0x43: return "Cavium";
    case 0x44: return "DEC";
    case 0x46: return "Fujitsu";
    case 0x48: return "HiSilicon";
    case 0x49: return "Infineon";
    case 0x4d: return "Motorola";
    case 0x4e: return "Nvidia";
    case 0x50: return "Applied Micro";
    case 0x51: return "Qualcomm";
    case 0x53: return "Samsung";
    case 0x54: return "Texas Instruments";
    case 0x56: return "Marvell";
    case 0x61: return "Apple";
    case 0x66: return "Faraday";
    case 0x68: return "HXT";
    case 0x69: return "Intel";
    case 0xc0: return "Ampere Computing";
    default: break;
    }
    sprintf( buf, "0x%x", v );
    return buf;
}

static const char* DecodeArmPart( uint32_t impl, uint32_t part )
{
    static char buf[16];
    switch( impl )
    {
    case 0x41:
        switch( part )
        {
        case 0x810: return "810";
        case 0x920: return "920";
        case 0x922: return "922";
        case 0x926: return "926";
        case 0x940: return "940";
        case 0x946: return "946";
        case 0x966: return "966";
        case 0xa20: return "1020";
        case 0xa22: return "1022";
        case 0xa26: return "1026";
        case 0xb02: return "11 MPCore";
        case 0xb36: return "1136";
        case 0xb56: return "1156";
        case 0xb76: return "1176";
        case 0xc05: return " Cortex-A5";
        case 0xc07: return " Cortex-A7";
        case 0xc08: return " Cortex-A8";
        case 0xc09: return " Cortex-A9";
        case 0xc0c: return " Cortex-A12";
        case 0xc0d: return " Rockchip RK3288";
        case 0xc0e: return " Cortex-A17";
        case 0xc0f: return " Cortex-A15";
        case 0xc14: return " Cortex-R4";
        case 0xc15: return " Cortex-R5";
        case 0xc17: return " Cortex-R7";
        case 0xc18: return " Cortex-R8";
        case 0xc20: return " Cortex-M0";
        case 0xc21: return " Cortex-M1";
        case 0xc23: return " Cortex-M3";
        case 0xc24: return " Cortex-M4";
        case 0xc27: return " Cortex-M7";
        case 0xc60: return " Cortex-M0+";
        case 0xd00: return " AArch64 simulator";
        case 0xd01: return " Cortex-A32";
        case 0xd02: return " Cortex-A34";
        case 0xd03: return " Cortex-A53";
        case 0xd04: return " Cortex-A35";
        case 0xd05: return " Cortex-A55";
        case 0xd06: return " Cortex-A65";
        case 0xd07: return " Cortex-A57";
        case 0xd08: return " Cortex-A72";
        case 0xd09: return " Cortex-A73";
        case 0xd0a: return " Cortex-A75";
        case 0xd0b: return " Cortex-A76";
        case 0xd0c: return " Neoverse N1";
        case 0xd0d: return " Cortex-A77";
        case 0xd0e: return " Cortex-A76AE";
        case 0xd0f: return " AEMv8";
        case 0xd13: return " Cortex-R52";
        case 0xd20: return " Cortex-M23";
        case 0xd21: return " Cortex-M33";
        case 0xd40: return " Neoverse V1";
        case 0xd41: return " Cortex-A78";
        case 0xd42: return " Cortex-A78AE";
        case 0xd43: return " Cortex-A65AE";
        case 0xd44: return " Cortex-X1";
        case 0xd47: return " Cortex-A710";
        case 0xd48: return " Cortex-X2";
        case 0xd49: return " Neoverse N2";
        case 0xd4a: return " Neoverse E1";
        case 0xd4b: return " Cortex-A78C";
        default: break;
        }
    case 0x42:
        switch( part )
        {
        case 0xf: return " Brahma B15";
        case 0x100: return " Brahma B53";
        case 0x516: return " ThunderX2";
        default: break;
        }
    case 0x43:
        switch( part )
        {
        case 0xa0: return " ThunderX";
        case 0xa1: return " ThunderX 88XX";
        case 0xa2: return " ThunderX 81XX";
        case 0xa3: return " ThunderX 83XX";
        case 0xaf: return " ThunderX2 99xx";
        case 0xb0: return " OcteonTX2";
        case 0xb1: return " OcteonTX2 T98";
        case 0xb2: return " OcteonTX2 T96";
        case 0xb3: return " OcteonTX2 F95";
        case 0xb4: return " OcteonTX2 F95N";
        case 0xb5: return " OcteonTX2 F95MM";
        case 0xb8: return " ThunderX3 T110";
        default: break;
        }
    case 0x44:
        switch( part )
        {
        case 0xa10: return " SA110";
        case 0xa11: return " SA1100";
        default: break;
        }
    case 0x46:
        switch( part )
        {
        case 0x1: return " A64FX";
        default: break;
        }
    case 0x48:
        switch( part )
        {
        case 0xd01: return " TSV100";
        case 0xd40: return " Kirin 980";
        default: break;
        }
    case 0x4e:
        switch( part )
        {
        case 0x0: return " Denver";
        case 0x3: return " Denver 2";
        case 0x4: return " Carmel";
        default: break;
        }
    case 0x50:
        switch( part )
        {
        case 0x0: return " X-Gene";
        default: break;
        }
    case 0x51:
        switch( part )
        {
        case 0xf: return " Scorpion";
        case 0x2d: return " Scorpion";
        case 0x4d: return " Krait";
        case 0x6f: return " Krait";
        case 0x200: return " Kryo";
        case 0x201: return " Kryo Silver (Snapdragon 821)";
        case 0x205: return " Kryo Gold";
        case 0x211: return " Kryo Silver (Snapdragon 820)";
        case 0x800: return " Kryo 260 / 280 Gold";
        case 0x801: return " Kryo 260 / 280 Silver";
        case 0x802: return " Kryo 385 Gold";
        case 0x803: return " Kryo 385 Silver";
        case 0x804: return " Kryo 485 Gold";
        case 0xc00: return " Falkor";
        case 0xc01: return " Saphira";
        default: break;
        }
    case 0x53:
        switch( part )
        {
        case 0x1: return " Exynos M1/M2";
        case 0x2: return " Exynos M3";
        default: break;
        }
    case 0x56:
        switch( part )
        {
        case 0x131: return " Feroceon 88FR131";
        case 0x581: return " PJ4 / PJ4B";
        case 0x584: return " PJ4B-MP / PJ4C";
        default: break;
        }
    case 0x61:
        switch( part )
        {
        case 0x1: return " Cyclone";
        case 0x2: return " Typhoon";
        case 0x3: return " Typhoon/Capri";
        case 0x4: return " Twister";
        case 0x5: return " Twister/Elba/Malta";
        case 0x6: return " Hurricane";
        case 0x7: return " Hurricane/Myst";
        default: break;
        }
    case 0x66:
        switch( part )
        {
        case 0x526: return " FA526";
        case 0x626: return " FA626";
        default: break;
        }
    case 0x68:
        switch( part )
        {
        case 0x0: return " Phecda";
        default: break;
        }
    default: break;
    }
    sprintf( buf, " 0x%x", part );
    return buf;
}

#elif defined __APPLE__ && TARGET_OS_IPHONE == 1

static const char* DecodeIosDevice( const char* id )
{
    static const char* DeviceTable[] = {
        "i386", "32-bit simulator",
        "x86_64", "64-bit simulator",
        "iPhone1,1", "iPhone",
        "iPhone1,2", "iPhone 3G",
        "iPhone2,1", "iPhone 3GS",
        "iPhone3,1", "iPhone 4 (GSM)",
        "iPhone3,2", "iPhone 4 (GSM)",
        "iPhone3,3", "iPhone 4 (CDMA)",
        "iPhone4,1", "iPhone 4S",
        "iPhone5,1", "iPhone 5 (A1428)",
        "iPhone5,2", "iPhone 5 (A1429)",
        "iPhone5,3", "iPhone 5c (A1456/A1532)",
        "iPhone5,4", "iPhone 5c (A1507/A1516/1526/A1529)",
        "iPhone6,1", "iPhone 5s (A1433/A1533)",
        "iPhone6,2", "iPhone 5s (A1457/A1518/A1528/A1530)",
        "iPhone7,1", "iPhone 6 Plus",
        "iPhone7,2", "iPhone 6",
        "iPhone8,1", "iPhone 6S",
        "iPhone8,2", "iPhone 6S Plus",
        "iPhone8,4", "iPhone SE",
        "iPhone9,1", "iPhone 7 (CDMA)",
        "iPhone9,2", "iPhone 7 Plus (CDMA)",
        "iPhone9,3", "iPhone 7 (GSM)",
        "iPhone9,4", "iPhone 7 Plus (GSM)",
        "iPhone10,1", "iPhone 8 (CDMA)",
        "iPhone10,2", "iPhone 8 Plus (CDMA)",
        "iPhone10,3", "iPhone X (CDMA)",
        "iPhone10,4", "iPhone 8 (GSM)",
        "iPhone10,5", "iPhone 8 Plus (GSM)",
        "iPhone10,6", "iPhone X (GSM)",
        "iPhone11,2", "iPhone XS",
        "iPhone11,4", "iPhone XS Max",
        "iPhone11,6", "iPhone XS Max China",
        "iPhone11,8", "iPhone XR",
        "iPhone12,1", "iPhone 11",
        "iPhone12,3", "iPhone 11 Pro",
        "iPhone12,5", "iPhone 11 Pro Max",
        "iPhone12,8", "iPhone SE 2nd Gen",
        "iPhone13,1", "iPhone 12 Mini",
        "iPhone13,2", "iPhone 12",
        "iPhone13,3", "iPhone 12 Pro",
        "iPhone13,4", "iPhone 12 Pro Max",
        "iPad1,1", "iPad (A1219/A1337)",
        "iPad2,1", "iPad 2 (A1395)",
        "iPad2,2", "iPad 2 (A1396)",
        "iPad2,3", "iPad 2 (A1397)",
        "iPad2,4", "iPad 2 (A1395)",
        "iPad2,5", "iPad Mini (A1432)",
        "iPad2,6", "iPad Mini (A1454)",
        "iPad2,7", "iPad Mini (A1455)",
        "iPad3,1", "iPad 3 (A1416)",
        "iPad3,2", "iPad 3 (A1403)",
        "iPad3,3", "iPad 3 (A1430)",
        "iPad3,4", "iPad 4 (A1458)",
        "iPad3,5", "iPad 4 (A1459)",
        "iPad3,6", "iPad 4 (A1460)",
        "iPad4,1", "iPad Air (A1474)",
        "iPad4,2", "iPad Air (A1475)",
        "iPad4,3", "iPad Air (A1476)",
        "iPad4,4", "iPad Mini 2 (A1489)",
        "iPad4,5", "iPad Mini 2 (A1490)",
        "iPad4,6", "iPad Mini 2 (A1491)",
        "iPad4,7", "iPad Mini 3 (A1599)",
        "iPad4,8", "iPad Mini 3 (A1600)",
        "iPad4,9", "iPad Mini 3 (A1601)",
        "iPad5,1", "iPad Mini 4 (A1538)",
        "iPad5,2", "iPad Mini 4 (A1550)",
        "iPad5,3", "iPad Air 2 (A1566)",
        "iPad5,4", "iPad Air 2 (A1567)",
        "iPad6,3", "iPad Pro 9.7\" (A1673)",
        "iPad6,4", "iPad Pro 9.7\" (A1674)",
        "iPad6,5", "iPad Pro 9.7\" (A1675)",
        "iPad6,7", "iPad Pro 12.9\" (A1584)",
        "iPad6,8", "iPad Pro 12.9\" (A1652)",
        "iPad6,11", "iPad 5th gen (A1822)",
        "iPad6,12", "iPad 5th gen (A1823)",
        "iPad7,1", "iPad Pro 12.9\" 2nd gen (A1670)",
        "iPad7,2", "iPad Pro 12.9\" 2nd gen (A1671/A1821)",
        "iPad7,3", "iPad Pro 10.5\" (A1701)",
        "iPad7,4", "iPad Pro 10.5\" (A1709)",
        "iPad7,5", "iPad 6th gen (A1893)",
        "iPad7,6", "iPad 6th gen (A1954)",
        "iPad7,11", "iPad 7th gen 10.2\" (Wifi)",
        "iPad7,12", "iPad 7th gen 10.2\" (Wifi+Cellular)",
        "iPad8,1", "iPad Pro 11\" (A1980)",
        "iPad8,2", "iPad Pro 11\" (A1980)",
        "iPad8,3", "iPad Pro 11\" (A1934/A1979/A2013)",
        "iPad8,4", "iPad Pro 11\" (A1934/A1979/A2013)",
        "iPad8,5", "iPad Pro 12.9\" 3rd gen (A1876)",
        "iPad8,6", "iPad Pro 12.9\" 3rd gen (A1876)",
        "iPad8,7", "iPad Pro 12.9\" 3rd gen (A1895/A1983/A2014)",
        "iPad8,8", "iPad Pro 12.9\" 3rd gen (A1895/A1983/A2014)",
        "iPad8,9", "iPad Pro 11\" 2nd gen (Wifi)",
        "iPad8,10", "iPad Pro 11\" 2nd gen (Wifi+Cellular)",
        "iPad8,11", "iPad Pro 12.9\" 4th gen (Wifi)",
        "iPad8,12", "iPad Pro 12.9\" 4th gen (Wifi+Cellular)",
        "iPad11,1", "iPad Mini 5th gen (A2133)",
        "iPad11,2", "iPad Mini 5th gen (A2124/A2125/A2126)",
        "iPad11,3", "iPad Air 3rd gen (A2152)",
        "iPad11,4", "iPad Air 3rd gen (A2123/A2153/A2154)",
        "iPad11,6", "iPad 8th gen (WiFi)",
        "iPad11,7", "iPad 8th gen (WiFi+Cellular)",
        "iPad13,1", "iPad Air 4th gen (WiFi)",
        "iPad13,2", "iPad Air 4th gen (WiFi+Cellular)",
        "iPad13,4", "iPad Pro 11\" 3rd gen",
        "iPad13,5", "iPad Pro 11\" 3rd gen",
        "iPad13,6", "iPad Pro 11\" 3rd gen",
        "iPad13,7", "iPad Pro 11\" 3rd gen",
        "iPad13,8", "iPad Pro 12.9\" 5th gen",
        "iPad13,9", "iPad Pro 12.9\" 5th gen",
        "iPad13,10", "iPad Pro 12.9\" 5th gen",
        "iPad13,11", "iPad Pro 12.9\" 5th gen",
        "iPod1,1", "iPod Touch",
        "iPod2,1", "iPod Touch 2nd gen",
        "iPod3,1", "iPod Touch 3rd gen",
        "iPod4,1", "iPod Touch 4th gen",
        "iPod5,1", "iPod Touch 5th gen",
        "iPod7,1", "iPod Touch 6th gen",
        "iPod9,1", "iPod Touch 7th gen",
        nullptr
    };

    auto ptr = DeviceTable;
    while( *ptr )
    {
        if( strcmp( ptr[0], id ) == 0 ) return ptr[1];
        ptr += 2;
    }
    return id;
}

#endif

}

/*** End of inlined file: TracyArmCpuTable.hpp ***/


/*** Start of inlined file: TracySysTrace.hpp ***/
#ifndef __TRACYSYSTRACE_HPP__
#define __TRACYSYSTRACE_HPP__

#if !defined TRACY_NO_SYSTEM_TRACING && ( defined _WIN32 || defined __CYGWIN__ || defined __linux__ )
#  define TRACY_HAS_SYSTEM_TRACING
#endif

#ifdef TRACY_HAS_SYSTEM_TRACING

#include <cstdint>

namespace tracy
{

bool SysTraceStart( int64_t& samplingPeriod );
void SysTraceStop();
void SysTraceWorker( void* ptr );

void SysTraceSendExternalName( uint64_t thread );

}

#endif

#endif

/*** End of inlined file: TracySysTrace.hpp ***/


/*** Start of inlined file: TracyC.h ***/
#ifndef __TRACYC_HPP__
#define __TRACYC_HPP__

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

TRACY_API void ___tracy_set_thread_name( const char* name );

#define TracyCSetThreadName( name ) ___tracy_set_thread_name( name );

#ifndef TRACY_ENABLE

typedef const void* TracyCZoneCtx;

#define TracyCZone(c,x)
#define TracyCZoneN(c,x,y)
#define TracyCZoneC(c,x,y)
#define TracyCZoneNC(c,x,y,z)
#define TracyCZoneEnd(c)
#define TracyCZoneText(c,x,y)
#define TracyCZoneName(c,x,y)
#define TracyCZoneColor(c,x)
#define TracyCZoneValue(c,x)

#define TracyCAlloc(x,y)
#define TracyCFree(x)
#define TracyCSecureAlloc(x,y)
#define TracyCSecureFree(x)

#define TracyCAllocN(x,y,z)
#define TracyCFreeN(x,y)
#define TracyCSecureAllocN(x,y,z)
#define TracyCSecureFreeN(x,y)

#define TracyCFrameMark
#define TracyCFrameMarkNamed(x)
#define TracyCFrameMarkStart(x)
#define TracyCFrameMarkEnd(x)
#define TracyCFrameImage(x,y,z,w,a)

#define TracyCPlot(x,y)
#define TracyCMessage(x,y)
#define TracyCMessageL(x)
#define TracyCMessageC(x,y,z)
#define TracyCMessageLC(x,y)
#define TracyCAppInfo(x,y)

#define TracyCZoneS(x,y,z)
#define TracyCZoneNS(x,y,z,w)
#define TracyCZoneCS(x,y,z,w)
#define TracyCZoneNCS(x,y,z,w,a)

#define TracyCAllocS(x,y,z)
#define TracyCFreeS(x,y)
#define TracyCSecureAllocS(x,y,z)
#define TracyCSecureFreeS(x,y)

#define TracyCAllocNS(x,y,z,w)
#define TracyCFreeNS(x,y,z)
#define TracyCSecureAllocNS(x,y,z,w)
#define TracyCSecureFreeNS(x,y,z)

#define TracyCMessageS(x,y,z)
#define TracyCMessageLS(x,y)
#define TracyCMessageCS(x,y,z,w)
#define TracyCMessageLCS(x,y,z)

#else

#ifndef TracyConcat
#  define TracyConcat(x,y) TracyConcatIndirect(x,y)
#endif
#ifndef TracyConcatIndirect
#  define TracyConcatIndirect(x,y) x##y
#endif

struct ___tracy_source_location_data
{
    const char* name;
    const char* function;
    const char* file;
    uint32_t line;
    uint32_t color;
};

struct ___tracy_c_zone_context
{
    uint32_t id;
    int active;
};

// Some containers don't support storing const types.
// This struct, as visible to user, is immutable, so treat it as if const was declared here.
typedef /*const*/ struct ___tracy_c_zone_context TracyCZoneCtx;

TRACY_API uint64_t ___tracy_alloc_srcloc( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz );
TRACY_API uint64_t ___tracy_alloc_srcloc_name( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz, const char* name, size_t nameSz );

TRACY_API TracyCZoneCtx ___tracy_emit_zone_begin( const struct ___tracy_source_location_data* srcloc, int active );
TRACY_API TracyCZoneCtx ___tracy_emit_zone_begin_callstack( const struct ___tracy_source_location_data* srcloc, int depth, int active );
TRACY_API TracyCZoneCtx ___tracy_emit_zone_begin_alloc( uint64_t srcloc, int active );
TRACY_API TracyCZoneCtx ___tracy_emit_zone_begin_alloc_callstack( uint64_t srcloc, int depth, int active );
TRACY_API void ___tracy_emit_zone_end( TracyCZoneCtx ctx );
TRACY_API void ___tracy_emit_zone_text( TracyCZoneCtx ctx, const char* txt, size_t size );
TRACY_API void ___tracy_emit_zone_name( TracyCZoneCtx ctx, const char* txt, size_t size );
TRACY_API void ___tracy_emit_zone_color( TracyCZoneCtx ctx, uint32_t color );
TRACY_API void ___tracy_emit_zone_value( TracyCZoneCtx ctx, uint64_t value );

#if defined TRACY_HAS_CALLSTACK && defined TRACY_CALLSTACK
#  define TracyCZone( ctx, active ) static const struct ___tracy_source_location_data TracyConcat(__tracy_source_location,__LINE__) = { NULL, __func__,  __FILE__, (uint32_t)__LINE__, 0 }; TracyCZoneCtx ctx = ___tracy_emit_zone_begin_callstack( &TracyConcat(__tracy_source_location,__LINE__), TRACY_CALLSTACK, active );
#  define TracyCZoneN( ctx, name, active ) static const struct ___tracy_source_location_data TracyConcat(__tracy_source_location,__LINE__) = { name, __func__,  __FILE__, (uint32_t)__LINE__, 0 }; TracyCZoneCtx ctx = ___tracy_emit_zone_begin_callstack( &TracyConcat(__tracy_source_location,__LINE__), TRACY_CALLSTACK, active );
#  define TracyCZoneC( ctx, color, active ) static const struct ___tracy_source_location_data TracyConcat(__tracy_source_location,__LINE__) = { NULL, __func__,  __FILE__, (uint32_t)__LINE__, color }; TracyCZoneCtx ctx = ___tracy_emit_zone_begin_callstack( &TracyConcat(__tracy_source_location,__LINE__), TRACY_CALLSTACK, active );
#  define TracyCZoneNC( ctx, name, color, active ) static const struct ___tracy_source_location_data TracyConcat(__tracy_source_location,__LINE__) = { name, __func__,  __FILE__, (uint32_t)__LINE__, color }; TracyCZoneCtx ctx = ___tracy_emit_zone_begin_callstack( &TracyConcat(__tracy_source_location,__LINE__), TRACY_CALLSTACK, active );
#else
#  define TracyCZone( ctx, active ) static const struct ___tracy_source_location_data TracyConcat(__tracy_source_location,__LINE__) = { NULL, __func__,  __FILE__, (uint32_t)__LINE__, 0 }; TracyCZoneCtx ctx = ___tracy_emit_zone_begin( &TracyConcat(__tracy_source_location,__LINE__), active );
#  define TracyCZoneN( ctx, name, active ) static const struct ___tracy_source_location_data TracyConcat(__tracy_source_location,__LINE__) = { name, __func__,  __FILE__, (uint32_t)__LINE__, 0 }; TracyCZoneCtx ctx = ___tracy_emit_zone_begin( &TracyConcat(__tracy_source_location,__LINE__), active );
#  define TracyCZoneC( ctx, color, active ) static const struct ___tracy_source_location_data TracyConcat(__tracy_source_location,__LINE__) = { NULL, __func__,  __FILE__, (uint32_t)__LINE__, color }; TracyCZoneCtx ctx = ___tracy_emit_zone_begin( &TracyConcat(__tracy_source_location,__LINE__), active );
#  define TracyCZoneNC( ctx, name, color, active ) static const struct ___tracy_source_location_data TracyConcat(__tracy_source_location,__LINE__) = { name, __func__,  __FILE__, (uint32_t)__LINE__, color }; TracyCZoneCtx ctx = ___tracy_emit_zone_begin( &TracyConcat(__tracy_source_location,__LINE__), active );
#endif

#define TracyCZoneEnd( ctx ) ___tracy_emit_zone_end( ctx );

#define TracyCZoneText( ctx, txt, size ) ___tracy_emit_zone_text( ctx, txt, size );
#define TracyCZoneName( ctx, txt, size ) ___tracy_emit_zone_name( ctx, txt, size );
#define TracyCZoneColor( ctx, color ) ___tracy_emit_zone_color( ctx, color );
#define TracyCZoneValue( ctx, value ) ___tracy_emit_zone_value( ctx, value );

TRACY_API void ___tracy_emit_memory_alloc( const void* ptr, size_t size, int secure );
TRACY_API void ___tracy_emit_memory_alloc_callstack( const void* ptr, size_t size, int depth, int secure );
TRACY_API void ___tracy_emit_memory_free( const void* ptr, int secure );
TRACY_API void ___tracy_emit_memory_free_callstack( const void* ptr, int depth, int secure );
TRACY_API void ___tracy_emit_memory_alloc_named( const void* ptr, size_t size, int secure, const char* name );
TRACY_API void ___tracy_emit_memory_alloc_callstack_named( const void* ptr, size_t size, int depth, int secure, const char* name );
TRACY_API void ___tracy_emit_memory_free_named( const void* ptr, int secure, const char* name );
TRACY_API void ___tracy_emit_memory_free_callstack_named( const void* ptr, int depth, int secure, const char* name );

TRACY_API void ___tracy_emit_message( const char* txt, size_t size, int callstack );
TRACY_API void ___tracy_emit_messageL( const char* txt, int callstack );
TRACY_API void ___tracy_emit_messageC( const char* txt, size_t size, uint32_t color, int callstack );
TRACY_API void ___tracy_emit_messageLC( const char* txt, uint32_t color, int callstack );

#if defined TRACY_HAS_CALLSTACK && defined TRACY_CALLSTACK
#  define TracyCAlloc( ptr, size ) ___tracy_emit_memory_alloc_callstack( ptr, size, TRACY_CALLSTACK, 0 )
#  define TracyCFree( ptr ) ___tracy_emit_memory_free_callstack( ptr, TRACY_CALLSTACK, 0 )
#  define TracyCSecureAlloc( ptr, size ) ___tracy_emit_memory_alloc_callstack( ptr, size, TRACY_CALLSTACK, 1 )
#  define TracyCSecureFree( ptr ) ___tracy_emit_memory_free_callstack( ptr, TRACY_CALLSTACK, 1 )

#  define TracyCAllocN( ptr, size, name ) ___tracy_emit_memory_alloc_callstack_named( ptr, size, TRACY_CALLSTACK, 0, name )
#  define TracyCFreeN( ptr, name ) ___tracy_emit_memory_free_callstack_named( ptr, TRACY_CALLSTACK, 0, name )
#  define TracyCSecureAllocN( ptr, size, name ) ___tracy_emit_memory_alloc_callstack_named( ptr, size, TRACY_CALLSTACK, 1, name )
#  define TracyCSecureFreeN( ptr, name ) ___tracy_emit_memory_free_callstack_named( ptr, TRACY_CALLSTACK, 1, name )

#  define TracyCMessage( txt, size ) ___tracy_emit_message( txt, size, TRACY_CALLSTACK );
#  define TracyCMessageL( txt ) ___tracy_emit_messageL( txt, TRACY_CALLSTACK );
#  define TracyCMessageC( txt, size, color ) ___tracy_emit_messageC( txt, size, color, TRACY_CALLSTACK );
#  define TracyCMessageLC( txt, color ) ___tracy_emit_messageLC( txt, color, TRACY_CALLSTACK );
#else
#  define TracyCAlloc( ptr, size ) ___tracy_emit_memory_alloc( ptr, size, 0 );
#  define TracyCFree( ptr ) ___tracy_emit_memory_free( ptr, 0 );
#  define TracyCSecureAlloc( ptr, size ) ___tracy_emit_memory_alloc( ptr, size, 1 );
#  define TracyCSecureFree( ptr ) ___tracy_emit_memory_free( ptr, 1 );

#  define TracyCAllocN( ptr, size, name ) ___tracy_emit_memory_alloc_named( ptr, size, 0, name );
#  define TracyCFreeN( ptr, name ) ___tracy_emit_memory_free_named( ptr, 0, name );
#  define TracyCSecureAllocN( ptr, size, name ) ___tracy_emit_memory_alloc_named( ptr, size, 1, name );
#  define TracyCSecureFreeN( ptr, name ) ___tracy_emit_memory_free_named( ptr, 1, name );

#  define TracyCMessage( txt, size ) ___tracy_emit_message( txt, size, 0 );
#  define TracyCMessageL( txt ) ___tracy_emit_messageL( txt, 0 );
#  define TracyCMessageC( txt, size, color ) ___tracy_emit_messageC( txt, size, color, 0 );
#  define TracyCMessageLC( txt, color ) ___tracy_emit_messageLC( txt, color, 0 );
#endif

TRACY_API void ___tracy_emit_frame_mark( const char* name );
TRACY_API void ___tracy_emit_frame_mark_start( const char* name );
TRACY_API void ___tracy_emit_frame_mark_end( const char* name );
TRACY_API void ___tracy_emit_frame_image( const void* image, uint16_t w, uint16_t h, uint8_t offset, int flip );

#define TracyCFrameMark ___tracy_emit_frame_mark( 0 );
#define TracyCFrameMarkNamed( name ) ___tracy_emit_frame_mark( name );
#define TracyCFrameMarkStart( name ) ___tracy_emit_frame_mark_start( name );
#define TracyCFrameMarkEnd( name ) ___tracy_emit_frame_mark_end( name );
#define TracyCFrameImage( image, width, height, offset, flip ) ___tracy_emit_frame_image( image, width, height, offset, flip );

TRACY_API void ___tracy_emit_plot( const char* name, double val );
TRACY_API void ___tracy_emit_message_appinfo( const char* txt, size_t size );

#define TracyCPlot( name, val ) ___tracy_emit_plot( name, val );
#define TracyCAppInfo( txt, color ) ___tracy_emit_message_appinfo( txt, color );

#ifdef TRACY_HAS_CALLSTACK
#  define TracyCZoneS( ctx, depth, active ) static const struct ___tracy_source_location_data TracyConcat(__tracy_source_location,__LINE__) = { NULL, __func__,  __FILE__, (uint32_t)__LINE__, 0 }; TracyCZoneCtx ctx = ___tracy_emit_zone_begin_callstack( &TracyConcat(__tracy_source_location,__LINE__), depth, active );
#  define TracyCZoneNS( ctx, name, depth, active ) static const struct ___tracy_source_location_data TracyConcat(__tracy_source_location,__LINE__) = { name, __func__,  __FILE__, (uint32_t)__LINE__, 0 }; TracyCZoneCtx ctx = ___tracy_emit_zone_begin_callstack( &TracyConcat(__tracy_source_location,__LINE__), depth, active );
#  define TracyCZoneCS( ctx, color, depth, active ) static const struct ___tracy_source_location_data TracyConcat(__tracy_source_location,__LINE__) = { NULL, __func__,  __FILE__, (uint32_t)__LINE__, color }; TracyCZoneCtx ctx = ___tracy_emit_zone_begin_callstack( &TracyConcat(__tracy_source_location,__LINE__), depth, active );
#  define TracyCZoneNCS( ctx, name, color, depth, active ) static const struct ___tracy_source_location_data TracyConcat(__tracy_source_location,__LINE__) = { name, __func__,  __FILE__, (uint32_t)__LINE__, color }; TracyCZoneCtx ctx = ___tracy_emit_zone_begin_callstack( &TracyConcat(__tracy_source_location,__LINE__), depth, active );

#  define TracyCAllocS( ptr, size, depth ) ___tracy_emit_memory_alloc_callstack( ptr, size, depth, 0 )
#  define TracyCFreeS( ptr, depth ) ___tracy_emit_memory_free_callstack( ptr, depth, 0 )
#  define TracyCSecureAllocS( ptr, size, depth ) ___tracy_emit_memory_alloc_callstack( ptr, size, depth, 1 )
#  define TracyCSecureFreeS( ptr, depth ) ___tracy_emit_memory_free_callstack( ptr, depth, 1 )

#  define TracyCAllocNS( ptr, size, depth, name ) ___tracy_emit_memory_alloc_callstack_named( ptr, size, depth, 0, name )
#  define TracyCFreeNS( ptr, depth, name ) ___tracy_emit_memory_free_callstack_named( ptr, depth, 0, name )
#  define TracyCSecureAllocNS( ptr, size, depth, name ) ___tracy_emit_memory_alloc_callstack_named( ptr, size, depth, 1, name )
#  define TracyCSecureFreeNS( ptr, depth, name ) ___tracy_emit_memory_free_callstack_named( ptr, depth, 1, name )

#  define TracyCMessageS( txt, size, depth ) ___tracy_emit_message( txt, size, depth );
#  define TracyCMessageLS( txt, depth ) ___tracy_emit_messageL( txt, depth );
#  define TracyCMessageCS( txt, size, color, depth ) ___tracy_emit_messageC( txt, size, color, depth );
#  define TracyCMessageLCS( txt, color, depth ) ___tracy_emit_messageLC( txt, color, depth );
#else
#  define TracyCZoneS( ctx, depth, active ) TracyCZone( ctx, active )
#  define TracyCZoneNS( ctx, name, depth, active ) TracyCZoneN( ctx, name, active )
#  define TracyCZoneCS( ctx, color, depth, active ) TracyCZoneC( ctx, color, active )
#  define TracyCZoneNCS( ctx, name, color, depth, active ) TracyCZoneNC( ctx, name, color, active )

#  define TracyCAllocS( ptr, size, depth ) TracyCAlloc( ptr, size )
#  define TracyCFreeS( ptr, depth ) TracyCFree( ptr )
#  define TracyCSecureAllocS( ptr, size, depth ) TracyCSecureAlloc( ptr, size )
#  define TracyCSecureFreeS( ptr, depth ) TracyCSecureFree( ptr )

#  define TracyCAllocNS( ptr, size, depth, name ) TracyCAllocN( ptr, size, name )
#  define TracyCFreeNS( ptr, depth, name ) TracyCFreeN( ptr, name )
#  define TracyCSecureAllocNS( ptr, size, depth, name ) TracyCSecureAllocN( ptr, size, name )
#  define TracyCSecureFreeNS( ptr, depth, name ) TracyCSecureFreeN( ptr, name )

#  define TracyCMessageS( txt, size, depth ) TracyCMessage( txt, size )
#  define TracyCMessageLS( txt, depth ) TracyCMessageL( txt )
#  define TracyCMessageCS( txt, size, color, depth ) TracyCMessageC( txt, size, color )
#  define TracyCMessageLCS( txt, color, depth ) TracyCMessageLC( txt, color )
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif

/*** End of inlined file: TracyC.h ***/

#ifdef TRACY_PORT
#  ifndef TRACY_DATA_PORT
#    define TRACY_DATA_PORT TRACY_PORT
#  endif
#  ifndef TRACY_BROADCAST_PORT
#    define TRACY_BROADCAST_PORT TRACY_PORT
#  endif
#endif

#ifdef __APPLE__
#  define TRACY_DELAYED_INIT
#else
#  ifdef __GNUC__
#    define init_order( val ) __attribute__ ((init_priority(val)))
#  else
#    define init_order(x)
#  endif
#endif

#if defined _WIN32 || defined __CYGWIN__
#  include <lmcons.h>
extern "C" typedef LONG (WINAPI *t_RtlGetVersion)( PRTL_OSVERSIONINFOW );
extern "C" typedef BOOL (WINAPI *t_GetLogicalProcessorInformationEx)( LOGICAL_PROCESSOR_RELATIONSHIP, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX, PDWORD );
#else
#  include <unistd.h>
#  include <climits>
#endif
#if defined __linux__
#  include <sys/sysinfo.h>
#  include <sys/utsname.h>
#endif

#if !defined _WIN32 && !defined __CYGWIN__ && ( defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64 )
#  include <cpuid.h>
#endif

#if !( ( ( defined _WIN32 || defined __CYGWIN__ ) && _WIN32_WINNT >= _WIN32_WINNT_VISTA ) || defined __linux__ )
#  include <mutex>
#endif

namespace tracy
{

#ifndef TRACY_DELAYED_INIT

struct InitTimeWrapper
{
    int64_t val;
};

struct ProducerWrapper
{
    tracy::moodycamel::ConcurrentQueue<QueueItem>::ExplicitProducer* ptr;
};

struct ThreadHandleWrapper
{
    uint64_t val;
};
#endif

#if defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64
static inline void CpuId( uint32_t* regs, uint32_t leaf )
{
    memset(regs, 0, sizeof(uint32_t) * 4);
#if defined _WIN32 || defined __CYGWIN__
    __cpuidex( (int*)regs, leaf, 0 );
#else
    __get_cpuid( leaf, regs, regs+1, regs+2, regs+3 );
#endif
}

static void InitFailure( const char* msg )
{
#if defined _WIN32 || defined __CYGWIN__
    bool hasConsole = false;
    bool reopen = false;
    const auto attached = AttachConsole( ATTACH_PARENT_PROCESS );
    if( attached )
    {
        hasConsole = true;
        reopen = true;
    }
    else
    {
        const auto err = GetLastError();
        if( err == ERROR_ACCESS_DENIED )
        {
            hasConsole = true;
        }
    }
    if( hasConsole )
    {
        fprintf( stderr, "Tracy Profiler initialization failure: %s\n", msg );
        if( reopen )
        {
            freopen( "CONOUT$", "w", stderr );
            fprintf( stderr, "Tracy Profiler initialization failure: %s\n", msg );
        }
    }
    else
    {
        MessageBoxA( nullptr, msg, "Tracy Profiler initialization failure", MB_ICONSTOP );
    }
#else
    fprintf( stderr, "Tracy Profiler initialization failure: %s\n", msg );
#endif
    exit( 0 );
}

static int64_t SetupHwTimer()
{
#if !defined TRACY_TIMER_QPC && !defined TRACY_TIMER_FALLBACK
    uint32_t regs[4];
    CpuId( regs, 1 );
    if( !( regs[3] & ( 1 << 4 ) ) ) InitFailure( "CPU doesn't support RDTSC instruction." );
    CpuId( regs, 0x80000007 );
    if( !( regs[3] & ( 1 << 8 ) ) )
    {
        const char* noCheck = GetEnvVar( "TRACY_NO_INVARIANT_CHECK" );
        if( !noCheck || noCheck[0] != '1' )
        {
#if defined _WIN32 || defined __CYGWIN__
            InitFailure( "CPU doesn't support invariant TSC.\nDefine TRACY_NO_INVARIANT_CHECK=1 to ignore this error, *if you know what you are doing*.\nAlternatively you may rebuild the application with the TRACY_TIMER_QPC or TRACY_TIMER_FALLBACK define to use lower resolution timer." );
#else
            InitFailure( "CPU doesn't support invariant TSC.\nDefine TRACY_NO_INVARIANT_CHECK=1 to ignore this error, *if you know what you are doing*.\nAlternatively you may rebuild the application with the TRACY_TIMER_FALLBACK define to use lower resolution timer." );
#endif
        }
    }
#endif

    return Profiler::GetTime();
}
#else
static int64_t SetupHwTimer()
{
    return Profiler::GetTime();
}
#endif

static const char* GetProcessName()
{
    const char* processName = "unknown";
#ifdef _WIN32
    static char buf[_MAX_PATH];
    GetModuleFileNameA( nullptr, buf, _MAX_PATH );
    const char* ptr = buf;
    while( *ptr != '\0' ) ptr++;
    while( ptr > buf && *ptr != '\\' && *ptr != '/' ) ptr--;
    if( ptr > buf ) ptr++;
    processName = ptr;
#elif defined __ANDROID__
#  if __ANDROID_API__ >= 21
    auto buf = getprogname();
    if( buf ) processName = buf;
#  endif
#elif defined _GNU_SOURCE || defined __CYGWIN__
    if( program_invocation_short_name ) processName = program_invocation_short_name;
#elif defined __APPLE__ || defined BSD
    auto buf = getprogname();
    if( buf ) processName = buf;
#endif
    return processName;
}

static const char* GetProcessExecutablePath()
{
#ifdef _WIN32
    static char buf[_MAX_PATH];
    GetModuleFileNameA( nullptr, buf, _MAX_PATH );
    return buf;
#elif defined __ANDROID__
    return nullptr;
#elif defined _GNU_SOURCE || defined __CYGWIN__
    return program_invocation_name;
#elif defined __APPLE__
    static char buf[1024];
    uint32_t size = 1024;
    _NSGetExecutablePath( buf, &size );
    return buf;
#elif defined __DragonFly__
    static char buf[1024];
    readlink( "/proc/curproc/file", buf, 1024 );
    return buf;
#elif defined __FreeBSD__
    static char buf[1024];
    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = -1;
    size_t cb = 1024;
    sysctl( mib, 4, buf, &cb, nullptr, 0 );
    return buf;
#elif defined __NetBSD__
    static char buf[1024];
    readlink( "/proc/curproc/exe", buf, 1024 );
    return buf;
#else
    return nullptr;
#endif
}

#if defined __linux__ && defined __ARM_ARCH
static uint32_t GetHex( char*& ptr, int skip )
{
    uint32_t ret;
    ptr += skip;
    char* end;
    if( ptr[0] == '0' && ptr[1] == 'x' )
    {
        ptr += 2;
        ret = strtol( ptr, &end, 16 );
    }
    else
    {
        ret = strtol( ptr, &end, 10 );
    }
    ptr = end;
    return ret;
}
#endif

static const char* GetHostInfo()
{
    static char buf[1024];
    auto ptr = buf;
#if defined _WIN32 || defined __CYGWIN__
    t_RtlGetVersion RtlGetVersion = (t_RtlGetVersion)GetProcAddress( GetModuleHandleA( "ntdll.dll" ), "RtlGetVersion" );
    if( !RtlGetVersion )
    {
#  ifdef __CYGWIN__
        ptr += sprintf( ptr, "OS: Windows (Cygwin)\n" );
#  elif defined __MINGW32__
        ptr += sprintf( ptr, "OS: Windows (MingW)\n" );
#  else
        ptr += sprintf( ptr, "OS: Windows\n" );
#  endif
    }
    else
    {
        RTL_OSVERSIONINFOW ver = { sizeof( RTL_OSVERSIONINFOW ) };
        RtlGetVersion( &ver );

#  ifdef __CYGWIN__
        ptr += sprintf( ptr, "OS: Windows %i.%i.%i (Cygwin)\n", ver.dwMajorVersion, ver.dwMinorVersion, ver.dwBuildNumber );
#  elif defined __MINGW32__
        ptr += sprintf( ptr, "OS: Windows %i.%i.%i (MingW)\n", (int)ver.dwMajorVersion, (int)ver.dwMinorVersion, (int)ver.dwBuildNumber );
#  else
        ptr += sprintf( ptr, "OS: Windows %i.%i.%i\n", ver.dwMajorVersion, ver.dwMinorVersion, ver.dwBuildNumber );
#  endif
    }
#elif defined __linux__
    struct utsname utsName;
    uname( &utsName );
#  if defined __ANDROID__
    ptr += sprintf( ptr, "OS: Linux %s (Android)\n", utsName.release );
#  else
    ptr += sprintf( ptr, "OS: Linux %s\n", utsName.release );
#  endif
#elif defined __APPLE__
#  if TARGET_OS_IPHONE == 1
    ptr += sprintf( ptr, "OS: Darwin (iOS)\n" );
#  elif TARGET_OS_MAC == 1
    ptr += sprintf( ptr, "OS: Darwin (OSX)\n" );
#  else
    ptr += sprintf( ptr, "OS: Darwin (unknown)\n" );
#  endif
#elif defined __DragonFly__
    ptr += sprintf( ptr, "OS: BSD (DragonFly)\n" );
#elif defined __FreeBSD__
    ptr += sprintf( ptr, "OS: BSD (FreeBSD)\n" );
#elif defined __NetBSD__
    ptr += sprintf( ptr, "OS: BSD (NetBSD)\n" );
#elif defined __OpenBSD__
    ptr += sprintf( ptr, "OS: BSD (OpenBSD)\n" );
#else
    ptr += sprintf( ptr, "OS: unknown\n" );
#endif

#if defined _MSC_VER
#  if defined __clang__
    ptr += sprintf( ptr, "Compiler: MSVC clang-cl %i.%i.%i\n", __clang_major__, __clang_minor__, __clang_patchlevel__ );
#  else
    ptr += sprintf( ptr, "Compiler: MSVC %i\n", _MSC_VER );
#  endif
#elif defined __clang__
    ptr += sprintf( ptr, "Compiler: clang %i.%i.%i\n", __clang_major__, __clang_minor__, __clang_patchlevel__ );
#elif defined __GNUC__
    ptr += sprintf( ptr, "Compiler: gcc %i.%i\n", __GNUC__, __GNUC_MINOR__ );
#else
    ptr += sprintf( ptr, "Compiler: unknown\n" );
#endif

#if defined _WIN32 || defined __CYGWIN__
#  ifndef __CYGWIN__
    InitWinSock();
#  endif
    char hostname[512];
    gethostname( hostname, 512 );

    DWORD userSz = UNLEN+1;
    char user[UNLEN+1];
    GetUserNameA( user, &userSz );

    ptr += sprintf( ptr, "User: %s@%s\n", user, hostname );
#else
    char hostname[_POSIX_HOST_NAME_MAX]{};
    char user[_POSIX_LOGIN_NAME_MAX]{};

    gethostname( hostname, _POSIX_HOST_NAME_MAX );
#  if defined __ANDROID__
    const auto login = getlogin();
    if( login )
    {
        strcpy( user, login );
    }
    else
    {
        memcpy( user, "(?)", 4 );
    }
#  else
    getlogin_r( user, _POSIX_LOGIN_NAME_MAX );
#  endif

    ptr += sprintf( ptr, "User: %s@%s\n", user, hostname );
#endif

#if defined __i386 || defined _M_IX86
    ptr += sprintf( ptr, "Arch: x86\n" );
#elif defined __x86_64__ || defined _M_X64
    ptr += sprintf( ptr, "Arch: x64\n" );
#elif defined __aarch64__
    ptr += sprintf( ptr, "Arch: ARM64\n" );
#elif defined __ARM_ARCH
    ptr += sprintf( ptr, "Arch: ARM\n" );
#else
    ptr += sprintf( ptr, "Arch: unknown\n" );
#endif

#if defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64
    uint32_t regs[4];
    char cpuModel[4*4*3];
    auto modelPtr = cpuModel;
    for( uint32_t i=0x80000002; i<0x80000005; ++i )
    {
        CpuId( regs, i );
        memcpy( modelPtr, regs, sizeof( regs ) ); modelPtr += sizeof( regs );
    }

    ptr += sprintf( ptr, "CPU: %s\n", cpuModel );
#elif defined __linux__ && defined __ARM_ARCH
    bool cpuFound = false;
    FILE* fcpuinfo = fopen( "/proc/cpuinfo", "rb" );
    if( fcpuinfo )
    {
        enum { BufSize = 4*1024 };
        char buf[BufSize];
        const auto sz = fread( buf, 1, BufSize, fcpuinfo );
        fclose( fcpuinfo );
        const auto end = buf + sz;
        auto cptr = buf;

        uint32_t impl = 0;
        uint32_t var = 0;
        uint32_t part = 0;
        uint32_t rev = 0;

        while( end - cptr > 20 )
        {
            while( end - cptr > 20 && memcmp( cptr, "CPU ", 4 ) != 0 )
            {
                cptr += 4;
                while( end - cptr > 20 && *cptr != '\n' ) cptr++;
                cptr++;
            }
            if( end - cptr <= 20 ) break;
            cptr += 4;
            if( memcmp( cptr, "implementer\t: ", 14 ) == 0 )
            {
                if( impl != 0 ) break;
                impl = GetHex( cptr, 14 );
            }
            else if( memcmp( cptr, "variant\t: ", 10 ) == 0 ) var = GetHex( cptr, 10 );
            else if( memcmp( cptr, "part\t: ", 7 ) == 0 ) part = GetHex( cptr, 7 );
            else if( memcmp( cptr, "revision\t: ", 11 ) == 0 ) rev = GetHex( cptr, 11 );
            while( *cptr != '\n' && *cptr != '\0' ) cptr++;
            cptr++;
        }

        if( impl != 0 || var != 0 || part != 0 || rev != 0 )
        {
            cpuFound = true;
            ptr += sprintf( ptr, "CPU: %s%s r%ip%i\n", DecodeArmImplementer( impl ), DecodeArmPart( impl, part ), var, rev );
        }
    }
    if( !cpuFound )
    {
        ptr += sprintf( ptr, "CPU: unknown\n" );
    }
#elif defined __APPLE__ && TARGET_OS_IPHONE == 1
    {
        size_t sz;
        sysctlbyname( "hw.machine", nullptr, &sz, nullptr, 0 );
        auto str = (char*)tracy_malloc( sz );
        sysctlbyname( "hw.machine", str, &sz, nullptr, 0 );
        ptr += sprintf( ptr, "Device: %s\n", DecodeIosDevice( str ) );
        tracy_free( str );
    }
#else
    ptr += sprintf( ptr, "CPU: unknown\n" );
#endif

    ptr += sprintf( ptr, "CPU cores: %i\n", std::thread::hardware_concurrency() );

#if defined _WIN32 || defined __CYGWIN__
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof( statex );
    GlobalMemoryStatusEx( &statex );
#  ifdef _MSC_VER
    ptr += sprintf( ptr, "RAM: %I64u MB\n", statex.ullTotalPhys / 1024 / 1024 );
#  else
    ptr += sprintf( ptr, "RAM: %llu MB\n", statex.ullTotalPhys / 1024 / 1024 );
#  endif
#elif defined __linux__
    struct sysinfo sysInfo;
    sysinfo( &sysInfo );
    ptr += sprintf( ptr, "RAM: %lu MB\n", sysInfo.totalram / 1024 / 1024 );
#elif defined __APPLE__
    size_t memSize;
    size_t sz = sizeof( memSize );
    sysctlbyname( "hw.memsize", &memSize, &sz, nullptr, 0 );
    ptr += sprintf( ptr, "RAM: %zu MB\n", memSize / 1024 / 1024 );
#elif defined BSD
    size_t memSize;
    size_t sz = sizeof( memSize );
    sysctlbyname( "hw.physmem", &memSize, &sz, nullptr, 0 );
    ptr += sprintf( ptr, "RAM: %zu MB\n", memSize / 1024 / 1024 );
#else
    ptr += sprintf( ptr, "RAM: unknown\n" );
#endif

    return buf;
}

static uint64_t GetPid()
{
#if defined _WIN32 || defined __CYGWIN__
    return uint64_t( GetCurrentProcessId() );
#else
    return uint64_t( getpid() );
#endif
}

void Profiler::AckServerQuery()
{
    QueueItem item;
    MemWrite( &item.hdr.type, QueueType::AckServerQueryNoop );
    NeedDataSize( QueueDataSize[(int)QueueType::AckServerQueryNoop] );
    AppendDataUnsafe( &item, QueueDataSize[(int)QueueType::AckServerQueryNoop] );
}

void Profiler::AckSourceCodeNotAvailable()
{
    QueueItem item;
    MemWrite( &item.hdr.type, QueueType::AckSourceCodeNotAvailable );
    NeedDataSize( QueueDataSize[(int)QueueType::AckSourceCodeNotAvailable] );
    AppendDataUnsafe( &item, QueueDataSize[(int)QueueType::AckSourceCodeNotAvailable] );
}

static BroadcastMessage& GetBroadcastMessage( const char* procname, size_t pnsz, int& len, int port )
{
    static BroadcastMessage msg;

    msg.broadcastVersion = BroadcastVersion;
    msg.protocolVersion = ProtocolVersion;
    msg.listenPort = port;

    memcpy( msg.programName, procname, pnsz );
    memset( msg.programName + pnsz, 0, WelcomeMessageProgramNameSize - pnsz );

    len = int( offsetof( BroadcastMessage, programName ) + pnsz + 1 );
    return msg;
}

#if defined _WIN32 || defined __CYGWIN__
static DWORD s_profilerThreadId = 0;
static char s_crashText[1024];

LONG WINAPI CrashFilter( PEXCEPTION_POINTERS pExp )
{
    if( !GetProfiler().IsConnected() ) return EXCEPTION_CONTINUE_SEARCH;

    const unsigned ec = pExp->ExceptionRecord->ExceptionCode;
    auto msgPtr = s_crashText;
    switch( ec )
    {
    case EXCEPTION_ACCESS_VIOLATION:
        msgPtr += sprintf( msgPtr, "Exception EXCEPTION_ACCESS_VIOLATION (0x%x). ", ec );
        switch( pExp->ExceptionRecord->ExceptionInformation[0] )
        {
        case 0:
            msgPtr += sprintf( msgPtr, "Read violation at address 0x%" PRIxPTR ".", pExp->ExceptionRecord->ExceptionInformation[1] );
            break;
        case 1:
            msgPtr += sprintf( msgPtr, "Write violation at address 0x%" PRIxPTR ".", pExp->ExceptionRecord->ExceptionInformation[1] );
            break;
        case 8:
            msgPtr += sprintf( msgPtr, "DEP violation at address 0x%" PRIxPTR ".", pExp->ExceptionRecord->ExceptionInformation[1] );
            break;
        default:
            break;
        }
        break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        msgPtr += sprintf( msgPtr, "Exception EXCEPTION_ARRAY_BOUNDS_EXCEEDED (0x%x). ", ec );
        break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        msgPtr += sprintf( msgPtr, "Exception EXCEPTION_DATATYPE_MISALIGNMENT (0x%x). ", ec );
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        msgPtr += sprintf( msgPtr, "Exception EXCEPTION_FLT_DIVIDE_BY_ZERO (0x%x). ", ec );
        break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        msgPtr += sprintf( msgPtr, "Exception EXCEPTION_ILLEGAL_INSTRUCTION (0x%x). ", ec );
        break;
    case EXCEPTION_IN_PAGE_ERROR:
        msgPtr += sprintf( msgPtr, "Exception EXCEPTION_IN_PAGE_ERROR (0x%x). ", ec );
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        msgPtr += sprintf( msgPtr, "Exception EXCEPTION_INT_DIVIDE_BY_ZERO (0x%x). ", ec );
        break;
    case EXCEPTION_PRIV_INSTRUCTION:
        msgPtr += sprintf( msgPtr, "Exception EXCEPTION_PRIV_INSTRUCTION (0x%x). ", ec );
        break;
    case EXCEPTION_STACK_OVERFLOW:
        msgPtr += sprintf( msgPtr, "Exception EXCEPTION_STACK_OVERFLOW (0x%x). ", ec );
        break;
    default:
        return EXCEPTION_CONTINUE_SEARCH;
    }

    {
        GetProfiler().SendCallstack( 60, "KiUserExceptionDispatcher" );

        TracyLfqPrepare( QueueType::CrashReport );
        item->crashReport.time = Profiler::GetTime();
        item->crashReport.text = (uint64_t)s_crashText;
        TracyLfqCommit;
    }

    HANDLE h = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );
    if( h == INVALID_HANDLE_VALUE ) return EXCEPTION_CONTINUE_SEARCH;

    THREADENTRY32 te = { sizeof( te ) };
    if( !Thread32First( h, &te ) )
    {
        CloseHandle( h );
        return EXCEPTION_CONTINUE_SEARCH;
    }

    const auto pid = GetCurrentProcessId();
    const auto tid = GetCurrentThreadId();

    do
    {
        if( te.th32OwnerProcessID == pid && te.th32ThreadID != tid && te.th32ThreadID != s_profilerThreadId )
        {
            HANDLE th = OpenThread( THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID );
            if( th != INVALID_HANDLE_VALUE )
            {
                SuspendThread( th );
                CloseHandle( th );
            }
        }
    }
    while( Thread32Next( h, &te ) );
    CloseHandle( h );

    {
        TracyLfqPrepare( QueueType::Crash );
        TracyLfqCommit;
    }

    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    GetProfiler().RequestShutdown();
    while( !GetProfiler().HasShutdownFinished() ) { std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) ); };

    TerminateProcess( GetCurrentProcess(), 1 );

    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

#ifdef __linux__
static long s_profilerTid = 0;
static char s_crashText[1024];
static std::atomic<bool> s_alreadyCrashed( false );

static void ThreadFreezer( int /*signal*/ )
{
    for(;;) sleep( 1000 );
}

static inline void HexPrint( char*& ptr, uint64_t val )
{
    if( val == 0 )
    {
        *ptr++ = '0';
        return;
    }

    static const char HexTable[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    char buf[16];
    auto bptr = buf;

    do
    {
        *bptr++ = HexTable[val%16];
        val /= 16;
    }
    while( val > 0 );

    do
    {
        *ptr++ = *--bptr;
    }
    while( bptr != buf );
}

static void CrashHandler( int signal, siginfo_t* info, void* /*ucontext*/ )
{
    bool expected = false;
    if( !s_alreadyCrashed.compare_exchange_strong( expected, true ) ) ThreadFreezer( signal );

    struct sigaction act = {};
    act.sa_handler = SIG_DFL;
    sigaction( SIGABRT, &act, nullptr );

    auto msgPtr = s_crashText;
    switch( signal )
    {
    case SIGILL:
        strcpy( msgPtr, "Illegal Instruction.\n" );
        while( *msgPtr ) msgPtr++;
        switch( info->si_code )
        {
        case ILL_ILLOPC:
            strcpy( msgPtr, "Illegal opcode.\n" );
            break;
        case ILL_ILLOPN:
            strcpy( msgPtr, "Illegal operand.\n" );
            break;
        case ILL_ILLADR:
            strcpy( msgPtr, "Illegal addressing mode.\n" );
            break;
        case ILL_ILLTRP:
            strcpy( msgPtr, "Illegal trap.\n" );
            break;
        case ILL_PRVOPC:
            strcpy( msgPtr, "Privileged opcode.\n" );
            break;
        case ILL_PRVREG:
            strcpy( msgPtr, "Privileged register.\n" );
            break;
        case ILL_COPROC:
            strcpy( msgPtr, "Coprocessor error.\n" );
            break;
        case ILL_BADSTK:
            strcpy( msgPtr, "Internal stack error.\n" );
            break;
        default:
            break;
        }
        break;
    case SIGFPE:
        strcpy( msgPtr, "Floating-point exception.\n" );
        while( *msgPtr ) msgPtr++;
        switch( info->si_code )
        {
        case FPE_INTDIV:
            strcpy( msgPtr, "Integer divide by zero.\n" );
            break;
        case FPE_INTOVF:
            strcpy( msgPtr, "Integer overflow.\n" );
            break;
        case FPE_FLTDIV:
            strcpy( msgPtr, "Floating-point divide by zero.\n" );
            break;
        case FPE_FLTOVF:
            strcpy( msgPtr, "Floating-point overflow.\n" );
            break;
        case FPE_FLTUND:
            strcpy( msgPtr, "Floating-point underflow.\n" );
            break;
        case FPE_FLTRES:
            strcpy( msgPtr, "Floating-point inexact result.\n" );
            break;
        case FPE_FLTINV:
            strcpy( msgPtr, "Floating-point invalid operation.\n" );
            break;
        case FPE_FLTSUB:
            strcpy( msgPtr, "Subscript out of range.\n" );
            break;
        default:
            break;
        }
        break;
    case SIGSEGV:
        strcpy( msgPtr, "Invalid memory reference.\n" );
        while( *msgPtr ) msgPtr++;
        switch( info->si_code )
        {
        case SEGV_MAPERR:
            strcpy( msgPtr, "Address not mapped to object.\n" );
            break;
        case SEGV_ACCERR:
            strcpy( msgPtr, "Invalid permissions for mapped object.\n" );
            break;
#  ifdef SEGV_BNDERR
        case SEGV_BNDERR:
            strcpy( msgPtr, "Failed address bound checks.\n" );
            break;
#  endif
#  ifdef SEGV_PKUERR
        case SEGV_PKUERR:
            strcpy( msgPtr, "Access was denied by memory protection keys.\n" );
            break;
#  endif
        default:
            break;
        }
        break;
    case SIGPIPE:
        strcpy( msgPtr, "Broken pipe.\n" );
        while( *msgPtr ) msgPtr++;
        break;
    case SIGBUS:
        strcpy( msgPtr, "Bus error.\n" );
        while( *msgPtr ) msgPtr++;
        switch( info->si_code )
        {
        case BUS_ADRALN:
            strcpy( msgPtr, "Invalid address alignment.\n" );
            break;
        case BUS_ADRERR:
            strcpy( msgPtr, "Nonexistent physical address.\n" );
            break;
        case BUS_OBJERR:
            strcpy( msgPtr, "Object-specific hardware error.\n" );
            break;
#  ifdef BUS_MCEERR_AR
        case BUS_MCEERR_AR:
            strcpy( msgPtr, "Hardware memory error consumed on a machine check; action required.\n" );
            break;
#  endif
#  ifdef BUS_MCEERR_AO
        case BUS_MCEERR_AO:
            strcpy( msgPtr, "Hardware memory error detected in process but not consumed; action optional.\n" );
            break;
#  endif
        default:
            break;
        }
        break;
    case SIGABRT:
        strcpy( msgPtr, "Abort signal from abort().\n" );
        break;
    default:
        abort();
    }
    while( *msgPtr ) msgPtr++;

    if( signal != SIGPIPE )
    {
        strcpy( msgPtr, "Fault address: 0x" );
        while( *msgPtr ) msgPtr++;
        HexPrint( msgPtr, uint64_t( info->si_addr ) );
        *msgPtr++ = '\n';
    }

    {
        GetProfiler().SendCallstack( 60, "__kernel_rt_sigreturn" );

        TracyLfqPrepare( QueueType::CrashReport );
        item->crashReport.time = Profiler::GetTime();
        item->crashReport.text = (uint64_t)s_crashText;
        TracyLfqCommit;
    }

    DIR* dp = opendir( "/proc/self/task" );
    if( !dp ) abort();

    const auto selfTid = syscall( SYS_gettid );

    struct dirent* ep;
    while( ( ep = readdir( dp ) ) != nullptr )
    {
        if( ep->d_name[0] == '.' ) continue;
        int tid = atoi( ep->d_name );
        if( tid != selfTid && tid != s_profilerTid )
        {
            syscall( SYS_tkill, tid, SIGPWR );
        }
    }
    closedir( dp );

    {
        TracyLfqPrepare( QueueType::Crash );
        TracyLfqCommit;
    }

    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    GetProfiler().RequestShutdown();
    while( !GetProfiler().HasShutdownFinished() ) { std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) ); };

    abort();
}
#endif

enum { QueuePrealloc = 256 * 1024 };

static Profiler* s_instance = nullptr;
static Thread* s_thread;
#ifndef TRACY_NO_FRAME_IMAGE
static Thread* s_compressThread;
#endif

#ifdef TRACY_HAS_SYSTEM_TRACING
static Thread* s_sysTraceThread = nullptr;
#endif

TRACY_API bool ProfilerAvailable() { return s_instance != nullptr; }

TRACY_API int64_t GetFrequencyQpc()
{
#if defined _WIN32 || defined __CYGWIN__
    LARGE_INTEGER t;
    QueryPerformanceFrequency( &t );
    return t.QuadPart;
#else
    return 0;
#endif
}

#ifdef TRACY_DELAYED_INIT
struct ThreadNameData;
TRACY_API moodycamel::ConcurrentQueue<QueueItem>& GetQueue();

struct ProfilerData
{
    int64_t initTime = SetupHwTimer();
    moodycamel::ConcurrentQueue<QueueItem> queue;
    Profiler profiler;
    std::atomic<uint32_t> lockCounter { 0 };
    std::atomic<uint8_t> gpuCtxCounter { 0 };
    std::atomic<ThreadNameData*> threadNameData { nullptr };
};

struct ProducerWrapper
{
    ProducerWrapper( ProfilerData& data ) : detail( data.queue ), ptr( data.queue.get_explicit_producer( detail ) ) {}
    moodycamel::ProducerToken detail;
    tracy::moodycamel::ConcurrentQueue<QueueItem>::ExplicitProducer* ptr;
};

struct ProfilerThreadData
{
    ProfilerThreadData( ProfilerData& data ) : token( data ), gpuCtx( { nullptr } ) {}
    ProducerWrapper token;
    GpuCtxWrapper gpuCtx;
#  ifdef TRACY_ON_DEMAND
    LuaZoneState luaZoneState;
#  endif
};

std::atomic<int> RpInitDone { 0 };
std::atomic<int> RpInitLock { 0 };
thread_local bool RpThreadInitDone = false;

#  ifdef TRACY_MANUAL_LIFETIME
ProfilerData* s_profilerData = nullptr;
static ProfilerThreadData& GetProfilerThreadData();
TRACY_API void StartupProfiler()
{
    s_profilerData = (ProfilerData*)tracy_malloc( sizeof( ProfilerData ) );
    new (s_profilerData) ProfilerData();
    s_profilerData->profiler.SpawnWorkerThreads();
    GetProfilerThreadData().token = ProducerWrapper( *s_profilerData );
}
static ProfilerData& GetProfilerData()
{
    assert( s_profilerData );
    return *s_profilerData;
}
TRACY_API void ShutdownProfiler()
{
    s_profilerData->~ProfilerData();
    tracy_free( s_profilerData );
    s_profilerData = nullptr;
    rpmalloc_finalize();
    RpThreadInitDone = false;
    RpInitDone.store( 0, std::memory_order_release );
}
#  else
static std::atomic<int> profilerDataLock { 0 };
static std::atomic<ProfilerData*> profilerData { nullptr };

static ProfilerData& GetProfilerData()
{
    auto ptr = profilerData.load( std::memory_order_acquire );
    if( !ptr )
    {
        int expected = 0;
        while( !profilerDataLock.compare_exchange_weak( expected, 1, std::memory_order_release, std::memory_order_relaxed ) ) { expected = 0; YieldThread(); }
        ptr = profilerData.load( std::memory_order_acquire );
        if( !ptr )
        {
            ptr = (ProfilerData*)tracy_malloc( sizeof( ProfilerData ) );
            new (ptr) ProfilerData();
            profilerData.store( ptr, std::memory_order_release );
        }
        profilerDataLock.store( 0, std::memory_order_release );
    }
    return *ptr;
}
#  endif

// GCC prior to 8.4 had a bug with function-inline thread_local variables. Versions of glibc beginning with
// 2.18 may attempt to work around this issue, which manifests as a crash while running static destructors
// if this function is compiled into a shared object. Unfortunately, centos7 ships with glibc 2.17. If running
// on old GCC, use the old-fashioned way as a workaround
// See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85400
#if defined(__GNUC__) && ((__GNUC__ < 8) || ((__GNUC__ == 8) && (__GNUC_MINOR__ < 4)))
struct ProfilerThreadDataKey
{
public:
    ProfilerThreadDataKey()
    {
        int val = pthread_key_create(&m_key, sDestructor);
        static_cast<void>(val); // unused
        assert(val == 0);
    }
    ~ProfilerThreadDataKey()
    {
        int val = pthread_key_delete(m_key);
        static_cast<void>(val); // unused
        assert(val == 0);
    }
    ProfilerThreadData& get()
    {
        void* p = pthread_getspecific(m_key);
        if (!p)
        {
            p = (ProfilerThreadData*)tracy_malloc( sizeof( ProfilerThreadData ) );
            new (p) ProfilerThreadData(GetProfilerData());
            pthread_setspecific(m_key, p);
        }
        return *static_cast<ProfilerThreadData*>(p);
    }
private:
    pthread_key_t m_key;

    static void sDestructor(void* p)
    {
        ((ProfilerThreadData*)p)->~ProfilerThreadData();
        tracy_free(p);
    }
};

static ProfilerThreadData& GetProfilerThreadData()
{
    static ProfilerThreadDataKey key;
    return key.get();
}
#else
static ProfilerThreadData& GetProfilerThreadData()
{
    thread_local ProfilerThreadData data( GetProfilerData() );
    return data;
}
#endif

TRACY_API moodycamel::ConcurrentQueue<QueueItem>::ExplicitProducer* GetToken() { return GetProfilerThreadData().token.ptr; }
TRACY_API Profiler& GetProfiler() { return GetProfilerData().profiler; }
TRACY_API moodycamel::ConcurrentQueue<QueueItem>& GetQueue() { return GetProfilerData().queue; }
TRACY_API int64_t GetInitTime() { return GetProfilerData().initTime; }
TRACY_API std::atomic<uint32_t>& GetLockCounter() { return GetProfilerData().lockCounter; }
TRACY_API std::atomic<uint8_t>& GetGpuCtxCounter() { return GetProfilerData().gpuCtxCounter; }
TRACY_API GpuCtxWrapper& GetGpuCtx() { return GetProfilerThreadData().gpuCtx; }
TRACY_API uint64_t GetThreadHandle() { return detail::GetThreadHandleImpl(); }
std::atomic<ThreadNameData*>& GetThreadNameData() { return GetProfilerData().threadNameData; }

#  ifdef TRACY_ON_DEMAND
TRACY_API LuaZoneState& GetLuaZoneState() { return GetProfilerThreadData().luaZoneState; }
#  endif

#  ifndef TRACY_MANUAL_LIFETIME
namespace
{
    const auto& __profiler_init = GetProfiler();
}
#  endif

#else

// MSVC static initialization order solution. gcc/clang uses init_order() to avoid all this.

// 1a. But s_queue is needed for initialization of variables in point 2.
extern moodycamel::ConcurrentQueue<QueueItem> s_queue;

// 2. If these variables would be in the .CRT$XCB section, they would be initialized only in main thread.
thread_local moodycamel::ProducerToken init_order(107) s_token_detail( s_queue );
thread_local ProducerWrapper init_order(108) s_token { s_queue.get_explicit_producer( s_token_detail ) };
thread_local ThreadHandleWrapper init_order(104) s_threadHandle { detail::GetThreadHandleImpl() };

#  ifdef _MSC_VER
// 1. Initialize these static variables before all other variables.
#    pragma warning( disable : 4075 )
#    pragma init_seg( ".CRT$XCB" )
#  endif

static InitTimeWrapper init_order(101) s_initTime { SetupHwTimer() };
std::atomic<int> init_order(102) RpInitDone( 0 );
std::atomic<int> init_order(102) RpInitLock( 0 );
thread_local bool RpThreadInitDone = false;
moodycamel::ConcurrentQueue<QueueItem> init_order(103) s_queue( QueuePrealloc );
std::atomic<uint32_t> init_order(104) s_lockCounter( 0 );
std::atomic<uint8_t> init_order(104) s_gpuCtxCounter( 0 );

thread_local GpuCtxWrapper init_order(104) s_gpuCtx { nullptr };

struct ThreadNameData;
static std::atomic<ThreadNameData*> init_order(104) s_threadNameDataInstance( nullptr );
std::atomic<ThreadNameData*>& s_threadNameData = s_threadNameDataInstance;

#  ifdef TRACY_ON_DEMAND
thread_local LuaZoneState init_order(104) s_luaZoneState { 0, false };
#  endif

static Profiler init_order(105) s_profiler;

TRACY_API moodycamel::ConcurrentQueue<QueueItem>::ExplicitProducer* GetToken() { return s_token.ptr; }
TRACY_API Profiler& GetProfiler() { return s_profiler; }
TRACY_API moodycamel::ConcurrentQueue<QueueItem>& GetQueue() { return s_queue; }
TRACY_API int64_t GetInitTime() { return s_initTime.val; }
TRACY_API std::atomic<uint32_t>& GetLockCounter() { return s_lockCounter; }
TRACY_API std::atomic<uint8_t>& GetGpuCtxCounter() { return s_gpuCtxCounter; }
TRACY_API GpuCtxWrapper& GetGpuCtx() { return s_gpuCtx; }
#  ifdef __CYGWIN__
// Hackfix for cygwin reporting memory frees without matching allocations. WTF?
TRACY_API uint64_t GetThreadHandle() { return detail::GetThreadHandleImpl(); }
#  else
TRACY_API uint64_t GetThreadHandle() { return s_threadHandle.val; }
#  endif

std::atomic<ThreadNameData*>& GetThreadNameData() { return s_threadNameData; }

#  ifdef TRACY_ON_DEMAND
TRACY_API LuaZoneState& GetLuaZoneState() { return s_luaZoneState; }
#  endif
#endif

Profiler::Profiler()
    : m_timeBegin( 0 )
    , m_mainThread( detail::GetThreadHandleImpl() )
    , m_epoch( std::chrono::duration_cast<std::chrono::seconds>( std::chrono::system_clock::now().time_since_epoch() ).count() )
    , m_shutdown( false )
    , m_shutdownManual( false )
    , m_shutdownFinished( false )
    , m_sock( nullptr )
    , m_broadcast( nullptr )
    , m_noExit( false )
    , m_userPort( 0 )
    , m_zoneId( 1 )
    , m_samplingPeriod( 0 )
    , m_stream( LZ4_createStream() )
    , m_buffer( (char*)tracy_malloc( TargetFrameSize*3 ) )
    , m_bufferOffset( 0 )
    , m_bufferStart( 0 )
    , m_lz4Buf( (char*)tracy_malloc( LZ4Size + sizeof( lz4sz_t ) ) )
    , m_serialQueue( 1024*1024 )
    , m_serialDequeue( 1024*1024 )
#ifndef TRACY_NO_FRAME_IMAGE
    , m_fiQueue( 16 )
    , m_fiDequeue( 16 )
#endif
    , m_frameCount( 0 )
    , m_isConnected( false )
#ifdef TRACY_ON_DEMAND
    , m_connectionId( 0 )
    , m_deferredQueue( 64*1024 )
#endif
    , m_paramCallback( nullptr )
    , m_queryData( nullptr )
    , m_crashHandlerInstalled( false )
{
    assert( !s_instance );
    s_instance = this;

#ifndef TRACY_DELAYED_INIT
#  ifdef _MSC_VER
    // 3. But these variables need to be initialized in main thread within the .CRT$XCB section. Do it here.
    s_token_detail = moodycamel::ProducerToken( s_queue );
    s_token = ProducerWrapper { s_queue.get_explicit_producer( s_token_detail ) };
    s_threadHandle = ThreadHandleWrapper { m_mainThread };
#  endif
#endif

    CalibrateTimer();
    CalibrateDelay();
    ReportTopology();

#ifndef TRACY_NO_EXIT
    const char* noExitEnv = GetEnvVar( "TRACY_NO_EXIT" );
    if( noExitEnv && noExitEnv[0] == '1' )
    {
        m_noExit = true;
    }
#endif

    const char* userPort = GetEnvVar( "TRACY_PORT" );
    if( userPort )
    {
        m_userPort = atoi( userPort );
    }

#if !defined(TRACY_DELAYED_INIT) || !defined(TRACY_MANUAL_LIFETIME)
    SpawnWorkerThreads();
#endif
}

void Profiler::SpawnWorkerThreads()
{
#ifdef TRACY_HAS_SYSTEM_TRACING
    if( SysTraceStart( m_samplingPeriod ) )
    {
        s_sysTraceThread = (Thread*)tracy_malloc( sizeof( Thread ) );
        new(s_sysTraceThread) Thread( SysTraceWorker, nullptr );
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }
#endif

    s_thread = (Thread*)tracy_malloc( sizeof( Thread ) );
    new(s_thread) Thread( LaunchWorker, this );

#ifndef TRACY_NO_FRAME_IMAGE
    s_compressThread = (Thread*)tracy_malloc( sizeof( Thread ) );
    new(s_compressThread) Thread( LaunchCompressWorker, this );
#endif

#if defined _WIN32 || defined __CYGWIN__
    s_profilerThreadId = GetThreadId( s_thread->Handle() );
    m_exceptionHandler = AddVectoredExceptionHandler( 1, CrashFilter );
#endif

#ifdef __linux__
    struct sigaction threadFreezer = {};
    threadFreezer.sa_handler = ThreadFreezer;
    sigaction( SIGPWR, &threadFreezer, &m_prevSignal.pwr );

    struct sigaction crashHandler = {};
    crashHandler.sa_sigaction = CrashHandler;
    crashHandler.sa_flags = SA_SIGINFO;
    sigaction( SIGILL, &crashHandler, &m_prevSignal.ill );
    sigaction( SIGFPE, &crashHandler, &m_prevSignal.fpe );
    sigaction( SIGSEGV, &crashHandler, &m_prevSignal.segv );
    sigaction( SIGPIPE, &crashHandler, &m_prevSignal.pipe );
    sigaction( SIGBUS, &crashHandler, &m_prevSignal.bus );
    sigaction( SIGABRT, &crashHandler, &m_prevSignal.abrt );
#endif

    m_crashHandlerInstalled = true;

#ifdef TRACY_HAS_CALLSTACK
    InitCallstack();
#endif

    m_timeBegin.store( GetTime(), std::memory_order_relaxed );
}

Profiler::~Profiler()
{
    m_shutdown.store( true, std::memory_order_relaxed );

#if defined _WIN32 || defined __CYGWIN__
    if( m_crashHandlerInstalled ) RemoveVectoredExceptionHandler( m_exceptionHandler );
#endif

#ifdef __linux__
    if( m_crashHandlerInstalled )
    {
        sigaction( SIGPWR, &m_prevSignal.pwr, nullptr );
        sigaction( SIGILL, &m_prevSignal.ill, nullptr );
        sigaction( SIGFPE, &m_prevSignal.fpe, nullptr );
        sigaction( SIGSEGV, &m_prevSignal.segv, nullptr );
        sigaction( SIGPIPE, &m_prevSignal.pipe, nullptr );
        sigaction( SIGBUS, &m_prevSignal.bus, nullptr );
        sigaction( SIGABRT, &m_prevSignal.abrt, nullptr );
    }
#endif

#ifdef TRACY_HAS_SYSTEM_TRACING
    if( s_sysTraceThread )
    {
        SysTraceStop();
        s_sysTraceThread->~Thread();
        tracy_free( s_sysTraceThread );
    }
#endif

#ifndef TRACY_NO_FRAME_IMAGE
    s_compressThread->~Thread();
    tracy_free( s_compressThread );
#endif

    s_thread->~Thread();
    tracy_free( s_thread );

    tracy_free( m_lz4Buf );
    tracy_free( m_buffer );
    LZ4_freeStream( (LZ4_stream_t*)m_stream );

    if( m_sock )
    {
        m_sock->~Socket();
        tracy_free( m_sock );
    }

    if( m_broadcast )
    {
        m_broadcast->~UdpBroadcast();
        tracy_free( m_broadcast );
    }

    assert( s_instance );
    s_instance = nullptr;
}

bool Profiler::ShouldExit()
{
    return s_instance->m_shutdown.load( std::memory_order_relaxed );
}

void Profiler::Worker()
{
#ifdef __linux__
    s_profilerTid = syscall( SYS_gettid );
#endif

    ThreadExitHandler threadExitHandler;

    SetThreadName( "Tracy Profiler" );

#ifdef TRACY_DATA_PORT
    const bool dataPortSearch = false;
    auto dataPort = m_userPort != 0 ? m_userPort : TRACY_DATA_PORT;
#else
    const bool dataPortSearch = m_userPort == 0;
    auto dataPort = m_userPort != 0 ? m_userPort : 8086;
#endif
#ifdef TRACY_BROADCAST_PORT
    const auto broadcastPort = TRACY_BROADCAST_PORT;
#else
    const auto broadcastPort = 8086;
#endif

    while( m_timeBegin.load( std::memory_order_relaxed ) == 0 ) std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );

    rpmalloc_thread_initialize();

    m_exectime = 0;
    const auto execname = GetProcessExecutablePath();
    if( execname )
    {
        struct stat st;
        if( stat( execname, &st ) == 0 )
        {
            m_exectime = (uint64_t)st.st_mtime;
        }
    }

    const auto procname = GetProcessName();
    const auto pnsz = std::min<size_t>( strlen( procname ), WelcomeMessageProgramNameSize - 1 );

    const auto hostinfo = GetHostInfo();
    const auto hisz = std::min<size_t>( strlen( hostinfo ), WelcomeMessageHostInfoSize - 1 );

    const uint64_t pid = GetPid();

    uint8_t flags = 0;

#ifdef TRACY_ON_DEMAND
    flags |= WelcomeFlag::OnDemand;
#endif
#ifdef __APPLE__
    flags |= WelcomeFlag::IsApple;
#endif
#ifndef TRACY_NO_CODE_TRANSFER
    flags |= WelcomeFlag::CodeTransfer;
#endif
#ifdef _WIN32
    flags |= WelcomeFlag::CombineSamples;
#endif

#if defined __i386 || defined _M_IX86
    uint8_t cpuArch = CpuArchX86;
#elif defined __x86_64__ || defined _M_X64
    uint8_t cpuArch = CpuArchX64;
#elif defined __aarch64__
    uint8_t cpuArch = CpuArchArm64;
#elif defined __ARM_ARCH
    uint8_t cpuArch = CpuArchArm32;
#else
    uint8_t cpuArch = CpuArchUnknown;
#endif

#if defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64
    uint32_t regs[4];
    char manufacturer[12];
    CpuId( regs, 0 );
    memcpy( manufacturer, regs+1, 4 );
    memcpy( manufacturer+4, regs+3, 4 );
    memcpy( manufacturer+8, regs+2, 4 );

    CpuId( regs, 1 );
    uint32_t cpuId = ( regs[0] & 0xFFF ) | ( ( regs[0] & 0xFFF0000 ) >> 4 );
#else
    const char manufacturer[12] = {};
    uint32_t cpuId = 0;
#endif

    WelcomeMessage welcome;
    MemWrite( &welcome.timerMul, m_timerMul );
    MemWrite( &welcome.initBegin, GetInitTime() );
    MemWrite( &welcome.initEnd, m_timeBegin.load( std::memory_order_relaxed ) );
    MemWrite( &welcome.delay, m_delay );
    MemWrite( &welcome.resolution, m_resolution );
    MemWrite( &welcome.epoch, m_epoch );
    MemWrite( &welcome.exectime, m_exectime );
    MemWrite( &welcome.pid, pid );
    MemWrite( &welcome.samplingPeriod, m_samplingPeriod );
    MemWrite( &welcome.flags, flags );
    MemWrite( &welcome.cpuArch, cpuArch );
    memcpy( welcome.cpuManufacturer, manufacturer, 12 );
    MemWrite( &welcome.cpuId, cpuId );
    memcpy( welcome.programName, procname, pnsz );
    memset( welcome.programName + pnsz, 0, WelcomeMessageProgramNameSize - pnsz );
    memcpy( welcome.hostInfo, hostinfo, hisz );
    memset( welcome.hostInfo + hisz, 0, WelcomeMessageHostInfoSize - hisz );

    moodycamel::ConsumerToken token( GetQueue() );

    ListenSocket listen;
    bool isListening = false;
    if( !dataPortSearch )
    {
        isListening = listen.Listen( dataPort, 4 );
    }
    else
    {
        for( uint32_t i=0; i<20; i++ )
        {
            if( listen.Listen( dataPort+i, 4 ) )
            {
                dataPort += i;
                isListening = true;
                break;
            }
        }
    }
    if( !isListening )
    {
        for(;;)
        {
            if( ShouldExit() )
            {
                m_shutdownFinished.store( true, std::memory_order_relaxed );
                return;
            }

            ClearQueues( token );
            std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        }
    }

#ifndef TRACY_NO_BROADCAST
    m_broadcast = (UdpBroadcast*)tracy_malloc( sizeof( UdpBroadcast ) );
    new(m_broadcast) UdpBroadcast();
#  ifdef TRACY_ONLY_LOCALHOST
    const char* addr = "127.255.255.255";
#  else
    const char* addr = "255.255.255.255";
#  endif
    if( !m_broadcast->Open( addr, broadcastPort ) )
    {
        m_broadcast->~UdpBroadcast();
        tracy_free( m_broadcast );
        m_broadcast = nullptr;
    }
#endif

    int broadcastLen = 0;
    auto& broadcastMsg = GetBroadcastMessage( procname, pnsz, broadcastLen, dataPort );
    uint64_t lastBroadcast = 0;

    // Connections loop.
    // Each iteration of the loop handles whole connection. Multiple iterations will only
    // happen in the on-demand mode or when handshake fails.
    for(;;)
    {
        // Wait for incoming connection
        for(;;)
        {
#ifndef TRACY_NO_EXIT
            if( !m_noExit && ShouldExit() )
            {
                if( m_broadcast )
                {
                    broadcastMsg.activeTime = -1;
                    m_broadcast->Send( broadcastPort, &broadcastMsg, broadcastLen );
                }
                m_shutdownFinished.store( true, std::memory_order_relaxed );
                return;
            }
#endif
            m_sock = listen.Accept();
            if( m_sock ) break;
#ifndef TRACY_ON_DEMAND
            ProcessSysTime();
#endif

            if( m_broadcast )
            {
                const auto t = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                if( t - lastBroadcast > 3000000000 )  // 3s
                {
                    lastBroadcast = t;
                    const auto ts = std::chrono::duration_cast<std::chrono::seconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
                    broadcastMsg.activeTime = int32_t( ts - m_epoch );
                    assert( broadcastMsg.activeTime >= 0 );
                    m_broadcast->Send( broadcastPort, &broadcastMsg, broadcastLen );
                }
            }
        }

        if( m_broadcast )
        {
            lastBroadcast = 0;
            broadcastMsg.activeTime = -1;
            m_broadcast->Send( broadcastPort, &broadcastMsg, broadcastLen );
        }

        // Handshake
        {
            char shibboleth[HandshakeShibbolethSize];
            auto res = m_sock->ReadRaw( shibboleth, HandshakeShibbolethSize, 2000 );
            if( !res || memcmp( shibboleth, HandshakeShibboleth, HandshakeShibbolethSize ) != 0 )
            {
                m_sock->~Socket();
                tracy_free( m_sock );
                m_sock = nullptr;
                continue;
            }

            uint32_t protocolVersion;
            res = m_sock->ReadRaw( &protocolVersion, sizeof( protocolVersion ), 2000 );
            if( !res )
            {
                m_sock->~Socket();
                tracy_free( m_sock );
                m_sock = nullptr;
                continue;
            }

            if( protocolVersion != ProtocolVersion )
            {
                HandshakeStatus status = HandshakeProtocolMismatch;
                m_sock->Send( &status, sizeof( status ) );
                m_sock->~Socket();
                tracy_free( m_sock );
                m_sock = nullptr;
                continue;
            }
        }

#ifdef TRACY_ON_DEMAND
        const auto currentTime = GetTime();
        ClearQueues( token );
        m_connectionId.fetch_add( 1, std::memory_order_release );
#endif
        m_isConnected.store( true, std::memory_order_release );

        HandshakeStatus handshake = HandshakeWelcome;
        m_sock->Send( &handshake, sizeof( handshake ) );

        LZ4_resetStream( (LZ4_stream_t*)m_stream );
        m_sock->Send( &welcome, sizeof( welcome ) );

        m_threadCtx = 0;
        m_refTimeSerial = 0;
        m_refTimeCtx = 0;
        m_refTimeGpu = 0;

#ifdef TRACY_ON_DEMAND
        OnDemandPayloadMessage onDemand;
        onDemand.frames = m_frameCount.load( std::memory_order_relaxed );
        onDemand.currentTime = currentTime;

        m_sock->Send( &onDemand, sizeof( onDemand ) );

        m_deferredLock.lock();
        for( auto& item : m_deferredQueue )
        {
            uint64_t ptr;
            uint16_t size;
            const auto idx = MemRead<uint8_t>( &item.hdr.idx );
            switch( (QueueType)idx )
            {
            case QueueType::MessageAppInfo:
                ptr = MemRead<uint64_t>( &item.messageFat.text );
                size = MemRead<uint16_t>( &item.messageFat.size );
                SendSingleString( (const char*)ptr, size );
                break;
            case QueueType::LockName:
                ptr = MemRead<uint64_t>( &item.lockNameFat.name );
                size = MemRead<uint16_t>( &item.lockNameFat.size );
                SendSingleString( (const char*)ptr, size );
                break;
            case QueueType::GpuContextName:
                ptr = MemRead<uint64_t>( &item.gpuContextNameFat.ptr );
                size = MemRead<uint16_t>( &item.gpuContextNameFat.size );
                SendSingleString( (const char*)ptr, size );
                break;
            default:
                break;
            }
            AppendData( &item, QueueDataSize[idx] );
        }
        m_deferredLock.unlock();
#endif

        // Main communications loop
        int keepAlive = 0;
        for(;;)
        {
            ProcessSysTime();
            const auto status = Dequeue( token );
            const auto serialStatus = DequeueSerial();
            if( status == DequeueStatus::ConnectionLost || serialStatus == DequeueStatus::ConnectionLost )
            {
                break;
            }
            else if( status == DequeueStatus::QueueEmpty && serialStatus == DequeueStatus::QueueEmpty )
            {
                if( ShouldExit() ) break;
                if( m_bufferOffset != m_bufferStart )
                {
                    if( !CommitData() ) break;
                }
                if( keepAlive == 500 )
                {
                    QueueItem ka;
                    ka.hdr.type = QueueType::KeepAlive;
                    AppendData( &ka, QueueDataSize[ka.hdr.idx] );
                    if( !CommitData() ) break;

                    keepAlive = 0;
                }
                else if( !m_sock->HasData() )
                {
                    keepAlive++;
                    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
                }
            }
            else
            {
                keepAlive = 0;
            }

            int quota = 500;
            bool connActive = true;
            while( quota-- && m_sock->HasData() )
            {
                connActive = HandleServerQuery();
                if( !connActive ) break;
            }
            if( !connActive ) break;
        }
        if( ShouldExit() ) break;

        m_isConnected.store( false, std::memory_order_release );
#ifdef TRACY_ON_DEMAND
        m_bufferOffset = 0;
        m_bufferStart = 0;
#endif

        m_sock->~Socket();
        tracy_free( m_sock );
        m_sock = nullptr;

#ifndef TRACY_ON_DEMAND
        // Client is no longer available here. Accept incoming connections, but reject handshake.
        for(;;)
        {
            if( ShouldExit() )
            {
                m_shutdownFinished.store( true, std::memory_order_relaxed );
                return;
            }

            ClearQueues( token );

            m_sock = listen.Accept();
            if( m_sock )
            {
                char shibboleth[HandshakeShibbolethSize];
                auto res = m_sock->ReadRaw( shibboleth, HandshakeShibbolethSize, 1000 );
                if( !res || memcmp( shibboleth, HandshakeShibboleth, HandshakeShibbolethSize ) != 0 )
                {
                    m_sock->~Socket();
                    tracy_free( m_sock );
                    m_sock = nullptr;
                    continue;
                }

                uint32_t protocolVersion;
                res = m_sock->ReadRaw( &protocolVersion, sizeof( protocolVersion ), 1000 );
                if( !res )
                {
                    m_sock->~Socket();
                    tracy_free( m_sock );
                    m_sock = nullptr;
                    continue;
                }

                HandshakeStatus status = HandshakeNotAvailable;
                m_sock->Send( &status, sizeof( status ) );
                m_sock->~Socket();
                tracy_free( m_sock );
            }
        }
#endif
    }
    // End of connections loop

    // Client is exiting. Send items remaining in queues.
    for(;;)
    {
        const auto status = Dequeue( token );
        const auto serialStatus = DequeueSerial();
        if( status == DequeueStatus::ConnectionLost || serialStatus == DequeueStatus::ConnectionLost )
        {
            m_shutdownFinished.store( true, std::memory_order_relaxed );
            return;
        }
        else if( status == DequeueStatus::QueueEmpty && serialStatus == DequeueStatus::QueueEmpty )
        {
            if( m_bufferOffset != m_bufferStart ) CommitData();
            break;
        }

        while( m_sock->HasData() )
        {
            if( !HandleServerQuery() )
            {
                m_shutdownFinished.store( true, std::memory_order_relaxed );
                return;
            }
        }
    }

    // Send client termination notice to the server
    QueueItem terminate;
    MemWrite( &terminate.hdr.type, QueueType::Terminate );
    if( !SendData( (const char*)&terminate, 1 ) )
    {
        m_shutdownFinished.store( true, std::memory_order_relaxed );
        return;
    }
    // Handle remaining server queries
    for(;;)
    {
        if( m_sock->HasData() )
        {
            while( m_sock->HasData() )
            {
                if( !HandleServerQuery() )
                {
                    m_shutdownFinished.store( true, std::memory_order_relaxed );
                    return;
                }
            }
            while( Dequeue( token ) == DequeueStatus::DataDequeued ) {}
            while( DequeueSerial() == DequeueStatus::DataDequeued ) {}
            if( m_bufferOffset != m_bufferStart )
            {
                if( !CommitData() )
                {
                    m_shutdownFinished.store( true, std::memory_order_relaxed );
                    return;
                }
            }
        }
        else
        {
            if( m_bufferOffset != m_bufferStart ) CommitData();
            std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        }
    }
}

#ifndef TRACY_NO_FRAME_IMAGE
void Profiler::CompressWorker()
{
    ThreadExitHandler threadExitHandler;

    SetThreadName( "Tracy DXT1" );
    while( m_timeBegin.load( std::memory_order_relaxed ) == 0 ) std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    rpmalloc_thread_initialize();
    for(;;)
    {
        const auto shouldExit = ShouldExit();

        {
            bool lockHeld = true;
            while( !m_fiLock.try_lock() )
            {
                if( m_shutdownManual.load( std::memory_order_relaxed ) )
                {
                    lockHeld = false;
                    break;
                }
            }
            if( !m_fiQueue.empty() ) m_fiQueue.swap( m_fiDequeue );
            if( lockHeld )
            {
                m_fiLock.unlock();
            }
        }

        const auto sz = m_fiDequeue.size();
        if( sz > 0 )
        {
            auto fi = m_fiDequeue.data();
            auto end = fi + sz;
            while( fi != end )
            {
                const auto w = fi->w;
                const auto h = fi->h;
                const auto csz = size_t( w * h / 2 );
                auto etc1buf = (char*)tracy_malloc( csz );
                CompressImageDxt1( (const char*)fi->image, etc1buf, w, h );
                tracy_free( fi->image );

                TracyLfqPrepare( QueueType::FrameImage );
                MemWrite( &item->frameImageFat.image, (uint64_t)etc1buf );
                MemWrite( &item->frameImageFat.frame, fi->frame );
                MemWrite( &item->frameImageFat.w, w );
                MemWrite( &item->frameImageFat.h, h );
                uint8_t flip = fi->flip;
                MemWrite( &item->frameImageFat.flip, flip );
                TracyLfqCommit;

                fi++;
            }
            m_fiDequeue.clear();
        }
        else
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
        }

        if( shouldExit )
        {
            return;
        }
    }
}
#endif

static void FreeAssociatedMemory( const QueueItem& item )
{
    if( item.hdr.idx >= (int)QueueType::Terminate ) return;

    uint64_t ptr;
    switch( item.hdr.type )
    {
    case QueueType::ZoneText:
    case QueueType::ZoneName:
        ptr = MemRead<uint64_t>( &item.zoneTextFat.text );
        tracy_free( (void*)ptr );
        break;
    case QueueType::MessageColor:
    case QueueType::MessageColorCallstack:
        ptr = MemRead<uint64_t>( &item.messageColorFat.text );
        tracy_free( (void*)ptr );
        break;
    case QueueType::Message:
    case QueueType::MessageCallstack:
#ifndef TRACY_ON_DEMAND
    case QueueType::MessageAppInfo:
#endif
        ptr = MemRead<uint64_t>( &item.messageFat.text );
        tracy_free( (void*)ptr );
        break;
    case QueueType::ZoneBeginAllocSrcLoc:
    case QueueType::ZoneBeginAllocSrcLocCallstack:
        ptr = MemRead<uint64_t>( &item.zoneBegin.srcloc );
        tracy_free( (void*)ptr );
        break;
    case QueueType::GpuZoneBeginAllocSrcLoc:
    case QueueType::GpuZoneBeginAllocSrcLocCallstack:
    case QueueType::GpuZoneBeginAllocSrcLocSerial:
    case QueueType::GpuZoneBeginAllocSrcLocCallstackSerial:
        ptr = MemRead<uint64_t>( &item.gpuZoneBegin.srcloc );
        tracy_free( (void*)ptr );
        break;
    case QueueType::CallstackSerial:
    case QueueType::Callstack:
        ptr = MemRead<uint64_t>( &item.callstackFat.ptr );
        tracy_free( (void*)ptr );
        break;
    case QueueType::CallstackAlloc:
        ptr = MemRead<uint64_t>( &item.callstackAllocFat.nativePtr );
        tracy_free( (void*)ptr );
        ptr = MemRead<uint64_t>( &item.callstackAllocFat.ptr );
        tracy_free( (void*)ptr );
        break;
    case QueueType::CallstackSample:
        ptr = MemRead<uint64_t>( &item.callstackSampleFat.ptr );
        tracy_free( (void*)ptr );
        break;
    case QueueType::FrameImage:
        ptr = MemRead<uint64_t>( &item.frameImageFat.image );
        tracy_free( (void*)ptr );
        break;
#ifndef TRACY_ON_DEMAND
    case QueueType::LockName:
        ptr = MemRead<uint64_t>( &item.lockNameFat.name );
        tracy_free( (void*)ptr );
        break;
    case QueueType::GpuContextName:
        ptr = MemRead<uint64_t>( &item.gpuContextNameFat.ptr );
        tracy_free( (void*)ptr );
        break;
#endif
#ifdef TRACY_ON_DEMAND
    case QueueType::MessageAppInfo:
    case QueueType::GpuContextName:
        // Don't free memory associated with deferred messages.
        break;
#endif
    default:
        break;
    }
}

void Profiler::ClearQueues( moodycamel::ConsumerToken& token )
{
    for(;;)
    {
        const auto sz = GetQueue().try_dequeue_bulk_single( token, [](const uint64_t&){}, []( QueueItem* item, size_t sz ) { assert( sz > 0 ); while( sz-- > 0 ) FreeAssociatedMemory( *item++ ); } );
        if( sz == 0 ) break;
    }

    ClearSerial();
}

void Profiler::ClearSerial()
{
    bool lockHeld = true;
    while( !m_serialLock.try_lock() )
    {
        if( m_shutdownManual.load( std::memory_order_relaxed ) )
        {
            lockHeld = false;
            break;
        }
    }
    for( auto& v : m_serialQueue ) FreeAssociatedMemory( v );
    m_serialQueue.clear();
    if( lockHeld )
    {
        m_serialLock.unlock();
    }

    for( auto& v : m_serialDequeue ) FreeAssociatedMemory( v );
    m_serialDequeue.clear();
}

Profiler::DequeueStatus Profiler::Dequeue( moodycamel::ConsumerToken& token )
{
    bool connectionLost = false;
    const auto sz = GetQueue().try_dequeue_bulk_single( token,
        [this, &connectionLost] ( const uint64_t& threadId )
        {
            if( threadId != m_threadCtx )
            {
                QueueItem item;
                MemWrite( &item.hdr.type, QueueType::ThreadContext );
                MemWrite( &item.threadCtx.thread, threadId );
                if( !AppendData( &item, QueueDataSize[(int)QueueType::ThreadContext] ) ) connectionLost = true;
                m_threadCtx = threadId;
                m_refTimeThread = 0;
            }
        },
        [this, &connectionLost] ( QueueItem* item, size_t sz )
        {
            if( connectionLost ) return;
            InitRpmalloc();
            assert( sz > 0 );
            int64_t refThread = m_refTimeThread;
            int64_t refCtx = m_refTimeCtx;
            int64_t refGpu = m_refTimeGpu;
            while( sz-- > 0 )
            {
                uint64_t ptr;
                uint16_t size;
                auto idx = MemRead<uint8_t>( &item->hdr.idx );
                if( idx < (int)QueueType::Terminate )
                {
                    switch( (QueueType)idx )
                    {
                    case QueueType::ZoneText:
                    case QueueType::ZoneName:
                        ptr = MemRead<uint64_t>( &item->zoneTextFat.text );
                        size = MemRead<uint16_t>( &item->zoneTextFat.size );
                        SendSingleString( (const char*)ptr, size );
                        tracy_free_fast( (void*)ptr );
                        break;
                    case QueueType::Message:
                    case QueueType::MessageCallstack:
                        ptr = MemRead<uint64_t>( &item->messageFat.text );
                        size = MemRead<uint16_t>( &item->messageFat.size );
                        SendSingleString( (const char*)ptr, size );
                        tracy_free_fast( (void*)ptr );
                        break;
                    case QueueType::MessageColor:
                    case QueueType::MessageColorCallstack:
                        ptr = MemRead<uint64_t>( &item->messageColorFat.text );
                        size = MemRead<uint16_t>( &item->messageColorFat.size );
                        SendSingleString( (const char*)ptr, size );
                        tracy_free_fast( (void*)ptr );
                        break;
                    case QueueType::MessageAppInfo:
                        ptr = MemRead<uint64_t>( &item->messageFat.text );
                        size = MemRead<uint16_t>( &item->messageFat.size );
                        SendSingleString( (const char*)ptr, size );
#ifndef TRACY_ON_DEMAND
                        tracy_free_fast( (void*)ptr );
#endif
                        break;
                    case QueueType::ZoneBeginAllocSrcLoc:
                    case QueueType::ZoneBeginAllocSrcLocCallstack:
                    {
                        int64_t t = MemRead<int64_t>( &item->zoneBegin.time );
                        int64_t dt = t - refThread;
                        refThread = t;
                        MemWrite( &item->zoneBegin.time, dt );
                        ptr = MemRead<uint64_t>( &item->zoneBegin.srcloc );
                        SendSourceLocationPayload( ptr );
                        tracy_free_fast( (void*)ptr );
                        break;
                    }
                    case QueueType::Callstack:
                        ptr = MemRead<uint64_t>( &item->callstackFat.ptr );
                        SendCallstackPayload( ptr );
                        tracy_free_fast( (void*)ptr );
                        break;
                    case QueueType::CallstackAlloc:
                        ptr = MemRead<uint64_t>( &item->callstackAllocFat.nativePtr );
                        if( ptr != 0 )
                        {
                            CutCallstack( (void*)ptr, "lua_pcall" );
                            SendCallstackPayload( ptr );
                            tracy_free_fast( (void*)ptr );
                        }
                        ptr = MemRead<uint64_t>( &item->callstackAllocFat.ptr );
                        SendCallstackAlloc( ptr );
                        tracy_free_fast( (void*)ptr );
                        break;
                    case QueueType::CallstackSample:
                    {
                        ptr = MemRead<uint64_t>( &item->callstackSampleFat.ptr );
                        SendCallstackPayload64( ptr );
                        tracy_free_fast( (void*)ptr );
                        int64_t t = MemRead<int64_t>( &item->callstackSampleFat.time );
                        int64_t dt = t - refCtx;
                        refCtx = t;
                        MemWrite( &item->callstackSampleFat.time, dt );
                        break;
                    }
                    case QueueType::FrameImage:
                    {
                        ptr = MemRead<uint64_t>( &item->frameImageFat.image );
                        const auto w = MemRead<uint16_t>( &item->frameImageFat.w );
                        const auto h = MemRead<uint16_t>( &item->frameImageFat.h );
                        const auto csz = size_t( w * h / 2 );
                        SendLongString( ptr, (const char*)ptr, csz, QueueType::FrameImageData );
                        tracy_free_fast( (void*)ptr );
                        break;
                    }
                    case QueueType::ZoneBegin:
                    case QueueType::ZoneBeginCallstack:
                    {
                        int64_t t = MemRead<int64_t>( &item->zoneBegin.time );
                        int64_t dt = t - refThread;
                        refThread = t;
                        MemWrite( &item->zoneBegin.time, dt );
                        break;
                    }
                    case QueueType::ZoneEnd:
                    {
                        int64_t t = MemRead<int64_t>( &item->zoneEnd.time );
                        int64_t dt = t - refThread;
                        refThread = t;
                        MemWrite( &item->zoneEnd.time, dt );
                        break;
                    }
                    case QueueType::GpuZoneBegin:
                    case QueueType::GpuZoneBeginCallstack:
                    {
                        int64_t t = MemRead<int64_t>( &item->gpuZoneBegin.cpuTime );
                        int64_t dt = t - refThread;
                        refThread = t;
                        MemWrite( &item->gpuZoneBegin.cpuTime, dt );
                        break;
                    }
                    case QueueType::GpuZoneBeginAllocSrcLoc:
                    case QueueType::GpuZoneBeginAllocSrcLocCallstack:
                    {
                        int64_t t = MemRead<int64_t>( &item->gpuZoneBegin.cpuTime );
                        int64_t dt = t - refThread;
                        refThread = t;
                        MemWrite( &item->gpuZoneBegin.cpuTime, dt );
                        ptr = MemRead<uint64_t>( &item->gpuZoneBegin.srcloc );
                        SendSourceLocationPayload( ptr );
                        tracy_free_fast( (void*)ptr );
                        break;
                    }
                    case QueueType::GpuZoneEnd:
                    {
                        int64_t t = MemRead<int64_t>( &item->gpuZoneEnd.cpuTime );
                        int64_t dt = t - refThread;
                        refThread = t;
                        MemWrite( &item->gpuZoneEnd.cpuTime, dt );
                        break;
                    }
                    case QueueType::GpuContextName:
                        ptr = MemRead<uint64_t>( &item->gpuContextNameFat.ptr );
                        size = MemRead<uint16_t>( &item->gpuContextNameFat.size );
                        SendSingleString( (const char*)ptr, size );
#ifndef TRACY_ON_DEMAND
                        tracy_free_fast( (void*)ptr );
#endif
                        break;
                    case QueueType::PlotData:
                    {
                        int64_t t = MemRead<int64_t>( &item->plotData.time );
                        int64_t dt = t - refThread;
                        refThread = t;
                        MemWrite( &item->plotData.time, dt );
                        break;
                    }
                    case QueueType::ContextSwitch:
                    {
                        int64_t t = MemRead<int64_t>( &item->contextSwitch.time );
                        int64_t dt = t - refCtx;
                        refCtx = t;
                        MemWrite( &item->contextSwitch.time, dt );
                        break;
                    }
                    case QueueType::ThreadWakeup:
                    {
                        int64_t t = MemRead<int64_t>( &item->threadWakeup.time );
                        int64_t dt = t - refCtx;
                        refCtx = t;
                        MemWrite( &item->threadWakeup.time, dt );
                        break;
                    }
                    case QueueType::GpuTime:
                    {
                        int64_t t = MemRead<int64_t>( &item->gpuTime.gpuTime );
                        int64_t dt = t - refGpu;
                        refGpu = t;
                        MemWrite( &item->gpuTime.gpuTime, dt );
                        break;
                    }
                    default:
                        assert( false );
                        break;
                    }
                }
                if( !AppendData( item++, QueueDataSize[idx] ) )
                {
                    connectionLost = true;
                    m_refTimeThread = refThread;
                    m_refTimeCtx = refCtx;
                    m_refTimeGpu = refGpu;
                    return;
                }
            }
            m_refTimeThread = refThread;
            m_refTimeCtx = refCtx;
            m_refTimeGpu = refGpu;
        }
    );
    if( connectionLost ) return DequeueStatus::ConnectionLost;
    return sz > 0 ? DequeueStatus::DataDequeued : DequeueStatus::QueueEmpty;
}

Profiler::DequeueStatus Profiler::DequeueContextSwitches( tracy::moodycamel::ConsumerToken& token, int64_t& timeStop )
{
    const auto sz = GetQueue().try_dequeue_bulk_single( token, [] ( const uint64_t& ) {},
        [this, &timeStop] ( QueueItem* item, size_t sz )
        {
            assert( sz > 0 );
            int64_t refCtx = m_refTimeCtx;
            while( sz-- > 0 )
            {
                FreeAssociatedMemory( *item );
                if( timeStop < 0 ) return;
                const auto idx = MemRead<uint8_t>( &item->hdr.idx );
                if( idx == (uint8_t)QueueType::ContextSwitch )
                {
                    const auto csTime = MemRead<int64_t>( &item->contextSwitch.time );
                    if( csTime > timeStop )
                    {
                        timeStop = -1;
                        m_refTimeCtx = refCtx;
                        return;
                    }
                    int64_t dt = csTime - refCtx;
                    refCtx = csTime;
                    MemWrite( &item->contextSwitch.time, dt );
                    if( !AppendData( item, QueueDataSize[(int)QueueType::ContextSwitch] ) )
                    {
                        timeStop = -2;
                        m_refTimeCtx = refCtx;
                        return;
                    }
                }
                else if( idx == (uint8_t)QueueType::ThreadWakeup )
                {
                    const auto csTime = MemRead<int64_t>( &item->threadWakeup.time );
                    if( csTime > timeStop )
                    {
                        timeStop = -1;
                        m_refTimeCtx = refCtx;
                        return;
                    }
                    int64_t dt = csTime - refCtx;
                    refCtx = csTime;
                    MemWrite( &item->threadWakeup.time, dt );
                    if( !AppendData( item, QueueDataSize[(int)QueueType::ThreadWakeup] ) )
                    {
                        timeStop = -2;
                        m_refTimeCtx = refCtx;
                        return;
                    }
                }
                item++;
            }
            m_refTimeCtx = refCtx;
        }
    );

    if( timeStop == -2 ) return DequeueStatus::ConnectionLost;
    return ( timeStop == -1 || sz > 0 ) ? DequeueStatus::DataDequeued : DequeueStatus::QueueEmpty;
}

Profiler::DequeueStatus Profiler::DequeueSerial()
{
    {
        bool lockHeld = true;
        while( !m_serialLock.try_lock() )
        {
            if( m_shutdownManual.load( std::memory_order_relaxed ) )
            {
                lockHeld = false;
                break;
            }
        }
        if( !m_serialQueue.empty() ) m_serialQueue.swap( m_serialDequeue );
        if( lockHeld )
        {
            m_serialLock.unlock();
        }
    }

    const auto sz = m_serialDequeue.size();
    if( sz > 0 )
    {
        InitRpmalloc();
        int64_t refSerial = m_refTimeSerial;
        int64_t refGpu = m_refTimeGpu;
        auto item = m_serialDequeue.data();
        auto end = item + sz;
        while( item != end )
        {
            uint64_t ptr;
            auto idx = MemRead<uint8_t>( &item->hdr.idx );
            if( idx < (int)QueueType::Terminate )
            {
                switch( (QueueType)idx )
                {
                case QueueType::CallstackSerial:
                    ptr = MemRead<uint64_t>( &item->callstackFat.ptr );
                    SendCallstackPayload( ptr );
                    tracy_free_fast( (void*)ptr );
                    break;
                case QueueType::LockWait:
                case QueueType::LockSharedWait:
                {
                    int64_t t = MemRead<int64_t>( &item->lockWait.time );
                    int64_t dt = t - refSerial;
                    refSerial = t;
                    MemWrite( &item->lockWait.time, dt );
                    break;
                }
                case QueueType::LockObtain:
                case QueueType::LockSharedObtain:
                {
                    int64_t t = MemRead<int64_t>( &item->lockObtain.time );
                    int64_t dt = t - refSerial;
                    refSerial = t;
                    MemWrite( &item->lockObtain.time, dt );
                    break;
                }
                case QueueType::LockRelease:
                case QueueType::LockSharedRelease:
                {
                    int64_t t = MemRead<int64_t>( &item->lockRelease.time );
                    int64_t dt = t - refSerial;
                    refSerial = t;
                    MemWrite( &item->lockRelease.time, dt );
                    break;
                }
                case QueueType::LockName:
                {
                    ptr = MemRead<uint64_t>( &item->lockNameFat.name );
                    uint16_t size = MemRead<uint16_t>( &item->lockNameFat.size );
                    SendSingleString( (const char*)ptr, size );
#ifndef TRACY_ON_DEMAND
                    tracy_free_fast( (void*)ptr );
#endif
                    break;
                }
                case QueueType::MemAlloc:
                case QueueType::MemAllocNamed:
                case QueueType::MemAllocCallstack:
                case QueueType::MemAllocCallstackNamed:
                {
                    int64_t t = MemRead<int64_t>( &item->memAlloc.time );
                    int64_t dt = t - refSerial;
                    refSerial = t;
                    MemWrite( &item->memAlloc.time, dt );
                    break;
                }
                case QueueType::MemFree:
                case QueueType::MemFreeNamed:
                case QueueType::MemFreeCallstack:
                case QueueType::MemFreeCallstackNamed:
                {
                    int64_t t = MemRead<int64_t>( &item->memFree.time );
                    int64_t dt = t - refSerial;
                    refSerial = t;
                    MemWrite( &item->memFree.time, dt );
                    break;
                }
                case QueueType::GpuZoneBeginSerial:
                case QueueType::GpuZoneBeginCallstackSerial:
                {
                    int64_t t = MemRead<int64_t>( &item->gpuZoneBegin.cpuTime );
                    int64_t dt = t - refSerial;
                    refSerial = t;
                    MemWrite( &item->gpuZoneBegin.cpuTime, dt );
                    break;
                }
                case QueueType::GpuZoneBeginAllocSrcLocSerial:
                case QueueType::GpuZoneBeginAllocSrcLocCallstackSerial:
                {
                    int64_t t = MemRead<int64_t>( &item->gpuZoneBegin.cpuTime );
                    int64_t dt = t - refSerial;
                    refSerial = t;
                    MemWrite( &item->gpuZoneBegin.cpuTime, dt );
                    ptr = MemRead<uint64_t>( &item->gpuZoneBegin.srcloc );
                    SendSourceLocationPayload( ptr );
                    tracy_free_fast( (void*)ptr );
                    break;
                }
                case QueueType::GpuZoneEndSerial:
                {
                    int64_t t = MemRead<int64_t>( &item->gpuZoneEnd.cpuTime );
                    int64_t dt = t - refSerial;
                    refSerial = t;
                    MemWrite( &item->gpuZoneEnd.cpuTime, dt );
                    break;
                }
                case QueueType::GpuTime:
                {
                    int64_t t = MemRead<int64_t>( &item->gpuTime.gpuTime );
                    int64_t dt = t - refGpu;
                    refGpu = t;
                    MemWrite( &item->gpuTime.gpuTime, dt );
                    break;
                }
                case QueueType::GpuContextName:
                {
                    ptr = MemRead<uint64_t>( &item->gpuContextNameFat.ptr );
                    uint16_t size = MemRead<uint16_t>( &item->gpuContextNameFat.size );
                    SendSingleString( (const char*)ptr, size );
#ifndef TRACY_ON_DEMAND
                    tracy_free_fast( (void*)ptr );
#endif
                    break;
                }
                default:
                    assert( false );
                    break;
                }
            }
            if( !AppendData( item, QueueDataSize[idx] ) ) return DequeueStatus::ConnectionLost;
            item++;
        }
        m_refTimeSerial = refSerial;
        m_refTimeGpu = refGpu;
        m_serialDequeue.clear();
    }
    else
    {
        return DequeueStatus::QueueEmpty;
    }
    return DequeueStatus::DataDequeued;
}

bool Profiler::CommitData()
{
    bool ret = SendData( m_buffer + m_bufferStart, m_bufferOffset - m_bufferStart );
    if( m_bufferOffset > TargetFrameSize * 2 ) m_bufferOffset = 0;
    m_bufferStart = m_bufferOffset;
    return ret;
}

bool Profiler::SendData( const char* data, size_t len )
{
    const lz4sz_t lz4sz = LZ4_compress_fast_continue( (LZ4_stream_t*)m_stream, data, m_lz4Buf + sizeof( lz4sz_t ), (int)len, LZ4Size, 1 );
    memcpy( m_lz4Buf, &lz4sz, sizeof( lz4sz ) );
    return m_sock->Send( m_lz4Buf, lz4sz + sizeof( lz4sz_t ) ) != -1;
}

void Profiler::SendString( uint64_t str, const char* ptr, size_t len, QueueType type )
{
    assert( type == QueueType::StringData ||
            type == QueueType::ThreadName ||
            type == QueueType::PlotName ||
            type == QueueType::FrameName ||
            type == QueueType::ExternalName ||
            type == QueueType::ExternalThreadName );

    QueueItem item;
    MemWrite( &item.hdr.type, type );
    MemWrite( &item.stringTransfer.ptr, str );

    assert( len <= std::numeric_limits<uint16_t>::max() );
    auto l16 = uint16_t( len );

    NeedDataSize( QueueDataSize[(int)type] + sizeof( l16 ) + l16 );

    AppendDataUnsafe( &item, QueueDataSize[(int)type] );
    AppendDataUnsafe( &l16, sizeof( l16 ) );
    AppendDataUnsafe( ptr, l16 );
}

void Profiler::SendSingleString( const char* ptr, size_t len )
{
    QueueItem item;
    MemWrite( &item.hdr.type, QueueType::SingleStringData );

    assert( len <= std::numeric_limits<uint16_t>::max() );
    auto l16 = uint16_t( len );

    NeedDataSize( QueueDataSize[(int)QueueType::SingleStringData] + sizeof( l16 ) + l16 );

    AppendDataUnsafe( &item, QueueDataSize[(int)QueueType::SingleStringData] );
    AppendDataUnsafe( &l16, sizeof( l16 ) );
    AppendDataUnsafe( ptr, l16 );
}

void Profiler::SendSecondString( const char* ptr, size_t len )
{
    QueueItem item;
    MemWrite( &item.hdr.type, QueueType::SecondStringData );

    assert( len <= std::numeric_limits<uint16_t>::max() );
    auto l16 = uint16_t( len );

    NeedDataSize( QueueDataSize[(int)QueueType::SecondStringData] + sizeof( l16 ) + l16 );

    AppendDataUnsafe( &item, QueueDataSize[(int)QueueType::SecondStringData] );
    AppendDataUnsafe( &l16, sizeof( l16 ) );
    AppendDataUnsafe( ptr, l16 );
}

void Profiler::SendLongString( uint64_t str, const char* ptr, size_t len, QueueType type )
{
    assert( type == QueueType::FrameImageData ||
            type == QueueType::SymbolCode ||
            type == QueueType::SourceCode );

    QueueItem item;
    MemWrite( &item.hdr.type, type );
    MemWrite( &item.stringTransfer.ptr, str );

    assert( len <= std::numeric_limits<uint32_t>::max() );
    assert( QueueDataSize[(int)type] + sizeof( uint32_t ) + len <= TargetFrameSize );
    auto l32 = uint32_t( len );

    NeedDataSize( QueueDataSize[(int)type] + sizeof( l32 ) + l32 );

    AppendDataUnsafe( &item, QueueDataSize[(int)type] );
    AppendDataUnsafe( &l32, sizeof( l32 ) );
    AppendDataUnsafe( ptr, l32 );
}

void Profiler::SendSourceLocation( uint64_t ptr )
{
    auto srcloc = (const SourceLocationData*)ptr;
    QueueItem item;
    MemWrite( &item.hdr.type, QueueType::SourceLocation );
    MemWrite( &item.srcloc.name, (uint64_t)srcloc->name );
    MemWrite( &item.srcloc.file, (uint64_t)srcloc->file );
    MemWrite( &item.srcloc.function, (uint64_t)srcloc->function );
    MemWrite( &item.srcloc.line, srcloc->line );
    MemWrite( &item.srcloc.r, uint8_t( ( srcloc->color       ) & 0xFF ) );
    MemWrite( &item.srcloc.g, uint8_t( ( srcloc->color >> 8  ) & 0xFF ) );
    MemWrite( &item.srcloc.b, uint8_t( ( srcloc->color >> 16 ) & 0xFF ) );
    AppendData( &item, QueueDataSize[(int)QueueType::SourceLocation] );
}

void Profiler::SendSourceLocationPayload( uint64_t _ptr )
{
    auto ptr = (const char*)_ptr;

    QueueItem item;
    MemWrite( &item.hdr.type, QueueType::SourceLocationPayload );
    MemWrite( &item.stringTransfer.ptr, _ptr );

    uint16_t len;
    memcpy( &len, ptr, sizeof( len ) );
    assert( len > 2 );
    len -= 2;
    ptr += 2;

    NeedDataSize( QueueDataSize[(int)QueueType::SourceLocationPayload] + sizeof( len ) + len );

    AppendDataUnsafe( &item, QueueDataSize[(int)QueueType::SourceLocationPayload] );
    AppendDataUnsafe( &len, sizeof( len ) );
    AppendDataUnsafe( ptr, len );
}

void Profiler::SendCallstackPayload( uint64_t _ptr )
{
    auto ptr = (uintptr_t*)_ptr;

    QueueItem item;
    MemWrite( &item.hdr.type, QueueType::CallstackPayload );
    MemWrite( &item.stringTransfer.ptr, _ptr );

    const auto sz = *ptr++;
    const auto len = sz * sizeof( uint64_t );
    const auto l16 = uint16_t( len );

    NeedDataSize( QueueDataSize[(int)QueueType::CallstackPayload] + sizeof( l16 ) + l16 );

    AppendDataUnsafe( &item, QueueDataSize[(int)QueueType::CallstackPayload] );
    AppendDataUnsafe( &l16, sizeof( l16 ) );

    if( compile_time_condition<sizeof( uintptr_t ) == sizeof( uint64_t )>::value )
    {
        AppendDataUnsafe( ptr, sizeof( uint64_t ) * sz );
    }
    else
    {
        for( uintptr_t i=0; i<sz; i++ )
        {
            const auto val = uint64_t( *ptr++ );
            AppendDataUnsafe( &val, sizeof( uint64_t ) );
        }
    }
}

void Profiler::SendCallstackPayload64( uint64_t _ptr )
{
    auto ptr = (uint64_t*)_ptr;

    QueueItem item;
    MemWrite( &item.hdr.type, QueueType::CallstackPayload );
    MemWrite( &item.stringTransfer.ptr, _ptr );

    const auto sz = *ptr++;
    const auto len = sz * sizeof( uint64_t );
    const auto l16 = uint16_t( len );

    NeedDataSize( QueueDataSize[(int)QueueType::CallstackPayload] + sizeof( l16 ) + l16 );

    AppendDataUnsafe( &item, QueueDataSize[(int)QueueType::CallstackPayload] );
    AppendDataUnsafe( &l16, sizeof( l16 ) );
    AppendDataUnsafe( ptr, sizeof( uint64_t ) * sz );
}

void Profiler::SendCallstackAlloc( uint64_t _ptr )
{
    auto ptr = (const char*)_ptr;

    QueueItem item;
    MemWrite( &item.hdr.type, QueueType::CallstackAllocPayload );
    MemWrite( &item.stringTransfer.ptr, _ptr );

    uint16_t len;
    memcpy( &len, ptr, 2 );
    ptr += 2;

    NeedDataSize( QueueDataSize[(int)QueueType::CallstackAllocPayload] + sizeof( len ) + len );

    AppendDataUnsafe( &item, QueueDataSize[(int)QueueType::CallstackAllocPayload] );
    AppendDataUnsafe( &len, sizeof( len ) );
    AppendDataUnsafe( ptr, len );
}

void Profiler::SendCallstackFrame( uint64_t ptr )
{
#ifdef TRACY_HAS_CALLSTACK
    const auto frameData = DecodeCallstackPtr( ptr );

    {
        SendSingleString( frameData.imageName );

        QueueItem item;
        MemWrite( &item.hdr.type, QueueType::CallstackFrameSize );
        MemWrite( &item.callstackFrameSize.ptr, ptr );
        MemWrite( &item.callstackFrameSize.size, frameData.size );

        AppendData( &item, QueueDataSize[(int)QueueType::CallstackFrameSize] );
    }

    InitRpmalloc();
    for( uint8_t i=0; i<frameData.size; i++ )
    {
        const auto& frame = frameData.data[i];

        SendSingleString( frame.name );
        SendSecondString( frame.file );

        QueueItem item;
        MemWrite( &item.hdr.type, QueueType::CallstackFrame );
        MemWrite( &item.callstackFrame.line, frame.line );
        MemWrite( &item.callstackFrame.symAddr, frame.symAddr );
        MemWrite( &item.callstackFrame.symLen, frame.symLen );

        AppendData( &item, QueueDataSize[(int)QueueType::CallstackFrame] );

        tracy_free_fast( (void*)frame.name );
        tracy_free_fast( (void*)frame.file );
    }
#endif
}

bool Profiler::HandleServerQuery()
{
    ServerQueryPacket payload;
    if( !m_sock->Read( &payload, sizeof( payload ), 10 ) ) return false;

    uint8_t type;
    uint64_t ptr;
    uint32_t extra;
    memcpy( &type, &payload.type, sizeof( payload.type ) );
    memcpy( &ptr, &payload.ptr, sizeof( payload.ptr ) );
    memcpy( &extra, &payload.extra, sizeof( payload.extra ) );

    switch( type )
    {
    case ServerQueryString:
        SendString( ptr, (const char*)ptr, QueueType::StringData );
        break;
    case ServerQueryThreadString:
        if( ptr == m_mainThread )
        {
            SendString( ptr, "Main thread", 11, QueueType::ThreadName );
        }
        else
        {
            SendString( ptr, GetThreadName( ptr ), QueueType::ThreadName );
        }
        break;
    case ServerQuerySourceLocation:
        SendSourceLocation( ptr );
        break;
    case ServerQueryPlotName:
        SendString( ptr, (const char*)ptr, QueueType::PlotName );
        break;
    case ServerQueryTerminate:
        return false;
    case ServerQueryCallstackFrame:
        SendCallstackFrame( ptr );
        break;
    case ServerQueryFrameName:
        SendString( ptr, (const char*)ptr, QueueType::FrameName );
        break;
    case ServerQueryDisconnect:
        HandleDisconnect();
        return false;
#ifdef TRACY_HAS_SYSTEM_TRACING
    case ServerQueryExternalName:
        SysTraceSendExternalName( ptr );
        break;
#endif
    case ServerQueryParameter:
        HandleParameter( ptr );
        break;
    case ServerQuerySymbol:
        HandleSymbolQuery( ptr );
        break;
#ifndef TRACY_NO_CODE_TRANSFER
    case ServerQuerySymbolCode:
        HandleSymbolCodeQuery( ptr, extra );
        break;
#endif
    case ServerQueryCodeLocation:
        SendCodeLocation( ptr );
        break;
    case ServerQuerySourceCode:
        HandleSourceCodeQuery();
        break;
    case ServerQueryDataTransfer:
        assert( !m_queryData );
        m_queryDataPtr = m_queryData = (char*)tracy_malloc( ptr + 11 );
        AckServerQuery();
        break;
    case ServerQueryDataTransferPart:
        memcpy( m_queryDataPtr, &ptr, 8 );
        memcpy( m_queryDataPtr+8, &extra, 4 );
        m_queryDataPtr += 12;
        AckServerQuery();
        break;
    default:
        assert( false );
        break;
    }

    return true;
}

void Profiler::HandleDisconnect()
{
    moodycamel::ConsumerToken token( GetQueue() );

#ifdef TRACY_HAS_SYSTEM_TRACING
    if( s_sysTraceThread )
    {
        auto timestamp = GetTime();
        for(;;)
        {
            const auto status = DequeueContextSwitches( token, timestamp );
            if( status == DequeueStatus::ConnectionLost )
            {
                return;
            }
            else if( status == DequeueStatus::QueueEmpty )
            {
                if( m_bufferOffset != m_bufferStart )
                {
                    if( !CommitData() ) return;
                }
            }
            if( timestamp < 0 )
            {
                if( m_bufferOffset != m_bufferStart )
                {
                    if( !CommitData() ) return;
                }
                break;
            }
            ClearSerial();
            if( m_sock->HasData() )
            {
                while( m_sock->HasData() )
                {
                    if( !HandleServerQuery() ) return;
                }
                if( m_bufferOffset != m_bufferStart )
                {
                    if( !CommitData() ) return;
                }
            }
            else
            {
                if( m_bufferOffset != m_bufferStart )
                {
                    if( !CommitData() ) return;
                }
                std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
            }
        }
    }
#endif

    QueueItem terminate;
    MemWrite( &terminate.hdr.type, QueueType::Terminate );
    if( !SendData( (const char*)&terminate, 1 ) ) return;
    for(;;)
    {
        ClearQueues( token );
        if( m_sock->HasData() )
        {
            while( m_sock->HasData() )
            {
                if( !HandleServerQuery() ) return;
            }
            if( m_bufferOffset != m_bufferStart )
            {
                if( !CommitData() ) return;
            }
        }
        else
        {
            if( m_bufferOffset != m_bufferStart )
            {
                if( !CommitData() ) return;
            }
            std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        }
    }
}

void Profiler::CalibrateTimer()
{
#ifdef TRACY_HW_TIMER
    std::atomic_signal_fence( std::memory_order_acq_rel );
    const auto t0 = std::chrono::high_resolution_clock::now();
    const auto r0 = GetTime();
    std::atomic_signal_fence( std::memory_order_acq_rel );
    std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
    std::atomic_signal_fence( std::memory_order_acq_rel );
    const auto t1 = std::chrono::high_resolution_clock::now();
    const auto r1 = GetTime();
    std::atomic_signal_fence( std::memory_order_acq_rel );

    const auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>( t1 - t0 ).count();
    const auto dr = r1 - r0;

    m_timerMul = double( dt ) / double( dr );
#else
    m_timerMul = 1.;
#endif
}

void Profiler::CalibrateDelay()
{
    constexpr int Iterations = 50000;

    auto mindiff = std::numeric_limits<int64_t>::max();
    for( int i=0; i<Iterations * 10; i++ )
    {
        const auto t0i = GetTime();
        const auto t1i = GetTime();
        const auto dti = t1i - t0i;
        if( dti > 0 && dti < mindiff ) mindiff = dti;
    }
    m_resolution = mindiff;

#ifdef TRACY_DELAYED_INIT
    m_delay = m_resolution;
#else
    constexpr int Events = Iterations * 2;   // start + end
    static_assert( Events < QueuePrealloc, "Delay calibration loop will allocate memory in queue" );

    static const tracy::SourceLocationData __tracy_source_location { nullptr, __FUNCTION__,  __FILE__, (uint32_t)__LINE__, 0 };
    const auto t0 = GetTime();
    for( int i=0; i<Iterations; i++ )
    {
        {
            TracyLfqPrepare( QueueType::ZoneBegin );
            MemWrite( &item->zoneBegin.time, Profiler::GetTime() );
            MemWrite( &item->zoneBegin.srcloc, (uint64_t)&__tracy_source_location );
            TracyLfqCommit;
        }
        {
            TracyLfqPrepare( QueueType::ZoneEnd );
            MemWrite( &item->zoneEnd.time, GetTime() );
            TracyLfqCommit;
        }
    }
    const auto t1 = GetTime();
    const auto dt = t1 - t0;
    m_delay = dt / Events;

    moodycamel::ConsumerToken token( GetQueue() );
    int left = Events;
    while( left != 0 )
    {
        const auto sz = GetQueue().try_dequeue_bulk_single( token, [](const uint64_t&){}, [](QueueItem* item, size_t sz){} );
        assert( sz > 0 );
        left -= (int)sz;
    }
    assert( GetQueue().size_approx() == 0 );
#endif
}

void Profiler::ReportTopology()
{
#ifndef TRACY_DELAYED_INIT
    struct CpuData
    {
        uint32_t package;
        uint32_t core;
        uint32_t thread;
    };

#if defined _WIN32 || defined __CYGWIN__
    t_GetLogicalProcessorInformationEx _GetLogicalProcessorInformationEx = (t_GetLogicalProcessorInformationEx)GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "GetLogicalProcessorInformationEx" );
    if( !_GetLogicalProcessorInformationEx ) return;

    DWORD psz = 0;
    _GetLogicalProcessorInformationEx( RelationProcessorPackage, nullptr, &psz );
    auto packageInfo = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)tracy_malloc( psz );
    auto res = _GetLogicalProcessorInformationEx( RelationProcessorPackage, packageInfo, &psz );
    assert( res );

    DWORD csz = 0;
    _GetLogicalProcessorInformationEx( RelationProcessorCore, nullptr, &csz );
    auto coreInfo = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)tracy_malloc( csz );
    res = _GetLogicalProcessorInformationEx( RelationProcessorCore, coreInfo, &csz );
    assert( res );

    SYSTEM_INFO sysinfo;
    GetSystemInfo( &sysinfo );
    const uint32_t numcpus = sysinfo.dwNumberOfProcessors;

    auto cpuData = (CpuData*)tracy_malloc( sizeof( CpuData ) * numcpus );
    for( uint32_t i=0; i<numcpus; i++ ) cpuData[i].thread = i;

    int idx = 0;
    auto ptr = packageInfo;
    while( (char*)ptr < ((char*)packageInfo) + psz )
    {
        assert( ptr->Relationship == RelationProcessorPackage );
        // FIXME account for GroupCount
        auto mask = ptr->Processor.GroupMask[0].Mask;
        int core = 0;
        while( mask != 0 )
        {
            if( mask & 1 ) cpuData[core].package = idx;
            core++;
            mask >>= 1;
        }
        ptr = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(((char*)ptr) + ptr->Size);
        idx++;
    }

    idx = 0;
    ptr = coreInfo;
    while( (char*)ptr < ((char*)coreInfo) + csz )
    {
        assert( ptr->Relationship == RelationProcessorCore );
        // FIXME account for GroupCount
        auto mask = ptr->Processor.GroupMask[0].Mask;
        int core = 0;
        while( mask != 0 )
        {
            if( mask & 1 ) cpuData[core].core = idx;
            core++;
            mask >>= 1;
        }
        ptr = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(((char*)ptr) + ptr->Size);
        idx++;
    }

    for( uint32_t i=0; i<numcpus; i++ )
    {
        auto& data = cpuData[i];

        TracyLfqPrepare( QueueType::CpuTopology );
        MemWrite( &item->cpuTopology.package, data.package );
        MemWrite( &item->cpuTopology.core, data.core );
        MemWrite( &item->cpuTopology.thread, data.thread );

#ifdef TRACY_ON_DEMAND
        DeferItem( *item );
#endif

        TracyLfqCommit;
    }

    tracy_free( cpuData );
    tracy_free( coreInfo );
    tracy_free( packageInfo );
#elif defined __linux__
    const int numcpus = std::thread::hardware_concurrency();
    auto cpuData = (CpuData*)tracy_malloc( sizeof( CpuData ) * numcpus );
    memset( cpuData, 0, sizeof( CpuData ) * numcpus );

    const char* basePath = "/sys/devices/system/cpu/cpu";
    for( int i=0; i<numcpus; i++ )
    {
        char path[1024];
        sprintf( path, "%s%i/topology/physical_package_id", basePath, i );
        char buf[1024];
        FILE* f = fopen( path, "rb" );
        if( !f )
        {
            tracy_free( cpuData );
            return;
        }
        auto read = fread( buf, 1, 1024, f );
        buf[read] = '\0';
        fclose( f );
        cpuData[i].package = uint32_t( atoi( buf ) );
        cpuData[i].thread = i;
        sprintf( path, "%s%i/topology/core_id", basePath, i );
        f = fopen( path, "rb" );
        read = fread( buf, 1, 1024, f );
        buf[read] = '\0';
        fclose( f );
        cpuData[i].core = uint32_t( atoi( buf ) );
    }

    for( int i=0; i<numcpus; i++ )
    {
        auto& data = cpuData[i];

        TracyLfqPrepare( QueueType::CpuTopology );
        MemWrite( &item->cpuTopology.package, data.package );
        MemWrite( &item->cpuTopology.core, data.core );
        MemWrite( &item->cpuTopology.thread, data.thread );

#ifdef TRACY_ON_DEMAND
        DeferItem( *item );
#endif

        TracyLfqCommit;
    }

    tracy_free( cpuData );
#endif
#endif
}

void Profiler::SendCallstack( int depth, const char* skipBefore )
{
#ifdef TRACY_HAS_CALLSTACK
    TracyLfqPrepare( QueueType::Callstack );
    auto ptr = Callstack( depth );
    CutCallstack( ptr, skipBefore );
    MemWrite( &item->callstackFat.ptr, (uint64_t)ptr );
    TracyLfqCommit;
#endif
}

void Profiler::CutCallstack( void* callstack, const char* skipBefore )
{
#ifdef TRACY_HAS_CALLSTACK
    auto data = (uintptr_t*)callstack;
    const auto sz = *data++;
    uintptr_t i;
    for( i=0; i<sz; i++ )
    {
        auto name = DecodeCallstackPtrFast( uint64_t( data[i] ) );
        const bool found = strcmp( name, skipBefore ) == 0;
        if( found )
        {
            i++;
            break;
        }
    }

    if( i != sz )
    {
        memmove( data, data + i, ( sz - i ) * sizeof( uintptr_t* ) );
        *--data = sz - i;
    }
#endif
}

#ifdef TRACY_HAS_SYSTIME
void Profiler::ProcessSysTime()
{
    if( m_shutdown.load( std::memory_order_relaxed ) ) return;
    auto t = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if( t - m_sysTimeLast > 100000000 )    // 100 ms
    {
        auto sysTime = m_sysTime.Get();
        if( sysTime >= 0 )
        {
            m_sysTimeLast = t;

            TracyLfqPrepare( QueueType::SysTimeReport );
            MemWrite( &item->sysTime.time, GetTime() );
            MemWrite( &item->sysTime.sysTime, sysTime );
            TracyLfqCommit;
        }
    }
}
#endif

void Profiler::HandleParameter( uint64_t payload )
{
    assert( m_paramCallback );
    const auto idx = uint32_t( payload >> 32 );
    const auto val = int32_t( payload & 0xFFFFFFFF );
    m_paramCallback( idx, val );
    AckServerQuery();
}

#ifdef __ANDROID__
// Implementation helpers of EnsureReadable(address).
// This is so far only needed on Android, where it is common for libraries to be mapped
// with only executable, not readable, permissions. Typical example (line from /proc/self/maps):
/*
746b63b000-746b6dc000 --xp 00042000 07:48 35                             /apex/com.android.runtime/lib64/bionic/libc.so
*/
// See https://github.com/wolfpld/tracy/issues/125 .
// To work around this, we parse /proc/self/maps and we use mprotect to set read permissions
// on any mappings that contain symbols addresses hit by HandleSymbolCodeQuery.

namespace {
// Holds some information about a single memory mapping.
struct MappingInfo {
    // Start of address range. Inclusive.
    uintptr_t start_address;
    // End of address range. Exclusive, so the mapping is the half-open interval
    // [start, end) and its length in bytes is `end - start`. As in /proc/self/maps.
    uintptr_t end_address;
    // Read/Write/Executable permissions.
    bool perm_r, perm_w, perm_x;
};
}  // anonymous namespace

// Internal implementation helper for LookUpMapping(address).
//
// Parses /proc/self/maps returning a vector<MappingInfo>.
// /proc/self/maps is assumed to be sorted by ascending address, so the resulting
// vector is sorted by ascending address too.
static std::vector<MappingInfo> ParseMappings()
{
    std::vector<MappingInfo> result;
    FILE* file = fopen( "/proc/self/maps", "r" );
    if( !file ) return result;
    char line[1024];
    while( fgets( line, sizeof( line ), file ) )
    {
        uintptr_t start_addr;
        uintptr_t end_addr;
        if( sscanf( line, "%lx-%lx", &start_addr, &end_addr ) != 2 ) continue;
        char* first_space = strchr( line, ' ' );
        if( !first_space ) continue;
        char* perm = first_space + 1;
        char* second_space = strchr( perm, ' ' );
        if( !second_space || second_space - perm != 4 ) continue;
        result.emplace_back();
        auto& mapping = result.back();
        mapping.start_address = start_addr;
        mapping.end_address = end_addr;
        mapping.perm_r = perm[0] == 'r';
        mapping.perm_w = perm[1] == 'w';
        mapping.perm_x = perm[2] == 'x';
    }
    fclose( file );
    return result;
}

// Internal implementation helper for LookUpMapping(address).
//
// Takes as input an `address` and a known vector `mappings`, assumed to be
// sorted by increasing addresses, as /proc/self/maps seems to be.
// Returns a pointer to the MappingInfo describing the mapping that this
// address belongs to, or nullptr if the address isn't in `mappings`.
static MappingInfo* LookUpMapping(std::vector<MappingInfo>& mappings, uintptr_t address)
{
    // Comparison function for std::lower_bound. Returns true if all addresses in `m1`
    // are lower than `addr`.
    auto Compare = []( const MappingInfo& m1, uintptr_t addr ) {
        // '<=' because the address ranges are half-open intervals, [start, end).
        return m1.end_address <= addr;
    };
    auto iter = std::lower_bound( mappings.begin(), mappings.end(), address, Compare );
    if( iter == mappings.end() || iter->start_address > address) {
        return nullptr;
    }
    return &*iter;
}

// Internal implementation helper for EnsureReadable(address).
//
// Takes as input an `address` and returns a pointer to a MappingInfo
// describing the mapping that this address belongs to, or nullptr if
// the address isn't in any known mapping.
//
// This function is stateful and not reentrant (assumes to be called from
// only one thread). It holds a vector of mappings parsed from /proc/self/maps.
//
// Attempts to react to mappings changes by re-parsing /proc/self/maps.
static MappingInfo* LookUpMapping(uintptr_t address)
{
    // Static state managed by this function. Not constant, we mutate that state as
    // we turn some mappings readable. Initially parsed once here, updated as needed below.
    static std::vector<MappingInfo> s_mappings = ParseMappings();
    MappingInfo* mapping = LookUpMapping( s_mappings, address );
    if( mapping ) return mapping;

    // This address isn't in any known mapping. Try parsing again, maybe
    // mappings changed.
    s_mappings = ParseMappings();
    return LookUpMapping( s_mappings, address );
}

// Internal implementation helper for EnsureReadable(address).
//
// Attempts to make the specified `mapping` readable if it isn't already.
// Returns true if and only if the mapping is readable.
static bool EnsureReadable( MappingInfo& mapping )
{
    if( mapping.perm_r )
    {
        // The mapping is already readable.
        return true;
    }
    int prot = PROT_READ;
    if( mapping.perm_w ) prot |= PROT_WRITE;
    if( mapping.perm_x ) prot |= PROT_EXEC;
    if( mprotect( reinterpret_cast<void*>( mapping.start_address ),
                  mapping.end_address - mapping.start_address, prot ) == -1 )
    {
        // Failed to make the mapping readable. Shouldn't happen, hasn't
        // been observed yet. If it happened in practice, we should consider
        // adding a bool to MappingInfo to track this to avoid retrying mprotect
        // everytime on such mappings.
        return false;
    }
    // The mapping is now readable. Update `mapping` so the next call will be fast.
    mapping.perm_r = true;
    return true;
}

// Attempts to set the read permission on the entire mapping containing the
// specified address. Returns true if and only if the mapping is now readable.
static bool EnsureReadable( uintptr_t address )
{
    MappingInfo* mapping = LookUpMapping(address);
    return mapping && EnsureReadable( *mapping );
}

#endif  // defined __ANDROID__

void Profiler::HandleSymbolQuery( uint64_t symbol )
{
#ifdef TRACY_HAS_CALLSTACK
    // Special handling for kernel frames
    if( symbol >> 63 != 0 )
    {
        SendSingleString( "<kernel>" );
        QueueItem item;
        MemWrite( &item.hdr.type, QueueType::SymbolInformation );
        MemWrite( &item.symbolInformation.line, 0 );
        MemWrite( &item.symbolInformation.symAddr, symbol );
        AppendData( &item, QueueDataSize[(int)QueueType::SymbolInformation] );
        return;
    }
#  ifdef __ANDROID__
    // On Android it's common for code to be in mappings that are only executable
    // but not readable.
    if( !EnsureReadable( symbol ) )
    {
        AckServerQuery();
        return;
    }
#  endif
    const auto sym = DecodeSymbolAddress( symbol );

    SendSingleString( sym.file );

    QueueItem item;
    MemWrite( &item.hdr.type, QueueType::SymbolInformation );
    MemWrite( &item.symbolInformation.line, sym.line );
    MemWrite( &item.symbolInformation.symAddr, symbol );

    AppendData( &item, QueueDataSize[(int)QueueType::SymbolInformation] );

    if( sym.needFree ) tracy_free( (void*)sym.file );
#endif
}

void Profiler::HandleSymbolCodeQuery( uint64_t symbol, uint32_t size )
{
#ifdef __ANDROID__
    // On Android it's common for code to be in mappings that are only executable
    // but not readable.
    if( !EnsureReadable( symbol ) )
    {
        AckServerQuery();
        return;
    }
#endif
    SendLongString( symbol, (const char*)symbol, size, QueueType::SymbolCode );
}

void Profiler::HandleSourceCodeQuery()
{
    assert( m_exectime != 0 );
    assert( m_queryData );

    InitRpmalloc();
    struct stat st;
    if( stat( m_queryData, &st ) == 0 && (uint64_t)st.st_mtime < m_exectime && st.st_size < ( TargetFrameSize - 16 ) )
    {
        FILE* f = fopen( m_queryData, "rb" );
        tracy_free_fast( m_queryData );
        if( f )
        {
            auto ptr = (char*)tracy_malloc_fast( st.st_size );
            auto rd = fread( ptr, 1, st.st_size, f );
            fclose( f );
            if( rd == (size_t)st.st_size )
            {
                SendLongString( (uint64_t)ptr, ptr, rd, QueueType::SourceCode );
            }
            else
            {
                AckSourceCodeNotAvailable();
            }
            tracy_free_fast( ptr );
        }
        else
        {
            AckSourceCodeNotAvailable();
        }
    }
    else
    {
        tracy_free_fast( m_queryData );
        AckSourceCodeNotAvailable();
    }
    m_queryData = nullptr;
}

void Profiler::SendCodeLocation( uint64_t ptr )
{
#ifdef TRACY_HAS_CALLSTACK
    const auto sym = DecodeCodeAddress( ptr );

    SendSingleString( sym.file );

    QueueItem item;
    MemWrite( &item.hdr.type, QueueType::CodeInformation );
    MemWrite( &item.codeInformation.ptr, ptr );
    MemWrite( &item.codeInformation.line, sym.line );
    MemWrite( &item.codeInformation.symAddr, sym.symAddr );

    AppendData( &item, QueueDataSize[(int)QueueType::CodeInformation] );

    if( sym.needFree ) tracy_free( (void*)sym.file );
#endif
}

#if ( defined _WIN32 || defined __CYGWIN__ ) && defined TRACY_TIMER_QPC
int64_t Profiler::GetTimeQpc()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter( &t );
    return t.QuadPart;
}
#endif

}

#ifdef __cplusplus
extern "C" {
#endif

TRACY_API TracyCZoneCtx ___tracy_emit_zone_begin( const struct ___tracy_source_location_data* srcloc, int active )
{
    ___tracy_c_zone_context ctx;
#ifdef TRACY_ON_DEMAND
    ctx.active = active && tracy::GetProfiler().IsConnected();
#else
    ctx.active = active;
#endif
    if( !ctx.active ) return ctx;
    const auto id = tracy::GetProfiler().GetNextZoneId();
    ctx.id = id;

#ifndef TRACY_NO_VERIFY
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneValidation );
        tracy::MemWrite( &item->zoneValidation.id, id );
        TracyLfqCommitC;
    }
#endif
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneBegin );
        tracy::MemWrite( &item->zoneBegin.time, tracy::Profiler::GetTime() );
        tracy::MemWrite( &item->zoneBegin.srcloc, (uint64_t)srcloc );
        TracyLfqCommitC;
    }
    return ctx;
}

TRACY_API TracyCZoneCtx ___tracy_emit_zone_begin_callstack( const struct ___tracy_source_location_data* srcloc, int depth, int active )
{
    ___tracy_c_zone_context ctx;
#ifdef TRACY_ON_DEMAND
    ctx.active = active && tracy::GetProfiler().IsConnected();
#else
    ctx.active = active;
#endif
    if( !ctx.active ) return ctx;
    const auto id = tracy::GetProfiler().GetNextZoneId();
    ctx.id = id;

#ifndef TRACY_NO_VERIFY
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneValidation );
        tracy::MemWrite( &item->zoneValidation.id, id );
        TracyLfqCommitC;
    }
#endif
    tracy::GetProfiler().SendCallstack( depth );
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneBeginCallstack );
        tracy::MemWrite( &item->zoneBegin.time, tracy::Profiler::GetTime() );
        tracy::MemWrite( &item->zoneBegin.srcloc, (uint64_t)srcloc );
        TracyLfqCommitC;
    }
    return ctx;
}

TRACY_API TracyCZoneCtx ___tracy_emit_zone_begin_alloc( uint64_t srcloc, int active )
{
    ___tracy_c_zone_context ctx;
#ifdef TRACY_ON_DEMAND
    ctx.active = active && tracy::GetProfiler().IsConnected();
#else
    ctx.active = active;
#endif
    if( !ctx.active )
    {
        tracy::tracy_free( (void*)srcloc );
        return ctx;
    }
    const auto id = tracy::GetProfiler().GetNextZoneId();
    ctx.id = id;

#ifndef TRACY_NO_VERIFY
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneValidation );
        tracy::MemWrite( &item->zoneValidation.id, id );
        TracyLfqCommitC;
    }
#endif
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneBeginAllocSrcLoc );
        tracy::MemWrite( &item->zoneBegin.time, tracy::Profiler::GetTime() );
        tracy::MemWrite( &item->zoneBegin.srcloc, srcloc );
        TracyLfqCommitC;
    }
    return ctx;
}

TRACY_API TracyCZoneCtx ___tracy_emit_zone_begin_alloc_callstack( uint64_t srcloc, int depth, int active )
{
    ___tracy_c_zone_context ctx;
#ifdef TRACY_ON_DEMAND
    ctx.active = active && tracy::GetProfiler().IsConnected();
#else
    ctx.active = active;
#endif
    if( !ctx.active )
    {
        tracy::tracy_free( (void*)srcloc );
        return ctx;
    }
    const auto id = tracy::GetProfiler().GetNextZoneId();
    ctx.id = id;

#ifndef TRACY_NO_VERIFY
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneValidation );
        tracy::MemWrite( &item->zoneValidation.id, id );
        TracyLfqCommitC;
    }
#endif
    tracy::GetProfiler().SendCallstack( depth );
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneBeginAllocSrcLocCallstack );
        tracy::MemWrite( &item->zoneBegin.time, tracy::Profiler::GetTime() );
        tracy::MemWrite( &item->zoneBegin.srcloc, srcloc );
        TracyLfqCommitC;
    }
    return ctx;
}

TRACY_API void ___tracy_emit_zone_end( TracyCZoneCtx ctx )
{
    if( !ctx.active ) return;
#ifndef TRACY_NO_VERIFY
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneValidation );
        tracy::MemWrite( &item->zoneValidation.id, ctx.id );
        TracyLfqCommitC;
    }
#endif
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneEnd );
        tracy::MemWrite( &item->zoneEnd.time, tracy::Profiler::GetTime() );
        TracyLfqCommitC;
    }
}

TRACY_API void ___tracy_emit_zone_text( TracyCZoneCtx ctx, const char* txt, size_t size )
{
    assert( size < std::numeric_limits<uint16_t>::max() );
    if( !ctx.active ) return;
    auto ptr = (char*)tracy::tracy_malloc( size );
    memcpy( ptr, txt, size );
#ifndef TRACY_NO_VERIFY
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneValidation );
        tracy::MemWrite( &item->zoneValidation.id, ctx.id );
        TracyLfqCommitC;
    }
#endif
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneText );
        tracy::MemWrite( &item->zoneTextFat.text, (uint64_t)ptr );
        tracy::MemWrite( &item->zoneTextFat.size, (uint16_t)size );
        TracyLfqCommitC;
    }
}

TRACY_API void ___tracy_emit_zone_name( TracyCZoneCtx ctx, const char* txt, size_t size )
{
    assert( size < std::numeric_limits<uint16_t>::max() );
    if( !ctx.active ) return;
    auto ptr = (char*)tracy::tracy_malloc( size );
    memcpy( ptr, txt, size );
#ifndef TRACY_NO_VERIFY
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneValidation );
        tracy::MemWrite( &item->zoneValidation.id, ctx.id );
        TracyLfqCommitC;
    }
#endif
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneName );
        tracy::MemWrite( &item->zoneTextFat.text, (uint64_t)ptr );
        tracy::MemWrite( &item->zoneTextFat.size, (uint16_t)size );
        TracyLfqCommitC;
    }
}

TRACY_API void ___tracy_emit_zone_color( TracyCZoneCtx ctx, uint32_t color ) {
    if( !ctx.active ) return;
#ifndef TRACY_NO_VERIFY
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneValidation );
        tracy::MemWrite( &item->zoneValidation.id, ctx.id );
        TracyLfqCommitC;
    }
#endif
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneColor );
        tracy::MemWrite( &item->zoneColor.r, uint8_t( ( color       ) & 0xFF ) );
        tracy::MemWrite( &item->zoneColor.g, uint8_t( ( color >> 8  ) & 0xFF ) );
        tracy::MemWrite( &item->zoneColor.b, uint8_t( ( color >> 16 ) & 0xFF ) );
        TracyLfqCommitC;
    }
}

TRACY_API void ___tracy_emit_zone_value( TracyCZoneCtx ctx, uint64_t value )
{
    if( !ctx.active ) return;
#ifndef TRACY_NO_VERIFY
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneValidation );
        tracy::MemWrite( &item->zoneValidation.id, ctx.id );
        TracyLfqCommitC;
    }
#endif
    {
        TracyLfqPrepareC( tracy::QueueType::ZoneValue );
        tracy::MemWrite( &item->zoneValue.value, value );
        TracyLfqCommitC;
    }
}

TRACY_API void ___tracy_emit_memory_alloc( const void* ptr, size_t size, int secure ) { tracy::Profiler::MemAlloc( ptr, size, secure != 0 ); }
TRACY_API void ___tracy_emit_memory_alloc_callstack( const void* ptr, size_t size, int depth, int secure ) { tracy::Profiler::MemAllocCallstack( ptr, size, depth, secure != 0 ); }
TRACY_API void ___tracy_emit_memory_free( const void* ptr, int secure ) { tracy::Profiler::MemFree( ptr, secure != 0 ); }
TRACY_API void ___tracy_emit_memory_free_callstack( const void* ptr, int depth, int secure ) { tracy::Profiler::MemFreeCallstack( ptr, depth, secure != 0 ); }
TRACY_API void ___tracy_emit_memory_alloc_named( const void* ptr, size_t size, int secure, const char* name ) { tracy::Profiler::MemAllocNamed( ptr, size, secure != 0, name ); }
TRACY_API void ___tracy_emit_memory_alloc_callstack_named( const void* ptr, size_t size, int depth, int secure, const char* name ) { tracy::Profiler::MemAllocCallstackNamed( ptr, size, depth, secure != 0, name ); }
TRACY_API void ___tracy_emit_memory_free_named( const void* ptr, int secure, const char* name ) { tracy::Profiler::MemFreeNamed( ptr, secure != 0, name ); }
TRACY_API void ___tracy_emit_memory_free_callstack_named( const void* ptr, int depth, int secure, const char* name ) { tracy::Profiler::MemFreeCallstackNamed( ptr, depth, secure != 0, name ); }
TRACY_API void ___tracy_emit_frame_mark( const char* name ) { tracy::Profiler::SendFrameMark( name ); }
TRACY_API void ___tracy_emit_frame_mark_start( const char* name ) { tracy::Profiler::SendFrameMark( name, tracy::QueueType::FrameMarkMsgStart ); }
TRACY_API void ___tracy_emit_frame_mark_end( const char* name ) { tracy::Profiler::SendFrameMark( name, tracy::QueueType::FrameMarkMsgEnd ); }
TRACY_API void ___tracy_emit_frame_image( const void* image, uint16_t w, uint16_t h, uint8_t offset, int flip ) { tracy::Profiler::SendFrameImage( image, w, h, offset, flip ); }
TRACY_API void ___tracy_emit_plot( const char* name, double val ) { tracy::Profiler::PlotData( name, val ); }
TRACY_API void ___tracy_emit_message( const char* txt, size_t size, int callstack ) { tracy::Profiler::Message( txt, size, callstack ); }
TRACY_API void ___tracy_emit_messageL( const char* txt, int callstack ) { tracy::Profiler::Message( txt, callstack ); }
TRACY_API void ___tracy_emit_messageC( const char* txt, size_t size, uint32_t color, int callstack ) { tracy::Profiler::MessageColor( txt, size, color, callstack ); }
TRACY_API void ___tracy_emit_messageLC( const char* txt, uint32_t color, int callstack ) { tracy::Profiler::MessageColor( txt, color, callstack ); }
TRACY_API void ___tracy_emit_message_appinfo( const char* txt, size_t size ) { tracy::Profiler::MessageAppInfo( txt, size ); }

TRACY_API uint64_t ___tracy_alloc_srcloc( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz ) {
    return tracy::Profiler::AllocSourceLocation( line, source, sourceSz, function, functionSz );
}

TRACY_API uint64_t ___tracy_alloc_srcloc_name( uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz, const char* name, size_t nameSz ) {
    return tracy::Profiler::AllocSourceLocation( line, source, sourceSz, function, functionSz, name, nameSz );
}

#ifdef __cplusplus
}
#endif

#endif

/*** End of inlined file: TracyProfiler.cpp ***/


/*** Start of inlined file: TracyCallstack.cpp ***/
#include <new>
#include <stdio.h>
#include <string.h>


/*** Start of inlined file: TracyStackFrames.hpp ***/
#ifndef __TRACYSTACKFRAMES_HPP__
#define __TRACYSTACKFRAMES_HPP__

#include <cstddef>

namespace tracy
{

struct StringMatch
{
    const char* str;
    size_t len;
};

extern const char** s_tracyStackFrames;
extern const StringMatch* s_tracySkipSubframes;

static constexpr int s_tracySkipSubframesMinLen = 7;

}

#endif

/*** End of inlined file: TracyStackFrames.hpp ***/


/*** Start of inlined file: TracyDebug.hpp ***/
#ifndef __TRACYPRINT_HPP__
#define __TRACYPRINT_HPP__

#ifdef TRACY_VERBOSE
#  include <stdio.h>
#  define TracyDebug(...) fprintf( stderr, __VA_ARGS__ );
#else
#  define TracyDebug(...)
#endif

#endif

/*** End of inlined file: TracyDebug.hpp ***/

#ifdef TRACY_HAS_CALLSTACK

#if TRACY_HAS_CALLSTACK == 1
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <psapi.h>
#  include <algorithm>
#  ifdef _MSC_VER
#    pragma warning( push )
#    pragma warning( disable : 4091 )
#  endif
#  include <dbghelp.h>
#  ifdef _MSC_VER
#    pragma warning( pop )
#  endif
#elif TRACY_HAS_CALLSTACK == 2 || TRACY_HAS_CALLSTACK == 3 || TRACY_HAS_CALLSTACK == 4 || TRACY_HAS_CALLSTACK == 6

/*** Start of inlined file: backtrace.hpp ***/
#ifndef BACKTRACE_H
#define BACKTRACE_H

#include <cstddef>
#include <cstdint>
#include <stdio.h>

namespace tracy
{

/* The backtrace state.  This struct is intentionally not defined in
   the public interface.  */

struct backtrace_state;

/* The type of the error callback argument to backtrace functions.
   This function, if not NULL, will be called for certain error cases.
   The DATA argument is passed to the function that calls this one.
   The MSG argument is an error message.  The ERRNUM argument, if
   greater than 0, holds an errno value.  The MSG buffer may become
   invalid after this function returns.

   As a special case, the ERRNUM argument will be passed as -1 if no
   debug info can be found for the executable, or if the debug info
   exists but has an unsupported version, but the function requires
   debug info (e.g., backtrace_full, backtrace_pcinfo).  The MSG in
   this case will be something along the lines of "no debug info".
   Similarly, ERRNUM will be passed as -1 if there is no symbol table,
   but the function requires a symbol table (e.g., backtrace_syminfo).
   This may be used as a signal that some other approach should be
   tried.  */

typedef void (*backtrace_error_callback) (void *data, const char *msg,
					  int errnum);

/* Create state information for the backtrace routines.  This must be
   called before any of the other routines, and its return value must
   be passed to all of the other routines.  FILENAME is the path name
   of the executable file; if it is NULL the library will try
   system-specific path names.  If not NULL, FILENAME must point to a
   permanent buffer.  If THREADED is non-zero the state may be
   accessed by multiple threads simultaneously, and the library will
   use appropriate atomic operations.  If THREADED is zero the state
   may only be accessed by one thread at a time.  This returns a state
   pointer on success, NULL on error.  If an error occurs, this will
   call the ERROR_CALLBACK routine.

   Calling this function allocates resources that cannot be freed.
   There is no backtrace_free_state function.  The state is used to
   cache information that is expensive to recompute.  Programs are
   expected to call this function at most once and to save the return
   value for all later calls to backtrace functions.  */

extern struct backtrace_state *backtrace_create_state (
    const char *filename, int threaded,
    backtrace_error_callback error_callback, void *data);

/* The type of the callback argument to the backtrace_full function.
   DATA is the argument passed to backtrace_full.  PC is the program
   counter.  FILENAME is the name of the file containing PC, or NULL
   if not available.  LINENO is the line number in FILENAME containing
   PC, or 0 if not available.  FUNCTION is the name of the function
   containing PC, or NULL if not available.  This should return 0 to
   continuing tracing.  The FILENAME and FUNCTION buffers may become
   invalid after this function returns.  */

typedef int (*backtrace_full_callback) (void *data, uintptr_t pc, uintptr_t lowaddr,
					const char *filename, int lineno,
					const char *function);

/* Get a full stack backtrace.  SKIP is the number of frames to skip;
   passing 0 will start the trace with the function calling
   backtrace_full.  DATA is passed to the callback routine.  If any
   call to CALLBACK returns a non-zero value, the stack backtrace
   stops, and backtrace returns that value; this may be used to limit
   the number of stack frames desired.  If all calls to CALLBACK
   return 0, backtrace returns 0.  The backtrace_full function will
   make at least one call to either CALLBACK or ERROR_CALLBACK.  This
   function requires debug info for the executable.  */

extern int backtrace_full (struct backtrace_state *state, int skip,
			   backtrace_full_callback callback,
			   backtrace_error_callback error_callback,
			   void *data);

/* The type of the callback argument to the backtrace_simple function.
   DATA is the argument passed to simple_backtrace.  PC is the program
   counter.  This should return 0 to continue tracing.  */

typedef int (*backtrace_simple_callback) (void *data, uintptr_t pc);

/* Get a simple backtrace.  SKIP is the number of frames to skip, as
   in backtrace.  DATA is passed to the callback routine.  If any call
   to CALLBACK returns a non-zero value, the stack backtrace stops,
   and backtrace_simple returns that value.  Otherwise
   backtrace_simple returns 0.  The backtrace_simple function will
   make at least one call to either CALLBACK or ERROR_CALLBACK.  This
   function does not require any debug info for the executable.  */

extern int backtrace_simple (struct backtrace_state *state, int skip,
			     backtrace_simple_callback callback,
			     backtrace_error_callback error_callback,
			     void *data);

/* Print the current backtrace in a user readable format to a FILE.
   SKIP is the number of frames to skip, as in backtrace_full.  Any
   error messages are printed to stderr.  This function requires debug
   info for the executable.  */

extern void backtrace_print (struct backtrace_state *state, int skip, FILE *);

/* Given PC, a program counter in the current program, call the
   callback function with filename, line number, and function name
   information.  This will normally call the callback function exactly
   once.  However, if the PC happens to describe an inlined call, and
   the debugging information contains the necessary information, then
   this may call the callback function multiple times.  This will make
   at least one call to either CALLBACK or ERROR_CALLBACK.  This
   returns the first non-zero value returned by CALLBACK, or 0.  */

extern int backtrace_pcinfo (struct backtrace_state *state, uintptr_t pc,
			     backtrace_full_callback callback,
			     backtrace_error_callback error_callback,
			     void *data);

/* The type of the callback argument to backtrace_syminfo.  DATA and
   PC are the arguments passed to backtrace_syminfo.  SYMNAME is the
   name of the symbol for the corresponding code.  SYMVAL is the
   value and SYMSIZE is the size of the symbol.  SYMNAME will be NULL
   if no error occurred but the symbol could not be found.  */

typedef void (*backtrace_syminfo_callback) (void *data, uintptr_t pc,
					    const char *symname,
					    uintptr_t symval,
					    uintptr_t symsize);

/* Given ADDR, an address or program counter in the current program,
   call the callback information with the symbol name and value
   describing the function or variable in which ADDR may be found.
   This will call either CALLBACK or ERROR_CALLBACK exactly once.
   This returns 1 on success, 0 on failure.  This function requires
   the symbol table but does not require the debug info.  Note that if
   the symbol table is present but ADDR could not be found in the
   table, CALLBACK will be called with a NULL SYMNAME argument.
   Returns 1 on success, 0 on error.  */

extern int backtrace_syminfo (struct backtrace_state *state, uintptr_t addr,
			      backtrace_syminfo_callback callback,
			      backtrace_error_callback error_callback,
			      void *data);

}

#endif

/*** End of inlined file: backtrace.hpp ***/


#  include <algorithm>
#  include <dlfcn.h>
#  include <cxxabi.h>
#  include <stdlib.h>

#elif TRACY_HAS_CALLSTACK == 5
#  include <dlfcn.h>
#  include <cxxabi.h>
#endif

#ifdef TRACY_DBGHELP_LOCK

#  define DBGHELP_INIT TracyConcat( TRACY_DBGHELP_LOCK, Init() )
#  define DBGHELP_LOCK TracyConcat( TRACY_DBGHELP_LOCK, Lock() );
#  define DBGHELP_UNLOCK TracyConcat( TRACY_DBGHELP_LOCK, Unlock() );

extern "C"
{
    void DBGHELP_INIT;
    void DBGHELP_LOCK;
    void DBGHELP_UNLOCK;
};
#endif

namespace tracy
{

static inline char* CopyString( const char* src, size_t sz )
{
    assert( strlen( src ) == sz );
    auto dst = (char*)tracy_malloc( sz + 1 );
    memcpy( dst, src, sz );
    dst[sz] = '\0';
    return dst;
}

static inline char* CopyString( const char* src )
{
    const auto sz = strlen( src );
    auto dst = (char*)tracy_malloc( sz + 1 );
    memcpy( dst, src, sz );
    dst[sz] = '\0';
    return dst;
}

static inline char* CopyStringFast( const char* src, size_t sz )
{
    assert( strlen( src ) == sz );
    auto dst = (char*)tracy_malloc_fast( sz + 1 );
    memcpy( dst, src, sz );
    dst[sz] = '\0';
    return dst;
}

static inline char* CopyStringFast( const char* src )
{
    const auto sz = strlen( src );
    auto dst = (char*)tracy_malloc_fast( sz + 1 );
    memcpy( dst, src, sz );
    dst[sz] = '\0';
    return dst;
}

#if TRACY_HAS_CALLSTACK == 1

enum { MaxCbTrace = 16 };
enum { MaxNameSize = 8*1024 };

int cb_num;
CallstackEntry cb_data[MaxCbTrace];

extern "C"
{
    typedef unsigned long (__stdcall *t_RtlWalkFrameChain)( void**, unsigned long, unsigned long );
    typedef DWORD (__stdcall *t_SymAddrIncludeInlineTrace)( HANDLE hProcess, DWORD64 Address );
    typedef BOOL (__stdcall *t_SymQueryInlineTrace)( HANDLE hProcess, DWORD64 StartAddress, DWORD StartContext, DWORD64 StartRetAddress, DWORD64 CurAddress, LPDWORD CurContext, LPDWORD CurFrameIndex );
    typedef BOOL (__stdcall *t_SymFromInlineContext)( HANDLE hProcess, DWORD64 Address, ULONG InlineContext, PDWORD64 Displacement, PSYMBOL_INFO Symbol );
    typedef BOOL (__stdcall *t_SymGetLineFromInlineContext)( HANDLE hProcess, DWORD64 qwAddr, ULONG InlineContext, DWORD64 qwModuleBaseAddress, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line64 );

    t_RtlWalkFrameChain RtlWalkFrameChain = 0;
    t_SymAddrIncludeInlineTrace _SymAddrIncludeInlineTrace = 0;
    t_SymQueryInlineTrace _SymQueryInlineTrace = 0;
    t_SymFromInlineContext _SymFromInlineContext = 0;
    t_SymGetLineFromInlineContext _SymGetLineFromInlineContext = 0;
}

#ifndef __CYGWIN__
struct ModuleCache
{
    uint64_t start;
    uint64_t end;
    char* name;
};

static FastVector<ModuleCache>* s_modCache;

struct KernelDriver
{
    uint64_t addr;
    const char* mod;
};

KernelDriver* s_krnlCache = nullptr;
size_t s_krnlCacheCnt;
#endif

void InitCallstack()
{
    RtlWalkFrameChain = (t_RtlWalkFrameChain)GetProcAddress( GetModuleHandleA( "ntdll.dll" ), "RtlWalkFrameChain" );
    _SymAddrIncludeInlineTrace = (t_SymAddrIncludeInlineTrace)GetProcAddress( GetModuleHandleA( "dbghelp.dll" ), "SymAddrIncludeInlineTrace" );
    _SymQueryInlineTrace = (t_SymQueryInlineTrace)GetProcAddress( GetModuleHandleA( "dbghelp.dll" ), "SymQueryInlineTrace" );
    _SymFromInlineContext = (t_SymFromInlineContext)GetProcAddress( GetModuleHandleA( "dbghelp.dll" ), "SymFromInlineContext" );
    _SymGetLineFromInlineContext = (t_SymGetLineFromInlineContext)GetProcAddress( GetModuleHandleA( "dbghelp.dll" ), "SymGetLineFromInlineContext" );

#ifdef TRACY_DBGHELP_LOCK
    DBGHELP_INIT;
    DBGHELP_LOCK;
#endif

    SymInitialize( GetCurrentProcess(), nullptr, true );
    SymSetOptions( SYMOPT_LOAD_LINES );

#ifndef __CYGWIN__
    DWORD needed;
    LPVOID dev[4096];
    if( EnumDeviceDrivers( dev, sizeof(dev), &needed ) != 0 )
    {
        char windir[MAX_PATH];
        if( !GetWindowsDirectoryA( windir, sizeof( windir ) ) ) memcpy( windir, "c:\\windows", 11 );
        const auto windirlen = strlen( windir );

        const auto sz = needed / sizeof( LPVOID );
        s_krnlCache = (KernelDriver*)tracy_malloc( sizeof(KernelDriver) * sz );
        int cnt = 0;
        for( size_t i=0; i<sz; i++ )
        {
            char fn[MAX_PATH];
            const auto len = GetDeviceDriverBaseNameA( dev[i], fn, sizeof( fn ) );
            if( len != 0 )
            {
                auto buf = (char*)tracy_malloc_fast( len+3 );
                buf[0] = '<';
                memcpy( buf+1, fn, len );
                memcpy( buf+len+1, ">", 2 );
                s_krnlCache[cnt++] = KernelDriver { (uint64_t)dev[i], buf };

                const auto len = GetDeviceDriverFileNameA( dev[i], fn, sizeof( fn ) );
                if( len != 0 )
                {
                    char full[MAX_PATH];
                    char* path = fn;

                    if( memcmp( fn, "\\SystemRoot\\", 12 ) == 0 )
                    {
                        memcpy( full, windir, windirlen );
                        strcpy( full + windirlen, fn + 11 );
                        path = full;
                    }

                    SymLoadModuleEx( GetCurrentProcess(), nullptr, path, nullptr, (DWORD64)dev[i], 0, nullptr, 0 );
                }
            }
        }
        s_krnlCacheCnt = cnt;
        std::sort( s_krnlCache, s_krnlCache + s_krnlCacheCnt, []( const KernelDriver& lhs, const KernelDriver& rhs ) { return lhs.addr > rhs.addr; } );
    }

    s_modCache = (FastVector<ModuleCache>*)tracy_malloc( sizeof( FastVector<ModuleCache> ) );
    new(s_modCache) FastVector<ModuleCache>( 512 );

    HANDLE proc = GetCurrentProcess();
    HMODULE mod[1024];
    if( EnumProcessModules( proc, mod, sizeof( mod ), &needed ) != 0 )
    {
        const auto sz = needed / sizeof( HMODULE );
        for( size_t i=0; i<sz; i++ )
        {
            MODULEINFO info;
            if( GetModuleInformation( proc, mod[i], &info, sizeof( info ) ) != 0 )
            {
                const auto base = uint64_t( info.lpBaseOfDll );
                char name[1024];
                const auto res = GetModuleFileNameA( mod[i], name, 1021 );
                if( res > 0 )
                {
                    auto ptr = name + res;
                    while( ptr > name && *ptr != '\\' && *ptr != '/' ) ptr--;
                    if( ptr > name ) ptr++;
                    const auto namelen = name + res - ptr;
                    auto cache = s_modCache->push_next();
                    cache->start = base;
                    cache->end = base + info.SizeOfImage;
                    cache->name = (char*)tracy_malloc_fast( namelen+3 );
                    cache->name[0] = '[';
                    memcpy( cache->name+1, ptr, namelen );
                    cache->name[namelen+1] = ']';
                    cache->name[namelen+2] = '\0';
                }
            }
        }
    }
#endif

#ifdef TRACY_DBGHELP_LOCK
    DBGHELP_UNLOCK;
#endif
}

TRACY_API uintptr_t* CallTrace( int depth )
{
    auto trace = (uintptr_t*)tracy_malloc( ( 1 + depth ) * sizeof( uintptr_t ) );
    const auto num = RtlWalkFrameChain( (void**)( trace + 1 ), depth, 0 );
    *trace = num;
    return trace;
}

const char* DecodeCallstackPtrFast( uint64_t ptr )
{
    static char ret[MaxNameSize];
    const auto proc = GetCurrentProcess();

    char buf[sizeof( SYMBOL_INFO ) + MaxNameSize];
    auto si = (SYMBOL_INFO*)buf;
    si->SizeOfStruct = sizeof( SYMBOL_INFO );
    si->MaxNameLen = MaxNameSize;

#ifdef TRACY_DBGHELP_LOCK
    DBGHELP_LOCK;
#endif
    if( SymFromAddr( proc, ptr, nullptr, si ) == 0 )
    {
        *ret = '\0';
    }
    else
    {
        memcpy( ret, si->Name, si->NameLen );
        ret[si->NameLen] = '\0';
    }
#ifdef TRACY_DBGHELP_LOCK
    DBGHELP_UNLOCK;
#endif
    return ret;
}

static const char* GetModuleName( uint64_t addr )
{
    if( ( addr >> 63 ) != 0 )
    {
#ifndef __CYGWIN__
        if( s_krnlCache )
        {
            auto it = std::lower_bound( s_krnlCache, s_krnlCache + s_krnlCacheCnt, addr, []( const KernelDriver& lhs, const uint64_t& rhs ) { return lhs.addr > rhs; } );
            if( it != s_krnlCache + s_krnlCacheCnt )
            {
                return it->mod;
            }
        }
#endif
        return "<kernel>";
    }

#ifndef __CYGWIN__
    for( auto& v : *s_modCache )
    {
        if( addr >= v.start && addr < v.end )
        {
            return v.name;
        }
    }

    HMODULE mod[1024];
    DWORD needed;
    HANDLE proc = GetCurrentProcess();

    InitRpmalloc();
    if( EnumProcessModules( proc, mod, sizeof( mod ), &needed ) != 0 )
    {
        const auto sz = needed / sizeof( HMODULE );
        for( size_t i=0; i<sz; i++ )
        {
            MODULEINFO info;
            if( GetModuleInformation( proc, mod[i], &info, sizeof( info ) ) != 0 )
            {
                const auto base = uint64_t( info.lpBaseOfDll );
                if( addr >= base && addr < base + info.SizeOfImage )
                {
                    char name[1024];
                    const auto res = GetModuleFileNameA( mod[i], name, 1021 );
                    if( res > 0 )
                    {
                        auto ptr = name + res;
                        while( ptr > name && *ptr != '\\' && *ptr != '/' ) ptr--;
                        if( ptr > name ) ptr++;
                        const auto namelen = name + res - ptr;
                        auto cache = s_modCache->push_next();
                        cache->start = base;
                        cache->end = base + info.SizeOfImage;
                        cache->name = (char*)tracy_malloc_fast( namelen+3 );
                        cache->name[0] = '[';
                        memcpy( cache->name+1, ptr, namelen );
                        cache->name[namelen+1] = ']';
                        cache->name[namelen+2] = '\0';
                        return cache->name;
                    }
                }
            }
        }
    }
#endif

    return "[unknown]";
}

CallstackSymbolData DecodeSymbolAddress( uint64_t ptr )
{
    CallstackSymbolData sym;
    IMAGEHLP_LINE64 line;
    DWORD displacement = 0;
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
#ifdef TRACY_DBGHELP_LOCK
    DBGHELP_LOCK;
#endif
    const auto res = SymGetLineFromAddr64( GetCurrentProcess(), ptr, &displacement, &line );
#ifdef TRACY_DBGHELP_LOCK
    DBGHELP_UNLOCK;
#endif
    if( res == 0 )
    {
        sym.file = "[unknown]";
        sym.line = 0;
    }
    else
    {
        sym.file = line.FileName;
        sym.line = line.LineNumber;
    }
    sym.needFree = false;
    return sym;
}

CallstackSymbolData DecodeCodeAddress( uint64_t ptr )
{
    CallstackSymbolData sym;
    const auto proc = GetCurrentProcess();
    bool done = false;

    char buf[sizeof( SYMBOL_INFO ) + MaxNameSize];
    auto si = (SYMBOL_INFO*)buf;
    si->SizeOfStruct = sizeof( SYMBOL_INFO );
    si->MaxNameLen = MaxNameSize;

    IMAGEHLP_LINE64 line;
    DWORD displacement = 0;
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

#ifdef TRACY_DBGHELP_LOCK
    DBGHELP_LOCK;
#endif
#if !defined TRACY_NO_CALLSTACK_INLINES
    if( _SymAddrIncludeInlineTrace )
    {
        DWORD inlineNum = _SymAddrIncludeInlineTrace( proc, ptr );
        DWORD ctx = 0;
        DWORD idx;
        BOOL doInline = FALSE;
        if( inlineNum != 0 ) doInline = _SymQueryInlineTrace( proc, ptr, 0, ptr, ptr, &ctx, &idx );
        if( doInline )
        {
            if( _SymGetLineFromInlineContext( proc, ptr, ctx, 0, &displacement, &line ) != 0 )
            {
                sym.file = line.FileName;
                sym.line = line.LineNumber;
                done = true;

                if( _SymFromInlineContext( proc, ptr, ctx, nullptr, si ) != 0 )
                {
                    sym.symAddr = si->Address;
                }
                else
                {
                    sym.symAddr = 0;
                }
            }
        }
    }
#endif
    if( !done )
    {
        if( SymGetLineFromAddr64( proc, ptr, &displacement, &line ) == 0 )
        {
            sym.file = "[unknown]";
            sym.line = 0;
            sym.symAddr = 0;
        }
        else
        {
            sym.file = line.FileName;
            sym.line = line.LineNumber;

            if( SymFromAddr( proc, ptr, nullptr, si ) != 0 )
            {
                sym.symAddr = si->Address;
            }
            else
            {
                sym.symAddr = 0;
            }
        }
    }
#ifdef TRACY_DBGHELP_LOCK
    DBGHELP_UNLOCK;
#endif
    sym.needFree = false;
    return sym;
}

CallstackEntryData DecodeCallstackPtr( uint64_t ptr )
{
    int write;
    const auto proc = GetCurrentProcess();
    InitRpmalloc();

#ifdef TRACY_DBGHELP_LOCK
    DBGHELP_LOCK;
#endif
#if !defined TRACY_NO_CALLSTACK_INLINES
    BOOL doInline = FALSE;
    DWORD ctx = 0;
    DWORD inlineNum = 0;
    if( _SymAddrIncludeInlineTrace )
    {
        inlineNum = _SymAddrIncludeInlineTrace( proc, ptr );
        if( inlineNum > MaxCbTrace - 1 ) inlineNum = MaxCbTrace - 1;
        DWORD idx;
        if( inlineNum != 0 ) doInline = _SymQueryInlineTrace( proc, ptr, 0, ptr, ptr, &ctx, &idx );
    }
    if( doInline )
    {
        write = inlineNum;
        cb_num = 1 + inlineNum;
    }
    else
#endif
    {
        write = 0;
        cb_num = 1;
    }

    char buf[sizeof( SYMBOL_INFO ) + MaxNameSize];
    auto si = (SYMBOL_INFO*)buf;
    si->SizeOfStruct = sizeof( SYMBOL_INFO );
    si->MaxNameLen = MaxNameSize;

    const auto moduleName = GetModuleName( ptr );
    const auto symValid = SymFromAddr( proc, ptr, nullptr, si ) != 0;

    IMAGEHLP_LINE64 line;
    DWORD displacement = 0;
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    {
        const char* filename;
        if( SymGetLineFromAddr64( proc, ptr, &displacement, &line ) == 0 )
        {
            filename = "[unknown]";
            cb_data[write].line = 0;
        }
        else
        {
            filename = line.FileName;
            cb_data[write].line = line.LineNumber;
        }

        cb_data[write].name = symValid ? CopyStringFast( si->Name, si->NameLen ) : CopyStringFast( moduleName );
        cb_data[write].file = CopyStringFast( filename );
        if( symValid )
        {
            cb_data[write].symLen = si->Size;
            cb_data[write].symAddr = si->Address;
        }
        else
        {
            cb_data[write].symLen = 0;
            cb_data[write].symAddr = 0;
        }
    }

#if !defined TRACY_NO_CALLSTACK_INLINES
    if( doInline )
    {
        for( DWORD i=0; i<inlineNum; i++ )
        {
            auto& cb = cb_data[i];
            const auto symInlineValid = _SymFromInlineContext( proc, ptr, ctx, nullptr, si ) != 0;
            const char* filename;
            if( _SymGetLineFromInlineContext( proc, ptr, ctx, 0, &displacement, &line ) == 0 )
            {
                filename = "[unknown]";
                cb.line = 0;
            }
            else
            {
                filename = line.FileName;
                cb.line = line.LineNumber;
            }

            cb.name = symInlineValid ? CopyStringFast( si->Name, si->NameLen ) : CopyStringFast( moduleName );
            cb.file = CopyStringFast( filename );
            if( symInlineValid )
            {
                cb.symLen = si->Size;
                cb.symAddr = si->Address;
            }
            else
            {
                cb.symLen = 0;
                cb.symAddr = 0;
            }

            ctx++;
        }
    }
#endif
#ifdef TRACY_DBGHELP_LOCK
    DBGHELP_UNLOCK;
#endif

    return { cb_data, uint8_t( cb_num ), moduleName };
}

#elif TRACY_HAS_CALLSTACK == 2 || TRACY_HAS_CALLSTACK == 3 || TRACY_HAS_CALLSTACK == 4 || TRACY_HAS_CALLSTACK == 6

enum { MaxCbTrace = 16 };

struct backtrace_state* cb_bts;
int cb_num;
CallstackEntry cb_data[MaxCbTrace];
int cb_fixup;

#ifdef __linux
struct KernelSymbol
{
    uint64_t addr;
    const char* name;
    const char* mod;
};

KernelSymbol* s_kernelSym = nullptr;
size_t s_kernelSymCnt;

static void InitKernelSymbols()
{
    FILE* f = fopen( "/proc/kallsyms", "rb" );
    if( !f ) return;
    tracy::FastVector<KernelSymbol> tmpSym( 1024 );
    size_t linelen = 16 * 1024;     // linelen must be big enough to prevent reallocs in getline()
    auto linebuf = (char*)tracy_malloc( linelen );
    ssize_t sz;
    while( ( sz = getline( &linebuf, &linelen, f ) ) != -1 )
    {
        auto ptr = linebuf;
        uint64_t addr = 0;
        while( *ptr != ' ' )
        {
            auto v = *ptr;
            if( v >= '0' && v <= '9' )
            {
                v -= '0';
            }
            else if( v >= 'a' && v <= 'f' )
            {
                v -= 'a';
                v += 10;
            }
            else if( v >= 'A' && v <= 'F' )
            {
                v -= 'A';
                v += 10;
            }
            else
            {
                assert( false );
            }
            assert( ( v & ~0xF ) == 0 );
            addr <<= 4;
            addr |= v;
            ptr++;
        }
        if( addr == 0 ) continue;
        ptr++;
        if( *ptr != 'T' && *ptr != 't' ) continue;
        ptr += 2;
        const auto namestart = ptr;
        while( *ptr != '\t' && *ptr != '\n' ) ptr++;
        const auto nameend = ptr;
        const char* modstart = nullptr;
        const char* modend;
        if( *ptr == '\t' )
        {
            ptr += 2;
            modstart = ptr;
            while( *ptr != ']' ) ptr++;
            modend = ptr;
        }

        auto strname = (char*)tracy_malloc_fast( nameend - namestart + 1 );
        memcpy( strname, namestart, nameend - namestart );
        strname[nameend-namestart] = '\0';

        char* strmod = nullptr;
        if( modstart )
        {
            strmod = (char*)tracy_malloc_fast( modend - modstart + 1 );
            memcpy( strmod, modstart, modend - modstart );
            strmod[modend-modstart] = '\0';
        }

        auto sym = tmpSym.push_next();
        sym->addr = addr;
        sym->name = strname;
        sym->mod = strmod;
    }
    tracy_free_fast( linebuf );
    fclose( f );
    if( tmpSym.empty() ) return;

    std::sort( tmpSym.begin(), tmpSym.end(), []( const KernelSymbol& lhs, const KernelSymbol& rhs ) { return lhs.addr > rhs.addr; } );
    s_kernelSymCnt = tmpSym.size();
    s_kernelSym = (KernelSymbol*)tracy_malloc_fast( sizeof( KernelSymbol ) * s_kernelSymCnt );
    memcpy( s_kernelSym, tmpSym.data(), sizeof( KernelSymbol ) * s_kernelSymCnt );
    TracyDebug( "Loaded %zu kernel symbols\n", s_kernelSymCnt );
}
#endif

void InitCallstack()
{
    cb_bts = backtrace_create_state( nullptr, 0, nullptr, nullptr );

#ifdef __linux
    InitKernelSymbols();
#endif
}

static int FastCallstackDataCb( void* data, uintptr_t pc, uintptr_t lowaddr, const char* fn, int lineno, const char* function )
{
    if( function )
    {
        strcpy( (char*)data, function );
    }
    else
    {
        const char* symname = nullptr;
        auto vptr = (void*)pc;
        Dl_info dlinfo;
        if( dladdr( vptr, &dlinfo ) )
        {
            symname = dlinfo.dli_sname;
        }
        if( symname )
        {
            strcpy( (char*)data, symname );
        }
        else
        {
            *(char*)data = '\0';
        }
    }
    return 1;
}

static void FastCallstackErrorCb( void* data, const char* /*msg*/, int /*errnum*/ )
{
    *(char*)data = '\0';
}

const char* DecodeCallstackPtrFast( uint64_t ptr )
{
    static char ret[1024];
    backtrace_pcinfo( cb_bts, ptr, FastCallstackDataCb, FastCallstackErrorCb, ret );
    return ret;
}

static int SymbolAddressDataCb( void* data, uintptr_t pc, uintptr_t lowaddr, const char* fn, int lineno, const char* function )
{
    auto& sym = *(CallstackSymbolData*)data;
    if( !fn )
    {
        sym.file = "[unknown]";
        sym.line = 0;
        sym.needFree = false;
    }
    else
    {
        sym.file = CopyString( fn );
        sym.line = lineno;
        sym.needFree = true;
    }

    return 1;
}

static void SymbolAddressErrorCb( void* data, const char* /*msg*/, int /*errnum*/ )
{
    auto& sym = *(CallstackSymbolData*)data;
    sym.file = "[unknown]";
    sym.line = 0;
    sym.needFree = false;
}

CallstackSymbolData DecodeSymbolAddress( uint64_t ptr )
{
    CallstackSymbolData sym;
    backtrace_pcinfo( cb_bts, ptr, SymbolAddressDataCb, SymbolAddressErrorCb, &sym );
    return sym;
}

static int CodeDataCb( void* data, uintptr_t pc, uintptr_t lowaddr, const char* fn, int lineno, const char* function )
{
    if( !fn ) return 1;

    const auto fnsz = strlen( fn );
    if( fnsz >= s_tracySkipSubframesMinLen )
    {
        auto ptr = s_tracySkipSubframes;
        do
        {
            if( fnsz >= ptr->len && memcmp( fn + fnsz - ptr->len, ptr->str, ptr->len ) == 0 ) return 0;
            ptr++;
        }
        while( ptr->str );
    }

    auto& sym = *(CallstackSymbolData*)data;
    sym.file = CopyString( fn );
    sym.line = lineno;
    sym.needFree = true;
    sym.symAddr = lowaddr;
    return 1;
}

static void CodeErrorCb( void* /*data*/, const char* /*msg*/, int /*errnum*/ )
{
}

CallstackSymbolData DecodeCodeAddress( uint64_t ptr )
{
    CallstackSymbolData sym = { "[unknown]", 0, false, 0 };
    backtrace_pcinfo( cb_bts, ptr, CodeDataCb, CodeErrorCb, &sym );
    return sym;
}

static int CallstackDataCb( void* /*data*/, uintptr_t pc, uintptr_t lowaddr, const char* fn, int lineno, const char* function )
{
    enum { DemangleBufLen = 64*1024 };
    char demangled[DemangleBufLen];

    cb_data[cb_num].symLen = 0;
    cb_data[cb_num].symAddr = (uint64_t)lowaddr;

    if( !fn && !function )
    {
        const char* symname = nullptr;
        auto vptr = (void*)pc;
        ptrdiff_t symoff = 0;

        Dl_info dlinfo;
        if( dladdr( vptr, &dlinfo ) )
        {
            symname = dlinfo.dli_sname;
            symoff = (char*)pc - (char*)dlinfo.dli_saddr;

            if( symname && symname[0] == '_' )
            {
                size_t len = DemangleBufLen;
                int status;
                abi::__cxa_demangle( symname, demangled, &len, &status );
                if( status == 0 )
                {
                    symname = demangled;
                }
            }
        }

        if( !symname ) symname = "[unknown]";

        if( symoff == 0 )
        {
            cb_data[cb_num].name = CopyStringFast( symname );
        }
        else
        {
            char buf[32];
            const auto offlen = sprintf( buf, " + %td", symoff );
            const auto namelen = strlen( symname );
            auto name = (char*)tracy_malloc_fast( namelen + offlen + 1 );
            memcpy( name, symname, namelen );
            memcpy( name + namelen, buf, offlen );
            name[namelen + offlen] = '\0';
            cb_data[cb_num].name = name;
        }

        cb_data[cb_num].file = CopyStringFast( "[unknown]" );
        cb_data[cb_num].line = 0;
    }
    else
    {
        if( !fn ) fn = "[unknown]";
        if( !function )
        {
            function = "[unknown]";
        }
        else
        {
            if( function[0] == '_' )
            {
                size_t len = DemangleBufLen;
                int status;
                abi::__cxa_demangle( function, demangled, &len, &status );
                if( status == 0 )
                {
                    function = demangled;
                }
            }
        }

        cb_data[cb_num].name = CopyStringFast( function );
        cb_data[cb_num].file = CopyStringFast( fn );
        cb_data[cb_num].line = lineno;
    }

    if( ++cb_num >= MaxCbTrace )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void CallstackErrorCb( void* /*data*/, const char* /*msg*/, int /*errnum*/ )
{
    for( int i=0; i<cb_num; i++ )
    {
        tracy_free_fast( (void*)cb_data[i].name );
        tracy_free_fast( (void*)cb_data[i].file );
    }

    cb_data[0].name = CopyStringFast( "[error]" );
    cb_data[0].file = CopyStringFast( "[error]" );
    cb_data[0].line = 0;

    cb_num = 1;
}

void SymInfoCallback( void* /*data*/, uintptr_t pc, const char* symname, uintptr_t symval, uintptr_t symsize )
{
    cb_data[cb_num-1].symLen = (uint32_t)symsize;
    cb_data[cb_num-1].symAddr = (uint64_t)symval;
}

void SymInfoError( void* /*data*/, const char* /*msg*/, int /*errnum*/ )
{
    cb_data[cb_num-1].symLen = 0;
    cb_data[cb_num-1].symAddr = 0;
}

CallstackEntryData DecodeCallstackPtr( uint64_t ptr )
{
    InitRpmalloc();
    if( ptr >> 63 == 0 )
    {
        cb_num = 0;
        backtrace_pcinfo( cb_bts, ptr, CallstackDataCb, CallstackErrorCb, nullptr );
        assert( cb_num > 0 );

        backtrace_syminfo( cb_bts, ptr, SymInfoCallback, SymInfoError, nullptr );

        const char* symloc = nullptr;
        Dl_info dlinfo;
        if( dladdr( (void*)ptr, &dlinfo ) ) symloc = dlinfo.dli_fname;

        return { cb_data, uint8_t( cb_num ), symloc ? symloc : "[unknown]" };
    }
#ifdef __linux
    else if( s_kernelSym )
    {
        auto it = std::lower_bound( s_kernelSym, s_kernelSym + s_kernelSymCnt, ptr, []( const KernelSymbol& lhs, const uint64_t& rhs ) { return lhs.addr > rhs; } );
        if( it != s_kernelSym + s_kernelSymCnt )
        {
            cb_data[0].name = CopyStringFast( it->name );
            cb_data[0].file = CopyStringFast( "<kernel>" );
            cb_data[0].line = 0;
            cb_data[0].symLen = 0;
            cb_data[0].symAddr = it->addr;
            return { cb_data, 1, it->mod ? it->mod : "<kernel>" };
        }
    }
#endif

    cb_data[0].name = CopyStringFast( "[unknown]" );
    cb_data[0].file = CopyStringFast( "<kernel>" );
    cb_data[0].line = 0;
    cb_data[0].symLen = 0;
    cb_data[0].symAddr = 0;
    return { cb_data, 1, "<kernel>" };
}

#elif TRACY_HAS_CALLSTACK == 5

void InitCallstack()
{
}

const char* DecodeCallstackPtrFast( uint64_t ptr )
{
    static char ret[1024];
    auto vptr = (void*)ptr;
    const char* symname = nullptr;
    Dl_info dlinfo;
    if( dladdr( vptr, &dlinfo ) && dlinfo.dli_sname )
    {
        symname = dlinfo.dli_sname;
    }
    if( symname )
    {
        strcpy( ret, symname );
    }
    else
    {
        *ret = '\0';
    }
    return ret;
}

CallstackSymbolData DecodeSymbolAddress( uint64_t ptr )
{
    const char* symloc = nullptr;
    Dl_info dlinfo;
    if( dladdr( (void*)ptr, &dlinfo ) ) symloc = dlinfo.dli_fname;
    if( !symloc ) symloc = "[unknown]";
    return CallstackSymbolData { symloc, 0, false, 0 };
}

CallstackSymbolData DecodeCodeAddress( uint64_t ptr )
{
    return DecodeSymbolAddress( ptr );
}

CallstackEntryData DecodeCallstackPtr( uint64_t ptr )
{
    static CallstackEntry cb;
    cb.line = 0;

    char* demangled = nullptr;
    const char* symname = nullptr;
    const char* symloc = nullptr;
    auto vptr = (void*)ptr;
    ptrdiff_t symoff = 0;
    void* symaddr = nullptr;

    Dl_info dlinfo;
    if( dladdr( vptr, &dlinfo ) )
    {
        symloc = dlinfo.dli_fname;
        symname = dlinfo.dli_sname;
        symoff = (char*)ptr - (char*)dlinfo.dli_saddr;
        symaddr = dlinfo.dli_saddr;

        if( symname && symname[0] == '_' )
        {
            size_t len = 0;
            int status;
            demangled = abi::__cxa_demangle( symname, nullptr, &len, &status );
            if( status == 0 )
            {
                symname = demangled;
            }
        }
    }

    if( !symname ) symname = "[unknown]";
    if( !symloc ) symloc = "[unknown]";

    if( symoff == 0 )
    {
        cb.name = CopyString( symname );
    }
    else
    {
        char buf[32];
        const auto offlen = sprintf( buf, " + %td", symoff );
        const auto namelen = strlen( symname );
        auto name = (char*)tracy_malloc( namelen + offlen + 1 );
        memcpy( name, symname, namelen );
        memcpy( name + namelen, buf, offlen );
        name[namelen + offlen] = '\0';
        cb.name = name;
    }

    cb.file = CopyString( "[unknown]" );
    cb.symLen = 0;
    cb.symAddr = (uint64_t)symaddr;

    if( demangled ) free( demangled );

    return { &cb, 1, symloc };
}

#endif

}

#endif

/*** End of inlined file: TracyCallstack.cpp ***/


/*** Start of inlined file: TracySysTime.cpp ***/
#ifdef TRACY_HAS_SYSTIME

#  if defined _WIN32 || defined __CYGWIN__
#    include <windows.h>
#  elif defined __linux__
#    include <stdio.h>
#    include <inttypes.h>
#  elif defined __APPLE__
#    include <mach/mach_host.h>
#    include <mach/host_info.h>
#  elif defined BSD
#    include <sys/types.h>
#    include <sys/sysctl.h>
#  endif

namespace tracy
{

#  if defined _WIN32 || defined __CYGWIN__

static inline uint64_t ConvertTime( const FILETIME& t )
{
    return ( uint64_t( t.dwHighDateTime ) << 32 ) | uint64_t( t.dwLowDateTime );
}

void SysTime::ReadTimes()
{
    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;

    GetSystemTimes( &idleTime, &kernelTime, &userTime );

    idle = ConvertTime( idleTime );
    const auto kernel = ConvertTime( kernelTime );
    const auto user = ConvertTime( userTime );
    used = kernel + user;
}

#  elif defined __linux__

void SysTime::ReadTimes()
{
    uint64_t user, nice, system;
    FILE* f = fopen( "/proc/stat", "r" );
    if( f )
    {
        int read = fscanf( f, "cpu %" PRIu64 " %" PRIu64 " %" PRIu64" %" PRIu64, &user, &nice, &system, &idle );
        fclose( f );
        if (read == 4)
        {
            used = user + nice + system;
        }
    }
}

#  elif defined __APPLE__

void SysTime::ReadTimes()
{
    host_cpu_load_info_data_t info;
    mach_msg_type_number_t cnt = HOST_CPU_LOAD_INFO_COUNT;
    host_statistics( mach_host_self(), HOST_CPU_LOAD_INFO, reinterpret_cast<host_info_t>( &info ), &cnt );
    used = info.cpu_ticks[CPU_STATE_USER] + info.cpu_ticks[CPU_STATE_NICE] + info.cpu_ticks[CPU_STATE_SYSTEM];
    idle = info.cpu_ticks[CPU_STATE_IDLE];
}

#  elif defined BSD

void SysTime::ReadTimes()
{
    u_long data[5];
    size_t sz = sizeof( data );
    sysctlbyname( "kern.cp_time", &data, &sz, nullptr, 0 );
    used = data[0] + data[1] + data[2] + data[3];
    idle = data[4];
}

#endif

SysTime::SysTime()
{
    ReadTimes();
}

float SysTime::Get()
{
    const auto oldUsed = used;
    const auto oldIdle = idle;

    ReadTimes();

    const auto diffIdle = idle - oldIdle;
    const auto diffUsed = used - oldUsed;

#if defined _WIN32 || defined __CYGWIN__
    return diffUsed == 0 ? -1 : ( diffUsed - diffIdle ) * 100.f / diffUsed;
#elif defined __linux__ || defined __APPLE__ || defined BSD
    const auto total = diffUsed + diffIdle;
    return total == 0 ? -1 : diffUsed * 100.f / total;
#endif
}

}

#endif

/*** End of inlined file: TracySysTime.cpp ***/


/*** Start of inlined file: TracySysTrace.cpp ***/
#ifdef TRACY_HAS_SYSTEM_TRACING

#ifndef TRACY_SAMPLING_HZ
#  if defined _WIN32 || defined __CYGWIN__
#    define TRACY_SAMPLING_HZ 8000
#  elif defined __linux__
#    define TRACY_SAMPLING_HZ 10000
#  endif
#endif

namespace tracy
{

static constexpr int GetSamplingFrequency()
{
#if defined _WIN32 || defined __CYGWIN__
    return TRACY_SAMPLING_HZ > 8000 ? 8000 : ( TRACY_SAMPLING_HZ < 1 ? 1 : TRACY_SAMPLING_HZ );
#else
    return TRACY_SAMPLING_HZ > 1000000 ? 1000000 : ( TRACY_SAMPLING_HZ < 1 ? 1 : TRACY_SAMPLING_HZ );
#endif
}

static constexpr int GetSamplingPeriod()
{
    return 1000000000 / GetSamplingFrequency();
}

}

#  if defined _WIN32 || defined __CYGWIN__

#    ifndef NOMINMAX
#      define NOMINMAX
#    endif

#    define INITGUID
#    include <cassert>
#    include <string.h>
#    include <windows.h>
#    include <dbghelp.h>
#    include <evntrace.h>
#    include <evntcons.h>
#    include <psapi.h>
#    include <winternl.h>

namespace tracy
{

static const GUID PerfInfoGuid = { 0xce1dbfb4, 0x137e, 0x4da6, { 0x87, 0xb0, 0x3f, 0x59, 0xaa, 0x10, 0x2c, 0xbc } };
static const GUID DxgKrnlGuid  = { 0x802ec45a, 0x1e99, 0x4b83, { 0x99, 0x20, 0x87, 0xc9, 0x82, 0x77, 0xba, 0x9d } };

static TRACEHANDLE s_traceHandle;
static TRACEHANDLE s_traceHandle2;
static EVENT_TRACE_PROPERTIES* s_prop;
static DWORD s_pid;

static EVENT_TRACE_PROPERTIES* s_propVsync;
static TRACEHANDLE s_traceHandleVsync;
static TRACEHANDLE s_traceHandleVsync2;
Thread* s_threadVsync = nullptr;

struct CSwitch
{
    uint32_t    newThreadId;
    uint32_t    oldThreadId;
    int8_t      newThreadPriority;
    int8_t      oldThreadPriority;
    uint8_t     previousCState;
    int8_t      spareByte;
    int8_t      oldThreadWaitReason;
    int8_t      oldThreadWaitMode;
    int8_t      oldThreadState;
    int8_t      oldThreadWaitIdealProcessor;
    uint32_t    newThreadWaitTime;
    uint32_t    reserved;
};

struct ReadyThread
{
    uint32_t    threadId;
    int8_t      adjustReason;
    int8_t      adjustIncrement;
    int8_t      flag;
    int8_t      reserverd;
};

struct ThreadTrace
{
    uint32_t processId;
    uint32_t threadId;
    uint32_t stackBase;
    uint32_t stackLimit;
    uint32_t userStackBase;
    uint32_t userStackLimit;
    uint32_t startAddr;
    uint32_t win32StartAddr;
    uint32_t tebBase;
    uint32_t subProcessTag;
};

struct StackWalkEvent
{
    uint64_t eventTimeStamp;
    uint32_t stackProcess;
    uint32_t stackThread;
    uint64_t stack[192];
};

struct VSyncInfo
{
    void*       dxgAdapter;
    uint32_t    vidPnTargetId;
    uint64_t    scannedPhysicalAddress;
    uint32_t    vidPnSourceId;
    uint32_t    frameNumber;
    int64_t     frameQpcTime;
    void*       hFlipDevice;
    uint32_t    flipType;
    uint64_t    flipFenceId;
};

#ifdef __CYGWIN__
extern "C" typedef DWORD (WINAPI *t_GetProcessIdOfThread)( HANDLE );
extern "C" typedef DWORD (WINAPI *t_GetProcessImageFileNameA)( HANDLE, LPSTR, DWORD );
extern "C" ULONG WMIAPI TraceSetInformation(TRACEHANDLE SessionHandle, TRACE_INFO_CLASS InformationClass, PVOID TraceInformation, ULONG InformationLength);
t_GetProcessIdOfThread GetProcessIdOfThread = (t_GetProcessIdOfThread)GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "GetProcessIdOfThread" );
t_GetProcessImageFileNameA GetProcessImageFileNameA = (t_GetProcessImageFileNameA)GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "K32GetProcessImageFileNameA" );
#endif

extern "C" typedef NTSTATUS (WINAPI *t_NtQueryInformationThread)( HANDLE, THREADINFOCLASS, PVOID, ULONG, PULONG );
extern "C" typedef BOOL (WINAPI *t_EnumProcessModules)( HANDLE, HMODULE*, DWORD, LPDWORD );
extern "C" typedef BOOL (WINAPI *t_GetModuleInformation)( HANDLE, HMODULE, LPMODULEINFO, DWORD );
extern "C" typedef DWORD (WINAPI *t_GetModuleBaseNameA)( HANDLE, HMODULE, LPSTR, DWORD );
extern "C" typedef HRESULT (WINAPI *t_GetThreadDescription)( HANDLE, PWSTR* );

t_NtQueryInformationThread NtQueryInformationThread = (t_NtQueryInformationThread)GetProcAddress( GetModuleHandleA( "ntdll.dll" ), "NtQueryInformationThread" );
t_EnumProcessModules _EnumProcessModules = (t_EnumProcessModules)GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "K32EnumProcessModules" );
t_GetModuleInformation _GetModuleInformation = (t_GetModuleInformation)GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "K32GetModuleInformation" );
t_GetModuleBaseNameA _GetModuleBaseNameA = (t_GetModuleBaseNameA)GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "K32GetModuleBaseNameA" );

static t_GetThreadDescription _GetThreadDescription = 0;

void WINAPI EventRecordCallback( PEVENT_RECORD record )
{
#ifdef TRACY_ON_DEMAND
    if( !GetProfiler().IsConnected() ) return;
#endif

    const auto& hdr = record->EventHeader;
    switch( hdr.ProviderId.Data1 )
    {
    case 0x3d6fa8d1:    // Thread Guid
        if( hdr.EventDescriptor.Opcode == 36 )
        {
            const auto cswitch = (const CSwitch*)record->UserData;

            TracyLfqPrepare( QueueType::ContextSwitch );
            MemWrite( &item->contextSwitch.time, hdr.TimeStamp.QuadPart );
            memcpy( &item->contextSwitch.oldThread, &cswitch->oldThreadId, sizeof( cswitch->oldThreadId ) );
            memcpy( &item->contextSwitch.newThread, &cswitch->newThreadId, sizeof( cswitch->newThreadId ) );
            memset( ((char*)&item->contextSwitch.oldThread)+4, 0, 4 );
            memset( ((char*)&item->contextSwitch.newThread)+4, 0, 4 );
            MemWrite( &item->contextSwitch.cpu, record->BufferContext.ProcessorNumber );
            MemWrite( &item->contextSwitch.reason, cswitch->oldThreadWaitReason );
            MemWrite( &item->contextSwitch.state, cswitch->oldThreadState );
            TracyLfqCommit;
        }
        else if( hdr.EventDescriptor.Opcode == 50 )
        {
            const auto rt = (const ReadyThread*)record->UserData;

            TracyLfqPrepare( QueueType::ThreadWakeup );
            MemWrite( &item->threadWakeup.time, hdr.TimeStamp.QuadPart );
            memcpy( &item->threadWakeup.thread, &rt->threadId, sizeof( rt->threadId ) );
            memset( ((char*)&item->threadWakeup.thread)+4, 0, 4 );
            TracyLfqCommit;
        }
        else if( hdr.EventDescriptor.Opcode == 1 || hdr.EventDescriptor.Opcode == 3 )
        {
            const auto tt = (const ThreadTrace*)record->UserData;

            uint64_t tid = tt->threadId;
            if( tid == 0 ) return;
            uint64_t pid = tt->processId;
            TracyLfqPrepare( QueueType::TidToPid );
            MemWrite( &item->tidToPid.tid, tid );
            MemWrite( &item->tidToPid.pid, pid );
            TracyLfqCommit;
        }
        break;
    case 0xdef2fe46:    // StackWalk Guid
        if( hdr.EventDescriptor.Opcode == 32 )
        {
            const auto sw = (const StackWalkEvent*)record->UserData;
            if( sw->stackProcess == s_pid )
            {
                const uint64_t sz = ( record->UserDataLength - 16 ) / 8;
                if( sz > 0 )
                {
                    auto trace = (uint64_t*)tracy_malloc( ( 1 + sz ) * sizeof( uint64_t ) );
                    memcpy( trace, &sz, sizeof( uint64_t ) );
                    memcpy( trace+1, sw->stack, sizeof( uint64_t ) * sz );
                    TracyLfqPrepare( QueueType::CallstackSample );
                    MemWrite( &item->callstackSampleFat.time, sw->eventTimeStamp );
                    MemWrite( &item->callstackSampleFat.thread, (uint64_t)sw->stackThread );
                    MemWrite( &item->callstackSampleFat.ptr, (uint64_t)trace );
                    TracyLfqCommit;
                }
            }
        }
        break;
    default:
        break;
    }
}

static constexpr const char* VsyncName[] = {
    "[0] Vsync",
    "[1] Vsync",
    "[2] Vsync",
    "[3] Vsync",
    "[4] Vsync",
    "[5] Vsync",
    "[6] Vsync",
    "[7] Vsync",
    "Vsync"
};

static uint32_t VsyncTarget[8] = {};

void WINAPI EventRecordCallbackVsync( PEVENT_RECORD record )
{
#ifdef TRACY_ON_DEMAND
    if( !GetProfiler().IsConnected() ) return;
#endif

    const auto& hdr = record->EventHeader;
    assert( hdr.ProviderId.Data1 == 0x802EC45A );
    assert( hdr.EventDescriptor.Id == 0x0011 );

    const auto vs = (const VSyncInfo*)record->UserData;

    int idx = 0;
    do
    {
        if( VsyncTarget[idx] == 0 )
        {
            VsyncTarget[idx] = vs->vidPnTargetId;
            break;
        }
        else if( VsyncTarget[idx] == vs->vidPnTargetId )
        {
            break;
        }
    }
    while( ++idx < 8 );

    TracyLfqPrepare( QueueType::FrameMarkMsg );
    MemWrite( &item->frameMark.time, hdr.TimeStamp.QuadPart );
    MemWrite( &item->frameMark.name, uint64_t( VsyncName[idx] ) );
    TracyLfqCommit;
}

static void SetupVsync()
{
#if _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
    const auto psz = sizeof( EVENT_TRACE_PROPERTIES ) + MAX_PATH;
    s_propVsync = (EVENT_TRACE_PROPERTIES*)tracy_malloc( psz );
    memset( s_propVsync, 0, sizeof( EVENT_TRACE_PROPERTIES ) );
    s_propVsync->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    s_propVsync->Wnode.BufferSize = psz;
#ifdef TRACY_TIMER_QPC
    s_propVsync->Wnode.ClientContext = 1;
#else
    s_propVsync->Wnode.ClientContext = 3;
#endif
    s_propVsync->LoggerNameOffset = sizeof( EVENT_TRACE_PROPERTIES );
    strcpy( ((char*)s_propVsync) + sizeof( EVENT_TRACE_PROPERTIES ), "TracyVsync" );

    auto backup = tracy_malloc( psz );
    memcpy( backup, s_propVsync, psz );

    const auto controlStatus = ControlTraceA( 0, "TracyVsync", s_propVsync, EVENT_TRACE_CONTROL_STOP );
    if( controlStatus != ERROR_SUCCESS && controlStatus != ERROR_WMI_INSTANCE_NOT_FOUND )
    {
        tracy_free( backup );
        tracy_free( s_propVsync );
        return;
    }

    memcpy( s_propVsync, backup, psz );
    tracy_free( backup );

    const auto startStatus = StartTraceA( &s_traceHandleVsync, "TracyVsync", s_propVsync );
    if( startStatus != ERROR_SUCCESS )
    {
        tracy_free( s_propVsync );
        return;
    }

    EVENT_FILTER_EVENT_ID fe = {};
    fe.FilterIn = TRUE;
    fe.Count = 1;
    fe.Events[0] = 0x0011;  // VSyncDPC_Info

    EVENT_FILTER_DESCRIPTOR desc = {};
    desc.Ptr = (ULONGLONG)&fe;
    desc.Size = sizeof( fe );
    desc.Type = EVENT_FILTER_TYPE_EVENT_ID;

    ENABLE_TRACE_PARAMETERS params = {};
    params.Version = ENABLE_TRACE_PARAMETERS_VERSION_2;
    params.EnableProperty = EVENT_ENABLE_PROPERTY_IGNORE_KEYWORD_0;
    params.SourceId = s_propVsync->Wnode.Guid;
    params.EnableFilterDesc = &desc;
    params.FilterDescCount = 1;

    uint64_t mask = 0x4000000000000001;   // Microsoft_Windows_DxgKrnl_Performance | Base
    if( EnableTraceEx2( s_traceHandleVsync, &DxgKrnlGuid, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_INFORMATION, mask, mask, 0, &params ) != ERROR_SUCCESS )
    {
        tracy_free( s_propVsync );
        return;
    }

    char loggerName[MAX_PATH];
    strcpy( loggerName, "TracyVsync" );

    EVENT_TRACE_LOGFILEA log = {};
    log.LoggerName = loggerName;
    log.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_RAW_TIMESTAMP;
    log.EventRecordCallback = EventRecordCallbackVsync;

    s_traceHandleVsync2 = OpenTraceA( &log );
    if( s_traceHandleVsync2 == (TRACEHANDLE)INVALID_HANDLE_VALUE )
    {
        CloseTrace( s_traceHandleVsync );
        tracy_free( s_propVsync );
        return;
    }

    s_threadVsync = (Thread*)tracy_malloc( sizeof( Thread ) );
    new(s_threadVsync) Thread( [] (void*) {
        ThreadExitHandler threadExitHandler;
        SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
        SetThreadName( "Tracy Vsync" );
        ProcessTrace( &s_traceHandleVsync2, 1, nullptr, nullptr );
    }, nullptr );
#endif
}

static constexpr int GetSamplingInterval()
{
    return GetSamplingPeriod() / 100;
}

bool SysTraceStart( int64_t& samplingPeriod )
{
    if( !_GetThreadDescription ) _GetThreadDescription = (t_GetThreadDescription)GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "GetThreadDescription" );

    s_pid = GetCurrentProcessId();

#if defined _WIN64
    constexpr bool isOs64Bit = true;
#else
    BOOL _iswow64;
    IsWow64Process( GetCurrentProcess(), &_iswow64 );
    const bool isOs64Bit = _iswow64;
#endif

    TOKEN_PRIVILEGES priv = {};
    priv.PrivilegeCount = 1;
    priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if( LookupPrivilegeValue( nullptr, SE_SYSTEM_PROFILE_NAME, &priv.Privileges[0].Luid ) == 0 ) return false;

    HANDLE pt;
    if( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &pt ) == 0 ) return false;
    const auto adjust = AdjustTokenPrivileges( pt, FALSE, &priv, 0, nullptr, nullptr );
    CloseHandle( pt );
    if( adjust == 0 ) return false;
    const auto status = GetLastError();
    if( status != ERROR_SUCCESS ) return false;

    if( isOs64Bit )
    {
        TRACE_PROFILE_INTERVAL interval = {};
        interval.Interval = GetSamplingInterval();
        const auto intervalStatus = TraceSetInformation( 0, TraceSampledProfileIntervalInfo, &interval, sizeof( interval ) );
        if( intervalStatus != ERROR_SUCCESS ) return false;
        samplingPeriod = GetSamplingPeriod();
    }

    const auto psz = sizeof( EVENT_TRACE_PROPERTIES ) + sizeof( KERNEL_LOGGER_NAME );
    s_prop = (EVENT_TRACE_PROPERTIES*)tracy_malloc( psz );
    memset( s_prop, 0, sizeof( EVENT_TRACE_PROPERTIES ) );
    ULONG flags = 0;
#ifndef TRACY_NO_CONTEXT_SWITCH
    flags = EVENT_TRACE_FLAG_CSWITCH | EVENT_TRACE_FLAG_DISPATCHER | EVENT_TRACE_FLAG_THREAD;
#endif
#ifndef TRACY_NO_SAMPLING
    if( isOs64Bit ) flags |= EVENT_TRACE_FLAG_PROFILE;
#endif
    s_prop->EnableFlags = flags;
    s_prop->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    s_prop->Wnode.BufferSize = psz;
    s_prop->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
#ifdef TRACY_TIMER_QPC
    s_prop->Wnode.ClientContext = 1;
#else
    s_prop->Wnode.ClientContext = 3;
#endif
    s_prop->Wnode.Guid = SystemTraceControlGuid;
    s_prop->BufferSize = 1024;
    s_prop->MinimumBuffers = std::thread::hardware_concurrency() * 4;
    s_prop->MaximumBuffers = std::thread::hardware_concurrency() * 6;
    s_prop->LoggerNameOffset = sizeof( EVENT_TRACE_PROPERTIES );
    memcpy( ((char*)s_prop) + sizeof( EVENT_TRACE_PROPERTIES ), KERNEL_LOGGER_NAME, sizeof( KERNEL_LOGGER_NAME ) );

    auto backup = tracy_malloc( psz );
    memcpy( backup, s_prop, psz );

    const auto controlStatus = ControlTrace( 0, KERNEL_LOGGER_NAME, s_prop, EVENT_TRACE_CONTROL_STOP );
    if( controlStatus != ERROR_SUCCESS && controlStatus != ERROR_WMI_INSTANCE_NOT_FOUND )
    {
        tracy_free( backup );
        tracy_free( s_prop );
        return false;
    }

    memcpy( s_prop, backup, psz );
    tracy_free( backup );

    const auto startStatus = StartTrace( &s_traceHandle, KERNEL_LOGGER_NAME, s_prop );
    if( startStatus != ERROR_SUCCESS )
    {
        tracy_free( s_prop );
        return false;
    }

    if( isOs64Bit )
    {
        CLASSIC_EVENT_ID stackId;
        stackId.EventGuid = PerfInfoGuid;
        stackId.Type = 46;
        const auto stackStatus = TraceSetInformation( s_traceHandle, TraceStackTracingInfo, &stackId, sizeof( stackId ) );
        if( stackStatus != ERROR_SUCCESS )
        {
            tracy_free( s_prop );
            return false;
        }
    }

#ifdef UNICODE
    WCHAR KernelLoggerName[sizeof( KERNEL_LOGGER_NAME )];
#else
    char KernelLoggerName[sizeof( KERNEL_LOGGER_NAME )];
#endif
    memcpy( KernelLoggerName, KERNEL_LOGGER_NAME, sizeof( KERNEL_LOGGER_NAME ) );
    EVENT_TRACE_LOGFILE log = {};
    log.LoggerName = KernelLoggerName;
    log.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_RAW_TIMESTAMP;
    log.EventRecordCallback = EventRecordCallback;

    s_traceHandle2 = OpenTrace( &log );
    if( s_traceHandle2 == (TRACEHANDLE)INVALID_HANDLE_VALUE )
    {
        CloseTrace( s_traceHandle );
        tracy_free( s_prop );
        return false;
    }

#ifndef TRACY_NO_VSYNC_CAPTURE
    SetupVsync();
#endif

    return true;
}

void SysTraceStop()
{
    if( s_threadVsync )
    {
        CloseTrace( s_traceHandleVsync2 );
        CloseTrace( s_traceHandleVsync );
        s_threadVsync->~Thread();
        tracy_free( s_threadVsync );
    }

    CloseTrace( s_traceHandle2 );
    CloseTrace( s_traceHandle );
}

void SysTraceWorker( void* ptr )
{
    ThreadExitHandler threadExitHandler;
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
    SetThreadName( "Tracy SysTrace" );
    ProcessTrace( &s_traceHandle2, 1, 0, 0 );
    ControlTrace( 0, KERNEL_LOGGER_NAME, s_prop, EVENT_TRACE_CONTROL_STOP );
    tracy_free( s_prop );
}

void SysTraceSendExternalName( uint64_t thread )
{
    bool threadSent = false;
    auto hnd = OpenThread( THREAD_QUERY_INFORMATION, FALSE, DWORD( thread ) );
    if( hnd == 0 )
    {
        hnd = OpenThread( THREAD_QUERY_LIMITED_INFORMATION, FALSE, DWORD( thread ) );
    }
    if( hnd != 0 )
    {
        if( _GetThreadDescription )
        {
            PWSTR tmp;
            _GetThreadDescription( hnd, &tmp );
            char buf[256];
            if( tmp )
            {
                auto ret = wcstombs( buf, tmp, 256 );
                if( ret != 0 )
                {
                    GetProfiler().SendString( thread, buf, ret, QueueType::ExternalThreadName );
                    threadSent = true;
                }
            }
        }
        const auto pid = GetProcessIdOfThread( hnd );
        if( !threadSent && NtQueryInformationThread && _EnumProcessModules && _GetModuleInformation && _GetModuleBaseNameA )
        {
            void* ptr;
            ULONG retlen;
            auto status = NtQueryInformationThread( hnd, (THREADINFOCLASS)9 /*ThreadQuerySetWin32StartAddress*/, &ptr, sizeof( &ptr ), &retlen );
            if( status == 0 )
            {
                const auto phnd = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid );
                if( phnd != INVALID_HANDLE_VALUE )
                {
                    HMODULE modules[1024];
                    DWORD needed;
                    if( _EnumProcessModules( phnd, modules, 1024 * sizeof( HMODULE ), &needed ) != 0 )
                    {
                        const auto sz = std::min( DWORD( needed / sizeof( HMODULE ) ), DWORD( 1024 ) );
                        for( DWORD i=0; i<sz; i++ )
                        {
                            MODULEINFO info;
                            if( _GetModuleInformation( phnd, modules[i], &info, sizeof( info ) ) != 0 )
                            {
                                if( (uint64_t)ptr >= (uint64_t)info.lpBaseOfDll && (uint64_t)ptr <= (uint64_t)info.lpBaseOfDll + (uint64_t)info.SizeOfImage )
                                {
                                    char buf2[1024];
                                    const auto modlen = _GetModuleBaseNameA( phnd, modules[i], buf2, 1024 );
                                    if( modlen != 0 )
                                    {
                                        GetProfiler().SendString( thread, buf2, modlen, QueueType::ExternalThreadName );
                                        threadSent = true;
                                    }
                                }
                            }
                        }
                    }
                    CloseHandle( phnd );
                }
            }
        }
        CloseHandle( hnd );
        if( !threadSent )
        {
            GetProfiler().SendString( thread, "???", 3, QueueType::ExternalThreadName );
            threadSent = true;
        }
        if( pid != 0 )
        {
            {
                uint64_t _pid = pid;
                TracyLfqPrepare( QueueType::TidToPid );
                MemWrite( &item->tidToPid.tid, thread );
                MemWrite( &item->tidToPid.pid, _pid );
                TracyLfqCommit;
            }
            if( pid == 4 )
            {
                GetProfiler().SendString( thread, "System", 6, QueueType::ExternalName );
                return;
            }
            else
            {
                const auto phnd = OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid );
                if( phnd != INVALID_HANDLE_VALUE )
                {
                    char buf2[1024];
                    const auto sz = GetProcessImageFileNameA( phnd, buf2, 1024 );
                    CloseHandle( phnd );
                    if( sz != 0 )
                    {
                        auto ptr = buf2 + sz - 1;
                        while( ptr > buf2 && *ptr != '\\' ) ptr--;
                        if( *ptr == '\\' ) ptr++;
                        GetProfiler().SendString( thread, ptr, QueueType::ExternalName );
                        return;
                    }
                }
            }
        }
    }

    if( !threadSent )
    {
        GetProfiler().SendString( thread, "???", 3, QueueType::ExternalThreadName );
    }
    GetProfiler().SendString( thread, "???", 3, QueueType::ExternalName );
}

}

#  elif defined __linux__

#    include <sys/types.h>
#    include <sys/stat.h>
#    include <sys/wait.h>
#    include <fcntl.h>
#    include <inttypes.h>
#    include <limits>
#    include <poll.h>
#    include <stdio.h>
#    include <stdlib.h>
#    include <string.h>
#    include <unistd.h>
#    include <atomic>
#    include <thread>
#    include <linux/perf_event.h>
#    include <linux/version.h>
#    include <sys/mman.h>
#    include <sys/ioctl.h>
#    include <sys/syscall.h>


/*** Start of inlined file: TracyRingBuffer.hpp ***/
namespace tracy
{

template<size_t Size>
class RingBuffer
{
public:
    RingBuffer( int fd, int id )
        : m_id( id )
        , m_fd( fd )
    {
        const auto pageSize = uint32_t( getpagesize() );
        assert( Size >= pageSize );
        assert( __builtin_popcount( Size ) == 1 );
        m_mapSize = Size + pageSize;
        auto mapAddr = mmap( nullptr, m_mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
        if( !mapAddr )
        {
            m_fd = 0;
            m_metadata = nullptr;
            close( fd );
            return;
        }
        m_metadata = (perf_event_mmap_page*)mapAddr;
        assert( m_metadata->data_offset == pageSize );
        m_buffer = ((char*)mapAddr) + pageSize;
        m_tail = m_metadata->data_tail;
    }

    ~RingBuffer()
    {
        if( m_metadata ) munmap( m_metadata, m_mapSize );
        if( m_fd ) close( m_fd );
    }

    RingBuffer( const RingBuffer& ) = delete;
    RingBuffer& operator=( const RingBuffer& ) = delete;

    RingBuffer( RingBuffer&& other )
    {
        memcpy( (char*)&other, (char*)this, sizeof( RingBuffer ) );
        m_metadata = nullptr;
        m_fd = 0;
    }

    RingBuffer& operator=( RingBuffer&& other )
    {
        memcpy( (char*)&other, (char*)this, sizeof( RingBuffer ) );
        m_metadata = nullptr;
        m_fd = 0;
        return *this;
    }

    bool IsValid() const { return m_metadata != nullptr; }
    int GetId() const { return m_id; }

    void Enable()
    {
        ioctl( m_fd, PERF_EVENT_IOC_ENABLE, 0 );
    }

    void Read( void* dst, uint64_t offset, uint64_t cnt )
    {
        auto src = ( m_tail + offset ) % Size;
        if( src + cnt <= Size )
        {
            memcpy( dst, m_buffer + src, cnt );
        }
        else
        {
            const auto s0 = Size - src;
            memcpy( dst, m_buffer + src, s0 );
            memcpy( (char*)dst + s0, m_buffer, cnt - s0 );
        }
    }

    void Advance( uint64_t cnt )
    {
        m_tail += cnt;
        StoreTail();
    }

    bool CheckTscCaps() const
    {
        return m_metadata->cap_user_time_zero;
    }

    int64_t ConvertTimeToTsc( int64_t timestamp ) const
    {
        if( !m_metadata->cap_user_time_zero ) return 0;
        const auto time = timestamp - m_metadata->time_zero;
        const auto quot = time / m_metadata->time_mult;
        const auto rem = time % m_metadata->time_mult;
        return ( quot << m_metadata->time_shift ) + ( rem << m_metadata->time_shift ) / m_metadata->time_mult;
    }

    uint64_t LoadHead() const
    {
        return std::atomic_load_explicit( (const volatile std::atomic<uint64_t>*)&m_metadata->data_head, std::memory_order_acquire );
    }

    uint64_t GetTail() const
    {
        return m_tail;
    }

private:
    void StoreTail()
    {
        std::atomic_store_explicit( (volatile std::atomic<uint64_t>*)&m_metadata->data_tail, m_tail, std::memory_order_release );
    }

    uint64_t m_tail;
    char* m_buffer;
    int m_id;
    perf_event_mmap_page* m_metadata;

    size_t m_mapSize;
    int m_fd;
};

}

/*** End of inlined file: TracyRingBuffer.hpp ***/

#    ifdef __ANDROID__

/*** Start of inlined file: TracySysTracePayload.hpp ***/
// File: 'extra/systrace/tracy_systrace.armv7' (1149 bytes)
// File: 'extra/systrace/tracy_systrace.aarch64' (1650 bytes)

// Exported using binary_to_compressed_c.cpp

namespace tracy
{

static const unsigned int tracy_systrace_armv7_size = 1149;
static const unsigned int tracy_systrace_armv7_data[1152/4] =
{
    0x464c457f, 0x00010101, 0x00000000, 0x00000000, 0x00280003, 0x00000001, 0x000001f0, 0x00000034, 0x00000000, 0x05000200, 0x00200034, 0x00280007,
    0x00000000, 0x00000006, 0x00000034, 0x00000034, 0x00000034, 0x000000e0, 0x000000e0, 0x00000004, 0x00000004, 0x00000003, 0x00000114, 0x00000114,
    0x00000114, 0x00000013, 0x00000013, 0x00000004, 0x00000001, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x000003fd, 0x000003fd, 0x00000005,
    0x00001000, 0x00000001, 0x000003fd, 0x000013fd, 0x000013fd, 0x00000080, 0x000000b3, 0x00000006, 0x00001000, 0x00000002, 0x00000400, 0x00001400,
    0x00001400, 0x0000007d, 0x000000b0, 0x00000006, 0x00000004, 0x6474e551, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000006,
    0x00000004, 0x70000001, 0x000003a4, 0x000003a4, 0x000003a4, 0x00000008, 0x00000008, 0x00000004, 0x00000004, 0x7379732f, 0x2f6d6574, 0x2f6e6962,
    0x6b6e696c, 0x00007265, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x00000000, 0x00000012, 0x00000016, 0x00000000,
    0x00000000, 0x00000012, 0x6f6c6400, 0x006e6570, 0x4342494c, 0x62696c00, 0x732e6c64, 0x6c64006f, 0x006d7973, 0x00000001, 0x00000003, 0x00000001,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00010001, 0x0000000d, 0x00000010, 0x00000000, 0x00050d63, 0x00020000, 0x00000008,
    0x00000000, 0x000014bc, 0x00000116, 0x000014c0, 0x00000216, 0xe52de004, 0xe59fe004, 0xe08fe00e, 0xe5bef008, 0x000012dc, 0xe28fc600, 0xe28cca01,
    0xe5bcf2dc, 0xe28fc600, 0xe28cca01, 0xe5bcf2d4, 0xe92d4ff0, 0xe28db01c, 0xe24dd024, 0xe24dd801, 0xe59f017c, 0xe3a01001, 0xe3a08001, 0xe08f0000,
    0xebfffff0, 0xe59f116c, 0xe1a04000, 0xe08f1001, 0xebffffef, 0xe59f1160, 0xe1a06000, 0xe1a00004, 0xe08f1001, 0xebffffea, 0xe59f1150, 0xe1a07000,
    0xe1a00004, 0xe08f1001, 0xebffffe5, 0xe59f1140, 0xe1a05000, 0xe1a00004, 0xe08f1001, 0xebffffe0, 0xe58d0004, 0xe1a00004, 0xe59f1128, 0xe08f1001,
    0xebffffdb, 0xe59f1120, 0xe1a0a000, 0xe1a00004, 0xe08f1001, 0xebffffd6, 0xe1a04000, 0xe59f010c, 0xe3a01000, 0xe3a09000, 0xe08f0000, 0xe12fff36,
    0xe1a06000, 0xe3700001, 0xca000001, 0xe3a00000, 0xe12fff37, 0xe3a00009, 0xe3a01001, 0xe1cd01bc, 0xe3a00008, 0xe1cd01b4, 0xe3090680, 0xe3400098,
    0xe3a02000, 0xe58d000c, 0xe28d0010, 0xe58d7000, 0xe58d6018, 0xe58d8010, 0xe58d9008, 0xe12fff35, 0xe3500000, 0xca00001d, 0xe28d7018, 0xe28d8010,
    0xe28d9020, 0xe1a00007, 0xe3a01001, 0xe3a02000, 0xe12fff35, 0xe3500000, 0xda00000a, 0xe1a00006, 0xe1a01009, 0xe3a02801, 0xe12fff3a, 0xe3500001,
    0xba00000e, 0xe1a02000, 0xe3a00001, 0xe1a01009, 0xe12fff34, 0xea000003, 0xe59d2004, 0xe28d0008, 0xe3a01000, 0xe12fff32, 0xe1a00008, 0xe3a01001,
    0xe3a02000, 0xe12fff35, 0xe3500001, 0xbaffffe4, 0xe59d1000, 0xe3a00000, 0xe12fff31, 0xe24bd01c, 0xe8bd8ff0, 0x00000198, 0x00000190, 0x00000181,
    0x00000172, 0x00000163, 0x00000159, 0x0000014a, 0x00000138, 0x7ffffe4c, 0x00000001, 0x6362696c, 0x006f732e, 0x6e65706f, 0x69786500, 0x6f700074,
    0x6e006c6c, 0x736f6e61, 0x7065656c, 0x61657200, 0x72770064, 0x00657469, 0x7379732f, 0x72656b2f, 0x2f6c656e, 0x75626564, 0x72742f67, 0x6e696361,
    0x72742f67, 0x5f656361, 0x65706970, 0x00000000, 0x00000003, 0x000014b0, 0x00000002, 0x00000010, 0x00000017, 0x000001b4, 0x00000014, 0x00000011,
    0x00000015, 0x00000000, 0x00000006, 0x00000128, 0x0000000b, 0x00000010, 0x00000005, 0x00000158, 0x0000000a, 0x0000001c, 0x6ffffef5, 0x00000174,
    0x00000001, 0x0000000d, 0x0000001e, 0x00000008, 0x6ffffffb, 0x00000001, 0x6ffffff0, 0x0000018c, 0x6ffffffe, 0x00000194, 0x6fffffff, 0x00000001,
};

static const unsigned int tracy_systrace_aarch64_size = 1650;
static const unsigned int tracy_systrace_aarch64_data[1652/4] =
{
    0x464c457f, 0x00010102, 0x00000000, 0x00000000, 0x00b70003, 0x00000001, 0x000002e0, 0x00000000, 0x00000040, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00380040, 0x00400006, 0x00000000, 0x00000006, 0x00000005, 0x00000040, 0x00000000, 0x00000040, 0x00000000, 0x00000040, 0x00000000,
    0x00000150, 0x00000000, 0x00000150, 0x00000000, 0x00000008, 0x00000000, 0x00000003, 0x00000004, 0x00000190, 0x00000000, 0x00000190, 0x00000000,
    0x00000190, 0x00000000, 0x00000015, 0x00000000, 0x00000015, 0x00000000, 0x00000001, 0x00000000, 0x00000001, 0x00000005, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000004e1, 0x00000000, 0x000004e1, 0x00000000, 0x00001000, 0x00000000, 0x00000001, 0x00000006,
    0x000004e8, 0x00000000, 0x000014e8, 0x00000000, 0x000014e8, 0x00000000, 0x0000018a, 0x00000000, 0x00000190, 0x00000000, 0x00001000, 0x00000000,
    0x00000002, 0x00000006, 0x000004e8, 0x00000000, 0x000014e8, 0x00000000, 0x000014e8, 0x00000000, 0x00000160, 0x00000000, 0x00000160, 0x00000000,
    0x00000008, 0x00000000, 0x6474e551, 0x00000006, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000008, 0x00000000, 0x7379732f, 0x2f6d6574, 0x2f6e6962, 0x6b6e696c, 0x34367265, 0x00000000, 0x00000001, 0x00000001,
    0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00090003, 0x000002e0, 0x00000000, 0x00000000, 0x00000000, 0x00000010, 0x00000012, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0000000a, 0x00000012, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x62696c00, 0x732e6c64, 0x6c64006f, 0x006d7973, 0x706f6c64, 0x4c006e65,
    0x00434249, 0x00000000, 0x00000000, 0x00000000, 0x00010001, 0x00000001, 0x00000010, 0x00000000, 0x00050d63, 0x00020000, 0x00000017, 0x00000000,
    0x00001668, 0x00000000, 0x00000402, 0x00000002, 0x00000000, 0x00000000, 0x00001670, 0x00000000, 0x00000402, 0x00000003, 0x00000000, 0x00000000,
    0xa9bf7bf0, 0xb0000010, 0xf9433211, 0x91198210, 0xd61f0220, 0xd503201f, 0xd503201f, 0xd503201f, 0xb0000010, 0xf9433611, 0x9119a210, 0xd61f0220,
    0xb0000010, 0xf9433a11, 0x9119c210, 0xd61f0220, 0xa9bb67fc, 0xa9015ff8, 0xa90257f6, 0xa9034ff4, 0xa9047bfd, 0x910103fd, 0xd14043ff, 0xd10083ff,
    0x90000000, 0x91124000, 0x52800021, 0x52800039, 0x97ffffec, 0x90000001, 0x91126021, 0xaa0003f7, 0x97ffffec, 0x90000001, 0xaa0003f8, 0x91127421,
    0xaa1703e0, 0x97ffffe7, 0x90000001, 0xaa0003f3, 0x91128821, 0xaa1703e0, 0x97ffffe2, 0x90000001, 0xaa0003f4, 0x91129c21, 0xaa1703e0, 0x97ffffdd,
    0x90000001, 0xaa0003f5, 0x9112c421, 0xaa1703e0, 0x97ffffd8, 0x90000001, 0xaa0003f6, 0x9112d821, 0xaa1703e0, 0x97ffffd3, 0xaa0003f7, 0x90000000,
    0x9112f000, 0x2a1f03e1, 0xd63f0300, 0x2a0003f8, 0x36f80060, 0x2a1f03e0, 0xd63f0260, 0x90000009, 0x3dc12120, 0x52800128, 0x79003be8, 0x52800108,
    0x910043e0, 0x52800021, 0x2a1f03e2, 0xb9001bf8, 0xb90013f9, 0x79002be8, 0x3d8003e0, 0xd63f0280, 0x7100001f, 0x5400036c, 0x910063e0, 0x52800021,
    0x2a1f03e2, 0xd63f0280, 0x7100001f, 0x5400018d, 0x910083e1, 0x52a00022, 0x2a1803e0, 0xd63f02c0, 0xf100041f, 0x540001eb, 0xaa0003e2, 0x910083e1,
    0x52800020, 0xd63f02e0, 0x14000004, 0x910003e0, 0xaa1f03e1, 0xd63f02a0, 0x910043e0, 0x52800021, 0x2a1f03e2, 0xd63f0280, 0x7100041f, 0x54fffceb,
    0x2a1f03e0, 0xd63f0260, 0x914043ff, 0x910083ff, 0xa9447bfd, 0xa9434ff4, 0xa94257f6, 0xa9415ff8, 0xa8c567fc, 0xd65f03c0, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00989680, 0x00000000, 0x6362696c, 0x006f732e, 0x6e65706f, 0x69786500, 0x6f700074, 0x6e006c6c, 0x736f6e61, 0x7065656c,
    0x61657200, 0x72770064, 0x00657469, 0x7379732f, 0x72656b2f, 0x2f6c656e, 0x75626564, 0x72742f67, 0x6e696361, 0x72742f67, 0x5f656361, 0x65706970,
    0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x00000001, 0x00000000, 0x6ffffef5, 0x00000000, 0x000001a8, 0x00000000, 0x00000005, 0x00000000,
    0x00000228, 0x00000000, 0x00000006, 0x00000000, 0x000001c8, 0x00000000, 0x0000000a, 0x00000000, 0x0000001c, 0x00000000, 0x0000000b, 0x00000000,
    0x00000018, 0x00000000, 0x00000015, 0x00000000, 0x00000000, 0x00000000, 0x00000003, 0x00000000, 0x00001650, 0x00000000, 0x00000002, 0x00000000,
    0x00000030, 0x00000000, 0x00000014, 0x00000000, 0x00000007, 0x00000000, 0x00000017, 0x00000000, 0x00000270, 0x00000000, 0x0000001e, 0x00000000,
    0x00000008, 0x00000000, 0x6ffffffb, 0x00000000, 0x00000001, 0x00000000, 0x6ffffffe, 0x00000000, 0x00000250, 0x00000000, 0x6fffffff, 0x00000000,
    0x00000001, 0x00000000, 0x6ffffff0, 0x00000000, 0x00000244, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x000002a0, 0x00000000, 0x000002a0,
};

}

/*** End of inlined file: TracySysTracePayload.hpp ***/


#    endif

#    ifdef __AVX2__
#      include <immintrin.h>
#    endif

namespace tracy
{

static const char BasePath[] = "/sys/kernel/debug/tracing/";
static const char TracingOn[] = "tracing_on";
static const char CurrentTracer[] = "current_tracer";
static const char TraceOptions[] = "trace_options";
static const char TraceClock[] = "trace_clock";
static const char SchedSwitch[] = "events/sched/sched_switch/enable";
static const char SchedWakeup[] = "events/sched/sched_wakeup/enable";
static const char BufferSizeKb[] = "buffer_size_kb";
static const char TracePipe[] = "trace_pipe";

static std::atomic<bool> traceActive { false };
static Thread* s_threadSampling = nullptr;
static int s_numCpus = 0;
static int s_numBuffers = 0;

static constexpr size_t RingBufSize = 64*1024;
static RingBuffer<RingBufSize>* s_ring = nullptr;

static int perf_event_open( struct perf_event_attr* hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags )
{
    return syscall( __NR_perf_event_open, hw_event, pid, cpu, group_fd, flags );
}

enum TraceEventId
{
    EventCallstack,
    EventCpuCycles,
    EventInstructionsRetired,
    EventCacheReference,
    EventCacheMiss,
    EventBranchRetired,
    EventBranchMiss
};

static void ProbePreciseIp( perf_event_attr& pe, unsigned long long config0, unsigned long long config1, pid_t pid )
{
    pe.config = config1;
    pe.precise_ip = 3;
    while( pe.precise_ip != 0 )
    {
        const int fd = perf_event_open( &pe, pid, 0, -1, PERF_FLAG_FD_CLOEXEC );
        if( fd != -1 )
        {
            close( fd );
            break;
        }
        pe.precise_ip--;
    }
    pe.config = config0;
    while( pe.precise_ip != 0 )
    {
        const int fd = perf_event_open( &pe, pid, 0, -1, PERF_FLAG_FD_CLOEXEC );
        if( fd != -1 )
        {
            close( fd );
            break;
        }
        pe.precise_ip--;
    }
    TracyDebug( "  Probed precise_ip: %i\n", pe.precise_ip );
}

static void ProbePreciseIp( perf_event_attr& pe, pid_t pid )
{
    pe.precise_ip = 3;
    while( pe.precise_ip != 0 )
    {
        const int fd = perf_event_open( &pe, pid, 0, -1, PERF_FLAG_FD_CLOEXEC );
        if( fd != -1 )
        {
            close( fd );
            break;
        }
        pe.precise_ip--;
    }
    TracyDebug( "  Probed precise_ip: %i\n", pe.precise_ip );
}

static bool IsGenuineIntel()
{
#if defined __i386 || defined __x86_64__
    uint32_t regs[4];
    __get_cpuid( 0, regs, regs+1, regs+2, regs+3 );
    char manufacturer[12];
    memcpy( manufacturer, regs+1, 4 );
    memcpy( manufacturer+4, regs+3, 4 );
    memcpy( manufacturer+8, regs+2, 4 );
    return memcmp( manufacturer, "GenuineIntel", 12 ) == 0;
#else
    return false;
#endif
}

static void SetupSampling( int64_t& samplingPeriod )
{
#ifndef CLOCK_MONOTONIC_RAW
    return;
#endif

#ifdef TRACY_NO_SAMPLE_RETIREMENT
    const bool noRetirement = true;
#else
    const char* noRetirementEnv = GetEnvVar( "TRACY_NO_SAMPLE_RETIREMENT" );
    const bool noRetirement = noRetirementEnv && noRetirementEnv[0] == '1';
#endif

#ifdef TRACY_NO_SAMPLE_CACHE
    const bool noCache = true;
#else
    const char* noCacheEnv = GetEnvVar( "TRACY_NO_SAMPLE_CACHE" );
    const bool noCache = noCacheEnv && noCacheEnv[0] == '1';
#endif

#ifdef TRACY_NO_SAMPLE_BRANCH
    const bool noBranch = true;
#else
    const char* noBranchEnv = GetEnvVar( "TRACY_NO_SAMPLE_BRANCH" );
    const bool noBranch = noBranchEnv && noBranchEnv[0] == '1';
#endif

    samplingPeriod = GetSamplingPeriod();
    uint32_t currentPid = (uint32_t)getpid();

    s_numCpus = (int)std::thread::hardware_concurrency();
    s_ring = (RingBuffer<RingBufSize>*)tracy_malloc( sizeof( RingBuffer<RingBufSize> ) * s_numCpus * 7 );
    s_numBuffers = 0;

    // Stack traces
    perf_event_attr pe = {};
    pe.type = PERF_TYPE_SOFTWARE;
    pe.size = sizeof( perf_event_attr );
    pe.config = PERF_COUNT_SW_CPU_CLOCK;
    pe.sample_freq = GetSamplingFrequency();
    pe.sample_type = PERF_SAMPLE_TID | PERF_SAMPLE_TIME | PERF_SAMPLE_CALLCHAIN;
#if LINUX_VERSION_CODE >= KERNEL_VERSION( 4, 8, 0 )
    pe.sample_max_stack = 127;
#endif
    pe.disabled = 1;
    pe.freq = 1;
    pe.inherit = 1;
#if !defined TRACY_HW_TIMER || !( defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64 )
    pe.use_clockid = 1;
    pe.clockid = CLOCK_MONOTONIC_RAW;
#endif

    TracyDebug( "Setup software sampling\n" );
    ProbePreciseIp( pe, currentPid );
    for( int i=0; i<s_numCpus; i++ )
    {
        const int fd = perf_event_open( &pe, currentPid, i, -1, PERF_FLAG_FD_CLOEXEC );
        if( fd == -1 )
        {
            for( int j=0; j<s_numBuffers; j++ ) s_ring[j].~RingBuffer<RingBufSize>();
            tracy_free( s_ring );
            return;
        }
        new( s_ring+s_numBuffers ) RingBuffer<RingBufSize>( fd, EventCallstack );
        s_numBuffers++;
    }

    // CPU cycles + instructions retired
    pe = {};
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof( perf_event_attr );
    pe.sample_freq = 5000;
    pe.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TIME;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_guest = 1;
    pe.exclude_hv = 1;
    pe.freq = 1;
    pe.inherit = 1;
#if !defined TRACY_HW_TIMER || !( defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64 )
    pe.use_clockid = 1;
    pe.clockid = CLOCK_MONOTONIC_RAW;
#endif

    if( !noRetirement )
    {
        TracyDebug( "Setup sampling cycles + retirement\n" );
        ProbePreciseIp( pe, PERF_COUNT_HW_CPU_CYCLES, PERF_COUNT_HW_INSTRUCTIONS, currentPid );
        for( int i=0; i<s_numCpus; i++ )
        {
            const int fd = perf_event_open( &pe, currentPid, i, -1, PERF_FLAG_FD_CLOEXEC );
            if( fd != -1 )
            {
                new( s_ring+s_numBuffers ) RingBuffer<RingBufSize>( fd, EventCpuCycles );
                s_numBuffers++;
                TracyDebug( "  Core %i ok\n", i );
            }
        }

        pe.config = PERF_COUNT_HW_INSTRUCTIONS;
        for( int i=0; i<s_numCpus; i++ )
        {
            const int fd = perf_event_open( &pe, currentPid, i, -1, PERF_FLAG_FD_CLOEXEC );
            if( fd != -1 )
            {
                new( s_ring+s_numBuffers ) RingBuffer<RingBufSize>( fd, EventInstructionsRetired );
                s_numBuffers++;
                TracyDebug( "  Core %i ok\n", i );
            }
        }
    }

    // cache reference + miss
    if( !noCache )
    {
        TracyDebug( "Setup sampling CPU cache references + misses\n" );
        ProbePreciseIp( pe, PERF_COUNT_HW_CACHE_REFERENCES, PERF_COUNT_HW_CACHE_MISSES, currentPid );
        if( IsGenuineIntel() )
        {
            pe.precise_ip = 0;
            TracyDebug( "  CPU is GenuineIntel, forcing precise_ip down to 0\n" );
        }
        for( int i=0; i<s_numCpus; i++ )
        {
            const int fd = perf_event_open( &pe, currentPid, i, -1, PERF_FLAG_FD_CLOEXEC );
            if( fd != -1 )
            {
                new( s_ring+s_numBuffers ) RingBuffer<RingBufSize>( fd, EventCacheReference );
                s_numBuffers++;
                TracyDebug( "  Core %i ok\n", i );
            }
        }

        pe.config = PERF_COUNT_HW_CACHE_MISSES;
        for( int i=0; i<s_numCpus; i++ )
        {
            const int fd = perf_event_open( &pe, currentPid, i, -1, PERF_FLAG_FD_CLOEXEC );
            if( fd != -1 )
            {
                new( s_ring+s_numBuffers ) RingBuffer<RingBufSize>( fd, EventCacheMiss );
                s_numBuffers++;
                TracyDebug( "  Core %i ok\n", i );
            }
        }
    }

    // branch retired + miss
    if( !noBranch )
    {
        TracyDebug( "Setup sampling CPU branch retirements + misses\n" );
        ProbePreciseIp( pe, PERF_COUNT_HW_BRANCH_INSTRUCTIONS, PERF_COUNT_HW_BRANCH_MISSES, currentPid );
        for( int i=0; i<s_numCpus; i++ )
        {
            const int fd = perf_event_open( &pe, currentPid, i, -1, PERF_FLAG_FD_CLOEXEC );
            if( fd != -1 )
            {
                new( s_ring+s_numBuffers ) RingBuffer<RingBufSize>( fd, EventBranchRetired );
                s_numBuffers++;
                TracyDebug( "  Core %i ok\n", i );
            }
        }

        pe.config = PERF_COUNT_HW_BRANCH_MISSES;
        for( int i=0; i<s_numCpus; i++ )
        {
            const int fd = perf_event_open( &pe, currentPid, i, -1, PERF_FLAG_FD_CLOEXEC );
            if( fd != -1 )
            {
                new( s_ring+s_numBuffers ) RingBuffer<RingBufSize>( fd, EventBranchMiss );
                s_numBuffers++;
                TracyDebug( "  Core %i ok\n", i );
            }
        }
    }

    s_threadSampling = (Thread*)tracy_malloc( sizeof( Thread ) );
    new(s_threadSampling) Thread( [] (void*) {
        ThreadExitHandler threadExitHandler;
        SetThreadName( "Tracy Sampling" );
        InitRpmalloc();
        sched_param sp = { 5 };
        pthread_setschedparam( pthread_self(), SCHED_FIFO, &sp );
#if defined TRACY_HW_TIMER && ( defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64 )
        for( int i=0; i<s_numBuffers; i++ )
        {
            if( s_ring[i].GetId() == EventCallstack && !s_ring[i].CheckTscCaps() )
            {
                for( int j=0; j<s_numBuffers; j++ ) s_ring[j].~RingBuffer<RingBufSize>();
                tracy_free_fast( s_ring );
                const char* err = "Tracy Profiler: sampling is disabled due to non-native scheduler clock. Are you running under a VM?";
                Profiler::MessageAppInfo( err, strlen( err ) );
                return;
            }
        }
#endif
        for( int i=0; i<s_numBuffers; i++ ) s_ring[i].Enable();
        for(;;)
        {
            bool hadData = false;
            for( int i=0; i<s_numBuffers; i++ )
            {
                if( !traceActive.load( std::memory_order_relaxed ) ) break;
                auto& ring = s_ring[i];
                const auto head = ring.LoadHead();
                const auto tail = ring.GetTail();
                if( head == tail ) continue;
                assert( head > tail );
                hadData = true;

                const auto end = head - tail;
                uint64_t pos = 0;
                while( pos < end )
                {
                    perf_event_header hdr;
                    ring.Read( &hdr, pos, sizeof( perf_event_header ) );
                    if( hdr.type == PERF_RECORD_SAMPLE )
                    {
                        auto offset = pos + sizeof( perf_event_header );
                        const auto id = ring.GetId();
                        if( id == EventCallstack )
                        {
                            // Layout:
                            //   u32 pid, tid
                            //   u64 time
                            //   u64 cnt
                            //   u64 ip[cnt]

                            uint32_t tid;
                            uint64_t t0;
                            uint64_t cnt;

                            offset += sizeof( uint32_t );
                            ring.Read( &tid, offset, sizeof( uint32_t ) );
                            offset += sizeof( uint32_t );
                            ring.Read( &t0, offset, sizeof( uint64_t ) );
                            offset += sizeof( uint64_t );
                            ring.Read( &cnt, offset, sizeof( uint64_t ) );
                            offset += sizeof( uint64_t );

                            if( cnt > 0 )
                            {
#if defined TRACY_HW_TIMER && ( defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64 )
                                t0 = ring.ConvertTimeToTsc( t0 );
                                if( t0 != 0 )
#endif
                                {
                                    auto trace = (uint64_t*)tracy_malloc_fast( ( 1 + cnt ) * sizeof( uint64_t ) );
                                    ring.Read( trace+1, offset, sizeof( uint64_t ) * cnt );

#if defined __x86_64__ || defined _M_X64
                                    // remove non-canonical pointers
                                    do
                                    {
                                        const auto test = (int64_t)trace[cnt];
                                        const auto m1 = test >> 63;
                                        const auto m2 = test >> 47;
                                        if( m1 == m2 ) break;
                                    }
                                    while( --cnt > 0 );
                                    for( uint64_t j=1; j<cnt; j++ )
                                    {
                                        const auto test = (int64_t)trace[j];
                                        const auto m1 = test >> 63;
                                        const auto m2 = test >> 47;
                                        if( m1 != m2 ) trace[j] = 0;
                                    }
#endif

                                    for( uint64_t j=1; j<=cnt; j++ )
                                    {
                                        if( trace[j] >= (uint64_t)-4095 )       // PERF_CONTEXT_MAX
                                        {
                                            memmove( trace+j, trace+j+1, sizeof( uint64_t ) * ( cnt - j ) );
                                            cnt--;
                                        }
                                    }

                                    memcpy( trace, &cnt, sizeof( uint64_t ) );

                                    TracyLfqPrepare( QueueType::CallstackSample );
                                    MemWrite( &item->callstackSampleFat.time, t0 );
                                    MemWrite( &item->callstackSampleFat.thread, (uint64_t)tid );
                                    MemWrite( &item->callstackSampleFat.ptr, (uint64_t)trace );
                                    TracyLfqCommit;
                                }
                            }
                        }
                        else
                        {
                            // Layout:
                            //   u64 ip
                            //   u64 time

                            uint64_t ip, t0;
                            ring.Read( &ip, offset, sizeof( uint64_t ) );
                            offset += sizeof( uint64_t );
                            ring.Read( &t0, offset, sizeof( uint64_t ) );

#if defined TRACY_HW_TIMER && ( defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64 )
                            t0 = ring.ConvertTimeToTsc( t0 );
                            if( t0 != 0 )
#endif
                            {
                                QueueType type;
                                switch( id )
                                {
                                case EventCpuCycles:
                                    type = QueueType::HwSampleCpuCycle;
                                    break;
                                case EventInstructionsRetired:
                                    type = QueueType::HwSampleInstructionRetired;
                                    break;
                                case EventCacheReference:
                                    type = QueueType::HwSampleCacheReference;
                                    break;
                                case EventCacheMiss:
                                    type = QueueType::HwSampleCacheMiss;
                                    break;
                                case EventBranchRetired:
                                    type = QueueType::HwSampleBranchRetired;
                                    break;
                                case EventBranchMiss:
                                    type = QueueType::HwSampleBranchMiss;
                                    break;
                                default:
                                    assert( false );
                                    break;
                                }

                                TracyLfqPrepare( type );
                                MemWrite( &item->hwSample.ip, ip );
                                MemWrite( &item->hwSample.time, t0 );
                                TracyLfqCommit;
                            }
                        }
                    }
                    pos += hdr.size;
                }
                assert( pos == end );
                ring.Advance( end );
            }
            if( !traceActive.load( std::memory_order_relaxed) ) break;
            if( !hadData )
            {
                std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
            }
        }

        for( int i=0; i<s_numBuffers; i++ ) s_ring[i].~RingBuffer<RingBufSize>();
        tracy_free_fast( s_ring );
    }, nullptr );
}

#ifdef __ANDROID__
static bool TraceWrite( const char* path, size_t psz, const char* val, size_t vsz )
{
    // Explanation for "su root sh -c": there are 2 flavors of "su" in circulation
    // on Android. The default Android su has the following syntax to run a command
    // as root:
    //   su root 'command'
    // and 'command' is exec'd not passed to a shell, so if shell interpretation is
    // wanted, one needs to do:
    //   su root sh -c 'command'
    // Besides that default Android 'su' command, some Android devices use a different
    // su with a command-line interface closer to the familiar util-linux su found
    // on Linux distributions. Fortunately, both the util-linux su and the one
    // in https://github.com/topjohnwu/Magisk seem to be happy with the above
    // `su root sh -c 'command'` command line syntax.
    char tmp[256];
    sprintf( tmp, "su root sh -c 'echo \"%s\" > %s%s'", val, BasePath, path );
    return system( tmp ) == 0;
}
#else
static bool TraceWrite( const char* path, size_t psz, const char* val, size_t vsz )
{
    char tmp[256];
    memcpy( tmp, BasePath, sizeof( BasePath ) - 1 );
    memcpy( tmp + sizeof( BasePath ) - 1, path, psz );

    int fd = open( tmp, O_WRONLY );
    if( fd < 0 ) return false;

    for(;;)
    {
        ssize_t cnt = write( fd, val, vsz );
        if( cnt == (ssize_t)vsz )
        {
            close( fd );
            return true;
        }
        if( cnt < 0 )
        {
            close( fd );
            return false;
        }
        vsz -= cnt;
        val += cnt;
    }
}
#endif

#ifdef __ANDROID__
void SysTraceInjectPayload()
{
    int pipefd[2];
    if( pipe( pipefd ) == 0 )
    {
        const auto pid = fork();
        if( pid == 0 )
        {
            // child
            close( pipefd[1] );
            if( dup2( pipefd[0], STDIN_FILENO ) >= 0 )
            {
                close( pipefd[0] );
                execlp( "su", "su", "root", "sh", "-c", "cat > /data/tracy_systrace", (char*)nullptr );
                exit( 1 );
            }
        }
        else if( pid > 0 )
        {
            // parent
            close( pipefd[0] );

#ifdef __aarch64__
            write( pipefd[1], tracy_systrace_aarch64_data, tracy_systrace_aarch64_size );
#else
            write( pipefd[1], tracy_systrace_armv7_data, tracy_systrace_armv7_size );
#endif
            close( pipefd[1] );
            waitpid( pid, nullptr, 0 );

            system( "su root sh -c 'chmod 700 /data/tracy_systrace'" );
        }
    }
}
#endif

bool SysTraceStart( int64_t& samplingPeriod )
{
#ifndef CLOCK_MONOTONIC_RAW
    return false;
#endif

    if( !TraceWrite( TracingOn, sizeof( TracingOn ), "0", 2 ) ) return false;
    if( !TraceWrite( CurrentTracer, sizeof( CurrentTracer ), "nop", 4 ) ) return false;
    TraceWrite( TraceOptions, sizeof( TraceOptions ), "norecord-cmd", 13 );
    TraceWrite( TraceOptions, sizeof( TraceOptions ), "norecord-tgid", 14 );
    TraceWrite( TraceOptions, sizeof( TraceOptions ), "noirq-info", 11 );
    TraceWrite( TraceOptions, sizeof( TraceOptions ), "noannotate", 11 );
#if defined TRACY_HW_TIMER && ( defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64 )
    if( !TraceWrite( TraceClock, sizeof( TraceClock ), "x86-tsc", 8 ) ) return false;
#else
    if( !TraceWrite( TraceClock, sizeof( TraceClock ), "mono_raw", 9 ) ) return false;
#endif
    if( !TraceWrite( SchedSwitch, sizeof( SchedSwitch ), "1", 2 ) ) return false;
    if( !TraceWrite( SchedWakeup, sizeof( SchedWakeup ), "1", 2 ) ) return false;
    if( !TraceWrite( BufferSizeKb, sizeof( BufferSizeKb ), "4096", 5 ) ) return false;

#if defined __ANDROID__ && ( defined __aarch64__ || defined __ARM_ARCH )
    SysTraceInjectPayload();
#endif

    if( !TraceWrite( TracingOn, sizeof( TracingOn ), "1", 2 ) ) return false;
    traceActive.store( true, std::memory_order_relaxed );

    SetupSampling( samplingPeriod );

    return true;
}

void SysTraceStop()
{
    TraceWrite( TracingOn, sizeof( TracingOn ), "0", 2 );
    traceActive.store( false, std::memory_order_relaxed );
    if( s_threadSampling )
    {
        s_threadSampling->~Thread();
        tracy_free( s_threadSampling );
    }
}

static uint64_t ReadNumber( const char*& data )
{
    auto ptr = data;
    assert( *ptr >= '0' && *ptr <= '9' );
    uint64_t val = 0;
    for(;;)
    {
        uint64_t q;
        memcpy( &q, ptr, 8 );
        for( int i=0; i<8; i++ )
        {
            const uint64_t v = ( q & 0xFF ) - '0';
            if( v > 9 )
            {
                data = ptr + i;
                return val;
            }
            val = val * 10 + v;
            q >>= 8;
        }
        ptr += 8;
    }
}

static uint8_t ReadState( char state )
{
    switch( state )
    {
    case 'D': return 101;
    case 'I': return 102;
    case 'R': return 103;
    case 'S': return 104;
    case 'T': return 105;
    case 't': return 106;
    case 'W': return 107;
    case 'X': return 108;
    case 'Z': return 109;
    default: return 100;
    }
}

#if defined __ANDROID__ && defined __ANDROID_API__ && __ANDROID_API__ < 18
/*-
 * Copyright (c) 2011 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Christos Zoulas.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp)
{
	char *ptr, *eptr;

	if (*buf == NULL || *bufsiz == 0) {
		*bufsiz = BUFSIZ;
		if ((*buf = (char*)malloc(*bufsiz)) == NULL)
			return -1;
	}

	for (ptr = *buf, eptr = *buf + *bufsiz;;) {
		int c = fgetc(fp);
		if (c == -1) {
			if (feof(fp))
				return ptr == *buf ? -1 : ptr - *buf;
			else
				return -1;
		}
		*ptr++ = c;
		if (c == delimiter) {
			*ptr = '\0';
			return ptr - *buf;
		}
		if (ptr + 2 >= eptr) {
			char *nbuf;
			size_t nbufsiz = *bufsiz * 2;
			ssize_t d = ptr - *buf;
			if ((nbuf = (char*)realloc(*buf, nbufsiz)) == NULL)
				return -1;
			*buf = nbuf;
			*bufsiz = nbufsiz;
			eptr = nbuf + nbufsiz;
			ptr = nbuf + d;
		}
	}
}

ssize_t getline(char **buf, size_t *bufsiz, FILE *fp)
{
	return getdelim(buf, bufsiz, '\n', fp);
}
#endif

#ifdef __AVX2__
static inline void AdvanceTo( const char*& line, char match )
{
    __m256i m = _mm256_set1_epi8( match );
    auto ptr = line;
    for(;;)
    {
        __m256i l = _mm256_loadu_si256( (const __m256i*)ptr );
        __m256i c = _mm256_cmpeq_epi8( l, m );
        auto b = uint32_t( _mm256_movemask_epi8( c ) );
        if( b != 0 )
        {
            line = ptr + __builtin_ctz( b );
            return;
        }
        ptr += 32;
    }
}
#else
static inline void AdvanceTo( const char*& line, char match )
{
    auto ptr = line;
    for(;;)
    {
        uint64_t l;
        memcpy( &l, ptr, 8 );
        for( int i=0; i<8; i++ )
        {
            if( ( l & 0xFF ) == uint8_t( match ) )
            {
                line = ptr + i;
                return;
            }
            l >>= 8;
        }
        ptr += 8;
    }
}
#endif

#ifdef __AVX2__
static inline void AdvanceToNot( const char*& line, char match )
{
    __m256i m = _mm256_set1_epi8( match );
    auto ptr = line;
    for(;;)
    {
        __m256i l = _mm256_loadu_si256( (const __m256i*)ptr );
        __m256i c = _mm256_cmpeq_epi8( l, m );
        auto b = ~uint32_t( _mm256_movemask_epi8( c ) );
        if( b != 0 )
        {
            line = ptr + __builtin_ctz( b );
            return;
        }
        ptr += 32;
    }
}
#else
static inline void AdvanceToNot( const char*& line, char match )
{
    auto ptr = line;
    for(;;)
    {
        uint64_t l;
        memcpy( &l, ptr, 8 );
        for( int i=0; i<8; i++ )
        {
            if( ( l & 0xFF ) != uint8_t( match ) )
            {
                line = ptr + i;
                return;
            }
            l >>= 8;
        }
        ptr += 8;
    }
}
#endif

#ifdef __AVX2__
template<int S>
static inline void AdvanceTo( const char*& line, const char* match )
{
    auto first = uint8_t( match[0] );
    __m256i m = _mm256_set1_epi8( first );
    auto ptr = line;
    for(;;)
    {
        __m256i l = _mm256_loadu_si256( (const __m256i*)ptr );
        __m256i c = _mm256_cmpeq_epi8( l, m );
        auto b = uint32_t( _mm256_movemask_epi8( c ) );
        while( b != 0 )
        {
            auto bit = __builtin_ctz( b );
            auto test = ptr + bit;
            if( memcmp( test, match, S ) == 0 )
            {
                line = test;
                return;
            }
            b ^= ( 1u << bit );
        }
        ptr += 32;
    }
}
#else
template<int S>
static inline void AdvanceTo( const char*& line, const char* match )
{
    auto first = uint8_t( match[0] );
    auto ptr = line;
    for(;;)
    {
        uint64_t l;
        memcpy( &l, ptr, 8 );
        for( int i=0; i<8; i++ )
        {
            if( ( l & 0xFF ) == first )
            {
                if( memcmp( ptr + i, match, S ) == 0 )
                {
                    line = ptr + i;
                    return;
                }
            }
            l >>= 8;
        }
        ptr += 8;
    }
}
#endif

static void HandleTraceLine( const char* line )
{
    line += 23;
    AdvanceTo( line, '[' );
    line++;
    const auto cpu = (uint8_t)ReadNumber( line );
    line++;      // ']'
    AdvanceToNot( line, ' ' );

#if defined TRACY_HW_TIMER && ( defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64 )
    const auto time = ReadNumber( line );
#else
    const auto ts = ReadNumber( line );
    line++;      // '.'
    const auto tus = ReadNumber( line );
    const auto time = ts * 1000000000ll + tus * 1000ll;
#endif

    line += 2;   // ': '
    if( memcmp( line, "sched_switch", 12 ) == 0 )
    {
        line += 14;

        AdvanceTo<8>( line, "prev_pid" );
        line += 9;

        const auto oldPid = ReadNumber( line );
        line++;

        AdvanceTo<10>( line, "prev_state" );
        line += 11;

        const auto oldState = (uint8_t)ReadState( *line );
        line += 5;

        AdvanceTo<8>( line, "next_pid" );
        line += 9;

        const auto newPid = ReadNumber( line );

        uint8_t reason = 100;

        TracyLfqPrepare( QueueType::ContextSwitch );
        MemWrite( &item->contextSwitch.time, time );
        MemWrite( &item->contextSwitch.oldThread, oldPid );
        MemWrite( &item->contextSwitch.newThread, newPid );
        MemWrite( &item->contextSwitch.cpu, cpu );
        MemWrite( &item->contextSwitch.reason, reason );
        MemWrite( &item->contextSwitch.state, oldState );
        TracyLfqCommit;
    }
    else if( memcmp( line, "sched_wakeup", 12 ) == 0 )
    {
        line += 14;

        AdvanceTo<4>( line, "pid=" );
        line += 4;

        const auto pid = ReadNumber( line );

        TracyLfqPrepare( QueueType::ThreadWakeup );
        MemWrite( &item->threadWakeup.time, time );
        MemWrite( &item->threadWakeup.thread, pid );
        TracyLfqCommit;
    }
}

#ifdef __ANDROID__
static void ProcessTraceLines( int fd )
{
    // Linux pipe buffer is 64KB, additional 1KB is for unfinished lines
    char* buf = (char*)tracy_malloc( (64+1)*1024 );
    char* line = buf;

    for(;;)
    {
        if( !traceActive.load( std::memory_order_relaxed ) ) break;

        const auto rd = read( fd, line, 64*1024 );
        if( rd <= 0 ) break;

#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() )
        {
            if( rd < 64*1024 )
            {
                assert( line[rd-1] == '\n' );
                line = buf;
                std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
            }
            else
            {
                const auto end = line + rd;
                line = end - 1;
                while( line > buf && *line != '\n' ) line--;
                if( line > buf )
                {
                    line++;
                    const auto lsz = end - line;
                    memmove( buf, line, lsz );
                    line = buf + lsz;
                }
            }
            continue;
        }
#endif

        const auto end = line + rd;
        line = buf;
        for(;;)
        {
            auto next = (char*)memchr( line, '\n', end - line );
            if( !next )
            {
                const auto lsz = end - line;
                memmove( buf, line, lsz );
                line = buf + lsz;
                break;
            }
            HandleTraceLine( line );
            line = ++next;
        }
        if( rd < 64*1024 )
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        }
    }

    tracy_free( buf );
}

void SysTraceWorker( void* ptr )
{
    ThreadExitHandler threadExitHandler;
    SetThreadName( "Tracy SysTrace" );
    int pipefd[2];
    if( pipe( pipefd ) == 0 )
    {
        const auto pid = fork();
        if( pid == 0 )
        {
            // child
            close( pipefd[0] );
            dup2( open( "/dev/null", O_WRONLY ), STDERR_FILENO );
            if( dup2( pipefd[1], STDOUT_FILENO ) >= 0 )
            {
                close( pipefd[1] );
                sched_param sp = { 4 };
                pthread_setschedparam( pthread_self(), SCHED_FIFO, &sp );
#if defined __ANDROID__ && ( defined __aarch64__ || defined __ARM_ARCH )
                execlp( "su", "su", "root", "sh", "-c", "/data/tracy_systrace", (char*)nullptr );
#endif
                execlp( "su", "su", "root", "sh", "-c", "cat /sys/kernel/debug/tracing/trace_pipe", (char*)nullptr );
                exit( 1 );
            }
        }
        else if( pid > 0 )
        {
            // parent
            close( pipefd[1] );
            sched_param sp = { 5 };
            pthread_setschedparam( pthread_self(), SCHED_FIFO, &sp );
            ProcessTraceLines( pipefd[0] );
            close( pipefd[0] );
            waitpid( pid, nullptr, 0 );
        }
    }
}
#else
static void ProcessTraceLines( int fd )
{
    // 32 bytes buffer space for wide unbound reads
    char* buf = (char*)tracy_malloc( 64*1024 + 32 );

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN | POLLERR;

    for(;;)
    {
        while( poll( &pfd, 1, 0 ) <= 0 )
        {
            if( !traceActive.load( std::memory_order_relaxed ) ) break;
            std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        }

        const auto rd = read( fd, buf, 64*1024 );
        if( rd <= 0 ) break;

#ifdef TRACY_ON_DEMAND
        if( !GetProfiler().IsConnected() ) continue;
#endif

        auto line = buf;
        const auto end = buf + rd;
        for(;;)
        {
            auto next = (char*)memchr( line, '\n', end - line );
            if( !next ) break;
            HandleTraceLine( line );
            line = ++next;
        }
    }

    tracy_free( buf );
}

void SysTraceWorker( void* ptr )
{
    ThreadExitHandler threadExitHandler;
    SetThreadName( "Tracy SysTrace" );
    char tmp[256];
    memcpy( tmp, BasePath, sizeof( BasePath ) - 1 );
    memcpy( tmp + sizeof( BasePath ) - 1, TracePipe, sizeof( TracePipe ) );

    int fd = open( tmp, O_RDONLY );
    if( fd < 0 ) return;
    sched_param sp = { 5 };
    pthread_setschedparam( pthread_self(), SCHED_FIFO, &sp );
    ProcessTraceLines( fd );
    close( fd );
}
#endif

void SysTraceSendExternalName( uint64_t thread )
{
    FILE* f;
    char fn[256];
    sprintf( fn, "/proc/%" PRIu64 "/comm", thread );
    f = fopen( fn, "rb" );
    if( f )
    {
        char buf[256];
        const auto sz = fread( buf, 1, 256, f );
        if( sz > 0 && buf[sz-1] == '\n' ) buf[sz-1] = '\0';
        GetProfiler().SendString( thread, buf, QueueType::ExternalThreadName );
        fclose( f );
    }
    else
    {
        GetProfiler().SendString( thread, "???", 3, QueueType::ExternalThreadName );
    }

    sprintf( fn, "/proc/%" PRIu64 "/status", thread );
    f = fopen( fn, "rb" );
    if( f )
    {
        int pid = -1;
        size_t lsz = 1024;
        auto line = (char*)tracy_malloc( lsz );
        for(;;)
        {
            auto rd = getline( &line, &lsz, f );
            if( rd <= 0 ) break;
            if( memcmp( "Tgid:\t", line, 6 ) == 0 )
            {
                pid = atoi( line + 6 );
                break;
            }
        }
        tracy_free_fast( line );
        fclose( f );
        if( pid >= 0 )
        {
            {
                uint64_t _pid = pid;
                TracyLfqPrepare( QueueType::TidToPid );
                MemWrite( &item->tidToPid.tid, thread );
                MemWrite( &item->tidToPid.pid, _pid );
                TracyLfqCommit;
            }
            sprintf( fn, "/proc/%i/comm", pid );
            f = fopen( fn, "rb" );
            if( f )
            {
                char buf[256];
                const auto sz = fread( buf, 1, 256, f );
                if( sz > 0 && buf[sz-1] == '\n' ) buf[sz-1] = '\0';
                GetProfiler().SendString( thread, buf, QueueType::ExternalName );
                fclose( f );
                return;
            }
        }
    }
    GetProfiler().SendString( thread, "???", 3, QueueType::ExternalName );
}

}

#  endif

#endif

/*** End of inlined file: TracySysTrace.cpp ***/


/*** Start of inlined file: TracySocket.cpp ***/
#include <cassert>
#include <inttypes.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  ifdef _MSC_VER
#    pragma warning(disable:4244)
#    pragma warning(disable:4267)
#  endif
#  define poll WSAPoll
#else
#  include <arpa/inet.h>
#  include <sys/socket.h>
#  include <sys/param.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <unistd.h>
#  include <poll.h>
#endif

#ifndef MSG_NOSIGNAL
#  define MSG_NOSIGNAL 0
#endif

namespace tracy
{

#ifdef _WIN32
typedef SOCKET socket_t;
#else
typedef int socket_t;
#endif

#ifdef _WIN32
struct __wsinit
{
    __wsinit()
    {
        WSADATA wsaData;
        if( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 )
        {
            fprintf( stderr, "Cannot init winsock.\n" );
            exit( 1 );
        }
    }
};

void InitWinSock()
{
    static __wsinit init;
}
#endif

enum { BufSize = 128 * 1024 };

Socket::Socket()
    : m_buf( (char*)tracy_malloc( BufSize ) )
    , m_bufPtr( nullptr )
    , m_sock( -1 )
    , m_bufLeft( 0 )
    , m_ptr( nullptr )
{
#ifdef _WIN32
    InitWinSock();
#endif
}

Socket::Socket( int sock )
    : m_buf( (char*)tracy_malloc( BufSize ) )
    , m_bufPtr( nullptr )
    , m_sock( sock )
    , m_bufLeft( 0 )
    , m_ptr( nullptr )
{
}

Socket::~Socket()
{
    tracy_free( m_buf );
    if( m_sock.load( std::memory_order_relaxed ) != -1 )
    {
        Close();
    }
    if( m_ptr )
    {
        freeaddrinfo( m_res );
#ifdef _WIN32
        closesocket( m_connSock );
#else
        close( m_connSock );
#endif
    }
}

bool Socket::Connect( const char* addr, uint16_t port )
{
    assert( !IsValid() );

    if( m_ptr )
    {
        const auto c = connect( m_connSock, m_ptr->ai_addr, m_ptr->ai_addrlen );
        if( c == -1 )
        {
#if defined _WIN32
            const auto err = WSAGetLastError();
            if( err == WSAEALREADY || err == WSAEINPROGRESS ) return false;
            if( err != WSAEISCONN )
            {
                freeaddrinfo( m_res );
                closesocket( m_connSock );
                m_ptr = nullptr;
                return false;
            }
#else
            const auto err = errno;
            if( err == EALREADY || err == EINPROGRESS ) return false;
            if( err != EISCONN )
            {
                freeaddrinfo( m_res );
                close( m_connSock );
                m_ptr = nullptr;
                return false;
            }
#endif
        }

#if defined _WIN32
        u_long nonblocking = 0;
        ioctlsocket( m_connSock, FIONBIO, &nonblocking );
#else
        int flags = fcntl( m_connSock, F_GETFL, 0 );
        fcntl( m_connSock, F_SETFL, flags & ~O_NONBLOCK );
#endif
        m_sock.store( m_connSock, std::memory_order_relaxed );
        freeaddrinfo( m_res );
        m_ptr = nullptr;
        return true;
    }

    struct addrinfo hints;
    struct addrinfo *res, *ptr;

    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char portbuf[32];
    sprintf( portbuf, "%" PRIu16, port );

    if( getaddrinfo( addr, portbuf, &hints, &res ) != 0 ) return false;
    int sock = 0;
    for( ptr = res; ptr; ptr = ptr->ai_next )
    {
        if( ( sock = socket( ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol ) ) == -1 ) continue;
#if defined __APPLE__
        int val = 1;
        setsockopt( sock, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof( val ) );
#endif
#if defined _WIN32
        u_long nonblocking = 1;
        ioctlsocket( sock, FIONBIO, &nonblocking );
#else
        int flags = fcntl( sock, F_GETFL, 0 );
        fcntl( sock, F_SETFL, flags | O_NONBLOCK );
#endif
        if( connect( sock, ptr->ai_addr, ptr->ai_addrlen ) == 0 )
        {
            break;
        }
        else
        {
#if defined _WIN32
            const auto err = WSAGetLastError();
            if( err != WSAEWOULDBLOCK )
            {
                closesocket( sock );
                continue;
            }
#else
            if( errno != EINPROGRESS )
            {
                close( sock );
                continue;
            }
#endif
        }
        m_res = res;
        m_ptr = ptr;
        m_connSock = sock;
        return false;
    }
    freeaddrinfo( res );
    if( !ptr ) return false;

#if defined _WIN32
    u_long nonblocking = 0;
    ioctlsocket( sock, FIONBIO, &nonblocking );
#else
    int flags = fcntl( sock, F_GETFL, 0 );
    fcntl( sock, F_SETFL, flags & ~O_NONBLOCK );
#endif

    m_sock.store( sock, std::memory_order_relaxed );
    return true;
}

bool Socket::ConnectBlocking( const char* addr, uint16_t port )
{
    assert( !IsValid() );
    assert( !m_ptr );

    struct addrinfo hints;
    struct addrinfo *res, *ptr;

    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char portbuf[32];
    sprintf( portbuf, "%" PRIu16, port );

    if( getaddrinfo( addr, portbuf, &hints, &res ) != 0 ) return false;
    int sock = 0;
    for( ptr = res; ptr; ptr = ptr->ai_next )
    {
        if( ( sock = socket( ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol ) ) == -1 ) continue;
#if defined __APPLE__
        int val = 1;
        setsockopt( sock, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof( val ) );
#endif
        if( connect( sock, ptr->ai_addr, ptr->ai_addrlen ) == -1 )
        {
#ifdef _WIN32
            closesocket( sock );
#else
            close( sock );
#endif
            continue;
        }
        break;
    }
    freeaddrinfo( res );
    if( !ptr ) return false;

    m_sock.store( sock, std::memory_order_relaxed );
    return true;
}

void Socket::Close()
{
    const auto sock = m_sock.load( std::memory_order_relaxed );
    assert( sock != -1 );
#ifdef _WIN32
    closesocket( sock );
#else
    close( sock );
#endif
    m_sock.store( -1, std::memory_order_relaxed );
}

int Socket::Send( const void* _buf, int len )
{
    const auto sock = m_sock.load( std::memory_order_relaxed );
    auto buf = (const char*)_buf;
    assert( sock != -1 );
    auto start = buf;
    while( len > 0 )
    {
        auto ret = send( sock, buf, len, MSG_NOSIGNAL );
        if( ret == -1 ) return -1;
        len -= ret;
        buf += ret;
    }
    return int( buf - start );
}

int Socket::GetSendBufSize()
{
    const auto sock = m_sock.load( std::memory_order_relaxed );
    int bufSize;
#if defined _WIN32
    int sz = sizeof( bufSize );
    getsockopt( sock, SOL_SOCKET, SO_SNDBUF, (char*)&bufSize, &sz );
#else
    socklen_t sz = sizeof( bufSize );
    getsockopt( sock, SOL_SOCKET, SO_SNDBUF, &bufSize, &sz );
#endif
    return bufSize;
}

int Socket::RecvBuffered( void* buf, int len, int timeout )
{
    if( len <= m_bufLeft )
    {
        memcpy( buf, m_bufPtr, len );
        m_bufPtr += len;
        m_bufLeft -= len;
        return len;
    }

    if( m_bufLeft > 0 )
    {
        memcpy( buf, m_bufPtr, m_bufLeft );
        const auto ret = m_bufLeft;
        m_bufLeft = 0;
        return ret;
    }

    if( len >= BufSize ) return Recv( buf, len, timeout );

    m_bufLeft = Recv( m_buf, BufSize, timeout );
    if( m_bufLeft <= 0 ) return m_bufLeft;

    const auto sz = len < m_bufLeft ? len : m_bufLeft;
    memcpy( buf, m_buf, sz );
    m_bufPtr = m_buf + sz;
    m_bufLeft -= sz;
    return sz;
}

int Socket::Recv( void* _buf, int len, int timeout )
{
    const auto sock = m_sock.load( std::memory_order_relaxed );
    auto buf = (char*)_buf;

    struct pollfd fd;
    fd.fd = (socket_t)sock;
    fd.events = POLLIN;

    if( poll( &fd, 1, timeout ) > 0 )
    {
        return recv( sock, buf, len, 0 );
    }
    else
    {
        return -1;
    }
}

int Socket::ReadUpTo( void* _buf, int len, int timeout )
{
    const auto sock = m_sock.load( std::memory_order_relaxed );
    auto buf = (char*)_buf;

    int rd = 0;
    while( len > 0 )
    {
        const auto res = recv( sock, buf, len, 0 );
        if( res == 0 ) break;
        if( res == -1 ) return -1;
        len -= res;
        rd += res;
        buf += res;
    }
    return rd;
}

bool Socket::Read( void* buf, int len, int timeout )
{
    auto cbuf = (char*)buf;
    while( len > 0 )
    {
        if( !ReadImpl( cbuf, len, timeout ) ) return false;
    }
    return true;
}

bool Socket::ReadImpl( char*& buf, int& len, int timeout )
{
    const auto sz = RecvBuffered( buf, len, timeout );
    switch( sz )
    {
    case 0:
        return false;
    case -1:
#ifdef _WIN32
    {
        auto err = WSAGetLastError();
        if( err == WSAECONNABORTED || err == WSAECONNRESET ) return false;
    }
#endif
    break;
    default:
        len -= sz;
        buf += sz;
        break;
    }
    return true;
}

bool Socket::ReadRaw( void* _buf, int len, int timeout )
{
    auto buf = (char*)_buf;
    while( len > 0 )
    {
        const auto sz = Recv( buf, len, timeout );
        if( sz <= 0 ) return false;
        len -= sz;
        buf += sz;
    }
    return true;
}

bool Socket::HasData()
{
    const auto sock = m_sock.load( std::memory_order_relaxed );
    if( m_bufLeft > 0 ) return true;

    struct pollfd fd;
    fd.fd = (socket_t)sock;
    fd.events = POLLIN;

    return poll( &fd, 1, 0 ) > 0;
}

bool Socket::IsValid() const
{
    return m_sock.load( std::memory_order_relaxed ) >= 0;
}

ListenSocket::ListenSocket()
    : m_sock( -1 )
{
#ifdef _WIN32
    InitWinSock();
#endif
}

ListenSocket::~ListenSocket()
{
    if( m_sock != -1 ) Close();
}

static int addrinfo_and_socket_for_family( uint16_t port, int ai_family, struct addrinfo** res )
{
    struct addrinfo hints;
    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = ai_family;
    hints.ai_socktype = SOCK_STREAM;
#ifndef TRACY_ONLY_LOCALHOST
    const char* onlyLocalhost = GetEnvVar( "TRACY_ONLY_LOCALHOST" );
    if( !onlyLocalhost || onlyLocalhost[0] != '1' )
    {
        hints.ai_flags = AI_PASSIVE;
    }
#endif
    char portbuf[32];
    sprintf( portbuf, "%" PRIu16, port );
    if( getaddrinfo( nullptr, portbuf, &hints, res ) != 0 ) return -1;
    int sock = socket( (*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol );
    if (sock == -1) freeaddrinfo( *res );
    return sock;
}

bool ListenSocket::Listen( uint16_t port, int backlog )
{
    assert( m_sock == -1 );

    struct addrinfo* res = nullptr;

#if !defined TRACY_ONLY_IPV4 && !defined TRACY_ONLY_LOCALHOST
    const char* onlyIPv4 = GetEnvVar( "TRACY_ONLY_IPV4" );
    if( !onlyIPv4 || onlyIPv4[0] != '1' )
    {
        m_sock = addrinfo_and_socket_for_family( port, AF_INET6, &res );
    }
#endif
    if (m_sock == -1)
    {
        // IPV6 protocol may not be available/is disabled. Try to create a socket
        // with the IPV4 protocol
        m_sock = addrinfo_and_socket_for_family( port, AF_INET, &res );
        if( m_sock == -1 ) return false;
    }
#if defined _WIN32 || defined __CYGWIN__
    unsigned long val = 0;
    setsockopt( m_sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&val, sizeof( val ) );
#elif defined BSD
    int val = 0;
    setsockopt( m_sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&val, sizeof( val ) );
    val = 1;
    setsockopt( m_sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof( val ) );
#else
    int val = 1;
    setsockopt( m_sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof( val ) );
#endif
    if( bind( m_sock, res->ai_addr, res->ai_addrlen ) == -1 ) { freeaddrinfo( res ); Close(); return false; }
    if( listen( m_sock, backlog ) == -1 ) { freeaddrinfo( res ); Close(); return false; }
    freeaddrinfo( res );
    return true;
}

Socket* ListenSocket::Accept()
{
    struct sockaddr_storage remote;
    socklen_t sz = sizeof( remote );

    struct pollfd fd;
    fd.fd = (socket_t)m_sock;
    fd.events = POLLIN;

    if( poll( &fd, 1, 10 ) > 0 )
    {
        int sock = accept( m_sock, (sockaddr*)&remote, &sz);
        if( sock == -1 ) return nullptr;

#if defined __APPLE__
        int val = 1;
        setsockopt( sock, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof( val ) );
#endif

        auto ptr = (Socket*)tracy_malloc( sizeof( Socket ) );
        new(ptr) Socket( sock );
        return ptr;
    }
    else
    {
        return nullptr;
    }
}

void ListenSocket::Close()
{
    assert( m_sock != -1 );
#ifdef _WIN32
    closesocket( m_sock );
#else
    close( m_sock );
#endif
    m_sock = -1;
}

UdpBroadcast::UdpBroadcast()
    : m_sock( -1 )
{
#ifdef _WIN32
    InitWinSock();
#endif
}

UdpBroadcast::~UdpBroadcast()
{
    if( m_sock != -1 ) Close();
}

bool UdpBroadcast::Open( const char* addr, uint16_t port )
{
    assert( m_sock == -1 );

    struct addrinfo hints;
    struct addrinfo *res, *ptr;

    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    char portbuf[32];
    sprintf( portbuf, "%" PRIu16, port );

    if( getaddrinfo( addr, portbuf, &hints, &res ) != 0 ) return false;
    int sock = 0;
    for( ptr = res; ptr; ptr = ptr->ai_next )
    {
        if( ( sock = socket( ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol ) ) == -1 ) continue;
#if defined __APPLE__
        int val = 1;
        setsockopt( sock, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof( val ) );
#endif
#if defined _WIN32
        unsigned long broadcast = 1;
        if( setsockopt( sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof( broadcast ) ) == -1 )
#else
        int broadcast = 1;
        if( setsockopt( sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof( broadcast ) ) == -1 )
#endif
        {
#ifdef _WIN32
            closesocket( sock );
#else
            close( sock );
#endif
            continue;
        }
        break;
    }
    freeaddrinfo( res );
    if( !ptr ) return false;

    m_sock = sock;
    inet_pton( AF_INET, addr, &m_addr );
    return true;
}

void UdpBroadcast::Close()
{
    assert( m_sock != -1 );
#ifdef _WIN32
    closesocket( m_sock );
#else
    close( m_sock );
#endif
    m_sock = -1;
}

int UdpBroadcast::Send( uint16_t port, const void* data, int len )
{
    assert( m_sock != -1 );
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( port );
    addr.sin_addr.s_addr = m_addr;
    return sendto( m_sock, (const char*)data, len, MSG_NOSIGNAL, (sockaddr*)&addr, sizeof( addr ) );
}

IpAddress::IpAddress()
    : m_number( 0 )
{
    *m_text = '\0';
}

IpAddress::~IpAddress()
{
}

void IpAddress::Set( const struct sockaddr& addr )
{
#if defined _WIN32 && ( !defined NTDDI_WIN10 || NTDDI_VERSION < NTDDI_WIN10 )
    struct sockaddr_in tmp;
    memcpy( &tmp, &addr, sizeof( tmp ) );
    auto ai = &tmp;
#else
    auto ai = (const struct sockaddr_in*)&addr;
#endif
    inet_ntop( AF_INET, &ai->sin_addr, m_text, 17 );
    m_number = ai->sin_addr.s_addr;
}

UdpListen::UdpListen()
    : m_sock( -1 )
{
#ifdef _WIN32
    InitWinSock();
#endif
}

UdpListen::~UdpListen()
{
    if( m_sock != -1 ) Close();
}

bool UdpListen::Listen( uint16_t port )
{
    assert( m_sock == -1 );

    int sock;
    if( ( sock = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) return false;

#if defined __APPLE__
    int val = 1;
    setsockopt( sock, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof( val ) );
#endif
#if defined _WIN32
    unsigned long reuse = 1;
    setsockopt( m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof( reuse ) );
#else
    int reuse = 1;
    setsockopt( m_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( reuse ) );
#endif
#if defined _WIN32
    unsigned long broadcast = 1;
    if( setsockopt( sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof( broadcast ) ) == -1 )
#else
    int broadcast = 1;
    if( setsockopt( sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof( broadcast ) ) == -1 )
#endif
    {
#ifdef _WIN32
        closesocket( sock );
#else
        close( sock );
#endif
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( port );
    addr.sin_addr.s_addr = INADDR_ANY;

    if( bind( sock, (sockaddr*)&addr, sizeof( addr ) ) == -1 )
    {
#ifdef _WIN32
        closesocket( sock );
#else
        close( sock );
#endif
        return false;
    }

    m_sock = sock;
    return true;
}

void UdpListen::Close()
{
    assert( m_sock != -1 );
#ifdef _WIN32
    closesocket( m_sock );
#else
    close( m_sock );
#endif
    m_sock = -1;
}

const char* UdpListen::Read( size_t& len, IpAddress& addr, int timeout )
{
    static char buf[2048];

    struct pollfd fd;
    fd.fd = (socket_t)m_sock;
    fd.events = POLLIN;
    if( poll( &fd, 1, timeout ) <= 0 ) return nullptr;

    sockaddr sa;
    socklen_t salen = sizeof( struct sockaddr );
    len = (size_t)recvfrom( m_sock, buf, 2048, 0, &sa, &salen );
    addr.Set( sa );

    return buf;
}

}

/*** End of inlined file: TracySocket.cpp ***/


/*** Start of inlined file: tracy_rpmalloc.cpp ***/
#ifdef TRACY_ENABLE

/* rpmalloc.c  -  Memory allocator  -  Public Domain  -  2016 Mattias Jansson
 *
 * This library provides a cross-platform lock free thread caching malloc implementation in C11.
 * The latest source code is always available at
 *
 * https://github.com/mjansson/rpmalloc
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without any restrictions.
 *
 */

/// Build time configurable limits
#ifndef HEAP_ARRAY_SIZE
//! Size of heap hashmap
#define HEAP_ARRAY_SIZE           47
#endif
#ifndef ENABLE_THREAD_CACHE
//! Enable per-thread cache
#define ENABLE_THREAD_CACHE       1
#endif
#ifndef ENABLE_GLOBAL_CACHE
//! Enable global cache shared between all threads, requires thread cache
#define ENABLE_GLOBAL_CACHE       1
#endif
#ifndef ENABLE_VALIDATE_ARGS
//! Enable validation of args to public entry points
#define ENABLE_VALIDATE_ARGS      0
#endif
#ifndef ENABLE_STATISTICS
//! Enable statistics collection
#define ENABLE_STATISTICS         0
#endif
#ifndef ENABLE_ASSERTS
//! Enable asserts
#define ENABLE_ASSERTS            0
#endif
#ifndef ENABLE_OVERRIDE
//! Override standard library malloc/free and new/delete entry points
#define ENABLE_OVERRIDE           0
#endif
#ifndef ENABLE_PRELOAD
//! Support preloading
#define ENABLE_PRELOAD            0
#endif
#ifndef DISABLE_UNMAP
//! Disable unmapping memory pages
#define DISABLE_UNMAP             0
#endif
#ifndef DEFAULT_SPAN_MAP_COUNT
//! Default number of spans to map in call to map more virtual memory (default values yield 4MiB here)
#define DEFAULT_SPAN_MAP_COUNT    64
#endif

#if ENABLE_THREAD_CACHE
#ifndef ENABLE_UNLIMITED_CACHE
//! Unlimited thread and global cache
#define ENABLE_UNLIMITED_CACHE    0
#endif
#ifndef ENABLE_UNLIMITED_THREAD_CACHE
//! Unlimited cache disables any thread cache limitations
#define ENABLE_UNLIMITED_THREAD_CACHE ENABLE_UNLIMITED_CACHE
#endif
#if !ENABLE_UNLIMITED_THREAD_CACHE
#ifndef THREAD_CACHE_MULTIPLIER
//! Multiplier for thread cache (cache limit will be span release count multiplied by this value)
#define THREAD_CACHE_MULTIPLIER 16
#endif
#ifndef ENABLE_ADAPTIVE_THREAD_CACHE
//! Enable adaptive size of per-thread cache (still bounded by THREAD_CACHE_MULTIPLIER hard limit)
#define ENABLE_ADAPTIVE_THREAD_CACHE  0
#endif
#endif
#endif

#if ENABLE_GLOBAL_CACHE && ENABLE_THREAD_CACHE
#ifndef ENABLE_UNLIMITED_GLOBAL_CACHE
//! Unlimited cache disables any global cache limitations
#define ENABLE_UNLIMITED_GLOBAL_CACHE ENABLE_UNLIMITED_CACHE
#endif
#if !ENABLE_UNLIMITED_GLOBAL_CACHE
//! Multiplier for global cache (cache limit will be span release count multiplied by this value)
#define GLOBAL_CACHE_MULTIPLIER (THREAD_CACHE_MULTIPLIER * 6)
#endif
#else
#  undef ENABLE_GLOBAL_CACHE
#  define ENABLE_GLOBAL_CACHE 0
#endif

#if !ENABLE_THREAD_CACHE || ENABLE_UNLIMITED_THREAD_CACHE
#  undef ENABLE_ADAPTIVE_THREAD_CACHE
#  define ENABLE_ADAPTIVE_THREAD_CACHE 0
#endif

#if DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#  error Must use global cache if unmap is disabled
#endif

#if defined( _WIN32 ) || defined( __WIN32__ ) || defined( _WIN64 )
#  define PLATFORM_WINDOWS 1
#  define PLATFORM_POSIX 0
#else
#  define PLATFORM_WINDOWS 0
#  define PLATFORM_POSIX 1
#endif

#define _Static_assert static_assert

/// Platform and arch specifics
#ifndef FORCEINLINE
#  if defined(_MSC_VER) && !defined(__clang__)
#    define FORCEINLINE inline __forceinline
#  else
#    define FORCEINLINE inline __attribute__((__always_inline__))
#  endif
#endif
#if PLATFORM_WINDOWS
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  if ENABLE_VALIDATE_ARGS
#    include <Intsafe.h>
#  endif
#else
#  include <unistd.h>
#  include <stdio.h>
#  include <stdlib.h>
#  if defined(__APPLE__)
#    if !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#      include <mach/mach_vm.h>
#    endif
#    include <mach/vm_statistics.h>
#    include <pthread.h>
#  endif
#  if defined(__HAIKU__)
#    include <OS.h>
#    include <pthread.h>
#  endif
#endif

#include <cstdint>
#include <string.h>

#if ENABLE_ASSERTS
#  undef NDEBUG
#  if defined(_MSC_VER) && !defined(_DEBUG)
#    define _DEBUG
#  endif
#  include <cassert>
#else
#  undef  assert
#  define assert(x) do {} while(0)
#endif
#if ENABLE_STATISTICS
#  include <stdio.h>
#endif

#include <atomic>

namespace tracy
{

typedef std::atomic<int32_t> atomic32_t;
typedef std::atomic<int64_t> atomic64_t;
typedef std::atomic<void*> atomicptr_t;

#define atomic_thread_fence_acquire() std::atomic_thread_fence(std::memory_order_acquire)
#define atomic_thread_fence_release() std::atomic_thread_fence(std::memory_order_release)

static FORCEINLINE int32_t atomic_load32(atomic32_t* src) { return std::atomic_load_explicit(src, std::memory_order_relaxed); }
static FORCEINLINE void    atomic_store32(atomic32_t* dst, int32_t val) { std::atomic_store_explicit(dst, val, std::memory_order_relaxed); }
static FORCEINLINE int32_t atomic_incr32(atomic32_t* val) { return std::atomic_fetch_add_explicit(val, 1, std::memory_order_relaxed) + 1; }
#if ENABLE_STATISTICS || ENABLE_ADAPTIVE_THREAD_CACHE
static FORCEINLINE int32_t atomic_decr32(atomic32_t* val) { return atomic_fetch_add_explicit(val, -1, memory_order_relaxed) - 1; }
#endif
static FORCEINLINE int32_t atomic_add32(atomic32_t* val, int32_t add) { return std::atomic_fetch_add_explicit(val, add, std::memory_order_relaxed) + add; }
static FORCEINLINE void*   atomic_load_ptr(atomicptr_t* src) { return std::atomic_load_explicit(src, std::memory_order_relaxed); }
static FORCEINLINE void    atomic_store_ptr(atomicptr_t* dst, void* val) { std::atomic_store_explicit(dst, val, std::memory_order_relaxed); }
static FORCEINLINE int     atomic_cas_ptr(atomicptr_t* dst, void* val, void* ref) { return std::atomic_compare_exchange_weak_explicit(dst, &ref, val, std::memory_order_release, std::memory_order_acquire); }

#if defined(_MSC_VER) && !defined(__clang__)
#  define EXPECTED(x) (x)
#  define UNEXPECTED(x) (x)
#else
#  define EXPECTED(x) __builtin_expect((x), 1)
#  define UNEXPECTED(x) __builtin_expect((x), 0)
#endif

/// Preconfigured limits and sizes
//! Granularity of a small allocation block
#define SMALL_GRANULARITY         16
//! Small granularity shift count
#define SMALL_GRANULARITY_SHIFT   4
//! Number of small block size classes
#define SMALL_CLASS_COUNT         65
//! Maximum size of a small block
#define SMALL_SIZE_LIMIT          (SMALL_GRANULARITY * (SMALL_CLASS_COUNT - 1))
//! Granularity of a medium allocation block
#define MEDIUM_GRANULARITY        512
//! Medium granularity shift count
#define MEDIUM_GRANULARITY_SHIFT  9
//! Number of medium block size classes
#define MEDIUM_CLASS_COUNT        61
//! Total number of small + medium size classes
#define SIZE_CLASS_COUNT          (SMALL_CLASS_COUNT + MEDIUM_CLASS_COUNT)
//! Number of large block size classes
#define LARGE_CLASS_COUNT         32
//! Maximum size of a medium block
#define MEDIUM_SIZE_LIMIT         (SMALL_SIZE_LIMIT + (MEDIUM_GRANULARITY * MEDIUM_CLASS_COUNT))
//! Maximum size of a large block
#define LARGE_SIZE_LIMIT          ((LARGE_CLASS_COUNT * _memory_span_size) - SPAN_HEADER_SIZE)
//! Size of a span header (must be a multiple of SMALL_GRANULARITY)
#define SPAN_HEADER_SIZE          96

#if ENABLE_VALIDATE_ARGS
//! Maximum allocation size to avoid integer overflow
#undef  MAX_ALLOC_SIZE
#define MAX_ALLOC_SIZE            (((size_t)-1) - _memory_span_size)
#endif

#define pointer_offset(ptr, ofs) (void*)((char*)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff(first, second) (ptrdiff_t)((const char*)(first) - (const char*)(second))

#define INVALID_POINTER ((void*)((uintptr_t)-1))

/// Data types
//! A memory heap, per thread
typedef struct heap_t heap_t;
//! Heap spans per size class
typedef struct heap_class_t heap_class_t;
//! Span of memory pages
typedef struct span_t span_t;
//! Span list
typedef struct span_list_t span_list_t;
//! Span active data
typedef struct span_active_t span_active_t;
//! Size class definition
typedef struct size_class_t size_class_t;
//! Global cache
typedef struct global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#define SPAN_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#define SPAN_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#define SPAN_FLAG_ALIGNED_BLOCKS 4U

#if ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
struct span_use_t {
	//! Current number of spans used (actually used, not in cache)
	atomic32_t current;
	//! High water mark of spans used
	uint32_t high;
#if ENABLE_STATISTICS
	//! Number of spans transitioned to global cache
	uint32_t spans_to_global;
	//! Number of spans transitioned from global cache
	uint32_t spans_from_global;
	//! Number of spans transitioned to thread cache
	uint32_t spans_to_cache;
	//! Number of spans transitioned from thread cache
	uint32_t spans_from_cache;
	//! Number of spans transitioned to reserved state
	uint32_t spans_to_reserved;
	//! Number of spans transitioned from reserved state
	uint32_t spans_from_reserved;
	//! Number of raw memory map calls
	uint32_t spans_map_calls;
#endif
};
typedef struct span_use_t span_use_t;
#endif

#if ENABLE_STATISTICS
struct size_class_use_t {
	//! Current number of allocations
	atomic32_t alloc_current;
	//! Peak number of allocations
	int32_t alloc_peak;
	//! Total number of allocations
	int32_t alloc_total;
	//! Total number of frees
	atomic32_t free_total;
	//! Number of spans in use
	uint32_t spans_current;
	//! Number of spans transitioned to cache
	uint32_t spans_peak;
	//! Number of spans transitioned to cache
	uint32_t spans_to_cache;
	//! Number of spans transitioned from cache
	uint32_t spans_from_cache;
	//! Number of spans transitioned from reserved state
	uint32_t spans_from_reserved;
	//! Number of spans mapped
	uint32_t spans_map_calls;
};
typedef struct size_class_use_t size_class_use_t;
#endif

typedef enum span_state_t {
	SPAN_STATE_ACTIVE = 0,
	SPAN_STATE_PARTIAL,
	SPAN_STATE_FULL
} span_state_t;

//A span can either represent a single span of memory pages with size declared by span_map_count configuration variable,
//or a set of spans in a continuous region, a super span. Any reference to the term "span" usually refers to both a single
//span or a super span. A super span can further be divided into multiple spans (or this, super spans), where the first
//(super)span is the master and subsequent (super)spans are subspans. The master span keeps track of how many subspans
//that are still alive and mapped in virtual memory, and once all subspans and master have been unmapped the entire
//superspan region is released and unmapped (on Windows for example, the entire superspan range has to be released
//in the same call to release the virtual memory range, but individual subranges can be decommitted individually
//to reduce physical memory use).
struct span_t {
	//! Free list
	void*       free_list;
	//! State
	uint32_t    state;
	//! Used count when not active (not including deferred free list)
	uint32_t    used_count;
	//! Block count
	uint32_t    block_count;
	//! Size class
	uint32_t    size_class;
	//! Index of last block initialized in free list
	uint32_t    free_list_limit;
	//! Span list size when part of a cache list, or size of deferred free list when partial/full
	uint32_t    list_size;
	//! Deferred free list
	atomicptr_t free_list_deferred;
	//! Size of a block
	uint32_t    block_size;
	//! Flags and counters
	uint32_t    flags;
	//! Number of spans
	uint32_t    span_count;
	//! Total span counter for master spans, distance for subspans
	uint32_t    total_spans_or_distance;
	//! Remaining span counter, for master spans
	atomic32_t  remaining_spans;
	//! Alignment offset
	uint32_t    align_offset;
	//! Owning heap
	heap_t*     heap;
	//! Next span
	span_t*     next;
	//! Previous span
	span_t*     prev;
};
_Static_assert(sizeof(span_t) <= SPAN_HEADER_SIZE, "span size mismatch");

struct heap_class_t {
	//! Free list of active span
	void*        free_list;
	//! Double linked list of partially used spans with free blocks for each size class.
	//  Current active span is at head of list. Previous span pointer in head points to tail span of list.
	span_t*      partial_span;
};

struct heap_t {
	//! Active and semi-used span data per size class
	heap_class_t span_class[SIZE_CLASS_COUNT];
#if ENABLE_THREAD_CACHE
	//! List of free spans (single linked list)
	span_t*      span_cache[LARGE_CLASS_COUNT];
	//! List of deferred free spans of class 0 (single linked list)
	atomicptr_t  span_cache_deferred;
#endif
#if ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
	//! Current and high water mark of spans used per span count
	span_use_t   span_use[LARGE_CLASS_COUNT];
#endif
	//! Mapped but unused spans
	span_t*      span_reserve;
	//! Master span for mapped but unused spans
	span_t*      span_reserve_master;
	//! Number of mapped but unused spans
	size_t       spans_reserved;
	//! Next heap in id list
	heap_t*      next_heap;
	//! Next heap in orphan list
	heap_t*      next_orphan;
	//! Memory pages alignment offset
	size_t       align_offset;
	//! Heap ID
	int32_t      id;
#if ENABLE_STATISTICS
	//! Number of bytes transitioned thread -> global
	size_t       thread_to_global;
	//! Number of bytes transitioned global -> thread
	size_t       global_to_thread;
	//! Allocation stats per size class
	size_class_use_t size_class_use[SIZE_CLASS_COUNT + 1];
#endif
};

struct size_class_t {
	//! Size of blocks in this class
	uint32_t block_size;
	//! Number of blocks in each chunk
	uint16_t block_count;
	//! Class index this class is merged with
	uint16_t class_idx;
};
_Static_assert(sizeof(size_class_t) == 8, "Size class size mismatch");

struct global_cache_t {
	//! Cache list pointer
	atomicptr_t cache;
	//! Cache size
	atomic32_t size;
	//! ABA counter
	atomic32_t counter;
};

/// Global data
//! Initialized flag
static int _rpmalloc_initialized;
//! Configuration
static rpmalloc_config_t _memory_config;
//! Memory page size
static size_t _memory_page_size;
//! Shift to divide by page size
static size_t _memory_page_size_shift;
//! Granularity at which memory pages are mapped by OS
static size_t _memory_map_granularity;
#if RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
static size_t _memory_span_size;
//! Shift to divide by span size
static size_t _memory_span_size_shift;
//! Mask to get to start of a memory span
static uintptr_t _memory_span_mask;
#else
//! Hardwired span size (64KiB)
#define _memory_span_size (64 * 1024)
#define _memory_span_size_shift 16
#define _memory_span_mask (~((uintptr_t)(_memory_span_size - 1)))
#endif
//! Number of spans to map in each map call
static size_t _memory_span_map_count;
//! Number of spans to release from thread cache to global cache (single spans)
static size_t _memory_span_release_count;
//! Number of spans to release from thread cache to global cache (large multiple spans)
static size_t _memory_span_release_count_large;
//! Global size classes
static size_class_t _memory_size_class[SIZE_CLASS_COUNT];
//! Run-time size limit of medium blocks
static size_t _memory_medium_size_limit;
//! Heap ID counter
static atomic32_t _memory_heap_id;
//! Huge page support
static int _memory_huge_pages;
#if ENABLE_GLOBAL_CACHE
//! Global span cache
static global_cache_t _memory_span_cache[LARGE_CLASS_COUNT];
#endif
//! All heaps
static atomicptr_t _memory_heaps[HEAP_ARRAY_SIZE];
//! Orphaned heaps
static atomicptr_t _memory_orphan_heaps;
//! Running orphan counter to avoid ABA issues in linked list
static atomic32_t _memory_orphan_counter;
#if ENABLE_STATISTICS
//! Active heap count
static atomic32_t _memory_active_heaps;
//! Number of currently mapped memory pages
static atomic32_t _mapped_pages;
//! Peak number of concurrently mapped memory pages
static int32_t _mapped_pages_peak;
//! Number of currently unused spans
static atomic32_t _reserved_spans;
//! Running counter of total number of mapped memory pages since start
static atomic32_t _mapped_total;
//! Running counter of total number of unmapped memory pages since start
static atomic32_t _unmapped_total;
//! Number of currently mapped memory pages in OS calls
static atomic32_t _mapped_pages_os;
//! Number of currently allocated pages in huge allocations
static atomic32_t _huge_pages_current;
//! Peak number of currently allocated pages in huge allocations
static int32_t _huge_pages_peak;
#endif

//! Current thread heap
#if (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
static pthread_key_t _memory_thread_heap;
#else
#  ifdef _MSC_VER
#    define _Thread_local __declspec(thread)
#    define TLS_MODEL
#  else
#    define TLS_MODEL __attribute__((tls_model("initial-exec")))
#    if !defined(__clang__) && defined(__GNUC__)
#      define _Thread_local __thread
#    endif
#  endif
static _Thread_local heap_t* _memory_thread_heap TLS_MODEL;
#endif

static inline heap_t*
get_thread_heap_raw(void) {
#if (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
	return pthread_getspecific(_memory_thread_heap);
#else
	return _memory_thread_heap;
#endif
}

//! Get the current thread heap
static inline heap_t*
get_thread_heap(void) {
	heap_t* heap = get_thread_heap_raw();
#if ENABLE_PRELOAD
	if (EXPECTED(heap != 0))
		return heap;
	rpmalloc_initialize();
	return get_thread_heap_raw();
#else
	return heap;
#endif
}

//! Set the current thread heap
static void
set_thread_heap(heap_t* heap) {
#if (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
	pthread_setspecific(_memory_thread_heap, heap);
#else
	_memory_thread_heap = heap;
#endif
}

//! Default implementation to map more virtual memory
static void*
_memory_map_os(size_t size, size_t* offset);

//! Default implementation to unmap virtual memory
static void
_memory_unmap_os(void* address, size_t size, size_t offset, size_t release);

//! Lookup a memory heap from heap ID
static heap_t*
_memory_heap_lookup(int32_t id) {
	uint32_t list_idx = id % HEAP_ARRAY_SIZE;
	heap_t* heap = (heap_t*)atomic_load_ptr(&_memory_heaps[list_idx]);
	while (heap && (heap->id != id))
		heap = heap->next_heap;
	return heap;
}

#if ENABLE_STATISTICS
#  define _memory_statistics_inc(counter, value) counter += value
#  define _memory_statistics_dec(counter, value) counter -= value
#  define _memory_statistics_add(atomic_counter, value) atomic_add32(atomic_counter, (int32_t)(value))
#  define _memory_statistics_add_peak(atomic_counter, value, peak) do { int32_t _cur_count = atomic_add32(atomic_counter, (int32_t)(value)); if (_cur_count > (peak)) peak = _cur_count; } while (0)
#  define _memory_statistics_sub(atomic_counter, value) atomic_add32(atomic_counter, -(int32_t)(value))
#  define _memory_statistics_inc_alloc(heap, class_idx) do { \
	int32_t alloc_current = atomic_incr32(&heap->size_class_use[class_idx].alloc_current); \
	if (alloc_current > heap->size_class_use[class_idx].alloc_peak) \
		heap->size_class_use[class_idx].alloc_peak = alloc_current; \
	heap->size_class_use[class_idx].alloc_total++; \
} while(0)
#  define _memory_statistics_inc_free(heap, class_idx) do { \
	atomic_decr32(&heap->size_class_use[class_idx].alloc_current); \
	atomic_incr32(&heap->size_class_use[class_idx].free_total); \
} while(0)
#else
#  define _memory_statistics_inc(counter, value) do {} while(0)
#  define _memory_statistics_dec(counter, value) do {} while(0)
#  define _memory_statistics_add(atomic_counter, value) do {} while(0)
#  define _memory_statistics_add_peak(atomic_counter, value, peak) do {} while (0)
#  define _memory_statistics_sub(atomic_counter, value) do {} while(0)
#  define _memory_statistics_inc_alloc(heap, class_idx) do {} while(0)
#  define _memory_statistics_inc_free(heap, class_idx) do {} while(0)
#endif

static void
_memory_heap_cache_insert(heap_t* heap, span_t* span);

//! Map more virtual memory
static void*
_memory_map(size_t size, size_t* offset) {
	assert(!(size % _memory_page_size));
	assert(size >= _memory_page_size);
	_memory_statistics_add_peak(&_mapped_pages, (size >> _memory_page_size_shift), _mapped_pages_peak);
	_memory_statistics_add(&_mapped_total, (size >> _memory_page_size_shift));
	return _memory_config.memory_map(size, offset);
}

//! Unmap virtual memory
static void
_memory_unmap(void* address, size_t size, size_t offset, size_t release) {
	assert(!release || (release >= size));
	assert(!release || (release >= _memory_page_size));
	if (release) {
		assert(!(release % _memory_page_size));
		_memory_statistics_sub(&_mapped_pages, (release >> _memory_page_size_shift));
		_memory_statistics_add(&_unmapped_total, (release >> _memory_page_size_shift));
	}
	_memory_config.memory_unmap(address, size, offset, release);
}

//! Declare the span to be a subspan and store distance from master span and span count
static void
_memory_span_mark_as_subspan_unless_master(span_t* master, span_t* subspan, size_t span_count) {
	assert((subspan != master) || (subspan->flags & SPAN_FLAG_MASTER));
	if (subspan != master) {
		subspan->flags = SPAN_FLAG_SUBSPAN;
		subspan->total_spans_or_distance = (uint32_t)((uintptr_t)pointer_diff(subspan, master) >> _memory_span_size_shift);
		subspan->align_offset = 0;
	}
	subspan->span_count = (uint32_t)span_count;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be checked by caller)
static span_t*
_memory_map_from_reserve(heap_t* heap, size_t span_count) {
	//Update the heap span reserve
	span_t* span = heap->span_reserve;
	heap->span_reserve = (span_t*)pointer_offset(span, span_count * _memory_span_size);
	heap->spans_reserved -= span_count;

	_memory_span_mark_as_subspan_unless_master(heap->span_reserve_master, span, span_count);
	if (span_count <= LARGE_CLASS_COUNT)
		_memory_statistics_inc(heap->span_use[span_count - 1].spans_from_reserved, 1);

	return span;
}

//! Get the aligned number of spans to map in based on wanted count, configured mapping granularity and the page size
static size_t
_memory_map_align_span_count(size_t span_count) {
	size_t request_count = (span_count > _memory_span_map_count) ? span_count : _memory_span_map_count;
	if ((_memory_page_size > _memory_span_size) && ((request_count * _memory_span_size) % _memory_page_size))
		request_count += _memory_span_map_count - (request_count % _memory_span_map_count);
	return request_count;
}

//! Store the given spans as reserve in the given heap
static void
_memory_heap_set_reserved_spans(heap_t* heap, span_t* master, span_t* reserve, size_t reserve_span_count) {
	heap->span_reserve_master = master;
	heap->span_reserve = reserve;
	heap->spans_reserved = reserve_span_count;
}

//! Setup a newly mapped span
static void
_memory_span_initialize(span_t* span, size_t total_span_count, size_t span_count, size_t align_offset) {
	span->total_spans_or_distance = (uint32_t)total_span_count;
	span->span_count = (uint32_t)span_count;
	span->align_offset = (uint32_t)align_offset;
	span->flags = SPAN_FLAG_MASTER;
	atomic_store32(&span->remaining_spans, (int32_t)total_span_count);
}

//! Map a akigned set of spans, taking configured mapping granularity and the page size into account
static span_t*
_memory_map_aligned_span_count(heap_t* heap, size_t span_count) {
	//If we already have some, but not enough, reserved spans, release those to heap cache and map a new
	//full set of spans. Otherwise we would waste memory if page size > span size (huge pages)
	size_t aligned_span_count = _memory_map_align_span_count(span_count);
	size_t align_offset = 0;
	span_t* span = (span_t*)_memory_map(aligned_span_count * _memory_span_size, &align_offset);
	if (!span)
		return 0;
	_memory_span_initialize(span, aligned_span_count, span_count, align_offset);
	_memory_statistics_add(&_reserved_spans, aligned_span_count);
	if (span_count <= LARGE_CLASS_COUNT)
		_memory_statistics_inc(heap->span_use[span_count - 1].spans_map_calls, 1);
	if (aligned_span_count > span_count) {
		if (heap->spans_reserved) {
			_memory_span_mark_as_subspan_unless_master(heap->span_reserve_master, heap->span_reserve, heap->spans_reserved);
			_memory_heap_cache_insert(heap, heap->span_reserve);
		}
		_memory_heap_set_reserved_spans(heap, span, (span_t*)pointer_offset(span, span_count * _memory_span_size), aligned_span_count - span_count);
	}
	return span;
}

//! Map in memory pages for the given number of spans (or use previously reserved pages)
static span_t*
_memory_map_spans(heap_t* heap, size_t span_count) {
	if (span_count <= heap->spans_reserved)
		return _memory_map_from_reserve(heap, span_count);
	return _memory_map_aligned_span_count(heap, span_count);
}

//! Unmap memory pages for the given number of spans (or mark as unused if no partial unmappings)
static void
_memory_unmap_span(span_t* span) {
	assert((span->flags & SPAN_FLAG_MASTER) || (span->flags & SPAN_FLAG_SUBSPAN));
	assert(!(span->flags & SPAN_FLAG_MASTER) || !(span->flags & SPAN_FLAG_SUBSPAN));

	int is_master = !!(span->flags & SPAN_FLAG_MASTER);
	span_t* master = is_master ? span : (span_t*)(pointer_offset(span, -(int32_t)(span->total_spans_or_distance * _memory_span_size)));
	assert(is_master || (span->flags & SPAN_FLAG_SUBSPAN));
	assert(master->flags & SPAN_FLAG_MASTER);

	size_t span_count = span->span_count;
	if (!is_master) {
		//Directly unmap subspans (unless huge pages, in which case we defer and unmap entire page range with master)
		assert(span->align_offset == 0);
		if (_memory_span_size >= _memory_page_size) {
			_memory_unmap(span, span_count * _memory_span_size, 0, 0);
			_memory_statistics_sub(&_reserved_spans, span_count);
		}
	} else {
		//Special double flag to denote an unmapped master
		//It must be kept in memory since span header must be used
		span->flags |= SPAN_FLAG_MASTER | SPAN_FLAG_SUBSPAN;
	}

	if (atomic_add32(&master->remaining_spans, -(int32_t)span_count) <= 0) {
		//Everything unmapped, unmap the master span with release flag to unmap the entire range of the super span
		assert(!!(master->flags & SPAN_FLAG_MASTER) && !!(master->flags & SPAN_FLAG_SUBSPAN));
		size_t unmap_count = master->span_count;
		if (_memory_span_size < _memory_page_size)
			unmap_count = master->total_spans_or_distance;
		_memory_statistics_sub(&_reserved_spans, unmap_count);
		_memory_unmap(master, unmap_count * _memory_span_size, master->align_offset, master->total_spans_or_distance * _memory_span_size);
	}
}

#if ENABLE_THREAD_CACHE

//! Unmap a single linked list of spans
static void
_memory_unmap_span_list(span_t* span) {
	size_t list_size = span->list_size;
	for (size_t ispan = 0; ispan < list_size; ++ispan) {
		span_t* next_span = span->next;
		_memory_unmap_span(span);
		span = next_span;
	}
	assert(!span);
}

//! Add span to head of single linked span list
static size_t
_memory_span_list_push(span_t** head, span_t* span) {
	span->next = *head;
	if (*head)
		span->list_size = (*head)->list_size + 1;
	else
		span->list_size = 1;
	*head = span;
	return span->list_size;
}

//! Remove span from head of single linked span list, returns the new list head
static span_t*
_memory_span_list_pop(span_t** head) {
	span_t* span = *head;
	span_t* next_span = 0;
	if (span->list_size > 1) {
		assert(span->next);
		next_span = span->next;
		assert(next_span);
		next_span->list_size = span->list_size - 1;
	}
	*head = next_span;
	return span;
}

//! Split a single linked span list
static span_t*
_memory_span_list_split(span_t* span, size_t limit) {
	span_t* next = 0;
	if (limit < 2)
		limit = 2;
	if (span->list_size > limit) {
		uint32_t list_size = 1;
		span_t* last = span;
		next = span->next;
		while (list_size < limit) {
			last = next;
			next = next->next;
			++list_size;
		}
		last->next = 0;
		assert(next);
		next->list_size = span->list_size - list_size;
		span->list_size = list_size;
		span->prev = 0;
	}
	return next;
}

#endif

//! Add a span to partial span double linked list at the head
static void
_memory_span_partial_list_add(span_t** head, span_t* span) {
	if (*head) {
		span->next = *head;
		//Maintain pointer to tail span
		span->prev = (*head)->prev;
		(*head)->prev = span;
	} else {
		span->next = 0;
		span->prev = span;
	}
	*head = span;
}

//! Add a span to partial span double linked list at the tail
static void
_memory_span_partial_list_add_tail(span_t** head, span_t* span) {
	span->next = 0;
	if (*head) {
		span_t* tail = (*head)->prev;
		tail->next = span;
		span->prev = tail;
		//Maintain pointer to tail span
		(*head)->prev = span;
	} else {
		span->prev = span;
		*head = span;
	}
}

//! Pop head span from partial span double linked list
static void
_memory_span_partial_list_pop_head(span_t** head) {
	span_t* span = *head;
	*head = span->next;
	if (*head) {
		//Maintain pointer to tail span
		(*head)->prev = span->prev;
	}
}

//! Remove a span from partial span double linked list
static void
_memory_span_partial_list_remove(span_t** head, span_t* span) {
	if (UNEXPECTED(*head == span)) {
		_memory_span_partial_list_pop_head(head);
	} else {
		span_t* next_span = span->next;
		span_t* prev_span = span->prev;
		prev_span->next = next_span;
		if (EXPECTED(next_span != 0)) {
			next_span->prev = prev_span;
		} else {
			//Update pointer to tail span
			(*head)->prev = prev_span;
		}
	}
}

#if ENABLE_GLOBAL_CACHE

//! Insert the given list of memory page spans in the global cache
static void
_memory_cache_insert(global_cache_t* cache, span_t* span, size_t cache_limit) {
	assert((span->list_size == 1) || (span->next != 0));
	int32_t list_size = (int32_t)span->list_size;
	//Unmap if cache has reached the limit
	if (atomic_add32(&cache->size, list_size) > (int32_t)cache_limit) {
#if !ENABLE_UNLIMITED_GLOBAL_CACHE
		_memory_unmap_span_list(span);
		atomic_add32(&cache->size, -list_size);
		return;
#endif
	}
	void* current_cache, *new_cache;
	do {
		current_cache = atomic_load_ptr(&cache->cache);
		span->prev = (span_t*)((uintptr_t)current_cache & _memory_span_mask);
		new_cache = (void*)((uintptr_t)span | ((uintptr_t)atomic_incr32(&cache->counter) & ~_memory_span_mask));
	} while (!atomic_cas_ptr(&cache->cache, new_cache, current_cache));
}

//! Extract a number of memory page spans from the global cache
static span_t*
_memory_cache_extract(global_cache_t* cache) {
	uintptr_t span_ptr;
	do {
		void* global_span = atomic_load_ptr(&cache->cache);
		span_ptr = (uintptr_t)global_span & _memory_span_mask;
		if (span_ptr) {
			span_t* span = (span_t*)span_ptr;
			//By accessing the span ptr before it is swapped out of list we assume that a contending thread
			//does not manage to traverse the span to being unmapped before we access it
			void* new_cache = (void*)((uintptr_t)span->prev | ((uintptr_t)atomic_incr32(&cache->counter) & ~_memory_span_mask));
			if (atomic_cas_ptr(&cache->cache, new_cache, global_span)) {
				atomic_add32(&cache->size, -(int32_t)span->list_size);
				return span;
			}
		}
	} while (span_ptr);
	return 0;
}

//! Finalize a global cache, only valid from allocator finalization (not thread safe)
static void
_memory_cache_finalize(global_cache_t* cache) {
	void* current_cache = atomic_load_ptr(&cache->cache);
	span_t* span = (span_t*)((uintptr_t)current_cache & _memory_span_mask);
	while (span) {
		span_t* skip_span = (span_t*)((uintptr_t)span->prev & _memory_span_mask);
		atomic_add32(&cache->size, -(int32_t)span->list_size);
		_memory_unmap_span_list(span);
		span = skip_span;
	}
	assert(!atomic_load32(&cache->size));
	atomic_store_ptr(&cache->cache, 0);
	atomic_store32(&cache->size, 0);
}

//! Insert the given list of memory page spans in the global cache
static void
_memory_global_cache_insert(span_t* span) {
	size_t span_count = span->span_count;
#if ENABLE_UNLIMITED_GLOBAL_CACHE
	_memory_cache_insert(&_memory_span_cache[span_count - 1], span, 0);
#else
	const size_t cache_limit = (GLOBAL_CACHE_MULTIPLIER * ((span_count == 1) ? _memory_span_release_count : _memory_span_release_count_large));
	_memory_cache_insert(&_memory_span_cache[span_count - 1], span, cache_limit);
#endif
}

//! Extract a number of memory page spans from the global cache for large blocks
static span_t*
_memory_global_cache_extract(size_t span_count) {
	span_t* span = _memory_cache_extract(&_memory_span_cache[span_count - 1]);
	assert(!span || (span->span_count == span_count));
	return span;
}

#endif

#if ENABLE_THREAD_CACHE
//! Adopt the deferred span cache list
static void
_memory_heap_cache_adopt_deferred(heap_t* heap) {
	atomic_thread_fence_acquire();
	span_t* span = (span_t*)atomic_load_ptr(&heap->span_cache_deferred);
	if (!span)
		return;
	do {
		span = (span_t*)atomic_load_ptr(&heap->span_cache_deferred);
	} while (!atomic_cas_ptr(&heap->span_cache_deferred, 0, span));
	while (span) {
		span_t* next_span = span->next;
		_memory_span_list_push(&heap->span_cache[0], span);
#if ENABLE_STATISTICS
		atomic_decr32(&heap->span_use[span->span_count - 1].current);
		++heap->size_class_use[span->size_class].spans_to_cache;
		--heap->size_class_use[span->size_class].spans_current;
#endif
		span = next_span;
	}
}
#endif

//! Insert a single span into thread heap cache, releasing to global cache if overflow
static void
_memory_heap_cache_insert(heap_t* heap, span_t* span) {
#if ENABLE_THREAD_CACHE
	size_t span_count = span->span_count;
	size_t idx = span_count - 1;
	_memory_statistics_inc(heap->span_use[idx].spans_to_cache, 1);
	if (!idx)
		_memory_heap_cache_adopt_deferred(heap);
#if ENABLE_UNLIMITED_THREAD_CACHE
	_memory_span_list_push(&heap->span_cache[idx], span);
#else
	const size_t release_count = (!idx ? _memory_span_release_count : _memory_span_release_count_large);
	size_t current_cache_size = _memory_span_list_push(&heap->span_cache[idx], span);
	if (current_cache_size <= release_count)
		return;
	const size_t hard_limit = release_count * THREAD_CACHE_MULTIPLIER;
	if (current_cache_size <= hard_limit) {
#if ENABLE_ADAPTIVE_THREAD_CACHE
		//Require 25% of high water mark to remain in cache (and at least 1, if use is 0)
		const size_t high_mark = heap->span_use[idx].high;
		const size_t min_limit = (high_mark >> 2) + release_count + 1;
		if (current_cache_size < min_limit)
			return;
#else
		return;
#endif
	}
	heap->span_cache[idx] = _memory_span_list_split(span, release_count);
	assert(span->list_size == release_count);
#if ENABLE_STATISTICS
	heap->thread_to_global += (size_t)span->list_size * span_count * _memory_span_size;
	heap->span_use[idx].spans_to_global += span->list_size;
#endif
#if ENABLE_GLOBAL_CACHE
	_memory_global_cache_insert(span);
#else
	_memory_unmap_span_list(span);
#endif
#endif
#else
	(void)sizeof(heap);
	_memory_unmap_span(span);
#endif
}

//! Extract the given number of spans from the different cache levels
static span_t*
_memory_heap_thread_cache_extract(heap_t* heap, size_t span_count) {
#if ENABLE_THREAD_CACHE
	size_t idx = span_count - 1;
	if (!idx)
		_memory_heap_cache_adopt_deferred(heap);
	if (heap->span_cache[idx]) {
#if ENABLE_STATISTICS
		heap->span_use[idx].spans_from_cache++;
#endif
		return _memory_span_list_pop(&heap->span_cache[idx]);
	}
#endif
	return 0;
}

static span_t*
_memory_heap_reserved_extract(heap_t* heap, size_t span_count) {
	if (heap->spans_reserved >= span_count)
		return _memory_map_spans(heap, span_count);
	return 0;
}

//! Extract a span from the global cache
static span_t*
_memory_heap_global_cache_extract(heap_t* heap, size_t span_count) {
#if ENABLE_GLOBAL_CACHE
	size_t idx = span_count - 1;
	heap->span_cache[idx] = _memory_global_cache_extract(span_count);
	if (heap->span_cache[idx]) {
#if ENABLE_STATISTICS
		heap->global_to_thread += (size_t)heap->span_cache[idx]->list_size * span_count * _memory_span_size;
		heap->span_use[idx].spans_from_global += heap->span_cache[idx]->list_size;
#endif
		return _memory_span_list_pop(&heap->span_cache[idx]);
	}
#endif
	return 0;
}

//! Get a span from one of the cache levels (thread cache, reserved, global cache) or fallback to mapping more memory
static span_t*
_memory_heap_extract_new_span(heap_t* heap, size_t span_count, uint32_t class_idx) {
	(void)sizeof(class_idx);
#if ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
	uint32_t idx = (uint32_t)span_count - 1;
	uint32_t current_count = (uint32_t)atomic_incr32(&heap->span_use[idx].current);
	if (current_count > heap->span_use[idx].high)
		heap->span_use[idx].high = current_count;
#if ENABLE_STATISTICS
	uint32_t spans_current = ++heap->size_class_use[class_idx].spans_current;
	if (spans_current > heap->size_class_use[class_idx].spans_peak)
		heap->size_class_use[class_idx].spans_peak = spans_current;
#endif
#endif
	span_t* span = _memory_heap_thread_cache_extract(heap, span_count);
	if (EXPECTED(span != 0)) {
		_memory_statistics_inc(heap->size_class_use[class_idx].spans_from_cache, 1);
		return span;
	}
	span = _memory_heap_reserved_extract(heap, span_count);
	if (EXPECTED(span != 0)) {
		_memory_statistics_inc(heap->size_class_use[class_idx].spans_from_reserved, 1);
		return span;
	}
	span = _memory_heap_global_cache_extract(heap, span_count);
	if (EXPECTED(span != 0)) {
		_memory_statistics_inc(heap->size_class_use[class_idx].spans_from_cache, 1);
		return span;
	}
	//Final fallback, map in more virtual memory
	span = _memory_map_spans(heap, span_count);
	_memory_statistics_inc(heap->size_class_use[class_idx].spans_map_calls, 1);
	return span;
}

//! Move the span (used for small or medium allocations) to the heap thread cache
static void
_memory_span_release_to_cache(heap_t* heap, span_t* span) {
	heap_class_t* heap_class = heap->span_class + span->size_class;
	assert(heap_class->partial_span != span);
	if (span->state == SPAN_STATE_PARTIAL)
		_memory_span_partial_list_remove(&heap_class->partial_span, span);
#if ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
	atomic_decr32(&heap->span_use[0].current);
#endif
	_memory_statistics_inc(heap->span_use[0].spans_to_cache, 1);
	_memory_statistics_inc(heap->size_class_use[span->size_class].spans_to_cache, 1);
	_memory_statistics_dec(heap->size_class_use[span->size_class].spans_current, 1);
	_memory_heap_cache_insert(heap, span);
}

//! Initialize a (partial) free list up to next system memory page, while reserving the first block
//! as allocated, returning number of blocks in list
static uint32_t
free_list_partial_init(void** list, void** first_block, void* page_start, void* block_start,
                       uint32_t block_count, uint32_t block_size) {
	assert(block_count);
	*first_block = block_start;
	if (block_count > 1) {
		void* free_block = pointer_offset(block_start, block_size);
		void* block_end = pointer_offset(block_start, block_size * block_count);
		//If block size is less than half a memory page, bound init to next memory page boundary
		if (block_size < (_memory_page_size >> 1)) {
			void* page_end = pointer_offset(page_start, _memory_page_size);
			if (page_end < block_end)
				block_end = page_end;
		}
		*list = free_block;
		block_count = 2;
		void* next_block = pointer_offset(free_block, block_size);
		while (next_block < block_end) {
			*((void**)free_block) = next_block;
			free_block = next_block;
			++block_count;
			next_block = pointer_offset(next_block, block_size);
		}
		*((void**)free_block) = 0;
	} else {
		*list = 0;
	}
	return block_count;
}

//! Initialize an unused span (from cache or mapped) to be new active span
static void*
_memory_span_set_new_active(heap_t* heap, heap_class_t* heap_class, span_t* span, uint32_t class_idx) {
	assert(span->span_count == 1);
	size_class_t* size_class = _memory_size_class + class_idx;
	span->size_class = class_idx;
	span->heap = heap;
	span->flags &= ~SPAN_FLAG_ALIGNED_BLOCKS;
	span->block_count = size_class->block_count;
	span->block_size = size_class->block_size;
	span->state = SPAN_STATE_ACTIVE;
	span->free_list = 0;

	//Setup free list. Only initialize one system page worth of free blocks in list
	void* block;
	span->free_list_limit = free_list_partial_init(&heap_class->free_list, &block,
		span, pointer_offset(span, SPAN_HEADER_SIZE), size_class->block_count, size_class->block_size);
	atomic_store_ptr(&span->free_list_deferred, 0);
	span->list_size = 0;
	atomic_thread_fence_release();

	_memory_span_partial_list_add(&heap_class->partial_span, span);
	return block;
}

//! Promote a partially used span (from heap used list) to be new active span
static void
_memory_span_set_partial_active(heap_class_t* heap_class, span_t* span) {
	assert(span->state == SPAN_STATE_PARTIAL);
	assert(span->block_count == _memory_size_class[span->size_class].block_count);
	//Move data to heap size class and set span as active
	heap_class->free_list = span->free_list;
	span->state = SPAN_STATE_ACTIVE;
	span->free_list = 0;
	assert(heap_class->free_list);
}

//! Mark span as full (from active)
static void
_memory_span_set_active_full(heap_class_t* heap_class, span_t* span) {
	assert(span->state == SPAN_STATE_ACTIVE);
	assert(span == heap_class->partial_span);
	_memory_span_partial_list_pop_head(&heap_class->partial_span);
	span->used_count = span->block_count;
	span->state = SPAN_STATE_FULL;
	span->free_list = 0;
}

//! Move span from full to partial state
static void
_memory_span_set_full_partial(heap_t* heap, span_t* span) {
	assert(span->state == SPAN_STATE_FULL);
	heap_class_t* heap_class = &heap->span_class[span->size_class];
	span->state = SPAN_STATE_PARTIAL;
	_memory_span_partial_list_add_tail(&heap_class->partial_span, span);
}

static void*
_memory_span_extract_deferred(span_t* span) {
	void* free_list;
	do {
		free_list = atomic_load_ptr(&span->free_list_deferred);
	} while ((free_list == INVALID_POINTER) || !atomic_cas_ptr(&span->free_list_deferred, INVALID_POINTER, free_list));
	span->list_size = 0;
	atomic_store_ptr(&span->free_list_deferred, 0);
	atomic_thread_fence_release();
	return free_list;
}

//! Pop first block from a free list
static void*
free_list_pop(void** list) {
	void* block = *list;
	*list = *((void**)block);
	return block;
}

//! Allocate a small/medium sized memory block from the given heap
static void*
_memory_allocate_from_heap_fallback(heap_t* heap, uint32_t class_idx) {
	heap_class_t* heap_class = &heap->span_class[class_idx];
	void* block;

	span_t* active_span = heap_class->partial_span;
	if (EXPECTED(active_span != 0)) {
		assert(active_span->state == SPAN_STATE_ACTIVE);
		assert(active_span->block_count == _memory_size_class[active_span->size_class].block_count);
		//Swap in free list if not empty
		if (active_span->free_list) {
			heap_class->free_list = active_span->free_list;
			active_span->free_list = 0;
			return free_list_pop(&heap_class->free_list);
		}
		//If the span did not fully initialize free list, link up another page worth of blocks
		if (active_span->free_list_limit < active_span->block_count) {
			void* block_start = pointer_offset(active_span, SPAN_HEADER_SIZE + (active_span->free_list_limit * active_span->block_size));
			active_span->free_list_limit += free_list_partial_init(&heap_class->free_list, &block,
				(void*)((uintptr_t)block_start & ~(_memory_page_size - 1)), block_start,
				active_span->block_count - active_span->free_list_limit, active_span->block_size);
			return block;
		}
		//Swap in deferred free list
		atomic_thread_fence_acquire();
		if (atomic_load_ptr(&active_span->free_list_deferred)) {
			heap_class->free_list = _memory_span_extract_deferred(active_span);
			return free_list_pop(&heap_class->free_list);
		}

		//If the active span is fully allocated, mark span as free floating (fully allocated and not part of any list)
		assert(!heap_class->free_list);
		assert(active_span->free_list_limit >= active_span->block_count);
		_memory_span_set_active_full(heap_class, active_span);
	}
	assert(!heap_class->free_list);

	//Try promoting a semi-used span to active
	active_span = heap_class->partial_span;
	if (EXPECTED(active_span != 0)) {
		_memory_span_set_partial_active(heap_class, active_span);
		return free_list_pop(&heap_class->free_list);
	}
	assert(!heap_class->free_list);
	assert(!heap_class->partial_span);

	//Find a span in one of the cache levels
	active_span = _memory_heap_extract_new_span(heap, 1, class_idx);

	//Mark span as owned by this heap and set base data, return first block
	return _memory_span_set_new_active(heap, heap_class, active_span, class_idx);
}

//! Allocate a small sized memory block from the given heap
static void*
_memory_allocate_small(heap_t* heap, size_t size) {
	//Small sizes have unique size classes
	const uint32_t class_idx = (uint32_t)((size + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_SHIFT);
	_memory_statistics_inc_alloc(heap, class_idx);
	if (EXPECTED(heap->span_class[class_idx].free_list != 0))
		return free_list_pop(&heap->span_class[class_idx].free_list);
	return _memory_allocate_from_heap_fallback(heap, class_idx);
}

//! Allocate a medium sized memory block from the given heap
static void*
_memory_allocate_medium(heap_t* heap, size_t size) {
	//Calculate the size class index and do a dependent lookup of the final class index (in case of merged classes)
	const uint32_t base_idx = (uint32_t)(SMALL_CLASS_COUNT + ((size - (SMALL_SIZE_LIMIT + 1)) >> MEDIUM_GRANULARITY_SHIFT));
	const uint32_t class_idx = _memory_size_class[base_idx].class_idx;
	_memory_statistics_inc_alloc(heap, class_idx);
	if (EXPECTED(heap->span_class[class_idx].free_list != 0))
		return free_list_pop(&heap->span_class[class_idx].free_list);
	return _memory_allocate_from_heap_fallback(heap, class_idx);
}

//! Allocate a large sized memory block from the given heap
static void*
_memory_allocate_large(heap_t* heap, size_t size) {
	//Calculate number of needed max sized spans (including header)
	//Since this function is never called if size > LARGE_SIZE_LIMIT
	//the span_count is guaranteed to be <= LARGE_CLASS_COUNT
	size += SPAN_HEADER_SIZE;
	size_t span_count = size >> _memory_span_size_shift;
	if (size & (_memory_span_size - 1))
		++span_count;
	size_t idx = span_count - 1;

	//Find a span in one of the cache levels
	span_t* span = _memory_heap_extract_new_span(heap, span_count, SIZE_CLASS_COUNT);

	//Mark span as owned by this heap and set base data
	assert(span->span_count == span_count);
	span->size_class = (uint32_t)(SIZE_CLASS_COUNT + idx);
	span->heap = heap;
	atomic_thread_fence_release();

	return pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a huge block by mapping memory pages directly
static void*
_memory_allocate_huge(size_t size) {
	size += SPAN_HEADER_SIZE;
	size_t num_pages = size >> _memory_page_size_shift;
	if (size & (_memory_page_size - 1))
		++num_pages;
	size_t align_offset = 0;
	span_t* span = (span_t*)_memory_map(num_pages * _memory_page_size, &align_offset);
	if (!span)
		return span;
	//Store page count in span_count
	span->size_class = (uint32_t)-1;
	span->span_count = (uint32_t)num_pages;
	span->align_offset = (uint32_t)align_offset;
	_memory_statistics_add_peak(&_huge_pages_current, num_pages, _huge_pages_peak);

	return pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a block larger than medium size
static void*
_memory_allocate_oversized(heap_t* heap, size_t size) {
	if (size <= LARGE_SIZE_LIMIT)
		return _memory_allocate_large(heap, size);
	return _memory_allocate_huge(size);
}

//! Allocate a block of the given size
static void*
_memory_allocate(heap_t* heap, size_t size) {
	if (EXPECTED(size <= SMALL_SIZE_LIMIT))
		return _memory_allocate_small(heap, size);
	else if (size <= _memory_medium_size_limit)
		return _memory_allocate_medium(heap, size);
	return _memory_allocate_oversized(heap, size);
}

//! Allocate a new heap
static heap_t*
_memory_allocate_heap(void) {
	void* raw_heap;
	void* next_raw_heap;
	uintptr_t orphan_counter;
	heap_t* heap;
	heap_t* next_heap;
	//Try getting an orphaned heap
	atomic_thread_fence_acquire();
	do {
		raw_heap = atomic_load_ptr(&_memory_orphan_heaps);
		heap = (heap_t*)((uintptr_t)raw_heap & ~(uintptr_t)0x1FF);
		if (!heap)
			break;
		next_heap = heap->next_orphan;
		orphan_counter = (uintptr_t)atomic_incr32(&_memory_orphan_counter);
		next_raw_heap = (void*)((uintptr_t)next_heap | (orphan_counter & (uintptr_t)0x1FF));
	} while (!atomic_cas_ptr(&_memory_orphan_heaps, next_raw_heap, raw_heap));

	if (!heap) {
		//Map in pages for a new heap
		size_t align_offset = 0;
		heap = (heap_t*)_memory_map((1 + (sizeof(heap_t) >> _memory_page_size_shift)) * _memory_page_size, &align_offset);
		if (!heap)
			return heap;
		memset((char*)heap, 0, sizeof(heap_t));
		heap->align_offset = align_offset;

		//Get a new heap ID
		do {
			heap->id = atomic_incr32(&_memory_heap_id);
			if (_memory_heap_lookup(heap->id))
				heap->id = 0;
		} while (!heap->id);

		//Link in heap in heap ID map
		size_t list_idx = heap->id % HEAP_ARRAY_SIZE;
		do {
			next_heap = (heap_t*)atomic_load_ptr(&_memory_heaps[list_idx]);
			heap->next_heap = next_heap;
		} while (!atomic_cas_ptr(&_memory_heaps[list_idx], heap, next_heap));
	}

	return heap;
}

//! Deallocate the given small/medium memory block in the current thread local heap
static void
_memory_deallocate_direct(span_t* span, void* block) {
	assert(span->heap == get_thread_heap_raw());
	uint32_t state = span->state;
	//Add block to free list
	*((void**)block) = span->free_list;
	span->free_list = block;
	if (UNEXPECTED(state == SPAN_STATE_ACTIVE))
		return;
	uint32_t used = --span->used_count;
	uint32_t free = span->list_size;
	if (UNEXPECTED(used == free))
		_memory_span_release_to_cache(span->heap, span);
	else if (UNEXPECTED(state == SPAN_STATE_FULL))
		_memory_span_set_full_partial(span->heap, span);
}

//! Put the block in the deferred free list of the owning span
static void
_memory_deallocate_defer(span_t* span, void* block) {
	atomic_thread_fence_acquire();
	if (span->state == SPAN_STATE_FULL) {
		if ((span->list_size + 1) == span->block_count) {
			//Span will be completely freed by deferred deallocations, no other thread can
			//currently touch it. Safe to move to owner heap deferred cache
			span_t* last_head;
			heap_t* heap = span->heap;
			do {
				last_head = (span_t*)atomic_load_ptr(&heap->span_cache_deferred);
				span->next = last_head;
			} while (!atomic_cas_ptr(&heap->span_cache_deferred, span, last_head));
			return;
		}
	}

	void* free_list;
	do {
		atomic_thread_fence_acquire();
		free_list = atomic_load_ptr(&span->free_list_deferred);
		*((void**)block) = free_list;
	} while ((free_list == INVALID_POINTER) || !atomic_cas_ptr(&span->free_list_deferred, INVALID_POINTER, free_list));
	++span->list_size;
	atomic_store_ptr(&span->free_list_deferred, block);
}

static void
_memory_deallocate_small_or_medium(span_t* span, void* p) {
	_memory_statistics_inc_free(span->heap, span->size_class);
	if (span->flags & SPAN_FLAG_ALIGNED_BLOCKS) {
		//Realign pointer to block start
		void* blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
		uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
		p = pointer_offset(p, -(int32_t)(block_offset % span->block_size));
	}
	//Check if block belongs to this heap or if deallocation should be deferred
	if (span->heap == get_thread_heap_raw())
		_memory_deallocate_direct(span, p);
	else
		_memory_deallocate_defer(span, p);
}

//! Deallocate the given large memory block to the current heap
static void
_memory_deallocate_large(span_t* span) {
	//Decrease counter
	assert(span->span_count == ((size_t)span->size_class - SIZE_CLASS_COUNT + 1));
	assert(span->size_class >= SIZE_CLASS_COUNT);
	assert(span->size_class - SIZE_CLASS_COUNT < LARGE_CLASS_COUNT);
	assert(!(span->flags & SPAN_FLAG_MASTER) || !(span->flags & SPAN_FLAG_SUBSPAN));
	assert((span->flags & SPAN_FLAG_MASTER) || (span->flags & SPAN_FLAG_SUBSPAN));
	//Large blocks can always be deallocated and transferred between heaps
	//Investigate if it is better to defer large spans as well through span_cache_deferred,
	//possibly with some heuristics to pick either scheme at runtime per deallocation
	heap_t* heap = get_thread_heap();
	if (!heap) return;
#if ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
	size_t idx = span->span_count - 1;
	atomic_decr32(&span->heap->span_use[idx].current);
#endif
	if ((span->span_count > 1) && !heap->spans_reserved) {
		heap->span_reserve = span;
		heap->spans_reserved = span->span_count;
		if (span->flags & SPAN_FLAG_MASTER) {
			heap->span_reserve_master = span;
		} else { //SPAN_FLAG_SUBSPAN
			uint32_t distance = span->total_spans_or_distance;
			span_t* master = (span_t*)pointer_offset(span, -(int32_t)(distance * _memory_span_size));
			heap->span_reserve_master = master;
			assert(master->flags & SPAN_FLAG_MASTER);
			assert(atomic_load32(&master->remaining_spans) >= (int32_t)span->span_count);
		}
		_memory_statistics_inc(heap->span_use[idx].spans_to_reserved, 1);
	} else {
		//Insert into cache list
		_memory_heap_cache_insert(heap, span);
	}
}

//! Deallocate the given huge span
static void
_memory_deallocate_huge(span_t* span) {
	//Oversized allocation, page count is stored in span_count
	size_t num_pages = span->span_count;
	_memory_unmap(span, num_pages * _memory_page_size, span->align_offset, num_pages * _memory_page_size);
	_memory_statistics_sub(&_huge_pages_current, num_pages);
}

//! Deallocate the given block
static void
_memory_deallocate(void* p) {
	//Grab the span (always at start of span, using span alignment)
	span_t* span = (span_t*)((uintptr_t)p & _memory_span_mask);
	if (UNEXPECTED(!span))
		return;
	if (EXPECTED(span->size_class < SIZE_CLASS_COUNT))
		_memory_deallocate_small_or_medium(span, p);
	else if (span->size_class != (uint32_t)-1)
		_memory_deallocate_large(span);
	else
		_memory_deallocate_huge(span);
}

//! Reallocate the given block to the given size
static void*
_memory_reallocate(void* p, size_t size, size_t oldsize, unsigned int flags) {
	if (p) {
		//Grab the span using guaranteed span alignment
		span_t* span = (span_t*)((uintptr_t)p & _memory_span_mask);
		if (span->heap) {
			if (span->size_class < SIZE_CLASS_COUNT) {
				//Small/medium sized block
				assert(span->span_count == 1);
				void* blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
				uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
				uint32_t block_idx = block_offset / span->block_size;
				void* block = pointer_offset(blocks_start, block_idx * span->block_size);
				if (!oldsize)
					oldsize = span->block_size - (uint32_t)pointer_diff(p, block);
				if ((size_t)span->block_size >= size) {
					//Still fits in block, never mind trying to save memory, but preserve data if alignment changed
					if ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
						memmove(block, p, oldsize);
					return block;
				}
			} else {
				//Large block
				size_t total_size = size + SPAN_HEADER_SIZE;
				size_t num_spans = total_size >> _memory_span_size_shift;
				if (total_size & (_memory_span_mask - 1))
					++num_spans;
				size_t current_spans = span->span_count;
				assert(current_spans == ((span->size_class - SIZE_CLASS_COUNT) + 1));
				void* block = pointer_offset(span, SPAN_HEADER_SIZE);
				if (!oldsize)
					oldsize = (current_spans * _memory_span_size) - (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
				if ((current_spans >= num_spans) && (num_spans >= (current_spans / 2))) {
					//Still fits in block, never mind trying to save memory, but preserve data if alignment changed
					if ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
						memmove(block, p, oldsize);
					return block;
				}
			}
		} else {
			//Oversized block
			size_t total_size = size + SPAN_HEADER_SIZE;
			size_t num_pages = total_size >> _memory_page_size_shift;
			if (total_size & (_memory_page_size - 1))
				++num_pages;
			//Page count is stored in span_count
			size_t current_pages = span->span_count;
			void* block = pointer_offset(span, SPAN_HEADER_SIZE);
			if (!oldsize)
				oldsize = (current_pages * _memory_page_size) - (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
			if ((current_pages >= num_pages) && (num_pages >= (current_pages / 2))) {
				//Still fits in block, never mind trying to save memory, but preserve data if alignment changed
				if ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
					memmove(block, p, oldsize);
				return block;
			}
		}
	} else {
		oldsize = 0;
	}

	//Size is greater than block size, need to allocate a new block and deallocate the old
	heap_t* heap = get_thread_heap();
	//Avoid hysteresis by overallocating if increase is small (below 37%)
	size_t lower_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
	size_t new_size = (size > lower_bound) ? size : ((size > oldsize) ? lower_bound : size);
	void* block = _memory_allocate(heap, new_size);
	if (p && block) {
		if (!(flags & RPMALLOC_NO_PRESERVE))
			memcpy(block, p, oldsize < new_size ? oldsize : new_size);
		_memory_deallocate(p);
	}

	return block;
}

//! Get the usable size of the given block
static size_t
_memory_usable_size(void* p) {
	//Grab the span using guaranteed span alignment
	span_t* span = (span_t*)((uintptr_t)p & _memory_span_mask);
	if (span->heap) {
		//Small/medium block
		if (span->size_class < SIZE_CLASS_COUNT) {
			void* blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
			return span->block_size - ((size_t)pointer_diff(p, blocks_start) % span->block_size);
		}

		//Large block
		size_t current_spans = (span->size_class - SIZE_CLASS_COUNT) + 1;
		return (current_spans * _memory_span_size) - (size_t)pointer_diff(p, span);
	}

	//Oversized block, page count is stored in span_count
	size_t current_pages = span->span_count;
	return (current_pages * _memory_page_size) - (size_t)pointer_diff(p, span);
}

//! Adjust and optimize the size class properties for the given class
static void
_memory_adjust_size_class(size_t iclass) {
	size_t block_size = _memory_size_class[iclass].block_size;
	size_t block_count = (_memory_span_size - SPAN_HEADER_SIZE) / block_size;

	_memory_size_class[iclass].block_count = (uint16_t)block_count;
	_memory_size_class[iclass].class_idx = (uint16_t)iclass;

	//Check if previous size classes can be merged
	size_t prevclass = iclass;
	while (prevclass > 0) {
		--prevclass;
		//A class can be merged if number of pages and number of blocks are equal
		if (_memory_size_class[prevclass].block_count == _memory_size_class[iclass].block_count)
			memcpy(_memory_size_class + prevclass, _memory_size_class + iclass, sizeof(_memory_size_class[iclass]));
		else
			break;
	}
}

static void
_memory_heap_finalize(void* heapptr) {
	heap_t* heap = (heap_t*)heapptr;
	if (!heap)
		return;
	//Release thread cache spans back to global cache
#if ENABLE_THREAD_CACHE
	_memory_heap_cache_adopt_deferred(heap);
	for (size_t iclass = 0; iclass < LARGE_CLASS_COUNT; ++iclass) {
		span_t* span = heap->span_cache[iclass];
#if ENABLE_GLOBAL_CACHE
		while (span) {
			assert(span->span_count == (iclass + 1));
			size_t release_count = (!iclass ? _memory_span_release_count : _memory_span_release_count_large);
			span_t* next = _memory_span_list_split(span, (uint32_t)release_count);
#if ENABLE_STATISTICS
			heap->thread_to_global += (size_t)span->list_size * span->span_count * _memory_span_size;
			heap->span_use[iclass].spans_to_global += span->list_size;
#endif
			_memory_global_cache_insert(span);
			span = next;
		}
#else
		if (span)
			_memory_unmap_span_list(span);
#endif
		heap->span_cache[iclass] = 0;
	}
#endif

	//Orphan the heap
	void* raw_heap;
	uintptr_t orphan_counter;
	heap_t* last_heap;
	do {
		last_heap = (heap_t*)atomic_load_ptr(&_memory_orphan_heaps);
		heap->next_orphan = (heap_t*)((uintptr_t)last_heap & ~(uintptr_t)0x1FF);
		orphan_counter = (uintptr_t)atomic_incr32(&_memory_orphan_counter);
		raw_heap = (void*)((uintptr_t)heap | (orphan_counter & (uintptr_t)0x1FF));
	} while (!atomic_cas_ptr(&_memory_orphan_heaps, raw_heap, last_heap));

	set_thread_heap(0);

#if ENABLE_STATISTICS
	atomic_decr32(&_memory_active_heaps);
	assert(atomic_load32(&_memory_active_heaps) >= 0);
#endif
}

#if defined(_MSC_VER) && !defined(__clang__) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#include <fibersapi.h>
static DWORD fls_key;
static void NTAPI
rp_thread_destructor(void* value) {
	if (value)
		rpmalloc_thread_finalize();
}
#endif

#if PLATFORM_POSIX
#  include <sys/mman.h>
#  include <sched.h>
#  ifdef __FreeBSD__
#    include <sys/sysctl.h>
#    define MAP_HUGETLB MAP_ALIGNED_SUPER
#  endif
#  ifndef MAP_UNINITIALIZED
#    define MAP_UNINITIALIZED 0
#  endif
#endif
#include <errno.h>

//! Initialize the allocator and setup global data
TRACY_API int
rpmalloc_initialize(void) {
	if (_rpmalloc_initialized) {
		rpmalloc_thread_initialize();
		return 0;
	}
	memset(&_memory_config, 0, sizeof(rpmalloc_config_t));
	return rpmalloc_initialize_config(0);
}

int
rpmalloc_initialize_config(const rpmalloc_config_t* config) {
	if (_rpmalloc_initialized) {
		rpmalloc_thread_initialize();
		return 0;
	}
	_rpmalloc_initialized = 1;

	if (config)
		memcpy(&_memory_config, config, sizeof(rpmalloc_config_t));

	if (!_memory_config.memory_map || !_memory_config.memory_unmap) {
		_memory_config.memory_map = _memory_map_os;
		_memory_config.memory_unmap = _memory_unmap_os;
	}

#if RPMALLOC_CONFIGURABLE
	_memory_page_size = _memory_config.page_size;
#else
	_memory_page_size = 0;
#endif
	_memory_huge_pages = 0;
	_memory_map_granularity = _memory_page_size;
	if (!_memory_page_size) {
#if PLATFORM_WINDOWS
		SYSTEM_INFO system_info;
		memset(&system_info, 0, sizeof(system_info));
		GetSystemInfo(&system_info);
		_memory_page_size = system_info.dwPageSize;
		_memory_map_granularity = system_info.dwAllocationGranularity;
		if (config && config->enable_huge_pages) {
			HANDLE token = 0;
			size_t large_page_minimum = GetLargePageMinimum();
			if (large_page_minimum)
				OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
			if (token) {
				LUID luid;
				if (LookupPrivilegeValue(0, SE_LOCK_MEMORY_NAME, &luid)) {
					TOKEN_PRIVILEGES token_privileges;
					memset(&token_privileges, 0, sizeof(token_privileges));
					token_privileges.PrivilegeCount = 1;
					token_privileges.Privileges[0].Luid = luid;
					token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
					if (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
						DWORD err = GetLastError();
						if (err == ERROR_SUCCESS) {
							_memory_huge_pages = 1;
							_memory_page_size = large_page_minimum;
							_memory_map_granularity = large_page_minimum;
						}
					}
				}
				CloseHandle(token);
			}
		}
#else
		_memory_page_size = (size_t)sysconf(_SC_PAGESIZE);
		_memory_map_granularity = _memory_page_size;
		if (config && config->enable_huge_pages) {
#if defined(__linux__)
			size_t huge_page_size = 0;
			FILE* meminfo = fopen("/proc/meminfo", "r");
			if (meminfo) {
				char line[128];
				while (!huge_page_size && fgets(line, sizeof(line) - 1, meminfo)) {
					line[sizeof(line) - 1] = 0;
					if (strstr(line, "Hugepagesize:"))
						huge_page_size = (size_t)strtol(line + 13, 0, 10) * 1024;
				}
				fclose(meminfo);
			}
			if (huge_page_size) {
				_memory_huge_pages = 1;
				_memory_page_size = huge_page_size;
				_memory_map_granularity = huge_page_size;
			}
#elif defined(__FreeBSD__)
			int rc;
			size_t sz = sizeof(rc);

			if (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, NULL, 0) == 0 && rc == 1) {
				_memory_huge_pages = 1;
				_memory_page_size = 2 * 1024 * 1024;
				_memory_map_granularity = _memory_page_size;
			}
#elif defined(__APPLE__)
			_memory_huge_pages = 1;
			_memory_page_size = 2 * 1024 * 1024;
			_memory_map_granularity = _memory_page_size;
#endif
		}
#endif
	} else {
		if (config && config->enable_huge_pages)
			_memory_huge_pages = 1;
	}

	//The ABA counter in heap orphan list is tied to using 512 (bitmask 0x1FF)
	if (_memory_page_size < 512)
		_memory_page_size = 512;
	if (_memory_page_size > (64 * 1024 * 1024))
		_memory_page_size = (64 * 1024 * 1024);
	_memory_page_size_shift = 0;
	size_t page_size_bit = _memory_page_size;
	while (page_size_bit != 1) {
		++_memory_page_size_shift;
		page_size_bit >>= 1;
	}
	_memory_page_size = ((size_t)1 << _memory_page_size_shift);

#if RPMALLOC_CONFIGURABLE
	size_t span_size = _memory_config.span_size;
	if (!span_size)
		span_size = (64 * 1024);
	if (span_size > (256 * 1024))
		span_size = (256 * 1024);
	_memory_span_size = 4096;
	_memory_span_size_shift = 12;
	while (_memory_span_size < span_size) {
		_memory_span_size <<= 1;
		++_memory_span_size_shift;
	}
	_memory_span_mask = ~(uintptr_t)(_memory_span_size - 1);
#endif

	_memory_span_map_count = ( _memory_config.span_map_count ? _memory_config.span_map_count : DEFAULT_SPAN_MAP_COUNT);
	if ((_memory_span_size * _memory_span_map_count) < _memory_page_size)
		_memory_span_map_count = (_memory_page_size / _memory_span_size);
	if ((_memory_page_size >= _memory_span_size) && ((_memory_span_map_count * _memory_span_size) % _memory_page_size))
		_memory_span_map_count = (_memory_page_size / _memory_span_size);

	_memory_config.page_size = _memory_page_size;
	_memory_config.span_size = _memory_span_size;
	_memory_config.span_map_count = _memory_span_map_count;
	_memory_config.enable_huge_pages = _memory_huge_pages;

	_memory_span_release_count = (_memory_span_map_count > 4 ? ((_memory_span_map_count < 64) ? _memory_span_map_count : 64) : 4);
	_memory_span_release_count_large = (_memory_span_release_count > 8 ? (_memory_span_release_count / 4) : 2);

#if (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
	if (pthread_key_create(&_memory_thread_heap, _memory_heap_finalize))
		return -1;
#endif
#if defined(_MSC_VER) && !defined(__clang__) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
    fls_key = FlsAlloc(&rp_thread_destructor);
#endif

	atomic_store32(&_memory_heap_id, 0);
	atomic_store32(&_memory_orphan_counter, 0);
#if ENABLE_STATISTICS
	atomic_store32(&_memory_active_heaps, 0);
	atomic_store32(&_reserved_spans, 0);
	atomic_store32(&_mapped_pages, 0);
	_mapped_pages_peak = 0;
	atomic_store32(&_mapped_total, 0);
	atomic_store32(&_unmapped_total, 0);
	atomic_store32(&_mapped_pages_os, 0);
	atomic_store32(&_huge_pages_current, 0);
	_huge_pages_peak = 0;
#endif

	//Setup all small and medium size classes
	size_t iclass = 0;
	_memory_size_class[iclass].block_size = SMALL_GRANULARITY;
	_memory_adjust_size_class(iclass);
	for (iclass = 1; iclass < SMALL_CLASS_COUNT; ++iclass) {
		size_t size = iclass * SMALL_GRANULARITY;
		_memory_size_class[iclass].block_size = (uint32_t)size;
		_memory_adjust_size_class(iclass);
	}
	//At least two blocks per span, then fall back to large allocations
	_memory_medium_size_limit = (_memory_span_size - SPAN_HEADER_SIZE) >> 1;
	if (_memory_medium_size_limit > MEDIUM_SIZE_LIMIT)
		_memory_medium_size_limit = MEDIUM_SIZE_LIMIT;
	for (iclass = 0; iclass < MEDIUM_CLASS_COUNT; ++iclass) {
		size_t size = SMALL_SIZE_LIMIT + ((iclass + 1) * MEDIUM_GRANULARITY);
		if (size > _memory_medium_size_limit)
			break;
		_memory_size_class[SMALL_CLASS_COUNT + iclass].block_size = (uint32_t)size;
		_memory_adjust_size_class(SMALL_CLASS_COUNT + iclass);
	}

	for (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx)
		atomic_store_ptr(&_memory_heaps[list_idx], 0);

	//Initialize this thread
	rpmalloc_thread_initialize();
	return 0;
}

//! Finalize the allocator
TRACY_API void
rpmalloc_finalize(void) {
	atomic_thread_fence_acquire();

	rpmalloc_thread_finalize();
	//rpmalloc_dump_statistics(stderr);

	//Free all thread caches
	for (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
		heap_t* heap = (heap_t*)atomic_load_ptr(&_memory_heaps[list_idx]);
		while (heap) {
			if (heap->spans_reserved) {
				span_t* span = _memory_map_spans(heap, heap->spans_reserved);
				_memory_unmap_span(span);
			}

			for (size_t iclass = 0; iclass < SIZE_CLASS_COUNT; ++iclass) {
				heap_class_t* heap_class = heap->span_class + iclass;
				span_t* span = heap_class->partial_span;
				while (span) {
					span_t* next = span->next;
					if (span->state == SPAN_STATE_ACTIVE) {
						uint32_t used_blocks = span->block_count;
						if (span->free_list_limit < span->block_count)
							used_blocks = span->free_list_limit;
						uint32_t free_blocks = 0;
						void* block = heap_class->free_list;
						while (block) {
							++free_blocks;
							block = *((void**)block);
						}
						block = span->free_list;
						while (block) {
							++free_blocks;
							block = *((void**)block);
						}
						if (used_blocks == (free_blocks + span->list_size))
							_memory_heap_cache_insert(heap, span);
					} else {
						if (span->used_count == span->list_size)
							_memory_heap_cache_insert(heap, span);
					}
					span = next;
				}
			}

#if ENABLE_THREAD_CACHE
			//Free span caches (other thread might have deferred after the thread using this heap finalized)
			_memory_heap_cache_adopt_deferred(heap);
			for (size_t iclass = 0; iclass < LARGE_CLASS_COUNT; ++iclass) {
				if (heap->span_cache[iclass])
					_memory_unmap_span_list(heap->span_cache[iclass]);
			}
#endif
			heap_t* next_heap = heap->next_heap;
			size_t heap_size = (1 + (sizeof(heap_t) >> _memory_page_size_shift)) * _memory_page_size;
			_memory_unmap(heap, heap_size, heap->align_offset, heap_size);
			heap = next_heap;
		}
	}

#if ENABLE_GLOBAL_CACHE
	//Free global caches
	for (size_t iclass = 0; iclass < LARGE_CLASS_COUNT; ++iclass)
		_memory_cache_finalize(&_memory_span_cache[iclass]);
#endif

	atomic_store_ptr(&_memory_orphan_heaps, 0);
	atomic_thread_fence_release();

#if (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
	pthread_key_delete(_memory_thread_heap);
#endif
#if defined(_MSC_VER) && !defined(__clang__) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
    FlsFree(fls_key);
#endif

#if ENABLE_STATISTICS
	//If you hit these asserts you probably have memory leaks or double frees in your code
	assert(!atomic_load32(&_mapped_pages));
	assert(!atomic_load32(&_reserved_spans));
	assert(!atomic_load32(&_mapped_pages_os));
#endif

	_rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
TRACY_API void
rpmalloc_thread_initialize(void) {
	if (!get_thread_heap_raw()) {
		heap_t* heap = _memory_allocate_heap();
		if (heap) {
			atomic_thread_fence_acquire();
#if ENABLE_STATISTICS
			atomic_incr32(&_memory_active_heaps);
#endif
			set_thread_heap(heap);
#if defined(_MSC_VER) && !defined(__clang__) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
			FlsSetValue(fls_key, heap);
#endif
		}
	}
}

//! Finalize thread, orphan heap
TRACY_API void
rpmalloc_thread_finalize(void) {
	heap_t* heap = get_thread_heap_raw();
	if (heap)
		_memory_heap_finalize(heap);
}

int
rpmalloc_is_thread_initialized(void) {
	return (get_thread_heap_raw() != 0) ? 1 : 0;
}

const rpmalloc_config_t*
rpmalloc_config(void) {
	return &_memory_config;
}

//! Map new pages to virtual memory
static void*
_memory_map_os(size_t size, size_t* offset) {
	//Either size is a heap (a single page) or a (multiple) span - we only need to align spans, and only if larger than map granularity
	size_t padding = ((size >= _memory_span_size) && (_memory_span_size > _memory_map_granularity)) ? _memory_span_size : 0;
	assert(size >= _memory_page_size);
#if PLATFORM_WINDOWS
	//Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not allocated unless/until the virtual addresses are actually accessed"
	void* ptr = VirtualAlloc(0, size + padding, (_memory_huge_pages ? MEM_LARGE_PAGES : 0) | MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!ptr) {
		assert(!"Failed to map virtual memory block");
		return 0;
	}
#else
	int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#  if defined(__APPLE__)
	int fd = (int)VM_MAKE_TAG(240U);
	if (_memory_huge_pages)
		fd |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
	void* ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#  elif defined(MAP_HUGETLB)
	void* ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, (_memory_huge_pages ? MAP_HUGETLB : 0) | flags, -1, 0);
#  else
	void* ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#  endif
	if ((ptr == MAP_FAILED) || !ptr) {
		assert("Failed to map virtual memory block" == 0);
		return 0;
	}
#endif
#if ENABLE_STATISTICS
	atomic_add32(&_mapped_pages_os, (int32_t)((size + padding) >> _memory_page_size_shift));
#endif
	if (padding) {
		size_t final_padding = padding - ((uintptr_t)ptr & ~_memory_span_mask);
		assert(final_padding <= _memory_span_size);
		assert(final_padding <= padding);
		assert(!(final_padding % 8));
		ptr = pointer_offset(ptr, final_padding);
		*offset = final_padding >> 3;
	}
	assert((size < _memory_span_size) || !((uintptr_t)ptr & ~_memory_span_mask));
	return ptr;
}

//! Unmap pages from virtual memory
static void
_memory_unmap_os(void* address, size_t size, size_t offset, size_t release) {
	assert(release || (offset == 0));
	assert(!release || (release >= _memory_page_size));
	assert(size >= _memory_page_size);
	if (release && offset) {
		offset <<= 3;
		address = pointer_offset(address, -(int32_t)offset);
#if PLATFORM_POSIX
		//Padding is always one span size
		release += _memory_span_size;
#endif
	}
#if !DISABLE_UNMAP
#if PLATFORM_WINDOWS
	if (!VirtualFree(address, release ? 0 : size, release ? MEM_RELEASE : MEM_DECOMMIT)) {
		assert(!"Failed to unmap virtual memory block");
	}
#else
	if (release) {
		if (munmap(address, release)) {
			assert("Failed to unmap virtual memory block" == 0);
		}
	}
	else {
#if defined(POSIX_MADV_FREE)
		if (posix_madvise(address, size, POSIX_MADV_FREE))
#endif
#if defined(POSIX_MADV_DONTNEED)
		if (posix_madvise(address, size, POSIX_MADV_DONTNEED)) {
			assert("Failed to madvise virtual memory block as free" == 0);
		}
#endif
	}
#endif
#endif
#if ENABLE_STATISTICS
	if (release)
		atomic_add32(&_mapped_pages_os, -(int32_t)(release >> _memory_page_size_shift));
#endif
}

// Extern interface

TRACY_API RPMALLOC_ALLOCATOR void*
rpmalloc(size_t size) {
#if ENABLE_VALIDATE_ARGS
	if (size >= MAX_ALLOC_SIZE) {
		errno = EINVAL;
		return 0;
	}
#endif
	heap_t* heap = get_thread_heap();
	return _memory_allocate(heap, size);
}

TRACY_API void
rpfree(void* ptr) {
	_memory_deallocate(ptr);
}

extern inline RPMALLOC_ALLOCATOR void*
rpcalloc(size_t num, size_t size) {
	size_t total;
#if ENABLE_VALIDATE_ARGS
#if PLATFORM_WINDOWS
	int err = SizeTMult(num, size, &total);
	if ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
		errno = EINVAL;
		return 0;
	}
#else
	int err = __builtin_umull_overflow(num, size, &total);
	if (err || (total >= MAX_ALLOC_SIZE)) {
		errno = EINVAL;
		return 0;
	}
#endif
#else
	total = num * size;
#endif
	heap_t* heap = get_thread_heap();
	void* block = _memory_allocate(heap, total);
	memset(block, 0, total);
	return block;
}

TRACY_API RPMALLOC_ALLOCATOR void*
rprealloc(void* ptr, size_t size) {
#if ENABLE_VALIDATE_ARGS
	if (size >= MAX_ALLOC_SIZE) {
		errno = EINVAL;
		return ptr;
	}
#endif
	return _memory_reallocate(ptr, size, 0, 0);
}

extern RPMALLOC_ALLOCATOR void*
rpaligned_realloc(void* ptr, size_t alignment, size_t size, size_t oldsize,
                  unsigned int flags) {
#if ENABLE_VALIDATE_ARGS
	if ((size + alignment < size) || (alignment > _memory_page_size)) {
		errno = EINVAL;
		return 0;
	}
#endif
	void* block;
	if (alignment > 32) {
		size_t usablesize = _memory_usable_size(ptr);
		if ((usablesize >= size) && (size >= (usablesize / 2)) && !((uintptr_t)ptr & (alignment - 1)))
			return ptr;

		block = rpaligned_alloc(alignment, size);
		if (ptr) {
			if (!oldsize)
				oldsize = usablesize;
			if (!(flags & RPMALLOC_NO_PRESERVE))
				memcpy(block, ptr, oldsize < size ? oldsize : size);
			rpfree(ptr);
		}
		//Mark as having aligned blocks
		span_t* span = (span_t*)((uintptr_t)block & _memory_span_mask);
		span->flags |= SPAN_FLAG_ALIGNED_BLOCKS;
	} else {
		block = _memory_reallocate(ptr, size, oldsize, flags);
	}
	return block;
}

extern RPMALLOC_ALLOCATOR void*
rpaligned_alloc(size_t alignment, size_t size) {
	if (alignment <= 16)
		return rpmalloc(size);

#if ENABLE_VALIDATE_ARGS
	if ((size + alignment) < size) {
		errno = EINVAL;
		return 0;
	}
	if (alignment & (alignment - 1)) {
		errno = EINVAL;
		return 0;
	}
#endif

	void* ptr = 0;
	size_t align_mask = alignment - 1;
	if (alignment < _memory_page_size) {
		ptr = rpmalloc(size + alignment);
		if ((uintptr_t)ptr & align_mask)
			ptr = (void*)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);
		//Mark as having aligned blocks
		span_t* span = (span_t*)((uintptr_t)ptr & _memory_span_mask);
		span->flags |= SPAN_FLAG_ALIGNED_BLOCKS;
		return ptr;
	}

	// Fallback to mapping new pages for this request. Since pointers passed
	// to rpfree must be able to reach the start of the span by bitmasking of
	// the address with the span size, the returned aligned pointer from this
	// function must be with a span size of the start of the mapped area.
	// In worst case this requires us to loop and map pages until we get a
	// suitable memory address. It also means we can never align to span size
	// or greater, since the span header will push alignment more than one
	// span size away from span start (thus causing pointer mask to give us
	// an invalid span start on free)
	if (alignment & align_mask) {
		errno = EINVAL;
		return 0;
	}
	if (alignment >= _memory_span_size) {
		errno = EINVAL;
		return 0;
	}

	size_t extra_pages = alignment / _memory_page_size;

	// Since each span has a header, we will at least need one extra memory page
	size_t num_pages = 1 + (size / _memory_page_size);
	if (size & (_memory_page_size - 1))
		++num_pages;

	if (extra_pages > num_pages)
		num_pages = 1 + extra_pages;

	size_t original_pages = num_pages;
	size_t limit_pages = (_memory_span_size / _memory_page_size) * 2;
	if (limit_pages < (original_pages * 2))
		limit_pages = original_pages * 2;

	size_t mapped_size, align_offset;
	span_t* span;

retry:
	align_offset = 0;
	mapped_size = num_pages * _memory_page_size;

	span = (span_t*)_memory_map(mapped_size, &align_offset);
	if (!span) {
		errno = ENOMEM;
		return 0;
	}
	ptr = pointer_offset(span, SPAN_HEADER_SIZE);

	if ((uintptr_t)ptr & align_mask)
		ptr = (void*)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);

	if (((size_t)pointer_diff(ptr, span) >= _memory_span_size) ||
	    (pointer_offset(ptr, size) > pointer_offset(span, mapped_size)) ||
	    (((uintptr_t)ptr & _memory_span_mask) != (uintptr_t)span)) {
		_memory_unmap(span, mapped_size, align_offset, mapped_size);
		++num_pages;
		if (num_pages > limit_pages) {
			errno = EINVAL;
			return 0;
		}
		goto retry;
	}

	//Store page count in span_count
	span->size_class = (uint32_t)-1;
	span->span_count = (uint32_t)num_pages;
	span->align_offset = (uint32_t)align_offset;
	_memory_statistics_add_peak(&_huge_pages_current, num_pages, _huge_pages_peak);

	return ptr;
}

extern inline RPMALLOC_ALLOCATOR void*
rpmemalign(size_t alignment, size_t size) {
	return rpaligned_alloc(alignment, size);
}

extern inline int
rpposix_memalign(void **memptr, size_t alignment, size_t size) {
	if (memptr)
		*memptr = rpaligned_alloc(alignment, size);
	else
		return EINVAL;
	return *memptr ? 0 : ENOMEM;
}

extern inline size_t
rpmalloc_usable_size(void* ptr) {
	return (ptr ? _memory_usable_size(ptr) : 0);
}

extern inline void
rpmalloc_thread_collect(void) {
}

void
rpmalloc_thread_statistics(rpmalloc_thread_statistics_t* stats) {
	memset(stats, 0, sizeof(rpmalloc_thread_statistics_t));
	heap_t* heap = get_thread_heap_raw();
	if (!heap)
		return;

	for (size_t iclass = 0; iclass < SIZE_CLASS_COUNT; ++iclass) {
		size_class_t* size_class = _memory_size_class + iclass;
		heap_class_t* heap_class = heap->span_class + iclass;
		span_t* span = heap_class->partial_span;
		while (span) {
			atomic_thread_fence_acquire();
			size_t free_count = span->list_size;
			if (span->state == SPAN_STATE_PARTIAL)
				free_count += (size_class->block_count - span->used_count);
			stats->sizecache = free_count * size_class->block_size;
			span = span->next;
		}
	}

#if ENABLE_THREAD_CACHE
	for (size_t iclass = 0; iclass < LARGE_CLASS_COUNT; ++iclass) {
		if (heap->span_cache[iclass])
			stats->spancache = (size_t)heap->span_cache[iclass]->list_size * (iclass + 1) * _memory_span_size;
		span_t* deferred_list = !iclass ? (span_t*)atomic_load_ptr(&heap->span_cache_deferred) : 0;
		//TODO: Incorrect, for deferred lists the size is NOT stored in list_size
		if (deferred_list)
			stats->spancache = (size_t)deferred_list->list_size * (iclass + 1) * _memory_span_size;
	}
#endif
#if ENABLE_STATISTICS
	stats->thread_to_global = heap->thread_to_global;
	stats->global_to_thread = heap->global_to_thread;

	for (size_t iclass = 0; iclass < LARGE_CLASS_COUNT; ++iclass) {
		stats->span_use[iclass].current = (size_t)atomic_load32(&heap->span_use[iclass].current);
		stats->span_use[iclass].peak = (size_t)heap->span_use[iclass].high;
		stats->span_use[iclass].to_global = (size_t)heap->span_use[iclass].spans_to_global;
		stats->span_use[iclass].from_global = (size_t)heap->span_use[iclass].spans_from_global;
		stats->span_use[iclass].to_cache = (size_t)heap->span_use[iclass].spans_to_cache;
		stats->span_use[iclass].from_cache = (size_t)heap->span_use[iclass].spans_from_cache;
		stats->span_use[iclass].to_reserved = (size_t)heap->span_use[iclass].spans_to_reserved;
		stats->span_use[iclass].from_reserved = (size_t)heap->span_use[iclass].spans_from_reserved;
		stats->span_use[iclass].map_calls = (size_t)heap->span_use[iclass].spans_map_calls;
	}
	for (size_t iclass = 0; iclass < SIZE_CLASS_COUNT; ++iclass) {
		stats->size_use[iclass].alloc_current = (size_t)atomic_load32(&heap->size_class_use[iclass].alloc_current);
		stats->size_use[iclass].alloc_peak = (size_t)heap->size_class_use[iclass].alloc_peak;
		stats->size_use[iclass].alloc_total = (size_t)heap->size_class_use[iclass].alloc_total;
		stats->size_use[iclass].free_total = (size_t)atomic_load32(&heap->size_class_use[iclass].free_total);
		stats->size_use[iclass].spans_to_cache = (size_t)heap->size_class_use[iclass].spans_to_cache;
		stats->size_use[iclass].spans_from_cache = (size_t)heap->size_class_use[iclass].spans_from_cache;
		stats->size_use[iclass].spans_from_reserved = (size_t)heap->size_class_use[iclass].spans_from_reserved;
		stats->size_use[iclass].map_calls = (size_t)heap->size_class_use[iclass].spans_map_calls;
	}
#endif
}

void
rpmalloc_global_statistics(rpmalloc_global_statistics_t* stats) {
	memset(stats, 0, sizeof(rpmalloc_global_statistics_t));
#if ENABLE_STATISTICS
	stats->mapped = (size_t)atomic_load32(&_mapped_pages) * _memory_page_size;
	stats->mapped_peak = (size_t)_mapped_pages_peak * _memory_page_size;
	stats->mapped_total = (size_t)atomic_load32(&_mapped_total) * _memory_page_size;
	stats->unmapped_total = (size_t)atomic_load32(&_unmapped_total) * _memory_page_size;
	stats->huge_alloc = (size_t)atomic_load32(&_huge_pages_current) * _memory_page_size;
	stats->huge_alloc_peak = (size_t)_huge_pages_peak * _memory_page_size;
#endif
#if ENABLE_GLOBAL_CACHE
	for (size_t iclass = 0; iclass < LARGE_CLASS_COUNT; ++iclass) {
		stats->cached += (size_t)atomic_load32(&_memory_span_cache[iclass].size) * (iclass + 1) * _memory_span_size;
	}
#endif
}

void
rpmalloc_dump_statistics(void* file) {
#if ENABLE_STATISTICS
	//If you hit this assert, you still have active threads or forgot to finalize some thread(s)
	assert(atomic_load32(&_memory_active_heaps) == 0);

	for (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
		heap_t* heap = atomic_load_ptr(&_memory_heaps[list_idx]);
		while (heap) {
			fprintf(file, "Heap %d stats:\n", heap->id);
			fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB FromCacheMiB FromReserveMiB MmapCalls\n");
			for (size_t iclass = 0; iclass < SIZE_CLASS_COUNT; ++iclass) {
				if (!heap->size_class_use[iclass].alloc_total) {
					assert(!atomic_load32(&heap->size_class_use[iclass].free_total));
					assert(!heap->size_class_use[iclass].spans_map_calls);
					continue;
				}
				fprintf(file, "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu %9u\n", (uint32_t)iclass,
					atomic_load32(&heap->size_class_use[iclass].alloc_current),
					heap->size_class_use[iclass].alloc_peak,
					heap->size_class_use[iclass].alloc_total,
					atomic_load32(&heap->size_class_use[iclass].free_total),
					_memory_size_class[iclass].block_size,
					_memory_size_class[iclass].block_count,
					heap->size_class_use[iclass].spans_current,
					heap->size_class_use[iclass].spans_peak,
					((size_t)heap->size_class_use[iclass].alloc_peak * (size_t)_memory_size_class[iclass].block_size) / (size_t)(1024 * 1024),
					((size_t)heap->size_class_use[iclass].spans_to_cache * _memory_span_size) / (size_t)(1024 * 1024),
					((size_t)heap->size_class_use[iclass].spans_from_cache * _memory_span_size) / (size_t)(1024 * 1024),
					((size_t)heap->size_class_use[iclass].spans_from_reserved * _memory_span_size) / (size_t)(1024 * 1024),
					heap->size_class_use[iclass].spans_map_calls);
			}
			fprintf(file, "Spans  Current     Peak  PeakMiB  Cached  ToCacheMiB FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB FromGlobalMiB  MmapCalls\n");
			for (size_t iclass = 0; iclass < LARGE_CLASS_COUNT; ++iclass) {
				if (!heap->span_use[iclass].high && !heap->span_use[iclass].spans_map_calls)
					continue;
				fprintf(file, "%4u: %8d %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n", (uint32_t)(iclass + 1),
					atomic_load32(&heap->span_use[iclass].current),
					heap->span_use[iclass].high,
					((size_t)heap->span_use[iclass].high * (size_t)_memory_span_size * (iclass + 1)) / (size_t)(1024 * 1024),
					heap->span_cache[iclass] ? heap->span_cache[iclass]->list_size : 0,
					((size_t)heap->span_use[iclass].spans_to_cache * (iclass + 1) * _memory_span_size) / (size_t)(1024 * 1024),
					((size_t)heap->span_use[iclass].spans_from_cache * (iclass + 1) * _memory_span_size) / (size_t)(1024 * 1024),
					((size_t)heap->span_use[iclass].spans_to_reserved * (iclass + 1) * _memory_span_size) / (size_t)(1024 * 1024),
					((size_t)heap->span_use[iclass].spans_from_reserved * (iclass + 1) * _memory_span_size) / (size_t)(1024 * 1024),
					((size_t)heap->span_use[iclass].spans_to_global * (size_t)_memory_span_size * (iclass + 1)) / (size_t)(1024 * 1024),
					((size_t)heap->span_use[iclass].spans_from_global * (size_t)_memory_span_size * (iclass + 1)) / (size_t)(1024 * 1024),
					heap->span_use[iclass].spans_map_calls);
			}
			fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
			fprintf(file, "%17zu %17zu\n", (size_t)heap->thread_to_global / (size_t)(1024 * 1024), (size_t)heap->global_to_thread / (size_t)(1024 * 1024));
			heap = heap->next_heap;
		}
	}

	fprintf(file, "Global stats:\n");
	size_t huge_current = (size_t)atomic_load32(&_huge_pages_current) * _memory_page_size;
	size_t huge_peak = (size_t)_huge_pages_peak * _memory_page_size;
	fprintf(file, "HugeCurrentMiB HugePeakMiB\n");
	fprintf(file, "%14zu %11zu\n", huge_current / (size_t)(1024 * 1024), huge_peak / (size_t)(1024 * 1024));

	size_t mapped = (size_t)atomic_load32(&_mapped_pages) * _memory_page_size;
	size_t mapped_os = (size_t)atomic_load32(&_mapped_pages_os) * _memory_page_size;
	size_t mapped_peak = (size_t)_mapped_pages_peak * _memory_page_size;
	size_t mapped_total = (size_t)atomic_load32(&_mapped_total) * _memory_page_size;
	size_t unmapped_total = (size_t)atomic_load32(&_unmapped_total) * _memory_page_size;
	size_t reserved_total = (size_t)atomic_load32(&_reserved_spans) * _memory_span_size;
	fprintf(file, "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB ReservedTotalMiB\n");
	fprintf(file, "%9zu %11zu %13zu %14zu %16zu %16zu\n",
		mapped / (size_t)(1024 * 1024),
		mapped_os / (size_t)(1024 * 1024),
		mapped_peak / (size_t)(1024 * 1024),
		mapped_total / (size_t)(1024 * 1024),
		unmapped_total / (size_t)(1024 * 1024),
		reserved_total / (size_t)(1024 * 1024));

	fprintf(file, "\n");
#else
	(void)sizeof(file);
#endif
}

}

#endif

/*** End of inlined file: tracy_rpmalloc.cpp ***/


/*** Start of inlined file: TracyDxt1.cpp ***/
#include <cassert>
#include <cstdint>
#include <string.h>

#ifdef __ARM_NEON
#  include <arm_neon.h>
#endif

#if defined __AVX__ && !defined __SSE4_1__
#  define __SSE4_1__
#endif

#if defined __SSE4_1__ || defined __AVX2__
#  ifdef _MSC_VER
#    include <intrin.h>
#  else
#    include <x86intrin.h>
#    ifndef _mm256_cvtsi256_si32
#      define _mm256_cvtsi256_si32( v ) ( _mm_cvtsi128_si32( _mm256_castsi256_si128( v ) ) )
#    endif
#  endif
#endif

namespace tracy
{

static inline uint16_t to565( uint8_t r, uint8_t g, uint8_t b )
{
    return ( ( r & 0xF8 ) << 8 ) | ( ( g & 0xFC ) << 3 ) | ( b >> 3 );
}

static inline uint16_t to565( uint32_t c )
{
    return
        ( ( c & 0xF80000 ) >> 19 ) |
        ( ( c & 0x00FC00 ) >> 5 ) |
        ( ( c & 0x0000F8 ) << 8 );
}

static const uint16_t DivTable[255*3+1] = {
    0xffff, 0xffff, 0xffff, 0xffff, 0xcccc, 0xaaaa, 0x9249, 0x8000, 0x71c7, 0x6666, 0x5d17, 0x5555, 0x4ec4, 0x4924, 0x4444, 0x4000,
    0x3c3c, 0x38e3, 0x35e5, 0x3333, 0x30c3, 0x2e8b, 0x2c85, 0x2aaa, 0x28f5, 0x2762, 0x25ed, 0x2492, 0x234f, 0x2222, 0x2108, 0x2000,
    0x1f07, 0x1e1e, 0x1d41, 0x1c71, 0x1bac, 0x1af2, 0x1a41, 0x1999, 0x18f9, 0x1861, 0x17d0, 0x1745, 0x16c1, 0x1642, 0x15c9, 0x1555,
    0x14e5, 0x147a, 0x1414, 0x13b1, 0x1352, 0x12f6, 0x129e, 0x1249, 0x11f7, 0x11a7, 0x115b, 0x1111, 0x10c9, 0x1084, 0x1041, 0x1000,
    0x0fc0, 0x0f83, 0x0f48, 0x0f0f, 0x0ed7, 0x0ea0, 0x0e6c, 0x0e38, 0x0e07, 0x0dd6, 0x0da7, 0x0d79, 0x0d4c, 0x0d20, 0x0cf6, 0x0ccc,
    0x0ca4, 0x0c7c, 0x0c56, 0x0c30, 0x0c0c, 0x0be8, 0x0bc5, 0x0ba2, 0x0b81, 0x0b60, 0x0b40, 0x0b21, 0x0b02, 0x0ae4, 0x0ac7, 0x0aaa,
    0x0a8e, 0x0a72, 0x0a57, 0x0a3d, 0x0a23, 0x0a0a, 0x09f1, 0x09d8, 0x09c0, 0x09a9, 0x0991, 0x097b, 0x0964, 0x094f, 0x0939, 0x0924,
    0x090f, 0x08fb, 0x08e7, 0x08d3, 0x08c0, 0x08ad, 0x089a, 0x0888, 0x0876, 0x0864, 0x0853, 0x0842, 0x0831, 0x0820, 0x0810, 0x0800,
    0x07f0, 0x07e0, 0x07d1, 0x07c1, 0x07b3, 0x07a4, 0x0795, 0x0787, 0x0779, 0x076b, 0x075d, 0x0750, 0x0743, 0x0736, 0x0729, 0x071c,
    0x070f, 0x0703, 0x06f7, 0x06eb, 0x06df, 0x06d3, 0x06c8, 0x06bc, 0x06b1, 0x06a6, 0x069b, 0x0690, 0x0685, 0x067b, 0x0670, 0x0666,
    0x065c, 0x0652, 0x0648, 0x063e, 0x0634, 0x062b, 0x0621, 0x0618, 0x060f, 0x0606, 0x05fd, 0x05f4, 0x05eb, 0x05e2, 0x05d9, 0x05d1,
    0x05c9, 0x05c0, 0x05b8, 0x05b0, 0x05a8, 0x05a0, 0x0598, 0x0590, 0x0588, 0x0581, 0x0579, 0x0572, 0x056b, 0x0563, 0x055c, 0x0555,
    0x054e, 0x0547, 0x0540, 0x0539, 0x0532, 0x052b, 0x0525, 0x051e, 0x0518, 0x0511, 0x050b, 0x0505, 0x04fe, 0x04f8, 0x04f2, 0x04ec,
    0x04e6, 0x04e0, 0x04da, 0x04d4, 0x04ce, 0x04c8, 0x04c3, 0x04bd, 0x04b8, 0x04b2, 0x04ad, 0x04a7, 0x04a2, 0x049c, 0x0497, 0x0492,
    0x048d, 0x0487, 0x0482, 0x047d, 0x0478, 0x0473, 0x046e, 0x0469, 0x0465, 0x0460, 0x045b, 0x0456, 0x0452, 0x044d, 0x0448, 0x0444,
    0x043f, 0x043b, 0x0436, 0x0432, 0x042d, 0x0429, 0x0425, 0x0421, 0x041c, 0x0418, 0x0414, 0x0410, 0x040c, 0x0408, 0x0404, 0x0400,
    0x03fc, 0x03f8, 0x03f4, 0x03f0, 0x03ec, 0x03e8, 0x03e4, 0x03e0, 0x03dd, 0x03d9, 0x03d5, 0x03d2, 0x03ce, 0x03ca, 0x03c7, 0x03c3,
    0x03c0, 0x03bc, 0x03b9, 0x03b5, 0x03b2, 0x03ae, 0x03ab, 0x03a8, 0x03a4, 0x03a1, 0x039e, 0x039b, 0x0397, 0x0394, 0x0391, 0x038e,
    0x038b, 0x0387, 0x0384, 0x0381, 0x037e, 0x037b, 0x0378, 0x0375, 0x0372, 0x036f, 0x036c, 0x0369, 0x0366, 0x0364, 0x0361, 0x035e,
    0x035b, 0x0358, 0x0355, 0x0353, 0x0350, 0x034d, 0x034a, 0x0348, 0x0345, 0x0342, 0x0340, 0x033d, 0x033a, 0x0338, 0x0335, 0x0333,
    0x0330, 0x032e, 0x032b, 0x0329, 0x0326, 0x0324, 0x0321, 0x031f, 0x031c, 0x031a, 0x0317, 0x0315, 0x0313, 0x0310, 0x030e, 0x030c,
    0x0309, 0x0307, 0x0305, 0x0303, 0x0300, 0x02fe, 0x02fc, 0x02fa, 0x02f7, 0x02f5, 0x02f3, 0x02f1, 0x02ef, 0x02ec, 0x02ea, 0x02e8,
    0x02e6, 0x02e4, 0x02e2, 0x02e0, 0x02de, 0x02dc, 0x02da, 0x02d8, 0x02d6, 0x02d4, 0x02d2, 0x02d0, 0x02ce, 0x02cc, 0x02ca, 0x02c8,
    0x02c6, 0x02c4, 0x02c2, 0x02c0, 0x02be, 0x02bc, 0x02bb, 0x02b9, 0x02b7, 0x02b5, 0x02b3, 0x02b1, 0x02b0, 0x02ae, 0x02ac, 0x02aa,
    0x02a8, 0x02a7, 0x02a5, 0x02a3, 0x02a1, 0x02a0, 0x029e, 0x029c, 0x029b, 0x0299, 0x0297, 0x0295, 0x0294, 0x0292, 0x0291, 0x028f,
    0x028d, 0x028c, 0x028a, 0x0288, 0x0287, 0x0285, 0x0284, 0x0282, 0x0280, 0x027f, 0x027d, 0x027c, 0x027a, 0x0279, 0x0277, 0x0276,
    0x0274, 0x0273, 0x0271, 0x0270, 0x026e, 0x026d, 0x026b, 0x026a, 0x0268, 0x0267, 0x0265, 0x0264, 0x0263, 0x0261, 0x0260, 0x025e,
    0x025d, 0x025c, 0x025a, 0x0259, 0x0257, 0x0256, 0x0255, 0x0253, 0x0252, 0x0251, 0x024f, 0x024e, 0x024d, 0x024b, 0x024a, 0x0249,
    0x0247, 0x0246, 0x0245, 0x0243, 0x0242, 0x0241, 0x0240, 0x023e, 0x023d, 0x023c, 0x023b, 0x0239, 0x0238, 0x0237, 0x0236, 0x0234,
    0x0233, 0x0232, 0x0231, 0x0230, 0x022e, 0x022d, 0x022c, 0x022b, 0x022a, 0x0229, 0x0227, 0x0226, 0x0225, 0x0224, 0x0223, 0x0222,
    0x0220, 0x021f, 0x021e, 0x021d, 0x021c, 0x021b, 0x021a, 0x0219, 0x0218, 0x0216, 0x0215, 0x0214, 0x0213, 0x0212, 0x0211, 0x0210,
    0x020f, 0x020e, 0x020d, 0x020c, 0x020b, 0x020a, 0x0209, 0x0208, 0x0207, 0x0206, 0x0205, 0x0204, 0x0203, 0x0202, 0x0201, 0x0200,
    0x01ff, 0x01fe, 0x01fd, 0x01fc, 0x01fb, 0x01fa, 0x01f9, 0x01f8, 0x01f7, 0x01f6, 0x01f5, 0x01f4, 0x01f3, 0x01f2, 0x01f1, 0x01f0,
    0x01ef, 0x01ee, 0x01ed, 0x01ec, 0x01eb, 0x01ea, 0x01e9, 0x01e9, 0x01e8, 0x01e7, 0x01e6, 0x01e5, 0x01e4, 0x01e3, 0x01e2, 0x01e1,
    0x01e0, 0x01e0, 0x01df, 0x01de, 0x01dd, 0x01dc, 0x01db, 0x01da, 0x01da, 0x01d9, 0x01d8, 0x01d7, 0x01d6, 0x01d5, 0x01d4, 0x01d4,
    0x01d3, 0x01d2, 0x01d1, 0x01d0, 0x01cf, 0x01cf, 0x01ce, 0x01cd, 0x01cc, 0x01cb, 0x01cb, 0x01ca, 0x01c9, 0x01c8, 0x01c7, 0x01c7,
    0x01c6, 0x01c5, 0x01c4, 0x01c3, 0x01c3, 0x01c2, 0x01c1, 0x01c0, 0x01c0, 0x01bf, 0x01be, 0x01bd, 0x01bd, 0x01bc, 0x01bb, 0x01ba,
    0x01ba, 0x01b9, 0x01b8, 0x01b7, 0x01b7, 0x01b6, 0x01b5, 0x01b4, 0x01b4, 0x01b3, 0x01b2, 0x01b2, 0x01b1, 0x01b0, 0x01af, 0x01af,
    0x01ae, 0x01ad, 0x01ad, 0x01ac, 0x01ab, 0x01aa, 0x01aa, 0x01a9, 0x01a8, 0x01a8, 0x01a7, 0x01a6, 0x01a6, 0x01a5, 0x01a4, 0x01a4,
    0x01a3, 0x01a2, 0x01a2, 0x01a1, 0x01a0, 0x01a0, 0x019f, 0x019e, 0x019e, 0x019d, 0x019c, 0x019c, 0x019b, 0x019a, 0x019a, 0x0199,
    0x0198, 0x0198, 0x0197, 0x0197, 0x0196, 0x0195, 0x0195, 0x0194, 0x0193, 0x0193, 0x0192, 0x0192, 0x0191, 0x0190, 0x0190, 0x018f,
    0x018f, 0x018e, 0x018d, 0x018d, 0x018c, 0x018b, 0x018b, 0x018a, 0x018a, 0x0189, 0x0189, 0x0188, 0x0187, 0x0187, 0x0186, 0x0186,
    0x0185, 0x0184, 0x0184, 0x0183, 0x0183, 0x0182, 0x0182, 0x0181, 0x0180, 0x0180, 0x017f, 0x017f, 0x017e, 0x017e, 0x017d, 0x017d,
    0x017c, 0x017b, 0x017b, 0x017a, 0x017a, 0x0179, 0x0179, 0x0178, 0x0178, 0x0177, 0x0177, 0x0176, 0x0175, 0x0175, 0x0174, 0x0174,
    0x0173, 0x0173, 0x0172, 0x0172, 0x0171, 0x0171, 0x0170, 0x0170, 0x016f, 0x016f, 0x016e, 0x016e, 0x016d, 0x016d, 0x016c, 0x016c,
    0x016b, 0x016b, 0x016a, 0x016a, 0x0169, 0x0169, 0x0168, 0x0168, 0x0167, 0x0167, 0x0166, 0x0166, 0x0165, 0x0165, 0x0164, 0x0164,
    0x0163, 0x0163, 0x0162, 0x0162, 0x0161, 0x0161, 0x0160, 0x0160, 0x015f, 0x015f, 0x015e, 0x015e, 0x015d, 0x015d, 0x015d, 0x015c,
    0x015c, 0x015b, 0x015b, 0x015a, 0x015a, 0x0159, 0x0159, 0x0158, 0x0158, 0x0158, 0x0157, 0x0157, 0x0156, 0x0156
};
static const uint16_t DivTableNEON[255*3+1] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x1c71, 0x1af2, 0x1999, 0x1861, 0x1745, 0x1642, 0x1555, 0x147a, 0x13b1, 0x12f6, 0x1249, 0x11a7, 0x1111, 0x1084, 0x1000,
    0x0f83, 0x0f0f, 0x0ea0, 0x0e38, 0x0dd6, 0x0d79, 0x0d20, 0x0ccc, 0x0c7c, 0x0c30, 0x0be8, 0x0ba2, 0x0b60, 0x0b21, 0x0ae4, 0x0aaa,
    0x0a72, 0x0a3d, 0x0a0a, 0x09d8, 0x09a9, 0x097b, 0x094f, 0x0924, 0x08fb, 0x08d3, 0x08ad, 0x0888, 0x0864, 0x0842, 0x0820, 0x0800,
    0x07e0, 0x07c1, 0x07a4, 0x0787, 0x076b, 0x0750, 0x0736, 0x071c, 0x0703, 0x06eb, 0x06d3, 0x06bc, 0x06a6, 0x0690, 0x067b, 0x0666,
    0x0652, 0x063e, 0x062b, 0x0618, 0x0606, 0x05f4, 0x05e2, 0x05d1, 0x05c0, 0x05b0, 0x05a0, 0x0590, 0x0581, 0x0572, 0x0563, 0x0555,
    0x0547, 0x0539, 0x052b, 0x051e, 0x0511, 0x0505, 0x04f8, 0x04ec, 0x04e0, 0x04d4, 0x04c8, 0x04bd, 0x04b2, 0x04a7, 0x049c, 0x0492,
    0x0487, 0x047d, 0x0473, 0x0469, 0x0460, 0x0456, 0x044d, 0x0444, 0x043b, 0x0432, 0x0429, 0x0421, 0x0418, 0x0410, 0x0408, 0x0400,
    0x03f8, 0x03f0, 0x03e8, 0x03e0, 0x03d9, 0x03d2, 0x03ca, 0x03c3, 0x03bc, 0x03b5, 0x03ae, 0x03a8, 0x03a1, 0x039b, 0x0394, 0x038e,
    0x0387, 0x0381, 0x037b, 0x0375, 0x036f, 0x0369, 0x0364, 0x035e, 0x0358, 0x0353, 0x034d, 0x0348, 0x0342, 0x033d, 0x0338, 0x0333,
    0x032e, 0x0329, 0x0324, 0x031f, 0x031a, 0x0315, 0x0310, 0x030c, 0x0307, 0x0303, 0x02fe, 0x02fa, 0x02f5, 0x02f1, 0x02ec, 0x02e8,
    0x02e4, 0x02e0, 0x02dc, 0x02d8, 0x02d4, 0x02d0, 0x02cc, 0x02c8, 0x02c4, 0x02c0, 0x02bc, 0x02b9, 0x02b5, 0x02b1, 0x02ae, 0x02aa,
    0x02a7, 0x02a3, 0x02a0, 0x029c, 0x0299, 0x0295, 0x0292, 0x028f, 0x028c, 0x0288, 0x0285, 0x0282, 0x027f, 0x027c, 0x0279, 0x0276,
    0x0273, 0x0270, 0x026d, 0x026a, 0x0267, 0x0264, 0x0261, 0x025e, 0x025c, 0x0259, 0x0256, 0x0253, 0x0251, 0x024e, 0x024b, 0x0249,
    0x0246, 0x0243, 0x0241, 0x023e, 0x023c, 0x0239, 0x0237, 0x0234, 0x0232, 0x0230, 0x022d, 0x022b, 0x0229, 0x0226, 0x0224, 0x0222,
    0x021f, 0x021d, 0x021b, 0x0219, 0x0216, 0x0214, 0x0212, 0x0210, 0x020e, 0x020c, 0x020a, 0x0208, 0x0206, 0x0204, 0x0202, 0x0200,
    0x01fe, 0x01fc, 0x01fa, 0x01f8, 0x01f6, 0x01f4, 0x01f2, 0x01f0, 0x01ee, 0x01ec, 0x01ea, 0x01e9, 0x01e7, 0x01e5, 0x01e3, 0x01e1,
    0x01e0, 0x01de, 0x01dc, 0x01da, 0x01d9, 0x01d7, 0x01d5, 0x01d4, 0x01d2, 0x01d0, 0x01cf, 0x01cd, 0x01cb, 0x01ca, 0x01c8, 0x01c7,
    0x01c5, 0x01c3, 0x01c2, 0x01c0, 0x01bf, 0x01bd, 0x01bc, 0x01ba, 0x01b9, 0x01b7, 0x01b6, 0x01b4, 0x01b3, 0x01b2, 0x01b0, 0x01af,
    0x01ad, 0x01ac, 0x01aa, 0x01a9, 0x01a8, 0x01a6, 0x01a5, 0x01a4, 0x01a2, 0x01a1, 0x01a0, 0x019e, 0x019d, 0x019c, 0x019a, 0x0199,
    0x0198, 0x0197, 0x0195, 0x0194, 0x0193, 0x0192, 0x0190, 0x018f, 0x018e, 0x018d, 0x018b, 0x018a, 0x0189, 0x0188, 0x0187, 0x0186,
    0x0184, 0x0183, 0x0182, 0x0181, 0x0180, 0x017f, 0x017e, 0x017d, 0x017b, 0x017a, 0x0179, 0x0178, 0x0177, 0x0176, 0x0175, 0x0174,
    0x0173, 0x0172, 0x0171, 0x0170, 0x016f, 0x016e, 0x016d, 0x016c, 0x016b, 0x016a, 0x0169, 0x0168, 0x0167, 0x0166, 0x0165, 0x0164,
    0x0163, 0x0162, 0x0161, 0x0160, 0x015f, 0x015e, 0x015d, 0x015c, 0x015b, 0x015a, 0x0159, 0x0158, 0x0158, 0x0157, 0x0156, 0x0155,
    0x0154, 0x0153, 0x0152, 0x0151, 0x0150, 0x0150, 0x014f, 0x014e, 0x014d, 0x014c, 0x014b, 0x014a, 0x014a, 0x0149, 0x0148, 0x0147,
    0x0146, 0x0146, 0x0145, 0x0144, 0x0143, 0x0142, 0x0142, 0x0141, 0x0140, 0x013f, 0x013e, 0x013e, 0x013d, 0x013c, 0x013b, 0x013b,
    0x013a, 0x0139, 0x0138, 0x0138, 0x0137, 0x0136, 0x0135, 0x0135, 0x0134, 0x0133, 0x0132, 0x0132, 0x0131, 0x0130, 0x0130, 0x012f,
    0x012e, 0x012e, 0x012d, 0x012c, 0x012b, 0x012b, 0x012a, 0x0129, 0x0129, 0x0128, 0x0127, 0x0127, 0x0126, 0x0125, 0x0125, 0x0124,
    0x0123, 0x0123, 0x0122, 0x0121, 0x0121, 0x0120, 0x0120, 0x011f, 0x011e, 0x011e, 0x011d, 0x011c, 0x011c, 0x011b, 0x011b, 0x011a,
    0x0119, 0x0119, 0x0118, 0x0118, 0x0117, 0x0116, 0x0116, 0x0115, 0x0115, 0x0114, 0x0113, 0x0113, 0x0112, 0x0112, 0x0111, 0x0111,
    0x0110, 0x010f, 0x010f, 0x010e, 0x010e, 0x010d, 0x010d, 0x010c, 0x010c, 0x010b, 0x010a, 0x010a, 0x0109, 0x0109, 0x0108, 0x0108,
    0x0107, 0x0107, 0x0106, 0x0106, 0x0105, 0x0105, 0x0104, 0x0104, 0x0103, 0x0103, 0x0102, 0x0102, 0x0101, 0x0101, 0x0100, 0x0100,
    0x00ff, 0x00ff, 0x00fe, 0x00fe, 0x00fd, 0x00fd, 0x00fc, 0x00fc, 0x00fb, 0x00fb, 0x00fa, 0x00fa, 0x00f9, 0x00f9, 0x00f8, 0x00f8,
    0x00f7, 0x00f7, 0x00f6, 0x00f6, 0x00f5, 0x00f5, 0x00f4, 0x00f4, 0x00f4, 0x00f3, 0x00f3, 0x00f2, 0x00f2, 0x00f1, 0x00f1, 0x00f0,
    0x00f0, 0x00f0, 0x00ef, 0x00ef, 0x00ee, 0x00ee, 0x00ed, 0x00ed, 0x00ed, 0x00ec, 0x00ec, 0x00eb, 0x00eb, 0x00ea, 0x00ea, 0x00ea,
    0x00e9, 0x00e9, 0x00e8, 0x00e8, 0x00e7, 0x00e7, 0x00e7, 0x00e6, 0x00e6, 0x00e5, 0x00e5, 0x00e5, 0x00e4, 0x00e4, 0x00e3, 0x00e3,
    0x00e3, 0x00e2, 0x00e2, 0x00e1, 0x00e1, 0x00e1, 0x00e0, 0x00e0, 0x00e0, 0x00df, 0x00df, 0x00de, 0x00de, 0x00de, 0x00dd, 0x00dd,
    0x00dd, 0x00dc, 0x00dc, 0x00db, 0x00db, 0x00db, 0x00da, 0x00da, 0x00da, 0x00d9, 0x00d9, 0x00d9, 0x00d8, 0x00d8, 0x00d7, 0x00d7,
    0x00d7, 0x00d6, 0x00d6, 0x00d6, 0x00d5, 0x00d5, 0x00d5, 0x00d4, 0x00d4, 0x00d4, 0x00d3, 0x00d3, 0x00d3, 0x00d2, 0x00d2, 0x00d2,
    0x00d1, 0x00d1, 0x00d1, 0x00d0, 0x00d0, 0x00d0, 0x00cf, 0x00cf, 0x00cf, 0x00ce, 0x00ce, 0x00ce, 0x00cd, 0x00cd, 0x00cd, 0x00cc,
    0x00cc, 0x00cc, 0x00cb, 0x00cb, 0x00cb, 0x00ca, 0x00ca, 0x00ca, 0x00c9, 0x00c9, 0x00c9, 0x00c9, 0x00c8, 0x00c8, 0x00c8, 0x00c7,
    0x00c7, 0x00c7, 0x00c6, 0x00c6, 0x00c6, 0x00c5, 0x00c5, 0x00c5, 0x00c5, 0x00c4, 0x00c4, 0x00c4, 0x00c3, 0x00c3, 0x00c3, 0x00c3,
    0x00c2, 0x00c2, 0x00c2, 0x00c1, 0x00c1, 0x00c1, 0x00c1, 0x00c0, 0x00c0, 0x00c0, 0x00bf, 0x00bf, 0x00bf, 0x00bf, 0x00be, 0x00be,
    0x00be, 0x00bd, 0x00bd, 0x00bd, 0x00bd, 0x00bc, 0x00bc, 0x00bc, 0x00bc, 0x00bb, 0x00bb, 0x00bb, 0x00ba, 0x00ba, 0x00ba, 0x00ba,
    0x00b9, 0x00b9, 0x00b9, 0x00b9, 0x00b8, 0x00b8, 0x00b8, 0x00b8, 0x00b7, 0x00b7, 0x00b7, 0x00b7, 0x00b6, 0x00b6, 0x00b6, 0x00b6,
    0x00b5, 0x00b5, 0x00b5, 0x00b5, 0x00b4, 0x00b4, 0x00b4, 0x00b4, 0x00b3, 0x00b3, 0x00b3, 0x00b3, 0x00b2, 0x00b2, 0x00b2, 0x00b2,
    0x00b1, 0x00b1, 0x00b1, 0x00b1, 0x00b0, 0x00b0, 0x00b0, 0x00b0, 0x00af, 0x00af, 0x00af, 0x00af, 0x00ae, 0x00ae, 0x00ae, 0x00ae,
    0x00ae, 0x00ad, 0x00ad, 0x00ad, 0x00ad, 0x00ac, 0x00ac, 0x00ac, 0x00ac, 0x00ac, 0x00ab, 0x00ab, 0x00ab, 0x00ab,
};

static tracy_force_inline uint64_t ProcessRGB( const uint8_t* src )
{
#ifdef __SSE4_1__
    __m128i px0 = _mm_loadu_si128(((__m128i*)src) + 0);
    __m128i px1 = _mm_loadu_si128(((__m128i*)src) + 1);
    __m128i px2 = _mm_loadu_si128(((__m128i*)src) + 2);
    __m128i px3 = _mm_loadu_si128(((__m128i*)src) + 3);

    __m128i smask = _mm_set1_epi32( 0xF8FCF8 );
    __m128i sd0 = _mm_and_si128( px0, smask );
    __m128i sd1 = _mm_and_si128( px1, smask );
    __m128i sd2 = _mm_and_si128( px2, smask );
    __m128i sd3 = _mm_and_si128( px3, smask );

    __m128i sc = _mm_shuffle_epi32(sd0, _MM_SHUFFLE(0, 0, 0, 0));

    __m128i sc0 = _mm_cmpeq_epi8(sd0, sc);
    __m128i sc1 = _mm_cmpeq_epi8(sd1, sc);
    __m128i sc2 = _mm_cmpeq_epi8(sd2, sc);
    __m128i sc3 = _mm_cmpeq_epi8(sd3, sc);

    __m128i sm0 = _mm_and_si128(sc0, sc1);
    __m128i sm1 = _mm_and_si128(sc2, sc3);
    __m128i sm = _mm_and_si128(sm0, sm1);

    if( _mm_testc_si128(sm, _mm_set1_epi32(-1)) )
    {
        return uint64_t( to565( src[0], src[1], src[2] ) ) << 16;
    }

    __m128i amask = _mm_set1_epi32( 0xFFFFFF );
    px0 = _mm_and_si128( px0, amask );
    px1 = _mm_and_si128( px1, amask );
    px2 = _mm_and_si128( px2, amask );
    px3 = _mm_and_si128( px3, amask );

    __m128i min0 = _mm_min_epu8( px0, px1 );
    __m128i min1 = _mm_min_epu8( px2, px3 );
    __m128i min2 = _mm_min_epu8( min0, min1 );

    __m128i max0 = _mm_max_epu8( px0, px1 );
    __m128i max1 = _mm_max_epu8( px2, px3 );
    __m128i max2 = _mm_max_epu8( max0, max1 );

    __m128i min3 = _mm_shuffle_epi32( min2, _MM_SHUFFLE( 2, 3, 0, 1 ) );
    __m128i max3 = _mm_shuffle_epi32( max2, _MM_SHUFFLE( 2, 3, 0, 1 ) );
    __m128i min4 = _mm_min_epu8( min2, min3 );
    __m128i max4 = _mm_max_epu8( max2, max3 );

    __m128i min5 = _mm_shuffle_epi32( min4, _MM_SHUFFLE( 0, 0, 2, 2 ) );
    __m128i max5 = _mm_shuffle_epi32( max4, _MM_SHUFFLE( 0, 0, 2, 2 ) );
    __m128i rmin = _mm_min_epu8( min4, min5 );
    __m128i rmax = _mm_max_epu8( max4, max5 );

    __m128i range1 = _mm_subs_epu8( rmax, rmin );
    __m128i range2 = _mm_sad_epu8( rmax, rmin );

    uint32_t vrange = _mm_cvtsi128_si32( range2 ) >> 1;
    __m128i range = _mm_set1_epi16( DivTable[vrange] );

    __m128i inset1 = _mm_srli_epi16( range1, 4 );
    __m128i inset = _mm_and_si128( inset1, _mm_set1_epi8( 0xF ) );
    __m128i min = _mm_adds_epu8( rmin, inset );
    __m128i max = _mm_subs_epu8( rmax, inset );

    __m128i c0 = _mm_subs_epu8( px0, rmin );
    __m128i c1 = _mm_subs_epu8( px1, rmin );
    __m128i c2 = _mm_subs_epu8( px2, rmin );
    __m128i c3 = _mm_subs_epu8( px3, rmin );

    __m128i is0 = _mm_maddubs_epi16( c0, _mm_set1_epi8( 1 ) );
    __m128i is1 = _mm_maddubs_epi16( c1, _mm_set1_epi8( 1 ) );
    __m128i is2 = _mm_maddubs_epi16( c2, _mm_set1_epi8( 1 ) );
    __m128i is3 = _mm_maddubs_epi16( c3, _mm_set1_epi8( 1 ) );

    __m128i s0 = _mm_hadd_epi16( is0, is1 );
    __m128i s1 = _mm_hadd_epi16( is2, is3 );

    __m128i m0 = _mm_mulhi_epu16( s0, range );
    __m128i m1 = _mm_mulhi_epu16( s1, range );

    __m128i p0 = _mm_packus_epi16( m0, m1 );

    __m128i p1 = _mm_or_si128( _mm_srai_epi32( p0, 6 ), _mm_srai_epi32( p0, 12 ) );
    __m128i p2 = _mm_or_si128( _mm_srai_epi32( p0, 18 ), p0 );
    __m128i p3 = _mm_or_si128( p1, p2 );
    __m128i p =_mm_shuffle_epi8( p3, _mm_set1_epi32( 0x0C080400 ) );

    uint32_t vmin = _mm_cvtsi128_si32( min );
    uint32_t vmax = _mm_cvtsi128_si32( max );
    uint32_t vp = _mm_cvtsi128_si32( p );

    return uint64_t( ( uint64_t( to565( vmin ) ) << 16 ) | to565( vmax ) | ( uint64_t( vp ) << 32 ) );
#elif defined __ARM_NEON
#  ifdef __aarch64__
    uint8x16x4_t px = vld4q_u8( src );

    uint8x16_t lr = px.val[0];
    uint8x16_t lg = px.val[1];
    uint8x16_t lb = px.val[2];

    uint8_t rmaxr = vmaxvq_u8( lr );
    uint8_t rmaxg = vmaxvq_u8( lg );
    uint8_t rmaxb = vmaxvq_u8( lb );

    uint8_t rminr = vminvq_u8( lr );
    uint8_t rming = vminvq_u8( lg );
    uint8_t rminb = vminvq_u8( lb );

    int rr = rmaxr - rminr;
    int rg = rmaxg - rming;
    int rb = rmaxb - rminb;

    int vrange1 = rr + rg + rb;
    uint16_t vrange2 = DivTableNEON[vrange1];

    uint8_t insetr = rr >> 4;
    uint8_t insetg = rg >> 4;
    uint8_t insetb = rb >> 4;

    uint8_t minr = rminr + insetr;
    uint8_t ming = rming + insetg;
    uint8_t minb = rminb + insetb;

    uint8_t maxr = rmaxr - insetr;
    uint8_t maxg = rmaxg - insetg;
    uint8_t maxb = rmaxb - insetb;

    uint8x16_t cr = vsubq_u8( lr, vdupq_n_u8( rminr ) );
    uint8x16_t cg = vsubq_u8( lg, vdupq_n_u8( rming ) );
    uint8x16_t cb = vsubq_u8( lb, vdupq_n_u8( rminb ) );

    uint16x8_t is0l = vaddl_u8( vget_low_u8( cr ), vget_low_u8( cg ) );
    uint16x8_t is0h = vaddl_u8( vget_high_u8( cr ), vget_high_u8( cg ) );
    uint16x8_t is1l = vaddw_u8( is0l, vget_low_u8( cb ) );
    uint16x8_t is1h = vaddw_u8( is0h, vget_high_u8( cb ) );

    int16x8_t range = vdupq_n_s16( vrange2 );
    uint16x8_t m0 = vreinterpretq_u16_s16( vqdmulhq_s16( vreinterpretq_s16_u16( is1l ), range ) );
    uint16x8_t m1 = vreinterpretq_u16_s16( vqdmulhq_s16( vreinterpretq_s16_u16( is1h ), range ) );

    uint8x8_t p00 = vmovn_u16( m0 );
    uint8x8_t p01 = vmovn_u16( m1 );
    uint8x16_t p0 = vcombine_u8( p00, p01 );

    uint32x4_t p1 = vaddq_u32( vshrq_n_u32( vreinterpretq_u32_u8( p0 ), 6 ), vshrq_n_u32( vreinterpretq_u32_u8( p0 ), 12 ) );
    uint32x4_t p2 = vaddq_u32( vshrq_n_u32( vreinterpretq_u32_u8( p0 ), 18 ), vreinterpretq_u32_u8( p0 ) );
    uint32x4_t p3 = vaddq_u32( p1, p2 );

    uint16x4x2_t p4 = vuzp_u16( vget_low_u16( vreinterpretq_u16_u32( p3 ) ), vget_high_u16( vreinterpretq_u16_u32( p3 ) ) );
    uint8x8x2_t p = vuzp_u8( vreinterpret_u8_u16( p4.val[0] ), vreinterpret_u8_u16( p4.val[0] ) );

    uint32_t vp;
    vst1_lane_u32( &vp, vreinterpret_u32_u8( p.val[0] ), 0 );

    return uint64_t( ( uint64_t( to565( minr, ming, minb ) ) << 16 ) | to565( maxr, maxg, maxb ) | ( uint64_t( vp ) << 32 ) );
#  else
    uint32x4_t px0 = vld1q_u32( (uint32_t*)src );
    uint32x4_t px1 = vld1q_u32( (uint32_t*)src + 4 );
    uint32x4_t px2 = vld1q_u32( (uint32_t*)src + 8 );
    uint32x4_t px3 = vld1q_u32( (uint32_t*)src + 12 );

    uint32x4_t smask = vdupq_n_u32( 0xF8FCF8 );
    uint32x4_t sd0 = vandq_u32( smask, px0 );
    uint32x4_t sd1 = vandq_u32( smask, px1 );
    uint32x4_t sd2 = vandq_u32( smask, px2 );
    uint32x4_t sd3 = vandq_u32( smask, px3 );

    uint32x4_t sc = vdupq_n_u32( sd0[0] );

    uint32x4_t sc0 = vceqq_u32( sd0, sc );
    uint32x4_t sc1 = vceqq_u32( sd1, sc );
    uint32x4_t sc2 = vceqq_u32( sd2, sc );
    uint32x4_t sc3 = vceqq_u32( sd3, sc );

    uint32x4_t sm0 = vandq_u32( sc0, sc1 );
    uint32x4_t sm1 = vandq_u32( sc2, sc3 );
    int64x2_t sm = vreinterpretq_s64_u32( vandq_u32( sm0, sm1 ) );

    if( sm[0] == -1 && sm[1] == -1 )
    {
        return uint64_t( to565( src[0], src[1], src[2] ) ) << 16;
    }

    uint32x4_t mask = vdupq_n_u32( 0xFFFFFF );
    uint8x16_t l0 = vreinterpretq_u8_u32( vandq_u32( mask, px0 ) );
    uint8x16_t l1 = vreinterpretq_u8_u32( vandq_u32( mask, px1 ) );
    uint8x16_t l2 = vreinterpretq_u8_u32( vandq_u32( mask, px2 ) );
    uint8x16_t l3 = vreinterpretq_u8_u32( vandq_u32( mask, px3 ) );

    uint8x16_t min0 = vminq_u8( l0, l1 );
    uint8x16_t min1 = vminq_u8( l2, l3 );
    uint8x16_t min2 = vminq_u8( min0, min1 );

    uint8x16_t max0 = vmaxq_u8( l0, l1 );
    uint8x16_t max1 = vmaxq_u8( l2, l3 );
    uint8x16_t max2 = vmaxq_u8( max0, max1 );

    uint8x16_t min3 = vreinterpretq_u8_u32( vrev64q_u32( vreinterpretq_u32_u8( min2 ) ) );
    uint8x16_t max3 = vreinterpretq_u8_u32( vrev64q_u32( vreinterpretq_u32_u8( max2 ) ) );

    uint8x16_t min4 = vminq_u8( min2, min3 );
    uint8x16_t max4 = vmaxq_u8( max2, max3 );

    uint8x16_t min5 = vcombine_u8( vget_high_u8( min4 ), vget_low_u8( min4 ) );
    uint8x16_t max5 = vcombine_u8( vget_high_u8( max4 ), vget_low_u8( max4 ) );

    uint8x16_t rmin = vminq_u8( min4, min5 );
    uint8x16_t rmax = vmaxq_u8( max4, max5 );

    uint8x16_t range1 = vsubq_u8( rmax, rmin );
    uint8x8_t range2 = vget_low_u8( range1 );
    uint8x8x2_t range3 = vzip_u8( range2, vdup_n_u8( 0 ) );
    uint16x4_t range4 = vreinterpret_u16_u8( range3.val[0] );

    uint16_t vrange1;
    uint16x4_t range5 = vpadd_u16( range4, range4 );
    uint16x4_t range6 = vpadd_u16( range5, range5 );
    vst1_lane_u16( &vrange1, range6, 0 );

    uint32_t vrange2 = ( 2 << 16 ) / uint32_t( vrange1 + 1 );
    uint16x8_t range = vdupq_n_u16( vrange2 );

    uint8x16_t inset = vshrq_n_u8( range1, 4 );
    uint8x16_t min = vaddq_u8( rmin, inset );
    uint8x16_t max = vsubq_u8( rmax, inset );

    uint8x16_t c0 = vsubq_u8( l0, rmin );
    uint8x16_t c1 = vsubq_u8( l1, rmin );
    uint8x16_t c2 = vsubq_u8( l2, rmin );
    uint8x16_t c3 = vsubq_u8( l3, rmin );

    uint16x8_t is0 = vpaddlq_u8( c0 );
    uint16x8_t is1 = vpaddlq_u8( c1 );
    uint16x8_t is2 = vpaddlq_u8( c2 );
    uint16x8_t is3 = vpaddlq_u8( c3 );

    uint16x4_t is4 = vpadd_u16( vget_low_u16( is0 ), vget_high_u16( is0 ) );
    uint16x4_t is5 = vpadd_u16( vget_low_u16( is1 ), vget_high_u16( is1 ) );
    uint16x4_t is6 = vpadd_u16( vget_low_u16( is2 ), vget_high_u16( is2 ) );
    uint16x4_t is7 = vpadd_u16( vget_low_u16( is3 ), vget_high_u16( is3 ) );

    uint16x8_t s0 = vcombine_u16( is4, is5 );
    uint16x8_t s1 = vcombine_u16( is6, is7 );

    uint16x8_t m0 = vreinterpretq_u16_s16( vqdmulhq_s16( vreinterpretq_s16_u16( s0 ), vreinterpretq_s16_u16( range ) ) );
    uint16x8_t m1 = vreinterpretq_u16_s16( vqdmulhq_s16( vreinterpretq_s16_u16( s1 ), vreinterpretq_s16_u16( range ) ) );

    uint8x8_t p00 = vmovn_u16( m0 );
    uint8x8_t p01 = vmovn_u16( m1 );
    uint8x16_t p0 = vcombine_u8( p00, p01 );

    uint32x4_t p1 = vaddq_u32( vshrq_n_u32( vreinterpretq_u32_u8( p0 ), 6 ), vshrq_n_u32( vreinterpretq_u32_u8( p0 ), 12 ) );
    uint32x4_t p2 = vaddq_u32( vshrq_n_u32( vreinterpretq_u32_u8( p0 ), 18 ), vreinterpretq_u32_u8( p0 ) );
    uint32x4_t p3 = vaddq_u32( p1, p2 );

    uint16x4x2_t p4 = vuzp_u16( vget_low_u16( vreinterpretq_u16_u32( p3 ) ), vget_high_u16( vreinterpretq_u16_u32( p3 ) ) );
    uint8x8x2_t p = vuzp_u8( vreinterpret_u8_u16( p4.val[0] ), vreinterpret_u8_u16( p4.val[0] ) );

    uint32_t vmin, vmax, vp;
    vst1q_lane_u32( &vmin, vreinterpretq_u32_u8( min ), 0 );
    vst1q_lane_u32( &vmax, vreinterpretq_u32_u8( max ), 0 );
    vst1_lane_u32( &vp, vreinterpret_u32_u8( p.val[0] ), 0 );

    return uint64_t( ( uint64_t( to565( vmin ) ) << 16 ) | to565( vmax ) | ( uint64_t( vp ) << 32 ) );
#  endif
#else
    uint32_t ref;
    memcpy( &ref, src, 4 );
    uint32_t refMask = ref & 0xF8FCF8;
    auto stmp = src + 4;
    for( int i=1; i<16; i++ )
    {
        uint32_t px;
        memcpy( &px, stmp, 4 );
        if( ( px & 0xF8FCF8 ) != refMask ) break;
        stmp += 4;
    }
    if( stmp == src + 64 )
    {
        return uint64_t( to565( ref ) ) << 16;
    }

    uint8_t min[3] = { src[0], src[1], src[2] };
    uint8_t max[3] = { src[0], src[1], src[2] };
    auto tmp = src + 4;
    for( int i=1; i<16; i++ )
    {
        for( int j=0; j<3; j++ )
        {
            if( tmp[j] < min[j] ) min[j] = tmp[j];
            else if( tmp[j] > max[j] ) max[j] = tmp[j];
        }
        tmp += 4;
    }

    const uint32_t range = DivTable[max[0] - min[0] + max[1] - min[1] + max[2] - min[2]];
    const uint32_t rmin = min[0] + min[1] + min[2];
    for( int i=0; i<3; i++ )
    {
        const uint8_t inset = ( max[i] - min[i] ) >> 4;
        min[i] += inset;
        max[i] -= inset;
    }

    uint32_t data = 0;
    for( int i=0; i<16; i++ )
    {
        const uint32_t c = src[0] + src[1] + src[2] - rmin;
        const uint8_t idx = ( c * range ) >> 16;
        data |= idx << (i*2);
        src += 4;
    }

    return uint64_t( ( uint64_t( to565( min[0], min[1], min[2] ) ) << 16 ) | to565( max[0], max[1], max[2] ) | ( uint64_t( data ) << 32 ) );
#endif
}

#ifdef __AVX2__
static tracy_force_inline void ProcessRGB_AVX( const uint8_t* src, char*& dst )
{
    __m256i px0 = _mm256_loadu_si256(((__m256i*)src) + 0);
    __m256i px1 = _mm256_loadu_si256(((__m256i*)src) + 1);
    __m256i px2 = _mm256_loadu_si256(((__m256i*)src) + 2);
    __m256i px3 = _mm256_loadu_si256(((__m256i*)src) + 3);

    __m256i smask = _mm256_set1_epi32( 0xF8FCF8 );
    __m256i sd0 = _mm256_and_si256( px0, smask );
    __m256i sd1 = _mm256_and_si256( px1, smask );
    __m256i sd2 = _mm256_and_si256( px2, smask );
    __m256i sd3 = _mm256_and_si256( px3, smask );

    __m256i sc = _mm256_shuffle_epi32(sd0, _MM_SHUFFLE(0, 0, 0, 0));

    __m256i sc0 = _mm256_cmpeq_epi8( sd0, sc );
    __m256i sc1 = _mm256_cmpeq_epi8( sd1, sc );
    __m256i sc2 = _mm256_cmpeq_epi8( sd2, sc );
    __m256i sc3 = _mm256_cmpeq_epi8( sd3, sc );

    __m256i sm0 = _mm256_and_si256( sc0, sc1 );
    __m256i sm1 = _mm256_and_si256( sc2, sc3 );
    __m256i sm = _mm256_and_si256( sm0, sm1 );

    const int64_t solid0 = 1 - _mm_testc_si128( _mm256_castsi256_si128( sm ), _mm_set1_epi32( -1 ) );
    const int64_t solid1 = 1 - _mm_testc_si128( _mm256_extracti128_si256( sm, 1 ), _mm_set1_epi32( -1 ) );

    if( solid0 + solid1 == 0 )
    {
        const auto c0 = uint64_t( to565( src[0], src[1], src[2] ) ) << 16;
        const auto c1 = uint64_t( to565( src[16], src[17], src[18] ) ) << 16;
        memcpy( dst, &c0, 8 );
        memcpy( dst+8, &c1, 8 );
        dst += 16;
        return;
    }

    __m256i amask = _mm256_set1_epi32( 0xFFFFFF );
    px0 = _mm256_and_si256( px0, amask );
    px1 = _mm256_and_si256( px1, amask );
    px2 = _mm256_and_si256( px2, amask );
    px3 = _mm256_and_si256( px3, amask );

    __m256i min0 = _mm256_min_epu8( px0, px1 );
    __m256i min1 = _mm256_min_epu8( px2, px3 );
    __m256i min2 = _mm256_min_epu8( min0, min1 );

    __m256i max0 = _mm256_max_epu8( px0, px1 );
    __m256i max1 = _mm256_max_epu8( px2, px3 );
    __m256i max2 = _mm256_max_epu8( max0, max1 );

    __m256i min3 = _mm256_shuffle_epi32( min2, _MM_SHUFFLE( 2, 3, 0, 1 ) );
    __m256i max3 = _mm256_shuffle_epi32( max2, _MM_SHUFFLE( 2, 3, 0, 1 ) );
    __m256i min4 = _mm256_min_epu8( min2, min3 );
    __m256i max4 = _mm256_max_epu8( max2, max3 );

    __m256i min5 = _mm256_shuffle_epi32( min4, _MM_SHUFFLE( 0, 0, 2, 2 ) );
    __m256i max5 = _mm256_shuffle_epi32( max4, _MM_SHUFFLE( 0, 0, 2, 2 ) );
    __m256i rmin = _mm256_min_epu8( min4, min5 );
    __m256i rmax = _mm256_max_epu8( max4, max5 );

    __m256i range1 = _mm256_subs_epu8( rmax, rmin );
    __m256i range2 = _mm256_sad_epu8( rmax, rmin );

    uint16_t vrange0 = DivTable[_mm256_cvtsi256_si32( range2 ) >> 1];
    uint16_t vrange1 = DivTable[_mm256_extract_epi16( range2, 8 ) >> 1];
    __m256i range00 = _mm256_set1_epi16( vrange0 );
    __m256i range = _mm256_inserti128_si256( range00, _mm_set1_epi16( vrange1 ), 1 );

    __m256i inset1 = _mm256_srli_epi16( range1, 4 );
    __m256i inset = _mm256_and_si256( inset1, _mm256_set1_epi8( 0xF ) );
    __m256i min = _mm256_adds_epu8( rmin, inset );
    __m256i max = _mm256_subs_epu8( rmax, inset );

    __m256i c0 = _mm256_subs_epu8( px0, rmin );
    __m256i c1 = _mm256_subs_epu8( px1, rmin );
    __m256i c2 = _mm256_subs_epu8( px2, rmin );
    __m256i c3 = _mm256_subs_epu8( px3, rmin );

    __m256i is0 = _mm256_maddubs_epi16( c0, _mm256_set1_epi8( 1 ) );
    __m256i is1 = _mm256_maddubs_epi16( c1, _mm256_set1_epi8( 1 ) );
    __m256i is2 = _mm256_maddubs_epi16( c2, _mm256_set1_epi8( 1 ) );
    __m256i is3 = _mm256_maddubs_epi16( c3, _mm256_set1_epi8( 1 ) );

    __m256i s0 = _mm256_hadd_epi16( is0, is1 );
    __m256i s1 = _mm256_hadd_epi16( is2, is3 );

    __m256i m0 = _mm256_mulhi_epu16( s0, range );
    __m256i m1 = _mm256_mulhi_epu16( s1, range );

    __m256i p0 = _mm256_packus_epi16( m0, m1 );

    __m256i p1 = _mm256_or_si256( _mm256_srai_epi32( p0, 6 ), _mm256_srai_epi32( p0, 12 ) );
    __m256i p2 = _mm256_or_si256( _mm256_srai_epi32( p0, 18 ), p0 );
    __m256i p3 = _mm256_or_si256( p1, p2 );
    __m256i p =_mm256_shuffle_epi8( p3, _mm256_set1_epi32( 0x0C080400 ) );

    __m256i mm0 = _mm256_unpacklo_epi8( _mm256_setzero_si256(), min );
    __m256i mm1 = _mm256_unpacklo_epi8( _mm256_setzero_si256(), max );
    __m256i mm2 = _mm256_unpacklo_epi64( mm1, mm0 );
    __m256i mmr = _mm256_slli_epi64( _mm256_srli_epi64( mm2, 11 ), 11 );
    __m256i mmg = _mm256_slli_epi64( _mm256_srli_epi64( mm2, 26 ), 5 );
    __m256i mmb = _mm256_srli_epi64( _mm256_slli_epi64( mm2, 16 ), 59 );
    __m256i mm3 = _mm256_or_si256( mmr, mmg );
    __m256i mm4 = _mm256_or_si256( mm3, mmb );
    __m256i mm5 = _mm256_shuffle_epi8( mm4, _mm256_set1_epi32( 0x09080100 ) );

    __m256i d0 = _mm256_unpacklo_epi32( mm5, p );
    __m256i d1 = _mm256_permute4x64_epi64( d0, _MM_SHUFFLE( 3, 2, 2, 0 ) );
    __m128i d2 = _mm256_castsi256_si128( d1 );

    __m128i mask = _mm_set_epi64x( 0xFFFF0000 | -solid1, 0xFFFF0000 | -solid0 );
    __m128i d3 = _mm_and_si128( d2, mask );
    _mm_storeu_si128( (__m128i*)dst, d3 );
    dst += 16;
}
#endif

void CompressImageDxt1( const char* src, char* dst, int w, int h )
{
    assert( (w % 4) == 0 && (h % 4) == 0 );

#ifdef __AVX2__
    if( w%8 == 0 )
    {
        uint32_t buf[8*4];
        int i = 0;

        auto blocks = w * h / 32;
        do
        {
            auto tmp = (char*)buf;
            memcpy( tmp,        src,          8*4 );
            memcpy( tmp + 8*4,  src + w * 4,  8*4 );
            memcpy( tmp + 16*4, src + w * 8,  8*4 );
            memcpy( tmp + 24*4, src + w * 12, 8*4 );
            src += 8*4;
            if( ++i == w/8 )
            {
                src += w * 3 * 4;
                i = 0;
            }

            ProcessRGB_AVX( (uint8_t*)buf, dst );
        }
        while( --blocks );
    }
    else
#endif
    {
        uint32_t buf[4*4];
        int i = 0;

        auto ptr = dst;
        auto blocks = w * h / 16;
        do
        {
            auto tmp = (char*)buf;
            memcpy( tmp,        src,          4*4 );
            memcpy( tmp + 4*4,  src + w * 4,  4*4 );
            memcpy( tmp + 8*4,  src + w * 8,  4*4 );
            memcpy( tmp + 12*4, src + w * 12, 4*4 );
            src += 4*4;
            if( ++i == w/4 )
            {
                src += w * 3 * 4;
                i = 0;
            }

            const auto c = ProcessRGB( (uint8_t*)buf );
            memcpy( ptr, &c, sizeof( uint64_t ) );
            ptr += sizeof( uint64_t );
        }
        while( --blocks );
    }
}

}

/*** End of inlined file: TracyDxt1.cpp ***/


/*** Start of inlined file: TracyAlloc.cpp ***/
#ifdef TRACY_ENABLE

#include <atomic>

namespace tracy
{

extern thread_local bool RpThreadInitDone;
extern std::atomic<int> RpInitDone;
extern std::atomic<int> RpInitLock;

tracy_no_inline static void InitRpmallocPlumbing()
{
    const auto done = RpInitDone.load( std::memory_order_acquire );
    if( !done )
    {
        int expected = 0;
        while( !RpInitLock.compare_exchange_weak( expected, 1, std::memory_order_release, std::memory_order_relaxed ) ) { expected = 0; YieldThread(); }
        const auto done = RpInitDone.load( std::memory_order_acquire );
        if( !done )
        {
            rpmalloc_initialize();
            RpInitDone.store( 1, std::memory_order_release );
        }
        RpInitLock.store( 0, std::memory_order_release );
    }
    rpmalloc_thread_initialize();
    RpThreadInitDone = true;
}

TRACY_API void InitRpmalloc()
{
    if( !RpThreadInitDone ) InitRpmallocPlumbing();
}

}

#endif

/*** End of inlined file: TracyAlloc.cpp ***/

#if TRACY_HAS_CALLSTACK == 2 || TRACY_HAS_CALLSTACK == 3 || TRACY_HAS_CALLSTACK == 4 || TRACY_HAS_CALLSTACK == 6

/*** Start of inlined file: alloc.cpp ***/

/*** Start of inlined file: config.h ***/
#include <climits>
#if __WORDSIZE == 64
#  define BACKTRACE_ELF_SIZE 64
#else
#  define BACKTRACE_ELF_SIZE 32
#endif

#define HAVE_DLFCN_H 1
#define HAVE_FCNTL 1
#define HAVE_INTTYPES_H 1
#define HAVE_LSTAT 1
#define HAVE_READLINK 1
#define HAVE_DL_ITERATE_PHDR 1
#define HAVE_ATOMIC_FUNCTIONS 1
#define HAVE_DECL_STRNLEN 1

#ifdef __APPLE__
#  define HAVE_MACH_O_DYLD_H 1
#elif defined BSD
#  define HAVE_KERN_PROC 1
#  define HAVE_KERN_PROC_ARGS 1
#endif

/*** End of inlined file: config.h ***/

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>


/*** Start of inlined file: internal.hpp ***/
#ifndef BACKTRACE_INTERNAL_H
#define BACKTRACE_INTERNAL_H

/* We assume that <sys/types.h> and "backtrace.h" have already been
   included.  */

#ifndef GCC_VERSION
# define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#endif

#if (GCC_VERSION < 2007)
# define __attribute__(x)
#endif

#ifndef ATTRIBUTE_UNUSED
# define ATTRIBUTE_UNUSED __attribute__ ((__unused__))
#endif

#ifndef ATTRIBUTE_MALLOC
# if (GCC_VERSION >= 2096)
#  define ATTRIBUTE_MALLOC __attribute__ ((__malloc__))
# else
#  define ATTRIBUTE_MALLOC
# endif
#endif

#ifndef ATTRIBUTE_FALLTHROUGH
# if (GCC_VERSION >= 7000)
#  define ATTRIBUTE_FALLTHROUGH __attribute__ ((__fallthrough__))
# else
#  define ATTRIBUTE_FALLTHROUGH
# endif
#endif

#ifndef HAVE_SYNC_FUNCTIONS

/* Define out the sync functions.  These should never be called if
   they are not available.  */

#define __sync_bool_compare_and_swap(A, B, C) (abort(), 1)
#define __sync_lock_test_and_set(A, B) (abort(), 0)
#define __sync_lock_release(A) abort()

#endif /* !defined (HAVE_SYNC_FUNCTIONS) */

#ifdef HAVE_ATOMIC_FUNCTIONS

/* We have the atomic builtin functions.  */

#define backtrace_atomic_load_pointer(p) \
    __atomic_load_n ((p), __ATOMIC_ACQUIRE)
#define backtrace_atomic_load_int(p) \
    __atomic_load_n ((p), __ATOMIC_ACQUIRE)
#define backtrace_atomic_store_pointer(p, v) \
    __atomic_store_n ((p), (v), __ATOMIC_RELEASE)
#define backtrace_atomic_store_size_t(p, v) \
    __atomic_store_n ((p), (v), __ATOMIC_RELEASE)
#define backtrace_atomic_store_int(p, v) \
    __atomic_store_n ((p), (v), __ATOMIC_RELEASE)

#else /* !defined (HAVE_ATOMIC_FUNCTIONS) */
#ifdef HAVE_SYNC_FUNCTIONS

/* We have the sync functions but not the atomic functions.  Define
   the atomic ones in terms of the sync ones.  */

extern void *backtrace_atomic_load_pointer (void *);
extern int backtrace_atomic_load_int (int *);
extern void backtrace_atomic_store_pointer (void *, void *);
extern void backtrace_atomic_store_size_t (size_t *, size_t);
extern void backtrace_atomic_store_int (int *, int);

#else /* !defined (HAVE_SYNC_FUNCTIONS) */

/* We have neither the sync nor the atomic functions.  These will
   never be called.  */

#define backtrace_atomic_load_pointer(p) (abort(), (void *) NULL)
#define backtrace_atomic_load_int(p) (abort(), 0)
#define backtrace_atomic_store_pointer(p, v) abort()
#define backtrace_atomic_store_size_t(p, v) abort()
#define backtrace_atomic_store_int(p, v) abort()

#endif /* !defined (HAVE_SYNC_FUNCTIONS) */
#endif /* !defined (HAVE_ATOMIC_FUNCTIONS) */

namespace tracy
{

/* The type of the function that collects file/line information.  This
   is like backtrace_pcinfo.  */

typedef int (*fileline) (struct backtrace_state *state, uintptr_t pc,
			 backtrace_full_callback callback,
			 backtrace_error_callback error_callback, void *data);

/* The type of the function that collects symbol information.  This is
   like backtrace_syminfo.  */

typedef void (*syminfo) (struct backtrace_state *state, uintptr_t pc,
			 backtrace_syminfo_callback callback,
			 backtrace_error_callback error_callback, void *data);

/* What the backtrace state pointer points to.  */

struct backtrace_state
{
  /* The name of the executable.  */
  const char *filename;
  /* Non-zero if threaded.  */
  int threaded;
  /* The master lock for fileline_fn, fileline_data, syminfo_fn,
     syminfo_data, fileline_initialization_failed and everything the
     data pointers point to.  */
  void *lock;
  /* The function that returns file/line information.  */
  fileline fileline_fn;
  /* The data to pass to FILELINE_FN.  */
  void *fileline_data;
  /* The function that returns symbol information.  */
  syminfo syminfo_fn;
  /* The data to pass to SYMINFO_FN.  */
  void *syminfo_data;
  /* Whether initializing the file/line information failed.  */
  int fileline_initialization_failed;
  /* The lock for the freelist.  */
  int lock_alloc;
  /* The freelist when using mmap.  */
  struct backtrace_freelist_struct *freelist;
};

/* Open a file for reading.  Returns -1 on error.  If DOES_NOT_EXIST
   is not NULL, *DOES_NOT_EXIST will be set to 0 normally and set to 1
   if the file does not exist.  If the file does not exist and
   DOES_NOT_EXIST is not NULL, the function will return -1 and will
   not call ERROR_CALLBACK.  On other errors, or if DOES_NOT_EXIST is
   NULL, the function will call ERROR_CALLBACK before returning.  */
extern int backtrace_open (const char *filename,
			   backtrace_error_callback error_callback,
			   void *data,
			   int *does_not_exist);

/* A view of the contents of a file.  This supports mmap when
   available.  A view will remain in memory even after backtrace_close
   is called on the file descriptor from which the view was
   obtained.  */

struct backtrace_view
{
  /* The data that the caller requested.  */
  const void *data;
  /* The base of the view.  */
  void *base;
  /* The total length of the view.  */
  size_t len;
};

/* Create a view of SIZE bytes from DESCRIPTOR at OFFSET.  Store the
   result in *VIEW.  Returns 1 on success, 0 on error.  */
extern int backtrace_get_view (struct backtrace_state *state, int descriptor,
			       off_t offset, uint64_t size,
			       backtrace_error_callback error_callback,
			       void *data, struct backtrace_view *view);

/* Release a view created by backtrace_get_view.  */
extern void backtrace_release_view (struct backtrace_state *state,
				    struct backtrace_view *view,
				    backtrace_error_callback error_callback,
				    void *data);

/* Close a file opened by backtrace_open.  Returns 1 on success, 0 on
   error.  */

extern int backtrace_close (int descriptor,
			    backtrace_error_callback error_callback,
			    void *data);

/* Sort without using memory.  */

extern void backtrace_qsort (void *base, size_t count, size_t size,
			     int (*compar) (const void *, const void *));

/* Allocate memory.  This is like malloc.  If ERROR_CALLBACK is NULL,
   this does not report an error, it just returns NULL.  */

extern void *backtrace_alloc (struct backtrace_state *state, size_t size,
			      backtrace_error_callback error_callback,
			      void *data) ATTRIBUTE_MALLOC;

/* Free memory allocated by backtrace_alloc.  If ERROR_CALLBACK is
   NULL, this does not report an error.  */

extern void backtrace_free (struct backtrace_state *state, void *mem,
			    size_t size,
			    backtrace_error_callback error_callback,
			    void *data);

/* A growable vector of some struct.  This is used for more efficient
   allocation when we don't know the final size of some group of data
   that we want to represent as an array.  */

struct backtrace_vector
{
  /* The base of the vector.  */
  void *base;
  /* The number of bytes in the vector.  */
  size_t size;
  /* The number of bytes available at the current allocation.  */
  size_t alc;
};

/* Grow VEC by SIZE bytes.  Return a pointer to the newly allocated
   bytes.  Note that this may move the entire vector to a new memory
   location.  Returns NULL on failure.  */

extern void *backtrace_vector_grow (struct backtrace_state *state, size_t size,
				    backtrace_error_callback error_callback,
				    void *data,
				    struct backtrace_vector *vec);

/* Finish the current allocation on VEC.  Prepare to start a new
   allocation.  The finished allocation will never be freed.  Returns
   a pointer to the base of the finished entries, or NULL on
   failure.  */

extern void* backtrace_vector_finish (struct backtrace_state *state,
				      struct backtrace_vector *vec,
				      backtrace_error_callback error_callback,
				      void *data);

/* Release any extra space allocated for VEC.  This may change
   VEC->base.  Returns 1 on success, 0 on failure.  */

extern int backtrace_vector_release (struct backtrace_state *state,
				     struct backtrace_vector *vec,
				     backtrace_error_callback error_callback,
				     void *data);

/* Free the space managed by VEC.  This will reset VEC.  */

static inline void
backtrace_vector_free (struct backtrace_state *state,
		       struct backtrace_vector *vec,
		       backtrace_error_callback error_callback, void *data)
{
  vec->alc += vec->size;
  vec->size = 0;
  backtrace_vector_release (state, vec, error_callback, data);
}

/* Read initial debug data from a descriptor, and set the
   fileline_data, syminfo_fn, and syminfo_data fields of STATE.
   Return the fileln_fn field in *FILELN_FN--this is done this way so
   that the synchronization code is only implemented once.  This is
   called after the descriptor has first been opened.  It will close
   the descriptor if it is no longer needed.  Returns 1 on success, 0
   on error.  There will be multiple implementations of this function,
   for different file formats.  Each system will compile the
   appropriate one.  */

extern int backtrace_initialize (struct backtrace_state *state,
				 const char *filename,
				 int descriptor,
				 backtrace_error_callback error_callback,
				 void *data,
				 fileline *fileline_fn);

/* An enum for the DWARF sections we care about.  */

enum dwarf_section
{
  DEBUG_INFO,
  DEBUG_LINE,
  DEBUG_ABBREV,
  DEBUG_RANGES,
  DEBUG_STR,
  DEBUG_ADDR,
  DEBUG_STR_OFFSETS,
  DEBUG_LINE_STR,
  DEBUG_RNGLISTS,

  DEBUG_MAX
};

/* Data for the DWARF sections we care about.  */

struct dwarf_sections
{
  const unsigned char *data[DEBUG_MAX];
  size_t size[DEBUG_MAX];
};

/* DWARF data read from a file, used for .gnu_debugaltlink.  */

struct dwarf_data;

/* Add file/line information for a DWARF module.  */

extern int backtrace_dwarf_add (struct backtrace_state *state,
				uintptr_t base_address,
				const struct dwarf_sections *dwarf_sections,
				int is_bigendian,
				struct dwarf_data *fileline_altlink,
				backtrace_error_callback error_callback,
				void *data, fileline *fileline_fn,
				struct dwarf_data **fileline_entry);

/* A data structure to pass to backtrace_syminfo_to_full.  */

struct backtrace_call_full
{
  backtrace_full_callback full_callback;
  backtrace_error_callback full_error_callback;
  void *full_data;
  int ret;
};

/* A backtrace_syminfo_callback that can call into a
   backtrace_full_callback, used when we have a symbol table but no
   debug info.  */

extern void backtrace_syminfo_to_full_callback (void *data, uintptr_t pc,
						const char *symname,
						uintptr_t symval,
						uintptr_t symsize);

/* An error callback that corresponds to
   backtrace_syminfo_to_full_callback.  */

extern void backtrace_syminfo_to_full_error_callback (void *, const char *,
						      int);

/* A test-only hook for elf_uncompress_zdebug.  */

extern int backtrace_uncompress_zdebug (struct backtrace_state *,
					const unsigned char *compressed,
					size_t compressed_size,
					backtrace_error_callback, void *data,
					unsigned char **uncompressed,
					size_t *uncompressed_size);

/* A test-only hook for elf_uncompress_lzma.  */

extern int backtrace_uncompress_lzma (struct backtrace_state *,
				      const unsigned char *compressed,
				      size_t compressed_size,
				      backtrace_error_callback, void *data,
				      unsigned char **uncompressed,
				      size_t *uncompressed_size);

}

#endif

/*** End of inlined file: internal.hpp ***/

namespace tracy
{

/* Allocation routines to use on systems that do not support anonymous
   mmap.  This implementation just uses malloc, which means that the
   backtrace functions may not be safely invoked from a signal
   handler.  */

/* Allocate memory like malloc.  If ERROR_CALLBACK is NULL, don't
   report an error.  */

void *
backtrace_alloc (struct backtrace_state *state ATTRIBUTE_UNUSED,
		 size_t size, backtrace_error_callback error_callback,
		 void *data)
{
  void *ret;

  ret = tracy_malloc (size);
  if (ret == NULL)
    {
      if (error_callback)
	error_callback (data, "malloc", errno);
    }
  return ret;
}

/* Free memory.  */

void
backtrace_free (struct backtrace_state *state ATTRIBUTE_UNUSED,
		void *p, size_t size ATTRIBUTE_UNUSED,
		backtrace_error_callback error_callback ATTRIBUTE_UNUSED,
		void *data ATTRIBUTE_UNUSED)
{
  tracy_free (p);
}

/* Grow VEC by SIZE bytes.  */

void *
backtrace_vector_grow (struct backtrace_state *state ATTRIBUTE_UNUSED,
		       size_t size, backtrace_error_callback error_callback,
		       void *data, struct backtrace_vector *vec)
{
  void *ret;

  if (size > vec->alc)
    {
      size_t alc;
      void *base;

      if (vec->size == 0)
	alc = 32 * size;
      else if (vec->size >= 4096)
	alc = vec->size + 4096;
      else
	alc = 2 * vec->size;

      if (alc < vec->size + size)
	alc = vec->size + size;

      base = tracy_realloc (vec->base, alc);
      if (base == NULL)
	{
	  error_callback (data, "realloc", errno);
	  return NULL;
	}

      vec->base = base;
      vec->alc = alc - vec->size;
    }

  ret = (char *) vec->base + vec->size;
  vec->size += size;
  vec->alc -= size;
  return ret;
}

/* Finish the current allocation on VEC.  */

void *
backtrace_vector_finish (struct backtrace_state *state,
			 struct backtrace_vector *vec,
			 backtrace_error_callback error_callback,
			 void *data)
{
  void *ret;

  /* With this allocator we call realloc in backtrace_vector_grow,
     which means we can't easily reuse the memory here.  So just
     release it.  */
  if (!backtrace_vector_release (state, vec, error_callback, data))
    return NULL;
  ret = vec->base;
  vec->base = NULL;
  vec->size = 0;
  vec->alc = 0;
  return ret;
}

/* Release any extra space allocated for VEC.  */

int
backtrace_vector_release (struct backtrace_state *state ATTRIBUTE_UNUSED,
			  struct backtrace_vector *vec,
			  backtrace_error_callback error_callback,
			  void *data)
{
  vec->alc = 0;

  if (vec->size == 0)
    {
      /* As of C17, realloc with size 0 is marked as an obsolescent feature, use
	 free instead.  */
      tracy_free (vec->base);
      vec->base = NULL;
      return 1;
    }

  vec->base = tracy_realloc (vec->base, vec->size);
  if (vec->base == NULL)
    {
      error_callback (data, "realloc", errno);
      return 0;
    }

  return 1;
}

}

/*** End of inlined file: alloc.cpp ***/



/*** Start of inlined file: dwarf.cpp ***/

/*** Start of inlined file: config.h ***/
#include <climits>
#if __WORDSIZE == 64
#  define BACKTRACE_ELF_SIZE 64
#else
#  define BACKTRACE_ELF_SIZE 32
#endif

#define HAVE_DLFCN_H 1
#define HAVE_FCNTL 1
#define HAVE_INTTYPES_H 1
#define HAVE_LSTAT 1
#define HAVE_READLINK 1
#define HAVE_DL_ITERATE_PHDR 1
#define HAVE_ATOMIC_FUNCTIONS 1
#define HAVE_DECL_STRNLEN 1

#ifdef __APPLE__
#  define HAVE_MACH_O_DYLD_H 1
#elif defined BSD
#  define HAVE_KERN_PROC 1
#  define HAVE_KERN_PROC_ARGS 1
#endif

/*** End of inlined file: config.h ***/

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


/*** Start of inlined file: filenames.hpp ***/
#ifndef GCC_VERSION
# define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#endif

#if (GCC_VERSION < 2007)
# define __attribute__(x)
#endif

#ifndef ATTRIBUTE_UNUSED
# define ATTRIBUTE_UNUSED __attribute__ ((__unused__))
#endif

#if defined(__MSDOS__) || defined(_WIN32) || defined(__OS2__) || defined (__CYGWIN__)
# define IS_DIR_SEPARATOR(c) ((c) == '/' || (c) == '\\')
# define HAS_DRIVE_SPEC(f) ((f)[0] != '\0' && (f)[1] == ':')
# define IS_ABSOLUTE_PATH(f) (IS_DIR_SEPARATOR((f)[0]) || HAS_DRIVE_SPEC(f))
#else
# define IS_DIR_SEPARATOR(c) ((c) == '/')
# define IS_ABSOLUTE_PATH(f) (IS_DIR_SEPARATOR((f)[0]))
#endif

/*** End of inlined file: filenames.hpp ***/

namespace tracy
{

/* DWARF constants.  */

enum dwarf_tag {
  DW_TAG_entry_point = 0x3,
  DW_TAG_compile_unit = 0x11,
  DW_TAG_inlined_subroutine = 0x1d,
  DW_TAG_subprogram = 0x2e,
};

enum dwarf_form {
  DW_FORM_addr = 0x01,
  DW_FORM_block2 = 0x03,
  DW_FORM_block4 = 0x04,
  DW_FORM_data2 = 0x05,
  DW_FORM_data4 = 0x06,
  DW_FORM_data8 = 0x07,
  DW_FORM_string = 0x08,
  DW_FORM_block = 0x09,
  DW_FORM_block1 = 0x0a,
  DW_FORM_data1 = 0x0b,
  DW_FORM_flag = 0x0c,
  DW_FORM_sdata = 0x0d,
  DW_FORM_strp = 0x0e,
  DW_FORM_udata = 0x0f,
  DW_FORM_ref_addr = 0x10,
  DW_FORM_ref1 = 0x11,
  DW_FORM_ref2 = 0x12,
  DW_FORM_ref4 = 0x13,
  DW_FORM_ref8 = 0x14,
  DW_FORM_ref_udata = 0x15,
  DW_FORM_indirect = 0x16,
  DW_FORM_sec_offset = 0x17,
  DW_FORM_exprloc = 0x18,
  DW_FORM_flag_present = 0x19,
  DW_FORM_ref_sig8 = 0x20,
  DW_FORM_strx = 0x1a,
  DW_FORM_addrx = 0x1b,
  DW_FORM_ref_sup4 = 0x1c,
  DW_FORM_strp_sup = 0x1d,
  DW_FORM_data16 = 0x1e,
  DW_FORM_line_strp = 0x1f,
  DW_FORM_implicit_const = 0x21,
  DW_FORM_loclistx = 0x22,
  DW_FORM_rnglistx = 0x23,
  DW_FORM_ref_sup8 = 0x24,
  DW_FORM_strx1 = 0x25,
  DW_FORM_strx2 = 0x26,
  DW_FORM_strx3 = 0x27,
  DW_FORM_strx4 = 0x28,
  DW_FORM_addrx1 = 0x29,
  DW_FORM_addrx2 = 0x2a,
  DW_FORM_addrx3 = 0x2b,
  DW_FORM_addrx4 = 0x2c,
  DW_FORM_GNU_addr_index = 0x1f01,
  DW_FORM_GNU_str_index = 0x1f02,
  DW_FORM_GNU_ref_alt = 0x1f20,
  DW_FORM_GNU_strp_alt = 0x1f21
};

enum dwarf_attribute {
  DW_AT_sibling = 0x01,
  DW_AT_location = 0x02,
  DW_AT_name = 0x03,
  DW_AT_ordering = 0x09,
  DW_AT_subscr_data = 0x0a,
  DW_AT_byte_size = 0x0b,
  DW_AT_bit_offset = 0x0c,
  DW_AT_bit_size = 0x0d,
  DW_AT_element_list = 0x0f,
  DW_AT_stmt_list = 0x10,
  DW_AT_low_pc = 0x11,
  DW_AT_high_pc = 0x12,
  DW_AT_language = 0x13,
  DW_AT_member = 0x14,
  DW_AT_discr = 0x15,
  DW_AT_discr_value = 0x16,
  DW_AT_visibility = 0x17,
  DW_AT_import = 0x18,
  DW_AT_string_length = 0x19,
  DW_AT_common_reference = 0x1a,
  DW_AT_comp_dir = 0x1b,
  DW_AT_const_value = 0x1c,
  DW_AT_containing_type = 0x1d,
  DW_AT_default_value = 0x1e,
  DW_AT_inline = 0x20,
  DW_AT_is_optional = 0x21,
  DW_AT_lower_bound = 0x22,
  DW_AT_producer = 0x25,
  DW_AT_prototyped = 0x27,
  DW_AT_return_addr = 0x2a,
  DW_AT_start_scope = 0x2c,
  DW_AT_bit_stride = 0x2e,
  DW_AT_upper_bound = 0x2f,
  DW_AT_abstract_origin = 0x31,
  DW_AT_accessibility = 0x32,
  DW_AT_address_class = 0x33,
  DW_AT_artificial = 0x34,
  DW_AT_base_types = 0x35,
  DW_AT_calling_convention = 0x36,
  DW_AT_count = 0x37,
  DW_AT_data_member_location = 0x38,
  DW_AT_decl_column = 0x39,
  DW_AT_decl_file = 0x3a,
  DW_AT_decl_line = 0x3b,
  DW_AT_declaration = 0x3c,
  DW_AT_discr_list = 0x3d,
  DW_AT_encoding = 0x3e,
  DW_AT_external = 0x3f,
  DW_AT_frame_base = 0x40,
  DW_AT_friend = 0x41,
  DW_AT_identifier_case = 0x42,
  DW_AT_macro_info = 0x43,
  DW_AT_namelist_items = 0x44,
  DW_AT_priority = 0x45,
  DW_AT_segment = 0x46,
  DW_AT_specification = 0x47,
  DW_AT_static_link = 0x48,
  DW_AT_type = 0x49,
  DW_AT_use_location = 0x4a,
  DW_AT_variable_parameter = 0x4b,
  DW_AT_virtuality = 0x4c,
  DW_AT_vtable_elem_location = 0x4d,
  DW_AT_allocated = 0x4e,
  DW_AT_associated = 0x4f,
  DW_AT_data_location = 0x50,
  DW_AT_byte_stride = 0x51,
  DW_AT_entry_pc = 0x52,
  DW_AT_use_UTF8 = 0x53,
  DW_AT_extension = 0x54,
  DW_AT_ranges = 0x55,
  DW_AT_trampoline = 0x56,
  DW_AT_call_column = 0x57,
  DW_AT_call_file = 0x58,
  DW_AT_call_line = 0x59,
  DW_AT_description = 0x5a,
  DW_AT_binary_scale = 0x5b,
  DW_AT_decimal_scale = 0x5c,
  DW_AT_small = 0x5d,
  DW_AT_decimal_sign = 0x5e,
  DW_AT_digit_count = 0x5f,
  DW_AT_picture_string = 0x60,
  DW_AT_mutable = 0x61,
  DW_AT_threads_scaled = 0x62,
  DW_AT_explicit = 0x63,
  DW_AT_object_pointer = 0x64,
  DW_AT_endianity = 0x65,
  DW_AT_elemental = 0x66,
  DW_AT_pure = 0x67,
  DW_AT_recursive = 0x68,
  DW_AT_signature = 0x69,
  DW_AT_main_subprogram = 0x6a,
  DW_AT_data_bit_offset = 0x6b,
  DW_AT_const_expr = 0x6c,
  DW_AT_enum_class = 0x6d,
  DW_AT_linkage_name = 0x6e,
  DW_AT_string_length_bit_size = 0x6f,
  DW_AT_string_length_byte_size = 0x70,
  DW_AT_rank = 0x71,
  DW_AT_str_offsets_base = 0x72,
  DW_AT_addr_base = 0x73,
  DW_AT_rnglists_base = 0x74,
  DW_AT_dwo_name = 0x76,
  DW_AT_reference = 0x77,
  DW_AT_rvalue_reference = 0x78,
  DW_AT_macros = 0x79,
  DW_AT_call_all_calls = 0x7a,
  DW_AT_call_all_source_calls = 0x7b,
  DW_AT_call_all_tail_calls = 0x7c,
  DW_AT_call_return_pc = 0x7d,
  DW_AT_call_value = 0x7e,
  DW_AT_call_origin = 0x7f,
  DW_AT_call_parameter = 0x80,
  DW_AT_call_pc = 0x81,
  DW_AT_call_tail_call = 0x82,
  DW_AT_call_target = 0x83,
  DW_AT_call_target_clobbered = 0x84,
  DW_AT_call_data_location = 0x85,
  DW_AT_call_data_value = 0x86,
  DW_AT_noreturn = 0x87,
  DW_AT_alignment = 0x88,
  DW_AT_export_symbols = 0x89,
  DW_AT_deleted = 0x8a,
  DW_AT_defaulted = 0x8b,
  DW_AT_loclists_base = 0x8c,
  DW_AT_lo_user = 0x2000,
  DW_AT_hi_user = 0x3fff,
  DW_AT_MIPS_fde = 0x2001,
  DW_AT_MIPS_loop_begin = 0x2002,
  DW_AT_MIPS_tail_loop_begin = 0x2003,
  DW_AT_MIPS_epilog_begin = 0x2004,
  DW_AT_MIPS_loop_unroll_factor = 0x2005,
  DW_AT_MIPS_software_pipeline_depth = 0x2006,
  DW_AT_MIPS_linkage_name = 0x2007,
  DW_AT_MIPS_stride = 0x2008,
  DW_AT_MIPS_abstract_name = 0x2009,
  DW_AT_MIPS_clone_origin = 0x200a,
  DW_AT_MIPS_has_inlines = 0x200b,
  DW_AT_HP_block_index = 0x2000,
  DW_AT_HP_unmodifiable = 0x2001,
  DW_AT_HP_prologue = 0x2005,
  DW_AT_HP_epilogue = 0x2008,
  DW_AT_HP_actuals_stmt_list = 0x2010,
  DW_AT_HP_proc_per_section = 0x2011,
  DW_AT_HP_raw_data_ptr = 0x2012,
  DW_AT_HP_pass_by_reference = 0x2013,
  DW_AT_HP_opt_level = 0x2014,
  DW_AT_HP_prof_version_id = 0x2015,
  DW_AT_HP_opt_flags = 0x2016,
  DW_AT_HP_cold_region_low_pc = 0x2017,
  DW_AT_HP_cold_region_high_pc = 0x2018,
  DW_AT_HP_all_variables_modifiable = 0x2019,
  DW_AT_HP_linkage_name = 0x201a,
  DW_AT_HP_prof_flags = 0x201b,
  DW_AT_HP_unit_name = 0x201f,
  DW_AT_HP_unit_size = 0x2020,
  DW_AT_HP_widened_byte_size = 0x2021,
  DW_AT_HP_definition_points = 0x2022,
  DW_AT_HP_default_location = 0x2023,
  DW_AT_HP_is_result_param = 0x2029,
  DW_AT_sf_names = 0x2101,
  DW_AT_src_info = 0x2102,
  DW_AT_mac_info = 0x2103,
  DW_AT_src_coords = 0x2104,
  DW_AT_body_begin = 0x2105,
  DW_AT_body_end = 0x2106,
  DW_AT_GNU_vector = 0x2107,
  DW_AT_GNU_guarded_by = 0x2108,
  DW_AT_GNU_pt_guarded_by = 0x2109,
  DW_AT_GNU_guarded = 0x210a,
  DW_AT_GNU_pt_guarded = 0x210b,
  DW_AT_GNU_locks_excluded = 0x210c,
  DW_AT_GNU_exclusive_locks_required = 0x210d,
  DW_AT_GNU_shared_locks_required = 0x210e,
  DW_AT_GNU_odr_signature = 0x210f,
  DW_AT_GNU_template_name = 0x2110,
  DW_AT_GNU_call_site_value = 0x2111,
  DW_AT_GNU_call_site_data_value = 0x2112,
  DW_AT_GNU_call_site_target = 0x2113,
  DW_AT_GNU_call_site_target_clobbered = 0x2114,
  DW_AT_GNU_tail_call = 0x2115,
  DW_AT_GNU_all_tail_call_sites = 0x2116,
  DW_AT_GNU_all_call_sites = 0x2117,
  DW_AT_GNU_all_source_call_sites = 0x2118,
  DW_AT_GNU_macros = 0x2119,
  DW_AT_GNU_deleted = 0x211a,
  DW_AT_GNU_dwo_name = 0x2130,
  DW_AT_GNU_dwo_id = 0x2131,
  DW_AT_GNU_ranges_base = 0x2132,
  DW_AT_GNU_addr_base = 0x2133,
  DW_AT_GNU_pubnames = 0x2134,
  DW_AT_GNU_pubtypes = 0x2135,
  DW_AT_GNU_discriminator = 0x2136,
  DW_AT_GNU_locviews = 0x2137,
  DW_AT_GNU_entry_view = 0x2138,
  DW_AT_VMS_rtnbeg_pd_address = 0x2201,
  DW_AT_use_GNAT_descriptive_type = 0x2301,
  DW_AT_GNAT_descriptive_type = 0x2302,
  DW_AT_GNU_numerator = 0x2303,
  DW_AT_GNU_denominator = 0x2304,
  DW_AT_GNU_bias = 0x2305,
  DW_AT_upc_threads_scaled = 0x3210,
  DW_AT_PGI_lbase = 0x3a00,
  DW_AT_PGI_soffset = 0x3a01,
  DW_AT_PGI_lstride = 0x3a02,
  DW_AT_APPLE_optimized = 0x3fe1,
  DW_AT_APPLE_flags = 0x3fe2,
  DW_AT_APPLE_isa = 0x3fe3,
  DW_AT_APPLE_block = 0x3fe4,
  DW_AT_APPLE_major_runtime_vers = 0x3fe5,
  DW_AT_APPLE_runtime_class = 0x3fe6,
  DW_AT_APPLE_omit_frame_ptr = 0x3fe7,
  DW_AT_APPLE_property_name = 0x3fe8,
  DW_AT_APPLE_property_getter = 0x3fe9,
  DW_AT_APPLE_property_setter = 0x3fea,
  DW_AT_APPLE_property_attribute = 0x3feb,
  DW_AT_APPLE_objc_complete_type = 0x3fec,
  DW_AT_APPLE_property = 0x3fed
};

enum dwarf_line_number_op {
  DW_LNS_extended_op = 0x0,
  DW_LNS_copy = 0x1,
  DW_LNS_advance_pc = 0x2,
  DW_LNS_advance_line = 0x3,
  DW_LNS_set_file = 0x4,
  DW_LNS_set_column = 0x5,
  DW_LNS_negate_stmt = 0x6,
  DW_LNS_set_basic_block = 0x7,
  DW_LNS_const_add_pc = 0x8,
  DW_LNS_fixed_advance_pc = 0x9,
  DW_LNS_set_prologue_end = 0xa,
  DW_LNS_set_epilogue_begin = 0xb,
  DW_LNS_set_isa = 0xc,
};

enum dwarf_extended_line_number_op {
  DW_LNE_end_sequence = 0x1,
  DW_LNE_set_address = 0x2,
  DW_LNE_define_file = 0x3,
  DW_LNE_set_discriminator = 0x4,
};

enum dwarf_line_number_content_type {
  DW_LNCT_path = 0x1,
  DW_LNCT_directory_index = 0x2,
  DW_LNCT_timestamp = 0x3,
  DW_LNCT_size = 0x4,
  DW_LNCT_MD5 = 0x5,
  DW_LNCT_lo_user = 0x2000,
  DW_LNCT_hi_user = 0x3fff
};

enum dwarf_range_list_entry {
  DW_RLE_end_of_list = 0x00,
  DW_RLE_base_addressx = 0x01,
  DW_RLE_startx_endx = 0x02,
  DW_RLE_startx_length = 0x03,
  DW_RLE_offset_pair = 0x04,
  DW_RLE_base_address = 0x05,
  DW_RLE_start_end = 0x06,
  DW_RLE_start_length = 0x07
};

enum dwarf_unit_type {
  DW_UT_compile = 0x01,
  DW_UT_type = 0x02,
  DW_UT_partial = 0x03,
  DW_UT_skeleton = 0x04,
  DW_UT_split_compile = 0x05,
  DW_UT_split_type = 0x06,
  DW_UT_lo_user = 0x80,
  DW_UT_hi_user = 0xff
};

#if !defined(HAVE_DECL_STRNLEN) || !HAVE_DECL_STRNLEN

/* If strnlen is not declared, provide our own version.  */

static size_t
xstrnlen (const char *s, size_t maxlen)
{
  size_t i;

  for (i = 0; i < maxlen; ++i)
    if (s[i] == '\0')
      break;
  return i;
}

#define strnlen xstrnlen

#endif

/* A buffer to read DWARF info.  */

struct dwarf_buf
{
  /* Buffer name for error messages.  */
  const char *name;
  /* Start of the buffer.  */
  const unsigned char *start;
  /* Next byte to read.  */
  const unsigned char *buf;
  /* The number of bytes remaining.  */
  size_t left;
  /* Whether the data is big-endian.  */
  int is_bigendian;
  /* Error callback routine.  */
  backtrace_error_callback error_callback;
  /* Data for error_callback.  */
  void *data;
  /* Non-zero if we've reported an underflow error.  */
  int reported_underflow;
};

/* A single attribute in a DWARF abbreviation.  */

struct attr
{
  /* The attribute name.  */
  enum dwarf_attribute name;
  /* The attribute form.  */
  enum dwarf_form form;
  /* The attribute value, for DW_FORM_implicit_const.  */
  int64_t val;
};

/* A single DWARF abbreviation.  */

struct abbrev
{
  /* The abbrev code--the number used to refer to the abbrev.  */
  uint64_t code;
  /* The entry tag.  */
  enum dwarf_tag tag;
  /* Non-zero if this abbrev has child entries.  */
  int has_children;
  /* The number of attributes.  */
  size_t num_attrs;
  /* The attributes.  */
  struct attr *attrs;
};

/* The DWARF abbreviations for a compilation unit.  This structure
   only exists while reading the compilation unit.  Most DWARF readers
   seem to a hash table to map abbrev ID's to abbrev entries.
   However, we primarily care about GCC, and GCC simply issues ID's in
   numerical order starting at 1.  So we simply keep a sorted vector,
   and try to just look up the code.  */

struct abbrevs
{
  /* The number of abbrevs in the vector.  */
  size_t num_abbrevs;
  /* The abbrevs, sorted by the code field.  */
  struct abbrev *abbrevs;
};

/* The different kinds of attribute values.  */

enum attr_val_encoding
{
  /* No attribute value.  */
  ATTR_VAL_NONE,
  /* An address.  */
  ATTR_VAL_ADDRESS,
  /* An index into the .debug_addr section, whose value is relative to
   * the DW_AT_addr_base attribute of the compilation unit.  */
  ATTR_VAL_ADDRESS_INDEX,
  /* A unsigned integer.  */
  ATTR_VAL_UINT,
  /* A sigd integer.  */
  ATTR_VAL_SINT,
  /* A string.  */
  ATTR_VAL_STRING,
  /* An index into the .debug_str_offsets section.  */
  ATTR_VAL_STRING_INDEX,
  /* An offset to other data in the containing unit.  */
  ATTR_VAL_REF_UNIT,
  /* An offset to other data within the .debug_info section.  */
  ATTR_VAL_REF_INFO,
  /* An offset to other data within the alt .debug_info section.  */
  ATTR_VAL_REF_ALT_INFO,
  /* An offset to data in some other section.  */
  ATTR_VAL_REF_SECTION,
  /* A type signature.  */
  ATTR_VAL_REF_TYPE,
  /* An index into the .debug_rnglists section.  */
  ATTR_VAL_RNGLISTS_INDEX,
  /* A block of data (not represented).  */
  ATTR_VAL_BLOCK,
  /* An expression (not represented).  */
  ATTR_VAL_EXPR,
};

/* An attribute value.  */

struct attr_val
{
  /* How the value is stored in the field u.  */
  enum attr_val_encoding encoding;
  union
  {
    /* ATTR_VAL_ADDRESS*, ATTR_VAL_UINT, ATTR_VAL_REF*.  */
    uint64_t uint;
    /* ATTR_VAL_SINT.  */
    int64_t sint;
    /* ATTR_VAL_STRING.  */
    const char *string;
    /* ATTR_VAL_BLOCK not stored.  */
  } u;
};

/* The line number program header.  */

struct line_header
{
  /* The version of the line number information.  */
  int version;
  /* Address size.  */
  int addrsize;
  /* The minimum instruction length.  */
  unsigned int min_insn_len;
  /* The maximum number of ops per instruction.  */
  unsigned int max_ops_per_insn;
  /* The line base for special opcodes.  */
  int line_base;
  /* The line range for special opcodes.  */
  unsigned int line_range;
  /* The opcode base--the first special opcode.  */
  unsigned int opcode_base;
  /* Opcode lengths, indexed by opcode - 1.  */
  const unsigned char *opcode_lengths;
  /* The number of directory entries.  */
  size_t dirs_count;
  /* The directory entries.  */
  const char **dirs;
  /* The number of filenames.  */
  size_t filenames_count;
  /* The filenames.  */
  const char **filenames;
};

/* A format description from a line header.  */

struct line_header_format
{
  int lnct;		/* LNCT code.  */
  enum dwarf_form form;	/* Form of entry data.  */
};

/* Map a single PC value to a file/line.  We will keep a vector of
   these sorted by PC value.  Each file/line will be correct from the
   PC up to the PC of the next entry if there is one.  We allocate one
   extra entry at the end so that we can use bsearch.  */

struct line
{
  /* PC.  */
  uintptr_t pc;
  /* File name.  Many entries in the array are expected to point to
     the same file name.  */
  const char *filename;
  /* Line number.  */
  int lineno;
  /* Index of the object in the original array read from the DWARF
     section, before it has been sorted.  The index makes it possible
     to use Quicksort and maintain stability.  */
  int idx;
};

/* A growable vector of line number information.  This is used while
   reading the line numbers.  */

struct line_vector
{
  /* Memory.  This is an array of struct line.  */
  struct backtrace_vector vec;
  /* Number of valid mappings.  */
  size_t count;
};

/* A function described in the debug info.  */

struct function
{
  /* The name of the function.  */
  const char *name;
  /* If this is an inlined function, the filename of the call
     site.  */
  const char *caller_filename;
  /* If this is an inlined function, the line number of the call
     site.  */
  int caller_lineno;
  /* Map PC ranges to inlined functions.  */
  struct function_addrs *function_addrs;
  size_t function_addrs_count;
};

/* An address range for a function.  This maps a PC value to a
   specific function.  */

struct function_addrs
{
  /* Range is LOW <= PC < HIGH.  */
  uint64_t low;
  uint64_t high;
  /* Function for this address range.  */
  struct function *function;
};

/* A growable vector of function address ranges.  */

struct function_vector
{
  /* Memory.  This is an array of struct function_addrs.  */
  struct backtrace_vector vec;
  /* Number of address ranges present.  */
  size_t count;
};

/* A DWARF compilation unit.  This only holds the information we need
   to map a PC to a file and line.  */

struct unit
{
  /* The first entry for this compilation unit.  */
  const unsigned char *unit_data;
  /* The length of the data for this compilation unit.  */
  size_t unit_data_len;
  /* The offset of UNIT_DATA from the start of the information for
     this compilation unit.  */
  size_t unit_data_offset;
  /* Offset of the start of the compilation unit from the start of the
     .debug_info section.  */
  size_t low_offset;
  /* Offset of the end of the compilation unit from the start of the
     .debug_info section.  */
  size_t high_offset;
  /* DWARF version.  */
  int version;
  /* Whether unit is DWARF64.  */
  int is_dwarf64;
  /* Address size.  */
  int addrsize;
  /* Offset into line number information.  */
  off_t lineoff;
  /* Offset of compilation unit in .debug_str_offsets.  */
  uint64_t str_offsets_base;
  /* Offset of compilation unit in .debug_addr.  */
  uint64_t addr_base;
  /* Offset of compilation unit in .debug_rnglists.  */
  uint64_t rnglists_base;
  /* Primary source file.  */
  const char *filename;
  /* Compilation command working directory.  */
  const char *comp_dir;
  /* Absolute file name, only set if needed.  */
  const char *abs_filename;
  /* The abbreviations for this unit.  */
  struct abbrevs abbrevs;

  /* The fields above this point are read in during initialization and
     may be accessed freely.  The fields below this point are read in
     as needed, and therefore require care, as different threads may
     try to initialize them simultaneously.  */

  /* PC to line number mapping.  This is NULL if the values have not
     been read.  This is (struct line *) -1 if there was an error
     reading the values.  */
  struct line *lines;
  /* Number of entries in lines.  */
  size_t lines_count;
  /* PC ranges to function.  */
  struct function_addrs *function_addrs;
  size_t function_addrs_count;
};

/* An address range for a compilation unit.  This maps a PC value to a
   specific compilation unit.  Note that we invert the representation
   in DWARF: instead of listing the units and attaching a list of
   ranges, we list the ranges and have each one point to the unit.
   This lets us do a binary search to find the unit.  */

struct unit_addrs
{
  /* Range is LOW <= PC < HIGH.  */
  uint64_t low;
  uint64_t high;
  /* Compilation unit for this address range.  */
  struct unit *u;
};

/* A growable vector of compilation unit address ranges.  */

struct unit_addrs_vector
{
  /* Memory.  This is an array of struct unit_addrs.  */
  struct backtrace_vector vec;
  /* Number of address ranges present.  */
  size_t count;
};

/* A growable vector of compilation unit pointer.  */

struct unit_vector
{
  struct backtrace_vector vec;
  size_t count;
};

/* The information we need to map a PC to a file and line.  */

struct dwarf_data
{
  /* The data for the next file we know about.  */
  struct dwarf_data *next;
  /* The data for .gnu_debugaltlink.  */
  struct dwarf_data *altlink;
  /* The base address for this file.  */
  uintptr_t base_address;
  /* A sorted list of address ranges.  */
  struct unit_addrs *addrs;
  /* Number of address ranges in list.  */
  size_t addrs_count;
  /* A sorted list of units.  */
  struct unit **units;
  /* Number of units in the list.  */
  size_t units_count;
  /* The unparsed DWARF debug data.  */
  struct dwarf_sections dwarf_sections;
  /* Whether the data is big-endian or not.  */
  int is_bigendian;
  /* A vector used for function addresses.  We keep this here so that
     we can grow the vector as we read more functions.  */
  struct function_vector fvec;
};

/* Report an error for a DWARF buffer.  */

static void
dwarf_buf_error (struct dwarf_buf *buf, const char *msg, int errnum)
{
  char b[200];

  snprintf (b, sizeof b, "%s in %s at %d",
	    msg, buf->name, (int) (buf->buf - buf->start));
  buf->error_callback (buf->data, b, errnum);
}

/* Require at least COUNT bytes in BUF.  Return 1 if all is well, 0 on
   error.  */

static int
require (struct dwarf_buf *buf, size_t count)
{
  if (buf->left >= count)
    return 1;

  if (!buf->reported_underflow)
    {
      dwarf_buf_error (buf, "DWARF underflow", 0);
      buf->reported_underflow = 1;
    }

  return 0;
}

/* Advance COUNT bytes in BUF.  Return 1 if all is well, 0 on
   error.  */

static int
advance (struct dwarf_buf *buf, size_t count)
{
  if (!require (buf, count))
    return 0;
  buf->buf += count;
  buf->left -= count;
  return 1;
}

/* Read one zero-terminated string from BUF and advance past the string.  */

static const char *
read_string (struct dwarf_buf *buf)
{
  const char *p = (const char *)buf->buf;
  size_t len = strnlen (p, buf->left);

  /* - If len == left, we ran out of buffer before finding the zero terminator.
       Generate an error by advancing len + 1.
     - If len < left, advance by len + 1 to skip past the zero terminator.  */
  size_t count = len + 1;

  if (!advance (buf, count))
    return NULL;

  return p;
}

/* Read one byte from BUF and advance 1 byte.  */

static unsigned char
read_byte (struct dwarf_buf *buf)
{
  const unsigned char *p = buf->buf;

  if (!advance (buf, 1))
    return 0;
  return p[0];
}

/* Read a signed char from BUF and advance 1 byte.  */

static signed char
read_sbyte (struct dwarf_buf *buf)
{
  const unsigned char *p = buf->buf;

  if (!advance (buf, 1))
    return 0;
  return (*p ^ 0x80) - 0x80;
}

/* Read a uint16 from BUF and advance 2 bytes.  */

static uint16_t
read_uint16 (struct dwarf_buf *buf)
{
  const unsigned char *p = buf->buf;

  if (!advance (buf, 2))
    return 0;
  if (buf->is_bigendian)
    return ((uint16_t) p[0] << 8) | (uint16_t) p[1];
  else
    return ((uint16_t) p[1] << 8) | (uint16_t) p[0];
}

/* Read a 24 bit value from BUF and advance 3 bytes.  */

static uint32_t
read_uint24 (struct dwarf_buf *buf)
{
  const unsigned char *p = buf->buf;

  if (!advance (buf, 3))
    return 0;
  if (buf->is_bigendian)
    return (((uint32_t) p[0] << 16) | ((uint32_t) p[1] << 8)
	    | (uint32_t) p[2]);
  else
    return (((uint32_t) p[2] << 16) | ((uint32_t) p[1] << 8)
	    | (uint32_t) p[0]);
}

/* Read a uint32 from BUF and advance 4 bytes.  */

static uint32_t
read_uint32 (struct dwarf_buf *buf)
{
  const unsigned char *p = buf->buf;

  if (!advance (buf, 4))
    return 0;
  if (buf->is_bigendian)
    return (((uint32_t) p[0] << 24) | ((uint32_t) p[1] << 16)
	    | ((uint32_t) p[2] << 8) | (uint32_t) p[3]);
  else
    return (((uint32_t) p[3] << 24) | ((uint32_t) p[2] << 16)
	    | ((uint32_t) p[1] << 8) | (uint32_t) p[0]);
}

/* Read a uint64 from BUF and advance 8 bytes.  */

static uint64_t
read_uint64 (struct dwarf_buf *buf)
{
  const unsigned char *p = buf->buf;

  if (!advance (buf, 8))
    return 0;
  if (buf->is_bigendian)
    return (((uint64_t) p[0] << 56) | ((uint64_t) p[1] << 48)
	    | ((uint64_t) p[2] << 40) | ((uint64_t) p[3] << 32)
	    | ((uint64_t) p[4] << 24) | ((uint64_t) p[5] << 16)
	    | ((uint64_t) p[6] << 8) | (uint64_t) p[7]);
  else
    return (((uint64_t) p[7] << 56) | ((uint64_t) p[6] << 48)
	    | ((uint64_t) p[5] << 40) | ((uint64_t) p[4] << 32)
	    | ((uint64_t) p[3] << 24) | ((uint64_t) p[2] << 16)
	    | ((uint64_t) p[1] << 8) | (uint64_t) p[0]);
}

/* Read an offset from BUF and advance the appropriate number of
   bytes.  */

static uint64_t
read_offset (struct dwarf_buf *buf, int is_dwarf64)
{
  if (is_dwarf64)
    return read_uint64 (buf);
  else
    return read_uint32 (buf);
}

/* Read an address from BUF and advance the appropriate number of
   bytes.  */

static uint64_t
read_address (struct dwarf_buf *buf, int addrsize)
{
  switch (addrsize)
    {
    case 1:
      return read_byte (buf);
    case 2:
      return read_uint16 (buf);
    case 4:
      return read_uint32 (buf);
    case 8:
      return read_uint64 (buf);
    default:
      dwarf_buf_error (buf, "unrecognized address size", 0);
      return 0;
    }
}

/* Return whether a value is the highest possible address, given the
   address size.  */

static int
is_highest_address (uint64_t address, int addrsize)
{
  switch (addrsize)
    {
    case 1:
      return address == (unsigned char) -1;
    case 2:
      return address == (uint16_t) -1;
    case 4:
      return address == (uint32_t) -1;
    case 8:
      return address == (uint64_t) -1;
    default:
      return 0;
    }
}

/* Read an unsigned LEB128 number.  */

static uint64_t
read_uleb128 (struct dwarf_buf *buf)
{
  uint64_t ret;
  unsigned int shift;
  int overflow;
  unsigned char b;

  ret = 0;
  shift = 0;
  overflow = 0;
  do
    {
      const unsigned char *p;

      p = buf->buf;
      if (!advance (buf, 1))
	return 0;
      b = *p;
      if (shift < 64)
	ret |= ((uint64_t) (b & 0x7f)) << shift;
      else if (!overflow)
	{
	  dwarf_buf_error (buf, "LEB128 overflows uint64_t", 0);
	  overflow = 1;
	}
      shift += 7;
    }
  while ((b & 0x80) != 0);

  return ret;
}

/* Read a signed LEB128 number.  */

static int64_t
read_sleb128 (struct dwarf_buf *buf)
{
  uint64_t val;
  unsigned int shift;
  int overflow;
  unsigned char b;

  val = 0;
  shift = 0;
  overflow = 0;
  do
    {
      const unsigned char *p;

      p = buf->buf;
      if (!advance (buf, 1))
	return 0;
      b = *p;
      if (shift < 64)
	val |= ((uint64_t) (b & 0x7f)) << shift;
      else if (!overflow)
	{
	  dwarf_buf_error (buf, "signed LEB128 overflows uint64_t", 0);
	  overflow = 1;
	}
      shift += 7;
    }
  while ((b & 0x80) != 0);

  if ((b & 0x40) != 0 && shift < 64)
    val |= ((uint64_t) -1) << shift;

  return (int64_t) val;
}

/* Return the length of an LEB128 number.  */

static size_t
leb128_len (const unsigned char *p)
{
  size_t ret;

  ret = 1;
  while ((*p & 0x80) != 0)
    {
      ++p;
      ++ret;
    }
  return ret;
}

/* Read initial_length from BUF and advance the appropriate number of bytes.  */

static uint64_t
read_initial_length (struct dwarf_buf *buf, int *is_dwarf64)
{
  uint64_t len;

  len = read_uint32 (buf);
  if (len == 0xffffffff)
    {
      len = read_uint64 (buf);
      *is_dwarf64 = 1;
    }
  else
    *is_dwarf64 = 0;

  return len;
}

/* Free an abbreviations structure.  */

static void
free_abbrevs (struct backtrace_state *state, struct abbrevs *abbrevs,
	      backtrace_error_callback error_callback, void *data)
{
  size_t i;

  for (i = 0; i < abbrevs->num_abbrevs; ++i)
    backtrace_free (state, abbrevs->abbrevs[i].attrs,
		    abbrevs->abbrevs[i].num_attrs * sizeof (struct attr),
		    error_callback, data);
  backtrace_free (state, abbrevs->abbrevs,
		  abbrevs->num_abbrevs * sizeof (struct abbrev),
		  error_callback, data);
  abbrevs->num_abbrevs = 0;
  abbrevs->abbrevs = NULL;
}

/* Read an attribute value.  Returns 1 on success, 0 on failure.  If
   the value can be represented as a uint64_t, sets *VAL and sets
   *IS_VALID to 1.  We don't try to store the value of other attribute
   forms, because we don't care about them.  */

static int
read_attribute (enum dwarf_form form, uint64_t implicit_val,
		struct dwarf_buf *buf, int is_dwarf64, int version,
		int addrsize, const struct dwarf_sections *dwarf_sections,
		struct dwarf_data *altlink, struct attr_val *val)
{
  /* Avoid warnings about val.u.FIELD may be used uninitialized if
     this function is inlined.  The warnings aren't valid but can
     occur because the different fields are set and used
     conditionally.  */
  memset (val, 0, sizeof *val);

  switch (form)
    {
    case DW_FORM_addr:
      val->encoding = ATTR_VAL_ADDRESS;
      val->u.uint = read_address (buf, addrsize);
      return 1;
    case DW_FORM_block2:
      val->encoding = ATTR_VAL_BLOCK;
      return advance (buf, read_uint16 (buf));
    case DW_FORM_block4:
      val->encoding = ATTR_VAL_BLOCK;
      return advance (buf, read_uint32 (buf));
    case DW_FORM_data2:
      val->encoding = ATTR_VAL_UINT;
      val->u.uint = read_uint16 (buf);
      return 1;
    case DW_FORM_data4:
      val->encoding = ATTR_VAL_UINT;
      val->u.uint = read_uint32 (buf);
      return 1;
    case DW_FORM_data8:
      val->encoding = ATTR_VAL_UINT;
      val->u.uint = read_uint64 (buf);
      return 1;
    case DW_FORM_data16:
      val->encoding = ATTR_VAL_BLOCK;
      return advance (buf, 16);
    case DW_FORM_string:
      val->encoding = ATTR_VAL_STRING;
      val->u.string = read_string (buf);
      return val->u.string == NULL ? 0 : 1;
    case DW_FORM_block:
      val->encoding = ATTR_VAL_BLOCK;
      return advance (buf, read_uleb128 (buf));
    case DW_FORM_block1:
      val->encoding = ATTR_VAL_BLOCK;
      return advance (buf, read_byte (buf));
    case DW_FORM_data1:
      val->encoding = ATTR_VAL_UINT;
      val->u.uint = read_byte (buf);
      return 1;
    case DW_FORM_flag:
      val->encoding = ATTR_VAL_UINT;
      val->u.uint = read_byte (buf);
      return 1;
    case DW_FORM_sdata:
      val->encoding = ATTR_VAL_SINT;
      val->u.sint = read_sleb128 (buf);
      return 1;
    case DW_FORM_strp:
      {
	uint64_t offset;

	offset = read_offset (buf, is_dwarf64);
	if (offset >= dwarf_sections->size[DEBUG_STR])
	  {
	    dwarf_buf_error (buf, "DW_FORM_strp out of range", 0);
	    return 0;
	  }
	val->encoding = ATTR_VAL_STRING;
	val->u.string =
	  (const char *) dwarf_sections->data[DEBUG_STR] + offset;
	return 1;
      }
    case DW_FORM_line_strp:
      {
	uint64_t offset;

	offset = read_offset (buf, is_dwarf64);
	if (offset >= dwarf_sections->size[DEBUG_LINE_STR])
	  {
	    dwarf_buf_error (buf, "DW_FORM_line_strp out of range", 0);
	    return 0;
	  }
	val->encoding = ATTR_VAL_STRING;
	val->u.string =
	  (const char *) dwarf_sections->data[DEBUG_LINE_STR] + offset;
	return 1;
      }
    case DW_FORM_udata:
      val->encoding = ATTR_VAL_UINT;
      val->u.uint = read_uleb128 (buf);
      return 1;
    case DW_FORM_ref_addr:
      val->encoding = ATTR_VAL_REF_INFO;
      if (version == 2)
	val->u.uint = read_address (buf, addrsize);
      else
	val->u.uint = read_offset (buf, is_dwarf64);
      return 1;
    case DW_FORM_ref1:
      val->encoding = ATTR_VAL_REF_UNIT;
      val->u.uint = read_byte (buf);
      return 1;
    case DW_FORM_ref2:
      val->encoding = ATTR_VAL_REF_UNIT;
      val->u.uint = read_uint16 (buf);
      return 1;
    case DW_FORM_ref4:
      val->encoding = ATTR_VAL_REF_UNIT;
      val->u.uint = read_uint32 (buf);
      return 1;
    case DW_FORM_ref8:
      val->encoding = ATTR_VAL_REF_UNIT;
      val->u.uint = read_uint64 (buf);
      return 1;
    case DW_FORM_ref_udata:
      val->encoding = ATTR_VAL_REF_UNIT;
      val->u.uint = read_uleb128 (buf);
      return 1;
    case DW_FORM_indirect:
      {
	uint64_t form;

	form = read_uleb128 (buf);
	if (form == DW_FORM_implicit_const)
	  {
	    dwarf_buf_error (buf,
			     "DW_FORM_indirect to DW_FORM_implicit_const",
			     0);
	    return 0;
	  }
	return read_attribute ((enum dwarf_form) form, 0, buf, is_dwarf64,
			       version, addrsize, dwarf_sections, altlink,
			       val);
      }
    case DW_FORM_sec_offset:
      val->encoding = ATTR_VAL_REF_SECTION;
      val->u.uint = read_offset (buf, is_dwarf64);
      return 1;
    case DW_FORM_exprloc:
      val->encoding = ATTR_VAL_EXPR;
      return advance (buf, read_uleb128 (buf));
    case DW_FORM_flag_present:
      val->encoding = ATTR_VAL_UINT;
      val->u.uint = 1;
      return 1;
    case DW_FORM_ref_sig8:
      val->encoding = ATTR_VAL_REF_TYPE;
      val->u.uint = read_uint64 (buf);
      return 1;
    case DW_FORM_strx: case DW_FORM_strx1: case DW_FORM_strx2:
    case DW_FORM_strx3: case DW_FORM_strx4:
      {
	uint64_t offset;

	switch (form)
	  {
	  case DW_FORM_strx:
	    offset = read_uleb128 (buf);
	    break;
	  case DW_FORM_strx1:
	    offset = read_byte (buf);
	    break;
	  case DW_FORM_strx2:
	    offset = read_uint16 (buf);
	    break;
	  case DW_FORM_strx3:
	    offset = read_uint24 (buf);
	    break;
	  case DW_FORM_strx4:
	    offset = read_uint32 (buf);
	    break;
	  default:
	    /* This case can't happen.  */
	    return 0;
	  }
	val->encoding = ATTR_VAL_STRING_INDEX;
	val->u.uint = offset;
	return 1;
      }
    case DW_FORM_addrx: case DW_FORM_addrx1: case DW_FORM_addrx2:
    case DW_FORM_addrx3: case DW_FORM_addrx4:
      {
	uint64_t offset;

	switch (form)
	  {
	  case DW_FORM_addrx:
	    offset = read_uleb128 (buf);
	    break;
	  case DW_FORM_addrx1:
	    offset = read_byte (buf);
	    break;
	  case DW_FORM_addrx2:
	    offset = read_uint16 (buf);
	    break;
	  case DW_FORM_addrx3:
	    offset = read_uint24 (buf);
	    break;
	  case DW_FORM_addrx4:
	    offset = read_uint32 (buf);
	    break;
	  default:
	    /* This case can't happen.  */
	    return 0;
	  }
	val->encoding = ATTR_VAL_ADDRESS_INDEX;
	val->u.uint = offset;
	return 1;
      }
    case DW_FORM_ref_sup4:
      val->encoding = ATTR_VAL_REF_SECTION;
      val->u.uint = read_uint32 (buf);
      return 1;
    case DW_FORM_ref_sup8:
      val->encoding = ATTR_VAL_REF_SECTION;
      val->u.uint = read_uint64 (buf);
      return 1;
    case DW_FORM_implicit_const:
      val->encoding = ATTR_VAL_UINT;
      val->u.uint = implicit_val;
      return 1;
    case DW_FORM_loclistx:
      /* We don't distinguish this from DW_FORM_sec_offset.  It
       * shouldn't matter since we don't care about loclists.  */
      val->encoding = ATTR_VAL_REF_SECTION;
      val->u.uint = read_uleb128 (buf);
      return 1;
    case DW_FORM_rnglistx:
      val->encoding = ATTR_VAL_RNGLISTS_INDEX;
      val->u.uint = read_uleb128 (buf);
      return 1;
    case DW_FORM_GNU_addr_index:
      val->encoding = ATTR_VAL_REF_SECTION;
      val->u.uint = read_uleb128 (buf);
      return 1;
    case DW_FORM_GNU_str_index:
      val->encoding = ATTR_VAL_REF_SECTION;
      val->u.uint = read_uleb128 (buf);
      return 1;
    case DW_FORM_GNU_ref_alt:
      val->u.uint = read_offset (buf, is_dwarf64);
      if (altlink == NULL)
	{
	  val->encoding = ATTR_VAL_NONE;
	  return 1;
	}
      val->encoding = ATTR_VAL_REF_ALT_INFO;
      return 1;
    case DW_FORM_strp_sup: case DW_FORM_GNU_strp_alt:
      {
	uint64_t offset;

	offset = read_offset (buf, is_dwarf64);
	if (altlink == NULL)
	  {
	    val->encoding = ATTR_VAL_NONE;
	    return 1;
	  }
	if (offset >= altlink->dwarf_sections.size[DEBUG_STR])
	  {
	    dwarf_buf_error (buf, "DW_FORM_strp_sup out of range", 0);
	    return 0;
	  }
	val->encoding = ATTR_VAL_STRING;
	val->u.string =
	  (const char *) altlink->dwarf_sections.data[DEBUG_STR] + offset;
	return 1;
      }
    default:
      dwarf_buf_error (buf, "unrecognized DWARF form", -1);
      return 0;
    }
}

/* If we can determine the value of a string attribute, set *STRING to
   point to the string.  Return 1 on success, 0 on error.  If we don't
   know the value, we consider that a success, and we don't change
   *STRING.  An error is only reported for some sort of out of range
   offset.  */

static int
resolve_string (const struct dwarf_sections *dwarf_sections, int is_dwarf64,
		int is_bigendian, uint64_t str_offsets_base,
		const struct attr_val *val,
		backtrace_error_callback error_callback, void *data,
		const char **string)
{
  switch (val->encoding)
    {
    case ATTR_VAL_STRING:
      *string = val->u.string;
      return 1;

    case ATTR_VAL_STRING_INDEX:
      {
	uint64_t offset;
	struct dwarf_buf offset_buf;

	offset = val->u.uint * (is_dwarf64 ? 8 : 4) + str_offsets_base;
	if (offset + (is_dwarf64 ? 8 : 4)
	    > dwarf_sections->size[DEBUG_STR_OFFSETS])
	  {
	    error_callback (data, "DW_FORM_strx value out of range", 0);
	    return 0;
	  }

	offset_buf.name = ".debug_str_offsets";
	offset_buf.start = dwarf_sections->data[DEBUG_STR_OFFSETS];
	offset_buf.buf = dwarf_sections->data[DEBUG_STR_OFFSETS] + offset;
	offset_buf.left = dwarf_sections->size[DEBUG_STR_OFFSETS] - offset;
	offset_buf.is_bigendian = is_bigendian;
	offset_buf.error_callback = error_callback;
	offset_buf.data = data;
	offset_buf.reported_underflow = 0;

	offset = read_offset (&offset_buf, is_dwarf64);
	if (offset >= dwarf_sections->size[DEBUG_STR])
	  {
	    dwarf_buf_error (&offset_buf,
				   "DW_FORM_strx offset out of range",
				   0);
	    return 0;
	  }
	*string = (const char *) dwarf_sections->data[DEBUG_STR] + offset;
	return 1;
      }

    default:
      return 1;
    }
}

/* Set *ADDRESS to the real address for a ATTR_VAL_ADDRESS_INDEX.
   Return 1 on success, 0 on error.  */

static int
resolve_addr_index (const struct dwarf_sections *dwarf_sections,
		    uint64_t addr_base, int addrsize, int is_bigendian,
		    uint64_t addr_index,
		    backtrace_error_callback error_callback, void *data,
		    uint64_t *address)
{
  uint64_t offset;
  struct dwarf_buf addr_buf;

  offset = addr_index * addrsize + addr_base;
  if (offset + addrsize > dwarf_sections->size[DEBUG_ADDR])
    {
      error_callback (data, "DW_FORM_addrx value out of range", 0);
      return 0;
    }

  addr_buf.name = ".debug_addr";
  addr_buf.start = dwarf_sections->data[DEBUG_ADDR];
  addr_buf.buf = dwarf_sections->data[DEBUG_ADDR] + offset;
  addr_buf.left = dwarf_sections->size[DEBUG_ADDR] - offset;
  addr_buf.is_bigendian = is_bigendian;
  addr_buf.error_callback = error_callback;
  addr_buf.data = data;
  addr_buf.reported_underflow = 0;

  *address = read_address (&addr_buf, addrsize);
  return 1;
}

/* Compare a unit offset against a unit for bsearch.  */

static int
units_search (const void *vkey, const void *ventry)
{
  const size_t *key = (const size_t *) vkey;
  const struct unit *entry = *((const struct unit *const *) ventry);
  size_t offset;

  offset = *key;
  if (offset < entry->low_offset)
    return -1;
  else if (offset >= entry->high_offset)
    return 1;
  else
    return 0;
}

/* Find a unit in PU containing OFFSET.  */

static struct unit *
find_unit (struct unit **pu, size_t units_count, size_t offset)
{
  struct unit **u;
  u = (struct unit**)bsearch (&offset, pu, units_count, sizeof (struct unit *), units_search);
  return u == NULL ? NULL : *u;
}

/* Compare function_addrs for qsort.  When ranges are nested, make the
   smallest one sort last.  */

static int
function_addrs_compare (const void *v1, const void *v2)
{
  const struct function_addrs *a1 = (const struct function_addrs *) v1;
  const struct function_addrs *a2 = (const struct function_addrs *) v2;

  if (a1->low < a2->low)
    return -1;
  if (a1->low > a2->low)
    return 1;
  if (a1->high < a2->high)
    return 1;
  if (a1->high > a2->high)
    return -1;
  return strcmp (a1->function->name, a2->function->name);
}

/* Compare a PC against a function_addrs for bsearch.  We always
   allocate an entra entry at the end of the vector, so that this
   routine can safely look at the next entry.  Note that if there are
   multiple ranges containing PC, which one will be returned is
   unpredictable.  We compensate for that in dwarf_fileline.  */

static int
function_addrs_search (const void *vkey, const void *ventry)
{
  const uintptr_t *key = (const uintptr_t *) vkey;
  const struct function_addrs *entry = (const struct function_addrs *) ventry;
  uintptr_t pc;

  pc = *key;
  if (pc < entry->low)
    return -1;
  else if (pc > (entry + 1)->low)
    return 1;
  else
    return 0;
}

/* Add a new compilation unit address range to a vector.  This is
   called via add_ranges.  Returns 1 on success, 0 on failure.  */

static int
add_unit_addr (struct backtrace_state *state, void *rdata,
	       uint64_t lowpc, uint64_t highpc,
	       backtrace_error_callback error_callback, void *data,
	       void *pvec)
{
  struct unit *u = (struct unit *) rdata;
  struct unit_addrs_vector *vec = (struct unit_addrs_vector *) pvec;
  struct unit_addrs *p;

  /* Try to merge with the last entry.  */
  if (vec->count > 0)
    {
      p = (struct unit_addrs *) vec->vec.base + (vec->count - 1);
      if ((lowpc == p->high || lowpc == p->high + 1)
	  && u == p->u)
	{
	  if (highpc > p->high)
	    p->high = highpc;
	  return 1;
	}
    }

  p = ((struct unit_addrs *)
       backtrace_vector_grow (state, sizeof (struct unit_addrs),
			      error_callback, data, &vec->vec));
  if (p == NULL)
    return 0;

  p->low = lowpc;
  p->high = highpc;
  p->u = u;

  ++vec->count;

  return 1;
}

/* Compare unit_addrs for qsort.  When ranges are nested, make the
   smallest one sort last.  */

static int
unit_addrs_compare (const void *v1, const void *v2)
{
  const struct unit_addrs *a1 = (const struct unit_addrs *) v1;
  const struct unit_addrs *a2 = (const struct unit_addrs *) v2;

  if (a1->low < a2->low)
    return -1;
  if (a1->low > a2->low)
    return 1;
  if (a1->high < a2->high)
    return 1;
  if (a1->high > a2->high)
    return -1;
  if (a1->u->lineoff < a2->u->lineoff)
    return -1;
  if (a1->u->lineoff > a2->u->lineoff)
    return 1;
  return 0;
}

/* Compare a PC against a unit_addrs for bsearch.  We always allocate
   an entry entry at the end of the vector, so that this routine can
   safely look at the next entry.  Note that if there are multiple
   ranges containing PC, which one will be returned is unpredictable.
   We compensate for that in dwarf_fileline.  */

static int
unit_addrs_search (const void *vkey, const void *ventry)
{
  const uintptr_t *key = (const uintptr_t *) vkey;
  const struct unit_addrs *entry = (const struct unit_addrs *) ventry;
  uintptr_t pc;

  pc = *key;
  if (pc < entry->low)
    return -1;
  else if (pc > (entry + 1)->low)
    return 1;
  else
    return 0;
}

/* Sort the line vector by PC.  We want a stable sort here to maintain
   the order of lines for the same PC values.  Since the sequence is
   being sorted in place, their addresses cannot be relied on to
   maintain stability.  That is the purpose of the index member.  */

static int
line_compare (const void *v1, const void *v2)
{
  const struct line *ln1 = (const struct line *) v1;
  const struct line *ln2 = (const struct line *) v2;

  if (ln1->pc < ln2->pc)
    return -1;
  else if (ln1->pc > ln2->pc)
    return 1;
  else if (ln1->idx < ln2->idx)
    return -1;
  else if (ln1->idx > ln2->idx)
    return 1;
  else
    return 0;
}

/* Find a PC in a line vector.  We always allocate an extra entry at
   the end of the lines vector, so that this routine can safely look
   at the next entry.  Note that when there are multiple mappings for
   the same PC value, this will return the last one.  */

static int
line_search (const void *vkey, const void *ventry)
{
  const uintptr_t *key = (const uintptr_t *) vkey;
  const struct line *entry = (const struct line *) ventry;
  uintptr_t pc;

  pc = *key;
  if (pc < entry->pc)
    return -1;
  else if (pc >= (entry + 1)->pc)
    return 1;
  else
    return 0;
}

/* Sort the abbrevs by the abbrev code.  This function is passed to
   both qsort and bsearch.  */

static int
abbrev_compare (const void *v1, const void *v2)
{
  const struct abbrev *a1 = (const struct abbrev *) v1;
  const struct abbrev *a2 = (const struct abbrev *) v2;

  if (a1->code < a2->code)
    return -1;
  else if (a1->code > a2->code)
    return 1;
  else
    {
      /* This really shouldn't happen.  It means there are two
	 different abbrevs with the same code, and that means we don't
	 know which one lookup_abbrev should return.  */
      return 0;
    }
}

/* Read the abbreviation table for a compilation unit.  Returns 1 on
   success, 0 on failure.  */

static int
read_abbrevs (struct backtrace_state *state, uint64_t abbrev_offset,
	      const unsigned char *dwarf_abbrev, size_t dwarf_abbrev_size,
	      int is_bigendian, backtrace_error_callback error_callback,
	      void *data, struct abbrevs *abbrevs)
{
  struct dwarf_buf abbrev_buf;
  struct dwarf_buf count_buf;
  size_t num_abbrevs;

  abbrevs->num_abbrevs = 0;
  abbrevs->abbrevs = NULL;

  if (abbrev_offset >= dwarf_abbrev_size)
    {
      error_callback (data, "abbrev offset out of range", 0);
      return 0;
    }

  abbrev_buf.name = ".debug_abbrev";
  abbrev_buf.start = dwarf_abbrev;
  abbrev_buf.buf = dwarf_abbrev + abbrev_offset;
  abbrev_buf.left = dwarf_abbrev_size - abbrev_offset;
  abbrev_buf.is_bigendian = is_bigendian;
  abbrev_buf.error_callback = error_callback;
  abbrev_buf.data = data;
  abbrev_buf.reported_underflow = 0;

  /* Count the number of abbrevs in this list.  */

  count_buf = abbrev_buf;
  num_abbrevs = 0;
  while (read_uleb128 (&count_buf) != 0)
    {
      if (count_buf.reported_underflow)
	return 0;
      ++num_abbrevs;
      // Skip tag.
      read_uleb128 (&count_buf);
      // Skip has_children.
      read_byte (&count_buf);
      // Skip attributes.
      while (read_uleb128 (&count_buf) != 0)
	{
	  uint64_t form;

	  form = read_uleb128 (&count_buf);
	  if ((enum dwarf_form) form == DW_FORM_implicit_const)
	    read_sleb128 (&count_buf);
	}
      // Skip form of last attribute.
      read_uleb128 (&count_buf);
    }

  if (count_buf.reported_underflow)
    return 0;

  if (num_abbrevs == 0)
    return 1;

  abbrevs->abbrevs = ((struct abbrev *)
		      backtrace_alloc (state,
				       num_abbrevs * sizeof (struct abbrev),
				       error_callback, data));
  if (abbrevs->abbrevs == NULL)
    return 0;
  abbrevs->num_abbrevs = num_abbrevs;
  memset (abbrevs->abbrevs, 0, num_abbrevs * sizeof (struct abbrev));

  num_abbrevs = 0;
  while (1)
    {
      uint64_t code;
      struct abbrev a;
      size_t num_attrs;
      struct attr *attrs;

      if (abbrev_buf.reported_underflow)
	goto fail;

      code = read_uleb128 (&abbrev_buf);
      if (code == 0)
	break;

      a.code = code;
      a.tag = (enum dwarf_tag) read_uleb128 (&abbrev_buf);
      a.has_children = read_byte (&abbrev_buf);

      count_buf = abbrev_buf;
      num_attrs = 0;
      while (read_uleb128 (&count_buf) != 0)
	{
	  uint64_t form;

	  ++num_attrs;
	  form = read_uleb128 (&count_buf);
	  if ((enum dwarf_form) form == DW_FORM_implicit_const)
	    read_sleb128 (&count_buf);
	}

      if (num_attrs == 0)
	{
	  attrs = NULL;
	  read_uleb128 (&abbrev_buf);
	  read_uleb128 (&abbrev_buf);
	}
      else
	{
	  attrs = ((struct attr *)
		   backtrace_alloc (state, num_attrs * sizeof *attrs,
				    error_callback, data));
	  if (attrs == NULL)
	    goto fail;
	  num_attrs = 0;
	  while (1)
	    {
	      uint64_t name;
	      uint64_t form;

	      name = read_uleb128 (&abbrev_buf);
	      form = read_uleb128 (&abbrev_buf);
	      if (name == 0)
		break;
	      attrs[num_attrs].name = (enum dwarf_attribute) name;
	      attrs[num_attrs].form = (enum dwarf_form) form;
	      if ((enum dwarf_form) form == DW_FORM_implicit_const)
		attrs[num_attrs].val = read_sleb128 (&abbrev_buf);
	      else
		attrs[num_attrs].val = 0;
	      ++num_attrs;
	    }
	}

      a.num_attrs = num_attrs;
      a.attrs = attrs;

      abbrevs->abbrevs[num_abbrevs] = a;
      ++num_abbrevs;
    }

  backtrace_qsort (abbrevs->abbrevs, abbrevs->num_abbrevs,
		   sizeof (struct abbrev), abbrev_compare);

  return 1;

 fail:
  free_abbrevs (state, abbrevs, error_callback, data);
  return 0;
}

/* Return the abbrev information for an abbrev code.  */

static const struct abbrev *
lookup_abbrev (struct abbrevs *abbrevs, uint64_t code,
	       backtrace_error_callback error_callback, void *data)
{
  struct abbrev key;
  void *p;

  /* With GCC, where abbrevs are simply numbered in order, we should
     be able to just look up the entry.  */
  if (code - 1 < abbrevs->num_abbrevs
      && abbrevs->abbrevs[code - 1].code == code)
    return &abbrevs->abbrevs[code - 1];

  /* Otherwise we have to search.  */
  memset (&key, 0, sizeof key);
  key.code = code;
  p = bsearch (&key, abbrevs->abbrevs, abbrevs->num_abbrevs,
	       sizeof (struct abbrev), abbrev_compare);
  if (p == NULL)
    {
      error_callback (data, "invalid abbreviation code", 0);
      return NULL;
    }
  return (const struct abbrev *) p;
}

/* This struct is used to gather address range information while
   reading attributes.  We use this while building a mapping from
   address ranges to compilation units and then again while mapping
   from address ranges to function entries.  Normally either
   lowpc/highpc is set or ranges is set.  */

struct pcrange {
  uint64_t lowpc;		/* The low PC value.  */
  int have_lowpc;		/* Whether a low PC value was found.  */
  int lowpc_is_addr_index;	/* Whether lowpc is in .debug_addr.  */
  uint64_t highpc;		/* The high PC value.  */
  int have_highpc;		/* Whether a high PC value was found.  */
  int highpc_is_relative;	/* Whether highpc is relative to lowpc.  */
  int highpc_is_addr_index;	/* Whether highpc is in .debug_addr.  */
  uint64_t ranges;		/* Offset in ranges section.  */
  int have_ranges;		/* Whether ranges is valid.  */
  int ranges_is_index;		/* Whether ranges is DW_FORM_rnglistx.  */
};

/* Update PCRANGE from an attribute value.  */

static void
update_pcrange (const struct attr* attr, const struct attr_val* val,
		struct pcrange *pcrange)
{
  switch (attr->name)
    {
    case DW_AT_low_pc:
      if (val->encoding == ATTR_VAL_ADDRESS)
	{
	  pcrange->lowpc = val->u.uint;
	  pcrange->have_lowpc = 1;
	}
      else if (val->encoding == ATTR_VAL_ADDRESS_INDEX)
	{
	  pcrange->lowpc = val->u.uint;
	  pcrange->have_lowpc = 1;
	  pcrange->lowpc_is_addr_index = 1;
	}
      break;

    case DW_AT_high_pc:
      if (val->encoding == ATTR_VAL_ADDRESS)
	{
	  pcrange->highpc = val->u.uint;
	  pcrange->have_highpc = 1;
	}
      else if (val->encoding == ATTR_VAL_UINT)
	{
	  pcrange->highpc = val->u.uint;
	  pcrange->have_highpc = 1;
	  pcrange->highpc_is_relative = 1;
	}
      else if (val->encoding == ATTR_VAL_ADDRESS_INDEX)
	{
	  pcrange->highpc = val->u.uint;
	  pcrange->have_highpc = 1;
	  pcrange->highpc_is_addr_index = 1;
	}
      break;

    case DW_AT_ranges:
      if (val->encoding == ATTR_VAL_UINT
	  || val->encoding == ATTR_VAL_REF_SECTION)
	{
	  pcrange->ranges = val->u.uint;
	  pcrange->have_ranges = 1;
	}
      else if (val->encoding == ATTR_VAL_RNGLISTS_INDEX)
	{
	  pcrange->ranges = val->u.uint;
	  pcrange->have_ranges = 1;
	  pcrange->ranges_is_index = 1;
	}
      break;

    default:
      break;
    }
}

/* Call ADD_RANGE for a low/high PC pair.  Returns 1 on success, 0 on
  error.  */

static int
add_low_high_range (struct backtrace_state *state,
		    const struct dwarf_sections *dwarf_sections,
		    uintptr_t base_address, int is_bigendian,
		    struct unit *u, const struct pcrange *pcrange,
		    int (*add_range) (struct backtrace_state *state,
				      void *rdata, uint64_t lowpc,
				      uint64_t highpc,
				      backtrace_error_callback error_callback,
				      void *data, void *vec),
		    void *rdata,
		    backtrace_error_callback error_callback, void *data,
		    void *vec)
{
  uint64_t lowpc;
  uint64_t highpc;

  lowpc = pcrange->lowpc;
  if (pcrange->lowpc_is_addr_index)
    {
      if (!resolve_addr_index (dwarf_sections, u->addr_base, u->addrsize,
			       is_bigendian, lowpc, error_callback, data,
			       &lowpc))
	return 0;
    }

  highpc = pcrange->highpc;
  if (pcrange->highpc_is_addr_index)
    {
      if (!resolve_addr_index (dwarf_sections, u->addr_base, u->addrsize,
			       is_bigendian, highpc, error_callback, data,
			       &highpc))
	return 0;
    }
  if (pcrange->highpc_is_relative)
    highpc += lowpc;

  /* Add in the base address of the module when recording PC values,
     so that we can look up the PC directly.  */
  lowpc += base_address;
  highpc += base_address;

  return add_range (state, rdata, lowpc, highpc, error_callback, data, vec);
}

/* Call ADD_RANGE for each range read from .debug_ranges, as used in
   DWARF versions 2 through 4.  */

static int
add_ranges_from_ranges (
    struct backtrace_state *state,
    const struct dwarf_sections *dwarf_sections,
    uintptr_t base_address, int is_bigendian,
    struct unit *u, uint64_t base,
    const struct pcrange *pcrange,
    int (*add_range) (struct backtrace_state *state, void *rdata,
		      uint64_t lowpc, uint64_t highpc,
		      backtrace_error_callback error_callback, void *data,
		      void *vec),
    void *rdata,
    backtrace_error_callback error_callback, void *data,
    void *vec)
{
  struct dwarf_buf ranges_buf;

  if (pcrange->ranges >= dwarf_sections->size[DEBUG_RANGES])
    {
      error_callback (data, "ranges offset out of range", 0);
      return 0;
    }

  ranges_buf.name = ".debug_ranges";
  ranges_buf.start = dwarf_sections->data[DEBUG_RANGES];
  ranges_buf.buf = dwarf_sections->data[DEBUG_RANGES] + pcrange->ranges;
  ranges_buf.left = dwarf_sections->size[DEBUG_RANGES] - pcrange->ranges;
  ranges_buf.is_bigendian = is_bigendian;
  ranges_buf.error_callback = error_callback;
  ranges_buf.data = data;
  ranges_buf.reported_underflow = 0;

  while (1)
    {
      uint64_t low;
      uint64_t high;

      if (ranges_buf.reported_underflow)
	return 0;

      low = read_address (&ranges_buf, u->addrsize);
      high = read_address (&ranges_buf, u->addrsize);

      if (low == 0 && high == 0)
	break;

      if (is_highest_address (low, u->addrsize))
	base = high;
      else
	{
	  if (!add_range (state, rdata,
			  low + base + base_address,
			  high + base + base_address,
			  error_callback, data, vec))
	    return 0;
	}
    }

  if (ranges_buf.reported_underflow)
    return 0;

  return 1;
}

/* Call ADD_RANGE for each range read from .debug_rnglists, as used in
   DWARF version 5.  */

static int
add_ranges_from_rnglists (
    struct backtrace_state *state,
    const struct dwarf_sections *dwarf_sections,
    uintptr_t base_address, int is_bigendian,
    struct unit *u, uint64_t base,
    const struct pcrange *pcrange,
    int (*add_range) (struct backtrace_state *state, void *rdata,
		      uint64_t lowpc, uint64_t highpc,
		      backtrace_error_callback error_callback, void *data,
		      void *vec),
    void *rdata,
    backtrace_error_callback error_callback, void *data,
    void *vec)
{
  uint64_t offset;
  struct dwarf_buf rnglists_buf;

  if (!pcrange->ranges_is_index)
    offset = pcrange->ranges;
  else
    offset = u->rnglists_base + pcrange->ranges * (u->is_dwarf64 ? 8 : 4);
  if (offset >= dwarf_sections->size[DEBUG_RNGLISTS])
    {
      error_callback (data, "rnglists offset out of range", 0);
      return 0;
    }

  rnglists_buf.name = ".debug_rnglists";
  rnglists_buf.start = dwarf_sections->data[DEBUG_RNGLISTS];
  rnglists_buf.buf = dwarf_sections->data[DEBUG_RNGLISTS] + offset;
  rnglists_buf.left = dwarf_sections->size[DEBUG_RNGLISTS] - offset;
  rnglists_buf.is_bigendian = is_bigendian;
  rnglists_buf.error_callback = error_callback;
  rnglists_buf.data = data;
  rnglists_buf.reported_underflow = 0;

  if (pcrange->ranges_is_index)
    {
      offset = read_offset (&rnglists_buf, u->is_dwarf64);
      offset += u->rnglists_base;
      if (offset >= dwarf_sections->size[DEBUG_RNGLISTS])
	{
	  error_callback (data, "rnglists index offset out of range", 0);
	  return 0;
	}
      rnglists_buf.buf = dwarf_sections->data[DEBUG_RNGLISTS] + offset;
      rnglists_buf.left = dwarf_sections->size[DEBUG_RNGLISTS] - offset;
    }

  while (1)
    {
      unsigned char rle;

      rle = read_byte (&rnglists_buf);
      if (rle == DW_RLE_end_of_list)
	break;
      switch (rle)
	{
	case DW_RLE_base_addressx:
	  {
	    uint64_t index;

	    index = read_uleb128 (&rnglists_buf);
	    if (!resolve_addr_index (dwarf_sections, u->addr_base,
				     u->addrsize, is_bigendian, index,
				     error_callback, data, &base))
	      return 0;
	  }
	  break;

	case DW_RLE_startx_endx:
	  {
	    uint64_t index;
	    uint64_t low;
	    uint64_t high;

	    index = read_uleb128 (&rnglists_buf);
	    if (!resolve_addr_index (dwarf_sections, u->addr_base,
				     u->addrsize, is_bigendian, index,
				     error_callback, data, &low))
	      return 0;
	    index = read_uleb128 (&rnglists_buf);
	    if (!resolve_addr_index (dwarf_sections, u->addr_base,
				     u->addrsize, is_bigendian, index,
				     error_callback, data, &high))
	      return 0;
	    if (!add_range (state, rdata, low + base_address,
			    high + base_address, error_callback, data,
			    vec))
	      return 0;
	  }
	  break;

	case DW_RLE_startx_length:
	  {
	    uint64_t index;
	    uint64_t low;
	    uint64_t length;

	    index = read_uleb128 (&rnglists_buf);
	    if (!resolve_addr_index (dwarf_sections, u->addr_base,
				     u->addrsize, is_bigendian, index,
				     error_callback, data, &low))
	      return 0;
	    length = read_uleb128 (&rnglists_buf);
	    low += base_address;
	    if (!add_range (state, rdata, low, low + length,
			    error_callback, data, vec))
	      return 0;
	  }
	  break;

	case DW_RLE_offset_pair:
	  {
	    uint64_t low;
	    uint64_t high;

	    low = read_uleb128 (&rnglists_buf);
	    high = read_uleb128 (&rnglists_buf);
	    if (!add_range (state, rdata, low + base + base_address,
			    high + base + base_address,
			    error_callback, data, vec))
	      return 0;
	  }
	  break;

	case DW_RLE_base_address:
	  base = read_address (&rnglists_buf, u->addrsize);
	  break;

	case DW_RLE_start_end:
	  {
	    uint64_t low;
	    uint64_t high;

	    low = read_address (&rnglists_buf, u->addrsize);
	    high = read_address (&rnglists_buf, u->addrsize);
	    if (!add_range (state, rdata, low + base_address,
			    high + base_address, error_callback, data,
			    vec))
	      return 0;
	  }
	  break;

	case DW_RLE_start_length:
	  {
	    uint64_t low;
	    uint64_t length;

	    low = read_address (&rnglists_buf, u->addrsize);
	    length = read_uleb128 (&rnglists_buf);
	    low += base_address;
	    if (!add_range (state, rdata, low, low + length,
			    error_callback, data, vec))
	      return 0;
	  }
	  break;

	default:
	  dwarf_buf_error (&rnglists_buf, "unrecognized DW_RLE value", -1);
	  return 0;
	}
    }

  if (rnglists_buf.reported_underflow)
    return 0;

  return 1;
}

/* Call ADD_RANGE for each lowpc/highpc pair in PCRANGE.  RDATA is
   passed to ADD_RANGE, and is either a struct unit * or a struct
   function *.  VEC is the vector we are adding ranges to, and is
   either a struct unit_addrs_vector * or a struct function_vector *.
   Returns 1 on success, 0 on error.  */

static int
add_ranges (struct backtrace_state *state,
	    const struct dwarf_sections *dwarf_sections,
	    uintptr_t base_address, int is_bigendian,
	    struct unit *u, uint64_t base, const struct pcrange *pcrange,
	    int (*add_range) (struct backtrace_state *state, void *rdata,
			      uint64_t lowpc, uint64_t highpc,
			      backtrace_error_callback error_callback,
			      void *data, void *vec),
	    void *rdata,
	    backtrace_error_callback error_callback, void *data,
	    void *vec)
{
  if (pcrange->have_lowpc && pcrange->have_highpc)
    return add_low_high_range (state, dwarf_sections, base_address,
			       is_bigendian, u, pcrange, add_range, rdata,
			       error_callback, data, vec);

  if (!pcrange->have_ranges)
    {
      /* Did not find any address ranges to add.  */
      return 1;
    }

  if (u->version < 5)
    return add_ranges_from_ranges (state, dwarf_sections, base_address,
				   is_bigendian, u, base, pcrange, add_range,
				   rdata, error_callback, data, vec);
  else
    return add_ranges_from_rnglists (state, dwarf_sections, base_address,
				     is_bigendian, u, base, pcrange, add_range,
				     rdata, error_callback, data, vec);
}

/* Find the address range covered by a compilation unit, reading from
   UNIT_BUF and adding values to U.  Returns 1 if all data could be
   read, 0 if there is some error.  */

static int
find_address_ranges (struct backtrace_state *state, uintptr_t base_address,
		     struct dwarf_buf *unit_buf,
		     const struct dwarf_sections *dwarf_sections,
		     int is_bigendian, struct dwarf_data *altlink,
		     backtrace_error_callback error_callback, void *data,
		     struct unit *u, struct unit_addrs_vector *addrs,
		     enum dwarf_tag *unit_tag)
{
  while (unit_buf->left > 0)
    {
      uint64_t code;
      const struct abbrev *abbrev;
      struct pcrange pcrange;
      struct attr_val name_val;
      int have_name_val;
      struct attr_val comp_dir_val;
      int have_comp_dir_val;
      size_t i;

      code = read_uleb128 (unit_buf);
      if (code == 0)
	return 1;

      abbrev = lookup_abbrev (&u->abbrevs, code, error_callback, data);
      if (abbrev == NULL)
	return 0;

      if (unit_tag != NULL)
	*unit_tag = abbrev->tag;

      memset (&pcrange, 0, sizeof pcrange);
      memset (&name_val, 0, sizeof name_val);
      have_name_val = 0;
      memset (&comp_dir_val, 0, sizeof comp_dir_val);
      have_comp_dir_val = 0;
      for (i = 0; i < abbrev->num_attrs; ++i)
	{
	  struct attr_val val;

	  if (!read_attribute (abbrev->attrs[i].form, abbrev->attrs[i].val,
			       unit_buf, u->is_dwarf64, u->version,
			       u->addrsize, dwarf_sections, altlink, &val))
	    return 0;

	  switch (abbrev->attrs[i].name)
	    {
	    case DW_AT_low_pc: case DW_AT_high_pc: case DW_AT_ranges:
	      update_pcrange (&abbrev->attrs[i], &val, &pcrange);
	      break;

	    case DW_AT_stmt_list:
	      if (abbrev->tag == DW_TAG_compile_unit
		  && (val.encoding == ATTR_VAL_UINT
		      || val.encoding == ATTR_VAL_REF_SECTION))
		u->lineoff = val.u.uint;
	      break;

	    case DW_AT_name:
	      if (abbrev->tag == DW_TAG_compile_unit)
		{
		  name_val = val;
		  have_name_val = 1;
		}
	      break;

	    case DW_AT_comp_dir:
	      if (abbrev->tag == DW_TAG_compile_unit)
		{
		  comp_dir_val = val;
		  have_comp_dir_val = 1;
		}
	      break;

	    case DW_AT_str_offsets_base:
	      if (abbrev->tag == DW_TAG_compile_unit
		  && val.encoding == ATTR_VAL_REF_SECTION)
		u->str_offsets_base = val.u.uint;
	      break;

	    case DW_AT_addr_base:
	      if (abbrev->tag == DW_TAG_compile_unit
		  && val.encoding == ATTR_VAL_REF_SECTION)
		u->addr_base = val.u.uint;
	      break;

	    case DW_AT_rnglists_base:
	      if (abbrev->tag == DW_TAG_compile_unit
		  && val.encoding == ATTR_VAL_REF_SECTION)
		u->rnglists_base = val.u.uint;
	      break;

	    default:
	      break;
	    }
	}

      // Resolve strings after we're sure that we have seen
      // DW_AT_str_offsets_base.
      if (have_name_val)
	{
	  if (!resolve_string (dwarf_sections, u->is_dwarf64, is_bigendian,
			       u->str_offsets_base, &name_val,
			       error_callback, data, &u->filename))
	    return 0;
	}
      if (have_comp_dir_val)
	{
	  if (!resolve_string (dwarf_sections, u->is_dwarf64, is_bigendian,
			       u->str_offsets_base, &comp_dir_val,
			       error_callback, data, &u->comp_dir))
	    return 0;
	}

      if (abbrev->tag == DW_TAG_compile_unit
	  || abbrev->tag == DW_TAG_subprogram)
	{
	  if (!add_ranges (state, dwarf_sections, base_address,
			   is_bigendian, u, pcrange.lowpc, &pcrange,
			   add_unit_addr, (void *) u, error_callback, data,
			   (void *) addrs))
	    return 0;

	  /* If we found the PC range in the DW_TAG_compile_unit, we
	     can stop now.  */
	  if (abbrev->tag == DW_TAG_compile_unit
	      && (pcrange.have_ranges
		  || (pcrange.have_lowpc && pcrange.have_highpc)))
	    return 1;
	}

      if (abbrev->has_children)
	{
	  if (!find_address_ranges (state, base_address, unit_buf,
				    dwarf_sections, is_bigendian, altlink,
				    error_callback, data, u, addrs, NULL))
	    return 0;
	}
    }

  return 1;
}

/* Build a mapping from address ranges to the compilation units where
   the line number information for that range can be found.  Returns 1
   on success, 0 on failure.  */

static int
build_address_map (struct backtrace_state *state, uintptr_t base_address,
		   const struct dwarf_sections *dwarf_sections,
		   int is_bigendian, struct dwarf_data *altlink,
		   backtrace_error_callback error_callback, void *data,
		   struct unit_addrs_vector *addrs,
		   struct unit_vector *unit_vec)
{
  struct dwarf_buf info;
  struct backtrace_vector units;
  size_t units_count;
  size_t i;
  struct unit **pu;
  size_t unit_offset = 0;
  struct unit_addrs *pa;

  memset (&addrs->vec, 0, sizeof addrs->vec);
  memset (&unit_vec->vec, 0, sizeof unit_vec->vec);
  addrs->count = 0;
  unit_vec->count = 0;

  /* Read through the .debug_info section.  FIXME: Should we use the
     .debug_aranges section?  gdb and addr2line don't use it, but I'm
     not sure why.  */

  info.name = ".debug_info";
  info.start = dwarf_sections->data[DEBUG_INFO];
  info.buf = info.start;
  info.left = dwarf_sections->size[DEBUG_INFO];
  info.is_bigendian = is_bigendian;
  info.error_callback = error_callback;
  info.data = data;
  info.reported_underflow = 0;

  memset (&units, 0, sizeof units);
  units_count = 0;

  while (info.left > 0)
    {
      const unsigned char *unit_data_start;
      uint64_t len;
      int is_dwarf64;
      struct dwarf_buf unit_buf;
      int version;
      int unit_type;
      uint64_t abbrev_offset;
      int addrsize;
      struct unit *u;
      enum dwarf_tag unit_tag;

      if (info.reported_underflow)
	goto fail;

      unit_data_start = info.buf;

      len = read_initial_length (&info, &is_dwarf64);
      unit_buf = info;
      unit_buf.left = len;

      if (!advance (&info, len))
	goto fail;

      version = read_uint16 (&unit_buf);
      if (version < 2 || version > 5)
	{
	  dwarf_buf_error (&unit_buf, "unrecognized DWARF version", -1);
	  goto fail;
	}

      if (version < 5)
	unit_type = 0;
      else
	{
	  unit_type = read_byte (&unit_buf);
	  if (unit_type == DW_UT_type || unit_type == DW_UT_split_type)
	    {
	      /* This unit doesn't have anything we need.  */
	      continue;
	    }
	}

      pu = ((struct unit **)
	    backtrace_vector_grow (state, sizeof (struct unit *),
				   error_callback, data, &units));
      if (pu == NULL)
	  goto fail;

      u = ((struct unit *)
	   backtrace_alloc (state, sizeof *u, error_callback, data));
      if (u == NULL)
	goto fail;

      *pu = u;
      ++units_count;

      if (version < 5)
	addrsize = 0; /* Set below.  */
      else
	addrsize = read_byte (&unit_buf);

      memset (&u->abbrevs, 0, sizeof u->abbrevs);
      abbrev_offset = read_offset (&unit_buf, is_dwarf64);
      if (!read_abbrevs (state, abbrev_offset,
			 dwarf_sections->data[DEBUG_ABBREV],
			 dwarf_sections->size[DEBUG_ABBREV],
			 is_bigendian, error_callback, data, &u->abbrevs))
	goto fail;

      if (version < 5)
	addrsize = read_byte (&unit_buf);

      switch (unit_type)
	{
	case 0:
	  break;
	case DW_UT_compile: case DW_UT_partial:
	  break;
	case DW_UT_skeleton: case DW_UT_split_compile:
	  read_uint64 (&unit_buf); /* dwo_id */
	  break;
	default:
	  break;
	}

      u->low_offset = unit_offset;
      unit_offset += len + (is_dwarf64 ? 12 : 4);
      u->high_offset = unit_offset;
      u->unit_data = unit_buf.buf;
      u->unit_data_len = unit_buf.left;
      u->unit_data_offset = unit_buf.buf - unit_data_start;
      u->version = version;
      u->is_dwarf64 = is_dwarf64;
      u->addrsize = addrsize;
      u->filename = NULL;
      u->comp_dir = NULL;
      u->abs_filename = NULL;
      u->lineoff = 0;

      /* The actual line number mappings will be read as needed.  */
      u->lines = NULL;
      u->lines_count = 0;
      u->function_addrs = NULL;
      u->function_addrs_count = 0;

      if (!find_address_ranges (state, base_address, &unit_buf, dwarf_sections,
				is_bigendian, altlink, error_callback, data,
				u, addrs, &unit_tag))
	goto fail;

      if (unit_buf.reported_underflow)
	goto fail;
    }
  if (info.reported_underflow)
    goto fail;

  /* Add a trailing addrs entry, but don't include it in addrs->count.  */
  pa = ((struct unit_addrs *)
	backtrace_vector_grow (state, sizeof (struct unit_addrs),
			       error_callback, data, &addrs->vec));
  if (pa == NULL)
    goto fail;
  pa->low = 0;
  --pa->low;
  pa->high = pa->low;
  pa->u = NULL;

  unit_vec->vec = units;
  unit_vec->count = units_count;
  return 1;

 fail:
  if (units_count > 0)
    {
      pu = (struct unit **) units.base;
      for (i = 0; i < units_count; i++)
	{
	  free_abbrevs (state, &pu[i]->abbrevs, error_callback, data);
	  backtrace_free (state, pu[i], sizeof **pu, error_callback, data);
	}
      backtrace_vector_free (state, &units, error_callback, data);
    }
  if (addrs->count > 0)
    {
      backtrace_vector_free (state, &addrs->vec, error_callback, data);
      addrs->count = 0;
    }
  return 0;
}

/* Add a new mapping to the vector of line mappings that we are
   building.  Returns 1 on success, 0 on failure.  */

static int
add_line (struct backtrace_state *state, struct dwarf_data *ddata,
	  uintptr_t pc, const char *filename, int lineno,
	  backtrace_error_callback error_callback, void *data,
	  struct line_vector *vec)
{
  struct line *ln;

  /* If we are adding the same mapping, ignore it.  This can happen
     when using discriminators.  */
  if (vec->count > 0)
    {
      ln = (struct line *) vec->vec.base + (vec->count - 1);
      if (pc == ln->pc && filename == ln->filename && lineno == ln->lineno)
	return 1;
    }

  ln = ((struct line *)
	backtrace_vector_grow (state, sizeof (struct line), error_callback,
			       data, &vec->vec));
  if (ln == NULL)
    return 0;

  /* Add in the base address here, so that we can look up the PC
     directly.  */
  ln->pc = pc + ddata->base_address;

  ln->filename = filename;
  ln->lineno = lineno;
  ln->idx = vec->count;

  ++vec->count;

  return 1;
}

/* Free the line header information.  */

static void
free_line_header (struct backtrace_state *state, struct line_header *hdr,
		  backtrace_error_callback error_callback, void *data)
{
  if (hdr->dirs_count != 0)
    backtrace_free (state, hdr->dirs, hdr->dirs_count * sizeof (const char *),
		    error_callback, data);
  backtrace_free (state, hdr->filenames,
		  hdr->filenames_count * sizeof (char *),
		  error_callback, data);
}

/* Read the directories and file names for a line header for version
   2, setting fields in HDR.  Return 1 on success, 0 on failure.  */

static int
read_v2_paths (struct backtrace_state *state, struct unit *u,
	       struct dwarf_buf *hdr_buf, struct line_header *hdr)
{
  const unsigned char *p;
  const unsigned char *pend;
  size_t i;

  /* Count the number of directory entries.  */
  hdr->dirs_count = 0;
  p = hdr_buf->buf;
  pend = p + hdr_buf->left;
  while (p < pend && *p != '\0')
    {
      p += strnlen((const char *) p, pend - p) + 1;
      ++hdr->dirs_count;
    }

  /* The index of the first entry in the list of directories is 1.  Index 0 is
     used for the current directory of the compilation.  To simplify index
     handling, we set entry 0 to the compilation unit directory.  */
  ++hdr->dirs_count;
  hdr->dirs = ((const char **)
	       backtrace_alloc (state,
				hdr->dirs_count * sizeof (const char *),
				hdr_buf->error_callback,
				hdr_buf->data));
  if (hdr->dirs == NULL)
    return 0;

  hdr->dirs[0] = u->comp_dir;
  i = 1;
  while (*hdr_buf->buf != '\0')
    {
      if (hdr_buf->reported_underflow)
	return 0;

      hdr->dirs[i] = read_string (hdr_buf);
      if (hdr->dirs[i] == NULL)
	return 0;
      ++i;
    }
  if (!advance (hdr_buf, 1))
    return 0;

  /* Count the number of file entries.  */
  hdr->filenames_count = 0;
  p = hdr_buf->buf;
  pend = p + hdr_buf->left;
  while (p < pend && *p != '\0')
    {
      p += strnlen ((const char *) p, pend - p) + 1;
      p += leb128_len (p);
      p += leb128_len (p);
      p += leb128_len (p);
      ++hdr->filenames_count;
    }

  /* The index of the first entry in the list of file names is 1.  Index 0 is
     used for the DW_AT_name of the compilation unit.  To simplify index
     handling, we set entry 0 to the compilation unit file name.  */
  ++hdr->filenames_count;
  hdr->filenames = ((const char **)
		    backtrace_alloc (state,
				     hdr->filenames_count * sizeof (char *),
				     hdr_buf->error_callback,
				     hdr_buf->data));
  if (hdr->filenames == NULL)
    return 0;
  hdr->filenames[0] = u->filename;
  i = 1;
  while (*hdr_buf->buf != '\0')
    {
      const char *filename;
      uint64_t dir_index;

      if (hdr_buf->reported_underflow)
	return 0;

      filename = read_string (hdr_buf);
      if (filename == NULL)
	return 0;
      dir_index = read_uleb128 (hdr_buf);
      if (IS_ABSOLUTE_PATH (filename)
	  || (dir_index < hdr->dirs_count && hdr->dirs[dir_index] == NULL))
	hdr->filenames[i] = filename;
      else
	{
	  const char *dir;
	  size_t dir_len;
	  size_t filename_len;
	  char *s;

	  if (dir_index < hdr->dirs_count)
	    dir = hdr->dirs[dir_index];
	  else
	    {
	      dwarf_buf_error (hdr_buf,
			       ("invalid directory index in "
				"line number program header"),
			       0);
	      return 0;
	    }
	  dir_len = strlen (dir);
	  filename_len = strlen (filename);
	  s = ((char *) backtrace_alloc (state, dir_len + filename_len + 2,
					 hdr_buf->error_callback,
					 hdr_buf->data));
	  if (s == NULL)
	    return 0;
	  memcpy (s, dir, dir_len);
	  /* FIXME: If we are on a DOS-based file system, and the
	     directory or the file name use backslashes, then we
	     should use a backslash here.  */
	  s[dir_len] = '/';
	  memcpy (s + dir_len + 1, filename, filename_len + 1);
	  hdr->filenames[i] = s;
	}

      /* Ignore the modification time and size.  */
      read_uleb128 (hdr_buf);
      read_uleb128 (hdr_buf);

      ++i;
    }

  return 1;
}

/* Read a single version 5 LNCT entry for a directory or file name in a
   line header.  Sets *STRING to the resulting name, ignoring other
   data.  Return 1 on success, 0 on failure.  */

static int
read_lnct (struct backtrace_state *state, struct dwarf_data *ddata,
	   struct unit *u, struct dwarf_buf *hdr_buf,
	   const struct line_header *hdr, size_t formats_count,
	   const struct line_header_format *formats, const char **string)
{
  size_t i;
  const char *dir;
  const char *path;

  dir = NULL;
  path = NULL;
  for (i = 0; i < formats_count; i++)
    {
      struct attr_val val;

      if (!read_attribute (formats[i].form, 0, hdr_buf, u->is_dwarf64,
			   u->version, hdr->addrsize, &ddata->dwarf_sections,
			   ddata->altlink, &val))
	return 0;
      switch (formats[i].lnct)
	{
	case DW_LNCT_path:
	  if (!resolve_string (&ddata->dwarf_sections, u->is_dwarf64,
			       ddata->is_bigendian, u->str_offsets_base,
			       &val, hdr_buf->error_callback, hdr_buf->data,
			       &path))
	    return 0;
	  break;
	case DW_LNCT_directory_index:
	  if (val.encoding == ATTR_VAL_UINT)
	    {
	      if (val.u.uint >= hdr->dirs_count)
		{
		  dwarf_buf_error (hdr_buf,
				   ("invalid directory index in "
				    "line number program header"),
				   0);
		  return 0;
		}
	      dir = hdr->dirs[val.u.uint];
	    }
	  break;
	default:
	  /* We don't care about timestamps or sizes or hashes.  */
	  break;
	}
    }

  if (path == NULL)
    {
      dwarf_buf_error (hdr_buf,
		       "missing file name in line number program header",
		       0);
      return 0;
    }

  if (dir == NULL)
    *string = path;
  else
    {
      size_t dir_len;
      size_t path_len;
      char *s;

      dir_len = strlen (dir);
      path_len = strlen (path);
      s = (char *) backtrace_alloc (state, dir_len + path_len + 2,
				    hdr_buf->error_callback, hdr_buf->data);
      if (s == NULL)
	return 0;
      memcpy (s, dir, dir_len);
      /* FIXME: If we are on a DOS-based file system, and the
	 directory or the path name use backslashes, then we should
	 use a backslash here.  */
      s[dir_len] = '/';
      memcpy (s + dir_len + 1, path, path_len + 1);
      *string = s;
    }

  return 1;
}

/* Read a set of DWARF 5 line header format entries, setting *PCOUNT
   and *PPATHS.  Return 1 on success, 0 on failure.  */

static int
read_line_header_format_entries (struct backtrace_state *state,
				 struct dwarf_data *ddata,
				 struct unit *u,
				 struct dwarf_buf *hdr_buf,
				 struct line_header *hdr,
				 size_t *pcount,
				 const char ***ppaths)
{
  size_t formats_count;
  struct line_header_format *formats;
  size_t paths_count;
  const char **paths;
  size_t i;
  int ret;

  formats_count = read_byte (hdr_buf);
  if (formats_count == 0)
    formats = NULL;
  else
    {
      formats = ((struct line_header_format *)
		 backtrace_alloc (state,
				  (formats_count
				   * sizeof (struct line_header_format)),
				  hdr_buf->error_callback,
				  hdr_buf->data));
      if (formats == NULL)
	return 0;

      for (i = 0; i < formats_count; i++)
	{
	  formats[i].lnct = (int) read_uleb128(hdr_buf);
	  formats[i].form = (enum dwarf_form) read_uleb128 (hdr_buf);
	}
    }

  paths_count = read_uleb128 (hdr_buf);
  if (paths_count == 0)
    {
      *pcount = 0;
      *ppaths = NULL;
      ret = 1;
      goto exit;
    }

  paths = ((const char **)
	   backtrace_alloc (state, paths_count * sizeof (const char *),
			    hdr_buf->error_callback, hdr_buf->data));
  if (paths == NULL)
    {
      ret = 0;
      goto exit;
    }
  for (i = 0; i < paths_count; i++)
    {
      if (!read_lnct (state, ddata, u, hdr_buf, hdr, formats_count,
		      formats, &paths[i]))
	{
	  backtrace_free (state, paths,
			  paths_count * sizeof (const char *),
			  hdr_buf->error_callback, hdr_buf->data);
	  ret = 0;
	  goto exit;
	}
    }

  *pcount = paths_count;
  *ppaths = paths;

  ret = 1;

 exit:
  if (formats != NULL)
    backtrace_free (state, formats,
		    formats_count * sizeof (struct line_header_format),
		    hdr_buf->error_callback, hdr_buf->data);

  return  ret;
}

/* Read the line header.  Return 1 on success, 0 on failure.  */

static int
read_line_header (struct backtrace_state *state, struct dwarf_data *ddata,
		  struct unit *u, int is_dwarf64, struct dwarf_buf *line_buf,
		  struct line_header *hdr)
{
  uint64_t hdrlen;
  struct dwarf_buf hdr_buf;

  hdr->version = read_uint16 (line_buf);
  if (hdr->version < 2 || hdr->version > 5)
    {
      dwarf_buf_error (line_buf, "unsupported line number version", -1);
      return 0;
    }

  if (hdr->version < 5)
    hdr->addrsize = u->addrsize;
  else
    {
      hdr->addrsize = read_byte (line_buf);
      /* We could support a non-zero segment_selector_size but I doubt
	 we'll ever see it.  */
      if (read_byte (line_buf) != 0)
	{
	  dwarf_buf_error (line_buf,
			   "non-zero segment_selector_size not supported",
			   -1);
	  return 0;
	}
    }

  hdrlen = read_offset (line_buf, is_dwarf64);

  hdr_buf = *line_buf;
  hdr_buf.left = hdrlen;

  if (!advance (line_buf, hdrlen))
    return 0;

  hdr->min_insn_len = read_byte (&hdr_buf);
  if (hdr->version < 4)
    hdr->max_ops_per_insn = 1;
  else
    hdr->max_ops_per_insn = read_byte (&hdr_buf);

  /* We don't care about default_is_stmt.  */
  read_byte (&hdr_buf);

  hdr->line_base = read_sbyte (&hdr_buf);
  hdr->line_range = read_byte (&hdr_buf);

  hdr->opcode_base = read_byte (&hdr_buf);
  hdr->opcode_lengths = hdr_buf.buf;
  if (!advance (&hdr_buf, hdr->opcode_base - 1))
    return 0;

  if (hdr->version < 5)
    {
      if (!read_v2_paths (state, u, &hdr_buf, hdr))
	return 0;
    }
  else
    {
      if (!read_line_header_format_entries (state, ddata, u, &hdr_buf, hdr,
					    &hdr->dirs_count,
					    &hdr->dirs))
	return 0;
      if (!read_line_header_format_entries (state, ddata, u, &hdr_buf, hdr,
					    &hdr->filenames_count,
					    &hdr->filenames))
	return 0;
    }

  if (hdr_buf.reported_underflow)
    return 0;

  return 1;
}

/* Read the line program, adding line mappings to VEC.  Return 1 on
   success, 0 on failure.  */

static int
read_line_program (struct backtrace_state *state, struct dwarf_data *ddata,
		   const struct line_header *hdr, struct dwarf_buf *line_buf,
		   struct line_vector *vec)
{
  uint64_t address;
  unsigned int op_index;
  const char *reset_filename;
  const char *filename;
  int lineno;

  address = 0;
  op_index = 0;
  if (hdr->filenames_count > 1)
    reset_filename = hdr->filenames[1];
  else
    reset_filename = "";
  filename = reset_filename;
  lineno = 1;
  while (line_buf->left > 0)
    {
      unsigned int op;

      op = read_byte (line_buf);
      if (op >= hdr->opcode_base)
	{
	  unsigned int advance;

	  /* Special opcode.  */
	  op -= hdr->opcode_base;
	  advance = op / hdr->line_range;
	  address += (hdr->min_insn_len * (op_index + advance)
		      / hdr->max_ops_per_insn);
	  op_index = (op_index + advance) % hdr->max_ops_per_insn;
	  lineno += hdr->line_base + (int) (op % hdr->line_range);
	  add_line (state, ddata, address, filename, lineno,
		    line_buf->error_callback, line_buf->data, vec);
	}
      else if (op == DW_LNS_extended_op)
	{
	  uint64_t len;

	  len = read_uleb128 (line_buf);
	  op = read_byte (line_buf);
	  switch (op)
	    {
	    case DW_LNE_end_sequence:
	      /* FIXME: Should we mark the high PC here?  It seems
		 that we already have that information from the
		 compilation unit.  */
	      address = 0;
	      op_index = 0;
	      filename = reset_filename;
	      lineno = 1;
	      break;
	    case DW_LNE_set_address:
	      address = read_address (line_buf, hdr->addrsize);
	      break;
	    case DW_LNE_define_file:
	      {
		const char *f;
		unsigned int dir_index;

		f = read_string (line_buf);
		if (f == NULL)
		  return 0;
		dir_index = read_uleb128 (line_buf);
		/* Ignore that time and length.  */
		read_uleb128 (line_buf);
		read_uleb128 (line_buf);
		if (IS_ABSOLUTE_PATH (f))
		  filename = f;
		else
		  {
		    const char *dir;
		    size_t dir_len;
		    size_t f_len;
		    char *p;

		    if (dir_index < hdr->dirs_count)
		      dir = hdr->dirs[dir_index];
		    else
		      {
			dwarf_buf_error (line_buf,
					 ("invalid directory index "
					  "in line number program"),
					 0);
			return 0;
		      }
		    dir_len = strlen (dir);
		    f_len = strlen (f);
		    p = ((char *)
			 backtrace_alloc (state, dir_len + f_len + 2,
					  line_buf->error_callback,
					  line_buf->data));
		    if (p == NULL)
		      return 0;
		    memcpy (p, dir, dir_len);
		    /* FIXME: If we are on a DOS-based file system,
		       and the directory or the file name use
		       backslashes, then we should use a backslash
		       here.  */
		    p[dir_len] = '/';
		    memcpy (p + dir_len + 1, f, f_len + 1);
		    filename = p;
		  }
	      }
	      break;
	    case DW_LNE_set_discriminator:
	      /* We don't care about discriminators.  */
	      read_uleb128 (line_buf);
	      break;
	    default:
	      if (!advance (line_buf, len - 1))
		return 0;
	      break;
	    }
	}
      else
	{
	  switch (op)
	    {
	    case DW_LNS_copy:
	      add_line (state, ddata, address, filename, lineno,
			line_buf->error_callback, line_buf->data, vec);
	      break;
	    case DW_LNS_advance_pc:
	      {
		uint64_t advance;

		advance = read_uleb128 (line_buf);
		address += (hdr->min_insn_len * (op_index + advance)
			    / hdr->max_ops_per_insn);
		op_index = (op_index + advance) % hdr->max_ops_per_insn;
	      }
	      break;
	    case DW_LNS_advance_line:
	      lineno += (int) read_sleb128 (line_buf);
	      break;
	    case DW_LNS_set_file:
	      {
		uint64_t fileno;

		fileno = read_uleb128 (line_buf);
		if (fileno >= hdr->filenames_count)
		  {
		    dwarf_buf_error (line_buf,
				     ("invalid file number in "
				      "line number program"),
				     0);
		    return 0;
		  }
		filename = hdr->filenames[fileno];
	      }
	      break;
	    case DW_LNS_set_column:
	      read_uleb128 (line_buf);
	      break;
	    case DW_LNS_negate_stmt:
	      break;
	    case DW_LNS_set_basic_block:
	      break;
	    case DW_LNS_const_add_pc:
	      {
		unsigned int advance;

		op = 255 - hdr->opcode_base;
		advance = op / hdr->line_range;
		address += (hdr->min_insn_len * (op_index + advance)
			    / hdr->max_ops_per_insn);
		op_index = (op_index + advance) % hdr->max_ops_per_insn;
	      }
	      break;
	    case DW_LNS_fixed_advance_pc:
	      address += read_uint16 (line_buf);
	      op_index = 0;
	      break;
	    case DW_LNS_set_prologue_end:
	      break;
	    case DW_LNS_set_epilogue_begin:
	      break;
	    case DW_LNS_set_isa:
	      read_uleb128 (line_buf);
	      break;
	    default:
	      {
		unsigned int i;

		for (i = hdr->opcode_lengths[op - 1]; i > 0; --i)
		  read_uleb128 (line_buf);
	      }
	      break;
	    }
	}
    }

  return 1;
}

/* Read the line number information for a compilation unit.  Returns 1
   on success, 0 on failure.  */

static int
read_line_info (struct backtrace_state *state, struct dwarf_data *ddata,
		backtrace_error_callback error_callback, void *data,
		struct unit *u, struct line_header *hdr, struct line **lines,
		size_t *lines_count)
{
  struct line_vector vec;
  struct dwarf_buf line_buf;
  uint64_t len;
  int is_dwarf64;
  struct line *ln;

  memset (&vec.vec, 0, sizeof vec.vec);
  vec.count = 0;

  memset (hdr, 0, sizeof *hdr);

  if (u->lineoff != (off_t) (size_t) u->lineoff
      || (size_t) u->lineoff >= ddata->dwarf_sections.size[DEBUG_LINE])
    {
      error_callback (data, "unit line offset out of range", 0);
      goto fail;
    }

  line_buf.name = ".debug_line";
  line_buf.start = ddata->dwarf_sections.data[DEBUG_LINE];
  line_buf.buf = ddata->dwarf_sections.data[DEBUG_LINE] + u->lineoff;
  line_buf.left = ddata->dwarf_sections.size[DEBUG_LINE] - u->lineoff;
  line_buf.is_bigendian = ddata->is_bigendian;
  line_buf.error_callback = error_callback;
  line_buf.data = data;
  line_buf.reported_underflow = 0;

  len = read_initial_length (&line_buf, &is_dwarf64);
  line_buf.left = len;

  if (!read_line_header (state, ddata, u, is_dwarf64, &line_buf, hdr))
    goto fail;

  if (!read_line_program (state, ddata, hdr, &line_buf, &vec))
    goto fail;

  if (line_buf.reported_underflow)
    goto fail;

  if (vec.count == 0)
    {
      /* This is not a failure in the sense of a generating an error,
	 but it is a failure in that sense that we have no useful
	 information.  */
      goto fail;
    }

  /* Allocate one extra entry at the end.  */
  ln = ((struct line *)
	backtrace_vector_grow (state, sizeof (struct line), error_callback,
			       data, &vec.vec));
  if (ln == NULL)
    goto fail;
  ln->pc = (uintptr_t) -1;
  ln->filename = NULL;
  ln->lineno = 0;
  ln->idx = 0;

  if (!backtrace_vector_release (state, &vec.vec, error_callback, data))
    goto fail;

  ln = (struct line *) vec.vec.base;
  backtrace_qsort (ln, vec.count, sizeof (struct line), line_compare);

  *lines = ln;
  *lines_count = vec.count;

  return 1;

 fail:
  backtrace_vector_free (state, &vec.vec, error_callback, data);
  free_line_header (state, hdr, error_callback, data);
  *lines = (struct line *) (uintptr_t) -1;
  *lines_count = 0;
  return 0;
}

static const char *read_referenced_name (struct dwarf_data *, struct unit *,
					 uint64_t, backtrace_error_callback,
					 void *);

/* Read the name of a function from a DIE referenced by ATTR with VAL.  */

static const char *
read_referenced_name_from_attr (struct dwarf_data *ddata, struct unit *u,
				struct attr *attr, struct attr_val *val,
				backtrace_error_callback error_callback,
				void *data)
{
  switch (attr->name)
    {
    case DW_AT_abstract_origin:
    case DW_AT_specification:
      break;
    default:
      return NULL;
    }

  if (attr->form == DW_FORM_ref_sig8)
    return NULL;

  if (val->encoding == ATTR_VAL_REF_INFO)
    {
      struct unit *unit
	= find_unit (ddata->units, ddata->units_count,
		     val->u.uint);
      if (unit == NULL)
	return NULL;

      uint64_t offset = val->u.uint - unit->low_offset;
      return read_referenced_name (ddata, unit, offset, error_callback, data);
    }

  if (val->encoding == ATTR_VAL_UINT
      || val->encoding == ATTR_VAL_REF_UNIT)
    return read_referenced_name (ddata, u, val->u.uint, error_callback, data);

  if (val->encoding == ATTR_VAL_REF_ALT_INFO)
    {
      struct unit *alt_unit
	= find_unit (ddata->altlink->units, ddata->altlink->units_count,
		     val->u.uint);
      if (alt_unit == NULL)
	return NULL;

      uint64_t offset = val->u.uint - alt_unit->low_offset;
      return read_referenced_name (ddata->altlink, alt_unit, offset,
				   error_callback, data);
    }

  return NULL;
}

/* Read the name of a function from a DIE referenced by a
   DW_AT_abstract_origin or DW_AT_specification tag.  OFFSET is within
   the same compilation unit.  */

static const char *
read_referenced_name (struct dwarf_data *ddata, struct unit *u,
		      uint64_t offset, backtrace_error_callback error_callback,
		      void *data)
{
  struct dwarf_buf unit_buf;
  uint64_t code;
  const struct abbrev *abbrev;
  const char *ret;
  size_t i;

  /* OFFSET is from the start of the data for this compilation unit.
     U->unit_data is the data, but it starts U->unit_data_offset bytes
     from the beginning.  */

  if (offset < u->unit_data_offset
      || offset - u->unit_data_offset >= u->unit_data_len)
    {
      error_callback (data,
		      "abstract origin or specification out of range",
		      0);
      return NULL;
    }

  offset -= u->unit_data_offset;

  unit_buf.name = ".debug_info";
  unit_buf.start = ddata->dwarf_sections.data[DEBUG_INFO];
  unit_buf.buf = u->unit_data + offset;
  unit_buf.left = u->unit_data_len - offset;
  unit_buf.is_bigendian = ddata->is_bigendian;
  unit_buf.error_callback = error_callback;
  unit_buf.data = data;
  unit_buf.reported_underflow = 0;

  code = read_uleb128 (&unit_buf);
  if (code == 0)
    {
      dwarf_buf_error (&unit_buf,
		      "invalid abstract origin or specification",
		      0);
      return NULL;
    }

  abbrev = lookup_abbrev (&u->abbrevs, code, error_callback, data);
  if (abbrev == NULL)
    return NULL;

  ret = NULL;
  for (i = 0; i < abbrev->num_attrs; ++i)
    {
      struct attr_val val;

      if (!read_attribute (abbrev->attrs[i].form, abbrev->attrs[i].val,
			   &unit_buf, u->is_dwarf64, u->version, u->addrsize,
			   &ddata->dwarf_sections, ddata->altlink, &val))
	return NULL;

      switch (abbrev->attrs[i].name)
	{
	case DW_AT_name:
	  /* Third name preference: don't override.  A name we found in some
	     other way, will normally be more useful -- e.g., this name is
	     normally not mangled.  */
	  if (ret != NULL)
	    break;
	  if (!resolve_string (&ddata->dwarf_sections, u->is_dwarf64,
			       ddata->is_bigendian, u->str_offsets_base,
			       &val, error_callback, data, &ret))
	    return NULL;
	  break;

	case DW_AT_linkage_name:
	case DW_AT_MIPS_linkage_name:
	  /* First name preference: override all.  */
	  {
	    const char *s;

	    s = NULL;
	    if (!resolve_string (&ddata->dwarf_sections, u->is_dwarf64,
				 ddata->is_bigendian, u->str_offsets_base,
				 &val, error_callback, data, &s))
	      return NULL;
	    if (s != NULL)
	      return s;
	  }
	  break;

	case DW_AT_specification:
	  /* Second name preference: override DW_AT_name, don't override
	     DW_AT_linkage_name.  */
	  {
	    const char *name;

	    name = read_referenced_name_from_attr (ddata, u, &abbrev->attrs[i],
						   &val, error_callback, data);
	    if (name != NULL)
	      ret = name;
	  }
	  break;

	default:
	  break;
	}
    }

  return ret;
}

/* Add a range to a unit that maps to a function.  This is called via
   add_ranges.  Returns 1 on success, 0 on error.  */

static int
add_function_range (struct backtrace_state *state, void *rdata,
		    uint64_t lowpc, uint64_t highpc,
		    backtrace_error_callback error_callback, void *data,
		    void *pvec)
{
  struct function *function = (struct function *) rdata;
  struct function_vector *vec = (struct function_vector *) pvec;
  struct function_addrs *p;

  if (vec->count > 0)
    {
      p = (struct function_addrs *) vec->vec.base + (vec->count - 1);
      if ((lowpc == p->high || lowpc == p->high + 1)
	  && function == p->function)
	{
	  if (highpc > p->high)
	    p->high = highpc;
	  return 1;
	}
    }

  p = ((struct function_addrs *)
       backtrace_vector_grow (state, sizeof (struct function_addrs),
			      error_callback, data, &vec->vec));
  if (p == NULL)
    return 0;

  p->low = lowpc;
  p->high = highpc;
  p->function = function;

  ++vec->count;

  return 1;
}

/* Read one entry plus all its children.  Add function addresses to
   VEC.  Returns 1 on success, 0 on error.  */

static int
read_function_entry (struct backtrace_state *state, struct dwarf_data *ddata,
		     struct unit *u, uint64_t base, struct dwarf_buf *unit_buf,
		     const struct line_header *lhdr,
		     backtrace_error_callback error_callback, void *data,
		     struct function_vector *vec_function,
		     struct function_vector *vec_inlined)
{
  while (unit_buf->left > 0)
    {
      uint64_t code;
      const struct abbrev *abbrev;
      int is_function;
      struct function *function;
      struct function_vector *vec;
      size_t i;
      struct pcrange pcrange;
      int have_linkage_name;

      code = read_uleb128 (unit_buf);
      if (code == 0)
	return 1;

      abbrev = lookup_abbrev (&u->abbrevs, code, error_callback, data);
      if (abbrev == NULL)
	return 0;

      is_function = (abbrev->tag == DW_TAG_subprogram
		     || abbrev->tag == DW_TAG_entry_point
		     || abbrev->tag == DW_TAG_inlined_subroutine);

      if (abbrev->tag == DW_TAG_inlined_subroutine)
	vec = vec_inlined;
      else
	vec = vec_function;

      function = NULL;
      if (is_function)
	{
	  function = ((struct function *)
		      backtrace_alloc (state, sizeof *function,
				       error_callback, data));
	  if (function == NULL)
	    return 0;
	  memset (function, 0, sizeof *function);
	}

      memset (&pcrange, 0, sizeof pcrange);
      have_linkage_name = 0;
      for (i = 0; i < abbrev->num_attrs; ++i)
	{
	  struct attr_val val;

	  if (!read_attribute (abbrev->attrs[i].form, abbrev->attrs[i].val,
			       unit_buf, u->is_dwarf64, u->version,
			       u->addrsize, &ddata->dwarf_sections,
			       ddata->altlink, &val))
	    return 0;

	  /* The compile unit sets the base address for any address
	     ranges in the function entries.  */
	  if (abbrev->tag == DW_TAG_compile_unit
	      && abbrev->attrs[i].name == DW_AT_low_pc)
	    {
	      if (val.encoding == ATTR_VAL_ADDRESS)
		base = val.u.uint;
	      else if (val.encoding == ATTR_VAL_ADDRESS_INDEX)
		{
		  if (!resolve_addr_index (&ddata->dwarf_sections,
					   u->addr_base, u->addrsize,
					   ddata->is_bigendian, val.u.uint,
					   error_callback, data, &base))
		    return 0;
		}
	    }

	  if (is_function)
	    {
	      switch (abbrev->attrs[i].name)
		{
		case DW_AT_call_file:
		  if (val.encoding == ATTR_VAL_UINT)
		    {
		      if (val.u.uint >= lhdr->filenames_count)
			{
			  dwarf_buf_error (unit_buf,
					   ("invalid file number in "
					    "DW_AT_call_file attribute"),
					   0);
			  return 0;
			}
		      function->caller_filename = lhdr->filenames[val.u.uint];
		    }
		  break;

		case DW_AT_call_line:
		  if (val.encoding == ATTR_VAL_UINT)
		    function->caller_lineno = val.u.uint;
		  break;

		case DW_AT_abstract_origin:
		case DW_AT_specification:
		  /* Second name preference: override DW_AT_name, don't override
		     DW_AT_linkage_name.  */
		  if (have_linkage_name)
		    break;
		  {
		    const char *name;

		    name
		      = read_referenced_name_from_attr (ddata, u,
							&abbrev->attrs[i], &val,
							error_callback, data);
		    if (name != NULL)
		      function->name = name;
		  }
		  break;

		case DW_AT_name:
		  /* Third name preference: don't override.  */
		  if (function->name != NULL)
		    break;
		  if (!resolve_string (&ddata->dwarf_sections, u->is_dwarf64,
				       ddata->is_bigendian,
				       u->str_offsets_base, &val,
				       error_callback, data, &function->name))
		    return 0;
		  break;

		case DW_AT_linkage_name:
		case DW_AT_MIPS_linkage_name:
		  /* First name preference: override all.  */
		  {
		    const char *s;

		    s = NULL;
		    if (!resolve_string (&ddata->dwarf_sections, u->is_dwarf64,
					 ddata->is_bigendian,
					 u->str_offsets_base, &val,
					 error_callback, data, &s))
		      return 0;
		    if (s != NULL)
		      {
			function->name = s;
			have_linkage_name = 1;
		      }
		  }
		  break;

		case DW_AT_low_pc: case DW_AT_high_pc: case DW_AT_ranges:
		  update_pcrange (&abbrev->attrs[i], &val, &pcrange);
		  break;

		default:
		  break;
		}
	    }
	}

      /* If we couldn't find a name for the function, we have no use
	 for it.  */
      if (is_function && function->name == NULL)
	{
	  backtrace_free (state, function, sizeof *function,
			  error_callback, data);
	  is_function = 0;
	}

      if (is_function)
	{
	  if (pcrange.have_ranges
	      || (pcrange.have_lowpc && pcrange.have_highpc))
	    {
	      if (!add_ranges (state, &ddata->dwarf_sections,
			       ddata->base_address, ddata->is_bigendian,
			       u, base, &pcrange, add_function_range,
			       (void *) function, error_callback, data,
			       (void *) vec))
		return 0;
	    }
	  else
	    {
	      backtrace_free (state, function, sizeof *function,
			      error_callback, data);
	      is_function = 0;
	    }
	}

      if (abbrev->has_children)
	{
	  if (!is_function)
	    {
	      if (!read_function_entry (state, ddata, u, base, unit_buf, lhdr,
					error_callback, data, vec_function,
					vec_inlined))
		return 0;
	    }
	  else
	    {
	      struct function_vector fvec;

	      /* Gather any information for inlined functions in
		 FVEC.  */

	      memset (&fvec, 0, sizeof fvec);

	      if (!read_function_entry (state, ddata, u, base, unit_buf, lhdr,
					error_callback, data, vec_function,
					&fvec))
		return 0;

	      if (fvec.count > 0)
		{
		  struct function_addrs *p;
		  struct function_addrs *faddrs;

		  /* Allocate a trailing entry, but don't include it
		     in fvec.count.  */
		  p = ((struct function_addrs *)
		       backtrace_vector_grow (state,
					      sizeof (struct function_addrs),
					      error_callback, data,
					      &fvec.vec));
		  if (p == NULL)
		    return 0;
		  p->low = 0;
		  --p->low;
		  p->high = p->low;
		  p->function = NULL;

		  if (!backtrace_vector_release (state, &fvec.vec,
						 error_callback, data))
		    return 0;

		  faddrs = (struct function_addrs *) fvec.vec.base;
		  backtrace_qsort (faddrs, fvec.count,
				   sizeof (struct function_addrs),
				   function_addrs_compare);

		  function->function_addrs = faddrs;
		  function->function_addrs_count = fvec.count;
		}
	    }
	}
    }

  return 1;
}

/* Read function name information for a compilation unit.  We look
   through the whole unit looking for function tags.  */

static void
read_function_info (struct backtrace_state *state, struct dwarf_data *ddata,
		    const struct line_header *lhdr,
		    backtrace_error_callback error_callback, void *data,
		    struct unit *u, struct function_vector *fvec,
		    struct function_addrs **ret_addrs,
		    size_t *ret_addrs_count)
{
  struct function_vector lvec;
  struct function_vector *pfvec;
  struct dwarf_buf unit_buf;
  struct function_addrs *p;
  struct function_addrs *addrs;
  size_t addrs_count;

  /* Use FVEC if it is not NULL.  Otherwise use our own vector.  */
  if (fvec != NULL)
    pfvec = fvec;
  else
    {
      memset (&lvec, 0, sizeof lvec);
      pfvec = &lvec;
    }

  unit_buf.name = ".debug_info";
  unit_buf.start = ddata->dwarf_sections.data[DEBUG_INFO];
  unit_buf.buf = u->unit_data;
  unit_buf.left = u->unit_data_len;
  unit_buf.is_bigendian = ddata->is_bigendian;
  unit_buf.error_callback = error_callback;
  unit_buf.data = data;
  unit_buf.reported_underflow = 0;

  while (unit_buf.left > 0)
    {
      if (!read_function_entry (state, ddata, u, 0, &unit_buf, lhdr,
				error_callback, data, pfvec, pfvec))
	return;
    }

  if (pfvec->count == 0)
    return;

  /* Allocate a trailing entry, but don't include it in
     pfvec->count.  */
  p = ((struct function_addrs *)
       backtrace_vector_grow (state, sizeof (struct function_addrs),
			      error_callback, data, &pfvec->vec));
  if (p == NULL)
    return;
  p->low = 0;
  --p->low;
  p->high = p->low;
  p->function = NULL;

  addrs_count = pfvec->count;

  if (fvec == NULL)
    {
      if (!backtrace_vector_release (state, &lvec.vec, error_callback, data))
	return;
      addrs = (struct function_addrs *) pfvec->vec.base;
    }
  else
    {
      /* Finish this list of addresses, but leave the remaining space in
	 the vector available for the next function unit.  */
      addrs = ((struct function_addrs *)
	       backtrace_vector_finish (state, &fvec->vec,
					error_callback, data));
      if (addrs == NULL)
	return;
      fvec->count = 0;
    }

  backtrace_qsort (addrs, addrs_count, sizeof (struct function_addrs),
		   function_addrs_compare);

  *ret_addrs = addrs;
  *ret_addrs_count = addrs_count;
}

/* See if PC is inlined in FUNCTION.  If it is, print out the inlined
   information, and update FILENAME and LINENO for the caller.
   Returns whatever CALLBACK returns, or 0 to keep going.  */

static int
report_inlined_functions (uintptr_t pc, struct function *function,
			  backtrace_full_callback callback, void *data,
			  const char **filename, int *lineno)
{
  struct function_addrs *p;
  struct function_addrs *match;
  struct function *inlined;
  int ret;

  if (function->function_addrs_count == 0)
    return 0;

  /* Our search isn't safe if pc == -1, as that is the sentinel
     value.  */
  if (pc + 1 == 0)
    return 0;

  p = ((struct function_addrs *)
       bsearch (&pc, function->function_addrs,
		function->function_addrs_count,
		sizeof (struct function_addrs),
		function_addrs_search));
  if (p == NULL)
    return 0;

  /* Here pc >= p->low && pc < (p + 1)->low.  The function_addrs are
     sorted by low, so if pc > p->low we are at the end of a range of
     function_addrs with the same low value.  If pc == p->low walk
     forward to the end of the range with that low value.  Then walk
     backward and use the first range that includes pc.  */
  while (pc == (p + 1)->low)
    ++p;
  match = NULL;
  while (1)
    {
      if (pc < p->high)
	{
	  match = p;
	  break;
	}
      if (p == function->function_addrs)
	break;
      if ((p - 1)->low < p->low)
	break;
      --p;
    }
  if (match == NULL)
    return 0;

  /* We found an inlined call.  */

  inlined = match->function;

  /* Report any calls inlined into this one.  */
  ret = report_inlined_functions (pc, inlined, callback, data,
				  filename, lineno);
  if (ret != 0)
    return ret;

  /* Report this inlined call.  */
  ret = callback (data, pc, match->low, *filename, *lineno, inlined->name);
  if (ret != 0)
    return ret;

  /* Our caller will report the caller of the inlined function; tell
     it the appropriate filename and line number.  */
  *filename = inlined->caller_filename;
  *lineno = inlined->caller_lineno;

  return 0;
}

/* Look for a PC in the DWARF mapping for one module.  On success,
   call CALLBACK and return whatever it returns.  On error, call
   ERROR_CALLBACK and return 0.  Sets *FOUND to 1 if the PC is found,
   0 if not.  */

static int
dwarf_lookup_pc (struct backtrace_state *state, struct dwarf_data *ddata,
		 uintptr_t pc, backtrace_full_callback callback,
		 backtrace_error_callback error_callback, void *data,
		 int *found)
{
  struct unit_addrs *entry;
  int found_entry;
  struct unit *u;
  int new_data;
  struct line *lines;
  struct line *ln;
  struct function_addrs *p;
  struct function_addrs *fmatch;
  struct function *function;
  const char *filename;
  int lineno;
  int ret;

  *found = 1;

  /* Find an address range that includes PC.  Our search isn't safe if
     PC == -1, as we use that as a sentinel value, so skip the search
     in that case.  */
  entry = (ddata->addrs_count == 0 || pc + 1 == 0
	   ? NULL
	   : (struct unit_addrs*)bsearch (&pc, ddata->addrs, ddata->addrs_count,
		      sizeof (struct unit_addrs), unit_addrs_search));

  if (entry == NULL)
    {
      *found = 0;
      return 0;
    }

  /* Here pc >= entry->low && pc < (entry + 1)->low.  The unit_addrs
     are sorted by low, so if pc > p->low we are at the end of a range
     of unit_addrs with the same low value.  If pc == p->low walk
     forward to the end of the range with that low value.  Then walk
     backward and use the first range that includes pc.  */
  while (pc == (entry + 1)->low)
    ++entry;
  found_entry = 0;
  while (1)
    {
      if (pc < entry->high)
	{
	  found_entry = 1;
	  break;
	}
      if (entry == ddata->addrs)
	break;
      if ((entry - 1)->low < entry->low)
	break;
      --entry;
    }
  if (!found_entry)
    {
      *found = 0;
      return 0;
    }

  /* We need the lines, lines_count, function_addrs,
     function_addrs_count fields of u.  If they are not set, we need
     to set them.  When running in threaded mode, we need to allow for
     the possibility that some other thread is setting them
     simultaneously.  */

  u = entry->u;
  lines = u->lines;

  /* Skip units with no useful line number information by walking
     backward.  Useless line number information is marked by setting
     lines == -1.  */
  while (entry > ddata->addrs
	 && pc >= (entry - 1)->low
	 && pc < (entry - 1)->high)
    {
      if (state->threaded)
	lines = (struct line *) backtrace_atomic_load_pointer (&u->lines);

      if (lines != (struct line *) (uintptr_t) -1)
	break;

      --entry;

      u = entry->u;
      lines = u->lines;
    }

  if (state->threaded)
    lines = backtrace_atomic_load_pointer (&u->lines);

  new_data = 0;
  if (lines == NULL)
    {
      struct function_addrs *function_addrs;
      size_t function_addrs_count;
      struct line_header lhdr;
      size_t count;

      /* We have never read the line information for this unit.  Read
	 it now.  */

      function_addrs = NULL;
      function_addrs_count = 0;
      if (read_line_info (state, ddata, error_callback, data, entry->u, &lhdr,
			  &lines, &count))
	{
	  struct function_vector *pfvec;

	  /* If not threaded, reuse DDATA->FVEC for better memory
	     consumption.  */
	  if (state->threaded)
	    pfvec = NULL;
	  else
	    pfvec = &ddata->fvec;
	  read_function_info (state, ddata, &lhdr, error_callback, data,
			      entry->u, pfvec, &function_addrs,
			      &function_addrs_count);
	  free_line_header (state, &lhdr, error_callback, data);
	  new_data = 1;
	}

      /* Atomically store the information we just read into the unit.
	 If another thread is simultaneously writing, it presumably
	 read the same information, and we don't care which one we
	 wind up with; we just leak the other one.  We do have to
	 write the lines field last, so that the acquire-loads above
	 ensure that the other fields are set.  */

      if (!state->threaded)
	{
	  u->lines_count = count;
	  u->function_addrs = function_addrs;
	  u->function_addrs_count = function_addrs_count;
	  u->lines = lines;
	}
      else
	{
	  backtrace_atomic_store_size_t (&u->lines_count, count);
	  backtrace_atomic_store_pointer (&u->function_addrs, function_addrs);
	  backtrace_atomic_store_size_t (&u->function_addrs_count,
					 function_addrs_count);
	  backtrace_atomic_store_pointer (&u->lines, lines);
	}
    }

  /* Now all fields of U have been initialized.  */

  if (lines == (struct line *) (uintptr_t) -1)
    {
      /* If reading the line number information failed in some way,
	 try again to see if there is a better compilation unit for
	 this PC.  */
      if (new_data)
	return dwarf_lookup_pc (state, ddata, pc, callback, error_callback,
				data, found);
      return callback (data, pc, 0, NULL, 0, NULL);
    }

  /* Search for PC within this unit.  */

  ln = (struct line *) bsearch (&pc, lines, entry->u->lines_count,
				sizeof (struct line), line_search);
  if (ln == NULL)
    {
      /* The PC is between the low_pc and high_pc attributes of the
	 compilation unit, but no entry in the line table covers it.
	 This implies that the start of the compilation unit has no
	 line number information.  */

      if (entry->u->abs_filename == NULL)
	{
	  const char *filename;

	  filename = entry->u->filename;
	  if (filename != NULL
	      && !IS_ABSOLUTE_PATH (filename)
	      && entry->u->comp_dir != NULL)
	    {
	      size_t filename_len;
	      const char *dir;
	      size_t dir_len;
	      char *s;

	      filename_len = strlen (filename);
	      dir = entry->u->comp_dir;
	      dir_len = strlen (dir);
	      s = (char *) backtrace_alloc (state, dir_len + filename_len + 2,
					    error_callback, data);
	      if (s == NULL)
		{
		  *found = 0;
		  return 0;
		}
	      memcpy (s, dir, dir_len);
	      /* FIXME: Should use backslash if DOS file system.  */
	      s[dir_len] = '/';
	      memcpy (s + dir_len + 1, filename, filename_len + 1);
	      filename = s;
	    }
	  entry->u->abs_filename = filename;
	}

      return callback (data, pc, 0, entry->u->abs_filename, 0, NULL);
    }

  /* Search for function name within this unit.  */

  if (entry->u->function_addrs_count == 0)
    return callback (data, pc, 0, ln->filename, ln->lineno, NULL);

  p = ((struct function_addrs *)
       bsearch (&pc, entry->u->function_addrs,
		entry->u->function_addrs_count,
		sizeof (struct function_addrs),
		function_addrs_search));
  if (p == NULL)
    return callback (data, pc, 0, ln->filename, ln->lineno, NULL);

  /* Here pc >= p->low && pc < (p + 1)->low.  The function_addrs are
     sorted by low, so if pc > p->low we are at the end of a range of
     function_addrs with the same low value.  If pc == p->low walk
     forward to the end of the range with that low value.  Then walk
     backward and use the first range that includes pc.  */
  while (pc == (p + 1)->low)
    ++p;
  fmatch = NULL;
  while (1)
    {
      if (pc < p->high)
	{
	  fmatch = p;
	  break;
	}
      if (p == entry->u->function_addrs)
	break;
      if ((p - 1)->low < p->low)
	break;
      --p;
    }
  if (fmatch == NULL)
    return callback (data, pc, 0, ln->filename, ln->lineno, NULL);

  function = fmatch->function;

  filename = ln->filename;
  lineno = ln->lineno;

  ret = report_inlined_functions (pc, function, callback, data,
				  &filename, &lineno);
  if (ret != 0)
    return ret;

  return callback (data, pc, fmatch->low, filename, lineno, function->name);
}

/* Return the file/line information for a PC using the DWARF mapping
   we built earlier.  */

static int
dwarf_fileline (struct backtrace_state *state, uintptr_t pc,
		backtrace_full_callback callback,
		backtrace_error_callback error_callback, void *data)
{
  struct dwarf_data *ddata;
  int found;
  int ret;

  if (!state->threaded)
    {
      for (ddata = (struct dwarf_data *) state->fileline_data;
	   ddata != NULL;
	   ddata = ddata->next)
	{
	  ret = dwarf_lookup_pc (state, ddata, pc, callback, error_callback,
				 data, &found);
	  if (ret != 0 || found)
	    return ret;
	}
    }
  else
    {
      struct dwarf_data **pp;

      pp = (struct dwarf_data **) (void *) &state->fileline_data;
      while (1)
	{
	  ddata = backtrace_atomic_load_pointer (pp);
	  if (ddata == NULL)
	    break;

	  ret = dwarf_lookup_pc (state, ddata, pc, callback, error_callback,
				 data, &found);
	  if (ret != 0 || found)
	    return ret;

	  pp = &ddata->next;
	}
    }

  /* FIXME: See if any libraries have been dlopen'ed.  */

  return callback (data, pc, 0, NULL, 0, NULL);
}

/* Initialize our data structures from the DWARF debug info for a
   file.  Return NULL on failure.  */

static struct dwarf_data *
build_dwarf_data (struct backtrace_state *state,
		  uintptr_t base_address,
		  const struct dwarf_sections *dwarf_sections,
		  int is_bigendian,
		  struct dwarf_data *altlink,
		  backtrace_error_callback error_callback,
		  void *data)
{
  struct unit_addrs_vector addrs_vec;
  struct unit_addrs *addrs;
  size_t addrs_count;
  struct unit_vector units_vec;
  struct unit **units;
  size_t units_count;
  struct dwarf_data *fdata;

  if (!build_address_map (state, base_address, dwarf_sections, is_bigendian,
			  altlink, error_callback, data, &addrs_vec,
			  &units_vec))
    return NULL;

  if (!backtrace_vector_release (state, &addrs_vec.vec, error_callback, data))
    return NULL;
  if (!backtrace_vector_release (state, &units_vec.vec, error_callback, data))
    return NULL;
  addrs = (struct unit_addrs *) addrs_vec.vec.base;
  units = (struct unit **) units_vec.vec.base;
  addrs_count = addrs_vec.count;
  units_count = units_vec.count;
  backtrace_qsort (addrs, addrs_count, sizeof (struct unit_addrs),
		   unit_addrs_compare);
  /* No qsort for units required, already sorted.  */

  fdata = ((struct dwarf_data *)
	   backtrace_alloc (state, sizeof (struct dwarf_data),
			    error_callback, data));
  if (fdata == NULL)
    return NULL;

  fdata->next = NULL;
  fdata->altlink = altlink;
  fdata->base_address = base_address;
  fdata->addrs = addrs;
  fdata->addrs_count = addrs_count;
  fdata->units = units;
  fdata->units_count = units_count;
  fdata->dwarf_sections = *dwarf_sections;
  fdata->is_bigendian = is_bigendian;
  memset (&fdata->fvec, 0, sizeof fdata->fvec);

  return fdata;
}

/* Build our data structures from the DWARF sections for a module.
   Set FILELINE_FN and STATE->FILELINE_DATA.  Return 1 on success, 0
   on failure.  */

int
backtrace_dwarf_add (struct backtrace_state *state,
		     uintptr_t base_address,
		     const struct dwarf_sections *dwarf_sections,
		     int is_bigendian,
		     struct dwarf_data *fileline_altlink,
		     backtrace_error_callback error_callback,
		     void *data, fileline *fileline_fn,
		     struct dwarf_data **fileline_entry)
{
  struct dwarf_data *fdata;

  fdata = build_dwarf_data (state, base_address, dwarf_sections, is_bigendian,
			    fileline_altlink, error_callback, data);
  if (fdata == NULL)
    return 0;

  if (fileline_entry != NULL)
    *fileline_entry = fdata;

  if (!state->threaded)
    {
      struct dwarf_data **pp;

      for (pp = (struct dwarf_data **) (void *) &state->fileline_data;
	   *pp != NULL;
	   pp = &(*pp)->next)
	;
      *pp = fdata;
    }
  else
    {
      while (1)
	{
	  struct dwarf_data **pp;

	  pp = (struct dwarf_data **) (void *) &state->fileline_data;

	  while (1)
	    {
	      struct dwarf_data *p;

	      p = backtrace_atomic_load_pointer (pp);

	      if (p == NULL)
		break;

	      pp = &p->next;
	    }

	  if (__sync_bool_compare_and_swap (pp, NULL, fdata))
	    break;
	}
    }

  *fileline_fn = dwarf_fileline;

  return 1;
}

}

/*** End of inlined file: dwarf.cpp ***/


/*** Start of inlined file: fileline.cpp ***/

/*** Start of inlined file: config.h ***/
#include <climits>
#if __WORDSIZE == 64
#  define BACKTRACE_ELF_SIZE 64
#else
#  define BACKTRACE_ELF_SIZE 32
#endif

#define HAVE_DLFCN_H 1
#define HAVE_FCNTL 1
#define HAVE_INTTYPES_H 1
#define HAVE_LSTAT 1
#define HAVE_READLINK 1
#define HAVE_DL_ITERATE_PHDR 1
#define HAVE_ATOMIC_FUNCTIONS 1
#define HAVE_DECL_STRNLEN 1

#ifdef __APPLE__
#  define HAVE_MACH_O_DYLD_H 1
#elif defined BSD
#  define HAVE_KERN_PROC 1
#  define HAVE_KERN_PROC_ARGS 1
#endif

/*** End of inlined file: config.h ***/

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#if defined (HAVE_KERN_PROC_ARGS) || defined (HAVE_KERN_PROC)
#include <sys/sysctl.h>
#endif

#ifdef HAVE_MACH_O_DYLD_H
#include <mach-o/dyld.h>
#endif

#ifndef HAVE_GETEXECNAME
#define getexecname() NULL
#endif

namespace tracy
{

#if !defined (HAVE_KERN_PROC_ARGS) && !defined (HAVE_KERN_PROC)

#define sysctl_exec_name1(state, error_callback, data) NULL
#define sysctl_exec_name2(state, error_callback, data) NULL

#else /* defined (HAVE_KERN_PROC_ARGS) || |defined (HAVE_KERN_PROC) */

static char *
sysctl_exec_name (struct backtrace_state *state,
		  int mib0, int mib1, int mib2, int mib3,
		  backtrace_error_callback error_callback, void *data)
{
  int mib[4];
  size_t len;
  char *name;
  size_t rlen;

  mib[0] = mib0;
  mib[1] = mib1;
  mib[2] = mib2;
  mib[3] = mib3;

  if (sysctl (mib, 4, NULL, &len, NULL, 0) < 0)
    return NULL;
  name = (char *) backtrace_alloc (state, len, error_callback, data);
  if (name == NULL)
    return NULL;
  rlen = len;
  if (sysctl (mib, 4, name, &rlen, NULL, 0) < 0)
    {
      backtrace_free (state, name, len, error_callback, data);
      return NULL;
    }
  return name;
}

#ifdef HAVE_KERN_PROC_ARGS

static char *
sysctl_exec_name1 (struct backtrace_state *state,
		   backtrace_error_callback error_callback, void *data)
{
  /* This variant is used on NetBSD.  */
  return sysctl_exec_name (state, CTL_KERN, KERN_PROC_ARGS, -1,
			   KERN_PROC_PATHNAME, error_callback, data);
}

#else

#define sysctl_exec_name1(state, error_callback, data) NULL

#endif

#ifdef HAVE_KERN_PROC

static char *
sysctl_exec_name2 (struct backtrace_state *state,
		   backtrace_error_callback error_callback, void *data)
{
  /* This variant is used on FreeBSD.  */
  return sysctl_exec_name (state, CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1,
			   error_callback, data);
}

#else

#define sysctl_exec_name2(state, error_callback, data) NULL

#endif

#endif /* defined (HAVE_KERN_PROC_ARGS) || |defined (HAVE_KERN_PROC) */

#ifdef HAVE_MACH_O_DYLD_H

static char *
macho_get_executable_path (struct backtrace_state *state,
			   backtrace_error_callback error_callback, void *data)
{
  uint32_t len;
  char *name;

  len = 0;
  if (_NSGetExecutablePath (NULL, &len) == 0)
    return NULL;
  name = (char *) backtrace_alloc (state, len, error_callback, data);
  if (name == NULL)
    return NULL;
  if (_NSGetExecutablePath (name, &len) != 0)
    {
      backtrace_free (state, name, len, error_callback, data);
      return NULL;
    }
  return name;
}

#else /* !defined (HAVE_MACH_O_DYLD_H) */

#define macho_get_executable_path(state, error_callback, data) NULL

#endif /* !defined (HAVE_MACH_O_DYLD_H) */

/* Initialize the fileline information from the executable.  Returns 1
   on success, 0 on failure.  */

static int
fileline_initialize (struct backtrace_state *state,
		     backtrace_error_callback error_callback, void *data)
{
  int failed;
  fileline fileline_fn;
  int pass;
  int called_error_callback;
  int descriptor;
  const char *filename;
  char buf[64];

  if (!state->threaded)
    failed = state->fileline_initialization_failed;
  else
    failed = backtrace_atomic_load_int (&state->fileline_initialization_failed);

  if (failed)
    {
      error_callback (data, "failed to read executable information", -1);
      return 0;
    }

  if (!state->threaded)
    fileline_fn = state->fileline_fn;
  else
    fileline_fn = backtrace_atomic_load_pointer (&state->fileline_fn);
  if (fileline_fn != NULL)
    return 1;

  /* We have not initialized the information.  Do it now.  */

  descriptor = -1;
  called_error_callback = 0;
  for (pass = 0; pass < 8; ++pass)
    {
      int does_not_exist;

      switch (pass)
	{
	case 0:
	  filename = state->filename;
	  break;
	case 1:
	  filename = getexecname ();
	  break;
	case 2:
	  filename = "/proc/self/exe";
	  break;
	case 3:
	  filename = "/proc/curproc/file";
	  break;
	case 4:
	  snprintf (buf, sizeof (buf), "/proc/%ld/object/a.out",
		    (long) getpid ());
	  filename = buf;
	  break;
	case 5:
	  filename = sysctl_exec_name1 (state, error_callback, data);
	  break;
	case 6:
	  filename = sysctl_exec_name2 (state, error_callback, data);
	  break;
	case 7:
	  filename = macho_get_executable_path (state, error_callback, data);
	  break;
	default:
	  abort ();
	}

      if (filename == NULL)
	continue;

      descriptor = backtrace_open (filename, error_callback, data,
				   &does_not_exist);
      if (descriptor < 0 && !does_not_exist)
	{
	  called_error_callback = 1;
	  break;
	}
      if (descriptor >= 0)
	break;
    }

  if (descriptor < 0)
    {
      if (!called_error_callback)
	{
	  if (state->filename != NULL)
	    error_callback (data, state->filename, ENOENT);
	  else
	    error_callback (data,
			    "libbacktrace could not find executable to open",
			    0);
	}
      failed = 1;
    }

  if (!failed)
    {
      if (!backtrace_initialize (state, filename, descriptor, error_callback,
				 data, &fileline_fn))
	failed = 1;
    }

  if (failed)
    {
      if (!state->threaded)
	state->fileline_initialization_failed = 1;
      else
	backtrace_atomic_store_int (&state->fileline_initialization_failed, 1);
      return 0;
    }

  if (!state->threaded)
    state->fileline_fn = fileline_fn;
  else
    {
      backtrace_atomic_store_pointer (&state->fileline_fn, fileline_fn);

      /* Note that if two threads initialize at once, one of the data
	 sets may be leaked.  */
    }

  return 1;
}

/* Given a PC, find the file name, line number, and function name.  */

int
backtrace_pcinfo (struct backtrace_state *state, uintptr_t pc,
		  backtrace_full_callback callback,
		  backtrace_error_callback error_callback, void *data)
{
  if (!fileline_initialize (state, error_callback, data))
    return 0;

  if (state->fileline_initialization_failed)
    return 0;

  return state->fileline_fn (state, pc, callback, error_callback, data);
}

/* Given a PC, find the symbol for it, and its value.  */

int
backtrace_syminfo (struct backtrace_state *state, uintptr_t pc,
		   backtrace_syminfo_callback callback,
		   backtrace_error_callback error_callback, void *data)
{
  if (!fileline_initialize (state, error_callback, data))
    return 0;

  if (state->fileline_initialization_failed)
    return 0;

  state->syminfo_fn (state, pc, callback, error_callback, data);
  return 1;
}

/* A backtrace_syminfo_callback that can call into a
   backtrace_full_callback, used when we have a symbol table but no
   debug info.  */

void
backtrace_syminfo_to_full_callback (void *data, uintptr_t pc,
				    const char *symname,
				    uintptr_t symval ATTRIBUTE_UNUSED,
				    uintptr_t symsize ATTRIBUTE_UNUSED)
{
  struct backtrace_call_full *bdata = (struct backtrace_call_full *) data;

  bdata->ret = bdata->full_callback (bdata->full_data, pc, 0, NULL, 0, symname);
}

/* An error callback that corresponds to
   backtrace_syminfo_to_full_callback.  */

void
backtrace_syminfo_to_full_error_callback (void *data, const char *msg,
					  int errnum)
{
  struct backtrace_call_full *bdata = (struct backtrace_call_full *) data;

  bdata->full_error_callback (bdata->full_data, msg, errnum);
}

}

/*** End of inlined file: fileline.cpp ***/


/*** Start of inlined file: mmapio.cpp ***/

/*** Start of inlined file: config.h ***/
#include <climits>
#if __WORDSIZE == 64
#  define BACKTRACE_ELF_SIZE 64
#else
#  define BACKTRACE_ELF_SIZE 32
#endif

#define HAVE_DLFCN_H 1
#define HAVE_FCNTL 1
#define HAVE_INTTYPES_H 1
#define HAVE_LSTAT 1
#define HAVE_READLINK 1
#define HAVE_DL_ITERATE_PHDR 1
#define HAVE_ATOMIC_FUNCTIONS 1
#define HAVE_DECL_STRNLEN 1

#ifdef __APPLE__
#  define HAVE_MACH_O_DYLD_H 1
#elif defined BSD
#  define HAVE_KERN_PROC 1
#  define HAVE_KERN_PROC_ARGS 1
#endif

/*** End of inlined file: config.h ***/

#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef HAVE_DECL_GETPAGESIZE
extern int getpagesize (void);
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

namespace tracy
{

/* This file implements file views and memory allocation when mmap is
   available.  */

/* Create a view of SIZE bytes from DESCRIPTOR at OFFSET.  */

int
backtrace_get_view (struct backtrace_state *state ATTRIBUTE_UNUSED,
		    int descriptor, off_t offset, uint64_t size,
		    backtrace_error_callback error_callback,
		    void *data, struct backtrace_view *view)
{
  size_t pagesize;
  unsigned int inpage;
  off_t pageoff;
  void *map;

  if ((uint64_t) (size_t) size != size)
    {
      error_callback (data, "file size too large", 0);
      return 0;
    }

  pagesize = getpagesize ();
  inpage = offset % pagesize;
  pageoff = offset - inpage;

  size += inpage;
  size = (size + (pagesize - 1)) & ~ (pagesize - 1);

  map = mmap (NULL, size, PROT_READ, MAP_PRIVATE, descriptor, pageoff);
  if (map == MAP_FAILED)
    {
      error_callback (data, "mmap", errno);
      return 0;
    }

  view->data = (char *) map + inpage;
  view->base = map;
  view->len = size;

  return 1;
}

/* Release a view read by backtrace_get_view.  */

void
backtrace_release_view (struct backtrace_state *state ATTRIBUTE_UNUSED,
			struct backtrace_view *view,
			backtrace_error_callback error_callback,
			void *data)
{
  union {
    const void *cv;
    void *v;
  } cc;

  cc.cv = view->base;
  if (munmap (cc.v, view->len) < 0)
    error_callback (data, "munmap", errno);
}

}

/*** End of inlined file: mmapio.cpp ***/


/*** Start of inlined file: posix.cpp ***/

/*** Start of inlined file: config.h ***/
#include <climits>
#if __WORDSIZE == 64
#  define BACKTRACE_ELF_SIZE 64
#else
#  define BACKTRACE_ELF_SIZE 32
#endif

#define HAVE_DLFCN_H 1
#define HAVE_FCNTL 1
#define HAVE_INTTYPES_H 1
#define HAVE_LSTAT 1
#define HAVE_READLINK 1
#define HAVE_DL_ITERATE_PHDR 1
#define HAVE_ATOMIC_FUNCTIONS 1
#define HAVE_DECL_STRNLEN 1

#ifdef __APPLE__
#  define HAVE_MACH_O_DYLD_H 1
#elif defined BSD
#  define HAVE_KERN_PROC 1
#  define HAVE_KERN_PROC_ARGS 1
#endif

/*** End of inlined file: config.h ***/

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif

namespace tracy
{

/* Open a file for reading.  */

int
backtrace_open (const char *filename, backtrace_error_callback error_callback,
		void *data, int *does_not_exist)
{
  int descriptor;

  if (does_not_exist != NULL)
    *does_not_exist = 0;

  descriptor = open (filename, (int) (O_RDONLY | O_BINARY | O_CLOEXEC));
  if (descriptor < 0)
    {
      /* If DOES_NOT_EXIST is not NULL, then don't call ERROR_CALLBACK
	 if the file does not exist.  We treat lacking permission to
	 open the file as the file not existing; this case arises when
	 running the libgo syscall package tests as root.  */
      if (does_not_exist != NULL && (errno == ENOENT || errno == EACCES))
	*does_not_exist = 1;
      else
	error_callback (data, filename, errno);
      return -1;
    }

#ifdef HAVE_FCNTL
  /* Set FD_CLOEXEC just in case the kernel does not support
     O_CLOEXEC. It doesn't matter if this fails for some reason.
     FIXME: At some point it should be safe to only do this if
     O_CLOEXEC == 0.  */
  fcntl (descriptor, F_SETFD, FD_CLOEXEC);
#endif

  return descriptor;
}

/* Close DESCRIPTOR.  */

int
backtrace_close (int descriptor, backtrace_error_callback error_callback,
		 void *data)
{
  if (close (descriptor) < 0)
    {
      error_callback (data, "close", errno);
      return 0;
    }
  return 1;
}

}

/*** End of inlined file: posix.cpp ***/


/*** Start of inlined file: sort.cpp ***/

/*** Start of inlined file: config.h ***/
#include <climits>
#if __WORDSIZE == 64
#  define BACKTRACE_ELF_SIZE 64
#else
#  define BACKTRACE_ELF_SIZE 32
#endif

#define HAVE_DLFCN_H 1
#define HAVE_FCNTL 1
#define HAVE_INTTYPES_H 1
#define HAVE_LSTAT 1
#define HAVE_READLINK 1
#define HAVE_DL_ITERATE_PHDR 1
#define HAVE_ATOMIC_FUNCTIONS 1
#define HAVE_DECL_STRNLEN 1

#ifdef __APPLE__
#  define HAVE_MACH_O_DYLD_H 1
#elif defined BSD
#  define HAVE_KERN_PROC 1
#  define HAVE_KERN_PROC_ARGS 1
#endif

/*** End of inlined file: config.h ***/

#include <cstddef>
#include <sys/types.h>

namespace tracy
{

/* The GNU glibc version of qsort allocates memory, which we must not
   do if we are invoked by a signal handler.  So provide our own
   sort.  */

static void
swap (char *a, char *b, size_t size)
{
  size_t i;

  for (i = 0; i < size; i++, a++, b++)
    {
      char t;

      t = *a;
      *a = *b;
      *b = t;
    }
}

void
backtrace_qsort (void *basearg, size_t count, size_t size,
		 int (*compar) (const void *, const void *))
{
  char *base = (char *) basearg;
  size_t i;
  size_t mid;

 tail_recurse:
  if (count < 2)
    return;

  /* The symbol table and DWARF tables, which is all we use this
     routine for, tend to be roughly sorted.  Pick the middle element
     in the array as our pivot point, so that we are more likely to
     cut the array in half for each recursion step.  */
  swap (base, base + (count / 2) * size, size);

  mid = 0;
  for (i = 1; i < count; i++)
    {
      if ((*compar) (base, base + i * size) > 0)
	{
	  ++mid;
	  if (i != mid)
	    swap (base + mid * size, base + i * size, size);
	}
    }

  if (mid > 0)
    swap (base, base + mid * size, size);

  /* Recurse with the smaller array, loop with the larger one.  That
     ensures that our maximum stack depth is log count.  */
  if (2 * mid < count)
    {
      backtrace_qsort (base, mid, size, compar);
      base += (mid + 1) * size;
      count -= mid + 1;
      goto tail_recurse;
    }
  else
    {
      backtrace_qsort (base + (mid + 1) * size, count - (mid + 1),
		       size, compar);
      count = mid;
      goto tail_recurse;
    }
}

}

/*** End of inlined file: sort.cpp ***/


/*** Start of inlined file: state.cpp ***/

/*** Start of inlined file: config.h ***/
#include <climits>
#if __WORDSIZE == 64
#  define BACKTRACE_ELF_SIZE 64
#else
#  define BACKTRACE_ELF_SIZE 32
#endif

#define HAVE_DLFCN_H 1
#define HAVE_FCNTL 1
#define HAVE_INTTYPES_H 1
#define HAVE_LSTAT 1
#define HAVE_READLINK 1
#define HAVE_DL_ITERATE_PHDR 1
#define HAVE_ATOMIC_FUNCTIONS 1
#define HAVE_DECL_STRNLEN 1

#ifdef __APPLE__
#  define HAVE_MACH_O_DYLD_H 1
#elif defined BSD
#  define HAVE_KERN_PROC 1
#  define HAVE_KERN_PROC_ARGS 1
#endif

/*** End of inlined file: config.h ***/

#include <string.h>
#include <sys/types.h>

namespace tracy
{

/* Create the backtrace state.  This will then be passed to all the
   other routines.  */

struct backtrace_state *
backtrace_create_state (const char *filename, int threaded,
			backtrace_error_callback error_callback,
			void *data)
{
  struct backtrace_state init_state;
  struct backtrace_state *state;

#ifndef HAVE_SYNC_FUNCTIONS
  if (threaded)
    {
      error_callback (data, "backtrace library does not support threads", 0);
      return NULL;
    }
#endif

  memset (&init_state, 0, sizeof init_state);
  init_state.filename = filename;
  init_state.threaded = threaded;

  state = ((struct backtrace_state *)
	   backtrace_alloc (&init_state, sizeof *state, error_callback, data));
  if (state == NULL)
    return NULL;
  *state = init_state;

  return state;
}

}

/*** End of inlined file: state.cpp ***/

#  if TRACY_HAS_CALLSTACK == 4

/*** Start of inlined file: macho.cpp ***/

/*** Start of inlined file: config.h ***/
#include <climits>
#if __WORDSIZE == 64
#  define BACKTRACE_ELF_SIZE 64
#else
#  define BACKTRACE_ELF_SIZE 32
#endif

#define HAVE_DLFCN_H 1
#define HAVE_FCNTL 1
#define HAVE_INTTYPES_H 1
#define HAVE_LSTAT 1
#define HAVE_READLINK 1
#define HAVE_DL_ITERATE_PHDR 1
#define HAVE_ATOMIC_FUNCTIONS 1
#define HAVE_DECL_STRNLEN 1

#ifdef __APPLE__
#  define HAVE_MACH_O_DYLD_H 1
#elif defined BSD
#  define HAVE_KERN_PROC 1
#  define HAVE_KERN_PROC_ARGS 1
#endif

/*** End of inlined file: config.h ***/

#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_MACH_O_DYLD_H
#include <mach-o/dyld.h>
#endif

namespace tracy
{

/* Mach-O file header for a 32-bit executable.  */

struct macho_header_32
{
  uint32_t magic;	/* Magic number (MACH_O_MAGIC_32) */
  uint32_t cputype;	/* CPU type */
  uint32_t cpusubtype;	/* CPU subtype */
  uint32_t filetype;	/* Type of file (object, executable) */
  uint32_t ncmds;	/* Number of load commands */
  uint32_t sizeofcmds;	/* Total size of load commands */
  uint32_t flags;	/* Flags for special features */
};

/* Mach-O file header for a 64-bit executable.  */

struct macho_header_64
{
  uint32_t magic;	/* Magic number (MACH_O_MAGIC_64) */
  uint32_t cputype;	/* CPU type */
  uint32_t cpusubtype;	/* CPU subtype */
  uint32_t filetype;	/* Type of file (object, executable) */
  uint32_t ncmds;	/* Number of load commands */
  uint32_t sizeofcmds;	/* Total size of load commands */
  uint32_t flags;	/* Flags for special features */
  uint32_t reserved;	/* Reserved */
};

/* Mach-O file header for a fat executable.  */

struct macho_header_fat
{
  uint32_t magic;	/* Magic number (MACH_O_MH_(MAGIC|CIGAM)_FAT(_64)?) */
  uint32_t nfat_arch;   /* Number of components */
};

/* Values for the header magic field.  */

#define MACH_O_MH_MAGIC_32	0xfeedface
#define MACH_O_MH_MAGIC_64	0xfeedfacf
#define MACH_O_MH_MAGIC_FAT	0xcafebabe
#define MACH_O_MH_CIGAM_FAT	0xbebafeca
#define MACH_O_MH_MAGIC_FAT_64	0xcafebabf
#define MACH_O_MH_CIGAM_FAT_64	0xbfbafeca

/* Value for the header filetype field.  */

#define MACH_O_MH_EXECUTE	0x02
#define MACH_O_MH_DYLIB		0x06
#define MACH_O_MH_DSYM		0x0a

/* A component of a fat file.  A fat file starts with a
   macho_header_fat followed by nfat_arch instances of this
   struct.  */

struct macho_fat_arch
{
  uint32_t cputype;	/* CPU type */
  uint32_t cpusubtype;	/* CPU subtype */
  uint32_t offset;	/* File offset of this entry */
  uint32_t size;	/* Size of this entry */
  uint32_t align;	/* Alignment of this entry */
};

/* A component of a 64-bit fat file.  This is used if the magic field
   is MAGIC_FAT_64.  This is only used when some file size or file
   offset is too large to represent in the 32-bit format.  */

struct macho_fat_arch_64
{
  uint32_t cputype;	/* CPU type */
  uint32_t cpusubtype;	/* CPU subtype */
  uint64_t offset;	/* File offset of this entry */
  uint64_t size;	/* Size of this entry */
  uint32_t align;	/* Alignment of this entry */
  uint32_t reserved;	/* Reserved */
};

/* Values for the fat_arch cputype field (and the header cputype
   field).  */

#define MACH_O_CPU_ARCH_ABI64 0x01000000

#define MACH_O_CPU_TYPE_X86 7
#define MACH_O_CPU_TYPE_ARM 12
#define MACH_O_CPU_TYPE_PPC 18

#define MACH_O_CPU_TYPE_X86_64 (MACH_O_CPU_TYPE_X86 | MACH_O_CPU_ARCH_ABI64)
#define MACH_O_CPU_TYPE_ARM64  (MACH_O_CPU_TYPE_ARM | MACH_O_CPU_ARCH_ABI64)
#define MACH_O_CPU_TYPE_PPC64  (MACH_O_CPU_TYPE_PPC | MACH_O_CPU_ARCH_ABI64)

/* The header of a load command.  */

struct macho_load_command
{
  uint32_t cmd;		/* The type of load command */
  uint32_t cmdsize;	/* Size in bytes of the entire command */
};

/* Values for the load_command cmd field.  */

#define MACH_O_LC_SEGMENT	0x01
#define MACH_O_LC_SYMTAB	0x02
#define MACH_O_LC_SEGMENT_64	0x19
#define MACH_O_LC_UUID		0x1b

/* The length of a section of segment name.  */

#define MACH_O_NAMELEN (16)

/* LC_SEGMENT load command.  */

struct macho_segment_command
{
  uint32_t cmd;			/* The type of load command (LC_SEGMENT) */
  uint32_t cmdsize;		/* Size in bytes of the entire command */
  char segname[MACH_O_NAMELEN];	/* Segment name */
  uint32_t vmaddr;		/* Virtual memory address */
  uint32_t vmsize;		/* Virtual memory size */
  uint32_t fileoff;		/* Offset of data to be mapped */
  uint32_t filesize;		/* Size of data in file */
  uint32_t maxprot;		/* Maximum permitted virtual protection */
  uint32_t initprot;		/* Initial virtual memory protection */
  uint32_t nsects;		/* Number of sections in this segment */
  uint32_t flags;		/* Flags */
};

/* LC_SEGMENT_64 load command.  */

struct macho_segment_64_command
{
  uint32_t cmd;			/* The type of load command (LC_SEGMENT) */
  uint32_t cmdsize;		/* Size in bytes of the entire command */
  char segname[MACH_O_NAMELEN];	/* Segment name */
  uint64_t vmaddr;		/* Virtual memory address */
  uint64_t vmsize;		/* Virtual memory size */
  uint64_t fileoff;		/* Offset of data to be mapped */
  uint64_t filesize;		/* Size of data in file */
  uint32_t maxprot;		/* Maximum permitted virtual protection */
  uint32_t initprot;		/* Initial virtual memory protection */
  uint32_t nsects;		/* Number of sections in this segment */
  uint32_t flags;		/* Flags */
};

/* LC_SYMTAB load command.  */

struct macho_symtab_command
{
  uint32_t cmd;		/* The type of load command (LC_SEGMENT) */
  uint32_t cmdsize;	/* Size in bytes of the entire command */
  uint32_t symoff;	/* File offset of symbol table */
  uint32_t nsyms;	/* Number of symbols */
  uint32_t stroff;	/* File offset of string table */
  uint32_t strsize;	/* String table size */
};

/* The length of a Mach-O uuid.  */

#define MACH_O_UUID_LEN (16)

/* LC_UUID load command.  */

struct macho_uuid_command
{
  uint32_t cmd;				/* Type of load command (LC_UUID) */
  uint32_t cmdsize;			/* Size in bytes of command */
  unsigned char uuid[MACH_O_UUID_LEN];	/* UUID */
};

/* 32-bit section header within a LC_SEGMENT segment.  */

struct macho_section
{
  char sectname[MACH_O_NAMELEN];	/* Section name */
  char segment[MACH_O_NAMELEN];		/* Segment of this section */
  uint32_t addr;			/* Address in memory */
  uint32_t size;			/* Section size */
  uint32_t offset;			/* File offset */
  uint32_t align;			/* Log2 of section alignment */
  uint32_t reloff;			/* File offset of relocations */
  uint32_t nreloc;			/* Number of relocs for this section */
  uint32_t flags;			/* Flags */
  uint32_t reserved1;
  uint32_t reserved2;
};

/* 64-bit section header within a LC_SEGMENT_64 segment.   */

struct macho_section_64
{
  char sectname[MACH_O_NAMELEN];	/* Section name */
  char segment[MACH_O_NAMELEN];		/* Segment of this section */
  uint64_t addr;			/* Address in memory */
  uint64_t size;			/* Section size */
  uint32_t offset;			/* File offset */
  uint32_t align;			/* Log2 of section alignment */
  uint32_t reloff;			/* File offset of section relocations */
  uint32_t nreloc;			/* Number of relocs for this section */
  uint32_t flags;			/* Flags */
  uint32_t reserved1;
  uint32_t reserved2;
  uint32_t reserved3;
};

/* 32-bit symbol data.  */

struct macho_nlist
{
  uint32_t n_strx;	/* Index of name in string table */
  uint8_t n_type;	/* Type flag */
  uint8_t n_sect;	/* Section number */
  uint16_t n_desc;	/* Stabs description field */
  uint32_t n_value;	/* Value */
};

/* 64-bit symbol data.  */

struct macho_nlist_64
{
  uint32_t n_strx;	/* Index of name in string table */
  uint8_t n_type;	/* Type flag */
  uint8_t n_sect;	/* Section number */
  uint16_t n_desc;	/* Stabs description field */
  uint64_t n_value;	/* Value */
};

/* Value found in nlist n_type field.  */

#define MACH_O_N_EXT	0x01	/* Extern symbol */
#define MACH_O_N_ABS	0x02	/* Absolute symbol */
#define MACH_O_N_SECT	0x0e	/* Defined in section */

#define MACH_O_N_TYPE	0x0e	/* Mask for type bits */
#define MACH_O_N_STAB	0xe0	/* Stabs debugging symbol */

/* Information we keep for a Mach-O symbol.  */

struct macho_symbol
{
  const char *name;	/* Symbol name */
  uintptr_t address;	/* Symbol address */
};

/* Information to pass to macho_syminfo.  */

struct macho_syminfo_data
{
  struct macho_syminfo_data *next;	/* Next module */
  struct macho_symbol *symbols;		/* Symbols sorted by address */
  size_t count;				/* Number of symbols */
};

/* Names of sections, indexed by enum dwarf_section in internal.h.  */

static const char * const dwarf_section_names[DEBUG_MAX] =
{
  "__debug_info",
  "__debug_line",
  "__debug_abbrev",
  "__debug_ranges",
  "__debug_str",
  "", /* DEBUG_ADDR */
  "__debug_str_offs",
  "", /* DEBUG_LINE_STR */
  "__debug_rnglists"
};

/* Forward declaration.  */

static int macho_add (struct backtrace_state *, const char *, int, off_t,
		      const unsigned char *, uintptr_t, int,
		      backtrace_error_callback, void *, fileline *, int *);

/* A dummy callback function used when we can't find any debug info.  */

static int
macho_nodebug (struct backtrace_state *state ATTRIBUTE_UNUSED,
	       uintptr_t pc ATTRIBUTE_UNUSED,
	       backtrace_full_callback callback ATTRIBUTE_UNUSED,
	       backtrace_error_callback error_callback, void *data)
{
  error_callback (data, "no debug info in Mach-O executable", -1);
  return 0;
}

/* A dummy callback function used when we can't find a symbol
   table.  */

static void
macho_nosyms (struct backtrace_state *state ATTRIBUTE_UNUSED,
	      uintptr_t addr ATTRIBUTE_UNUSED,
	      backtrace_syminfo_callback callback ATTRIBUTE_UNUSED,
	      backtrace_error_callback error_callback, void *data)
{
  error_callback (data, "no symbol table in Mach-O executable", -1);
}

/* Add a single DWARF section to DWARF_SECTIONS, if we need the
   section.  Returns 1 on success, 0 on failure.  */

static int
macho_add_dwarf_section (struct backtrace_state *state, int descriptor,
			 const char *sectname, uint32_t offset, uint64_t size,
			 backtrace_error_callback error_callback, void *data,
			 struct dwarf_sections *dwarf_sections)
{
  int i;

  for (i = 0; i < (int) DEBUG_MAX; ++i)
    {
      if (dwarf_section_names[i][0] != '\0'
	  && strncmp (sectname, dwarf_section_names[i], MACH_O_NAMELEN) == 0)
	{
	  struct backtrace_view section_view;

	  /* FIXME: Perhaps it would be better to try to use a single
	     view to read all the DWARF data, as we try to do for
	     ELF.  */

	  if (!backtrace_get_view (state, descriptor, offset, size,
				   error_callback, data, &section_view))
	    return 0;
	  dwarf_sections->data[i] = (const unsigned char *) section_view.data;
	  dwarf_sections->size[i] = size;
	  break;
	}
    }
  return 1;
}

/* Collect DWARF sections from a DWARF segment.  Returns 1 on success,
   0 on failure.  */

static int
macho_add_dwarf_segment (struct backtrace_state *state, int descriptor,
			 off_t offset, unsigned int cmd, const char *psecs,
			 size_t sizesecs, unsigned int nsects,
			 backtrace_error_callback error_callback, void *data,
			 struct dwarf_sections *dwarf_sections)
{
  size_t sec_header_size;
  size_t secoffset;
  unsigned int i;

  switch (cmd)
    {
    case MACH_O_LC_SEGMENT:
      sec_header_size = sizeof (struct macho_section);
      break;
    case MACH_O_LC_SEGMENT_64:
      sec_header_size = sizeof (struct macho_section_64);
      break;
    default:
      abort ();
    }

  secoffset = 0;
  for (i = 0; i < nsects; ++i)
    {
      if (secoffset + sec_header_size > sizesecs)
	{
	  error_callback (data, "section overflow withing segment", 0);
	  return 0;
	}

      switch (cmd)
	{
	case MACH_O_LC_SEGMENT:
	  {
	    struct macho_section section;

	    memcpy (&section, psecs + secoffset, sizeof section);
	    macho_add_dwarf_section (state, descriptor, section.sectname,
				     offset + section.offset, section.size,
				     error_callback, data, dwarf_sections);
	  }
	  break;

	case MACH_O_LC_SEGMENT_64:
	  {
	    struct macho_section_64 section;

	    memcpy (&section, psecs + secoffset, sizeof section);
	    macho_add_dwarf_section (state, descriptor, section.sectname,
				     offset + section.offset, section.size,
				     error_callback, data, dwarf_sections);
	  }
	  break;

	default:
	  abort ();
	}

      secoffset += sec_header_size;
    }

  return 1;
}

/* Compare struct macho_symbol for qsort.  */

static int
macho_symbol_compare (const void *v1, const void *v2)
{
  const struct macho_symbol *m1 = (const struct macho_symbol *) v1;
  const struct macho_symbol *m2 = (const struct macho_symbol *) v2;

  if (m1->address < m2->address)
    return -1;
  else if (m1->address > m2->address)
    return 1;
  else
    return 0;
}

/* Compare an address against a macho_symbol for bsearch.  We allocate
   one extra entry in the array so that this can safely look at the
   next entry.  */

static int
macho_symbol_search (const void *vkey, const void *ventry)
{
  const uintptr_t *key = (const uintptr_t *) vkey;
  const struct macho_symbol *entry = (const struct macho_symbol *) ventry;
  uintptr_t addr;

  addr = *key;
  if (addr < entry->address)
    return -1;
  else if (entry->name[0] == '\0'
	   && entry->address == ~(uintptr_t) 0)
    return -1;
  else if ((entry + 1)->name[0] == '\0'
	   && (entry + 1)->address == ~(uintptr_t) 0)
    return -1;
  else if (addr >= (entry + 1)->address)
    return 1;
  else
    return 0;
}

/* Return whether the symbol type field indicates a symbol table entry
   that we care about: a function or data symbol.  */

static int
macho_defined_symbol (uint8_t type)
{
  if ((type & MACH_O_N_STAB) != 0)
    return 0;
  if ((type & MACH_O_N_EXT) != 0)
    return 0;
  switch (type & MACH_O_N_TYPE)
    {
    case MACH_O_N_ABS:
      return 1;
    case MACH_O_N_SECT:
      return 1;
    default:
      return 0;
    }
}

/* Add symbol table information for a Mach-O file.  */

static int
macho_add_symtab (struct backtrace_state *state, int descriptor,
		  uintptr_t base_address, int is_64,
		  off_t symoff, unsigned int nsyms, off_t stroff,
		  unsigned int strsize,
		  backtrace_error_callback error_callback, void *data)
{
  size_t symsize;
  struct backtrace_view sym_view;
  int sym_view_valid;
  struct backtrace_view str_view;
  int str_view_valid;
  size_t ndefs;
  size_t symtaboff;
  unsigned int i;
  size_t macho_symbol_size;
  struct macho_symbol *macho_symbols;
  unsigned int j;
  struct macho_syminfo_data *sdata;

  sym_view_valid = 0;
  str_view_valid = 0;
  macho_symbol_size = 0;
  macho_symbols = NULL;

  if (is_64)
    symsize = sizeof (struct macho_nlist_64);
  else
    symsize = sizeof (struct macho_nlist);

  if (!backtrace_get_view (state, descriptor, symoff, nsyms * symsize,
			   error_callback, data, &sym_view))
    goto fail;
  sym_view_valid = 1;

  if (!backtrace_get_view (state, descriptor, stroff, strsize,
			   error_callback, data, &str_view))
    return 0;
  str_view_valid = 1;

  ndefs = 0;
  symtaboff = 0;
  for (i = 0; i < nsyms; ++i, symtaboff += symsize)
    {
      if (is_64)
	{
	  struct macho_nlist_64 nlist;

	  memcpy (&nlist, (const char *) sym_view.data + symtaboff,
		  sizeof nlist);
	  if (macho_defined_symbol (nlist.n_type))
	    ++ndefs;
	}
      else
	{
	  struct macho_nlist nlist;

	  memcpy (&nlist, (const char *) sym_view.data + symtaboff,
		  sizeof nlist);
	  if (macho_defined_symbol (nlist.n_type))
	    ++ndefs;
	}
    }

  /* Add 1 to ndefs to make room for a sentinel.  */
  macho_symbol_size = (ndefs + 1) * sizeof (struct macho_symbol);
  macho_symbols = ((struct macho_symbol *)
		   backtrace_alloc (state, macho_symbol_size, error_callback,
				    data));
  if (macho_symbols == NULL)
    goto fail;

  j = 0;
  symtaboff = 0;
  for (i = 0; i < nsyms; ++i, symtaboff += symsize)
    {
      uint32_t strx;
      uint64_t value;
      const char *name;

      strx = 0;
      value = 0;
      if (is_64)
	{
	  struct macho_nlist_64 nlist;

	  memcpy (&nlist, (const char *) sym_view.data + symtaboff,
		  sizeof nlist);
	  if (!macho_defined_symbol (nlist.n_type))
	    continue;

	  strx = nlist.n_strx;
	  value = nlist.n_value;
	}
      else
	{
	  struct macho_nlist nlist;

	  memcpy (&nlist, (const char *) sym_view.data + symtaboff,
		  sizeof nlist);
	  if (!macho_defined_symbol (nlist.n_type))
	    continue;

	  strx = nlist.n_strx;
	  value = nlist.n_value;
	}

      if (strx >= strsize)
	{
	  error_callback (data, "symbol string index out of range", 0);
	  goto fail;
	}

      name = (const char *) str_view.data + strx;
      if (name[0] == '_')
	++name;
      macho_symbols[j].name = name;
      macho_symbols[j].address = value + base_address;
      ++j;
    }

  sdata = ((struct macho_syminfo_data *)
	   backtrace_alloc (state, sizeof *sdata, error_callback, data));
  if (sdata == NULL)
    goto fail;

  /* We need to keep the string table since it holds the names, but we
     can release the symbol table.  */

  backtrace_release_view (state, &sym_view, error_callback, data);
  sym_view_valid = 0;
  str_view_valid = 0;

  /* Add a trailing sentinel symbol.  */
  macho_symbols[j].name = "";
  macho_symbols[j].address = ~(uintptr_t) 0;

  backtrace_qsort (macho_symbols, ndefs + 1, sizeof (struct macho_symbol),
		   macho_symbol_compare);

  sdata->next = NULL;
  sdata->symbols = macho_symbols;
  sdata->count = ndefs;

  if (!state->threaded)
    {
      struct macho_syminfo_data **pp;

      for (pp = (struct macho_syminfo_data **) (void *) &state->syminfo_data;
	   *pp != NULL;
	   pp = &(*pp)->next)
	;
      *pp = sdata;
    }
  else
    {
      while (1)
	{
	  struct macho_syminfo_data **pp;

	  pp = (struct macho_syminfo_data **) (void *) &state->syminfo_data;

	  while (1)
	    {
	      struct macho_syminfo_data *p;

	      p = backtrace_atomic_load_pointer (pp);

	      if (p == NULL)
		break;

	      pp = &p->next;
	    }

	  if (__sync_bool_compare_and_swap (pp, NULL, sdata))
	    break;
	}
    }

  return 1;

 fail:
  if (macho_symbols != NULL)
    backtrace_free (state, macho_symbols, macho_symbol_size,
		    error_callback, data);
  if (sym_view_valid)
    backtrace_release_view (state, &sym_view, error_callback, data);
  if (str_view_valid)
    backtrace_release_view (state, &str_view, error_callback, data);
  return 0;
}

/* Return the symbol name and value for an ADDR.  */

static void
macho_syminfo (struct backtrace_state *state, uintptr_t addr,
	       backtrace_syminfo_callback callback,
	       backtrace_error_callback error_callback ATTRIBUTE_UNUSED,
	       void *data)
{
  struct macho_syminfo_data *sdata;
  struct macho_symbol *sym;

  sym = NULL;
  if (!state->threaded)
    {
      for (sdata = (struct macho_syminfo_data *) state->syminfo_data;
	   sdata != NULL;
	   sdata = sdata->next)
	{
	  sym = ((struct macho_symbol *)
		 bsearch (&addr, sdata->symbols, sdata->count,
			  sizeof (struct macho_symbol), macho_symbol_search));
	  if (sym != NULL)
	    break;
	}
    }
  else
    {
      struct macho_syminfo_data **pp;

      pp = (struct macho_syminfo_data **) (void *) &state->syminfo_data;
      while (1)
	{
	  sdata = backtrace_atomic_load_pointer (pp);
	  if (sdata == NULL)
	    break;

	  sym = ((struct macho_symbol *)
		 bsearch (&addr, sdata->symbols, sdata->count,
			  sizeof (struct macho_symbol), macho_symbol_search));
	  if (sym != NULL)
	    break;

	  pp = &sdata->next;
	}
    }

  if (sym == NULL)
    callback (data, addr, NULL, 0, 0);
  else
    callback (data, addr, sym->name, sym->address, 0);
}

/* Look through a fat file to find the relevant executable.  Returns 1
   on success, 0 on failure (in both cases descriptor is closed).  */

static int
macho_add_fat (struct backtrace_state *state, const char *filename,
	       int descriptor, int swapped, off_t offset,
	       const unsigned char *match_uuid, uintptr_t base_address,
	       int skip_symtab, uint32_t nfat_arch, int is_64,
	       backtrace_error_callback error_callback, void *data,
	       fileline *fileline_fn, int *found_sym)
{
  int arch_view_valid;
  unsigned int cputype;
  size_t arch_size;
  struct backtrace_view arch_view;
  unsigned int i;

  arch_view_valid = 0;

#if defined (__x86_64__)
  cputype = MACH_O_CPU_TYPE_X86_64;
#elif defined (__i386__)
  cputype = MACH_O_CPU_TYPE_X86;
#elif defined (__aarch64__)
  cputype = MACH_O_CPU_TYPE_ARM64;
#elif defined (__arm__)
  cputype = MACH_O_CPU_TYPE_ARM;
#elif defined (__ppc__)
  cputype = MACH_O_CPU_TYPE_PPC;
#elif defined (__ppc64__)
  cputype = MACH_O_CPU_TYPE_PPC64;
#else
  error_callback (data, "unknown Mach-O architecture", 0);
  goto fail;
#endif

  if (is_64)
    arch_size = sizeof (struct macho_fat_arch_64);
  else
    arch_size = sizeof (struct macho_fat_arch);

  if (!backtrace_get_view (state, descriptor, offset,
			   nfat_arch * arch_size,
			   error_callback, data, &arch_view))
    goto fail;

  for (i = 0; i < nfat_arch; ++i)
    {
      uint32_t fcputype;
      uint64_t foffset;

      if (is_64)
	{
	  struct macho_fat_arch_64 fat_arch_64;

	  memcpy (&fat_arch_64,
		  (const char *) arch_view.data + i * arch_size,
		  arch_size);
	  fcputype = fat_arch_64.cputype;
	  foffset = fat_arch_64.offset;
	  if (swapped)
	    {
	      fcputype = __builtin_bswap32 (fcputype);
	      foffset = __builtin_bswap64 (foffset);
	    }
	}
      else
	{
	  struct macho_fat_arch fat_arch_32;

	  memcpy (&fat_arch_32,
		  (const char *) arch_view.data + i * arch_size,
		  arch_size);
	  fcputype = fat_arch_32.cputype;
	  foffset = (uint64_t) fat_arch_32.offset;
	  if (swapped)
	    {
	      fcputype = __builtin_bswap32 (fcputype);
	      foffset = (uint64_t) __builtin_bswap32 ((uint32_t) foffset);
	    }
	}

      if (fcputype == cputype)
	{
	  /* FIXME: What about cpusubtype?  */
	  backtrace_release_view (state, &arch_view, error_callback, data);
	  return macho_add (state, filename, descriptor, foffset, match_uuid,
			    base_address, skip_symtab, error_callback, data,
			    fileline_fn, found_sym);
	}
    }

  error_callback (data, "could not find executable in fat file", 0);

 fail:
  if (arch_view_valid)
    backtrace_release_view (state, &arch_view, error_callback, data);
  if (descriptor != -1)
    backtrace_close (descriptor, error_callback, data);
  return 0;
}

/* Look for the dsym file for FILENAME.  This is called if FILENAME
   does not have debug info or a symbol table.  Returns 1 on success,
   0 on failure.  */

static int
macho_add_dsym (struct backtrace_state *state, const char *filename,
		uintptr_t base_address, const unsigned char *uuid,
		backtrace_error_callback error_callback, void *data,
		fileline* fileline_fn)
{
  const char *p;
  const char *dirname;
  char *diralc;
  size_t dirnamelen;
  const char *basename;
  size_t basenamelen;
  const char *dsymsuffixdir;
  size_t dsymsuffixdirlen;
  size_t dsymlen;
  char *dsym;
  char *ps;
  int d;
  int does_not_exist;
  int dummy_found_sym;

  diralc = NULL;
  dirnamelen = 0;
  dsym = NULL;
  dsymlen = 0;

  p = strrchr (filename, '/');
  if (p == NULL)
    {
      dirname = ".";
      dirnamelen = 1;
      basename = filename;
      basenamelen = strlen (basename);
      diralc = NULL;
    }
  else
    {
      dirnamelen = p - filename;
      diralc = (char*)backtrace_alloc (state, dirnamelen + 1, error_callback, data);
      if (diralc == NULL)
	goto fail;
      memcpy (diralc, filename, dirnamelen);
      diralc[dirnamelen] = '\0';
      dirname = diralc;
      basename = p + 1;
      basenamelen = strlen (basename);
    }

  dsymsuffixdir = ".dSYM/Contents/Resources/DWARF/";
  dsymsuffixdirlen = strlen (dsymsuffixdir);

  dsymlen = (dirnamelen
	     + 1
	     + basenamelen
	     + dsymsuffixdirlen
	     + basenamelen
	     + 1);
  dsym = (char*)backtrace_alloc (state, dsymlen, error_callback, data);
  if (dsym == NULL)
    goto fail;

  ps = dsym;
  memcpy (ps, dirname, dirnamelen);
  ps += dirnamelen;
  *ps++ = '/';
  memcpy (ps, basename, basenamelen);
  ps += basenamelen;
  memcpy (ps, dsymsuffixdir, dsymsuffixdirlen);
  ps += dsymsuffixdirlen;
  memcpy (ps, basename, basenamelen);
  ps += basenamelen;
  *ps = '\0';

  if (diralc != NULL)
    {
      backtrace_free (state, diralc, dirnamelen + 1, error_callback, data);
      diralc = NULL;
    }

  d = backtrace_open (dsym, error_callback, data, &does_not_exist);
  if (d < 0)
    {
      /* The file does not exist, so we can't read the debug info.
	 Just return success.  */
      backtrace_free (state, dsym, dsymlen, error_callback, data);
      return 1;
    }

  if (!macho_add (state, dsym, d, 0, uuid, base_address, 1,
		  error_callback, data, fileline_fn, &dummy_found_sym))
    goto fail;

  backtrace_free (state, dsym, dsymlen, error_callback, data);

  return 1;

 fail:
  if (dsym != NULL)
    backtrace_free (state, dsym, dsymlen, error_callback, data);
  if (diralc != NULL)
    backtrace_free (state, diralc, dirnamelen, error_callback, data);
  return 0;
}

/* Add the backtrace data for a Macho-O file.  Returns 1 on success, 0
   on failure (in both cases descriptor is closed).

   FILENAME: the name of the executable.
   DESCRIPTOR: an open descriptor for the executable, closed here.
   OFFSET: the offset within the file of this executable, for fat files.
   MATCH_UUID: if not NULL, UUID that must match.
   BASE_ADDRESS: the load address of the executable.
   SKIP_SYMTAB: if non-zero, ignore the symbol table; used for dSYM files.
   FILELINE_FN: set to the fileline function, by backtrace_dwarf_add.
   FOUND_SYM: set to non-zero if we found the symbol table.
*/

static int
macho_add (struct backtrace_state *state, const char *filename, int descriptor,
	   off_t offset, const unsigned char *match_uuid,
	   uintptr_t base_address, int skip_symtab,
	   backtrace_error_callback error_callback, void *data,
	   fileline *fileline_fn, int *found_sym)
{
  struct backtrace_view header_view;
  struct macho_header_32 header;
  off_t hdroffset;
  int is_64;
  struct backtrace_view cmds_view;
  int cmds_view_valid;
  struct dwarf_sections dwarf_sections;
  int have_dwarf;
  unsigned char uuid[MACH_O_UUID_LEN];
  int have_uuid;
  size_t cmdoffset;
  unsigned int i;

  *found_sym = 0;

  cmds_view_valid = 0;

  /* The 32-bit and 64-bit file headers start out the same, so we can
     just always read the 32-bit version.  A fat header is shorter but
     it will always be followed by data, so it's OK to read extra.  */

  if (!backtrace_get_view (state, descriptor, offset,
			   sizeof (struct macho_header_32),
			   error_callback, data, &header_view))
    goto fail;

  memcpy (&header, header_view.data, sizeof header);

  backtrace_release_view (state, &header_view, error_callback, data);

  switch (header.magic)
    {
    case MACH_O_MH_MAGIC_32:
      is_64 = 0;
      hdroffset = offset + sizeof (struct macho_header_32);
      break;
    case MACH_O_MH_MAGIC_64:
      is_64 = 1;
      hdroffset = offset + sizeof (struct macho_header_64);
      break;
    case MACH_O_MH_MAGIC_FAT:
    case MACH_O_MH_MAGIC_FAT_64:
      {
	struct macho_header_fat fat_header;

	hdroffset = offset + sizeof (struct macho_header_fat);
	memcpy (&fat_header, &header, sizeof fat_header);
	return macho_add_fat (state, filename, descriptor, 0, hdroffset,
			      match_uuid, base_address, skip_symtab,
			      fat_header.nfat_arch,
			      header.magic == MACH_O_MH_MAGIC_FAT_64,
			      error_callback, data, fileline_fn, found_sym);
      }
    case MACH_O_MH_CIGAM_FAT:
    case MACH_O_MH_CIGAM_FAT_64:
      {
	struct macho_header_fat fat_header;
	uint32_t nfat_arch;

	hdroffset = offset + sizeof (struct macho_header_fat);
	memcpy (&fat_header, &header, sizeof fat_header);
	nfat_arch = __builtin_bswap32 (fat_header.nfat_arch);
	return macho_add_fat (state, filename, descriptor, 1, hdroffset,
			      match_uuid, base_address, skip_symtab,
			      nfat_arch,
			      header.magic == MACH_O_MH_CIGAM_FAT_64,
			      error_callback, data, fileline_fn, found_sym);
      }
    default:
      error_callback (data, "executable file is not in Mach-O format", 0);
      goto fail;
    }

  switch (header.filetype)
    {
    case MACH_O_MH_EXECUTE:
    case MACH_O_MH_DYLIB:
    case MACH_O_MH_DSYM:
      break;
    default:
      error_callback (data, "executable file is not an executable", 0);
      goto fail;
    }

  if (!backtrace_get_view (state, descriptor, hdroffset, header.sizeofcmds,
			   error_callback, data, &cmds_view))
    goto fail;
  cmds_view_valid = 1;

  memset (&dwarf_sections, 0, sizeof dwarf_sections);
  have_dwarf = 0;
  memset (&uuid, 0, sizeof uuid);
  have_uuid = 0;

  cmdoffset = 0;
  for (i = 0; i < header.ncmds; ++i)
    {
      const char *pcmd;
      struct macho_load_command load_command;

      if (cmdoffset + sizeof load_command > header.sizeofcmds)
	break;

      pcmd = (const char *) cmds_view.data + cmdoffset;
      memcpy (&load_command, pcmd, sizeof load_command);

      switch (load_command.cmd)
	{
	case MACH_O_LC_SEGMENT:
	  {
	    struct macho_segment_command segcmd;

	    memcpy (&segcmd, pcmd, sizeof segcmd);
	    if (memcmp (segcmd.segname,
			"__DWARF\0\0\0\0\0\0\0\0\0",
			MACH_O_NAMELEN) == 0)
	      {
		if (!macho_add_dwarf_segment (state, descriptor, offset,
					      load_command.cmd,
					      pcmd + sizeof segcmd,
					      (load_command.cmdsize
					       - sizeof segcmd),
					      segcmd.nsects, error_callback,
					      data, &dwarf_sections))
		  goto fail;
		have_dwarf = 1;
	      }
	  }
	  break;

	case MACH_O_LC_SEGMENT_64:
	  {
	    struct macho_segment_64_command segcmd;

	    memcpy (&segcmd, pcmd, sizeof segcmd);
	    if (memcmp (segcmd.segname,
			"__DWARF\0\0\0\0\0\0\0\0\0",
			MACH_O_NAMELEN) == 0)
	      {
		if (!macho_add_dwarf_segment (state, descriptor, offset,
					      load_command.cmd,
					      pcmd + sizeof segcmd,
					      (load_command.cmdsize
					       - sizeof segcmd),
					      segcmd.nsects, error_callback,
					      data, &dwarf_sections))
		  goto fail;
		have_dwarf = 1;
	      }
	  }
	  break;

	case MACH_O_LC_SYMTAB:
	  if (!skip_symtab)
	    {
	      struct macho_symtab_command symcmd;

	      memcpy (&symcmd, pcmd, sizeof symcmd);
	      if (!macho_add_symtab (state, descriptor, base_address, is_64,
				     offset + symcmd.symoff, symcmd.nsyms,
				     offset + symcmd.stroff, symcmd.strsize,
				     error_callback, data))
		goto fail;

	      *found_sym = 1;
	    }
	  break;

	case MACH_O_LC_UUID:
	  {
	    struct macho_uuid_command uuidcmd;

	    memcpy (&uuidcmd, pcmd, sizeof uuidcmd);
	    memcpy (&uuid[0], &uuidcmd.uuid[0], MACH_O_UUID_LEN);
	    have_uuid = 1;
	  }
	  break;

	default:
	  break;
	}

      cmdoffset += load_command.cmdsize;
    }

  if (!backtrace_close (descriptor, error_callback, data))
    goto fail;
  descriptor = -1;

  backtrace_release_view (state, &cmds_view, error_callback, data);
  cmds_view_valid = 0;

  if (match_uuid != NULL)
    {
      /* If we don't have a UUID, or it doesn't match, just ignore
	 this file.  */
      if (!have_uuid
	  || memcmp (match_uuid, &uuid[0], MACH_O_UUID_LEN) != 0)
	return 1;
    }

  if (have_dwarf)
    {
      int is_big_endian;

      is_big_endian = 0;
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      is_big_endian = 1;
#endif
#endif

      if (!backtrace_dwarf_add (state, base_address, &dwarf_sections,
				is_big_endian, NULL, error_callback, data,
				fileline_fn, NULL))
	goto fail;
    }

  if (!have_dwarf && have_uuid)
    {
      if (!macho_add_dsym (state, filename, base_address, &uuid[0],
			   error_callback, data, fileline_fn))
	goto fail;
    }

  return 1;

 fail:
  if (cmds_view_valid)
    backtrace_release_view (state, &cmds_view, error_callback, data);
  if (descriptor != -1)
    backtrace_close (descriptor, error_callback, data);
  return 0;
}

#ifdef HAVE_MACH_O_DYLD_H

/* Initialize the backtrace data we need from a Mach-O executable
   using the dyld support functions.  This closes descriptor.  */

int
backtrace_initialize (struct backtrace_state *state, const char *filename,
		      int descriptor, backtrace_error_callback error_callback,
		      void *data, fileline *fileline_fn)
{
  uint32_t c;
  uint32_t i;
  int closed_descriptor;
  int found_sym;
  fileline macho_fileline_fn;

  closed_descriptor = 0;
  found_sym = 0;
  macho_fileline_fn = macho_nodebug;

  c = _dyld_image_count ();
  for (i = 0; i < c; ++i)
    {
      uintptr_t base_address;
      const char *name;
      int d;
      fileline mff;
      int mfs;

      name = _dyld_get_image_name (i);
      if (name == NULL)
	continue;

      if (strcmp (name, filename) == 0 && !closed_descriptor)
	{
	  d = descriptor;
	  closed_descriptor = 1;
	}
      else
	{
	  int does_not_exist;

	  d = backtrace_open (name, error_callback, data, &does_not_exist);
	  if (d < 0)
	    continue;
	}

      base_address = _dyld_get_image_vmaddr_slide (i);

      mff = macho_nodebug;
      if (!macho_add (state, name, d, 0, NULL, base_address, 0,
		      error_callback, data, &mff, &mfs))
	return 0;

      if (mff != macho_nodebug)
	macho_fileline_fn = mff;
      if (mfs)
	found_sym = 1;
    }

  if (!closed_descriptor)
    backtrace_close (descriptor, error_callback, data);

  if (!state->threaded)
    {
      if (found_sym)
	state->syminfo_fn = macho_syminfo;
      else if (state->syminfo_fn == NULL)
	state->syminfo_fn = macho_nosyms;
    }
  else
    {
      if (found_sym)
	backtrace_atomic_store_pointer (&state->syminfo_fn, macho_syminfo);
      else
	(void) __sync_bool_compare_and_swap (&state->syminfo_fn, NULL,
					     macho_nosyms);
    }

  if (!state->threaded)
    *fileline_fn = state->fileline_fn;
  else
    *fileline_fn = backtrace_atomic_load_pointer (&state->fileline_fn);

  if (*fileline_fn == NULL || *fileline_fn == macho_nodebug)
    *fileline_fn = macho_fileline_fn;

  return 1;
}

#else /* !defined (HAVE_MACH_O_DYLD_H) */

/* Initialize the backtrace data we need from a Mach-O executable
   without using the dyld support functions.  This closes
   descriptor.  */

int
backtrace_initialize (struct backtrace_state *state, const char *filename,
		      int descriptor, backtrace_error_callback error_callback,
		      void *data, fileline *fileline_fn)
{
  fileline macho_fileline_fn;
  int found_sym;

  macho_fileline_fn = macho_nodebug;
  if (!macho_add (state, filename, descriptor, 0, NULL, 0, 0,
		  error_callback, data, &macho_fileline_fn, &found_sym))
    return 0;

  if (!state->threaded)
    {
      if (found_sym)
	state->syminfo_fn = macho_syminfo;
      else if (state->syminfo_fn == NULL)
	state->syminfo_fn = macho_nosyms;
    }
  else
    {
      if (found_sym)
	backtrace_atomic_store_pointer (&state->syminfo_fn, macho_syminfo);
      else
	(void) __sync_bool_compare_and_swap (&state->syminfo_fn, NULL,
					     macho_nosyms);
    }

  if (!state->threaded)
    *fileline_fn = state->fileline_fn;
  else
    *fileline_fn = backtrace_atomic_load_pointer (&state->fileline_fn);

  if (*fileline_fn == NULL || *fileline_fn == macho_nodebug)
    *fileline_fn = macho_fileline_fn;

  return 1;
}

#endif /* !defined (HAVE_MACH_O_DYLD_H) */

}

/*** End of inlined file: macho.cpp ***/


#  else

/*** Start of inlined file: elf.cpp ***/

/*** Start of inlined file: config.h ***/
#include <climits>
#if __WORDSIZE == 64
#  define BACKTRACE_ELF_SIZE 64
#else
#  define BACKTRACE_ELF_SIZE 32
#endif

#define HAVE_DLFCN_H 1
#define HAVE_FCNTL 1
#define HAVE_INTTYPES_H 1
#define HAVE_LSTAT 1
#define HAVE_READLINK 1
#define HAVE_DL_ITERATE_PHDR 1
#define HAVE_ATOMIC_FUNCTIONS 1
#define HAVE_DECL_STRNLEN 1

#ifdef __APPLE__
#  define HAVE_MACH_O_DYLD_H 1
#elif defined BSD
#  define HAVE_KERN_PROC 1
#  define HAVE_KERN_PROC_ARGS 1
#endif

/*** End of inlined file: config.h ***/

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_DL_ITERATE_PHDR
#include <link.h>
#endif

#ifndef S_ISLNK
 #ifndef S_IFLNK
  #define S_IFLNK 0120000
 #endif
 #ifndef S_IFMT
  #define S_IFMT 0170000
 #endif
 #define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif

#ifndef __GNUC__
#define __builtin_prefetch(p, r, l)
#ifndef unlikely
#define unlikely(x) (x)
#endif
#else
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif
#endif

namespace tracy
{

#if !defined(HAVE_DECL_STRNLEN) || !HAVE_DECL_STRNLEN

/* If strnlen is not declared, provide our own version.  */

static size_t
xstrnlen (const char *s, size_t maxlen)
{
  size_t i;

  for (i = 0; i < maxlen; ++i)
    if (s[i] == '\0')
      break;
  return i;
}

#define strnlen xstrnlen

#endif

#ifndef HAVE_LSTAT

/* Dummy version of lstat for systems that don't have it.  */

static int
xlstat (const char *path ATTRIBUTE_UNUSED, struct stat *st ATTRIBUTE_UNUSED)
{
  return -1;
}

#define lstat xlstat

#endif

#ifndef HAVE_READLINK

/* Dummy version of readlink for systems that don't have it.  */

static ssize_t
xreadlink (const char *path ATTRIBUTE_UNUSED, char *buf ATTRIBUTE_UNUSED,
	   size_t bufsz ATTRIBUTE_UNUSED)
{
  return -1;
}

#define readlink xreadlink

#endif

#ifndef HAVE_DL_ITERATE_PHDR

/* Dummy version of dl_iterate_phdr for systems that don't have it.  */

#define dl_phdr_info x_dl_phdr_info
#define dl_iterate_phdr x_dl_iterate_phdr

struct dl_phdr_info
{
  uintptr_t dlpi_addr;
  const char *dlpi_name;
};

static int
dl_iterate_phdr (int (*callback) (struct dl_phdr_info *,
				  size_t, void *) ATTRIBUTE_UNUSED,
		 void *data ATTRIBUTE_UNUSED)
{
  return 0;
}

#endif /* ! defined (HAVE_DL_ITERATE_PHDR) */

/* The configure script must tell us whether we are 32-bit or 64-bit
   ELF.  We could make this code test and support either possibility,
   but there is no point.  This code only works for the currently
   running executable, which means that we know the ELF mode at
   configure time.  */

#if BACKTRACE_ELF_SIZE != 32 && BACKTRACE_ELF_SIZE != 64
#error "Unknown BACKTRACE_ELF_SIZE"
#endif

/* <link.h> might #include <elf.h> which might define our constants
   with slightly different values.  Undefine them to be safe.  */

#undef EI_NIDENT
#undef EI_MAG0
#undef EI_MAG1
#undef EI_MAG2
#undef EI_MAG3
#undef EI_CLASS
#undef EI_DATA
#undef EI_VERSION
#undef ELF_MAG0
#undef ELF_MAG1
#undef ELF_MAG2
#undef ELF_MAG3
#undef ELFCLASS32
#undef ELFCLASS64
#undef ELFDATA2LSB
#undef ELFDATA2MSB
#undef EV_CURRENT
#undef ET_DYN
#undef EM_PPC64
#undef EF_PPC64_ABI
#undef SHN_LORESERVE
#undef SHN_XINDEX
#undef SHN_UNDEF
#undef SHT_PROGBITS
#undef SHT_SYMTAB
#undef SHT_STRTAB
#undef SHT_DYNSYM
#undef SHF_COMPRESSED
#undef STT_OBJECT
#undef STT_FUNC
#undef NT_GNU_BUILD_ID
#undef ELFCOMPRESS_ZLIB

/* Basic types.  */

typedef uint16_t b_elf_half;    /* Elf_Half.  */
typedef uint32_t b_elf_word;    /* Elf_Word.  */
typedef int32_t  b_elf_sword;   /* Elf_Sword.  */

#if BACKTRACE_ELF_SIZE == 32

typedef uint32_t b_elf_addr;    /* Elf_Addr.  */
typedef uint32_t b_elf_off;     /* Elf_Off.  */

typedef uint32_t b_elf_wxword;  /* 32-bit Elf_Word, 64-bit ELF_Xword.  */

#else

typedef uint64_t b_elf_addr;    /* Elf_Addr.  */
typedef uint64_t b_elf_off;     /* Elf_Off.  */
typedef uint64_t b_elf_xword;   /* Elf_Xword.  */
typedef int64_t  b_elf_sxword;  /* Elf_Sxword.  */

typedef uint64_t b_elf_wxword;  /* 32-bit Elf_Word, 64-bit ELF_Xword.  */

#endif

/* Data structures and associated constants.  */

#define EI_NIDENT 16

typedef struct {
  unsigned char	e_ident[EI_NIDENT];	/* ELF "magic number" */
  b_elf_half	e_type;			/* Identifies object file type */
  b_elf_half	e_machine;		/* Specifies required architecture */
  b_elf_word	e_version;		/* Identifies object file version */
  b_elf_addr	e_entry;		/* Entry point virtual address */
  b_elf_off	e_phoff;		/* Program header table file offset */
  b_elf_off	e_shoff;		/* Section header table file offset */
  b_elf_word	e_flags;		/* Processor-specific flags */
  b_elf_half	e_ehsize;		/* ELF header size in bytes */
  b_elf_half	e_phentsize;		/* Program header table entry size */
  b_elf_half	e_phnum;		/* Program header table entry count */
  b_elf_half	e_shentsize;		/* Section header table entry size */
  b_elf_half	e_shnum;		/* Section header table entry count */
  b_elf_half	e_shstrndx;		/* Section header string table index */
} b_elf_ehdr;  /* Elf_Ehdr.  */

#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6

#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFCLASS32 1
#define ELFCLASS64 2

#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

#define EV_CURRENT 1

#define ET_DYN 3

#define EM_PPC64 21
#define EF_PPC64_ABI 3

typedef struct {
  b_elf_word	sh_name;		/* Section name, index in string tbl */
  b_elf_word	sh_type;		/* Type of section */
  b_elf_wxword	sh_flags;		/* Miscellaneous section attributes */
  b_elf_addr	sh_addr;		/* Section virtual addr at execution */
  b_elf_off	sh_offset;		/* Section file offset */
  b_elf_wxword	sh_size;		/* Size of section in bytes */
  b_elf_word	sh_link;		/* Index of another section */
  b_elf_word	sh_info;		/* Additional section information */
  b_elf_wxword	sh_addralign;		/* Section alignment */
  b_elf_wxword	sh_entsize;		/* Entry size if section holds table */
} b_elf_shdr;  /* Elf_Shdr.  */

#define SHN_UNDEF	0x0000		/* Undefined section */
#define SHN_LORESERVE	0xFF00		/* Begin range of reserved indices */
#define SHN_XINDEX	0xFFFF		/* Section index is held elsewhere */

#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_DYNSYM 11

#define SHF_COMPRESSED 0x800

#if BACKTRACE_ELF_SIZE == 32

typedef struct
{
  b_elf_word	st_name;		/* Symbol name, index in string tbl */
  b_elf_addr	st_value;		/* Symbol value */
  b_elf_word	st_size;		/* Symbol size */
  unsigned char	st_info;		/* Symbol binding and type */
  unsigned char	st_other;		/* Visibility and other data */
  b_elf_half	st_shndx;		/* Symbol section index */
} b_elf_sym;  /* Elf_Sym.  */

#else /* BACKTRACE_ELF_SIZE != 32 */

typedef struct
{
  b_elf_word	st_name;		/* Symbol name, index in string tbl */
  unsigned char	st_info;		/* Symbol binding and type */
  unsigned char	st_other;		/* Visibility and other data */
  b_elf_half	st_shndx;		/* Symbol section index */
  b_elf_addr	st_value;		/* Symbol value */
  b_elf_xword	st_size;		/* Symbol size */
} b_elf_sym;  /* Elf_Sym.  */

#endif /* BACKTRACE_ELF_SIZE != 32 */

#define STT_OBJECT 1
#define STT_FUNC 2

typedef struct
{
  uint32_t namesz;
  uint32_t descsz;
  uint32_t type;
  char name[1];
} b_elf_note;

#define NT_GNU_BUILD_ID 3

#if BACKTRACE_ELF_SIZE == 32

typedef struct
{
  b_elf_word	ch_type;		/* Compresstion algorithm */
  b_elf_word	ch_size;		/* Uncompressed size */
  b_elf_word	ch_addralign;		/* Alignment for uncompressed data */
} b_elf_chdr;  /* Elf_Chdr */

#else /* BACKTRACE_ELF_SIZE != 32 */

typedef struct
{
  b_elf_word	ch_type;		/* Compression algorithm */
  b_elf_word	ch_reserved;		/* Reserved */
  b_elf_xword	ch_size;		/* Uncompressed size */
  b_elf_xword	ch_addralign;		/* Alignment for uncompressed data */
} b_elf_chdr;  /* Elf_Chdr */

#endif /* BACKTRACE_ELF_SIZE != 32 */

#define ELFCOMPRESS_ZLIB 1

/* Names of sections, indexed by enum dwarf_section in internal.h.  */

static const char * const dwarf_section_names[DEBUG_MAX] =
{
  ".debug_info",
  ".debug_line",
  ".debug_abbrev",
  ".debug_ranges",
  ".debug_str",
  ".debug_addr",
  ".debug_str_offsets",
  ".debug_line_str",
  ".debug_rnglists"
};

/* Information we gather for the sections we care about.  */

struct debug_section_info
{
  /* Section file offset.  */
  off_t offset;
  /* Section size.  */
  size_t size;
  /* Section contents, after read from file.  */
  const unsigned char *data;
  /* Whether the SHF_COMPRESSED flag is set for the section.  */
  int compressed;
};

/* Information we keep for an ELF symbol.  */

struct elf_symbol
{
  /* The name of the symbol.  */
  const char *name;
  /* The address of the symbol.  */
  uintptr_t address;
  /* The size of the symbol.  */
  size_t size;
};

/* Information to pass to elf_syminfo.  */

struct elf_syminfo_data
{
  /* Symbols for the next module.  */
  struct elf_syminfo_data *next;
  /* The ELF symbols, sorted by address.  */
  struct elf_symbol *symbols;
  /* The number of symbols.  */
  size_t count;
};

/* A view that works for either a file or memory.  */

struct elf_view
{
  struct backtrace_view view;
  int release; /* If non-zero, must call backtrace_release_view.  */
};

/* Information about PowerPC64 ELFv1 .opd section.  */

struct elf_ppc64_opd_data
{
  /* Address of the .opd section.  */
  b_elf_addr addr;
  /* Section data.  */
  const char *data;
  /* Size of the .opd section.  */
  size_t size;
  /* Corresponding section view.  */
  struct elf_view view;
};

/* Create a view of SIZE bytes from DESCRIPTOR/MEMORY at OFFSET.  */

static int
elf_get_view (struct backtrace_state *state, int descriptor,
	      const unsigned char *memory, size_t memory_size, off_t offset,
	      uint64_t size, backtrace_error_callback error_callback,
	      void *data, struct elf_view *view)
{
  if (memory == NULL)
    {
      view->release = 1;
      return backtrace_get_view (state, descriptor, offset, size,
				 error_callback, data, &view->view);
    }
  else
    {
      if ((uint64_t) offset + size > (uint64_t) memory_size)
	{
	  error_callback (data, "out of range for in-memory file", 0);
	  return 0;
	}
      view->view.data = (const void *) (memory + offset);
      view->view.base = NULL;
      view->view.len = size;
      view->release = 0;
      return 1;
    }
}

/* Release a view read by elf_get_view.  */

static void
elf_release_view (struct backtrace_state *state, struct elf_view *view,
		  backtrace_error_callback error_callback, void *data)
{
  if (view->release)
    backtrace_release_view (state, &view->view, error_callback, data);
}

/* Compute the CRC-32 of BUF/LEN.  This uses the CRC used for
   .gnu_debuglink files.  */

static uint32_t
elf_crc32 (uint32_t crc, const unsigned char *buf, size_t len)
{
  static const uint32_t crc32_table[256] =
    {
      0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
      0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
      0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
      0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
      0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
      0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
      0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
      0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
      0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
      0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
      0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
      0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
      0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
      0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
      0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
      0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
      0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
      0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
      0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
      0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
      0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
      0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
      0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
      0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
      0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
      0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
      0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
      0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
      0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
      0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
      0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
      0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
      0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
      0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
      0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
      0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
      0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
      0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
      0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
      0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
      0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
      0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
      0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
      0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
      0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
      0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
      0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
      0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
      0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
      0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
      0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
      0x2d02ef8d
    };
  const unsigned char *end;

  crc = ~crc;
  for (end = buf + len; buf < end; ++ buf)
    crc = crc32_table[(crc ^ *buf) & 0xff] ^ (crc >> 8);
  return ~crc;
}

/* Return the CRC-32 of the entire file open at DESCRIPTOR.  */

static uint32_t
elf_crc32_file (struct backtrace_state *state, int descriptor,
		backtrace_error_callback error_callback, void *data)
{
  struct stat st;
  struct backtrace_view file_view;
  uint32_t ret;

  if (fstat (descriptor, &st) < 0)
    {
      error_callback (data, "fstat", errno);
      return 0;
    }

  if (!backtrace_get_view (state, descriptor, 0, st.st_size, error_callback,
			   data, &file_view))
    return 0;

  ret = elf_crc32 (0, (const unsigned char *) file_view.data, st.st_size);

  backtrace_release_view (state, &file_view, error_callback, data);

  return ret;
}

/* A dummy callback function used when we can't find a symbol
   table.  */

static void
elf_nosyms (struct backtrace_state *state ATTRIBUTE_UNUSED,
	    uintptr_t addr ATTRIBUTE_UNUSED,
	    backtrace_syminfo_callback callback ATTRIBUTE_UNUSED,
	    backtrace_error_callback error_callback, void *data)
{
  error_callback (data, "no symbol table in ELF executable", -1);
}

/* A callback function used when we can't find any debug info.  */

static int
elf_nodebug (struct backtrace_state *state, uintptr_t pc,
	     backtrace_full_callback callback,
	     backtrace_error_callback error_callback, void *data)
{
  if (state->syminfo_fn != NULL && state->syminfo_fn != elf_nosyms)
    {
      struct backtrace_call_full bdata;

      /* Fetch symbol information so that we can least get the
	 function name.  */

      bdata.full_callback = callback;
      bdata.full_error_callback = error_callback;
      bdata.full_data = data;
      bdata.ret = 0;
      state->syminfo_fn (state, pc, backtrace_syminfo_to_full_callback,
			 backtrace_syminfo_to_full_error_callback, &bdata);
      return bdata.ret;
    }

  error_callback (data, "no debug info in ELF executable", -1);
  return 0;
}

/* Compare struct elf_symbol for qsort.  */

static int
elf_symbol_compare (const void *v1, const void *v2)
{
  const struct elf_symbol *e1 = (const struct elf_symbol *) v1;
  const struct elf_symbol *e2 = (const struct elf_symbol *) v2;

  if (e1->address < e2->address)
    return -1;
  else if (e1->address > e2->address)
    return 1;
  else
    return 0;
}

/* Compare an ADDR against an elf_symbol for bsearch.  We allocate one
   extra entry in the array so that this can look safely at the next
   entry.  */

static int
elf_symbol_search (const void *vkey, const void *ventry)
{
  const uintptr_t *key = (const uintptr_t *) vkey;
  const struct elf_symbol *entry = (const struct elf_symbol *) ventry;
  uintptr_t addr;

  addr = *key;
  if (addr < entry->address)
    return -1;
  else if (addr >= entry->address + entry->size)
    return 1;
  else
    return 0;
}

/* Initialize the symbol table info for elf_syminfo.  */

static int
elf_initialize_syminfo (struct backtrace_state *state,
			uintptr_t base_address,
			const unsigned char *symtab_data, size_t symtab_size,
			const unsigned char *strtab, size_t strtab_size,
			backtrace_error_callback error_callback,
			void *data, struct elf_syminfo_data *sdata,
			struct elf_ppc64_opd_data *opd)
{
  size_t sym_count;
  const b_elf_sym *sym;
  size_t elf_symbol_count;
  size_t elf_symbol_size;
  struct elf_symbol *elf_symbols;
  size_t i;
  unsigned int j;

  sym_count = symtab_size / sizeof (b_elf_sym);

  /* We only care about function symbols.  Count them.  */
  sym = (const b_elf_sym *) symtab_data;
  elf_symbol_count = 0;
  for (i = 0; i < sym_count; ++i, ++sym)
    {
      int info;

      info = sym->st_info & 0xf;
      if ((info == STT_FUNC || info == STT_OBJECT)
	  && sym->st_shndx != SHN_UNDEF)
	++elf_symbol_count;
    }

  elf_symbol_size = elf_symbol_count * sizeof (struct elf_symbol);
  elf_symbols = ((struct elf_symbol *)
		 backtrace_alloc (state, elf_symbol_size, error_callback,
				  data));
  if (elf_symbols == NULL)
    return 0;

  sym = (const b_elf_sym *) symtab_data;
  j = 0;
  for (i = 0; i < sym_count; ++i, ++sym)
    {
      int info;

      info = sym->st_info & 0xf;
      if (info != STT_FUNC && info != STT_OBJECT)
	continue;
      if (sym->st_shndx == SHN_UNDEF)
	continue;
      if (sym->st_name >= strtab_size)
	{
	  error_callback (data, "symbol string index out of range", 0);
	  backtrace_free (state, elf_symbols, elf_symbol_size, error_callback,
			  data);
	  return 0;
	}
      elf_symbols[j].name = (const char *) strtab + sym->st_name;
      /* Special case PowerPC64 ELFv1 symbols in .opd section, if the symbol
	 is a function descriptor, read the actual code address from the
	 descriptor.  */
      if (opd
	  && sym->st_value >= opd->addr
	  && sym->st_value < opd->addr + opd->size)
	elf_symbols[j].address
	  = *(const b_elf_addr *) (opd->data + (sym->st_value - opd->addr));
      else
	elf_symbols[j].address = sym->st_value;
      elf_symbols[j].address += base_address;
      elf_symbols[j].size = sym->st_size;
      ++j;
    }

  backtrace_qsort (elf_symbols, elf_symbol_count, sizeof (struct elf_symbol),
		   elf_symbol_compare);

  sdata->next = NULL;
  sdata->symbols = elf_symbols;
  sdata->count = elf_symbol_count;

  return 1;
}

/* Add EDATA to the list in STATE.  */

static void
elf_add_syminfo_data (struct backtrace_state *state,
		      struct elf_syminfo_data *edata)
{
  if (!state->threaded)
    {
      struct elf_syminfo_data **pp;

      for (pp = (struct elf_syminfo_data **) (void *) &state->syminfo_data;
	   *pp != NULL;
	   pp = &(*pp)->next)
	;
      *pp = edata;
    }
  else
    {
      while (1)
	{
	  struct elf_syminfo_data **pp;

	  pp = (struct elf_syminfo_data **) (void *) &state->syminfo_data;

	  while (1)
	    {
	      struct elf_syminfo_data *p;

	      p = backtrace_atomic_load_pointer (pp);

	      if (p == NULL)
		break;

	      pp = &p->next;
	    }

	  if (__sync_bool_compare_and_swap (pp, NULL, edata))
	    break;
	}
    }
}

/* Return the symbol name and value for an ADDR.  */

static void
elf_syminfo (struct backtrace_state *state, uintptr_t addr,
	     backtrace_syminfo_callback callback,
	     backtrace_error_callback error_callback ATTRIBUTE_UNUSED,
	     void *data)
{
  struct elf_syminfo_data *edata;
  struct elf_symbol *sym = NULL;

  if (!state->threaded)
    {
      for (edata = (struct elf_syminfo_data *) state->syminfo_data;
	   edata != NULL;
	   edata = edata->next)
	{
	  sym = ((struct elf_symbol *)
		 bsearch (&addr, edata->symbols, edata->count,
			  sizeof (struct elf_symbol), elf_symbol_search));
	  if (sym != NULL)
	    break;
	}
    }
  else
    {
      struct elf_syminfo_data **pp;

      pp = (struct elf_syminfo_data **) (void *) &state->syminfo_data;
      while (1)
	{
	  edata = backtrace_atomic_load_pointer (pp);
	  if (edata == NULL)
	    break;

	  sym = ((struct elf_symbol *)
		 bsearch (&addr, edata->symbols, edata->count,
			  sizeof (struct elf_symbol), elf_symbol_search));
	  if (sym != NULL)
	    break;

	  pp = &edata->next;
	}
    }

  if (sym == NULL)
    callback (data, addr, NULL, 0, 0);
  else
    callback (data, addr, sym->name, sym->address, sym->size);
}

/* Return whether FILENAME is a symlink.  */

static int
elf_is_symlink (const char *filename)
{
  struct stat st;

  if (lstat (filename, &st) < 0)
    return 0;
  return S_ISLNK (st.st_mode);
}

/* Return the results of reading the symlink FILENAME in a buffer
   allocated by backtrace_alloc.  Return the length of the buffer in
   *LEN.  */

static char *
elf_readlink (struct backtrace_state *state, const char *filename,
	      backtrace_error_callback error_callback, void *data,
	      size_t *plen)
{
  size_t len;
  char *buf;

  len = 128;
  while (1)
    {
      ssize_t rl;

      buf = (char*)backtrace_alloc (state, len, error_callback, data);
      if (buf == NULL)
	return NULL;
      rl = readlink (filename, buf, len);
      if (rl < 0)
	{
	  backtrace_free (state, buf, len, error_callback, data);
	  return NULL;
	}
      if ((size_t) rl < len - 1)
	{
	  buf[rl] = '\0';
	  *plen = len;
	  return buf;
	}
      backtrace_free (state, buf, len, error_callback, data);
      len *= 2;
    }
}

#define SYSTEM_BUILD_ID_DIR "/usr/lib/debug/.build-id/"

/* Open a separate debug info file, using the build ID to find it.
   Returns an open file descriptor, or -1.

   The GDB manual says that the only place gdb looks for a debug file
   when the build ID is known is in /usr/lib/debug/.build-id.  */

static int
elf_open_debugfile_by_buildid (struct backtrace_state *state,
			       const char *buildid_data, size_t buildid_size,
			       backtrace_error_callback error_callback,
			       void *data)
{
  const char * const prefix = SYSTEM_BUILD_ID_DIR;
  const size_t prefix_len = strlen (prefix);
  const char * const suffix = ".debug";
  const size_t suffix_len = strlen (suffix);
  size_t len;
  char *bd_filename;
  char *t;
  size_t i;
  int ret;
  int does_not_exist;

  len = prefix_len + buildid_size * 2 + suffix_len + 2;
  bd_filename = (char*)backtrace_alloc (state, len, error_callback, data);
  if (bd_filename == NULL)
    return -1;

  t = bd_filename;
  memcpy (t, prefix, prefix_len);
  t += prefix_len;
  for (i = 0; i < buildid_size; i++)
    {
      unsigned char b;
      unsigned char nib;

      b = (unsigned char) buildid_data[i];
      nib = (b & 0xf0) >> 4;
      *t++ = nib < 10 ? '0' + nib : 'a' + nib - 10;
      nib = b & 0x0f;
      *t++ = nib < 10 ? '0' + nib : 'a' + nib - 10;
      if (i == 0)
	*t++ = '/';
    }
  memcpy (t, suffix, suffix_len);
  t[suffix_len] = '\0';

  ret = backtrace_open (bd_filename, error_callback, data, &does_not_exist);

  backtrace_free (state, bd_filename, len, error_callback, data);

  /* gdb checks that the debuginfo file has the same build ID note.
     That seems kind of pointless to me--why would it have the right
     name but not the right build ID?--so skipping the check.  */

  return ret;
}

/* Try to open a file whose name is PREFIX (length PREFIX_LEN)
   concatenated with PREFIX2 (length PREFIX2_LEN) concatenated with
   DEBUGLINK_NAME.  Returns an open file descriptor, or -1.  */

static int
elf_try_debugfile (struct backtrace_state *state, const char *prefix,
		   size_t prefix_len, const char *prefix2, size_t prefix2_len,
		   const char *debuglink_name,
		   backtrace_error_callback error_callback, void *data)
{
  size_t debuglink_len;
  size_t try_len;
  char *Try;
  int does_not_exist;
  int ret;

  debuglink_len = strlen (debuglink_name);
  try_len = prefix_len + prefix2_len + debuglink_len + 1;
  Try = (char*)backtrace_alloc (state, try_len, error_callback, data);
  if (Try == NULL)
    return -1;

  memcpy (Try, prefix, prefix_len);
  memcpy (Try + prefix_len, prefix2, prefix2_len);
  memcpy (Try + prefix_len + prefix2_len, debuglink_name, debuglink_len);
  Try[prefix_len + prefix2_len + debuglink_len] = '\0';

  ret = backtrace_open (Try, error_callback, data, &does_not_exist);

  backtrace_free (state, Try, try_len, error_callback, data);

  return ret;
}

/* Find a separate debug info file, using the debuglink section data
   to find it.  Returns an open file descriptor, or -1.  */

static int
elf_find_debugfile_by_debuglink (struct backtrace_state *state,
				 const char *filename,
				 const char *debuglink_name,
				 backtrace_error_callback error_callback,
				 void *data)
{
  int ret;
  char *alc;
  size_t alc_len;
  const char *slash;
  int ddescriptor;
  const char *prefix;
  size_t prefix_len;

  /* Resolve symlinks in FILENAME.  Since FILENAME is fairly likely to
     be /proc/self/exe, symlinks are common.  We don't try to resolve
     the whole path name, just the base name.  */
  ret = -1;
  alc = NULL;
  alc_len = 0;
  while (elf_is_symlink (filename))
    {
      char *new_buf;
      size_t new_len;

      new_buf = elf_readlink (state, filename, error_callback, data, &new_len);
      if (new_buf == NULL)
	break;

      if (new_buf[0] == '/')
	filename = new_buf;
      else
	{
	  slash = strrchr (filename, '/');
	  if (slash == NULL)
	    filename = new_buf;
	  else
	    {
	      size_t clen;
	      char *c;

	      slash++;
	      clen = slash - filename + strlen (new_buf) + 1;
	      c = (char*)backtrace_alloc (state, clen, error_callback, data);
	      if (c == NULL)
		goto done;

	      memcpy (c, filename, slash - filename);
	      memcpy (c + (slash - filename), new_buf, strlen (new_buf));
	      c[slash - filename + strlen (new_buf)] = '\0';
	      backtrace_free (state, new_buf, new_len, error_callback, data);
	      filename = c;
	      new_buf = c;
	      new_len = clen;
	    }
	}

      if (alc != NULL)
	backtrace_free (state, alc, alc_len, error_callback, data);
      alc = new_buf;
      alc_len = new_len;
    }

  /* Look for DEBUGLINK_NAME in the same directory as FILENAME.  */

  slash = strrchr (filename, '/');
  if (slash == NULL)
    {
      prefix = "";
      prefix_len = 0;
    }
  else
    {
      slash++;
      prefix = filename;
      prefix_len = slash - filename;
    }

  ddescriptor = elf_try_debugfile (state, prefix, prefix_len, "", 0,
				   debuglink_name, error_callback, data);
  if (ddescriptor >= 0)
    {
      ret = ddescriptor;
      goto done;
    }

  /* Look for DEBUGLINK_NAME in a .debug subdirectory of FILENAME.  */

  ddescriptor = elf_try_debugfile (state, prefix, prefix_len, ".debug/",
				   strlen (".debug/"), debuglink_name,
				   error_callback, data);
  if (ddescriptor >= 0)
    {
      ret = ddescriptor;
      goto done;
    }

  /* Look for DEBUGLINK_NAME in /usr/lib/debug.  */

  ddescriptor = elf_try_debugfile (state, "/usr/lib/debug/",
				   strlen ("/usr/lib/debug/"), prefix,
				   prefix_len, debuglink_name,
				   error_callback, data);
  if (ddescriptor >= 0)
    ret = ddescriptor;

 done:
  if (alc != NULL && alc_len > 0)
    backtrace_free (state, alc, alc_len, error_callback, data);
  return ret;
}

/* Open a separate debug info file, using the debuglink section data
   to find it.  Returns an open file descriptor, or -1.  */

static int
elf_open_debugfile_by_debuglink (struct backtrace_state *state,
				 const char *filename,
				 const char *debuglink_name,
				 uint32_t debuglink_crc,
				 backtrace_error_callback error_callback,
				 void *data)
{
  int ddescriptor;

  ddescriptor = elf_find_debugfile_by_debuglink (state, filename,
						 debuglink_name,
						 error_callback, data);
  if (ddescriptor < 0)
    return -1;

  if (debuglink_crc != 0)
    {
      uint32_t got_crc;

      got_crc = elf_crc32_file (state, ddescriptor, error_callback, data);
      if (got_crc != debuglink_crc)
	{
	  backtrace_close (ddescriptor, error_callback, data);
	  return -1;
	}
    }

  return ddescriptor;
}

/* A function useful for setting a breakpoint for an inflation failure
   when this code is compiled with -g.  */

static void
elf_uncompress_failed(void)
{
}

/* *PVAL is the current value being read from the stream, and *PBITS
   is the number of valid bits.  Ensure that *PVAL holds at least 15
   bits by reading additional bits from *PPIN, up to PINEND, as
   needed.  Updates *PPIN, *PVAL and *PBITS.  Returns 1 on success, 0
   on error.  */

static int
elf_zlib_fetch (const unsigned char **ppin, const unsigned char *pinend,
		uint64_t *pval, unsigned int *pbits)
{
  unsigned int bits;
  const unsigned char *pin;
  uint64_t val;
  uint32_t next;

  bits = *pbits;
  if (bits >= 15)
    return 1;
  pin = *ppin;
  val = *pval;

  if (unlikely (pinend - pin < 4))
    {
      elf_uncompress_failed ();
      return 0;
    }

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) \
    && defined(__ORDER_BIG_ENDIAN__) \
    && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ \
        || __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
  /* We've ensured that PIN is aligned.  */
  next = *(const uint32_t *)pin;

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  next = __builtin_bswap32 (next);
#endif
#else
  next = pin[0] | (pin[1] << 8) | (pin[2] << 16) | (pin[3] << 24);
#endif

  val |= (uint64_t)next << bits;
  bits += 32;
  pin += 4;

  /* We will need the next four bytes soon.  */
  __builtin_prefetch (pin, 0, 0);

  *ppin = pin;
  *pval = val;
  *pbits = bits;
  return 1;
}

/* Huffman code tables, like the rest of the zlib format, are defined
   by RFC 1951.  We store a Huffman code table as a series of tables
   stored sequentially in memory.  Each entry in a table is 16 bits.
   The first, main, table has 256 entries.  It is followed by a set of
   secondary tables of length 2 to 128 entries.  The maximum length of
   a code sequence in the deflate format is 15 bits, so that is all we
   need.  Each secondary table has an index, which is the offset of
   the table in the overall memory storage.

   The deflate format says that all codes of a given bit length are
   lexicographically consecutive.  Perhaps we could have 130 values
   that require a 15-bit code, perhaps requiring three secondary
   tables of size 128.  I don't know if this is actually possible, but
   it suggests that the maximum size required for secondary tables is
   3 * 128 + 3 * 64 ... == 768.  The zlib enough program reports 660
   as the maximum.  We permit 768, since in addition to the 256 for
   the primary table, with two bytes per entry, and with the two
   tables we need, that gives us a page.

   A single table entry needs to store a value or (for the main table
   only) the index and size of a secondary table.  Values range from 0
   to 285, inclusive.  Secondary table indexes, per above, range from
   0 to 510.  For a value we need to store the number of bits we need
   to determine that value (one value may appear multiple times in the
   table), which is 1 to 8.  For a secondary table we need to store
   the number of bits used to index into the table, which is 1 to 7.
   And of course we need 1 bit to decide whether we have a value or a
   secondary table index.  So each entry needs 9 bits for value/table
   index, 3 bits for size, 1 bit what it is.  For simplicity we use 16
   bits per entry.  */

/* Number of entries we allocate to for one code table.  We get a page
   for the two code tables we need.  */

#define HUFFMAN_TABLE_SIZE (1024)

/* Bit masks and shifts for the values in the table.  */

#define HUFFMAN_VALUE_MASK 0x01ff
#define HUFFMAN_BITS_SHIFT 9
#define HUFFMAN_BITS_MASK 0x7
#define HUFFMAN_SECONDARY_SHIFT 12

/* For working memory while inflating we need two code tables, we need
   an array of code lengths (max value 15, so we use unsigned char),
   and an array of unsigned shorts used while building a table.  The
   latter two arrays must be large enough to hold the maximum number
   of code lengths, which RFC 1951 defines as 286 + 30.  */

#define ZDEBUG_TABLE_SIZE \
  (2 * HUFFMAN_TABLE_SIZE * sizeof (uint16_t) \
   + (286 + 30) * sizeof (uint16_t)	      \
   + (286 + 30) * sizeof (unsigned char))

#define ZDEBUG_TABLE_CODELEN_OFFSET \
  (2 * HUFFMAN_TABLE_SIZE * sizeof (uint16_t) \
   + (286 + 30) * sizeof (uint16_t))

#define ZDEBUG_TABLE_WORK_OFFSET \
  (2 * HUFFMAN_TABLE_SIZE * sizeof (uint16_t))

#ifdef BACKTRACE_GENERATE_FIXED_HUFFMAN_TABLE

/* Used by the main function that generates the fixed table to learn
   the table size.  */
static size_t final_next_secondary;

#endif

/* Build a Huffman code table from an array of lengths in CODES of
   length CODES_LEN.  The table is stored into *TABLE.  ZDEBUG_TABLE
   is the same as for elf_zlib_inflate, used to find some work space.
   Returns 1 on success, 0 on error.  */

static int
elf_zlib_inflate_table (unsigned char *codes, size_t codes_len,
			uint16_t *zdebug_table, uint16_t *table)
{
  uint16_t count[16];
  uint16_t start[16];
  uint16_t prev[16];
  uint16_t firstcode[7];
  uint16_t *next;
  size_t i;
  size_t j;
  unsigned int code;
  size_t next_secondary;

  /* Count the number of code of each length.  Set NEXT[val] to be the
     next value after VAL with the same bit length.  */

  next = (uint16_t *) (((unsigned char *) zdebug_table)
		       + ZDEBUG_TABLE_WORK_OFFSET);

  memset (&count[0], 0, 16 * sizeof (uint16_t));
  for (i = 0; i < codes_len; ++i)
    {
      if (unlikely (codes[i] >= 16))
	{
	  elf_uncompress_failed ();
	  return 0;
	}

      if (count[codes[i]] == 0)
	{
	  start[codes[i]] = i;
	  prev[codes[i]] = i;
	}
      else
	{
	  next[prev[codes[i]]] = i;
	  prev[codes[i]] = i;
	}

      ++count[codes[i]];
    }

  /* For each length, fill in the table for the codes of that
     length.  */

  memset (table, 0, HUFFMAN_TABLE_SIZE * sizeof (uint16_t));

  /* Handle the values that do not require a secondary table.  */

  code = 0;
  for (j = 1; j <= 8; ++j)
    {
      unsigned int jcnt;
      unsigned int val;

      jcnt = count[j];
      if (jcnt == 0)
	continue;

      if (unlikely (jcnt > (1U << j)))
	{
	  elf_uncompress_failed ();
	  return 0;
	}

      /* There are JCNT values that have this length, the values
	 starting from START[j] continuing through NEXT[VAL].  Those
	 values are assigned consecutive values starting at CODE.  */

      val = start[j];
      for (i = 0; i < jcnt; ++i)
	{
	  uint16_t tval;
	  size_t ind;
	  unsigned int incr;

	  /* In the compressed bit stream, the value VAL is encoded as
	     J bits with the value C.  */

	  if (unlikely ((val & ~HUFFMAN_VALUE_MASK) != 0))
	    {
	      elf_uncompress_failed ();
	      return 0;
	    }

	  tval = val | ((j - 1) << HUFFMAN_BITS_SHIFT);

	  /* The table lookup uses 8 bits.  If J is less than 8, we
	     don't know what the other bits will be.  We need to fill
	     in all possibilities in the table.  Since the Huffman
	     code is unambiguous, those entries can't be used for any
	     other code.  */

	  for (ind = code; ind < 0x100; ind += 1 << j)
	    {
	      if (unlikely (table[ind] != 0))
		{
		  elf_uncompress_failed ();
		  return 0;
		}
	      table[ind] = tval;
	    }

	  /* Advance to the next value with this length.  */
	  if (i + 1 < jcnt)
	    val = next[val];

	  /* The Huffman codes are stored in the bitstream with the
	     most significant bit first, as is required to make them
	     unambiguous.  The effect is that when we read them from
	     the bitstream we see the bit sequence in reverse order:
	     the most significant bit of the Huffman code is the least
	     significant bit of the value we read from the bitstream.
	     That means that to make our table lookups work, we need
	     to reverse the bits of CODE.  Since reversing bits is
	     tedious and in general requires using a table, we instead
	     increment CODE in reverse order.  That is, if the number
	     of bits we are currently using, here named J, is 3, we
	     count as 000, 100, 010, 110, 001, 101, 011, 111, which is
	     to say the numbers from 0 to 7 but with the bits
	     reversed.  Going to more bits, aka incrementing J,
	     effectively just adds more zero bits as the beginning,
	     and as such does not change the numeric value of CODE.

	     To increment CODE of length J in reverse order, find the
	     most significant zero bit and set it to one while
	     clearing all higher bits.  In other words, add 1 modulo
	     2^J, only reversed.  */

	  incr = 1U << (j - 1);
	  while ((code & incr) != 0)
	    incr >>= 1;
	  if (incr == 0)
	    code = 0;
	  else
	    {
	      code &= incr - 1;
	      code += incr;
	    }
	}
    }

  /* Handle the values that require a secondary table.  */

  /* Set FIRSTCODE, the number at which the codes start, for each
     length.  */

  for (j = 9; j < 16; j++)
    {
      unsigned int jcnt;
      unsigned int k;

      jcnt = count[j];
      if (jcnt == 0)
	continue;

      /* There are JCNT values that have this length, the values
	 starting from START[j].  Those values are assigned
	 consecutive values starting at CODE.  */

      firstcode[j - 9] = code;

      /* Reverse add JCNT to CODE modulo 2^J.  */
      for (k = 0; k < j; ++k)
	{
	  if ((jcnt & (1U << k)) != 0)
	    {
	      unsigned int m;
	      unsigned int bit;

	      bit = 1U << (j - k - 1);
	      for (m = 0; m < j - k; ++m, bit >>= 1)
		{
		  if ((code & bit) == 0)
		    {
		      code += bit;
		      break;
		    }
		  code &= ~bit;
		}
	      jcnt &= ~(1U << k);
	    }
	}
      if (unlikely (jcnt != 0))
	{
	  elf_uncompress_failed ();
	  return 0;
	}
    }

  /* For J from 9 to 15, inclusive, we store COUNT[J] consecutive
     values starting at START[J] with consecutive codes starting at
     FIRSTCODE[J - 9].  In the primary table we need to point to the
     secondary table, and the secondary table will be indexed by J - 9
     bits.  We count down from 15 so that we install the larger
     secondary tables first, as the smaller ones may be embedded in
     the larger ones.  */

  next_secondary = 0; /* Index of next secondary table (after primary).  */
  for (j = 15; j >= 9; j--)
    {
      unsigned int jcnt;
      unsigned int val;
      size_t primary; /* Current primary index.  */
      size_t secondary; /* Offset to current secondary table.  */
      size_t secondary_bits; /* Bit size of current secondary table.  */

      jcnt = count[j];
      if (jcnt == 0)
	continue;

      val = start[j];
      code = firstcode[j - 9];
      primary = 0x100;
      secondary = 0;
      secondary_bits = 0;
      for (i = 0; i < jcnt; ++i)
	{
	  uint16_t tval;
	  size_t ind;
	  unsigned int incr;

	  if ((code & 0xff) != primary)
	    {
	      uint16_t tprimary;

	      /* Fill in a new primary table entry.  */

	      primary = code & 0xff;

	      tprimary = table[primary];
	      if (tprimary == 0)
		{
		  /* Start a new secondary table.  */

		  if (unlikely ((next_secondary & HUFFMAN_VALUE_MASK)
				!= next_secondary))
		    {
		      elf_uncompress_failed ();
		      return 0;
		    }

		  secondary = next_secondary;
		  secondary_bits = j - 8;
		  next_secondary += 1 << secondary_bits;
		  table[primary] = (secondary
				    + ((j - 8) << HUFFMAN_BITS_SHIFT)
				    + (1U << HUFFMAN_SECONDARY_SHIFT));
		}
	      else
		{
		  /* There is an existing entry.  It had better be a
		     secondary table with enough bits.  */
		  if (unlikely ((tprimary & (1U << HUFFMAN_SECONDARY_SHIFT))
				== 0))
		    {
		      elf_uncompress_failed ();
		      return 0;
		    }
		  secondary = tprimary & HUFFMAN_VALUE_MASK;
		  secondary_bits = ((tprimary >> HUFFMAN_BITS_SHIFT)
				    & HUFFMAN_BITS_MASK);
		  if (unlikely (secondary_bits < j - 8))
		    {
		      elf_uncompress_failed ();
		      return 0;
		    }
		}
	    }

	  /* Fill in secondary table entries.  */

	  tval = val | ((j - 8) << HUFFMAN_BITS_SHIFT);

	  for (ind = code >> 8;
	       ind < (1U << secondary_bits);
	       ind += 1U << (j - 8))
	    {
	      if (unlikely (table[secondary + 0x100 + ind] != 0))
		{
		  elf_uncompress_failed ();
		  return 0;
		}
	      table[secondary + 0x100 + ind] = tval;
	    }

	  if (i + 1 < jcnt)
	    val = next[val];

	  incr = 1U << (j - 1);
	  while ((code & incr) != 0)
	    incr >>= 1;
	  if (incr == 0)
	    code = 0;
	  else
	    {
	      code &= incr - 1;
	      code += incr;
	    }
	}
    }

#ifdef BACKTRACE_GENERATE_FIXED_HUFFMAN_TABLE
  final_next_secondary = next_secondary;
#endif

  return 1;
}

#ifdef BACKTRACE_GENERATE_FIXED_HUFFMAN_TABLE

/* Used to generate the fixed Huffman table for block type 1.  */

#include <stdio.h>

static uint16_t table[ZDEBUG_TABLE_SIZE];
static unsigned char codes[288];

int
main ()
{
  size_t i;

  for (i = 0; i <= 143; ++i)
    codes[i] = 8;
  for (i = 144; i <= 255; ++i)
    codes[i] = 9;
  for (i = 256; i <= 279; ++i)
    codes[i] = 7;
  for (i = 280; i <= 287; ++i)
    codes[i] = 8;
  if (!elf_zlib_inflate_table (&codes[0], 288, &table[0], &table[0]))
    {
      fprintf (stderr, "elf_zlib_inflate_table failed\n");
      exit (EXIT_FAILURE);
    }

  printf ("static const uint16_t elf_zlib_default_table[%#zx] =\n",
	  final_next_secondary + 0x100);
  printf ("{\n");
  for (i = 0; i < final_next_secondary + 0x100; i += 8)
    {
      size_t j;

      printf (" ");
      for (j = i; j < final_next_secondary + 0x100 && j < i + 8; ++j)
	printf (" %#x,", table[j]);
      printf ("\n");
    }
  printf ("};\n");
  printf ("\n");

  for (i = 0; i < 32; ++i)
    codes[i] = 5;
  if (!elf_zlib_inflate_table (&codes[0], 32, &table[0], &table[0]))
    {
      fprintf (stderr, "elf_zlib_inflate_table failed\n");
      exit (EXIT_FAILURE);
    }

  printf ("static const uint16_t elf_zlib_default_dist_table[%#zx] =\n",
	  final_next_secondary + 0x100);
  printf ("{\n");
  for (i = 0; i < final_next_secondary + 0x100; i += 8)
    {
      size_t j;

      printf (" ");
      for (j = i; j < final_next_secondary + 0x100 && j < i + 8; ++j)
	printf (" %#x,", table[j]);
      printf ("\n");
    }
  printf ("};\n");

  return 0;
}

#endif

/* The fixed tables generated by the #ifdef'ed out main function
   above.  */

static const uint16_t elf_zlib_default_table[0x170] =
{
  0xd00, 0xe50, 0xe10, 0xf18, 0xd10, 0xe70, 0xe30, 0x1230,
  0xd08, 0xe60, 0xe20, 0x1210, 0xe00, 0xe80, 0xe40, 0x1250,
  0xd04, 0xe58, 0xe18, 0x1200, 0xd14, 0xe78, 0xe38, 0x1240,
  0xd0c, 0xe68, 0xe28, 0x1220, 0xe08, 0xe88, 0xe48, 0x1260,
  0xd02, 0xe54, 0xe14, 0xf1c, 0xd12, 0xe74, 0xe34, 0x1238,
  0xd0a, 0xe64, 0xe24, 0x1218, 0xe04, 0xe84, 0xe44, 0x1258,
  0xd06, 0xe5c, 0xe1c, 0x1208, 0xd16, 0xe7c, 0xe3c, 0x1248,
  0xd0e, 0xe6c, 0xe2c, 0x1228, 0xe0c, 0xe8c, 0xe4c, 0x1268,
  0xd01, 0xe52, 0xe12, 0xf1a, 0xd11, 0xe72, 0xe32, 0x1234,
  0xd09, 0xe62, 0xe22, 0x1214, 0xe02, 0xe82, 0xe42, 0x1254,
  0xd05, 0xe5a, 0xe1a, 0x1204, 0xd15, 0xe7a, 0xe3a, 0x1244,
  0xd0d, 0xe6a, 0xe2a, 0x1224, 0xe0a, 0xe8a, 0xe4a, 0x1264,
  0xd03, 0xe56, 0xe16, 0xf1e, 0xd13, 0xe76, 0xe36, 0x123c,
  0xd0b, 0xe66, 0xe26, 0x121c, 0xe06, 0xe86, 0xe46, 0x125c,
  0xd07, 0xe5e, 0xe1e, 0x120c, 0xd17, 0xe7e, 0xe3e, 0x124c,
  0xd0f, 0xe6e, 0xe2e, 0x122c, 0xe0e, 0xe8e, 0xe4e, 0x126c,
  0xd00, 0xe51, 0xe11, 0xf19, 0xd10, 0xe71, 0xe31, 0x1232,
  0xd08, 0xe61, 0xe21, 0x1212, 0xe01, 0xe81, 0xe41, 0x1252,
  0xd04, 0xe59, 0xe19, 0x1202, 0xd14, 0xe79, 0xe39, 0x1242,
  0xd0c, 0xe69, 0xe29, 0x1222, 0xe09, 0xe89, 0xe49, 0x1262,
  0xd02, 0xe55, 0xe15, 0xf1d, 0xd12, 0xe75, 0xe35, 0x123a,
  0xd0a, 0xe65, 0xe25, 0x121a, 0xe05, 0xe85, 0xe45, 0x125a,
  0xd06, 0xe5d, 0xe1d, 0x120a, 0xd16, 0xe7d, 0xe3d, 0x124a,
  0xd0e, 0xe6d, 0xe2d, 0x122a, 0xe0d, 0xe8d, 0xe4d, 0x126a,
  0xd01, 0xe53, 0xe13, 0xf1b, 0xd11, 0xe73, 0xe33, 0x1236,
  0xd09, 0xe63, 0xe23, 0x1216, 0xe03, 0xe83, 0xe43, 0x1256,
  0xd05, 0xe5b, 0xe1b, 0x1206, 0xd15, 0xe7b, 0xe3b, 0x1246,
  0xd0d, 0xe6b, 0xe2b, 0x1226, 0xe0b, 0xe8b, 0xe4b, 0x1266,
  0xd03, 0xe57, 0xe17, 0xf1f, 0xd13, 0xe77, 0xe37, 0x123e,
  0xd0b, 0xe67, 0xe27, 0x121e, 0xe07, 0xe87, 0xe47, 0x125e,
  0xd07, 0xe5f, 0xe1f, 0x120e, 0xd17, 0xe7f, 0xe3f, 0x124e,
  0xd0f, 0xe6f, 0xe2f, 0x122e, 0xe0f, 0xe8f, 0xe4f, 0x126e,
  0x290, 0x291, 0x292, 0x293, 0x294, 0x295, 0x296, 0x297,
  0x298, 0x299, 0x29a, 0x29b, 0x29c, 0x29d, 0x29e, 0x29f,
  0x2a0, 0x2a1, 0x2a2, 0x2a3, 0x2a4, 0x2a5, 0x2a6, 0x2a7,
  0x2a8, 0x2a9, 0x2aa, 0x2ab, 0x2ac, 0x2ad, 0x2ae, 0x2af,
  0x2b0, 0x2b1, 0x2b2, 0x2b3, 0x2b4, 0x2b5, 0x2b6, 0x2b7,
  0x2b8, 0x2b9, 0x2ba, 0x2bb, 0x2bc, 0x2bd, 0x2be, 0x2bf,
  0x2c0, 0x2c1, 0x2c2, 0x2c3, 0x2c4, 0x2c5, 0x2c6, 0x2c7,
  0x2c8, 0x2c9, 0x2ca, 0x2cb, 0x2cc, 0x2cd, 0x2ce, 0x2cf,
  0x2d0, 0x2d1, 0x2d2, 0x2d3, 0x2d4, 0x2d5, 0x2d6, 0x2d7,
  0x2d8, 0x2d9, 0x2da, 0x2db, 0x2dc, 0x2dd, 0x2de, 0x2df,
  0x2e0, 0x2e1, 0x2e2, 0x2e3, 0x2e4, 0x2e5, 0x2e6, 0x2e7,
  0x2e8, 0x2e9, 0x2ea, 0x2eb, 0x2ec, 0x2ed, 0x2ee, 0x2ef,
  0x2f0, 0x2f1, 0x2f2, 0x2f3, 0x2f4, 0x2f5, 0x2f6, 0x2f7,
  0x2f8, 0x2f9, 0x2fa, 0x2fb, 0x2fc, 0x2fd, 0x2fe, 0x2ff,
};

static const uint16_t elf_zlib_default_dist_table[0x100] =
{
  0x800, 0x810, 0x808, 0x818, 0x804, 0x814, 0x80c, 0x81c,
  0x802, 0x812, 0x80a, 0x81a, 0x806, 0x816, 0x80e, 0x81e,
  0x801, 0x811, 0x809, 0x819, 0x805, 0x815, 0x80d, 0x81d,
  0x803, 0x813, 0x80b, 0x81b, 0x807, 0x817, 0x80f, 0x81f,
  0x800, 0x810, 0x808, 0x818, 0x804, 0x814, 0x80c, 0x81c,
  0x802, 0x812, 0x80a, 0x81a, 0x806, 0x816, 0x80e, 0x81e,
  0x801, 0x811, 0x809, 0x819, 0x805, 0x815, 0x80d, 0x81d,
  0x803, 0x813, 0x80b, 0x81b, 0x807, 0x817, 0x80f, 0x81f,
  0x800, 0x810, 0x808, 0x818, 0x804, 0x814, 0x80c, 0x81c,
  0x802, 0x812, 0x80a, 0x81a, 0x806, 0x816, 0x80e, 0x81e,
  0x801, 0x811, 0x809, 0x819, 0x805, 0x815, 0x80d, 0x81d,
  0x803, 0x813, 0x80b, 0x81b, 0x807, 0x817, 0x80f, 0x81f,
  0x800, 0x810, 0x808, 0x818, 0x804, 0x814, 0x80c, 0x81c,
  0x802, 0x812, 0x80a, 0x81a, 0x806, 0x816, 0x80e, 0x81e,
  0x801, 0x811, 0x809, 0x819, 0x805, 0x815, 0x80d, 0x81d,
  0x803, 0x813, 0x80b, 0x81b, 0x807, 0x817, 0x80f, 0x81f,
  0x800, 0x810, 0x808, 0x818, 0x804, 0x814, 0x80c, 0x81c,
  0x802, 0x812, 0x80a, 0x81a, 0x806, 0x816, 0x80e, 0x81e,
  0x801, 0x811, 0x809, 0x819, 0x805, 0x815, 0x80d, 0x81d,
  0x803, 0x813, 0x80b, 0x81b, 0x807, 0x817, 0x80f, 0x81f,
  0x800, 0x810, 0x808, 0x818, 0x804, 0x814, 0x80c, 0x81c,
  0x802, 0x812, 0x80a, 0x81a, 0x806, 0x816, 0x80e, 0x81e,
  0x801, 0x811, 0x809, 0x819, 0x805, 0x815, 0x80d, 0x81d,
  0x803, 0x813, 0x80b, 0x81b, 0x807, 0x817, 0x80f, 0x81f,
  0x800, 0x810, 0x808, 0x818, 0x804, 0x814, 0x80c, 0x81c,
  0x802, 0x812, 0x80a, 0x81a, 0x806, 0x816, 0x80e, 0x81e,
  0x801, 0x811, 0x809, 0x819, 0x805, 0x815, 0x80d, 0x81d,
  0x803, 0x813, 0x80b, 0x81b, 0x807, 0x817, 0x80f, 0x81f,
  0x800, 0x810, 0x808, 0x818, 0x804, 0x814, 0x80c, 0x81c,
  0x802, 0x812, 0x80a, 0x81a, 0x806, 0x816, 0x80e, 0x81e,
  0x801, 0x811, 0x809, 0x819, 0x805, 0x815, 0x80d, 0x81d,
  0x803, 0x813, 0x80b, 0x81b, 0x807, 0x817, 0x80f, 0x81f,
};

/* Inflate a zlib stream from PIN/SIN to POUT/SOUT.  Return 1 on
   success, 0 on some error parsing the stream.  */

static int
elf_zlib_inflate (const unsigned char *pin, size_t sin, uint16_t *zdebug_table,
		  unsigned char *pout, size_t sout)
{
  unsigned char *porigout;
  const unsigned char *pinend;
  unsigned char *poutend;

  /* We can apparently see multiple zlib streams concatenated
     together, so keep going as long as there is something to read.
     The last 4 bytes are the checksum.  */
  porigout = pout;
  pinend = pin + sin;
  poutend = pout + sout;
  while ((pinend - pin) > 4)
    {
      uint64_t val;
      unsigned int bits;
      int last;

      /* Read the two byte zlib header.  */

      if (unlikely ((pin[0] & 0xf) != 8)) /* 8 is zlib encoding.  */
	{
	  /* Unknown compression method.  */
	  elf_uncompress_failed ();
	  return 0;
	}
      if (unlikely ((pin[0] >> 4) > 7))
	{
	  /* Window size too large.  Other than this check, we don't
	     care about the window size.  */
	  elf_uncompress_failed ();
	  return 0;
	}
      if (unlikely ((pin[1] & 0x20) != 0))
	{
	  /* Stream expects a predefined dictionary, but we have no
	     dictionary.  */
	  elf_uncompress_failed ();
	  return 0;
	}
      val = (pin[0] << 8) | pin[1];
      if (unlikely (val % 31 != 0))
	{
	  /* Header check failure.  */
	  elf_uncompress_failed ();
	  return 0;
	}
      pin += 2;

      /* Align PIN to a 32-bit boundary.  */

      val = 0;
      bits = 0;
      while ((((uintptr_t) pin) & 3) != 0)
	{
	  val |= (uint64_t)*pin << bits;
	  bits += 8;
	  ++pin;
	}

      /* Read blocks until one is marked last.  */

      last = 0;

      while (!last)
	{
	  unsigned int type;
	  const uint16_t *tlit;
	  const uint16_t *tdist;

	  if (!elf_zlib_fetch (&pin, pinend, &val, &bits))
	    return 0;

	  last = val & 1;
	  type = (val >> 1) & 3;
	  val >>= 3;
	  bits -= 3;

	  if (unlikely (type == 3))
	    {
	      /* Invalid block type.  */
	      elf_uncompress_failed ();
	      return 0;
	    }

	  if (type == 0)
	    {
	      uint16_t len;
	      uint16_t lenc;

	      /* An uncompressed block.  */

	      /* If we've read ahead more than a byte, back up.  */
	      while (bits > 8)
		{
		  --pin;
		  bits -= 8;
		}

	      val = 0;
	      bits = 0;
	      if (unlikely ((pinend - pin) < 4))
		{
		  /* Missing length.  */
		  elf_uncompress_failed ();
		  return 0;
		}
	      len = pin[0] | (pin[1] << 8);
	      lenc = pin[2] | (pin[3] << 8);
	      pin += 4;
	      lenc = ~lenc;
	      if (unlikely (len != lenc))
		{
		  /* Corrupt data.  */
		  elf_uncompress_failed ();
		  return 0;
		}
	      if (unlikely (len > (unsigned int) (pinend - pin)
			    || len > (unsigned int) (poutend - pout)))
		{
		  /* Not enough space in buffers.  */
		  elf_uncompress_failed ();
		  return 0;
		}
	      memcpy (pout, pin, len);
	      pout += len;
	      pin += len;

	      /* Align PIN.  */
	      while ((((uintptr_t) pin) & 3) != 0)
		{
		  val |= (uint64_t)*pin << bits;
		  bits += 8;
		  ++pin;
		}

	      /* Go around to read the next block.  */
	      continue;
	    }

	  if (type == 1)
	    {
	      tlit = elf_zlib_default_table;
	      tdist = elf_zlib_default_dist_table;
	    }
	  else
	    {
	      unsigned int nlit;
	      unsigned int ndist;
	      unsigned int nclen;
	      unsigned char codebits[19];
	      unsigned char *plenbase;
	      unsigned char *plen;
	      unsigned char *plenend;

	      /* Read a Huffman encoding table.  The various magic
		 numbers here are from RFC 1951.  */

	      if (!elf_zlib_fetch (&pin, pinend, &val, &bits))
		return 0;

	      nlit = (val & 0x1f) + 257;
	      val >>= 5;
	      ndist = (val & 0x1f) + 1;
	      val >>= 5;
	      nclen = (val & 0xf) + 4;
	      val >>= 4;
	      bits -= 14;
	      if (unlikely (nlit > 286 || ndist > 30))
		{
		  /* Values out of range.  */
		  elf_uncompress_failed ();
		  return 0;
		}

	      /* Read and build the table used to compress the
		 literal, length, and distance codes.  */

	      memset(&codebits[0], 0, 19);

	      /* There are always at least 4 elements in the
		 table.  */

	      if (!elf_zlib_fetch (&pin, pinend, &val, &bits))
		return 0;

	      codebits[16] = val & 7;
	      codebits[17] = (val >> 3) & 7;
	      codebits[18] = (val >> 6) & 7;
	      codebits[0] = (val >> 9) & 7;
	      val >>= 12;
	      bits -= 12;

	      if (nclen == 4)
		goto codebitsdone;

	      codebits[8] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 5)
		goto codebitsdone;

	      if (!elf_zlib_fetch (&pin, pinend, &val, &bits))
		return 0;

	      codebits[7] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 6)
		goto codebitsdone;

	      codebits[9] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 7)
		goto codebitsdone;

	      codebits[6] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 8)
		goto codebitsdone;

	      codebits[10] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 9)
		goto codebitsdone;

	      codebits[5] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 10)
		goto codebitsdone;

	      if (!elf_zlib_fetch (&pin, pinend, &val, &bits))
		return 0;

	      codebits[11] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 11)
		goto codebitsdone;

	      codebits[4] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 12)
		goto codebitsdone;

	      codebits[12] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 13)
		goto codebitsdone;

	      codebits[3] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 14)
		goto codebitsdone;

	      codebits[13] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 15)
		goto codebitsdone;

	      if (!elf_zlib_fetch (&pin, pinend, &val, &bits))
		return 0;

	      codebits[2] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 16)
		goto codebitsdone;

	      codebits[14] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 17)
		goto codebitsdone;

	      codebits[1] = val & 7;
	      val >>= 3;
	      bits -= 3;

	      if (nclen == 18)
		goto codebitsdone;

	      codebits[15] = val & 7;
	      val >>= 3;
	      bits -= 3;

	    codebitsdone:

	      if (!elf_zlib_inflate_table (codebits, 19, zdebug_table,
					   zdebug_table))
		return 0;

	      /* Read the compressed bit lengths of the literal,
		 length, and distance codes.  We have allocated space
		 at the end of zdebug_table to hold them.  */

	      plenbase = (((unsigned char *) zdebug_table)
			  + ZDEBUG_TABLE_CODELEN_OFFSET);
	      plen = plenbase;
	      plenend = plen + nlit + ndist;
	      while (plen < plenend)
		{
		  uint16_t t;
		  unsigned int b;
		  uint16_t v;

		  if (!elf_zlib_fetch (&pin, pinend, &val, &bits))
		    return 0;

		  t = zdebug_table[val & 0xff];

		  /* The compression here uses bit lengths up to 7, so
		     a secondary table is never necessary.  */
		  if (unlikely ((t & (1U << HUFFMAN_SECONDARY_SHIFT)) != 0))
		    {
		      elf_uncompress_failed ();
		      return 0;
		    }

		  b = (t >> HUFFMAN_BITS_SHIFT) & HUFFMAN_BITS_MASK;
		  val >>= b + 1;
		  bits -= b + 1;

		  v = t & HUFFMAN_VALUE_MASK;
		  if (v < 16)
		    *plen++ = v;
		  else if (v == 16)
		    {
		      unsigned int c;
		      unsigned int prev;

		      /* Copy previous entry 3 to 6 times.  */

		      if (unlikely (plen == plenbase))
			{
			  elf_uncompress_failed ();
			  return 0;
			}

		      /* We used up to 7 bits since the last
			 elf_zlib_fetch, so we have at least 8 bits
			 available here.  */

		      c = 3 + (val & 0x3);
		      val >>= 2;
		      bits -= 2;
		      if (unlikely ((unsigned int) (plenend - plen) < c))
			{
			  elf_uncompress_failed ();
			  return 0;
			}

		      prev = plen[-1];
		      switch (c)
			{
			case 6:
			  *plen++ = prev;
			  ATTRIBUTE_FALLTHROUGH;
			case 5:
			  *plen++ = prev;
			  ATTRIBUTE_FALLTHROUGH;
			case 4:
			  *plen++ = prev;
			}
		      *plen++ = prev;
		      *plen++ = prev;
		      *plen++ = prev;
		    }
		  else if (v == 17)
		    {
		      unsigned int c;

		      /* Store zero 3 to 10 times.  */

		      /* We used up to 7 bits since the last
			 elf_zlib_fetch, so we have at least 8 bits
			 available here.  */

		      c = 3 + (val & 0x7);
		      val >>= 3;
		      bits -= 3;
		      if (unlikely ((unsigned int) (plenend - plen) < c))
			{
			  elf_uncompress_failed ();
			  return 0;
			}

		      switch (c)
			{
			case 10:
			  *plen++ = 0;
			  ATTRIBUTE_FALLTHROUGH;
			case 9:
			  *plen++ = 0;
			  ATTRIBUTE_FALLTHROUGH;
			case 8:
			  *plen++ = 0;
			  ATTRIBUTE_FALLTHROUGH;
			case 7:
			  *plen++ = 0;
			  ATTRIBUTE_FALLTHROUGH;
			case 6:
			  *plen++ = 0;
			  ATTRIBUTE_FALLTHROUGH;
			case 5:
			  *plen++ = 0;
			  ATTRIBUTE_FALLTHROUGH;
			case 4:
			  *plen++ = 0;
			}
		      *plen++ = 0;
		      *plen++ = 0;
		      *plen++ = 0;
		    }
		  else if (v == 18)
		    {
		      unsigned int c;

		      /* Store zero 11 to 138 times.  */

		      /* We used up to 7 bits since the last
			 elf_zlib_fetch, so we have at least 8 bits
			 available here.  */

		      c = 11 + (val & 0x7f);
		      val >>= 7;
		      bits -= 7;
		      if (unlikely ((unsigned int) (plenend - plen) < c))
			{
			  elf_uncompress_failed ();
			  return 0;
			}

		      memset (plen, 0, c);
		      plen += c;
		    }
		  else
		    {
		      elf_uncompress_failed ();
		      return 0;
		    }
		}

	      /* Make sure that the stop code can appear.  */

	      plen = plenbase;
	      if (unlikely (plen[256] == 0))
		{
		  elf_uncompress_failed ();
		  return 0;
		}

	      /* Build the decompression tables.  */

	      if (!elf_zlib_inflate_table (plen, nlit, zdebug_table,
					   zdebug_table))
		return 0;
	      if (!elf_zlib_inflate_table (plen + nlit, ndist, zdebug_table,
					   zdebug_table + HUFFMAN_TABLE_SIZE))
		return 0;
	      tlit = zdebug_table;
	      tdist = zdebug_table + HUFFMAN_TABLE_SIZE;
	    }

	  /* Inflate values until the end of the block.  This is the
	     main loop of the inflation code.  */

	  while (1)
	    {
	      uint16_t t;
	      unsigned int b;
	      uint16_t v;
	      unsigned int lit;

	      if (!elf_zlib_fetch (&pin, pinend, &val, &bits))
		return 0;

	      t = tlit[val & 0xff];
	      b = (t >> HUFFMAN_BITS_SHIFT) & HUFFMAN_BITS_MASK;
	      v = t & HUFFMAN_VALUE_MASK;

	      if ((t & (1U << HUFFMAN_SECONDARY_SHIFT)) == 0)
		{
		  lit = v;
		  val >>= b + 1;
		  bits -= b + 1;
		}
	      else
		{
		  t = tlit[v + 0x100 + ((val >> 8) & ((1U << b) - 1))];
		  b = (t >> HUFFMAN_BITS_SHIFT) & HUFFMAN_BITS_MASK;
		  lit = t & HUFFMAN_VALUE_MASK;
		  val >>= b + 8;
		  bits -= b + 8;
		}

	      if (lit < 256)
		{
		  if (unlikely (pout == poutend))
		    {
		      elf_uncompress_failed ();
		      return 0;
		    }

		  *pout++ = lit;

		  /* We will need to write the next byte soon.  We ask
		     for high temporal locality because we will write
		     to the whole cache line soon.  */
		  __builtin_prefetch (pout, 1, 3);
		}
	      else if (lit == 256)
		{
		  /* The end of the block.  */
		  break;
		}
	      else
		{
		  unsigned int dist;
		  unsigned int len;

		  /* Convert lit into a length.  */

		  if (lit < 265)
		    len = lit - 257 + 3;
		  else if (lit == 285)
		    len = 258;
		  else if (unlikely (lit > 285))
		    {
		      elf_uncompress_failed ();
		      return 0;
		    }
		  else
		    {
		      unsigned int extra;

		      if (!elf_zlib_fetch (&pin, pinend, &val, &bits))
			return 0;

		      /* This is an expression for the table of length
			 codes in RFC 1951 3.2.5.  */
		      lit -= 265;
		      extra = (lit >> 2) + 1;
		      len = (lit & 3) << extra;
		      len += 11;
		      len += ((1U << (extra - 1)) - 1) << 3;
		      len += val & ((1U << extra) - 1);
		      val >>= extra;
		      bits -= extra;
		    }

		  if (!elf_zlib_fetch (&pin, pinend, &val, &bits))
		    return 0;

		  t = tdist[val & 0xff];
		  b = (t >> HUFFMAN_BITS_SHIFT) & HUFFMAN_BITS_MASK;
		  v = t & HUFFMAN_VALUE_MASK;

		  if ((t & (1U << HUFFMAN_SECONDARY_SHIFT)) == 0)
		    {
		      dist = v;
		      val >>= b + 1;
		      bits -= b + 1;
		    }
		  else
		    {
		      t = tdist[v + 0x100 + ((val >> 8) & ((1U << b) - 1))];
		      b = (t >> HUFFMAN_BITS_SHIFT) & HUFFMAN_BITS_MASK;
		      dist = t & HUFFMAN_VALUE_MASK;
		      val >>= b + 8;
		      bits -= b + 8;
		    }

		  /* Convert dist to a distance.  */

		  if (dist == 0)
		    {
		      /* A distance of 1.  A common case, meaning
			 repeat the last character LEN times.  */

		      if (unlikely (pout == porigout))
			{
			  elf_uncompress_failed ();
			  return 0;
			}

		      if (unlikely ((unsigned int) (poutend - pout) < len))
			{
			  elf_uncompress_failed ();
			  return 0;
			}

		      memset (pout, pout[-1], len);
		      pout += len;
		    }
		  else if (unlikely (dist > 29))
		    {
		      elf_uncompress_failed ();
		      return 0;
		    }
		  else
		    {
		      if (dist < 4)
			dist = dist + 1;
		      else
			{
			  unsigned int extra;

			  if (!elf_zlib_fetch (&pin, pinend, &val, &bits))
			    return 0;

			  /* This is an expression for the table of
			     distance codes in RFC 1951 3.2.5.  */
			  dist -= 4;
			  extra = (dist >> 1) + 1;
			  dist = (dist & 1) << extra;
			  dist += 5;
			  dist += ((1U << (extra - 1)) - 1) << 2;
			  dist += val & ((1U << extra) - 1);
			  val >>= extra;
			  bits -= extra;
			}

		      /* Go back dist bytes, and copy len bytes from
			 there.  */

		      if (unlikely ((unsigned int) (pout - porigout) < dist))
			{
			  elf_uncompress_failed ();
			  return 0;
			}

		      if (unlikely ((unsigned int) (poutend - pout) < len))
			{
			  elf_uncompress_failed ();
			  return 0;
			}

		      if (dist >= len)
			{
			  memcpy (pout, pout - dist, len);
			  pout += len;
			}
		      else
			{
			  while (len > 0)
			    {
			      unsigned int copy;

			      copy = len < dist ? len : dist;
			      memcpy (pout, pout - dist, copy);
			      len -= copy;
			      pout += copy;
			    }
			}
		    }
		}
	    }
	}
    }

  /* We should have filled the output buffer.  */
  if (unlikely (pout != poutend))
    {
      elf_uncompress_failed ();
      return 0;
    }

  return 1;
}

/* Verify the zlib checksum.  The checksum is in the 4 bytes at
   CHECKBYTES, and the uncompressed data is at UNCOMPRESSED /
   UNCOMPRESSED_SIZE.  Returns 1 on success, 0 on failure.  */

static int
elf_zlib_verify_checksum (const unsigned char *checkbytes,
			  const unsigned char *uncompressed,
			  size_t uncompressed_size)
{
  unsigned int i;
  unsigned int cksum;
  const unsigned char *p;
  uint32_t s1;
  uint32_t s2;
  size_t hsz;

  cksum = 0;
  for (i = 0; i < 4; i++)
    cksum = (cksum << 8) | checkbytes[i];

  s1 = 1;
  s2 = 0;

  /* Minimize modulo operations.  */

  p = uncompressed;
  hsz = uncompressed_size;
  while (hsz >= 5552)
    {
      for (i = 0; i < 5552; i += 16)
	{
	  /* Manually unroll loop 16 times.  */
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	  s1 = s1 + *p++;
	  s2 = s2 + s1;
	}
      hsz -= 5552;
      s1 %= 65521;
      s2 %= 65521;
    }

  while (hsz >= 16)
    {
      /* Manually unroll loop 16 times.  */
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;
      s1 = s1 + *p++;
      s2 = s2 + s1;

      hsz -= 16;
    }

  for (i = 0; i < hsz; ++i)
    {
      s1 = s1 + *p++;
      s2 = s2 + s1;
    }

  s1 %= 65521;
  s2 %= 65521;

  if (unlikely ((s2 << 16) + s1 != cksum))
    {
      elf_uncompress_failed ();
      return 0;
    }

  return 1;
}

/* Inflate a zlib stream from PIN/SIN to POUT/SOUT, and verify the
   checksum.  Return 1 on success, 0 on error.  */

static int
elf_zlib_inflate_and_verify (const unsigned char *pin, size_t sin,
			     uint16_t *zdebug_table, unsigned char *pout,
			     size_t sout)
{
  if (!elf_zlib_inflate (pin, sin, zdebug_table, pout, sout))
    return 0;
  if (!elf_zlib_verify_checksum (pin + sin - 4, pout, sout))
    return 0;
  return 1;
}

/* Uncompress the old compressed debug format, the one emitted by
   --compress-debug-sections=zlib-gnu.  The compressed data is in
   COMPRESSED / COMPRESSED_SIZE, and the function writes to
   *UNCOMPRESSED / *UNCOMPRESSED_SIZE.  ZDEBUG_TABLE is work space to
   hold Huffman tables.  Returns 0 on error, 1 on successful
   decompression or if something goes wrong.  In general we try to
   carry on, by returning 1, even if we can't decompress.  */

static int
elf_uncompress_zdebug (struct backtrace_state *state,
		       const unsigned char *compressed, size_t compressed_size,
		       uint16_t *zdebug_table,
		       backtrace_error_callback error_callback, void *data,
		       unsigned char **uncompressed, size_t *uncompressed_size)
{
  size_t sz;
  size_t i;
  unsigned char *po;

  *uncompressed = NULL;
  *uncompressed_size = 0;

  /* The format starts with the four bytes ZLIB, followed by the 8
     byte length of the uncompressed data in big-endian order,
     followed by a zlib stream.  */

  if (compressed_size < 12 || memcmp (compressed, "ZLIB", 4) != 0)
    return 1;

  sz = 0;
  for (i = 0; i < 8; i++)
    sz = (sz << 8) | compressed[i + 4];

  if (*uncompressed != NULL && *uncompressed_size >= sz)
    po = *uncompressed;
  else
    {
      po = (unsigned char *) backtrace_alloc (state, sz, error_callback, data);
      if (po == NULL)
	return 0;
    }

  if (!elf_zlib_inflate_and_verify (compressed + 12, compressed_size - 12,
				    zdebug_table, po, sz))
    return 1;

  *uncompressed = po;
  *uncompressed_size = sz;

  return 1;
}

/* Uncompress the new compressed debug format, the official standard
   ELF approach emitted by --compress-debug-sections=zlib-gabi.  The
   compressed data is in COMPRESSED / COMPRESSED_SIZE, and the
   function writes to *UNCOMPRESSED / *UNCOMPRESSED_SIZE.
   ZDEBUG_TABLE is work space as for elf_uncompress_zdebug.  Returns 0
   on error, 1 on successful decompression or if something goes wrong.
   In general we try to carry on, by returning 1, even if we can't
   decompress.  */

static int
elf_uncompress_chdr (struct backtrace_state *state,
		     const unsigned char *compressed, size_t compressed_size,
		     uint16_t *zdebug_table,
		     backtrace_error_callback error_callback, void *data,
		     unsigned char **uncompressed, size_t *uncompressed_size)
{
  const b_elf_chdr *chdr;
  unsigned char *po;

  *uncompressed = NULL;
  *uncompressed_size = 0;

  /* The format starts with an ELF compression header.  */
  if (compressed_size < sizeof (b_elf_chdr))
    return 1;

  chdr = (const b_elf_chdr *) compressed;

  if (chdr->ch_type != ELFCOMPRESS_ZLIB)
    {
      /* Unsupported compression algorithm.  */
      return 1;
    }

  if (*uncompressed != NULL && *uncompressed_size >= chdr->ch_size)
    po = *uncompressed;
  else
    {
      po = (unsigned char *) backtrace_alloc (state, chdr->ch_size,
					      error_callback, data);
      if (po == NULL)
	return 0;
    }

  if (!elf_zlib_inflate_and_verify (compressed + sizeof (b_elf_chdr),
				    compressed_size - sizeof (b_elf_chdr),
				    zdebug_table, po, chdr->ch_size))
    return 1;

  *uncompressed = po;
  *uncompressed_size = chdr->ch_size;

  return 1;
}

/* This function is a hook for testing the zlib support.  It is only
   used by tests.  */

int
backtrace_uncompress_zdebug (struct backtrace_state *state,
			     const unsigned char *compressed,
			     size_t compressed_size,
			     backtrace_error_callback error_callback,
			     void *data, unsigned char **uncompressed,
			     size_t *uncompressed_size)
{
  uint16_t *zdebug_table;
  int ret;

  zdebug_table = ((uint16_t *) backtrace_alloc (state, ZDEBUG_TABLE_SIZE,
						error_callback, data));
  if (zdebug_table == NULL)
    return 0;
  ret = elf_uncompress_zdebug (state, compressed, compressed_size,
			       zdebug_table, error_callback, data,
			       uncompressed, uncompressed_size);
  backtrace_free (state, zdebug_table, ZDEBUG_TABLE_SIZE,
		  error_callback, data);
  return ret;
}

/* Number of LZMA states.  */
#define LZMA_STATES (12)

/* Number of LZMA position states.  The pb value of the property byte
   is the number of bits to include in these states, and the maximum
   value of pb is 4.  */
#define LZMA_POS_STATES (16)

/* Number of LZMA distance states.  These are used match distances
   with a short match length: up to 4 bytes.  */
#define LZMA_DIST_STATES (4)

/* Number of LZMA distance slots.  LZMA uses six bits to encode larger
   match lengths, so 1 << 6 possible probabilities.  */
#define LZMA_DIST_SLOTS (64)

/* LZMA distances 0 to 3 are encoded directly, larger values use a
   probability model.  */
#define LZMA_DIST_MODEL_START (4)

/* The LZMA probability model ends at 14.  */
#define LZMA_DIST_MODEL_END (14)

/* LZMA distance slots for distances less than 127.  */
#define LZMA_FULL_DISTANCES (128)

/* LZMA uses four alignment bits.  */
#define LZMA_ALIGN_SIZE (16)

/* LZMA match length is encoded with 4, 5, or 10 bits, some of which
   are already known.  */
#define LZMA_LEN_LOW_SYMBOLS (8)
#define LZMA_LEN_MID_SYMBOLS (8)
#define LZMA_LEN_HIGH_SYMBOLS (256)

/* LZMA literal encoding.  */
#define LZMA_LITERAL_CODERS_MAX (16)
#define LZMA_LITERAL_CODER_SIZE (0x300)

/* LZMA is based on a large set of probabilities, each managed
   independently.  Each probability is an 11 bit number that we store
   in a uint16_t.  We use a single large array of probabilities.  */

/* Lengths of entries in the LZMA probabilities array.  The names used
   here are copied from the Linux kernel implementation.  */

#define LZMA_PROB_IS_MATCH_LEN (LZMA_STATES * LZMA_POS_STATES)
#define LZMA_PROB_IS_REP_LEN LZMA_STATES
#define LZMA_PROB_IS_REP0_LEN LZMA_STATES
#define LZMA_PROB_IS_REP1_LEN LZMA_STATES
#define LZMA_PROB_IS_REP2_LEN LZMA_STATES
#define LZMA_PROB_IS_REP0_LONG_LEN (LZMA_STATES * LZMA_POS_STATES)
#define LZMA_PROB_DIST_SLOT_LEN (LZMA_DIST_STATES * LZMA_DIST_SLOTS)
#define LZMA_PROB_DIST_SPECIAL_LEN (LZMA_FULL_DISTANCES - LZMA_DIST_MODEL_END)
#define LZMA_PROB_DIST_ALIGN_LEN LZMA_ALIGN_SIZE
#define LZMA_PROB_MATCH_LEN_CHOICE_LEN 1
#define LZMA_PROB_MATCH_LEN_CHOICE2_LEN 1
#define LZMA_PROB_MATCH_LEN_LOW_LEN (LZMA_POS_STATES * LZMA_LEN_LOW_SYMBOLS)
#define LZMA_PROB_MATCH_LEN_MID_LEN (LZMA_POS_STATES * LZMA_LEN_MID_SYMBOLS)
#define LZMA_PROB_MATCH_LEN_HIGH_LEN LZMA_LEN_HIGH_SYMBOLS
#define LZMA_PROB_REP_LEN_CHOICE_LEN 1
#define LZMA_PROB_REP_LEN_CHOICE2_LEN 1
#define LZMA_PROB_REP_LEN_LOW_LEN (LZMA_POS_STATES * LZMA_LEN_LOW_SYMBOLS)
#define LZMA_PROB_REP_LEN_MID_LEN (LZMA_POS_STATES * LZMA_LEN_MID_SYMBOLS)
#define LZMA_PROB_REP_LEN_HIGH_LEN LZMA_LEN_HIGH_SYMBOLS
#define LZMA_PROB_LITERAL_LEN \
  (LZMA_LITERAL_CODERS_MAX * LZMA_LITERAL_CODER_SIZE)

/* Offsets into the LZMA probabilities array.  This is mechanically
   generated from the above lengths.  */

#define LZMA_PROB_IS_MATCH_OFFSET 0
#define LZMA_PROB_IS_REP_OFFSET \
  (LZMA_PROB_IS_MATCH_OFFSET + LZMA_PROB_IS_MATCH_LEN)
#define LZMA_PROB_IS_REP0_OFFSET \
  (LZMA_PROB_IS_REP_OFFSET + LZMA_PROB_IS_REP_LEN)
#define LZMA_PROB_IS_REP1_OFFSET \
  (LZMA_PROB_IS_REP0_OFFSET + LZMA_PROB_IS_REP0_LEN)
#define LZMA_PROB_IS_REP2_OFFSET \
  (LZMA_PROB_IS_REP1_OFFSET + LZMA_PROB_IS_REP1_LEN)
#define LZMA_PROB_IS_REP0_LONG_OFFSET \
  (LZMA_PROB_IS_REP2_OFFSET + LZMA_PROB_IS_REP2_LEN)
#define LZMA_PROB_DIST_SLOT_OFFSET \
  (LZMA_PROB_IS_REP0_LONG_OFFSET + LZMA_PROB_IS_REP0_LONG_LEN)
#define LZMA_PROB_DIST_SPECIAL_OFFSET \
  (LZMA_PROB_DIST_SLOT_OFFSET + LZMA_PROB_DIST_SLOT_LEN)
#define LZMA_PROB_DIST_ALIGN_OFFSET \
  (LZMA_PROB_DIST_SPECIAL_OFFSET + LZMA_PROB_DIST_SPECIAL_LEN)
#define LZMA_PROB_MATCH_LEN_CHOICE_OFFSET \
  (LZMA_PROB_DIST_ALIGN_OFFSET + LZMA_PROB_DIST_ALIGN_LEN)
#define LZMA_PROB_MATCH_LEN_CHOICE2_OFFSET \
  (LZMA_PROB_MATCH_LEN_CHOICE_OFFSET + LZMA_PROB_MATCH_LEN_CHOICE_LEN)
#define LZMA_PROB_MATCH_LEN_LOW_OFFSET \
  (LZMA_PROB_MATCH_LEN_CHOICE2_OFFSET + LZMA_PROB_MATCH_LEN_CHOICE2_LEN)
#define LZMA_PROB_MATCH_LEN_MID_OFFSET \
  (LZMA_PROB_MATCH_LEN_LOW_OFFSET + LZMA_PROB_MATCH_LEN_LOW_LEN)
#define LZMA_PROB_MATCH_LEN_HIGH_OFFSET \
  (LZMA_PROB_MATCH_LEN_MID_OFFSET + LZMA_PROB_MATCH_LEN_MID_LEN)
#define LZMA_PROB_REP_LEN_CHOICE_OFFSET \
  (LZMA_PROB_MATCH_LEN_HIGH_OFFSET + LZMA_PROB_MATCH_LEN_HIGH_LEN)
#define LZMA_PROB_REP_LEN_CHOICE2_OFFSET \
  (LZMA_PROB_REP_LEN_CHOICE_OFFSET + LZMA_PROB_REP_LEN_CHOICE_LEN)
#define LZMA_PROB_REP_LEN_LOW_OFFSET \
  (LZMA_PROB_REP_LEN_CHOICE2_OFFSET + LZMA_PROB_REP_LEN_CHOICE2_LEN)
#define LZMA_PROB_REP_LEN_MID_OFFSET \
  (LZMA_PROB_REP_LEN_LOW_OFFSET + LZMA_PROB_REP_LEN_LOW_LEN)
#define LZMA_PROB_REP_LEN_HIGH_OFFSET \
  (LZMA_PROB_REP_LEN_MID_OFFSET + LZMA_PROB_REP_LEN_MID_LEN)
#define LZMA_PROB_LITERAL_OFFSET \
  (LZMA_PROB_REP_LEN_HIGH_OFFSET + LZMA_PROB_REP_LEN_HIGH_LEN)

#define LZMA_PROB_TOTAL_COUNT \
  (LZMA_PROB_LITERAL_OFFSET + LZMA_PROB_LITERAL_LEN)

/* Check that the number of LZMA probabilities is the same as the
   Linux kernel implementation.  */

#if LZMA_PROB_TOTAL_COUNT != 1846 + (1 << 4) * 0x300
 #error Wrong number of LZMA probabilities
#endif

/* Expressions for the offset in the LZMA probabilities array of a
   specific probability.  */

#define LZMA_IS_MATCH(state, pos) \
  (LZMA_PROB_IS_MATCH_OFFSET + (state) * LZMA_POS_STATES + (pos))
#define LZMA_IS_REP(state) \
  (LZMA_PROB_IS_REP_OFFSET + (state))
#define LZMA_IS_REP0(state) \
  (LZMA_PROB_IS_REP0_OFFSET + (state))
#define LZMA_IS_REP1(state) \
  (LZMA_PROB_IS_REP1_OFFSET + (state))
#define LZMA_IS_REP2(state) \
  (LZMA_PROB_IS_REP2_OFFSET + (state))
#define LZMA_IS_REP0_LONG(state, pos) \
  (LZMA_PROB_IS_REP0_LONG_OFFSET + (state) * LZMA_POS_STATES + (pos))
#define LZMA_DIST_SLOT(dist, slot) \
  (LZMA_PROB_DIST_SLOT_OFFSET + (dist) * LZMA_DIST_SLOTS + (slot))
#define LZMA_DIST_SPECIAL(dist) \
  (LZMA_PROB_DIST_SPECIAL_OFFSET + (dist))
#define LZMA_DIST_ALIGN(dist) \
  (LZMA_PROB_DIST_ALIGN_OFFSET + (dist))
#define LZMA_MATCH_LEN_CHOICE \
  LZMA_PROB_MATCH_LEN_CHOICE_OFFSET
#define LZMA_MATCH_LEN_CHOICE2 \
  LZMA_PROB_MATCH_LEN_CHOICE2_OFFSET
#define LZMA_MATCH_LEN_LOW(pos, sym) \
  (LZMA_PROB_MATCH_LEN_LOW_OFFSET + (pos) * LZMA_LEN_LOW_SYMBOLS + (sym))
#define LZMA_MATCH_LEN_MID(pos, sym) \
  (LZMA_PROB_MATCH_LEN_MID_OFFSET + (pos) * LZMA_LEN_MID_SYMBOLS + (sym))
#define LZMA_MATCH_LEN_HIGH(sym) \
  (LZMA_PROB_MATCH_LEN_HIGH_OFFSET + (sym))
#define LZMA_REP_LEN_CHOICE \
  LZMA_PROB_REP_LEN_CHOICE_OFFSET
#define LZMA_REP_LEN_CHOICE2 \
  LZMA_PROB_REP_LEN_CHOICE2_OFFSET
#define LZMA_REP_LEN_LOW(pos, sym) \
  (LZMA_PROB_REP_LEN_LOW_OFFSET + (pos) * LZMA_LEN_LOW_SYMBOLS + (sym))
#define LZMA_REP_LEN_MID(pos, sym) \
  (LZMA_PROB_REP_LEN_MID_OFFSET + (pos) * LZMA_LEN_MID_SYMBOLS + (sym))
#define LZMA_REP_LEN_HIGH(sym) \
  (LZMA_PROB_REP_LEN_HIGH_OFFSET + (sym))
#define LZMA_LITERAL(code, size) \
  (LZMA_PROB_LITERAL_OFFSET + (code) * LZMA_LITERAL_CODER_SIZE + (size))

/* Read an LZMA varint from BUF, reading and updating *POFFSET,
   setting *VAL.  Returns 0 on error, 1 on success.  */

static int
elf_lzma_varint (const unsigned char *compressed, size_t compressed_size,
		 size_t *poffset, uint64_t *val)
{
  size_t off;
  int i;
  uint64_t v;
  unsigned char b;

  off = *poffset;
  i = 0;
  v = 0;
  while (1)
    {
      if (unlikely (off >= compressed_size))
	{
	  elf_uncompress_failed ();
	  return 0;
	}
      b = compressed[off];
      v |= (b & 0x7f) << (i * 7);
      ++off;
      if ((b & 0x80) == 0)
	{
	  *poffset = off;
	  *val = v;
	  return 1;
	}
      ++i;
      if (unlikely (i >= 9))
	{
	  elf_uncompress_failed ();
	  return 0;
	}
    }
}

/* Normalize the LZMA range decoder, pulling in an extra input byte if
   needed.  */

static void
elf_lzma_range_normalize (const unsigned char *compressed,
			  size_t compressed_size, size_t *poffset,
			  uint32_t *prange, uint32_t *pcode)
{
  if (*prange < (1U << 24))
    {
      if (unlikely (*poffset >= compressed_size))
	{
	  /* We assume this will be caught elsewhere.  */
	  elf_uncompress_failed ();
	  return;
	}
      *prange <<= 8;
      *pcode <<= 8;
      *pcode += compressed[*poffset];
      ++*poffset;
    }
}

/* Read and return a single bit from the LZMA stream, reading and
   updating *PROB.  Each bit comes from the range coder.  */

static int
elf_lzma_bit (const unsigned char *compressed, size_t compressed_size,
	      uint16_t *prob, size_t *poffset, uint32_t *prange,
	      uint32_t *pcode)
{
  uint32_t bound;

  elf_lzma_range_normalize (compressed, compressed_size, poffset,
			    prange, pcode);
  bound = (*prange >> 11) * (uint32_t) *prob;
  if (*pcode < bound)
    {
      *prange = bound;
      *prob += ((1U << 11) - *prob) >> 5;
      return 0;
    }
  else
    {
      *prange -= bound;
      *pcode -= bound;
      *prob -= *prob >> 5;
      return 1;
    }
}

/* Read an integer of size BITS from the LZMA stream, most significant
   bit first.  The bits are predicted using PROBS.  */

static uint32_t
elf_lzma_integer (const unsigned char *compressed, size_t compressed_size,
		  uint16_t *probs, uint32_t bits, size_t *poffset,
		  uint32_t *prange, uint32_t *pcode)
{
  uint32_t sym;
  uint32_t i;

  sym = 1;
  for (i = 0; i < bits; i++)
    {
      int bit;

      bit = elf_lzma_bit (compressed, compressed_size, probs + sym, poffset,
			  prange, pcode);
      sym <<= 1;
      sym += bit;
    }
  return sym - (1 << bits);
}

/* Read an integer of size BITS from the LZMA stream, least
   significant bit first.  The bits are predicted using PROBS.  */

static uint32_t
elf_lzma_reverse_integer (const unsigned char *compressed,
			  size_t compressed_size, uint16_t *probs,
			  uint32_t bits, size_t *poffset, uint32_t *prange,
			  uint32_t *pcode)
{
  uint32_t sym;
  uint32_t val;
  uint32_t i;

  sym = 1;
  val = 0;
  for (i = 0; i < bits; i++)
    {
      int bit;

      bit = elf_lzma_bit (compressed, compressed_size, probs + sym, poffset,
			  prange, pcode);
      sym <<= 1;
      sym += bit;
      val += bit << i;
    }
  return val;
}

/* Read a length from the LZMA stream.  IS_REP picks either LZMA_MATCH
   or LZMA_REP probabilities.  */

static uint32_t
elf_lzma_len (const unsigned char *compressed, size_t compressed_size,
	      uint16_t *probs, int is_rep, unsigned int pos_state,
	      size_t *poffset, uint32_t *prange, uint32_t *pcode)
{
  uint16_t *probs_choice;
  uint16_t *probs_sym;
  uint32_t bits;
  uint32_t len;

  probs_choice = probs + (is_rep
			  ? LZMA_REP_LEN_CHOICE
			  : LZMA_MATCH_LEN_CHOICE);
  if (elf_lzma_bit (compressed, compressed_size, probs_choice, poffset,
		    prange, pcode))
    {
      probs_choice = probs + (is_rep
			      ? LZMA_REP_LEN_CHOICE2
			      : LZMA_MATCH_LEN_CHOICE2);
      if (elf_lzma_bit (compressed, compressed_size, probs_choice,
			poffset, prange, pcode))
	{
	  probs_sym = probs + (is_rep
			       ? LZMA_REP_LEN_HIGH (0)
			       : LZMA_MATCH_LEN_HIGH (0));
	  bits = 8;
	  len = 2 + 8 + 8;
	}
      else
	{
	  probs_sym = probs + (is_rep
			       ? LZMA_REP_LEN_MID (pos_state, 0)
			       : LZMA_MATCH_LEN_MID (pos_state, 0));
	  bits = 3;
	  len = 2 + 8;
	}
    }
  else
    {
      probs_sym = probs + (is_rep
			   ? LZMA_REP_LEN_LOW (pos_state, 0)
			   : LZMA_MATCH_LEN_LOW (pos_state, 0));
      bits = 3;
      len = 2;
    }

  len += elf_lzma_integer (compressed, compressed_size, probs_sym, bits,
			   poffset, prange, pcode);
  return len;
}

/* Uncompress one LZMA block from a minidebug file.  The compressed
   data is at COMPRESSED + *POFFSET.  Update *POFFSET.  Store the data
   into the memory at UNCOMPRESSED, size UNCOMPRESSED_SIZE.  CHECK is
   the stream flag from the xz header.  Return 1 on successful
   decompression.  */

static int
elf_uncompress_lzma_block (const unsigned char *compressed,
			   size_t compressed_size, unsigned char check,
			   uint16_t *probs, unsigned char *uncompressed,
			   size_t uncompressed_size, size_t *poffset)
{
  size_t off;
  size_t block_header_offset;
  size_t block_header_size;
  unsigned char block_flags;
  uint64_t header_compressed_size;
  uint64_t header_uncompressed_size;
  unsigned char lzma2_properties;
  uint32_t computed_crc;
  uint32_t stream_crc;
  size_t uncompressed_offset;
  size_t dict_start_offset;
  unsigned int lc;
  unsigned int lp;
  unsigned int pb;
  uint32_t range;
  uint32_t code;
  uint32_t lstate;
  uint32_t dist[4];

  off = *poffset;
  block_header_offset = off;

  /* Block header size is a single byte.  */
  if (unlikely (off >= compressed_size))
    {
      elf_uncompress_failed ();
      return 0;
    }
  block_header_size = (compressed[off] + 1) * 4;
  if (unlikely (off + block_header_size > compressed_size))
    {
      elf_uncompress_failed ();
      return 0;
    }

  /* Block flags.  */
  block_flags = compressed[off + 1];
  if (unlikely ((block_flags & 0x3c) != 0))
    {
      elf_uncompress_failed ();
      return 0;
    }

  off += 2;

  /* Optional compressed size.  */
  header_compressed_size = 0;
  if ((block_flags & 0x40) != 0)
    {
      *poffset = off;
      if (!elf_lzma_varint (compressed, compressed_size, poffset,
			    &header_compressed_size))
	return 0;
      off = *poffset;
    }

  /* Optional uncompressed size.  */
  header_uncompressed_size = 0;
  if ((block_flags & 0x80) != 0)
    {
      *poffset = off;
      if (!elf_lzma_varint (compressed, compressed_size, poffset,
			    &header_uncompressed_size))
	return 0;
      off = *poffset;
    }

  /* The recipe for creating a minidebug file is to run the xz program
     with no arguments, so we expect exactly one filter: lzma2.  */

  if (unlikely ((block_flags & 0x3) != 0))
    {
      elf_uncompress_failed ();
      return 0;
    }

  if (unlikely (off + 2 >= block_header_offset + block_header_size))
    {
      elf_uncompress_failed ();
      return 0;
    }

  /* The filter ID for LZMA2 is 0x21.  */
  if (unlikely (compressed[off] != 0x21))
    {
      elf_uncompress_failed ();
      return 0;
    }
  ++off;

  /* The size of the filter properties for LZMA2 is 1.  */
  if (unlikely (compressed[off] != 1))
    {
      elf_uncompress_failed ();
      return 0;
    }
  ++off;

  lzma2_properties = compressed[off];
  ++off;

  if (unlikely (lzma2_properties > 40))
    {
      elf_uncompress_failed ();
      return 0;
    }

  /* The properties describe the dictionary size, but we don't care
     what that is.  */

  /* Block header padding.  */
  if (unlikely (off + 4 > compressed_size))
    {
      elf_uncompress_failed ();
      return 0;
    }

  off = (off + 3) &~ (size_t) 3;

  if (unlikely (off + 4 > compressed_size))
    {
      elf_uncompress_failed ();
      return 0;
    }

  /* Block header CRC.  */
  computed_crc = elf_crc32 (0, compressed + block_header_offset,
			    block_header_size - 4);
  stream_crc = (compressed[off]
		| (compressed[off + 1] << 8)
		| (compressed[off + 2] << 16)
		| (compressed[off + 3] << 24));
  if (unlikely (computed_crc != stream_crc))
    {
      elf_uncompress_failed ();
      return 0;
    }
  off += 4;

  /* Read a sequence of LZMA2 packets.  */

  uncompressed_offset = 0;
  dict_start_offset = 0;
  lc = 0;
  lp = 0;
  pb = 0;
  lstate = 0;
  while (off < compressed_size)
    {
      unsigned char control;

      range = 0xffffffff;
      code = 0;

      control = compressed[off];
      ++off;
      if (unlikely (control == 0))
	{
	  /* End of packets.  */
	  break;
	}

      if (control == 1 || control >= 0xe0)
	{
	  /* Reset dictionary to empty.  */
	  dict_start_offset = uncompressed_offset;
	}

      if (control < 0x80)
	{
	  size_t chunk_size;

	  /* The only valid values here are 1 or 2.  A 1 means to
	     reset the dictionary (done above).  Then we see an
	     uncompressed chunk.  */

	  if (unlikely (control > 2))
	    {
	      elf_uncompress_failed ();
	      return 0;
	    }

	  /* An uncompressed chunk is a two byte size followed by
	     data.  */

	  if (unlikely (off + 2 > compressed_size))
	    {
	      elf_uncompress_failed ();
	      return 0;
	    }

	  chunk_size = compressed[off] << 8;
	  chunk_size += compressed[off + 1];
	  ++chunk_size;

	  off += 2;

	  if (unlikely (off + chunk_size > compressed_size))
	    {
	      elf_uncompress_failed ();
	      return 0;
	    }
	  if (unlikely (uncompressed_offset + chunk_size > uncompressed_size))
	    {
	      elf_uncompress_failed ();
	      return 0;
	    }

	  memcpy (uncompressed + uncompressed_offset, compressed + off,
		  chunk_size);
	  uncompressed_offset += chunk_size;
	  off += chunk_size;
	}
      else
	{
	  size_t uncompressed_chunk_start;
	  size_t uncompressed_chunk_size;
	  size_t compressed_chunk_size;
	  size_t limit;

	  /* An LZMA chunk.  This starts with an uncompressed size and
	     a compressed size.  */

	  if (unlikely (off + 4 >= compressed_size))
	    {
	      elf_uncompress_failed ();
	      return 0;
	    }

	  uncompressed_chunk_start = uncompressed_offset;

	  uncompressed_chunk_size = (control & 0x1f) << 16;
	  uncompressed_chunk_size += compressed[off] << 8;
	  uncompressed_chunk_size += compressed[off + 1];
	  ++uncompressed_chunk_size;

	  compressed_chunk_size = compressed[off + 2] << 8;
	  compressed_chunk_size += compressed[off + 3];
	  ++compressed_chunk_size;

	  off += 4;

	  /* Bit 7 (0x80) is set.
	     Bits 6 and 5 (0x40 and 0x20) are as follows:
	     0: don't reset anything
	     1: reset state
	     2: reset state, read properties
	     3: reset state, read properties, reset dictionary (done above) */

	  if (control >= 0xc0)
	    {
	      unsigned char props;

	      /* Bit 6 is set, read properties.  */

	      if (unlikely (off >= compressed_size))
		{
		  elf_uncompress_failed ();
		  return 0;
		}
	      props = compressed[off];
	      ++off;
	      if (unlikely (props > (4 * 5 + 4) * 9 + 8))
		{
		  elf_uncompress_failed ();
		  return 0;
		}
	      pb = 0;
	      while (props >= 9 * 5)
		{
		  props -= 9 * 5;
		  ++pb;
		}
	      lp = 0;
	      while (props > 9)
		{
		  props -= 9;
		  ++lp;
		}
	      lc = props;
	      if (unlikely (lc + lp > 4))
		{
		  elf_uncompress_failed ();
		  return 0;
		}
	    }

	  if (control >= 0xa0)
	    {
	      size_t i;

	      /* Bit 5 or 6 is set, reset LZMA state.  */

	      lstate = 0;
	      memset (&dist, 0, sizeof dist);
	      for (i = 0; i < LZMA_PROB_TOTAL_COUNT; i++)
		probs[i] = 1 << 10;
	      range = 0xffffffff;
	      code = 0;
	    }

	  /* Read the range code.  */

	  if (unlikely (off + 5 > compressed_size))
	    {
	      elf_uncompress_failed ();
	      return 0;
	    }

	  /* The byte at compressed[off] is ignored for some
	     reason.  */

	  code = ((compressed[off + 1] << 24)
		  + (compressed[off + 2] << 16)
		  + (compressed[off + 3] << 8)
		  + compressed[off + 4]);
	  off += 5;

	  /* This is the main LZMA decode loop.  */

	  limit = off + compressed_chunk_size;
	  *poffset = off;
	  while (*poffset < limit)
	    {
	      unsigned int pos_state;

	      if (unlikely (uncompressed_offset
			    == (uncompressed_chunk_start
				+ uncompressed_chunk_size)))
		{
		  /* We've decompressed all the expected bytes.  */
		  break;
		}

	      pos_state = ((uncompressed_offset - dict_start_offset)
			   & ((1 << pb) - 1));

	      if (elf_lzma_bit (compressed, compressed_size,
				probs + LZMA_IS_MATCH (lstate, pos_state),
				poffset, &range, &code))
		{
		  uint32_t len;

		  if (elf_lzma_bit (compressed, compressed_size,
				    probs + LZMA_IS_REP (lstate),
				    poffset, &range, &code))
		    {
		      int short_rep;
		      uint32_t next_dist;

		      /* Repeated match.  */

		      short_rep = 0;
		      if (elf_lzma_bit (compressed, compressed_size,
					probs + LZMA_IS_REP0 (lstate),
					poffset, &range, &code))
			{
			  if (elf_lzma_bit (compressed, compressed_size,
					    probs + LZMA_IS_REP1 (lstate),
					    poffset, &range, &code))
			    {
			      if (elf_lzma_bit (compressed, compressed_size,
						probs + LZMA_IS_REP2 (lstate),
						poffset, &range, &code))
				{
				  next_dist = dist[3];
				  dist[3] = dist[2];
				}
			      else
				{
				  next_dist = dist[2];
				}
			      dist[2] = dist[1];
			    }
			  else
			    {
			      next_dist = dist[1];
			    }

			  dist[1] = dist[0];
			  dist[0] = next_dist;
			}
		      else
			{
			  if (!elf_lzma_bit (compressed, compressed_size,
					    (probs
					     + LZMA_IS_REP0_LONG (lstate,
								  pos_state)),
					    poffset, &range, &code))
			    short_rep = 1;
			}

		      if (lstate < 7)
			lstate = short_rep ? 9 : 8;
		      else
			lstate = 11;

		      if (short_rep)
			len = 1;
		      else
			len = elf_lzma_len (compressed, compressed_size,
					    probs, 1, pos_state, poffset,
					    &range, &code);
		    }
		  else
		    {
		      uint32_t dist_state;
		      uint32_t dist_slot;
		      uint16_t *probs_dist;

		      /* Match.  */

		      if (lstate < 7)
			lstate = 7;
		      else
			lstate = 10;
		      dist[3] = dist[2];
		      dist[2] = dist[1];
		      dist[1] = dist[0];
		      len = elf_lzma_len (compressed, compressed_size,
					  probs, 0, pos_state, poffset,
					  &range, &code);

		      if (len < 4 + 2)
			dist_state = len - 2;
		      else
			dist_state = 3;
		      probs_dist = probs + LZMA_DIST_SLOT (dist_state, 0);
		      dist_slot = elf_lzma_integer (compressed,
						    compressed_size,
						    probs_dist, 6,
						    poffset, &range,
						    &code);
		      if (dist_slot < LZMA_DIST_MODEL_START)
			dist[0] = dist_slot;
		      else
			{
			  uint32_t limit;

			  limit = (dist_slot >> 1) - 1;
			  dist[0] = 2 + (dist_slot & 1);
			  if (dist_slot < LZMA_DIST_MODEL_END)
			    {
			      dist[0] <<= limit;
			      probs_dist = (probs
					    + LZMA_DIST_SPECIAL(dist[0]
								- dist_slot
								- 1));
			      dist[0] +=
				elf_lzma_reverse_integer (compressed,
							  compressed_size,
							  probs_dist,
							  limit, poffset,
							  &range, &code);
			    }
			  else
			    {
			      uint32_t dist0;
			      uint32_t i;

			      dist0 = dist[0];
			      for (i = 0; i < limit - 4; i++)
				{
				  uint32_t mask;

				  elf_lzma_range_normalize (compressed,
							    compressed_size,
							    poffset,
							    &range, &code);
				  range >>= 1;
				  code -= range;
				  mask = -(code >> 31);
				  code += range & mask;
				  dist0 <<= 1;
				  dist0 += mask + 1;
				}
			      dist0 <<= 4;
			      probs_dist = probs + LZMA_DIST_ALIGN (0);
			      dist0 +=
				elf_lzma_reverse_integer (compressed,
							  compressed_size,
							  probs_dist, 4,
							  poffset,
							  &range, &code);
			      dist[0] = dist0;
			    }
			}
		    }

		  if (unlikely (uncompressed_offset
				- dict_start_offset < dist[0] + 1))
		    {
		      elf_uncompress_failed ();
		      return 0;
		    }
		  if (unlikely (uncompressed_offset + len > uncompressed_size))
		    {
		      elf_uncompress_failed ();
		      return 0;
		    }

		  if (dist[0] == 0)
		    {
		      /* A common case, meaning repeat the last
			 character LEN times.  */
		      memset (uncompressed + uncompressed_offset,
			      uncompressed[uncompressed_offset - 1],
			      len);
		      uncompressed_offset += len;
		    }
		  else if (dist[0] + 1 >= len)
		    {
		      memcpy (uncompressed + uncompressed_offset,
			      uncompressed + uncompressed_offset - dist[0] - 1,
			      len);
		      uncompressed_offset += len;
		    }
		  else
		    {
		      while (len > 0)
			{
			  uint32_t copy;

			  copy = len < dist[0] + 1 ? len : dist[0] + 1;
			  memcpy (uncompressed + uncompressed_offset,
				  (uncompressed + uncompressed_offset
				   - dist[0] - 1),
				  copy);
			  len -= copy;
			  uncompressed_offset += copy;
			}
		    }
		}
	      else
		{
		  unsigned char prev;
		  unsigned char low;
		  size_t high;
		  uint16_t *lit_probs;
		  unsigned int sym;

		  /* Literal value.  */

		  if (uncompressed_offset > 0)
		    prev = uncompressed[uncompressed_offset - 1];
		  else
		    prev = 0;
		  low = prev >> (8 - lc);
		  high = (((uncompressed_offset - dict_start_offset)
			   & ((1 << lp) - 1))
			  << lc);
		  lit_probs = probs + LZMA_LITERAL (low + high, 0);
		  if (lstate < 7)
		    sym = elf_lzma_integer (compressed, compressed_size,
					    lit_probs, 8, poffset, &range,
					    &code);
		  else
		    {
		      unsigned int match;
		      unsigned int bit;
		      unsigned int match_bit;
		      unsigned int idx;

		      sym = 1;
		      if (uncompressed_offset >= dist[0] + 1)
			match = uncompressed[uncompressed_offset - dist[0] - 1];
		      else
			match = 0;
		      match <<= 1;
		      bit = 0x100;
		      do
			{
			  match_bit = match & bit;
			  match <<= 1;
			  idx = bit + match_bit + sym;
			  sym <<= 1;
			  if (elf_lzma_bit (compressed, compressed_size,
					    lit_probs + idx, poffset,
					    &range, &code))
			    {
			      ++sym;
			      bit &= match_bit;
			    }
			  else
			    {
			      bit &= ~ match_bit;
			    }
			}
		      while (sym < 0x100);
		    }

		  if (unlikely (uncompressed_offset >= uncompressed_size))
		    {
		      elf_uncompress_failed ();
		      return 0;
		    }

		  uncompressed[uncompressed_offset] = (unsigned char) sym;
		  ++uncompressed_offset;
		  if (lstate <= 3)
		    lstate = 0;
		  else if (lstate <= 9)
		    lstate -= 3;
		  else
		    lstate -= 6;
		}
	    }

	  elf_lzma_range_normalize (compressed, compressed_size, poffset,
				    &range, &code);

	  off = *poffset;
	}
    }

  /* We have reached the end of the block.  Pad to four byte
     boundary.  */
  off = (off + 3) &~ (size_t) 3;
  if (unlikely (off > compressed_size))
    {
      elf_uncompress_failed ();
      return 0;
    }

  switch (check)
    {
    case 0:
      /* No check.  */
      break;

    case 1:
      /* CRC32 */
      if (unlikely (off + 4 > compressed_size))
	{
	  elf_uncompress_failed ();
	  return 0;
	}
      computed_crc = elf_crc32 (0, uncompressed, uncompressed_offset);
      stream_crc = (compressed[off]
		    | (compressed[off + 1] << 8)
		    | (compressed[off + 2] << 16)
		    | (compressed[off + 3] << 24));
      if (computed_crc != stream_crc)
	{
	  elf_uncompress_failed ();
	  return 0;
	}
      off += 4;
      break;

    case 4:
      /* CRC64.  We don't bother computing a CRC64 checksum.  */
      if (unlikely (off + 8 > compressed_size))
	{
	  elf_uncompress_failed ();
	  return 0;
	}
      off += 8;
      break;

    case 10:
      /* SHA.  We don't bother computing a SHA checksum.  */
      if (unlikely (off + 32 > compressed_size))
	{
	  elf_uncompress_failed ();
	  return 0;
	}
      off += 32;
      break;

    default:
      elf_uncompress_failed ();
      return 0;
    }

  *poffset = off;

  return 1;
}

/* Uncompress LZMA data found in a minidebug file.  The minidebug
   format is described at
   https://sourceware.org/gdb/current/onlinedocs/gdb/MiniDebugInfo.html.
   Returns 0 on error, 1 on successful decompression.  For this
   function we return 0 on failure to decompress, as the calling code
   will carry on in that case.  */

static int
elf_uncompress_lzma (struct backtrace_state *state,
		     const unsigned char *compressed, size_t compressed_size,
		     backtrace_error_callback error_callback, void *data,
		     unsigned char **uncompressed, size_t *uncompressed_size)
{
  size_t header_size;
  size_t footer_size;
  unsigned char check;
  uint32_t computed_crc;
  uint32_t stream_crc;
  size_t offset;
  size_t index_size;
  size_t footer_offset;
  size_t index_offset;
  uint64_t index_compressed_size;
  uint64_t index_uncompressed_size;
  unsigned char *mem;
  uint16_t *probs;
  size_t compressed_block_size;

  /* The format starts with a stream header and ends with a stream
     footer.  */
  header_size = 12;
  footer_size = 12;
  if (unlikely (compressed_size < header_size + footer_size))
    {
      elf_uncompress_failed ();
      return 0;
    }

  /* The stream header starts with a magic string.  */
  if (unlikely (memcmp (compressed, "\375" "7zXZ\0", 6) != 0))
    {
      elf_uncompress_failed ();
      return 0;
    }

  /* Next come stream flags.  The first byte is zero, the second byte
     is the check.  */
  if (unlikely (compressed[6] != 0))
    {
      elf_uncompress_failed ();
      return 0;
    }
  check = compressed[7];
  if (unlikely ((check & 0xf8) != 0))
    {
      elf_uncompress_failed ();
      return 0;
    }

  /* Next comes a CRC of the stream flags.  */
  computed_crc = elf_crc32 (0, compressed + 6, 2);
  stream_crc = (compressed[8]
		| (compressed[9] << 8)
		| (compressed[10] << 16)
		| (compressed[11] << 24));
  if (unlikely (computed_crc != stream_crc))
    {
      elf_uncompress_failed ();
      return 0;
    }

  /* Now that we've parsed the header, parse the footer, so that we
     can get the uncompressed size.  */

  /* The footer ends with two magic bytes.  */

  offset = compressed_size;
  if (unlikely (memcmp (compressed + offset - 2, "YZ", 2) != 0))
    {
      elf_uncompress_failed ();
      return 0;
    }
  offset -= 2;

  /* Before that are the stream flags, which should be the same as the
     flags in the header.  */
  if (unlikely (compressed[offset - 2] != 0
		|| compressed[offset - 1] != check))
    {
      elf_uncompress_failed ();
      return 0;
    }
  offset -= 2;

  /* Before that is the size of the index field, which precedes the
     footer.  */
  index_size = (compressed[offset - 4]
		| (compressed[offset - 3] << 8)
		| (compressed[offset - 2] << 16)
		| (compressed[offset - 1] << 24));
  index_size = (index_size + 1) * 4;
  offset -= 4;

  /* Before that is a footer CRC.  */
  computed_crc = elf_crc32 (0, compressed + offset, 6);
  stream_crc = (compressed[offset - 4]
		| (compressed[offset - 3] << 8)
		| (compressed[offset - 2] << 16)
		| (compressed[offset - 1] << 24));
  if (unlikely (computed_crc != stream_crc))
    {
      elf_uncompress_failed ();
      return 0;
    }
  offset -= 4;

  /* The index comes just before the footer.  */
  if (unlikely (offset < index_size + header_size))
    {
      elf_uncompress_failed ();
      return 0;
    }

  footer_offset = offset;
  offset -= index_size;
  index_offset = offset;

  /* The index starts with a zero byte.  */
  if (unlikely (compressed[offset] != 0))
    {
      elf_uncompress_failed ();
      return 0;
    }
  ++offset;

  /* Next is the number of blocks.  We expect zero blocks for an empty
     stream, and otherwise a single block.  */
  if (unlikely (compressed[offset] == 0))
    {
      *uncompressed = NULL;
      *uncompressed_size = 0;
      return 1;
    }
  if (unlikely (compressed[offset] != 1))
    {
      elf_uncompress_failed ();
      return 0;
    }
  ++offset;

  /* Next is the compressed size and the uncompressed size.  */
  if (!elf_lzma_varint (compressed, compressed_size, &offset,
			&index_compressed_size))
    return 0;
  if (!elf_lzma_varint (compressed, compressed_size, &offset,
			&index_uncompressed_size))
    return 0;

  /* Pad to a four byte boundary.  */
  offset = (offset + 3) &~ (size_t) 3;

  /* Next is a CRC of the index.  */
  computed_crc = elf_crc32 (0, compressed + index_offset,
			    offset - index_offset);
  stream_crc = (compressed[offset]
		| (compressed[offset + 1] << 8)
		| (compressed[offset + 2] << 16)
		| (compressed[offset + 3] << 24));
  if (unlikely (computed_crc != stream_crc))
    {
      elf_uncompress_failed ();
      return 0;
    }
  offset += 4;

  /* We should now be back at the footer.  */
  if (unlikely (offset != footer_offset))
    {
      elf_uncompress_failed ();
      return 0;
    }

  /* Allocate space to hold the uncompressed data.  If we succeed in
     uncompressing the LZMA data, we never free this memory.  */
  mem = (unsigned char *) backtrace_alloc (state, index_uncompressed_size,
					   error_callback, data);
  if (unlikely (mem == NULL))
    return 0;
  *uncompressed = mem;
  *uncompressed_size = index_uncompressed_size;

  /* Allocate space for probabilities.  */
  probs = ((uint16_t *)
	   backtrace_alloc (state,
			    LZMA_PROB_TOTAL_COUNT * sizeof (uint16_t),
			    error_callback, data));
  if (unlikely (probs == NULL))
    {
      backtrace_free (state, mem, index_uncompressed_size, error_callback,
		      data);
      return 0;
    }

  /* Uncompress the block, which follows the header.  */
  offset = 12;
  if (!elf_uncompress_lzma_block (compressed, compressed_size, check, probs,
				  mem, index_uncompressed_size, &offset))
    {
      backtrace_free (state, mem, index_uncompressed_size, error_callback,
		      data);
      return 0;
    }

  compressed_block_size = offset - 12;
  if (unlikely (compressed_block_size
		!= ((index_compressed_size + 3) &~ (size_t) 3)))
    {
      elf_uncompress_failed ();
      backtrace_free (state, mem, index_uncompressed_size, error_callback,
		      data);
      return 0;
    }

  offset = (offset + 3) &~ (size_t) 3;
  if (unlikely (offset != index_offset))
    {
      elf_uncompress_failed ();
      backtrace_free (state, mem, index_uncompressed_size, error_callback,
		      data);
      return 0;
    }

  return 1;
}

/* This function is a hook for testing the LZMA support.  It is only
   used by tests.  */

int
backtrace_uncompress_lzma (struct backtrace_state *state,
			   const unsigned char *compressed,
			   size_t compressed_size,
			   backtrace_error_callback error_callback,
			   void *data, unsigned char **uncompressed,
			   size_t *uncompressed_size)
{
  return elf_uncompress_lzma (state, compressed, compressed_size,
			      error_callback, data, uncompressed,
			      uncompressed_size);
}

/* Add the backtrace data for one ELF file.  Returns 1 on success,
   0 on failure (in both cases descriptor is closed) or -1 if exe
   is non-zero and the ELF file is ET_DYN, which tells the caller that
   elf_add will need to be called on the descriptor again after
   base_address is determined.  */

static int
elf_add (struct backtrace_state *state, const char *filename, int descriptor,
	 const unsigned char *memory, size_t memory_size,
	 uintptr_t base_address, backtrace_error_callback error_callback,
	 void *data, fileline *fileline_fn, int *found_sym, int *found_dwarf,
	 struct dwarf_data **fileline_entry, int exe, int debuginfo,
	 const char *with_buildid_data, uint32_t with_buildid_size)
{
  struct elf_view ehdr_view;
  b_elf_ehdr ehdr;
  off_t shoff;
  unsigned int shnum;
  unsigned int shstrndx;
  struct elf_view shdrs_view;
  int shdrs_view_valid;
  const b_elf_shdr *shdrs;
  const b_elf_shdr *shstrhdr;
  size_t shstr_size;
  off_t shstr_off;
  struct elf_view names_view;
  int names_view_valid;
  const char *names;
  unsigned int symtab_shndx;
  unsigned int dynsym_shndx;
  unsigned int i;
  struct debug_section_info sections[DEBUG_MAX];
  struct debug_section_info zsections[DEBUG_MAX];
  struct elf_view symtab_view;
  int symtab_view_valid;
  struct elf_view strtab_view;
  int strtab_view_valid;
  struct elf_view buildid_view;
  int buildid_view_valid;
  const char *buildid_data;
  uint32_t buildid_size;
  struct elf_view debuglink_view;
  int debuglink_view_valid;
  const char *debuglink_name;
  uint32_t debuglink_crc;
  struct elf_view debugaltlink_view;
  int debugaltlink_view_valid;
  const char *debugaltlink_name;
  const char *debugaltlink_buildid_data;
  uint32_t debugaltlink_buildid_size;
  struct elf_view gnu_debugdata_view;
  int gnu_debugdata_view_valid;
  size_t gnu_debugdata_size;
  unsigned char *gnu_debugdata_uncompressed;
  size_t gnu_debugdata_uncompressed_size;
  off_t min_offset;
  off_t max_offset;
  off_t debug_size;
  struct elf_view debug_view;
  int debug_view_valid;
  unsigned int using_debug_view;
  uint16_t *zdebug_table;
  struct elf_view split_debug_view[DEBUG_MAX];
  unsigned char split_debug_view_valid[DEBUG_MAX];
  struct elf_ppc64_opd_data opd_data, *opd;
  struct dwarf_sections dwarf_sections;
  struct dwarf_data *fileline_altlink = NULL;

  if (!debuginfo)
    {
      *found_sym = 0;
      *found_dwarf = 0;
    }

  shdrs_view_valid = 0;
  names_view_valid = 0;
  symtab_view_valid = 0;
  strtab_view_valid = 0;
  buildid_view_valid = 0;
  buildid_data = NULL;
  buildid_size = 0;
  debuglink_view_valid = 0;
  debuglink_name = NULL;
  debuglink_crc = 0;
  debugaltlink_view_valid = 0;
  debugaltlink_name = NULL;
  debugaltlink_buildid_data = NULL;
  debugaltlink_buildid_size = 0;
  gnu_debugdata_view_valid = 0;
  gnu_debugdata_size = 0;
  debug_view_valid = 0;
  memset (&split_debug_view_valid[0], 0, sizeof split_debug_view_valid);
  opd = NULL;

  if (!elf_get_view (state, descriptor, memory, memory_size, 0, sizeof ehdr,
		     error_callback, data, &ehdr_view))
    goto fail;

  memcpy (&ehdr, ehdr_view.view.data, sizeof ehdr);

  elf_release_view (state, &ehdr_view, error_callback, data);

  if (ehdr.e_ident[EI_MAG0] != ELFMAG0
      || ehdr.e_ident[EI_MAG1] != ELFMAG1
      || ehdr.e_ident[EI_MAG2] != ELFMAG2
      || ehdr.e_ident[EI_MAG3] != ELFMAG3)
    {
      error_callback (data, "executable file is not ELF", 0);
      goto fail;
    }
  if (ehdr.e_ident[EI_VERSION] != EV_CURRENT)
    {
      error_callback (data, "executable file is unrecognized ELF version", 0);
      goto fail;
    }

#if BACKTRACE_ELF_SIZE == 32
#define BACKTRACE_ELFCLASS ELFCLASS32
#else
#define BACKTRACE_ELFCLASS ELFCLASS64
#endif

  if (ehdr.e_ident[EI_CLASS] != BACKTRACE_ELFCLASS)
    {
      error_callback (data, "executable file is unexpected ELF class", 0);
      goto fail;
    }

  if (ehdr.e_ident[EI_DATA] != ELFDATA2LSB
      && ehdr.e_ident[EI_DATA] != ELFDATA2MSB)
    {
      error_callback (data, "executable file has unknown endianness", 0);
      goto fail;
    }

  /* If the executable is ET_DYN, it is either a PIE, or we are running
     directly a shared library with .interp.  We need to wait for
     dl_iterate_phdr in that case to determine the actual base_address.  */
  if (exe && ehdr.e_type == ET_DYN)
    return -1;

  shoff = ehdr.e_shoff;
  shnum = ehdr.e_shnum;
  shstrndx = ehdr.e_shstrndx;

  if ((shnum == 0 || shstrndx == SHN_XINDEX)
      && shoff != 0)
    {
      struct elf_view shdr_view;
      const b_elf_shdr *shdr;

      if (!elf_get_view (state, descriptor, memory, memory_size, shoff,
			 sizeof shdr, error_callback, data, &shdr_view))
	goto fail;

      shdr = (const b_elf_shdr *) shdr_view.view.data;

      if (shnum == 0)
	shnum = shdr->sh_size;

      if (shstrndx == SHN_XINDEX)
	{
	  shstrndx = shdr->sh_link;

	  /* Versions of the GNU binutils between 2.12 and 2.18 did
	     not handle objects with more than SHN_LORESERVE sections
	     correctly.  All large section indexes were offset by
	     0x100.  There is more information at
	     http://sourceware.org/bugzilla/show_bug.cgi?id-5900 .
	     Fortunately these object files are easy to detect, as the
	     GNU binutils always put the section header string table
	     near the end of the list of sections.  Thus if the
	     section header string table index is larger than the
	     number of sections, then we know we have to subtract
	     0x100 to get the real section index.  */
	  if (shstrndx >= shnum && shstrndx >= SHN_LORESERVE + 0x100)
	    shstrndx -= 0x100;
	}

      elf_release_view (state, &shdr_view, error_callback, data);
    }

  if (shnum == 0 || shstrndx == 0)
    goto fail;

  /* To translate PC to file/line when using DWARF, we need to find
     the .debug_info and .debug_line sections.  */

  /* Read the section headers, skipping the first one.  */

  if (!elf_get_view (state, descriptor, memory, memory_size,
		     shoff + sizeof (b_elf_shdr),
		     (shnum - 1) * sizeof (b_elf_shdr),
		     error_callback, data, &shdrs_view))
    goto fail;
  shdrs_view_valid = 1;
  shdrs = (const b_elf_shdr *) shdrs_view.view.data;

  /* Read the section names.  */

  shstrhdr = &shdrs[shstrndx - 1];
  shstr_size = shstrhdr->sh_size;
  shstr_off = shstrhdr->sh_offset;

  if (!elf_get_view (state, descriptor, memory, memory_size, shstr_off,
		     shstrhdr->sh_size, error_callback, data, &names_view))
    goto fail;
  names_view_valid = 1;
  names = (const char *) names_view.view.data;

  symtab_shndx = 0;
  dynsym_shndx = 0;

  memset (sections, 0, sizeof sections);
  memset (zsections, 0, sizeof zsections);

  /* Look for the symbol table.  */
  for (i = 1; i < shnum; ++i)
    {
      const b_elf_shdr *shdr;
      unsigned int sh_name;
      const char *name;
      int j;

      shdr = &shdrs[i - 1];

      if (shdr->sh_type == SHT_SYMTAB)
	symtab_shndx = i;
      else if (shdr->sh_type == SHT_DYNSYM)
	dynsym_shndx = i;

      sh_name = shdr->sh_name;
      if (sh_name >= shstr_size)
	{
	  error_callback (data, "ELF section name out of range", 0);
	  goto fail;
	}

      name = names + sh_name;

      for (j = 0; j < (int) DEBUG_MAX; ++j)
	{
	  if (strcmp (name, dwarf_section_names[j]) == 0)
	    {
	      sections[j].offset = shdr->sh_offset;
	      sections[j].size = shdr->sh_size;
	      sections[j].compressed = (shdr->sh_flags & SHF_COMPRESSED) != 0;
	      break;
	    }
	}

      if (name[0] == '.' && name[1] == 'z')
	{
	  for (j = 0; j < (int) DEBUG_MAX; ++j)
	    {
	      if (strcmp (name + 2, dwarf_section_names[j] + 1) == 0)
		{
		  zsections[j].offset = shdr->sh_offset;
		  zsections[j].size = shdr->sh_size;
		  break;
		}
	    }
	}

      /* Read the build ID if present.  This could check for any
	 SHT_NOTE section with the right note name and type, but gdb
	 looks for a specific section name.  */
      if ((!debuginfo || with_buildid_data != NULL)
	  && !buildid_view_valid
	  && strcmp (name, ".note.gnu.build-id") == 0)
	{
	  const b_elf_note *note;

	  if (!elf_get_view (state, descriptor, memory, memory_size,
			     shdr->sh_offset, shdr->sh_size, error_callback,
			     data, &buildid_view))
	    goto fail;

	  buildid_view_valid = 1;
	  note = (const b_elf_note *) buildid_view.view.data;
	  if (note->type == NT_GNU_BUILD_ID
	      && note->namesz == 4
	      && strncmp (note->name, "GNU", 4) == 0
	      && shdr->sh_size <= 12 + ((note->namesz + 3) & ~ 3) + note->descsz)
	    {
	      buildid_data = &note->name[0] + ((note->namesz + 3) & ~ 3);
	      buildid_size = note->descsz;
	    }

	  if (with_buildid_size != 0)
	    {
	      if (buildid_size != with_buildid_size)
		goto fail;

	      if (memcmp (buildid_data, with_buildid_data, buildid_size) != 0)
		goto fail;
	    }
	}

      /* Read the debuglink file if present.  */
      if (!debuginfo
	  && !debuglink_view_valid
	  && strcmp (name, ".gnu_debuglink") == 0)
	{
	  const char *debuglink_data;
	  size_t crc_offset;

	  if (!elf_get_view (state, descriptor, memory, memory_size,
			     shdr->sh_offset, shdr->sh_size, error_callback,
			     data, &debuglink_view))
	    goto fail;

	  debuglink_view_valid = 1;
	  debuglink_data = (const char *) debuglink_view.view.data;
	  crc_offset = strnlen (debuglink_data, shdr->sh_size);
	  crc_offset = (crc_offset + 3) & ~3;
	  if (crc_offset + 4 <= shdr->sh_size)
	    {
	      debuglink_name = debuglink_data;
	      debuglink_crc = *(const uint32_t*)(debuglink_data + crc_offset);
	    }
	}

      if (!debugaltlink_view_valid
	  && strcmp (name, ".gnu_debugaltlink") == 0)
	{
	  const char *debugaltlink_data;
	  size_t debugaltlink_name_len;

	  if (!elf_get_view (state, descriptor, memory, memory_size,
			     shdr->sh_offset, shdr->sh_size, error_callback,
			     data, &debugaltlink_view))
	    goto fail;

	  debugaltlink_view_valid = 1;
	  debugaltlink_data = (const char *) debugaltlink_view.view.data;
	  debugaltlink_name = debugaltlink_data;
	  debugaltlink_name_len = strnlen (debugaltlink_data, shdr->sh_size);
	  if (debugaltlink_name_len < shdr->sh_size)
	    {
	      /* Include terminating zero.  */
	      debugaltlink_name_len += 1;

	      debugaltlink_buildid_data
		= debugaltlink_data + debugaltlink_name_len;
	      debugaltlink_buildid_size = shdr->sh_size - debugaltlink_name_len;
	    }
	}

      if (!gnu_debugdata_view_valid
	  && strcmp (name, ".gnu_debugdata") == 0)
	{
	  if (!elf_get_view (state, descriptor, memory, memory_size,
			     shdr->sh_offset, shdr->sh_size, error_callback,
			     data, &gnu_debugdata_view))
	    goto fail;

	  gnu_debugdata_size = shdr->sh_size;
	  gnu_debugdata_view_valid = 1;
	}

      /* Read the .opd section on PowerPC64 ELFv1.  */
      if (ehdr.e_machine == EM_PPC64
	  && (ehdr.e_flags & EF_PPC64_ABI) < 2
	  && shdr->sh_type == SHT_PROGBITS
	  && strcmp (name, ".opd") == 0)
	{
	  if (!elf_get_view (state, descriptor, memory, memory_size,
			     shdr->sh_offset, shdr->sh_size, error_callback,
			     data, &opd_data.view))
	    goto fail;

	  opd = &opd_data;
	  opd->addr = shdr->sh_addr;
	  opd->data = (const char *) opd_data.view.view.data;
	  opd->size = shdr->sh_size;
	}
    }

  if (symtab_shndx == 0)
    symtab_shndx = dynsym_shndx;
  if (symtab_shndx != 0 && !debuginfo)
    {
      const b_elf_shdr *symtab_shdr;
      unsigned int strtab_shndx;
      const b_elf_shdr *strtab_shdr;
      struct elf_syminfo_data *sdata;

      symtab_shdr = &shdrs[symtab_shndx - 1];
      strtab_shndx = symtab_shdr->sh_link;
      if (strtab_shndx >= shnum)
	{
	  error_callback (data,
			  "ELF symbol table strtab link out of range", 0);
	  goto fail;
	}
      strtab_shdr = &shdrs[strtab_shndx - 1];

      if (!elf_get_view (state, descriptor, memory, memory_size,
			 symtab_shdr->sh_offset, symtab_shdr->sh_size,
			 error_callback, data, &symtab_view))
	goto fail;
      symtab_view_valid = 1;

      if (!elf_get_view (state, descriptor, memory, memory_size,
			 strtab_shdr->sh_offset, strtab_shdr->sh_size,
			 error_callback, data, &strtab_view))
	goto fail;
      strtab_view_valid = 1;

      sdata = ((struct elf_syminfo_data *)
	       backtrace_alloc (state, sizeof *sdata, error_callback, data));
      if (sdata == NULL)
	goto fail;

      if (!elf_initialize_syminfo (state, base_address,
				   (const unsigned char*)symtab_view.view.data, symtab_shdr->sh_size,
				   (const unsigned char*)strtab_view.view.data, strtab_shdr->sh_size,
				   error_callback, data, sdata, opd))
	{
	  backtrace_free (state, sdata, sizeof *sdata, error_callback, data);
	  goto fail;
	}

      /* We no longer need the symbol table, but we hold on to the
	 string table permanently.  */
      elf_release_view (state, &symtab_view, error_callback, data);
      symtab_view_valid = 0;
      strtab_view_valid = 0;

      *found_sym = 1;

      elf_add_syminfo_data (state, sdata);
    }

  elf_release_view (state, &shdrs_view, error_callback, data);
  shdrs_view_valid = 0;
  elf_release_view (state, &names_view, error_callback, data);
  names_view_valid = 0;

  /* If the debug info is in a separate file, read that one instead.  */

  if (buildid_data != NULL)
    {
      int d;

      d = elf_open_debugfile_by_buildid (state, buildid_data, buildid_size,
					 error_callback, data);
      if (d >= 0)
	{
	  int ret;

	  elf_release_view (state, &buildid_view, error_callback, data);
	  if (debuglink_view_valid)
	    elf_release_view (state, &debuglink_view, error_callback, data);
	  if (debugaltlink_view_valid)
	    elf_release_view (state, &debugaltlink_view, error_callback, data);
	  ret = elf_add (state, "", d, NULL, 0, base_address, error_callback,
			 data, fileline_fn, found_sym, found_dwarf, NULL, 0,
			 1, NULL, 0);
	  if (ret < 0)
	    backtrace_close (d, error_callback, data);
	  else if (descriptor >= 0)
	    backtrace_close (descriptor, error_callback, data);
	  return ret;
	}
    }

  if (buildid_view_valid)
    {
      elf_release_view (state, &buildid_view, error_callback, data);
      buildid_view_valid = 0;
    }

  if (opd)
    {
      elf_release_view (state, &opd->view, error_callback, data);
      opd = NULL;
    }

  if (debuglink_name != NULL)
    {
      int d;

      d = elf_open_debugfile_by_debuglink (state, filename, debuglink_name,
					   debuglink_crc, error_callback,
					   data);
      if (d >= 0)
	{
	  int ret;

	  elf_release_view (state, &debuglink_view, error_callback, data);
	  if (debugaltlink_view_valid)
	    elf_release_view (state, &debugaltlink_view, error_callback, data);
	  ret = elf_add (state, "", d, NULL, 0, base_address, error_callback,
			 data, fileline_fn, found_sym, found_dwarf, NULL, 0,
			 1, NULL, 0);
	  if (ret < 0)
	    backtrace_close (d, error_callback, data);
	  else if (descriptor >= 0)
	    backtrace_close(descriptor, error_callback, data);
	  return ret;
	}
    }

  if (debuglink_view_valid)
    {
      elf_release_view (state, &debuglink_view, error_callback, data);
      debuglink_view_valid = 0;
    }

  if (debugaltlink_name != NULL)
    {
      int d;

      d = elf_open_debugfile_by_debuglink (state, filename, debugaltlink_name,
					   0, error_callback, data);
      if (d >= 0)
	{
	  int ret;

	  ret = elf_add (state, filename, d, NULL, 0, base_address,
			 error_callback, data, fileline_fn, found_sym,
			 found_dwarf, &fileline_altlink, 0, 1,
			 debugaltlink_buildid_data, debugaltlink_buildid_size);
	  elf_release_view (state, &debugaltlink_view, error_callback, data);
	  debugaltlink_view_valid = 0;
	  if (ret < 0)
	    {
	      backtrace_close (d, error_callback, data);
	      return ret;
	    }
	}
    }

  if (debugaltlink_view_valid)
    {
      elf_release_view (state, &debugaltlink_view, error_callback, data);
      debugaltlink_view_valid = 0;
    }

  if (gnu_debugdata_view_valid)
    {
      int ret;

      ret = elf_uncompress_lzma (state,
				 ((const unsigned char *)
				  gnu_debugdata_view.view.data),
				 gnu_debugdata_size, error_callback, data,
				 &gnu_debugdata_uncompressed,
				 &gnu_debugdata_uncompressed_size);

      elf_release_view (state, &gnu_debugdata_view, error_callback, data);
      gnu_debugdata_view_valid = 0;

      if (ret)
	{
	  ret = elf_add (state, filename, -1, gnu_debugdata_uncompressed,
			 gnu_debugdata_uncompressed_size, base_address,
			 error_callback, data, fileline_fn, found_sym,
			 found_dwarf, NULL, 0, 0, NULL, 0);
	  if (ret >= 0 && descriptor >= 0)
	    backtrace_close(descriptor, error_callback, data);
	  return ret;
	}
    }

  /* Read all the debug sections in a single view, since they are
     probably adjacent in the file.  If any of sections are
     uncompressed, we never release this view.  */

  min_offset = 0;
  max_offset = 0;
  debug_size = 0;
  for (i = 0; i < (int) DEBUG_MAX; ++i)
    {
      off_t end;

      if (sections[i].size != 0)
	{
	  if (min_offset == 0 || sections[i].offset < min_offset)
	    min_offset = sections[i].offset;
	  end = sections[i].offset + sections[i].size;
	  if (end > max_offset)
	    max_offset = end;
	  debug_size += sections[i].size;
	}
      if (zsections[i].size != 0)
	{
	  if (min_offset == 0 || zsections[i].offset < min_offset)
	    min_offset = zsections[i].offset;
	  end = zsections[i].offset + zsections[i].size;
	  if (end > max_offset)
	    max_offset = end;
	  debug_size += zsections[i].size;
	}
    }
  if (min_offset == 0 || max_offset == 0)
    {
      if (descriptor >= 0)
	{
	  if (!backtrace_close (descriptor, error_callback, data))
	    goto fail;
	}
      return 1;
    }

  /* If the total debug section size is large, assume that there are
     gaps between the sections, and read them individually.  */

  if (max_offset - min_offset < 0x20000000
      || max_offset - min_offset < debug_size + 0x10000)
    {
      if (!elf_get_view (state, descriptor, memory, memory_size, min_offset,
			 max_offset - min_offset, error_callback, data,
			 &debug_view))
	goto fail;
      debug_view_valid = 1;
    }
  else
    {
      memset (&split_debug_view[0], 0, sizeof split_debug_view);
      for (i = 0; i < (int) DEBUG_MAX; ++i)
	{
	  struct debug_section_info *dsec;

	  if (sections[i].size != 0)
	    dsec = &sections[i];
	  else if (zsections[i].size != 0)
	    dsec = &zsections[i];
	  else
	    continue;

	  if (!elf_get_view (state, descriptor, memory, memory_size,
			     dsec->offset, dsec->size, error_callback, data,
			     &split_debug_view[i]))
	    goto fail;
	  split_debug_view_valid[i] = 1;

	  if (sections[i].size != 0)
	    sections[i].data = ((const unsigned char *)
				split_debug_view[i].view.data);
	  else
	    zsections[i].data = ((const unsigned char *)
				 split_debug_view[i].view.data);
	}
    }

  /* We've read all we need from the executable.  */
  if (descriptor >= 0)
    {
      if (!backtrace_close (descriptor, error_callback, data))
	goto fail;
      descriptor = -1;
    }

  using_debug_view = 0;
  if (debug_view_valid)
    {
      for (i = 0; i < (int) DEBUG_MAX; ++i)
	{
	  if (sections[i].size == 0)
	    sections[i].data = NULL;
	  else
	    {
	      sections[i].data = ((const unsigned char *) debug_view.view.data
				  + (sections[i].offset - min_offset));
	      ++using_debug_view;
	    }

	  if (zsections[i].size == 0)
	    zsections[i].data = NULL;
	  else
	    zsections[i].data = ((const unsigned char *) debug_view.view.data
				 + (zsections[i].offset - min_offset));
	}
    }

  /* Uncompress the old format (--compress-debug-sections=zlib-gnu).  */

  zdebug_table = NULL;
  for (i = 0; i < (int) DEBUG_MAX; ++i)
    {
      if (sections[i].size == 0 && zsections[i].size > 0)
	{
	  unsigned char *uncompressed_data;
	  size_t uncompressed_size;

	  if (zdebug_table == NULL)
	    {
	      zdebug_table = ((uint16_t *)
			      backtrace_alloc (state, ZDEBUG_TABLE_SIZE,
					       error_callback, data));
	      if (zdebug_table == NULL)
		goto fail;
	    }

	  uncompressed_data = NULL;
	  uncompressed_size = 0;
	  if (!elf_uncompress_zdebug (state, zsections[i].data,
				      zsections[i].size, zdebug_table,
				      error_callback, data,
				      &uncompressed_data, &uncompressed_size))
	    goto fail;
	  sections[i].data = uncompressed_data;
	  sections[i].size = uncompressed_size;
	  sections[i].compressed = 0;

	  if (split_debug_view_valid[i])
	    {
	      elf_release_view (state, &split_debug_view[i],
				error_callback, data);
	      split_debug_view_valid[i] = 0;
	    }
	}
    }

  /* Uncompress the official ELF format
     (--compress-debug-sections=zlib-gabi).  */
  for (i = 0; i < (int) DEBUG_MAX; ++i)
    {
      unsigned char *uncompressed_data;
      size_t uncompressed_size;

      if (sections[i].size == 0 || !sections[i].compressed)
	continue;

      if (zdebug_table == NULL)
	{
	  zdebug_table = ((uint16_t *)
			  backtrace_alloc (state, ZDEBUG_TABLE_SIZE,
					   error_callback, data));
	  if (zdebug_table == NULL)
	    goto fail;
	}

      uncompressed_data = NULL;
      uncompressed_size = 0;
      if (!elf_uncompress_chdr (state, sections[i].data, sections[i].size,
				zdebug_table, error_callback, data,
				&uncompressed_data, &uncompressed_size))
	goto fail;
      sections[i].data = uncompressed_data;
      sections[i].size = uncompressed_size;
      sections[i].compressed = 0;

      if (debug_view_valid)
	--using_debug_view;
      else if (split_debug_view_valid[i])
	{
	  elf_release_view (state, &split_debug_view[i], error_callback, data);
	  split_debug_view_valid[i] = 0;
	}
    }

  if (zdebug_table != NULL)
    backtrace_free (state, zdebug_table, ZDEBUG_TABLE_SIZE,
		    error_callback, data);

  if (debug_view_valid && using_debug_view == 0)
    {
      elf_release_view (state, &debug_view, error_callback, data);
      debug_view_valid = 0;
    }

  for (i = 0; i < (int) DEBUG_MAX; ++i)
    {
      dwarf_sections.data[i] = sections[i].data;
      dwarf_sections.size[i] = sections[i].size;
    }

  if (!backtrace_dwarf_add (state, base_address, &dwarf_sections,
			    ehdr.e_ident[EI_DATA] == ELFDATA2MSB,
			    fileline_altlink,
			    error_callback, data, fileline_fn,
			    fileline_entry))
    goto fail;

  *found_dwarf = 1;

  return 1;

 fail:
  if (shdrs_view_valid)
    elf_release_view (state, &shdrs_view, error_callback, data);
  if (names_view_valid)
    elf_release_view (state, &names_view, error_callback, data);
  if (symtab_view_valid)
    elf_release_view (state, &symtab_view, error_callback, data);
  if (strtab_view_valid)
    elf_release_view (state, &strtab_view, error_callback, data);
  if (debuglink_view_valid)
    elf_release_view (state, &debuglink_view, error_callback, data);
  if (debugaltlink_view_valid)
    elf_release_view (state, &debugaltlink_view, error_callback, data);
  if (gnu_debugdata_view_valid)
    elf_release_view (state, &gnu_debugdata_view, error_callback, data);
  if (buildid_view_valid)
    elf_release_view (state, &buildid_view, error_callback, data);
  if (debug_view_valid)
    elf_release_view (state, &debug_view, error_callback, data);
  for (i = 0; i < (int) DEBUG_MAX; ++i)
    {
      if (split_debug_view_valid[i])
	elf_release_view (state, &split_debug_view[i], error_callback, data);
    }
  if (opd)
    elf_release_view (state, &opd->view, error_callback, data);
  if (descriptor >= 0)
    backtrace_close (descriptor, error_callback, data);
  return 0;
}

/* Data passed to phdr_callback.  */

struct phdr_data
{
  struct backtrace_state *state;
  backtrace_error_callback error_callback;
  void *data;
  fileline *fileline_fn;
  int *found_sym;
  int *found_dwarf;
  const char *exe_filename;
  int exe_descriptor;
};

/* Callback passed to dl_iterate_phdr.  Load debug info from shared
   libraries.  */

static int
#ifdef __i386__
__attribute__ ((__force_align_arg_pointer__))
#endif
phdr_callback (struct dl_phdr_info *info, size_t size ATTRIBUTE_UNUSED,
	       void *pdata)
{
  struct phdr_data *pd = (struct phdr_data *) pdata;
  const char *filename;
  int descriptor;
  int does_not_exist;
  fileline elf_fileline_fn;
  int found_dwarf;

  /* There is not much we can do if we don't have the module name,
     unless executable is ET_DYN, where we expect the very first
     phdr_callback to be for the PIE.  */
  if (info->dlpi_name == NULL || info->dlpi_name[0] == '\0')
    {
      if (pd->exe_descriptor == -1)
	return 0;
      filename = pd->exe_filename;
      descriptor = pd->exe_descriptor;
      pd->exe_descriptor = -1;
    }
  else
    {
      if (pd->exe_descriptor != -1)
	{
	  backtrace_close (pd->exe_descriptor, pd->error_callback, pd->data);
	  pd->exe_descriptor = -1;
	}

      filename = info->dlpi_name;
      descriptor = backtrace_open (info->dlpi_name, pd->error_callback,
				   pd->data, &does_not_exist);
      if (descriptor < 0)
	return 0;
    }

  if (elf_add (pd->state, filename, descriptor, NULL, 0, info->dlpi_addr,
	       pd->error_callback, pd->data, &elf_fileline_fn, pd->found_sym,
	       &found_dwarf, NULL, 0, 0, NULL, 0))
    {
      if (found_dwarf)
	{
	  *pd->found_dwarf = 1;
	  *pd->fileline_fn = elf_fileline_fn;
	}
    }

  return 0;
}

/* Initialize the backtrace data we need from an ELF executable.  At
   the ELF level, all we need to do is find the debug info
   sections.  */

int
backtrace_initialize (struct backtrace_state *state, const char *filename,
		      int descriptor, backtrace_error_callback error_callback,
		      void *data, fileline *fileline_fn)
{
  int ret;
  int found_sym;
  int found_dwarf;
  fileline elf_fileline_fn = elf_nodebug;
  struct phdr_data pd;

  ret = elf_add (state, filename, descriptor, NULL, 0, 0, error_callback, data,
		 &elf_fileline_fn, &found_sym, &found_dwarf, NULL, 1, 0, NULL,
		 0);
  if (!ret)
    return 0;

  pd.state = state;
  pd.error_callback = error_callback;
  pd.data = data;
  pd.fileline_fn = &elf_fileline_fn;
  pd.found_sym = &found_sym;
  pd.found_dwarf = &found_dwarf;
  pd.exe_filename = filename;
  pd.exe_descriptor = ret < 0 ? descriptor : -1;

  dl_iterate_phdr (phdr_callback, (void *) &pd);

  if (!state->threaded)
    {
      if (found_sym)
	state->syminfo_fn = elf_syminfo;
      else if (state->syminfo_fn == NULL)
	state->syminfo_fn = elf_nosyms;
    }
  else
    {
      if (found_sym)
	backtrace_atomic_store_pointer (&state->syminfo_fn, &elf_syminfo);
      else
	(void) __sync_bool_compare_and_swap (&state->syminfo_fn, NULL,
					     elf_nosyms);
    }

  if (!state->threaded)
    *fileline_fn = state->fileline_fn;
  else
    *fileline_fn = backtrace_atomic_load_pointer (&state->fileline_fn);

  if (*fileline_fn == NULL || *fileline_fn == elf_nodebug)
    *fileline_fn = elf_fileline_fn;

  return 1;
}

}

/*** End of inlined file: elf.cpp ***/


#  endif

/*** Start of inlined file: TracyStackFrames.cpp ***/
namespace tracy
{

const char* s_tracyStackFrames_[] = {
    "tracy::Callstack",
    "tracy::Callstack(int)",
    "tracy::GpuCtxScope::{ctor}",
    "tracy::Profiler::SendCallstack",
    "tracy::Profiler::SendCallstack(int)",
    "tracy::Profiler::SendCallstack(int, unsigned long)",
    "tracy::Profiler::MemAllocCallstack",
    "tracy::Profiler::MemAllocCallstack(void const*, unsigned long, int)",
    "tracy::Profiler::MemFreeCallstack",
    "tracy::Profiler::MemFreeCallstack(void const*, int)",
    "tracy::ScopedZone::{ctor}",
    "tracy::ScopedZone::ScopedZone(tracy::SourceLocationData const*, int, bool)",
    "tracy::CallTrace",
    "tracy::Profiler::Message",
    nullptr
};

const char** s_tracyStackFrames = s_tracyStackFrames_;

const StringMatch s_tracySkipSubframes_[] = {
    { "/include/arm_neon.h", 19 },
    { "/include/adxintrin.h", 20 },
    { "/include/ammintrin.h", 20 },
    { "/include/amxbf16intrin.h", 24 },
    { "/include/amxint8intrin.h", 24 },
    { "/include/amxtileintrin.h", 24 },
    { "/include/avx2intrin.h", 21 },
    { "/include/avx5124fmapsintrin.h", 29 },
    { "/include/avx5124vnniwintrin.h", 29 },
    { "/include/avx512bf16intrin.h", 27 },
    { "/include/avx512bf16vlintrin.h", 29 },
    { "/include/avx512bitalgintrin.h", 29 },
    { "/include/avx512bwintrin.h", 25 },
    { "/include/avx512cdintrin.h", 25 },
    { "/include/avx512dqintrin.h", 25 },
    { "/include/avx512erintrin.h", 25 },
    { "/include/avx512fintrin.h", 24 },
    { "/include/avx512ifmaintrin.h", 27 },
    { "/include/avx512ifmavlintrin.h", 29 },
    { "/include/avx512pfintrin.h", 25 },
    { "/include/avx512vbmi2intrin.h", 28 },
    { "/include/avx512vbmi2vlintrin.h", 30 },
    { "/include/avx512vbmiintrin.h", 27 },
    { "/include/avx512vbmivlintrin.h", 29 },
    { "/include/avx512vlbwintrin.h", 27 },
    { "/include/avx512vldqintrin.h", 27 },
    { "/include/avx512vlintrin.h", 25 },
    { "/include/avx512vnniintrin.h", 27 },
    { "/include/avx512vnnivlintrin.h", 29 },
    { "/include/avx512vp2intersectintrin.h", 35 },
    { "/include/avx512vp2intersectvlintrin.h", 37 },
    { "/include/avx512vpopcntdqintrin.h", 32 },
    { "/include/avx512vpopcntdqvlintrin.h", 34 },
    { "/include/avxintrin.h", 20 },
    { "/include/avxvnniintrin.h", 24 },
    { "/include/bmi2intrin.h", 21 },
    { "/include/bmiintrin.h", 20 },
    { "/include/bmmintrin.h", 20 },
    { "/include/cetintrin.h", 20 },
    { "/include/cldemoteintrin.h", 25 },
    { "/include/clflushoptintrin.h", 27 },
    { "/include/clwbintrin.h", 21 },
    { "/include/clzerointrin.h", 23 },
    { "/include/emmintrin.h", 20 },
    { "/include/enqcmdintrin.h", 23 },
    { "/include/f16cintrin.h", 21 },
    { "/include/fma4intrin.h", 21 },
    { "/include/fmaintrin.h", 20 },
    { "/include/fxsrintrin.h", 21 },
    { "/include/gfniintrin.h", 21 },
    { "/include/hresetintrin.h", 23 },
    { "/include/ia32intrin.h", 21 },
    { "/include/immintrin.h", 20 },
    { "/include/keylockerintrin.h", 26 },
    { "/include/lwpintrin.h", 20 },
    { "/include/lzcntintrin.h", 22 },
    { "/include/mmintrin.h", 19 },
    { "/include/movdirintrin.h", 23 },
    { "/include/mwaitxintrin.h", 23 },
    { "/include/nmmintrin.h", 20 },
    { "/include/pconfigintrin.h", 24 },
    { "/include/pkuintrin.h", 20 },
    { "/include/pmmintrin.h", 20 },
    { "/include/popcntintrin.h", 23 },
    { "/include/prfchwintrin.h", 23 },
    { "/include/rdseedintrin.h", 23 },
    { "/include/rtmintrin.h", 20 },
    { "/include/serializeintrin.h", 26 },
    { "/include/sgxintrin.h", 20 },
    { "/include/shaintrin.h", 20 },
    { "/include/smmintrin.h", 20 },
    { "/include/tbmintrin.h", 20 },
    { "/include/tmmintrin.h", 20 },
    { "/include/tsxldtrkintrin.h", 25 },
    { "/include/uintrintrin.h", 22 },
    { "/include/vaesintrin.h", 21 },
    { "/include/vpclmulqdqintrin.h", 27 },
    { "/include/waitpkgintrin.h", 24 },
    { "/include/wbnoinvdintrin.h", 25 },
    { "/include/wmmintrin.h", 20 },
    { "/include/x86gprintrin.h", 23 },
    { "/include/x86intrin.h", 20 },
    { "/include/xmmintrin.h", 20 },
    { "/include/xopintrin.h", 20 },
    { "/include/xsavecintrin.h", 23 },
    { "/include/xsaveintrin.h", 22 },
    { "/include/xsaveoptintrin.h", 25 },
    { "/include/xsavesintrin.h", 23 },
    { "/include/xtestintrin.h", 22 },
    { "/bits/atomic_base.h", 19 },
    { "/atomic", 7 },
    {}
};

const StringMatch* s_tracySkipSubframes = s_tracySkipSubframes_;

}

/*** End of inlined file: TracyStackFrames.cpp ***/


#endif

#ifdef _MSC_VER
#  pragma comment(lib, "ws2_32.lib")
#  pragma comment(lib, "dbghelp.lib")
#  pragma comment(lib, "advapi32.lib")
#  pragma comment(lib, "user32.lib")
#  pragma warning(pop)
#endif

#endif
