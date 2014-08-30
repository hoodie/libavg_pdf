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
//

#include <api.h>

#include <player/Player.h>
#include <player/RasterNode.h>
#include <player/TypeDefinition.h>

#include <graphics/MCTexture.h>


using namespace std;
using namespace boost::python;

namespace avg {

typedef int page_index_t ;

class AVG_API PageNode : public RasterNode
{
public:
      static void registerType();
                  PageNode();
                  PageNode(PopplerPage* page);
                  PageNode(const ArgList& args);
         virtual ~PageNode();
         
// PageNodes Own
    const string  getPopplerVersion() const;
       const int  getPageCount() const;
    const string  getText() const;
            void  open();
            void  setupContext();
            void  fill_bitmap(PopplerPage *page, double width, double height);
            void  rerender(PopplerPage* page, double width, double height);
            void  testFunction();
        IntPoint  getPageSize(page_index_t);
        IntPoint  getPageSize(PopplerPage* page);
            
// RasterNode overwrites            
    virtual void  renderFX();
    virtual void  render();
    virtual void  preRender(const VertexArrayPtr& pVA, bool bIsParentActive, float parentEffectiveOpacity);
    virtual void  connect(CanvasPtr pCanvas);
    virtual void  connectDisplay();
        IntPoint  getMediaSize();


private:
          avg::PixelFormat  m_pPixelFormat;
               PopplerPage *m_pPage;
              MCTexturePtr  m_pTex;
                 BitmapPtr  m_pBitmap;
                  IntPoint  m_pNodeSize;
                      bool  m_bNewSize;
                      bool  m_bNewBmp;
                       int  m_iPageCount;
};


}

