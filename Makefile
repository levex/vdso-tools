all: extract-vdso extract-vvar

extract-vdso: extract-vdso.c
	gcc extract-vdso.c -o extract-vdso

extract-vvar: extract-vvar.c
	gcc extract-vvar.c -o extract-vvar

clean:
	-@rm extract-vdso extract-vvar >/dev/null 2>&1 || true

.PHONY: all clean
