# winfiol clone
# test under ulipad scripts
import sys
import traceback
import wx
from mixins.Editor import TextEditor
from modules import Globals
import modules.meide as ui
from modules.wxctrl import FlatButtons
from modules import common
import wx.lib.dialogs
from modules import Mixin
from mako.template import Template
from mako.runtime import Context
from StringIO import StringIO
import wx.py.interpreter as Interpreter
        
class MyCanvas(wx.Window):
    def __init__(self, parent, codes):
        wx.Window.__init__(self, parent, -1, style=wx.SUNKEN_BORDER )
        self.parent = parent
        
        self.codes = codes
        self.SetBackgroundColour(wx.WHITE)
        
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.error = False
        
    def OnPaint(self, event):
        if not self.error and self.codes.get('OnPaint', None):
            dc = wx.PaintDC(self)
            dc.DrawText(self.codes['OnPaint'],7,7)
        else:
            dc = wx.PaintDC(self)
            dc.Clear()
            event.Skip
        
    def refresh(self):
        self.error = False
        self.Refresh(True)
        self.Update()

import logging
class MyCode(wx.Panel):
    # disable class browser
    RightIsVisible = False
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        self.parent = parent
        
        self.sizer = sizer = ui.VBox(padding=0, namebinding='widget').create(self).auto_layout()
        self.result = TextEditor(self, None, 'ouput test', 'titor', True)
        sizer.add(self.result, proportion=1, flag=wx.EXPAND|wx.ALL, border=3)
        
        box = sizer.add(ui.HBox(padding=2))
        self.btnRefresh = FlatButtons.FlatBitmapButton(self, -1, 
            common.getpngimage('images/classbrowserrefresh.gif'))
        box.add(self.btnRefresh)
        sizer.auto_fit(0)

        self.chat = TextEditor(self, None, '','')
        sizer.add(self.chat, proportion=1, flag=wx.EXPAND|wx.ALL, border=3)
        self.SetAutoLayout(True)

        if wx.Platform == '__WXMSW__':
            face1 = 'Arial'
            face2 = 'Times New Roman'
            face3 = 'Courier New'
            pb = 10
        else:
            face1 = 'Helvetica'
            face2 = 'Times'
            face3 = 'Courier'
            pb = 12

       
class MyFrame(wx.Frame):
    def __init__(self, parent):
        self.codes = {}

        wx.Frame.__init__(self, parent, -1, title=tr('Canvas Test'))
        
        self.splitter = wx.SplitterWindow(self)
        self.canvas = MyCanvas(self.splitter, self.codes)
        
        self.code_panel = MyCode(self.splitter)
        self.chat = self.code_panel.chat
        self.result = self.code_panel.result
        self.btnRefresh = self.code_panel.btnRefresh
        lexer = Globals.mainframe.lexers.getNamedLexer('python')
        if lexer:
            lexer.colourize(self.result)
        lexer2 = Globals.mainframe.lexers.getNamedLexer('makotmp')
        if lexer2:
            lexer2.colourize(self.chat)
        
        self.splitter.SetMinimumPaneSize(10)
#        self.splitter.SplitHorizontally(self.top, self.code_panel)
        self.splitter.SplitHorizontally(self.canvas, self.code_panel)
        self.output = ''
        self.interp = wx.py.interpreter.Interpreter(stdout=self,stderr=self)
        self.context = Context(self)
        self.lineBuf = ''
        wx.EVT_BUTTON(self.btnRefresh, self.btnRefresh.GetId(), self.OnRefresh)
        wx.EVT_KEY_DOWN(self.chat, self.OnKeyDown)        
        wx.CallAfter(self.chat.SetFocus)
        
        #tn = telnet_2_node('eon01','administrator','Administrator1@','>')
        #self.apgtn = APGTelnet(tn)
        #self.apgtn.mml_cmd('mml -a')
    def write(self, text):
        """Capture output from self.i.push()."""
        self.lineBuf += text
        self.result.AddText(':'+text)
        if text[-1] == '\n':
            self.result.AddText('>'+self.lineBuf)
            self.lineBuf = ''
    def OnRefresh(self, event):
        self.codes['OnPaint'] = self.chat.GetText()
        
        self.canvas.refresh()
        #r = self.apgtn.mml_cmd('dpwsp;')
        #self.code_panel.result.AddText(r)
    def OnSend(self, event=None):
        # if has selection: send selection
        # TODO, match to the begin and end of whole line
        # always keep last character of file lineend
        message = self.chat.GetSelectedText()
        # else get curLine
        if message == "":
            message,pos = self.chat.GetCurLine()
#            if message[-1]=='\n':
#                message = message[:-1]
        if message:
            #logging.info('Compiling and evaluating:\n%s' % message)
            self.put_message('me',message)
            #self.interp.push(message)
            

    def put_message(self, username, message):
        mytemplate = Template(message)
        mytemplate.render_context(self.context)
#        for line in message.splitlines():
#            self.result.AddText(line)
#            self.result.AddText('\n')
        #self.result.AddText(r+'\n')
        #TODO if two many outputs, focus should follow mouse/scroll
        # not the last line
        self.result.EnsureCaretVisible()

    def OnKeyDown(self, event):
        key = event.GetKeyCode()
        shift = event.ShiftDown()
        alt = event.AltDown()
        ctrl = event.ControlDown()
        if key == wx.WXK_RETURN and shift and not alt and not ctrl:
            self.OnSend()
            # re organize the lines, not just send the return
            self.chat.EnsureCaretVisible()
            #if not self.chat.GetEndAtLastLine(): self.chat.AddText('\n')
            #TODO if it is in the middle of line don't add return
            self.chat.AddText('\n')
        elif key == wx.WXK_RETURN and not shift and not alt and not ctrl:
            self.chat.AddText('\n')
        elif key == wx.WXK_F4:
            self.OnSend()
        else:
            event.Skip()

        
def run(win):
    import wx
    w = MyFrame(win)
    w.SetSize((400,600))
    w.Show()
    w.splitter.SetSashPosition(10,True)
    #w.chat.AddText('abc\ndef\nghi')
    t = '''<%def f(): return 'deff'%><%
        a = 3
        b = 4
    %>
${f()}${a+b}${a}ccc,
    '''
    w.chat.AddText(t)

    #w.result.AddText(code)
    #w.result.AddText(str(module))
    # w has to implement writer(self,text) function

run(win)
