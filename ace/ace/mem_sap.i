/* -*- C++ -*- */
// MEM_SAP.i,v 4.5 2001/04/03 05:43:08 nanbor Exp

// MEM_SAP.i


ASYS_INLINE
ACE_MEM_SAP_Node::ACE_MEM_SAP_Node (size_t cap)
  : capacity_ (cap),
    size_ (0),
    next_ (0)
{
}

ASYS_INLINE size_t
ACE_MEM_SAP_Node::size (void) const
{
  return this->size_;
}

ASYS_INLINE size_t
ACE_MEM_SAP_Node::capacity (void) const
{
  return this->capacity_;
}

ASYS_INLINE void *
ACE_MEM_SAP_Node::data (void)
{
  return  this + 1;
}

ASYS_INLINE
ACE_MEM_SAP::~ACE_MEM_SAP (void)
{
  // ACE_TRACE ("ACE_MEM_SAP::~ACE_MEM_SAP");
  delete this->shm_malloc_;
}


ASYS_INLINE ACE_MEM_SAP_Node *
ACE_MEM_SAP::acquire_buffer (const ssize_t size)
{
  ACE_TRACE ("ACE_MEM_SAP::acquire_buffer");
  if (this->shm_malloc_ == 0)
    return 0;                  // not initialized.

  ACE_MEM_SAP_Node *buf =
    ACE_reinterpret_cast (ACE_MEM_SAP_Node *,
                          this->shm_malloc_->malloc (sizeof (ACE_MEM_SAP_Node)
                                                     + size));
  if (buf != 0)
    return new (buf) ACE_MEM_SAP_Node (size);

  return 0;
}

ASYS_INLINE int
ACE_MEM_SAP::release_buffer (ACE_MEM_SAP_Node *buf)
{
  ACE_TRACE ("ACE_MEM_SAP::release_buffer");
  if (this->shm_malloc_ == 0)
    return -1;                  // not initialized.

  this->shm_malloc_->free (buf);
  return 0;
}
