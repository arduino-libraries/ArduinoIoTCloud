#ifndef ARDUINO_CLOUD_PROPERTY_GENERIC_H_
#define ARDUINO_CLOUD_PROPERTY_GENERIC_H_

#include "lib/tinycbor/cbor-lib.h"

typedef enum {
    READ    = 0b01,
    WRITE   = 0b10,
    READWRITE = READ | WRITE,
} permissionType;

typedef enum {
    INT,
    FLOAT,
    BOOL,
    STRING
} propertyType;

// definition of the default property update policy
static const int ON_CHANGE = -1;

class ArduinoCloudPropertyGeneric {
    public:

        ArduinoCloudPropertyGeneric(String const & name,
                                    propertyType const property_type,
                                    permissionType const permission,
                                    long const update_policy,
                                    void(*fn)(void))
        : _name(name),
          _property_type(property_type),
          _permission(permission),
          _update_policy(_update_policy),
          _last_updated(0),
          _tag(-1),
          callback(fn)
        {
        }

        inline String const & getName        () const { return _name;          }
        inline propertyType   getType        () const { return _property_type; }
        inline permissionType getPermission  () const { return _permission;    }
        inline long           getUpdatePolicy() const { return _update_policy; }
        inline int            getTag         () const { return _tag;           }

        inline  bool          canWrite       () const { return (_permission & WRITE); }
        inline  bool          canRead        () const { return (_permission & READ);  }



        virtual bool newData() const = 0;
        virtual void updateShadow() = 0;
        virtual void printinfo(Stream& stream) = 0;
        virtual void appendValue(CborEncoder* mapEncoder) = 0;

        bool shouldBeUpdated() const
        {
          if  (_update_policy == ON_CHANGE) return newData();
          else                              return ((millis() - _last_updated) > (_update_policy * 1000));
        }

        void append(CborEncoder * encoder) {
          if (!canRead()) {
            return;
          }
          CborEncoder mapEncoder;
          cbor_encoder_create_map(encoder, &mapEncoder, CborIndefiniteLength);
          if (_tag != -1) {
            cbor_encode_text_stringz(&mapEncoder, "t");
            cbor_encode_int(&mapEncoder, _tag);
          }
          else {
            cbor_encode_text_stringz(&mapEncoder, "n");
            cbor_encode_text_stringz(&mapEncoder, getName().c_str());
          }
          appendValue(&mapEncoder);
          cbor_encoder_close_container(encoder, &mapEncoder);
          _last_updated = millis();
        }

        void(*callback)(void) = NULL;

      private:

        String          _name;
        propertyType    _property_type;
        permissionType  _permission;
        long            _update_policy;
        long            _last_updated;
        int             _tag;

};

inline bool operator == (ArduinoCloudPropertyGeneric const & lhs, ArduinoCloudPropertyGeneric const & rhs) { return (lhs.getName() == rhs.getName()); }

#endif /* ARDUINO_CLOUD_PROPERTY_GENERIC_H_ */
