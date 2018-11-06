#!/bin/bash

VER=$(git log -1 --format=%h)
echo "VER=$VER" > .env

mkdir -p etc
cp backend/config.yaml etc/explorer.yaml
cp indexer/config.yaml etc/indexer.yaml

sed -i 's/db_host.*/db_host: "rethinkdb"/g' etc/explorer.yaml
sed -i 's/db_host.*/db_host: "rethinkdb"/g' etc/indexer.yaml

docker build -t mile-explorer:$VER .
