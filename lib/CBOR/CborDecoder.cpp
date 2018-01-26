#include "CborDecoder.h"
#include "Arduino.h"



CborInput::CborInput(void *data, int size) {
	this->data = (unsigned char *)data;
	this->size = size;
	this->offset = 0;
}

CborInput::~CborInput() {}


bool CborInput::hasBytes(unsigned int count) {
	return size - offset >= count;
}

unsigned char CborInput::getByte() {
	return data[offset++];
}

unsigned short CborInput::getShort() {
	unsigned short value = ((unsigned short)data[offset] << 8) | ((unsigned short)data[offset + 1]);
	offset += 2;
	return value;
}

uint32_t CborInput::getInt() {
	uint32_t value = ((uint32_t)data[offset] << 24) | ((uint32_t)data[offset + 1] << 16) | ((uint32_t)data[offset + 2] << 8) | ((uint32_t)data[offset + 3]);
	offset += 4;
	return value;
}

uint64_t CborInput::getLong() {
	uint64_t value = ((uint64_t)data[offset] << 56) | ((uint64_t)data[offset+1] << 48) | ((uint64_t)data[offset+2] << 40) | ((uint64_t)data[offset+3] << 32) | ((uint64_t)data[offset+4] << 24) | ((uint64_t)data[offset+5] << 16) | ((uint64_t)data[offset+6] << 8) | ((uint64_t)data[offset+7]);
	offset += 8;
	return value;
}

void CborInput::getBytes(void *to, int count) {
	memcpy(to, data + offset, count);
	offset += count;
}


CborReader::CborReader(CborInput &input) {
	this->input = &input;
	this->state = STATE_TYPE;
}

CborReader::CborReader(CborInput &input, CborListener &listener) {
	this->input = &input;
	this->listener = &listener;
	this->state = STATE_TYPE;
}


CborReader::~CborReader() {}


void CborReader::SetListener(CborListener &listener) {
	this->listener = &listener;
}

void CborReader::Run() {
	uint32_t temp;
	while(1) {
		if(state == STATE_TYPE) {
			if(input->hasBytes(1)) {
				unsigned char type = input->getByte();
				unsigned char majorType = type >> 5;
				unsigned char minorType = type & 31;

				switch(majorType) {
					case 0: // positive integer
						if(minorType < 24) {
							listener->OnInteger(minorType);
						} else if(minorType == 24) { // 1 byte
							currentLength = 1;
							state = STATE_PINT;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_PINT;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_PINT;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_PINT;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid integer type");
						}
						break;
					case 1: // negative integer
						if(minorType < 24) {
							listener->OnInteger(-1 - minorType);
						} else if(minorType == 24) { // 1 byte
							currentLength = 1;
							state = STATE_NINT;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_NINT;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_NINT;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_NINT;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid integer type");
						}
						break;
					case 2: // bytes
						if(minorType < 24) {
							state = STATE_BYTES_DATA;
							currentLength = minorType;
						} else if(minorType == 24) {
							state = STATE_BYTES_SIZE;
							currentLength = 1;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_BYTES_SIZE;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_BYTES_SIZE;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_BYTES_SIZE;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid bytes type");
						}
						break;
					case 3: // string
						if(minorType < 24) {
							state = STATE_STRING_DATA;
							currentLength = minorType;
						} else if(minorType == 24) {
							state = STATE_STRING_SIZE;
							currentLength = 1;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_STRING_SIZE;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_STRING_SIZE;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_STRING_SIZE;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid string type");
						}
						break;
					case 4: // array
						if(minorType < 24) {
							listener->OnArray(minorType);
						} else if(minorType == 24) {
							state = STATE_ARRAY;
							currentLength = 1;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_ARRAY;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_ARRAY;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_ARRAY;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid array type");
						}
						break;
					case 5: // map
						if(minorType < 24) {
							listener->OnMap(minorType);
						} else if(minorType == 24) {
							state = STATE_MAP;
							currentLength = 1;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_MAP;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_MAP;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_MAP;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid array type");
						}
						break;
					case 6: // tag
						if(minorType < 24) {
							listener->OnTag(minorType);
						} else if(minorType == 24) {
							state = STATE_TAG;
							currentLength = 1;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_TAG;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_TAG;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_TAG;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid tag type");
						}
						break;
					case 7: // special
						if(minorType < 24) {
							listener->OnSpecial(minorType);
						} else if(minorType == 24) {
							state = STATE_SPECIAL;
							currentLength = 1;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_SPECIAL;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_SPECIAL;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_SPECIAL;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid special type");
						}
						break;
				}
			} else break;
		} else if(state == STATE_PINT) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						listener->OnInteger(input->getByte());
						state = STATE_TYPE;
						break;
					case 2:
						listener->OnInteger(input->getShort());
						state = STATE_TYPE;
						break;
					case 4:
						temp = input->getInt();
						if(temp <= INT_MAX) {
							listener->OnInteger(temp);
						} else {
							listener->OnExtraInteger(temp, 1);
						}
						state = STATE_TYPE;
						break;
					case 8:
						listener->OnExtraInteger(input->getLong(), 1);
						state = STATE_TYPE;
						break;
				}
			} else break;
		} else if(state == STATE_NINT) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						listener->OnInteger(-(int32_t)input->getByte());
						state = STATE_TYPE;
						break;
					case 2:
						listener->OnInteger(-(int32_t)input->getShort());
						state = STATE_TYPE;
						break;
					case 4:
						temp = input->getInt();
						if(temp <= INT_MAX) {
							listener->OnInteger(-(int32_t) temp);
						} else if(temp == 2147483648u) {
							listener->OnInteger(INT_MIN);
						} else {
							listener->OnExtraInteger(temp, -1);
						}
						state = STATE_TYPE;
						break;
					case 8:
						listener->OnExtraInteger(input->getLong(), -1);
						break;
				}
			} else break;
		} else if(state == STATE_BYTES_SIZE) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						currentLength = input->getByte();
						state = STATE_BYTES_DATA;
						break;
					case 2:
						currentLength = input->getShort();
						state = STATE_BYTES_DATA;
						break;
					case 4:
						currentLength = input->getInt();
						state = STATE_BYTES_DATA;
						break;
					case 8:
						state = STATE_ERROR;
						listener->OnError("extra long bytes");
						break;
				}
			} else break;
		} else if(state == STATE_BYTES_DATA) {
			if(input->hasBytes(currentLength)) {
				unsigned char *data = new unsigned char[currentLength];
				input->getBytes(data, currentLength);
				state = STATE_TYPE;
				listener->OnBytes(data, currentLength);
			} else break;
		} else if(state == STATE_STRING_SIZE) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						currentLength = input->getByte();
						state = STATE_STRING_DATA;
						break;
					case 2:
						currentLength = input->getShort();
						state = STATE_STRING_DATA;
						break;
					case 4:
						currentLength = input->getInt();
						state = STATE_STRING_DATA;
						break;
					case 8:
						state = STATE_ERROR;
						listener->OnError("extra long array");
						break;
				}
			} else break;
		} else if(state == STATE_STRING_DATA) {
			if(input->hasBytes(currentLength)) {
				unsigned char data[currentLength + 1];
				input->getBytes(data, currentLength);
				data[currentLength] = '\0';
				state = STATE_TYPE;
				String str = String((char*)data);
				listener->OnString(str);
				str = "";
			} else break;
		} else if(state == STATE_ARRAY) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						listener->OnArray(input->getByte());
						state = STATE_TYPE;
						break;
					case 2:
						listener->OnArray(currentLength = input->getShort());
						state = STATE_TYPE;
						break;
					case 4:
						listener->OnArray(input->getInt());
						state = STATE_TYPE;
						break;
					case 8:
						state = STATE_ERROR;
						listener->OnError("extra long array");
						break;
				}
			} else break;
		} else if(state == STATE_MAP) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						listener->OnMap(input->getByte());
						state = STATE_TYPE;
						break;
					case 2:
						listener->OnMap(currentLength = input->getShort());
						state = STATE_TYPE;
						break;
					case 4:
						listener->OnMap(input->getInt());
						state = STATE_TYPE;
						break;
					case 8:
						state = STATE_ERROR;
						listener->OnError("extra long map");
						break;
				}
			} else break;
		} else if(state == STATE_TAG) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						listener->OnTag(input->getByte());
						state = STATE_TYPE;
						break;
					case 2:
						listener->OnTag(input->getShort());
						state = STATE_TYPE;
						break;
					case 4:
						listener->OnTag(input->getInt());
						state = STATE_TYPE;
						break;
					case 8:
						listener->OnExtraTag(input->getLong());
						state = STATE_TYPE;
						break;
				}
			} else break;
		} else if(state == STATE_SPECIAL) {
			if (input->hasBytes(currentLength)) {
				switch (currentLength) {
					case 1:
						listener->OnSpecial(input->getByte());
						state = STATE_TYPE;
						break;
					case 2:
						listener->OnSpecial(input->getShort());
						state = STATE_TYPE;
						break;
					case 4:
						listener->OnSpecial(input->getInt());
						state = STATE_TYPE;
						break;
					case 8:
						listener->OnExtraSpecial(input->getLong());
						state = STATE_TYPE;
						break;
				}
			} else break;
		} else if(state == STATE_ERROR) {
			break;
		} else {
			Serial.print("UNKNOWN STATE");
		}
	}
}

void CborReader::GetCborData(String &Cborpackage) {
	uint32_t temp;
	volatile char commaChar = ',';
	int tempint;

	while(1) {
		if(state == STATE_TYPE) {
			if(input->hasBytes(1)) {
				unsigned char type = input->getByte();
				unsigned char majorType = type >> 5;
				unsigned char minorType = type & 31;
				//Serial.write(majorType);
				//Serial.println("minorType:" + minorType);

				switch(majorType) {
					case 0: // positive integer
						if(minorType < 24) {
							listener->OnInteger(minorType);
							Cborpackage += minorType ;
							Cborpackage += commaChar;
						} else if(minorType == 24) { // 1 byte
							currentLength = 1;
							state = STATE_PINT;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_PINT;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_PINT;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_PINT;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid integer type");
						}
						break;
					case 1: // negative integer
						if(minorType < 24) {
							listener->OnInteger(-1 - minorType);
							Cborpackage += -1 -minorType ;
							Cborpackage += commaChar;

						} else if(minorType == 24) { // 1 byte
							currentLength = 1;
							state = STATE_NINT;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_NINT;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_NINT;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_NINT;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid integer type");
						}
						break;
					case 2: // bytes
						if(minorType < 24) {
							state = STATE_BYTES_DATA;
							currentLength = minorType;
						} else if(minorType == 24) {
							state = STATE_BYTES_SIZE;
							currentLength = 1;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_BYTES_SIZE;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_BYTES_SIZE;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_BYTES_SIZE;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid bytes type");
						}
						break;
					case 3: // string
						if(minorType < 24) {
							state = STATE_STRING_DATA;
							currentLength = minorType;
						} else if(minorType == 24) {
							state = STATE_STRING_SIZE;
							currentLength = 1;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_STRING_SIZE;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_STRING_SIZE;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_STRING_SIZE;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid string type");
						}
						break;
					case 4: // array
						if(minorType < 24) {
							listener->OnArray(minorType);
						} else if(minorType == 24) {
							state = STATE_ARRAY;
							currentLength = 1;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_ARRAY;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_ARRAY;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_ARRAY;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid array type");
						}
						break;
					case 5: // map
						if(minorType < 24) {
							listener->OnMap(minorType);
						} else if(minorType == 24) {
							state = STATE_MAP;
							currentLength = 1;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_MAP;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_MAP;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_MAP;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid array type");
						}
						break;
					case 6: // tag
						if(minorType < 24) {
							listener->OnTag(minorType);
						} else if(minorType == 24) {
							state = STATE_TAG;
							currentLength = 1;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_TAG;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_TAG;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_TAG;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid tag type");
						}
						break;
					case 7: // special
						if(minorType < 24) {
							listener->OnSpecial(minorType);
							Serial.print("h");
						} else if(minorType == 24) {
							state = STATE_SPECIAL;
							currentLength = 1;
						} else if(minorType == 25) { // 2 byte
							currentLength = 2;
							state = STATE_SPECIAL;
						} else if(minorType == 26) { // 4 byte
							currentLength = 4;
							state = STATE_SPECIAL;
						} else if(minorType == 27) { // 8 byte
							currentLength = 8;
							state = STATE_SPECIAL;
						} else {
							state = STATE_ERROR;
							listener->OnError("invalid special type");
						}
						break;
				}
			} else break;
		} else if(state == STATE_PINT) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						temp =(int32_t) input->getByte();
						listener->OnInteger(temp);
						Cborpackage += temp;
						Cborpackage += commaChar;
						state = STATE_TYPE;
						break;
					case 2:
						temp =(int32_t) input->getShort();
						listener->OnInteger(temp);
						Cborpackage += temp;
						Cborpackage += commaChar;
						state = STATE_TYPE;
						break;
					case 4:
						temp =(int32_t) input->getInt();
						if(temp <= INT_MAX) {
							listener->OnInteger(temp);
							Cborpackage += temp;
							Cborpackage += commaChar;

						} else {

							listener->OnExtraInteger(temp, 1);
							Cborpackage += temp ;
							Cborpackage += commaChar;

						}
						state = STATE_TYPE;
						break;
					case 8:
						temp =(int32_t) input->getLong();
						listener->OnExtraInteger(temp, 1);

						Cborpackage += temp ;
						Cborpackage += commaChar;
						state = STATE_TYPE;
						break;
				}
			} else break;
		} else if(state == STATE_NINT) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						listener->OnInteger(-(int32_t)input->getByte());
						Cborpackage +=  (int32_t) input->getByte() ;
						Cborpackage += commaChar;
						Serial.print("h");

						state = STATE_TYPE;
						break;
					case 2:
						listener->OnInteger(-(int32_t)input->getShort());
						Cborpackage +=  (int32_t) input->getShort();
						Cborpackage += commaChar;
						Serial.print("h");

						state = STATE_TYPE;
						break;
					case 4:
						temp = input->getInt();
						if(temp <= INT_MAX) {
							listener->OnInteger(-(int32_t) temp);
							Cborpackage += (int32_t) temp ;

						Serial.print("h");
							Cborpackage += commaChar;
						} else if(temp == 2147483648u) {
							listener->OnInteger(INT_MIN);
							Serial.print("h");

							Cborpackage += INT_MIN ;
							Cborpackage += commaChar;
						} else {
							listener->OnExtraInteger(temp, -1);
						}
						state = STATE_TYPE;
						break;
					case 8:
						listener->OnExtraInteger(input->getLong(), -1);
						Serial.print("h");
						//Cborpackage += input->getLong() ;
						Cborpackage += commaChar;

						break;
				}
			} else break;
		} else if(state == STATE_BYTES_SIZE) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						currentLength = input->getByte();
						state = STATE_BYTES_DATA;
						break;
					case 2:
						currentLength = input->getShort();
						state = STATE_BYTES_DATA;
						break;
					case 4:
						currentLength = input->getInt();
						state = STATE_BYTES_DATA;
						break;
					case 8:
						state = STATE_ERROR;
						listener->OnError("extra long bytes");
						break;
				}
			} else break;
		} else if(state == STATE_BYTES_DATA) {
			if(input->hasBytes(currentLength)) {
				unsigned char *data = new unsigned char[currentLength];
				input->getBytes(data, currentLength);
				state = STATE_TYPE;
				listener->OnBytes(data, currentLength);
				Cborpackage +=  String(INT_MIN, DEC) ;
				Cborpackage += commaChar;
			} else break;
		} else if(state == STATE_STRING_SIZE) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						currentLength = input->getByte();
						state = STATE_STRING_DATA;
						break;
					case 2:
						currentLength = input->getShort();
						state = STATE_STRING_DATA;
						break;
					case 4:
						currentLength = input->getInt();
						state = STATE_STRING_DATA;
						break;
					case 8:
						state = STATE_ERROR;
						listener->OnError("extra long array");
						break;
				}
			} else break;
		} else if(state == STATE_STRING_DATA) {
			if(input->hasBytes(currentLength)) {
				unsigned char data[currentLength];
				//Serial.print("currentLength:");
				//Serial.println(currentLength);
				input->getBytes(data, currentLength);
				state = STATE_TYPE;
				String str = (const char *) data;
				Cborpackage +=  (const char *) data;
				Cborpackage += commaChar;
				listener->OnString(str);
				str = "";
			} else break;
		} else if(state == STATE_ARRAY) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						listener->OnArray(input->getByte());
						state = STATE_TYPE;
						break;
					case 2:
						listener->OnArray(currentLength = input->getShort());
						state = STATE_TYPE;
						break;
					case 4:
						listener->OnArray(input->getInt());
						state = STATE_TYPE;
						break;
					case 8:
						state = STATE_ERROR;
						listener->OnError("extra long array");
						break;
				}
			} else break;
		} else if(state == STATE_MAP) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						listener->OnMap(input->getByte());
						state = STATE_TYPE;
						break;
					case 2:
						listener->OnMap(currentLength = input->getShort());
						state = STATE_TYPE;
						break;
					case 4:
						listener->OnMap(input->getInt());
						state = STATE_TYPE;
						break;
					case 8:
						state = STATE_ERROR;
						listener->OnError("extra long map");
						break;
				}
			} else break;
		} else if(state == STATE_TAG) {
			if(input->hasBytes(currentLength)) {
				switch(currentLength) {
					case 1:
						listener->OnTag(input->getByte());
						state = STATE_TYPE;
						break;
					case 2:
						listener->OnTag(input->getShort());
						state = STATE_TYPE;
						break;
					case 4:
						listener->OnTag(input->getInt());
						state = STATE_TYPE;
						break;
					case 8:
						listener->OnExtraTag(input->getLong());
						state = STATE_TYPE;
						break;
				}
			} else break;
		} else if(state == STATE_SPECIAL) {
			if (input->hasBytes(currentLength)) {
				switch (currentLength) {
					case 1:
						listener->OnSpecial(input->getByte());
						state = STATE_TYPE;
						break;
					case 2:
						listener->OnSpecial(input->getShort());
						state = STATE_TYPE;
						break;
					case 4:
						listener->OnSpecial(input->getInt());
						state = STATE_TYPE;
						break;
					case 8:
						listener->OnExtraSpecial(input->getLong());
						state = STATE_TYPE;
						break;
				}
			} else break;
		} else if(state == STATE_ERROR) {
			break;
		} else {
			Serial.print("UNKNOWN STATE");
		}
	}
}


// TEST HANDLERS

void CborDebugListener::OnInteger(int32_t value) {
	Serial.print("integer:");
	Serial.println(value);
	//Cborpackage +=  value;
	//Cborpackage += commaChar;
}

void CborDebugListener::OnBytes(unsigned char *data, unsigned int size) {
	Serial.print("bytes with size");
	Serial.println(size);
}

void CborDebugListener::OnString(String &str) {
	//Serial.print("string:");
	//int lastStringLength = str.length();
	//Serial.print(lastStringLength);
	//Serial.println(str);
}

void CborDebugListener::OnArray(unsigned int size) {
	Serial.print("array:");
	Serial.println(size);
}

void CborDebugListener::OnMap(unsigned int size) {
	Serial.print("map:");
	Serial.println(size);
}

void CborDebugListener::OnTag(uint32_t tag) {
	Serial.print("tag:");
	Serial.println(tag);

}

void CborDebugListener::OnSpecial(uint32_t code) {
	Serial.println("special" + code);
}

void CborDebugListener::OnError(const char *error) {
	Serial.print("error:");

	Serial.println(error);
}

void CborDebugListener::OnExtraInteger(uint64_t value, int sign) {
	if(sign >= 0) {
		Serial.println("extra integer: %llu\n" + value);
	} else {
		Serial.println("extra integer: -%llu\n" + value);
	}
}

void CborDebugListener::OnExtraTag(uint64_t tag) {
	Serial.println("extra tag: %llu\n" + tag);
}

void CborDebugListener::OnExtraSpecial(uint64_t tag) {
	Serial.println("extra special: %llu\n" + tag);

}
