#include "dashboard/elements/plot_time.h"
#include "dashboard/elements/value_display.h"
#include "ui/element_editors/curves_time_editor.h"

QTBPlotTime::QTBPlotTime(QTBoard *dashboard) :
    QTBDashboardElement (dashboard),
    mLayout(new QCPLayoutGrid),
    mLegendPosition(lpRight),
    mLegendVisible(true),
    mYAxisTicksVisible(false),
    mYAxisLabelsVisible(true),
    mYAxisGridVisible(true),
    mXAxisTicksVisible(false),
    mXAxisLabelsVisible(true),
    mXAxisGridVisible(true),
    mYAxisScale(asAuto),
    mXAxisHistory(30),
    mXAxisDirection(adRightToLeft),
    mThresholdsVisible(true)
{
    setParametersMaxCount(8);
    setConfigurationMode(QTBParameterConfiguration::cmGraph);
}

void QTBPlotTime::clearElement()
{
    if(mLayout) {
        for(int i=0;i<mLegendLayout->elementCount();i++) {
            auto * element = qobject_cast<QTBValueDisplay *>(mLegendLayout->elementAt(i));
            if(element)
                element->clearElement();
        }

        QList<QCPAbstractItem*> items = mAxisRect->items();
        for(int i=0; i<items.count();i++)
            mBoard->removeItem(items.at(i));

        mAxisRect->clearPlottables();
        mLegendLayout->clear();
        mLayout->clear();
        delete mLayout;
    }
}

void QTBPlotTime::initializeElement(QTBoard *dashboard)
{
    if(dashboard) {
        QTBDashboardElement::initializeElement(dashboard);

        mLayout->initializeParentPlot(dashboard);
        mLayout->setLayer(QLatin1String("main"));
        //        mLayout->setParent(this);

        mAxisRect = new QTBAxisRect(dashboard);
        mAxisRect->setMinimumMargins(QMargins(50,15,10,15));

        mLegendLayout = new QTBLayoutGrid();
        mLegendLayout->initializeParentPlot(dashboard);
        //        mLegendLayout->setMinimumSize(50,50);
        mLegendLayout->setRowSpacing(0);
        mLegendLayout->setMargins(QMargins(0,0,0,0));
        mLegendLayout->setLayer(QLatin1String("main"));

        mLegendEmptyElementFirst = new QCPLayoutElement(dashboard);
        mLegendEmptyElementFirst->setMinimumSize(0.1,0.1);
        mLegendEmptyElementLast = new QCPLayoutElement(dashboard);
        mLegendEmptyElementLast->setMinimumSize(0.1,0.1);
        mLegendLayout->addElement(mLegendEmptyElementFirst);
        mLegendLayout->addElement(mLegendEmptyElementLast);
        mLegendLayout->setRowStretchFactor(0,0.01);

        mLayout->addElement(mAxisRect);

        QSharedPointer<QCPAxisTickerDateTime> timeTicker(new QCPAxisTickerDateTime);
        timeTicker->setDateTimeFormat("HH:mm:ss");
        timeTicker->setDateTimeSpec(Qt::UTC);
        timeTicker->setTickCount(3);
        mAxisRect->axis(QCPAxis::atBottom)->setTicker(timeTicker);
    }
}

void QTBPlotTime::edit()
{
    QTBCurvesTimeEditor editor(this);
    editor.exec();
}

void QTBPlotTime::loadSettings(QSettings *settings)
{
    settings->beginGroup("General");
    if(settings->contains("LegendVisible"))
        setLegendVisible(settings->value("LegendVisible").toBool());
    if(settings->contains("LegendPosition"))
        setLegendPosition(QTBPlotTime::LegendPosition(settings->value("LegendPosition").toInt()));
    if(settings->contains("TransparentBackground"))
        setTransparentBackground(settings->value("TransparentBackground").toBool());
    settings->endGroup();

    settings->beginGroup("ValueAxis");
    if(settings->contains("TicksVisible"))
        setYAxisTicksVisible(settings->value("TicksVisible").toBool());
    if(settings->contains("LabelsVisible"))
        setYAxisLabelsVisible(settings->value("LabelsVisible").toBool());
    if(settings->contains("GridVisible"))
        setYAxisGridVisible(settings->value("GridVisible").toBool());
    if(settings->contains("ScaleMode"))
        setYAxisScale(YAxisScale(settings->value("ScaleMode").toInt()));
    if(settings->contains("ScaleMin"))
        setYAxisMinCustom(settings->value("ScaleMin").toDouble());
    if(settings->contains("ScaleMax"))
        setYAxisMaxCustom(settings->value("ScaleMax").toDouble());
    if(settings->contains("ThresholdsVisible"))
        setThresholdsVisible(settings->value("ThresholdsVisible").toBool());
    settings->endGroup();

    settings->beginGroup("TimeAxis");
    if(settings->contains("TicksVisible"))
        setXAxisTicksVisible(settings->value("TicksVisible").toBool());
    if(settings->contains("LabelsVisible"))
        setXAxisLabelsVisible(settings->value("LabelsVisible").toBool());
    if(settings->contains("GridVisible"))
        setXAxisGridVisible(settings->value("GridVisible").toBool());
    if(settings->contains("History"))
        setXAxisHistory(settings->value("History").toInt());
    if(settings->contains("Direction"))
        setXAxisDirection(XAxisDirection(settings->value("Direction").toInt()));
    settings->endGroup();
}

void QTBPlotTime::loadConfigurations(QSettings *settings)
{
    QTBDashboardElement::loadConfigurations(settings);

    for(int i=0; i< parametersCount(); i++) {
        QSharedPointer<QTBDashboardParameter> dashParam = dashParameter(i);
        if(dashParam) {
            auto *legend = new QTBValueDisplay(mBoard);
            legend->initializeElement(mBoard);
            legend->addParameter(dashParam);
            legend->setTransparentBackground(true);
            if(mLegendPosition == lpLeft ||
                    mLegendPosition == lpRight) {
                legend->setOrientation(QTBValueDisplay::doVerticalAlignLeftRight);
            } else {
                legend->setOrientation(QTBValueDisplay::doVerticalAlignCenter);
            }

            mLegendLayout->take(mLegendEmptyElementLast);
            mLegendLayout->simplify();
            mLegendLayout->addElement(legend);
            mLegendLayout->addElement(mLegendEmptyElementLast);
            mLegendLayout->setRowStretchFactor(mLegendLayout->rowCount()-1 , 0.01);

            new QTBGraph(mAxisRect->axis(QCPAxis::atBottom), mAxisRect->axis(QCPAxis::atLeft));
        }
    }

    updateElement();
}

void QTBPlotTime::saveSettings(QSettings *settings)
{
    settings->beginGroup("General");
    settings->setValue("LegendVisible", mLegendVisible);
    settings->setValue("LegendPosition", mLegendPosition);
    settings->setValue("TransparentBackground", mTransparentBackground);
    settings->endGroup();

    settings->beginGroup("ValueAxis");
    settings->setValue("TicksVisible", mYAxisTicksVisible);
    settings->setValue("LabelsVisible", mYAxisLabelsVisible);
    settings->setValue("GridVisible", mYAxisGridVisible);
    settings->setValue("ScaleMode", mYAxisScale);
    settings->setValue("ScaleMin", mYAxisMinCustom);
    settings->setValue("ScaleMax", mYAxisMaxCustom);
    settings->setValue("ThresholdsVisible", mThresholdsVisible);
    settings->endGroup();

    settings->beginGroup("TimeAxis");
    settings->setValue("TicksVisible", mXAxisTicksVisible);
    settings->setValue("LabelsVisible", mXAxisLabelsVisible);
    settings->setValue("GridVisible", mXAxisGridVisible);
    settings->setValue("History", mXAxisHistory);
    settings->setValue("Direction", mXAxisDirection);
    settings->endGroup();
}

QSharedPointer<QTBDashboardParameter> QTBPlotTime::addParameter(QExplicitlySharedDataPointer<QTBParameterConfiguration> parameterSettings)
{
    QSharedPointer<QTBDashboardParameter> dashParam = QTBDashboardElement::addParameter(parameterSettings);
    return dashParam;
}

QSharedPointer<QTBDashboardParameter> QTBPlotTime::addParameter(QString paramLabel)
{
    QSharedPointer<QTBDashboardParameter> dashParam = QTBDashboardElement::addParameter(paramLabel);
    return dashParam;
}

void QTBPlotTime::addParameter(QSharedPointer<QTBDashboardParameter> dashParameter)
{
    QTBDashboardElement::addParameter(dashParameter);

    if(mLegendLayout->elementCount() == parametersMaxCount() + 2) {

        auto *legend = qobject_cast<QTBValueDisplay *>(mLegendLayout->elementAt(parametersCount()));
        if(legend) {
            legend->addParameter(dashParameter);
            mAxisRect->graphs().last()->data()->clear();

            if(mBoard->dataManager()) {
                QTBDataSerie serie = mBoard->dataManager()->dataSerie(dashParameter->getParameterId());
                QCPDataContainer<QTBDataSample>::const_iterator it;
                for (it = serie.constBegin(); it != serie.constEnd(); ++it)
                    mAxisRect->graphs().last()->addData(it->datationSecFromMidnight(), it->value().toDouble());
            }
        }
    } else {
        auto *legend = new QTBValueDisplay(mBoard);
        legend->initializeElement(mBoard);
        legend->addParameter(dashParameter);
        legend->setTransparentBackground(true);
        if(mLegendPosition == lpLeft ||
                mLegendPosition == lpRight) {
            legend->setOrientation(QTBValueDisplay::doVerticalAlignLeftRight);
        } else {
            legend->setOrientation(QTBValueDisplay::doVerticalAlignCenter);
        }

        mLegendLayout->take(mLegendEmptyElementLast);
        mLegendLayout->simplify();
        mLegendLayout->addElement(legend);
        mLegendLayout->addElement(mLegendEmptyElementLast);
        mLegendLayout->setRowStretchFactor(mLegendLayout->rowCount()-1 , 0.01);
        mLegendLayout->needUpdate(true);

        new QTBGraph(mAxisRect->axis(QCPAxis::atBottom), mAxisRect->axis(QCPAxis::atLeft));

        if(mBoard->dataManager()) {
            QTBDataSerie serie = mBoard->dataManager()->dataSerie(dashParameter->getParameterId());
            QCPDataContainer<QTBDataSample>::const_iterator it;
            for (it = serie.constBegin(); it != serie.constEnd(); ++it)
                mAxisRect->graphs().last()->addData(it->datationSecFromMidnight(), it->value().toDouble());
        }
    }

    updateElement();
}

void QTBPlotTime::removeDashParameter(int index)
{
    mBoard->removeGraph(mAxisRect->graphs().at(index));
    QCPLayoutElement *el = mLegendLayout->takeAt(index + 1);

    auto * element = qobject_cast<QTBValueDisplay *>(el);
    if(element)
        element->clearElement();

    el->deleteLater();

    mLegendLayout->simplify();
    mLegendLayout->needUpdate(true);
    QTBDashboardElement::removeDashParameter(index);

    updateElement();
}

void QTBPlotTime::processNewSamples()
{
    for(int i=0;i<mLegendLayout->elementCount();i++) {
        auto * element = qobject_cast<QTBValueDisplay *>(mLegendLayout->elementAt(i));
        if(element) {
            element->processNewSamples();
        }
    }
    for(int i=0; i< parametersCount(); i++) {
        QSharedPointer<QTBDashboardParameter> dashParam = dashParameter(i);
        if(dashParam && dashParam->getParameterId() > 0) {
            if(i < mAxisRect->graphs().count()) {
                mAxisRect->graphs().at(i)->addData(dashParam->getTimestamp(), dashParam->getValueDouble());
                mAxisRect->graphs().at(i)->data()->removeBefore(dashParam->getTimestamp()-(mXAxisHistory+1));
            }
        }
    }

    updateAxes();
}

void QTBPlotTime::processHistoricalSamples()
{
    if(mBoard->dataManager()) {
        for(int i=0; i< parametersCount(); i++) {
            QSharedPointer<QTBDashboardParameter> dashParam = dashParameter(i);
            if(dashParam) {
                if(i < mAxisRect->graphs().count()) {
                    QTBDataSerie serie = mBoard->dataManager()->dataSerie(dashParam->getParameterId());
                    QCPDataContainer<QTBDataSample>::const_iterator it;
                    for (it = serie.constBegin(); it != serie.constEnd(); ++it)
                        mAxisRect->graphs().at(i)->addData(it->datationSecFromMidnight(), it->value().toDouble());
                }
            }
        }
    }
}

void QTBPlotTime::updateLegendSize()
{
    if(mLegendVisible) {
    QSizeF size = mBoard->dashboardLayout()->singleElementSize();

    if(mLegendPosition == lpLeft ||
            mLegendPosition == lpRight) {
            mLegendLayout->setMinimumSize(size.width(), 0.1);
        for(int i=1; i< mLegendLayout->elementCount() - 1;i++) {
            mLegendLayout->elementAt(i)->setMinimumSize(size.width(), 0);
            mLegendLayout->elementAt(i)->setMaximumSize(size.width(), size.height());
        }
    } else {
            mLegendLayout->setMinimumSize(0.1, size.height());
        for(int i=1; i< mLegendLayout->elementCount() - 1;i++) {
                mLegendLayout->elementAt(i)->setMinimumSize(size.width(), 0);
                mLegendLayout->elementAt(i)->setMaximumSize(size.width(), size.height());
            }
        }
    }
}

void QTBPlotTime::updateAxes()
{
    switch(mYAxisScale) {
    case asAuto:
    {
        mAxisRect->axis(QCPAxis::atLeft)->rescale();
        if(mAxisRect->axis(QCPAxis::atLeft)->range() != mAutoRange) {
            mAxisRect->axis(QCPAxis::atLeft)->scaleRange(1.05, mAxisRect->axis(QCPAxis::atLeft)->range().center());
            mAutoRange = mAxisRect->axis(QCPAxis::atLeft)->range();
        }
        mYAxisMinCustom = mAxisRect->axis(QCPAxis::atLeft)->range().lower;
        mYAxisMaxCustom = mAxisRect->axis(QCPAxis::atLeft)->range().upper;
    }
        break;
    case asParam:
    {
        bool validRange = false;
        for(int i=0; i< parametersCount(); i++) {
            QSharedPointer<QTBDashboardParameter> dashParam = dashParameter(i);
            if(dashParam) {
                if(i==0) {
                    if(dashParam->parameterConfiguration()->validRange()) {
                        validRange = true;
                        mYAxisMinCustom = dashParam->parameterConfiguration()->rangeMinimum();
                        mYAxisMaxCustom = dashParam->parameterConfiguration()->rangeMaximum();
                    }
                } else {
                    if(dashParam->parameterConfiguration()->validRange()) {
                        validRange = true;
                        if(dashParam->parameterConfiguration()->rangeMinimum() < mYAxisMinCustom)
                            mYAxisMinCustom = dashParam->parameterConfiguration()->rangeMinimum();
                        if(dashParam->parameterConfiguration()->rangeMaximum() > mYAxisMaxCustom)
                            mYAxisMaxCustom = dashParam->parameterConfiguration()->rangeMaximum();
                    }
                }
            }
        }

        if(!validRange) {
            mAxisRect->axis(QCPAxis::atLeft)->rescale();
            if(mAxisRect->axis(QCPAxis::atLeft)->range() != mAutoRange) {
                mAxisRect->axis(QCPAxis::atLeft)->scaleRange(1.05, mAxisRect->axis(QCPAxis::atLeft)->range().center());
                mAutoRange = mAxisRect->axis(QCPAxis::atLeft)->range();
            }
            mYAxisMinCustom = mAxisRect->axis(QCPAxis::atLeft)->range().lower;
            mYAxisMaxCustom = mAxisRect->axis(QCPAxis::atLeft)->range().upper;
        } else {
            mAxisRect->axis(QCPAxis::atLeft)->setRange(QCPRange(mYAxisMinCustom, mYAxisMaxCustom));
        }
        break;
    }
    case asCustom:
        mAxisRect->axis(QCPAxis::atLeft)->setRange(QCPRange(mYAxisMinCustom, mYAxisMaxCustom));
        break;
    }

    if(mBoard->liveDataRefreshEnabled())
        mAxisRect->axis(QCPAxis::atBottom)->setRange(mBoard->currentTimestamp() - mXAxisHistory, mBoard->currentTimestamp());
}

void QTBPlotTime::updateThresholdsItems()
{
    QList<QCPAbstractItem*> items = mAxisRect->items();
    for(int i=0; i<items.count();i++)
        mBoard->removeItem(items.at(i));

    for(int i=0; i< parametersCount(); i++) {
        QSharedPointer<QTBDashboardParameter> dashParam = dashParameter(i);
        if(dashParam) {
            QTBGraph *graph = nullptr;
            if(i < mAxisRect->graphs().count()) {
                graph = qobject_cast<QTBGraph*>(mAxisRect->graphs().at(i));
                if(graph)
                    graph->clearColoredSegments();
            }
            double lower = -std::numeric_limits<double>::max();
            QMap<double, QTBColorSettings> lowThr = dashParam->parameterConfiguration()->thresholdsSettingsRef().lowThresholds();
            QMap<double, QTBColorSettings>::iterator lowIt;
            for (lowIt = lowThr.begin();
                 lowIt != lowThr.end(); ++lowIt) {


                if(mThresholdsVisible && dashParam->parameterConfiguration()->itemsThresholdsVisible()) {
                    auto* l = new QCPItemStraightLine(mBoard);
                    l->setLayer(QLatin1String("grid"));
                    l->point1->setType(QCPItemPosition::ptPlotCoords);
                    l->point2->setType(QCPItemPosition::ptPlotCoords);
                    l->point1->setAxes(mAxisRect->axis(QCPAxis::atBottom), mAxisRect->axis(QCPAxis::atLeft));
                    l->point2->setAxes(mAxisRect->axis(QCPAxis::atBottom), mAxisRect->axis(QCPAxis::atLeft));
                    l->point1->setCoords(0.0, lowIt.key());
                    l->point2->setCoords(1.0, lowIt.key());
                    l->setClipAxisRect(mAxisRect);
                    l->setClipToAxisRect(true);
                    QColor color = lowIt.value().color();
                    color.setAlpha(150);
                    QPen pen = QPen(color);
                    pen.setStyle(Qt::DashDotDotLine);
                    l->setPen(pen);
                }

                if(graph && dashParam->parameterConfiguration()->itemColorMode() == QTBParameterConfiguration::icDynamic) {

                    QBrush brush;
                    switch(dashParam->parameterConfiguration()->graphBrush()) {
                    case QTBParameterConfiguration::bsNone :
                    {
                        brush = Qt::NoBrush;
                    }
                        break;
                    case QTBParameterConfiguration::bsFilled :
                    {
                        QColor color = lowIt.value().color();
                        color.setAlpha(50);
                        brush = QBrush(color);
                    }
                        break;
                    case QTBParameterConfiguration::bsGradient :
                        QColor color = lowIt.value().color();
                        color.setAlpha(200);

                        QLinearGradient gradient(0, 1, 0, 0);
                        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
                        gradient.setColorAt(1, color);
                        gradient.setColorAt(0.1, QColor::fromRgbF(0, 0, 0, 0));
                        gradient.setColorAt(0, QColor::fromRgbF(0, 0, 0, 0));
                        brush = QBrush(gradient);
                        break;
                    }

                    graph->addColoredSegment(lower, lowIt.key(), QPen(lowIt.value().color()), brush);
                    lower = lowIt.key();
                }
            }

            QMap<double, QTBColorSettings> highThr = dashParam->parameterConfiguration()->thresholdsSettingsRef().highThresholds();
            QMapIterator<double, QTBColorSettings> highIt(highThr);
            highIt.toBack();
            double upper = std::numeric_limits<double>::max();
            while (highIt.hasPrevious()) {
                highIt.previous();
                if(mThresholdsVisible && dashParam->parameterConfiguration()->itemsThresholdsVisible()) {
                    auto* l = new QCPItemStraightLine(mBoard);
                    l->setLayer(QLatin1String("grid"));
                    l->point1->setType(QCPItemPosition::ptPlotCoords);
                    l->point2->setType(QCPItemPosition::ptPlotCoords);
                    l->point1->setAxes(mAxisRect->axis(QCPAxis::atBottom), mAxisRect->axis(QCPAxis::atLeft));
                    l->point2->setAxes(mAxisRect->axis(QCPAxis::atBottom), mAxisRect->axis(QCPAxis::atLeft));
                    l->point1->setCoords(0.0, highIt.key());
                    l->point2->setCoords(1.0, highIt.key());
                    l->setClipAxisRect(mAxisRect);
                    l->setClipToAxisRect(true);
                    QColor color = highIt.value().color();
                    color.setAlpha(150);
                    QPen pen = QPen(color);
                    pen.setStyle(Qt::DashDotDotLine);
                    l->setPen(pen);
                }

                if(graph && dashParam->parameterConfiguration()->itemColorMode() == QTBParameterConfiguration::icDynamic) {
                    QBrush brush;
                    switch(dashParam->parameterConfiguration()->graphBrush()) {
                    case QTBParameterConfiguration::bsNone :
                    {
                        brush = Qt::NoBrush;
                    }
                        break;
                    case QTBParameterConfiguration::bsFilled :
                    {
                        QColor color = highIt.value().color();
                        color.setAlpha(50);
                        brush = QBrush(color);
                    }
                        break;
                    case QTBParameterConfiguration::bsGradient :
                        QColor color = highIt.value().color();
                        color.setAlpha(200);

                        QLinearGradient gradient(0, 1, 0, 0);
                        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
                        gradient.setColorAt(1, color);
                        gradient.setColorAt(0.1, QColor::fromRgbF(0, 0, 0, 0));
                        gradient.setColorAt(0, QColor::fromRgbF(0, 0, 0, 0));
                        brush = QBrush(gradient);
                        break;
                    }

                    graph->addColoredSegment(highIt.key(), upper, QPen(highIt.value().color()), brush);
                    upper = highIt.key();
                }
            }
        }
    }
}

void QTBPlotTime::updateGraphsStyle()
{
    for(int i=0; i< parametersCount(); i++) {
        QSharedPointer<QTBDashboardParameter> dashParam = dashParameter(i);
        if(dashParam) {
            if(i < mAxisRect->graphs().count()) {
                if(dashParam->parameterConfiguration()->itemColorMode() == QTBParameterConfiguration::icCustom)
                    mAxisRect->graphs().at(i)->setPen(dashParam->parameterConfiguration()->itemStaticColor());
                else
                    mAxisRect->graphs().at(i)->setPen(dashParam->parameterConfiguration()->defaultColorSettingsRef().color());

                switch(dashParam->parameterConfiguration()->graphBrush()) {
                case QTBParameterConfiguration::bsNone :
                {
                    mAxisRect->graphs().at(i)->setBrush(Qt::NoBrush);
                }
                    break;
                case QTBParameterConfiguration::bsFilled :
                {
                    QColor color = dashParam->parameterConfiguration()->defaultColorSettingsRef().color();
                    color.setAlpha(50);
                    mAxisRect->graphs().at(i)->setBrush(color);
                    QTBGraph *graph = qobject_cast<QTBGraph*>(mAxisRect->graphs().at(i));
                    if(graph)
                        graph->setFillStyle(QTBGraph::fsZero);
                }
                    break;
                case QTBParameterConfiguration::bsGradient :
                    QColor color = dashParam->parameterConfiguration()->defaultColorSettingsRef().color();
                    color.setAlpha(200);
                    QLinearGradient gradient(0, 1, 0, 0);
                    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
                    gradient.setColorAt(1, color);
                    gradient.setColorAt(0.1, QColor::fromRgbF(0, 0, 0, 0));
                    gradient.setColorAt(0, QColor::fromRgbF(0, 0, 0, 0));
                    QBrush brush(gradient);

                    mAxisRect->graphs().at(i)->setBrush(brush);
                    QTBGraph *graph = qobject_cast<QTBGraph*>(mAxisRect->graphs().at(i));
                    if(graph)
                        graph->setFillStyle(QTBGraph::fsBottom);
                    break;
                }
                mAxisRect->graphs().at(i)->setLineStyle(dashParam->parameterConfiguration()->graphLineStyle());
                mAxisRect->graphs().at(i)->setScatterStyle(dashParam->parameterConfiguration()->scatterStyle());
            }
        }
    }
}

void QTBPlotTime::updateLayout()
{
    mLayout->take(mAxisRect);
    mLayout->take(mLegendLayout);
    mLegendLayout->setVisible(false);
    mLayout->simplify();

    switch(mLegendPosition) {
    case lpRight:
        mLayout->setFillOrder(QCPLayoutGrid::foColumnsFirst);
        mLayout->addElement(mAxisRect);

        if(mLegendLayout->elementCount() > 2 && mLegendVisible) {
            mLayout->addElement(mLegendLayout);
            mLegendLayout->setVisible(true);
            mLayout->setColumnStretchFactor(0,1);
            mLayout->setColumnStretchFactor(1,0.01);
        }

        mLegendLayout->setFillOrder(QCPLayoutGrid::foRowsFirst);
        for(int i=0;i<mLegendLayout->elementCount();i++) {
            auto * element = qobject_cast<QTBValueDisplay *>(mLegendLayout->elementAt(i));
            if(element) {
                element->setOrientation(QTBValueDisplay::doVerticalAlignLeftRight);
            }
        }

        mLegendLayout->setRowStretchFactor(0,0.01);
        mLegendLayout->setRowStretchFactor(mLegendLayout->elementCount() - 1,0.01);

        break;
    case lpLeft:
        mLayout->setFillOrder(QCPLayoutGrid::foColumnsFirst);
        if(mLegendLayout->elementCount() > 2 && mLegendVisible) {
            mLayout->addElement(mLegendLayout);
            mLayout->addElement(mAxisRect);
            mLayout->setColumnStretchFactor(0,0.01);
            mLayout->setColumnStretchFactor(1,1);

            mLegendLayout->setVisible(true);
        } else {
            mLayout->addElement(mAxisRect);
        }

        mLegendLayout->setFillOrder(QCPLayoutGrid::foRowsFirst);
        for(int i=0;i<mLegendLayout->elementCount();i++) {
            auto * element = qobject_cast<QTBValueDisplay *>(mLegendLayout->elementAt(i));
            if(element) {
                element->setOrientation(QTBValueDisplay::doVerticalAlignLeftRight);
            }
        }
        mLegendLayout->setRowStretchFactor(0,0.01);
        mLegendLayout->setRowStretchFactor(mLegendLayout->elementCount() - 1,0.01);
        break;
    case lpBottom:
        mLayout->setFillOrder(QCPLayoutGrid::foRowsFirst);
        mLayout->addElement(mAxisRect);
        if(mLegendLayout->elementCount() > 2 && mLegendVisible) {
            mLayout->addElement(mLegendLayout);
            mLegendLayout->setVisible(true);
            mLayout->setRowStretchFactor(0,1);
            mLayout->setRowStretchFactor(1,0.01);
        }

        mLegendLayout->setFillOrder(QCPLayoutGrid::foColumnsFirst);
        for(int i=0;i<mLegendLayout->elementCount();i++) {
            auto * element = qobject_cast<QTBValueDisplay *>(mLegendLayout->elementAt(i));
            if(element) {
                element->setOrientation(QTBValueDisplay::doVerticalAlignCenter);
            }
        }
        mLegendLayout->setColumnStretchFactor(0,0.01);
        mLegendLayout->setColumnStretchFactor(mLegendLayout->elementCount() - 1,0.01);
        break;
    case lpTop:
        mLayout->setFillOrder(QCPLayoutGrid::foRowsFirst);

        if(mLegendLayout->elementCount() > 2 && mLegendVisible) {
            mLayout->addElement(mLegendLayout);
            mLayout->addElement(mAxisRect);
            mLayout->setRowStretchFactor(0,0.01);
            mLayout->setRowStretchFactor(1,1);
            mLegendLayout->setVisible(true);
        } else {
            mLayout->addElement(mAxisRect);
        }

        mLegendLayout->setFillOrder(QCPLayoutGrid::foColumnsFirst);
        for(int i=0;i<mLegendLayout->elementCount();i++) {
            auto * element = qobject_cast<QTBValueDisplay *>(mLegendLayout->elementAt(i));
            if(element) {
                element->setOrientation(QTBValueDisplay::doVerticalAlignCenter);
            }
        }
        mLegendLayout->setColumnStretchFactor(0,0.01);
        mLegendLayout->setColumnStretchFactor(mLegendLayout->elementCount() - 1,0.01);
        break;
    }

    if(mYAxisLabelsVisible || mYAxisGridVisible || mYAxisTicksVisible)
        mAxisRect->axis(QCPAxis::atLeft)->setVisible(true);
    else
        mAxisRect->axis(QCPAxis::atLeft)->setVisible(false);


    if(mXAxisLabelsVisible || mXAxisGridVisible || mXAxisTicksVisible)
        mAxisRect->axis(QCPAxis::atBottom)->setVisible(true);
    else
        mAxisRect->axis(QCPAxis::atBottom)->setVisible(false);

    if(mYAxisLabelsVisible)
        mAxisRect->setMinimumMargins(QMargins(50,15,22,15));
    else
        mAxisRect->setMinimumMargins(QMargins(22,15,22,15));
}

void QTBPlotTime::updateElement()
{
    updateLegendSize();
    updateLayout();

    for(int i=0;i<mLegendLayout->elementCount();i++) {
        auto * element = qobject_cast<QTBValueDisplay *>(mLegendLayout->elementAt(i));
        if(element) {
            element->updateElement();
        }
    }

    updateThresholdsItems();
    updateGraphsStyle();
}

void QTBPlotTime::update(QCPLayoutElement::UpdatePhase phase)
{
    QTBDashboardElement::update(phase);
    switch (phase)
    {
    case upPreparation:
    {
        updateLegendSize();
        break;
    }
    case upLayout:
    {
        mLayout->setOuterRect(rect());
        break;
    }
    default: break;
    }

    mLayout->update(phase);
}

bool QTBPlotTime::thresholdsVisible() const
{
    return mThresholdsVisible;
}

void QTBPlotTime::setThresholdsVisible(bool thresholdsVisible)
{
    if(thresholdsVisible != mThresholdsVisible) {
        mThresholdsVisible = thresholdsVisible;
        updateThresholdsItems();
    }
}

QTBPlotTime::LegendPosition QTBPlotTime::legendPosition() const
{
    return mLegendPosition;
}

void QTBPlotTime::setLegendPosition(const LegendPosition &legendPosition)
{
    if(legendPosition != mLegendPosition) {
        mLegendPosition = legendPosition;
    }
}

bool QTBPlotTime::legendVisible() const
{
    return mLegendVisible;
}

void QTBPlotTime::setLegendVisible(bool legendVisible)
{
    if(legendVisible != mLegendVisible) {
        mLegendVisible = legendVisible;
    }
}

bool QTBPlotTime::yAxisTicksVisible() const
{
    return mYAxisTicksVisible;
}

void QTBPlotTime::setYAxisTicksVisible(bool yAxisTicksVisible)
{
    if(yAxisTicksVisible != mYAxisTicksVisible) {
        mYAxisTicksVisible = yAxisTicksVisible;
        if(mYAxisTicksVisible) {
            mAxisRect->axis(QCPAxis::atLeft)->setBasePen(QPen(mBoard->frontColor()));
            mAxisRect->axis(QCPAxis::atLeft)->setTickPen(QPen(mBoard->frontColor()));
            mAxisRect->axis(QCPAxis::atLeft)->setSubTickPen(QPen(mBoard->frontColor()));
        } else {
            mAxisRect->axis(QCPAxis::atLeft)->setBasePen(Qt::NoPen);
            mAxisRect->axis(QCPAxis::atLeft)->setTickPen(Qt::NoPen);
            mAxisRect->axis(QCPAxis::atLeft)->setSubTickPen(Qt::NoPen);
        }
    }
}

bool QTBPlotTime::yAxisLabelsVisible() const
{
    return mYAxisLabelsVisible;
}

void QTBPlotTime::setYAxisLabelsVisible(bool yAxisLabelsVisible)
{
    if(mYAxisLabelsVisible != yAxisLabelsVisible) {
        mYAxisLabelsVisible = yAxisLabelsVisible;
        mAxisRect->axis(QCPAxis::atLeft)->setTickLabels(mYAxisLabelsVisible);
    }
}

bool QTBPlotTime::yAxisGridVisible() const
{
    return mYAxisGridVisible;
}

void QTBPlotTime::setYAxisGridVisible(bool yAxisGridVisible)
{
    if(mYAxisGridVisible != yAxisGridVisible) {
        mYAxisGridVisible = yAxisGridVisible;
        mAxisRect->axis(QCPAxis::atLeft)->grid()->setVisible(mYAxisGridVisible);
    }
}

QTBPlotTime::YAxisScale QTBPlotTime::yAxisScale() const
{
    return mYAxisScale;
}

void QTBPlotTime::setYAxisScale(const YAxisScale &yAxisScale)
{
    mYAxisScale = yAxisScale;
}

double QTBPlotTime::yAxisMaxCustom() const
{
    return mYAxisMaxCustom;
}

void QTBPlotTime::setYAxisMaxCustom(double yAxisMaxCustom)
{
    mYAxisMaxCustom = yAxisMaxCustom;
}

double QTBPlotTime::yAxisMinCustom() const
{
    return mYAxisMinCustom;
}

void QTBPlotTime::setYAxisMinCustom(double yAxisMinCustom)
{
    mYAxisMinCustom = yAxisMinCustom;
}

bool QTBPlotTime::xAxisTicksVisible() const
{
    return mXAxisTicksVisible;
}

void QTBPlotTime::setXAxisTicksVisible(bool xAxisTicksVisible)
{
    if(mXAxisTicksVisible != xAxisTicksVisible) {
        mXAxisTicksVisible = xAxisTicksVisible;

        if(mXAxisTicksVisible) {
            mAxisRect->axis(QCPAxis::atBottom)->setBasePen(QPen(mBoard->frontColor()));
            mAxisRect->axis(QCPAxis::atBottom)->setTickPen(QPen(mBoard->frontColor()));
            mAxisRect->axis(QCPAxis::atBottom)->setSubTickPen(QPen(mBoard->frontColor()));
        } else {
            mAxisRect->axis(QCPAxis::atBottom)->setBasePen(Qt::NoPen);
            mAxisRect->axis(QCPAxis::atBottom)->setTickPen(Qt::NoPen);
            mAxisRect->axis(QCPAxis::atBottom)->setSubTickPen(Qt::NoPen);
        }
    }
}

bool QTBPlotTime::xAxisLabelsVisible() const
{
    return mXAxisLabelsVisible;
}

void QTBPlotTime::setXAxisLabelsVisible(bool xAxisLabelsVisible)
{
    if(mXAxisLabelsVisible != xAxisLabelsVisible) {
        mXAxisLabelsVisible = xAxisLabelsVisible;
        mAxisRect->axis(QCPAxis::atBottom)->setTickLabels(mXAxisLabelsVisible);
    }
}

bool QTBPlotTime::xAxisGridVisible() const
{
    return mXAxisGridVisible;
}

void QTBPlotTime::setXAxisGridVisible(bool xAxisGridVisible)
{
    if(mXAxisGridVisible != xAxisGridVisible) {
        mXAxisGridVisible = xAxisGridVisible;
        mAxisRect->axis(QCPAxis::atBottom)->grid()->setVisible(mXAxisGridVisible);
    }
}

int QTBPlotTime::xAxisHistory() const
{
    return mXAxisHistory;
}

void QTBPlotTime::setXAxisHistory(int xAxisHistory)
{
    mXAxisHistory = xAxisHistory;
}

QTBPlotTime::XAxisDirection QTBPlotTime::xAxisDirection() const
{
    return mXAxisDirection;
}

void QTBPlotTime::setXAxisDirection(const XAxisDirection &xAxisDirection)
{
    if(mXAxisDirection != xAxisDirection) {
        mXAxisDirection = xAxisDirection;
        if(mXAxisDirection == adRightToLeft) {
            mAxisRect->axis(QCPAxis::atBottom)->setRangeReversed(false);
        } else {
            mAxisRect->axis(QCPAxis::atBottom)->setRangeReversed(true);
        }
    }
}

int QTBPlotTime::defaultWidth()
{
    return 4*mBoard->dashboardLayout()->singleElementColumnCount();
}

int QTBPlotTime::defaultHeight()
{
    return 3*mBoard->dashboardLayout()->singleElementRowCount();
}
