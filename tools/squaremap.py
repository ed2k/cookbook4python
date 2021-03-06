#! /usr/bin/env python
import wx, sys, os, logging
import wx.lib.newevent
log = logging.getLogger( 'squaremap' )
log.addHandler(logging.StreamHandler() )
#log.setLevel( logging.DEBUG )

SquareHighlightEvent, EVT_SQUARE_HIGHLIGHTED = wx.lib.newevent.NewEvent()
SquareSelectionEvent, EVT_SQUARE_SELECTED = wx.lib.newevent.NewEvent()
SquareActivationEvent, EVT_SQUARE_ACTIVATED = wx.lib.newevent.NewEvent()


class HotMapNavigator(object):
    ''' Utility class for navigating the hot map and finding nodes. '''

    @classmethod
    def findNode(class_, hot_map, targetNode, parentNode=None):
        ''' Find the target node in the hot_map. '''
        if parentNode == targetNode: return parentNode,hot_map,0
        for index, node in enumerate(hot_map):
            if node == targetNode:
                return parentNode, hot_map, index
            result = class_.findNode(node.children, targetNode, node)
            if result:
                return result
        return None
    
    @classmethod
    def findNodeAtPosition(class_, parent, position):
        ''' Retrieve the node at the given position. '''
        for node in parent.children:
            x,y,w,h = node.rect
            x,y,w,h = (int(x),int(y),int(w),int(h))
            if wx.Rect(x,y,w,h).Contains(position):
                return class_.findNodeAtPosition(node, position)
        return parent

    @staticmethod
    def firstChild(hot_map, index):
        ''' Return the first child of the node indicated by index. '''
        children = hot_map[index].children
        if children:
            return children[0]
        else:
            return hot_map[index] # No children, return the node itself
        
    @staticmethod
    def nextChild(hotmap, index):
        ''' Return the next sibling of the node indicated by index. '''
        nextChildIndex = min(index + 1, len(hotmap) - 1)
        return hotmap[nextChildIndex]
    
    @staticmethod
    def previousChild(hotmap, index):
        ''' Return the previous sibling of the node indicated by index. '''
        previousChildIndex = max(0, index - 1)
        return hotmap[previousChildIndex]

    @staticmethod
    def firstNode(hot_map):
        ''' Return the very first node in the hot_map. '''
        return hot_map[0]
    
    @classmethod
    def lastNode(class_, hot_map):
        ''' Return the very last node (recursively) in the hot map. '''
        children = hot_map[-1].children
        if children:
            return class_.lastNode(children)
        else:
            return hot_map[-1] # Return the last node
    
    
class SquareMap( wx.Panel ):
    """Construct a nested-box trees structure view"""

    BackgroundColor = wx.Color( 128,128,128 )
    max_depth = None
    max_depth_seen = None
    def __init__( 
        self,  parent=None, id=-1, pos=wx.DefaultPosition, 
        size=wx.DefaultSize, 
        style=wx.TAB_TRAVERSAL|wx.NO_BORDER|wx.FULL_REPAINT_ON_RESIZE, 
        name='SquareMap', model = None,
        adapter = None,
        labels = True, # set to True to draw textual labels within the boxes
        highlight = True, # set to False to turn of highlighting
        padding = 2, # amount to reduce the children's box from the parent's box
    ):
        super( SquareMap, self ).__init__(
            parent, id, pos, size, style, name
        )
        self.model = model     
        self.root_view_node = model
        self.reCalculate = True        
        self.padding = padding
        self.labels = labels
        self.highlight = highlight
        self.selectedNode = None
        self.highlightedNode = None
        self.Bind( wx.EVT_PAINT, self.OnPaint)
        self.Bind( wx.EVT_SIZE, self.OnSize )
        if highlight:
            self.Bind( wx.EVT_MOTION, self.OnMouse )
        self.Bind( wx.EVT_LEFT_UP, self.OnClickRelease )
        self.Bind( wx.EVT_LEFT_DCLICK, self.OnDoubleClick )
        self.Bind( wx.EVT_KEY_UP, self.OnKeyUp )
        #self.hot_map = []
        self.adapter = adapter or DefaultAdapter()
        self.DEFAULT_PEN = wx.Pen( wx.BLACK, 1, wx.SOLID )
        self.SELECTED_PEN = wx.Pen( wx.WHITE, 2, wx.SOLID )
        self.OnSize(None)
    def OnMouse( self, event ):
        """Handle mouse-move event by selecting a given element"""
        node = HotMapNavigator.findNodeAtPosition(self.root_view_node, event.GetPosition())        
        self.SetHighlight( node, event.GetPosition() )

    def OnClickRelease( self, event ):
        """Release over a given square in the map"""
        #node = HotMapNavigator.findNodeAtPosition(self.hot_map, event.GetPosition())
        node = HotMapNavigator.findNodeAtPosition(self.root_view_node, event.GetPosition())
        self.SetSelected( node, event.GetPosition() )
        
    def OnDoubleClick(self, event):
        """Double click on a given square in the map"""
        node = HotMapNavigator.findNodeAtPosition(self.root_view_node, event.GetPosition())
        if node:
            wx.PostEvent( self, SquareActivationEvent( node=node, point=event.GetPosition(), map=self ) )
    
    def OnKeyUp(self, event):
        event.Skip()
        if not self.selectedNode or not self.root_view_node:
            return
        
        if event.KeyCode == wx.WXK_HOME:
            x,y,w,h = self.root_view_node.rect
            self.root_view_node = self.model
            self.highlightedNode = self.model
            self.calculation(self.model.children,self.model,x,y,w,h)
            self.UpdateDrawing()
            return
        elif event.KeyCode == wx.WXK_END:
            self.SetSelected(HotMapNavigator.lastNode(self.model.children))
            return
        
        parent, children, index  = HotMapNavigator.findNode(self.model.children, self.selectedNode,self.model)
        
        if event.KeyCode == wx.WXK_DOWN:
            self.SetSelected(HotMapNavigator.nextChild(children, index))
        elif event.KeyCode == wx.WXK_UP:
            self.SetSelected(HotMapNavigator.previousChild(children, index))
        elif event.KeyCode == wx.WXK_RIGHT:
            self.SetSelected(HotMapNavigator.firstChild(children, index))
        elif event.KeyCode == wx.WXK_LEFT and parent:
            self.SetSelected(parent)
        elif event.KeyCode == wx.WXK_RETURN:
            print 'view node',self.selectedNode
            x,y,w,h = self.root_view_node.rect
            self.root_view_node = self.selectedNode
            self.highlightedNode = self.selectedNode
            self.calculation(self.selectedNode.children,self.selectedNode,x,y,w,h)
            wx.PostEvent(self, SquareActivationEvent(node=self.selectedNode,
                                                     map=self))
            self.UpdateDrawing()
    def GetSelected(self):
        return self.selectedNode
            
    def SetSelected( self, node, point=None, propagate=True ):
        """Set the given node selected in the square-map"""
        if node == self.selectedNode:
            return

        if self.selectedNode:
          self.drawOneBox(self.selectedNode,False)
        self.selectedNode = node 
        self.drawOneBox(node)        
        #self.Refresh()
        if node:
            wx.PostEvent( self, SquareSelectionEvent( node=node, point=point, map=self ) )

    def SetHighlight( self, node, point=None, propagate=True ):
        """Set the currently-highlighted node"""
        if node == self.highlightedNode:
            return
        self.highlightedNode = node 
        #self.Refresh()

        if node and propagate:
            wx.PostEvent( self, SquareHighlightEvent( node=node, point=point, map=self ) )

    def SetModel( self, model, adapter=None ):
        """Set our model object (root of the tree)"""
        self.model = model
        if adapter is not None:
            self.adapter = adapter
        self.Refresh()
        
    def Refresh(self):
        self.UpdateDrawing()
    
    def OnPaint(self, event):
        dc = wx.BufferedPaintDC(self, self._buffer)
       
    def OnSize(self, event):
        # The buffer is initialized in here, so that the buffer is always
        # the same size as the Window.
        width, height = self.GetClientSizeTuple()
        # Make new off-screen bitmap: this bitmap will always have the
        # current drawing in it, so it can be used to save the image to
        # a file, or whatever.
        if width and height:
            # Macs can generate events with 0-size values
            self._buffer = wx.EmptyBitmap(width, height)
            x,y,w,h = self.root_view_node.rect
            if w!=0 :
              self.scale(self.root_view_node,float(width)/w,float(height)/h)                 
            else: self.calculation(self.root_view_node.children,self.root_view_node,0,0,width,height)
            self.UpdateDrawing()
    def scale(self,node,wscale,hscale):
        x,y,w,h = node.rect
        node.rect = (x*wscale,y*hscale,w*wscale,h*hscale)
        for n  in node.children:
            self.scale(n,wscale,hscale)
    def UpdateDrawing(self):
        dc = wx.BufferedDC(wx.ClientDC(self), self._buffer)
        self.Draw(dc)
        
    def Draw(self, dc):
        ''' Draw the tree map on the device context. '''
        #if self.reCalculate: self.hot_map = []
        dc.BeginDrawing()
        brush = wx.Brush( self.BackgroundColor  )
        dc.SetBackground( brush )
        dc.Clear()
        if self.root_view_node:
            self.max_depth_seen = 0
            dc.SetFont(self.FontForLabels(dc))
            w, h = dc.GetSize()
            self.DrawBox( dc, self.root_view_node, 0,0,w,h, [])
        dc.EndDrawing()
        #self.reCalculate = False
        
    def FontForLabels(self, dc):
        ''' Return the default GUI font, scaled for printing if necessary. '''
        font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        scale = dc.GetPPI()[0] / wx.ScreenDC().GetPPI()[0]
        font.SetPointSize(scale*font.GetPointSize())
        return font
    
    def BrushForNode( self, node, depth=0 ):
        """Create brush to use to display the given node"""
        if node == self.selectedNode:
            color = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        elif node == self.highlightedNode:
            color = wx.Color( red=255, green=155, blue=0 )
        else:
            color = self.adapter.background_color(node, depth)
            if not color:
                red = (depth * 10)%255
                green = 255-((depth * 5)%255)
                blue = (depth * 25)%255
                color = wx.Color( red, green, blue )
        return wx.Brush( color  )
    
    def PenForNode( self, node, depth=0 ):
        """Determine the pen to use to display the given node"""
        if node == self.selectedNode:
            return self.SELECTED_PEN
        return self.DEFAULT_PEN

    def TextForegroundForNode(self, node, depth=0):
        """Determine the text foreground color to use to display the label of
           the given node"""
        if node == self.selectedNode:
            fg_color = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHTTEXT)
        else:
            fg_color = self.adapter.foreground_color(node, depth)
            if not fg_color:
                fg_color = wx.SystemSettings_GetColour(wx.SYS_COLOUR_WINDOWTEXT)
        return fg_color
    def drawOneBox(self, node, highlight=True):
        dc = wx.BufferedDC(wx.ClientDC(self), self._buffer)        
        dc.BeginDrawing()
        brush = wx.Brush( self.BackgroundColor  )
        #dc.SetBackground( brush )
        dc.SetBrush( wx.Brush(wx.RED, wx.TRANSPARENT) )
        x,y,w, h = node.rect
        
        if highlight:
            dc.SetPen(self.SELECTED_PEN)
        else:
            dc.SetPen(wx.Pen( wx.GREEN, 2, wx.SOLID ))
            dc.DrawRoundedRectangle( x,y,w,h, self.padding*3 )        
            dc.SetPen(self.DEFAULT_PEN)
        dc.SetFont(self.FontForLabels(dc))
        dc.DrawRoundedRectangle( x,y,w,h, self.padding*3 )        
        dc.EndDrawing()
        
    def DrawBox( self, dc, node, x,y,w,h, hot_map, depth=0 ):
        """Draw a model-node's box and all children nodes"""
        log.debug( 'Draw: %s to (%s,%s,%s,%s) depth %s',
            node.path, x,y,w,h, depth,
        )
        if depth > 4: return
        
        if self.max_depth and depth > self.max_depth:
            return
        self.max_depth_seen = max( (self.max_depth_seen,depth))
        dc.SetBrush( self.BrushForNode( node, depth ) )
        dc.SetPen( self.PenForNode( node, depth ) )
        if sys.platform == 'darwin':
            # Macs don't like drawing small rounded rects...
            if w < self.padding*2 or h < self.padding*2:
                dc.DrawRectangle( x,y,w,h)
            else:
                dc.DrawRoundedRectangle( x,y,w,h, self.padding )
        else:
            dc.DrawRoundedRectangle( x,y,w,h, self.padding*3 )
#        self.DrawIconAndLabel(dc, node, x, y, w, h, depth)
        #if self.reCalculate:
        #    hot_map.append( (wx.Rect( int(x),int(y),int(w),int(h)), node, [] ) )
        x += self.padding
        y += self.padding
        w -= self.padding*2
        h -= self.padding*2
        
        empty = self.adapter.empty( node )
        icon_drawn = False
        if self.max_depth and depth == self.max_depth:
            self.DrawIconAndLabel(dc, node, x, y, w, h, depth)
            icon_drawn = True
        elif empty:
            # is a fraction of the space which is empty...
            log.debug( '  empty space fraction: %s', empty )
            new_h = h * (1.0-empty)
            self.DrawIconAndLabel(dc, node, x, y, w, h-new_h, depth)
            icon_drawn = True
            y += (h-new_h)
            h = new_h
            
        if w >self.padding*2 and h> self.padding*2:
            children = self.adapter.children( node )
            if len(children) > w*h: return
            if children:
                log.debug( '  children: %d', len(children) )
                #self.LayoutChildren( dc, children, node, x,y,w,h, hot_map[-1][2], depth+1 )
                self.drawChild(dc,node,depth+1)
            else:
                log.debug( '  no children' )
                if not icon_drawn:
                    self.DrawIconAndLabel(dc, node, x, y, w, h, depth)
        else:
            log.debug( '  not enough space: children skipped' )
                
    def DrawIconAndLabel(self, dc, node, x, y, w, h, depth):
        ''' Draw the icon, if any, and the label, if any, of the node. '''
        dc.SetClippingRegion(x+1, y+1, w-2, h-2) # Don't draw outside the box
        icon = self.adapter.icon(node, node==self.selectedNode)
        if icon and h >= icon.GetHeight() and w >= icon.GetWidth():
            iconWidth = icon.GetWidth() + 2
            dc.DrawIcon(icon, x+2, y+2) 
        else:
            iconWidth = 0
        if self.labels and h >= dc.GetTextExtent('ABC')[1]:
            dc.SetTextForeground(self.TextForegroundForNode(node, depth))
            dc.DrawText(self.adapter.label(node), x + iconWidth + 2, y+2)
        dc.DestroyClippingRegion()
        
    def calculation(self, childs,parent,  x,y,w,h):
       parent.rect = ( x,(y),(w),(h))
       if (w*h < 10):         return
       if len(childs) > w*h:  return
       self.calc0(parent.children, x,y,w,h)
    def calc0(self, childs, x,y,w,h,depth=0):
       log.debug('calc %d %s %d,%d %dx%d' ,len(childs),x,y,w,h)      
       if depth > 4:
        return
       nodes = [ (self.adapter.value(n),n) for n in childs ]
       nodes.sort()
       total = self.adapter.children_sum( childs )
       
       if total:
           (firstSize,firstNode) = nodes[-1]
           rest = [node for (size,node) in nodes[:-1]]
           fraction = firstSize/float(total)
           if w >= h:
               new_w = (w*fraction)
               if new_w:
                   firstNode.rect = ( (x),(y),(new_w),(h))
                   self.calc0(firstNode.children, x,y, new_w, h, depth+1 )
               else:
                   return # no other node will show up as non-0 either
               w = w-new_w
               x += new_w 
           else:
               new_h = (h*fraction)
               if new_h:
                   firstNode.rect = ( (x),(y),(w),(new_h))            
                   self.calc0( firstNode.children, x,y, w, new_h , depth+1)
               else:
                   return # no other node will show up as non-0 either
               h = h-new_h
               y += new_h 
           if rest and (h > self.padding*2) and (w > self.padding*2):
               self.calc0( rest, x,y,w,h )
       

    def drawChild(self, dc, node,depth):
        log.debug( 'dchilds %s %d %d',node.path,len(node.children),depth)
        for child in node.children:
            x,y,w,h = child.rect;
            self.DrawBox(dc,child, x,y,w,h,[],depth)

class DefaultAdapter( object ):
    """Default adapter class for adapting node-trees to SquareMap API"""
    def children( self, node ):
        """Retrieve the set of nodes which are children of this node"""
        return node.children
    def value( self, node, parent=None ):
        """Return value used to compare size of this node"""
        return node.size
    def label( self, node ):
        """Return textual description of this node"""
        return node.path
    def overall( self, node ):
        """Calculate overall size of the node including children and empty space"""
        return sum( [self.value(value,node) for value in self.children(node)] )
    def children_sum( self, children ):
        """Calculate children's total sum"""
        return sum( [n.size for n in children] )
    def empty( self, node ):
        """Calculate empty space as a fraction of total space"""
        overall = self.overall( node )
        if overall:
            return (overall - self.children_sum( self.children(node)))/float(overall)
        return 0
    def background_color(self, node, depth):
        ''' The color to use as background color of the node. '''
        return None
    def foreground_color(self, node, depth):
        ''' The color to use for the label. '''
        return None
    def icon(self, node, isSelected):
        ''' The icon to display in the node. '''
        return None
    def parents( self, node ):
        """Retrieve/calculate the set of parents for the given node"""
        return []


class TestApp(wx.App):
    """Basic application for holding the viewing Frame"""
    def OnInit(self):
        """Initialise the application"""
        wx.InitAllImageHandlers()
        self.frame = frame = wx.Frame( None, size=(800,600)
        )
        frame.CreateStatusBar()
        
        model = self.get_model2( sys.argv[1]) 
        #model = self.get_model2( '') 
        self.sq = SquareMap( frame, model=model)
        EVT_SQUARE_HIGHLIGHTED( self.sq, self.OnSquareSelected )
        EVT_SQUARE_SELECTED( self.sq, self.OnSquareSelected )
        frame.Show(True)
        self.SetTopWindow(frame)
        return True
    def get_model2(self,path):
        prev = []
        for line in file(path):
            f= line.split()
            depth = int(f[0])
            n = Node(' '.join(f[1:-1]),int(f[-1]),[])
            if depth >= len(prev):
                prev.append(n)
            if depth == 0:
                continue
            prev[depth-1].children.append(n)
            if prev[depth]!=n:
                prev[depth] = n  
        return prev[0]
    def get_model( self, path ):
        nodes = []
        for f in os.listdir( path ):
            full = os.path.join( path,f )
            if not os.path.islink( full ):
                if os.path.isfile( full ):
                    nodes.append( Node( full, os.stat( full ).st_size, () ) )
                elif os.path.isdir( full ):
                    nodes.append( self.get_model( full ))
        head,tail = os.path.split(path)                    
        return Node( path, sum([x.size for x in nodes]), nodes )
    def OnSquareSelected( self, event ):
        text = self.sq.adapter.label( event.node )
        self.frame.SetStatusText( text )

class Node( object ):
    """Really dumb file-system node object"""
    def __init__( self, path, size, children ):
        self.path = path
        self.size = size
        self.children = children 
        self.rect = wx.Rect( 0,0,0,0)
    def __repr__( self ):
        return '%r %r %r %r '%(self.rect, self.path, self.size, len(self.children) )
        

usage = 'squaremap.py somedirectory'
        
def main():
    """Mainloop for the application"""
    if not sys.argv[1:]:
        print usage
    else:
        app = TestApp(0)
        app.MainLoop()

if __name__ == "__main__":
    main()
