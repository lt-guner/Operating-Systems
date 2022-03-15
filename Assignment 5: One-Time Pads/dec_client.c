#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ctype.h>


void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[]) {
	
	// exit if incorrect arguments
    if (argc != 4)
    {
        error("USAGE: ./client.c ciphertext keygenfile port\n");
    }

    // -------------------------------------- FILE PROCESSING ----------------------------------------------------------------------------------------------------
    
    // variables to process the input files
    FILE* inputfile = fopen(argv[1], "r");
    FILE* cipherscript = fopen(argv[2], "r");
    size_t bufsize = 32;
    char *cipherChars;
    char *keyChars;
    size_t cipherCharsCount;
    size_t keyCharsCount;

    // if etiher cipher text or cipher code file is null print a send to stderr
    if (inputfile == NULL ){
        error("CLIENT: File to encode not found\n");
    }
    if (cipherscript == NULL){
        error("CLIENT: cipher script not found\n");
    }
    
    // pull the data from the the ciphertext file
    cipherChars = (char *)malloc(bufsize * sizeof(char));
    cipherCharsCount = getline(&cipherChars, &bufsize, inputfile);

    // check whethet there is data or if the file did not open or space is not available
    if(cipherCharsCount == 0){
        error("CLIENT: Cipher text file is empty\n");
    }
    else if (cipherCharsCount == -1){
        error("CLIENT: Failed to read cipher text file\n");
    }
    else if (cipherChars == NULL){
        error("CLIENT: Space could not be allocated for cipher text processing\n");
    }

    // pull the data from the the cipher script
    keyChars = (char *)malloc(bufsize * sizeof(char));
    keyCharsCount = getline(&keyChars, &bufsize, cipherscript);

    // check whether there is data or if the file did not open
    if(keyCharsCount == 0){
        error("CLIENT: Cipher file is empty\n");
    }
    else if (keyCharsCount == -1){
        error("CLIENT: Failed to read cipher file\n");
    }
    else if (keyChars == NULL){
        error("CLIENT: Space could not be allocated for cipher script processing\n");
    }

    // error if cipher is shorter than cipher
    if (strlen(keyChars) < strlen(cipherChars)){
        error("CLIENT: Cipher script is shorter than cipher text\n");
    }

    // iterate through cipher text to make sure it has valid characters
    for(int i = 0; i < strlen(cipherChars); i++)
    {
        // check if it contains the valid inputs in the of upper case, space, and newline
        if (isupper(cipherChars[i]) != 0 || cipherChars[i] == ' ' || cipherChars[i] == '\n') 
        {
            // skip
        }
        // if not then print stderr message
        else{
            error("CLIENT: Invalid character in ciphertext file\n");
        }
    }

    // iterate through cipherscript to make sure it has valid characters
    for(int i = 0; i < strlen(keyChars); i++)
    {
        // check if it contains the valid inputs in the of upper case, space, and newline
        if (isupper(keyChars[i]) != 0 || keyChars[i] == ' ' || keyChars[i] == '\n') 
        {
            // skip
        }
        // if not then print stderr message
        else{
            error("CLIENT: Invalid character in cipher script file\n");
        }
    }

	// ------------------------------------------- CLIENT SETUP --------------------------------------------------------------
    // Note: Client setup has been adapted from the source code on the assignment page

	//connect to server
    int socketFD, portNumber;
    struct sockaddr_in serverAddress;
    struct hostent* hostInfo;
    char buffer[1024];

	// Clear out the address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));
    
    // The address should be network capable
    serverAddress.sin_family = AF_INET;
    
    // Store the port number
    portNumber = atoi(argv[3]);
    serverAddress.sin_port = htons(portNumber);

	// Get the DNS entry for this host name and exit if it doesnt exist
    hostInfo = gethostbyname("localhost"); 
    if (hostInfo == NULL) 
    {
        error("CLIENT: Error no such host\n");
    }

	// Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)hostInfo->h_addr, hostInfo->h_length);

    // Create a socket and print to stderr if fails
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0)
    {
        error("CLIENT: Error opening socket\n");
    }

	// Connect to server and return failure if not connected
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    {
        error("CLIENT: Error connecting\n");
    }

	// -------------------------------------- HANDSHAKE AND ENCRYPT -------------------------------------------------------------------

	// variables needed for sending and receiving data
	memset(buffer, '\0', 1024);
	char cipherTextSize[10];
	int charsRead;
	int charsWritten;
    int curByte;

	// -- handshake --
	
	// send over the handshake of dec_client
    charsWritten = 0;
	charsWritten = send(socketFD, "dec_client", strlen("dec_client"), 0);
	// error message if the handshake was not written
	if (charsWritten < 0){
        error("CLIENT: Handshake could not be sent by the client\n");
	}

	// Get get return handshake from enc_server
	memset(buffer, '\0', sizeof(buffer));
    charsRead = 0;
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
	// print error message if the server did not send a reply
	if (charsRead < 0){
        error("CLIENT: Handshake could not be received by server\n");
	}
	// if connection cannot be confirmed with correct handshake, then exit,
	if (strcmp(buffer, "dec_server") != 0) {
		close(socketFD);
		fprintf(stderr, "CLIENT: Could not connect to port %s, terminating process\n", argv[3]);
		exit(2);
	}

	// -- encryption --
	
	// need to send the size of the cipher text to the server
	sprintf(cipherTextSize, "%ld", cipherCharsCount);
    charsWritten = 0;
	charsWritten = send(socketFD, cipherTextSize, strlen(cipherTextSize), 0);
	// send an error message to the if the nothing was written
	if (charsWritten< 0){
        error("CLIENT: Error sending over the buffer size of cipher text\n");
	} 
	
	// get request for cipherChars
	memset(buffer, '\0', sizeof(buffer));
    charsRead = 0;
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
	// send message if the cipher text request could not be read
	if (charsRead < 0){
        error("CLIENT: Error reading the cipher text request\n");
	}
	// if the proper request was not sent over then display message and exit
	if (strcmp(buffer, "reqCipherText") != 0){
        error("CLIENT: Server failed to send proper cipher text request\n");
	}

	// write the cipher text to the server if the cipher request passes
    charsWritten = 0;
	charsWritten = send(socketFD, cipherChars, cipherCharsCount, 0);
	// if the chars were not written then send an error message
	if (charsWritten < 0) {
        error("CLIENT: Error writing the cipher text to the socket\n");
	}

	// Get cipher key request request
	memset(buffer, '\0', sizeof(buffer));
    charsRead = 0;
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
	// send message if the cipher key request could not be read
	if (charsRead < 0){
        error("CLIENT: Error reading the cipher key request\n");
	}
	// if the proper request was not sent over then display message and exit
	if (strcmp(buffer, "reqCipherKey") != 0){
        error("CLIENT: Server failed to send proper cipher key request\n");
	}

	// write the cipher key to the server
    charsWritten = 0;
	charsWritten = send(socketFD, keyChars, cipherCharsCount, 0);
	// if nothing was written send an error message
	if (charsWritten < 0){
        error("CLIENT: Error sending over cipher key\n");
	}
    
    // Get the encrypted cipherChars from server
    curByte = 0;
    charsRead = 0;
    while(curByte < cipherCharsCount){
        charsRead = recv(socketFD, cipherChars+curByte, cipherCharsCount, 0);
        // check to to see if data was received
        if (charsRead < 0){
            error("SERVER: ERROR reading from socket\n");
        }
        // increment position of the byte
        curByte += charsRead;
    }
    
	//print the encrypted text
	printf("%s", cipherChars);
    // Close the socket
	close(socketFD);

	exit(0);
}