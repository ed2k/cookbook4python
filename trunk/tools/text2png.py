#!/usr/bin/env python
#coding=utf-8

"""
This class generate image file from text, it required PIL (python imaging library)
installed.

code sample :


"""

def getImage(value, width = 400, extension = "PNG"):
   """ Get an image with PIL library 
   value code barre value
   height height in pixel of the bar code
   extension image file extension"""
   import Image, ImageFont, ImageDraw
   from string import lower, upper
   
   
   
   height = 80
   # Create a new image
   position = 8
   im = Image.new("1",(width,height))
   
   # Load font
   font_width = 22
   font = ImageFont.truetype("SimHei.ttf", font_width)

   
   # Create drawer
   draw = ImageDraw.Draw(im)
   
   # Erase image
   draw.rectangle(((0,0),(im.size[0],im.size[1])),fill=256)
   
   # Draw 
   h = 0
   for line in value.splitlines():
     draw.text((0, h), line, font=font, fill=0)
     h += font_width
   
        
   # Save the result image
   im.save('t'+"."+lower(extension), upper(extension))

def test():
    getImage(u'test测试test\r你好、\r\n欲穷千里目，更上一层楼。\r\n')

if __name__ == "__main__":
   test()
