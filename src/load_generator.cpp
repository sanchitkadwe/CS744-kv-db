// loadgen.cpp -- very small load generator
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>
#include <random>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
using namespace std;

atomic<long> total_requests(0);
atomic<long> total_success(0);
atomic<long long> total_latency_ms(0);

long now_ms() {
    using namespace chrono;
    return chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
}

string make_get(string host, int port, const string &key) {
    string req = "GET /kv/" + key + " HTTP/1.1\r\nHost: " + host + "\r\n\r\n";
    return req;
}
string make_put(string host, int port, const string &key, const string &val) {
    stringstream ss;
    ss << "PUT /kv/" << key << " HTTP/1.1\r\nHost: " << host << "\r\nContent-Length: " << val.size() << "\r\n\r\n" << val;
    return ss.str();
}
string make_del(string host, int port, const string &key) {
    string req = "DELETE /kv/" + key + " HTTP/1.1\r\nHost: " + host + "\r\n\r\n";
    return req;
}

string http_request(const string &host, int port, const string &req) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "";
    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &serv.sin_addr);
    if (connect(sock, (sockaddr*)&serv, sizeof(serv)) < 0) { close(sock); return ""; }
    send(sock, req.c_str(), req.size(), 0);
    char buf[4096];
    string resp;
    ssize_t n = 0;
    while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) resp.append(buf, buf + n);
    close(sock);
    return resp;
}

void worker(int id, const string host, int port, int duration, int keyspace, const string workload) {
    mt19937 rng(id + chrono::steady_clock::now().time_since_epoch().count());
    uniform_int_distribution<int> rd(0, keyspace-1);
    auto end = chrono::steady_clock::now() + chrono::seconds(duration);
    int popular_count = max(1, keyspace/10);
    vector<int> popular;
    for (int i=0;i<popular_count;i++) popular.push_back(i);

    while (chrono::steady_clock::now() < end) {
        string req;
        int k;
        if (workload == "put_all") {
            k = rd(rng);
            req = make_put(host, port, "k" + to_string(k), "v" + to_string(rng()%100000));
        } else if (workload == "get_all") {
            k = rd(rng);
            req = make_get(host, port, "k" + to_string(k));
        } else { // get_popular
            k = popular[rng()%popular.size()];
            req = make_get(host, port, "k" + to_string(k));
        }
        long t0 = now_ms();
        string resp = http_request(host, port, req);
        long t1 = now_ms();
        total_requests++;
        if (!resp.empty()) {
            total_success++;
            total_latency_ms += (t1 - t0);
        }
    }
}

int main(int argc, char** argv) {
    string host = "127.0.0.1";
    int port = 8080;
    int clients = 5;
    int duration = 30;
    int keyspace = 100;
    string workload = "get_popular";

    for (int i=1;i<argc;i++) {
        string a = argv[i];
        if (a=="--host" && i+1<argc) host = argv[++i];
        else if (a=="--port" && i+1<argc) port = stoi(argv[++i]);
        else if (a=="--clients" && i+1<argc) clients = stoi(argv[++i]);
        else if (a=="--duration" && i+1<argc) duration = stoi(argv[++i]);
        else if (a=="--keyspace" && i+1<argc) keyspace = stoi(argv[++i]);
        else if (a=="--workload" && i+1<argc) workload = argv[++i];
    }

    cout << "Loadgen: clients=" << clients << " duration=" << duration << "s workload=" << workload << endl;
    vector<thread> threads;
    for (int i=0;i<clients;i++) threads.emplace_back(worker, i, host, port, duration, keyspace, workload);
    for (auto &t : threads) if (t.joinable()) t.join();

    long tot = total_requests.load();
    long succ = total_success.load();
    long long lat = total_latency_ms.load();
    double avg_rt = succ ? (double)lat / succ : 0.0;
    double throughput = (double)succ / duration;
    cout << "Total requests: " << tot << " success: " << succ << "\n";
    cout << "Throughput (req/s): " << throughput << " avg rt (ms): " << avg_rt << endl;
    return 0;
}
