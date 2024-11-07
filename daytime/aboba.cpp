sockaddr_in srv_addr{};
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr.s_addr = inet_addr(ip_address.c_str());

    std::string MSG = "Hi, this is a client program, what time is it now n";

    // Отправка сообщения на сервер с использованием sendto вместо connect
    int rc = sendto(client_socket, MSG.c_str(), MSG.size(), 0,
                     reinterpret_cast<sockaddr*>(&srv_addr), sizeof(srv_addr));
    
    if (rc == -1) {
        throw std::system_error(errno, std::generic_category(), "Ошибка отправки сообщения");
    }

    int buflen = 1024;                              // начальный размер массива
    std::unique_ptr<char[]> buf(new char[buflen]);  // начальный массив

    sockaddr_in from_addr{};
    socklen_t from_len = sizeof(from_addr);

    // Прием данных от сервера с использованием recvfrom вместо recv
    rc = recvfrom(client_socket, buf.get(), buflen, 0,
                   reinterpret_cast<sockaddr*>(&from_addr), &from_len);
    
    if (rc == -1) {
        throw std::system_error(errno, std::generic_category(), "Ошибка получения сообщения");
    }

    std::string res(buf.get(), rc);                 // сохраняем массив в строку
    if (rc == buflen) {                              // массив полон?
        int tail_size;                              // да
        ioctl(client_socket, FIONREAD, &tail_size); // узнаем остаток в буфере приема
        if (tail_size > 0) {                         // остаток есть?
            if (tail_size > buflen) // да, остаток больше размера массива?
                // да, пересоздаем массив в размер остатка
                buf = std::unique_ptr<char[]>(new char[tail_size]);
            // нет, используем старый массив
            rc = recvfrom(client_socket, buf.get(), tail_size, 0,
                            reinterpret_cast<sockaddr*>(&from_addr), &from_len); // принять остаток
            res.append(buf.get(), rc);                         // добавляем остаток в строку
        }
    }

    close(client_socket); 
