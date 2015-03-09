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

#import "AppView.h"
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

@implementation AppView

- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat*)format {
    self = [super initWithFrame:frameRect pixelFormat:format];
    return self;
}


- (void)prepareOpenGL {

    // this sets swap interval for double buffering
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    [[self window] setAcceptsMouseMovedEvents:YES];

    timer = [NSTimer timerWithTimeInterval:0.001
                                    target:self
                                  selector:@selector(timerEvent:)
                                  userInfo:nil
                                   repeats:YES];

    [[NSRunLoop mainRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];

    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_MULTISAMPLE);
    glPointSize(1.0f);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}


- (void)timerEvent:(NSTimer *)t {
    [renderProxy update];
    [self render];
}


- (BOOL) acceptsFirstResponder {
    return NO;
}


- (BOOL) becomeFirstResponder {
    return YES;
}


- (void) setRenderProxy:(id)proxy {
    renderProxy = proxy;
}


- (void) render {
    NSRect backingBounds = [self convertRectToBacking:[self bounds]];
    GLsizei backingPixelWidth  = (GLsizei)(backingBounds.size.width),
    backingPixelHeight = (GLsizei)(backingBounds.size.height);

    // Set viewport
    glViewport(0, 0, backingPixelWidth, backingPixelHeight);

    glClearColor(0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    [renderProxy render];

    [[self openGLContext] flushBuffer];
}


- (void) drawRect: (NSRect) bounds {
    [self render];
}


- (void) dealloc {
    [super dealloc];
}
@end
