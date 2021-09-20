# olaris_sr_test
A simple test app written in c showing how to connect to Olaris Speech Recognition Engine.

## Preparation

Install libcurl and libjson-c
```
sudo apt install libcurl3-dev
sudo apt install libcurl3-gnutls
sudo apt install libjson-c-dev
```

Install libwebsockets (do not get it using apt as it will be too old):
```
git clone https://github.com/warmcat/libwebsockets
cd libwebsockets
git checkout 7126d848575e4c9d80a1e4893018f55126b42fd1
cmake .
make
sudo make install
```

## Build
```
make
```

## Prepare json config file 

It should contain olaris API credentials:

```
{
   "api_key": "YOUR_API_KEY",
   "product_name": "YOUR_PRODUCT_NAME",
   "organization_id": "YOUR_ORGANIZATION_ID"
}
```

## Prepare audio file
It must be raw with samplingRate=8000, 16bit, signed, 1-channel, little-endian.
Ex:
```
sox research/online-decoder-client/konnichiwa.wav -r 8000 -b 16 -c 1 -e signed --endian little konnichiwa.r-8000.b-16.c-1.e-signed.endian-little.raw

sox research/online-decoder-client/konnichiwa-pad.wav -r 8000 -b 16 -c 1 -e signed --endianness little konnichiwav-pad.r-8000.b-16.c-1.e-signed.endianness-little.raw
```
  

## Test
```
./olaris_test PATH_TO_JSON_CONFIG_FILE PATH_TO_AUDIO_FILE
```

## Sample test output
```
takeshi:olaris_sr_test$ ./olaris_test ~/tmp/olaris.json konnichiwa.r-8000.b-16.c-1.e-signed.endian-little.raw 
... ABRIDGED ...
callback_http with reason=8
{"type": "decoding", "result": "", "raw": ""}
decoding
callback_http with reason=8
{"type": "decoding", "result": "\u3053\u3093\u306b\u3061\u306f", "raw": "\u3053\u3093\u306b\u3061\u306f+\u611f\u52d5\u8a5e "}
decoding
callback_http with reason=8
{"type": "decoding", "result": "\u3053\u3093\u306b\u3061\u306f", "raw": "\u3053\u3093\u306b\u3061\u306f+\u611f\u52d5\u8a5e "}
decoding
callback_http with reason=8
{"type": "decoding", "result": "\u3053\u3093\u306b\u3061\u306f", "raw": "\u3053\u3093\u306b\u3061\u306f+\u611f\u52d5\u8a5e "}
decoding
^C
takeshi:olaris_sr_test$ 

If we pass the result to a decoder like this:
  https://www.online-toolz.com/tools/text-unicode-entities-convertor.php

we get:
  \u3053\u3093\u306b\u3061\u306f => こんにちは
```

So it is OK.


