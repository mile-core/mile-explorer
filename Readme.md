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

    $ git clone https://bitbucket.org/mile-core/mile-explorer
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
2. OSX 10.13

## How to build docker container

### Build base docker image (skip the stage if the image built before)
    $ docker build -f Dockerfile_base -t mile:base .

### Build MILE Explorer docker image
    $docker build -f Dockerfile_app -t explorer:$(git log -1 --format=%h) .
    

# MILE Explorer JSON-RPC API

## Explore blocks

**Get all allowed methods**

#####http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"help","params":{},"id":"0"}
```
#####http response
```
{"id":"12","jsonrpc":"2.0","result":["get-block","get-block-history","get-block-history-state","get-transaction","get-wallet-history-blocks","get-wallet-history-state","get-wallet-history-transactions","help"]}
```

**Get chains history state**

#####http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache
   
{"jsonrpc":"2.0","method":"get-block-history-state","params":{},"id":"1"}   
```
#####http response
```   
    {"id":"1","jsonrpc":"2.0","result":{"count":11,"first-id":0}}
```

**Get chains history from block *first-id* with *limit* of history length**

#####http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"get-block-history","params":{"first-id":10, "limit":1},"id":"2"}
```
#####http response
```
{
	"id": "2",
	"jsonrpc": "2.0",
	"result": [{
		"block-id": 10,
		"id": "10",
		"merkleRoot": "51cb7ca795c2a7760c0175806c9a7f4dd64c2ced8f5c0db913181bc0fba3c79e",
		"previousBlockDigest": "35ea0fc8d644468dcf77f048406dc0d808c475eaca9c271e893b2dc93f4693e1",
		"producerPublicKeySize": "7",
		"signature": [{
			"producerPublicKey": "335141f7c5669632802de4178f0e1b60e5f76b6c7368646faaff4b5518a72869",
			"producerSignature": "8dd53414d30a9bd491fd4add0a7b92b5b7c749b111b6d240e894d3de163411d7b0d948c9e6db65dc4192d6122f40cf81b5007e1751b027b3df849dcafa967800"
		}, {
			"producerPublicKey": "a3dff17b767c20e19d9cd99db46f44cc037f89b2507272fa98bc8175ac149a2b",
			"producerSignature": "6e40a008c82133691fab51a4e239fa57ee51ac9d9593baf0621c9a9831e3b15d303e6f0dc3128e8f84b8e558b37460904074ae4a21c6fea5f763ce2066ea970b"
		}, {
			"producerPublicKey": "6281570bf2c4f81027bddcc4626ba955fdf98ebb73cfb676a4e9042568baecac",
			"producerSignature": "db3dd67a2015609ad50af4c93d466d620121b378ab7a66707152ba0a13886f8f0027fb4161c888d8f070d43c953bea4dcf1530938c07f897c8c9ca67fce7db04"
		}, {
			"producerPublicKey": "56dc24954d799a8b7860996c5627e5b7ee5c8aede2c6bbe16bdd515b0595ef7c",
			"producerSignature": "9bbc00b39ba737e53c49f2d2866361791217cc58b7f46e172721073dc83a497be6631ccdaa8616c18a35b564a544bdfd50cce9bee22c1553599aed03ae26e907"
		}, {
			"producerPublicKey": "44f850f89cae3145292a91517ab3cafa9466feb7a045d4b30e24c58332996169",
			"producerSignature": "227ba5e33d586c4276f9acfb6ca89c6556a744fdc0426cca0ed7f0e588f3fe25e2c48e0fe354e3642f1f3905bb2ec02958aea8127372dcc128c6e2095d6ce20d"
		}, {
			"producerPublicKey": "648c1214fbdc112a9dc9ff808715542f334752470fabade455c4e36a204bba0c",
			"producerSignature": "ef3e5b14a616ab8fc0adc46e1e35c44957713e745dd0ad9e9a5a429e32561797b12ca8e8544ac24d71f9ab631bf8df6da399c8b848d534e7ecc2c573c5f02c01"
		}, {
			"producerPublicKey": "f08f23599598239796b813746a3d72af35cf64cd894397b0494457d20eeeff1e",
			"producerSignature": "6bc0315feb0687d3440f7bd81d9da73155ec9d630a3c996075752e05df2837f53968f91295e1a11f855f27a1839ba5de04b6cb97a4d2a7e3d0f9b9c64377ef0b"
		}],
		"timeStamp": "19523180146000000",
		"transactionCount": "2",
		"transactions": [{
			"amount": "10",
			"assetCode": "0",
			"code": "10",
			"destinationWalletPublicKey": "2vXm4s5WY84xYtr6L8aBd4KXQ4eS4aWbD1ppQsYBe57thNYctP",
			"id": "4",
			"name": "TransferAssetsTransaction",
			"signature": "ad56dcc4573864cfb27ad04201f3b552fe5e1125b299f85ab706812d726184006d0493a0ec510c2c0d30f01066e7c41681b1a1a62c09408ddec2a78d3372d500",
			"signed": "true",
			"sourceWalletPublicKey": "2XEbswkyZJVNg1ayvyt4XNrJ4Eb41tNdPUdEhnMJpvTKVxgKSc"
		}, {
			"amountOfCoins": "0",
			"amountOfTokens": "0",
			"code": "11",
			"destinationWalletPublicKey": null,
			"id": "1",
			"masterNodeWalletPublicKey": "335141f7c5669632802de4178f0e1b60e5f76b6c7368646faaff4b5518a72869",
			"name": "DistributionFeeTransaction",
			"signature": "50a764db74d4a2bf43218b4b1049d583adf338c0a30694d9d3722cbce8375e08c692bb1624f25884ee8b1ebd5c9e16c5c6e7a93e6084d3ae3a4c587c5f7e1e0f",
			"signed": "true"
		}],
		"version": "1"
	}]
}
```

**Get certain block from chain by *id*. This is equal *get-block-history* with *limit:1* but more efficient**

#####http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"get-block","params":{"id":0},"id":"0"}
```
#####http response
```
{
	"id": "0",
	"jsonrpc": "2.0",
	"result": {
		"block-id": 2,
		"id": "2",
		"merkleRoot": "d5b32f523c5bc8fbc9ae56936e9baa08c72ccef36a649f197ab1ace729d11767",
		"previousBlockDigest": "4000c0d33ec28572b4357b98876807008256e86f6f5eec7bb5d1f5e8a91eb7fe",
		"producerPublicKeySize": "7",
		"signature": [{
			"producerPublicKey": "335141f7c5669632802de4178f0e1b60e5f76b6c7368646faaff4b5518a72869",
			"producerSignature": "d1fc86af21c333ec7f26db043a7e99f3b9438ff2ba1a96dde0a65c129d4d67f2f2549424d350642f4356466c40adb95aeea2728db1324056eaf3b28790b11d05"
		}, {
			"producerPublicKey": "a3dff17b767c20e19d9cd99db46f44cc037f89b2507272fa98bc8175ac149a2b",
			"producerSignature": "86003cefd1a3a16d2cb2c611106f286ff5aa91b08c3ee0ec3909b03dad8d866d27e640d052888b93112538b9d0fa219bd3048368d97ff3af4041482b008c6c03"
		}, {
			"producerPublicKey": "6281570bf2c4f81027bddcc4626ba955fdf98ebb73cfb676a4e9042568baecac",
			"producerSignature": "587e276ed932b6fbc5598708c020a66c62e60dcf30c3de0c52c85c4f7da7aacb332650c6f311b27029bfc3f9774b1254261aaebbabb9c32756f2562b76ad3c0e"
		}, {
			"producerPublicKey": "56dc24954d799a8b7860996c5627e5b7ee5c8aede2c6bbe16bdd515b0595ef7c",
			"producerSignature": "4aa6f80b70082ed241abe167d5c73da8bbc108f000ac21eb20922e4b731c462b4d885c52e989fc938fa8a87df4b33da9c378450b86445942e00ac85b97ebba05"
		}, {
			"producerPublicKey": "44f850f89cae3145292a91517ab3cafa9466feb7a045d4b30e24c58332996169",
			"producerSignature": "abd704ac00778b6b3404bf01b20e0e752a9167cdd2f0b6a7a306e75970305ea0d095a99265a21b3f586c317390a6d72d6d92e15553cf4a77fbf260140f6e3a06"
		}, {
			"producerPublicKey": "648c1214fbdc112a9dc9ff808715542f334752470fabade455c4e36a204bba0c",
			"producerSignature": "8a75af112795d6055c5a0c672d94afd4033ad711b54376a04df9844e2f93964acaf88fcdd73508efc6a20613e11452276a46c1294682840c67a493449771490f"
		}, {
			"producerPublicKey": "f08f23599598239796b813746a3d72af35cf64cd894397b0494457d20eeeff1e",
			"producerSignature": "aef1fe5dd5518d55a8bcb98fcf9719ef434a4b4d7c85282f9687c35cf89fb09b48edfc2ee495769a9587556f35200ce8d3978a65e10a404b38dd08792cd38308"
		}],
		"timeStamp": "19522808130000000",
		"transactionCount": "2",
		"transactions": [{
			"amount": "333333.333",
			"assetCode": "0",
			"code": "10",
			"destinationWalletPublicKey": "zVG4iPaggWUUaDEkyEyFBv8dNYSaFMm2C7WS8nSMKWLsSh9x",
			"id": "2",
			"name": "TransferAssetsTransaction",
			"signature": "46317788f335fe033bcf1d1dd7979d7cbbeb4842a8f644cff314aa4419a6b3769bf8ead0db45b584f5b3afe3409f7f7447363a34a8d974e7ac6fbd49237f3600",
			"signed": "true",
			"sourceWalletPublicKey": "2cv1HqiCE8BeHc4Y7vucckMysx3PV7KND2mQiMdXqDJg812Fd5"
		}, {
			"amountOfCoins": "0",
			"amountOfTokens": "0",
			"code": "11",
			"destinationWalletPublicKey": null,
			"id": "1",
			"masterNodeWalletPublicKey": "335141f7c5669632802de4178f0e1b60e5f76b6c7368646faaff4b5518a72869",
			"name": "DistributionFeeTransaction",
			"signature": "50a764db74d4a2bf43218b4b1049d583adf338c0a30694d9d3722cbce8375e08c692bb1624f25884ee8b1ebd5c9e16c5c6e7a93e6084d3ae3a4c587c5f7e1e0f",
			"signed": "true"
		}],
		"version": "1"
	}
}
```

**Get *wallet* block *id*s for a known *public-key***

#####http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"get-wallet-history-blocks","params":{"public-key":"zVG4iPaggWUUaDEkyEyFBv8dNYSaFMm2C7WS8nSMKWLsSh9x", "first-id": 0, "limit": 2},"id":"12"}
```
#####http response
```
{"id":"12","jsonrpc":"2.0","result":{"blocks":["1","2"]}}
```

## Explore wallet and transactions

**Get *wallet* history state for a known *public-key***

#####http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"get-wallet-history-state","params":{"public-key":"2cv1HqiCE8BeHc4Y7vucckMysx3PV7KND2mQiMdXqDJg812Fd5"},"id":"12"}

```
#####http response
```
{"id":"12","jsonrpc":"2.0","result":{"block":{"count":3,"first-id":0},"transaction":{"count":3,"first-id":0}}}
```

**Get *wallet* transaction history for the known *public-key* from *first-id* with *limit* of transactions list**

#####http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"get-wallet-history-transactions","params":{"public-key":"2cv1HqiCE8BeHc4Y7vucckMysx3PV7KND2mQiMdXqDJg812Fd5", "first-id": 0, "limit": 3},"id":"12"}
```

#####http response
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

#####http request
```
POST https://explorer.testnet.mile.global/v1/api
Accept: */*
Cache-Control: no-cache

{"jsonrpc":"2.0","method":"get-transaction","params":{"public-key":"2XEbswkyZJVNg1ayvyt4XNrJ4Eb41tNdPUdEhnMJpvTKVxgKSc", "id":1},"id":"10"}
```
#####http response
```
{
	"id": "10",
	"jsonrpc": "2.0",
	"result": {
		"amount": "5",
		"assetCode": "1",
		"code": "10",
		"destinationWalletPublicKey": "PbqQ3Rf6NWCNjfrM1cZ9S4RLiqUYykUQGxYhX2nDue7Fr7XFe",
		"digest": "69ec3efa8f96467d7aa3588694702eedde187f3d7f3bd366ee5e501571452cbf",
		"name": "TransferAssetsTransaction",
		"signed": "false",
		"sourceWalletPublicKey": "111111111111111111111111111111115RyRTN",
		"transaction-id": 10
	}
}
```