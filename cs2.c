/**
 * Copyright (c) 2015 Jacob Martin
 *
 * CipherSaber-2 file encryption/decryption, designed to be ran from the
 * command line with a key, source file path, and destination file path.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ROUNDS 20

// See function implementations for clarifying comments.
int checkCommand(int argc, char ** argv);
void initState(unsigned char * state, unsigned char * key, int keyLength);
void cipher(unsigned char * state, unsigned char * data, int length);
void decrypt(unsigned char * state, char * key,
	char * srcPath, char * destPath);
void encrypt(unsigned char * state, char * key,
	char * srcPath, char * destPath);
unsigned char * fileToBytes(char * path, int * stringLength);
int stringToFile(char * string, char * path);
int bytesToFile(unsigned char * bytes, int length, char * path);

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

	state = malloc(256);
	key   = malloc(256);

	// Grab the key from the argument.
	strcpy(key, argv[2]);

	if (strcmp(argv[1], "decrypt") == 0) {
		decrypt(state, key, argv[3], argv[4]);
	}
	else {
		encrypt(state, key, argv[3], argv[4]);
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

/**
 * Initialize the state array.
 */
void initState(unsigned char * state, unsigned char * key, int keyLength)
{
	unsigned char c;
	int i, j, n;

	for (i = 0; i < 256; ++i) {
		state[i] = i;
	}

	j =  0;

	for (n = 0; n < ROUNDS; ++n) {
		for (i = 0; i < 256; ++i) {
			j += state[i] + key[i % keyLength];
			j %= 256;

			// Swap the ith and jth elements.
			c = state[i];
			state[i] = state[j];
			state[j] = c;
		}
	}
}

/**
 * Cipher some data.
 */
void cipher(unsigned char * state, unsigned char * data, int length)
{
	int i, j, k, n;
	unsigned char c;

	i = j = 0;

	for (n = 0; n < length; ++n) {
		i = (i + 1) % 256;
		j = (j + state[i]) % 256;

		// Swap the ith and jth elements.
		c = state[i];
		state[i] = state[j];
		state[j] = c;

		k = (state[i] + state[j]) % 256;
		data[n] ^= state[k];
	}
}

/**
 * Decrypt a file.
 */
void decrypt(unsigned char * state, char * key,
	char * srcPath, char * destPath)
{
	unsigned char * bytes;
	int keyLength;
	int dataLength;

	bytes = fileToBytes(srcPath, &dataLength);
	if ( ! bytes) return;
	keyLength = strlen(key);

	// Append the IV from the file to the key.
	memcpy(key + keyLength, bytes, 10);

	initState(state, (unsigned char *) key, keyLength + 10);

	cipher(state, bytes + 10, dataLength - 10);

	// Write the decrypted string to the destination file.
	stringToFile((char *) bytes + 10, destPath);

	free(bytes);
}

/**
 * Encrypt a file.
 */
void encrypt(unsigned char * state, char * key,
	char * srcPath, char * destPath)
{
	unsigned char * bytes;
	unsigned char * bytesPrepended;
	unsigned char random[10];
	int keyLength;
	int dataLength;
	FILE * randomFile;

	bytes = fileToBytes(srcPath, &dataLength);
	if ( ! bytes) return;
	keyLength = strlen(key);

	randomFile = fopen("/dev/urandom", "r");
	if ( ! randomFile) {
		printf("Could not access /dev/urandom.\n");
		return;
	}
	fread(&random, 1, 10, randomFile);
	fclose(randomFile);

	// Append the IV from random to the key.
	memcpy(key + keyLength, random, 10);

	initState(state, (unsigned char *) key, keyLength + 10);

	cipher(state, bytes, dataLength);

	// Prepend random to the encrypted bytes.
	bytesPrepended = malloc(dataLength + 10);
	memcpy(bytesPrepended, random, 10);
	memcpy(bytesPrepended + 10, bytes, dataLength);

	// Write the prepended, encrypted bytes to the destination file.
	bytesToFile(bytesPrepended, dataLength + 10, destPath);

	free(bytes);
	free(bytesPrepended);
}

/**
 * Read the contents of a file into a cstring.
 */
unsigned char * fileToBytes(char * path, int * stringLength)
{
	FILE * file;
	unsigned char * data;

	file = fopen(path, "r");
	if ( ! file) {
		printf("Could not open file: %s\n", path);
		return 0;
	}

	fseek(file, 0L, SEEK_END);
	*stringLength = ftell(file);
	rewind(file);

	data = malloc(*stringLength + 1);
	if ( ! data) {
		printf("Could not allocate enough memory.\n");
		return 0;
	}

	// Null terminate the string.
	data[*stringLength] = 0;

	fread(data, *stringLength, 1, file);
	fclose(file);
	return data;
}

/**
 * Write the contents of a cstring to a file.
 */
int stringToFile(char * string, char * path)
{
	FILE * file;

	file = fopen(path, "w");
	if ( ! file) {
		printf("Could not create/open file: %s\n", path);
		return 0;
	}

	fputs(string, file);
	fclose(file);
	return 1;
}

/**
 * Write the provided bytes to a file.
 */
int bytesToFile(unsigned char * bytes, int length, char * path)
{
	FILE * file;

	file = fopen(path, "wb");
	if ( ! file) {
		printf("Could not create/open file: %s\n", path);
		return 0;
	}

	fwrite(bytes, 1, length, file);
	fclose(file);
	return 1;
}
