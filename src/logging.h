/* logging.h */
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

#ifndef _LOGGING_H
#define _LOGGING_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOGLEVEL_ERROR   0
#define LOGLEVEL_WARNING 1
#define LOGLEVEL_INFO    2
#define LOGLEVEL_VERBOSE 3
#define LOGLEVEL_DEBUG   4

extern int loglevel;

extern void log_vprintf(int level, const char *fmt, va_list ap);

extern void log_printf(int level, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));

extern void log_error(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

extern void log_warning(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

extern void log_info(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

extern void log_verbose(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

extern void log_debug(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
};
#endif

#endif
