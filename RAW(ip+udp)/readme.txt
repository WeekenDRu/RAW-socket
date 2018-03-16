gcc udpserver.c -o udpserver
gcc rawudpclient.c -o rawudpclient

./udpserver [ip server(127.0.0.1)] ==> port dest

sudo ./rawudpclient [ip destination(127.0.0.1)] [port dest] [ip source(127.0.0.1)]
