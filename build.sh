export GNU_INSTALL_ROOT=/usr/local
export GNU_PREFIX=arm-none-eabi
make -e -f gcc/gcc_s130.Makefile $@

ident _build/gcc_s130_xxaa.out
