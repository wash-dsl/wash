doxygen Doxyfile.in && sphinx-build -b html -Dbreathe_projects.my_project=doc_out/xml . doc_out/sphinx/ && cp -a doc_out/html/. ../docs
