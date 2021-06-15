g++ ./lib/nativemsg.c main.c ./lib/cJSON.c ./lib/serialib.cpp -rdynamic -o nativehost -fpermissive -lcrypto
