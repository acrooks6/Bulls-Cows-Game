#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>

#include<stdio.h>
#include<sys/types.h>
#include<signal.h>

#include "packet.h" // defined by us
#include "Bulls_And_Cows.h"
#include "project1_server.h"

using namespace std;

// Function prototypes
void SampleHandler(/*You decide*/);

int main(int argc, char *argv[])
{
    unsigned short int tcp_server_port;
    sockaddr_in tcp_server_addr;        // socket addr for this process
    socklen_t tcp_server_addr_len = sizeof(tcp_server_addr);
    sockaddr_in tcp_client_addr;        // socket addr for client
    socklen_t tcp_client_addr_len = sizeof(tcp_client_addr);

    int tcp_server_fd;      //socket file descriptor
    int tcp_client_fd;

    unsigned short int udp_server_port;
    sockaddr_in udp_server_addr;        // socket addr for this process
    socklen_t udp_server_addr_len = sizeof(udp_server_addr);
    sockaddr_in udp_client_addr;        // socket addr for client
    socklen_t udp_client_addr_len = sizeof(udp_client_addr);
    int udp_server_fd;      //socket file descriptor

    int bytes_recieved;
    int bytes_sent;

    char send_buf[buffer_len + 20];

    parse_argv(argc, argv); // In fact, this is not necessary.
                            // But I leave it here to capture invalid
                            // parameters.

    cout << "[SYS] Parent process for TCP communication." << endl; 
    cout << "[TCP] Bulls and Cows game server started..." << endl;

    // TCP: CREATE A TCP SERVER FOR ANY INCOMING CONNECTION
    //      REMEMBER TO CHECK ALL RETURN VALUES
    tcp_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // if return val is -1,  socket creation failed
    if (tcp_server_fd < 0)
    {
        cerr << "[ERR] Unable to create TCP socket." << endl;
        exit(1);
    }
    //Initialize the socket address struct by setting all bytes to 0
    memset(&tcp_server_addr, 0, sizeof(tcp_server_addr));

    tcp_server_addr.sin_family = AF_INET;       // Internet Family
    tcp_server_addr.sin_port = 0;               // Let the OS choose the port
    tcp_server_addr.sin_addr.s_addr;            // wild card machine address

    //Bind the socket name to the socket
    if(bind(tcp_server_fd, (sockaddr *)&tcp_server_addr, sizeof(tcp_server_addr)) < 0)
    {
        cerr << "[ERR] Unable to bind TCP socket" << endl;
        exit(1);
    }

    //Get the socket name, to obtain the port number assigned by the OS
    getsockname(tcp_server_fd, (struct sockaddr *) & tcp_server_addr, &tcp_server_addr_len) ;
    tcp_server_port = ntohs(tcp_server_addr.sin_port);

    //Print the port name
    cout << "[TCP] Socket has port: " << tcp_server_port << endl;

    //listen to the socket, wait for incoming connections
    listen(tcp_server_fd, 1);

    while(1){
        // TCP: ACCEPT NEW INCOMING CONNECTION/CLIENT
        tcp_client_fd = accept(tcp_server_fd, (struct sockaddr *) &tcp_client_addr, &tcp_client_addr_len);
        
        // FOR EACH NEW CONNECTION / CLIENT
        // CREATE / FORK A CHILD PROCESS TO HANDLE IT
        int pid = fork();
        // REMEMBER TO CHECK ALL RETURN VALUES
        if (pid < 0)
        {
            cerr << "[ERR] fork() failed" << endl;
            exit(1);
        }
        // FOR CHILD PROCESS
        //     CREATE A UDP SERVER FOR THE GAME
        //     FOR EACH CHILD WE INVOKE A HANDLER FOR IT
        //     SampleHandler
        else if (pid == 0)
        {
            cout << "[SYS] Child process forked" << endl;

            My_Packet join;
            bytes_recieved = read(tcp_client_fd, &join, sizeof(join));
            if (bytes_recieved < 0)
            {
                cerr << "[ERR] Error recieving message from client" << endl;
                exit(1);
            }
            
            else if (bytes_recieved > 0)            /// Recieve joing request from client
            {
                if (join.type == JOIN)
                {
                    udp_server_fd = socket(AF_INET, SOCK_DGRAM, 0);         /// Create UDP socket
                    if (udp_server_fd < 0)
                    {
                        cerr << "[ERR] Unable to create UDP socket." << endl;
                        exit(1);
                    }
                    memset(&udp_server_addr, 0, sizeof(udp_server_addr));
           
                    udp_server_addr.sin_family = AF_INET;       // Internet Family
                    udp_server_addr.sin_port = 0;               // Let the OS choose the port
                    udp_server_addr.sin_addr.s_addr = INADDR_ANY;            // wild card machine address

                    //Bind the socket name to the socket
                    if(bind(udp_server_fd, (sockaddr *) &udp_server_addr, sizeof(udp_server_addr)) < 0)
                    {
                        cerr << "[ERR] Unable to bind UDP socket" << endl;
                        exit(1);
                    }

                    //Get the socket name, to obtain the port number assigned by the OS
                    getsockname(udp_server_fd, (struct sockaddr *) & udp_server_addr, &udp_server_addr_len) ;
                    udp_server_port = ntohs(udp_server_addr.sin_port);

                    cout << "[TCP] JOIN request recieved" << endl;
                    My_Packet grant;
                    grant.type = JOIN_GRANT;
                    sprintf(grant.buffer, "%u", udp_server_port);

                    cout << "[UDP] Socket has port: " << grant.buffer << endl;
                    bytes_sent = write(tcp_client_fd, &grant, sizeof(grant));           /// Send JOIN_GRANT message to client
                    if (bytes_sent < 0)
                    {
                        cerr << "[ERR] Error sending message to client" << endl;
                        exit(1);
                    }

                    if (close(tcp_client_fd) < 0)                                       ///close the tcp connection
                    {
                        cerr << "[ERR] Error when closing client socket" << endl;
                        exit(1);
                    }

                    memset(send_buf, 0, sizeof(send_buf));
                    sprintf(send_buf, "[UDP:%u] Gameplay server started:", udp_server_port);
                    cout << send_buf << endl;

                    memset(send_buf, 0, sizeof(send_buf));
                    sprintf(send_buf, "[TCP] Sent: JOIN GRANT %u", udp_server_port);
                    cout << send_buf << endl;

                    int sn;
                    int bulls;
                    int cows;
                    Bulls_And_Cows game;
                    game.Restart_Game();
                    game.get_secret_number(sn);

                    memset(send_buf, 0, sizeof(send_buf));
                    sprintf(send_buf, "[UDP:%u] Secret Number: %d", udp_server_port, sn);
                    cout << send_buf << endl;

                    memset(send_buf, 0, sizeof(send_buf));
                    sprintf(send_buf, "[UDP:%u] A new game has started:", udp_server_port);
                    cout << send_buf << endl;

                    while(1)                
                    {   
                        My_Packet reciever;
                        My_Packet sender;
                        
                        bytes_recieved = recvfrom(udp_server_fd, &reciever, sizeof(reciever), 0, (sockaddr *) &udp_client_addr, &udp_client_addr_len);
                        if (bytes_recieved < 0)
                        {
                            cerr << "[ERR] Error recieving message from client" << endl;
                            exit(1);
                        }

                        if (reciever.type == EXIT)
                        {
                            memset(send_buf, 0, sizeof(send_buf));
                            sprintf(send_buf, "[UDP:%u] EXIT recieved, ending game", udp_server_port);
                            cout << send_buf << endl;
                            
                            sender.type = EXIT_GRANT;
                            memset(sender.buffer, 0, sizeof(sender.buffer));

                            bytes_sent = sendto(udp_server_fd, &sender, sizeof(sender), 0,
                                             (sockaddr *)&udp_client_addr, udp_client_addr_len);
                            if (bytes_sent < 0)
                            {
                                cerr << "[ERR] Error sending message to client" << endl;
                                exit(1);
                            }
                            
                            if(close(udp_server_fd) < 0)                                    ///< Close the socket when client exits
                            {
                                cerr << "[ERR] Error when closing server socket" << endl;
                                exit(1);
                            }

                            memset(send_buf, 0, sizeof(send_buf));
                            sprintf(send_buf, "[UDP:%u] Player has left the game", udp_server_port);
                            cout << send_buf << endl;

                            cout << "[SYS] child process terminated" << endl;
                            kill(getpid(), SIGKILL);                                      ///< Just want to make extra sure the process is dead
                            cout << "You shouldn't be able to see this message" << endl;

                            break;
                        }
                        else
                        {
                        // If the client correctly guesses the secret number (wins), game.Guess returns true
                            if(game.Guess(reciever.buffer, bulls, cows))
                            {
                                memset(send_buf, 0, sizeof(send_buf));
                                sprintf(send_buf, "[UDP:%u] Recieved: GUESS ", udp_server_port);
                                cout << send_buf << reciever.buffer << endl;

                                sprintf(sender.buffer, "Congratulations! You have won the game by guessing the secret number: %d" 
                                                        " The Game will now restart with a different secret number, type EXIT if you want to stop playing", sn);
                                game.Restart_Game();
                                game.get_secret_number(sn);
                                memset(send_buf, 0, sizeof(send_buf));
                                sprintf(send_buf, "[UDP:%u] New Secret Number: %d", udp_server_port, sn);
                                cout << send_buf << endl;
                            }
                            else
                            {
                                memset(send_buf, 0, sizeof(send_buf));
                                sprintf(send_buf, "[UDP:%u] Recieved: GUESS ", udp_server_port);
                                cout << send_buf << reciever.buffer << endl;
                                sprintf(sender.buffer, "%dA%dB", bulls, cows);
                                sender.type = RESPONSE;
                            }
                    
                            memset(send_buf, 0, sizeof(send_buf));
                            sprintf(send_buf, "[UDP:%u] Sent: RESPONSE ", udp_server_port);
                            cout << send_buf << sender.buffer << endl;

                            bytes_sent = sendto(udp_server_fd, &sender, sizeof(sender), 0,
                                                (sockaddr *)&udp_client_addr, udp_client_addr_len);
                            if (bytes_sent < 0)
                            {
                                cerr << "[ERR] Error sending message to client" << endl;
                                exit(1);
                            }

                        }
                    }

                }
                //KILL THE PROCESS IF message isnt of type JOIN
                else
                {
                    cout << "Recieved wrong type of message, destroying myself now" << endl;
                    if (close(tcp_client_fd) < 0)
                    {
                        cerr << "[ERR] Error when closing client socket" << endl;
                        exit(1);
                    }
                    kill(getpid(), SIGKILL);
                    cout << "You shouldn't be able to see this message" << endl;
                    break;
                }

            }
        }
        // FOR PARENT PROCESS
        //     CONTINUE THE LOOP TO ACCEPT NEW CONNECTION/CLIENT
    }

    return 0;
}

/*********************************
 * Name:    SampleHandler
 * Purpose: The function to handle UDP communication from/to a client
 * Receive: You decide
 * Return:  You decide
*********************************/
//My Apologies for not using the handler function
void SampleHandler(/*You decide*/)
{
    // UDP: CREATE A UDP SERVER FOR GAME FOR ONE CLIENT
    //      OBTAIN THE UDP PORT NUMBER
    //      WAIT FOR THE CLIENT TO ISSUE JOIN

    // TCP: THE SERVER REPLIES THE UDP SERVER PORT TO THE CLIENT

    // REMEMBER TO CLOSE THE TCP SOCKET

    while(1) // UDP: GAMEPLAY
    {
        // GAME: START A NEW GAME IF THE PLAYER HAS ALL FOUR
        //       DIGITS CORRECTLY

        // UDP: RECV/SEND INTERACTIONS
    }
}

