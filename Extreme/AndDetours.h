/*
 *
 *	@author : rrrfff@foxmail.com
 *  https://github.com/rrrfff/AndDetours
 *
 */
#pragma once
#undef  __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/cdefs.h>
#if __has_include("utility")
# include <utility>
#endif // std::forward
#define __page_size          4096
#define __page_align(x)      reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(x) & -(__page_size))
#define __attribute          __attribute__
#define __hidden             __attribute((visibility("hidden")))
#define __selectany          __attribute((weak))
#define __noinline           __attribute((noinline))
#define hidden               visibility("hidden")
#define aligned(x)           __aligned__(x)
#define packed			     __packed__
#define __atomic_increase(p) __sync_add_and_fetch(p, 1)
#define __func_cast(x)       reinterpret_cast<void *>(x)
#define __func(f)            __func_cast(&f)
#define __func_type(f)       __typeof__(&f)
#define __func_origin(x)     reinterpret_cast<T>(x->origin)

//-------------------------------------------------------------------------

#if defined(__arm__)
# include "ARM32.h"
#elif defined(__aarch64__)
# include "ARM64.h"
#elif defined(__mips__) && !defined(__LP64__)
# include "MIPS32.h"
#elif defined(__mips__) && defined(__LP64__)
# include "MIPS64.h"
#elif defined(__i386__)
# include "X86.h"
#elif defined(__x86_64__)
# include "X86_64.h"
#endif

//-------------------------------------------------------------------------

template<typename T, class K = cpu_insns> class detours : public K::insns
{
public:
	detours *hook(void *func, T my_func) {
		return static_cast<detours *>(K::hook(func, reinterpret_cast<void *>(my_func)));
	}
	detours *hook(T func, T my_func) {
		return static_cast<detours *>(K::hook(reinterpret_cast<void *>(func),
										   reinterpret_cast<void *>(my_func)));
	}
#if __has_include("utility")
	template<class... P> auto invoke(P &&... args) {
		return __func_origin(this)(std::forward<P>(args)...);
	}
#else
	T invoke() {
		return __func_origin(this);
	}
#endif
};