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
    path  = "file://" + os.path.abspath(possible_path)
    
if path == None:
  print "no valid path"
  exit
  
popNode = libavg.PopplerNode(path = path)
print popNode.size


class MyMainDiv(app.MainDiv):
    def onInit(self):
        self.toggleTouchVisualization()
        self.popNode  = popNode
        self.backNode = libavg.RectNode( fillcolor = "FFFFFF", size = libavg.Point2D(200,200), fillopacity = 1) 
        self.appendChild( self.backNode )
        self.appendChild( self.popNode )
        popNode.rerender(0)
        
        #self.popNode.size *= .5
        self.backNode.size = self.popNode.size

        app.keyboardmanager.bindKeyDown("v", self.handle_vers, help, modifiers=libavg.KEYMOD_NONE)
        app.keyboardmanager.bindKeyDown("j", self.handle_next, help, modifiers=libavg.KEYMOD_NONE)
        app.keyboardmanager.bindKeyDown("k", self.handle_prev, help, modifiers=libavg.KEYMOD_NONE)
        app.keyboardmanager.bindKeyDown("s", self.handle_size, help, modifiers=libavg.KEYMOD_NONE)
        app.keyboardmanager.bindKeyDown("i", self.handle_incr, help, modifiers=libavg.KEYMOD_NONE)
        app.keyboardmanager.bindKeyDown("d", self.handle_decr, help, modifiers=libavg.KEYMOD_NONE)
        app.keyboardmanager.bindKeyDown("t", self.handle_test, help, modifiers=libavg.KEYMOD_NONE)
        
    def handle_test(self):
      page = self.popNode.current_page
      self.popNode.rerender(page)
      self.popNode.adaptSize()
      
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
      page = self.popNode.current_page
      self.popNode.rerender(page-1)

    def handle_next(self):
      page = self.popNode.current_page
      self.popNode.rerender(page+1)

    def onExit(self):
        pass

    def onFrame(self):
        pass

appapp = app.App() 

def start():
    sys.argv = []
    appapp.run(MyMainDiv())

if "start" in sys.argv:
  start()
  
hello = "hello world"

print
print "q = quit"
