#!/bin/sh

usage () {
    echo "Usage:"
    echo "\t-h"
    echo "\t-a [indexer,explorer]"
    echo "\t-c /app/config.yml"
    echo ""
    exit 1
}

if [ $# -lt 1 ]; then
    usage
fi
FLAG=false
CONF=""
RUN=""

while getopts "a:c:h" opt; do
    case $opt in
        a) FLAG=true
           if [ "$OPTARG" = "indexer" ] ; then RUN=/app/build/indexer/mile-indexer; fi
           if [ "$OPTARG" = "explorer" ]; then RUN=/app/build/backend/mile-explorer-backend; fi
           if [ "$RUN" = "" ]; then usage; fi
        ;;
        c) CONF="--config=$OPTARG";;
        h) usage;;
    esac
done

if ! $FLAG; then
    echo "-a is required" >&2
    usage
fi
$RUN $CONF

