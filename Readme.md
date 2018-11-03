# MILE Blockchain Explorer Index and Backend JSON-RPC API
 
## Requirements
1. c++17
1. cmake >= 3.9
1. boost >= 1.66.0 (do not update to 1.68!)
1. openssl == 1.0.2p
1. [Docker](https://www.docker.com/)
1. [RethinkDB](https://rethinkdb.com)
1. [Mile CSA API](https://github.com/mile-core/mile-csa-api)
1. [Mile CSA JSON-RPC API](https://github.com/mile-core/mile-csa-jsonrpc-client)

## Build on Ubuntu 18.04 (boost version must be updated to 1.66.0-1.67.0)

    $ git clone https://github.com/mile-core/mile-explorer
    $ cd mile-explorer
    $ git submodule update --init --recursive --remote
    $ cd vendor/librethinkdbxx && make; cd ../..;
    $ mkdir build; cd build; cmake ..; make -j4
    
## Build with user defined boost and open ssl paths
    $ cmake CMakeLists.txt  -DBoost_NO_BOOST_CMAKE=TRUE -DBoost_NO_SYSTEM_PATHS=TRUE -DBOOST_ROOT=/home/USER1/boost_1_67_0/ -DBoost_LIBRARY_DIRS=/home/USER1/boost_1_67_0/stage/lib/ -DOPENSSL_INCLUDE_DIR=/home/USER1/openssl-1.0.2p/include -DOPENSSL_SSL_LIBRARY=/home/USER1/openssl-1.0.2p/libssl.so -DOPENSSL_CRYPTO_LIBRARY=/home/USER1/openssl-1.0.2p/libcrypto.so ..
    $ make -j4

## Boost updates (if it needs)

    $ wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.gz
    $ tar -xzf boost_1_*
    $ cd boost_1_*
    $ ./bootstrap.sh --prefix=/usr
    $ sudo ./b2 install --prefix=/usr --with=all -j4

## Tested
1. Ubuntu 18.04
1. OSX 10.13

## Install RethinkDB in [rethinkdb docker](https://hub.docker.com/r/library/rethinkdb/)

    $ docker pull rethinkdb
    $ mkdir rethinkdb; cd rethinkdb
    $ docker run --name mile-rethink -v "$PWD:/data" -d rethinkdb
    # open in browser http://localhost:8080

# MILE Explorer JSON-RPC API

## Help

###### http request 
```
POST http://localhost:8042/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"help","params":{},"id":"10"}

```

## Ping

###### http request 
```
POST http://localhost:8042/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"ping","params":{},"id":"10"}

```

## Explore blocks

**Get all allowed methods**

###### http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"help","params":{},"id":"0"}
```
###### http response
```
{"id":"12","jsonrpc":"2.0","result":["get-block","get-block-history","get-block-history-state","get-transaction","get-wallet-history-blocks","get-wallet-history-state","get-wallet-history-transactions","help"]}
```

**Get chains history state**

###### http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache
   
{"jsonrpc":"2.0","method":"get-block-history-state","params":{},"id":"1"}   
```
###### http response
```   
    {"id":"1","jsonrpc":"2.0","result":{"count":11,"first":0}}
```

**Get chains history from block *first* with *limit* of history length**

###### http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"get-block-history","params":{"first":10, "limit":1},"id":"2"}
```
###### http response
```
{
	"id": "2",
	"jsonrpc": "2.0",
	"result": [{
		"block-id": 10,
		"id": "10",
		....
		"transactions": [{
			"amount": "10",
			"assetCode": "0",
			"code": "10",
		...
		}, {
			
		}],
		"version": "1"
	}]
}
```

**Get certain block from chain by *id*. This is equal *get-block-history* with *limit:1* but more efficient**

###### http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"get-block","params":{"id":0},"id":"0"}
```
###### http response
```
{
	"id": "0",
	"jsonrpc": "2.0",
	"result": {
		"block-id": 2,
		"id": "2",
		...],
		"version": "1"
	}
}
```

**Get *wallet* block *id*s for a known *public-key***

###### http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"get-wallet-history-blocks","params":{"public-key":"zVG4iPaggWUUaDEkyEyFBv8dNYSaFMm2C7WS8nSMKWLsSh9x", "first": 0, "limit": 2},"id":"12"}
```
###### http response
```
{"id":"12","jsonrpc":"2.0","result":{"blocks":["1","2"]}}
```

## Explore wallet and transactions

**Get *wallet* history state for a known *public-key***

###### http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"get-wallet-history-state","params":{"public-key":"2cv1HqiCE8BeHc4Y7vucckMysx3PV7KND2mQiMdXqDJg812Fd5"},"id":"12"}

```
###### http response
```
{"id":"12","jsonrpc":"2.0","result":{"block":{"count":3,"first":0},"transaction":{"count":3,"first":0}}}
```

**Get *wallet* transaction history for the known *public-key* from *first* with *limit* of transactions list**

###### http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"get-wallet-history-transactions","params":{"public-key":"2cv1HqiCE8BeHc4Y7vucckMysx3PV7KND2mQiMdXqDJg812Fd5", "first": 0, "limit": 3},"id":"12"}
```

###### http response
```
{
	"id": "12",
	"jsonrpc": "2.0",
	"result": {
		"transactions": [{
				"public-key": "111111111111111111111111111111115RyRTN",
				"id": 20
			},
			{
				"public-key": "111111111111111111111111111111115RyRTN",
				"id": 21
			},
			{
				"public-key": "2cv1HqiCE8BeHc4Y7vucckMysx3PV7KND2mQiMdXqDJg812Fd5",
				"id": 1
			}
		]
	}
}
```

**Get certain transactions for the known wallet with *public-key* and *id***

###### http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"get-transaction","params":{"public-key":"2XEbswkyZJVNg1ayvyt4XNrJ4Eb41tNdPUdEhnMJpvTKVxgKSc", "id":1},"id":"10"}
```
###### http response
```
{
    "id": "1",
    "jsonrpc": "2.0",
    "result": {
        "asset": [
            {
                "amount": "10000.00",
                "code": "0"
            }
        ],
        "block-id": 404,
        "description": "",
        "fee": "0.00",
        "from": "25XmmZXDXbYrQMgSXQcCdcGvjec6iSR4hQ5AxfQJMC8NkMdGzZ",
        "id": "25XmmZXDXbYrQMgSXQcCdcGvjec6iSR4hQ5AxfQJMC8NkMdGzZ:10675441032758331955",
        "to": "2bTcCpZkXgtTcHokTH1iM8kwioVK85TdCwDvUH8c37g4CjiNti",
        "transaction-id": 1.0675441032758331e+19,
        "transaction-type": "TransferAssetsTransaction"
    }
}

```