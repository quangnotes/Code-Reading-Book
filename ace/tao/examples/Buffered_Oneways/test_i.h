// test_i.h,v 1.2 2001/05/22 20:44:02 sharath Exp

// ============================================================================
//
// = LIBRARY
//   TAO/examples/Buffered_Oneways/
//
// = FILENAME
//   test_i.h
//
// = AUTHOR
//   Irfan Pyarali
//
// ============================================================================

#ifndef TAO_BUFFERED_ONEWAYS_TEST_I_H
#define TAO_BUFFERED_ONEWAYS_TEST_I_H

#include "testS.h"

class test_i : public POA_test
{
  // = TITLE
  //   Simple implementation.
  //
public:
  test_i (CORBA::ORB_ptr orb);
  // ctor.

  // = The test interface methods.
  void method (CORBA::ULong request_number,
               CORBA::Environment &)
    ACE_THROW_SPEC ((CORBA::SystemException));

  void shutdown (CORBA::Environment &)
    ACE_THROW_SPEC ((CORBA::SystemException));

private:
  CORBA::ORB_var orb_;
  // The ORB.
};

#endif /* TAO_BUFFERED_ONEWAYS_TEST_I_H */
