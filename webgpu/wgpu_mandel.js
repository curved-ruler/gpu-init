
let canvas   = null;
let context  = null;
let device   = null;
let format   = null;
let pipeline = null;

let uniformBuffer = null;
let renderPassDescriptor = null;
let commandEncoder = null;

//const pixr = window.devicePixelRatio || 1;

let csize    = [];
let tr       = [1,0,1,0];
let scale    = 2.5;
let pos      = { x:0.0,  y:0.0 };


const vertex_wgsl = `\

//@vertex //Firefox
@stage(vertex) //Chromium
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

struct Uniforms
{
    tr : vec4<f32>
};

@binding(0) @group(0) var<uniform> uniforms : Uniforms;

//@fragment //Firefox
@stage(fragment) //Chromium
fn main(@builtin(position) fragcoord : vec4<f32>) -> @location(0) vec4<f32>
{
    //var m:f32 = mandel(fragcoord.x, fragcoord.y);
    //return vec4<f32>(m, m, m, 1.0);
    
    var z = vec2<f32>(0.0);
    var x = uniforms.tr.x * fragcoord.x + uniforms.tr.y;
    var y = uniforms.tr.z * fragcoord.y + uniforms.tr.w;
    var i = i32(0);
    var n = i32(100);
    for (i=0 ; i<n ; i=i+1)
    {
        z = vec2<f32>(z.x*z.x - z.y*z.y + x, 2.0*z.x*z.y + y);
        if (length(z) > 2.0) { break; }
    }
    var c = f32(i)/f32(n);
    return vec4<f32>( c,c,c, 1.0 );
}
`;


let init = async function ()
{
    document.removeEventListener("DOMContentLoaded", init);
    
    if (!navigator.gpu) { alert('ERROR: WebGPU is not available'); return; }

    const adapter = await navigator.gpu.requestAdapter();
    device        = await adapter.requestDevice();

    canvas  = document.getElementById('canvas');
    context = canvas.getContext('webgpu');

    format = context.getPreferredFormat(adapter);

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
        }
    });
    
    uniformBuffer = device.createBuffer({
        size: 4*4,
        usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    const uniformBindGroup = device.createBindGroup({
        layout: pipeline.getBindGroupLayout(0),
        entries: [
            { binding: 0, resource: { buffer: uniformBuffer } }
        ],
    });
    
    resize();
    
    const textureView    = context.getCurrentTexture().createView();

    renderPassDescriptor = {
        colorAttachments: [{
            view: textureView,
            clearValue: { r: 0.2, g: 0.2, b: 0.2, a: 1.0 }, //Chromium
            loadOp: 'clear', //Chromium
            //loadValue: { r: 0.2, g: 0.2, b: 0.2, a: 1.0 }, //Firefox
            storeOp: 'store'
        }]
    };
    
    draw();
};

let draw = function ()
{
    if (!context) return;

    tr = [scale/csize[1], -(csize[0]*scale)/(csize[1]*2.0)-pos.x, scale/csize[1], -scale/2.0+pos.y];
    trbuf = new Float32Array(tr);
    
    device.queue.writeBuffer(
        uniformBuffer,
        0,
        trbuf.buffer,
        trbuf.byteOffset,
        trbuf.byteLength
    );
    
    const commandEncoder = device.createCommandEncoder();
    const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
    passEncoder.setPipeline(pipeline);
    passEncoder.draw(6, 1, 0, 0);
    passEncoder.end(); //Chromium
    //passEncoder.endPass(); //Firefox

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



document.addEventListener("DOMContentLoaded", init);
//window.addEventListener("resize", function() { resize(); draw(); });
