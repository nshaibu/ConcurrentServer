#if !defined(ERRNO_LOGS_H)
#define ERRNO_LOGS_H

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <pthread.h>
#include <mysql.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>


#define TIME_BUZ 40
#define BUFF_DEF_STR 100


#define GET_LOG_FILENAME(file) ({\
                               char *env_=getenv("HOME"); \
                               if (env_) \
                                 snprintf(file, BUFF_DEF_STR, "%s/%s", env_, "concurrent_chat_2018-01-29.log"); \
                              })

#ifdef _GNUC_
#define NORETURN __attribute__ (( noreturn ))
#else
#define NORETURN
#endif

//define log msgs
#define MEMORY_NOT_ALLOC "Cannot not allocate memory"
#define MAX_CON_REACH "Maximum connection reached"
#define THREAD_NOT_START "Cannot start thread"
#define SOCKET_BINDING_FAILED "Failed to bind to socket"
#define SOCKET_NOT_CREATED "Cannot create tcp socket"
#define MYSQL_INIT_FAILED "Cannot initialize mysql structure"
#define MYSQL_CONNECTION_FAILED "Failed to connect to the mysql server"
#define MYSQL_QUERY_FAILED "mysql query execution failed"
#define MYSQL_RESULT_CANT_READ "Cannot not read mysql query result"
#define MYSQL_NO_USER "No such user on the system(mysql)"
#define SIGNAL_FAILED "Setting up unix signal failed"


typedef enum { MYSQL_ERRORS, STD_ERRORS } error_type;
typedef enum { DO_EXIT, DONT_EXIT } do_exit;

/*The logging levels for the server*/
typedef enum {
	LOGS_WARNING,
	LOGS_INFO,
	LOGS_FATAL_ERRORS
} logs_level;

typedef enum {
	WRITE_STDDER,
	WRITE_STDOUT,
	WRITE_BOTH_STDOUT_STDERR,
	NO_WRITE
} write_ops;

struct logs_struct {
	FILE *stdlog;			//stream IO for logs (buffered logs)
	char format[30];	//format for logging
	
	pthread_mutex_t log_mutex;  //mutex device for logs
};

void init_logs_object();

void log_errors(
                pthread_t *thread_id,        /*The Thread id [if not thread then assign -1]*/
                error_type errtype,         /*Error type whether [MYSQL_ERRORS/STD_ERRORS]*/
                do_exit ex,                 /*Determine whether to exit the program [DO_EXIT/DONT_EXIT]*/
                write_ops op,               /*The stream struct to write to [WRITE_STDOUT/WRITE_STDDER/WRITE_BOTH_STDOUT_STDERR]*/
                logs_level lev,             /*The error level [LOGS_FATAL_ERRORS|LOGS_INFO/LOGS_WARNING]*/
                const char *errstr,         /*The string to display*/
                void *Object,               /*The object to distroy*/
                void (*dtor)(void*)         /*The Object's destructor*/
) NORETURN;

void destroy_logs_object() NORETURN;

/* Destructors to be passed to log_errors to destroy Object*/
void error_ignore(void *);       /*Ignore destructor if Object parameter is not user(set to NULL)*/
void error_close_fd(void *fd);  /*Function pointer to close sockets and file descriptors*/


#endif
