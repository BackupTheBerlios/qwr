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
   
/* history: this software has been written originally by Joern Seger in 2001 and 
   has been used and modified in several projects since then */

#include <sstream>
#include "timev.h"

timev::timev(const unsigned int i)
{
  /* infinity-test */
  if (~i) {
    tv_sec = int(i/1000);
    tv_usec = (i%1000)*1000;
    _infinite = false;
  }
  else {
    _infinite = true;
    tv_sec = ~0;
    tv_usec = ~0;
  }
}

timev& timev::set2now()
{
  timev& time = *this;
#ifndef __ECOS
  gettimeofday(&time, 0);
#else
  timespec tmp;
  clock_gettime(0, &tmp);
  tv_sec = tmp.tv_sec;
  tv_usec = (unsigned int) (tmp.tv_nsec/1000);
#endif

  /* there is an effect, that the time is not really over, because of
     rounding errors */
  time.tv_usec = (int(time.tv_usec/1000)*1000);
  if (_infinite)
    _infinite = false;
  return(time);
}

bool timev::expired()
{
  timev now;
  now.set2now();
  return (operator<=(*this,now));
}

timev timev::expireTime()
{
  timev now;
  now.set2now();
  return((*this) - now);
}

bool timev::set()
{
  if ((tv_sec == 0) &&
      (tv_usec == 0))
    return false;
  return true;
}

void timev::set_infinite()
{
  _infinite = true;
  tv_sec = ~0;
  tv_usec = ~0;
}

void timev::clear()
{
  _infinite = false;
  tv_sec = 0;
  tv_usec = 0;
}

timev& timev::operator= (const timev& n)
{
  tv_sec = n.tv_sec;  
  /* if we have a real systemtime value, we ignore the last four
     digits */
  tv_usec = (int(n.tv_usec/1000)*1000);

  _infinite = n.infinite();

  return(*this);
}  

timev& timev::operator+= (const timev& n)
{
  if (infinite() || n.infinite()){
    set_infinite();
    return (*this);
  }

  tv_usec += n.tv_usec;
  tv_sec += int (tv_usec/MAXUSECPERSEC);
  tv_usec = tv_usec%MAXUSECPERSEC;
  tv_sec += n.tv_sec;
  return (*this);
}

timev& timev::operator-= (const timev& n)
{
  if (infinite())
    return (*this);

  if (n.infinite()){
    tv_sec = tv_usec = 0;
    return (*this);
  }
    
  tv_usec -= n.tv_usec;
  if(tv_usec < 0) {
    tv_usec = MAXUSECPERSEC+tv_usec;
    tv_sec--;
  }
  tv_sec -= n.tv_sec;
  if (tv_sec < 0) {
    tv_sec = 0;
    tv_usec = 0;
  }

  return (*this);
}

timev& timev::operator/= (const int n)
{
  int uebertrag = tv_sec%n;
  tv_sec /= n;
  
  /* I can't explain this in short, please think about it yourself */
  tv_usec += (uebertrag*MAXUSECPERSEC)/10;
  tv_usec /= n;

  return (*this);
}

bool timev::operator< (const timev& b )
{
  if (infinite())
    return false;

  if (b.infinite())
    return true;

  if (tv_sec < b.tv_sec)
    return (true);
  
  if ((tv_sec == b.tv_sec) && (tv_usec < b.tv_usec))
    return (true);
  
  return (false);
}

bool timev::operator> (const timev& b )
{
  if (b.infinite())
    return false;

  if (infinite())
    return true;
  
  if (tv_sec > b.tv_sec)
    return (true);

  if ((tv_sec == b.tv_sec) && (tv_usec > b.tv_usec))
    return (true);

  return (false);
}

/* value in mysec */
bool timev::operator==(const int value)
{
  if (infinite())
    return false;

  int s_value = int(value/1000);
  int my_value = (value%1000);
  if ((tv_sec == s_value) && (tv_usec == my_value*1000))
    return (true);
  return (false);
}

bool timev::operator==(const timev& n)
{
  if ((tv_sec == n.tv_sec) && (tv_usec == n.tv_usec))
    return (true);
  return (false);
 
}


std::string timev::toString() 
{
  std::stringstream stream;
  if (infinite()) {
    stream << "INFINITE"; 
    return (stream.str()); 
  }
  timev tm = *this;  
  if (tv_sec>360000) {
    timev now;
    now.set2now();
    tm -= now;
    stream << "period from now: ";
  }

  stream<<int(tm.tv_sec/60)<<" min : "<<(tm.tv_sec%60)<<" sec : ";
  stream<<tm.tv_usec/1000<<" msec ";

  return (stream.str());

}


/****************************************************/

timev operator+ (timev a, timev b)
{
  timev r = a;
  return (r+=b);
}

timev operator- (timev a, timev b)
{
  timev r = a;
  return (r-=b);
}

timev operator/ (timev a, int b)
{
  timev r = a;
  return (r/=b);
}

bool operator< (const timev& a, const timev& b )
{
  timev r = a;
  return (r<b);
}

bool operator> (const timev& a, const timev& b )
{
  timev r = a;
  return (r>b);
}

bool operator== (const timev& a, const timev& b )
{
  timev r = a;
  return(r==b);
}

bool operator>= (const timev& a, const timev& b )
{
  timev r = a;
  if (r>b)
    return (true);
  if (r==b)
    return (true);
  return (true);
}

bool operator<= (const timev& a, const timev& b )
{
  timev r = a;
  if (r<b)
    return (true);
  if (r==b)
    return (true);
  return (false);
}
