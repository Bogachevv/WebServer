#pragma once
#include <string>


enum class socket_domain{
    INET,
    UNIX
};

enum class socket_type{
    STREAM,
    DGRAM
};

class base_socket {
    base_socket(const base_socket &other);
    const base_socket &operator=(const base_socket &other);

protected:
    int socket_fd;

    virtual ~base_socket() noexcept;
public:
    base_socket();
    explicit base_socket(int sock_fd);
};


class active_socket : public base_socket{
    bool connected;

public:
    active_socket();

    explicit active_socket(int sock_fd);

    void connect(const std::string &ip, int port);

    bool is_connected() const;

    void shutdown(bool rd = true, bool wr = true);

    void send_msg(const char *buf, size_t len);

    void receive_msg(char *buf, size_t len);

    active_socket &operator<<(const std::string &msg);

    active_socket &operator>>(std::string &msg);

    active_socket &operator<<(int msg);

    active_socket &operator>>(int &msg);

    ~active_socket() noexcept override;

};


class listen_socket : public base_socket{
public:
    using callback_t = void (*)(active_socket&);

private:
    bool is_alive;
    callback_t callback_ptr;

    void listen_loop();

public:
    listen_socket(const std::string &ip, int port, callback_t callback_ptr = nullptr);

    void start_listening(int backlog);

    void stop_listening();

    void set_callback(callback_t new_callback_ptr);

    ~listen_socket() override = default;
};
