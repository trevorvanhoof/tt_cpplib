# tt_cpplib
Compact C++ utilities for Windows.

I find myself building similar utilities every project I mess around with, so I decided to start cleaning and collecting utilities.

Disclaimer: 
I am not as versed in C++, and frankly usually care more about building stuff than learning esoteric details about a language I dislike. 
Much of this code will be inefficient, or not following good paradigms, use at your own risk.

## Modules

### tt_messages

Utilities for formatting strings, logging, warning and raising errors.

If you are not debugging, every message will be shown as a dialog, we do not use exceptions so a fatal error will likely crash directly after.
If you are debugging, every message will be sent to the output log, warnings and errors will break.

TODO: Port to std::string

### tt_customwindow

A Window class that receives messages in a slightly higher level format.
Subclass and implement the events. Note the CreateGLContext() utility function.

### OpenGL

#### tt_gl_h.h

Include this in any header that needs knowledge of OpenGL.
It avoids including Windows.h because it is an intrusive header.

#### tt_glext_win.h

Include in any source file that needs to call into OpenGL.
Beware that it includes Windows.h. It does use the VC_EXTRALEAN and WIN32_LEAN_AND_MEAN macros.

#### tt_gl_cpp.h

This auto-generated file defines all OpenGL functions in existance as macros that use wglGetProcAddress and a cast to get the function as-needed.

TODO: Convert to an Initialize function that loads the functions exactly once.

#### tt_globjects

Wraps OpenGL concepts in an object oriented way where the instances owmn their resources.
I work on this as I need it, so only some concepts are wrapped so far: Mesh (VAO), Image (texture2D), Program, Shader.
An additional high level concept include a Material, which holds one possible set of uniforms for a program.

#### tt_debugdraw_2d

Draw lines, rectangles, triangle fans, and on-screen text.
Shoutout to FiraCode: https://github.com/tonsky/FiraCode, a great font that I am not benefitting from here because I use a sprite font based on it.

#### tt_debugdraw_3d

A subclass of the 2D drawer that supports transforming 3D points into 2D before drawing them. 
The 3D math is currently not performed in the shader, but instead we depend on https://github.com/trevorvanhoof/MMath.

### tt_json

A single header Json reader & writer, with optional trailing comma support. You should probably be using rapidjson instead.
I wrote this for fun & to match the implementation of a C# parser I wrote: https://github.com/trevorvanhoof/MiniJsonParser
