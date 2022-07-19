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

#ifndef	LIBAGGROCOW_H
#define	LIBAGGROCOW_H	1

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

enum ac_rc
{
	/* Success */
	AC_OK	= 0,
	/* General error */
	AC_FAIL,
	/* Data format error */
	AC_DATAERR,
	/* Cannot open input */
	AC_NOINPUT,
	/* Input/Output error */
	AC_IOERR,
	/* Configuration error */
	AC_CONFIG,
	/* Invalid value(s) provided by the caller */
	AC_EINVAL,
	/* System error */
	AC_OSERR
};

enum ac_status
{
	/* Success */
	AC_STATUS_OK	= 0,
	/* One or more processes failed */
	AC_STATUS_INCOMPLETE
};

/* Structure representing the result of a single test case */
struct ac_test_case_result
{
	/* Largest Minimum Distance for allocating all cows of a given test case. */
	unsigned long int lmd;
};

/* Structure representing a single test case */
struct ac_test_case
{
	/* Number of stalls in the list of stall indices */
	size_t				 tc_nstalls;
	/* Number of cows to allocate */
	unsigned long int		 tc_ncows;
	/* List of stall indices available for cow placement */
	unsigned long int		*tc_stalls;
	/* Result of processing the test case */
	struct ac_test_case_result	 tc_result;
};

/* A type signature of a function handling the results of test cases */
typedef int (*ac_test_case_result_handler_t)(size_t tcord,
		struct ac_test_case *tc, struct ac_test_case_result *tcr);

/* Structure representing the final result of the entire test case set */
struct ac_test_set_result
{
	/* Number of test cases seen in set */
	size_t ntc;
	/* Number of successfully processed test cases */
	size_t nptc;
	/* Status of the overall set completion */
	enum ac_status status;
};

/* Structure representing a collection of test cases */
struct ac_test_set
{
	/* Number of test cases in the set */
	size_t				 ts_ntc;
	/* Result of processing the test set */
	struct ac_test_set_result	 ts_result;
	/* List of test cases */
	struct ac_test_case		*ts_tcs;
	/* Original path to the test set input */
	char				*ts_inputpath;
};

/* A type signature of a function handling the results of test sets */
typedef int (*ac_test_set_result_handler_t)(struct ac_test_set *ts,
		struct ac_test_set_result *tsr);

/* Structure representing a context of a single aggrcow run */
struct ac_ctx
{
	/* List of pointers to test sets */
	struct ac_test_set		*ac_tss;
	/* Number of test sets in the list */
	size_t				 ac_nts;
	/* A pointer to a function for tests set processing result handling */ 
	ac_test_set_result_handler_t	 ac_ts_result_handler;
	/* A pointer to a function for test case processing result handling */ 
	ac_test_case_result_handler_t	 ac_tc_result_handler;
};

/* Initialize an allocated <ac_ctx> structure
 * @ctx pointer to an allocated <ac_ctx> structure
 *
 * Sets up the intial state of a context structure and sets up the default
 * handlers for handling test set and case results.
 */
void ac_ctx_init(struct ac_ctx *ctx);

/* Perform a shallow-copy of a test set to the list of test sets in a <ac_ctx> structure
 * @ctx pointer to an allocated <ac_ctx> structure
 * @ts pointer to an <ac_test_set> to add to the list of test sets
 *
 * The given <ac_test_set> structure is copied field-for-field into the
 * contexts storage. The associated test cases of the set are *not* copied,
 * only the references are, hence they should *not* be free()'d while the test
 * set or the context object is still necessary.
 *
 * Note: the *_destroy() family of functions will recursively deallocate all
 * the associated resources. This leaves the user with their own copy of the
 * structure that they should deallocate themselves, without free()'ing its
 * resources.
 *
 * @return <AC_EINVAL> if either <ctx> or <ts> is a NULL pointer,
 *         <AC_OSERR> upon failure to add the test case,
 *         <AC_OK> otherwise.
 */
enum ac_rc ac_ctx_add_test_set(struct ac_ctx *ctx, struct ac_test_set *ts);

/* Processes all test sets currently assigned to the context
 * @ctx pointer to a <ac_ctx> structure with assigned test cases.
 *
 * Iterates over the list of known test sets and processes each in turn.
 * The processing stops upon the first failed test set.
 *
 * @return <AC_OK> upon success; upon failure, returns just like
 *         <ac_test_case_process>.
 */
enum ac_rc ac_ctx_process_test_sets(struct ac_ctx *ctx);

/* Processes the results of test sets of a given context object
 * @ctx pointer to an <ac_ctx> object
 *
 * Invokes the associated <ac_ts_result_handler> of the given context for every
 * test set of the given context object, if it is set. If the handler returns
 * success (0), and an <ac_tc_result_handler> for the given context is set,
 * invokes that in turn for every <ac_test_case> of the test set.
 *
 * Note: if the context has no test set result handler installed, the test case
 * result handler is *not* invoked.
 */
void ac_ctx_process_results(struct ac_ctx *ctx);

/* Destroy an allocated <ac_ctx> structure
 * @ctx pointer to an allocatad <ac_ctx> structure
 *
 * Deallocates any resources associated with the context object, including test
 * sets and any test cases, as well as removes any installed result handlers.
 * The resulting context object is *not* reusable without prior
 * reinitialization via a call to <ac_ctx_init>.
 *
 * In the case that <ctx> is a NULL pointer, gracefully returns.
 */
void ac_ctx_destroy(struct ac_ctx *ctx);

/* Builds an <ac_test_set> in <ts> by reading from a file at <path>
 * @path path to a file on the local file system containing the test set data
 * @ts pointer to an allocated <ac_test_set> structure to hold the test data
 *
 * @return <AC_EINVAL> in the case that <path> or <ts> are NULL pointers, or
 *         <path> is an empty string,
 *         <AC_NOINPUT> in the case that <path> cannot be opened for reading,
 *         <AC_OSERR> in the case of failing to allocate memory for the
 *         tracking of the original input file path, as provided by the caller,
 *         <AC_OK> otherwise.
 */
enum ac_rc ac_test_set_from_path(const char *path, struct ac_test_set *ts);

/* Process test cases of a given test set
 * @ts pointer to an instance of <ac_test_set>
 *
 * Processes a given test sets associated test cases in turn. Terminates
 * prematurely upon the first failure to process a test case. Populates the
 * associated <ac_test_set_result> of the given test set.
 *
 * @return <AC_OK> upon success; upon failure, returns just like
 *         <ac_test_case_process>.
 */
enum ac_rc ac_test_set_process(struct ac_test_set *ts);

void ac_test_set_destroy(struct ac_test_set *ts);

/* Assemble an instance of struct <ac_test_case> from its constituent parts
 * @nstalls number of stalls available for cow placement
 * @ncows number of cows available for placement
 * @stalls pointer to an array, of length <nstalls>, of available stall indices
 * @tc pointer to an instance of struct <ac_test_case> to fill with test data
 *
 * A utility function to tersely compose a <ac_test_case> structure.
 * Clears the memory pointed to by <tc> and writes the test case data
 * given by the caller.
 *
 * @return <AC_EINVAL> if any of <nstalls> or <ncows> is 0, or <stalls> or
 *         <tc> are NULL pointers, otherwise <AC_OK>.
 */
enum ac_rc ac_test_case_from_parts(unsigned long int nstalls,
		unsigned long int ncows, unsigned long int *stalls,
		struct ac_test_case *tc);

/* Process a given test case
 * @tc pointer to an instance of <ac_test_case>
 *
 * Processes the test case given in <tc> and, upon success, writes the result
 * to the associated <ac_test_case_result> structure.  
 *
 * @return <AC_EINVAL> if <tc> is a NULL pointer, <AC_OK> otherwise.
 */
enum ac_rc ac_test_case_process(struct ac_test_case *tc);

/* Destroy an allocated <ac_test_case> structure
 * @tc pointer to an instance of <ac_test_case>
 *
 * Deallocates any resources associated with the test case object and clears
 * the memory pointed to by <tc>.
 *
 * In the case that <tc> is a NULL pointer, gracefully returns.
 */
void ac_test_case_destroy(struct ac_test_case *tc);

/* Return string describing the given return code 
 * @rc a variant of the <ac_rc> enumerator
 *
 * The returned string is statically allocated and should *not* be free()'d by
 * the caller. In the case of an unrecognized <ac_rc> variant, the string
 * "Unknown error" is returned.
 */
const char *ac_strrc(enum ac_rc rc);

#endif /* !LIBAGGROCOW_H */
