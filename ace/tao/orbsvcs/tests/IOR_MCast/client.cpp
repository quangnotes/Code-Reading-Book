// client.cpp,v 1.1 2001/09/05 22:07:49 doccvs Exp

#include "ior_mcast_client_i.h"

int main (int argc, char *argv [])
{

  ACE_DECLARE_NEW_CORBA_ENV;

  ACE_TRY
    {

      ior_mcast_Client_i client;

      int init_result;
      init_result = client.init (argc, argv, ACE_TRY_ENV);
      ACE_TRY_CHECK;

      return 0;
    }
  ACE_CATCH (CORBA::SystemException, ex)
    {
      //
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "client");
    }
  ACE_ENDTRY;
  ACE_CHECK_RETURN (-1);

  return 0;
}
