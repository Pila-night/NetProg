
#include <arpa/inet.h>
#include <boost/program_options.hpp>
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

bool is_valid_ip(const string& ip)
{
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) != 0;
}

bool is_valid_port(int port) { return port > 0 && port <= 65535; }

int main(int argc, char** argv)
{
    po::options_description desc("Options");
    desc.add_options()("help,h", "Вывести справку")("ip,i", po::value<string>()->default_value("172.16.40.1"),
                                                    "IP-адрес")("port,p", po::value<int>()->default_value(13), "Порт");

    po::variables_map vm;
    try {
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

        string ip_address = vm["ip"].as<string>();
        int port = vm["port"].as<int>();

        if(!is_valid_ip(ip_address)) {
            cerr << "Ошибка: некорректный IP-адрес: " << ip_address << endl;
            return 1;
        }

        if(!is_valid_port(port)) {
            cerr << "Ошибка: некорректный порт: " << port << ". Порт должен быть в диапазоне 1-65535." << endl;
            return 1;
        }

        int client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(client_socket == -1) {
            throw std::system_error(errno, std::generic_category(), "Ошибка создания сокета");
        }

        sockaddr_in srv_addr{};
        srv_addr.sin_family = AF_INET;
        srv_addr.sin_port = htons(port);
        srv_addr.sin_addr.s_addr = inet_addr(ip_address.c_str());

        string MSG = "Hi, this is a client program, what time is it now \n";

        int rc =
            sendto(client_socket, MSG.c_str(), MSG.size(), 0, reinterpret_cast<sockaddr*>(&srv_addr), sizeof(srv_addr));

        if(rc == -1) {
            throw std::system_error(errno, std::generic_category(), "Ошибка отправки сообщения");
        }

        int buflen = 1024;
        std::unique_ptr<char[]> buf(new char[buflen]);
        sockaddr_in from_addr{};
        socklen_t from_len = sizeof(from_addr);

        rc = recvfrom(client_socket, buf.get(), buflen, 0, reinterpret_cast<sockaddr*>(&from_addr), &from_len);

        if(rc == -1) {
            throw std::system_error(errno, std::generic_category(), "Ошибка получения сообщения");
        }

        std::string res(buf.get(), rc);                 // сохраняем массив в строку
        if(rc == buflen) {                              // массив полон?
            int tail_size;                              // да
            ioctl(client_socket, FIONREAD, &tail_size); // узнаем остаток в буфере приема
            if(tail_size > 0) {                         // остаток есть?
                if(tail_size > buflen) // да, остаток больше размера массива?
                    // да, пересоздаем массив в размер остатка
                    buf = std::unique_ptr<char[]>(new char[tail_size]);
                // нет, используем старый массив
                rc = recvfrom(client_socket, buf.get(), tail_size, 0, reinterpret_cast<sockaddr*>(&from_addr),
                              &from_len);  // принять остаток
                res.append(buf.get(), rc); // добавляем остаток в строку
            }
        }
        cout << "Время на сервере: " << res << endl;

        close(client_socket);
    } catch(const std::system_error& e) {
        cerr << e.what() << endl;
        return 1;
    } catch(const std::exception& e) {
        cerr << "Непредвиденная ошибка: " << e.what() << endl;
        return 1;
    } catch(...) {
        cerr << "Неизвестная ошибка" << endl;
        return 1;
    }

    return 0;
}
