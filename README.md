gcc -g -Wall -o server.o -c server.c
gcc -g -Wall -o request.o -c request.c
request.c:13:20: error: field ‘arrival_time’ has incomplete type
     struct timeval arrival_time;
                    ^~~~~~~~~~~~
request.c:14:20: error: field ‘dispatch_interval’ has incomplete type
     struct timeval dispatch_interval;
                    ^~~~~~~~~~~~~~~~~
request.c:15:20: error: field ‘pickUp_time’ has incomplete type
     struct timeval pickUp_time;
                    ^~~~~~~~~~~
Makefile:29: recipe for target 'request.o' failed
make: *** [request.o] Error 1
