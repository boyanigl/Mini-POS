#ifndef DATABASE_H
#define DATABASE_H


class DB {
    sqlite3 *pDb;
    sqlite3_stmt* comp_stmt;

    public:
    DB();
    DB(const DB&) = delete;
    DB& operator=(const DB&) = delete;
    ~DB();

    retType InsertSale(double price, time_t time, bool status);

    retType PrintLastSales(int n);

    retType Recon();
};

#endif