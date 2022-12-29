#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <sstream>
#include <cstring>

#include <netdb.h>
#include <unistd.h>

#include "packet.h"      // defined by us
#include "project1_client.h" // some supporting functions.

using namespace std;

int main(int argc, char *argv[])
{

    sockaddr_in tcp_server_addr;
    int tcp_connection_fd;

    int bytes_recieved;
    int bytes_sent;

    char                 *server_name_str = 0;
    unsigned short int   tcp_server_port;

    char send_buf[buffer_len];

    // prase the argvs, obtain server_name and tcp_server_port
    parse_argv(argc, argv, &server_name_str, tcp_server_port);

    struct hostent *hostEnd = gethostbyname(server_name_str);

    cout << "[TCP] Bulls and Cows client started..." << endl;
    cout << "[TCP] Connecting to server: " << server_name_str
         << ":" << tcp_server_port << endl;

    My_Packet grant;

    while(1)
    {
        // TCP: CONNECT TO SERVER
        tcp_connection_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (tcp_connection_fd < 0)
        {
            cerr << "[ERR] Unable to create TCP socket" << endl;
            exit(1);
        }

        memset(&tcp_server_addr, 0, sizeof(tcp_server_addr));
        tcp_server_addr.sin_family = AF_INET;
        tcp_server_addr.sin_port = htons(tcp_server_port);

        memcpy(&tcp_server_addr.sin_addr, hostEnd -> h_addr, hostEnd -> h_length);

        if (connect(tcp_connection_fd, (sockaddr *)&tcp_server_addr, sizeof(tcp_server_addr)) < 0)
        {
            cerr << "[ERR] Unable to connect to server." << endl;
            if (close(tcp_connection_fd < 0))
            {
                cerr << "[ERR] Error when closing TCP socket" << endl;
            }
            exit(1);
        }
        cout << "Connection Estabished" << endl;
        // TCP: THE FIRST COMMAND MUST BE JOIN
        //      THE RESPONSE MUST BE JOIN_GRANT WITH A UDP PORT NUMBER

        memset(send_buf, 0, sizeof(send_buf));

        My_Packet join;
        join.type = JOIN;
        bytes_sent = send(tcp_connection_fd, &join, sizeof(join), 0);
        if (bytes_sent < 0)
        {
            cerr << "[ERR] Error sending message to server" << endl;
            exit(1);
        }

        bytes_recieved = read(tcp_connection_fd, &grant, sizeof(grant));
        if (bytes_recieved < 0)
        {
            cerr << "[ERR] Error recieving message from server" << endl;
            exit(1);
        }

        if (close(tcp_connection_fd) < 0)
        {
            cerr << "[ERR] Error when closing TCP socket" << endl;
            exit(1);
        }
        break;
      
        // GOT THE PORT NUMBER
        // EXIT THE LOOP AND CONTINUES TO UDP GAME PLAY
    }

    sockaddr_in udp_server_addr;
    int udp_socket_fd;
    unsigned short int   udp_server_port;
    socklen_t udp_server_addr_len = sizeof(udp_server_addr);

    sscanf(grant.buffer, "%hd", &udp_server_port);
    // Game play using UDP
    cout << "[UDP] Guesses will be sent to: " << server_name_str 
         << " at port:" << udp_server_port << endl;

    udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket_fd < 0)
    {
        cerr << "[ERR] Unable to create UDP socket" << endl;
        exit(1);
    }

    memset(&udp_server_addr, 0, sizeof(udp_server_addr));
    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_port = htons(udp_server_port);

    memcpy(&udp_server_addr.sin_addr, hostEnd -> h_addr, hostEnd -> h_length);

    cout << "[GAM] Please start guessing!" << endl;
    cout << "[GAM] Make sure to use the GUESS command before a 4-digit guess" << endl;
    cout << "[GAM] Type Exit at anytime to stop playing" << endl;
    while(1)
    {
        
        My_Packet outgoing_pkt;
        My_Packet incoming_pkt;
        
        udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0); //Clearing the socket
        // GAME PLAY IN UDP
        
        while(get_command(outgoing_pkt) == false){}
        // UDP: SEND/RECV INTERACTIONS
        bytes_sent = sendto(udp_socket_fd, &outgoing_pkt, sizeof(outgoing_pkt), 0, (sockaddr *) &udp_server_addr, sizeof(udp_server_addr));
        if (bytes_sent < 0)
        {
            cerr << "[ERR] Error sending message to server" << endl;
            exit(1);
        }

        if(outgoing_pkt.type == GUESS)
        {
            memset(send_buf, 0, sizeof(send_buf));
            sprintf(send_buf, "[UDP] Sent: GUESS ");
            cout << send_buf << outgoing_pkt.buffer << endl;
        }

        if(outgoing_pkt.type == EXIT)
        {
            cout << "[UDP] Sent: EXIT" << endl;
        }
        
        bytes_recieved = recvfrom(udp_socket_fd, &incoming_pkt, sizeof(incoming_pkt), 0, (sockaddr *) &udp_server_addr, &udp_server_addr_len);
        if (bytes_recieved < 0)
        {
            cerr << "[ERR] Error recieving message from server" << endl;
            exit(1);
        }
         if(incoming_pkt.type == RESPONSE)
         {
            memset(send_buf, 0, sizeof(send_buf));
            sprintf(send_buf, "[UDP] Recieved: RESPONSE ");
            cout << send_buf << incoming_pkt.buffer << endl;
         }

        if(incoming_pkt.type == EXIT_GRANT)
        {
            memset(send_buf, 0, sizeof(send_buf));
            sprintf(send_buf, "[UDP] Recieved: EXIT GRANT Exit granted, goodbye");
            cout << send_buf << endl;

            if(close(udp_socket_fd) < 0)                                    ///< Close the socket when client exits
            {
                cerr << "[ERR] Error when closing client socket" << endl;
                exit(1);
            }
            break;
        }
    }

    return 0;
}

