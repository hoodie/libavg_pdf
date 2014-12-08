import libavg
from libavg import Point2D, DivNode


class DivNodePlus(DivNode):

    def __init__(self, parent = None, *args, **kwargs):
        super(DivNodePlus,self).__init__(crop = True, *args, **kwargs)
        self.registerInstance(self, parent)
        self.old_size = self.size
        self.subscribe(self.SIZE_CHANGED, self.resizeChildren)
        self.tell_resizeChildren = False

    ## children need to be handled nicer, don't you agree?
    @property
    def children(self):
        return [self.getChild(i) for i in xrange(self.getNumChildren())]

    def children_each(self, block):
        for child in self.children:
            block.__call__(child)

    def resizeChildren(self, new_size):
        if self.old_size.x == 0:
            self.old_size = self.size
            #print "     DivNodePlus::resizeChildren() did not work -> {0}".format(self.old_size)
            #return

        try:
            ratio = new_size.x/self.old_size.x
        except ZeroDivisionError:
            return False

        if ratio == 1:
            return False

        if self.tell_resizeChildren:
            print "--> resizing children"
        for i in xrange(self.getNumChildren()):
            child = self.getChild(i)
            #print "     DivNodePlus::resizeChildren() resizing by {0}".format(ratio)
            child.size = child.size * ratio
            child.pos  = child.pos  * ratio
            if child.__class__ == libavg.WordsNode:
                try:
                    child.fontsize = child.fontsize * ratio
                except RuntimeError:
                    print "fontsize remained unchanged"

        self.old_size = self.size
        return True

    def resize(self, new_size):
        old_size  = self.size
        self.size = new_size
        self.resizeChildren(self.size)

    def scale(self, ratio):
        self.resize(self.size * ratio)


    ## make sure size reflects the content of the div, right after every appendChild
    def getMediaSize(self):
        size = libavg.Point2D()
        for child in self.children:
            size.x = max( size.x, child.pos.x + child.size.x )
            size.y = max( size.y, child.pos.y + child.size.y )
        return size

    ## reset the size, according to content
    def appendChild(self, node):
        #print "appending Child"
        super(DivNodePlus, self).appendChild(node)
        if self.getMediaSize().x > 0 and self.getMediaSize().y > 0:
            self.old_size = self.size = self.getMediaSize()


    def fillParent(self, size = None):
        # takes size only for SIZE_CHANGED callback
        self.size = self.parent.size
        self.pos = Point2D(0,0)

    def snapToBottom(self, other = None):
        if other == None:
            other = self.parent
        self.y = other.y+other.height - self.height
