#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <argp.h>


//*****************************************************************
// argument parsing

// implementing argp library for argument parsing
const char *argp_program_version = "argp-ex3 1.0";
const char *argp_program_bug_address = "<bug-gnu-utils@gnu.org>";


struct arguments {
        int verbose;
        char *worker_path, *num_of_workers, *mechanism_for_wait, *x, *n;
};

static struct argp_option options[] = {
        {"verbose", 'v', 0, 0, "Produce verbose output"},
        {"x_value",  'x', "STRING4", 0, "value of x"},
        {"number_of_terms",  'n', "STRING5", 0, "value of n"},
        {0}
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
        struct arguments *arguments = state->input;

        switch (key)
        {
        case 'v':
                arguments->verbose = 1;
                break;
        case 'x':
                arguments->x = arg;
                break;
        case 'n':
                arguments->n= arg;
                break;
        default:
                return ARGP_ERR_UNKNOWN;
        }
        return 0;
}
static char args_doc[] = "";
static char doc[] = "Worker program";
static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv) {
        struct arguments arguments;
        // default argument values
        arguments.x = "";
        arguments.n = "";
        arguments.verbose = 0;
        argp_parse(&argp, argc, argv, 0, 0, &arguments);
        int x = atoi(arguments.x);
        int n = atoi(arguments.n);
        long double x_pow_n = 1;
        long double n_factorial = 1;
        long double result = 0.0;
        int a = n;
        if (n == 0) result = 1;
        else if(n == 1) result = x;
        else {
                while(n > 0)
                {
                        x_pow_n = x_pow_n * (long double) x;
                        n_factorial = n_factorial * (long double) n;
                        n--;
                }
                result = x_pow_n/n_factorial;
        }
        if (isatty(1)) {
                printf("%d^%d/%d!: %Lf\n", x,  a, a, result);
        }
        else {
                printf("%Lf\n",result);
        }
}
