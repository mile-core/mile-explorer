FROM milecore/boost:1.67

ADD . /build
COPY docker-entrypoint.sh /usr/local/bin/

RUN apt update && \
    apt install -y build-essential cmake python3-dev libssl-dev libcurl4-gnutls-dev git && \

    cd /build/vendor/librethinkdbxx && \
    make && \
    mkdir -p /build/build && \
    cd /build/build && \
    cmake -DCMAKE_BUILD_TYPE=Release ../ && \
    make && \

    mkdir -p /app/backend /app/indexer /app/etc && \
    cp /build/build/indexer/mile-indexer /app/indexer && \
    cp /build/indexer/config.yaml /app/etc/indexer.yaml && \

    cp /build/build/backend/mile-explorer-backend /app/backend && \
    cp /build/backend/config.yaml /app/etc/explorer.yaml && \

    chmod +x /usr/local/bin/docker-entrypoint.sh && \

    apt autoremove -y --purge build-essential cmake libssl-dev libcurl4-gnutls-dev git && \
    apt clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* /build

WORKDIR /app

ENTRYPOINT [ "docker-entrypoint.sh" ]
CMD [ "" ]

