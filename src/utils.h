/****************************************************************
 *** utils.h
 *** 
 *** To use utils.c/utils.h, be sure this is in your configure.ac file:

AC_CHECK_HEADERS([err.h err.h sys/mman.h sys/resource.h unistd.h])
AC_CHECK_FUNCS([ishexnumber unistd.h err errx warn warnx pread64 pread _lseeki64 ])

***
 ****************************************************************/



#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <stdint.h>
#include <sys/time.h>

#ifndef __BEGIN_DECLS
#if defined(__cplusplus)
#define __BEGIN_DECLS   extern "C" {
#define __END_DECLS     }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif
#endif

__BEGIN_DECLS

#ifdef HAVE_ERR_H
#include <err.h>
#else
void err(int eval,const char *fmt,...) __attribute__((format(printf, 2, 0))) __attribute__ ((__noreturn__));
void errx(int eval,const char *fmt,...) __attribute__((format(printf, 2, 0))) __attribute__ ((__noreturn__));
void warn(const char *fmt, ...) __attribute__((format(printf, 1, 0)));
void warnx(const char *fmt,...) __attribute__((format(printf, 1, 0)));
#endif

#ifndef HAVE_LOCALTIME_R
void localtime_r(time_t *t,struct tm *tm);
#endif

#ifndef HAVE_GMTIME_R
void gmtime_r(time_t *t,struct tm *tm);
#endif

int64_t get_filesize(int fd);

__END_DECLS

#endif
