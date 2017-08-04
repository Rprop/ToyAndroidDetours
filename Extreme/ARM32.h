/*
 *
 *	@author : rrrfff@foxmail.com
 *  https://github.com/rrrfff/AndDetours
 *
 */
#pragma once
#include "AndDetours.h"

//-------------------------------------------------------------------------

class arm32
{
public:
	struct __packed insns
	{
		uint32_t ldr0;
		uint32_t addr0;
		uint32_t origin[2];
		uint32_t ldr1;
		uint32_t addr1;
	};
	static __attribute((weak, hidden, aligned(__page_size))) insns __insns_pool[__page_size / sizeof(insns)];

	static void __hidden init() {
		::mprotect(__page_align(__insns_pool), __page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
	}

	static void __attribute((noinline, naked, hidden)) __insns() {
		__asm (
		".arm" "\n"
		"LDR PC, [PC, #-4]" "\n"
		".word 0" "\n"
		".word 0" "\n"
		".word 0" "\n"
		"LDR PC, [PC, #-4]" "\n"
		".word 0" "\n"
		:::);
	}

	static insns *hook(void *func, void *my_func) {
		static volatile uint32_t __index = -1;

		uint_fast32_t i = __atomic_increase(&__index);

		insns *s = __insns_pool + i;
		::memcpy(s, reinterpret_cast<void *>(__insns), sizeof(insns));
		s->addr0 = reinterpret_cast<uintptr_t>(my_func);
		s->addr1 = reinterpret_cast<uintptr_t>(func) + sizeof(s->origin);
		::memcpy(s->origin, func, sizeof(s->origin));
		::mprotect(__page_align(func), __page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
		::memcpy(func, s, sizeof(s->origin));
#if defined(__clang__) || GCC_VERSION >= 40300
		__builtin___clear_cache(reinterpret_cast<char *>(func), 
								reinterpret_cast<char *>(func) + sizeof(s->origin));
#endif
		return s;
	}
};
__attribute((weak, aligned(__page_size))) arm32::insns arm32::__insns_pool[__page_size / sizeof(insns)];

//-------------------------------------------------------------------------

typedef arm32 cpu_insns;