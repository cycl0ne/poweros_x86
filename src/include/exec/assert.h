/*
** Assert Handling
**
**
*/

#ifndef ASSERT_H
#define ASSERT_H

void Alert(UINT32 alertNum, const char *fmt, ...);

#ifdef USEASSERT
#define allert_assert(fmt, ...) \
	Alert((1<<31), fmt, ##__VA_ARGS__)

#define ASSERT(expr) \
	do { \
		if (!(expr)) \
			alert_assert("%s() at %s:%u:\n%s", \
			    __func__, __FILE__, __LINE__, #expr); \
	} while (0)

#define ASSERT_VERBOSE(expr, msg) \
	do { \
		if (!(expr)) \
			alert_assert("%s() at %s:%u:\n%s, %s", \
			    __func__, __FILE__, __LINE__, #expr, msg); \
	} while (0)

#else

#define ASSERT(expr)
#define ASSERT_VERBOSE(expr, msg)

#endif
#endif
