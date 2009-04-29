import wx
from mixins.Editor import TextEditor
from mixins.LexerBase import LexerBase
from mixins.NCustomLexer import *
    
class YawsLexer2(CustomLexer):

    metaname = 'erlang'
    casesensitive = True

    keywords = ("import module export compile include case receive after end of spawn")

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
        self.addSyntaxItem('p_default',    'Default',              wx.stc.STC_ERLANG_DEFAULT,       self.STC_STYLE_TEXT)
        self.addSyntaxItem('commentline',  'Comment line',         wx.stc.STC_ERLANG_COMMENT,       self.STC_STYLE_COMMENT)
        self.addSyntaxItem('number',       'Number',               wx.stc.STC_ERLANG_NUMBER,        self.STC_STYLE_NUMBER)
        self.addSyntaxItem('string',       'String',               wx.stc.STC_ERLANG_STRING,        self.STC_STYLE_STRING)
        self.addSyntaxItem('character',    'Character',            wx.stc.STC_ERLANG_CHARACTER,     self.STC_STYLE_CHARACTER)
        self.addSyntaxItem('keyword',      'Keyword',              wx.stc.STC_ERLANG_VARIABLE,      self.STC_STYLE_KEYWORD1)
        self.addSyntaxItem('triple',       'Triple quotes',        wx.stc.STC_ERLANG_KEYWORD,       self.STC_STYLE_KEYWORD2)
        self.addSyntaxItem('tripledouble', 'Triple double quotes', wx.stc.STC_ERLANG_ATOM,          self.STC_STYLE_IDENTIFIER)
        self.addSyntaxItem('classname',    'Class definition',     wx.stc.STC_ERLANG_RECORD,        self.STC_STYLE_TAG)
        self.addSyntaxItem('defname',      'Function or method',   wx.stc.STC_ERLANG_MACRO,         self.STC_STYLE_DEFNAME)
        self.addSyntaxItem('operator',     'Operators',            wx.stc.STC_ERLANG_OPERATOR,      self.STC_STYLE_OPERATOR)
        self.addSyntaxItem('identifier',   'Identifiers',          wx.stc.STC_ERLANG_NODE_NAME,     self.STC_STYLE_CLASSNAME)
        self.addSyntaxItem('commentblock', 'Comment blocks',       wx.stc.STC_ERLANG_SEPARATOR,     self.STC_STYLE_COMMENTBLOCK)
        self.addSyntaxItem('wwww',         'Comment blocks',       wx.stc.STC_ERLANG_FUNCTION_NAME, self.STC_STYLE_ATTRNAME)
        self.addSyntaxItem('stringeol',    'EOL unclosed string',  wx.stc.STC_ERLANG_UNKNOWN,       self.STC_STYLE_STRINGEOL)
    def styleneeded(self, win, pos):
        print pos
        super(YawsLexer2,self).styleneeded(win,pos)
        

        
                                                                                                   
def run(win):
    
    f = wx.Frame(win)
    f.SetSize((600,500))
    ed = TextEditor(f,None, 'ouput test', 'titor', True)
    lex = YawsLexer2('erlang','PPP|*.pay;*.pya',wx.stc.STC_LEX_CONTAINER,'a.stx')
    lex.colourize(ed)
    ed.AddText('\n'+lex.preview_code)

    f.Show()


