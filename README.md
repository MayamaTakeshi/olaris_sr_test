# olaris_sr_test
A simple test app written in c showing how to connect to Olaris speech Recognition engine.

## Preparation

Install libcurl and libjson-c
```
sudo apt install libcurl3-dev
sudo apt install libcurl3-gnutls
sudo apt install libjson-c-dev
```

Install libwebsockets (do not get using apt as it will be too old):
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

## Prepare json config file with olaris API credentials. It should be like this:
```
{
   "api_key": "YOUR_API_KEY",
   "product_name": "YOUR_PRODUCT_NAME",
   "organization_id": "YOUR_ORGANIZATION_ID",
}
```

## Test
```
./olaris_test PATH_TO_SON_CONFIG_FILE
```

## Sample test output
```
takeshi:olaris_sr_test$ ./olaris_test ~/tmp/olaris.json 
Response Code: 200
Size: 181
[2021/09/19 14:31:40:3801] N: LWS: 4.2.99-v4.2.0-200-g7126d848, NET CLI SRV H1 H2 WS ConMon IPv6-absent
[2021/09/19 14:31:40:3803] N:  ++ [wsi|0|pipe] (1)
[2021/09/19 14:31:40:3803] N:  ++ [vh|0|netlink] (1)
[2021/09/19 14:31:40:3804] N:  ++ [vh|1|default||-1] (2)
callback_http
callback_http
callback_http
[2021/09/19 14:31:40:3808] N:  ++ [wsicli|0|WS/h1/realtime.stt.batoner.works] (1)
callback_http
callback_http
callback_http
callback_http
callback_http
callback_http
callback_http
callback_http
callback_http
callback_http
callback_http
callback_http
callback_http
callback_http
established
callback_http
[2021/09/19 14:31:41:8336] N: 
[2021/09/19 14:31:41:8336] N: 0000: 7B 22 74 79 70 65 22 3A 20 22 64 65 63 6F 64 69    {"type": "decodi
[2021/09/19 14:31:41:8336] N: 0010: 6E 67 22 2C 20 22 72 65 73 75 6C 74 22 3A 20 22    ng", "result": "
[2021/09/19 14:31:41:8336] N: 0020: 22 7D                                              "}              
[2021/09/19 14:31:41:8336] N: 
```

After the above response ("decoding") the engine is waiting for audio to be sent in the WebSocket connection.


