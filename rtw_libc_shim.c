// SPDX-License-Identifier: GPL-2.0
/*
 * Local string/memory primitives for the StrykerOSS UML kernel.
 *
 * Why this file exists
 * --------------------
 * The guest kernel is built as ARCH=um SUBARCH=arm64, and arch/um has no
 * include/asm/string.h of its own, so asm/string.h resolves to
 * arch/arm64/include/asm/string.h. That header defines __HAVE_ARCH_MEMCPY,
 * __HAVE_ARCH_MEMSET, __HAVE_ARCH_STRCMP and the rest, which makes
 * lib/string.c skip those functions entirely:
 *
 *     #ifndef __HAVE_ARCH_MEMCPY
 *     void *memcpy(...) { ... }
 *     EXPORT_SYMBOL(memcpy);
 *     #endif
 *
 * The arm64 assembly versions in arch/arm64/lib/ are not exported, so the
 * symbols simply do not exist in the kernel's __ksymtab. An out-of-tree
 * module that calls memcpy() gets:
 *
 *     88XXau: Unknown symbol memcpy (err -2)
 *
 * The in-tree modules never hit this because they are linked into vmlinux.
 *
 * So the module carries its own copies. They are plain byte loops: the driver
 * only uses them on descriptor and control buffers, never in a path where the
 * arch-optimised routine would make a measurable difference.
 */

#include <linux/types.h>
#include <linux/compiler.h>

void *memcpy(void *dest, const void *src, __kernel_size_t count)
{
	char *tmp = dest;
	const char *s = src;

	while (count--)
		*tmp++ = *s++;
	return dest;
}

void *memmove(void *dest, const void *src, __kernel_size_t count)
{
	char *tmp = dest;
	const char *s = src;

	if (dest <= src) {
		while (count--)
			*tmp++ = *s++;
	} else {
		tmp += count;
		s += count;
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}

void *memset(void *s, int c, __kernel_size_t count)
{
	char *xs = s;

	while (count--)
		*xs++ = (char)c;
	return s;
}

int memcmp(const void *cs, const void *ct, __kernel_size_t count)
{
	const unsigned char *su1 = cs;
	const unsigned char *su2 = ct;

	while (count--) {
		if (*su1 != *su2)
			return *su1 - *su2;
		su1++;
		su2++;
	}
	return 0;
}

char *strcpy(char *dest, const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		;
	return tmp;
}

int strcmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;

	for (;;) {
		c1 = (unsigned char)*cs++;
		c2 = (unsigned char)*ct++;
		if (c1 != c2)
			return c1 - c2;
		if (!c1)
			break;
	}
	return 0;
}

int strncmp(const char *cs, const char *ct, __kernel_size_t count)
{
	unsigned char c1, c2;

	while (count) {
		c1 = (unsigned char)*cs++;
		c2 = (unsigned char)*ct++;
		if (c1 != c2)
			return c1 - c2;
		if (!c1)
			break;
		count--;
	}
	return 0;
}

char *strchr(const char *s, int c)
{
	for (;; ++s) {
		if (*s == (char)c)
			return (char *)s;
		if (!*s)
			return NULL;
	}
}

char *strrchr(const char *s, int c)
{
	const char *last = NULL;

	do {
		if (*s == (char)c)
			last = s;
	} while (*s++);

	return (char *)last;
}

__kernel_size_t strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		;
	return sc - s;
}

__kernel_size_t strnlen(const char *s, __kernel_size_t count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		;
	return sc - s;
}

void *memchr(const void *s, int c, __kernel_size_t n)
{
	const unsigned char *p = s;

	while (n-- != 0) {
		if ((unsigned char)c == *p)
			return (void *)(p);
		p++;
	}
	return NULL;
}
