import wx
class MyFrame(wx.Frame):
    def __init__(self, parent, id, title):
        wx.Frame.__init__(self, parent, id, title, wx.DefaultPosition, wx.Size(450, 350))

        hbox = wx.BoxSizer(wx.HORIZONTAL)
        vbox = wx.BoxSizer(wx.VERTICAL)
        panel1 = wx.Panel(self, -1)
        panel2 = wx.Panel(self, -1)

        self.tree = wx.TreeCtrl(panel1, 1, wx.DefaultPosition, (-1,-1), wx.TR_HIDE_ROOT|wx.TR_HAS_BUTTONS)
        root = self.tree.AddRoot('Programmer')
        os = self.tree.AppendItem(root, 'Operating Systems')
        pl = self.tree.AppendItem(root, 'Programming Languages')
        tk = self.tree.AppendItem(root, 'Toolkits')
        items = 'Linux FreeBSD OpenBSD NetBSD Solaris'.split()
        self.AddTreeNodes(os,items)
       
        i=[['Compiled languages','Java', 'C++','C','Pascal'],
        ['Scripting languages', 'Python','Ruby','Tcl','PHP']]
        i=['Qt','MFC','wxPython','GTK+','Swing']
        self.AddTreeNodes(pl,items)
        self.AddTreeNodes(tk,i)
        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.OnSelChanged, id=1)
        self.display = wx.StaticText(panel2, -1, '',(10,10), style=wx.ALIGN_CENTRE)
        vbox.Add(self.tree, 1, wx.EXPAND)
        hbox.Add(panel1, 1, wx.EXPAND)
        hbox.Add(panel2, 1, wx.EXPAND)
        panel1.SetSizer(vbox)
        self.SetSizer(hbox)
        self.Centre()

    def OnSelChanged(self, event):
        item =  event.GetItem()
        self.display.SetLabel(self.tree.GetItemText(item))
    def AddTreeNodes(self, parentItem, items):
         for item in items:
             if type(item) == str:
                 self.tree.AppendItem(parentItem, item)
             else:
                 newItem = self.tree.AppendItem(parentItem, item[0])
                 self.AddTreeNodes(newItem, item[0])


w = MyFrame(win, -1, 'treectrl.py')
w.Show(True)
