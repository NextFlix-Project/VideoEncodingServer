#include <iostream>
#include <string>
#include <cstdlib>

#include "./src/Server/Server.h"
#include "./src/VideoEncoder/VideoEncoder.h"

int main(int argc, char *argv[])
{
    NextFlix::Server *server = new NextFlix::Server(8800);

    return 0;
}
