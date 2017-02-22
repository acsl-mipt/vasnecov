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

GLuint VasnecovFigure::pointsAmount() const
{
    QMutexLocker locker(mtx_data);

    return m_points.rawVerticesSize();
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

    return designerSetType(type);
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

void VasnecovFigure::createCircle(GLfloat r, const QColor &color, GLuint factor)
{
    if(r > 0.0 && factor > 0)
    {
        QMutexLocker locker(mtx_data);

        designerSetType(VasnecovFigure::TypePolylineLoop);

        if(color.isValid())
        {
            m_color.set(color);
        }

        std::vector<QVector3D> circ;
        circ.reserve(factor);

        QVector3D kt;
        for(GLuint i = 0; i < factor; ++i)
        {
            float angle(2*M_PI*i/factor);
            kt.setX(r*cos(angle));
            kt.setY(r*sin(angle));
            circ.push_back(kt);
        }
        m_points.set(circ);
    }
}

void VasnecovFigure::createArc(GLfloat r, GLfloat startAngle, GLfloat spanAngle, const QColor &color, GLuint factor)
{
    if(r > 0.0 && spanAngle != 0.0 && factor > 0)
    {
        QMutexLocker locker(mtx_data);

        designerSetType(VasnecovFigure::TypePolyline);

        if(color.isValid())
        {
            m_color.set(color);
        }

        startAngle *= c_degToRad;
        spanAngle *= c_degToRad;

        std::vector<QVector3D> circ;
        GLfloat endAngle = startAngle + spanAngle;
        GLuint steps(std::abs(static_cast<float>(factor) * spanAngle / (2.0f*M_PI)));
        if(steps == 0)
        {
            // Получается слишком маленький отрезок, поэтому он просто рисуется по двум точкам
            steps = 1;
        }
        GLfloat pointAngle(0.0);

        circ.reserve(steps + 1);

        QVector3D kt;
        for(GLuint i = 0; i < steps; ++i)
        {
            if(spanAngle > 0)
            {
                pointAngle = 2*M_PI*i/factor;
            }
            else
            {
                pointAngle = -2*M_PI*i/factor;
            }
            kt.setX(r*cos(pointAngle));
            kt.setY(r*sin(pointAngle));
            circ.push_back(kt);
        }

        kt.setX(r*cos(endAngle));
        kt.setY(r*sin(endAngle));
        circ.push_back(kt);

        m_points.set(circ);
    }
}

void VasnecovFigure::createPie(GLfloat r, GLfloat startAngle, GLfloat spanAngle, const QColor &color, GLuint factor)
{
    if(r > 0.0 && spanAngle != 0.0 && factor > 0)
    {
        QMutexLocker locker(mtx_data);

        designerSetType(VasnecovFigure::TypePolygons);

        if(color.isValid())
        {
            m_color.set(color);
        }

        startAngle *= c_degToRad;
        spanAngle *= c_degToRad;

        std::vector<QVector3D> circ;
        GLfloat endAngle = startAngle + spanAngle;
        GLuint steps(std::abs(static_cast<float>(factor) * spanAngle / (2.0f*M_PI)));
        if(steps == 0)
        {
            // Получается слишком маленький отрезок, поэтому он просто рисуется по двум точкам
            steps = 1;
        }
        GLfloat pointAngle(0.0);

        circ.reserve(1 + steps + 1);
        circ.push_back(QVector3D(0, 0, 0)); // Начальная позиция в центре круга

        QVector3D kt;
        for(GLuint i = 0; i < steps; ++i)
        {
            if(spanAngle > 0)
            {
                pointAngle = 2*M_PI*i/factor;
            }
            else
            {
                pointAngle = -2*M_PI*i/factor;
            }
            kt.setX(r*cos(pointAngle));
            kt.setY(r*sin(pointAngle));
            circ.push_back(kt);
        }

        kt.setX(r*cos(endAngle));
        kt.setY(r*sin(endAngle));
        circ.push_back(kt);

        m_points.set(circ);
    }
}

void VasnecovFigure::createSquareGrid(GLfloat width, GLfloat height, const QColor &color, GLuint horizontals, GLuint verticals)
{
    if(horizontals || verticals)
    {
        QMutexLocker locker(mtx_data);

        designerSetType(VasnecovFigure::TypeLines);

        if(color.isValid())
        {
            m_color.set(color);
        }

        std::vector<QVector3D> points;
        points.reserve((horizontals + verticals) * 2);

        if(horizontals)
        {
            GLfloat step((horizontals > 1) ? height/(horizontals - 1) : 0.0f);

            QVector3D p1(- width / 2.0f,
                         (horizontals > 1) ? - height / 2.0f : 0.0f,
                         0.0f);
            QVector3D p2( width / 2.0f, p1.y(), 0.0f);

            for(GLuint i = 0; i < horizontals; ++i)
            {
                points.push_back(p1);
                points.push_back(p2);

                p1.setY(p1.y() + step);
                p2.setY(p1.y());
            }
        }
        if(verticals)
        {
            GLfloat step((verticals > 1) ? width/(verticals - 1) : 0.0f);

            QVector3D p1((verticals > 1) ? - width / 2.0f : 0.0f,
                         - height / 2.0f,
                         0.0f);
            QVector3D p2(p1.x(), height / 2.0f, 0.0f);

            for(GLuint i = 0; i < verticals; ++i)
            {
                points.push_back(p1);
                points.push_back(p2);

                p1.setX(p1.x() + step);
                p2.setX(p1.x());
            }
        }

        m_points.set(points);
    }
}

GLboolean VasnecovFigure::designerSetType(VasnecovFigure::Types type)
{
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