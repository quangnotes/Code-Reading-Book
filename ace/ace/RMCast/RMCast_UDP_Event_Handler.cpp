//
// RMCast_UDP_Event_Handler.cpp,v 1.3 2000/10/10 00:06:30 coryan Exp
//

#include "RMCast_UDP_Event_Handler.h"
#include "RMCast_IO_UDP.h"

#if !defined (__ACE_INLINE__)
# include "RMCast_UDP_Event_Handler.i"
#endif /* ! __ACE_INLINE__ */

ACE_RCSID(ace, RMCast_UDP_Event_Handler, "RMCast_UDP_Event_Handler.cpp,v 1.3 2000/10/10 00:06:30 coryan Exp")

ACE_RMCast_UDP_Event_Handler::~ACE_RMCast_UDP_Event_Handler (void)
{
}

ACE_HANDLE
ACE_RMCast_UDP_Event_Handler::get_handle (void) const
{
  return this->io_udp_->get_handle ();
}

int
ACE_RMCast_UDP_Event_Handler::handle_input (ACE_HANDLE h)
{
  return this->io_udp_->handle_input (h);
}

int
ACE_RMCast_UDP_Event_Handler::handle_timeout (const ACE_Time_Value &,
                                              const void *)
{
  // @@ return this->io_udp_->handle_timeout ();
  return 0;
}
