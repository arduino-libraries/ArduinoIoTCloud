/*
   Copyright 2014-2015 Stanislav Ovsyannikov

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

	   Unless required by applicable law or agreed to in writing, software
	   distributed under the License is distributed on an "AS IS" BASIS,
	   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	   See the License for the specific language governing permissions and
	   limitations under the License.
*/




#ifndef CBOREN_H
#define CBOREN_H

#include "Arduino.h"

class CborOutput {
public:
    virtual unsigned char *getData() = 0;
    virtual unsigned int getSize() = 0;
    virtual void putByte(unsigned char value) = 0;
    virtual void putBytes(const unsigned char *data, const unsigned int size) = 0;
};

class CborStaticOutput : public CborOutput {
public:
	CborStaticOutput(unsigned int capacity);
	~CborStaticOutput();
	virtual unsigned char *getData();
	virtual unsigned int getSize();
	virtual void putByte(unsigned char value);
	virtual void putBytes(const unsigned char *data, const unsigned int size);
private:
	unsigned char *buffer;
	unsigned int capacity;
	unsigned int offset;
};


class CborDynamicOutput : public CborOutput {
public:
    CborDynamicOutput();
    CborDynamicOutput(uint32_t initalCapacity);
    ~CborDynamicOutput();


    virtual unsigned char *getData();
    virtual unsigned int getSize();
    virtual void putByte(unsigned char value);
    virtual void putBytes(const unsigned char *data, const unsigned int size);
private:
    void init(unsigned int initalCapacity);
    unsigned char *buffer;
    unsigned int capacity;
    unsigned int offset;
};

class CborWriter {
public:
	CborWriter(CborOutput &output);
	~CborWriter();

	void writeInt(const int value);
	//void writeInt(const int32_t value);
	void writeInt(const int64_t value);
	void writeInt(const uint32_t value);
	void writeInt(const uint64_t value);
	void writeBytes(const unsigned char *data, const unsigned int size);
	void writeString(const char *data, const unsigned int size);
	void writeString(const String str);
	void writeArray(const unsigned int size);
	void writeMap(const unsigned int size);
	void writeTag(const uint32_t tag);
	void writeSpecial(const uint32_t special);
private:
	void writeTypeAndValue(uint8_t majorType, const uint32_t value);
	void writeTypeAndValue(uint8_t majorType, const uint64_t value);
	CborOutput *output;
};

class CborSerializable {
public:
	virtual void Serialize(CborWriter &writer) = 0;
};
#endif
