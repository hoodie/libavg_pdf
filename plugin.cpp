#include <glib/poppler.h>
#include <base/Exception.h>
#include <wrapper/WrapHelper.h>
#include <wrapper/raw_constructor.hpp>
#include "PopplerNode.h"

using namespace avg;
using namespace boost::python;

char popplerNodeName[] = "popplernode";

BOOST_PYTHON_MODULE(popplerplugin) {
  
  // TODO translate PopplerRectangle into libavg pendant
  class_<PopplerRectangle>("PopplerRectangle")
    .add_property("x1", &PopplerRectangle::x1).add_property("x2", &PopplerRectangle::x2)
    .add_property("y1", &PopplerRectangle::y1).add_property("y2", &PopplerRectangle::y2);
      
  class_<avg::Pixel32>("Pixel32").add_property("string", &avg::Pixel32::getColorString);

  class_<_Annotation>("Annotation") // from wrapper.h
    .add_property("area",       &Annotation::area)  
    .add_property("name",       &Annotation::name)  
    .add_property("box",        &Annotation::box)  
    .add_property("modified",   &Annotation::modified)
    .add_property("label",      &Annotation::label)
    .add_property("color",      &Annotation::color)
    .add_property("contents",   &Annotation::contents);
  
  class_<_Box>("Box") // from wrapper.h
    .add_property("x",      &Box::x)
    .add_property("y",      &Box::y)
    .add_property("x2",     &Box::x2)
    .add_property("y2",     &Box::y2)
    .add_property("payload",&Box::payload)
    .add_property("width",  &Box::width)
    .add_property("height", &Box::height);
    
  class_<PopplerNode, bases<RasterNode>, boost::noncopyable>("PopplerNode", no_init)
    .def( "__init__", raw_constructor(createNode<popplerNodeName>) )
    .def( "next",               &PopplerNode::getPopplerVersion)
    .def( "rerender",           &PopplerNode::rerender)
    .def( "resize",             &PopplerNode::resize)
    .def( "getPageLayout",      &PopplerNode::getPageTextLayout)
    .def( "getPageSize",        &PopplerNode::getPageSize)
    .def( "getPageAnnotations", &PopplerNode::getPageAnnotations)
    .def( "getPageImages",      &PopplerNode::getPageImages)
    .def( "getPageCount",       &PopplerNode::getPageCount )
    .def( "getCurrentPage",     &PopplerNode::getCurrentPage)
    .def( "setCurrentPage",     &PopplerNode::setCurrentPage)
    
    .add_property( "path",
                  &PopplerNode::getPath,
                  &PopplerNode::setPath )
    .add_property( "render_annots",
                  &PopplerNode::getRenderAnnotations,
                  &PopplerNode::setRenderAnnotations )
    .add_property( "pageCount",       &PopplerNode::getPageCount )
    //.add_property( "currentPage",     &PopplerNode::getCurrentPage)
    //.add_property( "title",           &PopplerNode::getDocumentTitle )
    //.add_property( "subject",         &PopplerNode::getDocumentSubject )
    //.add_property( "author",          &PopplerNode::getDocumentAuthor )
    .add_property( "poppler_version", &PopplerNode::getPopplerVersion );
    
}


AVG_PLUGIN_API PyObject* registerPlugin() {
#if PY_MAJOR_VERSION < 3
    initpopplerplugin();
    PyObject* pyPopplerModule = PyImport_ImportModule("popplerplugin");
#else
    PyObject* pyPopplerModule = PyInit_popplerplugin();
#endif

    avg::PopplerNode::registerType();
    return pyPopplerModule;
}
