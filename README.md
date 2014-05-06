% SumatraPDF User-Defined Bookmarks
% 2014-05-06
% mh@tin-pot.net

User-Defined Bookmarks for SumatraPDF
=====================================


Motivation
----------

While *SumatraPDF* uses PDF bookmarks quite nicely to let the user navigate
the document (Menu _View_ | _Bookmarks_, or the `F12` key), not every PDF
document has them.

For example, the author recently bought the PDF version of ISO/IEC 14882:2011
from the ANSI online store (this is the "C++11" Standard). This version - branded
as an ANSI standard, but of course with identical content - does *not* have
section bookmarks embedded in the PDF file. Obviously, navigating in a 1356-page
tome without an outline is less than comfortable - and this licensed PDF file
is also "protected", so I can't add and embed my own bookmarks ...

While *SumatraPDF* allows to set "Favorites", ie pointers into documents, this
is not quite what I wanted: Favorites have no hierarchical structure, and in
my opinion the only "correct" solution would be to have *SumatraPDF* load the
document's bookmarks from outside, but then handle them *exactly as if* they
had been read from the PDF file in the first place.

I have now a modified *SumatraPDF* application which does exactly that:

- If the PDF file has embedded bookmarks, these are used in the *SumatraPDF*
  bookmarks pane.

- Otherwise, if I have defined my own Table of Contents, this is used and 
  displayed in the *SumatraPDF* bookmarks pane - with no noticeable difference
  (except that user-defined bookmarks can only point to *pages*, not to
  *locations* in page).



How it Works
------------

The concept is very easy:

 1. Store a user-defined "Table of Content" as a text file.

 2. When opening a PDF document in *SumatraPDF*, look if there is such a
    file for the document.

 3. When requested, produce a PDF bookmark tree from the JSON content,
    and not only from the PDF bookmarks as before.

I chose to store user-defined ToC's as JSON files, inside the
folder where *SumatraPDF*'s settings file is stored (that is, in
`%USERPROFILE%\AppData\Roaming\SumatraPDF`).

Furthermore, the "ToC file" for a PDF document is found by using (the
hexadecimal representation of) its MD5 hash value as a file name. As a
consequence, the ToC is bound to the document's *content*, and you can
rename, duplicate, move the PDF file around at will - the ToC will still
be associated with the document.


Implementation Notes
--------------------

Here is an overview of the modifications I have done:

 - I use [*cJSON*](http://sourceforge.net/projects/cjson/) to read JSON files:
   this adds the files `cJSON.h` and `cJSON.c` to `src/`.

 - I modified `src/BaseEngine.h` to (1) keep the MD5 hash around (which is
   computed anyway); and (2) provide derived classes with access to the
   JSON object from the user-defined ToC file. 

 - I added `src/BaseEngine,cpp`: This is where the user ToC file is searched and read.
   The parsed result - a `cJSON` object pointer - is kept around for use in derived classes.

 - I modified `src/PdfEngine.cpp`: Where the `GetTocTree()` method of `PdfEngineImpl` did
   only provide PDF-embedded data, it does now create a hierarchical `PdfDocItem` from
   the user's ToC definition (as parsed and stored in a `cJSON` object).
   Accordingly, the `PdfEngineImpl::HasTocTree() const` does (also) return `true`
   if a user-defined ToC is available.

Note that the settings file of *SumatraPDF* is completely oblivious of all this.
Therefore, you can pass "ToC files" from one user/installation/machine to the
next.


How to Use
----------

Right now this is a bare-minimum implementation: there is no user
interface for user-defined ToC's at all.

However, I found that creating a JSON file with a Table of Contents for
a given PDF - for example, the C++11 Standard again - using a decent
text editor (like *Vim* ...) is quite easy and efficient: after copy-
pasting the contents list from the PDF document into a text file, there
is some cleanup to do, but this is a fairly regular (expression ;-)
search-and-replace job.

The "ToC file" for the C++11 Standard, stored as
`641fdec2c42285a3c4ecc754a03d7676.json` in my
`AppData\Roaming\SumatraPDF\` directory, has about 360 bookmark entries;
it took me about 15 minutes to generate it. It looks like this:

~~~~
{
    "name" : "ISO/IEC 14882:2011 Programming Languages - C++",
    "md5"  : "641fdec2c42285a3c4ecc754a03d7676",
    "page1": 17,
    "items" : [
	{ "header" : "Contents", "pageno" : -3 }, 
	{ "header" : "List of Tables", "pageno" : -11 }, 
	{ "header" : "List of Figures", "pageno" : -15 }, 
	{ "header" : "1 General", "pageno" : 1, "items" : [
	    { "header" : "1.1 Scope", "pageno" : 1 }, 
	    { "header" : "1.2 Normative references", "pageno" : 1 }, 
	    { "header" : "1.3 Terms and definitions", "pageno" : 2 }, 

/* many more lines, not shown here */

	    { "header" : "D.10 auto_ptr", "pageno" : 1245 }, 
	    { "header" : "D.11 Violating exception-specifications", "pageno" : 1247 } 
	] }, 
	{ "header" : "E Universal character names for identifier characters", "pageno" : 1249, "items" : [
	    { "header" : "E.1 Ranges of characters allowed", "pageno" : 1249 }, 
	    { "header" : "E.2 Ranges of characters disallowed initially", "pageno" : 1249 } 
	] }, 
	{ "header" : "F Cross references", "pageno" : 1250 }, 
	{ "header" : "Index", "pageno" : 1268 }, 
	{ "header" : "Index of grammar productions", "pageno" : 1297 }, 
	{ "header" : "Index of library names", "pageno" : 1300 }, 
	{ "header" : "Index of implementation-defined behavior", "pageno" : 1336 }
    ]
}
~~~

The syntax of a JSON "ToC file" should be obvious from this example.

The `"name"` and `"md5"` items are not evaluated, but are there for the
orientation of the human reader.

The `"page1"` number specifies which PDF page corresponds to the page with
the number "1" printed on it - in this case, the "proper" page 1 is on the
17th PDF page (this is the number displayed in the *SumatraPDF* page number
control). The `"pageno"` values are interpreted according to the following
convention:

 1. Non-negative `"pageno"` values are "proper" page numbers. They take
    the `"page1"` value into account: A `"pageno"` value `1` means the 
    PDF page indicated by `"page1"`.

 2. Negative `"pageno"` values directly indicate PDF pages: the `"pageno"`
    value `-3` points to the 3rd PDF page, and `-11` to the 11th.

This convention allows to use the page numbers printed in the "Contents" 
section of a PDF document directly in the "ToC file".


Further Work
------------

I'm not sure of a "ToC-editing" GUI would be of much use; at least not
for me ... ;-)

Other nice-to-have features would be:

- Reasonable handling and diagnostics of JSON parse errors (currently 
  there is *none*) - this is the most important next step, IMO.

- Instead of burying the "ToC files" in the user's `AppData` directory,
  use a more flexible scheme (eg, store a map from MD5 hashes to "ToC file"
  pathnames in the *SumatraPDF* settings file).

- Provide (*Vim*? *Tcl*?) scripts to aid generating "ToC files" from
  "Contents" listings copy-pasted out of PDF documents.

- Let the user choose whether to use the embedded PDF bookmarks or the
  "ToC file" in case a PDF document has both (and remember this choice in
  the file history).

- Expand the feature to other media types too, eg for `.xps`.


