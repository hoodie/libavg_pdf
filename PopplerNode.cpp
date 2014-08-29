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

#include "PopplerNode.h"

using namespace std;
using namespace boost::python;
using namespace avg;

char popplerNodeName[] = "popplernode";


void PopplerNode::
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

PopplerNode:: ~PopplerNode() {

}

PopplerNode::
PopplerNode(const ArgList& args)
    : m_pPixelFormat(avg::R8G8B8A8)
    , m_pPdfPath("")
    , m_bNewBmp(false)
    , m_iPageCount(-1)
    , m_iCurrentPage(-1)
{
  
  // TODO initialize with `path` or with &PopplerPage (to display multiple pages at once)
  
  AVG_TRACE( Logger::category::PLUGIN, Logger::severity::INFO, "PopplerNode c'tor gets Argument path= "  << args.getArgVal<string>("path") );
  AVG_TRACE( Logger::category::PLUGIN, Logger::severity::INFO, "PopplerNode constructed with " << m_pPdfPath );
  args.setMembers(this);
  
  if(!this->loadDocument()) 
    cout << "[fail] could not open document" << endl; // TODO load some placeholder in case of loadfailure
    
}

void
PopplerNode::
setPath(std::string path) {
    std::cout << "setting path to \"" << path << "\"" << std::endl;
    m_pPdfPath = path;
}

const string PopplerNode::getPopplerVersion() const { return poppler_get_version(); }

IntPoint
PopplerNode::
getMediaSize()
{
  // TODO store a currentPage
  PopplerPage *page = poppler_document_get_page( m_pDocument, 0);
  double width, height;
  poppler_page_get_size(page, &width, &height);
  return IntPoint(width,height);
}

const string
PopplerNode::
getPath() const {
    return m_pPdfPath;
}

bool
PopplerNode::
loadDocument()
{
    cout << "--- loading document (" << m_pPdfPath << ")" << endl;
    GError *error = NULL;
    m_pDocument = poppler_document_new_from_file(m_pPdfPath.c_str(), NULL, &error);

    if(m_pDocument == NULL) {
        cout << "[fail] Problem loading " << m_pPdfPath << endl;
        cout << error->message << endl;
        return false;
    }
    cout << "[ok] loaded document " << poppler_document_get_title(m_pDocument) << endl;
    m_iPageCount = poppler_document_get_n_pages(m_pDocument);
    
    if(m_iCurrentPage > 0){
      m_iCurrentPage = 0;
      //m_vPageBitmaps = std::vector<avg::BitmapPtr>;
      
      for(int i = 0; i< m_iPageCount; ++i){
        m_vPages.at(i) = poppler_document_get_page(m_pDocument,i);
      }
      
    }
    else
      cout << "document seems to have 0 pages";
      return false;

    return true;
}


void PopplerNode
::fill_bitmap(PopplerPage *page, double width = 0, double height= 0)
{
  //cout << "--- fill_bitmap()" << endl;
  cairo_surface_t *surface;
  cairo_t *cairo;
  //double xscale, yscale;
  
  IntPoint size = IntPoint(width, height);
  if (width == 0 or height==0)
    size = getMediaSize();
  
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size.x, size.y);
  cairo   = cairo_create(surface);
  
  //cout << "---- created surface" << endl;
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

void PopplerNode
::rerender(int page_index, double width = 0, double height = 0)
{
  if(page_index < 0 or page_index >= m_iPageCount)
    return;
  
  PopplerPage *page = m_vPages[page_index];
  
  fill_bitmap(page, width, height);
  
}

void PopplerNode:: open()
{
    //cout << "+++ open()" << endl;
    
    setViewport(-32767, -32767, -32767, -32767);
    PopplerPage *page = poppler_document_get_page(m_pDocument, 0);
    IntPoint     size = getMediaSize();
    bool bMipmap = getMaterial().getUseMipmaps();

    m_pTex    = GLContextManager::get()->createTexture(size, m_pPixelFormat, bMipmap);
    getSurface()->create(m_pPixelFormat,m_pTex);
    fill_bitmap(page);
    newSurface();
    setupFX();
    //cout << "++++ set bitmap" << endl;
}

void PopplerNode:: connect(CanvasPtr pCanvas)
{
  //cout << "... connect()" << endl;
  RasterNode::connect(pCanvas);
}

void PopplerNode::connectDisplay()
{
  //cout << "... connectDisplay()" << endl;
  RasterNode::connectDisplay();
  open();
}

void PopplerNode:: preRender(const VertexArrayPtr& pVA, bool bIsParentActive, float parentEffectiveOpacity)
{
  Node::preRender(pVA, bIsParentActive, parentEffectiveOpacity);
  //cout << "... preRender()" << endl;
  //PopplerPage* page = poppler_document_get_page(m_document, 0);
  if(isVisible()) {
    if(m_bNewBmp) {
      //ScopeTime Timer(); # TODO time poppler rendering
      GLContextManager::get()->scheduleTexUpload(m_pTex, m_pBitmap);
      m_bNewBmp = false;
    }
  }
  calcVertexArray(pVA);
}

void PopplerNode:: renderFX() {
    RasterNode::renderFX(getSize(), Pixel32(255, 255, 255, 255), false);
}

void PopplerNode:: render() {
  //cout << "... render()" << endl;
  //ScopeTimer Timer(CameraProfilingZone);
  blt32(getTransform(), getSize(), getEffectiveOpacity(), getBlendMode());
}

void PopplerNode:: testFunction(){
  // implements all sorts of test stuff
  cout << "---testfunction--x" << endl;
  
  cout << AreaNode::getSize().x <<  ", " << AreaNode::getSize().y << endl;
  rerender(0, AreaNode::getSize().x, AreaNode::getSize().y);
  
  cout << "---testfunction---" << endl << endl;
}



BOOST_PYTHON_MODULE(popplernode) {
    class_<PopplerNode, bases<RasterNode>, boost::noncopyable>("PopplerNode", no_init)
    .def( "__init__", raw_constructor(createNode<popplerNodeName>) )
    .def( "next", &PopplerNode::getPopplerVersion)
    .def( "test", &PopplerNode::testFunction)
    .add_property( "path",
                   &PopplerNode::getPath,
                   &PopplerNode::setPath
                 )
    // TODO add size with resizing and scaling
    // TODO add rerender()
    .add_property( "mediaSize",
                   &PopplerNode::getMediaSize
                 )
    .add_property( "poppler_version",
                   &PopplerNode::getPopplerVersion
                 );

    //.add_property( "fillcolor",
    //    make_function(&ColorNode::getFillColor, return_value_policy<copy_const_reference>()),
    //    &ColorNode::setFillColor);
}

AVG_PLUGIN_API void registerPlugin() {
    initpopplernode();
    object mainModule(handle<>(borrowed(PyImport_AddModule("__builtin__"))));
    object popplerModule(handle<>(PyImport_ImportModule("popplernode")));
    mainModule.attr("popplerplugin") = popplerModule;

    avg::PopplerNode::registerType();
}

