#include <limits.h>
#include <stddef.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>



void *
bsearch(const void *key,
	const void *base,
	size_t nmemb,
	size_t size,
	int (*compar)(const void *, const void *))
{
	size_t low, high, mid;
	const void *ptr;
	int comparison;

	low = 0;
	high = nmemb;
	while (low < high) {
		mid = low + (high - low) / 2;
		ptr = (void *)(((const char *)base) + (mid * size));
		comparison = (*compar)(key, ptr);
		if (comparison < 0)
			high = mid;
		else if (comparison > 0)
			low = mid + 1;
		else
			return (void *)ptr;
	}

	return NULL;
}
