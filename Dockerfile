FROM milecore/boost:1.67

RUN apt update && \
    apt install -y build-essential cmake python3-dev libssl-dev libcurl4-gnutls-dev git

ADD . /app
WORKDIR /app

COPY docker-entrypoint.sh /usr/local/bin/
RUN cd /app/vendor/librethinkdbxx && \
    make && \
    cd /app && \
    mkdir -p build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release ../ && \
    make && \
    chmod +x /usr/local/bin/docker-entrypoint.sh && \

    apt autoremove -y --purge build-essential cmake python3-dev libssl-dev libcurl4-gnutls-dev git && \
    apt clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ENTRYPOINT [ "docker-entrypoint.sh" ]
CMD [ "" ]

