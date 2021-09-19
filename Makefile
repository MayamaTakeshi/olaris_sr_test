default: build

build: clean
	gcc  -Wall -o olaris_test main.c util.c ws_client.c -lcurl -ljson-c -lwebsockets -lbsd

clean:
	rm -rf olaris_test
