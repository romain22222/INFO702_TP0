/****************************************************************************
** Author: J.-O. Lachaud, University Savoie Mont Blanc
** (adapted from Qt colliding mices example)
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
****************************************************************************/

#include <cmath>
#include <cassert>
#include <QGraphicsScene>
#include <QRandomGenerator>
#include <QPainter>
#include <QStyleOption>
#include <QBitmap>
#include <QImage>
#include "objects.hpp"
#include <iostream>

static const double Pi = 3.14159265358979323846264338327950288419717;
// static double TwoPi = 2.0 * Pi;

// Global variables for simplicity.
QRandomGenerator RG;
LogicalScene *logical_scene = 0;

///////////////////////////////////////////////////////////////////////////////
// class Disk
///////////////////////////////////////////////////////////////////////////////

Disk::Disk(qreal r, const MasterShape *master_shape)
        : _r(r), _master_shape(master_shape) {}

QPointF
Disk::randomPoint() const
{
    QPointF p;
    do
    {
        p = QPointF((RG.generateDouble() * 2.0 - 1.0),
                                (RG.generateDouble() * 2.0 - 1.0));
    } while ((p.x() * p.x() + p.y() * p.y()) > 1.0);
    return p * _r;
}

bool Disk::isInside(const QPointF &p) const
{
    return QPointF::dotProduct(p, p) <= _r * _r;
}

QRectF
Disk::boundingRect() const
{
    return QRectF(-_r, -_r, 2.0 * _r, 2.0 * _r);
}

void Disk::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(_master_shape->currentColor());
    painter->drawEllipse(QPointF(0.0, 0.0), _r, _r);
}
///////////////////////////////////////////////////////////////////////////////
// class Rectangle
///////////////////////////////////////////////////////////////////////////////
Rectangle::Rectangle(QPointF upLeft, QPointF downRight, const MasterShape *master_shape)
        : _ul(upLeft), _dr(downRight), _master_shape(master_shape) {}

QPointF
Rectangle::randomPoint() const
{
    return QPointF(RG.generateDouble() * (_dr.x() - _ul.x()) + _ul.x(),
                                 RG.generateDouble() * (_dr.y() - _ul.y()) + _ul.y());
}

bool Rectangle::isInside(const QPointF &p) const
{
    return (p.x() >= _ul.x()) && (p.x() <= _dr.x()) && (p.y() >= _ul.y()) && (p.y() <= _dr.y());
}

QRectF
Rectangle::boundingRect() const
{
    return QRectF(_ul, _dr);
}

void Rectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(_master_shape->currentColor());
    painter->drawRect(QRectF(_ul, _dr));
}

///////////////////////////////////////////////////////////////////////////////
// class Union
///////////////////////////////////////////////////////////////////////////////

Union::Union(GraphicalShape *s1, GraphicalShape *s2)
        : _s1(s1), _s2(s2), _state()
{
    _s1->setParentItem(this);
    _s2->setParentItem(this);
}

Union::Union()
        : _s1(nullptr), _s2(nullptr), _state() {}

QPointF
Union::randomPoint() const
{
    _state = !_state;
    return _state ? _s1->randomPoint() : _s2->randomPoint();
}

bool Union::isInside(const QPointF &p) const
{
    return _s1->isInside(p) || _s2->isInside(p);
}

QRectF
Union::boundingRect() const
{
    return _s1->boundingRect() | _s2->boundingRect();
}

void Union::paint(QPainter *painter, const QStyleOptionGraphicsItem *s, QWidget *w)
{
    //_s1->paint(painter, s, w);
    //_s2->paint(painter, s, w);
}
///////////////////////////////////////////////////////////////////////////////
// class Transformation
///////////////////////////////////////////////////////////////////////////////

double degreToRadian(double angle)
{
    return angle * Pi / 180;
}

Transformation::Transformation(GraphicalShape *f, QPointF dx, qreal angle)
        : _f(f), _dx(dx), _a(angle)
{
    _f->setParentItem(this);
    setPos(pos() + _dx);
    setRotation(rotation() + _a);
};

QPointF
Transformation::randomPoint() const
{
    QPointF rp = _f->randomPoint();
    double radA = degreToRadian(_a);
    return QPointF(rp.x() * ::cos(radA) - rp.y() * ::sin(radA), rp.y() * ::cos(radA) + rp.x() * ::sin(radA)) + _dx;
}

bool Transformation::isInside(const QPointF &p) const
{
    QPointF untP = p - _dx;
    double radA = degreToRadian(_a);
    return _f->isInside(
            QPointF(
                    untP.x() * ::cos(-radA) - untP.y() * ::sin(-radA),
                    untP.y() * ::cos(-radA) + untP.x() * ::sin(-radA)));
}

QRectF
Transformation::boundingRect() const
{
    return _f->boundingRect();
}

void Transformation::setAngle(double a)
{
    setRotation(a);
    _a = a;
}

void Transformation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *w)
{
    //_f->paint(painter, option, w);
}

///////////////////////////////////////////////////////////////////////////////
// ImageShape
///////////////////////////////////////////////////////////////////////////////

ImageShape::ImageShape(const QPixmap &pixmap, const MasterShape *master_shape)
        : _pixmap(pixmap), _master_shape(master_shape)
{
    _mask = _pixmap.mask();
    _mask_img = QImage(_mask.toImage().convertToFormat(QImage::Format_Mono));
}

QPointF
ImageShape::randomPoint() const
{
    int x, y;
    do
    {
        x = _mask_img.width() * RG.generateDouble();
        y = _mask_img.height() * RG.generateDouble();
    } while (_mask_img.pixelIndex(x, y) == 0);
    return QPointF(x, y);
}

bool ImageShape::isInside(const QPointF &p) const
{

    return _mask_img.valid(p.x(), p.y()) && _mask_img.pixelIndex(p.x(), p.y()) != 0;
}

QRectF
ImageShape::boundingRect() const
{
    return QRectF(pos(), _pixmap.size());
}

void ImageShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->drawPixmap(QPointF(0.0, 0.0), _pixmap);
    if (_master_shape->currentState() == MasterShape::Collision)
    {
        painter->setOpacity(0.5);
        painter->setBackgroundMode(Qt::TransparentMode);
        painter->setPen(_master_shape->currentColor());
        painter->drawPixmap(QPointF(0.0, 0.0), _mask);
    }
}

///////////////////////////////////////////////////////////////////////////////
// class NiceAsteroid
///////////////////////////////////////////////////////////////////////////////

NiceAsteroid::NiceAsteroid(QColor cok, QColor cko, double speed, QPixmap& asteroid_pixmap)
        : MasterShape(cok, cko), _speed(speed)
{
    ImageShape* i = new ImageShape(asteroid_pixmap, this);
    // transformation
    Transformation* t1 = new Transformation(i, QPointF(IMAGE_SIZE / 2., IMAGE_SIZE / 2.), 0);
    Transformation* t2 = new Transformation(t1, QPointF(0, 0), 10.);
    this->setGraphicalShape(t2);
    _t=t2;
}

void NiceAsteroid::advance(int step)
{
    if (!step)
        return;
    setPos(mapToParent(_speed, 0.0));
    _t->setAngle(_t->_a + 2.0);
    MasterShape::advance(step);
}

///////////////////////////////////////////////////////////////////////////////
// class MasterShape
///////////////////////////////////////////////////////////////////////////////

MasterShape::MasterShape(QColor cok, QColor cko)
        : _f(0), _state(Ok), _cok(cok), _cko(cko)
{
}

void MasterShape::setGraphicalShape(GraphicalShape *f)
{
    _f = f;
    if (_f != 0)
        _f->setParentItem(this);
}

QColor
MasterShape::currentColor() const
{
    if (_state == Ok)
        return _cok;
    else
        return _cko;
}

MasterShape::State
MasterShape::currentState() const
{
    return _state;
}

void MasterShape::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
    // nothing to do, Qt automatically calls paint of every QGraphicsItem
}

void MasterShape::advance(int step)
{
    if (!step)
        return;

    // (I) Garde les objets dans la scene.
    auto p = scenePos(); // pareil que pos si MasterShape est bien à la racine.
    // pos() est dans les coordonnées parent et setPos aussi.
    if (p.x() < -SZ_BD)
    {
        auto point = parentItem() != 0
                                         ? parentItem()->mapFromScene(QPointF(IMAGE_SIZE + SZ_BD - 1, p.y()))
                                         : QPointF(IMAGE_SIZE + SZ_BD - 1, p.y());
        setPos(point);
    }
    else if (p.x() > IMAGE_SIZE + SZ_BD)
    {
        auto point = parentItem() != 0
                                         ? parentItem()->mapFromScene(QPointF(-SZ_BD + 1, p.y()))
                                         : QPointF(-SZ_BD + 1, p.y());
        setPos(point);
    }
    if (p.y() < -SZ_BD)
    {
        auto point = parentItem() != 0 ? parentItem()->mapFromScene(QPointF(p.x(), IMAGE_SIZE + SZ_BD - 1))
                                                                     : QPointF(p.x(), IMAGE_SIZE + SZ_BD - 1);
        setPos(point);
    }
    else if (p.y() > IMAGE_SIZE + SZ_BD)
    {
        auto point = parentItem() != 0
                                         ? parentItem()->mapFromScene(QPointF(p.x(), -SZ_BD + 1))
                                         : QPointF(p.x(), -SZ_BD + 1);
        setPos(point);
    }

    // (II) regarde les intersections avec les autres objets.
    if (logical_scene->intersect(this))
        _state = Collision;
    else
        _state = Ok;
}

QPointF
MasterShape::randomPoint() const
{
    assert(_f != 0);
    return mapToParent(_f->randomPoint());
}

bool MasterShape::isInside(const QPointF &p) const
{
    assert(_f != 0);
    return _f->isInside(mapFromParent(p));
}

QRectF
MasterShape::boundingRect() const
{
    assert(_f != 0);
    return mapRectToParent(_f->boundingRect());
}

///////////////////////////////////////////////////////////////////////////////
// class MasterShape
///////////////////////////////////////////////////////////////////////////////

Asteroid::Asteroid(QColor cok, QColor cko, double speed, double r)
        : MasterShape(cok, cko), _speed(speed)
{
    // This shape is very simple : just a disk.
    Disk *d = new Disk(r, this);
    // Tells the asteroid that it is composed of just a disk.
    this->setGraphicalShape(d);
}

void Asteroid::advance(int step)
{
    if (!step)
        return;
    setPos(mapToParent(_speed, 0.0));
    MasterShape::advance(step);
}

///////////////////////////////////////////////////////////////////////////////
// class SpaceTruck
///////////////////////////////////////////////////////////////////////////////

SpaceTruck::SpaceTruck(QColor cok, QColor cko, double speed)
        : MasterShape(cok, cko), _speed(speed)
{
    Rectangle *d1 = new Rectangle(QPointF(-80, -10), QPointF(0, 10), this);
    Rectangle *d2 = new Rectangle(QPointF(10, -10), QPointF(30, 10), this);
    Rectangle *d3 = new Rectangle(QPointF(0, -3), QPointF(10, 3), this);
    Union *u23 = new Union(d2, d3);
    Union *u = new Union(d1, u23);
    // Tells the space truck that it is composed of the union of the previous shapes.
    this->setGraphicalShape(u);
}
void SpaceTruck::advance(int step)
{
    if (!step)
        return;
    setPos(mapToParent(_speed, 0.0));
    // setrotation with slow speed
    setRotation(rotation() + RG.generateDouble() * 2.0);
    MasterShape::advance(step);
}

///////////////////////////////////////////////////////////////////////////////
// class Enterprise
///////////////////////////////////////////////////////////////////////////////

Enterprise::Enterprise(QColor cok, QColor cko, double speed)
        : MasterShape(cok, cko), _speed(speed)
{
    Rectangle *r1 = new Rectangle(QPointF(-100, -8), QPointF(0, 8), this);
    Rectangle *r2 = new Rectangle(QPointF(-100, -8), QPointF(0, 8), this);
    Rectangle *rb = new Rectangle(QPointF(-40, -9), QPointF(40, 9), this);
    Rectangle *s1 = new Rectangle(QPointF(-25, -5), QPointF(25, 5), this);
    Rectangle *s2 = new Rectangle(QPointF(-25, -5), QPointF(25, 5), this);
    Disk *d = new Disk(40.0, this);
    Transformation *t1 = new Transformation(r1, QPointF(0., 40.0));
    Transformation *t2 = new Transformation(r2, QPointF(0., -40.0));
    Transformation *td = new Transformation(d, QPointF(70., 0.0));
    Transformation *ts1 = new Transformation(s1, QPointF(-30.0, 0.0), 0.0);
    Transformation *us1 = new Transformation(ts1, QPointF(0.0, 0.0), 45.0);
    Transformation *ts2 = new Transformation(s2, QPointF(-30.0, 0.0), 0.0);
    Transformation *us2 = new Transformation(ts2, QPointF(0.0, 0.0), -45.0);
    Union *back = new Union(t1, t2);
    Union *head = new Union(rb, td);
    Union *legs = new Union(us1, us2);
    Union *body = new Union(legs, back);
    Union *all = new Union(head, body);
    this->setGraphicalShape(all);
}
void Enterprise::advance(int step)
{
    if (!step)
        return;
    setRotation(rotation() + RG.generateDouble() * _speed / 10.);
    setPos(mapToParent(_speed, 0.0));
    MasterShape::advance(step);
}

///////////////////////////////////////////////////////////////////////////////
// class LogicalScene
///////////////////////////////////////////////////////////////////////////////

LogicalScene::LogicalScene(int n)
        : nb_tested(n) {}

bool LogicalScene::intersect(MasterShape *f1, MasterShape *f2)
{
    for (int i = 0; i < nb_tested; ++i)
    {
        if (f2->isInside(f1->randomPoint()) || f1->isInside(f2->randomPoint()))
            return true;
    }
    return false;
}

bool LogicalScene::intersect(MasterShape *f1)
{
    for (auto f : formes)
        if ((f != f1) && intersect(f, f1))
            return true;
    return false;
}
