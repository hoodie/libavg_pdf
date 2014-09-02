#include <base/Logger.h>
#include <base/Exception.h>
#include <player/OGLSurface.h>
#include <graphics/GLContextManager.h>
#include <graphics/BitmapLoader.h>
#include <graphics/OGLHelper.h>

#include <string>
#include <iostream>
#include <vector>
#include <glib/poppler.h>
#include <cairo-pdf.h>
#include <boost/python.hpp>

#include "PopplerNode.h"

#include "wrapper.h"

using namespace std;
using namespace avg;

namespace py = boost::python;

void
PopplerNode::
registerType()
{
    TypeDefinition
    def = TypeDefinition("popplernode", "rasternode",
                         ExportedObject::buildObject<PopplerNode>)
          .addArg( Arg<std::string>("path","",false,offsetof(PopplerNode,m_pPdfPath)) ) ;
    //.addArg(  Arg<string>("fillcolor",   "0F0F0F",  false,  offsetof(ColorNode,  m_sFillColorName) ));

    //const char* allowedParentNodeNames[] = {"avg", 0};
    const char* allowedParentNodeNames[] = {"avg", "div",  0};
    //TypeRegistry::get()->registerType(def);
    TypeRegistry::get()->registerType(def, allowedParentNodeNames);
}

PopplerNode:: PopplerNode() { }
PopplerNode:: ~PopplerNode() { }

PopplerNode::PopplerNode(const ArgList& args)
    : m_pPixelFormat(avg::B8G8R8A8)
    , m_pPdfPath("")
    , m_bNewSize(false)
    , m_bNewBmp(false)
    , m_iPageCount(-1)
    , m_iCurrentPage(-1)
{
  AVG_TRACE( Logger::category::PLUGIN, Logger::severity::INFO, "PopplerNode c'tor gets Argument path= "  << args.getArgVal<string>("path") );
  AVG_TRACE( Logger::category::PLUGIN, Logger::severity::INFO, "PopplerNode constructed with " << m_pPdfPath );
  args.setMembers(this);
  
  
  if(!this->loadDocument()) {
    cout << "[fail] could not open document" << endl; // TODO load some placeholder in case of loadfailure
  }
    
}

void
PopplerNode::
setPath(std::string path)
{
  std::cout << "setting path to \"" << path << "\"" << std::endl;
  m_pPdfPath = path;
}


const string
PopplerNode::
getPath() const
{
    return m_pPdfPath;
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
getMediaSize()
{
  return m_pNodeSize;
}

IntPoint
PopplerNode::
getPageSize(PopplerPage* page)
{
  double width, height;
  poppler_page_get_size(page, &width, &height);
  return IntPoint(width,height);
}

IntPoint
PopplerNode::
getPageSize(page_index_t index)
{
  return getPageSize(m_vPages[index]);
}

const string PopplerNode::getDocumentTitle()   const { return poppler_document_get_title(m_pDocument); }
const string PopplerNode::getDocumentAuthor()  const { return poppler_document_get_author(m_pDocument); }
const string PopplerNode::getDocumentSubject() const { return poppler_document_get_subject(m_pDocument); }
const string PopplerNode::getPageText()        const { return poppler_page_get_text(m_vPages[m_iCurrentPage]) ;}

py::list
PopplerNode::
getPageTextLayout(page_index_t index) const
{
  
  PopplerPage* page = m_vPages[index];
  
  PopplerRectangle* rectangles; 
  guint n_rectangles; 
  poppler_page_get_text_layout(page, &rectangles, &n_rectangles);
  
  boost::shared_ptr<PopplerRectangle[]>rects(rectangles);
  
  py::list plist;
  for (guint i =0 ; i< n_rectangles; ++i)
    plist.append<PopplerRectangle>(rects[i]);
  
  return plist;
}

py::list
PopplerNode::
getPageAnnotations(page_index_t index) const
{
  
  PopplerPage* page = m_vPages[index];
  GList* lptr;
  GList* glist = poppler_page_get_annot_mapping(page);
  py::list plist; 
  for (lptr = glist; lptr != NULL; lptr = lptr->next)
  {
    
    Annotation a;
    
    PopplerAnnot* pannot = (PopplerAnnot*)(((PopplerAnnotMapping*)lptr->data)->annot);
    PopplerColor* pcolor = poppler_annot_get_color(pannot);
    
    //a.name =  annot->getName()->getCString();
    a.name     = poppler_annot_get_name(pannot);
    a.contents = poppler_annot_get_contents(pannot);
    a.modified = poppler_annot_get_modified(pannot);
    a.area     = ((PopplerAnnotMapping*)lptr->data)->area;
    a.color.red   = pcolor->red;
    a.color.green = pcolor->green;
    a.color.blue  = pcolor->blue;
    
    plist.append( a );
    
  }
    
  return plist;
}

bool
PopplerNode::
loadDocument()
{
  //cout << "--- loading document (" << m_pPdfPath << ")" << endl;
  GError *error = NULL;
  m_pDocument = poppler_document_new_from_file(m_pPdfPath.c_str(), NULL, &error);
  
  if(m_pDocument == NULL) {
    //cout << "[fail] Problem loading " << m_pPdfPath << endl;
    //cout << error->message << endl;
    return false;
  }
  //cout << "[ok] loaded document " << poppler_document_get_title(m_pDocument) << endl;
  m_iPageCount = poppler_document_get_n_pages(m_pDocument);
  
  if(m_iPageCount > 0){
    m_vPages = std::vector<PopplerPage*> (m_iPageCount);
    m_vPageBitmaps = std::vector<avg::BitmapPtr>(m_iPageCount);
    
    for(int i = 0; i< m_iPageCount; ++i){
      m_vPages.at(i) = (poppler_document_get_page(m_pDocument,i));
    }
    setCurrentPage(0);
    
  }
  else
    cout << "document seems to have 0 pages";
  return false;
  
  return true;
}

void
PopplerNode::
setCurrentPage(page_index_t page_index)
{
  m_iCurrentPage = page_index;
  m_pNodeSize = getPageSize(page_index);
}

void
PopplerNode::
fill_bitmap(page_index_t page_index, double width = 0, double height= 0)
{
  
  PopplerPage* page = m_vPages[page_index];
  
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

  std::clog << "pagesize to:  " << getPageSize(m_iCurrentPage).x << " ," << getPageSize(m_iCurrentPage).y << endl;
  std::clog << "scaling to:  " << xscale << " ," << yscale << endl;

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
  
  cout << "getting till here" << endl;
  m_vPageBitmaps.at(page_index) = m_pBitmap;
  
  m_bNewBmp           = true;

  cairo_destroy(cairo);
  cairo_surface_destroy(surface);
}

void
PopplerNode::
rerender(page_index_t page_index)
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
  clog << "rerendering page: " << page_index << endl;
  
  if (width != 0 and height!=0)
  {
    clog << "resetting bitmap cache ";
    m_vPageBitmaps.clear();
    m_vPageBitmaps = std::vector<avg::BitmapPtr>(m_iPageCount);
    m_bNewSize = true;
  }
  clog << "resizing to " << width << ", " << height;
  fill_bitmap(page_index, width, height);
}

void
PopplerNode::
adaptSize(){
  resize(getCurrentPage(), AreaNode::getSize().x, AreaNode::getSize().y);
}

void
PopplerNode::
open()
{
  setViewport(-32767, -32767, -32767, -32767);
  setupContext();
}

void
PopplerNode::
setupContext(){
  bool bMipmap = getMaterial().getUseMipmaps();
  m_pTex    = GLContextManager::get()->createTexture(m_pNodeSize, m_pPixelFormat, bMipmap);
  getSurface()->create(m_pPixelFormat,m_pTex);
  newSurface();
  setupFX();
}

void
PopplerNode::
connect(CanvasPtr pCanvas)
{
  RasterNode::connect(pCanvas);
}

void
PopplerNode::
connectDisplay()
{
  RasterNode::connectDisplay();
  open();
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

void
PopplerNode::
renderFX() {
    RasterNode::renderFX(getSize(), Pixel32(255, 255, 255, 255), false);
}

void PopplerNode:: render() {
  //cout << "... render()" << endl;
  //ScopeTimer Timer(CameraProfilingZone);
  blt32(getTransform(), getSize(), getEffectiveOpacity(), getBlendMode());
}
