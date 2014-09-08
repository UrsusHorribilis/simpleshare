
#import <Cocoa/Cocoa.h>
#import "PXListViewDropHighlight.h"

extern NSString * const PXListViewSelectionDidChange;

@class PXListView, PXListViewCell;

@protocol PXListViewDelegate <NSObject>

@required
- (NSUInteger)numberOfRowsInListView:(PXListView*)aListView;
- (CGFloat)listView:(PXListView*)aListView heightOfRow:(NSUInteger)row;
- (PXListViewCell*)listView:(PXListView*)aListView cellForRow:(NSUInteger)row;

@optional
- (void)listViewSelectionDidChange:(NSNotification*)aNotification;
- (void)listView:(PXListView*)aListView rowDoubleClicked:(NSUInteger)rowIndex;
- (void)listView:(PXListView*)aListView rowClicked:(NSUInteger)rowIndex;

- (BOOL)listView:(PXListView*)aListView writeRowsWithIndexes:(NSIndexSet*)rowIndexes toPasteboard:(NSPasteboard *)pboard;
- (NSDragOperation)listView:(PXListView*)aListView
               validateDrop:(id <NSDraggingInfo>)info
                proposedRow:(NSUInteger)row
      proposedDropHighlight:(PXListViewDropHighlight)dropHighlight;
- (BOOL)listView:(PXListView*)aListView
      acceptDrop:(id <NSDraggingInfo>)info
             row:(NSUInteger)row
   dropHighlight:(PXListViewDropHighlight)dropHighlight;

@end