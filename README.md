# KV HTTP Server with LRU cache + MySQL + Load Generator

## Overview
This project implements:
- A multi-threaded HTTP server providing KV storage with in-memory LRU cache and persistent MySQL storage.
- A connection pool to MySQL.
- A thread-pool worker model for client request processing.
- A load generator that can run different workloads and report throughput and average response time.

## Files
- `server.cpp` - main server implementation (socket listener, HTTP parsing, endpoints).
- `threadpool.cpp/h` - simple thread pool implementation.
- `lru_cache.cpp/h` - in-memory LRU cache implementation.
- `db_pool.cpp/h` - MySQL connection pool + DB functions (get/put/delete).
- `utils.cpp/h` - utility helpers (timing, socket helpers).
- `load_generator.cpp` - load generator client program.
- `Makefile` - build.
- `schema.sql` - SQL DDL to create `kv_store` table in MySQL.

## Prerequisites
- Linux (Ubuntu/Debian)
- g++
- mysql-server
- libmysqlclient-dev

Important Commands:
```bash
sudo apt update
sudo apt install g++ make libmysqlclient-dev mysql-server


MySQL Server commands : 
sudo service mysql start
mysql -u root -p
USE kvdb;
SELECT * FROM kv_store;



Build : 
make
./bin/kv_server

Load generator : 
./bin/loadgen --clients 5 --duration 20 --workload get_popular
