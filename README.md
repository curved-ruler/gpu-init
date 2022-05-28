# gpu-init

Simple starter files for GPU projects

**! WIP !**

Note that WebGPU standard is not ready,  
quick status check: https://github.com/gpuweb/gpuweb/milestones  
browser support: https://github.com/gpuweb/gpuweb/wiki/Implementation-Status  

For OpenGL context loading, glad is used: https://github.com/Dav1dde/glad__
Desktop multiplatform windowing: [GLFW](https://www.glfw.org/) or [SDL2](https://www.libsdl.org/)__

The apps display the Mandelbrot and Julia sets.
At least, opengl-glfw and webgl, these are more or less ready.
webgpu is displaying a single triangle right now, not resizable.
opencl contains two command-line apps - device querying and a simple parallel vector add.

Julia controls:  
Windows are resizable, and F11 - toggle fullscreen  
Mouse move  - set Julia parameter  
Mouse drag  - pan  
Mouse wheel - zoom  
Q key - freeze parameter  

(no smartphone support atm)
