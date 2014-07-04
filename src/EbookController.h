/* Copyright 2014 the SumatraPDF project authors (see AUTHORS file).
   License: GPLv3 */

#ifndef EbookController_h
#define EbookController_h

#include "Controller.h"
#include "Doc.h"
#include "Sigslot.h"

class   EbookController;
struct  EbookControls;
class   HtmlPage;
struct  DrawInstr;
class   EbookFormattingThread;
struct  EbookFormattingData;
class   HtmlFormatterArgs;
class   HtmlFormatter;
namespace mui { class Control; }
using namespace mui;

class EbookController : public Controller, public sigslot::has_slots
{
public:
    EbookController(EbookControls *ctrls, ControllerCallback *cb);
    virtual ~EbookController();

    virtual const WCHAR *FilePath() const { return _doc.GetFilePath(); }
    virtual const WCHAR *DefaultFileExt() const { return _doc.GetDefaultFileExt(); }
    virtual int PageCount() const { return GetMaxPageCount(); }
    virtual WCHAR *GetProperty(DocumentProperty prop) { return _doc.GetProperty(prop); }

    virtual int CurrentPageNo() const { return currPageNo; }
    virtual void GoToPage(int pageNo, bool addNavPoint);
    virtual bool CanNavigate(int dir) const;
    virtual void Navigate(int dir);

    virtual void SetDisplayMode(DisplayMode mode, bool keepContinuous=false);
    virtual DisplayMode GetDisplayMode() const { return IsDoublePage() ? DM_FACING : DM_SINGLE_PAGE; }
    virtual void SetPresentationMode(bool enable) { /* not supported */ }
    virtual void SetFullScreenMode(bool enable) { /* not supported */ }
    virtual void SetZoomVirtual(float zoom, PointI *fixPt=NULL) { /* not supported */ }
    virtual float GetZoomVirtual() const { return 100; }
    virtual float GetNextZoomStep(float towards) const { return 100; }
    virtual void SetViewPortSize(SizeI size);

    virtual bool HasTocTree() const { return _doc.HasToc(); }
    virtual DocTocItem *GetTocTree();
    virtual void ScrollToLink(PageDestination *dest);
    virtual PageDestination *GetNamedDest(const WCHAR *name);

    virtual void UpdateDisplayState(DisplayState *ds);
    virtual void CreateThumbnail(SizeI size, ThumbnailCallback *tnCb);

    virtual bool GoToNextPage();
    virtual bool GoToPrevPage(bool toBottom=false);

    virtual EbookController *AsEbook() { return this; }

public:
    // the following is specific to EbookController

    const Doc *doc() const { return &_doc; }

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam, bool& wasHandled);
    void EnableMessageHandling(bool enable) { handleMsgs = enable; }
    void UpdateDocumentColors();
    void RequestRepaint();
    void HandlePagesFromEbookLayout(EbookFormattingData *ebookLayout);
    void TriggerLayout();
    void SetDoc(Doc newDoc, int startReparseIdxArg=-1, DisplayMode displayMode=DM_AUTOMATIC);
    int  ResolvePageAnchor(const WCHAR *id);
    void CopyNavHistory(EbookController& orig);
    int  CurrentTocPageNo() const;

    // call SetDoc before using this EbookController
    static EbookController *Create(HWND hwnd, ControllerCallback *cb);

protected:

    EbookControls * ctrls;

    Doc             _doc;

    // TODO: this should be recycled along with pages so that its
    // memory use doesn't grow without bounds
    PoolAllocator   textAllocator;

    Vec<HtmlPage*> *    pages;

    // pages being sent from background formatting thread
    Vec<HtmlPage*> *    incomingPages;

    // currPageNo is in range 1..$numberOfPages. 
    int             currPageNo;
    // reparseIdx of the current page (the first one if we're showing 2)
    int             currPageReparseIdx;

    // size of the page for which pages were generated
    SizeI           pageSize;

    EbookFormattingThread * formattingThread;
    int                     formattingThreadNo;

    // whether HandleMessage passes messages on to ctrls->mainWnd
    bool            handleMsgs;

    // parallel lists mapping anchor IDs to reparseIdxs
    WStrVec *   pageAnchorIds;
    Vec<int> *  pageAnchorIdxs;

    Vec<int>    navHistory;
    size_t      navHistoryIx;

    Vec<HtmlPage*> *GetPages();
    void        UpdateStatus();
    bool        FormattingInProgress() const { return formattingThread != NULL; }
    void        StopFormattingThread();
    void        CloseCurrentDocument();
    int         GetMaxPageCount() const;
    bool        IsDoublePage() const;
    void        ExtractPageAnchors();
    void        AddNavPoint();
    void        OnClickedLink(int pageNo, DrawInstr *link);

    // event handlers
    void        ClickedNext(Control *c, int x, int y);
    void        ClickedPrev(Control *c, int x, int y);
    void        ClickedProgress(Control *c, int x, int y);
    void        SizeChangedPage(Control *c, int dx, int dy);
    void        ClickedPage1(Control *c, int x, int y);
    void        ClickedPage2(Control *c, int x, int y);
};

HtmlFormatterArgs *CreateFormatterArgsDoc(Doc doc, int dx, int dy, Allocator *textAllocator=NULL);
HtmlFormatter *CreateFormatter(Doc doc, HtmlFormatterArgs* args);

#endif
