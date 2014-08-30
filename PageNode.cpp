#include <base/Logger.h>
#include <base/Exception.h>
#include <player/OGLSurface.h>
#include <graphics/GLContextManager.h>
#include <graphics/BitmapLoader.h>
#include <graphics/OGLHelper.h>
#include <wrapper/WrapHelper.h>
#include <wrapper/raw_constructor.hpp>

#include <string>
#include <iostream>
#include <vector>
#include <glib/poppler.h>
#include <cairo-pdf.h>

#include "PageNode.h"

using namespace std;
using namespace boost::python;
using namespace avg;

void PageNode::
registerType()
{
    TypeDefinition
    def = TypeDefinition("pagenode", "rasternode", ExportedObject::buildObject<PageNode>) ;
    //.addArg(  Arg<string>("fillcolor",   "0F0F0F",  false,  offsetof(ColorNode,  m_sFillColorName) ));

    //const char* allowedParentNodeNames[] = {"avg", 0};
    const char* allowedParentNodeNames[] = {"avg", "div",  0};
    //TypeRegistry::get()->registerType(def);
    TypeRegistry::get()->registerType(def, allowedParentNodeNames);
}

PageNode::  PageNode() { }
PageNode:: ~PageNode() { }

PageNode::
PageNode(PopplerPage* page)
    : m_pPixelFormat(avg::B8G8R8A8)
    , m_bNewSize(false)
    , m_bNewBmp(false)
{
 m_pPage = page; 
}
PageNode::
PageNode(const ArgList& args)
    : m_pPixelFormat(avg::B8G8R8A8)
    , m_bNewSize(false)
    , m_bNewBmp(false)
{
  AVG_TRACE( Logger::category::PLUGIN, Logger::severity::INFO,
             "PopplerNode c'tor gets Argument page= "  << args.getArgVal<PopplerPage*>("page") );
  AVG_TRACE( Logger::category::PLUGIN, Logger::severity::INFO,
             "PageNode constructed with " << m_pPage );
  args.setMembers(this);
}

const string
PageNode::
getPopplerVersion() const
{
  return poppler_get_version();
}

IntPoint
PageNode::
getMediaSize()
{
  return m_pNodeSize;
}

IntPoint
PageNode::
getPageSize(PopplerPage *page)
{
  // TODO store a currentPage
  double width, height;
  poppler_page_get_size(page, &width, &height);
  return IntPoint(width,height);
}

void PageNode
::fill_bitmap(PopplerPage *page, double width = 0, double height= 0)
{
  std::clog << "--- fill_bitmap()" << endl;
  cairo_surface_t *surface;
  cairo_t *cairo;
  
  IntPoint size = IntPoint(width, height);
  if (width == 0 or height==0)
    size = m_pNodeSize;
  else {
    m_pNodeSize = size;
    m_bNewSize  = true;
    std::clog << "setting new size: " << width << " ," << height << endl;
  }
    
  
  double xscale, yscale;
  xscale = size.x / (double)getPageSize(page).x;
  yscale = size.y / (double)getPageSize(page).y;
  
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size.x, size.y);
  cairo   = cairo_create(surface);
  
  cairo_scale(cairo, xscale,yscale);
  
  poppler_page_render( page, cairo );
  
  cairo_set_operator(cairo, CAIRO_OPERATOR_DEST_OVER);
  cairo_set_source_rgba(cairo, 1., 0., 0., 0.);
  cairo_paint(cairo);
  
  cairo_surface_flush(surface);
  unsigned char* data = cairo_image_surface_get_data(surface);
  int stride          = cairo_image_surface_get_stride(surface);
  m_pBitmap           = avg::BitmapPtr(
    new avg::Bitmap(size, m_pPixelFormat, data, stride, true)
  );
  m_bNewBmp           = true;
  
  cairo_destroy(cairo);
  cairo_surface_destroy(surface);
}

void PageNode
::rerender(PopplerPage *page, double width = 0, double height = 0)
{
  cout << "resizing to " << width << ", " << height;
  fill_bitmap(page, width, height);
}

void PageNode:: open()
{
    //cout << "+++ open()" << endl;
    
    setViewport(-32767, -32767, -32767, -32767);
    setupContext();
    fill_bitmap(m_pPage);
}

void PageNode::setupContext(){

    bool bMipmap = getMaterial().getUseMipmaps();
    m_pTex    = GLContextManager::get()->createTexture(m_pNodeSize, m_pPixelFormat, bMipmap);
    getSurface()->create(m_pPixelFormat,m_pTex);
    newSurface();
    setupFX();
    //cout << "++++ set bitmap" << endl;
}

void PageNode:: connect(CanvasPtr pCanvas)
{
  //cout << "... connect()" << endl;
  RasterNode::connect(pCanvas);
}

void PageNode::connectDisplay()
{
  //cout << "... connectDisplay()" << endl;
  RasterNode::connectDisplay();
  open();
}

void PageNode:: preRender(const VertexArrayPtr& pVA, bool bIsParentActive, float parentEffectiveOpacity)
{
  Node::preRender(pVA, bIsParentActive, parentEffectiveOpacity);
  //cout << "... preRender()" << endl;
  //PopplerPage* page = poppler_document_get_page(m_document, 0);
  if(isVisible()) {
    if(m_bNewSize) {
      setupContext();
      m_bNewSize = false;
    }
    if(m_bNewBmp) {
      //ScopeTime Timer(); # TODO time poppler rendering
      GLContextManager::get()->scheduleTexUpload(m_pTex, m_pBitmap);
      cout << "happily rendering" << endl;
      m_bNewBmp = false;
    }
  }
  calcVertexArray(pVA);
}

void PageNode:: renderFX() {
    RasterNode::renderFX(getSize(), Pixel32(255, 255, 255, 255), false);
}

void PageNode:: render() {
  //cout << "... render()" << endl;
  //ScopeTimer Timer(CameraProfilingZone);
  blt32(getTransform(), getSize(), getEffectiveOpacity(), getBlendMode());
}

void PageNode:: testFunction(){
  // implements all sorts of test stuff
  cout << "---testfunction--x" << endl;
  
  //cout << AreaNode::getSize().x <<  ", " << AreaNode::getSize().y << endl;
  rerender(0, AreaNode::getSize().x, AreaNode::getSize().y);
  cout << m_pNodeSize << endl;
  
  
  cout << "---testfunction---" << endl << endl;
}
