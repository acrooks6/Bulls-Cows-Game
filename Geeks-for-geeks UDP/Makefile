all: UDPserver.o UDPclient.o
	g++ -g -o server server.o 
	g++ -g -o client client.o 



server.o: UDPserver.c 
	g++ -Wall -c UDPserver.c 
	
client.o: UDPclient.c  
	g++ -Wall -c UDPclient.c 

