#include "webserver.h"

int main() {
    webserver server(1234, false, 8, 3, 0);
    
    server.start();

    return 0;
}