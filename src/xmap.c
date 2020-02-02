/*
 * =====================================================================================
 *
 *       Filename:  xmap.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/10/2019 05:21:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "xopt.h"
#include "xm_context.h"
#include "xm_work.h"

static int file_exists(char *name)
{
	FILE *file = fopen(name, "r");
	if (!file)
		return 0;
	fclose(file);
	return 1;
}

int main(int argc,char **argv){

    xm_context_t *context;
    xm_work_t *work;
    xm_pool_t *mp;

	struct gengetopt_args_info args;
	struct cmdline_parser_params *params;
	params = cmdline_parser_params_create();
	params->initialize = 1;
	params->override = 0;
	params->check_required = 0;

	int config_loaded = 0;

	if (cmdline_parser_ext(argc, argv, &args, params) != 0) {
		exit(EXIT_SUCCESS);
	}
	if (args.config_given || file_exists(args.config_arg)) {
		params->initialize = 0;
		params->override = 0;
		if (cmdline_parser_config_file(args.config_arg, &args,
					       params) != 0) {
			exit(EXIT_FAILURE);
		}
		config_loaded = 1;
	}

	if (args.help_given) {
		cmdline_parser_print_help();
		exit(EXIT_SUCCESS);
	}

	if (args.version_given) {
		cmdline_parser_print_version();
		exit(EXIT_SUCCESS);
	}
	if (args.list_probe_modules_given) {
		xm_probes_dump();
		exit(EXIT_SUCCESS);
	}

	if (cmdline_parser_required(&args, CMDLINE_PARSER_PACKAGE) != 0) {
		exit(EXIT_FAILURE);
	}

    mp = xm_pool_create(4096);
    if(mp == NULL){

        fprintf(stderr,"create memory pool failed!");
        return -1;
    }

    context = xm_context_create(mp,&args);
    if(context == NULL){

        fprintf(stderr,"create context failed!");
        return -1;
    }

    work = xm_work_create(context);
    if(work == NULL){

        fprintf(stderr,"create work failed!");
        return -1;
    }

    xm_work_start(work);


    return 0;
}
