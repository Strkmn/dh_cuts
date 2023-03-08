/* See LICENSE file for copyright and license details.
 * 
 * A test suite for dh_cuts, implemented with dh_cuts.
 * Weirdness ensues.
 */

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define DH_IMPLEMENT_HERE
#include "dh_cuts.h"

static int
spawn_test(const char *description, int should_crash)
{
	pid_t pid;
	int test_status;
	dh_push("%s", description);
	pid = fork();
	if (pid == 0) {
		dh_init(fopen("/dev/null", "w"));
		return 0;
	}
	waitpid(pid, &test_status, 0);
	dh_assert(! (should_crash && test_status == 0));
	dh_assert(! ((!should_crash) && test_status != 0));
	dh_pop();
	return 1;
}

static void
test_crash_recovery(void)
{
	if (spawn_test("crash recovery", 0))
		return;
	dh_branch( raise(SIGBUS); )
	exit(0);
}

static void
test_forfeit_on_failure(void)
{
	if (spawn_test("forfeit on failure", 1))
		return;
	dh_throw("always throw");
	exit(0);
}

static void
test_stack_overflow(void)
{
	int i;
	if (spawn_test("stack overflow", 0))
		return;
	dh_branch(
		for (i = 0; i < DH_MAX_DEPTH * 10; i++)
			dh_push("step %d", i + 1);
		for (i = 0; i < DH_MAX_DEPTH * 10; i++)
			dh_pop();
	)
	exit(0);
}

int
main()
{
	dh_init(stdout);
	dh_push("dh_cuts self-tests");
	dh_branch( test_crash_recovery(); )
	dh_branch( test_forfeit_on_failure(); )
	dh_branch( test_stack_overflow(); )
	dh_pop();
	dh_summarize();
	return EXIT_SUCCESS;
}

