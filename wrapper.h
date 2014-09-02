#include <string>

namespace avg {

  typedef   int page_index_t ;

  typedef struct _Color{
    int red;
    int green;
    int blue;
  } Color;

  typedef struct _Annotation{
    PopplerRectangle area;
        std::string  name,contents,label;
        std::string  modified;
               _Color  color;
  //       enum  type; // TODO implement annot_type enum
  } Annotation;   

}