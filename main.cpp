#include "webserver.h"

int main() {
    webserver server(1234, false, 8, 3, 1);
    
    server.start();

    return 0;
}