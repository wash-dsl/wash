
.. _program_listing_file_docs_conf.py:

Program Listing for File conf.py
================================

|exhale_lsh| :ref:`Return to documentation for file <file_docs_conf.py>` (``docs/conf.py``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: py

   from cgitb import html
   import subprocess, os
   
   extensions = ["breathe", 'sphinx.ext.todo', 'exhale']
   
   html_theme = "sphinx_rtd_theme"
   
   
   breathe_projects = {
       "my_project": ".."
   }
   # Breathe configuration
   breathe_default_project = "my_project"
   # Setup the exhale extension
   exhale_args = {
       # These arguments are required
       "containmentFolder":     "./api",
       "rootFileName":          "library_root.rst",
       "doxygenStripFromPath":  "..",
       # Heavily encouraged optional argument (see docs)
       "rootFileTitle":         "Library API",
       # Suggested optional arguments
       "createTreeView":        True,
       # TIP: if using the sphinx-bootstrap-theme, you need
       # "treeViewIsBootstrap": True,
       "exhaleExecutesDoxygen": True,
       "exhaleDoxygenStdin":    "INPUT = .."
   }
   
   # Tell sphinx what the primary language being documented is.
   primary_domain = 'cpp'
   
   # Tell sphinx what the pygments highlight language should be.
   highlight_language = 'cpp'
   
   read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'
   
   if read_the_docs_build:
      print("hi")
      subprocess.call('cd ../doxygen; doxygen', shell=True)
