#include "threadpool.h"
#include "http_conn.h"
int main() {
    threadpool<http_conn>* pool = NULL;
    pool = new threadpool<http_conn>;
    delete pool;
    return 0; 
}