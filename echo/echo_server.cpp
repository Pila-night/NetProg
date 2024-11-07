#include <arpa/inet.h>
#include <boost/program_options.hpp>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;
namespace po = boost::program_options;

void logMessage(const std::string& message, const std::string& logFilePath)
{
    std::ofstream logFile(logFilePath, std::ios_base::app);
    if(!logFile.is_open()) {
        std::cerr << "Не удалось открыть файл для записи логов." << std::endl;
        return;
    }
    time_t now = std::time(nullptr);
    tm localTime = *std::localtime(&now);
    ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    string formattedMessage = oss.str() + " - " + message;
    logFile << formattedMessage << endl;
    cout << formattedMessage << endl;
    logFile.close();
}

bool is_valid_port(int port) { return port > 0 && port <= 65535; }

int main(int argc, char** argv)
{
    po::options_description desc("Options");
    desc.add_options()("help,h", "Вывести справку")("port,p", po::value<int>()->default_value(7), "Порт для сервера")(
        "logfile,l", po::value<string>()->default_value("server.log"), "Файл для логов");

    po::variables_map vm;

    if(argc == 1) {
        cout << desc << endl;
        return 0;
    }
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")) {
        cout << desc << endl;
        return 0;
    }

    int port = vm["port"].as<int>();
    string logFilePath = vm["logfile"].as<string>();
    if(!is_valid_port(port)) {
        cerr << "Ошибка: некорректный порт: " << port << ". Порт должен быть в диапазоне 1-65535." << endl;
        return 1;
    }
    try {
        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(server_socket == -1) {
            throw std::system_error(errno, std::generic_category(), "Ошибка при создании сокета");
        }

        unique_ptr<sockaddr_in> server_addr(new sockaddr_in);
        server_addr->sin_family = AF_INET;
        server_addr->sin_port = htons(port);
        server_addr->sin_addr.s_addr = INADDR_ANY;
        int rb = bind(server_socket, (sockaddr*)server_addr.get(), sizeof(sockaddr_in));
        if(rb == -1) {
            throw std::system_error(errno, std::generic_category(), "Ошибка привязки сокета");
        }

        rb = listen(server_socket, 5);
        if(rb == -1) {
            throw std::system_error(errno, std::generic_category(), "Ошибка при прослушивании сокета");
        }

        logMessage("Сервер начал прослушку на порту " + to_string(port) + ".", logFilePath);

        while(true) {
            sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);

            int work_sock = accept(server_socket, (sockaddr*)&client_addr, &len);
            if(work_sock == -1) {
                throw std::system_error(errno, std::generic_category(), "Ошибка при принятии соединения");
            }
            std::string client_ip = inet_ntoa(client_addr.sin_addr);
            logMessage("Новое подключение от: " + client_ip, logFilePath);

            int buflen = 1024;                              // начальный размер массива
            std::unique_ptr<char[]> buf(new char[buflen]);  // начальный массив
            int rc = recv(work_sock, buf.get(), buflen, 0); // принять данные

            if(rc < 0) {
                throw std::system_error(errno, std::generic_category(), "Ошибка получения данных");
            } else if(rc == 0) {
                logMessage("Соединение закрыто клиентом.", logFilePath);
                close(work_sock);
                return 0;
            }

            std::string res(buf.get(), rc); // сохраняем массив в строку

            if(rc == buflen) {                          // массив полон?
                int tail_size;                          // да
                ioctl(work_sock, FIONREAD, &tail_size); // узнаем остаток в буфере приема

                if(tail_size > 0) {          // остаток есть?
                    if(tail_size > buflen) { // да, остаток больше размера массива?
                        // пересоздаем массив в размер остатка
                        buf = std::unique_ptr<char[]>(new char[tail_size]);
                    }
                    rc = recv(work_sock, buf.get(), tail_size, 0); // принять остаток
                    if(rc < 0) {
                        throw std::system_error(errno, std::generic_category(), "Ошибка получения остаточных данных");
                    }
                    res.append(buf.get(), rc); // добавляем остаток в строку
                }
            }
            logMessage("Общее количество полученных байт: " + std::to_string(res.size()) + ", содержимое: " + res,
                       logFilePath);

            int sent_bytes = send(work_sock, res.c_str(), res.size(), 0);
            if(sent_bytes < 0) {
                throw std::system_error(errno, std::generic_category(), "Ошибка отправки данных");
            }

            logMessage("Отправлено " + std::to_string(sent_bytes) + " байт обратно клиенту.", logFilePath);

            close(work_sock);
        }

        close(server_socket);
    } catch(const std::system_error& e) {
        logMessage(e.what(), logFilePath);
        return 1;
    } catch(const std::exception& e) {
        logMessage("Произошла неожиданная ошибка: " + string(e.what()), logFilePath);
        return 1;
    } catch(...) {
        logMessage("Произошла неизвестная ошибка.", logFilePath);
        return 1;
    }

    return 0;
}
