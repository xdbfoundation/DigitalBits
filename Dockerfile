FROM ubuntu

WORKDIR /opt/digitalbits-core

COPY . .

RUN apt update && apt-get install -y git \
                    build-essential pkg-config \
                    autoconf automake libtool \
                    bison flex libpq-dev \
                    libunwind-dev parallel \
                    gcc-6 g++-6 cpp-6

RUN ./autogen.sh
RUN ./configure

# make -jN where N is amount of your cpu cores - 1
RUN make -j3 
RUN make install
