/* SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2022 Juris Miščenko <jxlambda@protonmail.com>
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <getopt.h>
#include <stdbool.h>

#include <aggrocow.h>

#define	PROGNAME	"aggrcow"

static void usage(int) __attribute__((__noreturn__));
static void version(void) __attribute__((__noreturn__));
static int test_case_result_handler(size_t tcord, struct ac_test_case *tc,
		struct ac_test_case_result *tcr);
static int test_set_result_handler(struct ac_test_set *ts, struct ac_test_set_result *tsr);
static int verbose_test_case_result_handler(size_t tcord, struct ac_test_case *tc,
		struct ac_test_case_result *tcr);
static int verbose_test_set_result_handler(struct ac_test_set *ts, struct ac_test_set_result *tsr);

int main(int argc, char *argv[])
{
	int ret = EXIT_SUCCESS;
	int i, opt;
	bool verbose = false;
	enum ac_rc rc = AC_OK;
	struct ac_ctx ctx;
	const char *optstring = "hVv";

	while (EOF != (opt = getopt(argc, argv, optstring)))
	{
		switch (opt)
		{
		case 'h':
			usage(EXIT_SUCCESS);
		case 'V':
			version();
		case 'v':
			verbose = true;
			break;
		default:
			usage(EX_USAGE);
		}
	}

	argc -= optind;
	argv += optind;

	if (0 == argc)
		usage(EX_USAGE);

	ac_ctx_init(&ctx);

	do
	{
		for (i = 0; i < argc; i++)
		{
			const char *path = argv[i];
			struct ac_test_set ts;

			memset(&ts, 0, sizeof(ts));

			rc = ac_test_set_from_path(path, &ts);
			if (AC_OK != rc)
			{
				fprintf(stderr, "Failed to build test set from input '%s': %s\n",
						path, ac_strrc(rc));
				break;
			}

			rc = ac_ctx_add_test_set(&ctx, &ts);
			if (AC_OK != rc)
			{
				fprintf(stderr, "Failed to add test set from input '%s': %s\n",
						path, ac_strrc(rc));
				break;
			}
		}

		if (AC_OK != rc)
			break;

		if (true == verbose)
		{
			ctx.ac_tc_result_handler = verbose_test_case_result_handler;
			ctx.ac_ts_result_handler = verbose_test_set_result_handler;
		}
		else
		{
			ctx.ac_tc_result_handler = test_case_result_handler;
			ctx.ac_ts_result_handler = test_set_result_handler;
		}

		rc = ac_ctx_process_test_sets(&ctx);

		ac_ctx_process_results(&ctx);
		ac_ctx_destroy(&ctx);

		if (AC_OK != rc)
			ret = EXIT_FAILURE;
	}
	while (false);

	return ret;
}

static void usage(int ret)
{
	FILE *_output = stdout;

	if (EXIT_SUCCESS != ret)
		_output = stderr;

	fprintf(_output, "usage: %s [-h|-V] | [-v] FILE [FILE [..]]\n", PROGNAME);

	exit(ret);
}

static void version(void)
{
	exit(EXIT_SUCCESS);
}

static int test_set_result_handler(struct ac_test_set *ts __attribute__((unused)),
		struct ac_test_set_result *tsr __attribute__((unused)))
{
	return 0;
}

static int test_case_result_handler(size_t tcord __attribute__((unused)),
		struct ac_test_case *tc __attribute__((unused)),
		struct ac_test_case_result *tcr)
{
	return printf("%ld\n", tcr->lmd);
}

static int verbose_test_set_result_handler(struct ac_test_set *ts,
		struct ac_test_set_result *tsr)
{
	const char *source;

	source = (0 == strcmp(ts->ts_inputpath, "-")) ?
		"stdin" : ts->ts_inputpath;

	printf( "[*] Test source: [%s]\n"
		"[*] Test cases [total/processed]: [%zu/%zu]\n",
			source,
			tsr->ntc, tsr->nptc);

	return 0;
}

static int verbose_test_case_result_handler(size_t tcord,
		struct ac_test_case *tc __attribute__((unused)),
		struct ac_test_case_result *tcr)
{
	printf("%5zu) Largest Minimum Distance: %ld\n", tcord, tcr->lmd);

	return 0;
}
