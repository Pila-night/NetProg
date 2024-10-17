#include <arpa/inet.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

int main()
{
    try {
        int client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (client_socket == -1) {
            throw std::system_error(errno, std::generic_category(), "Error creating socket");
        }

        unique_ptr<sockaddr_in> srv_addr(new sockaddr_in);
        srv_addr->sin_family = AF_INET;    
        srv_addr->sin_port = htons(13); 
        srv_addr->sin_addr.s_addr = inet_addr("172.16.40.1");

        int rc = connect(client_socket, reinterpret_cast<sockaddr*>(srv_addr.get()), sizeof(sockaddr_in));
        if (rc == -1) {
            throw std::system_error(errno, std::generic_category(), "Error connecting to server");
        }

        string MSG = "Hi, this is a client program, what time is it now";
        rc = send(client_socket, MSG.c_str(), MSG.size(), 0);
        if (rc == -1) {
            throw std::system_error(errno, std::generic_category(), "Error sending message");
        }
       
        char buf[1024] = { 0 }; 
        rc = recv(client_socket, buf, sizeof(buf), 0);
        if (rc == -1) {
            throw std::system_error(errno, std::generic_category(), "Error receiving message");
        }
        
        cout << "Daytime on the server: " << buf << endl;
        
        close(client_socket);
    } catch (const std::system_error& e) {
        cerr << e.what() << endl;
        return 1;
    } catch (const std::exception& e) {
        cerr << "An unexpected error occurred: " << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "An unknown error occurred." << endl;
        return 1;
    }

    return 0;
}
