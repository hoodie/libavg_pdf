import os, time, math, cairo
import libavg

from libavg  import gesture, DivNode, Publisher, avg
from widgets import DivNodePlus

#TODO replace DivNodePlus
#TODO use avg logging

libavg.player.loadPlugin("popplerplugin")
from popplerplugin import PopplerNode

exit

class PdfNode(DivNodePlus):

    HIGHLIGHTING_STARTED = avg.Publisher.genMessageID()
    HIGHLIGHTING_ENDED   = avg.Publisher.genMessageID()

    HIGHLIGHT_COLOR = "424242"
    # TODO: copy DivNodePlus functionality over to PdfNode to remove dependency for later extraction

    def __init__(self, path = None, show_annotations = True, parent=None, *args, **kwargs):
        super(PdfNode,self).__init__(*args, **kwargs)
        self.registerInstance(self, parent)
        self.publish(self.HIGHLIGHTING_STARTED)
        self.publish(self.HIGHLIGHTING_ENDED  )

        self.crop             = True
        self.background_color= "FFFFFF"
        self.__path          = None

        self.__backNode      = None
        self.__layoutsNode   = None
        self.__annotsNode    = None
        self.__popplerNode   = None
        self.__highlightNode = None
        self.__drag_start    = None

        self.__show_Back     = True
        self.__show_Layout   = False # debug only
        self.__show_Annots   = show_annotations
        self.__show_Poppler  = False

        self.__rendered      = False
        self.__annotNodes    = {}
        self.local_annotations = []

        if path and os.path.exists(path):
            self.__path  = os.path.abspath(path)
            self.__popplerstyle_path  = self.__path
            self.__popplerNode = PopplerNode(path = self.__popplerstyle_path)
            self.__popplerNode.render_annots = False
            self.appendChild(self.__popplerNode)

        else:
            print "path {0} does not exist".format(path)

        self.__layoutLists   = [self.__popplerNode.getPageLayout(i) for i in range(self.pageCount)]


    ## properties
    @property
    def scale(self):
        return self.size.x / self.__popplerNode.getPageSize(self.currentPage)[0]

    @property
    def path(self):
        return self.__path

    @property
    def pageCount(self):
        return self.__popplerNode.pageCount;

    @property
    def currentPage(self):
        page = self.__popplerNode.getCurrentPage();
        return page
    
    def setHighlightColor(self, color):
        self.HIGHLIGHT_COLOR = color



    ## public methods
    def prev(self):
        self.__setupPage(self.currentPage - 1)
        self.__fitSize(  self.__popplerNode.size )

    def next(self):
        self.__setupPage(self.currentPage + 1)
        self.__fitSize(  self.__popplerNode.size )

    def getPageImage(self, page_index = 0, image_index = 0):
        return self.__popplerNode.getPageImage(page_index, image_index)

    def getPageImageNode(self, page_index = 0, image_index = 0):
        imageNode = libavg.ImageNode()
        imageNode.setBitmap( self.getPageImage(page_index, image_index) )
        return imageNode

    def getPageImages(self, page_index = 0):
        return self.__popplerNode.getPageImages(page_index)

    def getPageSize(self, page_index = 0):
        return self.__popplerNode.getPageSize(page_index)

    def getAnnotations(self, page_index = -1):
        if page_index < 0:
            page_index = self.currentPage

        snippets = []
        for annotation in self.__popplerNode.getPageAnnotations(page_index):
            snippets.append(annotation.box.payload)
            self.local_annotations.append(annotation.box.payload) # used for icon
        return snippets





    ## rendering in multiple layers
    def render(self):
        if self.__rendered:
            self.__rerender()
            return
        self.__rendered = True

        print "    PdfNode::__render()"
        self.__setupPage(0)
        self.__fitSize(self.size)

    def __rerender(self):
        print "    PdfNode::__rerender()"
        self.__popplerNode.rerender()

    def __fitSize(self, size):
        print "    PdfNode::__fitSize({0})".format(size)

        if self.__backNode != None:
            self.__backNode.size = size

        if self.__layoutsNode != None:
            self.__layoutsNode.size  = size

        if self.__annotsNode != None:
            self.__annotsNode.size  = size

        self.__popplerNode.size = size 

    def __setupPage(self, page_index = -1):
        print "    PdfNode::__setupPage({0})".format(page_index)

        if 0 <= page_index and page_index < self.__popplerNode.getPageCount():

            self.__popplerNode.setCurrentPage(page_index)

            if self.__show_Annots:
                if self.__annotsNode != None:
                    self.removeChild(self.__annotsNode)
                self.__annotsNode   = self.__getAnnotsNode(page_index)
                self.appendChild(self.__annotsNode)

            if self.__show_Back:
                if self.__backNode:
                    self.removeChild(self.__backNode)
                self.__backNode     = self.__renderBack(page_index)
                self.appendChild(self.__backNode)

            if self.__show_Layout:
                if self.__layoutsNode != None:
                    self.removeChild(self.__layoutsNode)
                self.__layoutsNode  = self.__renderLayouts(page_index)
                self.appendChild(self.__layoutsNode)

            # adapt z-index
            self.reorderChild(self.__popplerNode, self.getNumChildren()-1)
            self.reorderChild(self.__annotsNode, self.getNumChildren()-1)

    def __getAnnotsNode(self, page_index = -1):
        # rendering from annotNode or looking up in self._
        print "    PdfNode::__getAnnotsNode({0})".format(page_index)
        if page_index in self.__annotNodes.keys():
            annotNode = self.__annotNodes[page_index]
            return annotNode

        elif 0 <= page_index < self.pageCount:
            annotNode = self.__renderAnnots(page_index)
            self.__annotNodes[page_index] = annotNode
            return annotNode
        return None

    def __renderBack(self, page_index = -1):
        return libavg.RectNode(
                fillcolor = self.background_color,
                size = self.__popplerNode.size,
                fillopacity = 1)

    # returns a DivNodePlus
    def __renderAnnots(self,page_index):
        annotNode = DivNodePlus()

        annotNode.size = self.__popplerNode.getPageSize(page_index)
        cp = self.currentPage

        print "    PdfNode::__renderAnnots()"
        for annotation in self.__popplerNode.getPageAnnotations(cp):
            area = annotation.area
            box  = annotation.box

            page_size = self.__popplerNode.getPageSize(cp)
            w = page_size[0]
            h = page_size[1]

            annot =  DivNodePlus()
            annot.size = self.__popplerNode.getPageSize(cp)

            if len(annotation.contents) > 0:
                text = annotation.contents
            else:
                text = box.payload

            rect = libavg.RectNode(
                    pos = libavg.Point2D( box.x, box.y ),
                    size = libavg.Point2D( box.width , box.height),
                    fillcolor = annotation.color.string[:-2], fillopacity = 1,
                    strokewidth = 0, parent = annot)

            words = libavg.WordsNode(
                    pos = libavg.Point2D( area.x1, h - area.y2),
                    size= libavg.Point2D( area.x2 - area.x1 +10, area.y2 - area.y1),
                    fontsize = 8 , color="000000", text=text,
                    alignment="left", parent = annot)


            annotNode.appendChild(annot)
            print box.payload
            #print "\nappending annotation ( " + str(rect.size) + " ) -> " + str( annot.size )

        return annotNode

    def __renderLayouts(self,page_index):
        print  "    PdfNode::__renderLayouts({0}) # size = {1}".format(page_index, self.size)
        #print "       {0}".format( self.__popplerNode.getPageSize(page_index))
        layoutsNode = DivNodePlus() # way too damn slow
        layoutsNode.size = self.__popplerNode.getPageSize(page_index)

        for layout in self.__popplerNode.getPageLayout(page_index):

            rect = libavg.RectNode(
                    pos = libavg.Point2D( layout.x, layout.y),
                    size = libavg.Point2D( layout.width, layout.height ),
                    color = "000000", opacity = 1,
                    fillcolor = "FF55FF", fillopacity = .1,
                    parent = layoutsNode
                    )
        layoutsNode.size = self.size
        layoutsNode.tell_resizeChildren = True
        return layoutsNode








    ## highlighting text

    HIGHLIGHTING_ENABLED = None
    def enable_highlighting(self):
        # TODO realize highlighting with a dragRecognizer
        if not self.HIGHLIGHTING_ENABLED:
            self.HIGHLIGHTING_ENABLED = True
            self.dragRecognizer = gesture.DragRecognizer( self,
                direction=gesture.DragRecognizer.HORIZONTAL,
                minDragDist = 0.0,
                moveHandler = self.__onDragMove,
                possibleHandler = self.__onDragPossible,
                detectedHandler = self.__onDragDetected,
                #upHandler = self.__onDrag...,
                endHandler = self.__onDragEnd,

                )
            #self.subscribe(self.CURSOR_DOWN, self.__onMouseDown)
            #self.subscribe(self.CURSOR_UP,   self.__onMouseUp)

    def disable_highlighting(self):
        if self.HIGHLIGHTING_ENABLED:
            self.HIGHLIGHTING_ENABLED = False
            if not self.dragRecognizer == None:
              self.dragRecognizer.enable(False)


    def __onDragPossible(self):
        #print "__onDragPossible"
        self.__drag_start = self.dragRecognizer.contacts[0].events[0].pos

    def __onDragDetected(self):
        #print "__onDragDetected"
        self.notifySubscribers(self.HIGHLIGHTING_STARTED, [])
        self.__drag_start = self.dragRecognizer.contacts[0].events[0].pos
        self.__resetHighlight()
        self.__showHighlightNode()

    def __onDragEnd(self):
        #print "__onDragEnd"
        highlightedText = self.__getHighlightedText()
        self.local_annotations.append(highlightedText)
        self.notifySubscribers(self.HIGHLIGHTING_ENDED, [highlightedText])
        print highlightedText

    def __onDragMove(self, offset):
        #print "__onDragMove"
        pos = offset+ self.__drag_start
        self.__testForLayoutBox(pos)

    def __getHighlightedText(self):
        highlightedText = ""
        if self.__highlight_first > 0 and self.__highlight_last > 0:
            for box_index in xrange(self.__highlight_first, self.__highlight_last+1):
                box = self.__layoutLists[self.currentPage][box_index]
                payload = box.payload
                if len(payload) == 0:
                    payload = " "
                highlightedText += payload
        return highlightedText


    def __showHighlightNode(self):
        self.__highlightNode = DivNodePlus()
        self.__highlightNode.size = self.size
        self.appendChild(self.__highlightNode)
        self.reorderChild(self.__highlightNode, self.getNumChildren()-3)
        self.__highlightedBoxes = {}

    def __resetHighlight(self):
        page_index = self.currentPage
        self.__highlight_first = self.__highlight_last = -1
        self.__hideHighlightNode()
        self.__highlightedBoxes = {}

    def __hideHighlightNode(self):
        if self.__highlightNode != None:
            self.removeChild(self.__highlightNode)
            self.__highlightNode = None

    def __testForLayoutBox(self, pos):
        pos = (pos - self.pos) / self.scale # corresponds to original page size

        for i, lbox in enumerate(self.__layoutLists[self.currentPage]):
            if lbox.x <=  pos.x and pos.x <= lbox.x2 and lbox.y2 <= pos.y and pos.y <=lbox.y:
                if self.__highlight_first == -1:
                    self.__highlight_first = i
                self.__highlight_last = i
                self.__renderHighlightBoxes()
                

    def __renderHighlightBoxes(self):
        start = min(self.__highlight_first, self.__highlight_last)
        end   = max(self.__highlight_first, self.__highlight_last)
        gonners = []

        # add new boxes ( going forwards )
        for i in xrange(start, end+1):
            if not (i in self.__highlightedBoxes.keys()):
                #print "adding box", i
                box = self.__renderHighlightBox(i)
                self.__highlightedBoxes[i] = box
        
        # remove out of range boxes ( e.g. if going backwards )
        for i, box in self.__highlightedBoxes.iteritems():
            if (i < start or end < i) and i in self.__highlightedBoxes.keys():
                gonners.append(i)
                #print "removing box", i

        for i in gonners:
            box = self.__highlightedBoxes[i]
            self.__highlightedBoxes.pop(i)
            self.__highlightNode.removeChild(box)


    def __renderHighlightBox(self, box_index ):
            box = self.__layoutLists[self.currentPage][box_index]
            return libavg.RectNode(
                    fillcolor = self.HIGHLIGHT_COLOR, fillopacity = 1, opacity = 0,
                    size = libavg.Point2D(box.width, box.height)*self.scale,
                    pos  = libavg.Point2D(box.x, box.y)*self.scale,
                    parent = self.__highlightNode)

    def __renderHighlightBoxDEBUG(self, box_index ):
            box = self.__layoutLists[self.currentPage][box_index]
            text = box.payload
            div = DivNode(
                    size = libavg.Point2D(box.width, -box.height)*self.scale,
                    pos  = libavg.Point2D(box.x, box.y)*self.scale,
                    parent = self.__highlightNode
                    )

            libavg.RectNode(
                    fillcolor = self.HIGHLIGHT_COLOR, fillopacity = 1, opacity = 0,
                    size = libavg.Point2D(box.width, box.height)*self.scale,
                    parent = div
                    )

            words = libavg.WordsNode(
                    size = libavg.Point2D(abs(box.width), abs(box.height))*self.scale,
                    fontsize = 12 , color="000000", text=text,
                    alignment="left", parent = div)


            return div
