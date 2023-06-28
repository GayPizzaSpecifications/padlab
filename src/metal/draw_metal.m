#include "draw.h"
#include "metal_shader_types.h"
#include "metalShader.h"
#include "maths.h"
#include <SDL_metal.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>


@interface MetalRenderer : NSObject

@property (nonatomic, readwrite) uint32_t drawColour;

- (id) init:(SDL_Window*)window;
- (void) dealloc;

- (unsigned) resizeBuffer:(void**)buf itemSize:(unsigned)nsz reserveLen:(unsigned)reserve requiredNum:(unsigned)count;
- (void) resizeMtlBuffer:(id <MTLBuffer>*)buf length:(unsigned)length;

- (size) getDrawSize;
- (void) setClear:(uint32_t)colour;
- (void) setView:(size)viewportSize;
- (void) clearVerticesAndIndices;
- (void) reserveVertices:(unsigned)count;
- (void) reserveIndices:(unsigned)count;
- (uint16_t) queueVertex:(int)x :(int)y;
- (uint16_t) queueVertexF:(float)x :(float)y;
- (uint16_t) queueIndex:(uint16_t)idx;
- (void) queueIndices:(uint16_t*)idcs count:(unsigned)count;
- (void) present;

@end

#define DRAWLIST_CHUNK_SIZE 480
#define DRAWLIST_INIT_SIZE (DRAWLIST_CHUNK_SIZE * 3)

@implementation MetalRenderer
{
	SDL_Window* _window;
	SDL_MetalView _view;
	id<MTLDevice> _dev;
	CAMetalLayer* _layer;
	id<MTLCommandQueue> _queue;
	MTLRenderPassDescriptor* _passDesc;
	id<MTLRenderPipelineState> _pso;
	MTLViewport _viewport;
	vector_float4 _drawColourF;

	ShaderVertex* _vtxList;
	uint16_t* _idxList;
	unsigned _vtxListCount, _vtxListReserve, _idxListCount, _idxListReserve;
	id<MTLBuffer> _vtxMtlBuffer, _idxMtlBuffer;
}

- (id) init:(SDL_Window*)window
{
	if (!(self = [super init]))
		return nil;

	self.drawColour = BLACK;
	_vtxList = NULL;
	_idxList = NULL;
	_vtxListCount = 0;
	_idxListCount = 0;
	_vtxListReserve = 0;
	_idxListReserve = 0;
	_vtxMtlBuffer = nil;
	_idxMtlBuffer = nil;

	// Create Metal view
	_window = window;
	_view = SDL_Metal_CreateView(_window);

	// Get Metal device
#if 1
	// Default device
	_dev = MTLCreateSystemDefaultDevice();
	fprintf(stderr, "Default MTL device \"%s\"\n", [_dev.name UTF8String]);
#else
	// Non-low power device
	NSArray<id<MTLDevice>>* devices = MTLCopyAllDevices();
	for (id<MTLDevice> i in devices)
	{
		if (!i.supportsRaytracing) continue;
		if (_dev && i.isLowPower) continue;
		_dev = i;
	}
	fprintf(stderr, "You have selected funny device \"%s\"\n", [_dev.name UTF8String]);
#endif

	// Setup Metal layer
	_layer = (__bridge CAMetalLayer*)SDL_Metal_GetLayer(_view);
	_layer.device = _dev;
	_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;

	_queue = [_dev newCommandQueue];
	_passDesc = [MTLRenderPassDescriptor new];
	_passDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
	_passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
	[self setClear:self.drawColour]; // passDesc.colorAttachments[0].clearColor = curColour

	// Compile shaders
	__autoreleasing NSError* err = nil;
	dispatch_data_t shaderData = dispatch_data_create(shader_metallib, SHADER_METALLIB_SIZE, nil, nil);
	id<MTLLibrary> lib = [_dev newLibraryWithData:shaderData error:&err];
	if (!lib)
	{
		fprintf(stderr, "Metal shader compilation failed:\n%s\n", [[err localizedDescription] UTF8String]);
		return nil;
	}
	id<MTLFunction> vertPrg = [lib newFunctionWithName:@"vertexMain"];
	id<MTLFunction> fragPrg = [lib newFunctionWithName:@"fragmentMain"];

	// Setup render pipeline state
	MTLRenderPipelineDescriptor* pipeDesc = [[MTLRenderPipelineDescriptor alloc] init];
	pipeDesc.vertexFunction = vertPrg;
	pipeDesc.fragmentFunction = fragPrg;
	pipeDesc.colorAttachments[0].pixelFormat = _layer.pixelFormat;
	_pso = [_dev newRenderPipelineStateWithDescriptor:pipeDesc error:&err];
	if (!_pso)
	{
		fprintf(stderr, "Pipeline state creation failed: %s\n", [[err localizedDescription] UTF8String]);
		return nil;
	}
	[pipeDesc release];

	// Set viewport
	[self setView:[self getDrawSize]];

	return self;
}

- (void) dealloc
{
	SDL_Metal_DestroyView(_view);
	[super dealloc];
}


- (unsigned) resizeBuffer:(void**)buf itemSize:(unsigned)nsz reserveLen:(unsigned)reserve requiredNum:(unsigned)count
{
	if (*buf && count * nsz <= reserve)
		return reserve;

	// Calculate new capacity
	unsigned newCapacity = (count + DRAWLIST_CHUNK_SIZE - 1) / DRAWLIST_CHUNK_SIZE * DRAWLIST_CHUNK_SIZE;
	if (!*buf)
		newCapacity = MAX(newCapacity, DRAWLIST_INIT_SIZE);

	// (Re)allocate and return new reserve size
	unsigned newReserve = newCapacity * nsz;
	*buf = realloc(*buf, newReserve);
	return newReserve;
}

- (void) resizeMtlBuffer:(id <MTLBuffer>*)buf length:(unsigned)length
{
	if (*buf && length <= [*buf length])
		return;

	id<MTLBuffer> new = [_dev newBufferWithLength:length options:MTLResourceStorageModeManaged];
	if (*buf)
	{
		[*buf setPurgeableState:MTLPurgeableStateEmpty];
		[*buf release];
	}
	*buf = new;
}


- (size) getDrawSize
{
	size out;
	SDL_Metal_GetDrawableSize(_window, &out.w, &out.h);
	return out;
}

- (void)setDrawColour:(uint32_t)colour
{
	_drawColour = colour;
	const float mul = 1.0f / 255.0f;
	_drawColourF = (vector_float4){
		(float)((colour & 0xFF000000) >> 24) * mul,
		(float)((colour & 0x00FF0000) >> 16) * mul,
		(float)((colour & 0x0000FF00) >>  8) * mul,
		(float)((colour & 0x000000FF)) * mul };
}

- (void) setClear:(uint32_t)colour
{
	const double mul = 1.0 / 255.0;
	_passDesc.colorAttachments[0].clearColor = MTLClearColorMake(
		(double)((colour & 0xFF000000) >> 24) * mul,
		(double)((colour & 0x00FF0000) >> 16) * mul,
		(double)((colour & 0x0000FF00) >>  8) * mul,
		(double)((colour & 0x000000FF)) * mul);
}

- (void) setView:(size)viewportSize
{
	_viewport = (MTLViewport){
		.originX = 0.0, .originY = 0.0,
		.width = viewportSize.w, .height = viewportSize.h,
		.znear = 1.0, .zfar = -1.0 };
}

- (void) clearVerticesAndIndices
{
	_vtxListCount = 0;
	_idxListCount = 0;
}

- (void) reserveVertices:(unsigned)count
{
	count += _vtxListCount;
	if (count * sizeof(ShaderVertex) > _vtxListReserve)
		_vtxListReserve = [self resizeBuffer:(void**)&_vtxList
			itemSize:sizeof(ShaderVertex) reserveLen:_vtxListReserve requiredNum:count];
}

- (void) reserveIndices:(unsigned)count
{
	count += _idxListCount;
	if (count * sizeof(uint16_t) > _idxListReserve)
		_idxListReserve = [self resizeBuffer:(void**)&_idxList
			itemSize:sizeof(uint16_t) reserveLen:_idxListReserve requiredNum:count];
}

- (uint16_t) queueVertex:(int)x :(int)y
{
	if (_vtxListCount * sizeof(ShaderVertex) >= _vtxListReserve)
		[self reserveVertices:1];
	_vtxList[_vtxListCount] = (ShaderVertex){
		.position = { (float)x, (float)y },
		.colour = _drawColourF };
	return _vtxListCount++;
}

- (uint16_t) queueVertexF:(float)x :(float)y
{
	if (_vtxListCount * sizeof(ShaderVertex) >= _vtxListReserve)
		[self reserveVertices:1];
	_vtxList[_vtxListCount] = (ShaderVertex){
		.position = { x, y },
		.colour = _drawColourF };
	return _vtxListCount++;
}

- (uint16_t) queueIndex:(uint16_t)idx
{
	if (_idxListCount * sizeof(uint16_t) >= _idxListReserve)
		[self reserveIndices:1];
	_idxList[_idxListCount++] = idx;
	return idx;
}

- (void) queueIndices:(uint16_t*)idcs count:(unsigned)count
{
	if ((_idxListCount + count) * sizeof(uint16_t) >= _idxListReserve)
		[self reserveIndices:count];
	memcpy(&_idxList[_idxListCount], idcs, count * sizeof(uint16_t));
	_idxListCount += count;
}

- (void) present
{
	// Create or recreate buffers if needed & fill with data
	if (_vtxListCount)
	{
		unsigned copyLen = _vtxListCount * sizeof(ShaderVertex);
		if (_vtxMtlBuffer == nil || [_vtxMtlBuffer length] < copyLen)
			[self resizeMtlBuffer:&_vtxMtlBuffer length:_vtxListReserve];
		memcpy(_vtxMtlBuffer.contents, _vtxList, copyLen);
	}
	if (_idxListCount)
	{
		unsigned copyLen = _idxListCount * sizeof(uint16_t);
		if (_idxMtlBuffer == nil || [_idxMtlBuffer length] < copyLen)
			[self resizeMtlBuffer:&_idxMtlBuffer length:_idxListReserve];
		memcpy(_idxMtlBuffer.contents, _idxList, copyLen);
	}

	@autoreleasepool
	{
		id<CAMetalDrawable> rt = [_layer nextDrawable];
		_passDesc.colorAttachments[0].texture = rt.texture;

		id<MTLCommandBuffer> cmdBuf = [_queue commandBuffer];
		id<MTLRenderCommandEncoder> enc = [cmdBuf renderCommandEncoderWithDescriptor:_passDesc];

		[enc setViewport:_viewport];
		[enc setCullMode:MTLCullModeNone];
		[enc setRenderPipelineState:_pso];

		if (_vtxMtlBuffer != nil && _idxMtlBuffer != nil)
		{
			[enc setVertexBuffer:_vtxMtlBuffer offset:0 atIndex:ShaderInputIdxVerticies];
			const vector_float2 viewportScale = { (float)(1.0 / _viewport.width), (float)(1.0 / _viewport.height) };
			[enc setVertexBytes:&viewportScale length:sizeof(vector_float2) atIndex:ShaderInputViewportScale];
			[enc drawIndexedPrimitives:MTLPrimitiveTypeLine
				indexCount:_idxListCount indexType:MTLIndexTypeUInt16
				indexBuffer:_idxMtlBuffer indexBufferOffset:0];
		}

		[enc endEncoding];
		[cmdBuf presentDrawable:rt];
		[cmdBuf commit];

		[self clearVerticesAndIndices];
	}
}

@end


static MetalRenderer* renderer = nil;

void DrawWindowHints(void) {}

int InitDraw(SDL_Window* window)
{
	renderer = [[MetalRenderer alloc] init:window];
	if (!renderer)
		return -1;

	return 0;
}


void QuitDraw(void)
{
	[renderer release];
}


size GetDrawSizeInPixels(void)
{
	return renderer ? [renderer getDrawSize] : (size){ 0, 0 };
}


void SetDrawViewport(size size)
{
	[renderer setView:size];
}


void SetDrawColour(uint32_t c)
{
	renderer.drawColour = c;
}


void DrawClear(void)
{
	[renderer setClear:renderer.drawColour];
	[renderer clearVerticesAndIndices];
}


void DrawPoint(int x, int y)
{
	DrawCircleSteps(x, y, 1, 4);
}


void DrawRect(int x, int y, int w, int h)
{
	[renderer reserveVertices:4];
	uint16_t i00 = [renderer queueVertex:x     :y];
	uint16_t i10 = [renderer queueVertex:x + w :y];
	uint16_t i11 = [renderer queueVertex:x + w :y + h];
	uint16_t i01 = [renderer queueVertex:x     :y + h];
	uint16_t indices[] = { i00, i10, i10, i11, i11, i01, i01, i00 };
	[renderer queueIndices:indices count:sizeof(indices) / sizeof(uint16_t)];
}


void DrawLine(int x1, int y1, int x2, int y2)
{
	[renderer queueIndex:[renderer queueVertex:x1 :y1]];
	[renderer queueIndex:[renderer queueVertex:x2 :y2]];
}


void DrawCircleSteps(int x, int y, int r, int steps)
{
	const float fx = (float)x, fy = (float)y;
	const float stepSz = (float)TAU / (float)abs(steps);
	const float mag = (float)r;

	// Draw whole circle in a single loop
	[renderer reserveVertices:steps];
	[renderer reserveIndices:steps * 2];
	uint16_t base = [renderer queueIndex:[renderer queueVertex:x + r :y]];
	for (int i = 1; i < steps; ++i)
	{
		const float theta = stepSz * (float)i;
		uint16_t ii = [renderer queueVertexF:fx + cosf(theta) * mag :fy + sinf(theta) * mag];
		[renderer queueIndices:(uint16_t[]){ ii, ii } count:2];
	}
	[renderer queueIndex:base];
}


void DrawArcSteps(int x, int y, int r, int startAng, int endAng, int steps)
{
	const float fx = (float)x, fy = (float)y;
	const float magw = (float)r, magh = (float)r;

	const float start = (float)startAng * (float)DEG2RAD;
	const float stepSz = (float)(endAng - startAng) / (float)abs(steps) * (float)DEG2RAD;
	[renderer reserveVertices:steps];
	[renderer reserveIndices:steps * 2];
	uint16_t ii = [renderer queueVertexF:fx + cosf(start) * magw :fy - sinf(start) * magh];
	for (int i = 1; i <= steps; ++i)
	{
		const float theta = start + stepSz * (float)i;
		uint16_t iii = [renderer queueVertexF:fx + cosf(theta) * magw :fy - sinf(theta) * magh];
		[renderer queueIndices:(uint16_t[]){ ii, iii } count:2];
		ii = iii;
	}
}

void DrawPresent(void)
{
	[renderer present];
}
