from cgitb import html
import subprocess, os

extensions = ["breathe", 'sphinx.ext.todo']

html_theme = "sphinx_rtd_theme"


breathe_projects = {"myproject": "../"}
# Breathe configuration
breathe_default_project = "my_project"


read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

if read_the_docs_build:
   print("hi")
   subprocess.call('cd ../doxygen; doxygen', shell=True)