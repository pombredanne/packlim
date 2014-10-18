#ifndef _LOG_H_INCLUDED
#	define _LOG_H_INCLUDED

enum log_levels {
	LOG_DEBUG	= 0,
	LOG_INFO	= 1,
	LOG_WARN	= 2,
	LOG_ERR		= 3
};

void log_write(const int level, const char *format, ...);

#endif