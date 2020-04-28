/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 */

#ifndef OTA_STORAGE_MOCK_H_
#define OTA_STORAGE_MOCK_H_

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <OTAStorage.h>

#include <vector>

/**************************************************************************************
   CLASS DECLARATION
 **************************************************************************************/

class OTAStorage_Mock : public OTAStorage
{
public:

           OTAStorage_Mock();
  virtual ~OTAStorage_Mock() { }


  virtual bool   init  () override;
  virtual bool   open  () override;
  virtual size_t write (uint8_t const * const buf, size_t const num_bytes) override;
  virtual void   close () override;
  virtual void   remove() override;
  virtual void   deinit() override;

  bool _init_return_val;
  bool _open_return_val;
  std::vector <uint8_t> _binary;

};

#endif /* OTA_STORAGE_MOCK_H_ */
