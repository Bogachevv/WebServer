#pragma once

#include <windows.h>

class WSA;

class WSA_ptr{
    static int ref_c;
    WSA *ptr;

public:
    explicit WSA_ptr(WSA *wsa_ptr);

    WSA_ptr(const WSA_ptr&);
    WSA_ptr &operator=(const WSA_ptr&) = delete;

    WSA *operator->();

    ~WSA_ptr();

};

class WSA {
private:
    friend WSA_ptr;
    static WSA *object_ptr;
    WSAData wsaData;

    WSA();
    ~WSA();
public:
    WSA(const WSA&) = delete;
    WSA &operator=(const WSA&) = delete;

    static WSA_ptr getWSA();

};
