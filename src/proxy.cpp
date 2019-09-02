#include "proxy.hpp"

#include <cstdlib>

#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h> //inet_ntoa()
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/param.h>
#include <sys/select.h>

#include <vector>
#include <string>
#include "naiveconfig.hpp"

#include <iostream>

proxy::proxy()
{

}

proxy::~proxy()
{

}
