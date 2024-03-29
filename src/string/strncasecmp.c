#include <ctype.h>
#include <stddef.h>
#include <string.h>



int strncasecmp(const char *s, const char *t, size_t n)
{
	int s_bis = 0;
	int t_bis = 0;
	size_t i = 0;

	while (*s && *t && i++ < n) {
		s_bis = tolower(*s);
		t_bis = tolower(*t);

		if (s_bis == t_bis) {
			if (*s == '\0') {
				return 0;
			}
			s++;
			t++;
		}
		else {
			break;
		}
	}
	return s_bis - t_bis;
}
