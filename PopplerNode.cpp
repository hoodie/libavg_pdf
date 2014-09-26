#include <base/Logger.h>
#include <base/Exception.h>
#include <player/OGLSurface.h>
#include <graphics/GLContextManager.h>
#include <graphics/BitmapLoader.h>
#include <graphics/OGLHelper.h>
#include <graphics/Pixel32.h>

#include <string>
#include <iostream>
#include <vector>
#include <glib/poppler.h>
#include <cairo-pdf.h>
#include <boost/python.hpp>

#include "PopplerNode.h"

using namespace std;
using namespace avg;

namespace py = boost::python;

typedef PopplerRectangle PopplerRect;

string plprfx = "   [ PopplerNode ] ";
const char* annottypenames[26] = {
  "UNKNOWN",    "TEXT",      "LINK",             "FREE_TEXT",  "LINE",
  "SQUARE",     "CIRCLE",    "POLYGON",          "POLY_LINE",  "HIGHLIGHT",
  "UNDERLINE",  "SQUIGGLY",  "STRIKE_OUT",       "STAMP",      "CARET",
  "INK",        "POPUP",     "FILE_ATTACHMENT",  "SOUND",      "MOVIE",
  "WIDGET",     "SCREEN",    "PRINTER_MARK",     "TRAP_NET",   "WATERMARK",  "3D"
};

void
PopplerNode::
registerType()
{
  TypeDefinition
    def = TypeDefinition("popplernode", "rasternode",
        ExportedObject::buildObject<PopplerNode>)
    .addArg( Arg<std::string>("path","",false,offsetof(PopplerNode,m_pRelPdfPath)) ) ;
    //.addArg( Arg<std::string>("render_annots","",false,offsetof(PopplerNode,m_bRenderAnnotations)) ) ;
  //.addArg(  Arg<string>("fillcolor",   "0F0F0F",  false,  offsetof(ColorNode,  m_sFillColorName) ));

    //const char* allowedParentNodeNames[] = {"avg", 0};
    const char* allowedParentNodeNames[] = {"avg", "div",  0};
    //TypeRegistry::get()->registerType(def);
    TypeRegistry::get()->registerType(def, allowedParentNodeNames);
}

PopplerNode::  PopplerNode() { }
PopplerNode:: ~PopplerNode() { }

//TODO add: opened correctly boolean to python api
PopplerNode::PopplerNode(const ArgList& args)
    : m_pPixelFormat(avg::B8G8R8A8)
    , m_pRelPdfPath("")
    , m_pPdfPath("")
    , m_bNewSize(false)
    , m_bNewBmp(false)
    , m_iPageCount(-1)
    , m_iCurrentPage(-1)
{
  AVG_TRACE( Logger::category::PLUGIN, Logger::severity::INFO, "PopplerNode c'tor gets Argument path= "  << args.getArgVal<string>("path") );
  AVG_TRACE( Logger::category::PLUGIN, Logger::severity::INFO, "PopplerNode constructed with " << m_pRelPdfPath );
  args.setMembers(this);
  
  char longer_path [PATH_MAX+1];
  char* path = realpath(m_pRelPdfPath.c_str(), longer_path);
  m_pPdfPath = std::string("file://").append(std::string(path));

  m_bRenderAnnotations = true;


  
  if(!this->loadDocument()) {
    cerr << "[fail] could not open document" << endl; // TODO load some placeholder in case of loadfailure
  }
    
}

void
PopplerNode::
setPath(std::string path)
{
  m_pPdfPath = path;
}


const string
PopplerNode::
getPath() const
{
  return m_pPdfPath;
}

void
PopplerNode::
setRenderAnnotations(bool render_annots) {
  m_bRenderAnnotations = render_annots;
}

const bool
PopplerNode::
getRenderAnnotations() const{
  return m_bRenderAnnotations;
}

const string
PopplerNode::
getPopplerVersion() const
{
  return poppler_get_version();
}

const int
PopplerNode::
getPageCount() const
{
  return m_iPageCount;
}

const int
PopplerNode::
getCurrentPage() const
{
  return m_iCurrentPage;
}

IntPoint
PopplerNode::
getPageSize(page_index_t index) const
{
  PopplerPage* page = m_vPages[index];
  double width, height;
  poppler_page_get_size(page, &width, &height);
  // TODO find alternative to IntPoint that takes doubles etc
  return IntPoint(width,height);
}

const string PopplerNode::getDocumentTitle()   const { return poppler_document_get_title(m_pDocument); }
const string PopplerNode::getDocumentAuthor()  const { return poppler_document_get_author(m_pDocument); }
const string PopplerNode::getDocumentSubject() const { return poppler_document_get_subject(m_pDocument); }

const
string
PopplerNode::
getPageText(page_index_t page_index) const
{
  return poppler_page_get_text(m_vPages[page_index]) ;
}

py::list
PopplerNode::
getPageTextLayout(page_index_t index) const
{
  
  PopplerPage* page = m_vPages[index];
  
  PopplerRectangle* rectangles; 
  guint n_rectangles; 
  poppler_page_get_text_layout(page, &rectangles, &n_rectangles);
  
  py::list plist;
  for (guint i =0 ; i< n_rectangles; ++i){
    Box box = boxFromPopplerRectangle(rectangles[i]);
    box.payload = poppler_page_get_selected_text( page, POPPLER_SELECTION_GLYPH, &rectangles[i] );
    box.height *= -1;
    plist.append<_Box>(box);
  }

  g_free(rectangles);
  
  return plist;
}

const _Box
PopplerNode::
boxFromPopplerRectangle(PopplerRectangle rect) const
{
  Box box;
  box.x = rect.x1;
  box.y = rect.y2;
  box.x2 = rect.x2;
  box.y2 = rect.y1;
  box.width  = abs(rect.x2 - rect.x1);
  box.height = abs(rect.y2 - rect.y1);
  return box;
}

const PopplerRectangle
PopplerNode::
popplerRectangleFromBox(Box box) const
{
  PopplerRectangle rect;
  rect.x1 = box.x;
  rect.y1 = box.y + box.height;
  rect.x2 = box.x + box.width;
  rect.y2 = box.y;
  return rect;
}


py::list
PopplerNode::
getPageAnnotations(page_index_t index) const
{
  
  PopplerPage* page = m_vPages[index];
  double width, height;
  poppler_page_get_size(page, &width, &height);

  GList* lptr;
  GList* mapping_list = poppler_page_get_annot_mapping(page);
  py::list plist;  

  for (lptr = mapping_list; lptr; lptr = lptr->next)
  {
    Annotation a;
    

    PopplerAnnotMapping* map = (PopplerAnnotMapping*)lptr->data;
    PopplerAnnot* pannot     = map->annot;
    PopplerColor* pcolor     = poppler_annot_get_color(pannot);
    PopplerRectangle rect    = map->area;
    
    PopplerAnnotType type = poppler_annot_get_annot_type(pannot);
    if( poppler_annot_get_annot_type(pannot) != POPPLER_ANNOT_HIGHLIGHT and
        poppler_annot_get_annot_type(pannot) != POPPLER_ANNOT_UNDERLINE) {
      cout << plprfx << "Annotation: " << annottypenames[type] << " is not supported" << endl;
      continue; // some annotations just want to watch the world burn
    }
    
    a.name     = poppler_annot_get_name(pannot);
    a.contents = poppler_annot_get_contents(pannot);
    a.modified = poppler_annot_get_modified(pannot);
    a.area     = rect;
    a.box      = boxFromPopplerRectangle(rect);
    a.box.y    = abs(height - a.box.y); // corrects rotation

    PopplerRect new_rect;
    new_rect.x1 = rect.x1;
    new_rect.x2 = rect.x2;
    new_rect.y1 = abs(height - rect.y1);
    new_rect.y2 = abs(height - rect.y2);

    a.box.payload = poppler_page_get_selected_text( page, POPPLER_SELECTION_GLYPH, &new_rect );
    a.color       = Pixel32(pcolor->red ,pcolor->green,pcolor->blue);
    
    plist.append( a );
    
  }
  
  return plist;
}

bool
PopplerNode::
loadDocument()
{
  //cout << plprfx << "--- loading document (" << m_pPdfPath << ")" << endl;
  GError *error = NULL;
  m_pDocument = poppler_document_new_from_file(m_pPdfPath.c_str(), NULL, &error);
  
  if(m_pDocument == NULL) {
    //cout << plprfx << "[fail] Problem loading " << m_pPdfPath << endl;
    //cout << plprfx << error->message << endl;
    cerr << "[fail] poppler did not open the document " << m_pPdfPath << endl;
    return false;
  }
  //cout << plprfx << "[ok] loaded document " << poppler_document_get_title(m_pDocument) << endl;
  m_iPageCount = poppler_document_get_n_pages(m_pDocument);
  
  if(m_iPageCount > 0){
    m_vPages        = std::vector<PopplerPage*> (m_iPageCount);
    m_vPageBitmaps  = std::vector<avg::BitmapPtr>(m_iPageCount);
    
    for(int i = 0; i< m_iPageCount; ++i){
      m_vPages.at(i) = (poppler_document_get_page(m_pDocument,i));
    }
    
    m_iCurrentPage = 0;
    m_pNodeSize = getPageSize(0);
    //cout << plprfx << "[ok] poppler opened " << m_pPdfPath << endl;
  }
  else {
    cerr << "document seems to have 0 pages";
    return false;
  }

  return true;
}

void
PopplerNode::
fill_bitmap(page_index_t page_index, double width = 0, double height= 0)
{
  
  PopplerPage* page = m_vPages[page_index];
  
  //std::clog << "--- fill_bitmap()" << endl;
  cairo_surface_t *surface;
  cairo_t *cairo;

  IntPoint size = IntPoint(width, height);
  if (width == 0 or height==0)
    size = m_pNodeSize;
  else {
    m_pNodeSize = size;
    m_bNewSize  = true;
    //std::clog << "setting new size: " << width << " ," << height << endl;
  }


  double pwidth, pheight;
  double xscale, yscale;
  poppler_page_get_size(page, &pwidth, &pheight);
  xscale = size.x / pwidth;
  yscale = size.y / pheight;

  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size.x, size.y);
  // TODO save a few microseconds using: cairo_image_surface_create_for_data()
  cairo   = cairo_create(surface);
  
  //clog << "xscale: " << xscale << endl;
  cairo_scale(cairo, xscale,yscale);

  //remove_all_annotations(page);

  if(m_bRenderAnnotations)
    poppler_page_render( page, cairo );
  else
    poppler_page_render_for_printing_with_options( page, cairo, POPPLER_PRINT_DOCUMENT );

  cairo_set_operator(cairo, CAIRO_OPERATOR_DEST_OVER);
  cairo_set_source_rgba(cairo, 1., 0., 0., 0.);
  cairo_paint(cairo);

  cairo_surface_flush(surface);
  unsigned char* data = cairo_image_surface_get_data(surface);
  int stride          = cairo_image_surface_get_stride(surface);
  m_pBitmap           = avg::BitmapPtr(
    new avg::Bitmap(size, m_pPixelFormat, data, stride, true)
  );
  
  m_vPageBitmaps.at(page_index) = m_pBitmap;
  
  m_bNewBmp           = true;

  cairo_destroy(cairo);
  cairo_surface_destroy(surface);
}

void
PopplerNode::
setCurrentPage(page_index_t page_index)
{
  if(page_index < 0 or page_index >= m_iPageCount){
    cerr << "page_index out of bound" << endl;
    return ;
  }
  m_iCurrentPage = page_index;
  if(m_vPageBitmaps.at(page_index) != NULL){
    m_pBitmap = m_vPageBitmaps.at(page_index);
    m_bNewBmp = true;
  }
  
  else
    resize(page_index, 0,0);
}

void
PopplerNode::
resize(page_index_t page_index, double width = 0, double height = 0)
{
  if(page_index < 0 or page_index >= m_iPageCount)
    return;
  //clog << "rerendering page: " << page_index << endl;
  
  if (width != 0 and height!=0)
  {
    //clog << "resetting bitmap cache ";
    m_vPageBitmaps.clear();
    m_vPageBitmaps = std::vector<avg::BitmapPtr>(m_iPageCount);
    m_bNewSize = true;
  }
  //clog << "resizing to " << width << ", " << height;
  fill_bitmap(page_index, width, height);
}

//void
//PopplerNode::
//remove_all_annotations(PopplerPage* page)
//{
//  GList* lptr;
//  GList* mapping_list = poppler_page_get_annot_mapping(page);
//  py::list plist;  
//
//  for (lptr = mapping_list; lptr; lptr = lptr->next)
//  {
//    PopplerAnnotMapping* map = (PopplerAnnotMapping*)lptr->data;
//    PopplerAnnot* pannot     = map->annot;
//    poppler_page_remove_annot(page, pannot);
//  }
//
//}

void
PopplerNode::
rerender()
{
  resize(getCurrentPage(), AreaNode::getSize().x, AreaNode::getSize().y);
}

void
PopplerNode::
setupContext()
{
  bool bMipmap = getMaterial().getUseMipmaps();
  m_pTex    = GLContextManager::get()->createTexture(m_pNodeSize, m_pPixelFormat, bMipmap);
  getSurface()->create(m_pPixelFormat,m_pTex);
  newSurface();
  setupFX();
}

void
PopplerNode::
connectDisplay()
{ 
  RasterNode::connectDisplay();
  setViewport(-32767, -32767, -32767, -32767);
  setupContext();
}

void
PopplerNode::
preRender(const VertexArrayPtr& pVA, bool bIsParentActive, float parentEffectiveOpacity)
{
  Node::preRender(pVA, bIsParentActive, parentEffectiveOpacity);
  if(isVisible()) {
    if(m_bNewSize) {
      setupContext();
      m_bNewSize = false;
    }
    if(m_bNewBmp) {
      //ScopeTime Timer(); # TODO time poppler rendering
      GLContextManager::get()->scheduleTexUpload(m_pTex, m_pBitmap);
      m_bNewBmp = false;
    }
  }
  calcVertexArray(pVA);
}

  // TODO remove render() and renderFX() as soonn as libavg permits
void PopplerNode:: render()   { blt32(); }
