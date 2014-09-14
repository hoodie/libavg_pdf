//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2014 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//
//  Original author of this file is Hendrik Sollich <hendrik@hoodie.de>

#ifndef __WRAPPER_H__ 
#define __WRAPPER_H__ 
#include <string>

namespace avg {

typedef int page_index_t ;

  typedef struct _Color{
    int red;
    int green;
    int blue;
  } Color;

  typedef struct _Box{
    double x, y, x2, y2;
    double width, height;
    std::string payload;
  } Box;
  
  typedef struct _Annotation{
    PopplerRectangle area;
        std::string  name,contents,label;
        std::string  modified;
             _Color  color;
               _Box  box;
  //       enum  type; // TODO implement annot_type enum
  } Annotation;   

}
#endif /* __WRAPPER_H__ */
