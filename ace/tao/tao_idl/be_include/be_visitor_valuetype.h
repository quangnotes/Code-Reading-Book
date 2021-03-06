
// be_visitor_valuetype.h,v 1.4 2000/05/09 18:23:59 parsons Exp
//
/* -*- c++ -*- */
// ============================================================================
//
// = LIBRARY
//    TAO IDL
//
// = FILENAME
//    be_visitor_valuetype.h
//
// = DESCRIPTION
//    Concrete visitor for the Valuetype class
//
// ============================================================================

#ifndef TAO_BE_VISITOR_VALUETYPE_H
#define TAO_BE_VISITOR_VALUETYPE_H

#ifdef IDL_HAS_VALUETYPE

#include "be_visitor_scope.h"

#include "be_visitor_valuetype/valuetype.h"
#include "be_visitor_valuetype/valuetype_ch.h"
#include "be_visitor_valuetype/valuetype_obv_ch.h"
#include "be_visitor_valuetype/valuetype_obv_ci.h"
#include "be_visitor_valuetype/valuetype_obv_cs.h"
#include "be_visitor_valuetype/valuetype_ci.h"
#include "be_visitor_valuetype/valuetype_cs.h"
#include "be_visitor_valuetype/cdr_op_ch.h"
#include "be_visitor_valuetype/cdr_op_ci.h"
#include "be_visitor_valuetype/marshal_ch.h"
#include "be_visitor_valuetype/marshal_cs.h"
#include "be_visitor_valuetype/arglist.h"
#include "be_visitor_valuetype/field_ch.h"
#include "be_visitor_valuetype/field_ci.h"
#include "be_visitor_valuetype/field_cs.h"
#include "be_visitor_valuetype/field_cdr_ch.h"
#include "be_visitor_valuetype/field_cdr_ci.h"
#include "be_visitor_valuetype/obv_module.h"
#include "be_visitor_valuetype/ami_exception_holder_ch.h"
#include "be_visitor_valuetype/ami_exception_holder_cs.h"

#endif /* IDL_HAS_VALUETYPE */

#endif /* TAO_BE_VISITOR_VALUETYPE_H */
