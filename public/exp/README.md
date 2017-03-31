# Experimental

This folder is experimental. Do not depend on these files unless you're willing
to experience breaking changes.

# Design

There is both a C and C++ API defined here. The C API is the interface to the
internal PDFium code. The C++ API calls through the C API to provide a nice
C++ abstraction on top of the C API.

We use a C API as the basis to make it simpler to write bindings to other
languages in the future.
