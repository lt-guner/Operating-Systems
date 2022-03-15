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
	
	// if correct parameters are not passed then exit with error
	if (argc != 2) 
	{ 
		error("USAGE: ./enc_server port#\n");
	} 
	
	// ---------------------------- SERVER SETUP ---------------------------------------
	// adopted from the source code provided on the assignment page

	// needed variables
	int listenSocket, portNumber;
	struct sockaddr_in serverAddress, clientAddress;
  	socklen_t sizeOfClientInfo = sizeof(clientAddress);

	// Clear out the address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));

	// The address should be network capable
    serverAddress.sin_family = AF_INET;

	 // Store the port number
    portNumber = atoi(argv[1]);
    serverAddress.sin_port = htons(portNumber);

	// Allow a client at any address to connect to this server
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	// Create the socket that will listen for connections
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket < 0) 
	{
		error("SERVER: Error opening socket\n");
	}

	// Associate the socket to the port
	if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	{
		error("SERVER: Error binding port\n");
	}
	
	// Start listening for connetions. Allow up to 5 connections to queue up
  	listen(listenSocket, 5); 

	// variables we will need into order to process, keep track, and handle child and parent processes
	pid_t parentPID = getpid();
	int childSocket;
	int numChildProcs = 0;
	int childProcs[5];
	int curChildStatus; // used with waitpid
	pid_t curChild;

	// A while loop that continues while a parent process is running that matches the parent process
	while (parentPID == getpid()) {

		// if there are more than 0 child processes then proceed
		if (numChildProcs > 0) {
			// for loop through number of child processes and use the waitpid
			for (int i = 0; i < numChildProcs; i++) {
				curChild = waitpid(childProcs[i], &curChildStatus, WNOHANG);
				// replace with the most recent child process if exists
				if (curChild != 0) {
					childProcs[i] = childProcs[numChildProcs - 1];
					numChildProcs--;
				}
			}
		}

		// as long as there is no more than 5 child processes we can proceed
		if (numChildProcs < 5) {
	
			// Accept the connection request which creates a connection socket
			childSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
    		if (childSocket < 0){
      			error("SERVER: Error on accept\n");
    		}

			// create a spawnId and use in a switch statement Hint: mimic assignment 3 smallsh
			pid_t spawnPid = fork();
			switch (spawnPid) {
			
			// with case 0 we have a successfule child process
			case 0: ;
				// ------------------------------ HANDSHAKE & ENCRYPT ---------------------------------------------------
				// variables needed for handshakes and encrypoting
				char buffer[1024];
				memset(buffer, '\0', 1024);
				int charsRead;
				int charsWritten;
				int curChar = 0;
				char* plainText;
				int textSize;
				char* cipherKey;
				int curByte;

				// -- HANDSHAKE --

				// read the handshake form the client
				charsRead = 0;
				charsRead = recv(childSocket, buffer, 1023, 0);
				// if nothing is read then error
				if (charsRead < 0){
					error("SERVER: Error reading handshake from client\n");
				}
				// if a handshake is received make check if its from enc_client
				if (strcmp(buffer, "enc_client") == 0) {
					// send handshake back with enc_server if enc_client was received
					charsWritten = 0;
					charsWritten = send(childSocket, "enc_server", strlen("enc_server"), 0);
					// if not written then show error message
					if (charsWritten < 0){
						error("SERVER: Error writing handshake message to client\n");
					}
				}
				// else send a incorrect response
				else{
					charsWritten = send(childSocket, "wrong server", 2, 0);
					charsWritten = 0;
					if (charsWritten < 0){
						error("SERVER: Erorr writing handshake message to client\n");
					} 
				} 

				// -- encryption with received text --

				// getsize of the plaintext sent by the client
				charsRead = 0;
				charsRead = recv(childSocket, buffer, 1023, 0);
				// error is nothing was receieved
				if (charsRead < 0) {
					error("SERVER: Error getting the text size from the client\n");
				}

				// resize plainText and cipherKey to accomodate incoming text size
				textSize = atoi(buffer) + 1;
				cipherKey = (char *)malloc((textSize) * sizeof(char));
				memset(cipherKey, '\0', textSize);
				plainText = (char *)malloc((textSize) * sizeof(char));
				memset(plainText, '\0', textSize);
				
				// request plainText from client
				charsWritten = 0;
				charsWritten = send(childSocket, "reqPlainText", strlen("reqPlainText"), 0);
				// if not receieved then through an error
				if (charsWritten < 0) {
					error("SERVER: ERROR writing plain text request to client\n");
				}

				// receive the plain text from the client and use loop if everything is not all bytes read at once
				curByte = 0;
				charsRead = 0;
				while(curByte < textSize-1){
					charsRead = recv(childSocket, plainText+curByte, textSize-1, 0);
					// issues ith receiving the plain text
					if (charsRead < 0) {
						error("SERVER: Error reading plain text from client\n");
					}
					// increment position of the byte
					curByte += charsRead;
				}
																						
				// request cipherKey from client
				charsWritten = 0;
				charsWritten = send(childSocket, "reqCipherKey", strlen("reqCipherKey"), 0);
				// if nothing is written produce an error
				if (charsWritten < 0) {
					error("SERVER: ERROR writing cipherKey request to socket\n");
				}
				
				// recieve the cipher key and use loop if everything is not all bytes read at once
				curByte = 0;
				charsRead = 0;
				while(curByte < textSize-1){
					charsRead = recv(childSocket, cipherKey+curByte, textSize-1, 0);
					// check to to see if data was received
					if (charsRead < 0){
						error("SERVER: ERROR reading from socket\n");
					}
					// increment position of the byte
					curByte += charsRead;
				}

				// for loop to produce the ciphertext
				for (int i = 0; i < strlen(plainText); i++) {
					// if its not a newline or terminator, then proceed
					if (plainText[i] != '\n' && plainText[i] != '\0')
					{	
						// process the current plain text char if not a space
						if(plainText[i] != ' '){
							plainText[i] -= 65;
							// if the cipher does not contain a space then proceeed
							if(cipherKey[i] != ' '){
								// add the key to the plain text and modulus
								plainText[i] += cipherKey[i] - 65;
								plainText[i] = plainText[i] % 27;
								// if it is 26 make it space with 32
								if(plainText[i] == 26){
									plainText[i] = 32;
								}
								// else make it a character by adding 65
								else{
									plainText[i] += 65;
								}
							}
							// if space in cipher text then proceed
							else{
								// add 26 to plain text for space and take modulus
								plainText[i] += 26;
								plainText[i] = plainText[i] % 27;
								// if plain text is still space then set to 32
								if(plainText[i] == 26){
									plainText[i] = 32;
								}
								// else make it a upper case character
								else{
									plainText[i] += 65;
								}
							}
						}
						// if the plain text char is a space
						else{
							// set to plaintext to space to start out
							plainText[i] = 26;
							// if cipher key is not a space
							if(cipherKey[i] != ' '){
								// add the cipher key to plain text and take modulus
								plainText[i] += cipherKey[i] - 65;
								plainText[i] = plainText[i] % 27;
								// if modulus is 26 then make plaintext a space
								if(plainText[i] == 26){
									plainText[i] = 32;
								}
								// else uppcase
								else{
									plainText[i] += 65;
								}
							}
							// else add 26 for an additional space and take the modulus of 52 % 27 which should be Z
							else{
								plainText[i] += 26;
								plainText[i] = plainText[i] % 27;
								plainText[i] += 65;
							} 
						}
					}
					// make a new line of newline or terminatior is found and break
					else if (plainText[i] == '\n' || plainText[i] == '\0') 
					{
						plainText[i] = '\n';
						break;
					}
				}

				// send encrypted text back to client
				charsWritten = send(childSocket, plainText, strlen(plainText), 0);
        		// if nothing was written then eroor
				if (charsWritten < 0) {
          			error("SERVER: Error writing to encrypted text to client\n");
        		}
				// clost the childScoket
				close(childSocket);
				break;
			case -1:
				// fork has failed
				error("SERVER: Fialed to fork\n");
				break;
			default:
				// if it is default the add the spawnid to the processes and increment the count of the chile processes
				childProcs[numChildProcs] = spawnPid;
				numChildProcs++;
				break;
			}
		}
		else {
			//waiting for a child to finish and release a socket
		}

	}
	
	// Close the listening socket	
	close(listenSocket); 	

	return 0;
}