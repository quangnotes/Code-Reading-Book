// -*- C++ -*-

//=============================================================================
/**
 *  @file LF_Event.h
 *
 *  LF_Event.h,v 1.3 2001/09/12 21:36:29 bala Exp
 *
 *  @author Carlos O'Ryan <coryan@uci.edu>
 */
//=============================================================================

#ifndef TAO_LF_EVENT_H
#define TAO_LF_EVENT_H
#include "ace/pre.h"

#include "corbafwd.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

class TAO_Transport;
class TAO_LF_Follower;
class TAO_Leader_Follower;
/**
 * @class TAO_LF_Event
 *
 * @brief Use the Leader/Follower loop to wait for one specific event.
 *
 * The Leader/Follower event loop is used to wait for incoming
 * responses, as well as to wait for all the data to be flushed.
 * This class encapsulates this event loop. It uses Template Method to
 * parametrize the 'waited for' predicate (i.e. reply received or
 * message sent.)
 *
 * @todo Implementing the Leader/Followers loop in this class, as
 * well as the callbacks to communicate that an event has completed
 * leads to excessive coupling.  A better design would use a separate
 * class to signal the events, that would allow us to remove the
 * Leader/Followers logic from the ORB.  However, that requires other
 * major changes and it somewhat complicates the design.
 *
 */
class TAO_Export TAO_LF_Event
{
public:

  friend class TAO_Leader_Follower;

  /// Constructor
  TAO_LF_Event (void);

  /// Destructor
  virtual ~TAO_LF_Event (void);

  /// Bind a follower
  /**
   * An event can be waited on by at most one follower thread, this
   * method is used to bind the waiting thread to the event, in order
   * to let the event signal any important state changes.
   *
   * @return -1 if the LF_Event is already bound, 0 otherwise
   */
  int bind (TAO_LF_Follower *follower);

  /// Unbind the follower
  int unbind (void);

  //@{
  /** @name State management
   *
   * A Leader/Followers event goes through several states during its
   * lifetime. We use an enum to represent those states and state
   * changes are validated according to the rules below.
   *
   */
  enum {
    /// The event is created, initial state can only move to
    /// LFS_ACTIVE
    LFS_IDLE,
    /// The event is active, can change to any of the following
    /// states, each of them is a final state
    LFS_ACTIVE,
    /// The event has completed successfully
    LFS_SUCCESS,
    /// A failure has been detected while the event was active
    LFS_FAILURE,
    /// The event has timed out
    LFS_TIMEOUT,
    /// The connection was closed while the state was active
    LFS_CONNECTION_CLOSED
  };

  /// Change the state
  void state_changed (int new_state);

  /// Return 1 if the condition was satisfied successfully, 0 if it
  /// has not
  int successful (void) const;

  /// Return 1 if an error was detected while waiting for the
  /// event
  int error_detected (void) const;
  //@}

  /// Check if we should keep waiting.
  int keep_waiting (void);

protected:
  /// Validate the state change
  void state_changed_i (int new_state);

private:

  /// Check whether we have reached the final state..
  int is_state_final (void);

  /// Set the state.
  void set_state (int new_state);

private:
  /// The current state
  int state_;

  /// The bound follower thread
  TAO_LF_Follower *follower_;
};

#if defined (__ACE_INLINE__)
# include "LF_Event.inl"
#endif /* __ACE_INLINE__ */

#include "ace/post.h"
#endif  /* TAO_LF_EVENT_H */
