//
//  VideoCaptureManagerLion.m
//  BoxSimpleShare
//
//  Created by Alexander Kievsky on 12/8/11.
//

#import "VideoCaptureManagerLion.h"

@interface NSObject (VideoManagerNSObjectExtensions)

- (id)initWithDisplayID:(CGDirectDisplayID)displayID;
- (void)setCropRect:(CGRect)rect;

@end

@implementation VideoCaptureManagerLion


- (id)init {
    if (self = [super init]) {
        static BOOL libraryLoaded = NO;
        if (!libraryLoaded) {
            NSBundle* bundle = [NSBundle bundleWithURL:[NSURL fileURLWithPath:@"/System/Library/Frameworks/AVFoundation.framework"]];
            BOOL loaded = [bundle load];
            NSLog(@"AVFoundation loaded = %d", loaded);
            libraryLoaded = YES;
        }
    }
    return self;
}

/*
 Rect received in this method is flipped - (0,0) in top-left corner, but avfoundation need bottom-left (non-flipped)
 */
- (void)startVideoCaptureInRect:(NSRect)rect screen:(NSScreen*)screen {
    // Create a capture session
    Class avCaptureSessionClass = NSClassFromString(@"AVCaptureSession");
    _session = [[avCaptureSessionClass alloc] init];
    
    // Set the session preset as you wish
    [_session performSelector:@selector(setSessionPreset:) withObject:@"AVCaptureSessionPresetHigh"];
    
    NSRect screenFrame = [screen frame];   
    NSSize screenSize = screenFrame.size;

    assert(!NSEqualSizes(screenSize, NSZeroSize));
    
    NSRect localScreenCaptureRect = rect;
    
    localScreenCaptureRect.origin.x -= screenFrame.origin.x;
    localScreenCaptureRect.origin.y -= screenFrame.origin.y;
    
    CGRect captureRect;
    captureRect.size = NSSizeToCGSize(localScreenCaptureRect.size);
    captureRect.origin.x = localScreenCaptureRect.origin.x;
    captureRect.origin.y = screenSize.height - localScreenCaptureRect.origin.y - localScreenCaptureRect.size.height;
    
    // Create a ScreenInput with the display and add it to the session
    Class avCaptureScreenInput = NSClassFromString(@"AVCaptureScreenInput");
    
    CGDirectDisplayID displayId = [[[screen deviceDescription] objectForKey:@"NSScreenNumber"] intValue];
    id input = [[[avCaptureScreenInput alloc] initWithDisplayID:displayId] autorelease];
    [input setCropRect:captureRect];
    if (!input) {
        [_session release];
        _session = nil;
        return;
    }
    [_session performSelector:@selector(addInput:) withObject:input];
    
    // Create a MovieFileOutput and add it to the session
    Class avCaptureMovieFileOutputClass = NSClassFromString(@"AVCaptureMovieFileOutput");
    _movieFileOutput = [[[avCaptureMovieFileOutputClass alloc] init] autorelease];
    [_session performSelector:@selector(addOutput:) withObject:_movieFileOutput];
    
    // Start running the session
    [_session performSelector:@selector(startRunning) withObject:nil];
    
    [_tempFileName release];
    _tempFileName = [[NSTemporaryDirectory() stringByAppendingPathComponent:@"movie_tmp.mov"] retain];
    NSURL* destPath = [NSURL fileURLWithPath:_tempFileName];
    // Delete any existing movie file first
    if ([[NSFileManager defaultManager] fileExistsAtPath:[destPath path]])
    {
        NSError*err;
        if (![[NSFileManager defaultManager] removeItemAtPath:[destPath path] error:&err])
        {
            NSLog(@"Error deleting existing movie %@",[err localizedDescription]);
        }
    }
    
    [_movieFileOutput performSelector:@selector(startRecordingToOutputFileURL:recordingDelegate:) withObject:destPath withObject:self];
}

- (void)saveCurrentMovieToFile:(NSString*)filename exportFormat:(long)exportFormat {
    [_movieFileOutput performSelector:@selector(stopRecording) withObject:nil];
    [_movie release];
    NSError* error;
    _movie = [[QTMovie movieWithFile:_tempFileName error:&error] retain];
    if (error) {
        NSLog(@"CAPTURING ERROR : %@", error);
    }
    [_movie detachFromCurrentThread];
    
    NSDictionary* dict = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], QTMovieFlatten, 
                          [NSNumber numberWithBool:YES], QTMovieExport,
                          [NSNumber numberWithLong:exportFormat], QTMovieExportType,
                          nil];
    NSMutableDictionary* info = [NSMutableDictionary dictionary];
    [info setObject:dict forKey:@"exportAttrs"];
    [info setObject:filename forKey:@"path"];
    
    [self performSelectorInBackground:@selector(saveMovieWithInfo:) withObject:info];

}

- (void)saveMovieWithInfo:(NSDictionary*)dict {
    [QTMovie enterQTKitOnThread];
    [_movie attachToCurrentThread];
    NSString* path = [dict objectForKey:@"path"];
//    path = @"/Users/kiev/Documents/tmp_simpleshare/movie.mov";
    if ([[NSFileManager defaultManager] fileExistsAtPath:path]) {
        [[NSFileManager defaultManager] removeItemAtURL:[NSURL fileURLWithPath:path] error:NULL];
    }
    [_movie writeToFile:path withAttributes:[dict objectForKey:@"exportAttrs"] error:NULL];
    [[NSFileManager defaultManager] removeItemAtURL:[NSURL fileURLWithPath:_tempFileName] error:NULL];
    [_tempFileName release];
    _tempFileName = nil;
    
    [_movie release];
    _movie = nil;
    [QTMovie exitQTKitOnThread];
    
    [self performSelectorOnMainThread:@selector(notifySaveFinishedToPath:) withObject:path waitUntilDone:NO];
}
         

- (void)captureOutput:(id)captureOutput didFinishRecordingToOutputFileAtURL:(NSURL*)outputFileURL fromConnections:(NSArray*)connections error:(NSError*)error {
    NSLog(@"Did finish recording to %@ due to error %@", [outputFileURL description], [error description]);
    
    // Stop running the session
    [_session performSelector:@selector(stopRunning) withObject:nil];
    
    // Release the session
    [_session release];
    _session = nil;
}

@end