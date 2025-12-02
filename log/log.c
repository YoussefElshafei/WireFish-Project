/*
 * File: log.c
 * Implements printf-style logging to stderr with level filtering.
 * 
 * Youssef Khafagy
 */

#include <stdio.h>
#include <stdarg.h>
#include "log.h"


//Starting log level and stores it
static LogLevel currentLevel = LOG_INFO;



/*
 * This is what sets what level of log messages we want to show
 * chosenLevel = the level to use
 * no return
 */
void log_set_level(LogLevel chosenLevel){
    currentLevel = chosenLevel;
}


/*
 * helper so we dont type the same code 4 times
 * messageLevel = level for the message
 * tag = the text like it can be something like "debug"
 * message = printf-style message text
 * args = extra stuff for the format
 */
static void write_log_message(LogLevel messageLevel, const char *tag, const char *message, va_list args){

    //dont print if it's below the level we set
    if(messageLevel < currentLevel){
        return;
    }

    //print tag first
    fprintf(stderr,"[%s] ", tag);

    // vfprintf is like printfbut works with "..."
    vfprintf(stderr, message, args); 
    
    //just make it look nice
    fprintf(stderr,"\n");
}



/*
 * prints a debug log message
 */
void log_debug(const char *message, ...){
    // "..." means the function can take any number of extra arguments, its called variadic arguments
    // va_list, va_start and va_end are tools used to read those extra arguments
    va_list args; 
    va_start(args, message);
    write_log_message(LOG_DEBUG, "debug : ", message, args);
    va_end(args);
}



/*
 * prints an info log message
 */
void log_info(const char *message, ...){
    va_list args;
    va_start(args, message);
    write_log_message(LOG_INFO, "Info : ", message, args);
    va_end(args);
}



/*
 * prints a warning message
 */
void log_warn(const char *message, ...){
    va_list args;
    va_start(args, message);
    write_log_message(LOG_WARN, "warn : ", message, args);
    va_end(args);
}



/*
 * prints an error message
 */
void log_error(const char *message, ...){
    va_list args;
    va_start(args, message);
    write_log_message(LOG_ERROR, "error : ", message, args);
    va_end(args);
}

/*
 * prints a small help message explaining how to use the logger
 */
void log_help(void) {
    printf("\n");
    printf("Log usage :\n");
    printf("    log_set_level(LOG_DEBUG / LOG_INFO / LOG_WARN / LOG_ERROR);\n");
    printf("    log_debug('message  '): \n");
    printf("    log_info('message   '):\n");
    printf("    log_warn(('message  '): \n");
    printf("    log_error('message  '): \n");
    printf("levels:\n");
    printf("    LOG_DEBUG = 0\n");
    printf("    LOG_INFO  = 1\n");
    printf("    LOG_WARN  = 2\n");
    printf("    LOG_ERROR = 3\n");
}
