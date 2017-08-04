/*
 *
 *	@author : rrrfff@foxmail.com
 *  https://github.com/rrrfff/AndDetours
 *
 */
#pragma once
#include "AndDetours.h"
#include "LDasm.h"

//-------------------------------------------------------------------------

class x86
{
public:
	struct __packed insns
	{
		uint8_t  jmp0;
		int32_t  offset0;
		uint8_t  origin[1u + 4u + 14u];
	};
	static __attribute((weak, hidden, aligned(__page_size))) insns __insns_pool[__page_size / sizeof(insns)];

	static void __hidden init() {
		::mprotect(__page_align(__insns_pool), __page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
	}

	static insns *hook(void *func, void *my_func) {
		static constexpr uint8_t  __jmp   = 0xE9u;
		static constexpr uint32_t __jmp_s = 1u + 4u;
		static constexpr uint32_t __avail = 14u;
		static volatile uint32_t  __index = -1;

		unsigned char *pfunc = static_cast<unsigned char *>(evaluate_jmp(func));

		uint32_t   l = 0, fix = 0;
		ldasm_data ld;
		do 
		{
			if (pfunc[l] == 0xE8u) { // CALL + WORD OFFSET
				fix = l + 1;
			} //if
			l += ldasm(pfunc + l, &ld);
		} while (l < __jmp_s);
		if (l > __avail) return NULL;

		uint32_t i  = __atomic_increase(&__index);
		insns *s    = __insns_pool + i;
		s->jmp0     = __jmp;
		s->offset0  = reinterpret_cast<int32_t>(my_func) - reinterpret_cast<int32_t>(pfunc) - __jmp_s;
		::memcpy(s->origin, pfunc, l);	

		// fixs for indirect CALL 0xE8
		if (fix + sizeof(uint32_t) == l) {
			// `CALL` is the last instruction, convert to `PUSH JMP` to workaround some issue
			auto cp = reinterpret_cast<unsigned char *>(reinterpret_cast<intptr_t>(&s->origin) + fix - 1u);
			cp[0]   = 0x68u;
			auto of = *reinterpret_cast<uint32_t *>(cp + 1);
			*reinterpret_cast<uint32_t *>(cp + 1u) = reinterpret_cast<uint32_t>(pfunc) + l;
			cp[5]   = __jmp;
			*reinterpret_cast<uint32_t *>(cp + 6u) =
				reinterpret_cast<uint32_t>(pfunc) + l + of - reinterpret_cast<int32_t>(cp + 6u + sizeof(uint32_t));
//			DEBUG_LOGI("fixed call at %u using push/jmp", fix);
		} else {
			if (fix > 0) {
				int32_t *poffset = reinterpret_cast<int32_t *>(s->origin + fix);
//				*poffset = pfunc + fix + sizeof(int32_t) + *poffset - (s->origin + fix + sizeof(int32_t));
				*poffset = pfunc + *poffset - s->origin;
//				DEBUG_LOGI("fixed call at %u, new = 0x%.8X", fix, *poffset);
			} //if
			insns *sp   = reinterpret_cast<insns *>(reinterpret_cast<uint32_t>(&s->origin) + l);
			sp->jmp0    = __jmp;
			sp->offset0 = reinterpret_cast<int32_t>(pfunc) + l - (reinterpret_cast<int32_t>(&s->origin) + l + __jmp_s);
		} //if

		::mprotect(__page_align(pfunc), __page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
		::memcpy(pfunc, s, l);
		::mprotect(__page_align(pfunc), __page_size, PROT_READ | PROT_EXEC);

#if defined(__clang__) || GCC_VERSION >= 40300
		__builtin___clear_cache(static_cast<char *>(func),
								static_cast<char *>(func) + l);
#endif
		return s;
	}
};
__attribute((weak, aligned(__page_size))) x86::insns x86::__insns_pool[__page_size / sizeof(insns)];

//-------------------------------------------------------------------------

typedef x86 cpu_insns;