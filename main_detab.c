#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <ptk.h>

   /*
    * Output buffer.
    */

static int nout = 0;
#ifdef  DEBUG
static char out[8] = { 0 };
#else
static char out[8192] = { 0 };
#endif

  /*
   * Usage information.
   */

static void usage (FILE *fp, const char *name)
{
	(void) fprintf (fp, "usage:  %s [ -h ] [ -v... ] < input_file\n", name);
	exit (fp != stdout);
}

  /*
   * Decode characters.
   */

static const char *decode (char c, char *work)
{
	switch (c) {
	case '\b':
		return strcpy (work, "\\b");
	case '\n':
		return strcpy (work, "\\n");
	case '\r':
		return strcpy (work, "\\r");
	case '\t':
		return strcpy (work, "\\t");
	case '\\':
		return strcpy (work, "\\\\");
	default:
		work[0] = c;
		work[1] = '\0';
		return work;
	}
}

  /*
   * Record an action.
   */

static void record (const char *file, int line, const char *direction, int c, int count, const char *name, const char *buffer)
{
	int i;
	char temp[8];

	(void) fprintf (stderr, "%s (%d):  %s '%s'", file, line, direction, decode (c, temp));
	(void) fprintf (stderr, " n%s = %d, %s = \"", name, count, name);
	for (i = 0; buffer[i]; ++i)
		(void) fprintf (stderr, "%s", decode (buffer[i], temp));
	(void) fprintf (stderr, "\"\n");
}

   /*
    * Dump and clear output buffer.
    */

static void dump ()
{
	ssize_t delta, nwritten;

	if (nout > 0) {
		nwritten = 0;
		do {
			if ((delta = write (STDOUT_FILENO, out + nwritten, nout - nwritten)) <= 0)
				die (__FILE__, __LINE__, NULL);
			nwritten += delta;
		} while (nwritten < nout);
		nout = 0;
	}
	(void) memset (out, 0, sizeof (out));
}

   /*
    * Stage a character for output.
    */

static void stage (int c)
{
	if (nout >= sizeof (out))
		dump ();
	out[nout++] = c;
	if (vflag > 0)
		record (__FILE__, __LINE__, "output", c, nout, "out", out);
}

   /*
    * Main routine.
    */

int main (int argc, char *const *argv)
{
	int c, column, hflag, i, nerrs, nspaces;
	ssize_t nread;
#ifdef  DEBUG
	char in[8];
#else
	char in[1024];
#endif

   /*
    * Initializations.
    */
	hflag = 0;
	nerrs = 0;
   /*
    * Get command line options.
    */
	while ((c = getopt (argc, argv, "hv")) != EOF)
		switch (c) {
		case 'h':
			hflag = 0;
			break;
		case 'v':
			++vflag;
			break;
		default:
			++nerrs;
			break;
		}
   /*
    * Feedback?
    */
	if (nerrs > 0)
		usage (stderr, argv[0]);
	if (hflag)
		usage (stdout, argv[0]);
   /*
    * Initialize input buffer and input buffer index.
    */
	i = 0;
	if ((nread = read (STDIN_FILENO, memset (in, 0, sizeof (in)), sizeof (in))) <= 0)
		die (__FILE__, __LINE__, NULL);
   /*
    * Cycle through input.
    */
	column = 0;
	nspaces = 0;
	while (nread > 0) {
		if (vflag > 0)
			record (__FILE__, __LINE__, "input", in[i], nread, "in", in);
		switch (in[i]) {
		case '\n':
			stage ('\n');
			column = 0;
			nspaces = 0;
			break;
		case ' ':
			++column;
			++nspaces;
			break;
		case '\t':
			do {
				++column;
				++nspaces;
			} while ((column % 8) != 0);
			break;
		default:
			while (nspaces-- > 0)
				stage (' ');
			++column;
			nspaces = 0;
			stage (in[i]);
			break;
		}
		++i;
		if (i >= nread) {
			i = 0;
			nread = read (STDIN_FILENO, memset (in, 0, sizeof (in)), sizeof (in));
		}
	}
   /*
    * Flush output buffer.
    */
	dump ();
   /*
    * Return success.
    */
	return 0;
}
