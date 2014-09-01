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
#include <boost/python.hpp>

#include "PageNode.h"

namespace py = boost::python;

namespace avg {

class AVG_API PopplerNode : public PageNode
{
public:
      static void registerType();
                  PopplerNode();
                  PopplerNode(const ArgList& args);
         virtual ~PopplerNode();
         
// PopplerNodes Own
            void  setPath(std::string path);
    const string  getPath() const;
    const string  getPopplerVersion() const;
       const int  getPageCount() const;
    const string  getDocumentTitle() const;
    const string  getDocumentAuthor() const;
    const string  getDocumentSubject() const;
    const string  getPageText() const;
 //RectVectorPtr  getPageTextLayout(page_index_t) const;
        py::list  getPageTextLayout(page_index_t) const;
        py::list  getPageAnnotations(page_index_t) const;
            bool  loadDocument();
            void  setCurrentPage(page_index_t);
            void  open();
            void  setupContext();
            void  fill_bitmap(PopplerPage*, double width, double height);
            void  rerender(page_index_t   );
            void  resize(page_index_t   , double width, double height);
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
              MCTexturePtr  m_pTex;
               std::string  m_pPdfPath;
                 BitmapPtr  m_pBitmap;
                  IntPoint  m_pNodeSize;
                      bool  m_bNewSize;
                      bool  m_bNewBmp;
           PopplerDocument* m_pDocument;
                       int  m_iPageCount;
              page_index_t  m_iCurrentPage;
 std::vector<PopplerPage*>  m_vPages;
    std::vector<BitmapPtr>  m_vPageBitmaps;
};


}

