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
#include <glib/poppler.h>

#include "wrapper.h"

namespace py = boost::python;

namespace avg {
  
typedef int page_index_t;

class AVG_API PopplerNode : public RasterNode
{
public:
      static void registerType();
                  PopplerNode();
                  PopplerNode(const ArgList& args);
         virtual ~PopplerNode();
         
// PopplerNodes Own
                      void  setPath(avg::UTF8String);
              const string  getPath()             const;
                      void  setRenderAnnotations(bool);
                const bool  getRenderAnnotations()const;

              const string  getPopplerVersion()   const;
                 const int  getPageCount()        const;
        const page_index_t  getCurrentPage()      const;

              const string  getDocumentTitle()        const ;
              const string  getDocumentAuthor()       const ;
              const string  getDocumentSubject()      const ;
              const string  getPageText(page_index_t) const ;

                  IntPoint  getPageSize(page_index_t)        const;
                  py::list  getPageTextLayout(page_index_t)  const;
                  py::list  getPageAnnotations(page_index_t) const;
                  py::list  getPageImageFrames(page_index_t) const;
                  py::list  getPageImages(page_index_t)      const;

                 BitmapPtr  getPageImage(page_index_t page_index, unsigned int image_id) const;
           cairo_surface_t* renderPageSurface(page_index_t, double, double, bool) const;
                 BitmapPtr  renderPageBitmap(page_index_t, double, double, bool) const;
                 BitmapPtr  renderPageBitmap2(page_index_t) const;
                   // void  renderPageRegionBitmap(page_index_t, _Box*, double, double, bool) const;
              // BitmapPtr  renderPageRegionBitmap(page_index_t, _Box*, double, double, bool) const;

// helpers and converters
                const _Box  boxFromPopplerRectangle(PopplerRectangle) const;
    const PopplerRectangle  popplerRectangleFromBox(Box) const;

                      bool  loadDocument();
                      void  setCurrentPage(page_index_t);
                 BitmapPtr  surface_to_bitmap(cairo_surface_t*) const;
                      void  fill_main_bitmap();
                      void  resize(double width, double height);
                      void  rerender();
                    //void  remove_all_annotations(PopplerPage*);

// RasterNode setup
                      void  connectDisplay();
                      void  preRender(const VertexArrayPtr& pVA, bool bIsParentActive, float parentEffectiveOpacity);
                      void  setupContext();
                      void  render();

private:
          avg::PixelFormat  m_pPixelFormat;
           PopplerDocument* m_pDocument;
              MCTexturePtr  m_pTex;
           avg::UTF8String  m_pRelPdfPath;
           avg::UTF8String  m_pPdfPath;
                 BitmapPtr  m_pBitmap;
                  IntPoint  m_pNodeSize;
                      bool  m_bRenderAnnotations;
                      bool  m_bNewSize;
                      bool  m_bNewBmp;
                       int  m_iPageCount;
              page_index_t  m_iCurrentPage;
    std::vector<BitmapPtr>  m_vPageBitmaps;
 std::vector<PopplerPage*>  m_vPages;
};


}

