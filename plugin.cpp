#include <glib/poppler.h>
#include <base/Exception.h>
#include <wrapper/WrapHelper.h>
#include <wrapper/raw_constructor.hpp>
#include "PopplerNode.h"
#include "wrapper.h"

using namespace avg;
using namespace boost::python;

char popplerNodeName[] = "popplernode";
char pageNodeName[]    = "pagenode";

BOOST_PYTHON_MODULE(popplerplugin) {
  
  class_<PopplerRectangle>("PopplerRectangle")
    .add_property("x1", &PopplerRectangle::x1).add_property("x2", &PopplerRectangle::x2)
    .add_property("y1", &PopplerRectangle::y1).add_property("y2", &PopplerRectangle::y2);
      
  class_<PageNode, bases<RasterNode>, boost::noncopyable>("PageNode", no_init)
    .def( "__init__", raw_constructor(createNode<pageNodeName>) )
    .add_property( "poppler_version", &PopplerNode::getPopplerVersion );
  
    
  class_<_Color>("Color")
    .add_property("red",    &Color::red  )
    .add_property("green",  &Color::green)
    .add_property("blue",   &Color::blue );
    
  class_<_Annotation>("Annotation")
    .add_property("area",  &Annotation::area)  
    .add_property("name",  &Annotation::name)  
    //.add_property("annot", &Annotation::annot)
    .add_property("modified", &Annotation::modified)
    .add_property("color", &Annotation::color)
    .add_property("contents", &Annotation::contents);
  
    
  class_<PopplerNode, bases<RasterNode>, boost::noncopyable>("PopplerNode", no_init)
    .def( "__init__", raw_constructor(createNode<popplerNodeName>) )
    .def( "next",        &PopplerNode::getPopplerVersion)
    .def( "test",        &PopplerNode::testFunction)
    .def( "layout",      &PopplerNode::getPageTextLayout)
    .def( "resize",      &PopplerNode::resize)
    .def( "rerender",    &PopplerNode::rerender)
    .def( "annotations", &PopplerNode::getPageAnnotations)
    
    .add_property( "path",
                  &PopplerNode::getPath,
                  &PopplerNode::setPath )
    .add_property( "mediaSize",       &PopplerNode::getMediaSize )
    .add_property( "pageCount",       &PopplerNode::getPageCount )
    .add_property( "title",           &PopplerNode::getDocumentTitle )
    .add_property( "subject",         &PopplerNode::getDocumentSubject )
    .add_property( "author",          &PopplerNode::getDocumentAuthor )
    .add_property( "poppler_version", &PopplerNode::getPopplerVersion );
    
    //.add_property( "fillcolor",
    //    make_function(&ColorNode::getFillColor, return_value_policy<copy_const_reference>()),
    //    &ColorNode::setFillColor);
}


AVG_PLUGIN_API void registerPlugin() {
    initpopplerplugin();
    object mainModule(handle<>(borrowed(PyImport_AddModule("__builtin__"))));
    object popplerModule(handle<>(PyImport_ImportModule("popplerplugin")));
    mainModule.attr("popplerplugin") = popplerModule;

    avg::PageNode::registerType();
    avg::PopplerNode::registerType();
}
