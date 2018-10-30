#pragma once

#include "ProductModel.h"

class BomberModel : public ProductModel
{
public:
    static uint propellersCount() {return _propellersCount;}

    BomberModel(VasnecovUniverse *universe,
                VasnecovWorld *world);
    BomberModel();
    ~BomberModel();

    void setVisible(bool visible = true);
    void setCoordinates(const QVector3D &coord);
    void setAngles(const QVector3D &angles);

    bool create(const QString &name,
                VasnecovProduct *parent = 0);
    void clear();
    void rotatePropellers(float angle);
    void setGhostMode(bool ghost = true);
    void setTheoreticalPosition(const QVector3D &position);
    void clearTheoreticalPosition();
    void clearTrack();

private:
    void removeBomber();
    void redrawLabel();
    void recalculateMistakeLine();
    void increaseTrack(const QVector3D &point);

private:
    static const uint _propellersCount = 4;

    VasnecovProduct *_beams;
    VasnecovProduct *_body;
    VasnecovProduct *_hat;
    VasnecovProduct *_battery;
    VasnecovProduct *_propellers[_propellersCount];
    VasnecovProduct *_motors[_propellersCount];

    VasnecovFigure  *_outletBody;
    VasnecovFigure  *_outletCircles[_propellersCount];
    VasnecovFigure  *_projectionLine;
    VasnecovFigure  *_mistakeLine;
    VasnecovFigure  *_track;
    std::vector<QVector3D> _trackPoints;

    QVector3D      *_theoreticalPosition;
    VasnecovLabel  *_theoreticalLabel;
    QImage          _theoreticalLabelImage; // FIXME: refactor

private:
    Q_DISABLE_COPY(BomberModel)
};
