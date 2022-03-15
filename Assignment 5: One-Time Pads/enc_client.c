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
        error("USAGE: ./client.c plaintext keygenfile port\n");
    }

    // -------------------------------------- FILE PROCESSING ----------------------------------------------------------------------------------------------------
    
    // variables to process the input files
    FILE* inputfile = fopen(argv[1], "r");
    FILE* cipherscript = fopen(argv[2], "r");
    size_t bufsize = 32;
    char *textChars;
    char *keyChars;
    size_t textCharsCount;
    size_t keyCharsCount;

    // if etiher enconding or cipher code file is null print a send to stderr
    if (inputfile == NULL ){
        error("CLIENT: File to encode not found\n");
    }
    if (cipherscript == NULL){
        error("CLIENT: cipher script not found\n");
    }
    
    // pull the data from the the plaintext file
    textChars = (char *)malloc(bufsize * sizeof(char));
    textCharsCount = getline(&textChars, &bufsize, inputfile);

    // check whethet there is data or if the file did not open or space is not available
    if(textCharsCount == 0){
        error("CLIENT: Plain text file is empty\n");
    }
    else if (textCharsCount == -1){
        error("CLIENT: Failed to read plain text file\n");
    }
    else if (textChars == NULL){
        error("CLIENT: Space could not be allocated for plain text processing\n");
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

    // exit with stderr if cipher script is shorter than plain text
    if (strlen(keyChars) < strlen(textChars)){
        error("CLIENT: Cipher script is shorter than plain text\n");
    }

    // iterate through plaintext to make sure it has valid characters
    for(int i = 0; i < strlen(textChars); i++)
    {
        // check if it contains the valid inputs in the of upper case, space, and newline
        if (isupper(textChars[i]) != 0 || textChars[i] == ' ' || textChars[i] == '\n') 
        {
            // skip
        }
        // if not then print stderr message
        else{
            error("CLIENT: Invalid character in plaintext file\n");
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
	char plainTextSize[10];
	int charsRead;
	int charsWritten;
    int curByte;

	// -- handshake --
	
	// send over the handshake of enc_client
    charsWritten = 0;
	charsWritten = send(socketFD, "enc_client", strlen("enc_client"), 0);
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
	if (strcmp(buffer, "enc_server") != 0) {
		close(socketFD);
		fprintf(stderr, "CLIENT: Could not connect to port %s, terminating process\n", argv[3]);
		exit(2);
	}

	// -- encryption --
	
	// need to send the size of the plain text to the server
	sprintf(plainTextSize, "%ld", textCharsCount);
    charsWritten = 0;
	charsWritten = send(socketFD, plainTextSize, strlen(plainTextSize), 0);
	// send an error message to the if the nothing was written
	if (charsWritten< 0){
        error("CLIENT: Error sending over the buffer size of plain text\n");
	} 
	
	// get request for textChars
	memset(buffer, '\0', sizeof(buffer));
    charsRead = 0;
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
	// send message if the plain text request could not be read
	if (charsRead < 0){
        error("CLIENT: Error reading the plain text request\n");
	}
	// if the proper request was not sent over then display message and exit
	if (strcmp(buffer, "reqPlainText") != 0){
        error("CLIENT: Server failed to send proper plain text request\n");
	}

	// write the plain text to the server if the plain request passes
    charsWritten = 0;
	charsWritten = send(socketFD, textChars, textCharsCount, 0);
	// if the chars were not written then send an error message
	if (charsWritten < 0) {
        error("CLIENT: Error writing the plain text to the socket\n");
	}

	// Get cipher request request
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
	charsWritten = send(socketFD, keyChars, textCharsCount, 0);
	// if nothing was written send an error message
	if (charsWritten < 0){
        error("CLIENT: Error sending over cipher key\n");
	}

    // Get the encrypted textChars from server and use loop to get everything in case bytes were dropped
    curByte = 0;
    charsRead = 0;
    while(curByte < textCharsCount){
        charsRead = recv(socketFD, textChars+curByte, textCharsCount, 0);
        // check to to see if data was received
        if (charsRead < 0){
            error("SERVER: ERROR reading from socket\n");
        }
        // increment position of the byte
        curByte += charsRead;
    }
	
	//print the encrypted text
	printf("%s", textChars);
    // Close the socket
	close(socketFD);

	exit(0);
}