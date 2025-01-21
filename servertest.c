#include "segel.h"

void runTests() {
    printf("Starting tests for the web server:\n");

    

    // Test 6: Check multiple concurrent VIP and GET requests
    printf("Test personal: Sending concurrent VIP and regular requests\n");
    
    
    for (int i = 0; i < 8; i++){
		char command1[256];
		sprintf(command1, "./client localhost 8080 /home.html GET &");
		system(command1);
	}
	
	for (int i = 0; i < 4; i++){
		char command1[256];
		sprintf(command1, "./client localhost 8080 /home.html REAL &");
		system(command1);
	}
    
    for (int i = 0; i < 5; i++){
		char command1[256];
		sprintf(command1, "./client localhost 8080 /home.html GET &");
		system(command1);
	}
	
	for (int i = 0; i < 3; i++){
		char command1[256];
		sprintf(command1, "./client localhost 8080 /home.html REAL &");
		system(command1);
	}
}

int main() {
    runTests();
    return 0;
}
