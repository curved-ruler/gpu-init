# gpu-init

Simple starter files for GPU projects

## 

Note that WebGPU standard is not ready,<br/>
quick status check: https://github.com/gpuweb/gpuweb/milestones <br/>
browser support: https://github.com/gpuweb/gpuweb/wiki/Implementation-Status <br/>

For OpenGL context loading, glad is used: https://github.com/Dav1dde/glad <br/>
Desktop multiplatform windowing: [GLFW](https://www.glfw.org/) or [SDL2](https://www.libsdl.org/) <br/>

## 

The apps display the Mandelbrot and Julia sets. (except opencl, that is command line atm)<br/>
Controls:<br/>
Windows are resizable, and F11 - toggle fullscreen<br/>
Mouse move  - set Julia parameter<br/>
Mouse drag  - pan<br/>
Mouse wheel - zoom<br/>
Q key - freeze parameter<br/>

## Bugs/Todo

GLFW: multiple monitor support: https://github.com/glfw/glfw/issues/1699 <br/>
SDL: app poses a full load on the CPU, vsync or manual sleep didn't work properly.<br/>
WebGPU: currently only a triangle is shown, the Julia app does not work properly yet.<br/>
OpenCL: some more app is coming.<br/>
Vulkan: coming.<br/>
(no smartphone or controller support atm)
