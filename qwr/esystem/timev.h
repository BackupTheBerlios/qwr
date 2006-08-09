/*
 * time class derived from timeval  
 * Copyright (c) 2005-2006 Joern Seger
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
      
/* History:
   2001 created JS
   2005 reused by the esystem JS
*/
   
#ifndef timev_h
#define timev_h

#include <sys/time.h>
#include <string>

#define MAXSECPERDAY 86400
#define MAXUSECPERSEC 1000000

/* more easy to handle */
//#warning FIXME: timev -> infinit not embeded into operators 

//! Timev Class was designed for easy access to system time
/*! to add, sub, compare, set, reset etc time, many operators are
  opened upon the well known system timeval struct, with this version,
  also infinit time could be handled correct (hopefully)*/
class timev : public timeval {
 private:
  bool _infinite;
 public:
  timev():_infinite(false) {};
  timev& set2now(); //!< sets the actual time
  timev(const unsigned int i); //!< sets time in mSec
  timev& operator=(const timev& n); //!< copies timev into another object
  timev& operator+=(const timev& n); //!< adds time
  timev& operator-=(const timev& n); //!< subs time
  timev& operator/=(const int n); //!< divs time
  bool expired(); //!< test if the time is less NOW 
  timev expireTime(); //!< returns the time from now to saved time
  bool set(); //!< reset time to NULL
  void set_infinite(); //!< sets time to infinite
  bool infinite() const { return (_infinite); } //!< test if time is infinite
  void clear();
  bool operator<(const timev& n); //!< test if less
  bool operator>(const timev& n); //!< test if greater
  bool operator==(const int value); //!< test if eq value (mSec)
  bool operator==(const timev& n); //!< test of equality

  std::string toString();

}; 

timev operator+ (timev a, timev b);
timev operator- (timev a, timev b);
timev operator/ (timev a, int b);
bool operator< (const timev& a, const timev& b);
bool operator> (const timev& a, const timev& b);
bool operator<= (const timev& a, const timev& b);
bool operator>= (const timev& a, const timev& b);


#endif //timev_h
