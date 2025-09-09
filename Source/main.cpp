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



int main(int argc, char * argv[]){
    retType res = E_NOT_OK;

    /* Dedicate memory for the DB*/
    DB localDb;

    /* Form the given parameters into param vector*/
    std::string command;
    std::vector<std::string> params;
    for(int i = 1 ; i < argc ; ++i){
        std::string arg = argv[i];
        if ( i == 1){
            command = arg;
        } else {
            params.push_back(arg);
        }
    }


    // sale command
    if(command == "sale" && strcmp(argv[5],"NULL")!=0 && strcmp(argv[7],"NULL")!=0){
        // if an explicit server is given, connect to it
        Pos pos(argv[5], atoi(argv[7]));
        std::time_t time;
        int cnt = 2;

        while(res != E_OK && cnt--){
            /* If the server drops the connection right after HELLO, reconnect once and resume the
            SALE. */
            res = pos.Handshake();
            if(res != E_OK){
                std::cerr << "Handshake with server has failed\n";
                return res;
            }
            
            res = pos.PushSale(argv[3],&time);
        }

        
        localDb.InsertSale(atof(argv[3]),time,res);
    }else if(command == "last"){
        //last command
        localDb.PrintLastSales(atoi(argv[3]));
    }
    else if(command == "recon"){
        // recon command
        localDb.Recon();
    }
    else{
        std::cerr << "Invalid command or parameters\n";
        res = E_NOT_OK;
    }



    return res;
}