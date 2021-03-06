// -*- C++ -*-

// ===================================================================
/**
 *  @file   ORBInitInfo.h
 *
 *  ORBInitInfo.h,v 1.11 2001/08/16 17:41:57 othman Exp
 *
 *  @author Ossama Othman <ossama@uci.edu>
 */
// ===================================================================

#ifndef TAO_ORB_INIT_INFO_H
#define TAO_ORB_INIT_INFO_H

#include "ace/pre.h"

#include "corbafwd.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "PortableInterceptorC.h"
#include "LocalObject.h"
#include "StringSeqC.h"

// This is to remove "inherits via dominance" warnings from MSVC.
// MSVC is being a little too paranoid.
#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#endif /* _MSC_VER >= 1200 */
#pragma warning(disable:4250)
#endif /* _MSC_VER */

#if defined (__BORLANDC__)
#pragma option push -w-rvl -w-rch -w-ccc -w-inl
#endif /* __BORLANDC__ */

class TAO_ORB_Core;
class TAO_ORBInitInfo_var;
class TAO_ORBInitInfo;
typedef TAO_ORBInitInfo *TAO_ORBInitInfo_ptr;

/**
 * @class TAO_ORBInitInfo
 *
 * @brief An implementation of the PortableInterceptor::ORBInitInfo
 *        interface.
 *
 * This class encapsulates the data passed to ORBInitializers during
 * ORB initialization.
 */
class TAO_Export TAO_ORBInitInfo :
  public virtual PortableInterceptor::ORBInitInfo,
  public virtual TAO_Local_RefCounted_Object
{
  friend CORBA::ORB_ptr CORBA::ORB_init (int &,
                                         char *argv[],
                                         const char *,
                                         CORBA_Environment &);

public:

  /// Constructor.
  TAO_ORBInitInfo (TAO_ORB_Core *orb_core,
                   int argc,
                   char *argv[]);

  /**
   * @name PortableInterceptor::ORBInitInfo Methods
   *
   * These methods are exported by the
   * PortableInterceptor::ORBInitInfo interface.
   */

  //@{
  /// Return the argument vector for the ORB currently being
  /// initialized as a string sequence.
  virtual CORBA::StringSeq * arguments (
      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())
    ACE_THROW_SPEC ((CORBA::SystemException));

  /// Return the ORBid for the ORB currently being initialized.
  virtual char * orb_id (
      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())
    ACE_THROW_SPEC ((CORBA::SystemException));

  /// Return the CodecFactory for the ORB currently being
  /// initialized.
  virtual IOP::CodecFactory_ptr codec_factory (
      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())
    ACE_THROW_SPEC ((CORBA::SystemException));

  /// Register a mapping between a string and a corresponding object
  /// reference with the ORB being initialized.
  /**
   * This methid is particularly useful for registering references to
   * local (locality constrained) objects.  Note that this method
   * should be called in ORBInitializer::pre_init() so that the
   * registered reference will be available to the
   * resolve_initial_references() that may be called in the
   * ORBInitializer::post_init() call.
   */
  virtual void register_initial_reference (
      const char * id,
      CORBA::Object_ptr obj,
      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())
    ACE_THROW_SPEC ((CORBA::SystemException,
                     PortableInterceptor::ORBInitInfo::InvalidName));

  /// Obtain a reference to an object that may not yet be available
  /// via the usual CORBA::ORB::resolve_initial_references() mechanism
  /// since the ORB may not be fully initialized yet.
  virtual CORBA::Object_ptr resolve_initial_references (
      const char * id,
      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())
    ACE_THROW_SPEC ((CORBA::SystemException,
                     PortableInterceptor::ORBInitInfo::InvalidName));

  /// Register a client request interceptor with the ORB currently
  /// being initialized.
  virtual void add_client_request_interceptor (
      PortableInterceptor::ClientRequestInterceptor_ptr interceptor,
      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())
    ACE_THROW_SPEC ((CORBA::SystemException,
                     PortableInterceptor::ORBInitInfo::DuplicateName));

  /// Register a server request interceptor with the ORB currently
  /// being initialized.
  virtual void add_server_request_interceptor (
      PortableInterceptor::ServerRequestInterceptor_ptr interceptor,
      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())
    ACE_THROW_SPEC ((CORBA::SystemException,
                     PortableInterceptor::ORBInitInfo::DuplicateName));

  /// Register an IOR interceptor with the ORB currently being
  /// initialized.
  virtual void add_ior_interceptor (
      PortableInterceptor::IORInterceptor_ptr interceptor,
      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())
    ACE_THROW_SPEC ((CORBA::SystemException,
                     PortableInterceptor::ORBInitInfo::DuplicateName));

  /// Reserve a slot in table found within the
  /// PortableInterceptor::Current object.
  virtual PortableInterceptor::SlotId allocate_slot_id (
      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())
    ACE_THROW_SPEC ((CORBA::SystemException));

  /// Register a policy factory of the given policy type with the ORB
  /// currently being initialized.
  virtual void register_policy_factory (
      CORBA::PolicyType type,
      PortableInterceptor::PolicyFactory_ptr policy_factory,
      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())
    ACE_THROW_SPEC ((CORBA::SystemException));
  //@}

  /**
   * @name TAO Extensions
   *
   * These methods are not part of the PortableInterceptor
   * specification, and are TAO-specific extensions.
   */

  //@{
  /// Allocate a slot in the ORB's TSS resources.
  /**
   * TAO uses a single TSS key for these resources, so it is useful to
   * place TSS objects in TAO's TSS resources on platforms where the
   * number of TSS keys is low.  The returned SlotId can be used to
   * index into the array stored in ORB's TSS resources structure.
   * @par
   * An accompanying cleanup function (e.g. a TSS destructor) can also
   * be registered.
   */
  size_t allocate_tss_slot_id (
      ACE_CLEANUP_FUNC cleanup,
      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())
    ACE_THROW_SPEC ((CORBA::SystemException));

  /// Return a pointer to the ORB Core associated with the ORB being
  /// initialized.
  /**
   * The ORB Core is essentialy fully initialized by the time
   * ORBInitializer::post_init() is invoked.  As such, it is generally
   * best if this method is used in that method.
   *
   * @note Only use this method if you know what you are doing.
   */
  TAO_ORB_Core *orb_core (void) const;
  //@}

  /**
   * @name Reference Related Methods
   */
  //@{
#if !defined(__GNUC__) || !defined (ACE_HAS_GNUG_PRE_2_8)
  typedef TAO_ORBInitInfo_ptr _ptr_type;
  typedef TAO_ORBInitInfo_var _var_type;
#endif /* ! __GNUC__ || g++ >= 2.8 */

  static TAO_ORBInitInfo_ptr _duplicate (TAO_ORBInitInfo_ptr obj);

  static TAO_ORBInitInfo_ptr _narrow (
      CORBA::Object_ptr obj,
      CORBA::Environment &ACE_TRY_ENV =
        TAO_default_environment ()
    );

  static TAO_ORBInitInfo_ptr _unchecked_narrow (
      CORBA::Object_ptr obj,
      CORBA::Environment &ACE_TRY_ENV =
        TAO_default_environment ()
    );

  static TAO_ORBInitInfo_ptr _nil (void)
    {
      return (TAO_ORBInitInfo_ptr)0;
    }

  virtual void *_tao_QueryInterface (ptr_arith_t type);

  virtual const char* _interface_repository_id (void) const;
  //@}

protected:

  /// Destructor is protected to force instantiation on the heap since
  /// ORBInitInfo is reference counted.
  ~TAO_ORBInitInfo (void);

  /// Check if this ORBInitInfo instance is valid.  Once post_init()
  /// has been called on each of the ORBInitializers, this ORBInitInfo
  /// is no longer valid.  Throw an exception in that case.
  void check_validity (CORBA::Environment &ACE_TRY_ENV);

private:

  /// Prevent copying through the copy constructor and the assignment
  /// operator.
  ACE_UNIMPLEMENTED_FUNC (
    TAO_ORBInitInfo (const TAO_ORBInitInfo &))
  ACE_UNIMPLEMENTED_FUNC (void operator= (const TAO_ORBInitInfo &))

private:

  /// Reference to the ORB Core.
  TAO_ORB_Core *orb_core_;

  /// The number of arguments in the argument vector passed to
  /// CORBA::ORB_init().
  int argc_;

  /// The argument vector passed to CORBA::ORB_init().
  char **argv_;

  /// Reference to the CodecFactory returned by
  /// ORBInitInfo::codec_factory().
  IOP::CodecFactory_var codec_factory_;

};

/**
 * @class TAO_ORBInitInfo_var
 */
class TAO_Export TAO_ORBInitInfo_var : public TAO_Base_var
{
public:

  TAO_ORBInitInfo_var (void); // default constructor
  TAO_ORBInitInfo_var (TAO_ORBInitInfo_ptr p) : ptr_ (p) {}
  TAO_ORBInitInfo_var (const TAO_ORBInitInfo_var &); // copy constructor
  ~TAO_ORBInitInfo_var (void); // destructor

  TAO_ORBInitInfo_var &operator= (TAO_ORBInitInfo_ptr);
  TAO_ORBInitInfo_var &operator= (const TAO_ORBInitInfo_var &);
  TAO_ORBInitInfo_ptr operator-> (void) const;

  operator const TAO_ORBInitInfo_ptr &() const;
  operator TAO_ORBInitInfo_ptr &();
  // in, inout, out, _retn
  TAO_ORBInitInfo_ptr in (void) const;
  TAO_ORBInitInfo_ptr &inout (void);
  TAO_ORBInitInfo_ptr &out (void);
  TAO_ORBInitInfo_ptr _retn (void);
  TAO_ORBInitInfo_ptr ptr (void) const;

  // Hooks used by template sequence and object manager classes
  // for non-defined forward declared interfaces.
  static TAO_ORBInitInfo_ptr duplicate (TAO_ORBInitInfo_ptr);
  static void release (TAO_ORBInitInfo_ptr);
  static TAO_ORBInitInfo_ptr nil (void);
  static TAO_ORBInitInfo_ptr narrow (
      CORBA::Object *,
      CORBA::Environment &
    );
  static CORBA::Object * upcast (void *);

private:

  TAO_ORBInitInfo_ptr ptr_;
  // Unimplemented - prevents widening assignment.
  TAO_ORBInitInfo_var (const TAO_Base_var &rhs);
  TAO_ORBInitInfo_var &operator= (const TAO_Base_var &rhs);

};


#if defined (__ACE_INLINE__)
#include "ORBInitInfo.inl"
#endif  /* __ACE_INLINE__ */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma warning(pop)
#endif /* _MSC_VER */

#if defined (__BORLANDC__)
#pragma option pop
#endif /* __BORLANDC__ */

#include "ace/post.h"

#endif /* TAO_ORB_INIT_INFO_H */
