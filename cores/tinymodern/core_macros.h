/*==============================================================================

  core_macros.h - Simple but handy macros.

  Copyright 2010 Rowdy Dog Software.

  This file is part of Arduino-Tiny.

  Arduino-Tiny is free software: you can redistribute it and/or modify it 
  under the terms of the GNU Lesser General Public License as published by 
  the Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  Arduino-Tiny is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License 
  along with Arduino-Tiny.  If not, see <http://www.gnu.org/licenses/>.

==============================================================================*/

#ifndef core_macros_h
#define core_macros_h


/*=============================================================================
  Bitmask macros
=============================================================================*/

#define MASK1(b1)                         ( (1<<b1) )
#define MASK2(b1,b2)                      ( (1<<b1) | (1<<b2) )
#define MASK3(b1,b2,b3)                   ( (1<<b1) | (1<<b2) | (1<<b3) )
#define MASK4(b1,b2,b3,b4)                ( (1<<b1) | (1<<b2) | (1<<b3) | (1<<b4) )
#define MASK5(b1,b2,b3,b4,b5)             ( (1<<b1) | (1<<b2) | (1<<b3) | (1<<b4) | (1<<b5) )
#define MASK6(b1,b2,b3,b4,b5,b6)          ( (1<<b1) | (1<<b2) | (1<<b3) | (1<<b4) | (1<<b5) | (1<<b6) )
#define MASK7(b1,b2,b3,b4,b5,b6,b7)       ( (1<<b1) | (1<<b2) | (1<<b3) | (1<<b4) | (1<<b5) | (1<<b6) | (1<<b7) )
#define MASK8(b1,b2,b3,b4,b5,b6,b7,b8)    ( (1<<b1) | (1<<b2) | (1<<b3) | (1<<b4) | (1<<b5) | (1<<b6) | (1<<b7) | (1<<b8) )


#endif
