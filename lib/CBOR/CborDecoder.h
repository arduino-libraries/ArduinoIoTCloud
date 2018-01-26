#ifndef CBORDE_H
#define CBORDE_H

#include "Arduino.h"

#define INT_MAX 2
#define INT_MIN 2


typedef enum {
	STATE_TYPE,
	STATE_PINT,	
    STATE_NINT,
    STATE_BYTES_SIZE,
    STATE_BYTES_DATA,
    STATE_STRING_SIZE,
    STATE_STRING_DATA,
    STATE_ARRAY,
    STATE_MAP,
    STATE_TAG,
    STATE_SPECIAL,
    STATE_ERROR
} CborReaderState;

class CborInput {
	
public:
	CborInput(void *data, int size);
	~CborInput();

	bool hasBytes(unsigned int count);
	unsigned char getByte();
	unsigned short getShort();
	uint32_t getInt();
	uint64_t getLong();
	void getBytes(void *to, int count);
private:
	unsigned char *data;
	int size;
	int offset;
};




class CborListener {
public:
	virtual void OnInteger(int32_t value) = 0;
	virtual void OnBytes(unsigned char *data, unsigned int size) = 0;
	virtual void OnString(String &str) = 0;
	virtual void OnArray(unsigned int size) = 0;
	virtual void OnMap(unsigned int size) = 0;
	virtual void OnTag(uint32_t tag) = 0;
	virtual void OnSpecial(uint32_t code) = 0;
	virtual void OnError(const char *error) = 0;
    virtual void OnExtraInteger(uint64_t value, int sign) {}
    virtual void OnExtraTag(uint64_t tag) {}
    virtual void OnExtraSpecial(uint64_t tag) {}
};

class CborDebugListener : public CborListener {
public:
	virtual void OnInteger(int32_t value);
	virtual void OnBytes(unsigned char *data, unsigned int size);
	virtual void OnString(String &str);
	virtual void OnArray(unsigned int size);
	virtual void OnMap(unsigned int size);
	virtual void OnTag(uint32_t tag);
	virtual void OnSpecial(uint32_t code);
	virtual void OnError(const char *error);

    virtual void OnExtraInteger(uint64_t value, int sign);
    virtual void OnExtraTag(uint64_t tag);
    virtual void OnExtraSpecial(uint64_t tag);
};

class CborReader {
public:
	CborReader(CborInput &input);
	CborReader(CborInput &input, CborListener &listener);
	~CborReader();
	void Run();
	void SetListener(CborListener &listener);
	void GetCborData(String &Cborpackage);
private:
	CborListener *listener;
	CborInput *input;
	CborReaderState state;
	unsigned int currentLength;
};


class CborExampleListener : public CborListener {
  public:
    void OnInteger(int32_t value);
    void OnBytes(unsigned char *data, unsigned int size);
    void OnString(String &str);
    void OnArray(unsigned int size);
    void OnMap(unsigned int size) ;
    void OnTag(uint32_t tag);
    void OnSpecial(uint32_t code);
    void OnError(const char *error);
  };



#endif
