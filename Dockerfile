FROM ubuntu:18.04

RUN apt update && \
    apt install -y build-essential cmake python3-dev libssl-dev libcurl4-gnutls-dev git wget

RUN cd /tmp && \
    wget http://downloads.sourceforge.net/project/boost/boost/1.67.0/boost_1_67_0.tar.gz && \
    tar zxfv boost_1_67_0.tar.gz && \
    cd boost_1_67_0 && \
    ./bootstrap.sh --prefix=/usr  && \
    ./b2 install --prefix=/usr --with=all -j4  && \
    ldconfig && \
    apt clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ADD . /app
WORKDIR /app

RUN git --version

RUN cd /app/vendor/librethinkdbxx && \
    make && \
    cd /app && \
    mkdir -p build && \
    cd build && \
    cmake ../ && \
    make -j4 && \

    apt autoremove -y --purge build-essential cmake python3-dev libssl-dev libcurl4-gnutls-dev git wget && \
    apt clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

#ENTRYPOINT [ "/app/build/indexer/mile-indexer" ]
#CMD [ "--config /app/config.yaml" ]

