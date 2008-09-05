#!/usr/bin/env python
#
# Copyright 2007 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import cgi
import datetime
import wsgiref.handlers

from google.appengine.ext import db
from google.appengine.api import users
from google.appengine.ext import webapp

class Greeting(db.Model):
  author = db.UserProperty()
  content = db.StringProperty(multiline=True)
  date = db.DateTimeProperty(auto_now_add=True)


class MainPage(webapp.RequestHandler):
  def get(self):
    w = self.response.out.write
    w('<html>')
    w(''' <head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
    <title>Build from Script</title>
    
    <style type="text/css">
    /*margin and padding on body element
      can introduce errors in determining
      element position and are not recommended;
      we turn them off as a foundation for YUI
      CSS treatments. */
    body {
    	margin:0;
    	padding:0;
    }
    </style>
    
    <link rel="stylesheet" type="text/css" href="http://yui.yahooapis.com/2.5.2/build/fonts/fonts-min.css" />
    <link rel="stylesheet" type="text/css" href="http://yui.yahooapis.com/2.5.2/build/tabview/assets/skins/sam/tabview.css" />
    <script type="text/javascript" src="http://yui.yahooapis.com/2.5.2/build/yahoo-dom-event/yahoo-dom-event.js"></script>
    <script type="text/javascript" src="http://yui.yahooapis.com/2.5.2/build/element/element-beta-min.js"></script>
    
    <script type="text/javascript" src="http://yui.yahooapis.com/2.5.2/build/tabview/tabview-min.js"></script>
    
</head> ''')    
    w('<body  class=" yui-skin-sam">')

    greetings = db.GqlQuery("SELECT * "
                            "FROM Greeting "
                            "ORDER BY date DESC LIMIT 10")

    for greeting in greetings:
      if greeting.author:
        w('<b>%s</b> wrote:' % greeting.author.nickname())
      else:
        w('An anonymous person wrote:')
      w('<blockquote>%s</blockquote>' %
                              cgi.escape(greeting.content))
   
    w("""
          <form action="/sign" method="post">
            <div><textarea name="content" rows="3" cols="60"></textarea></div>
            <div><input type="submit" value="Sign Guestbook"></div>
          </form>
<div id="container"></div>
<script type="text/javascript">
(function() {
    var tabView = new YAHOO.widget.TabView();

    tabView.addTab( new YAHOO.widget.Tab({
        label: 'lorem',
        content: '<div id="container2> container 2</div>',
        active: true
    }));

    tabView.addTab( new YAHOO.widget.Tab({
        label: 'ipsum',
        content: '<ul><li><a href="#">Lorem ipsum dolor sit amet.</a></li><li><a href="#">Lorem ipsum dolor sit amet.</a></li><li><a href="#">Lorem ipsum dolor sit amet.</a></li><li><a href="#">Lorem ipsum dolor sit amet.</a></li></ul>'

    }));

    tabView.addTab( new YAHOO.widget.Tab({
        label: 'dolor',
        content: '<form action="#"><fieldset><legend>Lorem Ipsum</legend><label for="foo"> <input id="foo" name="foo"></label><input type="submit" value="submit"></fieldset></form>'

    }));
    YAHOO.log("The example has finished loading; as you interact with it, you'll see log messages appearing here.", "info", "example");

    tabView.appendTo('container');

    var tabView2 = new YAHOO.widget.TabView();
    
    tabView2.addTab( new YAHOO.widget.Tab({
        label: 'lorem',
        content: '<div id="container3></div>',
        active: true
    }));
    
    tabView2.addTab( new YAHOO.widget.Tab({
        label: 'ipsum',
        content: '<ul><li><a href="#">Lorem ipsum dolor sit amet.</a></li><li><a href="#">Lorem ipsum dolor sit amet.</a></li><li><a href="#">Lorem ipsum dolor sit amet.</a></li><li><a href="#">Lorem ipsum dolor sit amet.</a></li></ul>'
    
    }));
    
    tabView2.addTab( new YAHOO.widget.Tab({
        label: 'dolor',
        content: '<form action="#"><fieldset><legend>Lorem Ipsum</legend><label for="foo"> <input id="foo" name="foo"></label><input type="submit" value="submit"></fieldset></form>'
    
    }));
    
    tabView2.appendTo('container2');   
    function handleClick(e){
      alert(e.target.get('label'));
    } 
    for (var i = 0;i < 3;i++)
    tabView2.getTab(i).addListener('click', handleClick);
})();
</script>


        </body>        
      </html>""")


class Guestbook(webapp.RequestHandler):
  def post(self):
    greeting = Greeting()

    if users.get_current_user():
      greeting.author = users.get_current_user()

    greeting.content = self.request.get('content')
    greeting.put()
    self.redirect('/guestbook')


application = webapp.WSGIApplication([
  ('/guestbook', MainPage),
  ('/sign', Guestbook)
], debug=True)






def main():
  wsgiref.handlers.CGIHandler().run(application)


if __name__ == '__main__':
  main()
