#include "vasnecovfigure.h"
#include "technologist.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

/*!
 \brief

 \fn VasnecovFigure::VasnecovFigure
 \param mutex
 \param pipeline
 \param imya_
*/
VasnecovFigure::VasnecovFigure(QMutex *mutex, VasnecovPipeline *pipeline, const GLstring &name) :
    VasnecovElement(mutex, pipeline, name),
    m_type(raw_wasUpdated, Type, VasnecovPipeline::LoopLine),
    m_points(raw_wasUpdated, Points, true),
    m_thickness(raw_wasUpdated, Thickness, 1.0f),
    m_lighting(raw_wasUpdated, Lighting, false),
    m_depth(raw_wasUpdated, Depth, true)
{
}
/*!
 \brief

 \fn VasnecovFigure::~VasnecovFigure
*/
VasnecovFigure::~VasnecovFigure()
{
}

/*!
 \brief

 \fn VasnecovFigure::ZadatTochki
 \param toch
*/
void VasnecovFigure::setPoints(const std::vector <QVector3D> &points)
{
    QMutexLocker locker(mtx_data);

    m_points.set(points);
}
/*!
 \brief

 \fn VasnecovFigure::DobavitTochku
 \param toch
*/
void VasnecovFigure::addLastPoint(const QVector3D &point)
{
    QMutexLocker locker(mtx_data);

    m_points.addLast(point);
}

void VasnecovFigure::removeLastPoint()
{
    QMutexLocker locker(mtx_data);

    m_points.removeLast();
}

void VasnecovFigure::replaceLastPoint(const QVector3D &point)
{
    QMutexLocker locker(mtx_data);

    m_points.replaceLast(point);
}
/*!
 \brief

 \fn VasnecovFigure::OchistitTochki
*/
void VasnecovFigure::clearPoints()
{
    QMutexLocker locker(mtx_data);

    m_points.clear();
}

void VasnecovFigure::addFirstPoint(const QVector3D &point)
{
    QMutexLocker locker(mtx_data);

    m_points.addFirst(point);
}
void VasnecovFigure::removeFirstPoint()
{
    QMutexLocker locker(mtx_data);

    m_points.removeFirst();
}

void VasnecovFigure::replaceFirstPoint(const QVector3D &point)
{
    QMutexLocker locker(mtx_data);

    m_points.replaceFirst(point);
}
/*!
 \brief

 \fn VasnecovFigure::ZadatTip
 \param tip_
 \return GLboolean
*/
GLboolean VasnecovFigure::setType(VasnecovFigure::Types type)
{
    QMutexLocker locker(mtx_data);

    switch(type)
    {
        case TypePolyline:
            m_type.set(VasnecovPipeline::PolyLine);
            return true;
        case TypePolylineLoop:
            m_type.set(VasnecovPipeline::LoopLine);
            return true;
        case TypePolygons:
            m_type.set(VasnecovPipeline::FanTriangle);
            m_lighting.set(true);
            return true;
        case TypeLines:
            m_type.set(VasnecovPipeline::Lines);
            return true;
        case TypePoints:
            m_type.set(VasnecovPipeline::Points);
            return true;
        case TypeTriangles:
            m_type.set(VasnecovPipeline::Triangles);
            m_lighting.set(true);
            return true;
        default:
            return false;
    }
}

VasnecovFigure::Types VasnecovFigure::type() const
{
    QMutexLocker locker(mtx_data);

    switch(m_type.raw())
    {
        case VasnecovPipeline::PolyLine:
            return TypePolyline;
        case VasnecovPipeline::LoopLine:
            return TypePolylineLoop;
        case VasnecovPipeline::FanTriangle:
            return TypePolygons;
        case VasnecovPipeline::Lines:
            return TypeLines;
        case VasnecovPipeline::Points:
            return TypePoints;
        case VasnecovPipeline::Triangles:
            return TypeTriangles;
        default:
            return TypeUnknown;
    }
}
/*!
 \brief

 \fn VasnecovFigure::ZadatTolschinu
 \param tol_
 \return GLint
*/
GLint VasnecovFigure::setThickness(GLfloat thick)
{
    if(thick >= 1.0f && thick <= 16.0f)
    {
        QMutexLocker locker(mtx_data);
        m_thickness.set(thick);

        return 1;
    }
    else
    {
        return 0;
    }
}

GLfloat VasnecovFigure::thickness() const
{
    QMutexLocker locker(mtx_data);

    return m_thickness.raw();
}

void VasnecovFigure::setOptimization(GLboolean optimize)
{
    QMutexLocker locker(mtx_data);

    m_points.setOptimization(optimize);
}
GLboolean VasnecovFigure::optimization() const
{
    QMutexLocker locker(mtx_data);

    return m_points.optimization();
}

GLenum VasnecovFigure::renderUpdateData()
{
    // Проверка прозрачности
    GLboolean transp = false;

    if(m_color.raw().alphaF() < 1.0f)
    {
        transp = true;
    }
    m_isTransparency.set(transp);

    // Далее, как обычно
    GLenum updated(raw_wasUpdated);

    if(raw_wasUpdated)
    {
        if((raw_wasUpdated & Points) != Points) // FIXME: correct updating
        {
            pure_pipeline->setSomethingWasUpdated();
        }

        // Копирование сырых данных в основные
        m_type.update();
        m_points.update();

        m_thickness.update();
        m_lighting.update();
        m_depth.update();

        VasnecovElement::renderUpdateData();
    }

    return updated;
}

/*!
 \brief

 \fn VasnecovFigure::RisovatFiguru
*/
void VasnecovFigure::renderDraw()
{
    if(!m_isHidden.pure())
    {
        renderApplyTranslation();

        pure_pipeline->activateLamps(m_lighting.pure());
        pure_pipeline->activateDepth(m_depth.pure());

        pure_pipeline->setColor(m_color.pure());
        pure_pipeline->setLineWidth(m_thickness.pure());
        pure_pipeline->setPointSize(m_thickness.pure());

        pure_pipeline->drawElements(m_type.pure(),
                                    m_points.pureIndices(),
                                    m_points.pureVertices());
    }
}

GLfloat VasnecovFigure::renderCalculateDistanceToPlane(const QVector3D &planePoint, const QVector3D &normal)
{
    QVector3D centerPoint = m_points.cm();

    if(m_alienMs.pure())
    {
        centerPoint = (*m_alienMs.pure()) * m_Ms.pure() * centerPoint;
    }
    else
    {
        centerPoint = m_Ms.pure() * centerPoint;
    }

    pure_distance = centerPoint.distanceToPlane(planePoint, normal);

    return pure_distance;
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
