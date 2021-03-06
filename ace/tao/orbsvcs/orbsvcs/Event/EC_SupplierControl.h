/* -*- C++ -*- */
/**
 *  @file   EC_SupplierControl.h
 *
 *  EC_SupplierControl.h,v 1.6 2000/10/31 03:11:12 coryan Exp
 *
 *  @author Carlos O'Ryan (coryan@cs.wustl.edu)
 *
 * Based on previous work by Tim Harrison (harrison@cs.wustl.edu) and
 * other members of the DOC group. More details can be found in:
 *
 * http://doc.ece.uci.edu/~coryan/EC/index.html
 */

#ifndef TAO_EC_SUPPLIERCONTROL_H
#define TAO_EC_SUPPLIERCONTROL_H
#include "ace/pre.h"

#include "orbsvcs/Event/event_export.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "tao/corba.h"

class TAO_EC_Event_Channel;
class TAO_EC_ProxyPushConsumer;

/**
 * @class TAO_EC_SupplierControl
 *
 * @brief SupplierControl
 *
 * Defines the interface for the supplier control strategy.
 * This strategy handles misbehaving or failing suppliers.
 */
class TAO_RTEvent_Export TAO_EC_SupplierControl
{
public:
  /// Constructor.  It does not assume ownership of the <event_channel>
  /// parameter.
  TAO_EC_SupplierControl (void);

  /// destructor...
  virtual ~TAO_EC_SupplierControl (void);

  /// Activate any internal threads or timers used to poll the state of
  /// the suppliers
  virtual int activate (void);
  virtual int shutdown (void);

  /**
   * Invoked by helper classes when they detect that a supplier does
   * not exists (i.e. _non_existent() returns true and/or the
   * CORBA::OBJECT_NOT_EXIST exception has been raised).
   */
  virtual void supplier_not_exist (TAO_EC_ProxyPushConsumer *proxy,
                                   CORBA::Environment &);
};

#if defined (__ACE_INLINE__)
#include "EC_SupplierControl.i"
#endif /* __ACE_INLINE__ */

#include "ace/post.h"
#endif /* TAO_EC_SUPPLIERCONTROL_H */
