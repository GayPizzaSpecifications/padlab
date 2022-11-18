#include "draw.h"
#include "metalShader.h"
#include <SDL_metal.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

SDL_Window* window = NULL;
SDL_MetalView view = NULL;
CAMetalLayer* layer;
id<MTLDevice> dev = nil;
id<MTLCommandQueue> queue;
MTLRenderPassDescriptor* passDesc;
id<MTLRenderPipelineState> pso; // gonna fine u on the belgrave & lillydale line
uint32_t curColour = BLACK;
MTLViewport viewport;


void DrawWindowHints(void) {}

int InitDraw(SDL_Window* _window)
{
	// Create Metal view
	window = _window;
	view = SDL_Metal_CreateView(window);

	// Get Metal device
#if 1
	dev = MTLCreateSystemDefaultDevice();
	fprintf(stderr, "Default MTL device \"%s\"\n", [dev.name UTF8String]);
#else
	NSArray<id<MTLDevice>>* devices = MTLCopyAllDevices();
	for (id<MTLDevice> i in devices)
	{
		if (!i.supportsRaytracing) continue;
		if (dev && i.isLowPower) continue;
		dev = i;
	}
	fprintf(stderr, "You have selected funny device \"%s\"\n", [dev.name UTF8String]);
#endif

	// Setup Metal layer
	layer = (__bridge CAMetalLayer*)SDL_Metal_GetLayer(view);
	layer.device = dev;
	layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	//layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;

	queue = [dev newCommandQueue];
	passDesc = [MTLRenderPassDescriptor new];
	passDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
	passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
	DrawClear(); // passDesc.colorAttachments[0].clearColor = curColour

	// Compile shady bois
	__autoreleasing NSError* booboo = nil;
	dispatch_data_t shaderData = dispatch_data_create(shader_metallib, SHADER_METALLIB_SIZE, nil, nil);
	id<MTLLibrary> lib = [dev newLibraryWithData:shaderData error:&booboo];
	if (!lib)
	{
		fprintf(stderr, "Metal shader compilation failed:\n%s\n", [[booboo localizedDescription] UTF8String]);
		return -1;
	}
	id<MTLFunction> vertPrg = [lib newFunctionWithName:@"vertexMain"];
	id<MTLFunction> fragPrg = [lib newFunctionWithName:@"fragmentMain"];

	// Setup render pipeline state
	MTLRenderPipelineDescriptor* pipeDesc = [[MTLRenderPipelineDescriptor alloc] init];
	pipeDesc.vertexFunction = vertPrg;
	pipeDesc.fragmentFunction = fragPrg;
	pipeDesc.colorAttachments[0].pixelFormat = layer.pixelFormat;
	pso = [dev newRenderPipelineStateWithDescriptor:pipeDesc error:&booboo];
	if (!pso)
	{
		fprintf(stderr, "Your Myki has expired and u have been slapped with a $237 fine: %s\n", [[booboo localizedDescription] UTF8String]);
		return -1;
	}
	[pipeDesc release];

	return 0;
}


void QuitDraw(void)
{
	SDL_Metal_DestroyView(view);
}


size GetDrawSizeInPixels(void)
{
	size out;
	SDL_Metal_GetDrawableSize(SDL_GL_GetCurrentWindow(), &out.w, &out.h);
	return out;
}


void SetDrawViewport(size size)
{
	viewport = (MTLViewport){
		.originX = 0.0,
		.originY = 0.0,
		.width = size.w,
		.height = size.h,
		.znear = 1.0,
		.zfar = -1.0};
}


void SetDrawColour(uint32_t c)
{
	curColour = c;
}


void DrawClear(void)
{
	const double mul = 1.0 / 255.0;
	passDesc.colorAttachments[0].clearColor = MTLClearColorMake(
		(double)((curColour & 0xFF000000) >> 24) * mul,
		(double)((curColour & 0x00FF0000) >> 16) * mul,
		(double)((curColour & 0x0000FF00) >>  8) * mul,
		(double)((curColour & 0x000000FF)) * mul);
}


void DrawPoint(int x, int y)
{
	
}


void DrawRect(int x, int y, int w, int h)
{
	
}


void DrawLine(int x1, int y1, int x2, int y2)
{
	
}


void DrawCircleSteps(int x, int y, int r, int steps)
{
	
}


void DrawArcSteps(int x, int y, int r, int startAng, int endAng, int steps)
{
	
}


void DrawPresent(void)
{
	@autoreleasepool {
		id<CAMetalDrawable> rt = [layer nextDrawable];
		passDesc.colorAttachments[0].texture = rt.texture;

		id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
		id<MTLRenderCommandEncoder> enc = [cmdBuf renderCommandEncoderWithDescriptor:passDesc];

		[enc setViewport:viewport];

		[enc setRenderPipelineState:pso];
		[enc endEncoding];
		[cmdBuf presentDrawable:rt]; // u go to software premieres
		[cmdBuf commit]; // is that linus torvolds?
		// hear songs on pandora radiowo
	}
}
