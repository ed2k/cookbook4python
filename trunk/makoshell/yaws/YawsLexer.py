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
from mixins.LexerBase import LexerBase
    
class YawsLexer(LexerBase):

    metaname = 'Yaws'
    casesensitive = True

    keywords = ('import module export compile include case receive after end of spawn')

    preview_code = """-module(floater_proxy).

-compile(export_all).
%-export([test/0,start/3,listen/3,one_socket/1]).


test() -> 
start("142.133.118.126",10100).

start(Host,Port)->
    {ok, SockTarget} = gen_tcp:connect(Host,Port, [binary, {packet, 0}]),      
    Pid = spawn(?MODULE,one_socket,["flproxyA","flproxyB",SockTarget]),
    inet:setopts(SockTarget, [{active, true}]),
    gen_tcp:controlling_process(SockTarget, Pid),
    io:format("fl_proxy: target ~p handler ~p~n",[SockTarget,Pid]).

"""
    
#    comment_begin = '%'

    def pre_colourize(self, win):
        #FOLDING
        win.enablefolder = True
        win.SetProperty("fold", "1")


    def initSyntaxItems(self):
        self.addSyntaxItem('p_default',         'Default',              wx.stc.STC_ERLANG_DEFAULT,           self.STC_STYLE_TEXT)
        self.addSyntaxItem('commentline',       'Comment line',         wx.stc.STC_ERLANG_COMMENT,       self.STC_STYLE_COMMENT)
        self.addSyntaxItem('number',            'Number',               wx.stc.STC_ERLANG_NUMBER,            self.STC_STYLE_NUMBER)
        self.addSyntaxItem('string',            'String',               wx.stc.STC_ERLANG_STRING,            self.STC_STYLE_STRING)
        self.addSyntaxItem('character',         'Character',            wx.stc.STC_ERLANG_CHARACTER,         self.STC_STYLE_CHARACTER)
        self.addSyntaxItem('keyword',           'Keyword',              wx.stc.STC_ERLANG_VARIABLE,              self.STC_STYLE_KEYWORD1)
        self.addSyntaxItem('triple',            'Triple quotes',        wx.stc.STC_ERLANG_KEYWORD,            self.STC_STYLE_TRIPLE)
        self.addSyntaxItem('tripledouble',      'Triple double quotes', wx.stc.STC_ERLANG_ATOM,      self.STC_STYLE_TRIPLE)
        self.addSyntaxItem('classname',         'Class definition',     wx.stc.STC_ERLANG_RECORD,         self.STC_STYLE_CLASSNAME)
        self.addSyntaxItem('defname',           'Function or method',   wx.stc.STC_ERLANG_MACRO,           self.STC_STYLE_DEFNAME)
        self.addSyntaxItem('operator',          'Operators',            wx.stc.STC_ERLANG_OPERATOR,          self.STC_STYLE_OPERATOR)
        self.addSyntaxItem('identifier',        'Identifiers',          wx.stc.STC_ERLANG_NODE_NAME,        self.STC_STYLE_IDENTIFIER)
        self.addSyntaxItem('commentblock',      'Comment blocks',       wx.stc.STC_ERLANG_SEPARATOR,      self.STC_STYLE_COMMENTBLOCK)
        self.addSyntaxItem('wwww',              'Comment blocks',       wx.stc.STC_ERLANG_FUNCTION_NAME,      self.STC_STYLE_COMMENTBLOCK)
        self.addSyntaxItem('stringeol',         'EOL unclosed string',  wx.stc.STC_ERLANG_UNKNOWN,         self.STC_STYLE_STRINGEOL)


class TclLexer(LexerBase):
    metaname = 'tcl'

    keywords = ("foreach and def end in or self unless proc begin defined "
                "ensure module redo super until BEGIN break do false next rescue "
                "then when END case else for nil retry true while alias class "
                "elsif if not return undef yield",)

    preview_code = """# Hello World in tcl
puts "Hello World!"
proc a {b} {
 puts [aaa b c]
 set a { bb
 }
}
"""

    def pre_colourize(self, win):
        #FOLDING
        win.enablefolder = True
        win.SetProperty("fold", "1")
        win.SetProperty("tab.timmy.whinge.level", "1")

    def initSyntaxItems(self):
        self.addSyntaxItem('p_default',         'Default',              wx.stc.STC_TCL_DEFAULT,           self.STC_STYLE_TEXT)
        self.addSyntaxItem('commentline',       'Comment line',         wx.stc.STC_TCL_COMMENTLINE,       self.STC_STYLE_COMMENT)
        self.addSyntaxItem('number',            'Number',               wx.stc.STC_TCL_NUMBER,            self.STC_STYLE_NUMBER)
        self.addSyntaxItem('string',            'String',               wx.stc.STC_TCL_WORD2,            self.STC_STYLE_CHARACTER)
        self.addSyntaxItem('character',         'Character',            wx.stc.STC_TCL_WORD3,         self.STC_STYLE_CHARACTER)
        self.addSyntaxItem('keyword',           'Keyword',              wx.stc.STC_TCL_WORD,              self.STC_STYLE_KEYWORD1)
        self.addSyntaxItem('triple',            'Triple quotes',        wx.stc.STC_TCL_WORD4,            self.STC_STYLE_CHARACTER)
        self.addSyntaxItem('tripledouble',      'Triple double quotes', wx.stc.STC_TCL_WORD5,      self.STC_STYLE_CHARACTER)
        self.addSyntaxItem('classname',         'Class definition',     wx.stc.STC_TCL_WORD6,         self.STC_STYLE_CLASSNAME)
        self.addSyntaxItem('defname',           'Function or method',   wx.stc.STC_TCL_WORD7,           self.STC_STYLE_DEFNAME)
        self.addSyntaxItem('operator',          'Operators',            wx.stc.STC_TCL_OPERATOR,          self.STC_STYLE_OPERATOR)
        self.addSyntaxItem('identifier',        'Identifiers',          wx.stc.STC_TCL_IDENTIFIER,        self.STC_STYLE_IDENTIFIER)
        self.addSyntaxItem('commentblock',      'Comment blocks',       wx.stc.STC_TCL_COMMENT,      self.STC_STYLE_COMMENT)
        self.addSyntaxItem('stringeol',         'EOL unclosed string',  wx.stc.STC_TCL_WORD8,         self.STC_STYLE_STRINGEOL)
