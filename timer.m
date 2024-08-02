#import <Cocoa/Cocoa.h>

@interface CountdownView : NSView
@property (nonatomic, assign) NSInteger secondsLeft;
@property (nonatomic, assign) BOOL isGameOver;
@end

@implementation CountdownView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    NSString *displayString;
    if (self.isGameOver) {
        displayString = @"GAME OVER";
    } else {
        NSInteger hours = self.secondsLeft / 3600;
        NSInteger minutes = (self.secondsLeft % 3600) / 60;
        NSInteger seconds = self.secondsLeft % 60;
        displayString = [NSString stringWithFormat:@"%02ld:%02ld:%02ld", (long)hours, (long)minutes, (long)seconds];
    }
    
    NSDictionary *attributes = @{
        NSFontAttributeName: [NSFont fontWithName:@"IBM 3270" size:27],
        NSForegroundColorAttributeName: [NSColor redColor]
    };
    
    NSSize textSize = [displayString sizeWithAttributes:attributes];
    NSPoint point = NSMakePoint((dirtyRect.size.width - textSize.width) / 2,
                                (dirtyRect.size.height - textSize.height) / 2);
    
    [displayString drawAtPoint:point withAttributes:attributes];
}

@end

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (strong, nonatomic) NSWindow *window;
@property (strong, nonatomic) NSTimer *timer;
@property (strong, nonatomic) CountdownView *countdownView;
@property (assign, nonatomic) NSInteger initialMinutes;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    NSRect screenRect = [[NSScreen mainScreen] frame];
    NSRect windowRect = NSMakeRect(screenRect.size.width - 200, screenRect.size.height - 100, 200, 100);
    
    self.window = [[NSWindow alloc] initWithContentRect:windowRect
                                              styleMask:NSWindowStyleMaskBorderless
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    
    [self.window setBackgroundColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0]];
    [self.window setOpaque:NO];
    [self.window setLevel:NSFloatingWindowLevel];
    [self.window makeKeyAndOrderFront:nil];
    
    self.countdownView = [[CountdownView alloc] initWithFrame:windowRect];
    self.countdownView.secondsLeft = self.initialMinutes * 60;
    self.countdownView.isGameOver = NO;
    [self.window setContentView:self.countdownView];
    
    self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                 repeats:YES
                                                   block:^(NSTimer * _Nonnull timer) {
        if (self.countdownView.secondsLeft > 0) {
            self.countdownView.secondsLeft--;
        } else if (!self.countdownView.isGameOver) {
            self.countdownView.isGameOver = YES;
        }
        [self.countdownView setNeedsDisplay:YES];
    }];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    [self.timer invalidate];
}

@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSApplication *application = [NSApplication sharedApplication];
        AppDelegate *delegate = [[AppDelegate alloc] init];
        
        // Parse command line arguments
        if (argc > 1) {
            int minutes = atoi(argv[1]);
            if (minutes > 0) {
                delegate.initialMinutes = minutes;
            } else {
                delegate.initialMinutes = 60;
            }
        } else {
            delegate.initialMinutes = 60;
        }
        
        [application setDelegate:delegate];
        [application run];
    }
    return 0;
}