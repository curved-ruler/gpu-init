
# gpu-init

Simple starter files for GPU projects

### Third-party

Note that WebGPU standard is not ready,<br/>
quick status check: https://github.com/gpuweb/gpuweb/milestones <br/>
browser support: https://github.com/gpuweb/gpuweb/wiki/Implementation-Status <br/>

For OpenGL context loading, glad is used: https://github.com/Dav1dde/glad <br/>
Desktop multiplatform windowing: [GLFW](https://www.glfw.org/) or [SDL2](https://www.libsdl.org/) <br/>

### Controls

The apps display the Mandelbrot and Julia sets.<br/>
OpenCL has two more small apps for command line device query and parallel vecadd<br/>
<br/>
Windows are resizable, and F11 - toggle fullscreen<br/>
Mouse move  - set Julia parameter<br/>
Mouse drag  - pan<br/>
Mouse wheel - zoom<br/>
Q key - freeze parameter<br/>
Ctrl-C and Ctrl-V: copies and pastes the Julia parameter. (Paste doesn't work for web apps, instead they show the parameter at all times)

### Bugs/Todo

GLFW: multiple monitor support: https://github.com/glfw/glfw/issues/1699 <br/>
SDL: app poses a full load on the CPU, vsync or manual sleep didn't work properly with X11<br/>
WebGPU: not working at all right now<br/>
OpenCL: resizing could crash the app<br/>
Vulkan: coming<br/>
(no smartphone or controller support atm)
