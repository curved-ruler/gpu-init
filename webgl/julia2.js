
var get_context = function (canvas_id)
{
    var canvas = document.getElementById(canvas_id);
    var glc;
    if (!canvas)
    {
        console.error('No <canvas> with id: ' + canvas_id);
        return null;
    }
    try
    {
        glc = canvas.getContext('webgl');
    }
    catch (e)
    {
        console.error('Error creating WebGL Context!: ' + e.toString());
    }
    if (!glc)
    {
        console.error('NO GL');
    }
    
    return glc;
};

var get_context_2 = function (canvas_id)
{
    var canvas = document.getElementById(canvas_id);
    var glc2;
    if (!canvas)
    {
        console.error('No <canvas> with id: ' + canvas_id);
        return null;
    }
    try
    {
        glc2 = canvas.getContext('webgl2');
    }
    catch (e)
    {
        console.error('Error creating WebGL 2.0 Context!: ' + e.toString());
    }
    if (!glc2)
    {
        console.error('NO GL');
    }
    
    return glc2;
};

var compile_shader = function (gl, str, type)
{ 
    var shader = gl.createShader(type);
    
    gl.shaderSource(shader, str);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS))
    {
        console.error('Error while compiling shader: ' + gl.getShaderInfoLog(shader));
        window.alert('Error while compiling shader:\n' + gl.getShaderInfoLog(shader));
        return null;
    }
    return shader;
};

var create_vf_program = function (gl, vs_str, fs_str)
{
    var vfp = { glprog : gl.createProgram() };
    
    vfp.vs = compile_shader(gl, vs_str, gl.VERTEX_SHADER);
    gl.attachShader(vfp.glprog, vfp.vs);
            
    vfp.fs = compile_shader(gl, fs_str, gl.FRAGMENT_SHADER);
    gl.attachShader(vfp.glprog, vfp.fs);
    
    gl.linkProgram(vfp.glprog);
    
    if (!gl.getProgramParameter(vfp.glprog, gl.LINK_STATUS))
    {
        console.error('Error while linking program: ' + getProgramInfoLog());
        window.alert('Error while linking program:\n' + getProgramInfoLog());
    }
    
    return vfp;
};

var glprog = {
    get_context       : get_context,
    get_context_2     : get_context_2,
    create_vf_program : create_vf_program
};

var vs = `\
#version 300 es

layout(location = 0) in vec2  pos;

void main ()
{
    gl_Position = vec4(pos, 0.0, 1.0);
}
`;

var fs = `\
#version 300 es
precision highp float;

uniform vec4 tr;
uniform vec2 mouse;
out vec4 color;

// HSV to RGB by Inigo Quilez https://www.shadertoy.com/view/lsS3Wc
vec3 hsv2rgb (in vec3 c)
{
    vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0);
    return c.z * mix(vec3(1.0), rgb, c.y);
}
// Mandelbrot
float mand(in float x, in float y)
{
    vec2 z = vec2(0.0);
    int i = 0;
    int n = 100;
    for (i=0 ; i<n ; ++i)
    {
        z = vec2(z.x*z.x - z.y*z.y + x, 2.0*z.x*z.y + y);
        if (length(z) > 2.0) break;
    }
    return (float(i)/float(n));
}
// Julia
vec3 col(in float x, in float y)
{
    vec2 z = vec2(x,y);
    vec2 c = vec2(mouse.x, mouse.y);
    int i = 0;
    int n = 500;
    for (i=0 ; i<n ; ++i)
    {
        z = vec2(z.x*z.x - z.y*z.y + c.x, 2.0*z.x*z.y + c.y);
        if (length(z) > 2.0) break;
    }
    float t = fract(float(i) / float(n) * 2.0);
    return hsv2rgb(vec3(0.1, 0.9, t)) + vec3(0.1*mand(x,y));
}

void main()
{
    float x = tr.x * gl_FragCoord.x + tr.y;
    float y = tr.z * gl_FragCoord.y + tr.w;
    color = vec4(col(x, y), 1.0);
}
`;


//***************************************************
//***************************************************
//***************************************************

var gl     = null;
var shader = null;
var canvas = null;
var cwidth, cheight;

var pos       = { x:0.0,  y:0.0 };
var mouse_pos = { x:0.0,  y:0.0 };
var mouse_dom = { x:null, y:null };
var tr = [1,0,1,0];
var scale = 2.5;
var grabbed = 0;
var mouse_param = true;
var screen_quad_buffer = null;
var start_func = "";
var menu_hidden = false;



var make_quad = function ()
{
    var screen_quad = [
        -1.0,  1.0,
        -1.0, -1.0,
         1.0, -1.0,
        
        -1.0,  1.0,
         1.0, -1.0,
         1.0,  1.0
    ];
    
    screen_quad_buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, screen_quad_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(screen_quad), gl.STATIC_DRAW);
};

var draw = function ()
{
    if (!gl) return;
    if (!shader) return;
    gl.useProgram(shader.glprog);
    
    gl.disable(gl.BLEND);
    gl.disable(gl.DEPTH_TEST);
    
    tr = [scale/cheight, -(cwidth*scale)/(cheight*2.0)-pos.x, scale/cheight, -scale/2.0+pos.y];
    //console.log("TR", tr);
    gl.uniform4fv(shader.tr, tr);
    gl.uniform2f(shader.mouse, mouse_pos.x, mouse_pos.y);
    
    gl.bindBuffer(gl.ARRAY_BUFFER, screen_quad_buffer);
    gl.vertexAttribPointer(shader.pos, 2, gl.FLOAT, false, 0*4, 0*4);
    gl.drawArrays(gl.TRIANGLES, 0, 6);
};

var zoomin  = function () { scale *= 0.8; };
var zoomout = function () { scale *= 1.25;  };
var handle_wheel = function (event)
{
    if (event.deltaY < 0) zoomin();
    else                  zoomout();
    
    draw();
};

var handle_mouse_down = function (event)
{
    grabbed = 1;
};
var handle_mouse_up = function (event)
{
    grabbed = 0;
};
var handle_mouse_move = function (event)
{
    if (grabbed === 1)
    {
        var a = scale/cheight;
        pos.x += event.movementX * a;
        pos.y += event.movementY * a;
        draw();
    }
    else if (mouse_param)
    {
        mouse_pos.x = tr[0] * event.clientX + tr[1];
        mouse_pos.y = tr[2] * (cheight-event.clientY) + tr[3];
        draw();
    }
};
var handle_key_down = function (event)
{
    if (event.key === "q" || event.key === "Q"/* && event.ctrlKey */)
    {
        mouse_param = !mouse_param;
    }
};

var resize = function ()
{
    if (!canvas || !gl) return;
    
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    cwidth  = canvas.width;
    cheight = canvas.height;
    gl.viewport(0, 0, canvas.width, canvas.height);
};

var create_shader = function ()
{
    shader = glprog.create_vf_program(gl, vs, fs);
        
    shader.pos = gl.getAttribLocation(shader.glprog, "pos");
    gl.enableVertexAttribArray(shader.pos);
    
    shader.tr    = gl.getUniformLocation(shader.glprog, "tr");
    shader.mouse = gl.getUniformLocation(shader.glprog, "mouse");
};

var init = function ()
{
    document.removeEventListener("DOMContentLoaded", init);
    
    canvas = document.getElementById('canvas');
    gl     = glprog.get_context_2('canvas');
    canvas.addEventListener("mousemove", handle_mouse_move);
    canvas.addEventListener("mousedown", handle_mouse_down);
    canvas.addEventListener("mouseup",   handle_mouse_up);
    canvas.addEventListener("wheel",     handle_wheel);
    
    resize();
    make_quad();
    create_shader();
    draw();
    
    window.alert("Usage: \n\nMouse move - set parameter\nMouse drag - pan\nMouse wheel - zoom\n\nQ key - freeze parameter");
};


document.addEventListener("DOMContentLoaded", init);
document.addEventListener("keydown",   handle_key_down);
window.addEventListener("resize", function() { resize(); draw(); });

