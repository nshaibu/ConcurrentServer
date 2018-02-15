/*===========================================================================================
# Copyright (C) 2018 Nafiu Shaibu.
# Purpose: 
#-------------------------------------------------------------------------------------------
# This is a free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3 of the License, or (at your option)
# any later version.

# This is distributed in the hopes that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#===========================================================================================
*/

#include "errno_logs.h"

static struct logs_struct logs;

/*Function pointer to close sockets and file descriptors*/
void error_close_fd(void *fd) {
	close( *(int*)fd );
}

/*Ignore destructor if Object parameter is not user(set to NULL)*/
void error_ignore(void *v) { }

void init_logs_object() 
{
	char file_name[BUFF_DEF_STR];
	
	memset(file_name, '\0', BUFF_DEF_STR);
	
	GET_LOG_FILENAME(file_name);
	
	if (strlen(file_name) == 0 ) {
		fprintf(stderr, "%s %s %d\n", "Failed to get environment variable", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	logs.stdlog = fopen(file_name, "a+"); //open in appending mode
	if (logs.stdlog == NULL) {
		fprintf(stderr, "%s %s %d\n", "Failed to open log file", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	pthread_mutex_init(&(logs.log_mutex), NULL);
	strcpy(logs.format, "[%s] [%s] [%s] %s ('%s')\n");
}


void log_errors(
                pthread_t *thread_id,       /*The Thread id [if not thread then assign NULL]*/
                error_type errtype,         /*Error type whether [MYSQL_ERRORS/STD_ERRORS]*/
                do_exit ex,                 /*Determine whether to exit the program [DO_EXIT/DONT_EXIT]*/
                write_ops op,               /*The stream struct to write to [WRITE_STDOUT/WRITE_STDDER/WRITE_BOTH_STDOUT_STDERR]*/
                logs_level lev,             /*The error level [LOGS_FATAL_ERRORS|LOGS_INFO/LOGS_WARNING]*/
                const char *errstr,         /*The string to display*/
                void *Object,               /*The object to distroy*/
                void (*dtor)(void*)         /*The Object's destructor*/
 ) {
	char time_buf[TIME_BUZ];
	char default_str[BUFF_DEF_STR];
	
	char level[10];
	
	time_t t = time(NULL);
	(void*)ctime_r(&t, time_buf);
	
	int len = strlen(time_buf);
	time_buf[len-1]='\0';
	
	
	switch (lev) {
		case LOGS_WARNING: strcpy(level, "WARNING"); break;
		case LOGS_INFO: strcpy(level, "INFO"); break;
		case LOGS_FATAL_ERRORS: strcpy(level, "FATAL"); break;
	}
	
	if (errtype == MYSQL_ERRORS) 
		snprintf( default_str, BUFF_DEF_STR, "%s", mysql_error(Object) );
	if (errtype == STD_ERRORS)
		strerror_r(errno, default_str, BUFF_DEF_STR);
	
	switch (op) {
		case WRITE_STDDER: 
			fprintf(stderr, logs.format, time_buf, level, (thread_id)? "Threads" : "Main Thread", errstr, default_str); 
			fflush(stderr); 
			break;
		case WRITE_STDOUT: 
			fprintf(stdout, logs.format, time_buf, level, (thread_id)? "Threads" : "Main Thread", errstr, default_str); 
			fflush(stdout); 
			break;
		case WRITE_BOTH_STDOUT_STDERR:
			fprintf(stderr, logs.format, time_buf, level, (thread_id)? "Threads" : "Main Thread", errstr, default_str);
			fprintf(stdout, logs.format, time_buf, level, (thread_id)? "Threads" : "Main Thread", errstr, default_str);
			
			fflush(stderr);
			fflush(stdout);
		break;
		case NO_WRITE: break;
	}
	
	pthread_mutex_lock( &(logs.log_mutex) );
	
	fprintf(logs.stdlog, logs.format, time_buf, level, (thread_id)? "Threads" : "Main Thread", errstr, default_str);
	
	pthread_mutex_unlock( &(logs.log_mutex) );
	
	if ( ex == DO_EXIT) {
		dtor(Object);    //destroy object
		
		if (thread_id == NULL) 
			exit(EXIT_FAILURE);
		else
			pthread_cancel( pthread_self() );
	}
}


void destroy_logs_object() {
	fclose(logs.stdlog);
	
	pthread_mutex_destroy( &(logs.log_mutex) );
}
