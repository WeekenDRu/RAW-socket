gcc udpserver.c -o udpserver
gcc rawudpclient.c -o rawudpclient

./udpserver [ip server(127.0.0.1)] ==> port dest

sudo ./rawudpclient [MAC_DST] [NAME_INT] [IP_DST] [PORT_DST] [IP_SRC]

sudo ./rawudpclient B8:88:E3:9F:29:11 enp5s0f0 127.0.0.1 3456 127.0.0.2
