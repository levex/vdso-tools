/*
 *
 * A tool to extract the vDSO shared object from the current process (this executable).
 * Author: Levente Kurusa <levex@linux.com>
 * License: MIT License
 *
 * Compile:
 *   $ gcc extract-vdso.c -o extract-vdsoA
 *
 * Usage:
 *   $ ./extract-vdso > vdso-sample.so
 *
 * Read the symbols:
 *   $ readelf -s vdso-sample.so
 */

#include <sys/auxv.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int
main(int argc, char **argvp)
{
	void *vdso, *ptr;
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

	for (ptr = vdso;; ptr += page_size) {
		rc = write(1, ptr, page_size);
		if (rc == page_size)
			continue;
		else if (errno == EFAULT) {
			fprintf(stderr, "[  INFO  ] Done, good-bye\n");
			return 0;
		} else {
			fprintf(stderr, "[  ERROR ] Unexpected error: %s\n", strerror(errno));
			return errno;
		}
	}

	fprintf(stderr, "[  WARN  ] Wrote the entire contents of the RAM to memory\n");
	return 2;
}
