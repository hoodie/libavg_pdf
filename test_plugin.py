#!/usr/bin/env python2

import sys,os
import libavg
from   libavg import app
q = quit

libavg.player.pluginPath = "."
libavg.player.loadPlugin("popplerplugin")
libavg.PopplerNode = popplerplugin.PopplerNode
PopplerNode = popplerplugin.PopplerNode

for possible_path in sys.argv:
  if os.path.exists(possible_path):
    path  = os.path.abspath(possible_path)
    
if path == None:
  print "no valid path"
  exit
  
popNode = libavg.PopplerNode(path = path)


class MyMainDiv(app.MainDiv):
    def onInit(self):
        self.toggleTouchVisualization()
        self.popNode  = popNode
        self.backNode = libavg.RectNode( fillcolor = "FFFFFF", size = libavg.Point2D(200,200), fillopacity = 1) 
        self.appendChild( self.backNode )
        self.appendChild( self.popNode )
        popNode.size = libavg.Point2D(200,200)  # TODO render PopplerNode without settings size first
        popNode.setCurrentPage(0)
        popNode.rerender()

        self.backNode.size = self.popNode.size
        print self.backNode.size

        app.keyboardmanager.bindKeyDown("v", self.handle_vers, help, modifiers=libavg.KEYMOD_NONE)
        app.keyboardmanager.bindKeyDown("j", self.handle_next, help, modifiers=libavg.KEYMOD_NONE)
        app.keyboardmanager.bindKeyDown("k", self.handle_prev, help, modifiers=libavg.KEYMOD_NONE)
        app.keyboardmanager.bindKeyDown("s", self.handle_size, help, modifiers=libavg.KEYMOD_NONE)
        app.keyboardmanager.bindKeyDown("i", self.handle_incr, help, modifiers=libavg.KEYMOD_NONE)
        app.keyboardmanager.bindKeyDown("d", self.handle_decr, help, modifiers=libavg.KEYMOD_NONE)
        app.keyboardmanager.bindKeyDown("t", self.handle_test, help, modifiers=libavg.KEYMOD_NONE)

        app.keyboardmanager.bindKeyDown("f", self.show_figure, help, modifiers=libavg.KEYMOD_NONE)

    def show_figure(self):
        imageNode = libavg.ImageNode()
        imageNode.setBitmap( popNode.getPageImage(0,0) )
        self.appendChild(imageNode)
        
    def handle_test(self):
      self.popNode.rerender()
      
    def handle_incr(self):
      self.popNode.size *= 1.1
      self.backNode.size = self.popNode.size
      
    def handle_decr(self):
      self.popNode.size *= 0.9
      self.backNode.size = self.popNode.size
      
    def handle_vers(self):
      print self.popNode.poppler_version
      
    def handle_size(self):
      print self.popNode.size
      
    def handle_prev(self):
      page = self.popNode.currentPage
      self.popNode.setCurrentPage(page-1)

    def handle_next(self):
      page = self.popNode.currentPage
      self.popNode.setCurrentPage(page+1)

    def onExit(self):
        pass

    def onFrame(self):
        pass

appapp = app.App() 

def start():
    sys.argv = []
    appapp.run(MyMainDiv())

if "annots" in sys.argv:
    print "ANNOTS"
    annots = popNode.getPageAnnotations(0)
    print annots

elif "image" in sys.argv:
    image = popNode.getPageImage(0,0)
    print image
    
elif "images" in sys.argv:
    print "IMAGES"
    images = popNode.getPageImages(0)
    for image in images:
        print "x:{x} y:{y}".format(x = image.x, y = image.y)
        print "width:{width} height:{height}".format(width = image.width, height = image.height)


elif "start" in sys.argv:
    start()
  
hello = "hello world"

print
print "q = quit"
