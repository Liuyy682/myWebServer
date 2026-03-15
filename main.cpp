#include "webserver.h"

int main() {
    webserver server(8080, false, 8, 3);
    
    server.start();

    return 0;
}