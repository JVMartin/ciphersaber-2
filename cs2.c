#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ROUNDS 20

int checkCommand(int argc, char ** argv);
void initState(unsigned char * state, unsigned char * key, int keyLength);
void decrypt(unsigned char * state, char * key,
	char * srcPath, char * destPath);
char * getFileContents(char * path);
//void decrypt();

int main(int argc, char ** argv)
{
	if ( ! checkCommand(argc, argv)) {
		printf("Usage:\n");
		printf("	cs2 encrypt <key> <source path> <dest path>\n");
		printf("	cs2 decrypt <key> <source path> <dest path>\n");
		return 0;
	}

	unsigned char * state;
	char * key;

	state = (unsigned char *) malloc(256);
	key   = (char *)          malloc(256);

	// Grab the key from the argument.
	strcpy(key, argv[2]);

	if (strcmp(argv[1], "decrypt") == 0) {
		decrypt(state, key, argv[3], argv[4]);
	}
	else {
	}

	free(state);
	free(key);
	return 0;
}

/**
 * Check the formatting of the command call.
 */
int checkCommand(int argc, char ** argv)
{
	if (argc != 5) {
		return 0;
	}

	if (strcmp(argv[1], "encrypt") != 0 &&
		strcmp(argv[1], "decrypt") != 0) {
		return 0;
	}

	if (strlen(argv[2]) > 246) {
		printf("That key is too long.\n");
		return 0;
	}
	
	return 1;
}

void initState(unsigned char * state, unsigned char * key, int keyLength)
{
	unsigned char c;
	int i, j, n;

	// Initialize the state array.
	for (i = 0; i < 255; ++i) {
		state[i] = i;
	}

	j =  0;

	for (n = 0; n < ROUNDS; ++n) {
		for (i = 0; i < 255; ++i) {
			j += state[i];
			j += key[i % keyLength];
			j = j % 256;

			// Swap the ith and jth elements of the state array.
			c = state[i];
			state[i] = state[j];
			state[j] = c;
		}
	}
}

void decrypt(unsigned char * state, char * key,
	char * srcPath, char * destPath)
{
	FILE * destFile;
	char * data;
	unsigned char * appendedKey;
	int keyLength;
	int i;

	data = getFileContents(srcPath);

	printf("Key: %s\n", key);
	keyLength = strlen(key);
	for (i = 0; i < keyLength; ++i) {
		printf("%d: %d\n", i, key[i]);
	}
	appendedKey = malloc(keyLength + 11);
	memcpy(appendedKey, key, keyLength);
	memcpy(appendedKey + keyLength, data, 10);
	appendedKey[keyLength + 9] = '\0';
	printf("Key: %s\n", appendedKey);
	for (i = 0; i < keyLength + 10; ++i) {
		printf("%d: %d\n", i, appendedKey[i]);
	}

	initState(state, appendedKey, keyLength + 10);

	free(data);
	free(appendedKey);
}

char * getFileContents(char * path)
{
	FILE * file;
	int fileSize;
	char * data;

	file = fopen(path, "r");
	if ( ! file) {
		printf("Could not open file: %s\n", path);
		return;
	}

	fseek(file, 0L, SEEK_END);
	fileSize = ftell(file);
	rewind(file);

	printf("File size: %d bytes.\n", fileSize);

	data = malloc(fileSize);
	if ( ! data) {
		printf("Could not allocate enough memory.\n");
		return;
	}

	fread(data, fileSize, 1, file);
	fclose(file);
	return data;
}