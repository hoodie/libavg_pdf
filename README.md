# libavg PopplerNode Plugin

Render PDF documents in libavg.
It loads the entire document,
manages pages,
caches rendered pages and 
reads annotations and page layout.

## Requirementse:

-   poppler-glib
-   libavg >= 1.8.2


## Example: 

Warnung! These examples are slightly dated.
Checkout plugin.h and PdfNode.py for api.

```python
libavg.player.loadPlugin("popplerplugin")
popplerNode = popplerplugin.PopplerNode(path = path)

if os.path.exists(possible_path):
  path  = "file://" + os.path.abspath(possible_path)

popplerNode = libavg.PopplerNode(path = path) # loads the pdf file

someDivNode.appendChild( popplerNode )

popplerNode.rerender() # triggers first render

## switching pages

cp = popplerNode.currentPage # contains the current page

popplerNode.setCurrentPage(cp+1) # renders next page
popplerNode.setCurrentPage(cp-1) # renders previous page

# previously rendered pages are being cached

## size handeling is inherited from AreaNode
# PopplerNode does not setCurrentPage automatically after resizing
popplerNode.size *= 0.5

# trigger render to sharpen up
popplerNode.rerender() # this will drop the cache of rendered pages


```

## TODO ( not yet implemented functionality )

-   higher level wrapper written in python
*   background psetCurrentPageing of pages 
-   modifing pdf, including:

    -   adding annotations

## warning

if this documentation turns out to be out of date, take a look at `test_plugin.py` which is basically a little test implementation
