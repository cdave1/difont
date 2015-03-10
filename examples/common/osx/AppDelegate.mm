/*
 * Copyright (c) 2015 David Petrie david@davidpetrie.com
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software. Permission is granted to anyone to use this software for
 * any purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim
 * that you wrote the original software. If you use this software in a product, an
 * acknowledgment in the product documentation would be appreciated but is not
 * required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#import "AppDelegate.h"
#include <algorithm>
#include "OpenGLShim.h"
#include "HelloWorld.h"

@interface FullScreenWindow : NSWindow
- (BOOL)canBecomeKeyWindow;
@end


@implementation FullScreenWindow
- (BOOL)canBecomeKeyWindow {
    return YES;
}
@end


@implementation AppDelegate

@synthesize window, glView;


static HelloWorld *helloWorld;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    NSRect mainDisplayRect = [[NSScreen mainScreen] frame];
    NSRect viewRect = NSMakeRect(0.0, 0.0, mainDisplayRect.size.width, mainDisplayRect.size.height);
    self.window = [[FullScreenWindow alloc] initWithContentRect:viewRect
                                                      styleMask:NSBorderlessWindowMask
                                                        backing:NSBackingStoreBuffered
                                                          defer:YES];
    [self.window setLevel:NSMainMenuWindowLevel+1];
    [self.window setOpaque:YES];
    [self.window setHidesOnDeactivate:YES];

    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAMultisample,
        NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)1,
        NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute)4,
        0
    };

    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    self.glView = [[AppView alloc] initWithFrame:viewRect pixelFormat: pixelFormat];
    [self.window setContentView: self.glView];
    [self.glView convertRectToBacking:viewRect];
    [self.glView setRenderProxy:self];
    [self.window makeKeyAndOrderFront:self];
    [self.window makeFirstResponder:self.glView];

    NSString *path = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"DIN Alternate Bold.ttf"];
    helloWorld = new HelloWorld(viewRect.size.width, viewRect.size.height, 1.0f);
    helloWorld->SetupFonts([path UTF8String]);
}


- (void) update {
}


- (void) render {
    if (helloWorld) {
        helloWorld->Render();
    }
}


- (void) createFailed {
    [NSApp terminate:self];
}


- (void) dealloc {
    [self.glView release];
    [super dealloc];
}

@end
