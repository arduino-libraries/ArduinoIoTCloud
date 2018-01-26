cbor-cpp
========

CBOR C++ serialization library

Just a simple SAX-like Concise Binary Object Representation (CBOR).

[http://tools.ietf.org/html/rfc7049](http://tools.ietf.org/html/rfc7049)

#### Examples

Writing:

```C++
  CborDynamicOutput output;
  CborWriter writer(output);

  writer.writeTag(123);
  writer.writeArray(3);
  writer.writeString("hello");
  writer.writeString("world");
  writer.writeInt(321);

  unsigned char *data = output.getData();
  int size = output.getSize();
```

Reading:

```C++
  class CborExampleListener : public CborListener {
  public:
    virtual void OnInteger(int32_t value);
    virtual void OnBytes(unsigned char *data, unsigned int size);
    virtual void OnString(std::string &str);
    virtual void OnArray(unsigned int size);
    virtual void OnMap(unsigned int size);
    virtual void OnTag(uint32_t tag);
    virtual void OnSpecial(uint32_t code);
    virtual void OnError(const char *error);
  };

  ...
  
  void CborExampleListener::OnInteger(int32_t value) {
    printf("integer: %d\n", value);
  }
  
  void CborExampleListener::OnBytes(unsigned char *data, unsigned int size) {
    printf("bytes with size: %d", size);
  }
  
  void CborExampleListener::OnString(string &str) {
    printf("string: '%.*s'\n", (int)str.size(), str.c_str());
  }
  
  void CborExampleListener::OnArray(unsigned int size) {
    printf("array: %d\n", size);
  }
  
  void CborExampleListener::OnMap(unsigned int size) {
    printf("map: %d\n", size);
  }
  
  void CborExampleListener::OnTag(unsigned int tag) {
    printf("tag: %d\n", tag);
  }
  
  void CborExampleListener::OnSpecial(unsigned int code) {
    printf("special: %d\n", code);
  }
  
  void CborExampleListener::OnError(const char *error) {
    printf("error: %s\n", error);
  }
  
  ...
  CborInput input(data, size);
  CborReader reader(input);
  CborExampleListener listener;
  reader.SetListener(listener);
  reader.Run();
```
