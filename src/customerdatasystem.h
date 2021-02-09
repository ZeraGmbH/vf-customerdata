#ifndef CUSTOMERDATASYSTEM_H
#define CUSTOMERDATASYSTEM_H

#include <ve_eventsystem.h>
#include <QJsonDocument>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QFile>
#include <QUuid>
#include <QFutureWatcher>
#include <functional>

#include <veinmoduleentity.h>
#include <veinmodulecomponent.h>
#include <veinsharedcomp.h>
#include <veinrpcfuture.h>

#include "modman_util.h"

namespace VeinComponent
{
class RemoteProcedureData;
}

namespace VfCustomerdata {



class CustomerDataSystem : public QObject
{
    Q_OBJECT
public:
    enum RPCResultCodes {
        CDS_CANCELED = -64,
        CDS_EINVAL = -EINVAL, //invalid parameters
        CDS_EEXIST = -EEXIST, //file exists (when creating files)
        CDS_ENOENT = -ENOENT, //file doesn't exist (when removing files)
        CDS_SUCCESS = 0,
        CDS_QFILEDEVICE_FILEERROR_BEGIN = QFileDevice::ReadError //if the resultCode is >= CDS_QFILEDEVICE_FILEERROR_BEGIN then it is a QFileDevice::FileError
    };

    explicit CustomerDataSystem(QString p_customerDataPath,int p_entityId=200,QObject *t_parent = nullptr);

    // EventSystem interface
public:
    /**
   * @brief Adds the CustomerData entity and the components via vein framework
   */
    void initOnce();

    VfCpp::VeinModuleEntity* entity() const;
    void setEntity(VfCpp::VeinModuleEntity* entity);

public slots:
    QVariant RPC_Open(QVariantMap p_params);
    QVariant RPC_Close(QVariantMap p_params);
    QVariant RPC_Save(QVariantMap p_params);
    QVariant RPC_AddFile(QVariantMap p_params);
    QVariant RPC_RemoveFile(QVariantMap p_params);

private:
    VfCpp::VeinModuleEntity::Ptr m_entity;

    QString m_currentCustomerFileName;
    QJsonDocument m_currentCustomerDocument;


    VfCpp::VeinSharedComp<QString> m_fileSelected;

    QMap< QString,VfCpp::VeinSharedComp<QString> > m_fileEntrieComponents;

    QString m_customerDataPath;
};

}

#endif // CUSTOMERDATASYSTEM_H
