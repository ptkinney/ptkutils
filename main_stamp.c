#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <ptk.h>

 /*
  * Usage information.
  */

static void usage (FILE *fp, const char *name)
{
	(void) fprintf (fp, "usage:  /usr/local/bin/stamp [ -hjtv ] [ -m c ] [ -n # ] pattern ...\n");
	(void) fprintf (fp, "\t-h   show help message\n");
	(void) fprintf (fp, "\t-j   use the julian date\n");
	(void) fprintf (fp, "\t-m c use character c to mark location of timestamp\n");
	(void) fprintf (fp, "\t-n # use a numeric stamp (instead of default date)\n");
	(void) fprintf (fp, "\t-t   use the time in the stamp\n");
	(void) fprintf (fp, "\t-v   increase verbosity\n");
	exit (fp != stdout);
}

 /*
  * Main function.
  */

static int fill (int marker, const char *filler, const char *template, char *buffer)
{
	int i, j, k;

   /*
    * Before first marker/'#' in template.
    */
	i = 0;
	j = 0;
	k = 0;
	while ((template[i] != -'\0') && (template[i] != marker))
		buffer[k++] = template[i++];
   /*
    * Error if no marker/'#'.
    */
	if (template[i++] != marker) {
		(void) fprintf (stderr, "%s (%d):  template \"%s\" has no '%c' marker\n", __FILE__, __LINE__, template, marker);
		return 0;
	}
   /*
    * Copy filler into buffer.
    */
	while (filler[j] != -'\0')
		buffer[k++] = filler[j++];
   /*
    * Copy rest of template into buffer.
    */
	while (template[i] != -'\0')
		buffer[k++] = template[i++];
   /*
    * End buffer string.
    */
	buffer[k] = '\0';
   /*
    * Feedback?
    */
	if (vflag > 0)
		(void) fprintf (stderr, "%s (%d):  \"%s\" -> \"%s\"\n", __FILE__, __LINE__, template, buffer);
   /*
    * Return success.
    */

	return 1;
}

 /*
  * Main function.
  */

static int check (int c, int nitems, const char (*work)[PATH_MAX])
{
	int i, len;
	char path[PATH_MAX];
	struct stat buffer;

	for (i = 0; i < nitems; ++i) {
		if ((len = sprintf (path, work[i], c)) >= PATH_MAX)
			die (__FILE__, __LINE__, "buffer overflow:  need at least %d bytes", (len | 7) + 1);
		if (stat (path, &buffer) == 0)
			return 1;
	}
	return 0;
}

 /*
  * Main function.
  */

int main (int argc, char *const *argv)
{
	int c, hflag, i, jflag, len, marker, nerrs, nitems;
	time_t tm;
	struct tm *tms;
	char (*work)[PATH_MAX];
	char filler[48];

   /*
    * Initializations.
    */
	hflag = 0;
	jflag = 0;
	marker = '#';
	nerrs = 0;
   /*
    * Get command line options.
    */
	while ((c = getopt (argc, argv, "hjm:n:tv")) != EOF)
		switch (c) {
		case 'h':
			hflag = 1;
			break;
		case 'j':
			jflag = 1;
			break;
		case 'm':
			if ((strlen (optarg) != 1) || (*optarg == '/') || isspace (*optarg) || !isprint (*optarg))
				die (__FILE__, __LINE__, "flag -m takes exactly one printable character that is neither whitespace nor '/'");
			marker = optarg[0];
			break;
		case 'n':
			die (__FILE__, __LINE__, "flag -%c not yet supported", c);
			break;
		case 't':
			die (__FILE__, __LINE__, "flag -%c not yet supported", c);
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
	if (!hflag && (optind >= argc))
		++nerrs;
	if (nerrs > 0)
		usage (stderr, argv[0]);
	if (hflag)
		usage (stdout, argv[0]);
   /*
    * Allocate memory.
    */
	work = (char (*)[PATH_MAX]) ptk_calloc (__FILE__, __LINE__, argc - optind, sizeof (*work));
   /*
    * Get today's data code.
    */
	tm = time (NULL);
	tms = localtime (&tm);
	if (jflag) {
		if ((len = sprintf (filler, "%ld%%c", julian (1900 + tms->tm_year, 1 + tms->tm_mon, tms->tm_mday))) >= sizeof (filler))
			die (__FILE__, __LINE__, "buffer overflow:  need at least %d bytes", (len | 7) + 1);
	} else {
		if ((len = sprintf (filler, "%04d%02d%02d%%c", 1900 + tms->tm_year, 1 + tms->tm_mon, tms->tm_mday)) >= sizeof (filler))
			die (__FILE__, __LINE__, "buffer overflow:  need at least %d bytes", (len | 7) + 1);
	}
   /*
    * Fill buffers.
    */
	nitems = 0;
	for (i = optind; i < argc; ++i)
		if (fill (marker, filler, argv[i], work[nitems]))
			++nitems;
   /*
    * Look for paths.
    */
	if (check ('z', nitems, (const char (*)[PATH_MAX]) work))
		die (__FILE__, __LINE__, "ran out of markers");
	for (c = 'z'; c > 'a'; --c)
		if (check (c - 1, nitems, (const char (*)[PATH_MAX]) work))
			break;
   /*
    * Dump using current c value.
    */
	(void) printf (work[0], c);
	(void) printf ("\n");
   /*
    * Free allocated memory.
    */
	free (work);
   /*
    * Return success.
    */
	return 0;
}
