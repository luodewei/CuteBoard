#include <QDateTime>
#include <qmath.h>
#include "dashboard/dashboard.h"
#include "project/project.h"
#include "dashboard/dashboard_parameter.h"

QTBDashboardParameter::QTBDashboardParameter(QTBoard *board):
    mParameterId(0),
    mBoard(board),
    mParameterConfigurationIsShared(false),
    mSharedParameterConfiguration(new QTBParameterConfiguration ()),
    mExclusiveParameterConfiguration(new QTBParameterConfiguration ()),
    mParameterConfiguration(new QTBParameterConfiguration ())
{

}

QTBDashboardParameter::QTBDashboardParameter(const QSharedPointer<QTBParameter>& dataParameter, QTBoard *board):
    mParameterId(0),
    mBoard(board),
    mParameterConfigurationIsShared(false),
    mExclusiveParameterConfiguration(new QTBParameterConfiguration ()),
    mParameterConfiguration(new QTBParameterConfiguration ())
{
    if(dataParameter) {
        mParameterId = dataParameter->parameterId();
        mLabel = dataParameter->label();
        mUnit = dataParameter->unit();
        mExclusiveParameterConfiguration->setLabel(mLabel);
        mParameterConfiguration = mExclusiveParameterConfiguration;
        mParameterConfiguration->defaultColorSettingsRef().setColor(mBoard->randomColor());
    }
}

QTBDashboardParameter::QTBDashboardParameter(QExplicitlySharedDataPointer<QTBParameterConfiguration> paramProperties, QTBoard *board):
    mParameterId(0),
    mBoard(board),
    mParameterConfigurationIsShared(true),
    mSharedParameterConfiguration(paramProperties),
    mExclusiveParameterConfiguration(new QTBParameterConfiguration ()),
    mParameterConfiguration(new QTBParameterConfiguration ())
{
    if(paramProperties) {
        mLabel = paramProperties->label();
        mExclusiveParameterConfiguration = paramProperties;
        mExclusiveParameterConfiguration.detach();
        mParameterConfiguration = mSharedParameterConfiguration;
    }
}

QTBDashboardParameter::QTBDashboardParameter(const QString& parameterLabel, QTBoard *board) :
    mParameterId(0),
    mBoard(board),
    mParameterConfigurationIsShared(false),
    mExclusiveParameterConfiguration(new QTBParameterConfiguration ()),
    mParameterConfiguration(new QTBParameterConfiguration ())
{
    if(!parameterLabel.isEmpty()) {
        mLabel = parameterLabel;
        mExclusiveParameterConfiguration->setLabel(mLabel);
        mParameterConfiguration = mExclusiveParameterConfiguration;
        mParameterConfiguration->defaultColorSettingsRef().setColor(mBoard->randomColor());
    }
}

QTBDashboardParameter::~QTBDashboardParameter()
= default;

QString QTBDashboardParameter::getDisplayedLabel() const
{
    if(mParameterConfiguration->userLabelEnabled()) {
        return mParameterConfiguration->userDefinedLabel();
    }
    
    return mParameterConfiguration->label();
    
}

QString QTBDashboardParameter::getDisplayedUnit() const
{
    if(mParameterConfiguration->userUnitEnabled()) {
        return mParameterConfiguration->userDefinedUnit();
    }
    
    return mUnit;
    
}

QString QTBDashboardParameter::getValueString(ValueStringFormat format) const
{
    if(mParameterId > 0) {
        switch(format) {
        case vsfDecimal:
            return mSample.value().toString(mParameterConfiguration->precision());
        case vsfHexa:
        {
            return QString("%1").arg(mSample.value().uint32_value() , 8, 16, QChar('0')).toUpper();
        }
        case vsfHexaLsb16:
        {
            struct s_fToBits
            {
                quint16     lsb;
                quint16     msb;
            };

            union int32_int16{
                s_fToBits fToBits;
                quint32 ui32;
            } toUint32{};

            toUint32.ui32 = mSample.value().uint32_value();
            quint32 value =  toUint32.fToBits.lsb;
            return QString("%1").arg(value , 4, 16, QChar('0')).toUpper();
        }
        case vsfHexaMsb16:
        {
            struct s_fToBits
            {
                quint16     lsb;
                quint16     msb;
            };

            union int32_int16{
                s_fToBits fToBits;
                quint32 ui32;
            } toUint32{};

            toUint32.ui32 = mSample.value().uint32_value();
            quint32 value =  toUint32.fToBits.msb;
            return QString("%1").arg(value , 4, 16, QChar('0')).toUpper();
        }
        }
    }
    return QString("X");
}

double QTBDashboardParameter::getValueDouble() const
{
    if(mParameterId > 0) {
        return mSample.value().toDouble();
    }
    return qQNaN();
}

quint32 QTBDashboardParameter::getValueBinaryWeight32() const
{
    if(mParameterId > 0) {
        return mSample.value().uint32_value();
    }
    return 0;
}


QString QTBDashboardParameter::getStateString() const
{
    QString text;
    if(mParameterId > 0) {
        if(mParameterConfiguration->statesSettingsRef().active()) {
            text = mParameterConfiguration->statesSettingsRef().text(mSample.value().toUint32());
        }
    }

    return text;
}

QString QTBDashboardParameter::getBitDescription(int bitNumber) const
{
    return mParameterConfiguration->bitfieldsSettingsRef().bitDescriptions().at(bitNumber);
}

bool QTBDashboardParameter::getBitLogic(int bitNumber) const
{
    return mParameterConfiguration->bitfieldsSettingsRef().bitLogics().at(bitNumber);
}

void QTBDashboardParameter::update(UpdateMode mode)
{
    if(mParameterId > 0) {
        mSample = mBoard->dataManager()->lastSample(mParameterId);
        if(mode != umValueOnly) {
            mColor = mParameterConfiguration->defaultColorSettingsRef().color();
            mForegroundColor = mParameterConfiguration->defaultColorSettingsRef().foregroundColor();
            mBackgroundBrush = mParameterConfiguration->defaultColorSettingsRef().backgroundBrush();

            if(mode == umValue) {
                bool validColor = false;
                QTBColorSettings cs;

                double value = mSample.value().toDouble();
                if(mParameterConfiguration->validRange() && mParameterConfiguration->outOfRangeColorEnabled()) {
                    if(value < mParameterConfiguration->rangeMinimum() || value > mParameterConfiguration->rangeMaximum() ) {
                        mColor = mParameterConfiguration->outOfRangeColorSettingsRef().color();
                        mForegroundColor = mParameterConfiguration->outOfRangeColorSettingsRef().foregroundColor();
                        mBackgroundBrush = mParameterConfiguration->outOfRangeColorSettingsRef().backgroundBrush();
                        validColor = true;
                    }
                }

                if(!validColor) {
                    if(mParameterConfiguration->thresholdsSettingsRef().active()) {
                        cs = mParameterConfiguration->thresholdsSettingsRef().colorSettings(value,validColor);
                        if(validColor) {
                            mColor = cs.color();
                            mForegroundColor = cs.foregroundColor();
                            mBackgroundBrush = cs.backgroundBrush();
                        }
                    }
                }
            } else {
                bool validColor = false;
                QTBColorSettings cs;

                if(mParameterConfiguration->statesSettingsRef().active()) {
                    cs = mParameterConfiguration->statesSettingsRef().colorSettings( mSample.value().toUint32(),validColor);
                    if(validColor) {
                        mColor = cs.color();
                        mForegroundColor = cs.foregroundColor();
                        mBackgroundBrush = cs.backgroundBrush();
                    }
                }
            }
        }
    } else {
        mColor = QColor(0,0,0,0);
        mForegroundColor = mColor;
        mBackgroundBrush = Qt::NoBrush;
    }
}

bool QTBDashboardParameter::configurationHasChanged()
{
    return mParameterConfiguration->modified();
}

void QTBDashboardParameter::modificationsApplied()
{
    mParameterConfiguration->setModified(false);
}

bool QTBDashboardParameter::connected() const
{
    return mParameterConfigurationIsShared;
}

void QTBDashboardParameter::disconnectSharedConfiguration()
{
    mParameterConfigurationIsShared = false;
    mSharedParameterConfiguration.reset();
    mParameterConfiguration = mExclusiveParameterConfiguration;
}

void QTBDashboardParameter::saveParameterSettings(QSettings *settings, QTBParameterConfiguration::ConfigurationMode mode)
{
    settings->setValue("Connected", mParameterConfigurationIsShared);
    if(mParameterConfigurationIsShared) {
        settings->setValue("ConnectedParamLabel", mSharedParameterConfiguration->label());
        settings->setValue("ConnectedProperties", mSharedParameterConfiguration->description());
    } else {
        mExclusiveParameterConfiguration->save(settings, mode);
    }
}

void QTBDashboardParameter::loadParameterSettings(QSettings *settings, QTBParameterConfiguration::ConfigurationMode mode)
{
    mParameterConfigurationIsShared = settings->value("Connected").toBool();
    if(mParameterConfigurationIsShared) {
        QString paramlabel = settings->value("ConnectedParamLabel").toString();
        QString properties = settings->value("ConnectedProperties").toString();

        QExplicitlySharedDataPointer<QTBParameterConfiguration> prop = mBoard->project()->parameterSettings(paramlabel, properties);
        if(prop) {
            mSharedParameterConfiguration = prop;
            mLabel = mSharedParameterConfiguration->label();
            mExclusiveParameterConfiguration = prop;
            mExclusiveParameterConfiguration.detach();
            mParameterConfigurationIsShared = true;
            mParameterConfiguration = mSharedParameterConfiguration;
        } else {
            qDebug() << "Cant find Param Config" << paramlabel << properties;
            mParameterConfigurationIsShared = false;
            mLabel = paramlabel;
            mExclusiveParameterConfiguration->setLabel(mLabel);
            mParameterConfiguration = mExclusiveParameterConfiguration;
            mParameterConfiguration->defaultColorSettingsRef().setColor(mBoard->randomColor());
        }
    } else {
        if(!mExclusiveParameterConfiguration)
            mExclusiveParameterConfiguration = QExplicitlySharedDataPointer<QTBParameterConfiguration>(new QTBParameterConfiguration());
        mExclusiveParameterConfiguration->load(settings, mode);
        mLabel = mExclusiveParameterConfiguration->label();
        mParameterConfigurationIsShared = false;
        mParameterConfiguration = mExclusiveParameterConfiguration;
    }
}

QExplicitlySharedDataPointer<QTBParameterConfiguration> QTBDashboardParameter::sharedParameterConfiguration() const
{
    return mSharedParameterConfiguration;
}

QExplicitlySharedDataPointer<QTBParameterConfiguration> QTBDashboardParameter::exclusiveParameterConfiguration() const
{
    return mExclusiveParameterConfiguration;
}

double QTBDashboardParameter::getTimestamp() const
{
    if(mParameterId > 0) {
        return mSample.datationSecFromMidnight();
    }
    return 0;
}

QColor QTBDashboardParameter::getColor() const
{
    return mColor;
}

QColor QTBDashboardParameter::getForegroundColor() const
{
    return mForegroundColor;
}

QBrush QTBDashboardParameter::getBackgroundBrush() const
{
    return mBackgroundBrush;
}

QString QTBDashboardParameter::getLabel() const
{
    return mLabel;
}

void QTBDashboardParameter::setLabel(const QString &label)
{
    mLabel = label;
}

QString QTBDashboardParameter::getUnit() const
{
    return mUnit;
}

void QTBDashboardParameter::setUnit(const QString &unit)
{
    mUnit = unit;
}

quint32 QTBDashboardParameter::getParameterId() const
{
    return mParameterId;
}

void QTBDashboardParameter::setParameterId(const quint32 &parameterId)
{
    mParameterId = parameterId;
}

QExplicitlySharedDataPointer<QTBParameterConfiguration> QTBDashboardParameter::parameterConfiguration() const
{
    return mParameterConfiguration;
}
