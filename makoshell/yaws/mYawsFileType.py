#   Programmer:     limodou
#   E-mail:         limodou@gmail.com
#
#   Copyleft 2006 limodou
#
#   Distributed under the terms of the GPL (GNU Public License)
#
#   UliPad is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#   $Id$

import wx
from modules import Mixin
from mixins import FiletypeBase

class YawsFiletype(FiletypeBase.FiletypeBase):

    __mixinname__ = 'Yawsfiletype'
    menulist = [ (None,
        [ #how to make sure id number is not used
            (900, 'IDM_Yaws', 'Yaws', wx.ITEM_NORMAL, None, ''),
        ]),
    ]
    toollist = []               #your should not use supperclass's var
    toolbaritems= {}

def add_filetypes(filetypes):
    filetypes.extend([('Yaws', YawsFiletype)])
Mixin.setPlugin('changefiletype', 'add_filetypes', add_filetypes)

def add_html_menu(menulist):
    menulist.extend([('IDM_Yaws', #parent menu id
            [ #how to make sure id number is not used
                (200, 'IDM_Yaws_TEST', tr('This is a test menu'), wx.ITEM_NORMAL, 'OnYawsTest', tr('This is a test.')),
            ]),
    ])
Mixin.setPlugin('Yawsfiletype', 'add_menu', add_html_menu)

def OnYawsTest(win, event):
    print 'Yaws menu test'
Mixin.setMixin('mainframe', 'OnYawsTest', OnYawsTest)

class TclFiletype(FiletypeBase.FiletypeBase):

    __mixinname__ = 'Tclfiletype'
    menulist = [ (None,
        [ #how to make sure id number is not used
            (901, 'IDM_TCL', 'tcl', wx.ITEM_NORMAL, None, ''),
        ]),
    ]
    toollist = []               #your should not use supperclass's var
    toolbaritems= {}

def add_filetypes_tcl(filetypes):
    filetypes.extend([('tcl', TclFiletype)])
Mixin.setPlugin('changefiletype', 'add_filetypes', add_filetypes_tcl)

def add_html_menu(menulist):
    menulist.extend([('IDM_Tcl', #parent menu id
            [ #how to make sure id number is not used
                (201, 'IDM_TCL_TEST', tr('This is a test menu'), wx.ITEM_NORMAL, 'OnTclTest', tr('This is a test.')),
            ]),
    ])
Mixin.setPlugin('Yawsfiletype', 'add_menu', add_html_menu)

def OnTclTest(win, event):
    print 'tcl menu test'
Mixin.setMixin('mainframe', 'OnTclTest', OnTclTest)

def OnAddCurrentPath(self, event):
    item = self.tree.GetSelection()
    if not self.is_ok(item): return
    filename = self.get_node_filename(item)
    if self.isFile(item):
        item = self.tree.GetItemParent(item)
        filename = self.get_node_filename(item)
    path = filename
    if path:
        self.addpath(path)
        if self.pref.open_project_setting_dlg:
            wx.CallAfter(self.OnSetProject)

def addmenu(app, name, othermenu):
    app.popmenulist.extend([('IDPM_ADD', [
      (99, 'IDPM_ADD_CURDIR', tr('Add Current Directory'), wx.ITEM_NORMAL, 
         'OnAddCurrentPath', ''),   
    ]), ])
    

Mixin.setPlugin('dirbrowser', 'other_popup_menu', addmenu)