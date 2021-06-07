/* logging.cxx */
/*
 * Copyright 2021 Alexander Samarin <sasha.devel@gmail.com>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <stdarg.h>
#include <stdio.h>

#include "logging.h"

#define loglevel_check(_level) do { \
	if (loglevel < _level) \
		return; \
} while (0)

int loglevel = 0;

static void log_vprintf_internal(int level, const char *fmt, va_list ap)
{
	const char *level_name;
	switch (level) {
	case LOGLEVEL_ERROR:   level_name = "ERROR";   break;
	case LOGLEVEL_WARNING: level_name = "WARNING"; break;
	case LOGLEVEL_INFO:    level_name = "INFO";    break;
	case LOGLEVEL_VERBOSE: level_name = "VERBOSE"; break;
	case LOGLEVEL_DEBUG:   level_name = "DEBUG";   break;
	default: level_name = "TRACE"; break;
	}

	fprintf(stderr, "%s:main:", level_name);
	vfprintf(stderr, fmt, ap);
}

void log_vprintf(int level, const char *fmt, va_list ap)
{
	loglevel_check(level);
	log_vprintf_internal(level, fmt, ap);
}

void log_printf(int level, const char *fmt, ...)
{
	loglevel_check(level);

	va_list ap;
	va_start(ap, fmt);
	log_vprintf(level, fmt, ap);
	va_end(ap);
}

void log_error(const char *fmt, ...)
{
	int level = LOGLEVEL_ERROR;
	loglevel_check(level);

	va_list ap;
	va_start(ap, fmt);
	log_vprintf(level, fmt, ap);
	va_end(ap);
}

void log_warning(const char *fmt, ...)
{
	int level = LOGLEVEL_WARNING;
	loglevel_check(level);

	va_list ap;
	va_start(ap, fmt);
	log_vprintf(level, fmt, ap);
	va_end(ap);
}

void log_info(const char *fmt, ...)
{
	int level = LOGLEVEL_INFO;
	loglevel_check(level);

	va_list ap;
	va_start(ap, fmt);
	log_vprintf(level, fmt, ap);
	va_end(ap);
}

void log_verbose(const char *fmt, ...)
{
	int level = LOGLEVEL_VERBOSE;
	loglevel_check(level);

	va_list ap;
	va_start(ap, fmt);
	log_vprintf(level, fmt, ap);
	va_end(ap);
}

void log_debug(const char *fmt, ...)
{
	int level = LOGLEVEL_DEBUG;
	loglevel_check(level);

	va_list ap;
	va_start(ap, fmt);
	log_vprintf(level, fmt, ap);
	va_end(ap);
}
