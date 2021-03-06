/* Simple malloc tests.
 */
#include <stdlib.h>
#include <string.h>
#if defined(__SDCC_pic16)
#include <malloc.h>
#endif
#include <testfwk.h>

#if defined(__SDCC_pic16)
__xdata char heap[100];
#endif

void mallocfree(void)
{
	char *a, *b, *c;
	char d[25];

	a = malloc(16);
	ASSERT(a);
	memset(a, 2, 16);
	b = malloc(16);
	ASSERT(b);
	memset(b, 1, 16);
	c = calloc(16, 1);
	ASSERT(c);

	memset(d, 2, 16);
	ASSERT(!memcmp(d, a, 16));
	memset(d, 1, 16);
	ASSERT(!memcmp(d, b, 16));
	memset(d, 0, 16);
	ASSERT(!memcmp(d, c, 16));

	free(b);
	b = malloc(20);
	memset(b, 3, 20);

	memset(d, 2, 16);
	ASSERT(!memcmp(d, a, 16));
	memset(d, 3, 20);
	ASSERT(!memcmp(d, b, 20));
	memset(d, 0, 16);
	ASSERT(!memcmp(d, c, 16));

	free(b);
	b = malloc(10);
	memset(b, 4, 10);

	memset(d, 2, 16);
	ASSERT(!memcmp(d, a, 16));
	memset(d, 4, 20);
	ASSERT(!memcmp(d, b, 10));
	memset(d, 0, 16);
	ASSERT(!memcmp(d, c, 16));

	free(b);
	b = malloc(8);
	memset(b, 5, 8);
	b = realloc(b, 4);
	memset(d, 5, 8);
	ASSERT(!memcmp(d, b, 4));

	free(a);
	b = realloc(b, 8);
	ASSERT(!memcmp(d, b, 4));

	free(b);
	free(c);

	a = malloc(10);
	memset(a, 6, 10);
	b = malloc(10);
	memset(b, 7, 10);
	c = malloc(10);
	memset(c, 8, 10);
	free(b);
	a = realloc(a, 25);
	memset(a + 10, 6, 15);

	memset(d, 6, 25);
	ASSERT(!memcmp(d, a, 25));
	memset(d, 8, 10);
	ASSERT(!memcmp(d, c, 10));
}

void
testMalloc (void)
{
  void __xdata *p1, *p2, *p3;
  char *p;
  unsigned char i;

#if !defined(PORT_HOST) && !defined(__SDCC_gbz80) && !defined(__SDCC_z80)
#if defined(__SDCC_pic16)
  _initHeap (heap, sizeof heap);
#endif

#ifndef PORT_HOST
  p1 = malloc (0xFFFF);
  ASSERT (p1 == NULL);
  LOG (("p1 == NULL when out of memory\n"));
#endif

  p1 = malloc (2000);
  ASSERT (p1 == NULL);
  LOG (("p1 == NULL when out of memory\n"));
#ifdef PORT_HOST
  LOG (("p1: %p\n", p1));
#else
  LOG (("p1: %u\n", (unsigned) p1));
#endif
#endif

  p1 = malloc (5);
  ASSERT (p1 != NULL);
#ifdef PORT_HOST
  LOG (("p1: %p\n", p1));
#else
  LOG (("p1: %u\n", (unsigned) p1));
#endif

  p2 = malloc (20);
  ASSERT (p2 != NULL);
#ifdef PORT_HOST
  LOG (("p2: %p\n", p2));
#else
  LOG (("p2: %u\n", (unsigned) p2));
#endif

  p = (char*)p2;
  for (i = 0; i < 20; i++, p++)
    *p = i;

  p2 = realloc (p2, 25);
  ASSERT (p2 != NULL);
#ifdef PORT_HOST
  LOG (("p2, after expanding realloc: %p\n", p2));
#else
  LOG (("p2, after expanding realloc: %u\n", (unsigned) p2));
#endif

  p = (char*)p2;
  for (i = 0; i < 20; i++, p++)
    ASSERT (*p == i);

  p2 = realloc (p2, 15);
  ASSERT (p2 != NULL);
#ifdef PORT_HOST
  LOG (("p2, after shrinking realloc: %p\n", p2));
#else
  LOG (("p2, after shrinking realloc: %u\n", (unsigned) p2));
#endif

  p = (char*)p2;
  for (i = 0; i < 15; i++, p++)
    ASSERT (*p == i);

  free (p2);

  p3 = malloc (10);
  ASSERT (p3 != NULL);
#ifdef PORT_HOST
  LOG (("p3, after freeing p2: %p\n", p3));
#else
  LOG (("p3, after freeing p2: %u\n", (unsigned) p3));
#endif

  mallocfree();
}

