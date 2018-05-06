/*
 *
 * A tool to extract the vvar region from the current process (this executable).
 * Author: Levente Kurusa <levex@linux.com>
 * License: MIT License
 *
 * Compile:
 *   $ gcc extract-vvar.c -o extract-vvar
 *
 * Usage:
 *   $ ./extract-vvar > vvar-sample.blob
 *
 */

#include <sys/auxv.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int
main(int argc, char **argvp)
{
	void *vdso, *ptr, *vvar_end, *vvar_start;
	int rc, page_size;

	vdso = (void *) getauxval(AT_SYSINFO_EHDR);
	if (!vdso) {
		fprintf(stderr, "[  ERROR ] vDSO region not found\n");
		return 1;
	}

	page_size = getauxval(AT_PAGESZ);
	if (!page_size) {
		page_size = 0x1000;
		fprintf(stderr, "[  WARN  ] AT_PAGESZ was not passed, assuming page size of 4K\n");
	}

	fprintf(stderr, "[  INFO  ] vDSO region at %p\n", vdso);

	/* the vvar region is always directly before the vDSO region */
	vvar_end = vdso;

	/* Find the start of the vvar region by looking for ENOMEM from msync(2) */
	for (vvar_start = vvar_end;; vvar_start -= page_size) {
		rc = msync(vvar_start, page_size, MS_SYNC);
		if (!rc)
			continue;

		if (errno == ENOMEM) {
			vvar_start += page_size;
			fprintf(stderr, "[  INFO  ] vvar region at [%p - %p]\n", vvar_start, vvar_end);
			break;
		} else {
			fprintf(stderr, "[  ERROR ] Unexpected error: %s\n", strerror(errno));
			return errno;
		}
	}

	for (ptr = vvar_start; ptr != vvar_end; ptr += page_size) {
		rc = write(1, ptr, page_size);
		if (rc == page_size)
			continue;
		else {
			fprintf(stderr, "[  INFO  ] Done, good-bye\n");
			return 0;
		}
	}

	fprintf(stderr, "[  WARN  ] This was not meant to happen.\n");
	return 2;
}
