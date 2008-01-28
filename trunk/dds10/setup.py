import sys, os
from distutils.core import setup, Extension

# prepare the extension for building
pydds_ext = Extension('pydds', 
                      sources=['pydds.c', 'dds.cpp']
                     )

setup (name = "pydds",
       version = "1.00",
       author = "Bo Haglund",
       maintainer = "Alex Martelli",
       maintainer_email = "aleaxit@gmail.com",
       description = "Double Dummy Solver bridge module for Python",

       ext_modules = [ pydds_ext ]
)
