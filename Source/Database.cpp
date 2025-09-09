#include <iostream>
#include <vector>
#include <stack>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <poll.h>
#include <sqlite3.h> 
#include <chrono>
#include <thread>

/* local libs*/
#include "Types.h"
#include "Database.h"
#include "Pos.h"

DB::DB(){
        char *zErrMsg = 0;
        retType res;

        res = sqlite3_open("transactions.db", &pDb);
        if(res != SQLITE_OK){
            std::cerr << "Cannot open database\n" << std::endl;
            sqlite3_close(pDb);
        }

        const char* create_sql =
        "CREATE TABLE IF NOT EXISTS sales("
        " price   REAL,"
        " timestamp INTEGER,"
        " status  INTEGER "/* 0 approved, 1 declined */
        ");";
        res = sqlite3_exec(pDb, create_sql, nullptr, nullptr, nullptr);
        if (res != SQLITE_OK) {
            std::cerr << "CREATE TABLE failed\n";
            sqlite3_close(pDb);
        }

    };


DB::~DB(){
        sqlite3_close(pDb);
    }

retType DB::InsertSale(double price, time_t time, bool status){
        retType res;
        const char* insert_sql = "INSERT INTO sales(price,timestamp,status) VALUES(?,?,?);";

        res = sqlite3_prepare_v2(pDb, insert_sql, -1, &comp_stmt, nullptr);
        if (res != SQLITE_OK) {
            std::cerr << "Failed to prepare insert statement\n";
            sqlite3_close(pDb);
            return E_NOT_OK;
        }
        
        res = sqlite3_bind_double(comp_stmt, 1, price);
        res |= sqlite3_bind_int64(comp_stmt, 2, time);
        res |= sqlite3_bind_int(comp_stmt, 3, status);

        if (res != SQLITE_OK) {
            std::cerr << "Binding into the sql statement has failed\n";
            sqlite3_close(pDb);
            return E_NOT_OK;
        }

        res = sqlite3_step(comp_stmt);
        if (res != SQLITE_DONE) {
            std::cerr << "Evaluation of sql statement has failed\n";
            sqlite3_close(pDb);
            return E_NOT_OK;
        }

        res = sqlite3_finalize(comp_stmt);
        if (res != SQLITE_OK) {
            std::cerr << "Finalizing of sql statement has failed\n";
            sqlite3_close(pDb);
            return E_NOT_OK;
        }


        return E_OK;
    }

retType DB::PrintLastSales(int n){
        retType res = E_OK;
        const char* select_sql = "SELECT price,timestamp FROM sales ORDER BY timestamp DESC LIMIT ?;";

        res = sqlite3_prepare_v2(pDb, select_sql, -1, &comp_stmt, nullptr);
        if (res != SQLITE_OK) {
            std::cerr << "Failed to prepare select statement\n";
            sqlite3_close(pDb);
            return E_NOT_OK;
        }

        res = sqlite3_bind_int(comp_stmt, 1, n);
        if (res != SQLITE_OK) {
            std::cerr << "Binding into the sql statement has failed\n";
            sqlite3_close(pDb);
            return E_NOT_OK;
        }

        std::cout << "Last " << n << " sales:\n";
        while (sqlite3_step(comp_stmt) == SQLITE_ROW)  {
            double price = sqlite3_column_double(comp_stmt, 0);
            std::time_t timestamp = sqlite3_column_int64(comp_stmt, 1);
            std::cout << "Price: " << price << std::endl;
            std::cout << "Time:" << ctime(&timestamp) << std::endl;
        }

        res = sqlite3_finalize(comp_stmt);
        if (res != SQLITE_OK) {
            std::cerr << "Finalizing of sql statement has failed\n";
            sqlite3_close(pDb);
            return E_NOT_OK;
        }

        return E_OK;
    }

retType DB::Recon(){
        //  (prints daily totals: count + sum of approved/declined)
        retType res = E_OK;
        int cntAppr = 0;
        int cntDecl = 0;
        const char* select_sql = "SELECT price,timestamp,status FROM sales WHERE timestamp >= ?;";
            // current system time
        auto now = std::chrono::system_clock::now();

        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm = *std::localtime(&t);

        // set h:m:s = 0 → midnight today
        local_tm.tm_hour = 0;
        local_tm.tm_min  = 0;
        local_tm.tm_sec  = 0;

        // convert back to time_t (Unix seconds since epoch)
        std::time_t midnight = std::mktime(&local_tm);

        res = sqlite3_prepare_v2(pDb, select_sql, -1, &comp_stmt, nullptr);
        if (res != SQLITE_OK) {
            std::cerr << "Failed to prepare select statement\n";
            sqlite3_close(pDb);
            return E_NOT_OK;
        }
        res = sqlite3_bind_int64(comp_stmt, 1, midnight);
        if (res != SQLITE_OK) {
            std::cerr << "Binding into the sql statement has failed\n";
            sqlite3_close(pDb);
            return E_NOT_OK;
        }

        while(sqlite3_step(comp_stmt) == SQLITE_ROW){
            double price = sqlite3_column_double(comp_stmt, 0);
            std::time_t timestamp = sqlite3_column_int64(comp_stmt, 1);
            bool status = sqlite3_column_int(comp_stmt, 2);
            if(status == E_OK){
                cntAppr++;
            }else{
                cntDecl++;
            }

        }

        std::cout << "Approved: " << cntAppr << std::endl;
        std::cout << "Declined: " << cntDecl << std::endl;
        std::cout << "Total: " << cntAppr + cntDecl << std::endl;    

        res = sqlite3_finalize(comp_stmt);
        if (res != SQLITE_OK) {
            std::cerr << "Finalizing of sql statement has failed\n";
            sqlite3_close(pDb);
            return E_NOT_OK;
        }
        return E_OK;
    }