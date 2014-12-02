#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math, libavg

from libavg import avg, player, geom
from PdfNode import PdfNode

canvas = player.createMainCanvas(size=(1024,768))
rootNode = canvas.getRootNode()
avg.WordsNode(pos=(10,10), text="Hello World", parent=rootNode)


pdf = PdfNode('../docs/paper.pdf')
#pdf.render()
pdf.size *= 0.4
pdf.render()


print "pageimages"
i = 9
print i


def drawPersonIcon(color = "339933", pos = (0,0)):
    personIcon = libavg.DivNode(pos = pos)
    personIcon.size = (150,150)
    geom.PieSlice(radius = 50, startangle=-math.pi, endangle=0, fillcolor=color, fillopacity=1, pos = (80,160), parent = personIcon )
    libavg.CircleNode(r=40, pos = (80,80), fillcolor=color, fillopacity=1, parent = personIcon)
    return personIcon


rootNode.appendChild(drawPersonIcon(color = "990000", pos =(0,0)))
rootNode.appendChild(drawPersonIcon(color = "009900", pos =(150,0)))

rootNode.appendChild(pdf.getPage())

player.play()
