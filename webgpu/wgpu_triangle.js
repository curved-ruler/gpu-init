
let init = async function ()
{
    if (!navigator.gpu) { alert('ERROR: WebGPU is not available'); return; }

    const adapter = await navigator.gpu.requestAdapter();
    const device  = await adapter.requestDevice();

    const canvas  = document.getElementById('canvas');
    const context = canvas.getContext('webgpu');

    const devicePixelRatio = window.devicePixelRatio || 1;
    const presentationSize = [
        canvas.width  * devicePixelRatio,
        canvas.height * devicePixelRatio,
    ];
    const presentationFormat = context.getPreferredFormat(adapter);

    context.configure({
        device,
        format: presentationFormat,
        size: presentationSize,
    });

    const vertexShaderWgslCode =
        `
        @vertex //Firefox
        //@stage(vertex) //Chromium
        fn main(@builtin(vertex_index) VertexIndex : u32)
            -> @builtin(position) vec4<f32> {
                var pos = array<vec2<f32>, 3>(
                vec2<f32>(0.0, 0.5),
                vec2<f32>(-0.5, -0.5),
                vec2<f32>(0.5, -0.5));

            return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
        }
    `;

    const fragmentShaderWgslCode =
        `
        @fragment //Firefox
        //@stage(fragment) //Chromium
        fn main() -> @location(0) vec4<f32> {
            return vec4<f32>(0.0, 0.6, 0.0, 1.0);
        }
    `;

    const pipeline = device.createRenderPipeline({
        vertex: {
            module: device.createShaderModule({
                code: vertexShaderWgslCode
            }),
            entryPoint: 'main'
        },
        fragment: {
            module: device.createShaderModule({
                code: fragmentShaderWgslCode
            }),
            entryPoint: 'main',
            targets: [{
                format: presentationFormat,
            }]
        },
        primitive: {
            topology: 'triangle-list',
        }
    });

    const commandEncoder = device.createCommandEncoder();
    const textureView    = context.getCurrentTexture().createView();

    const renderPassDescriptor = {
        colorAttachments: [{
            view: textureView,
            loadValue: { r: 0.2, g: 0.2, b: 0.2, a: 1.0 },
            storeOp: 'store'
        }]
    };

    const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
    passEncoder.setPipeline(pipeline);
    passEncoder.draw(3, 1, 0, 0);
    //passEncoder.end(); //Chromium
    passEncoder.endPass(); //Firefox

    device.queue.submit([commandEncoder.finish()]);
};



document.addEventListener("DOMContentLoaded", init);
