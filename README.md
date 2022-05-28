# gpu-init

Simple starter files for GPU projects

**! WIP !**

Note that WebGPU standard is not ready,<br/>
quick status check: https://github.com/gpuweb/gpuweb/milestones <br/>
browser support: https://github.com/gpuweb/gpuweb/wiki/Implementation-Status <br/>

For OpenGL context loading, glad is used: https://github.com/Dav1dde/glad <br/>
Desktop multiplatform windowing: [GLFW](https://www.glfw.org/) or [SDL2](https://www.libsdl.org/) <br/>

The apps display the Mandelbrot and Julia sets.
At least, opengl-glfw and webgl, these are more or less ready.
webgpu is displaying a single triangle right now, not resizable.
opencl contains two command-line apps - device querying and a simple parallel vector add.

Julia controls:<br/>
Windows are resizable, and F11 - toggle fullscreen<br/>
Mouse move  - set Julia parameter<br/>
Mouse drag  - pan<br/>
Mouse wheel - zoom<br/>
Q key - freeze parameter<br/>

(no smartphone or controller support atm)
