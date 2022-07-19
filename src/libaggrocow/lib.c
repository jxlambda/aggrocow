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

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "aggrocow.h"

static int compar_uli(const void *a, const void *b)
{
	unsigned long int x = *(unsigned long int *)a;
	unsigned long int y = *(unsigned long int *)b;

	if (x < y)
		return -1;
	else if (x > y)
		return 1;

	return 0;
}

static bool can_distribute_cows_at_min_distance(unsigned long int *stalls, size_t nstalls,
		unsigned long int ncows, unsigned long min_distance)
{
	unsigned long int ncows_alloc, prev_stall, curr_stall;
	size_t i;

	/* Start by always placing a cow in the first available stall */
	ncows_alloc = 1;
	prev_stall = stalls[0];

	for (i = 1; i < nstalls; i++)
	{
		curr_stall = stalls[i];

		if (min_distance > curr_stall - prev_stall)
			continue;

		ncows_alloc++;

		if (ncows_alloc == ncows)
			return true;

		prev_stall = curr_stall;
	}

	return false;
}

static unsigned long int find_largest_min_cow_dist(unsigned long int *stalls, size_t nstalls,
		unsigned long int ncows)
{
	unsigned long int lbound, rbound, m;

	lbound = 0;
	rbound = stalls[nstalls - 1];

	do
	{
		m = (lbound + rbound) / 2;

		if (true == can_distribute_cows_at_min_distance(stalls,
				nstalls, ncows, m))
		{
			lbound = m + 1;
		}
		else
			rbound = m;
	}
	while (lbound < rbound);

	return lbound - 1;
}

static void test_case_from_parts(size_t nstalls, unsigned long int ncows,
		unsigned long int *stalls, struct ac_test_case *tc)
{
	memset(tc, 0, sizeof(*tc));

	tc->tc_nstalls = nstalls;
	tc->tc_ncows = ncows;
	tc->tc_stalls = stalls;
}

static enum ac_rc test_case_from_file(FILE *fp, struct ac_test_case *tc)
{
	int rc;
	enum ac_rc ret = AC_OK;
	size_t i, nstalls, ncows, *stalls;
	char buf[BUFSIZ];

	if (NULL == fgets(buf, sizeof(buf), fp))
	{
		if (0 != feof(fp))
			return AC_DATAERR;
		else
			return AC_IOERR;
	}

	rc = sscanf(buf, "%zu %lu", &nstalls, &ncows);
	if (EOF == rc || 2 != rc)
	{
		/*
		 *     The value EOF is returned if the end of input is
		 *     reached before either the first successful conversion
		 *     or a matching failure occurs.
		 *
		 * -- man scanf.3
		 *
		 * If we've not matched exactly our two input items,
		 * some sort of matching failure has occurred. Consider
		 * this to be an input data error for now.
		 */
		return AC_DATAERR;
	}

	stalls = (size_t *)reallocarray(NULL, nstalls, sizeof(size_t));
	if (NULL == stalls)
		return AC_OSERR;

	memset(stalls, 0, nstalls * sizeof(size_t));

	for (i = 0; i < nstalls; i++)
	{
		if (NULL == fgets(buf, sizeof(buf), fp))
		{
			if (0 != feof(fp))
				ret = AC_DATAERR;
			else
				ret = AC_IOERR;

			break;
		}

		rc = sscanf(buf, "%lu", &stalls[i]);
		if (EOF == rc || 1 != rc)
		{
			ret = AC_DATAERR;	

			break;
		}
	}

	if (AC_OK != ret)
	{
		free(stalls);

		return ret;
	}

	qsort(stalls, nstalls, sizeof(size_t), compar_uli);

	return ac_test_case_from_parts(nstalls, ncows, stalls, tc);
}

static enum ac_rc test_set_from_file(FILE *fp, struct ac_test_set *ts)
{
	int rc;
	enum ac_rc ret;
	size_t i, ncases;
	char buf[BUFSIZ];

	if (NULL == fgets(buf, sizeof(buf), fp))
	{
		if (0 != feof(fp))
		{
			/*
			 * Reaching EOF at the start of the file could only
			 * mean that the file is empty. Consider this to be
			 * invalid data provided by the user
			 */
			return AC_DATAERR;
		}
		else
			return AC_IOERR;
	}

	rc = sscanf(buf, "%zu", &ncases);
	if (EOF == rc || 1 != rc)
		return AC_DATAERR;

	/* The special case where the hobbitses try to trick us */
	if (0 == ncases)
	{
		ts->ts_ntc = 0;

		return AC_OK;
	}

	ts->ts_tcs = (struct ac_test_case *)calloc(ncases, sizeof(struct ac_test_case));
	if (NULL == ts->ts_tcs)
		return AC_OSERR;

	for (i = 0; i < ncases; i++)
	{
		struct ac_test_case *tc = &ts->ts_tcs[i];

		ret = test_case_from_file(fp, tc);

		if (AC_OK != ret)
			break;

		ts->ts_ntc++;
	}

	return ret;
}

static enum ac_rc ctx_add_test_set(struct ac_ctx *ctx, struct ac_test_set *ts)
{
	struct ac_test_set *tss, *_ts;

	/*
	 * XXX: clang-tidy thinks it's suspicious to ask for the size of a
	 * pointer here, because:
	 *
	 *     A common mistake is to compute the size of a pointer instead
	 *     of its pointee.
	 *
	 * -- https://releases.llvm.org/14.0.0/tools/clang/tools/extra/docs/clang-tidy/checks/bugprone-sizeof-expression.html#suspicious-usage-of-sizeof-a
	 *
	 * whereas here it's intentional.
	 */
	tss = (struct ac_test_set *)reallocarray(ctx->ac_tss,
			ctx->ac_nts + 1, sizeof(*ctx->ac_tss));
	if (NULL == tss)
		return AC_OSERR;

	_ts = &tss[ctx->ac_nts];

	memcpy(_ts, ts, sizeof(*_ts));

	ctx->ac_tss = tss;
	ctx->ac_nts++;

	return AC_OK;
}

enum ac_rc ac_test_case_from_parts(size_t nstalls, unsigned long int ncows,
		unsigned long int *stalls, struct ac_test_case *tc)
{
	if (0 == nstalls || 0 == ncows || NULL == stalls || NULL == tc)
		return AC_EINVAL;

	if (nstalls < ncows)
		return AC_EINVAL;

	test_case_from_parts(nstalls, ncows, stalls, tc);

	return AC_OK;
}

void ac_ctx_init(struct ac_ctx *ctx)
{
	memset(ctx, 0, sizeof(*ctx));
}

enum ac_rc ac_ctx_add_test_set(struct ac_ctx *ctx, struct ac_test_set *ts)
{
	if (NULL == ctx || NULL == ts)
		return AC_EINVAL;

	return ctx_add_test_set(ctx, ts);
}

enum ac_rc ac_test_set_from_path(const char *path, struct ac_test_set *ts)
{
	enum ac_rc ret;
	FILE *fp;

	if (NULL == path || NULL == ts)
		return AC_EINVAL;

	if (0 == strlen(path))
		return AC_EINVAL;

	memset(ts, 0, sizeof(*ts));

	ts->ts_inputpath = strdup(path);
	if (NULL == ts->ts_inputpath)
		return AC_OSERR;

	if (0 == strncmp(path, "-", strlen(path)))
		fp = stdin;
	else if (NULL == (fp = fopen(path, "r")))
		return AC_NOINPUT;

	ret = test_set_from_file(fp, ts);

	if (0 != strncmp(path, "-", strlen(path)))
		fclose(fp);

	return ret;
}

enum ac_rc ac_test_case_process(struct ac_test_case *tc)
{
	if (NULL == tc)
		return AC_EINVAL;

	tc->tc_result.lmd = find_largest_min_cow_dist(tc->tc_stalls,
			tc->tc_nstalls, tc->tc_ncows);

	return AC_OK;
}

enum ac_rc ac_test_set_process(struct ac_test_set *ts)
{
	size_t i;
	enum ac_rc ret = AC_OK;

	ts->ts_result.ntc = ts->ts_ntc;

	for (i = 0; i < ts->ts_ntc; i++)
	{
		struct ac_test_case *tc = &ts->ts_tcs[i];

		if (AC_OK != (ret = ac_test_case_process(tc)))
		{
			ts->ts_result.status = AC_STATUS_INCOMPLETE;	

			break;
		}

		ts->ts_result.nptc++;
	}

	if (AC_OK == ret)
		ts->ts_result.status = AC_STATUS_OK;

	return ret;
}

enum ac_rc ac_ctx_process_test_sets(struct ac_ctx *ctx)
{
	enum ac_rc ret = AC_OK;
	size_t i;

	for (i = 0; i < ctx->ac_nts; i++)
	{
		struct ac_test_set *ts = &ctx->ac_tss[i];

		if (AC_OK != (ret = ac_test_set_process(ts)))
			break;
	}

	return ret;
}

void ac_test_case_destroy(struct ac_test_case *tc)
{
	if (NULL == tc)
		return;

	free(tc->tc_stalls);

	memset(tc, 0, sizeof(*tc));
}

void ac_test_set_destroy(struct ac_test_set *ts)
{
	size_t i;

	if (NULL == ts)
		return;

	for (i = 0; i < ts->ts_ntc; i++)
		ac_test_case_destroy(&ts->ts_tcs[i]);

	free(ts->ts_tcs);
	free(ts->ts_inputpath);

	memset(ts, 0, sizeof(*ts));
}

void ac_ctx_destroy(struct ac_ctx *ctx)
{
	size_t i;

	if (NULL == ctx)
		return;

	for (i = 0; i < ctx->ac_nts; i++)
		ac_test_set_destroy(&ctx->ac_tss[i]);

	free(ctx->ac_tss);

	memset(ctx, 0, sizeof(*ctx));
}

void ac_ctx_process_results(struct ac_ctx *ctx)
{
	size_t i, j;

	if (NULL == ctx)
		return;

	if (NULL == ctx->ac_ts_result_handler)
		return;

	for (i = 0; i < ctx->ac_nts; i++)
	{
		struct ac_test_set *ts = &ctx->ac_tss[i];

		if (0 != ctx->ac_ts_result_handler(ts, &ts->ts_result))
			continue;

		if (NULL == ctx->ac_tc_result_handler)
			continue;

		for (j = 0; j < ts->ts_ntc; j++)
		{
			struct ac_test_case *tc = &ts->ts_tcs[j];

			ctx->ac_tc_result_handler(j + 1, tc, &tc->tc_result);
		}
	}
}

const char *ac_strrc(enum ac_rc rc)
{
	const char *ret;

	switch (rc)
	{
	case AC_OK:
		ret = "Success";
		break;
	case AC_FAIL:
		ret = "General error";
		break;
	case AC_DATAERR:
		ret = "Data format error";
		break;
	case AC_NOINPUT:
		ret = "Cannot open input";
		break;
	case AC_IOERR:
		ret = "Input/Output error";
		break;
	case AC_CONFIG:
		ret = "Configuration error";
		break;
	case AC_EINVAL:
		ret = "Invalid value(s) provided";
		break;
	case AC_OSERR:
		ret = "System error";
		break;
	default:
		ret = "Unknown error";
	}

	return ret;
}
