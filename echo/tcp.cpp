#include <arpa/inet.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <chrono>

using namespace std;

void logMessage(const string& message) {
    ofstream logFile("server.log", ios::app); 
    if (logFile.is_open()) {
        auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        logFile << put_time(localtime(&now), "%Y-%m-%d %H:%M:%S") << " - " << message << endl;
        logFile.close();
    } else {
        cerr << "Не удалось открыть файл для записи логов." << endl;
    }
}

int main() {
    try {
        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(server_socket == -1) {
            throw std::system_error(errno, std::generic_category(), "Ошибка при создании сокета");
        }

        unique_ptr<sockaddr_in> server_addr(new sockaddr_in);
        server_addr->sin_family = AF_INET;
        server_addr->sin_port = htons(7777); 
        server_addr->sin_addr.s_addr = INADDR_ANY;

        int rb = bind(server_socket, (sockaddr*)server_addr.get(), sizeof(sockaddr_in));
        if(rb == -1) {
            throw std::system_error(errno, std::generic_category(), "Ошибка привязки сокета");
        }

        rb = listen(server_socket, 5);
        if(rb == -1) {
            throw std::system_error(errno, std::generic_category(), "Ошибка при прослушивании сокета");
        }

        logMessage("Сервер начал прослушку.");
        while(true) {
            sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);

            int work_sock = accept(server_socket, (sockaddr*)&client_addr, &len);
            if(work_sock == -1) {
                throw std::system_error(errno, std::generic_category(), "Ошибка при принятии соединения");
            }

            char buf[1024];                               
            int rc = recv(work_sock, buf, sizeof(buf), 0); 
            if(rc < 0) {
                throw std::system_error(errno, std::generic_category(), "Ошибка получения данных");
            } else if(rc == 0) {
                logMessage("Соединение закрыто клиентом.");
                close(work_sock); 
                continue; 
            }
            logMessage("Получено " + to_string(rc) + " байт.");

            int sent_bytes = send(work_sock, buf, rc, 0); 
            if(sent_bytes < 0) {
                throw std::system_error(errno, std::generic_category(), "Ошибка отправки данных");
            }

            logMessage("Отправлено " + to_string(sent_bytes) + " байт обратно клиенту.");

            close(work_sock);
        }

        close(server_socket);
    } catch (const std::system_error& e) {
        logMessage(e.what());
        return 1;
    } catch (const std::exception& e) {
        logMessage("Произошла неожиданная ошибка: " + string(e.what()));
        return 1;
    } catch (...) {
        logMessage("Произошла неизвестная ошибка.");
        return 1;
    }

    return 0;
}
