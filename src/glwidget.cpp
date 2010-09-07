/***************************************************************************
 *   Copyright (C) 2010 by Heiko Koehn ( KoehnHeiko@googlemail.com )       *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QtGui>
#include <QtOpenGL>

#include <math.h>
#include <QDebug>
#include "glwidget.h"
#include "Host.h"
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif


bool ortho;
bool selectionMode = false;
float xpos = 0, ypos = 0, zpos = 0, xrot = 0, yrot = 0, angle=0.0;
float zmax, zmin;

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    mpMap = 0;
    xDist=0.0;
    yDist=0.0;
    zDist=0.0;
    xRot=1.0;
    yRot=5.0;
    zRot=10.0;
    ortho=false;//true;
    xDist = 0;
    yDist = 0;
    zDist = -1 ;
    mScale = 1.0;
    zmax = 9999999.0;
    zmin = 9999999.0;
    mShowTopLevels = 9999999;
    mShowBottomLevels = 999999;
    setAttribute( Qt::WA_OpaquePaintEvent );
}


GLWidget::GLWidget(TMap * pM, QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    mpHost = 0;
    mpMap = pM;
}


GLWidget::~GLWidget()
{
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}

static void qNormalizeAngle(int &angle)
{
    angle /= 10;
}

void GLWidget::fullView()
{
    mShowTopLevels = 9999999;
    mShowBottomLevels = 999999;
    updateGL();
}


void GLWidget::singleView()
{
    mShowTopLevels = 0;
    mShowBottomLevels = 0;
    updateGL();
}

void GLWidget::increaseTop()
{
    mShowTopLevels += 1;
    updateGL();
}

void GLWidget::reduceTop()
{
    if( mShowTopLevels <= 0 ) mShowTopLevels = abs(zmax);
    if( abs(mShowTopLevels) > abs(zmax) )
        mShowTopLevels = abs(zmax);
    mShowTopLevels--;
    updateGL();
}

void GLWidget::increaseBottom()
{
    mShowBottomLevels++;
    updateGL();
}

void GLWidget::reduceBottom()
{
    if( mShowBottomLevels <= 0 ) mShowBottomLevels = abs(zmin);
    if( abs(mShowBottomLevels) > abs(zmin) )
        mShowBottomLevels = abs(zmin);
    mShowBottomLevels--;
    updateGL();
}

void GLWidget::defaultView()
{
    xRot=1.0;
    yRot=5.0;
    zRot=10.0;
    mScale = 1.0;
    updateGL();
}

void GLWidget::sideView()
{
    xRot=7.0;
    yRot=-10.0;
    zRot=0.0;
    mScale = 1.0;
    updateGL();
}

void GLWidget::topView()
{
    xRot=0.0;
    yRot=0.0;
    zRot=15.0;
    mScale = 1.0;
    updateGL();
}


void GLWidget::goRoom( const QString & s )
{

}

void GLWidget::setScale(int angle)
{
    mScale = 150/((float)angle+300);
    resizeGL(width(),height());
    updateGL();
    return;
}

void GLWidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    xRot=angle;
    updateGL();
    return;
}

void GLWidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    yRot=angle;
    updateGL();
    return;
}

void GLWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    zRot=angle;
    updateGL();
    return;
}

void GLWidget::setXDist(int angle)
{
    xDist=angle;
    updateGL();
    return;
}

void GLWidget::setYDist(int angle)
{
    yDist=angle;
    updateGL();
    return;
}

void GLWidget::setZDist(int angle)
{
    zDist=angle;
    updateGL();
    return;
}


void GLWidget::initializeGL()
{
    qglClearColor( Qt::black );
    xRot = 1;
    yRot = 5;
    zRot = 10;
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glShadeModel( GL_SMOOTH );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearDepth(1.0);
}

void GLWidget::paintGL()
{
    QTime __t; __t.start();
    float px,py,pz;
    if( ! mpMap->rooms.contains( mpMap->mRoomId ) )
    {
        qDebug()<<"ERROR: roomID not in rooms map";
        return;
    }
    px = static_cast<float>(mpMap->rooms[mpMap->mRoomId]->x);
    py = static_cast<float>(mpMap->rooms[mpMap->mRoomId]->y);
    pz = static_cast<float>(mpMap->rooms[mpMap->mRoomId]->z);
    TArea * pArea = mpMap->areas[mpMap->rooms[mpMap->mRoomId]->area];
    if( ! pArea ) return;
    zmax = static_cast<float>(pArea->max_z);
    zmin = static_cast<float>(pArea->min_z);
    float zEbene;
    glEnable(GL_CULL_FACE);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);

    glClearColor (0.0,0.0,0.0,1.0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    GLfloat diffuseLight[] = {0.07, 0.07, 0.07, 1.0};
    GLfloat diffuseLight2[] = {0.5, 0.5, 0.5, 1.0};
    GLfloat ambientLight[] = {0.3, 0.3, 0.3, 1.0};
    GLfloat ambientLight2[] = {0.01, 0.01, 0.01, 1.0};
    if( !mpMap->rooms.contains(mpMap->mRoomId) ) return;
    int env = mpMap->rooms[mpMap->mRoomId]->environment;

    GLfloat specularLight[] = {.01, .01, .01, 1.};
    GLfloat light0Pos[] = {5000.0, 4000.0, 1000.0, 0};
    GLfloat light1Pos[] = {5000.0, 1000.0, 1000.0, 0};
    glLightfv( GL_LIGHT0, GL_AMBIENT, ambientLight );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseLight );
    glLightfv (GL_LIGHT0, GL_POSITION, light0Pos );
    glLightfv( GL_LIGHT1, GL_AMBIENT, ambientLight2 );
    glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuseLight2 );
    glLightfv (GL_LIGHT1, GL_POSITION, light1Pos );
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    glLightfv (GL_LIGHT0, GL_POSITION, light0Pos );
    glLightfv (GL_LIGHT1, GL_POSITION, light1Pos );
    glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);//GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();

    dehnung = 4.0;

    if( ! mpMap )
    {
        return;
    }
    glDisable( GL_FOG );
    glEnable(GL_BLEND);
    glEnable( GL_LIGHTING );
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    if( zRot <= 0 )
        zEbene = zmax;
    else
        zEbene = zmin;

    glEnable (GL_LINE_SMOOTH);
    glEnable (GL_LINE_STIPPLE);
    glLineWidth(1.0);
    int quads=0;
    int verts=0;
    float ebenenColor2[][4] = { 0.9, 0.5, 0.0, 1.,
                               165.0/255.0, 102.0/255.0, 167.0/255.0, 1.,
                               170.0/255.0, 10.0/255.0, 127.0/255.0, 1.,
                               203.0/255.0, 135.0/255.0, 101.0/255.0, 1.,
                               154.0/255.0, 154.0/255.0, 115.0/255.0, 1.,
                               107.0/255.0, 154.0/255.0, 100.0/255.0, 1.,
                               154.0/255.0, 184.0/255.0, 111.0/255.0, 1.,
                               67.0/255.0, 154.0/255.0, 148.0/255.0, 1.,
                               154.0/255.0, 118.0/255.0, 151.0/255.0, 1.,
                               208.0/255.0, 213.0/255.0, 164.0/255.0, 1.,
                               213.0/255.0, 169.0/255.0, 158.0/255.0, 1.,
                               139.0/255.0, 209.0/255.0, 0, 1.,
                               163.0/255.0, 209.0/255.0, 202.0/255.0, 1.,
                               158.0/255.0, 156.0/255.0, 209.0/255.0, 1.,
                               209.0/255.0, 144.0/255.0, 162.0/255.0, 1.,
                               209.0/255.0, 183.0/255.0, 78.0/255.0, 1.,
                               111.0/255.0, 209.0/255.0, 88.0/255.0, 1.,
                               95.0/255.0, 120.0/255.0, 209.0/255.0, 1.,
                               31.0/255.0, 209.0/255.0, 126.0/255.0, 1.,
                               1, 170.0/255.0, 1, 1.,
                               158.0/255.0, 105.0/255.0, 158.0/255.0, 1.,
                               68.0/255.0, 189.0/255.0, 189.0/255.0, 1.,
                               0.1, 0.69, 0.49, 1.,
                               0.0, 0.15, 1.0, 1.,
                               0.12, 0.02, 0.20, 1.,
                               0.0, 0.3, 0.1, 1. };
    float ebenenColor[][4] = { 0.5, 0.6, 0.5, 0.2,
                                0.233, 0.498, 0.113, 0.2,
                                0.666, 0.333, 0.498, 0.2,
                                0.5, 0.333, 0.666, 0.2,
                                0.69, 0.458, 0.0, 0.2,
                                0.333, 0.0, 0.49, 0.2,
                                133.0/255.0, 65.0/255.0, 98.0/255.0, 0.2,
                                0.3, 0.3, 0.0, 0.2,
                                0.6, 0.2, 0.6, 0.2,
                                0.6, 0.6, 0.2, 0.2,
                                0.4, 0.1, 0.4, 0.2,
                                0.4, 0.4, 0.1, 0.2,
                                0.3, 0.1, 0.3, 0.2,
                                0.3, 0.3, 0.1, 0.2,
                                0.2, 0.1, 0.2, 0.2,
                                0.2, 0.2, 0.1, 0.2,
                                0.24, 0.1, 0.5, 0.2,
                                0.1, 0.1, 0.0, 0.2,
                                0.54, 0.6, 0.2, 0.2,
                                0.2, 0.2, 0.5, 0.2,
                                0.6, 0.6, 0.2, 0.2,
                                0.6, 0.4, 0.6, 0.2,
                                0.4, 0.4, 0.1, 0.2,
                                0.4, 0.2, 0.4, 0.2,
                                0.2, 0.2, 0.0, 0.2,
                                0.2, 0.1, 0.3, 0.2 };

    //FIXME:
    int Dbg_cnt = 0;

    while( true )
    {
        if( zRot <= 0 )
        {
            if( zEbene < zmin )
            {
                break;
            }
        }
        else
        {
            if( zEbene > zmax )
            {
                break;
            }
        }
        for( int i=0; i<pArea->rooms.size(); i++ )
        {
            float rx = static_cast<float>(mpMap->rooms[pArea->rooms[i]]->x);
            float ry = static_cast<float>(mpMap->rooms[pArea->rooms[i]]->y);
            float rz = static_cast<float>(mpMap->rooms[pArea->rooms[i]]->z);
            if( rz != zEbene ) continue;
            if( rz > pz )
                if( abs(rz-pz) > mShowTopLevels ) continue;
            if( rz < pz )
                if( abs(rz-pz) > mShowBottomLevels ) continue;
            QList<int> exitList;
            exitList.push_back( mpMap->rooms[pArea->rooms[i]]->north );
            exitList.push_back( mpMap->rooms[pArea->rooms[i]]->northwest );
            exitList.push_back( mpMap->rooms[pArea->rooms[i]]->east );
            exitList.push_back( mpMap->rooms[pArea->rooms[i]]->southeast );
            exitList.push_back( mpMap->rooms[pArea->rooms[i]]->south );
            exitList.push_back( mpMap->rooms[pArea->rooms[i]]->southwest );
            exitList.push_back( mpMap->rooms[pArea->rooms[i]]->west );
            exitList.push_back( mpMap->rooms[pArea->rooms[i]]->northwest );
            exitList.push_back( mpMap->rooms[pArea->rooms[i]]->up );
            exitList.push_back( mpMap->rooms[pArea->rooms[i]]->down );
            int e = mpMap->rooms[pArea->rooms[i]]->z;
            int ef;
            ef = abs(e%26);
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ebenenColor[ef]);
            glMateriali(GL_FRONT, GL_SHININESS, 36);
            glDisable(GL_DEPTH_TEST);
            if( rz <= pz )
            {
                if( ( rz == pz ) && ( rx == px ) && ( ry == py ) )
                {
                    glDisable(GL_BLEND);
                    glEnable( GL_LIGHTING );
                    float mc3[] = { 1.0f, 0.0f, 0.0f, 1.0f };
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mc3);
                    glMateriali(GL_FRONT, GL_SHININESS, 96);
                    glColor4f(1.0, 0.0, 0.0, 1.0);
                }
                else
                {
                    glDisable(GL_BLEND);
                    glEnable( GL_LIGHTING );
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ebenenColor[ef]);
                    glMateriali(GL_FRONT, GL_SHININESS, 36);
                    glColor4f(ebenenColor[ef][0],
                              ebenenColor[ef][1],
                              ebenenColor[ef][2],
                              ebenenColor[ef][3]);
                }
                for( int k=0; k<exitList.size(); k++ )
                {
                    bool areaExit = false;
                    if( exitList[k] == -1 ) continue;
                    if( ! mpMap->rooms.contains( exitList[k] ) )
                    {
                        continue;
                    }
                    if( mpMap->rooms[exitList[k]]->area != mpMap->rooms[mpMap->mRoomId]->area )
                    {
                        areaExit = true;
                    }
                    float ex = static_cast<float>(mpMap->rooms[exitList[k]]->x);
                    float ey = static_cast<float>(mpMap->rooms[exitList[k]]->y);
                    float ez = static_cast<float>(mpMap->rooms[exitList[k]]->z);
                    QVector3D p1( ex, ey, ez );
                    QVector3D p2( rx, ry, rz );
                    glLoadIdentity();
                    gluLookAt(px*0.1+xRot, py*0.1+yRot, pz*0.1+zRot, px*0.1, py*0.1, pz*0.1,0.0,1.0,0.0);
                    glScalef( 0.1, 0.1, 0.1);
                    if( areaExit )
                        glLineWidth(1);//1/mScale+2);
                    else
                        glLineWidth(1);//1/mScale);
                    if( exitList[k] == mpMap->mRoomId || ( ( rz == pz ) && ( rx == px ) && ( ry == py ) ) )
                    {
                        glDisable(GL_BLEND);
                        glEnable( GL_LIGHTING );
                        float mc3[] = { 1.0f, 0.0f, 0.0f, 1.0f };
                        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mc3);
                        glMateriali(GL_FRONT, GL_SHININESS, 96);
                        glColor4f(1.0, 0.0, 0.0, 1.0);
                    }
                    else
                    {
                        glDisable(GL_BLEND);
                        glEnable( GL_LIGHTING );
                        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ebenenColor[ef]);
                        glMateriali(GL_FRONT, GL_SHININESS, 36);
                        glColor4f(ebenenColor[ef][0],
                                  ebenenColor[ef][1],
                                  ebenenColor[ef][2],
                                  ebenenColor[ef][3]);
                    }
                    glBegin(GL_LINES);
                    if( ! areaExit )
                    {
                        glVertex3f( p1.x(), p1.y(), p1.z() );
                    }
                    else
                    {
                        if( mpMap->rooms[pArea->rooms[i]]->north == exitList[k] )
                            glVertex3f( p2.x(), p2.y()+1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->south == exitList[k] )
                            glVertex3f( p2.x(), p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->west == exitList[k] )
                            glVertex3f( p2.x()-1, p2.y(), p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->east == exitList[k] )
                            glVertex3f( p2.x()+1, p2.y(), p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->southwest == exitList[k] )
                            glVertex3f( p2.x()-1, p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->southeast == exitList[k] )
                            glVertex3f( p2.x()+1, p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->northeast == exitList[k] )
                            glVertex3f( p2.x()+1, p2.y()+1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->northwest == exitList[k] )
                            glVertex3f( p2.x()-1, p2.y()+1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->up == exitList[k] )
                            glVertex3f( p2.x(), p2.y(), p2.z()+1 );
                        else if( mpMap->rooms[pArea->rooms[i]]->down == exitList[k] )
                            glVertex3f( p2.x(), p2.y(), p2.z()-1 );
                    }
                    glVertex3f( p2.x(), p2.y(), p2.z() );
                    glEnd();
                    verts++;
                    if( areaExit )
                    {
                        glDisable(GL_BLEND);
                        glEnable( GL_LIGHTING );
                        glDisable(GL_LIGHT1);
                        float mc4[] = { 85.0/255.0, 170.0/255.0, 0.0/255.0, 1.0 };
                        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mc4);
                        glMateriali(GL_FRONT, GL_SHININESS, 56);
                        glColor4f(85.0/255.0, 170.0/255.0, 0.0/255.0, 1.0);
                        glLoadIdentity();
                        gluLookAt(px*0.1+xRot, py*0.1+yRot, pz*0.1+zRot, px*0.1, py*0.1, pz*0.1,0.0,1.0,0.0);
                        glScalef( 0.1, 0.1, 0.1);
                        if( mpMap->rooms[pArea->rooms[i]]->north == exitList[k] )
                            glTranslatef( p2.x(), p2.y()+1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->south == exitList[k] )
                            glTranslatef( p2.x(), p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->west == exitList[k] )
                            glTranslatef( p2.x()-1, p2.y(), p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->east == exitList[k] )
                            glTranslatef( p2.x()+1, p2.y(), p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->southwest == exitList[k] )
                            glTranslatef( p2.x()-1, p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->southeast == exitList[k] )
                            glTranslatef( p2.x()+1, p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->northeast == exitList[k] )
                            glTranslatef( p2.x()+1, p2.y()+1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->northwest == exitList[k] )
                            glTranslatef( p2.x()-1, p2.y()+1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->up == exitList[k] )
                            glTranslatef( p2.x(), p2.y(), p2.z()+1 );
                        else if( mpMap->rooms[pArea->rooms[i]]->down == exitList[k] )
                            glTranslatef( p2.x(), p2.y(), p2.z()-1 );


                        glLoadName( exitList[k] );
                        quads++;
                        glBegin( GL_QUADS );
                        glNormal3f(0.57735, -0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                        glNormal3f(0.57735, 0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

                        glNormal3f(-0.57735, 0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, 0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                        glNormal3f(0.57735, 0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, 0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                        glNormal3f(-0.57735, 0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

                        glNormal3f(0.57735, 0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, 0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glEnd();
                        //drauf
                        float mc3[] = { 0.2, 0.2, 0.6, 0.2 };
                        int env = mpMap->rooms[exitList[k]]->environment;
                        if( mpMap->envColors.contains(env) )
                            env = mpMap->envColors[env];
                        else
                        {
                            if( ! mpMap->customEnvColors.contains(env))
                            {
                                env = 1;
                            }
                        }
                        switch( env )
                        {
                        case 1:
                            glColor4b(128,50,50,2);
                            mc3[0]=128.0/255.0; mc3[1]=0.0/255.0; mc3[2]=0.0/255.0; mc3[3]=0.2;
                            break;

                        case 2:
                            glColor4b(128,128,50, 2);
                            mc3[0]=0.0/255.0; mc3[1]=128.0/255.0; mc3[2]=0.0/255.0; mc3[3]=0.2;
                            break;
                        case 3:
                            glColor4b(50,128,50,2);
                            mc3[0]=128.0/255.0; mc3[1]=128.0/255.0; mc3[2]=0.0/255.0; mc3[3]=0.2;
                            break;

                        case 4:
                            glColor4b(50,50,128,2);
                            mc3[0]=0.0/255.0; mc3[1]=0.0/255.0; mc3[2]=128.0/255.0; mc3[3]=0.2;
                            break;

                        case 5:
                            glColor4b(128,50,128,2);
                            mc3[0]=128.0/255.0; mc3[1]=128.0/255.0; mc3[2]=0.0/255.0; mc3[3]=0.2;
                            break;
                        case 6:
                            glColor4b(50,128,128,2);
                            mc3[0]=0.0/255.0; mc3[1]=128.0/255.0; mc3[2]=128.0/255.0; mc3[3]=0.2;
                            break;
                        case 7:
                            glColor4b(52,38,78,2);
                            mc3[0]=128.0/255.0; mc3[1]=128.0/255.0; mc3[2]=128.0/255.0; mc3[3]=0.2;
                            break;
                        case 8:
                            glColor4b(65, 55, 35, 2);
                            mc3[0]=55.0/255.0; mc3[1]=55.0/255.0; mc3[2]=55.0/255.0; mc3[3]=0.2;
                            break;

                        case 9:
                            glColor4b(175,50,50,2);
                            mc3[0]=255.0/255.0; mc3[1]=50.0/255.0; mc3[2]=50.0/255.0; mc3[3]=0.2;
                            break;

                        case 10:
                            glColor4b(255,255,50,2);
                            mc3[0]=50.0/255.0; mc3[1]=255.0/255.0; mc3[2]=50.0/255.0; mc3[3]=0.2;
                            break;
                        case 11:
                            glColor4b(50,175,175,2);
                            mc3[0]=255.0/255.0; mc3[1]=255.0/255.0; mc3[2]=50.0/255.0; mc3[3]=0.2;
                            break;

                        case 12:
                            glColor4b(175,175,50,2);
                            mc3[0]=50.0/255.0; mc3[1]=50.0/255.0; mc3[2]=255.0/255.0; mc3[3]=0.2;
                            break;

                        case 13:
                            glColor4b(175,50,175,2);
                            mc3[0]=255.0/255.0; mc3[1]=50.0/255.0; mc3[2]=255.0/255.0; mc3[3]=0.2;
                            break;
                        case 14:
                            glColor4b(50,175,50,2);
                            mc3[0]=50.0/255.0; mc3[1]=255.0/255.0; mc3[2]=255.0/255.0; mc3[3]=0.2;
                            break;
                        case 15:
                            glColor4b(50,50,175,2);
                            mc3[0]=255.0/255.0; mc3[1]=255.0/255.0; mc3[2]=255.0/255.0; mc3[3]=0.2;
                            break;
                        default: //user defined room color
                            int __r, __g, __b;
                            if( ! mpMap->customEnvColors.contains(env) ) break;
                            QColor &_c = mpMap->customEnvColors[env];
                            glColor4b(_c.red(),_c.green(),_c.blue(),25);
                            mc3[0]=_c.redF();
                            mc3[1]=_c.greenF();
                            mc3[2]=_c.blueF();
                            mc3[3]=0.2;
                        }
                        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mc3);
                        glMateriali(GL_FRONT, GL_SHININESS, 36);
                        glDisable(GL_DEPTH_TEST);
                        glLoadIdentity();
                        gluLookAt(px*0.1+xRot, py*0.1+yRot, pz*0.1+zRot, px*0.1, py*0.1, pz*0.1,0.0,1.0,0.0);
                        glScalef( 0.05, 0.05, 0.020);
                        if( mpMap->rooms[pArea->rooms[i]]->north == exitList[k] )
                            glTranslatef( 2*p2.x(), 2*(p2.y()+1), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->south == exitList[k] )
                            glTranslatef( 2*p2.x(), 2*(p2.y()-1), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->west == exitList[k] )
                            glTranslatef( 2*(p2.x()-1), 2*p2.y(), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->east == exitList[k] )
                            glTranslatef( 2*(p2.x()+1), 2*p2.y(), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->southwest == exitList[k] )
                            glTranslatef( 2*(p2.x()-1), 2*(p2.y()-1), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->southeast == exitList[k] )
                            glTranslatef( 2*(p2.x()+1), 2*(p2.y()-1), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->northeast == exitList[k] )
                            glTranslatef( 2*(p2.x()+1), 2*(p2.y()+1), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->northwest == exitList[k] )
                            glTranslatef( 2*(p2.x()-1), 2*(p2.y()+1), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->up == exitList[k] )
                            glTranslatef( 2*p2.x(), 2*p2.y(), 5.0*(p2.z()+1+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->down == exitList[k] )
                            glTranslatef( 2*p2.x(), 2*p2.y(), 5.0*(p2.z()-1+0.25) );

                        glBegin( GL_QUADS );
                        glNormal3f(0.57735, -0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                        glNormal3f(0.57735, 0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

                        glNormal3f(-0.57735, 0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, 0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                        glNormal3f(0.57735, 0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, 0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                        glNormal3f(-0.57735, 0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

                        glNormal3f(0.57735, 0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, 0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glEnd();
                    }

                }
            }
            else
            {


                for( int k=0; k<exitList.size(); k++ )
                {
                    bool areaExit = false;
                    if( exitList[k] == -1 ) continue;
                    if( ! mpMap->rooms.contains( exitList[k] ) )
                    {
                        continue;
                    }
                    if( mpMap->rooms[exitList[k]]->area != mpMap->rooms[mpMap->mRoomId]->area )
                    {
                        areaExit = true;
                    }
                    float ex = static_cast<float>(mpMap->rooms[exitList[k]]->x);
                    float ey = static_cast<float>(mpMap->rooms[exitList[k]]->y);
                    float ez = static_cast<float>(mpMap->rooms[exitList[k]]->z);
                    QVector3D p1( ex, ey, ez );
                    QVector3D p2( rx, ry, rz );
                    glLoadIdentity();
                    gluLookAt(px*0.1+xRot, py*0.1+yRot, pz*0.1+zRot, px*0.1, py*0.1, pz*0.1,0.0,1.0,0.0);
                    glScalef( 0.1, 0.1, 0.1);
                    if( areaExit )
                        glLineWidth(1);//1/mScale+2);
                    else
                        glLineWidth(1);//1/mScale);
                    if( exitList[k] == mpMap->mRoomId || ( ( rz == pz ) && ( rx == px ) && ( ry == py ) ) )
                    {
                        glDisable(GL_BLEND);
                        glEnable( GL_LIGHTING );
                        float mc3[] = { 1.0f, 0.0f, 0.0f, 1.0f };
                        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mc3);
                        glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 96);
                        glColor4f(1.0, 0.0, 0.0, 1.0);
                    }
                    else
                    {
                        glEnable(GL_BLEND);
                        glEnable( GL_LIGHTING );
                        glEnable(GL_LIGHT1);
                        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ebenenColor2[ef]);
                        glMateriali(GL_FRONT, GL_SHININESS, 56);//gut:36
                        glColor4f(ebenenColor2[ef][0],
                                  ebenenColor2[ef][1],
                                  ebenenColor2[ef][2],
                                  ebenenColor2[ef][3]);
                    }
                    glBegin(GL_LINES);
                    if( ! areaExit )
                    {
                        glVertex3f( p1.x(), p1.y(), p1.z() );
                    }
                    else
                    {
                        if( mpMap->rooms[pArea->rooms[i]]->north == exitList[k] )
                            glVertex3f( p2.x(), p2.y()+1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->south == exitList[k] )
                            glVertex3f( p2.x(), p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->west == exitList[k] )
                            glVertex3f( p2.x()-1, p2.y(), p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->east == exitList[k] )
                            glVertex3f( p2.x()+1, p2.y(), p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->southwest == exitList[k] )
                            glVertex3f( p2.x()-1, p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->southeast == exitList[k] )
                            glVertex3f( p2.x()+1, p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->northeast == exitList[k] )
                            glVertex3f( p2.x()+1, p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->northwest == exitList[k] )
                            glVertex3f( p2.x()-1, p2.y()+1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->up == exitList[k] )
                            glVertex3f( p2.x(), p2.y(), p2.z()+1 );
                        else if( mpMap->rooms[pArea->rooms[i]]->down == exitList[k] )
                            glVertex3f( p2.x(), p2.y(), p2.z()-1 );
                    }
                    glVertex3f( p2.x(), p2.y(), p2.z() );
                    glEnd();
                    verts++;
                    if( areaExit )
                    {
                        glDisable(GL_BLEND);
                        glEnable( GL_LIGHTING );
                        glDisable(GL_LIGHT1);
                        float mc4[] = { 85.0/255.0, 170.0/255.0, 0.0/255.0, 1.0 };
                        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mc4);
                        glMateriali(GL_FRONT, GL_SHININESS, 56);
                        glColor4f(85.0/255.0, 170.0/255.0, 0.0/255.0, 1.0);
                        glLoadIdentity();
                        gluLookAt(px*0.1+xRot, py*0.1+yRot, pz*0.1+zRot, px*0.1, py*0.1, pz*0.1,0.0,1.0,0.0);
                        glScalef( 0.1, 0.1, 0.1);
                        if( mpMap->rooms[pArea->rooms[i]]->north == exitList[k] )
                            glTranslatef( p2.x(), p2.y()+1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->south == exitList[k] )
                            glTranslatef( p2.x(), p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->west == exitList[k] )
                            glTranslatef( p2.x()-1, p2.y(), p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->east == exitList[k] )
                            glTranslatef( p2.x()+1, p2.y(), p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->southwest == exitList[k] )
                            glTranslatef( p2.x()-1, p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->southeast == exitList[k] )
                            glTranslatef( p2.x()+1, p2.y()-1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->northeast == exitList[k] )
                            glTranslatef( p2.x()+1, p2.y()+1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->northwest == exitList[k] )
                            glTranslatef( p2.x()-1, p2.y()+1, p2.z() );
                        else if( mpMap->rooms[pArea->rooms[i]]->up == exitList[k] )
                            glTranslatef( p2.x(), p2.y(), p2.z()+1 );
                        else if( mpMap->rooms[pArea->rooms[i]]->down == exitList[k] )
                            glTranslatef( p2.x(), p2.y(), p2.z()-1 );

                        glLoadName( exitList[k] );
                        quads++;
                        glBegin( GL_QUADS );
                        glNormal3f(0.57735, -0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                        glNormal3f(0.57735, 0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

                        glNormal3f(-0.57735, 0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, 0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                        glNormal3f(0.57735, 0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, 0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                        glNormal3f(-0.57735, 0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

                        glNormal3f(0.57735, 0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, 0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glEnd();

                        //drauf
                        float mc3[] = { 0.2, 0.2, 0.6, 0.2 };
                        int env = mpMap->rooms[exitList[k]]->environment;
                        if( mpMap->envColors.contains(env) )
                            env = mpMap->envColors[env];
                        else
                        {
                            if( ! mpMap->customEnvColors.contains(env))
                            {
                                env = 1;
                            }
                        }
                        switch( env )
                        {
                        case 1:
                            glColor4b(128,50,50,2);
                            mc3[0]=128.0/255.0; mc3[1]=0.0/255.0; mc3[2]=0.0/255.0; mc3[3]=0.2;
                            break;

                        case 2:
                            glColor4b(128,128,50, 2);
                            mc3[0]=0.0/255.0; mc3[1]=128.0/255.0; mc3[2]=0.0/255.0; mc3[3]=0.2;
                            break;
                        case 3:
                            glColor4b(50,128,50,2);
                            mc3[0]=128.0/255.0; mc3[1]=128.0/255.0; mc3[2]=0.0/255.0; mc3[3]=0.2;
                            break;

                        case 4:
                            glColor4b(50,50,128,2);
                            mc3[0]=0.0/255.0; mc3[1]=0.0/255.0; mc3[2]=128.0/255.0; mc3[3]=0.2;
                            break;

                        case 5:
                            glColor4b(128,50,128,2);
                            mc3[0]=128.0/255.0; mc3[1]=128.0/255.0; mc3[2]=0.0/255.0; mc3[3]=0.2;
                            break;
                        case 6:
                            glColor4b(50,128,128,2);
                            mc3[0]=0.0/255.0; mc3[1]=128.0/255.0; mc3[2]=128.0/255.0; mc3[3]=0.2;
                            break;
                        case 7:
                            glColor4b(52,38,78,2);
                            mc3[0]=128.0/255.0; mc3[1]=128.0/255.0; mc3[2]=128.0/255.0; mc3[3]=0.2;
                            break;
                        case 8:
                            glColor4b(65, 55, 35, 2);
                            mc3[0]=55.0/255.0; mc3[1]=55.0/255.0; mc3[2]=55.0/255.0; mc3[3]=0.2;
                            break;

                        case 9:
                            glColor4b(175,50,50,2);
                            mc3[0]=255.0/255.0; mc3[1]=50.0/255.0; mc3[2]=50.0/255.0; mc3[3]=0.2;
                            break;

                        case 10:
                            glColor4b(255,255,50,2);
                            mc3[0]=50.0/255.0; mc3[1]=255.0/255.0; mc3[2]=50.0/255.0; mc3[3]=0.2;
                            break;
                        case 11:
                            glColor4b(50,175,175,2);
                            mc3[0]=255.0/255.0; mc3[1]=255.0/255.0; mc3[2]=50.0/255.0; mc3[3]=0.2;
                            break;

                        case 12:
                            glColor4b(175,175,50,2);
                            mc3[0]=50.0/255.0; mc3[1]=50.0/255.0; mc3[2]=255.0/255.0; mc3[3]=0.2;
                            break;

                        case 13:
                            glColor4b(175,50,175,2);
                            mc3[0]=255.0/255.0; mc3[1]=50.0/255.0; mc3[2]=255.0/255.0; mc3[3]=0.2;
                            break;
                        case 14:
                            glColor4b(50,175,50,2);
                            mc3[0]=50.0/255.0; mc3[1]=255.0/255.0; mc3[2]=255.0/255.0; mc3[3]=0.2;
                            break;
                        case 15:
                            glColor4b(50,50,175,2);
                            mc3[0]=255.0/255.0; mc3[1]=255.0/255.0; mc3[2]=255.0/255.0; mc3[3]=0.2;
                            break;
                        default: //user defined room color
                            int __r, __g, __b;
                            if( ! mpMap->customEnvColors.contains(env) ) break;
                            QColor &_c = mpMap->customEnvColors[env];
                            glColor4b(_c.red(),_c.green(),_c.blue(),255);
                            mc3[0]=_c.redF();
                            mc3[1]=_c.greenF();
                            mc3[2]=_c.blueF();
                            mc3[3]=0.2;
                        }
                        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mc3);
                        glMateriali(GL_FRONT, GL_SHININESS, 36);
                        glDisable(GL_DEPTH_TEST);
                        glLoadIdentity();
                        gluLookAt(px*0.1+xRot, py*0.1+yRot, pz*0.1+zRot, px*0.1, py*0.1, pz*0.1,0.0,1.0,0.0);
                        glScalef( 0.05, 0.05, 0.020);
                        if( mpMap->rooms[pArea->rooms[i]]->north == exitList[k] )
                            glTranslatef( 2*p2.x(), 2*(p2.y()+1), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->south == exitList[k] )
                            glTranslatef( 2*p2.x(), 2*(p2.y()-1), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->west == exitList[k] )
                            glTranslatef( 2*(p2.x()-1), 2*p2.y(), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->east == exitList[k] )
                            glTranslatef( 2*(p2.x()+1), 2*p2.y(), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->southwest == exitList[k] )
                            glTranslatef( 2*(p2.x()-1), 2*(p2.y()-1), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->southeast == exitList[k] )
                            glTranslatef( 2*(p2.x()+1), 2*(p2.y()-1), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->northeast == exitList[k] )
                            glTranslatef( 2*(p2.x()+1), 2*(p2.y()+1), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->northwest == exitList[k] )
                            glTranslatef( 2*(p2.x()-1), 2*(p2.y()+1), 5.0*(p2.z()+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->up == exitList[k] )
                            glTranslatef( 2*p2.x(), 2*p2.y(), 5.0*(p2.z()+1+0.25) );
                        else if( mpMap->rooms[pArea->rooms[i]]->down == exitList[k] )
                            glTranslatef( 2*p2.x(), 2*p2.y(), 5.0*(p2.z()-1+0.25) );

                        glBegin( GL_QUADS );
                        glNormal3f(0.57735, -0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                        glNormal3f(0.57735, 0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

                        glNormal3f(-0.57735, 0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, 0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                        glNormal3f(0.57735, 0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(0.57735, 0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, -0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                        glNormal3f(-0.57735, 0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, -0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

                        glNormal3f(0.57735, 0.57735, -0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, -0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                        glNormal3f(-0.57735, 0.57735, 0.57735);
                        glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glNormal3f(0.57735, 0.57735, 0.57735);
                        glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                        glEnd();
                    }

                }
                glFlush();

            }
        }
        label1:

        if( zRot <= 0)
            zEbene -= 1.0;
        else
            zEbene += 1.0;
    }

    if( zRot <= 0 )
        zEbene = zmax;
    else
        zEbene = zmin;

    quads = 0;
    zEbene = zmin;
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_LIGHT1);

    while( true )
    {
        if( zEbene > zmax )
        {
            break;
        }
        for( int i=0; i<pArea->rooms.size(); i++ )
        {
            glDisable(GL_LIGHT1);
            float rx = static_cast<float>(mpMap->rooms[pArea->rooms[i]]->x);
            float ry = static_cast<float>(mpMap->rooms[pArea->rooms[i]]->y);
            float rz = static_cast<float>(mpMap->rooms[pArea->rooms[i]]->z);
            if( rz != zEbene ) continue;

            if( rz > pz )
                if( abs(rz-pz) > mShowTopLevels ) continue;
            if( rz < pz )
                if( abs(rz-pz) > mShowBottomLevels ) continue;

            int e = mpMap->rooms[pArea->rooms[i]]->z;
            int ef = abs(e%26);
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ebenenColor[ef]);
            glMateriali(GL_FRONT, GL_SHININESS, 56);//gut:96

            if( ( rz == pz ) && ( rx == px ) && ( ry == py ) )
            {
                glDisable(GL_BLEND);
                glEnable( GL_LIGHTING );
                glDisable(GL_LIGHT1);
                float mc3[] = { 1.0f, 0.0f, 0.0f, 1.0f };
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mc3);
                glMateriali(GL_FRONT, GL_SHININESS, 96);
                glColor4f(1.0, 0.0, 0.0, 1.0);
            }
            else if( pArea->rooms[i] == mTarget )
            {
                glDisable(GL_BLEND);
                glEnable( GL_LIGHTING );
                glDisable(GL_LIGHT1);
                float mc4[] = { 0.0, 1.0, 0.0, 1.0 };
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mc4);
                glMateriali(GL_FRONT, GL_SHININESS, 96);//36
                glColor4f(0.0, 1.0, 0.0, 1.0);
            }
            else if( rz <= pz )
            {
                glDisable(GL_BLEND);
                glEnable( GL_LIGHTING );
                glDisable(GL_LIGHT1);
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ebenenColor2[ef]);
                glMateriali(GL_FRONT, GL_SHININESS, 96);
                glColor4f(ebenenColor[ef][0],
                          ebenenColor[ef][1],
                          ebenenColor[ef][2],
                          ebenenColor[ef][3]);
            }
            else
            {
                glEnable(GL_BLEND);
                glEnable( GL_LIGHTING );
                glEnable(GL_LIGHT1);
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ebenenColor[ef]);
                glMateriali(GL_FRONT, GL_SHININESS, 96);//56);//gut:36
                glColor4f(ebenenColor2[ef][0],
                          ebenenColor2[ef][1],
                          ebenenColor2[ef][2],
                          ebenenColor2[ef][3]);

                glLoadIdentity();
                gluLookAt(px*0.1+xRot, py*0.1+yRot, pz*0.1+zRot, px*0.1, py*0.1, pz*0.1,0.0,1.0,0.0);
                glScalef( 0.1, 0.1, 0.1);
                glTranslatef( rx, ry, rz );

                glLoadName( pArea->rooms[i] );
                quads++;
                glBegin( GL_QUADS );
                glNormal3f(0.57735, -0.57735, 0.57735);
                glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                glNormal3f(-0.57735, -0.57735, 0.57735);
                glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                glNormal3f(-0.57735, -0.57735, -0.57735);
                glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                glNormal3f(0.57735, -0.57735, -0.57735);
                glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);


                glNormal3f(0.57735, 0.57735, 0.57735);
                glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                glNormal3f(-0.57735, 0.57735, 0.57735);
                glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                glNormal3f(-0.57735, -0.57735, 0.57735);
                glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                glNormal3f(0.57735, -0.57735, 0.57735);
                glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

                glNormal3f(-0.57735, 0.57735, -0.57735);
                glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                glNormal3f(0.57735, 0.57735, -0.57735);
                glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                glNormal3f(0.57735, -0.57735, -0.57735);
                glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                glNormal3f(-0.57735, -0.57735, -0.57735);
                glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                glNormal3f(0.57735, 0.57735, -0.57735);
                glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                glNormal3f(0.57735, 0.57735, 0.57735);
                glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                glNormal3f(0.57735, -0.57735, 0.57735);
                glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                glNormal3f(0.57735, -0.57735, -0.57735);
                glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                glNormal3f(-0.57735, 0.57735, 0.57735);
                glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                glNormal3f(-0.57735, 0.57735, -0.57735);
                glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                glNormal3f(-0.57735, -0.57735, -0.57735);
                glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                glNormal3f(-0.57735, -0.57735, 0.57735);
                glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

                glNormal3f(0.57735, 0.57735, -0.57735);
                glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                glNormal3f(-0.57735, 0.57735, -0.57735);
                glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                glNormal3f(-0.57735, 0.57735, 0.57735);
                glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                glNormal3f(0.57735, 0.57735, 0.57735);
                glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                glEnd();

                float mc3[] = { 0.2, 0.2, 0.6, 0.2 };
                int env = mpMap->rooms[pArea->rooms[i]]->environment;
                if( mpMap->envColors.contains(env) )
                    env = mpMap->envColors[env];
                else
                {
                    if( ! mpMap->customEnvColors.contains(env))
                    {
                        env = 1;
                    }
                }

                switch( env )
                {
                case 1:
                    glColor4b(128,50,50,2);
                    mc3[0]=128.0/255.0; mc3[1]=0.0/255.0; mc3[2]=0.0/255.0; mc3[3]=0.2;
                    break;

                case 2:
                    glColor4b(128,128,50, 2);
                    mc3[0]=0.0/255.0; mc3[1]=128.0/255.0; mc3[2]=0.0/255.0; mc3[3]=0.2;
                    break;
                case 3:
                    glColor4b(50,128,50,2);
                    mc3[0]=128.0/255.0; mc3[1]=128.0/255.0; mc3[2]=0.0/255.0; mc3[3]=0.2;
                    break;

                case 4:
                    glColor4b(50,50,128,2);
                    mc3[0]=0.0/255.0; mc3[1]=0.0/255.0; mc3[2]=128.0/255.0; mc3[3]=0.2;
                    break;

                case 5:
                    glColor4b(128,50,128,2);
                    mc3[0]=128.0/255.0; mc3[1]=128.0/255.0; mc3[2]=0.0/255.0; mc3[3]=0.2;
                    break;
                case 6:
                    glColor4b(50,128,128,2);
                    mc3[0]=0.0/255.0; mc3[1]=128.0/255.0; mc3[2]=128.0/255.0; mc3[3]=0.2;
                    break;
                case 7:
                    glColor4b(52,38,78,2);
                    mc3[0]=128.0/255.0; mc3[1]=128.0/255.0; mc3[2]=128.0/255.0; mc3[3]=0.2;
                    break;
                case 8:
                    glColor4b(65, 55, 35, 2);
                    mc3[0]=55.0/255.0; mc3[1]=55.0/255.0; mc3[2]=55.0/255.0; mc3[3]=0.2;
                    break;

                case 9:
                    glColor4b(175,50,50,2);
                    mc3[0]=255.0/255.0; mc3[1]=50.0/255.0; mc3[2]=50.0/255.0; mc3[3]=0.2;
                    break;

                case 10:
                    glColor4b(255,255,50,2);
                    mc3[0]=50.0/255.0; mc3[1]=255.0/255.0; mc3[2]=50.0/255.0; mc3[3]=0.2;
                    break;
                case 11:
                    glColor4b(50,175,175,2);
                    mc3[0]=255.0/255.0; mc3[1]=255.0/255.0; mc3[2]=50.0/255.0; mc3[3]=0.2;
                    break;

                case 12:
                    glColor4b(175,175,50,2);
                    mc3[0]=50.0/255.0; mc3[1]=50.0/255.0; mc3[2]=255.0/255.0; mc3[3]=0.2;
                    break;

                case 13:
                    glColor4b(175,50,175,2);
                    mc3[0]=255.0/255.0; mc3[1]=50.0/255.0; mc3[2]=255.0/255.0; mc3[3]=0.2;
                    break;
                case 14:
                    glColor4b(50,175,50,2);
                    mc3[0]=50.0/255.0; mc3[1]=255.0/255.0; mc3[2]=255.0/255.0; mc3[3]=0.2;
                    break;
                case 15:
                    glColor4b(50,50,175,2);
                    mc3[0]=255.0/255.0; mc3[1]=255.0/255.0; mc3[2]=255.0/255.0; mc3[3]=0.2;
                    break;
                default: //user defined room color
                    int __r, __g, __b;
                    if( ! mpMap->customEnvColors.contains(env) ) break;
                    QColor &_c = mpMap->customEnvColors[env];
                    glColor4b(_c.red(),_c.green(),_c.blue(),255);
                    mc3[0]=_c.redF();
                    mc3[1]=_c.greenF();
                    mc3[2]=_c.blueF();
                    mc3[3]=0.2;
                }
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mc3);
                glMateriali(GL_FRONT, GL_SHININESS, 56);
                glDisable(GL_DEPTH_TEST);
                glLoadIdentity();
                gluLookAt(px*0.1+xRot, py*0.1+yRot, pz*0.1+zRot, px*0.1, py*0.1, pz*0.1,0.0,1.0,0.0);
                glScalef( 0.075, 0.075, 0.020);
                glTranslatef( 1.333333333*rx,1.333333333*ry,5.0*(rz+0.25) );//+0.4

                glBegin( GL_QUADS );
                glNormal3f(0.57735, -0.57735, 0.57735);
                glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                glNormal3f(-0.57735, -0.57735, 0.57735);
                glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                glNormal3f(-0.57735, -0.57735, -0.57735);
                glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                glNormal3f(0.57735, -0.57735, -0.57735);
                glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                glNormal3f(0.57735, 0.57735, 0.57735);
                glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                glNormal3f(-0.57735, 0.57735, 0.57735);
                glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                glNormal3f(-0.57735, -0.57735, 0.57735);
                glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                glNormal3f(0.57735, -0.57735, 0.57735);
                glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

                glNormal3f(-0.57735, 0.57735, -0.57735);
                glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                glNormal3f(0.57735, 0.57735, -0.57735);
                glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                glNormal3f(0.57735, -0.57735, -0.57735);
                glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                glNormal3f(-0.57735, -0.57735, -0.57735);
                glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                glNormal3f(0.57735, 0.57735, -0.57735);
                glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                glNormal3f(0.57735, 0.57735, 0.57735);
                glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                glNormal3f(0.57735, -0.57735, 0.57735);
                glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
                glNormal3f(0.57735, -0.57735, -0.57735);
                glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

                glNormal3f(-0.57735, 0.57735, 0.57735);
                glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                glNormal3f(-0.57735, 0.57735, -0.57735);
                glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                glNormal3f(-0.57735, -0.57735, -0.57735);
                glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
                glNormal3f(-0.57735, -0.57735, 0.57735);
                glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

                glNormal3f(0.57735, 0.57735, -0.57735);
                glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                glNormal3f(-0.57735, 0.57735, -0.57735);
                glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
                glNormal3f(-0.57735, 0.57735, 0.57735);
                glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                glNormal3f(0.57735, 0.57735, 0.57735);
                glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
                glEnd();

                continue;
            }

            float mc3[] = { 0.2f, 0.2f, 0.7f, 1.0f };
            glLoadIdentity();
            gluLookAt(px*0.1+xRot, py*0.1+yRot, pz*0.1+zRot, px*0.1, py*0.1, pz*0.1,0.0,1.0,0.0);
            glScalef( 0.1, 0.1, 0.1);
            glTranslatef( rx, ry, rz );

            glLoadName( pArea->rooms[i] );
            quads++;
            glBegin( GL_QUADS );
            glNormal3f(0.57735, -0.57735, 0.57735);
            glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
            glNormal3f(-0.57735, -0.57735, 0.57735);
            glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
            glNormal3f(-0.57735, -0.57735, -0.57735);
            glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
            glNormal3f(0.57735, -0.57735, -0.57735);
            glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

            glNormal3f(0.57735, 0.57735, 0.57735);
            glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
            glNormal3f(-0.57735, 0.57735, 0.57735);
            glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
            glNormal3f(-0.57735, -0.57735, 0.57735);
            glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
            glNormal3f(0.57735, -0.57735, 0.57735);
            glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

            glNormal3f(-0.57735, 0.57735, -0.57735);
            glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
            glNormal3f(0.57735, 0.57735, -0.57735);
            glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
            glNormal3f(0.57735, -0.57735, -0.57735);
            glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
            glNormal3f(-0.57735, -0.57735, -0.57735);
            glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

            glNormal3f(0.57735, 0.57735, -0.57735);
            glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
            glNormal3f(0.57735, 0.57735, 0.57735);
            glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
            glNormal3f(0.57735, -0.57735, 0.57735);
            glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
            glNormal3f(0.57735, -0.57735, -0.57735);
            glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

            glNormal3f(-0.57735, 0.57735, 0.57735);
            glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
            glNormal3f(-0.57735, 0.57735, -0.57735);
            glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
            glNormal3f(-0.57735, -0.57735, -0.57735);
            glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
            glNormal3f(-0.57735, -0.57735, 0.57735);
            glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

            glNormal3f(0.57735, 0.57735, -0.57735);
            glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
            glNormal3f(-0.57735, 0.57735, -0.57735);
            glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
            glNormal3f(-0.57735, 0.57735, 0.57735);
            glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
            glNormal3f(0.57735, 0.57735, 0.57735);
            glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);

            glEnd();

            int env = mpMap->rooms[pArea->rooms[i]]->environment;
            if( mpMap->envColors.contains(env) )
                env = mpMap->envColors[env];
            else
            {
                if( ! mpMap->customEnvColors.contains(env))
                {
                    env = 1;
                }
            }
            switch( env )
            {
            case 1:
                glColor4b(128,50,50,255);
                mc3[0]=128.0/255.0; mc3[1]=0.0/255.0; mc3[2]=0.0/255.0; mc3[3]=255.0/255.0;
                break;

            case 2:
                glColor4b(128,128,50, 255);
                mc3[0]=0.0/255.0; mc3[1]=128.0/255.0; mc3[2]=0.0/255.0; mc3[3]=255.0/255.0;
                break;
            case 3:
                glColor4b(50,128,50,255);
                mc3[0]=128.0/255.0; mc3[1]=128.0/255.0; mc3[2]=0.0/255.0; mc3[3]=255.0/255.0;
                break;

            case 4:
                glColor4b(50,50,128,255);
                mc3[0]=0.0/255.0; mc3[1]=0.0/255.0; mc3[2]=128.0/255.0; mc3[3]=255.0/255.0;
                break;

            case 5:
                glColor4b(128,50,128,255);
                mc3[0]=128.0/255.0; mc3[1]=128.0/255.0; mc3[2]=0.0/255.0; mc3[3]=255.0/255.0;
                break;
            case 6:
                glColor4b(50,128,128,255);
                mc3[0]=0.0/255.0; mc3[1]=128.0/255.0; mc3[2]=128.0/255.0; mc3[3]=255.0/255.0;
                break;
            case 7:
                glColor4b(52,38,78,255);
                mc3[0]=128.0/255.0; mc3[1]=128.0/255.0; mc3[2]=128.0/255.0; mc3[3]=255.0/255.0;
                break;
            case 8:
                glColor4b(65, 55, 35, 255);
                mc3[0]=55.0/255.0; mc3[1]=55.0/255.0; mc3[2]=55.0/255.0; mc3[3]=255.0/255.0;
                break;

            case 9:
                glColor4b(175,50,50,255);
                mc3[0]=255.0/255.0; mc3[1]=50.0/255.0; mc3[2]=50.0/255.0; mc3[3]=255.0/255.0;
                break;

            case 10:
                glColor4b(255,255,50,255);
                mc3[0]=50.0/255.0; mc3[1]=255.0/255.0; mc3[2]=50.0/255.0; mc3[3]=255.0/255.0;
                break;
            case 11:
                glColor4b(50,175,175,255);
                mc3[0]=255.0/255.0; mc3[1]=255.0/255.0; mc3[2]=50.0/255.0; mc3[3]=255.0/255.0;
                break;

            case 12:
                glColor4b(175,175,50,255);
                mc3[0]=50.0/255.0; mc3[1]=50.0/255.0; mc3[2]=255.0/255.0; mc3[3]=255.0/255.0;
                break;

            case 13:
                glColor4b(175,50,175,255);
                mc3[0]=255.0/255.0; mc3[1]=50.0/255.0; mc3[2]=255.0/255.0; mc3[3]=255.0/255.0;
                break;
            case 14:
                glColor4b(50,175,50,255);
                mc3[0]=50.0/255.0; mc3[1]=255.0/255.0; mc3[2]=255.0/255.0; mc3[3]=255.0/255.0;
                break;
            case 15:
                glColor4b(50,50,175,255);
                mc3[0]=255.0/255.0; mc3[1]=255.0/255.0; mc3[2]=255.0/255.0; mc3[3]=255.0/255.0;
                break;
            default: //user defined room color
                int __r, __g, __b;
                if( ! mpMap->customEnvColors.contains(env) ) break;
                QColor &_c = mpMap->customEnvColors[env];
                glColor4b(_c.red(),_c.green(),_c.blue(),255);
                mc3[0]=_c.redF();
                mc3[1]=_c.greenF();
                mc3[2]=_c.blueF();
                mc3[3]=1.0;
            }
            glDisable(GL_DEPTH_TEST);
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mc3);
            glMateriali(GL_FRONT, GL_SHININESS, 56);
            glLoadIdentity();
            gluLookAt(px*0.1+xRot, py*0.1+yRot, pz*0.1+zRot, px*0.1, py*0.1, pz*0.1,0.0,1.0,0.0);
            glScalef( 0.075, 0.075, 0.020);
            glTranslatef( 1.333333333*rx,1.333333333*ry,5.0*(rz+0.25) );//+0.4

            glBegin( GL_QUADS );
            glNormal3f(0.57735, -0.57735, 0.57735);
            glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
            glNormal3f(-0.57735, -0.57735, 0.57735);
            glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
            glNormal3f(-0.57735, -0.57735, -0.57735);
            glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
            glNormal3f(0.57735, -0.57735, -0.57735);
            glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

            glNormal3f(0.57735, 0.57735, 0.57735);
            glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
            glNormal3f(-0.57735, 0.57735, 0.57735);
            glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
            glNormal3f(-0.57735, -0.57735, 0.57735);
            glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
            glNormal3f(0.57735, -0.57735, 0.57735);
            glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

            glNormal3f(-0.57735, 0.57735, -0.57735);
            glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
            glNormal3f(0.57735, 0.57735, -0.57735);
            glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
            glNormal3f(0.57735, -0.57735, -0.57735);
            glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
            glNormal3f(-0.57735, -0.57735, -0.57735);
            glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

            glNormal3f(0.57735, 0.57735, -0.57735);
            glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
            glNormal3f(0.57735, 0.57735, 0.57735);
            glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
            glNormal3f(0.57735, -0.57735, 0.57735);
            glVertex3f(1.0/dehnung, -1.0/dehnung, 1.0/dehnung);
            glNormal3f(0.57735, -0.57735, -0.57735);
            glVertex3f(1.0/dehnung, -1.0/dehnung, -1.0/dehnung);

            glNormal3f(-0.57735, 0.57735, 0.57735);
            glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
            glNormal3f(-0.57735, 0.57735, -0.57735);
            glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
            glNormal3f(-0.57735, -0.57735, -0.57735);
            glVertex3f(-1.0/dehnung, -1.0/dehnung, -1.0/dehnung);
            glNormal3f(-0.57735, -0.57735, 0.57735);
            glVertex3f(-1.0/dehnung, -1.0/dehnung, 1.0/dehnung);

            glNormal3f(0.57735, 0.57735, -0.57735);
            glVertex3f(1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
            glNormal3f(-0.57735, 0.57735, -0.57735);
            glVertex3f(-1.0/dehnung, 1.0/dehnung, -1.0/dehnung);
            glNormal3f(-0.57735, 0.57735, 0.57735);
            glVertex3f(-1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
            glNormal3f(0.57735, 0.57735, 0.57735);
            glVertex3f(1.0/dehnung, 1.0/dehnung, 1.0/dehnung);
            glEnd();
        }

        zEbene += 1.0;
    }
//    qDebug()<<"FINAL: mQuads.size()="<<mQuads.size()<<"area.size()="<<pArea->rooms.size()<<" quads="<<quads<<" verts="<<verts;
//    qDebug()<<"mScale="<<mScale<<" 1/mScale="<<1/mScale<<" env="<<env;
//    cout<<"dif r="<<diffuseLight[0]<<" g="<<diffuseLight[1]<<" b="<<diffuseLight[2]<<endl;
//    cout << "xRot:"<<xRot<<" yRot:"<<yRot<<" zRot:"<<zRot<<endl;
    glFlush();
    qDebug()<<"render:"<<__t.elapsed();
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport( 0, 0, (GLint)w, (GLint)h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    if( !ortho )
        gluPerspective( 60*mScale, (GLfloat)w / (GLfloat)h, 0.0001, 10000.0);
    else
        gluOrtho2D( 0.0, (GLdouble) w, 0.0, (GLdouble) h );
    glMatrixMode( GL_MODELVIEW );
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        int x = event->x();
        int y = height()-event->y();//opengl ursprungspunkt liegt unten links
        GLuint buff[16] = {0};
        GLint hits;
        GLint view[4];
        int id;
        glSelectBuffer( 16, buff );
        glGetIntegerv( GL_VIEWPORT, view );
        glRenderMode( GL_SELECT );
        glInitNames();
        glPushName( 0 );
        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        glLoadIdentity();
        gluPickMatrix(x, y, 0.1, 0.1, view);
        gluPerspective( 60*mScale, (GLfloat)width() / (GLfloat)height(), 0.0001, 10000.0);
        glMatrixMode(GL_MODELVIEW);

        mQuads.clear();
        mTarget=-22;
        selectionMode = true;
        paintGL();
        selectionMode = false;

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        hits = glRenderMode(GL_RENDER);
        int i;

        for (i = 0; i < hits; i++)
        {
            mTarget = buff[i * 4 + 3];
            unsigned int minZ = buff[i * 4 + 1];
            unsigned int maxZ = buff[i * 4 + 2];
        }
        glViewport( 0, 0, (GLint)width(), (GLint)height() );
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        gluPerspective( 60*mScale, (GLfloat)width() / (GLfloat)height(), 0.0001, 10000.0);
        glMatrixMode(GL_MODELVIEW);
        updateGL();
        if( mpMap->rooms.contains(mTarget) )
        {
            mpMap->mTargetID = mTarget;
            if( mpMap->findPath( mpMap->mRoomId, mpMap->mTargetID) )
            {
               qDebug()<<"glwidget: starting speedwalk path length="<<mpMap->mPathList.size();
               mpMap->mpHost->startSpeedWalk();
            }
            else
            {
                QMessageBox msgBox;
                msgBox.setText("Cannot find a path to this room using regular exits.#glWidget\n");
                msgBox.exec();
            }
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("ERROR: Target room cannot be found in map db.\n");
            msgBox.exec();
        }
    }
}

void GLWidget::mouseMoveEvent( QMouseEvent * event )
{
}

void GLWidget::wheelEvent ( QWheelEvent * e )
{
    int delta = e->delta() / 8 / 15;
    if( e->delta() < 0 )
    {
        if( abs(mScale) < 0.3 )
            mScale -= 0.01;
        else
            mScale -= 0.03;
        resizeGL(width(),height());
        updateGL();
        e->accept();
        return;
    }
    if( e->delta() > 0 )
    {
        if( abs(mScale) < 0.3 )
            mScale += 0.01;
        else
            mScale += 0.03;
        resizeGL(width(),height());
        updateGL();
        e->accept();
        return;
    }
    e->ignore();
    return;
}

GLuint GLWidget::makeObject()
{
}









