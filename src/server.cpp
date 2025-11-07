#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <signal.h>
#include "cache.h"
#include "db.h"
#include "threadpool.h"
using namespace std;

static bool running=true;
void sigint_handler(int){running=false;}

string http_response(int code,string body){
    string status=(code==200?"200 OK":(code==404?"404 Not Found":"500 Internal"));
    stringstream ss;
    ss<<"HTTP/1.1 "<<status<<"\r\nContent-Length: "<<body.size()<<"\r\nConnection: close\r\n\r\n"<<body;
    return ss.str();
}

// small helper to convert int->string and vice versa
string to_str(int n){ return to_string(n); }
int to_int(const string &s){ return stoi(s); }

void handle(int cfd,LRUCache &cache,SimpleDB &db){
    char buf[8192]; ssize_t n=recv(cfd,buf,sizeof(buf)-1,0); if(n<=0){close(cfd);return;}
    string req(buf,n);
    string method,path,body; size_t pos=req.find("\r\n\r\n"); if(pos!=string::npos)body=req.substr(pos+4);
    stringstream ss(req.substr(0,req.find("\r\n"))); ss>>method>>path;

    // expect /kv/<integer>
    string key_str = path.substr(4);
    int key;
    try { key = to_int(key_str); }
    catch (...) {
        string resp=http_response(500,"Key must be integer");
        send(cfd,resp.c_str(),resp.size(),0); close(cfd); return;
    }

    string resp;
    if(method=="PUT"){
        db.put_kv(key,body);
        cache.put(to_str(key),body);
        resp=http_response(200,"OK");
    }
    else if(method=="GET"){
        auto v=cache.get(to_str(key));
        string val;
        if(v.has_value()) resp=http_response(200,v.value());
        else if(db.get_kv(key,val)){ cache.put(to_str(key),val); resp=http_response(200,val);}
        else resp=http_response(404,"Not Found");
    }
    else if(method=="DELETE"){
        db.del_kv(key);
        cache.erase(to_str(key));
        resp=http_response(200,"Deleted");
    }
    else resp=http_response(500,"Bad Method");

    send(cfd,resp.c_str(),resp.size(),0);
    close(cfd);
}

int main(){
    signal(SIGINT,sigint_handler);
    int port=8080;
    LRUCache cache(100);
    SimpleDB db("127.0.0.1","kvuser","kvpass","kvdb");
    if(!db.is_connected()){cerr<<"DB connection failed\n";return 1;}
    ThreadPool pool(4);
    int sfd=socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_port=htons(port);addr.sin_addr.s_addr=INADDR_ANY;
    bind(sfd,(sockaddr*)&addr,sizeof(addr)); listen(sfd,128);
    cout<<"Server listening on port "<<port<<endl;
    while(running){
        sockaddr_in c{};socklen_t l=sizeof(c);
        int cfd=accept(sfd,(sockaddr*)&c,&l);
        if(cfd<0)continue;
        pool.enqueue([cfd,&cache,&db](){handle(cfd,cache,db);});
    }
    close(sfd);pool.shutdown();
}
