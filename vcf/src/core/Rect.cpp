/**
*Copyright (c) 2000-2001, Jim Crafton
*All rights reserved.
*Redistribution and use in source and binary forms, with or without
*modification, are permitted provided that the following conditions
*are met:
*	Redistributions of source code must retain the above copyright
*	notice, this list of conditions and the following disclaimer.
*
*	Redistributions in binary form must reproduce the above copyright
*	notice, this list of conditions and the following disclaimer in 
*	the documentation and/or other materials provided with the distribution.
*
*THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
*AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS
*OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
*PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
*LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
*NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
*SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*NB: This software will not save the world.
*/

/* Generated by Together */
#include "FoundationKit.h"
#include <math.h>

using namespace VCF;


VCF::Rect::Rect( const double & left, const double & top, const double & right, const double & bottom )
{	
	this->setRect( left, top, right, bottom );
}

VCF::Rect::Rect()
{	
	this->setRect( 0.0, 0.0, 0.0, 0.0 );
}

VCF::Rect::Rect( const Rect& rect )
{	
	this->setRect( rect.m_left, rect.m_top, rect.m_right, rect.m_bottom );
}

void VCF::Rect::inflate( const double & x, const double & y )
{
	this->m_left += (-x);
	this->m_right += x;

	this->m_top += (-y);
	this->m_bottom += y;
}

bool VCF::Rect::containsRect( Rect* rect )
{
	return false;
}

bool VCF::Rect::containsPt( Point * point )
{
	bool result = false;
	if ( (point->m_x >= this->m_left) &&  (point->m_y >= this->m_top) &&
	     (point->m_x < this->m_right) &&  (point->m_y < this->m_bottom) ){
		result = true;
	}
	return result;
}

void VCF::Rect::normalize()
{

}

double VCF::Rect::getHeight()
{
	return fabs( this->m_bottom - this->m_top );
}

double VCF::Rect::getWidth()
{
	return fabs( this->m_right - this->m_left );
}

bool VCF::Rect::operator == ( const Rect& rectToCompare )const 
{
	bool result = false;
	
	result = (this->m_bottom == rectToCompare.m_bottom) &&
			     (this->m_right == rectToCompare.m_right) &&
				 (this->m_top == rectToCompare.m_top) &&
				 (this->m_left == rectToCompare.m_left);	
	
	return result;
}

void VCF::Rect::setRect( const double & left, const double & top, const double & right, const double & bottom )
{
	this->m_left = left;
	this->m_right = right;
	this->m_top = top;
	this->m_bottom = bottom;
}

String VCF::Rect::toString()
{
	String result = "";

	char tmp[256];
	memset(tmp, 0, 256 );
	sprintf( tmp, "%.3f,%.3f,%.3f,%.3f", m_left, m_top, m_right, m_bottom );
	
	result = tmp;

	return result;
}

void VCF::Rect::saveToStream( OutputStream * stream )
{
	stream->write( m_left );
	stream->write( m_top );
	stream->write( m_right );
	stream->write( m_bottom );
}

void VCF::Rect::loadFromStream( InputStream * stream )
{
	stream->read( m_left );
	stream->read( m_top );
	stream->read( m_right );
	stream->read( m_bottom );
}