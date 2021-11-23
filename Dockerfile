FROM ubuntu:latest

ARG BINUTILS_VERSION=2.37
ARG GCC_VERSION=11.2.0
ARG TARGET=i686-elf

ENV PREFIX="/opt/cross"
ENV TARGET=${TARGET}
ENV PATH="$PREFIX/bin:$PATH"

RUN apt-get update && export DEBIAN_FRONTEND=noninteractive && apt-get -y install --no-install-recommends \
	software-properties-common \
	sudo \
	wget \
	make \
	build-essential \
	bison \
	flex \
	libgmp3-dev \
	libmpc-dev \
	libmpfr-dev \
	texinfo \
	libisl-dev \
	qemu-system-x86 \
	nasm \
	parted

RUN mkdir -p src && cd src && \
	wget https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz && \
	tar -xzvf binutils-${BINUTILS_VERSION}.tar.gz && \
	mkdir -p /src/binutils && cd /src/binutils && \
	../binutils-${BINUTILS_VERSION}/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror && \
	make && \
	make install && cd /src && mkdir -p gcc && \
	wget https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz && \
	tar -xzvf gcc-${GCC_VERSION}.tar.gz && \
	cd gcc && \
	../gcc-${GCC_VERSION}/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers && \
	make all-gcc && \
	make all-target-libgcc && \
	make install-gcc && \
	make install-target-libgcc

WORKDIR /momo-bootloader

CMD ["bash"]
