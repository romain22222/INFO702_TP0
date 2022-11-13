/****************************************************************************
** Author: J.-O. Lachaud, University Savoie Mont Blanc
** (vaguely adapted from Qt colliding mices example)
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
****************************************************************************/

#include <cmath>
#include <QtWidgets>
#include "objects.hpp"

static const int AsteroidCount = 3;
static const int RectangleCount = 2;
static const int EnterpriseCount = 1;
static const int NiceCount = 1;

void testLogicalView(MasterShape* shape, QGraphicsScene& view) {
    QColor cko( 255, 240, 0 );
    for (int i = 0; i < 10000; ++i) {
        MasterShape* asteroid = new Asteroid( cko, cko, 0, 1);
        asteroid->setPos(shape->randomPoint());
        view.addItem( asteroid );
    }
}

void testIsInside(MasterShape* shape, QGraphicsScene& view) {
    QColor cko( 255, 240, 0 );
    MasterShape* a = new Asteroid( cko, cko, 0, 50);
    a->setPos(shape->boundingRect().bottomLeft());
    view.addItem( a );
    MasterShape* a2 = new Asteroid( cko, cko, 0, 50);
    a2->setPos(shape->boundingRect().topRight());
    view.addItem( a2 );
    for (int i = 0; i < IMAGE_SIZE; ++i) {
        for (int j = 0; j < IMAGE_SIZE; ++j) {
            if (shape->isInside(QPointF(i,j))) {
                MasterShape* asteroid = new Asteroid( cko, cko, 0, 10);
                asteroid->setPos(QPointF(i,j));
                view.addItem( asteroid );
            }
        }
    }
}


int main(int argc, char **argv)
{
  // Initializes Qt.
  QApplication app(argc, argv);
  // Initializes the random generator.
  srand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

  // Creates a graphics scene where we will put graphical objects.
  QGraphicsScene graphical_scene;
  graphical_scene.setSceneRect(0, 0, IMAGE_SIZE, IMAGE_SIZE);
  graphical_scene.setItemIndexMethod(QGraphicsScene::NoIndex);

  // We choose to check intersection with 100 random points.
  logical_scene = new LogicalScene( 100 );

  QPixmap* asteroid_pixmap = new QPixmap(":/images/asteroid.gif");

  for (int i = 0; i < AsteroidCount; ++i) {
    QColor cok( 150, 130, 110 );
    QColor cko( 255, 240, 0 );

    // A master shape gathers all the elements of the shape.
    MasterShape* asteroid = new Asteroid( cok, cko,
                                          0,//( rand() % 20 + 20 ) / 10.0 /* speed */,
                                          (double) (10 + rand() % 40) /* radius */ );
    // Set direction and position
    asteroid->setRotation(rand() % 360);
    asteroid->setPos( IMAGE_SIZE/2 + ::sin((i * 6.28) / AsteroidCount) * 200,
                      IMAGE_SIZE/2 + ::cos((i * 6.28) / AsteroidCount) * 200 );
    // Add it to the graphical scene
    graphical_scene.addItem( asteroid );
    // and to the logical scene
    logical_scene->formes.push_back( asteroid );
  }

  for (int i = 0; i < RectangleCount; ++i) {
    QColor cok( 150, 130, 110 );
    QColor cko( 255, 240, 0 );

    // A master shape gathers all the elements of the shape.
    MasterShape* spaceTruck = new SpaceTruck( cok, cko,
                                            0);//( rand() % 20 + 20 ) / 10.0 /* speed */);
    // Set direction and position
    spaceTruck->setRotation(rand() % 360);
    spaceTruck->setPos( IMAGE_SIZE/2 + ::sin((i * 6.28) / RectangleCount) * 200,
                      IMAGE_SIZE/2 + ::cos((i * 6.28) / RectangleCount) * 200 );
    // Add it to the graphical scene
    graphical_scene.addItem( spaceTruck );
    // and to the logical scene
    logical_scene->formes.push_back( spaceTruck );
  }

  for (int i = 0; i < EnterpriseCount; ++i) {
    QColor cok( 150, 130, 110 );
    QColor cko( 255, 240, 0 );

    // A master shape gathers all the elements of the shape.
    MasterShape* enterprise = new Enterprise( cok, cko,
                                            0);//( rand() % 20 + 10.0 ) / 10.0 /* speed */);
    enterprise->setPos( IMAGE_SIZE/2.0, IMAGE_SIZE/2.0);
    // Add it to the graphical scene
    graphical_scene.addItem( enterprise );
    //testLogicalView(enterprise, graphical_scene);
    // and to the logical scene
    logical_scene->formes.push_back( enterprise );
  }
  
  for (int i = 0; i < NiceCount; ++i) {
      QColor cok( 150, 130, 110 );
      QColor cko( 255, 240, 0 );

      // A master shape gathers all the elements of the shape.
      MasterShape* nice_asteroid = new NiceAsteroid( cok, cko,
                                              ( rand() % 20 + 10.0 ) / 10.0 /* speed */,
                                              *asteroid_pixmap);
//       nice_asteroid->setPos( IMAGE_SIZE/2 + ::sin((i * 6.28) / NiceCount) * 200,
//                           IMAGE_SIZE/2 + ::cos((i * 6.28) / NiceCount) * 200 );
      // Add it to the graphical scene
      graphical_scene.addItem( nice_asteroid );
      //testLogicalView(nice_asteroid, graphical_scene);
      //testIsInside(nice_asteroid, graphical_scene);
      // and to the logical scene
      logical_scene->formes.push_back( nice_asteroid );
    }


  QGraphicsView view(&graphical_scene);
  view.setRenderHint(QPainter::Antialiasing);
  view.setBackgroundBrush(QPixmap(":/images/stars.jpg"));
  view.setCacheMode(QGraphicsView::CacheBackground);
  view.setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
  view.setDragMode(QGraphicsView::NoDrag); // QGraphicsView::ScrollHandDrag
  view.setWindowTitle(QT_TRANSLATE_NOOP(QGraphicsView, "Space - the final frontier"));
  view.setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
  view.setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
  view.resize( IMAGE_SIZE, IMAGE_SIZE );
  view.show();

  // Creates a timer that will call `advance()` method regularly.
  QTimer timer;
  QObject::connect(&timer, SIGNAL(timeout()), &graphical_scene, SLOT(advance()));
  timer.start( 30 ); // every 30ms
  
  return app.exec();
}


