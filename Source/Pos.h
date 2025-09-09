#ifndef POS_H
#define POS_H


class Pos {
    std::stack<std::string> sales;
    int clientSocket;

    public:
    Pos();
    Pos(std::string address, unsigned short port);

    ~Pos();

    retType Handshake();

    retType PushSale(char* price, std::time_t *time);

};

#endif