/*
 * File: main.c
 * Entry point: invokes app_run() and returns its exit code.
 * 
 * Responsibilities:
 *  - Call cli_parse to turn argc/argv into a CommandLine struct
 *  - Call app_run to dispatch to scan / trace / monitor
 *  - Return appropriate exit status
 * 
 * Author: Shan Truong - 400576105 - truons8
 * Date: December 3, 2025
 * Coures: 2XC3
 */

#include "app.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Main entry point.
 * @param argc Argument count
 * @param argv Argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
int main(int argc, char *argv[]){

    CommandLine cmd;

    /* Parse command-line arguments into cmd.
     * cli_parse will print errors and exit() on invalid input.
     * On success, it returns EXIT_SUCCESS and fills cmd.
     */
    int parse_result = cli_parse(argc, argv, &cmd);

    if(parse_result != EXIT_SUCCESS){

        /* This is mostly defensive; cli_parse already exits on error. */
        fprintf(stderr, "Error: cli_parse failed.\n");
        return EXIT_FAILURE;
    }

    /* Run the application with the parsed command-line options. */
    int app_result = app_run(&cmd);

    /* app_run returns 0 on success, non-zero on failure. */
    if(app_result != 0){
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}