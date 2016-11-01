#include "../inc/main.h"

unsigned int numTok;
char buffer[MAX_INPUT];
/*Buffer to save user input.*/

char *tok[MAX_TOK];
/*array of saving string and number of tokens*/

char* readLine(void){

	fgets(buffer, MAX_INPUT, stdin);
	/*fgets method will store input from stdin to buffer thats size of BUF_SIZE*/
	if(strlen(buffer) != 1){
		buffer[strlen(buffer)-1] = '\0';
	}else{
		return '\0';
	}
	/*Since the last character in buffer is '\n' 
	  it should be changed to nullcharacter which is '\0'*/
	return buffer;
}
/*readLine method will read input until user hit '\n' character*/

char** parseTok(char* input){
	/*int i is for indexing of array and saving number of token*/
	numTok=0;
	
	tok[numTok++] = strtok(input," ");
	/*first token is saved to tok[0] then increment i by 1*/
	
	while((tok[numTok]=strtok(NULL," "))!=NULL){
	/*This while loop will iterate tokens in input until there is no token left
	  each token will be saved to temp*/
		numTok++;
		/*increse numTok by 1*/	
		if(numTok>=MAX_TOK){
			printf("Please use less than %d space total when typing\n",MAX_TOK );
			return NULL;
		}
	}
	return tok;
}
