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

#ifndef QTCANVAS_H
#define QTCANVAS_H

#include <qpixmap.h>
#include <qbrush.h>
#include <qpen.h>
#include <qpolygon.h>
#include <qscrollarea.h>

class QtCanvasSprite;
class QtCanvasPolygonalItem;
class QtCanvasRectangle;
class QtCanvasPolygon;
class QtCanvasEllipse;
class QtCanvasText;
class QtCanvasLine;
class QtCanvasChunk;
class QtCanvas;
class QtCanvasItem;
class QtCanvasView;
class QtCanvasPixmap;

typedef QList<QtCanvasItem *> QtCanvasItemList;


class QtCanvasItemExtra;

class QtCanvasItem
{
public:
    QtCanvasItem(QtCanvas* canvas);
    virtual ~QtCanvasItem();

    double x() const
	{ return myx; }
    double y() const
	{ return myy; }
    double z() const
	{ return myz; } // (depth)

    virtual void moveBy(double dx, double dy);
    void move(double x, double y);
    void setX(double a) { move(a,y()); }
    void setY(double a) { move(x(),a); }
    void setZ(double a) { myz=a; changeChunks(); }

    bool animated() const;
    virtual void setAnimated(bool y);
    virtual void setVelocity(double vx, double vy);
    void setXVelocity(double vx) { setVelocity(vx,yVelocity()); }
    void setYVelocity(double vy) { setVelocity(xVelocity(),vy); }
    double xVelocity() const;
    double yVelocity() const;
    virtual void advance(int stage);

    virtual bool collidesWith(const QtCanvasItem*) const=0;

    QtCanvasItemList collisions(bool exact /* NO DEFAULT */) const;

    virtual void setCanvas(QtCanvas*);

    virtual void draw(QPainter&)=0;

    void show();
    void hide();

    virtual void setVisible(bool yes);
    bool isVisible() const
	{ return (bool)vis; }
    virtual void setSelected(bool yes);
    bool isSelected() const
	{ return (bool)sel; }
    virtual void setEnabled(bool yes);
    bool isEnabled() const
	{ return (bool)ena; }
    virtual void setActive(bool yes);
    bool isActive() const
	{ return (bool)act; }
    bool visible() const
	{ return (bool)vis; }
    bool selected() const
	{ return (bool)sel; }
    bool enabled() const
	{ return (bool)ena; }
    bool active() const
	{ return (bool)act; }

    enum RttiValues {
	Rtti_Item = 0,
	Rtti_Sprite = 1,
	Rtti_PolygonalItem = 2,
	Rtti_Text = 3,
	Rtti_Polygon = 4,
	Rtti_Rectangle = 5,
	Rtti_Ellipse = 6,
	Rtti_Line = 7,
	Rtti_Spline = 8
    };

    virtual int rtti() const;
    static int RTTI;

    virtual QRect boundingRect() const=0;
    virtual QRect boundingRectAdvanced() const;

    QtCanvas* canvas() const
	{ return cnv; }

protected:
    void update() { changeChunks(); }

private:
    // For friendly subclasses...

    friend class QtCanvasPolygonalItem;
    friend class QtCanvasSprite;
    friend class QtCanvasRectangle;
    friend class QtCanvasPolygon;
    friend class QtCanvasEllipse;
    friend class QtCanvasText;
    friend class QtCanvasLine;

    virtual QPolygon chunks() const;
    virtual void addToChunks();
    virtual void removeFromChunks();
    virtual void changeChunks();
    virtual bool collidesWith(const QtCanvasSprite*,
			       const QtCanvasPolygonalItem*,
			       const QtCanvasRectangle*,
			       const QtCanvasEllipse*,
			       const QtCanvasText*) const = 0;
    // End of friend stuff

    QtCanvas* cnv;
    static QtCanvas* current_canvas;
    double myx,myy,myz;
    QtCanvasItemExtra *ext;
    QtCanvasItemExtra& extra();
    uint ani:1;
    uint vis:1;
    uint val:1;
    uint sel:1;
    uint ena:1;
    uint act:1;
};


class QtCanvasData;

class QtCanvas : public QObject
{
    Q_OBJECT
public:
    QtCanvas(QObject* parent = 0);
    QtCanvas(int w, int h);
    QtCanvas(QPixmap p, int h, int v, int tilewidth, int tileheight);

    virtual ~QtCanvas();

    virtual void setTiles(QPixmap tiles, int h, int v,
			   int tilewidth, int tileheight);
    virtual void setBackgroundPixmap(const QPixmap& p);
    QPixmap backgroundPixmap() const;

    virtual void setBackgroundColor(const QColor& c);
    QColor backgroundColor() const;

    virtual void setTile(int x, int y, int tilenum);
    int tile(int x, int y) const
	{ return grid[x+y*htiles]; }

    int tilesHorizontally() const
	{ return htiles; }
    int tilesVertically() const
	{ return vtiles; }

    int tileWidth() const
	{ return tilew; }
    int tileHeight() const
	{ return tileh; }

    virtual void resize(int width, int height);
    int width() const
	{ return awidth; }
    int height() const
	{ return aheight; }
    QSize size() const
	{ return QSize(awidth,aheight); }
    QRect rect() const
	{ return QRect(0, 0, awidth, aheight); }
    bool onCanvas(int x, int y) const
	{ return x>=0 && y>=0 && x<awidth && y<aheight; }
    bool onCanvas(const QPoint& p) const
	{ return onCanvas(p.x(),p.y()); }
    bool validChunk(int x, int y) const
	{ return x>=0 && y>=0 && x<chwidth && y<chheight; }
    bool validChunk(const QPoint& p) const
	{ return validChunk(p.x(),p.y()); }

    int chunkSize() const
	{ return chunksize; }
    virtual void retune(int chunksize, int maxclusters=100);

    bool sameChunk(int x1, int y1, int x2, int y2) const
	{ return x1/chunksize==x2/chunksize && y1/chunksize==y2/chunksize; }
    virtual void setChangedChunk(int i, int j);
    virtual void setChangedChunkContaining(int x, int y);
    virtual void setAllChanged();
    virtual void setChanged(const QRect& area);
    virtual void setUnchanged(const QRect& area);

    // These call setChangedChunk.
    void addItemToChunk(QtCanvasItem*, int i, int j);
    void removeItemFromChunk(QtCanvasItem*, int i, int j);
    void addItemToChunkContaining(QtCanvasItem*, int x, int y);
    void removeItemFromChunkContaining(QtCanvasItem*, int x, int y);

    QtCanvasItemList allItems();
    QtCanvasItemList collisions(const QPoint&) const;
    QtCanvasItemList collisions(const QRect&) const;
    QtCanvasItemList collisions(const QPolygon& pa, const QtCanvasItem* item,
				bool exact) const;

    void drawArea(const QRect&, QPainter* p, bool double_buffer=false);

    // These are for QtCanvasView to call
    virtual void addView(QtCanvasView*);
    virtual void removeView(QtCanvasView*);

    void drawCanvasArea(const QRect&, QPainter* p=0, bool double_buffer=true);
    void drawViewArea(QtCanvasView* view, QPainter* p, const QRect& r, bool dbuf);

    // These are for QtCanvasItem to call
    virtual void addItem(QtCanvasItem*);
    virtual void addAnimation(QtCanvasItem*);
    virtual void removeItem(QtCanvasItem*);
    virtual void removeAnimation(QtCanvasItem*);

    virtual void setAdvancePeriod(int ms);
    virtual void setUpdatePeriod(int ms);

signals:
    void resized();

public slots:
    virtual void advance();
    virtual void update();

protected:
    virtual void drawBackground(QPainter&, const QRect& area);
    virtual void drawForeground(QPainter&, const QRect& area);

private:
    void init(int w, int h, int chunksze=16, int maxclust=100);

    QtCanvasChunk& chunk(int i, int j) const;
    QtCanvasChunk& chunkContaining(int x, int y) const;

    QRect changeBounds();

    int awidth,aheight;
    int chunksize;
    int maxclusters;
    int chwidth,chheight;
    QtCanvasChunk* chunks;

    QtCanvasData* d;

    void initTiles(QPixmap p, int h, int v, int tilewidth, int tileheight);
    ushort *grid;
    ushort htiles;
    ushort vtiles;
    ushort tilew;
    ushort tileh;
    bool oneone;
    QPixmap pm;
    QTimer* update_timer;
    QColor bgcolor;
    bool debug_redraw_areas;

    friend void qt_unview(QtCanvas* c);

    Q_DISABLE_COPY(QtCanvas)
};

class QtCanvasViewData;

class QtCanvasView : public QScrollArea
{
    Q_OBJECT
    Q_PROPERTY(bool highQualityRendering READ highQualityRendering WRITE setHighQualityRendering)
public:

    QtCanvasView(QWidget* parent=0);
    QtCanvasView(QtCanvas* viewing, QWidget* parent=0);
    ~QtCanvasView();

    QtCanvas* canvas() const
	{ return viewing; }
    void setCanvas(QtCanvas* v);

    const QMatrix &worldMatrix() const;
    const QMatrix &inverseWorldMatrix() const;
    bool setWorldMatrix(const QMatrix &);

    virtual QSize sizeHint() const;

    bool highQualityRendering() const;
public slots:
    void setHighQualityRendering(bool enable);
    
protected:
    friend class QtCanvasWidget;
    virtual void drawContents(QPainter *p, int cx, int cy, int cw, int ch);

    virtual void contentsMousePressEvent( QMouseEvent* );
    virtual void contentsMouseReleaseEvent( QMouseEvent* );
    virtual void contentsMouseDoubleClickEvent( QMouseEvent* );
    virtual void contentsMouseMoveEvent( QMouseEvent* );
    virtual void contentsDragEnterEvent( QDragEnterEvent * );
    virtual void contentsDragMoveEvent( QDragMoveEvent * );
    virtual void contentsDragLeaveEvent( QDragLeaveEvent * );
    virtual void contentsDropEvent( QDropEvent * );
    virtual void contentsWheelEvent( QWheelEvent * );
    virtual void contentsContextMenuEvent( QContextMenuEvent * );

private:
    friend class QtCanvas;
    void drawContents(QPainter*);
    QtCanvas* viewing;
    QtCanvasViewData* d;

private slots:
    void updateContentsSize();

private:
    Q_DISABLE_COPY(QtCanvasView)
};


class QtCanvasPixmap : public QPixmap
{
public:
#ifndef QT_NO_IMAGEIO
    QtCanvasPixmap(const QString& datafilename);
#endif
    QtCanvasPixmap(const QImage& image);
    QtCanvasPixmap(const QPixmap&, const QPoint& hotspot);
    ~QtCanvasPixmap();

    int offsetX() const
	{ return hotx; }
    int offsetY() const
	{ return hoty; }
    void setOffset(int x, int y) { hotx = x; hoty = y; }

private:
    Q_DISABLE_COPY(QtCanvasPixmap)

    void init(const QImage&);
    void init(const QPixmap& pixmap, int hx, int hy);

    friend class QtCanvasSprite;
    friend class QtCanvasPixmapArray;
    friend bool qt_testCollision(const QtCanvasSprite* s1, const QtCanvasSprite* s2);

    int hotx,hoty;

    QImage* collision_mask;
};


class QtCanvasPixmapArray
{
public:
    QtCanvasPixmapArray();
#ifndef QT_NO_IMAGEIO
    QtCanvasPixmapArray(const QString& datafilenamepattern, int framecount=0);
#endif
    QtCanvasPixmapArray(const QList<QPixmap> &pixmaps, const QPolygon &hotspots = QPolygon());
    ~QtCanvasPixmapArray();

#ifndef QT_NO_IMAGEIO
    bool readPixmaps(const QString& datafilenamepattern, int framecount=0);
    bool readCollisionMasks(const QString& filenamepattern);
#endif

    // deprecated
    bool operator!(); // Failure check.
    bool isValid() const;

    QtCanvasPixmap* image(int i) const
	{ return img ? img[i] : 0; }
    void setImage(int i, QtCanvasPixmap* p);
    uint count() const
	{ return (uint)framecount; }

private:
    Q_DISABLE_COPY(QtCanvasPixmapArray)

#ifndef QT_NO_IMAGEIO
    bool readPixmaps(const QString& datafilenamepattern, int framecount, bool maskonly);
#endif

    void reset();
    int framecount;
    QtCanvasPixmap** img;
};


class QtCanvasSprite : public QtCanvasItem
{
public:
    QtCanvasSprite(QtCanvasPixmapArray* array, QtCanvas* canvas);

    void setSequence(QtCanvasPixmapArray* seq);

    virtual ~QtCanvasSprite();

    void move(double x, double y);
    virtual void move(double x, double y, int frame);
    void setFrame(int);
    enum FrameAnimationType { Cycle, Oscillate };
    virtual void setFrameAnimation(FrameAnimationType=Cycle, int step=1, int state=0);
    int frame() const
	{ return frm; }
    int frameCount() const
	{ return images->count(); }

    int rtti() const;
    static int RTTI;

    bool collidesWith(const QtCanvasItem*) const;

    QRect boundingRect() const;

    // is there a reason for these to be protected? Lars
//protected:

    int width() const;
    int height() const;

    int leftEdge() const;
    int topEdge() const;
    int rightEdge() const;
    int bottomEdge() const;

    int leftEdge(int nx) const;
    int topEdge(int ny) const;
    int rightEdge(int nx) const;
    int bottomEdge(int ny) const;
    QtCanvasPixmap* image() const
	{ return images->image(frm); }
    virtual QtCanvasPixmap* imageAdvanced() const;
    QtCanvasPixmap* image(int f) const
	{ return images->image(f); }
    virtual void advance(int stage);

public:
    void draw(QPainter& painter);

private:
    Q_DISABLE_COPY(QtCanvasSprite)

    void addToChunks();
    void removeFromChunks();
    void changeChunks();

    int frm;
    ushort anim_val;
    uint anim_state:2;
    uint anim_type:14;
    bool collidesWith(const QtCanvasSprite*,
		       const QtCanvasPolygonalItem*,
		       const QtCanvasRectangle*,
		       const QtCanvasEllipse*,
		       const QtCanvasText*) const;

    friend bool qt_testCollision(const QtCanvasSprite* s1,
				  const QtCanvasSprite* s2);

    QtCanvasPixmapArray* images;
};

class QPolygonalProcessor;

class QtCanvasPolygonalItem : public QtCanvasItem
{
public:
    QtCanvasPolygonalItem(QtCanvas* canvas);
    virtual ~QtCanvasPolygonalItem();

    bool collidesWith(const QtCanvasItem*) const;

    virtual void setPen(QPen p);
    virtual void setBrush(QBrush b);

    QPen pen() const
	{ return pn; }
    QBrush brush() const
	{ return br; }

    virtual QPolygon areaPoints() const=0;
    virtual QPolygon areaPointsAdvanced() const;
    QRect boundingRect() const;

    int rtti() const;
    static int RTTI;

protected:
    void draw(QPainter &);
    virtual void drawShape(QPainter &) = 0;

    bool winding() const;
    void setWinding(bool);

    void invalidate();
    bool isValid() const
	{ return (bool)val; }

private:
    void scanPolygon(const QPolygon& pa, int winding,
		      QPolygonalProcessor& process) const;
    QPolygon chunks() const;

    bool collidesWith(const QtCanvasSprite*,
		       const QtCanvasPolygonalItem*,
		       const QtCanvasRectangle*,
		       const QtCanvasEllipse*,
		       const QtCanvasText*) const;

    QBrush br;
    QPen pn;
    uint wind:1;
};


class QtCanvasRectangle : public QtCanvasPolygonalItem
{
public:
    QtCanvasRectangle(QtCanvas* canvas);
    QtCanvasRectangle(const QRect&, QtCanvas* canvas);
    QtCanvasRectangle(int x, int y, int width, int height, QtCanvas* canvas);

    ~QtCanvasRectangle();

    int width() const;
    int height() const;
    void setSize(int w, int h);
    QSize size() const
	{ return QSize(w,h); }
    QPolygon areaPoints() const;
    QRect rect() const
	{ return QRect(int(x()),int(y()),w,h); }

    bool collidesWith(const QtCanvasItem*) const;

    int rtti() const;
    static int RTTI;

protected:
    void drawShape(QPainter &);
    QPolygon chunks() const;

private:
    bool collidesWith(  const QtCanvasSprite*,
			 const QtCanvasPolygonalItem*,
			 const QtCanvasRectangle*,
			 const QtCanvasEllipse*,
			 const QtCanvasText*) const;

    int w, h;
};


class QtCanvasPolygon : public QtCanvasPolygonalItem
{
public:
    QtCanvasPolygon(QtCanvas* canvas);
    ~QtCanvasPolygon();
    void setPoints(QPolygon);
    QPolygon points() const;
    void moveBy(double dx, double dy);

    QPolygon areaPoints() const;

    int rtti() const;
    static int RTTI;

protected:
    void drawShape(QPainter &);
    QPolygon poly;
};


class QtCanvasSpline : public QtCanvasPolygon
{
public:
    QtCanvasSpline(QtCanvas* canvas);
    ~QtCanvasSpline();

    void setControlPoints(QPolygon, bool closed=true);
    QPolygon controlPoints() const;
    bool closed() const;

    int rtti() const;
    static int RTTI;

private:
    void recalcPoly();
    QPolygon bez;
    bool cl;
};


class QtCanvasLine : public QtCanvasPolygonalItem
{
public:
    QtCanvasLine(QtCanvas* canvas);
    ~QtCanvasLine();
    void setPoints(int x1, int y1, int x2, int y2);

    QPoint startPoint() const
	{ return QPoint(x1,y1); }
    QPoint endPoint() const
	{ return QPoint(x2,y2); }

    int rtti() const;
    static int RTTI;

    void setPen(QPen p);
    void moveBy(double dx, double dy);

protected:
    void drawShape(QPainter &);
    QPolygon areaPoints() const;

private:
    int x1,y1,x2,y2;
};


class QtCanvasEllipse : public QtCanvasPolygonalItem
{

public:
    QtCanvasEllipse(QtCanvas* canvas);
    QtCanvasEllipse(int width, int height, QtCanvas* canvas);
    QtCanvasEllipse(int width, int height, int startangle, int angle,
		    QtCanvas* canvas);

    ~QtCanvasEllipse();

    int width() const;
    int height() const;
    void setSize(int w, int h);
    void setAngles(int start, int length);
    int angleStart() const
	{ return a1; }
    int angleLength() const
	{ return a2; }
    QPolygon areaPoints() const;

    bool collidesWith(const QtCanvasItem*) const;

    int rtti() const;
    static int RTTI;

protected:
    void drawShape(QPainter &);

private:
    bool collidesWith(const QtCanvasSprite*,
		       const QtCanvasPolygonalItem*,
		       const QtCanvasRectangle*,
		       const QtCanvasEllipse*,
		       const QtCanvasText*) const;
    int w, h;
    int a1, a2;
};


class QtCanvasTextExtra;

class QtCanvasText : public QtCanvasItem
{
public:
    QtCanvasText(QtCanvas* canvas);
    QtCanvasText(const QString&, QtCanvas* canvas);
    QtCanvasText(const QString&, QFont, QtCanvas* canvas);

    virtual ~QtCanvasText();

    void setText(const QString&);
    void setFont(const QFont&);
    void setColor(const QColor&);
    QString text() const;
    QFont font() const;
    QColor color() const;

    void moveBy(double dx, double dy);

    int textFlags() const
	{ return flags; }
    void setTextFlags(int);

    QRect boundingRect() const;

    bool collidesWith(const QtCanvasItem*) const;

    int rtti() const;
    static int RTTI;

protected:
    virtual void draw(QPainter&);

private:
    Q_DISABLE_COPY(QtCanvasText)

    void addToChunks();
    void removeFromChunks();
    void changeChunks();

    void setRect();
    QRect brect;
    QString txt;
    int flags;
    QFont fnt;
    QColor col;
    QtCanvasTextExtra* extra;

    bool collidesWith(const QtCanvasSprite*,
                      const QtCanvasPolygonalItem*,
                      const QtCanvasRectangle*,
                      const QtCanvasEllipse*,
                      const QtCanvasText*) const;
};

#endif // QTCANVAS_H
