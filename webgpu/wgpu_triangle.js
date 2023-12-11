
let canvas   = null;

//const pixr = window.devicePixelRatio || 1;

let csize    = [];


const vertex_wgsl = `\
@vertex //Firefox
//@stage(vertex) //Chromium
fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
    var pos = array<vec2<f32>, 3>(
    vec2<f32>(0.0, 0.5),
    vec2<f32>(-0.5, -0.5),
    vec2<f32>(0.5, -0.5));
    
    return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
}
`;

const fragment_wgsl = `\
@fragment //Firefox
//@stage(fragment) //Chromium
fn main() -> @location(0) vec4<f32> {
    return vec4<f32>(0.0, 0.6, 0.0, 1.0);
}
`;


let init = async function ()
{
    document.removeEventListener("DOMContentLoaded", init);
    
    if (!navigator.gpu) { alert('ERROR: WebGPU is not available'); return; }

    const adapter = await navigator.gpu.requestAdapter();
    const device  = await adapter.requestDevice();

    canvas  = document.getElementById('canvas');
    const context = canvas.getContext('webgpu');
    const format = navigator.gpu.getPreferredCanvasFormat(adapter);
    
    canvas.width  = window.innerWidth;
    canvas.height = window.innerHeight;
    csize = [ canvas.width, canvas.height ];
    
    context.configure({
        device: device,
        format: format,
        size:   csize
    });

    const pipeline = device.createRenderPipeline({
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
        layout: 'auto'
    });
    
    const commandEncoder = device.createCommandEncoder();
    const textureView    = context.getCurrentTexture().createView();

    const renderPassDescriptor = {
        colorAttachments: [{
            view: textureView,
            clearValue: { r: 0.2, g: 0.2, b: 0.2, a: 1.0 },
            loadOp: 'clear',
            //loadValue: { r: 0.2, g: 0.2, b: 0.2, a: 1.0 }, //Firefox
            storeOp: 'store'
        }]
    };

    const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
    passEncoder.setPipeline(pipeline);
    passEncoder.draw(3, 1, 0, 0);
    passEncoder.end();

    device.queue.submit([commandEncoder.finish()]);
};


document.addEventListener("DOMContentLoaded", init);
//window.addEventListener("resize", function() { resize(); draw(); });
