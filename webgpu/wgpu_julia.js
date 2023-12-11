
let canvas   = null;
let context  = null;
let device   = null;
let format   = null;
let pipeline = null;

let uniformBuffer = null;
let uniformBindGroup = null;
let renderPassDescriptor = null;
let commandEncoder = null;

//const pixr = window.devicePixelRatio || 1;

let csize     = [];
let pos       = { x:0.0,  y:0.0 };
let mouse_pos = { x:0.1,  y:0.1 };
let tr        = [1,0,1,0];
let scale     = 2.5;
let grabbed   = 0;
let mouse_param = true;
let params = null;


const vertex_wgsl = `\

@vertex //Firefox
//@stage(vertex) //Chromium
fn main(@builtin(vertex_index) vi : u32) -> @builtin(position) vec4<f32>
{
    var pos = array<vec2<f32>, 6>(
        vec2<f32>(-1.0,  1.0),
        vec2<f32>(-1.0, -1.0),
        vec2<f32>( 1.0, -1.0),
        
        vec2<f32>(-1.0,  1.0),
        vec2<f32>( 1.0, -1.0),
        vec2<f32>( 1.0,  1.0)
    );
    
    return vec4<f32>(pos[vi], 0.0, 1.0);
}
`;

const fragment_wgsl = `\

struct trUniforms
{
    tr1 : f32,
    tr2 : f32,
    tr3 : f32,
    tr4 : f32,
    mx  : f32,
    my  : f32
};
@binding(0) @group(0) var<uniform> uni : trUniforms;


fn mandel (xy : vec2<f32>) -> f32
{
    var z = vec2<f32>(0.0);
    var x = uni.tr1 * xy.x + uni.tr2;
    var y = uni.tr3 * xy.y + uni.tr4;
    var i = i32(0);
    var n = i32(100);
    for (i=0 ; i<n ; i=i+1)
    {
        z = vec2<f32>(z.x*z.x - z.y*z.y + x, 2.0*z.x*z.y + y);
        if (length(z) > 2.0) { break; }
    }
    return f32(i)/f32(n);
}

fn julia (xy : vec2<f32>) -> f32
{
    var z2 = vec2<f32>(xy.x, xy.y);
    var i2 = i32(0);
    var n2 = i32(200);
    for (i2=0 ; i2<n2 ; i2=i2+1)
    {
        z2 = vec2<f32>(z2.x*z2.x - z2.y*z2.y + uni.mx, 2.0*z2.x*z2.y + uni.my);
        if (length(z2) > 2.0) { break; }
    }
    return fract((f32(i2) / f32(n2)) * 2.0);
}

@fragment //Firefox
//@stage(fragment) //Chromium
fn main(@builtin(position) fragcoord : vec4<f32>) -> @location(0) vec4<f32>
{
    // Mandel
    var z = vec2<f32>(0.0);
    var x = uni.tr1 * fragcoord.x + uni.tr2;
    var y = uni.tr3 * fragcoord.y + uni.tr4;
    var i = i32(0);
    var n = i32(100);
    for (i=0 ; i<n ; i=i+1)
    {
        z = vec2<f32>(z.x*z.x - z.y*z.y + x, 2.0*z.x*z.y + y);
        if (length(z) > 2.0) { break; }
    }
    var m = f32(i)/f32(n);
    
    // Julia
    var z2 = vec2<f32>(x,y);
    var i2 = i32(0);
    var n2 = i32(200);
    for (i2=0 ; i2<n2 ; i2=i2+1)
    {
        z2 = vec2<f32>(z2.x*z2.x - z2.y*z2.y + uni.mx, 2.0*z2.x*z2.y + uni.my);
        if (length(z2) > 2.0) { break; }
    }
    var j = fract(f32(i2) / f32(n2) * 2.0);
    
    // Colour
    var c   = vec3<f32>(0.1, 0.9, j);
    var cx  = vec3<f32>(c.x*6.0) + vec3<f32>(0.0,4.0,2.0);
    cx.x = cx.x - 6.0 * floor(cx.x/6.0);
    cx.y = cx.y - 6.0 * floor(cx.y/6.0);
    cx.z = cx.z - 6.0 * floor(cx.z/6.0);
    var rgb = clamp(abs(cx - vec3<f32>(3.0))-vec3<f32>(1.0), vec3<f32>(0.0), vec3<f32>(1.0));
    var jc  = c.z * mix(vec3<f32>(1.0), rgb, c.y);
    return vec4<f32>(jc + 0.1*m, 1.0);
}
`;


let init = async function ()
{
    document.removeEventListener("DOMContentLoaded", init);
    
    if (!navigator.gpu) { alert('ERROR: WebGPU is not available'); return; }
    
    params = document.getElementById('params');
    canvas = document.getElementById('canvas');
    canvas.addEventListener("mousemove", handle_mouse_move);
    canvas.addEventListener("mousedown", handle_mouse_down);
    canvas.addEventListener("mouseup",   handle_mouse_up);
    canvas.addEventListener("wheel",     handle_wheel);

    const adapter = await navigator.gpu.requestAdapter();
    device        = await adapter.requestDevice();

    context = canvas.getContext('webgpu');

    format = navigator.gpu.getPreferredCanvasFormat(adapter);
    
    const bindGroupLayout = device.createBindGroupLayout({
        entries: [{
            binding: 0,
            visibility: GPUShaderStage.FRAGMENT,
            buffer: {},
        }]
    });

    const pipelineLayout = device.createPipelineLayout({
        bindGroupLayouts: [
            bindGroupLayout, // @group(0)
        ]
    });
    
    pipeline = device.createRenderPipeline({
        vertex: {
            module: device.createShaderModule({
                code: vertex_wgsl
            }),
            entryPoint: 'main'
        },
        fragment: {
            module: device.createShaderModule({
                code: fragment_wgsl
            }),
            entryPoint: 'main',
            targets: [{
                format: format,
            }]
        },
        primitive: {
            topology: 'triangle-list',
        },
        layout: pipelineLayout
    });
    
    uniformBuffer = device.createBuffer({
        size: 6*4,
        usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    uniformBindGroup = device.createBindGroup({
        layout: pipeline.getBindGroupLayout(0),
        entries: [
            { binding: 0, resource: { buffer: uniformBuffer } }
        ],
    });
    
    resize();
    
    const textureView = context.getCurrentTexture().createView();

    renderPassDescriptor = {
        colorAttachments: [{
            view: textureView,
            clearValue: { r: 0.2, g: 0.2, b: 0.2, a: 1.0 },
            loadOp: 'clear',
            //loadValue: { r: 0.2, g: 0.2, b: 0.2, a: 1.0 }, //Firefox
            storeOp: 'store'
        }]
    };
    
    draw();
};

let draw = function ()
{
    if (!context) return;

    tr  = [scale/csize[1], -(csize[0]*scale)/(csize[1]*2.0)-pos.x, scale/csize[1], -scale/2.0+pos.y];
    let trm = [tr[0], tr[1], tr[2], tr[3], mouse_pos.x, mouse_pos.y];
    let trbuf = new Float32Array(trm);
    
    device.queue.writeBuffer(
        uniformBuffer,
        0,
        trbuf.buffer,
        trbuf.byteOffset,
        trbuf.byteLength
    );
    
    const textureView = context.getCurrentTexture().createView();
    renderPassDescriptor = {
        colorAttachments: [{
            view: textureView,
            clearValue: { r: 0.2, g: 0.2, b: 0.2, a: 1.0 },
            loadOp: 'clear',
            //loadValue: { r: 0.2, g: 0.2, b: 0.2, a: 1.0 }, //Firefox
            storeOp: 'store'
        }]
    };
    
    const commandEncoder = device.createCommandEncoder();
    const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, uniformBindGroup);
    passEncoder.draw(6, 1, 0, 0);
    passEncoder.end();

    device.queue.submit([commandEncoder.finish()]);
};

let resize = function ()
{
    if (!context) return;
    
    canvas.width  = window.innerWidth;
    canvas.height = window.innerHeight;
    csize = [ canvas.width, canvas.height ];
    
    context.configure({
        device: device,
        format: format,
        size:   csize
    });
};



let zoomin  = function () { scale *= 0.8; };
let zoomout = function () { scale *= 1.25;  };
let handle_wheel = function (event)
{
    if (event.deltaY < 0) zoomin();
    else                  zoomout();
    
    draw();
};
let handle_mouse_down = function (event)
{
    grabbed = 1;
};
let handle_mouse_up = function (event)
{
    grabbed = 0;
};
let handle_mouse_move = function (event)
{
    if (grabbed === 1)
    {
        let a = scale/csize[1];
        pos.x += event.movementX * a;
        pos.y -= event.movementY * a;
        draw();
    }
    else if (mouse_param)
    {
        mouse_pos.x = tr[0] * event.clientX + tr[1];
        mouse_pos.y = tr[2] * event.clientY + tr[3];
        params.textContent = "(" + mouse_pos.x + ", " + mouse_pos.y + ")";
        draw();
    }
};
let handle_key_down = function (event)
{
    if (event.key === "q" || event.key === "Q"/* && event.ctrlKey */)
    {
        mouse_param = !mouse_param;
    }
    else if ((event.key === "c" || event.key === "C") && event.ctrlKey)
    {
        let p = "(" + mouse_pos.x + ", " + mouse_pos.y + ")";
        navigator.clipboard.writeText(p);
        console.log("P", p);
    }
    else if ((event.key === "v" || event.key === "V") && event.ctrlKey)
    {
        // Permissions API ...
    }
};


document.addEventListener("DOMContentLoaded", init);
document.addEventListener("keydown", handle_key_down);
//window.addEventListener("resize", function() { resize(); draw(); });
