/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtcanvas.h"
#include <QApplication>
#include <QBitmap>
#include <QDesktopWidget>
#include <QImage>
#include <QPainter>
#include <QTimer>
#include <qhash.h>
#include <qset.h>
#include <qalgorithms.h>
#include <qevent.h>
#include <qpainterpath.h>

#include <stdlib.h>
using namespace Qt;

class QtCanvasData {
public:
    QtCanvasData()
    {
    }

    QList<QtCanvasView *> viewList;
    QSet<QtCanvasItem *> itemDict;
    QSet<QtCanvasItem *> animDict;
};

class QtCanvasViewData {
public:
    QtCanvasViewData() {}
    QMatrix xform;
    QMatrix ixform;
    bool highQuality;
};

// clusterizer

class QtCanvasClusterizer {
public:
    QtCanvasClusterizer(int maxclusters);
    ~QtCanvasClusterizer();

    void add(int x, int y); // 1x1 rectangle (point)
    void add(int x, int y, int w, int h);
    void add(const QRect& rect);

    void clear();
    int clusters() const { return count; }
    const QRect& operator[](int i) const;

private:
    QRect* cluster;
    int count;
    const int maxcl;
};

static
void include(QRect& r, const QRect& rect)
{
    if (rect.left() < r.left()) {
            r.setLeft(rect.left());
    }
    if (rect.right()>r.right()) {
            r.setRight(rect.right());
    }
    if (rect.top() < r.top()) {
            r.setTop(rect.top());
    }
    if (rect.bottom()>r.bottom()) {
            r.setBottom(rect.bottom());
    }
}

/*
A QtCanvasClusterizer groups rectangles (QRects) into non-overlapping rectangles
by a merging heuristic.
*/
QtCanvasClusterizer::QtCanvasClusterizer(int maxclusters) :
    cluster(new QRect[maxclusters]),
    count(0),
    maxcl(maxclusters)
{ }

QtCanvasClusterizer::~QtCanvasClusterizer()
{
    delete [] cluster;
}

void QtCanvasClusterizer::clear()
{
    count = 0;
}

void QtCanvasClusterizer::add(int x, int y)
{
    add(QRect(x, y, 1, 1));
}

void QtCanvasClusterizer::add(int x, int y, int w, int h)
{
    add(QRect(x, y, w, h));
}

void QtCanvasClusterizer::add(const QRect& rect)
{
    QRect biggerrect(rect.x()-1, rect.y()-1, rect.width()+2, rect.height()+2);

    //assert(rect.width()>0 && rect.height()>0);

    int cursor;

    for (cursor = 0; cursor < count; cursor++) {
        if (cluster[cursor].contains(rect)) {
            // Wholly contained already.
            return;
        }
    }

    int lowestcost = 9999999;
    int cheapest = -1;
    cursor = 0;
    while(cursor < count) {
        if (cluster[cursor].intersects(biggerrect)) {
            QRect larger = cluster[cursor];
            include(larger, rect);
            int cost = larger.width()*larger.height() -
                       cluster[cursor].width()*cluster[cursor].height();

            if (cost < lowestcost) {
                bool bad = false;
                for (int c = 0; c < count && !bad; c++) {
                    bad = cluster[c].intersects(larger) && c!= cursor;
                }
                if (!bad) {
                    cheapest = cursor;
                    lowestcost = cost;
                }
            }
        }
        cursor++;
    }

    if (cheapest>= 0) {
        include(cluster[cheapest], rect);
        return;
    }

    if (count < maxcl) {
        cluster[count++] = rect;
        return;
    }

    // Do cheapest of:
    //     add to closest cluster
    //     do cheapest cluster merge, add to new cluster

    lowestcost = 9999999;
    cheapest = -1;
    cursor = 0;
    while(cursor < count) {
        QRect larger = cluster[cursor];
        include(larger, rect);
        int cost = larger.width()*larger.height()
                - cluster[cursor].width()*cluster[cursor].height();
        if (cost < lowestcost) {
            bool bad = false;
            for (int c = 0; c < count && !bad; c++) {
                bad = cluster[c].intersects(larger) && c!= cursor;
            }
            if (!bad) {
                cheapest = cursor;
                lowestcost = cost;
            }
        }
        cursor++;
    }

    // ###
    // could make an heuristic guess as to whether we need to bother
    // looking for a cheap merge.

    int cheapestmerge1 = -1;
    int cheapestmerge2 = -1;

    int merge1 = 0;
    while(merge1 < count) {
        int merge2 = 0;
        while(merge2 < count) {
            if(merge1!= merge2) {
                QRect larger = cluster[merge1];
                include(larger, cluster[merge2]);
                int cost = larger.width()*larger.height()
                    - cluster[merge1].width()*cluster[merge1].height()
                    - cluster[merge2].width()*cluster[merge2].height();
                if (cost < lowestcost) {
                    bool bad = false;
                    for (int c = 0; c < count && !bad; c++) {
                        bad = cluster[c].intersects(larger) && c!= cursor;
                    }
                    if (!bad) {
                        cheapestmerge1 = merge1;
                        cheapestmerge2 = merge2;
                        lowestcost = cost;
                    }
                }
            }
            merge2++;
        }
        merge1++;
    }

    if (cheapestmerge1>= 0) {
        include(cluster[cheapestmerge1], cluster[cheapestmerge2]);
        cluster[cheapestmerge2] = cluster[count--];
    } else {
        // if (!cheapest) debugRectangles(rect);
        include(cluster[cheapest], rect);
    }

    // NB: clusters do not intersect (or intersection will
    //     overwrite). This is a result of the above algorithm,
    //     given the assumption that (x, y) are ordered topleft
    //     to bottomright.

    // ###
    //
    // add explicit x/y ordering to that comment, move it to the top
    // and rephrase it as pre-/post-conditions.
}

const QRect& QtCanvasClusterizer::operator[](int i) const
{
    return cluster[i];
}

// end of clusterizer


class QtCanvasItemLess
{
public:
    inline bool operator()(const QtCanvasItem *i1, const QtCanvasItem *i2) const
    {
        if (i1->z() == i2->z())
            return i1 > i2;
        return (i1->z() > i2->z());
    }
};


class QtCanvasChunk {
public:
    QtCanvasChunk() : changed(true) { }
    // Other code assumes lists are not deleted. Assignment is also
    // done on ChunkRecs. So don't add that sort of thing here.

    void sort()
    {
        qSort(m_list.begin(), m_list.end(), QtCanvasItemLess());
    }

    const QtCanvasItemList &list() const
    {
        return m_list;
    }

    void add(QtCanvasItem* item)
    {
        m_list.prepend(item);
        changed = true;
    }

    void remove(QtCanvasItem* item)
    {
        m_list.removeAll(item);
        changed = true;
    }

    void change()
    {
        changed = true;
    }

    bool hasChanged() const
    {
        return changed;
    }

    bool takeChange()
    {
        bool y = changed;
        changed = false;
        return y;
    }

private:
    QtCanvasItemList m_list;
    bool changed;
};


static int gcd(int a, int b)
{
    int r;
    while ((r = a%b)) {
        a = b;
        b = r;
    }
    return b;
}

static int scm(int a, int b)
{
    int g = gcd(a, b);
    return a/g*b;
}



/*
    \class QtCanvas qtcanvas.h
    \brief The QtCanvas class provides a 2D area that can contain QtCanvasItem objects.

    The QtCanvas class manages its 2D graphic area and all the canvas
    items the area contains. The canvas has no visual appearance of
    its own. Instead, it is displayed on screen using a QtCanvasView.
    Multiple QtCanvasView widgets may be associated with a canvas to
    provide multiple views of the same canvas.

    The canvas is optimized for large numbers of items, particularly
    where only a small percentage of the items change at any
    one time. If the entire display changes very frequently, you should
    consider using your own custom QtScrollView subclass.

    Qt provides a rich
    set of canvas item classes, e.g. QtCanvasEllipse, QtCanvasLine,
    QtCanvasPolygon, QtCanvasPolygonalItem, QtCanvasRectangle, QtCanvasSpline,
    QtCanvasSprite and QtCanvasText. You can subclass to create your own
    canvas items; QtCanvasPolygonalItem is the most common base class used
    for this purpose.

    Items appear on the canvas after their \link QtCanvasItem::show()
    show()\endlink function has been called (or \link
    QtCanvasItem::setVisible() setVisible(true)\endlink), and \e after
    update() has been called. The canvas only shows items that are
    \link QtCanvasItem::setVisible() visible\endlink, and then only if
    \l update() is called. (By default the canvas is white and so are
    canvas items, so if nothing appears try changing colors.)

    If you created the canvas without passing a width and height to
    the constructor you must also call resize().

    Although a canvas may appear to be similar to a widget with child
    widgets, there are several notable differences:

    \list
    \i Canvas items are usually much faster to manipulate and redraw than
    child widgets, with the speed advantage becoming especially great when
    there are \e many canvas items and non-rectangular items. In most
    situations canvas items are also a lot more memory efficient than child
    widgets.

    \i It's easy to detect overlapping items (collision detection).

    \i The canvas can be larger than a widget. A million-by-million canvas
    is perfectly possible. At such a size a widget might be very
    inefficient, and some window systems might not support it at all,
    whereas QtCanvas scales well. Even with a billion pixels and a million
    items, finding a particular canvas item, detecting collisions, etc.,
    is still fast (though the memory consumption may be prohibitive
    at such extremes).

    \i Two or more QtCanvasView objects can view the same canvas.

    \i An arbitrary transformation matrix can be set on each QtCanvasView
    which makes it easy to zoom, rotate or shear the viewed canvas.

    \i Widgets provide a lot more functionality, such as input (QKeyEvent,
    QMouseEvent etc.) and layout management (QGridLayout etc.).

    \endlist

    A canvas consists of a background, a number of canvas items organized by
    x, y and z coordinates, and a foreground. A canvas item's z coordinate
    can be treated as a layer number -- canvas items with a higher z
    coordinate appear in front of canvas items with a lower z coordinate.

    The background is white by default, but can be set to a different color
    using setBackgroundColor(), or to a repeated pixmap using
    setBackgroundPixmap() or to a mosaic of smaller pixmaps using
    setTiles(). Individual tiles can be set with setTile(). There
    are corresponding get functions, e.g. backgroundColor() and
    backgroundPixmap().

    Note that QtCanvas does not inherit from QWidget, even though it has some
    functions which provide the same functionality as those in QWidget. One
    of these is setBackgroundPixmap(); some others are resize(), size(),
    width() and height(). \l QtCanvasView is the widget used to display a
    canvas on the screen.

    Canvas items are added to a canvas by constructing them and passing the
    canvas to the canvas item's constructor. An item can be moved to a
    different canvas using QtCanvasItem::setCanvas().

    Canvas items are movable (and in the case of QtCanvasSprites, animated)
    objects that inherit QtCanvasItem. Each canvas item has a position on the
    canvas (x, y coordinates) and a height (z coordinate), all of which are
    held as floating-point numbers. Moving canvas items also have x and y
    velocities. It's possible for a canvas item to be outside the canvas
    (for example QtCanvasItem::x() is greater than width()). When a canvas
    item is off the canvas, onCanvas() returns false and the canvas
    disregards the item. (Canvas items off the canvas do not slow down any
    of the common operations on the canvas.)

    Canvas items can be moved with QtCanvasItem::move(). The advance()
    function moves all QtCanvasItem::animated() canvas items and
    setAdvancePeriod() makes QtCanvas move them automatically on a periodic
    basis. In the context of the QtCanvas classes, to `animate' a canvas item
    is to set it in motion, i.e. using QtCanvasItem::setVelocity(). Animation
    of a canvas item itself, i.e. items which change over time, is enabled
    by calling QtCanvasSprite::setFrameAnimation(), or more generally by
    subclassing and reimplementing QtCanvasItem::advance(). To detect collisions
    use one of the QtCanvasItem::collisions() functions.

    The changed parts of the canvas are redrawn (if they are visible in a
    canvas view) whenever update() is called. You can either call update()
    manually after having changed the contents of the canvas, or force
    periodic updates using setUpdatePeriod(). If you have moving objects on
    the canvas, you must call advance() every time the objects should
    move one step further. Periodic calls to advance() can be forced using
    setAdvancePeriod(). The advance() function will call
    QtCanvasItem::advance() on every item that is \link
    QtCanvasItem::animated() animated\endlink and trigger an update of the
    affected areas afterwards. (A canvas item that is `animated' is simply
    a canvas item that is in motion.)

    QtCanvas organizes its canvas items into \e chunks; these are areas on
    the canvas that are used to speed up most operations. Many operations
    start by eliminating most chunks (i.e. those which haven't changed)
    and then process only the canvas items that are in the few interesting
    (i.e. changed) chunks. A valid chunk, validChunk(), is one which is on
    the canvas.

    The chunk size is a key factor to QtCanvas's speed: if there are too many
    chunks, the speed benefit of grouping canvas items into chunks is
    reduced. If the chunks are too large, it takes too long to process each
    one. The QtCanvas constructor tries to pick a suitable size, but you
    can call retune() to change it at any time. The chunkSize() function
    returns the current chunk size. The canvas items always make sure
    they're in the right chunks; all you need to make sure of is that
    the canvas uses the right chunk size. A good rule of thumb is that
    the size should be a bit smaller than the average canvas item
    size. If you have moving objects, the chunk size should be a bit
    smaller than the average size of the moving items.

    The foreground is normally nothing, but if you reimplement
    drawForeground(), you can draw things in front of all the canvas
    items.

    Areas can be set as changed with setChanged() and set unchanged with
    setUnchanged(). The entire canvas can be set as changed with
    setAllChanged(). A list of all the items on the canvas is returned by
    allItems().

    An area can be copied (painted) to a QPainter with drawArea().

    If the canvas is resized it emits the resized() signal.

    The examples/canvas application and the 2D graphics page of the
    examples/demo application demonstrate many of QtCanvas's facilities.

    \sa QtCanvasView QtCanvasItem
*/
void QtCanvas::init(int w, int h, int chunksze, int mxclusters)
{
    d = new QtCanvasData;
    awidth = w;
    aheight = h;
    chunksize = chunksze;
    maxclusters = mxclusters;
    chwidth = (w+chunksize-1)/chunksize;
    chheight = (h+chunksize-1)/chunksize;
    chunks = new QtCanvasChunk[chwidth*chheight];
    update_timer = 0;
    bgcolor = white;
    grid = 0;
    htiles = 0;
    vtiles = 0;
    debug_redraw_areas = false;
}

/*
    Create a QtCanvas with no size. \a parent is passed to the QObject
    superclass.

    \warning You \e must call resize() at some time after creation to
    be able to use the canvas.
*/
QtCanvas::QtCanvas(QObject* parent)
    : QObject(parent)
{
    init(0, 0);
}

/*
    Constructs a QtCanvas that is \a w pixels wide and \a h pixels high.
*/
QtCanvas::QtCanvas(int w, int h)
{
    init(w, h);
}

/*
    Constructs a QtCanvas which will be composed of \a h tiles
    horizontally and \a v tiles vertically. Each tile will be an image
    \a tilewidth by \a tileheight pixels taken from pixmap \a p.

    The pixmap \a p is a list of tiles, arranged left to right, (and
    in the case of pixmaps that have multiple rows of tiles, top to
    bottom), with tile 0 in the top-left corner, tile 1 next to the
    right, and so on, e.g.

    \table
    \row \i 0 \i 1 \i 2 \i 3
    \row \i 4 \i 5 \i 6 \i 7
    \endtable

    The QtCanvas is initially sized to show exactly the given number of
    tiles horizontally and vertically. If it is resized to be larger,
    the entire matrix of tiles will be repeated as often as necessary
    to cover the area. If it is smaller, tiles to the right and bottom
    will not be visible.

    \sa setTiles()
*/
QtCanvas::QtCanvas(QPixmap p,
        int h, int v, int tilewidth, int tileheight)
{
    init(h*tilewidth, v*tileheight, scm(tilewidth, tileheight));
    setTiles(p, h, v, tilewidth, tileheight);
}

/*
    Destroys the canvas and all the canvas's canvas items.
*/
QtCanvas::~QtCanvas()
{
    for (int i = 0; i < d->viewList.size(); ++i)
        d->viewList[i]->viewing = 0;
    QtCanvasItemList all = allItems();
    for (QtCanvasItemList::Iterator it = all.begin(); it!= all.end(); ++it)
        delete *it;
    delete [] chunks;
    delete [] grid;
    delete d;
}

/*
\internal
Returns the chunk at a chunk position \a i, \a j.
*/
QtCanvasChunk& QtCanvas::chunk(int i, int j) const
{
    return chunks[i+chwidth*j];
}

/*
\internal
Returns the chunk at a pixel position \a x, \a y.
*/
QtCanvasChunk& QtCanvas::chunkContaining(int x, int y) const
{
    return chunk(x/chunksize, y/chunksize);
}

/*
    Returns a list of all the items in the canvas.
*/
QtCanvasItemList QtCanvas::allItems()
{
    return d->itemDict.toList();
}


/*
    Changes the size of the canvas to have a width of \a w and a
    height of \a h. This is a slow operation.
*/
void QtCanvas::resize(int w, int h)
{
    if (awidth == w && aheight == h)
        return;

    QList<QtCanvasItem *> hidden;
    for (QSet<QtCanvasItem *>::const_iterator it = d->itemDict.begin(); it != d->itemDict.end(); ++it) {
        if ((*it)->isVisible()) {
            (*it)->hide();
            hidden.append(*it);
        }
    }

    int nchwidth = (w+chunksize-1)/chunksize;
    int nchheight = (h+chunksize-1)/chunksize;

    QtCanvasChunk* newchunks = new QtCanvasChunk[nchwidth*nchheight];

    // Commit the new values.
    //
    awidth = w;
    aheight = h;
    chwidth = nchwidth;
    chheight = nchheight;
    delete [] chunks;
    chunks = newchunks;

    for (int i = 0; i < hidden.size(); ++i)
        hidden.at(i)->show();

    setAllChanged();

    emit resized();
}

/*
    \fn void QtCanvas::resized()

    This signal is emitted whenever the canvas is resized. Each
    QtCanvasView connects to this signal to keep the scrollview's size
    correct.
*/

/*
    Change the efficiency tuning parameters to \a mxclusters clusters,
    each of size \a chunksze. This is a slow operation if there are
    many objects on the canvas.

    The canvas is divided into chunks which are rectangular areas \a
    chunksze wide by \a chunksze high. Use a chunk size which is about
    the average size of the canvas items. If you choose a chunk size
    which is too small it will increase the amount of calculation
    required when drawing since each change will affect many chunks.
    If you choose a chunk size which is too large the amount of
    drawing required will increase because for each change, a lot of
    drawing will be required since there will be many (unchanged)
    canvas items which are in the same chunk as the changed canvas
    items.

    Internally, a canvas uses a low-resolution "chunk matrix" to keep
    track of all the items in the canvas. A 64x64 chunk matrix is the
    default for a 1024x1024 pixel canvas, where each chunk collects
    canvas items in a 16x16 pixel square. This default is also
    affected by setTiles(). You can tune this default using this
    function. For example if you have a very large canvas and want to
    trade off speed for memory then you might set the chunk size to 32
    or 64.

    The \a mxclusters argument is the number of rectangular groups of
    chunks that will be separately drawn. If the canvas has a large
    number of small, dispersed items, this should be about that
    number. Our testing suggests that a large number of clusters is
    almost always best.

*/
void QtCanvas::retune(int chunksze, int mxclusters)
{
    maxclusters = mxclusters;

    if (chunksize!= chunksze) {
        QList<QtCanvasItem *> hidden;
        for (QSet<QtCanvasItem *>::const_iterator it = d->itemDict.begin(); it != d->itemDict.end(); ++it) {
            if ((*it)->isVisible()) {
                (*it)->hide();
                hidden.append(*it);
            }
        }

        chunksize = chunksze;

        int nchwidth = (awidth+chunksize-1)/chunksize;
        int nchheight = (aheight+chunksize-1)/chunksize;

        QtCanvasChunk* newchunks = new QtCanvasChunk[nchwidth*nchheight];

        // Commit the new values.
        //
        chwidth = nchwidth;
        chheight = nchheight;
        delete [] chunks;
        chunks = newchunks;

        for (int i = 0; i < hidden.size(); ++i)
            hidden.at(i)->show();
    }
}

/*
    \fn int QtCanvas::width() const

    Returns the width of the canvas, in pixels.
*/

/*
    \fn int QtCanvas::height() const

    Returns the height of the canvas, in pixels.
*/

/*
    \fn QSize QtCanvas::size() const

    Returns the size of the canvas, in pixels.
*/

/*
    \fn QRect QtCanvas::rect() const

    Returns a rectangle the size of the canvas.
*/


/*
    \fn bool QtCanvas::onCanvas(int x, int y) const

    Returns true if the pixel position (\a x, \a y) is on the canvas;
    otherwise returns false.

    \sa validChunk()
*/

/*
    \fn bool QtCanvas::onCanvas(const QPoint& p) const
    \overload

    Returns true if the pixel position \a p is on the canvas;
    otherwise returns false.

    \sa validChunk()
*/

/*
    \fn bool QtCanvas::validChunk(int x, int y) const

    Returns true if the chunk position (\a x, \a y) is on the canvas;
    otherwise returns false.

    \sa onCanvas()
*/

/*
  \fn bool QtCanvas::validChunk(const QPoint& p) const
  \overload

  Returns true if the chunk position \a p is on the canvas; otherwise
  returns false.

  \sa onCanvas()
*/

/*
    \fn int QtCanvas::chunkSize() const

    Returns the chunk size of the canvas.

    \sa retune()
*/

/*
\fn bool QtCanvas::sameChunk(int x1, int y1, int x2, int y2) const
\internal
Tells if the points (\a x1, \a y1) and (\a x2, \a y2) are within the same chunk.
*/

/*
\internal
This method adds an the item \a item to the list of QtCanvasItem objects
in the QtCanvas. The QtCanvasItem class calls this.
*/
void QtCanvas::addItem(QtCanvasItem* item)
{
    d->itemDict.insert(item);
}

/*
\internal
This method adds the item \a item to the list of QtCanvasItem objects
to be moved. The QtCanvasItem class calls this.
*/
void QtCanvas::addAnimation(QtCanvasItem* item)
{
    d->animDict.insert(item);
}

/*
\internal
This method adds the item \a item  to the list of QtCanvasItem objects
which are no longer to be moved. The QtCanvasItem class calls this.
*/
void QtCanvas::removeAnimation(QtCanvasItem* item)
{
    d->animDict.remove(item);
}

/*
\internal
This method removes the item \a item from the list of QtCanvasItem objects
in this QtCanvas. The QtCanvasItem class calls this.
*/
void QtCanvas::removeItem(QtCanvasItem* item)
{
    d->itemDict.remove(item);
}

/*
\internal
This method adds the view \a view to the list of QtCanvasView objects
viewing this QtCanvas. The QtCanvasView class calls this.
*/
void QtCanvas::addView(QtCanvasView* view)
{
    d->viewList.append(view);
    if (htiles>1 || vtiles>1 || pm.isNull()) {
        QPalette::ColorRole role = view->widget()->backgroundRole();
        QPalette viewPalette = view->widget()->palette();
        viewPalette.setColor(role, backgroundColor());
        view->widget()->setPalette(viewPalette);
    }
}

/*
\internal
This method removes the view \a view from the list of QtCanvasView objects
viewing this QtCanvas. The QtCanvasView class calls this.
*/
void QtCanvas::removeView(QtCanvasView* view)
{
    d->viewList.removeAll(view);
}

/*
    Sets the canvas to call advance() every \a ms milliseconds. Any
    previous setting by setAdvancePeriod() or setUpdatePeriod() is
    overridden.

    If \a ms is less than 0 advancing will be stopped.
*/
void QtCanvas::setAdvancePeriod(int ms)
{
    if (ms < 0) {
        if (update_timer)
            update_timer->stop();
    } else {
        if (update_timer)
            delete update_timer;
        update_timer = new QTimer(this);
        connect(update_timer, SIGNAL(timeout()), this, SLOT(advance()));
        update_timer->start(ms);
    }
}

/*
    Sets the canvas to call update() every \a ms milliseconds. Any
    previous setting by setAdvancePeriod() or setUpdatePeriod() is
    overridden.

    If \a ms is less than 0 automatic updating will be stopped.
*/
void QtCanvas::setUpdatePeriod(int ms)
{
    if (ms < 0) {
        if (update_timer)
            update_timer->stop();
    } else {
        if (update_timer)
            delete update_timer;
        update_timer = new QTimer(this);
        connect(update_timer, SIGNAL(timeout()), this, SLOT(update()));
        update_timer->start(ms);
    }
}

/*
    Moves all QtCanvasItem::animated() canvas items on the canvas and
    refreshes all changes to all views of the canvas. (An `animated'
    item is an item that is in motion; see setVelocity().)

    The advance takes place in two phases. In phase 0, the
    QtCanvasItem::advance() function of each QtCanvasItem::animated()
    canvas item is called with paramater 0. Then all these canvas
    items are called again, with parameter 1. In phase 0, the canvas
    items should not change position, merely examine other items on
    the canvas for which special processing is required, such as
    collisions between items. In phase 1, all canvas items should
    change positions, ignoring any other items on the canvas. This
    two-phase approach allows for considerations of "fairness",
    although no QtCanvasItem subclasses supplied with Qt do anything
    interesting in phase 0.

    The canvas can be configured to call this function periodically
    with setAdvancePeriod().

    \sa update()
*/
void QtCanvas::advance()
{
    QSetIterator<QtCanvasItem *> it = d->animDict;
    while (it.hasNext()) {
        QtCanvasItem *i = it.next();
        if (i)
            i->advance(0);
    }
    // we expect the dict contains the exact same items as in the
    // first pass.
    it.toFront();
    while (it.hasNext()) {
        QtCanvasItem* i = it.next();
        if (i)
            i->advance(1);
    }
    update();
}

// Don't call this unless you know what you're doing.
// p is in the content's co-ordinate example.
/*
  \internal
*/
void QtCanvas::drawViewArea(QtCanvasView* view, QPainter* p, const QRect& vr, bool)
{
    QMatrix wm = view->worldMatrix();
    QMatrix iwm = wm.inverted();
    // ivr = covers all chunks in vr
    QRect ivr = iwm.mapRect(vr);

    p->setMatrix(wm);
    drawCanvasArea(ivr, p, false);
}

/*
    Repaints changed areas in all views of the canvas.

    \sa advance()
*/
void QtCanvas::update()
{
    QRect r = changeBounds();
    for (int i = 0; i < d->viewList.size(); ++i) {
        QtCanvasView* view = d->viewList.at(i);
        if (!r.isEmpty()) {
            QRect tr = view->worldMatrix().mapRect(r);
            view->widget()->update(tr);
        }
    }
    setUnchanged(r);
}


/*
    Marks the whole canvas as changed.
    All views of the canvas will be entirely redrawn when
    update() is called next.
*/
void QtCanvas::setAllChanged()
{
    setChanged(QRect(0, 0, width(), height()));
}

/*
    Marks \a area as changed. This \a area will be redrawn in all
    views that are showing it when update() is called next.
*/
void QtCanvas::setChanged(const QRect& area)
{
    QRect thearea = area.intersected(QRect(0, 0, width(), height()));

    int mx = (thearea.x()+thearea.width()+chunksize)/chunksize;
    int my = (thearea.y()+thearea.height()+chunksize)/chunksize;
    if (mx>chwidth)
        mx = chwidth;
    if (my>chheight)
        my = chheight;

    int x = thearea.x()/chunksize;
    while(x < mx) {
        int y = thearea.y()/chunksize;
        while(y < my) {
            chunk(x, y).change();
            y++;
        }
        x++;
    }
}

/*
    Marks \a area as \e unchanged. The area will \e not be redrawn in
    the views for the next update(), unless it is marked or changed
    again before the next call to update().
*/
void QtCanvas::setUnchanged(const QRect& area)
{
    QRect thearea = area.intersected(QRect(0, 0, width(), height()));

    int mx = (thearea.x()+thearea.width()+chunksize)/chunksize;
    int my = (thearea.y()+thearea.height()+chunksize)/chunksize;
    if (mx>chwidth)
        mx = chwidth;
    if (my>chheight)
        my = chheight;

    int x = thearea.x()/chunksize;
    while(x < mx) {
        int y = thearea.y()/chunksize;
        while(y < my) {
            chunk(x, y).takeChange();
            y++;
        }
        x++;
    }
}


/*
  \internal
*/
QRect QtCanvas::changeBounds()
{
    QRect area = QRect(0, 0, width(), height());

    int mx = (area.x()+area.width()+chunksize)/chunksize;
    int my = (area.y()+area.height()+chunksize)/chunksize;
    if (mx > chwidth)
        mx = chwidth;
    if (my > chheight)
        my = chheight;

    QRect result;

    int x = area.x()/chunksize;
    while(x < mx) {
        int y = area.y()/chunksize;
        while(y < my) {
            QtCanvasChunk& ch = chunk(x, y);
            if (ch.hasChanged())
                result |= QRect(x*chunksize, y*chunksize, chunksize + 1, chunksize + 1);
            y++;
        }
        x++;
    }

    return result;
}

/*
    Paints all canvas items that are in the area \a clip to \a
    painter, using double-buffering if \a dbuf is true.

    e.g. to print the canvas to a printer:
    \code
    QPrinter pr;
    if (pr.setup()) {
        QPainter p(&pr);
        canvas.drawArea(canvas.rect(), &p);
    }
    \endcode
*/
void QtCanvas::drawArea(const QRect& clip, QPainter* painter, bool dbuf)
{
    if (painter)
        drawCanvasArea(clip, painter, dbuf);
}

#include <QDebug>
/*
  \internal
*/
void QtCanvas::drawCanvasArea(const QRect& inarea, QPainter* p, bool /*double_buffer*/)
{
    QRect area = inarea.intersected(QRect(0, 0, width(), height()));

    if (!p) return; // Nothing to do.

    int lx = area.x()/chunksize;
    int ly = area.y()/chunksize;
    int mx = area.right()/chunksize;
    int my = area.bottom()/chunksize;
    if (mx>= chwidth)
        mx = chwidth-1;
    if (my>= chheight)
        my = chheight-1;

    QtCanvasItemList allvisible;

    // Stores the region within area that need to be drawn. It is relative
    // to area.topLeft()  (so as to keep within bounds of 16-bit XRegions)
    QRegion rgn;

    for (int x = lx; x <= mx; x++) {
        for (int y = ly; y <= my; y++) {
            // Only reset change if all views updating, and
            // wholy within area. (conservative:  ignore entire boundary)
            //
            // Disable this to help debugging.
            //
            if (!p) {
                if (chunk(x, y).takeChange()) {
                    // ### should at least make bands
                    rgn |= QRegion(x*chunksize-area.x(), y*chunksize-area.y(),
                                   chunksize, chunksize);
                    allvisible += chunk(x, y).list();
                }
            } else {
                allvisible += chunk(x, y).list();
            }
        }
    }
    qSort(allvisible.begin(), allvisible.end(), QtCanvasItemLess());

    drawBackground(*p, area);
    if (!allvisible.isEmpty()) {
        QtCanvasItem* prev = 0;
        for (int i = allvisible.size() - 1; i >= 0; --i) {
            QtCanvasItem *g = allvisible[i];
            if (g != prev) {
                g->draw(*p);
                prev = g;
            }
        }
    }

    drawForeground(*p, area);
}

/*
\internal
This method to informs the QtCanvas that a given chunk is
`dirty' and needs to be redrawn in the next Update.

(\a x, \a y) is a chunk location.

The sprite classes call this. Any new derived class of QtCanvasItem
must do so too. SetChangedChunkContaining can be used instead.
*/
void QtCanvas::setChangedChunk(int x, int y)
{
    if (validChunk(x, y)) {
        QtCanvasChunk& ch = chunk(x, y);
        ch.change();
    }
}

/*
\internal
This method to informs the QtCanvas that the chunk containing a given
pixel is `dirty' and needs to be redrawn in the next Update.

(\a x, \a y) is a pixel location.

The item classes call this. Any new derived class of QtCanvasItem must
do so too. SetChangedChunk can be used instead.
*/
void QtCanvas::setChangedChunkContaining(int x, int y)
{
    if (x>= 0 && x < width() && y>= 0 && y < height()) {
        QtCanvasChunk& chunk = chunkContaining(x, y);
        chunk.change();
    }
}

/*
\internal
This method adds the QtCanvasItem \a g to the list of those which need to be
drawn if the given chunk at location (\a x, \a y) is redrawn. Like
SetChangedChunk and SetChangedChunkContaining, this method marks the
chunk as `dirty'.
*/
void QtCanvas::addItemToChunk(QtCanvasItem* g, int x, int y)
{
    if (validChunk(x, y)) {
        chunk(x, y).add(g);
    }
}

/*
\internal
This method removes the QtCanvasItem \a g from the list of those which need to
be drawn if the given chunk at location (\a x, \a y) is redrawn. Like
SetChangedChunk and SetChangedChunkContaining, this method marks the chunk
as `dirty'.
*/
void QtCanvas::removeItemFromChunk(QtCanvasItem* g, int x, int y)
{
    if (validChunk(x, y)) {
        chunk(x, y).remove(g);
    }
}


/*
\internal
This method adds the QtCanvasItem \a g to the list of those which need to be
drawn if the chunk containing the given pixel (\a x, \a y) is redrawn. Like
SetChangedChunk and SetChangedChunkContaining, this method marks the
chunk as `dirty'.
*/
void QtCanvas::addItemToChunkContaining(QtCanvasItem* g, int x, int y)
{
    if (x>= 0 && x < width() && y>= 0 && y < height()) {
        chunkContaining(x, y).add(g);
    }
}

/*
\internal
This method removes the QtCanvasItem \a g from the list of those which need to
be drawn if the chunk containing the given pixel (\a x, \a y) is redrawn.
Like SetChangedChunk and SetChangedChunkContaining, this method
marks the chunk as `dirty'.
*/
void QtCanvas::removeItemFromChunkContaining(QtCanvasItem* g, int x, int y)
{
    if (x>= 0 && x < width() && y>= 0 && y < height()) {
        chunkContaining(x, y).remove(g);
    }
}

/*
    Returns the color set by setBackgroundColor(). By default, this is
    white.

    This function is not a reimplementation of
    QWidget::backgroundColor() (QtCanvas is not a subclass of QWidget),
    but all QtCanvasViews that are viewing the canvas will set their
    backgrounds to this color.

    \sa setBackgroundColor(), backgroundPixmap()
*/
QColor QtCanvas::backgroundColor() const
{
    return bgcolor;
}

/*
    Sets the solid background to be the color \a c.

    \sa backgroundColor(), setBackgroundPixmap(), setTiles()
*/
void QtCanvas::setBackgroundColor(const QColor& c)
{
    if (bgcolor != c) {
        bgcolor = c;
        for (int i = 0; i < d->viewList.size(); ++i) {
            QtCanvasView *view = d->viewList.at(i);
            QPalette::ColorRole role = view->widget()->backgroundRole();
            QPalette viewPalette = view->widget()->palette();
            viewPalette.setColor(role, bgcolor);
            view->widget()->setPalette(viewPalette);
        }
        setAllChanged();
    }
}

/*
    Returns the pixmap set by setBackgroundPixmap(). By default,
    this is a null pixmap.

    \sa setBackgroundPixmap(), backgroundColor()
*/
QPixmap QtCanvas::backgroundPixmap() const
{
    return pm;
}

/*
    Sets the solid background to be the pixmap \a p repeated as
    necessary to cover the entire canvas.

    \sa backgroundPixmap(), setBackgroundColor(), setTiles()
*/
void QtCanvas::setBackgroundPixmap(const QPixmap& p)
{
    setTiles(p, 1, 1, p.width(), p.height());
    for (int i = 0; i < d->viewList.size(); ++i) {
        QtCanvasView* view = d->viewList.at(i);
        view->widget()->update();
    }
}

/*
    This virtual function is called for all updates of the canvas. It
    renders any background graphics using the painter \a painter, in
    the area \a clip. If the canvas has a background pixmap or a tiled
    background, that graphic is used, otherwise the canvas is cleared
    using the background color.

    If the graphics for an area change, you must explicitly call
    setChanged(const QRect&) for the result to be visible when
    update() is next called.

    \sa setBackgroundColor(), setBackgroundPixmap(), setTiles()
*/
void QtCanvas::drawBackground(QPainter& painter, const QRect& clip)
{
    if (pm.isNull()) {
        painter.fillRect(clip, bgcolor);
    } else if (!grid) {
        for (int x = clip.x()/pm.width();
            x < (clip.x()+clip.width()+pm.width()-1)/pm.width(); x++)
        {
            for (int y = clip.y()/pm.height();
                y < (clip.y()+clip.height()+pm.height()-1)/pm.height(); y++)
            {
                painter.drawPixmap(x*pm.width(), y*pm.height(), pm);
            }
        }
    } else {
        const int x1 = clip.left()/tilew;
        int x2 = clip.right()/tilew;
        const int y1 = clip.top()/tileh;
        int y2 = clip.bottom()/tileh;

        const int roww = pm.width()/tilew;

        for (int j = y1; j <= y2; j++) {
            int jj = j%tilesVertically();
            for (int i = x1; i <= x2; i++) {
                int t = tile(i%tilesHorizontally(), jj);
                int tx = t % roww;
                int ty = t / roww;
                painter.drawPixmap(i*tilew, j*tileh, pm, 
                                tx*tilew, ty*tileh, tilew, tileh);
            }
        }
    }
}

/*
    This virtual function is called for all updates of the canvas. It
    renders any foreground graphics using the painter \a painter, in
    the area \a clip.

    If the graphics for an area change, you must explicitly call
    setChanged(const QRect&) for the result to be visible when
    update() is next called.

    The default is to draw nothing.
*/
void QtCanvas::drawForeground(QPainter& painter, const QRect& clip)
{
    if (debug_redraw_areas) {
        painter.setPen(red);
        painter.setBrush(NoBrush);
        painter.drawRect(clip);
    }
}

/*
    Sets the QtCanvas to be composed of \a h tiles horizontally and \a
    v tiles vertically. Each tile will be an image \a tilewidth by \a
    tileheight pixels from pixmap \a p.

    The pixmap \a p is a list of tiles, arranged left to right, (and
    in the case of pixmaps that have multiple rows of tiles, top to
    bottom), with tile 0 in the top-left corner, tile 1 next to the
    right, and so on, e.g.

    \table
    \row \i 0 \i 1 \i 2 \i 3
    \row \i 4 \i 5 \i 6 \i 7
    \endtable

    If the canvas is larger than the matrix of tiles, the entire
    matrix is repeated as necessary to cover the whole canvas. If it
    is smaller, tiles to the right and bottom are not visible.

    The width and height of \a p must be a multiple of \a tilewidth
    and \a tileheight. If they are not the function will do nothing.

    If you want to unset any tiling set, then just pass in a null
    pixmap and 0 for \a h, \a v, \a tilewidth, and
    \a tileheight.
*/
void QtCanvas::setTiles(QPixmap p, 
                        int h, int v, int tilewidth, int tileheight)
{
    if (!p.isNull() && (!tilewidth || !tileheight ||
         p.width() % tilewidth != 0 || p.height() % tileheight != 0))
        return;

    htiles = h;
    vtiles = v;
    delete[] grid;
    pm = p;
    if (h && v && !p.isNull()) {
        grid = new ushort[h*v];
        memset(grid, 0, h*v*sizeof(ushort));
        tilew = tilewidth;
        tileh = tileheight;
    } else {
        grid = 0;
    }
    if (h + v > 10) {
        int s = scm(tilewidth, tileheight);
        retune(s < 128 ? s : qMax(tilewidth, tileheight));
    }
    setAllChanged();
}

/*
    \fn int QtCanvas::tile(int x, int y) const

    Returns the tile at position (\a x, \a y). Initially, all tiles
    are 0.

    The parameters must be within range, i.e.
        0 \< \a x \< tilesHorizontally() and
        0 \< \a y \< tilesVertically().

    \sa setTile()
*/

/*
    \fn int QtCanvas::tilesHorizontally() const

    Returns the number of tiles horizontally.
*/

/*
    \fn int QtCanvas::tilesVertically() const

    Returns the number of tiles vertically.
*/

/*
    \fn int QtCanvas::tileWidth() const

    Returns the width of each tile.
*/

/*
    \fn int QtCanvas::tileHeight() const

    Returns the height of each tile.
*/


/*
    Sets the tile at (\a x, \a y) to use tile number \a tilenum, which
    is an index into the tile pixmaps. The canvas will update
    appropriately when update() is next called.

    The images are taken from the pixmap set by setTiles() and are
    arranged left to right, (and in the case of pixmaps that have
    multiple rows of tiles, top to bottom), with tile 0 in the
    top-left corner, tile 1 next to the right, and so on, e.g.

    \table
    \row \i 0 \i 1 \i 2 \i 3
    \row \i 4 \i 5 \i 6 \i 7
    \endtable

    \sa tile() setTiles()
*/
void QtCanvas::setTile(int x, int y, int tilenum)
{
    ushort& t = grid[x+y*htiles];
    if (t != tilenum) {
        t = tilenum;
        if (tilew == tileh && tilew == chunksize)
            setChangedChunk(x, y);          // common case
        else
            setChanged(QRect(x*tilew, y*tileh, tilew, tileh));
    }
}


// lesser-used data in canvas item, plus room for extension.
// Be careful adding to this - check all usages.
class QtCanvasItemExtra {
    QtCanvasItemExtra() : vx(0.0), vy(0.0) { }
    double vx, vy;
    friend class QtCanvasItem;
};


/*
    \class QtCanvasItem qtcanvas.h
    \brief The QtCanvasItem class provides an abstract graphic object on a QtCanvas.

    A variety of QtCanvasItem subclasses provide immediately usable
    behaviour. This class is a pure abstract superclass providing the
    behaviour that is shared among all the concrete canvas item classes.
    QtCanvasItem is not intended for direct subclassing. It is much easier
    to subclass one of its subclasses, e.g. QtCanvasPolygonalItem (the
    commonest base class), QtCanvasRectangle, QtCanvasSprite, QtCanvasEllipse
    or QtCanvasText.

    Canvas items are added to a canvas by constructing them and passing the
    canvas to the canvas item's constructor. An item can be moved to a
    different canvas using setCanvas().

    Items appear on the canvas after their \link show() show()\endlink
    function has been called (or \link setVisible()
    setVisible(true)\endlink), and \e after update() has been called. The
    canvas only shows items that are \link setVisible() visible\endlink,
    and then only if \l update() is called. If you created the canvas
    without passing a width and height to the constructor you'll also need
    to call \link QtCanvas::resize() resize()\endlink. Since the canvas
    background defaults to white and canvas items default to white,
    you may need to change colors to see your items.

    A QtCanvasItem object can be moved in the x(), y() and z() dimensions
    using functions such as move(), moveBy(), setX(), setY() and setZ(). A
    canvas item can be set in motion, `animated', using setAnimated() and
    given a velocity in the x and y directions with setXVelocity() and
    setYVelocity() -- the same effect can be achieved by calling
    setVelocity(). Use the collidesWith() function to see if the canvas item
    will collide on the \e next advance(1) and use collisions() to see what
    collisions have occurred.

    Use QtCanvasSprite or your own subclass of QtCanvasSprite to create canvas
    items which are animated, i.e. which change over time.

    The size of a canvas item is given by boundingRect(). Use
    boundingRectAdvanced() to see what the size of the canvas item will be
    \e after the next advance(1) call.

    The rtti() function is used for identifying subclasses of QtCanvasItem.
    The canvas() function returns a pointer to the canvas which contains the
    canvas item.

    QtCanvasItem provides the show() and isVisible() functions like those in
    QWidget.

    QtCanvasItem also provides the setEnabled(), setActive() and
    setSelected() functions; these functions set the relevant boolean and
    cause a repaint but the boolean values they set are not used in
    QtCanvasItem itself. You can make use of these booleans in your subclasses.

    By default, canvas items have no velocity, no size, and are not in
    motion. The subclasses provided in Qt do not change these defaults
    except where noted.

*/

/*
    \enum QtCanvasItem::RttiValues

    This enum is used to name the different types of canvas item.

    \value Rtti_Item Canvas item abstract base class
    \value Rtti_Ellipse
    \value Rtti_Line
    \value Rtti_Polygon
    \value Rtti_PolygonalItem
    \value Rtti_Rectangle
    \value Rtti_Spline
    \value Rtti_Sprite
    \value Rtti_Text

*/

/*
    \fn void QtCanvasItem::update()

    Call this function to repaint the canvas's changed chunks.
*/

/*
    Constructs a QtCanvasItem on canvas \a canvas.

    \sa setCanvas()
*/
QtCanvasItem::QtCanvasItem(QtCanvas* canvas) :
    cnv(canvas),
    myx(0), myy(0), myz(0)
{
    ani = 0;
    vis = 0;
    val = 0;
    sel = 0;
    ena = 0;
    act = 0;

    ext = 0;
    if (cnv) cnv->addItem(this);
}

/*
    Destroys the QtCanvasItem and removes it from its canvas.
*/
QtCanvasItem::~QtCanvasItem()
{
    if (cnv) {
        cnv->removeItem(this);
        cnv->removeAnimation(this);
    }
    delete ext;
}

QtCanvasItemExtra& QtCanvasItem::extra()
{
    if (!ext)
        ext = new QtCanvasItemExtra;
    return *ext;
}

/*
    \fn double QtCanvasItem::x() const

    Returns the horizontal position of the canvas item. Note that
    subclasses often have an origin other than the top-left corner.
*/

/*
    \fn double QtCanvasItem::y() const

    Returns the vertical position of the canvas item. Note that
    subclasses often have an origin other than the top-left corner.
*/

/*
    \fn double QtCanvasItem::z() const

    Returns the z index of the canvas item, which is used for visual
    order: higher-z items obscure (are in front of) lower-z items.
*/

/*
    \fn void QtCanvasItem::setX(double x)

    Moves the canvas item so that its x-position is \a x.

    \sa x(), move()
*/

/*
    \fn void QtCanvasItem::setY(double y)

    Moves the canvas item so that its y-position is \a y.

    \sa y(), move()
*/

/*
    \fn void QtCanvasItem::setZ(double z)

    Sets the z index of the canvas item to \a z. Higher-z items
    obscure (are in front of) lower-z items.

    \sa z(), move()
*/


/*
    Moves the canvas item relative to its current position by (\a dx,
    \a dy).
*/
void QtCanvasItem::moveBy(double dx, double dy)
{
    if (dx || dy) {
        removeFromChunks();
        myx += dx;
        myy += dy;
        addToChunks();
    }
}


/*
    Moves the canvas item to the absolute position (\a x, \a y).
*/
void QtCanvasItem::move(double x, double y)
{
    moveBy(x-myx, y-myy);
}


/*
    Returns true if the canvas item is in motion; otherwise returns
    false.

    \sa setVelocity(), setAnimated()
*/
bool QtCanvasItem::animated() const
{
    return (bool)ani;
}

/*
    Sets the canvas item to be in motion if \a y is true, or not if \a
    y is false. The speed and direction of the motion is set with
    setVelocity(), or with setXVelocity() and setYVelocity().

    \sa advance(), QtCanvas::advance()
*/
void QtCanvasItem::setAnimated(bool y)
{
    if (y != (bool)ani) {
        ani = (uint)y;
        if (y) {
            cnv->addAnimation(this);
        } else {
            cnv->removeAnimation(this);
        }
    }
}

/*
    \fn void QtCanvasItem::setXVelocity(double vx)

    Sets the horizontal component of the canvas item's velocity to \a vx.

    \sa setYVelocity() setVelocity()
*/

/*
    \fn void QtCanvasItem::setYVelocity(double vy)

    Sets the vertical component of the canvas item's velocity to \a vy.

    \sa setXVelocity() setVelocity()
*/

/*
    Sets the canvas item to be in motion, moving by \a vx and \a vy
    pixels in the horizontal and vertical directions respectively.

    \sa advance() setXVelocity() setYVelocity()
*/
void QtCanvasItem::setVelocity(double vx, double vy)
{
    if (ext || vx!= 0.0 || vy!= 0.0) {
        if (!ani)
            setAnimated(true);
        extra().vx = vx;
        extra().vy = vy;
    }
}

/*
    Returns the horizontal velocity component of the canvas item.
*/
double QtCanvasItem::xVelocity() const
{
    return ext ? ext->vx : 0;
}

/*
    Returns the vertical velocity component of the canvas item.
*/
double QtCanvasItem::yVelocity() const
{
    return ext ? ext->vy : 0;
}

/*
    The default implementation moves the canvas item, if it is
    animated(), by the preset velocity if \a phase is 1, and does
    nothing if \a phase is 0.

    Note that if you reimplement this function, the reimplementation
    must not change the canvas in any way, for example it must not add
    or remove items.

    \sa QtCanvas::advance() setVelocity()
*/
void QtCanvasItem::advance(int phase)
{
    if (ext && phase == 1)
        moveBy(ext->vx, ext->vy);
}

/*
    \fn void QtCanvasItem::draw(QPainter& painter)

    This abstract virtual function draws the canvas item using \a painter.
*/

/*
    Sets the QtCanvas upon which the canvas item is to be drawn to \a c.

    \sa canvas()
*/
void QtCanvasItem::setCanvas(QtCanvas* c)
{
    bool v = isVisible();
    setVisible(false);
    if (cnv) {
        if (ext)
            cnv->removeAnimation(this);
        cnv->removeItem(this);
    }
    cnv = c;
    if (cnv) {
        cnv->addItem(this);
        if (ext)
            cnv->addAnimation(this);
    }
    setVisible(v);
}

/*
    \fn QtCanvas* QtCanvasItem::canvas() const

    Returns the canvas containing the canvas item.
*/

/* Shorthand for setVisible(true). */
void QtCanvasItem::show()
{
    setVisible(true);
}

/* Shorthand for setVisible(false). */
void QtCanvasItem::hide()
{
    setVisible(false);
}

/*
    Makes the canvas item visible if \a yes is true, or invisible if
    \a yes is false. The change takes effect when QtCanvas::update() is
    next called.
*/
void QtCanvasItem::setVisible(bool yes)
{
    if ((bool)vis!= yes) {
        if (yes) {
            vis = (uint)yes;
            addToChunks();
        } else {
            removeFromChunks();
            vis = (uint)yes;
        }
    }
}
/*
    \obsolete
    \fn bool QtCanvasItem::visible() const
    Use isVisible() instead.
*/

/*
    \fn bool QtCanvasItem::isVisible() const

    Returns true if the canvas item is visible; otherwise returns
    false.

    Note that in this context true does \e not mean that the canvas
    item is currently in a view, merely that if a view is showing the
    area where the canvas item is positioned, and the item is not
    obscured by items with higher z values, and the view is not
    obscured by overlaying windows, it would be visible.

    \sa setVisible(), z()
*/

/*
    \obsolete
    \fn bool QtCanvasItem::selected() const
    Use isSelected() instead.
*/

/*
    \fn bool QtCanvasItem::isSelected() const

    Returns true if the canvas item is selected; otherwise returns false.
*/

/*
    Sets the selected flag of the item to \a yes. If this changes the
    item's selected state the item will be redrawn when
    QtCanvas::update() is next called.

    The QtCanvas, QtCanvasItem and the Qt-supplied QtCanvasItem
    subclasses do not make use of this value. The setSelected()
    function is supplied because many applications need it, but it is
    up to you how you use the isSelected() value.
*/
void QtCanvasItem::setSelected(bool yes)
{
    if ((bool)sel!= yes) {
        sel = (uint)yes;
        changeChunks();
    }
}

/*
    \obsolete
    \fn bool QtCanvasItem::enabled() const
    Use isEnabled() instead.
*/

/*
    \fn bool QtCanvasItem::isEnabled() const

    Returns true if the QtCanvasItem is enabled; otherwise returns false.
*/

/*
    Sets the enabled flag of the item to \a yes. If this changes the
    item's enabled state the item will be redrawn when
    QtCanvas::update() is next called.

    The QtCanvas, QtCanvasItem and the Qt-supplied QtCanvasItem
    subclasses do not make use of this value. The setEnabled()
    function is supplied because many applications need it, but it is
    up to you how you use the isEnabled() value.
*/
void QtCanvasItem::setEnabled(bool yes)
{
    if (ena!= (uint)yes) {
        ena = (uint)yes;
        changeChunks();
    }
}

/*
    \obsolete
    \fn bool QtCanvasItem::active() const
    Use isActive() instead.
*/

/*
    \fn bool QtCanvasItem::isActive() const

    Returns true if the QtCanvasItem is active; otherwise returns false.
*/

/*
    Sets the active flag of the item to \a yes. If this changes the
    item's active state the item will be redrawn when
    QtCanvas::update() is next called.

    The QtCanvas, QtCanvasItem and the Qt-supplied QtCanvasItem
    subclasses do not make use of this value. The setActive() function
    is supplied because many applications need it, but it is up to you
    how you use the isActive() value.
*/
void QtCanvasItem::setActive(bool yes)
{
    if (act!= (uint)yes) {
        act = (uint)yes;
        changeChunks();
    }
}

bool qt_testCollision(const QtCanvasSprite* s1, const QtCanvasSprite* s2)
{
    const QImage* s2image = s2->imageAdvanced()->collision_mask;
    QRect s2area = s2->boundingRectAdvanced();

    QRect cyourarea(s2area.x(), s2area.y(),
            s2area.width(), s2area.height());

    QImage* s1image = s1->imageAdvanced()->collision_mask;

    QRect s1area = s1->boundingRectAdvanced();

    QRect ourarea = s1area.intersected(cyourarea);

    if (ourarea.isEmpty())
        return false;

    int x2 = ourarea.x()-cyourarea.x();
    int y2 = ourarea.y()-cyourarea.y();
    int x1 = ourarea.x()-s1area.x();
    int y1 = ourarea.y()-s1area.y();
    int w = ourarea.width();
    int h = ourarea.height();

    if (!s2image) {
        if (!s1image)
            return w>0 && h>0;
        // swap everything around
        int t;
        t = x1; x1 = x2; x2 = t;
        t = y1; x1 = y2; y2 = t;
        s2image = s1image;
        s1image = 0;
    }

    // s2image != 0

    // A non-linear search may be more efficient.
    // Perhaps spiralling out from the center, or a simpler
    // vertical expansion from the centreline.

    // We assume that sprite masks don't have
    // different bit orders.
    //
    // Q_ASSERT(s1image->bitOrder() == s2image->bitOrder());

    if (s1image) {
        if (s1image->format() == QImage::Format_MonoLSB) {
            for (int j = 0; j < h; j++) {
                uchar* ml = s1image->scanLine(y1+j);
                const uchar* yl = s2image->scanLine(y2+j);
                for (int i = 0; i < w; i++) {
                    if (*(yl + ((x2+i) >> 3)) & (1 << ((x2+i) & 7))
                    && *(ml + ((x1+i) >> 3)) & (1 << ((x1+i) & 7)))
                    {
                        return true;
                    }
                }
            }
        } else {
            for (int j = 0; j < h; j++) {
                uchar* ml = s1image->scanLine(y1+j);
                const uchar* yl = s2image->scanLine(y2+j);
                for (int i = 0; i < w; i++) {
                    if (*(yl + ((x2+i) >> 3)) & (1 << (7-((x2+i) & 7)))
                    && *(ml + ((x1+i) >> 3)) & (1 << (7-((x1+i) & 7))))
                    {
                        return true;
                    }
                }
            }
        }
    } else {
        if (s2image->format() == QImage::Format_MonoLSB) {
            for (int j = 0; j < h; j++) {
                const uchar* yl = s2image->scanLine(y2+j);
                for (int i = 0; i < w; i++) {
                    if (*(yl + ((x2+i) >> 3)) & (1 << ((x2+i) & 7)))
                    {
                        return true;
                    }
                }
            }
        } else {
            for (int j = 0; j< h; j++) {
                const uchar* yl = s2image->scanLine(y2+j);
                for (int i = 0; i < w; i++) {
                    if (*(yl + ((x2+i) >> 3)) & (1 << (7-((x2+i) & 7))))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

static bool collision_double_dispatch(const QtCanvasSprite* s1,
                                       const QtCanvasPolygonalItem* p1,
                                       const QtCanvasRectangle* r1,
                                       const QtCanvasEllipse* e1,
                                       const QtCanvasText* t1,
                                       const QtCanvasSprite* s2,
                                       const QtCanvasPolygonalItem* p2,
                                       const QtCanvasRectangle* r2,
                                       const QtCanvasEllipse* e2,
                                       const QtCanvasText* t2)
{
    const QtCanvasItem* i1 = s1 ?
                            (const QtCanvasItem*)s1 : p1 ?
                            (const QtCanvasItem*)p1 : r1 ?
                            (const QtCanvasItem*)r1 : e1 ?
                            (const QtCanvasItem*)e1 : (const QtCanvasItem*)t1;
    const QtCanvasItem* i2 = s2 ?
                            (const QtCanvasItem*)s2 : p2 ?
                            (const QtCanvasItem*)p2 : r2 ?
                            (const QtCanvasItem*)r2 : e2 ?
                            (const QtCanvasItem*)e2 : (const QtCanvasItem*)t2;

    if (s1 && s2) {
        // a
        return qt_testCollision(s1, s2);
    } else if ((r1 || t1 || s1) && (r2 || t2 || s2)) {
        // b
        QRect rc1 = i1->boundingRectAdvanced();
        QRect rc2 = i2->boundingRectAdvanced();
        return rc1.intersects(rc2);
    } else if (e1 && e2
                && e1->angleLength()>= 360*16 && e2->angleLength()>= 360*16
                && e1->width() == e1->height()
                && e2->width() == e2->height()) {
        // c
        double xd = (e1->x()+e1->xVelocity())-(e2->x()+e1->xVelocity());
        double yd = (e1->y()+e1->yVelocity())-(e2->y()+e1->yVelocity());
        double rd = (e1->width()+e2->width())/2;
        return xd*xd+yd*yd <= rd*rd;
    } else if (p1 && (p2 || s2 || t2)) {
        // d
        QPolygon pa1 = p1->areaPointsAdvanced();
        QPolygon pa2 = p2 ? p2->areaPointsAdvanced()
                          : QPolygon(i2->boundingRectAdvanced());
        bool col = !(QRegion(pa1) & QRegion(pa2, Qt::WindingFill)).isEmpty();

        return col;
    } else {
        return collision_double_dispatch(s2, p2, r2, e2, t2, 
                                         s1, p1, r1, e1, t1);
    }
}

/*
    \fn bool QtCanvasItem::collidesWith(const QtCanvasItem* other) const

    Returns true if the canvas item will collide with the \a other
    item \e after they have moved by their current velocities;
    otherwise returns false.

    \sa collisions()
*/


/*
    \class QtCanvasSprite qtcanvas.h
    \brief The QtCanvasSprite class provides an animated canvas item on a QtCanvas.

    A canvas sprite is an object which can contain any number of images
    (referred to as frames), only one of which is current, i.e.
    displayed, at any one time. The images can be passed in the
    constructor or set or changed later with setSequence(). If you
    subclass QtCanvasSprite you can change the frame that is displayed
    periodically, e.g. whenever QtCanvasItem::advance(1) is called to
    create the effect of animation.

    The current frame can be set with setFrame() or with move(). The
    number of frames available is given by frameCount(). The bounding
    rectangle of the current frame is returned by boundingRect().

    The current frame's image can be retrieved with image(); use
    imageAdvanced() to retrieve the image for the frame that will be
    shown after advance(1) is called. Use the image() overload passing
    it an integer index to retrieve a particular image from the list of
    frames.

    Use width() and height() to retrieve the dimensions of the current
    frame.

    Use leftEdge() and rightEdge() to retrieve the current frame's
    left-hand and right-hand x-coordinates respectively. Use
    bottomEdge() and topEdge() to retrieve the current frame's bottom
    and top y-coordinates respectively. These functions have an overload
    which will accept an integer frame number to retrieve the
    coordinates of a particular frame.

    QtCanvasSprite draws very quickly, at the expense of memory.

    The current frame's image can be drawn on a painter with draw().

    Like any other canvas item, canvas sprites can be moved with
    move() which sets the x and y coordinates and the frame number, as
    well as with QtCanvasItem::move() and QtCanvasItem::moveBy(), or by
    setting coordinates with QtCanvasItem::setX(), QtCanvasItem::setY()
    and QtCanvasItem::setZ().

*/


/*
  \reimp
*/
bool QtCanvasSprite::collidesWith(const QtCanvasItem* i) const
{
    return i->collidesWith(this, 0, 0, 0, 0);
}

/*
    Returns true if the canvas item collides with any of the given
    items; otherwise returns false. The parameters, \a s, \a p, \a r,
    \a e and \a t, are all the same object, this is just a type
    resolution trick.
*/
bool QtCanvasSprite::collidesWith(const QtCanvasSprite* s,
                                  const QtCanvasPolygonalItem* p,
                                  const QtCanvasRectangle* r,
                                  const QtCanvasEllipse* e,
                                  const QtCanvasText* t) const
{
    return collision_double_dispatch(s, p, r, e, t, this, 0, 0, 0, 0);
}

/*
  \reimp
*/
bool QtCanvasPolygonalItem::collidesWith(const QtCanvasItem* i) const
{
    return i->collidesWith(0, this, 0, 0, 0);
}

bool QtCanvasPolygonalItem::collidesWith(const QtCanvasSprite* s,
                                 const QtCanvasPolygonalItem* p,
                                 const QtCanvasRectangle* r,
                                 const QtCanvasEllipse* e,
                                 const QtCanvasText* t) const
{
    return collision_double_dispatch(s, p, r, e, t, 0, this, 0, 0, 0);
}

/*
  \reimp
*/
bool QtCanvasRectangle::collidesWith(const QtCanvasItem* i) const
{
    return i->collidesWith(0, this, this, 0, 0);
}

bool QtCanvasRectangle::collidesWith(const QtCanvasSprite* s,
                                 const QtCanvasPolygonalItem* p,
                                 const QtCanvasRectangle* r,
                                 const QtCanvasEllipse* e,
                                 const QtCanvasText* t) const
{
    return collision_double_dispatch(s, p, r, e, t, 0, this, this, 0, 0);
}


/*
  \reimp
*/
bool QtCanvasEllipse::collidesWith(const QtCanvasItem* i) const
{
    return i->collidesWith(0,this, 0, this, 0);
}

bool QtCanvasEllipse::collidesWith(const QtCanvasSprite* s,
                                 const QtCanvasPolygonalItem* p,
                                 const QtCanvasRectangle* r,
                                 const QtCanvasEllipse* e,
                                 const QtCanvasText* t) const
{
    return collision_double_dispatch(s, p, r, e, t, 0, this, 0, this, 0);
}

/*
  \reimp
*/
bool QtCanvasText::collidesWith(const QtCanvasItem* i) const
{
    return i->collidesWith(0, 0, 0, 0, this);
}

bool QtCanvasText::collidesWith(const QtCanvasSprite* s,
                                 const QtCanvasPolygonalItem* p,
                                 const QtCanvasRectangle* r,
                                 const QtCanvasEllipse* e,
                                 const QtCanvasText* t) const
{
    return collision_double_dispatch(s, p, r, e, t, 0, 0, 0, 0, this);
}

/*
    Returns the list of canvas items that this canvas item has
    collided with.

    A collision is generally defined as occurring when the pixels of
    one item draw on the pixels of another item, but not all
    subclasses are so precise. Also, since pixel-wise collision
    detection can be slow, this function works in either exact or
    inexact mode, according to the \a exact parameter.

    If \a exact is true, the canvas items returned have been
    accurately tested for collision with the canvas item.

    If \a exact is false, the canvas items returned are \e near the
    canvas item. You can test the canvas items returned using
    collidesWith() if any are interesting collision candidates. By
    using this approach, you can ignore some canvas items for which
    collisions are not relevant.

    The returned list is a list of QtCanvasItems, but often you will
    need to cast the items to their subclass types. The safe way to do
    this is to use rtti() before casting. This provides some of the
    functionality of the standard C++ dynamic cast operation even on
    compilers where dynamic casts are not available.

    Note that a canvas item may be `on' a canvas, e.g. it was created
    with the canvas as parameter, even though its coordinates place it
    beyond the edge of the canvas's area. Collision detection only
    works for canvas items which are wholly or partly within the
    canvas's area.

    Note that if items have a velocity (see \l setVelocity()), then
    collision testing is done based on where the item \e will be when
    it moves, not its current location. For example, a "ball" item
    doesn't need to actually embed into a "wall" item before a
    collision is detected. For items without velocity, plain
    intersection is used.
*/
QtCanvasItemList QtCanvasItem::collisions(bool exact) const
{
    return canvas()->collisions(chunks(), this, exact);
}

/*
    Returns a list of canvas items that collide with the point \a p.
    The list is ordered by z coordinates, from highest z coordinate
    (front-most item) to lowest z coordinate (rear-most item).
*/
QtCanvasItemList QtCanvas::collisions(const QPoint& p) const
{
    return collisions(QRect(p, QSize(1, 1)));
}

/*
    \overload

    Returns a list of items which collide with the rectangle \a r. The
    list is ordered by z coordinates, from highest z coordinate
    (front-most item) to lowest z coordinate (rear-most item).
*/
QtCanvasItemList QtCanvas::collisions(const QRect& r) const
{
    QtCanvasRectangle i(r, (QtCanvas*)this);
    i.setPen(NoPen);
    i.show(); // doesn't actually show, since we destroy it
    QtCanvasItemList l = i.collisions(true);
    qSort(l.begin(), l.end(), QtCanvasItemLess());
    return l;
}

/*
    \overload

    Returns a list of canvas items which intersect with the chunks
    listed in \a chunklist, excluding \a item. If \a exact is true, 
    only those which actually \link QtCanvasItem::collidesWith()
    collide with\endlink \a item are returned; otherwise canvas items
    are included just for being in the chunks.

    This is a utility function mainly used to implement the simpler
    QtCanvasItem::collisions() function.
*/
QtCanvasItemList QtCanvas::collisions(const QPolygon& chunklist, 
            const QtCanvasItem* item, bool exact) const
{
    QSet<QtCanvasItem *> seen;
    QtCanvasItemList result;
    for (int i = 0; i <(int)chunklist.count(); i++) {
        int x = chunklist[i].x();
        int y = chunklist[i].y();
        if (validChunk(x, y)) {
            const QtCanvasItemList &l = chunk(x, y).list();
            for (int i = 0; i < l.size(); ++i) {
                QtCanvasItem *g = l.at(i);
                if (g != item) {
                    if (!seen.contains(g)) {
                        seen.insert(g);
                        if (!exact || item->collidesWith(g))
                            result.append(g);
                    }
                }
            }
        }
    }
    return result;
}

/*
  \internal
  Adds the item to all the chunks it covers.
*/
void QtCanvasItem::addToChunks()
{
    if (isVisible() && canvas()) {
        QPolygon pa = chunks();
        for (int i = 0; i < (int)pa.count(); i++)
            canvas()->addItemToChunk(this, pa[i].x(), pa[i].y());
        val = (uint)true;
    }
}

/*
  \internal
  Removes the item from all the chunks it covers.
*/
void QtCanvasItem::removeFromChunks()
{
    if (isVisible() && canvas()) {
        QPolygon pa = chunks();
        for (int i = 0; i < (int)pa.count(); i++)
            canvas()->removeItemFromChunk(this, pa[i].x(), pa[i].y());
    }
}

/*
  \internal
  Sets all the chunks covered by the item to be refreshed with QtCanvas::update()
  is next called.
*/
void QtCanvasItem::changeChunks()
{
    if (isVisible() && canvas()) {
        if (!val)
            addToChunks();
        QPolygon pa = chunks();
        for (int i = 0; i < (int)pa.count(); i++)
            canvas()->setChangedChunk(pa[i].x(), pa[i].y());
    }
}

/*
    \fn QRect QtCanvasItem::boundingRect() const

    Returns the bounding rectangle in pixels that the canvas item covers.

    \sa boundingRectAdvanced()
*/

/*
    Returns the bounding rectangle of pixels that the canvas item \e
    will cover after advance(1) is called.

    \sa boundingRect()
*/
QRect QtCanvasItem::boundingRectAdvanced() const
{
    int dx = int(x()+xVelocity())-int(x());
    int dy = int(y()+yVelocity())-int(y());
    QRect r = boundingRect();
    r.translate(dx, dy);
    return r;
}

/*
    \class QtCanvasPixmap qtcanvas.h
    \brief The QtCanvasPixmap class provides pixmaps for QtCanvasSprites.

    If you want to show a single pixmap on a QtCanvas use a
    QtCanvasSprite with just one pixmap.

    When pixmaps are inserted into a QtCanvasPixmapArray they are held
    as QtCanvasPixmaps. \l{QtCanvasSprite}s are used to show pixmaps on
    \l{QtCanvas}es and hold their pixmaps in a QtCanvasPixmapArray. If
    you retrieve a frame (pixmap) from a QtCanvasSprite it will be
    returned as a QtCanvasPixmap.

    The pixmap is a QPixmap and can only be set in the constructor.
    There are three different constructors, one taking a QPixmap, one
    a QImage and one a file name that refers to a file in any
    supported file format (see QImageReader).

    QtCanvasPixmap can have a hotspot which is defined in terms of an (x,
    y) offset. When you create a QtCanvasPixmap from a PNG file or from
    a QImage that has a QImage::offset(), the offset() is initialized
    appropriately, otherwise the constructor leaves it at (0, 0). You
    can set it later using setOffset(). When the QtCanvasPixmap is used
    in a QtCanvasSprite, the offset position is the point at
    QtCanvasItem::x() and QtCanvasItem::y(), not the top-left corner of
    the pixmap.

    Note that for QtCanvasPixmap objects created by a QtCanvasSprite, the
    position of each QtCanvasPixmap object is set so that the hotspot
    stays in the same position.

    \sa QtCanvasPixmapArray QtCanvasItem QtCanvasSprite
*/


/*
    Constructs a QtCanvasPixmap that uses the image stored in \a
    datafilename.
*/
QtCanvasPixmap::QtCanvasPixmap(const QString& datafilename)
{
    QImage image(datafilename);
    init(image);
}


/*
    Constructs a QtCanvasPixmap from the image \a image.
*/
QtCanvasPixmap::QtCanvasPixmap(const QImage& image)
{
    init(image);
}
/*
    Constructs a QtCanvasPixmap from the pixmap \a pm using the offset
    \a offset.
*/
QtCanvasPixmap::QtCanvasPixmap(const QPixmap& pm, const QPoint& offset)
{
    init(pm, offset.x(), offset.y());
}

void QtCanvasPixmap::init(const QImage& image)
{
    this->QPixmap::operator = (QPixmap::fromImage(image));
    hotx = image.offset().x();
    hoty = image.offset().y();
#ifndef QT_NO_IMAGE_DITHER_TO_1
    if(image.hasAlphaChannel()) {
        QImage i = image.createAlphaMask();
        collision_mask = new QImage(i);
    } else
#endif
        collision_mask = 0;
}

void QtCanvasPixmap::init(const QPixmap& pixmap, int hx, int hy)
{
    (QPixmap&)*this = pixmap;
    hotx = hx;
    hoty = hy;
    if(pixmap.hasAlphaChannel())  {
        QImage i = mask().toImage();
        collision_mask = new QImage(i);
    } else
        collision_mask = 0;
}

/*
    Destroys the pixmap.
*/
QtCanvasPixmap::~QtCanvasPixmap()
{
    delete collision_mask;
}

/*
    \fn int QtCanvasPixmap::offsetX() const

    Returns the x-offset of the pixmap's hotspot.

    \sa setOffset()
*/

/*
    \fn int QtCanvasPixmap::offsetY() const

    Returns the y-offset of the pixmap's hotspot.

    \sa setOffset()
*/

/*
    \fn void QtCanvasPixmap::setOffset(int x, int y)

    Sets the offset of the pixmap's hotspot to (\a x, \a y).

    \warning Do not call this function if any QtCanvasSprites are
    currently showing this pixmap.
*/

/*
    \class QtCanvasPixmapArray qtcanvas.h
    \brief The QtCanvasPixmapArray class provides an array of QtCanvasPixmaps.

    This class is used by QtCanvasSprite to hold an array of pixmaps.
    It is used to implement animated sprites, i.e. images that change
    over time, with each pixmap in the array holding one frame.

    Depending on the constructor you use you can load multiple pixmaps
    into the array either from a directory (specifying a wildcard
    pattern for the files), or from a list of QPixmaps. You can also
    read in a set of pixmaps after construction using readPixmaps().

    Individual pixmaps can be set with setImage() and retrieved with
    image(). The number of pixmaps in the array is returned by
    count().

    QtCanvasSprite uses an image's mask for collision detection. You
    can change this by reading in a separate set of image masks using
    readCollisionMasks().

*/

/*
    Constructs an invalid array (i.e. isValid() will return false).
    You must call readPixmaps() before being able to use this
    QtCanvasPixmapArray.
*/
QtCanvasPixmapArray::QtCanvasPixmapArray()
: framecount(0), img(0)
{
}

/*
    Constructs a QtCanvasPixmapArray from files.

    The \a fc parameter sets the number of frames to be loaded for
    this image.

    If \a fc is not 0, \a datafilenamepattern should contain "%1", 
    e.g. "foo%1.png". The actual filenames are formed by replacing the
    %1 with four-digit integers from 0 to (fc - 1), e.g. foo0000.png,
    foo0001.png, foo0002.png, etc.

    If \a fc is 0, \a datafilenamepattern is asssumed to be a
    filename, and the image contained in this file will be loaded as
    the first (and only) frame.

    If \a datafilenamepattern does not exist, is not readable, isn't
    an image, or some other error occurs, the array ends up empty and
    isValid() returns false.
*/

QtCanvasPixmapArray::QtCanvasPixmapArray(const QString& datafilenamepattern,
                                        int fc)
: framecount(0), img(0)
{
    readPixmaps(datafilenamepattern, fc);
}

/*
  \obsolete
  Use QtCanvasPixmapArray::QtCanvasPixmapArray(QtValueList<QPixmap>, QPolygon)
  instead.

  Constructs a QtCanvasPixmapArray from the list of QPixmaps \a
  list. The \a hotspots list has to be of the same size as \a list.
*/
QtCanvasPixmapArray::QtCanvasPixmapArray(const QList<QPixmap> &list, const QPolygon &hotspots)
    : framecount(list.count()),
      img(new QtCanvasPixmap*[list.count()])
{
    if (list.count() != hotspots.count()) {
        qWarning("QtCanvasPixmapArray: lists have different lengths");
        reset();
        img = 0;
    } else {
        for (int i = 0; i < framecount; i++)
            img[i] = new QtCanvasPixmap(list.at(i), hotspots.at(i));
    }
}


/*
    Destroys the pixmap array and all the pixmaps it contains.
*/
QtCanvasPixmapArray::~QtCanvasPixmapArray()
{
    reset();
}

void QtCanvasPixmapArray::reset()
{
    for (int i = 0; i < framecount; i++)
        delete img[i];
    delete [] img;
    img = 0;
    framecount = 0;
}

/*
    Reads one or more pixmaps into the pixmap array.

    If \a fc is not 0, \a filenamepattern should contain "%1", e.g.
    "foo%1.png". The actual filenames are formed by replacing the %1
    with four-digit integers from 0 to (fc - 1), e.g. foo0000.png,
    foo0001.png, foo0002.png, etc.

    If \a fc is 0, \a filenamepattern is asssumed to be a filename,
    and the image contained in this file will be loaded as the first
    (and only) frame.

    If \a filenamepattern does not exist, is not readable, isn't an
    image, or some other error occurs, this function will return
    false, and isValid() will return false; otherwise this function
    will return true.

    \sa isValid()
*/
bool QtCanvasPixmapArray::readPixmaps(const QString& filenamepattern,
                                      int fc)
{
    return readPixmaps(filenamepattern, fc, false);
}

/*
    Reads new collision masks for the array.

    By default, QtCanvasSprite uses the image mask of a sprite to
    detect collisions. Use this function to set your own collision
    image masks.

    If count() is 1 \a filename must specify a real filename to read
    the mask from. If count() is greater than 1, the \a filename must
    contain a "%1" that will get replaced by the number of the mask to
    be loaded, just like QtCanvasPixmapArray::readPixmaps().

    All collision masks must be 1-bit images or this function call
    will fail.

    If the file isn't readable, contains the wrong number of images,
    or there is some other error, this function will return false, and
    the array will be flagged as invalid; otherwise this function
    returns true.

    \sa isValid()
*/
bool QtCanvasPixmapArray::readCollisionMasks(const QString& filename)
{
    return readPixmaps(filename, framecount, true);
}


bool QtCanvasPixmapArray::readPixmaps(const QString& datafilenamepattern,
                                      int fc, bool maskonly)
{
    if (!maskonly) {
        reset();
        framecount = fc;
        if (!framecount)
            framecount = 1;
        img = new QtCanvasPixmap*[framecount];
    }
    if (!img)
        return false;

    bool ok = true;
    bool arg = fc > 1;
    if (!arg)
        framecount = 1;
    for (int i = 0; i < framecount; i++) {
        QString r;
        r.sprintf("%04d", i);
        if (maskonly) {
            if (!img[i]->collision_mask)
                img[i]->collision_mask = new QImage();
            img[i]->collision_mask->load(
                arg ? datafilenamepattern.arg(r) : datafilenamepattern);
            ok = ok
               && !img[i]->collision_mask->isNull()
               && img[i]->collision_mask->depth() == 1;
        } else {
            img[i] = new QtCanvasPixmap(
                arg ? datafilenamepattern.arg(r) : datafilenamepattern);
            ok = ok && !img[i]->isNull();
        }
    }
    if (!ok) {
        reset();
    }
    return ok;
}

/*
  \obsolete

  Use isValid() instead.

  This returns false if the array is valid, and true if it is not.
*/
bool QtCanvasPixmapArray::operator!()
{
    return img == 0;
}

/*
    Returns true if the pixmap array is valid; otherwise returns
    false.
*/
bool QtCanvasPixmapArray::isValid() const
{
    return (img != 0);
}

/*
    \fn QtCanvasPixmap* QtCanvasPixmapArray::image(int i) const

    Returns pixmap \a i in the array, if \a i is non-negative and less
    than than count(), and returns an unspecified value otherwise.
*/

// ### wouldn't it be better to put empty QtCanvasPixmaps in there instead of
// initializing the additional elements in the array to 0? Lars
/*
    Replaces the pixmap at index \a i with pixmap \a p.

    The array takes ownership of \a p and will delete \a p when the
    array itself is deleted.

    If \a i is beyond the end of the array the array is extended to at
    least i+1 elements, with elements count() to i-1 being initialized
    to 0.
*/
void QtCanvasPixmapArray::setImage(int i, QtCanvasPixmap* p)
{
    if (i >= framecount) {
        QtCanvasPixmap** newimg = new QtCanvasPixmap*[i+1];
        memcpy(newimg, img, sizeof(QtCanvasPixmap *)*framecount);
        memset(newimg + framecount, 0, sizeof(QtCanvasPixmap *)*(i+1 - framecount));
        framecount = i+1;
        delete [] img;
        img = newimg;
    }
    delete img[i]; img[i] = p;
}

/*
    \fn uint QtCanvasPixmapArray::count() const

    Returns the number of pixmaps in the array.
*/

/*
    Returns the x-coordinate of the current left edge of the sprite.
    (This may change as the sprite animates since different frames may
    have different left edges.)

    \sa rightEdge() bottomEdge() topEdge()
*/
int QtCanvasSprite::leftEdge() const
{
    return int(x()) - image()->hotx;
}

/*
    \overload

    Returns what the x-coordinate of the left edge of the sprite would
    be if the sprite (actually its hotspot) were moved to x-position
    \a nx.

    \sa rightEdge() bottomEdge() topEdge()
*/
int QtCanvasSprite::leftEdge(int nx) const
{
    return nx - image()->hotx;
}

/*
    Returns the y-coordinate of the top edge of the sprite. (This may
    change as the sprite animates since different frames may have
    different top edges.)

    \sa leftEdge() rightEdge() bottomEdge()
*/
int QtCanvasSprite::topEdge() const
{
    return int(y()) - image()->hoty;
}

/*
    \overload

    Returns what the y-coordinate of the top edge of the sprite would
    be if the sprite (actually its hotspot) were moved to y-position
    \a ny.

    \sa leftEdge() rightEdge() bottomEdge()
*/
int QtCanvasSprite::topEdge(int ny) const
{
    return ny - image()->hoty;
}

/*
    Returns the x-coordinate of the current right edge of the sprite.
    (This may change as the sprite animates since different frames may
    have different right edges.)

    \sa leftEdge() bottomEdge() topEdge()
*/
int QtCanvasSprite::rightEdge() const
{
    return leftEdge() + image()->width()-1;
}

/*
    \overload

    Returns what the x-coordinate of the right edge of the sprite
    would be if the sprite (actually its hotspot) were moved to
    x-position \a nx.

    \sa leftEdge() bottomEdge() topEdge()
*/
int QtCanvasSprite::rightEdge(int nx) const
{
    return leftEdge(nx) + image()->width()-1;
}

/*
    Returns the y-coordinate of the current bottom edge of the sprite.
    (This may change as the sprite animates since different frames may
    have different bottom edges.)

    \sa leftEdge() rightEdge() topEdge()
*/
int QtCanvasSprite::bottomEdge() const
{
    return topEdge() + image()->height()-1;
}

/*
    \overload

    Returns what the y-coordinate of the top edge of the sprite would
    be if the sprite (actually its hotspot) were moved to y-position
    \a ny.

    \sa leftEdge() rightEdge() topEdge()
*/
int QtCanvasSprite::bottomEdge(int ny) const
{
    return topEdge(ny) + image()->height()-1;
}

/*
    \fn QtCanvasPixmap* QtCanvasSprite::image() const

    Returns the current frame's image.

    \sa frame(), setFrame()
*/

/*
    \fn QtCanvasPixmap* QtCanvasSprite::image(int f) const
    \overload

    Returns the image for frame \a f. Does not do any bounds checking on \a f.
*/

/*
    Returns the image the sprite \e will have after advance(1) is
    called. By default this is the same as image().
*/
QtCanvasPixmap* QtCanvasSprite::imageAdvanced() const
{
    return image();
}

/*
    Returns the bounding rectangle for the image in the sprite's
    current frame. This assumes that the images are tightly cropped
    (i.e. do not have transparent pixels all along a side).
*/
QRect QtCanvasSprite::boundingRect() const
{
    return QRect(leftEdge(), topEdge(), width(), height());
}


/*
  \internal
  Returns the chunks covered by the item.
*/
QPolygon QtCanvasItem::chunks() const
{
    QPolygon r;
    int n = 0;
    QRect br = boundingRect();
    if (isVisible() && canvas()) {
        int chunksize = canvas()->chunkSize();
        br &= QRect(0, 0, canvas()->width(), canvas()->height());
        if (br.isValid()) {
            r.resize((br.width()/chunksize+2)*(br.height()/chunksize+2));
            for (int j = br.top()/chunksize; j <= br.bottom()/chunksize; j++) {
                for (int i = br.left()/chunksize; i <= br.right()/chunksize; i++) {
                    r[n++] = QPoint(i, j);
                }
            }
        }
    }
    r.resize(n);
    return r;
}


/*
  \internal
  Add the sprite to the chunks in its QtCanvas which it overlaps.
*/
void QtCanvasSprite::addToChunks()
{
    if (isVisible() && canvas()) {
        int chunksize = canvas()->chunkSize();
        for (int j = topEdge()/chunksize; j <= bottomEdge()/chunksize; j++) {
            for (int i = leftEdge()/chunksize; i <= rightEdge()/chunksize; i++) {
                canvas()->addItemToChunk(this, i, j);
            }
        }
    }
}

/*
  \internal
  Remove the sprite from the chunks in its QtCanvas which it overlaps.

  \sa addToChunks()
*/
void QtCanvasSprite::removeFromChunks()
{
    if (isVisible() && canvas()) {
        int chunksize = canvas()->chunkSize();
        for (int j = topEdge()/chunksize; j <= bottomEdge()/chunksize; j++) {
            for (int i = leftEdge()/chunksize; i <= rightEdge()/chunksize; i++) {
                canvas()->removeItemFromChunk(this, i, j);
            }
        }
    }
}

/*
    The width of the sprite for the current frame's image.

    \sa frame()
*/
//### mark: Why don't we have width(int) and height(int) to be
//consistent with leftEdge() and leftEdge(int)?
int QtCanvasSprite::width() const
{
    return image()->width();
}

/*
    The height of the sprite for the current frame's image.

    \sa frame()
*/
int QtCanvasSprite::height() const
{
    return image()->height();
}


/*
    Draws the current frame's image at the sprite's current position
    on painter \a painter.
*/
void QtCanvasSprite::draw(QPainter& painter)
{
    painter.drawPixmap(leftEdge(), topEdge(), *image());
}

/*
    \class QtCanvasView qtcanvas.h
    \brief The QtCanvasView class provides an on-screen view of a QtCanvas.

    A QtCanvasView is widget which provides a view of a QtCanvas.

    If you want users to be able to interact with a canvas view,
    subclass QtCanvasView. You might then reimplement
    QtScrollView::contentsMousePressEvent(). For example:

    \code
    void MyCanvasView::contentsMousePressEvent(QMouseEvent* e)
    {
        QtCanvasItemList l = canvas()->collisions(e->pos());
        for (QtCanvasItemList::Iterator it = l.begin(); it!= l.end(); ++it) {
            if ((*it)->rtti() == QtCanvasRectangle::RTTI)
                qDebug("A QtCanvasRectangle lies somewhere at this point");
        }
    }
    \endcode

    The canvas view shows canvas canvas(); this can be changed using
    setCanvas().

    A transformation matrix can be used to transform the view of the
    canvas in various ways, for example, zooming in or out or rotating.
    For example:

    \code
    QMatrix wm;
    wm.scale(2, 2);   // Zooms in by 2 times
    wm.rotate(90);    // Rotates 90 degrees counter clockwise
                        // around the origin.
    wm.translate(0, -canvas->height());
                        // moves the canvas down so what was visible
                        // before is still visible.
    myCanvasView->setWorldMatrix(wm);
    \endcode

    Use setWorldMatrix() to set the canvas view's world matrix: you must
    ensure that the world matrix is invertible. The current world matrix
    is retrievable with worldMatrix(), and its inversion is retrievable
    with inverseWorldMatrix().

    Example:

    The following code finds the part of the canvas that is visible in
    this view, i.e. the bounding rectangle of the view in canvas coordinates.

    \code
    QRect rc = QRect(myCanvasView->contentsX(), myCanvasView->contentsY(),
                        myCanvasView->visibleWidth(), myCanvasView->visibleHeight());
    QRect canvasRect = myCanvasView->inverseWorldMatrix().mapRect(rc);
    \endcode

    \sa QMatrix QPainter::setWorldMatrix()

*/

class QtCanvasWidget : public QWidget
{
public:
    QtCanvasWidget(QtCanvasView *view) : QWidget(view) { m_view = view; }
protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e) {
        m_view->contentsMousePressEvent(e);
    }
    void mouseMoveEvent(QMouseEvent *e) {
        m_view->contentsMouseMoveEvent(e);
    }
    void mouseReleaseEvent(QMouseEvent *e) {
        m_view->contentsMouseReleaseEvent(e);
    }
    void mouseDoubleClickEvent(QMouseEvent *e) {
        m_view->contentsMouseDoubleClickEvent(e);
    }
    void dragEnterEvent(QDragEnterEvent *e) {
        m_view->contentsDragEnterEvent(e);
    }
    void dragMoveEvent(QDragMoveEvent *e) {
        m_view->contentsDragMoveEvent(e);
    }
    void dragLeaveEvent(QDragLeaveEvent *e) {
        m_view->contentsDragLeaveEvent(e);
    }
    void dropEvent(QDropEvent *e) {
        m_view->contentsDropEvent(e);
    }
    void wheelEvent(QWheelEvent *e) {
        m_view->contentsWheelEvent(e);
    }
    void contextMenuEvent(QContextMenuEvent *e) {
        m_view->contentsContextMenuEvent(e);
    }

    QtCanvasView *m_view;
};

void QtCanvasWidget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    if (m_view->d->highQuality) {
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
    }
    m_view->drawContents(&p, e->rect().x(), e->rect().y(), e->rect().width(), e->rect().height());
}

/*
    Constructs a QtCanvasView with parent \a parent. The canvas view
    is not associated with a canvas, so you must to call setCanvas()
    to view a canvas.
*/
QtCanvasView::QtCanvasView(QWidget* parent)
    : QScrollArea(parent)
{
    d = new QtCanvasViewData;
    setWidget(new QtCanvasWidget(this));
    d->highQuality = false;
    viewing = 0;
    setCanvas(0);
}

/*
    \overload

    Constructs a QtCanvasView which views canvas \a canvas, with parent
    \a parent.
*/
QtCanvasView::QtCanvasView(QtCanvas* canvas, QWidget* parent)
    : QScrollArea(parent)
{
    d = new QtCanvasViewData;
    d->highQuality = false;
    setWidget(new QtCanvasWidget(this));
    viewing = 0;
    setCanvas(canvas);
}

/*
    Destroys the canvas view. The associated canvas is \e not deleted.
*/
QtCanvasView::~QtCanvasView()
{
    delete d;
    d = 0;
    setCanvas(0);
}

/*
    \property QtCanvasView::highQualityRendering
    \brief whether high quality rendering is turned on

    If high quality rendering is turned on, the canvas view will paint itself
    using the QPainter::Antialiasing and QPainter::SmoothPixmapTransform
    rendering flags.

    Enabling these flag will usually improve the visual appearance on the screen
    at the cost of rendering speed.
*/
bool QtCanvasView::highQualityRendering() const
{
    return d->highQuality;
}

void QtCanvasView::setHighQualityRendering(bool enable)
{
    d->highQuality = enable;
    widget()->update();
}


void QtCanvasView::contentsMousePressEvent(QMouseEvent *e)
{
    e->ignore();
}

void QtCanvasView::contentsMouseReleaseEvent(QMouseEvent *e)
{
    e->ignore();
}

void QtCanvasView::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
    e->ignore();
}

void QtCanvasView::contentsMouseMoveEvent(QMouseEvent *e)
{
    e->ignore();
}

void QtCanvasView::contentsDragEnterEvent(QDragEnterEvent *)
{
}

void QtCanvasView::contentsDragMoveEvent(QDragMoveEvent *)
{
}

void QtCanvasView::contentsDragLeaveEvent(QDragLeaveEvent *)
{
}

void QtCanvasView::contentsDropEvent(QDropEvent *)
{
}

void QtCanvasView::contentsWheelEvent(QWheelEvent *e)
{
    e->ignore();
}

void QtCanvasView::contentsContextMenuEvent(QContextMenuEvent *e)
{
    e->ignore();
}

/*
    \fn QtCanvas* QtCanvasView::canvas() const

    Returns a pointer to the canvas which the QtCanvasView is currently
    showing.
*/


/*
    Sets the canvas that the QtCanvasView is showing to the canvas \a
    canvas.
*/
void QtCanvasView::setCanvas(QtCanvas* canvas)
{
    if (viewing == canvas)
        return;

    if (viewing) {
        disconnect(viewing);
        viewing->removeView(this);
    }
    viewing = canvas;
    if (viewing) {
        connect(viewing, SIGNAL(resized()), this, SLOT(updateContentsSize()));
        viewing->addView(this);
    }
    if (d) // called by d'tor
        updateContentsSize();
    update();
}

/*
    Returns a reference to the canvas view's current transformation matrix.

    \sa setWorldMatrix() inverseWorldMatrix()
*/
const QMatrix &QtCanvasView::worldMatrix() const
{
    return d->xform;
}

/*
    Returns a reference to the inverse of the canvas view's current
    transformation matrix.

    \sa setWorldMatrix() worldMatrix()
*/
const QMatrix &QtCanvasView::inverseWorldMatrix() const
{
    return d->ixform;
}

/*
    Sets the transformation matrix of the QtCanvasView to \a wm. The
    matrix must be invertible (i.e. if you create a world matrix that
    zooms out by 2 times, then the inverse of this matrix is one that
    will zoom in by 2 times).

    When you use this, you should note that the performance of the
    QtCanvasView will decrease considerably.

    Returns false if \a wm is not invertable; otherwise returns true.

    \sa worldMatrix() inverseWorldMatrix() QMatrix::isInvertible()
*/
bool QtCanvasView::setWorldMatrix(const QMatrix & wm)
{
    bool ok = wm.isInvertible();
    if (ok) {
        d->xform = wm;
        d->ixform = wm.inverted();
        updateContentsSize();
        widget()->update();
    }
    return ok;
}

void QtCanvasView::updateContentsSize()
{
    if (viewing) {
        QRect br;
        br = d->xform.mapRect(QRect(0, 0, viewing->width(), viewing->height()));

        widget()->resize(br.size());
    } else {
        widget()->resize(size());
    }
}

/*
    Repaints part of the QtCanvas that the canvas view is showing
    starting at \a cx by \a cy, with a width of \a cw and a height of \a
    ch using the painter \a p.
*/
void QtCanvasView::drawContents(QPainter *p, int cx, int cy, int cw, int ch)
{
    if (!viewing)
        return;
    QPainterPath clipPath;
    clipPath.addRect(viewing->rect());
    p->setClipPath(d->xform.map(clipPath), Qt::IntersectClip);
    viewing->drawViewArea(this, p, QRect(cx, cy, cw, ch), false);
}

/*
    Suggests a size sufficient to view the entire canvas.
*/
QSize QtCanvasView::sizeHint() const
{
    if (!canvas())
        return QScrollArea::sizeHint();
    // should maybe take transformations into account
    return (canvas()->size() + 2 * QSize(frameWidth(), frameWidth()))
           .boundedTo(3 * QApplication::desktop()->size() / 4);
}

/*
    \class QtCanvasPolygonalItem qtcanvas.h
    \brief The QtCanvasPolygonalItem class provides a polygonal canvas item
    on a QtCanvas.

    The mostly rectangular classes, such as QtCanvasSprite and
    QtCanvasText, use the object's bounding rectangle for movement, 
    repainting and collision calculations. For most other items, the
    bounding rectangle can be far too large -- a diagonal line being
    the worst case, and there are many other cases which are also bad.
    QtCanvasPolygonalItem provides polygon-based bounding rectangle
    handling, etc., which is much faster for non-rectangular items.

    Derived classes should try to define as small an area as possible
    to maximize efficiency, but the polygon must \e definitely be
    contained completely within the polygonal area. Calculating the
    exact requirements is usually difficult, but if you allow a small
    overestimate it can be easy and quick, while still getting almost
    all of QtCanvasPolygonalItem's speed.

    Note that all subclasses \e must call hide() in their destructor
    since hide() needs to be able to access areaPoints().

    Normally, QtCanvasPolygonalItem uses the odd-even algorithm for
    determining whether an object intersects this object. You can
    change this to the winding algorithm using setWinding().

    The bounding rectangle is available using boundingRect(). The
    points bounding the polygonal item are retrieved with
    areaPoints(). Use areaPointsAdvanced() to retrieve the bounding
    points the polygonal item \e will have after
    QtCanvasItem::advance(1) has been called.

    If the shape of the polygonal item is about to change while the
    item is visible, call invalidate() before updating with a
    different result from \l areaPoints().

    By default, QtCanvasPolygonalItem objects have a black pen and no
    brush (the default QPen and QBrush constructors). You can change
    this with setPen() and setBrush(), but note that some
    QtCanvasPolygonalItem subclasses only use the brush, ignoring the
    pen setting.

    The polygonal item can be drawn on a painter with draw().
    Subclasses must reimplement drawShape() to draw themselves.

    Like any other canvas item polygonal items can be moved with
    QtCanvasItem::move() and QtCanvasItem::moveBy(), or by setting coordinates
    with QtCanvasItem::setX(), QtCanvasItem::setY() and QtCanvasItem::setZ().

*/


/*
  Since most polygonal items don't have a pen, the default is
  NoPen and a black brush.
*/
static const QPen& defaultPolygonPen()
{
    static QPen* dp = 0;
    if (!dp)
        dp = new QPen;
    return *dp;
}

static const QBrush& defaultPolygonBrush()
{
    static QBrush* db = 0;
    if (!db)
        db = new QBrush;
    return *db;
}

/*
    Constructs a QtCanvasPolygonalItem on the canvas \a canvas.
*/
QtCanvasPolygonalItem::QtCanvasPolygonalItem(QtCanvas* canvas) :
    QtCanvasItem(canvas),
    br(defaultPolygonBrush()),
    pn(defaultPolygonPen())
{
    wind = 0;
}

/*
    Note that all subclasses \e must call hide() in their destructor
    since hide() needs to be able to access areaPoints().
*/
QtCanvasPolygonalItem::~QtCanvasPolygonalItem()
{
}

/*
    Returns true if the polygonal item uses the winding algorithm to
    determine the "inside" of the polygon. Returns false if it uses
    the odd-even algorithm.

    The default is to use the odd-even algorithm.

    \sa setWinding()
*/
bool QtCanvasPolygonalItem::winding() const
{
    return wind;
}

/*
    If \a enable is true, the polygonal item will use the winding
    algorithm to determine the "inside" of the polygon; otherwise the
    odd-even algorithm will be used.

    The default is to use the odd-even algorithm.

    \sa winding()
*/
void QtCanvasPolygonalItem::setWinding(bool enable)
{
    wind = enable;
}

/*
    Invalidates all information about the area covered by the canvas
    item. The item will be updated automatically on the next call that
    changes the item's status, for example, move() or update(). Call
    this function if you are going to change the shape of the item (as
    returned by areaPoints()) while the item is visible.
*/
void QtCanvasPolygonalItem::invalidate()
{
    val = (uint)false;
    removeFromChunks();
}

/*
    \fn QtCanvasPolygonalItem::isValid() const

    Returns true if the polygonal item's area information has not been
    invalidated; otherwise returns false.

    \sa invalidate()
*/

/*
    Returns the points the polygonal item \e will have after
    QtCanvasItem::advance(1) is called, i.e. what the points are when
    advanced by the current xVelocity() and yVelocity().
*/
QPolygon QtCanvasPolygonalItem::areaPointsAdvanced() const
{
    int dx = int(x()+xVelocity())-int(x());
    int dy = int(y()+yVelocity())-int(y());
    QPolygon r = areaPoints();
    r.detach(); // Explicit sharing is stupid.
    if (dx || dy)
        r.translate(dx, dy);
    return r;
}

//#define QCANVAS_POLYGONS_DEBUG
#ifdef QCANVAS_POLYGONS_DEBUG
static QWidget* dbg_wid = 0;
static QPainter* dbg_ptr = 0;
#endif

class QPolygonalProcessor {
public:
    QPolygonalProcessor(QtCanvas* c, const QPolygon& pa) :
        canvas(c)
    {
        QRect pixelbounds = pa.boundingRect();
        int cs = canvas->chunkSize();
        QRect canvasbounds = pixelbounds.intersected(canvas->rect());
        bounds.setLeft(canvasbounds.left()/cs);
        bounds.setRight(canvasbounds.right()/cs);
        bounds.setTop(canvasbounds.top()/cs);
        bounds.setBottom(canvasbounds.bottom()/cs);
        bitmap = QImage(bounds.width(), bounds.height(), QImage::Format_MonoLSB);
        pnt = 0;
        bitmap.fill(0);
#ifdef QCANVAS_POLYGONS_DEBUG
        dbg_start();
#endif
    }

    inline void add(int x, int y)
    {
        if (pnt >= (int)result.size()) {
            result.resize(pnt*2+10);
        }
        result[pnt++] = QPoint(x+bounds.x(), y+bounds.y());
#ifdef QCANVAS_POLYGONS_DEBUG
        if (dbg_ptr) {
            int cs = canvas->chunkSize();
            QRect r(x*cs+bounds.x()*cs, y*cs+bounds.y()*cs, cs-1, cs-1);
            dbg_ptr->setPen(Qt::blue);
            dbg_ptr->drawRect(r);
        }
#endif
    }

    inline void addBits(int x1, int x2, uchar newbits, int xo, int yo)
    {
        for (int i = x1; i <= x2; i++)
            if (newbits & (1 <<i))
                add(xo+i, yo);
    }

#ifdef QCANVAS_POLYGONS_DEBUG
    void dbg_start()
    {
        if (!dbg_wid) {
            dbg_wid = new QWidget;
            dbg_wid->resize(800, 600);
            dbg_wid->show();
            dbg_ptr = new QPainter(dbg_wid);
            dbg_ptr->setBrush(Qt::NoBrush);
        }
        dbg_ptr->fillRect(dbg_wid->rect(), Qt::white);
    }
#endif

    void doSpans(int n, QPoint* pt, int* w)
    {
        int cs = canvas->chunkSize();
        for (int j = 0; j < n; j++) {
            int y = pt[j].y()/cs-bounds.y();
            if (y >= bitmap.height() || y < 0) continue;
            uchar* l = bitmap.scanLine(y);
            int x = pt[j].x();
            int x1 = x/cs-bounds.x();
            if (x1 > bounds.width()) continue;
            x1  = qMax(0,x1);
            int x2 = (x+w[j])/cs-bounds.x();
            if (x2 < 0) continue;
            x2 = qMin(bounds.width(), x2);
            int x1q = x1/8;
            int x1r = x1%8;
            int x2q = x2/8;
            int x2r = x2%8;
#ifdef QCANVAS_POLYGONS_DEBUG
            if (dbg_ptr) dbg_ptr->setPen(Qt::yellow);
#endif
            if (x1q == x2q) {
                uchar newbits = (~l[x1q]) & (((2 <<(x2r-x1r))-1) <<x1r);
                if (newbits) {
#ifdef QCANVAS_POLYGONS_DEBUG
                    if (dbg_ptr) dbg_ptr->setPen(Qt::darkGreen);
#endif
                    addBits(x1r, x2r, newbits, x1q*8, y);
                    l[x1q] |= newbits;
                }
            } else {
#ifdef QCANVAS_POLYGONS_DEBUG
                if (dbg_ptr) dbg_ptr->setPen(Qt::blue);
#endif
                uchar newbits1 = (~l[x1q]) & (0xff <<x1r);
                if (newbits1) {
#ifdef QCANVAS_POLYGONS_DEBUG
                    if (dbg_ptr) dbg_ptr->setPen(Qt::green);
#endif
                    addBits(x1r, 7, newbits1, x1q*8, y);
                    l[x1q] |= newbits1;
                }
                for (int i = x1q+1; i < x2q; i++) {
                    if (l[i] != 0xff) {
                        addBits(0, 7, ~l[i], i*8, y);
                        l[i] = 0xff;
                    }
                }
                uchar newbits2 = (~l[x2q]) & (0xff>>(7-x2r));
                if (newbits2) {
#ifdef QCANVAS_POLYGONS_DEBUG
                    if (dbg_ptr) dbg_ptr->setPen(Qt::red);
#endif
                    addBits(0, x2r, newbits2, x2q*8, y);
                    l[x2q] |= newbits2;
                }
            }
#ifdef QCANVAS_POLYGONS_DEBUG
            if (dbg_ptr) {
                dbg_ptr->drawLine(pt[j], pt[j]+QPoint(w[j], 0));
            }
#endif
        }
        result.resize(pnt);
    }

    int pnt;
    QPolygon result;
    QtCanvas* canvas;
    QRect bounds;
    QImage bitmap;
};


QPolygon QtCanvasPolygonalItem::chunks() const
{
    QPolygon pa = areaPoints();

    if (!pa.size()) {
        pa.detach(); // Explicit sharing is stupid.
        return pa;
    }

    QPolygonalProcessor processor(canvas(), pa);

    scanPolygon(pa, wind, processor);

    return processor.result;
}
/*
    Simply calls QtCanvasItem::chunks().
*/
QPolygon QtCanvasRectangle::chunks() const
{
    // No need to do a polygon scan!
    return QtCanvasItem::chunks();
}

/*
    Returns the bounding rectangle of the polygonal item, based on
    areaPoints().
*/
QRect QtCanvasPolygonalItem::boundingRect() const
{
    return areaPoints().boundingRect();
}

/*
    Reimplemented from QtCanvasItem, this draws the polygonal item by
    setting the pen and brush for the item on the painter \a p and
    calling drawShape().
*/
void QtCanvasPolygonalItem::draw(QPainter & p)
{
    p.setPen(pn);
    p.setBrush(br);
    drawShape(p);
}

/*
    \fn void QtCanvasPolygonalItem::drawShape(QPainter & p)

    Subclasses must reimplement this function to draw their shape. The
    pen and brush of \a p are already set to pen() and brush() prior
    to calling this function.

    \sa draw()
*/

/*
    \fn QPen QtCanvasPolygonalItem::pen() const

    Returns the QPen used to draw the outline of the item, if any.

    \sa setPen()
*/

/*
    \fn QBrush QtCanvasPolygonalItem::brush() const

    Returns the QBrush used to fill the item, if filled.

    \sa setBrush()
*/

/*
    Sets the QPen used when drawing the item to the pen \a p.
    Note that many QtCanvasPolygonalItems do not use the pen value.

    \sa setBrush(), pen(), drawShape()
*/
void QtCanvasPolygonalItem::setPen(QPen p)
{
    if (pn != p) {
        removeFromChunks();
        pn = p;
        addToChunks();
    }
}

/*
    Sets the QBrush used when drawing the polygonal item to the brush \a b.

    \sa setPen(), brush(), drawShape()
*/
void QtCanvasPolygonalItem::setBrush(QBrush b)
{
    if (br != b) {
        br = b;
        changeChunks();
    }
}


/*
    \class QtCanvasPolygon qtcanvas.h
    \brief The QtCanvasPolygon class provides a polygon on a QtCanvas.

    Paints a polygon with a QBrush. The polygon's points can be set in
    the constructor or set or changed later using setPoints(). Use
    points() to retrieve the points, or areaPoints() to retrieve the
    points relative to the canvas's origin.

    The polygon can be drawn on a painter with drawShape().

    Like any other canvas item polygons can be moved with
    QtCanvasItem::move() and QtCanvasItem::moveBy(), or by setting
    coordinates with QtCanvasItem::setX(), QtCanvasItem::setY() and
    QtCanvasItem::setZ().

    Note: QtCanvasPolygon does not use the pen.
*/

/*
    Constructs a point-less polygon on the canvas \a canvas. You
    should call setPoints() before using it further.
*/
QtCanvasPolygon::QtCanvasPolygon(QtCanvas* canvas) :
    QtCanvasPolygonalItem(canvas)
{
}

/*
    Destroys the polygon.
*/
QtCanvasPolygon::~QtCanvasPolygon()
{
    hide();
}

/*
    Draws the polygon using the painter \a p.

    Note that QtCanvasPolygon does not support an outline (the pen is
    always NoPen).
*/
void QtCanvasPolygon::drawShape(QPainter & p)
{
    // ### why can't we draw outlines? We could use drawPolyline for it. Lars
    // ### see other message. Warwick

    p.setPen(NoPen); // since QRegion(QPolygon) excludes outline :-()-:
    p.drawPolygon(poly);
}

/*
    Sets the points of the polygon to be \a pa. These points will have
    their x and y coordinates automatically translated by x(), y() as
    the polygon is moved.
*/
void QtCanvasPolygon::setPoints(QPolygon pa)
{
    removeFromChunks();
    poly = pa;
    poly.detach(); // Explicit sharing is stupid.
    poly.translate((int)x(), (int)y());
    addToChunks();
}

/*
  \reimp
*/
void QtCanvasPolygon::moveBy(double dx, double dy)
{
    // Note: does NOT call QtCanvasPolygonalItem::moveBy(), since that
    // only does half this work.
    //
    int idx = int(x()+dx)-int(x());
    int idy = int(y()+dy)-int(y());
    if (idx || idy) {
        removeFromChunks();
        poly.translate(idx, idy);
    }
    myx+= dx;
    myy+= dy;
    if (idx || idy) {
        addToChunks();
    }
}

/*
    \class QtCanvasSpline qtcanvas.h
    \brief The QtCanvasSpline class provides multi-bezier splines on a QtCanvas.

    A QtCanvasSpline is a sequence of 4-point bezier curves joined
    together to make a curved shape.

    You set the control points of the spline with setControlPoints().

    If the bezier is closed(), then the first control point will be
    re-used as the last control point. Therefore, a closed bezier must
    have a multiple of 3 control points and an open bezier must have
    one extra point.

    The beziers are not necessarily joined "smoothly". To ensure this,
    set control points appropriately (general reference texts about
    beziers will explain this in detail).

    Like any other canvas item splines can be moved with
    QtCanvasItem::move() and QtCanvasItem::moveBy(), or by setting
    coordinates with QtCanvasItem::setX(), QtCanvasItem::setY() and
    QtCanvasItem::setZ().

*/

/*
    Create a spline with no control points on the canvas \a canvas.

    \sa setControlPoints()
*/
QtCanvasSpline::QtCanvasSpline(QtCanvas* canvas) :
    QtCanvasPolygon(canvas), 
    cl(true)
{
}

/*
    Destroy the spline.
*/
QtCanvasSpline::~QtCanvasSpline()
{
}

/*
    Set the spline control points to \a ctrl.

    If \a close is true, then the first point in \a ctrl will be
    re-used as the last point, and the number of control points must
    be a multiple of 3. If \a close is false, one additional control
    point is required, and the number of control points must be one of
    (4, 7, 10, 13, ...).

    If the number of control points doesn't meet the above conditions, 
    the number of points will be truncated to the largest number of
    points that do meet the requirement.
*/
void QtCanvasSpline::setControlPoints(QPolygon ctrl, bool close)
{
    if ((int)ctrl.count() % 3 != (close ? 0 : 1)) {
        qWarning("QtCanvasSpline::setControlPoints(): Number of points doesn't fit.");
        int numCurves = (ctrl.count() - (close ? 0 : 1))/ 3;
        ctrl.resize(numCurves*3 + (close ? 0 : 1));
    }

    cl = close;
    bez = ctrl;
    recalcPoly();
}

/*
    Returns the current set of control points.

    \sa setControlPoints(), closed()
*/
QPolygon QtCanvasSpline::controlPoints() const
{
    return bez;
}

/*
    Returns true if the control points are a closed set; otherwise
    returns false.
*/
bool QtCanvasSpline::closed() const
{
    return cl;
}

void QtCanvasSpline::recalcPoly()
{
    if (bez.count() == 0)
        return;

    QPainterPath path;
    path.moveTo(bez[0]);
    for (int i = 1; i < (int)bez.count()-1; i+= 3) {
        path.cubicTo(bez[i], bez[i+1], cl ? bez[(i+2)%bez.size()] : bez[i+2]);
    }
    QPolygon p = path.toFillPolygon().toPolygon();
    QtCanvasPolygon::setPoints(p);
}

/*
    \fn QPolygon QtCanvasPolygonalItem::areaPoints() const

    This function must be reimplemented by subclasses. It \e must
    return the points bounding (i.e. outside and not touching) the
    shape or drawing errors will occur.
*/

/*
    \fn QPolygon QtCanvasPolygon::points() const

    Returns the vertices of the polygon, not translated by the position.

    \sa setPoints(), areaPoints()
*/
QPolygon QtCanvasPolygon::points() const
{
    QPolygon pa = areaPoints();
    pa.translate(int(-x()), int(-y()));
    return pa;
}

/*
    Returns the vertices of the polygon translated by the polygon's
    current x(), y() position, i.e. relative to the canvas's origin.

    \sa setPoints(), points()
*/
QPolygon QtCanvasPolygon::areaPoints() const
{
    return poly;
}

/*
    \class QtCanvasLine qtcanvas.h
    \brief The QtCanvasLine class provides a line on a QtCanvas.

    The line inherits functionality from QtCanvasPolygonalItem, for
    example the setPen() function. The start and end points of the
    line are set with setPoints().

    Like any other canvas item lines can be moved with
    QtCanvasItem::move() and QtCanvasItem::moveBy(), or by setting
    coordinates with QtCanvasItem::setX(), QtCanvasItem::setY() and
    QtCanvasItem::setZ().
*/

/*
    Constructs a line from (0, 0) to (0, 0) on \a canvas.

    \sa setPoints()
*/
QtCanvasLine::QtCanvasLine(QtCanvas* canvas) :
    QtCanvasPolygonalItem(canvas)
{
    x1 = y1 = x2 = y2 = 0;
}

/*
    Destroys the line.
*/
QtCanvasLine::~QtCanvasLine()
{
    hide();
}

/*
  \reimp
*/
void QtCanvasLine::setPen(QPen p)
{
    QtCanvasPolygonalItem::setPen(p);
}

/*
    \fn QPoint QtCanvasLine::startPoint () const

    Returns the start point of the line.

    \sa setPoints(), endPoint()
*/

/*
    \fn QPoint QtCanvasLine::endPoint () const

    Returns the end point of the line.

    \sa setPoints(), startPoint()
*/

/*
    Sets the line's start point to (\a xa, \a ya) and its end point to
    (\a xb, \a yb).
*/
void QtCanvasLine::setPoints(int xa, int ya, int xb, int yb)
{
    if (x1 != xa || x2 != xb || y1 != ya || y2 != yb) {
        removeFromChunks();
        x1 = xa;
        y1 = ya;
        x2 = xb;
        y2 = yb;
        addToChunks();
    }
}

/*
  \reimp
*/
void QtCanvasLine::drawShape(QPainter &p)
{
    p.drawLine((int)(x()+x1), (int)(y()+y1), (int)(x()+x2), (int)(y()+y2));
}

/*
    \reimp

    Note that the area defined by the line is somewhat thicker than
    the line that is actually drawn.
*/
QPolygon QtCanvasLine::areaPoints() const
{
    QPolygon p(4);
    int xi = int(x());
    int yi = int(y());
    int pw = pen().width();
    int dx = qAbs(x1-x2);
    int dy = qAbs(y1-y2);
    pw = pw*4/3+2; // approx pw*sqrt(2)
    int px = x1 < x2 ? -pw : pw ;
    int py = y1 < y2 ? -pw : pw ;
    if (dx && dy && (dx > dy ? (dx*2/dy <= 2) : (dy*2/dx <= 2))) {
        // steep
        if (px == py) {
            p[0] = QPoint(x1+xi, y1+yi+py);
            p[1] = QPoint(x2+xi-px, y2+yi);
            p[2] = QPoint(x2+xi, y2+yi-py);
            p[3] = QPoint(x1+xi+px, y1+yi);
        } else {
            p[0] = QPoint(x1+xi+px, y1+yi);
            p[1] = QPoint(x2+xi, y2+yi-py);
            p[2] = QPoint(x2+xi-px, y2+yi);
            p[3] = QPoint(x1+xi, y1+yi+py);
        }
    } else if (dx > dy) {
        // horizontal
        p[0] = QPoint(x1+xi+px, y1+yi+py);
        p[1] = QPoint(x2+xi-px, y2+yi+py);
        p[2] = QPoint(x2+xi-px, y2+yi-py);
        p[3] = QPoint(x1+xi+px, y1+yi-py);
    } else {
        // vertical
        p[0] = QPoint(x1+xi+px, y1+yi+py);
        p[1] = QPoint(x2+xi+px, y2+yi-py);
        p[2] = QPoint(x2+xi-px, y2+yi-py);
        p[3] = QPoint(x1+xi-px, y1+yi+py);
    }
    return p;
}

/*
    \reimp

*/

void QtCanvasLine::moveBy(double dx, double dy)
{
    QtCanvasPolygonalItem::moveBy(dx, dy);
}

/*
    \class QtCanvasRectangle qtcanvas.h
    \brief The QtCanvasRectangle class provides a rectangle on a QtCanvas.

    This item paints a single rectangle which may have any pen() and
    brush(), but may not be tilted/rotated. For rotated rectangles,
    use QtCanvasPolygon.

    The rectangle's size and initial position can be set in the
    constructor. The size can be set or changed later using setSize().
    Use height() and width() to retrieve the rectangle's dimensions.

    The rectangle can be drawn on a painter with drawShape().

    Like any other canvas item rectangles can be moved with
    QtCanvasItem::move() and QtCanvasItem::moveBy(), or by setting
    coordinates with QtCanvasItem::setX(), QtCanvasItem::setY() and
    QtCanvasItem::setZ().

*/

/*
    Constructs a rectangle at position (0,0) with both width and
    height set to 32 pixels on \a canvas.
*/
QtCanvasRectangle::QtCanvasRectangle(QtCanvas* canvas) :
    QtCanvasPolygonalItem(canvas),
    w(32), h(32)
{
}

/*
    Constructs a rectangle positioned and sized by \a r on \a canvas.
*/
QtCanvasRectangle::QtCanvasRectangle(const QRect& r, QtCanvas* canvas) :
    QtCanvasPolygonalItem(canvas),
    w(r.width()), h(r.height())
{
    move(r.x(), r.y());
}

/*
    Constructs a rectangle at position (\a x, \a y) and size \a width
    by \a height, on \a canvas.
*/
QtCanvasRectangle::QtCanvasRectangle(int x, int y, int width, int height,
        QtCanvas* canvas) :
    QtCanvasPolygonalItem(canvas),
    w(width), h(height)
{
    move(x, y);
}

/*
    Destroys the rectangle.
*/
QtCanvasRectangle::~QtCanvasRectangle()
{
    hide();
}


/*
    Returns the width of the rectangle.
*/
int QtCanvasRectangle::width() const
{
    return w;
}

/*
    Returns the height of the rectangle.
*/
int QtCanvasRectangle::height() const
{
    return h;
}

/*
    Sets the \a width and \a height of the rectangle.
*/
void QtCanvasRectangle::setSize(int width, int height)
{
    if (w != width || h != height) {
        removeFromChunks();
        w = width;
        h = height;
        addToChunks();
    }
}

/*
    \fn QSize QtCanvasRectangle::size() const

    Returns the width() and height() of the rectangle.

    \sa rect(), setSize()
*/

/*
    \fn QRect QtCanvasRectangle::rect() const

    Returns the integer-converted x(), y() position and size() of the
    rectangle as a QRect.
*/

/*
  \reimp
*/
QPolygon QtCanvasRectangle::areaPoints() const
{
    QPolygon pa(4);
    int pw = (pen().width()+1)/2;
    if (pw < 1) pw = 1;
    if (pen() == NoPen) pw = 0;
    pa[0] = QPoint((int)x()-pw, (int)y()-pw);
    pa[1] = pa[0] + QPoint(w+pw*2, 0);
    pa[2] = pa[1] + QPoint(0, h+pw*2);
    pa[3] = pa[0] + QPoint(0, h+pw*2);
    return pa;
}

/*
    Draws the rectangle on painter \a p.
*/
void QtCanvasRectangle::drawShape(QPainter & p)
{
    p.drawRect((int)x(), (int)y(), w, h);
}


/*
    \class QtCanvasEllipse qtcanvas.h
    \brief The QtCanvasEllipse class provides an ellipse or ellipse segment on a QtCanvas.

    A canvas item that paints an ellipse or ellipse segment with a QBrush.
    The ellipse's height, width, start angle and angle length can be set
    at construction time. The size can be changed at runtime with
    setSize(), and the angles can be changed (if you're displaying an
    ellipse segment rather than a whole ellipse) with setAngles().

    Note that angles are specified in 16ths of a degree.

    \target anglediagram
    \img qcanvasellipse.png Ellipse

    If a start angle and length angle are set then an ellipse segment
    will be drawn. The start angle is the angle that goes from zero in a
    counter-clockwise direction (shown in green in the diagram). The
    length angle is the angle from the start angle in a
    counter-clockwise direction (shown in blue in the diagram). The blue
    segment is the segment of the ellipse that would be drawn. If no
    start angle and length angle are specified the entire ellipse is
    drawn.

    The ellipse can be drawn on a painter with drawShape().

    Like any other canvas item ellipses can be moved with move() and
    moveBy(), or by setting coordinates with setX(), setY() and setZ().

    Note: QtCanvasEllipse does not use the pen.
*/

/*
    Constructs a 32x32 ellipse, centered at (0, 0) on \a canvas.
*/
QtCanvasEllipse::QtCanvasEllipse(QtCanvas* canvas) :
    QtCanvasPolygonalItem(canvas),
    w(32), h(32),
    a1(0), a2(360*16)
{
}

/*
    Constructs a \a width by \a height pixel ellipse, centered at
    (0, 0) on \a canvas.
*/
QtCanvasEllipse::QtCanvasEllipse(int width, int height, QtCanvas* canvas) :
    QtCanvasPolygonalItem(canvas), 
    w(width), h(height),
    a1(0), a2(360*16)
{
}

// ### add a constructor taking degrees in float. 1/16 degrees is stupid. Lars
// ### it's how QPainter does it, so QtCanvas does too for consistency. If it's
// ###  a good idea, it should be added to QPainter, not just to QtCanvas. Warwick
/*
    Constructs a \a width by \a height pixel ellipse, centered at
    (0, 0) on \a canvas. Only a segment of the ellipse is drawn,
    starting at angle \a startangle, and extending for angle \a angle
    (the angle length).

    Note that angles are specified in sixteenths of a degree.
*/
QtCanvasEllipse::QtCanvasEllipse(int width, int height, 
    int startangle, int angle, QtCanvas* canvas) :
    QtCanvasPolygonalItem(canvas), 
    w(width), h(height), 
    a1(startangle), a2(angle)
{
}

/*
    Destroys the ellipse.
*/
QtCanvasEllipse::~QtCanvasEllipse()
{
    hide();
}

/*
    Returns the width of the ellipse.
*/
int QtCanvasEllipse::width() const
{
    return w;
}

/*
    Returns the height of the ellipse.
*/
int QtCanvasEllipse::height() const
{
    return h;
}

/*
    Sets the \a width and \a height of the ellipse.
*/
void QtCanvasEllipse::setSize(int width, int height)
{
    if (w != width || h != height) {
        removeFromChunks();
        w = width;
        h = height;
        addToChunks();
    }
}

/*
    \fn int QtCanvasEllipse::angleStart() const

    Returns the start angle in 16ths of a degree. Initially
    this will be 0.

    \sa setAngles(), angleLength()
*/

/*
    \fn int QtCanvasEllipse::angleLength() const

    Returns the length angle (the extent of the ellipse segment) in
    16ths of a degree. Initially this will be 360 * 16 (a complete
    ellipse).

    \sa setAngles(), angleStart()
*/

/*
    Sets the angles for the ellipse. The start angle is \a start and
    the extent of the segment is \a length (the angle length) from the
    \a start. The angles are specified in 16ths of a degree. By
    default the ellipse will start at 0 and have an angle length of
    360 * 16 (a complete ellipse).

    \sa angleStart(), angleLength()
*/
void QtCanvasEllipse::setAngles(int start, int length)
{
    if (a1 != start || a2 != length) {
        removeFromChunks();
        a1 = start;
        a2 = length;
        addToChunks();
    }
}

/*
  \reimp
*/
QPolygon QtCanvasEllipse::areaPoints() const
{
    QPainterPath path;
    path.arcTo(QRectF(x()-w/2.0+0.5-1, y()-h/2.0+0.5-1, w+3, h+3), a1/16., a2/16.);
    return path.toFillPolygon().toPolygon();
}

/*
    Draws the ellipse, centered at x(), y() using the painter \a p.

    Note that QtCanvasEllipse does not support an outline (the pen is
    always NoPen).
*/
void QtCanvasEllipse::drawShape(QPainter & p)
{
    p.setPen(NoPen); // since QRegion(QPolygon) excludes outline :-()-:
    if (!a1 && a2 == 360*16) {
        p.drawEllipse(int(x()-w/2.0+0.5), int(y()-h/2.0+0.5), w, h);
    } else {
        p.drawPie(int(x()-w/2.0+0.5), int(y()-h/2.0+0.5), w, h, a1, a2);
    }
}


/*
    \class QtCanvasText
    \brief The QtCanvasText class provides a text object on a QtCanvas.

    A canvas text item has text with font, color and alignment
    attributes. The text and font can be set in the constructor or set
    or changed later with setText() and setFont(). The color is set
    with setColor() and the alignment with setTextFlags(). The text
    item's bounding rectangle is retrieved with boundingRect().

    The text can be drawn on a painter with draw().

    Like any other canvas item text items can be moved with
    QtCanvasItem::move() and QtCanvasItem::moveBy(), or by setting
    coordinates with QtCanvasItem::setX(), QtCanvasItem::setY() and
    QtCanvasItem::setZ().
*/

/*
    Constructs a QtCanvasText with the text "\<text\>", on \a canvas.
*/
QtCanvasText::QtCanvasText(QtCanvas* canvas) :
    QtCanvasItem(canvas), 
    txt("<text>"), flags(0)
{
    setRect();
}

// ### add textflags to the constructor? Lars
/*
    Constructs a QtCanvasText with the text \a t, on canvas \a canvas.
*/
QtCanvasText::QtCanvasText(const QString& t, QtCanvas* canvas) :
    QtCanvasItem(canvas), 
    txt(t), flags(0)
{
    setRect();
}

// ### see above
/*
    Constructs a QtCanvasText with the text \a t and font \a f, on the
    canvas \a canvas.
*/
QtCanvasText::QtCanvasText(const QString& t, QFont f, QtCanvas* canvas) :
    QtCanvasItem(canvas), 
    txt(t), flags(0), 
    fnt(f)
{
    setRect();
}

/*
    Destroys the canvas text item.
*/
QtCanvasText::~QtCanvasText()
{
    removeFromChunks();
}

/*
    Returns the bounding rectangle of the text.
*/
QRect QtCanvasText::boundingRect() const { return brect; }

void QtCanvasText::setRect()
{
    brect = QFontMetrics(fnt).boundingRect(int(x()), int(y()), 0, 0, flags, txt);
}

/*
    \fn int QtCanvasText::textFlags() const

    Returns the currently set alignment flags.

    \sa setTextFlags() Qt::AlignmentFlag Qt::TextFlag
*/


/*
    Sets the alignment flags to \a f. These are a bitwise OR of the
    flags available to QPainter::drawText() -- see the
    \l{Qt::AlignmentFlag}s and \l{Qt::TextFlag}s.

    \sa setFont() setColor()
*/
void QtCanvasText::setTextFlags(int f)
{
    if (flags != f) {
        removeFromChunks();
        flags = f;
        setRect();
        addToChunks();
    }
}

/*
    Returns the text item's text.

    \sa setText()
*/
QString QtCanvasText::text() const
{
    return txt;
}


/*
    Sets the text item's text to \a t. The text may contain newlines.

    \sa text(), setFont(), setColor() setTextFlags()
*/
void QtCanvasText::setText(const QString& t)
{
    if (txt != t) {
        removeFromChunks();
        txt = t;
        setRect();
        addToChunks();
    }
}

/*
    Returns the font in which the text is drawn.

    \sa setFont()
*/
QFont QtCanvasText::font() const
{
    return fnt;
}

/*
    Sets the font in which the text is drawn to font \a f.

    \sa font()
*/
void QtCanvasText::setFont(const QFont& f)
{
    if (f != fnt) {
        removeFromChunks();
        fnt = f;
        setRect();
        addToChunks();
    }
}

/*
    Returns the color of the text.

    \sa setColor()
*/
QColor QtCanvasText::color() const
{
    return col;
}

/*
    Sets the color of the text to the color \a c.

    \sa color(), setFont()
*/
void QtCanvasText::setColor(const QColor& c)
{
    col = c;
    changeChunks();
}


/*
  \reimp
*/
void QtCanvasText::moveBy(double dx, double dy)
{
    int idx = int(x()+dx)-int(x());
    int idy = int(y()+dy)-int(y());
    if (idx || idy) {
        removeFromChunks();
    }
    myx+= dx;
    myy+= dy;
    if (idx || idy) {
        brect.translate(idx, idy);
        addToChunks();
    }
}

/*
    Draws the text using the painter \a painter.
*/
void QtCanvasText::draw(QPainter& painter)
{
    painter.setFont(fnt);
    painter.setPen(col);
    painter.drawText(painter.fontMetrics().boundingRect(int(x()), int(y()), 0, 0, flags, txt), flags, txt);
}

/*
  \reimp
*/
void QtCanvasText::changeChunks()
{
    if (isVisible() && canvas()) {
        int chunksize = canvas()->chunkSize();
        for (int j = brect.top()/chunksize; j <= brect.bottom()/chunksize; j++) {
            for (int i = brect.left()/chunksize; i <= brect.right()/chunksize; i++) {
                canvas()->setChangedChunk(i, j);
            }
        }
    }
}

/*
    Adds the text item to the appropriate chunks.
*/
void QtCanvasText::addToChunks()
{
    if (isVisible() && canvas()) {
        int chunksize = canvas()->chunkSize();
        for (int j = brect.top()/chunksize; j <= brect.bottom()/chunksize; j++) {
            for (int i = brect.left()/chunksize; i <= brect.right()/chunksize; i++) {
                canvas()->addItemToChunk(this, i, j);
            }
        }
    }
}

/*
    Removes the text item from the appropriate chunks.
*/
void QtCanvasText::removeFromChunks()
{
    if (isVisible() && canvas()) {
        int chunksize = canvas()->chunkSize();
        for (int j = brect.top()/chunksize; j <= brect.bottom()/chunksize; j++) {
            for (int i = brect.left()/chunksize; i <= brect.right()/chunksize; i++) {
                canvas()->removeItemFromChunk(this, i, j);
            }
        }
    }
}


/*
    Returns 0 (QtCanvasItem::Rtti_Item).

    Make your derived classes return their own values for rtti(), so
    that you can distinguish between objects returned by
    QtCanvas::at(). You should use values greater than 1000 to allow
    for extensions to this class.

    Overuse of this functionality can damage its extensibility. For
    example, once you have identified a base class of a QtCanvasItem
    found by QtCanvas::at(), cast it to that type and call meaningful
    methods rather than acting upon the object based on its rtti
    value.

    For example:

    \code
        QtCanvasItem* item;
        // Find an item, e.g. with QtCanvasItem::collisions().
        ...
        if (item->rtti() == MySprite::RTTI) {
            MySprite* s = (MySprite*)item;
            if (s->isDamagable()) s->loseHitPoints(1000);
            if (s->isHot()) myself->loseHitPoints(1000);
            ...
        }
    \endcode
*/
int QtCanvasItem::rtti() const { return RTTI; }
int QtCanvasItem::RTTI = Rtti_Item;

/*
    Returns 1 (QtCanvasItem::Rtti_Sprite).

    \sa QtCanvasItem::rtti()
*/
int QtCanvasSprite::rtti() const { return RTTI; }
int QtCanvasSprite::RTTI = Rtti_Sprite;

/*
    Returns 2 (QtCanvasItem::Rtti_PolygonalItem).

    \sa QtCanvasItem::rtti()
*/
int QtCanvasPolygonalItem::rtti() const { return RTTI; }
int QtCanvasPolygonalItem::RTTI = Rtti_PolygonalItem;

/*
    Returns 3 (QtCanvasItem::Rtti_Text).

    \sa QtCanvasItem::rtti()
*/
int QtCanvasText::rtti() const { return RTTI; }
int QtCanvasText::RTTI = Rtti_Text;

/*
    Returns 4 (QtCanvasItem::Rtti_Polygon).

    \sa QtCanvasItem::rtti()
*/
int QtCanvasPolygon::rtti() const { return RTTI; }
int QtCanvasPolygon::RTTI = Rtti_Polygon;

/*
    Returns 5 (QtCanvasItem::Rtti_Rectangle).

    \sa QtCanvasItem::rtti()
*/
int QtCanvasRectangle::rtti() const { return RTTI; }
int QtCanvasRectangle::RTTI = Rtti_Rectangle;

/*
    Returns 6 (QtCanvasItem::Rtti_Ellipse).

    \sa QtCanvasItem::rtti()
*/
int QtCanvasEllipse::rtti() const { return RTTI; }
int QtCanvasEllipse::RTTI = Rtti_Ellipse;

/*
    Returns 7 (QtCanvasItem::Rtti_Line).

    \sa QtCanvasItem::rtti()
*/
int QtCanvasLine::rtti() const { return RTTI; }
int QtCanvasLine::RTTI = Rtti_Line;

/*
    Returns 8 (QtCanvasItem::Rtti_Spline).

    \sa QtCanvasItem::rtti()
*/
int QtCanvasSpline::rtti() const { return RTTI; }
int QtCanvasSpline::RTTI = Rtti_Spline;

/*
    Constructs a QtCanvasSprite which uses images from the
    QtCanvasPixmapArray \a a.

    The sprite in initially positioned at (0, 0) on \a canvas, using
    frame 0.
*/
QtCanvasSprite::QtCanvasSprite(QtCanvasPixmapArray* a, QtCanvas* canvas) :
    QtCanvasItem(canvas), 
    frm(0), 
    anim_val(0), 
    anim_state(0), 
    anim_type(0), 
    images(a)
{
}


/*
    Set the array of images used for displaying the sprite to the
    QtCanvasPixmapArray \a a.

    If the current frame() is larger than the number of images in \a
    a, the current frame will be reset to 0.
*/
void QtCanvasSprite::setSequence(QtCanvasPixmapArray* a)
{
    bool isvisible = isVisible();
    if (isvisible && images)
        hide();
    images = a;
    if (frm >= (int)images->count())
        frm = 0;
    if (isvisible)
        show();
}

/*
\internal

Marks any chunks the sprite touches as changed.
*/
void QtCanvasSprite::changeChunks()
{
    if (isVisible() && canvas()) {
        int chunksize = canvas()->chunkSize();
        for (int j = topEdge()/chunksize; j <= bottomEdge()/chunksize; j++) {
            for (int i = leftEdge()/chunksize; i <= rightEdge()/chunksize; i++) {
                canvas()->setChangedChunk(i, j);
            }
        }
    }
}

/*
    Destroys the sprite and removes it from the canvas. Does \e not
    delete the images.
*/
QtCanvasSprite::~QtCanvasSprite()
{
    removeFromChunks();
}

/*
    Sets the animation frame used for displaying the sprite to \a f, 
    an index into the QtCanvasSprite's QtCanvasPixmapArray. The call
    will be ignored if \a f is larger than frameCount() or smaller
    than 0.

    \sa frame() move()
*/
void QtCanvasSprite::setFrame(int f)
{
    move(x(), y(), f);
}

/*
    \enum QtCanvasSprite::FrameAnimationType

    This enum is used to identify the different types of frame
    animation offered by QtCanvasSprite.

    \value Cycle at each advance the frame number will be incremented by
    1 (modulo the frame count).
    \value Oscillate at each advance the frame number will be
    incremented by 1 up to the frame count then decremented to by 1 to
    0, repeating this sequence forever.
*/

/*
    Sets the animation characteristics for the sprite.

    For \a type == \c Cycle, the frames will increase by \a step
    at each advance, modulo the frameCount().

    For \a type == \c Oscillate, the frames will increase by \a step
    at each advance, up to the frameCount(), then decrease by \a step
    back to 0, repeating forever.

    The \a state parameter is for internal use.
*/
void QtCanvasSprite::setFrameAnimation(FrameAnimationType type, int step, int state)
{
    anim_val = step;
    anim_type = type;
    anim_state = state;
    setAnimated(true);
}

/*
    Extends the default QtCanvasItem implementation to provide the
    functionality of setFrameAnimation().

    The \a phase is 0 or 1: see QtCanvasItem::advance() for details.

    \sa QtCanvasItem::advance() setVelocity()
*/
void QtCanvasSprite::advance(int phase)
{
    if (phase == 1) {
        int nf = frame();
        if (anim_type == Oscillate) {
            if (anim_state)
                nf += anim_val;
            else
                nf -= anim_val;
            if (nf < 0) {
                nf = abs(anim_val);
                anim_state = !anim_state;
            } else if (nf >= frameCount()) {
                nf = frameCount()-1-abs(anim_val);
                anim_state = !anim_state;
            }
        } else {
            nf = (nf + anim_val + frameCount()) % frameCount();
        }
        move(x()+xVelocity(), y()+yVelocity(), nf);
    }
}


/*
    \fn int QtCanvasSprite::frame() const

    Returns the index of the current animation frame in the
    QtCanvasSprite's QtCanvasPixmapArray.

    \sa setFrame(), move()
*/

/*
    \fn int QtCanvasSprite::frameCount() const

    Returns the number of frames in the QtCanvasSprite's
    QtCanvasPixmapArray.
*/


/*
    Moves the sprite to (\a x, \a y).
*/
void QtCanvasSprite::move(double x, double y) { QtCanvasItem::move(x, y); }

/*
    \fn void QtCanvasSprite::move(double nx, double ny, int nf)

    Moves the sprite to (\a nx, \a ny) and sets the current
    frame to \a nf. \a nf will be ignored if it is larger than
    frameCount() or smaller than 0.
*/
void QtCanvasSprite::move(double nx, double ny, int nf)
{
    if (isVisible() && canvas()) {
        hide();
        QtCanvasItem::move(nx, ny);
        if (nf >= 0 && nf < frameCount())
            frm = nf;
        show();
    } else {
        QtCanvasItem::move(nx, ny);
        if (nf >= 0 && nf < frameCount())
            frm = nf;
    }
}


class QPoint;

class QtPolygonScanner {
public:
    virtual ~QtPolygonScanner() {}
    void scan(const QPolygon& pa, bool winding, int index = 0, int npoints = -1);
    void scan(const QPolygon& pa, bool winding, int index, int npoints, bool stitchable);
    enum Edge { Left = 1, Right = 2, Top = 4, Bottom = 8 };
    void scan(const QPolygon& pa, bool winding, int index, int npoints, Edge edges);
    virtual void processSpans(int n, QPoint* point, int* width) = 0;
};


// Based on Xserver code miFillGeneralPoly...
/*
 *
 *     Written by Brian Kelleher;  Oct. 1985
 *
 *     Routine to fill a polygon.  Two fill rules are
 *     supported: frWINDING and frEVENODD.
 *
 *     See fillpoly.h for a complete description of the algorithm.
 */

/*
 *     These are the data structures needed to scan
 *     convert regions.  Two different scan conversion
 *     methods are available -- the even-odd method, and
 *     the winding number method.
 *     The even-odd rule states that a point is inside
 *     the polygon if a ray drawn from that point in any
 *     direction will pass through an odd number of
 *     path segments.
 *     By the winding number rule, a point is decided
 *     to be inside the polygon if a ray drawn from that
 *     point in any direction passes through a different
 *     number of clockwise and counterclockwise path
 *     segments.
 *
 *     These data structures are adapted somewhat from
 *     the algorithm in (Foley/Van Dam) for scan converting
 *     polygons.
 *     The basic algorithm is to start at the top (smallest y)
 *     of the polygon, stepping down to the bottom of
 *     the polygon by incrementing the y coordinate.  We
 *     keep a list of edges which the current scanline crosses, 
 *     sorted by x.  This list is called the Active Edge Table (AET)
 *     As we change the y-coordinate, we update each entry in
 *     in the active edge table to reflect the edges new xcoord.
 *     This list must be sorted at each scanline in case
 *     two edges intersect.
 *     We also keep a data structure known as the Edge Table (ET), 
 *     which keeps track of all the edges which the current
 *     scanline has not yet reached.  The ET is basically a
 *     list of ScanLineList structures containing a list of
 *     edges which are entered at a given scanline.  There is one
 *     ScanLineList per scanline at which an edge is entered.
 *     When we enter a new edge, we move it from the ET to the AET.
 *
 *     From the AET, we can implement the even-odd rule as in
 *     (Foley/Van Dam).
 *     The winding number rule is a little trickier.  We also
 *     keep the EdgeTableEntries in the AET linked by the
 *     nextWETE (winding EdgeTableEntry) link.  This allows
 *     the edges to be linked just as before for updating
 *     purposes, but only uses the edges linked by the nextWETE
 *     link as edges representing spans of the polygon to
 *     drawn (as with the even-odd rule).
 */

/* $XConsortium: miscanfill.h, v 1.5 94/04/17 20:27:50 dpw Exp $ */
/*

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish, 
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/


/*
 *     scanfill.h
 *
 *     Written by Brian Kelleher; Jan 1985
 *
 *     This file contains a few macros to help track
 *     the edge of a filled object.  The object is assumed
 *     to be filled in scanline order, and thus the
 *     algorithm used is an extension of Bresenham's line
 *     drawing algorithm which assumes that y is always the
 *     major axis.
 *     Since these pieces of code are the same for any filled shape, 
 *     it is more convenient to gather the library in one
 *     place, but since these pieces of code are also in
 *     the inner loops of output primitives, procedure call
 *     overhead is out of the question.
 *     See the author for a derivation if needed.
 */

/*
 *  In scan converting polygons, we want to choose those pixels
 *  which are inside the polygon.  Thus, we add .5 to the starting
 *  x coordinate for both left and right edges.  Now we choose the
 *  first pixel which is inside the pgon for the left edge and the
 *  first pixel which is outside the pgon for the right edge.
 *  Draw the left pixel, but not the right.
 *
 *  How to add .5 to the starting x coordinate:
 *      If the edge is moving to the right, then subtract dy from the
 *  error term from the general form of the algorithm.
 *      If the edge is moving to the left, then add dy to the error term.
 *
 *  The reason for the difference between edges moving to the left
 *  and edges moving to the right is simple:  If an edge is moving
 *  to the right, then we want the algorithm to flip immediately.
 *  If it is moving to the left, then we don't want it to flip until
 *  we traverse an entire pixel.
 */
#define BRESINITPGON(dy, x1, x2, xStart, d, m, m1, incr1, incr2) { \
    int dx;      /* local storage */ \
\
    /* \
     *  if the edge is horizontal, then it is ignored \
     *  and assumed not to be processed.  Otherwise, do this stuff. \
     */ \
    if ((dy) != 0) { \
        xStart = (x1); \
        dx = (x2) - xStart; \
        if (dx < 0) { \
            m = dx / (dy); \
            m1 = m - 1; \
            incr1 = -2 * dx + 2 * (dy) * m1; \
            incr2 = -2 * dx + 2 * (dy) * m; \
            d = 2 * m * (dy) - 2 * dx - 2 * (dy); \
        } else { \
            m = dx / (dy); \
            m1 = m + 1; \
            incr1 = 2 * dx - 2 * (dy) * m1; \
            incr2 = 2 * dx - 2 * (dy) * m; \
            d = -2 * m * (dy) + 2 * dx; \
        } \
    } \
}

#define BRESINCRPGON(d, minval, m, m1, incr1, incr2) { \
    if (m1 > 0) { \
        if (d > 0) { \
            minval += m1; \
            d += incr1; \
        } \
        else { \
            minval += m; \
            d += incr2; \
        } \
    } else {\
        if (d >= 0) { \
            minval += m1; \
            d += incr1; \
        } \
        else { \
            minval += m; \
            d += incr2; \
        } \
    } \
}


/*
 *     This structure contains all of the information needed
 *     to run the bresenham algorithm.
 *     The variables may be hardcoded into the declarations
 *     instead of using this structure to make use of
 *     register declarations.
 */
typedef struct {
    int minor;         /* minor axis        */
    int d;           /* decision variable */
    int m, m1;       /* slope and slope+1 */
    int incr1, incr2; /* error increments */
} BRESINFO;


#define BRESINITPGONSTRUCT(dmaj, min1, min2, bres) \
        BRESINITPGON(dmaj, min1, min2, bres.minor, bres.d, \
                     bres.m, bres.m1, bres.incr1, bres.incr2)

#define BRESINCRPGONSTRUCT(bres) \
        BRESINCRPGON(bres.d, bres.minor, bres.m, bres.m1, bres.incr1, bres.incr2)


typedef struct _EdgeTableEntry {
     int ymax;             /* ycoord at which we exit this edge. */
     BRESINFO bres;        /* Bresenham info to run the edge     */
     struct _EdgeTableEntry *next;       /* next in the list     */
     struct _EdgeTableEntry *back;       /* for insertion sort   */
     struct _EdgeTableEntry *nextWETE;   /* for winding num rule */
     int ClockWise;        /* flag for winding number rule       */
} EdgeTableEntry;


typedef struct _ScanLineList{
     int scanline;              /* the scanline represented */
     EdgeTableEntry *edgelist;  /* header node              */
     struct _ScanLineList *next;  /* next in the list       */
} ScanLineList;


typedef struct {
     int ymax;                 /* ymax for the polygon     */
     int ymin;                 /* ymin for the polygon     */
     ScanLineList scanlines;   /* header node              */
} EdgeTable;


/*
 * Here is a struct to help with storage allocation
 * so we can allocate a big chunk at a time, and then take
 * pieces from this heap when we need to.
 */
#define SLLSPERBLOCK 25

typedef struct _ScanLineListBlock {
     ScanLineList SLLs[SLLSPERBLOCK];
     struct _ScanLineListBlock *next;
} ScanLineListBlock;

/*
 * number of points to buffer before sending them off
 * to scanlines() :  Must be an even number
 */
#define NUMPTSTOBUFFER 200

/*
 *
 *     a few macros for the inner loops of the fill code where
 *     performance considerations don't allow a procedure call.
 *
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The winding number rule is in effect, so we must notify
 *     the caller when the edge has been removed so he
 *     can reorder the Winding Active Edge Table.
 */
#define EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET) { \
   if (pAET->ymax == y) {          /* leaving this edge */ \
      pPrevAET->next = pAET->next; \
      pAET = pPrevAET->next; \
      fixWAET = 1; \
      if (pAET) \
         pAET->back = pPrevAET; \
   } \
   else { \
      BRESINCRPGONSTRUCT(pAET->bres); \
      pPrevAET = pAET; \
      pAET = pAET->next; \
   } \
}


/*
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The even-odd rule is in effect.
 */
#define EVALUATEEDGEEVENODD(pAET, pPrevAET, y) { \
   if (pAET->ymax == y) {          /* leaving this edge */ \
      pPrevAET->next = pAET->next; \
      pAET = pPrevAET->next; \
      if (pAET) \
         pAET->back = pPrevAET; \
   } \
   else { \
      BRESINCRPGONSTRUCT(pAET->bres) \
      pPrevAET = pAET; \
      pAET = pAET->next; \
   } \
}

/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, 
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, 
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#define MAXINT 0x7fffffff
#define MININT -MAXINT

/*
 *     fillUtils.c
 *
 *     Written by Brian Kelleher;  Oct. 1985
 *
 *     This module contains all of the utility functions
 *     needed to scan convert a polygon.
 *
 */
/*
 *     InsertEdgeInET
 *
 *     Insert the given edge into the edge table.
 *     First we must find the correct bucket in the
 *     Edge table, then find the right slot in the
 *     bucket.  Finally, we can insert it.
 *
 */
static bool
miInsertEdgeInET(EdgeTable *ET, EdgeTableEntry *ETE, 
        int scanline, ScanLineListBlock **SLLBlock, int *iSLLBlock)
{
    register EdgeTableEntry *start, *prev;
    register ScanLineList *pSLL, *pPrevSLL;
    ScanLineListBlock *tmpSLLBlock;

    /*
     * find the right bucket to put the edge into
     */
    pPrevSLL = &ET->scanlines;
    pSLL = pPrevSLL->next;
    while (pSLL && (pSLL->scanline < scanline))
    {
        pPrevSLL = pSLL;
        pSLL = pSLL->next;
    }

    /*
     * reassign pSLL (pointer to ScanLineList) if necessary
     */
    if ((!pSLL) || (pSLL->scanline > scanline))
    {
        if (*iSLLBlock > SLLSPERBLOCK-1)
        {
            tmpSLLBlock = 
                  (ScanLineListBlock *)malloc(sizeof(ScanLineListBlock));
            if (!tmpSLLBlock)
                return false;
            (*SLLBlock)->next = tmpSLLBlock;
            tmpSLLBlock->next = 0;
            *SLLBlock = tmpSLLBlock;
            *iSLLBlock = 0;
        }
        pSLL = &((*SLLBlock)->SLLs[(*iSLLBlock)++]);

        pSLL->next = pPrevSLL->next;
        pSLL->edgelist = 0;
        pPrevSLL->next = pSLL;
    }
    pSLL->scanline = scanline;

    /*
     * now insert the edge in the right bucket
     */
    prev = 0;
    start = pSLL->edgelist;
    while (start && (start->bres.minor < ETE->bres.minor))
    {
        prev = start;
        start = start->next;
    }
    ETE->next = start;

    if (prev)
        prev->next = ETE;
    else
        pSLL->edgelist = ETE;
    return true;
}

/*
 *     CreateEdgeTable
 *
 *     This routine creates the edge table for
 *     scan converting polygons.
 *     The Edge Table (ET) looks like:
 *
 *    EdgeTable
 *     --------
 *    |  ymax  |        ScanLineLists
 *    |scanline|-->------------>-------------->...
 *     --------   |scanline|   |scanline|
 *                |edgelist|   |edgelist|
 *                ---------    ---------
 *                    |             |
 *                    |             |
 *                    V             V
 *              list of ETEs   list of ETEs
 *
 *     where ETE is an EdgeTableEntry data structure, 
 *     and there is one ScanLineList per scanline at
 *     which an edge is initially entered.
 *
 */

typedef struct {
#if defined(Q_OS_MAC)
    int y, x;
#else
    int x, y;
#endif

} DDXPointRec, *DDXPointPtr;

/*
 *     Clean up our act.
 */
static void
miFreeStorage(ScanLineListBlock   *pSLLBlock)
{
    register ScanLineListBlock   *tmpSLLBlock;

    while (pSLLBlock)
    {
        tmpSLLBlock = pSLLBlock->next;
        free(pSLLBlock);
        pSLLBlock = tmpSLLBlock;
    }
}

static bool
miCreateETandAET(int count, DDXPointPtr pts, EdgeTable *ET, 
        EdgeTableEntry *AET, EdgeTableEntry *pETEs, ScanLineListBlock *pSLLBlock)
{
    register DDXPointPtr top, bottom;
    register DDXPointPtr PrevPt, CurrPt;
    int iSLLBlock = 0;

    int dy;

    if (count < 2)  return true;

    /*
     *  initialize the Active Edge Table
     */
    AET->next = 0;
    AET->back = 0;
    AET->nextWETE = 0;
    AET->bres.minor = MININT;

    /*
     *  initialize the Edge Table.
     */
    ET->scanlines.next = 0;
    ET->ymax = MININT;
    ET->ymin = MAXINT;
    pSLLBlock->next = 0;

    PrevPt = &pts[count-1];

    /*
     *  for each vertex in the array of points.
     *  In this loop we are dealing with two vertices at
     *  a time -- these make up one edge of the polygon.
     */
    while (count--)
    {
        CurrPt = pts++;

        /*
         *  find out which point is above and which is below.
         */
        if (PrevPt->y > CurrPt->y)
        {
            bottom = PrevPt, top = CurrPt;
            pETEs->ClockWise = 0;
        }
        else
        {
            bottom = CurrPt, top = PrevPt;
            pETEs->ClockWise = 1;
        }

        /*
         * don't add horizontal edges to the Edge table.
         */
        if (bottom->y != top->y)
        {
            pETEs->ymax = bottom->y-1;  /* -1 so we don't get last scanline */

            /*
             *  initialize integer edge algorithm
             */
            dy = bottom->y - top->y;
            BRESINITPGONSTRUCT(dy, top->x, bottom->x, pETEs->bres)

            if (!miInsertEdgeInET(ET, pETEs, top->y, &pSLLBlock, &iSLLBlock))
            {
                miFreeStorage(pSLLBlock->next);
                return false;
            }

            ET->ymax = qMax(ET->ymax, PrevPt->y);
            ET->ymin = qMin(ET->ymin, PrevPt->y);
            pETEs++;
        }

        PrevPt = CurrPt;
    }
    return true;
}

/*
 *     loadAET
 *
 *     This routine moves EdgeTableEntries from the
 *     EdgeTable into the Active Edge Table, 
 *     leaving them sorted by smaller x coordinate.
 *
 */

static void
miloadAET(EdgeTableEntry *AET, EdgeTableEntry *ETEs)
{
    register EdgeTableEntry *pPrevAET;
    register EdgeTableEntry *tmp;

    pPrevAET = AET;
    AET = AET->next;
    while (ETEs)
    {
        while (AET && (AET->bres.minor < ETEs->bres.minor))
        {
            pPrevAET = AET;
            AET = AET->next;
        }
        tmp = ETEs->next;
        ETEs->next = AET;
        if (AET)
            AET->back = ETEs;
        ETEs->back = pPrevAET;
        pPrevAET->next = ETEs;
        pPrevAET = ETEs;

        ETEs = tmp;
    }
}

/*
 *     computeWAET
 *
 *     This routine links the AET by the
 *     nextWETE (winding EdgeTableEntry) link for
 *     use by the winding number rule.  The final
 *     Active Edge Table (AET) might look something
 *     like:
 *
 *     AET
 *     ----------  ---------   ---------
 *     |ymax    |  |ymax    |  |ymax    |
 *     | ...    |  |...     |  |...     |
 *     |next    |->|next    |->|next    |->...
 *     |nextWETE|  |nextWETE|  |nextWETE|
 *     ---------   ---------   ^--------
 *         |                   |       |
 *         V------------------->       V---> ...
 *
 */
static void
micomputeWAET(EdgeTableEntry *AET)
{
    register EdgeTableEntry *pWETE;
    register int inside = 1;
    register int isInside = 0;

    AET->nextWETE = 0;
    pWETE = AET;
    AET = AET->next;
    while (AET)
    {
        if (AET->ClockWise)
            isInside++;
        else
            isInside--;

        if ((!inside && !isInside) ||
            (inside &&  isInside))
        {
            pWETE->nextWETE = AET;
            pWETE = AET;
            inside = !inside;
        }
        AET = AET->next;
    }
    pWETE->nextWETE = 0;
}

/*
 *     InsertionSort
 *
 *     Just a simple insertion sort using
 *     pointers and back pointers to sort the Active
 *     Edge Table.
 *
 */

static int
miInsertionSort(EdgeTableEntry *AET)
{
    register EdgeTableEntry *pETEchase;
    register EdgeTableEntry *pETEinsert;
    register EdgeTableEntry *pETEchaseBackTMP;
    register int changed = 0;

    AET = AET->next;
    while (AET)
    {
        pETEinsert = AET;
        pETEchase = AET;
        while (pETEchase->back->bres.minor > AET->bres.minor)
            pETEchase = pETEchase->back;

        AET = AET->next;
        if (pETEchase != pETEinsert)
        {
            pETEchaseBackTMP = pETEchase->back;
            pETEinsert->back->next = AET;
            if (AET)
                AET->back = pETEinsert->back;
            pETEinsert->next = pETEchase;
            pETEchase->back->next = pETEinsert;
            pETEchase->back = pETEinsert;
            pETEinsert->back = pETEchaseBackTMP;
            changed = 1;
        }
    }
    return changed;
}

/*
    \overload
*/
void QtPolygonScanner::scan(const QPolygon& pa, bool winding, int index, int npoints)
{
    scan(pa, winding, index, npoints, true);
}

/*
    \overload

    If \a stitchable is false, the right and bottom edges of the
    polygon are included. This causes adjacent polygons to overlap.
*/
void QtPolygonScanner::scan(const QPolygon& pa, bool winding, int index, int npoints, bool stitchable)
{
    scan(pa, winding, index, npoints, 
        stitchable ? Edge(Left+Top) : Edge(Left+Right+Top+Bottom));
}

/*
    Calls processSpans() for all scanlines of the polygon defined by
    \a npoints starting at \a index in \a pa.

    If \a winding is true, the Winding algorithm rather than the
    Odd-Even rule is used.

    The \a edges is any bitwise combination of:
    \list
    \i QtPolygonScanner::Left
    \i QtPolygonScanner::Right
    \i QtPolygonScanner::Top
    \i QtPolygonScanner::Bottom
    \endlist
    \a edges determines which edges are included.

    \warning The edges feature does not work properly.

*/
void QtPolygonScanner::scan(const QPolygon& pa, bool winding, int index, int npoints, Edge edges)
{


    DDXPointPtr ptsIn = (DDXPointPtr)pa.data();
    ptsIn += index;
    register EdgeTableEntry *pAET;  /* the Active Edge Table   */
    register int y;                 /* the current scanline    */
    register int nPts = 0;          /* number of pts in buffer */
    register EdgeTableEntry *pWETE; /* Winding Edge Table      */
    register ScanLineList *pSLL;    /* Current ScanLineList    */
    register DDXPointPtr ptsOut;      /* ptr to output buffers   */
    int *width;
    DDXPointRec FirstPoint[NUMPTSTOBUFFER]; /* the output buffers */
    int FirstWidth[NUMPTSTOBUFFER];
    EdgeTableEntry *pPrevAET;       /* previous AET entry      */
    EdgeTable ET;                   /* Edge Table header node  */
    EdgeTableEntry AET;             /* Active ET header node   */
    EdgeTableEntry *pETEs;          /* Edge Table Entries buff */
    ScanLineListBlock SLLBlock;     /* header for ScanLineList */
    int fixWAET = 0;
    int edge_l = (edges & Left) ? 1 : 0;
    int edge_r = (edges & Right) ? 1 : 0;
    int edge_t = 1; //#### (edges & Top) ? 1 : 0;
    int edge_b = (edges & Bottom) ? 1 : 0;

    if (npoints == -1)
        npoints = pa.size();

    if (npoints < 3)
        return;

    if(!(pETEs = (EdgeTableEntry *)
        malloc(sizeof(EdgeTableEntry) * npoints)))
        return;
    ptsOut = FirstPoint;
    width = FirstWidth;
    if (!miCreateETandAET(npoints, ptsIn, &ET, &AET, pETEs, &SLLBlock))
    {
        free(pETEs);
        return;
    }
    pSLL = ET.scanlines.next;

    if (!winding)
    {
        /*
         *  for each scanline
         */
        for (y = ET.ymin+1-edge_t; y < ET.ymax+edge_b; y++)
        {
            /*
             *  Add a new edge to the active edge table when we
             *  get to the next edge.
             */
            if (pSLL && y == pSLL->scanline)
            {
                miloadAET(&AET, pSLL->edgelist);
                pSLL = pSLL->next;
            }
            pPrevAET = &AET;
            pAET = AET.next;

            /*
             *  for each active edge
             */
            while (pAET)
            {
                ptsOut->x = pAET->bres.minor + 1 - edge_l;
                ptsOut++->y = y;
                *width++ = pAET->next->bres.minor - pAET->bres.minor
                    - 1 + edge_l + edge_r;
                nPts++;

                /*
                 *  send out the buffer when its full
                 */
                if (nPts == NUMPTSTOBUFFER)
                {
                    processSpans(nPts, (QPoint*)FirstPoint, FirstWidth);
                    ptsOut = FirstPoint;
                    width = FirstWidth;
                    nPts = 0;
                }
                EVALUATEEDGEEVENODD(pAET, pPrevAET, y)
                EVALUATEEDGEEVENODD(pAET, pPrevAET, y)
            }
            miInsertionSort(&AET);
        }
    }
    else      /* default to WindingNumber */
    {
        /*
         *  for each scanline
         */
        for (y = ET.ymin+1-edge_t; y < ET.ymax+edge_b; y++)
        {
            /*
             *  Add a new edge to the active edge table when we
             *  get to the next edge.
             */
            if (pSLL && y == pSLL->scanline)
            {
                miloadAET(&AET, pSLL->edgelist);
                micomputeWAET(&AET);
                pSLL = pSLL->next;
            }
            pPrevAET = &AET;
            pAET = AET.next;
            pWETE = pAET;

            /*
             *  for each active edge
             */
            while (pAET)
            {
                /*
                 *  if the next edge in the active edge table is
                 *  also the next edge in the winding active edge
                 *  table.
                 */
                if (pWETE == pAET)
                {
                    ptsOut->x = pAET->bres.minor + 1 - edge_l;
                    ptsOut++->y = y;
                    *width++ = pAET->nextWETE->bres.minor - pAET->bres.minor - 1 + edge_l + edge_r;
                    nPts++;

                    /*
                     *  send out the buffer
                     */
                    if (nPts == NUMPTSTOBUFFER)
                    {
                        processSpans(nPts, (QPoint*)FirstPoint, FirstWidth);
                        ptsOut = FirstPoint;
                        width  = FirstWidth;
                        nPts = 0;
                    }

                    pWETE = pWETE->nextWETE;
                    while (pWETE != pAET) {
                        EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET)
                    }
                    pWETE = pWETE->nextWETE;
                }
                EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET)
            }

            /*
             *  reevaluate the Winding active edge table if we
             *  just had to resort it or if we just exited an edge.
             */
            if (miInsertionSort(&AET) || fixWAET)
            {
                micomputeWAET(&AET);
                fixWAET = 0;
            }
        }
    }

    /*
     *     Get any spans that we missed by buffering
     */


    processSpans(nPts, (QPoint*)FirstPoint, FirstWidth);
    free(pETEs);
    miFreeStorage(SLLBlock.next);
}
/***** END OF X11-based CODE *****/





class QtCanvasPolygonScanner : public QtPolygonScanner {
    QPolygonalProcessor& processor;
public:
    QtCanvasPolygonScanner(QPolygonalProcessor& p) :
        processor(p)
    {
    }
    void processSpans(int n, QPoint* point, int* width)
    {
        processor.doSpans(n, point, width);
    }
};

void QtCanvasPolygonalItem::scanPolygon(const QPolygon& pa, int winding, QPolygonalProcessor& process) const
{
    QtCanvasPolygonScanner scanner(process);
    scanner.scan(pa, winding);
}
