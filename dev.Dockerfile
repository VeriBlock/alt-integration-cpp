FROM ubuntu:18.04
RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        gpg \
        gpg-agent \
        wget \
        software-properties-common \
        curl \
        python3 \
        python3-pip \
        python3-setuptools \
    && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    add-apt-repository -y "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-9 main" && \
    apt-add-repository -y ppa:bitcoin/bitcoin && \
    add-apt-repository -y ppa:ubuntu-toolchain-r/test

# install dependencies
RUN apt-get update && apt-get upgrade -y && \
    apt-get install --no-install-recommends -y \
        libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools \
        libqrencode-dev \
        libprotobuf-dev protobuf-compiler \
        libzmq3-dev \
        libdb4.8-dev \
        libdb4.8++-dev \
        libzmq3-dev \
        libminiupnpc-dev \
        build-essential \
        libtool \
        autotools-dev \
        automake \
        pkg-config \
        libssl-dev \
        libevent-dev \
        bsdmainutils \
        libb2-dev

# install tools
RUN apt-get install --no-install-recommends -y \
        libz-dev \
        xz-utils \
        gcc-9 \
        g++-9 \
        llvm-9 \
        clang-9 \
        clang-tidy-9 \
        clang-format-9 \
        git \
        ccache \
        lcov \
        vim \
        unzip \
        cmake \
    && rm -rf /var/lib/apt/lists/*

# set default compilers and tools
RUN update-alternatives --install /usr/bin/gcov         gcov         /usr/bin/gcov-9               90 && \
    update-alternatives --install /usr/bin/gcc          gcc          /usr/bin/gcc-9                90 && \
    update-alternatives --install /usr/bin/g++          g++          /usr/bin/g++-9                90 && \
    update-alternatives --install /usr/bin/clang        clang        /usr/lib/llvm-9/bin/clang-9   90 && \
    update-alternatives --install /usr/bin/clang++      clang++      /usr/bin/clang++-9            90 && \
    update-alternatives --install /usr/bin/clang-tidy   clang-tidy   /usr/bin/clang-tidy-9         90 && \
    update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-9       90 && \
    update-alternatives --install /usr/bin/python       python       /usr/bin/python3              90

WORKDIR /tmp

RUN mkdir -p boost && \
    ( \
      cd boost; \
      wget https://dl.bintray.com/boostorg/release/1.65.1/source/boost_1_65_1.tar.gz; \
      tar -zxf boost_1_65_1.tar.gz; \
      cd boost_1_65_1/; \
      ./bootstrap.sh; \
	  export PYTHON_VERSION=3.6; \
	  export PYTHON_ROOT=/usr; \
	  cd libs/python/build; \
	  sudo ./bjam cxxflags=-fPIC; \
	  sudo cp -df bin-stage/libboost_python.so* /usr/local/lib; \
      cd ../../..; \
	  sudo cp -rf boost /usr/local/include; \
    ) && \
    rm -rf boost

RUN ldconfig

ENV SONAR_CLI_VERSION=4.2.0.1873
RUN set -e; \
    mkdir -p /opt/sonar; \
    curl -L -o /tmp/sonar.zip https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-${SONAR_CLI_VERSION}-linux.zip; \
    unzip -o -d /tmp/sonar-scanner /tmp/sonar.zip; \
    mv /tmp/sonar-scanner/sonar-scanner-${SONAR_CLI_VERSION}-linux /opt/sonar/scanner; \
    ln -s -f /opt/sonar/scanner/bin/sonar-scanner /usr/local/bin/sonar-scanner; \
    rm -rf /tmp/sonar*

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
RUN pip3 install --upgrade setuptools wheel bashlex compiledb gcovr

WORKDIR /