//
//  AppDelegate.m
//  Promys
//
//  Created by Olivier Debon on 20/01/2018.
//  Copyright © 2018 Olivier DEBON. All rights reserved.
//

#import "AppDelegate.h"
#import "promys.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;

@end

@implementation AppDelegate {
    NSText *message;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    NSView *view = [self.window contentView];
    CGRect frame = view.frame;
#define TEXT_HEIGHT 20.0
    message = [[ NSText alloc ] initWithFrame:NSMakeRect(0.0, (frame.size.height-TEXT_HEIGHT)/2.0 - 2.0, frame.size.width, TEXT_HEIGHT)];
    message.backgroundColor = [ NSColor windowBackgroundColor ] ;
    [ message setAlignment:NSTextAlignmentCenter ];
    [ view addSubview:message ];
    
    [message setString:@"Starting..."];

    Promys *promys = [[ Promys alloc ] init ];
    
    [promys start];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

-(void)showMessage:(NSString*)text {
    [message setString:text ];
}

@end
