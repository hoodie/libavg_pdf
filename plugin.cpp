#include <glib/poppler.h>
#include <base/Exception.h>
#include <wrapper/WrapHelper.h>
#include <wrapper/raw_constructor.hpp>
#include "PopplerNode.h"

#include "wrapper.h"

using namespace avg;
using namespace boost::python;

char popplerNodeName[] = "popplernode";

BOOST_PYTHON_MODULE(popplerplugin) {
  
  // TODO translate PopplerRectangle into libavg pendant
  class_<PopplerRectangle>("PopplerRectangle")
    .add_property("x1", &PopplerRectangle::x1).add_property("x2", &PopplerRectangle::x2)
    .add_property("y1", &PopplerRectangle::y1).add_property("y2", &PopplerRectangle::y2);
      
  class_<_Annotation>("Annotation") // from wrapper.h
    .add_property("area",       &Annotation::area)  
    .add_property("name",       &Annotation::name)  
    .add_property("modified",   &Annotation::modified)
    .add_property("label",      &Annotation::label)
    .add_property("color",      &Annotation::color)
    .add_property("contents",   &Annotation::contents);
  
  class_<_Color>("Color") // from wrapper.h
    .add_property("red",    &Color::red  )
    .add_property("green",  &Color::green)
    .add_property("blue",   &Color::blue );
    

  class_<PopplerNode, bases<RasterNode>, boost::noncopyable>("PopplerNode", no_init)
    .def( "__init__", raw_constructor(createNode<popplerNodeName>) )
    .def( "next",           &PopplerNode::getPopplerVersion)
    .def( "rerender",       &PopplerNode::rerender)
    .def( "resize",         &PopplerNode::resize)
    .def( "getPageLayout",  &PopplerNode::getPageTextLayout)
    .def( "setPage",        &PopplerNode::setPage)
    .def( "getPageAnnotations", &PopplerNode::getPageAnnotations)
    .def( "getPageCount",   &PopplerNode::getPageCount )
    .def( "getCurrentPage", &PopplerNode::getCurrentPage)
    
    .add_property( "path",
                  &PopplerNode::getPath,
                  &PopplerNode::setPath )
    .add_property( "mediaSize",       &PopplerNode::getMediaSize )
    .add_property( "pageCount",       &PopplerNode::getPageCount )
    //.add_property( "currentPage",     &PopplerNode::getCurrentPage)
    //.add_property( "title",           &PopplerNode::getDocumentTitle )
    //.add_property( "subject",         &PopplerNode::getDocumentSubject )
    //.add_property( "author",          &PopplerNode::getDocumentAuthor )
    .add_property( "poppler_version", &PopplerNode::getPopplerVersion );
    
}


AVG_PLUGIN_API void registerPlugin() {
    initpopplerplugin();
    object mainModule(handle<>(borrowed(PyImport_AddModule("__builtin__"))));
    object popplerModule(handle<>(PyImport_ImportModule("popplerplugin")));
    mainModule.attr("popplerplugin") = popplerModule;

    avg::PopplerNode::registerType();
}
