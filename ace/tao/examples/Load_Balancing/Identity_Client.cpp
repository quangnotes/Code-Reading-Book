// Identity_Client.cpp,v 1.6 2001/03/26 21:16:52 coryan Exp
// ============================================================================
//
// = LIBRARY
//    TAO/examples/Load_Balancing
//
// = FILENAME
//    Identity_Client.cpp
//
// = AUTHOR
//    Marina Spivak <marina@cs.wustl.edu>
//
// ============================================================================

#include "Identity_Client.h"
#include "IdentityC.h"
#include "Load_BalancerC.h"

#include "tao/debug.h"

#include "ace/Get_Opt.h"

Identity_Client::Identity_Client (void)
  : group_factory_ior_ (0),
    number_of_invocations_ (5),
    use_random_ (0)
{
}

int
Identity_Client::parse_args (int argc, char *argv[])
{
  ACE_Get_Opt get_opts (argc, argv, "di:n:r");
  int c;

  while ((c = get_opts ()) != -1)
    switch (c)
      {
      case 'd':  // debug flag.
        TAO_debug_level++;
        break;
      case 'i': // ior of the <Object_Group_Factory> object.
        this->group_factory_ior_ = get_opts.optarg;
        break;
      case 'n': // number of times to make invocation on an <Identity> object.
        this->number_of_invocations_ = ACE_OS::atoi (get_opts.optarg);
        break;
      case 'r': // flag signifying to obtain references to <Identity>
                // objects from the random <Object_Group> rather than
                // from the round robin one.
        this->use_random_ = 1;
        break;
      case '?':
      default:
        ACE_ERROR_RETURN ((LM_ERROR,
                           "usage:  %s"
                           " [-d]"
                           " [-i] <Object_Group_Factory_ior>"
                           " [-n] <number_of_invocations>"
                           " [-r]"
                           "\n",
                           argv [0]),
                          -1);
      }

  // Indicates successful parsing of command line.
  return 0;
}

int
Identity_Client::init (int argc,
                       char* argv[])
{
  int result;

  ACE_DECLARE_NEW_CORBA_ENV;
  ACE_TRY
    {
      result = this->orb_manager_.init (argc,
                                        argv,
                                        ACE_TRY_ENV);
      ACE_TRY_CHECK;
      if (result == -1)
        return result;

      // Check the non-ORB arguments.
      result = this->parse_args (argc, argv);
      if (result < 0)
        return result;
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "Identity_Client::init");
      return -1;
    }
  ACE_ENDTRY;
  ACE_CHECK_RETURN (-1);

  return 0;
}

int
Identity_Client::run (CORBA::Environment &ACE_TRY_ENV)
{
  ACE_DEBUG ((LM_DEBUG, "Identity_Client: Initialized \n"));

  // Contact the <Object_Group_Factory> to obtain an <Object_Group>.
  CORBA::ORB_var orb = orb_manager_.orb ();
  CORBA::Object_var obj =
    orb->string_to_object (this->group_factory_ior_,
                           ACE_TRY_ENV);
  ACE_CHECK_RETURN (-1);
  Load_Balancer::Object_Group_Factory_var factory =
    Load_Balancer::Object_Group_Factory::_narrow (obj.in (),
                                                  ACE_TRY_ENV);
  ACE_CHECK_RETURN (-1);

  if (CORBA::is_nil (factory.in ()))
    ACE_ERROR_RETURN ((LM_ERROR,
                       "Identity_Client: problems using the factory ior\n"),
                      -1);

  const char *group_name;
  if (this->use_random_)
    group_name = "Identity, Random";
  else
    group_name = "Identity, Round Robin";

  ACE_DEBUG ((LM_DEBUG,
              "Identity_Client: Requesting Object Group "
              "with id <%s>\n", group_name));
  Load_Balancer::Object_Group_var object_group =
    factory->resolve (group_name,
                      ACE_TRY_ENV);
  ACE_CHECK_RETURN (-1);

  // List <Object_Group>'s id.
  CORBA::String_var id = object_group->id (ACE_TRY_ENV);
  ACE_CHECK_RETURN (-1);

  if (ACE_OS::strcmp (id.in (), group_name) != 0)
    ACE_ERROR_RETURN ((LM_ERROR,
                       "Identity_Client: incorrect object group"
                       "returned from factory->resolve\n"),
                      -1);

  // List all <Object_Group>s members.
  ACE_DEBUG ((LM_DEBUG,
              "Identity_Client: Requesting member list of <%s> Object Group\n",
              group_name));

  Load_Balancer::Member_ID_List_var id_list =
    object_group->members (ACE_TRY_ENV);
  ACE_CHECK_RETURN (-1);

  ACE_DEBUG ((LM_DEBUG,
              "Identity_Client: The Group contains %d members:\n",
              id_list->length ()));
  for (CORBA::ULong i = 0; i < id_list->length (); ++i)
    ACE_DEBUG ((LM_DEBUG, "                 <%s>\n",
                (id_list[i]).in ()));

  // Perform <number_of_invocations_> method calls on <Identity>
  // objects, which are members of the <Object_Group>.  Before each
  // invocations, we get an <Identity> reference to use for that
  // invocation from our <Object_Group>.
  Identity_var identity_object;
  CORBA::String_var identity;

  ACE_DEBUG ((LM_DEBUG,
              "Identity_Client: Performing %d invocation(s), "
              "consulting the <%s> Group\n"
              "                 for Identity object "
              "to use before each invocation\n",
              this->number_of_invocations_,
              group_name));

  for (size_t ind = 0; ind < this->number_of_invocations_; ++ind)
    {
      obj = object_group->resolve (ACE_TRY_ENV);
      ACE_CHECK_RETURN (-1);

      identity_object = Identity::_narrow (obj.in (),
                                           ACE_TRY_ENV);
      ACE_CHECK_RETURN (-1);
      if (CORBA::is_nil (identity_object.in ()))
        ACE_ERROR_RETURN ((LM_ERROR,
                           "Identity_Client: cannot narrow an object received from"
                           "<Object_Group::resolve> to <Identity>\n"),
                          -1);
      identity_object->get_name (identity.out (),
                                 ACE_TRY_ENV);
      ACE_CHECK_RETURN (-1);
    }

  ACE_DEBUG ((LM_DEBUG,
              "Identity_Client: Done\n"));

  return 0;
}

Identity_Client::~Identity_Client (void)
{
}

int
main (int argc, char *argv[])
{
  int result = 0;
  Identity_Client client;

  if (client.init (argc, argv) == -1)
    return 1;

  ACE_DECLARE_NEW_CORBA_ENV;
  ACE_TRY
    {
      result = client.run (ACE_TRY_ENV);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "Identity_Client");
      return 1;
    }
  ACE_ENDTRY;
  ACE_CHECK_RETURN (1);

  if (result == -1)
    return 1;
  else
    return 0;
}
