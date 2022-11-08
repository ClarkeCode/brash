#include "bytes.h"

bool isSystemBigEndian() {
	int i = 1;
	char* bytes = (char*) &i;
	return bytes[0] != 1;
}


void _flip8BytesIfSmallEndian(bool systemIsBigE, void* sourceBuffer, void* outBuffer) {
	byte_t* source = sourceBuffer;
	byte_t* output = outBuffer;

	if (systemIsBigE) {
		//no conversion needed
		output[0] = *(source + 0);
		output[1] = *(source + 1);
		output[2] = *(source + 2);
		output[3] = *(source + 3);
		output[4] = *(source + 4);
		output[5] = *(source + 5);
		output[6] = *(source + 6);
		output[7] = *(source + 7);
	}
	else {
		output[0] = *(source + 7);
		output[1] = *(source + 6);
		output[2] = *(source + 5);
		output[3] = *(source + 4);
		output[4] = *(source + 3);
		output[5] = *(source + 2);
		output[6] = *(source + 1);
		output[7] = *(source + 0);
	}
}

void transpose8Bytes(void* sourceBuffer, void* outBuffer) {
	_flip8BytesIfSmallEndian(isSystemBigEndian(), sourceBuffer, outBuffer);
}

//Assumes that the source buffer is the byte representation of a double in Big-Endian order
double readDoubleFromBytes(void* sourceBuffer) {
	double numericVal = 0;
	transpose8Bytes(sourceBuffer, &numericVal);
	return numericVal;
}

void transpose2Bytes(void* sourceBuffer, void* outBuffer) {
	byte_t* source = sourceBuffer;
	byte_t* output = outBuffer;

	if (isSystemBigEndian()) {
		//no conversion needed
		output[0] = *(source + 0);
		output[1] = *(source + 1);
	}
	else {
		output[0] = *(source + 1);
		output[1] = *(source + 0);
	}
}
//Assumes that the source buffer is the byte representation of a double in Big-Endian order
int16_t readInt16FromBytes(void* sourceBuffer) {
	int16_t numericVal = 0;
	transpose2Bytes(sourceBuffer, &numericVal);
	return numericVal;
}
