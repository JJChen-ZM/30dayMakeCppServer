server:
	g++ src/util.cpp src/InetAddress.cpp src/Socket.cpp src/Buffer.cpp client.cpp -o client && \
	g++ src/util.cpp server.cpp src/Epoll.cpp src/InetAddress.cpp src/Socket.cpp src/Channel.cpp src/Server.cpp src/EventLoop.cpp src/Acceptor.cpp src/Connection.cpp src/Buffer.cpp -o server
clean:
	rm -rf client server