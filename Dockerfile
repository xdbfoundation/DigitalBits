FROM ubuntu

WORKDIR /opt/digitalbits-core

COPY . .

ENV CC=clang-8
ENV CXX=clang++-8
ENV CFLAGS="-O3 -g1 -fno-omit-frame-pointer"
ENV CXXFLAGS="$CFLAGS -stdlib=libc++" 

RUN apt-get update
RUN apt-get install -y software-properties-common
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt-get update

RUN apt-get install -y git \
                    build-essential pkg-config \
                    autoconf automake libtool \
                    bison flex libpq-dev \
                    libunwind-dev parallel \
                    clang-8 \
                    gcc-6 g++-6 cpp-6 \
                    libc++-8-dev libc++abi-8-dev

RUN ./autogen.sh
RUN ./configure

# make -jN where N is amount of your cpu cores - 1
RUN make 
RUN make install
